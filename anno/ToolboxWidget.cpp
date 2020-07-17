#include "ToolboxWidget.h"

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
}

ToolboxWidget::~ToolboxWidget()
{
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
    Q_UNUSED(previous)
    auto index = proxy_.mapToSource(current);
    emit SelectionChanged(definitions_->GetDefinition(index), definitions_->GetCategory(index));
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
    Q_UNUSED(parent)
    Q_UNUSED(first)
    Q_UNUSED(last)
    ui.treeView->expandAll();
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

void ToolboxWidget::AddMarkerType() {
    auto type = LabelTypeFromString(((QAction*)sender())->objectName());
    auto definitions = (LabelDefinitionsTreeModel*)proxy_.sourceModel();
    definitions->CreateMarkerType(type);
}

ToolboxProxyModel::ToolboxProxyModel(QObject *parent)
: QSortFilterProxyModel(parent)
{
}

void ToolboxProxyModel::SetFilterFileModel(std::shared_ptr<FileModel> file) {
    file_ = file;
    invalidateFilter();
}

void ToolboxProxyModel::EnableFileFilter(bool value) {
    filter_enabled_ = value;
    invalidateFilter();
}

bool ToolboxProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const {
    if (!filter_enabled_ || !file_) {
        return true;
    }

    auto definitions = (LabelDefinitionsTreeModel*)sourceModel();
    QModelIndex index = definitions->index(sourceRow, 0, sourceParent);
    auto definition = definitions->GetDefinition(index);
    return !definition || definition->AllowedForFile(file_.get());
}
