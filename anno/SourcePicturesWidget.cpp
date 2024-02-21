// Anno Labeling Tool
// 2020-2024 (c) urobots GmbH, https://urobots.io/en/portfolio/anno/

#include "SourcePicturesWidget.h"
#include "messagebox.h"
#include <QInputDialog>
#include <QMenu>
#include <QRegExp>
#include <QShortcut>
#include <QSortFilterProxyModel>
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
    sort_filter_model_->setRecursiveFilteringEnabled(true);
    ui.treeView->setModel(sort_filter_model_);

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
}

void SourcePicturesWidget::OnTextFilterChanged() {
    auto text = ui.filter_lineEdit->text();
    if (text.length() && !tree_model_->IsCompletelyLoaded()) {
        tree_model_->LoadCompletely();
    }

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    sort_filter_model_->setFilterRegExp(QRegExp(text, Qt::CaseInsensitive,  QRegExp::Wildcard));
#else
    // TODO(ap): qt6
#endif
}

void SourcePicturesWidget::OnNavigationPathChanged(QString path) {
    if (!is_tree_callback_) {
        is_navigation_callback_ = true;

        if (!path.isEmpty() && path[0] == '/') {
            path.removeAt(0);
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
