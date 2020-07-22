#include "LabelDefinitionPropertiesWidget.h"

LabelDefinitionPropertiesWidget::LabelDefinitionPropertiesWidget(QWidget *parent)
: QWidget(parent)
{
    ui.setupUi(this);
    
    connect(ui.description_lineEdit, &QLineEdit::textChanged, this, &LabelDefinitionPropertiesWidget::OnDescriptionTextChanged);
    connect(ui.rendering_script_textEdit, &QTextEdit::textChanged, this, &LabelDefinitionPropertiesWidget::OnRenderingScriptTextChanged);

    setEnabled(false);
}

LabelDefinitionPropertiesWidget::~LabelDefinitionPropertiesWidget()
{
}

void LabelDefinitionPropertiesWidget::OnDescriptionTextChanged() {
    if (definition_) {
        definition_->set_description(ui.description_lineEdit->text());
    }
}

void LabelDefinitionPropertiesWidget::OnRenderingScriptTextChanged() {
    if (definition_) {
        auto text = ui.rendering_script_textEdit->toPlainText();
        definition_->set_rendering_script(text);
    }
}

void LabelDefinitionPropertiesWidget::Select(std::shared_ptr<LabelDefinition> def, std::shared_ptr<LabelCategory> category) {
    if (definition_ == def && category_ == category) 
        return;

    definition_ = def;
    
    ui.rendering_script_textEdit->blockSignals(true);
    ui.rendering_script_textEdit->setText(def ? def->get_rendering_script() : QString());
    ui.rendering_script_textEdit->blockSignals(false);

    ui.description_lineEdit->blockSignals(true);
    ui.description_lineEdit->setText(def ? def->get_description() : QString());
    ui.description_lineEdit->blockSignals(false);

    setEnabled(def != nullptr);
}

