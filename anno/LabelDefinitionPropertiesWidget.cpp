#include "LabelDefinitionPropertiesWidget.h"
#include "messagebox.h"
#include <QInputDialog>
#include <QColorDialog>

using namespace urobots::qt_helpers;

LabelDefinitionPropertiesWidget::LabelDefinitionPropertiesWidget(QWidget *parent)
: QWidget(parent)
{
    ui.setupUi(this);
    
    connect(ui.description_lineEdit, &QLineEdit::textChanged, this, &LabelDefinitionPropertiesWidget::OnDescriptionTextChanged);
    connect(ui.rendering_script_textEdit, &QTextEdit::textChanged, this, &LabelDefinitionPropertiesWidget::OnRenderingScriptTextChanged);
    connect(ui.change_category_color_toolButton, &QToolButton::clicked, this, &LabelDefinitionPropertiesWidget::OnChangeCategoryColor);
    connect(ui.change_category_value_toolButton, &QToolButton::clicked, this, &LabelDefinitionPropertiesWidget::OnChangeCategoryValue);

    ui.stackedWidget->setCurrentWidget(ui.empty_page);
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

void LabelDefinitionPropertiesWidget::UpdateCategoryData() {
    QPixmap pixmap(10, 10);
    pixmap.fill(category_->color);
    ui.category_color_label->setPixmap(pixmap);

    ui.category_value_lineEdit->setText(QString("%0").arg(category_->value));
}

void LabelDefinitionPropertiesWidget::Select(std::shared_ptr<LabelDefinition> def, std::shared_ptr<LabelCategory> category) {
    if (definition_ == def && category_ == category) 
        return;

    category_ = category;
    definition_ = def;
    
    if (category_) {
        ui.stackedWidget->setCurrentWidget(ui.category_page);        
        
        UpdateCategoryData();        
    }
    else if (definition_) {
        ui.stackedWidget->setCurrentWidget(ui.marker_page);

        ui.rendering_script_textEdit->blockSignals(true);
        ui.rendering_script_textEdit->setText(def ? def->get_rendering_script() : QString());
        ui.rendering_script_textEdit->blockSignals(false);

        ui.description_lineEdit->blockSignals(true);
        ui.description_lineEdit->setText(def ? def->get_description() : QString());
        ui.description_lineEdit->blockSignals(false);

        setEnabled(def != nullptr);
    }
    else {
        ui.stackedWidget->setCurrentWidget(ui.empty_page);
    }

}

void LabelDefinitionPropertiesWidget::OnChangeCategoryValue() {
    if (category_) {
        auto value = category_->value;
        while (true) {
            bool ok;
            value = QInputDialog::getInt(
                this, tr("Category Value"),
                tr("New category value:"),
                value,
                -99999, 99999, 1, &ok);

            if (!ok || value == category_->value) {
                return;
            }

            bool value_valid = true;
            for (auto c : category_->definition->categories) {
                if (c->value == value) {
                    messagebox::Critical(tr("Value %0 is already used by category %1").arg(value).arg(c->name));
                    value_valid = false;
                    break;
                }
            }

            if (value_valid) {
                category_->value = value;
                emit category_->definition->Changed();
                UpdateCategoryData();
                break;
            }
        }
    }
}

void LabelDefinitionPropertiesWidget::OnChangeCategoryColor() {
    if (category_) {
        auto color = QColorDialog::getColor(category_->color, this, tr("Caregory Color"));
        if (color.isValid()) {
            category_->color = color;
            emit category_->definition->Changed();
            UpdateCategoryData();
        }
    }
}

