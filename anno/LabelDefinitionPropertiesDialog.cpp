#include "LabelDefinitionPropertiesDialog.h"
#include "ui_LabelDefinitionPropertiesDialog.h"

LabelDefinitionPropertiesDialog::LabelDefinitionPropertiesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LabelDefinitionPropertiesDialog)
{
    ui->setupUi(this);
}

LabelDefinitionPropertiesDialog::~LabelDefinitionPropertiesDialog()
{
    delete ui;
}
