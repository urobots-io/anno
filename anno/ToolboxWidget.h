#pragma once
#include "FileModel.h"
#include "LabelDefinitionsTreeModel.h"
#include "ui_ToolboxWidget.h"
#include <QSortFilterProxyModel>
#include <QWidget>

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
    std::shared_ptr<FileModel> file_;
    bool filter_enabled_ = true;
};

class ToolboxWidget : public QWidget
{
	Q_OBJECT

public:
	ToolboxWidget(QWidget *parent);
	~ToolboxWidget();

signals:
    void SelectionChanged(LabelDefinition*, LabelCategory*);
    void DoubleClick(LabelCategory*);

public slots:
    void SetDefinitionsModel(std::shared_ptr<LabelDefinitionsTreeModel>);
    void SetFile(std::shared_ptr<FileModel>);
    void CleanupSelection();
    void EnableFileFilter(bool);
    void ShowAddMarkerMenu();
    void AddMarkerType();

private slots:
	void OnCurrentChanged(const QModelIndex &current, const QModelIndex &previous);	
    void OnItemDoubleClick(const QModelIndex & mindex);    
    void OnRowsAdded(const QModelIndex &parent, int first, int last);

private:
	Ui::ToolboxWidget ui;
    ToolboxProxyModel proxy_;
    std::shared_ptr<LabelDefinitionsTreeModel> definitions_;    
};
