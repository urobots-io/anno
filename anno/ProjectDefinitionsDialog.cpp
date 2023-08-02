// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
//
// Anno Labeling Tool
// 2020-2023 (c) urobots GmbH, https://urobots.io

#include "ProjectDefinitionsDialog.h"
#include "ApplicationModel.h"
#include "ErrorsListDialog.h"
#include "Highlighter.h"
#include "PropertyDatabase.h"
#include "qjson_helpers.h"
#include <QPushButton>

ProjectDefinitionsDialog::ProjectDefinitionsDialog(ApplicationModel *model, QWidget *parent)
: QDialog(parent)
, model_(model)
{
    ui.setupUi(this);

    new Highlighter(ui.textEdit->document(), palette(), Highlighter::JSon);

    ui.textEdit->setText(QJsonDocument(model_->get_user_data()).toJson());

    connect(ui.buttonBox->button(QDialogButtonBox::Cancel), &QPushButton::clicked, this, &ProjectDefinitionsDialog::close);
	connect(ui.buttonBox->button(QDialogButtonBox::Ok), &QPushButton::clicked, this, &ProjectDefinitionsDialog::OnOk);
}

ProjectDefinitionsDialog::~ProjectDefinitionsDialog()
{
}

void ProjectDefinitionsDialog::keyPressEvent(QKeyEvent *e) {
    if (e->key() != Qt::Key_Escape)
        QDialog::keyPressEvent(e);
    else
        e->ignore();
}

void ProjectDefinitionsDialog::OnOk() {
	Apply(true);
}

void ProjectDefinitionsDialog::OnApply() {
    Apply(false);
}

void ProjectDefinitionsDialog::Apply(bool close_if_success) {
    auto content = ui.textEdit->document()->toPlainText();
    
    QStringList errors;
    QString json_error;    
    auto json = LoadJsonFromText(content.toUtf8(), json_error);
    if (json.isNull()) {
        errors << json_error;
    }
    else {
        model_->set_user_data(json.object());
        model_->SetModified();
    }

    if (errors.size()) {
        ErrorsListDialog dialog(tr("Definition error"), tr("Definition contains errors and cannot be applied."), errors, this);
        dialog.exec();
    }
    else {
        PropertyDatabase::Instance().Modify();
        if (close_if_success) {
            close();
        }
    }
}


