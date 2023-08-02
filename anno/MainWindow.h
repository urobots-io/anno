#pragma once
#include "ApplicationModel.h"
#include "DesktopWidget.h"
#include "ui_MainWindow.h"
#include "settings.h"
#include <QtWidgets/QMainWindow>

class LabelDefinitionPropertiesWidget;
class SourcePicturesWidget;
class ToolboxWidget;

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = Q_NULLPTR);
    ~MainWindow() override;

	ApplicationModel & GetModel() { return  model_; }

    void OpenProject(const QString & full_file_name);

protected:	
	void closeEvent(QCloseEvent *event) override;    

public slots:    
	void OnDesktopMousePosChanged(QPointF);
	void OnOpenProject();
    void OnOpenDatasetProject();
    bool OnSave(bool select_new_filename = false);
    bool OnSaveAs();
	void OnNewProject();	
    void ShowAboutDialog();    
    void OnShow3dView();
    void OpenRecentProject(const QString & full_file_name);
    void OnImageFileChanged(std::shared_ptr<FileModel>);
    void CanUndoChanged(bool);
    void CanRedoChanged(bool);    
    void OnUndo();
    void OnRedo();
    void OnToolboxSelection(std::shared_ptr<LabelDefinition>, std::shared_ptr<LabelCategory>);
    void OnDeleteRequest(std::shared_ptr<LabelDefinition>, std::shared_ptr<LabelCategory>, bool);
    void OnDesktopCreationModeChanged(bool);
    void OnDesktopWorldScaleChanged(double);
    void OnToolboxDoubleClick(std::shared_ptr<LabelCategory>);
	void OnProjectSettings();
    void UpdateApplicationTitle();
    void UpdateProjectControls();
    void OnEvaluateInROI();

private:	
	bool CloseActiveProject();

private:
	Ui::MainWindowClass ui;

	ApplicationModel model_;

	RecentActionsList recent_projects_;

    std::shared_ptr<FileModel> selected_file_;
};
