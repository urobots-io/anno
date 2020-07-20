#pragma once
#include "FileModel.h"
#include "LabelDefinitionsTreeModel.h"
#include <QSortFilterProxyModel>

class ToolboxProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    ToolboxProxyModel(QObject *parent = 0);

    void SetFilterFileModel(std::shared_ptr<FileModel>);
    void EnableFileFilter(bool);
    
protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

private:
    LabelDefinitionsTreeModel* SourceModel() const;

private:
    std::shared_ptr<FileModel> file_;
    bool filter_enabled_ = true;
};
