#pragma once
#include "ApplicationModel.h"
#include "DesktopWidget.h"
#include "ui_MainWindow.h"
#include "settings.h"
#include <QtWidgets/QMainWindow>
#include <QLabel>
#include <QPlainTextEdit>

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
    void InitToolbars();
    void InitTextViewer();

    /// Shows the text file instead of the image view, or the image view if file_id is empty.
    void ShowTextFile(QString file_id);

private:
	Ui::MainWindowClass ui;

	ApplicationModel model_;

	RecentActionsList recent_projects_;

    std::shared_ptr<FileModel> selected_file_;

    // read-only viewer for text files, shown instead of the image view
    QPlainTextEdit *text_view_ = nullptr;
    QLabel *text_message_ = nullptr;
};
