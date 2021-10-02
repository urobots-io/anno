#pragma once
#include "FileModel.h"
#include "LabelDefinitionsTreeModel.h"
#include "ToolboxProxyModel.h"
#include "ui_ToolboxWidget.h"
#include <QWidget>

class ToolboxWidget : public QWidget
{
	Q_OBJECT

public:
	ToolboxWidget(QWidget *parent);
	~ToolboxWidget();

signals:
    void SelectionChanged(std::shared_ptr<LabelDefinition>, std::shared_ptr<LabelCategory>);
    void DoubleClick(std::shared_ptr<LabelCategory>);
    void DeleteRequested(std::shared_ptr<LabelDefinition>, std::shared_ptr<LabelCategory>, bool delete_only_instances);    

public slots:
    void SetDefinitionsModel(std::shared_ptr<LabelDefinitionsTreeModel>);
    void SetFile(std::shared_ptr<FileModel>);
    void CleanupSelection();
    void EnableFileFilter(bool);
    void ShowAddMarkerMenu();
    void AddMarkerType();
    void OnCustomContextMenu(const QPoint &point);
    void OnError(QString);

private slots:
	void OnCurrentChanged(const QModelIndex &current, const QModelIndex &previous);	
    void OnItemDoubleClick(const QModelIndex & mindex);    
    void OnRowsAdded(const QModelIndex &parent, int first, int last);
    void RenameItem();
    void AddCategory();
    void DeleteMarker();
    void CloneMarker();
    void DeleteMarkerFromImages();
    void DeleteCategory();
    void DeleteCategoryFromImages();
    void ToggleTreeOpenState();
    void ShowLabelDefinitionProperties();

private:
    template<class T>
    void AddAction(QMenu *menu, QString icon, QString text, T callback);

    void MakeCurrent(QModelIndex index, bool start_editing);

private:
	Ui::ToolboxWidget ui;
    ToolboxProxyModel proxy_;
    std::shared_ptr<LabelDefinitionsTreeModel> definitions_;  
    QMenu *marker_menu_ = nullptr;
    QMenu *category_menu_ = nullptr;
    QModelIndex menu_index_;
    bool toggle_closes_ = true;
};
