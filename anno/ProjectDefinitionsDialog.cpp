#include "ProjectDefinitionsDialog.h"
#include "ApplicationModel.h"
#include "Highlighter.h"
#include "messagebox.h"
#include "PropertyDatabase.h"
#include "qjson_helpers.h"
#include <QPushButton>

using namespace urobots::qt_helpers;


ProjectDefinitionsDialog::ProjectDefinitionsDialog(ApplicationModel *model, QWidget *parent)
: QDialog(parent)
, model_(model)
{
    ui.setupUi(this);

    new Highlighter(ui.textEdit->document(), palette());
    
    auto def = model_->GenerateHeader();
    ui.textEdit->setText(QJsonDocument(def).toJson());

    connect(ui.buttonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked, this, &ProjectDefinitionsDialog::OnApply);
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
    QString error;
    auto json = LoadJsonFromText(content.toUtf8(), error);
    if (json.isNull()) {
        messagebox::Critical(error);
        return;
    }

    if (!model_->ApplyHeader(json.object(), error)) {
        messagebox::Critical(error);
        return;
    }

    PropertyDatabase::Instance().Modify();
    if (close_if_success) {
        close();
    }
}


