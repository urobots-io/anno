// Anno Labeling Tool
// 2020-2024 (c) urobots GmbH, https://urobots.io/en/portfolio/anno/

#include "MainWindow.h"
#include "AboutDialog.h"
#include "ToolboxWidget.h"
#include "Desktop3dWindow.h"
#include "ErrorsListDialog.h"
#include "ImageSettingsWidget.h"
#include "LabelDefinitionPropertiesWidget.h"
#include "messagebox.h"
#include "NavigationWidget.h"
#include "ProjectDefinitionsDialog.h"
#include "ProjectSettingsWidget.h"
#include "LabelPropertiesWidget.h"
#include "SourcePicturesWidget.h"
#include <QCloseEvent>
#include <QDockWidget>
#include <QFileDialog>
#include <QInputDialog>
#include <QSettings>
#include <QShortcut>
#include <QStringBuilder>

using namespace std;
using namespace urobots::qt_helpers;

#define NEW_PROJECT_IMAGE_FOLDER "NEW_PROJECT_IMAGE_FOLDER"
#define SAVE_PROJECT_FOLDER "SAVE_PROJECT_FOLDER"
#define OPEN_PROJECT_FOLDER "OPEN_PROJECT_FOLDER"

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, model_(this)
{
    setWindowIcon(QIcon(":/MainWindow/Resources/anno.ico"));

	ui.setupUi(this);

    ui.centralWidget->setBackgroundRole(QPalette::Shadow);
    ui.desktop_controls_widget->setBackgroundRole(QPalette::Base);
    ui.tools_tabs_widget->setBackgroundRole(QPalette::Base);

	ui.desktop->Init(&model_);	

    connect(ui.desktop, &DesktopWidget::is_creation_mode_changed, this, &MainWindow::OnDesktopCreationModeChanged);
	connect(ui.desktop, &DesktopWidget::mouse_pos_changed, this, &MainWindow::OnDesktopMousePosChanged);    
    connect(ui.fit_image_to_desktop_pushButton, &QPushButton::clicked, ui.desktop, &DesktopWidget::FitBackgroundToView);
    
    connect(ui.fit_image_to_view_when_selected_action, &QAction::toggled, ui.desktop, &DesktopWidget::set_fit_to_view_on_load);
    ui.fit_image_to_view_when_selected_action->setChecked(true);
    ui.enable_toolbox_filter_action->setChecked(true);
   	        
    connect(&model_, &ApplicationModel::filesystem_changed, this, &MainWindow::UpdateProjectControls);
	connect(&model_, &ApplicationModel::project_filename_changed, this, &MainWindow::UpdateApplicationTitle);
    connect(&model_, &ApplicationModel::is_modified_changed, this, &MainWindow::UpdateApplicationTitle);
	
    // setup scale controls
    ui.scale_doubleSpinBox->setValue(ui.desktop->get_world_scale());
    ui.scale_horizontalSlider->setValue(ui.desktop->get_world_scale_power() + 50);
    connect(ui.desktop, &DesktopWidget::world_scale_changed, this, &MainWindow::OnDesktopWorldScaleChanged);
    connect(ui.scale_doubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) { 
        ui.desktop->SetWorldScaleAndUpdate(value, false);
    });
    connect(ui.scale_horizontalSlider, &QSlider::valueChanged, [this](int value) {
        ui.desktop->SetWorldScaleAndUpdate(value - 50);
    });    

    // Accelerators
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q), this, SLOT(close()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_S), this, SLOT(OnSave()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_O), this, SLOT(OnOpenProject()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Z), this, SLOT(OnUndo()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Y), this, SLOT(OnRedo()));

    // UI actions
    connect(ui.about_action, &QAction::triggered, this, &MainWindow::ShowAboutDialog);
    connect(ui.exit_action, &QAction::triggered, this, &MainWindow::close);
	connect(ui.new_project_action, &QAction::triggered, this, &MainWindow::OnNewProject);
	connect(ui.save_action, &QAction::triggered, this, &MainWindow::OnSave);
    connect(ui.save_as_action, &QAction::triggered, this, &MainWindow::OnSaveAs);
    connect(ui.open_action, &QAction::triggered, this, &MainWindow::OnOpenProject);
    connect(ui.open_dataset_action, &QAction::triggered, this, &MainWindow::OnOpenDatasetProject);
    connect(ui.action3d_View, &QAction::triggered, this, &MainWindow::OnShow3dView);
	connect(ui.project_settings_action, &QAction::triggered, this, &MainWindow::OnProjectSettings);
    connect(ui.evaluate_in_roi_action, &QAction::triggered, this, &MainWindow::OnEvaluateInROI);

	// files tree
    ui.files_tree->Init(&model_);
    connect(ui.files_tree, &SourcePicturesWidget::FileModelSelected, this, &MainWindow::OnImageFileChanged);

    // toolbox
    connect(&model_, &ApplicationModel::label_definitions_changed, ui.toolbox, &ToolboxWidget::SetDefinitionsModel);
    connect(ui.files_tree, &SourcePicturesWidget::FileModelSelected, ui.toolbox, &ToolboxWidget::SetFile);
    connect(ui.toolbox, &ToolboxWidget::SelectionChanged, this, &MainWindow::OnToolboxSelection);
    connect(ui.toolbox, &ToolboxWidget::DoubleClick, this, &MainWindow::OnToolboxDoubleClick);
    connect(ui.toolbox, &ToolboxWidget::DeleteRequested, this, &MainWindow::OnDeleteRequest);
    connect(ui.enable_toolbox_filter_action, &QAction::toggled, ui.toolbox, &ToolboxWidget::EnableFileFilter);

	// selected label
    connect(ui.desktop, &DesktopWidget::selected_label_changed, ui.label_editor, &LabelPropertiesWidget::OnSelectedLabelChanged);
    connect(ui.label_editor, &LabelPropertiesWidget::DeleteLabel, ui.desktop, &DesktopWidget::DeleteLabel);
    connect(ui.desktop, &DesktopWidget::image_properties_changed, ui.image_properties, &ImagePropertiesWidget::setProperties);

    // project settings
    ui.project_settings->Init(&model_);

    // image settings
    ui.image_settings->Init(&ui.desktop->GetBackgroundImage());

    ui.mouse_pos_label->setWidthHint(120);
    ui.color_value_label->setWidthHint(120);
    ui.mouse_pos_label->setElideMode(Qt::ElideRight);
    ui.color_value_label->setElideMode(Qt::ElideRight);

    // navigation
    ui.mainToolBar->addSeparator();

    NavigationWidget* navigationWidget = new NavigationWidget(model_.get_navigation_model(), this);    
    ui.mainToolBar->addWidget(navigationWidget);

    /*
    QWidget *spacerWidget = new QWidget(this);
    spacerWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    spacerWidget->setVisible(true);
    ui.mainToolBar->addWidget(spacerWidget);
    */



    UpdateApplicationTitle();
    UpdateProjectControls();

    recent_projects_.Init("recentProjectsList", ui.menuRecent_Projects);
    connect(&recent_projects_, &RecentActionsList::ActionSelected, this, &MainWindow::OpenRecentProject);	

    setFocus();

    ui.statusBar->hide();
    ui.splitter->setSizes(QList<int>() << 1000 << 5000 << 1000);
}

MainWindow::~MainWindow() {
	model_.ClearProject();
}

void MainWindow::ShowAboutDialog() {
    AboutDialog about(this);
    about.exec();
}

void MainWindow::OnToolboxSelection(std::shared_ptr<LabelDefinition> definition, std::shared_ptr<LabelCategory> category) {    
    if (category) {
        ui.desktop->set_category_for_creation(category);
        ui.desktop->set_is_creation_mode(true);
    }
    else {
        ui.desktop->set_is_creation_mode(false);
    }
    ui.definition_editor->Select(definition, category);
}

void MainWindow::OnDeleteRequest(std::shared_ptr<LabelDefinition> definition, std::shared_ptr<LabelCategory> category, bool delete_only_instances) {
    QString message, caption;
    if (definition) {
        caption = tr("Delete marker type");
        if (delete_only_instances) {
            message = tr("Delete all instances of the marker type '%0'?");
        }
        else {
            message = tr("Delete the marker type '%0'?");
        }
    }
    else if (category) {
        caption = tr("Delete category");
        if (delete_only_instances) {
            message = tr("Delete all instances of the category '%0'?");
        }
        else {
            message = tr("Delete the category '%0'?");
        }
    }
    else {
        return;
    }

    message = message.arg(definition ? definition->get_type_name() : category->get_name());
    message += tr("\nWarning: this operation cannot be undone, it clears the undo stack.");

    if (messagebox::Question(message, caption)) {
        if (definition) {
            model_.Delete(definition, delete_only_instances);
        }
        else if (category) {
            model_.Delete(category, delete_only_instances);
        }
    }
}

void MainWindow::OnDesktopCreationModeChanged(bool value) {
    if (!value) {
        ui.toolbox->DismissCreation();
    }
}

void MainWindow::OnDesktopMousePosChanged(QPointF value) {
    ui.mouse_pos_label->setText(QString("%0, %1")
                                .arg(QString::number(value.x(), 'g', 5))
                                .arg(QString::number(value.y(), 'g', 5)));

    int ix = static_cast<int>(value.x() + 0.5);
    int iy = static_cast<int>(value.y() + 0.5);

	vector<float> pixel_values = ui.desktop->GetBackgroundImage().GetBackgroundPixelValues(ix, iy);

    ui.color_widget->SetColor(pixel_values);

	QString pixel_text;
    for (auto v : pixel_values) {
        if (pixel_text.size()) pixel_text = pixel_text % " ";
        pixel_text = pixel_text % QString("%0").arg(v);
    }
	ui.color_value_label->setText(pixel_text);
}

void MainWindow::UpdateApplicationTitle() {
    QString file_name = model_.get_project_filename();
    if (file_name.isEmpty()) {
        if (model_.get_filesystem() != nullptr) {
            file_name = "<unnamed>";
        }
        else {
            file_name = "<empty>";
        }
    }
	
    QString mod_mark;
    if (model_.get_is_modified()) mod_mark = tr("*");

	setWindowTitle(tr("anno - ") % file_name % mod_mark);
}

void MainWindow::UpdateProjectControls() {
    bool project_exists = model_.get_filesystem() != nullptr;
    ui.save_action->setEnabled(project_exists);
    ui.save_as_action->setEnabled(project_exists && !model_.get_project_filename().startsWith("http://"));
    ui.project_settings_action->setEnabled(project_exists);
}

bool MainWindow::OnSaveAs() {
    return OnSave(true);
}

bool MainWindow::OnSave(bool select_new_filename) {
    QString project_filename;
    if (select_new_filename || model_.get_project_filename().isEmpty()) {
        // Get save filename
        auto dir_value = QSettings().value(SAVE_PROJECT_FOLDER).toString();
        project_filename = QFileDialog::getSaveFileName(
            this,
            tr("Save project as"),
            dir_value % "/" + "my project",
            tr("Anno files (*.anno);;All Files (*.*)"));

        if (project_filename.isNull())
            return false;
        
        QSettings().setValue(SAVE_PROJECT_FOLDER, QFileInfo(project_filename).absolutePath());
    }

    QStringList errors;
    auto result = model_.SaveProject(errors, project_filename);
    if (result) {
        if (!project_filename.isEmpty()) {
            recent_projects_.AddValue(project_filename);
        }
    }
    else {        
        ErrorsListDialog dialog(tr("Save project error"), 
            tr("Failed to save project %0").arg(project_filename), errors);
        dialog.exec();
    }

    return result;
}

void MainWindow::OpenRecentProject(const QString & full_file_name) {
	if (!CloseActiveProject()) {
		return;
	}

    OpenProject(full_file_name);
}

void MainWindow::OnNewProject() {
    if (!CloseActiveProject()) {
        return;
    }

    // select folder with image files    
    auto images_folder = QSettings().value(NEW_PROJECT_IMAGE_FOLDER).toString();
    QFileDialog dialog(this, tr("Select folder with images"), images_folder);
    dialog.setFileMode(QFileDialog::Directory);
    //dialog.setOption(QFileDialog::DontUseNativeDialog);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    images_folder = dialog.selectedFiles().first();
    QSettings().setValue(NEW_PROJECT_IMAGE_FOLDER, images_folder);

    // create a project
    model_.NewProject(images_folder);
}

void MainWindow::OnOpenProject() {
	if (!CloseActiveProject()) {
		return;
	}

    auto projects_folder = QSettings().value(OPEN_PROJECT_FOLDER).toString();
	auto full_file_name = QFileDialog::getOpenFileName(
		this,
		tr("Open project"), 
		projects_folder, 
		tr("All files (*.*);;Anno files (*.anno)"));
	
    if (!full_file_name.isNull()) {
        projects_folder = QFileInfo(full_file_name).absoluteDir().path();
        QSettings().setValue(OPEN_PROJECT_FOLDER, projects_folder);

        OpenProject(full_file_name);
    }
}

void MainWindow::OnOpenDatasetProject() {
	if (!CloseActiveProject()) {
		return;
	}

    auto project_filename = model_.get_project_filename();
    if (!project_filename.startsWith("http://")) {
        project_filename = "http://127.0.0.1:8000/";
    }

    bool ok;
    QString url = QInputDialog::getText(this, 
        tr("Open DataSet Project"),
        tr("URL:"), 
        QLineEdit::Normal,
        project_filename, 
        &ok);

    if (ok) {
        OpenProject(url);
    }
}


void MainWindow::OpenProject(const QString & full_file_name) {
    if (full_file_name.startsWith("anno:")) {
        OpenProject(full_file_name.mid(5));
        return;
    }

    recent_projects_.AddValue(full_file_name);
    
    QStringList errors;
    if (!model_.OpenProject(full_file_name, errors)) {
        ErrorsListDialog dialog(tr("Open project error"), tr("Failed to open project %0").arg(full_file_name), errors);
        dialog.exec();        
    }
    else {        
#if _DEBUG
        // select first file
        if (auto file = model_.GetFirstFileModel()) {
            //emit ui.files_tree->FileModelSelected(file);
        }
#endif
    }
}

void MainWindow::closeEvent(QCloseEvent *event) {
	if (CloseActiveProject()) {
		event->accept();
        emit model_.ApplicationShutdown();
	}
	else {
		event->ignore();
	}
}

bool MainWindow::CloseActiveProject() {
	if (model_.get_is_modified()) {
		QMessageBox::StandardButton ret;
		ret = QMessageBox::warning(this, "",
			tr("The project has been modified.\nDo you want to save your changes?"),
			QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
		
        if (ret == QMessageBox::Save) {
            if (!OnSave()) {
                return false;
            }
		}
		else if (ret == QMessageBox::Cancel) {
			return false;
		}
	}

#if _DEBUG
    // 'unselect' file. this is needed, because in debug mode file
    // is artificially selected, see usage of GetFirstModel above
    emit ui.files_tree->FileModelSelected(nullptr);
#endif

    model_.ClearProject();
	return true;
}

void MainWindow::OnProjectSettings() {
	ProjectDefinitionsDialog dialog(&model_, this);
	dialog.exec();
}

void MainWindow::OnShow3dView() {
    auto window = new Desktop3dWindow(&model_, nullptr);
    connect(&model_, &ApplicationModel::ApplicationShutdown, window, &Desktop3dWindow::close);
    connect(ui.files_tree, &SourcePicturesWidget::FileModelSelected, window, &Desktop3dWindow::OnSelectedImageFileChanged);
    window->OnSelectedImageFileChanged(ui.desktop->GetFile());
    window->show();
}

void MainWindow::OnImageFileChanged(std::shared_ptr<FileModel> value) {
    if (selected_file_) {
        selected_file_->GetUndoStack()->disconnect(this);
        ui.undo_action->disconnect(selected_file_->GetUndoStack());
        ui.redo_action->disconnect(selected_file_->GetUndoStack());
    }   

    selected_file_ = value;
    ui.desktop->SetFile(value);

    if (selected_file_) {
        auto stack = selected_file_->GetUndoStack();
        connect(stack, &QUndoStack::canUndoChanged, this, &MainWindow::CanUndoChanged);
        connect(stack, &QUndoStack::canRedoChanged, this, &MainWindow::CanRedoChanged);
        
        CanUndoChanged(stack->canUndo());
        CanRedoChanged(stack->canRedo());

        connect(ui.undo_action, &QAction::triggered, stack, &QUndoStack::undo);
        connect(ui.redo_action, &QAction::triggered, stack, &QUndoStack::redo);
    }
}

void MainWindow::CanUndoChanged(bool value) {    
    ui.undo_action->setEnabled(value);
}

void MainWindow::CanRedoChanged(bool value) {
    ui.redo_action->setEnabled(value);
}

void MainWindow::OnUndo() {
    if (ui.undo_action->isEnabled()) {
        ui.undo_action->trigger();
        ui.desktop->AbortCreation();
    }
}

void MainWindow::OnRedo() {
    if (ui.redo_action->isEnabled()) {
        ui.redo_action->trigger();
        ui.desktop->AbortCreation();
    }
}

void MainWindow::OnDesktopWorldScaleChanged(double value) {
    ui.scale_doubleSpinBox->blockSignals(true);
    ui.scale_horizontalSlider->blockSignals(true);

    if (value > ui.scale_doubleSpinBox->maximum())
        ui.scale_doubleSpinBox->setValue(ui.scale_doubleSpinBox->maximum());
    else
        ui.scale_doubleSpinBox->setValue(value);

    int new_value = ui.desktop->get_world_scale_power() + 50;
    if (new_value > ui.scale_horizontalSlider->maximum())
        ui.scale_horizontalSlider->setValue(ui.scale_horizontalSlider->maximum());
    else
        ui.scale_horizontalSlider->setValue(new_value);

    ui.scale_doubleSpinBox->blockSignals(false);
    ui.scale_horizontalSlider->blockSignals(false);
}

void MainWindow::OnToolboxDoubleClick(std::shared_ptr<LabelCategory> category) {
    if (category) {
        if (auto def = category->GetDefinition()) {
            if (def->is_shared()) {
                if (auto file = ui.desktop->GetFile()) {
                    file->CreateDefaultSharedLabel(category);
                }
            }
        }
    }
}

void MainWindow::OnEvaluateInROI() {
    if (!selected_file_ || ui.desktop->get_is_loading_image()) {
        messagebox::Critical("Please select an image and a rectanular ROI (optional).");
        return;
    }

    auto label = ui.desktop->get_selected_label();    
    auto definition = label ? label->GetDefinition() : nullptr;

    bool roi_label_selected = definition && definition->value_type == LabelType::rect;

    auto& image = ui.desktop->GetBackgroundImage();

    QProgressDialog progress(tr("Evaluating..."), QString(), 0, 0, this);
    progress.show();

    QStringList errors;
    bool evaluate_ok;
    if (roi_label_selected) {
        auto p0 = label->GetHandles()[0]->GetPosition();
        auto p1 = label->GetHandles()[1]->GetPosition();

        int x = int(min(p0.x(), p1.x()));
        int y = int(min(p0.y(), p1.y()));
        int width = int(fabs(p1.x() - p0.x()));
        int height = int(fabs(p1.y() - p0.y()));
        auto cropped_image = image.CropImage(QRect(x, y, width, height));

        evaluate_ok = model_.Evaluate(selected_file_, cropped_image, QPointF(x, y), errors);
    }
    else {
        evaluate_ok = model_.Evaluate(selected_file_, image.GetImageData(), QPointF(0, 0), errors);
    }

    progress.close();

    if (evaluate_ok && errors.empty()) {
        return;
    }


    ErrorsListDialog dialog(tr("Evaluate error"),
                            evaluate_ok ? tr("Completed with errors:") : tr("Failed with errors"), errors);
    dialog.exec();
}


