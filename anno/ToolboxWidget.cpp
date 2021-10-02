// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
//
// Anno Labeling Tool
// 2020-2021 (c) urobots GmbH, https://urobots.io

#include "ToolboxWidget.h"
#include "messagebox.h"
#include <QDebug>
#include <QMenu>
#include <LabelDefinitionPropertiesDialog.h>

ToolboxWidget::ToolboxWidget(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
    ui.treeView->setModel(&proxy_);
    connect(ui.treeView, &QTreeView::doubleClicked, this, &ToolboxWidget::OnItemDoubleClick);    
    connect(
        ui.treeView->selectionModel(),
        &QItemSelectionModel::currentChanged,
        this,
        &ToolboxWidget::OnCurrentChanged);
    connect(&proxy_, &QAbstractItemModel::rowsInserted, this, &ToolboxWidget::OnRowsAdded);
    connect(ui.add_marker_type_pushButton, &QPushButton::clicked, this, &ToolboxWidget::ShowAddMarkerMenu);
    connect(ui.properties_pushButton, &QPushButton::clicked, this, &ToolboxWidget::ShowLabelDefinitionProperties);
    ui.properties_pushButton->setEnabled(false);

    connect(ui.toggle_tree_state_pushButton, &QPushButton::clicked, this, &ToolboxWidget::ToggleTreeOpenState);

    ui.treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui.treeView, &QWidget::customContextMenuRequested, this, &ToolboxWidget::OnCustomContextMenu);

    marker_menu_ = new QMenu(topLevelWidget());
    AddAction(marker_menu_, "rename.ico", tr("Rename"), &ToolboxWidget::RenameItem);
    AddAction(marker_menu_, "add.ico", tr("Add Category"), &ToolboxWidget::AddCategory);
    marker_menu_->addSeparator();
    AddAction(marker_menu_, "copy.ico", tr("Clone"), &ToolboxWidget::CloneMarker);
    marker_menu_->addSeparator();
    AddAction(marker_menu_, "delete.ico", tr("Delete"), &ToolboxWidget::DeleteMarker);
    AddAction(marker_menu_, "clean.ico", tr("Remove from images"), &ToolboxWidget::DeleteMarkerFromImages);

    category_menu_ = new QMenu(topLevelWidget());
    AddAction(category_menu_, "rename.ico", tr("Rename"), &ToolboxWidget::RenameItem);
    category_menu_->addSeparator();
    AddAction(category_menu_, "delete.ico", tr("Delete"), &ToolboxWidget::DeleteCategory);
    AddAction(category_menu_, "clean.ico", tr("Remove from images"), &ToolboxWidget::DeleteCategoryFromImages);
}

ToolboxWidget::~ToolboxWidget()
{
}

template<class T>
void ToolboxWidget::AddAction(QMenu *menu, QString icon, QString text, T callback) {
    auto action = icon.isEmpty()
        ? new QAction(text, menu)
        : new QAction(QIcon(":/MainWindow/Resources/" + icon), text, menu);
    connect(action, &QAction::triggered, this, callback);
    menu->addAction(action);
}

void ToolboxWidget::SetDefinitionsModel(std::shared_ptr<LabelDefinitionsTreeModel> model) {
    if (definitions_) {
        definitions_->disconnect(this);
    }

    CleanupSelection();    

    proxy_.setSourceModel(nullptr);    

	definitions_ = model;

    if (definitions_) {
        connect(definitions_.get(), &LabelDefinitionsTreeModel::Error, this, &ToolboxWidget::OnError);
    }

    proxy_.setSourceModel(model.get());

    ui.treeView->expandAll();

    CleanupSelection();

    ui.treeView->setEnabled(model.get());
    ui.add_marker_type_pushButton->setEnabled(model.get());
}

void ToolboxWidget::OnCurrentChanged(const QModelIndex &current, const QModelIndex &previous) {
    Q_UNUSED(previous);
    if (!current.isValid()) {
        return;
    }

    auto index = proxy_.mapToSource(current);
    auto def = definitions_->GetDefinition(index);
    auto cat = definitions_->GetCategory(index);

#ifdef _DEBUG
    if (def) qDebug() << "Selected Definition: " << def->type_name;
    if (cat) qDebug() << "Selected Category: " << cat->get_name();
#endif

    bool enabled = def && !cat;
    ui.properties_pushButton->setEnabled(enabled);
    qDebug() << enabled;

    emit SelectionChanged(def, cat);
}

void ToolboxWidget::ShowLabelDefinitionProperties() {
    auto current = ui.treeView->selectionModel()->currentIndex();
    if (!current.isValid()) {
        return;
    }

    auto index = proxy_.mapToSource(current);
    auto def = definitions_->GetDefinition(index);

    LabelDefinitionPropertiesDialog dialog(def, definitions_, this);
    dialog.exec();
}

void ToolboxWidget::SetFile(std::shared_ptr<FileModel> file) {
    proxy_.SetFilterFileModel(file);    
}

void ToolboxWidget::EnableFileFilter(bool value) {
    proxy_.EnableFileFilter(value);
}

void ToolboxWidget::CleanupSelection() {
	if (definitions_) {
	    ui.treeView->selectionModel()->clearCurrentIndex();
	    ui.treeView->selectionModel()->clearSelection();
        auto index = proxy_.mapFromSource(definitions_->GetSelectModeIndex());
	    ui.treeView->selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);
	}
}

void ToolboxWidget::OnItemDoubleClick(const QModelIndex & mindex) {
    auto category = definitions_->GetCategory(proxy_.mapToSource(mindex));
    if (category) {
        emit DoubleClick(category);
    }
}

void ToolboxWidget::OnRowsAdded(const QModelIndex &parent, int first, int last) {
    Q_UNUSED(parent);
    Q_UNUSED(first);
    Q_UNUSED(last);

    for (int i = first; i <= last; ++i) {        
        ui.treeView->expand(proxy_.index(i, 0, parent));
    }
}

void ToolboxWidget::ShowAddMarkerMenu() {
    QMenu context_menu(this);

    for (int i = int(LabelType::circle); i < int(LabelType::max_types); ++i) {
        auto type_name = LabelTypeToString(LabelType(i));
        auto a = new QAction(tr(type_name.toLatin1()), &context_menu);
        a->setObjectName(type_name);
        connect(a, &QAction::triggered, this, &ToolboxWidget::AddMarkerType);                        
        context_menu.addAction(a);
    }

    context_menu.exec(ui.widget->mapToGlobal(ui.add_marker_type_pushButton->geometry().bottomRight()));
}

void ToolboxWidget::MakeCurrent(QModelIndex index, bool start_editing) {
    ui.treeView->selectionModel()->clearCurrentIndex();
    ui.treeView->selectionModel()->clearSelection();
    ui.treeView->selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);
    if (start_editing) {
        ui.treeView->scrollTo(index);
        ui.treeView->edit(index);
    }
    OnCurrentChanged(index, QModelIndex());
}

void ToolboxWidget::AddMarkerType() {
    auto type = LabelTypeFromString(((QAction*)sender())->objectName());
    if (definitions_) {
        auto index = definitions_->CreateMarkerType(type);
        if (index.isValid()) {
            MakeCurrent(proxy_.mapFromSource(index), true);            
        }
    }
}

void ToolboxWidget::OnCustomContextMenu(const QPoint &point) {
    menu_index_ = proxy_.mapToSource(ui.treeView->indexAt(point));
    if (!menu_index_.isValid() || (!menu_index_.parent().isValid() && menu_index_.row() == 0)) {
        return;
    }

    auto is_marker = ((LabelDefinitionsTreeModel*)proxy_.sourceModel())->GetDefinition(menu_index_) != nullptr;
    if (auto menu = (is_marker ? marker_menu_ : category_menu_)) {
        menu->popup(ui.treeView->viewport()->mapToGlobal(point));
    }
}

void ToolboxWidget::RenameItem() {
    if (menu_index_.isValid()) {
        ui.treeView->edit(proxy_.mapFromSource(menu_index_));
    }
}

void ToolboxWidget::AddCategory() {
    if (!menu_index_.isValid()) {
        return;
    }

    if (definitions_) {
        auto index = definitions_->CreateCategory(menu_index_);
        if (index.isValid()) {
            MakeCurrent(proxy_.mapFromSource(index), true);
        }
    }
}

void ToolboxWidget::CloneMarker() {
    if (definitions_) {
        if (auto marker = definitions_->GetDefinition(menu_index_)) {
            auto index = definitions_->CloneDefinition(marker);
            if (index.isValid()) {                
                ui.treeView->edit(proxy_.mapFromSource(index));
            }
        }
    }
}

void ToolboxWidget::DeleteMarker() {
    if (definitions_) {
        if (auto marker = definitions_->GetDefinition(menu_index_)) {
            emit DeleteRequested(marker, nullptr, false);
        }
    }
}

void ToolboxWidget::DeleteMarkerFromImages() {
    if (definitions_) {
        if (auto marker = definitions_->GetDefinition(menu_index_)) {
            emit DeleteRequested(marker, nullptr, true);
        }
    }
}

void ToolboxWidget::DeleteCategory() {
    if (definitions_) {
        if (auto category = definitions_->GetCategory(menu_index_)) {
            emit DeleteRequested(nullptr, category, false);
        }
    }
}

void ToolboxWidget::DeleteCategoryFromImages() {
    if (definitions_) {
        if (auto category = definitions_->GetCategory(menu_index_)) {
            emit DeleteRequested(nullptr, category, true);
        }
    }
}

void ToolboxWidget::OnError(QString message) {
    urobots::qt_helpers::messagebox::Critical(message);
}


void ToolboxWidget::ToggleTreeOpenState() {
    if (!definitions_) {
        return;
    }

    int num_opened = 0;
    int num_closed = 0;
    for (auto d : definitions_->GetDefinitions()) {
        auto index = proxy_.mapFromSource(definitions_->GetIndex(d.get()));
        if (ui.treeView->isExpanded(index)) {
            ++num_opened;
        }
        else {
            ++num_closed;
        }

    }

    bool close = num_opened > num_closed;
    if (num_opened == num_closed) {
        close = toggle_closes_;
        toggle_closes_ = !toggle_closes_;
    }
    
    if (close) {
        ui.treeView->collapseAll();
    }
    else {
        ui.treeView->expandAll();
    }    
}

