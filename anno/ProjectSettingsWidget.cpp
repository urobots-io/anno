#include "ProjectSettingsWidget.h"
#include "ApplicationModel.h"
#include "Highlighter.h"
#include "ProjectDefinitionsDialog.h"

ProjectSettingsWidget::ProjectSettingsWidget(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);

    new Highlighter(ui.textEdit->document(), palette(), Highlighter::JScript);
}

void ProjectSettingsWidget::Init(ApplicationModel* model) {
    model_ = model;

    connect(ui.textEdit, &QTextEdit::textChanged, this, &ProjectSettingsWidget::OnTextChanged);
    connect(model_, &ApplicationModel::image_script_changed, this, &ProjectSettingsWidget::OnImageScriptChanged);
    
    connect(ui.edit_definitions_pushButton, &QPushButton::clicked, this, &ProjectSettingsWidget::OnEditDefinitions);
}

ProjectSettingsWidget::~ProjectSettingsWidget()
{
}

void ProjectSettingsWidget::OnImageScriptChanged(QString value) {
    if (script_ != value) {
        ui.textEdit->disconnect(this);
        ui.textEdit->setText(value);
        connect(ui.textEdit, &QTextEdit::textChanged, this, &ProjectSettingsWidget::OnTextChanged);
    }
}

void ProjectSettingsWidget::OnTextChanged() {
    script_ = ui.textEdit->toPlainText();
    model_->set_image_script(script_);
    model_->set_is_modified(true);
}

void ProjectSettingsWidget::OnEditDefinitions() {
    ProjectDefinitionsDialog dialog(model_, this);
    dialog.exec();
}

