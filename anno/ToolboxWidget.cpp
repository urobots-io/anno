#include "ToolboxWidget.h"
#include <QMenu>

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

    ui.treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui.treeView, &QWidget::customContextMenuRequested, this, &ToolboxWidget::OnCustomContextMenu);

    marker_menu_ = new QMenu(topLevelWidget());
    AddAction(marker_menu_, QString(), tr("Rename"), &ToolboxWidget::RenameItem);
    AddAction(marker_menu_, "add.ico", tr("Add Category"), &ToolboxWidget::AddCategory);
    marker_menu_->addSeparator();
    AddAction(marker_menu_, "delete.ico", tr("Delete"), &ToolboxWidget::DeleteMarker);
    AddAction(marker_menu_, "delete.ico", tr("Delete from images"), &ToolboxWidget::DeleteMarkerFromImages);

    category_menu_ = new QMenu(topLevelWidget());
    AddAction(category_menu_, QString(), tr("Rename"), &ToolboxWidget::RenameItem);
    category_menu_->addSeparator();
    AddAction(category_menu_, "delete.ico", tr("Delete"), &ToolboxWidget::DeleteCategory);
    AddAction(category_menu_, "delete.ico", tr("Delete from images"), &ToolboxWidget::DeleteCategoryFromImages);
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
    CleanupSelection();    

    proxy_.setSourceModel(nullptr);    

	definitions_ = model;

    proxy_.setSourceModel(model.get());

    ui.treeView->expandAll();

    CleanupSelection();

    ui.treeView->setEnabled(model.get());
    ui.add_marker_type_pushButton->setEnabled(model.get());
}

void ToolboxWidget::OnCurrentChanged(const QModelIndex &current, const QModelIndex &previous) {
    Q_UNUSED(previous);
    auto index = proxy_.mapToSource(current);
    auto def = definitions_->GetDefinition(index);
    auto cat = definitions_->GetCategory(index);
#ifdef _DEBUG
    if (def) qDebug(QString("Selected Type %0").arg(def->type_name).toLatin1());
    if (cat) qDebug(QString("Selected Category %0").arg(cat->name).toLatin1());
#endif
    emit SelectionChanged(def, cat);
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

void ToolboxWidget::DeleteMarker() {
    if (definitions_) {
        if (auto marker = definitions_->GetDefinition(menu_index_)) {
            DeleteRequested(marker, nullptr, false);
        }
    }
}

void ToolboxWidget::DeleteMarkerFromImages() {
    if (definitions_) {
        if (auto marker = definitions_->GetDefinition(menu_index_)) {
            DeleteRequested(marker, nullptr, true);
        }
    }
}

void ToolboxWidget::DeleteCategory() {
    if (definitions_) {
        if (auto category = definitions_->GetCategory(menu_index_)) {
            DeleteRequested(nullptr, category, false);
        }
    }
}

void ToolboxWidget::DeleteCategoryFromImages() {
    if (definitions_) {
        if (auto category = definitions_->GetCategory(menu_index_)) {
            DeleteRequested(nullptr, category, false);
        }
    }
}

