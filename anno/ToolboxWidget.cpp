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

