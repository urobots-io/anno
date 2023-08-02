// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
//
// Anno Labeling Tool
// 2020-2023 (c) urobots GmbH, https://urobots.io

#include "ToolboxProxyModel.h"

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

LabelDefinitionsTreeModel* ToolboxProxyModel::SourceModel() const {
    return static_cast<LabelDefinitionsTreeModel*>(sourceModel());
}

bool ToolboxProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const {
    if (!filter_enabled_ || !file_) {
        return true;
    }

    if (auto definitions = SourceModel()) {
        QModelIndex index = definitions->index(sourceRow, 0, sourceParent);
        auto definition = definitions->GetDefinition(index);
        return !definition || definition->AllowedForFile(file_.get());
    }

    return true;
}

