// Anno Labeling Tool
// 2020-2024 (c) urobots GmbH, https://urobots.io/en/portfolio/anno/

#include "SourcePicturesWidget.h"
#include "messagebox.h"
#include <QHBoxLayout>
#include <QInputDialog>
#include <QMenu>
#include <QPushButton>
#include <QRegExp>
#include <QRegularExpression>
#include <QShortcut>
#include <QSortFilterProxyModel>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidgetAction>
#include <map>
#include <set>
#ifdef Q_OS_WIN
#include <Shlobj.h>
#endif
#ifdef Q_OS_MAC
#include <QProcess>
#endif

SourcePicturesWidget::SourcePicturesWidget(QWidget *parent)
    : QWidget(parent)
{
	ui.setupUi(this);
    setBackgroundRole(QPalette::Mid);
}

void SourcePicturesWidget::Init(ApplicationModel *model) {
    model_ = model;

    ui.treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui.treeView, &QWidget::customContextMenuRequested, 
        this, &SourcePicturesWidget::OnCustomContextMenu);

    // initialize filter
    sort_filter_model_ = new SourcePicturesProxyModel(this);
    ui.treeView->setModel(sort_filter_model_);
    InitTypesFilter();

    connect(ui.filter_lineEdit, &QLineEdit::textChanged, this, &SourcePicturesWidget::OnTextFilterChanged);
    connect(ui.filter_labeled_checkBox, &QCheckBox::stateChanged, this, &SourcePicturesWidget::OnLoadedFilterChanged);

    OnNewFileSystem(model_->get_filesystem());
	
	connect(
		model_, &ApplicationModel::filesystem_changed,
		this, &SourcePicturesWidget::OnNewFileSystem);

    new QShortcut(QKeySequence(Qt::Key_PageDown), this, SLOT(SelectNextFile()));
    new QShortcut(QKeySequence(Qt::Key_PageUp), this, SLOT(SelectPreviousFile()));

    ui.treeView->setAcceptDrops(true);

    // file menu
    file_menu_ = new QMenu(topLevelWidget());
    AddAction(file_menu_, QString(), tr("Rename"), &SourcePicturesWidget::OnRenameFile);
    AddAction(file_menu_, QString(), tr("Reveal in Explorer"), &SourcePicturesWidget::OnRevealInExplorer);
    file_menu_->addSeparator();
    AddAction(file_menu_, "delete.ico", tr("Delete file"), &SourcePicturesWidget::OnDeleteFile);
    AddAction(file_menu_, "clean.ico", tr("Remove markers"), &SourcePicturesWidget::OnRemoveMarkers);        
    
    // folder menu
    folder_menu_ = new QMenu(topLevelWidget());
    rename_folder_action_ = AddAction(folder_menu_, QString(), tr("Rename"), &SourcePicturesWidget::OnRenameFile);
    AddAction(folder_menu_, QString(), tr("Reveal in Explorer"), &SourcePicturesWidget::OnRevealInExplorer);
    folder_menu_->addSeparator();
    AddAction(folder_menu_, "refresh.ico", tr("Reload folder"), &SourcePicturesWidget::OnReloadFolder);    
    AddAction(folder_menu_, "add.ico", tr("Create subfolder..."), &SourcePicturesWidget::OnCreateFolder);
    folder_menu_->addSeparator();
    delete_folder_action_ = AddAction(folder_menu_, "delete.ico", tr("Delete folder"), &SourcePicturesWidget::OnDeleteFile);
    AddAction(folder_menu_, "clean.ico", tr("Remove markers"), &SourcePicturesWidget::OnRemoveMarkers);


    connect(model_->get_navigation_model(), &NavigationModel::current_path_changed, this, &SourcePicturesWidget::OnNavigationPathChanged);
}

SourcePicturesWidget::~SourcePicturesWidget()
{
}

template<class T>
QAction* SourcePicturesWidget::AddAction(QMenu *menu, QString icon, QString text, T callback) {
    auto action = icon.isEmpty() 
        ? new QAction(text, menu)
        : new QAction(QIcon(":/MainWindow/Resources/" + icon), text, menu);
    connect(action, &QAction::triggered, this, callback);
    menu->addAction(action);
    return action;
}

void SourcePicturesWidget::OnLoadedFilterChanged(bool value) {
    if (!tree_model_->IsCompletelyLoaded() && value) {
        tree_model_->LoadCompletely();
    }

    sort_filter_model_->set_show_labeled_only(value);
    ui.filter_types_toolButton->setEnabled(value);
}

void SourcePicturesWidget::InitTypesFilter() {
    // popup content: tree of marker types and their categories + All / None buttons
    auto popup = new QWidget(this);
    auto layout = new QVBoxLayout(popup);
    layout->setContentsMargins(4, 4, 4, 4);
    layout->setSpacing(3);

    types_filter_tree_ = new QTreeWidget(popup);
    types_filter_tree_->setHeaderHidden(true);
    types_filter_tree_->setSelectionMode(QAbstractItemView::NoSelection);
    types_filter_tree_->setMinimumSize(240, 220);
    layout->addWidget(types_filter_tree_);

    auto buttons_layout = new QHBoxLayout();
    auto all_button = new QPushButton(tr("All"), popup);
    auto none_button = new QPushButton(tr("None"), popup);
    buttons_layout->addWidget(all_button);
    buttons_layout->addWidget(none_button);
    buttons_layout->addStretch();
    layout->addLayout(buttons_layout);

    auto menu = new QMenu(this);
    auto action = new QWidgetAction(menu);
    action->setDefaultWidget(popup);
    menu->addAction(action);
    ui.filter_types_toolButton->setMenu(menu);

    connect(menu, &QMenu::aboutToShow, this, &SourcePicturesWidget::RebuildTypesFilterTree);
    connect(all_button, &QPushButton::clicked, this, [this] { SetAllTypesFilterItems(true); });
    connect(none_button, &QPushButton::clicked, this, [this] { SetAllTypesFilterItems(false); });

    connect(types_filter_tree_, &QTreeWidget::itemChanged, this, [this](QTreeWidgetItem *, int) {
        if (types_filter_tree_updating_ || types_filter_apply_pending_) {
            return;
        }
        // checking a marker type changes all its categories, apply them at once
        types_filter_apply_pending_ = true;
        QTimer::singleShot(0, this, [this] {
            types_filter_apply_pending_ = false;
            ApplyTypesFilter();
        });
    });

    connect(model_, &ApplicationModel::label_definitions_changed, this, [this] { UpdateTypesFilterButton(); });
}

void SourcePicturesWidget::RebuildTypesFilterTree() {
    types_filter_tree_updating_ = true;
    types_filter_tree_->clear();

    // number of images with markers of every type and category
    std::map<QString, int> type_counts;
    std::map<std::pair<QString, int>, int> category_counts;
    for (const auto &file : model_->GetFileModels()) {
        std::set<QString> types;
        std::set<std::pair<QString, int>> categories;
        for (const auto &label : file.second->labels_) {
            auto category = label->GetCategory();
            auto definition = category ? category->GetDefinition() : nullptr;
            if (definition) {
                types.insert(definition->get_type_name());
                categories.insert({ definition->get_type_name(), category->get_value() });
            }
        }
        for (const auto &t : types) ++type_counts[t];
        for (const auto &c : categories) ++category_counts[c];
    }

    const auto &excluded = sort_filter_model_->GetExcludedCategories();
    if (auto definitions = model_->get_label_definitions()) {
        for (const auto &definition : definitions->GetDefinitions()) {
            if (definition->categories.empty()) {
                continue;
            }

            auto type_name = definition->get_type_name();
            auto type_item = new QTreeWidgetItem(types_filter_tree_);
            type_item->setText(0, QString("%0 (%1)").arg(type_name).arg(type_counts[type_name]));
            type_item->setFlags(type_item->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsAutoTristate);

            for (const auto &category : definition->categories) {
                auto key = std::make_pair(type_name, category->get_value());
                auto item = new QTreeWidgetItem(type_item);
                item->setText(0, QString("%0 (%1)").arg(category->get_name()).arg(category_counts[key]));
                item->setIcon(0, LabelCategory::CreateColorIcon(category->get_color()));
                item->setData(0, Qt::UserRole, type_name);
                item->setData(0, Qt::UserRole + 1, category->get_value());
                item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
                item->setCheckState(0, excluded.count(key) ? Qt::Unchecked : Qt::Checked);
            }
        }
    }

    types_filter_tree_->expandAll();
    types_filter_tree_updating_ = false;
}

void SourcePicturesWidget::SetAllTypesFilterItems(bool checked) {
    types_filter_tree_updating_ = true;
    for (int i = 0; i < types_filter_tree_->topLevelItemCount(); ++i) {
        auto type_item = types_filter_tree_->topLevelItem(i);
        for (int j = 0; j < type_item->childCount(); ++j) {
            type_item->child(j)->setCheckState(0, checked ? Qt::Checked : Qt::Unchecked);
        }
    }
    types_filter_tree_updating_ = false;
    ApplyTypesFilter();
}

void SourcePicturesWidget::ApplyTypesFilter() {
    std::set<std::pair<QString, int>> excluded;
    for (int i = 0; i < types_filter_tree_->topLevelItemCount(); ++i) {
        auto type_item = types_filter_tree_->topLevelItem(i);
        for (int j = 0; j < type_item->childCount(); ++j) {
            auto item = type_item->child(j);
            if (item->checkState(0) != Qt::Checked) {
                excluded.insert({ item->data(0, Qt::UserRole).toString(), item->data(0, Qt::UserRole + 1).toInt() });
            }
        }
    }

    sort_filter_model_->SetExcludedCategories(excluded);
    UpdateTypesFilterButton();
}

void SourcePicturesWidget::UpdateTypesFilterButton() {
    int total = 0;
    int selected = 0;
    const auto &excluded = sort_filter_model_->GetExcludedCategories();
    if (auto definitions = model_->get_label_definitions()) {
        for (const auto &definition : definitions->GetDefinitions()) {
            for (const auto &category : definition->categories) {
                ++total;
                if (!excluded.count({ definition->get_type_name(), category->get_value() })) {
                    ++selected;
                }
            }
        }
    }

    QString text;
    if (selected == total) {
        text = tr("All");
    }
    else if (selected == 0) {
        text = tr("None");
    }
    else {
        text = tr("%0 of %1").arg(selected).arg(total);
    }
    ui.filter_types_toolButton->setText(text);
}

void SourcePicturesWidget::OnTextFilterChanged() {
    auto text = ui.filter_lineEdit->text();
    if (text.length() && !tree_model_->IsCompletelyLoaded()) {
        tree_model_->LoadCompletely();
    }

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    sort_filter_model_->setFilterRegExp(QRegExp(text, Qt::CaseInsensitive,  QRegExp::Wildcard));
#else
    if (text.isEmpty()) {
        sort_filter_model_->setFilterRegularExpression("");
    }
    else {
        if (!text.contains("*")) {
            text = "*" + text + "*";
        }
        sort_filter_model_->setFilterRegularExpression(QRegularExpression::fromWildcard(text, Qt::CaseInsensitive));
    }
#endif
}

void SourcePicturesWidget::OnNavigationPathChanged(QString path) {
    if (!is_tree_callback_) {
        is_navigation_callback_ = true;

        if (!path.isEmpty() && path[0] == '/') {
            path.remove(0, 1);
        }

        auto index = tree_model_->index(path);
        if (index.isValid()) {           
            ui.treeView->setCurrentIndex(sort_filter_model_->mapFromSource(index));
        }

        is_navigation_callback_ = false;
    }
}

void SourcePicturesWidget::OnCurrentChanged(const QModelIndex &current, const QModelIndex &previous) {
    Q_UNUSED(previous)
    if (tree_model_) {
        auto index = sort_filter_model_->mapToSource(current);
        auto file_info = tree_model_->GetFileInfo(index);
        auto file = tree_model_->GetFileModel(index);
        emit FileModelSelected(file);

        if (!is_navigation_callback_) {
            is_tree_callback_ = true;
            model_->get_navigation_model()->SetPath("/" + file_info.name);
            is_tree_callback_ = false;
        }
    }
    else {
        emit FileModelSelected({});

        if (!is_navigation_callback_) {
            is_tree_callback_ = true;
            model_->get_navigation_model()->SetPath("/");
            is_tree_callback_ = false;
        }
    }
}

void SourcePicturesWidget::OnNewFileSystem(std::shared_ptr<FilesystemInterface> filesystem) {
    ui.treeView->selectionModel()->clearCurrentIndex();
    ui.treeView->selectionModel()->clearSelection();

    if (tree_model_) {
        delete tree_model_;
    }    

    tree_model_ = new SourcePicturesTreeModel(filesystem, model_, this);    

    sort_filter_model_->setSourceModel(tree_model_);

    ui.treeView->selectionModel()->disconnect(this);

	connect(ui.treeView->selectionModel(), &QItemSelectionModel::currentChanged,
		this, &SourcePicturesWidget::OnCurrentChanged);

    ui.treeView->setEnabled(filesystem != nullptr);

    if (filesystem) {
        ui.treeView->expand(sort_filter_model_->mapFromSource(tree_model_->GetFilesRootIndex()));
    }

    if (ui.filter_labeled_checkBox->isChecked()) {
        ui.filter_labeled_checkBox->setChecked(false);
    }

    // the marker types filter is not stored, reset it for every project
    sort_filter_model_->SetExcludedCategories({});
    UpdateTypesFilterButton();
}

void SourcePicturesWidget::SelectFile(int offset) {
    auto model = static_cast<SourcePicturesTreeModel*>(ui.treeView->model());
    if (!model)
        return;

    auto selection_model = ui.treeView->selectionModel();
    auto indexes = selection_model->selectedIndexes();

    QModelIndex next_index;
    if (indexes.size()) {
        auto i = indexes.last();
        next_index = model->index(i.row() + offset, 0, i.parent());
    }
    else {
        // 1st child of the root element
        // xxx next_index = model->root_path_index_.child(0, 0);
    }

    if (next_index.isValid()) {
        ui.treeView->setCurrentIndex(next_index);
    }
}

void SourcePicturesWidget::SelectNextFile() {
    SelectFile(1);
}

void SourcePicturesWidget::SelectPreviousFile() {
    SelectFile(-1);
}

void SourcePicturesWidget::OnCustomContextMenu(const QPoint &point) {
    menu_index_ = sort_filter_model_->mapToSource(ui.treeView->indexAt(point));
    if (!menu_index_.isValid()) {
        return;
    }

    auto info = tree_model_->GetFileInfo(menu_index_);
    auto menu = (info.is_folder ? folder_menu_ : file_menu_);

    if (info.is_folder) {
        bool is_root_folder = info.name.isEmpty();
        if (delete_folder_action_) delete_folder_action_->setEnabled(!is_root_folder);
        if (rename_folder_action_) rename_folder_action_->setEnabled(!is_root_folder);
    }

    menu->popup(ui.treeView->viewport()->mapToGlobal(point));
}

void SourcePicturesWidget::OnDeleteFile() {
    if (menu_index_.isValid()) {
        QString error, title, question;

        auto info = tree_model_->GetFileInfo(menu_index_);
        if (info.is_folder) {
            question = tr("Permanently delete folder \"%0\" and its content?").arg(info.name);
            title = tr("Delete folder");
        }
        else {
            question = tr("Permanently delete \"%0\" and its markers?").arg(info.name);
            title = tr("Delete file");
        }
        
        if (messagebox::Question(question, title)) {
            if (!tree_model_->Remove(menu_index_, error)) {
                messagebox::Critical(error);
            }
        }
    }
}

void SourcePicturesWidget::OnRenameFile() {
    if (menu_index_.isValid()) {
        ui.treeView->edit(sort_filter_model_->mapFromSource(menu_index_));
    }
}

void SourcePicturesWidget::OnReloadFolder() {
    if (menu_index_.isValid()) {       
        tree_model_->ReloadFolder(menu_index_,-1,0);
    }
}

void SourcePicturesWidget::OnRevealInExplorer() {
    if (!menu_index_.isValid()) {
        return;
    }
    auto info = tree_model_->GetFileInfo(menu_index_);
    auto local_filename = tree_model_->GetFileSystem()->GetLocalPath(info.name);
    if (!local_filename.length()) {
        return;
    }

#ifdef Q_OS_WIN    
    CoInitialize(0);
    auto wname = QFileInfo(local_filename).absoluteFilePath().replace("/", "\\").toStdWString();
    auto pidl = ILCreateFromPath(wname.c_str());
    if (pidl) {
        SHOpenFolderAndSelectItems(pidl, 0, 0, 0);
        ILFree(pidl);
    }
    CoUninitialize();
#endif

#ifdef Q_OS_MAC
    QStringList script_args;
    script_args << QLatin1String("-e")
            << QString::fromLatin1("tell application \"Finder\" to reveal POSIX file \"%1\"")
            .arg(local_filename);
    QProcess::execute(QLatin1String("/usr/bin/osascript"), script_args);
    script_args.clear();
    script_args << QLatin1String("-e")
            << QLatin1String("tell application \"Finder\" to activate");
    QProcess::execute("/usr/bin/osascript", script_args);
#endif

}

void SourcePicturesWidget::OnRemoveMarkers() {
    if (menu_index_.isValid()) {
        tree_model_->RemoveMarkers(menu_index_);
    }    
}

void SourcePicturesWidget::OnCreateFolder() {
    bool ok;
    QString name = QInputDialog::getText(this,
        tr("Create subfolder"),
        tr("Name:"),
        QLineEdit::Normal,
        QString(),
        &ok);

    if (ok) {
        if (!tree_model_->CreateSubfolder(menu_index_, name)) {
            messagebox::Critical(tr("Failed to create folder"));
        }
    }
}
