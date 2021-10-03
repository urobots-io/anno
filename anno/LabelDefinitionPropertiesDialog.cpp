#include "LabelDefinitionPropertiesDialog.h"

using namespace std;

LabelDefinitionPropertiesDialog::LabelDefinitionPropertiesDialog(shared_ptr<LabelDefinition> definition, shared_ptr<LabelDefinitionsTreeModel> definitions, QWidget *parent)
: QDialog(parent)
, definition_(definition)
, definitions_(definitions)
{
    ui.setupUi(this);

    // Init shared instances page.
    auto shared_count = int(definition_->shared_labels.size());
    ui.spinBox->setValue(shared_count);
    ui.plainTextEdit->setMaxFilenames(shared_count);
    onSharedLabelsCountChanged(shared_count);

    QStringList filenames;
    for (auto s: definition_->filename_filter) {
        filenames << QString::fromStdString(s);
    }
    ui.plainTextEdit->setPlainText(filenames.join("\n"));

    connect(ui.spinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &LabelDefinitionPropertiesDialog::onSharedLabelsCountChanged);
    connect(ui.spinBox, QOverload<int>::of(&QSpinBox::valueChanged), ui.plainTextEdit, &FilenamesEditorWidget::setMaxFilenames);
}

LabelDefinitionPropertiesDialog::~LabelDefinitionPropertiesDialog()
{
}

void LabelDefinitionPropertiesDialog::onSharedLabelsCountChanged(int count) {
    ui.shared_status_label->setText(count == 0 ? tr(" - labels are not shared") : QString());
}