// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
//
// Anno Labeling Tool
// 2020-2023 (c) urobots GmbH, https://urobots.io

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
    connect(model_, &ApplicationModel::project_script_changed, this, &ProjectSettingsWidget::OnProjectScriptChanged);
    
    connect(ui.edit_definitions_pushButton, &QPushButton::clicked, this, &ProjectSettingsWidget::OnEditDefinitions);
}

ProjectSettingsWidget::~ProjectSettingsWidget()
{
}

void ProjectSettingsWidget::OnProjectScriptChanged(QString value) {
    if (script_ != value) {
        ui.textEdit->disconnect(this);
        ui.textEdit->setText(value);
        connect(ui.textEdit, &QTextEdit::textChanged, this, &ProjectSettingsWidget::OnTextChanged);
    }
}

void ProjectSettingsWidget::OnTextChanged() {
    script_ = ui.textEdit->toPlainText();
    model_->set_project_script(script_);
    model_->set_is_modified(true);
}

void ProjectSettingsWidget::OnEditDefinitions() {
    ProjectDefinitionsDialog dialog(model_, this);
    dialog.exec();
}

