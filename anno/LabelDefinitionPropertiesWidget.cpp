// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
//
// Anno Labeling Tool
// 2020-2021 (c) urobots GmbH, https://urobots.io

#include "LabelDefinitionPropertiesWidget.h"
#include "Highlighter.h"
#include "messagebox.h"
#include "ScriptPainter.h"
#include <QColorDialog>
#include <QInputDialog>
#include <QMenu>
#include <QMetaMethod>

using namespace urobots::qt_helpers;

LabelDefinitionPropertiesWidget::LabelDefinitionPropertiesWidget(QWidget *parent)
: QWidget(parent)
{
    ui.setupUi(this);

    new Highlighter(ui.rendering_script_textEdit->document(), palette(), Highlighter::JScript);
    
    connect(ui.description_lineEdit, &QLineEdit::textChanged, this, &LabelDefinitionPropertiesWidget::OnDescriptionTextChanged);
    connect(ui.rendering_script_textEdit, &QTextEdit::textChanged, this, &LabelDefinitionPropertiesWidget::OnRenderingScriptTextChanged);
    connect(ui.change_category_color_toolButton, &QToolButton::clicked, this, &LabelDefinitionPropertiesWidget::OnChangeCategoryColor);
    connect(ui.change_category_value_toolButton, &QToolButton::clicked, this, &LabelDefinitionPropertiesWidget::OnChangeCategoryValue);
    connect(ui.add_code_line_pushButton, &QPushButton::clicked, this, &LabelDefinitionPropertiesWidget::ShowAddCodeLineMenu);

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

        ui.value_type_lineEdit->setText(def ? LabelTypeToString(def->value_type) : QString());

        setEnabled(def != nullptr);
    }
    else {
        ui.stackedWidget->setCurrentWidget(ui.empty_page);
    }

}

void LabelDefinitionPropertiesWidget::OnChangeCategoryValue() {
    if (auto category = category_) {
        auto value = category->value;
        while (true) {
            bool ok;
            value = QInputDialog::getInt(
                this, tr("Category Value"),
                tr("New category value:"),
                value,
                -99999, 99999, 1, &ok);

            if (!ok || value == category->value) {
                return;
            }

            bool value_valid = true;
            if (auto def = category->GetDefinition()) {
                for (auto c : def->categories) {
                    if (c->value == value) {
                        messagebox::Critical(tr("Value %0 is already used by category %1").arg(value).arg(c->name));
                        value_valid = false;
                        break;
                    }
                }

                if (value_valid) {
                    category->value = value;
                    emit def->Changed();
                    if (category_) {
                        UpdateCategoryData();
                    }
                    break;
                }
            }
        }
    }
}

void LabelDefinitionPropertiesWidget::OnChangeCategoryColor() {
    if (auto category = category_) {
        auto color = QColorDialog::getColor(category->color, this, tr("Caregory Color"));
        if (color.isValid()) {
            category->color = color;
            if (auto def = category->GetDefinition()) {
                emit def->Changed();
            }
            if (category_) {
                UpdateCategoryData();
            }
        }
    }
}

void LabelDefinitionPropertiesWidget::ShowAddCodeLineMenu() {    
    QMenu context_menu(this);

    std::vector<QAction*> actions;
    auto &meta = ScriptPainter::staticMetaObject;
    for (int i = 0; i < meta.methodCount(); ++i) {
        auto m = meta.method(i);
        if (m.methodType() == m.Slot) {
            auto name = QString::fromLatin1(m.name());
            if (!name[0].isUpper()) {
                // Ignore default qt functions starting with lowcase.
                continue;
            }
            
            QStringList params;
            for (auto p : m.parameterNames()) {
                auto pname = QString::fromLatin1(p).trimmed();
                if (!pname.isEmpty()) {
                    params << pname;
                }                    
            }

            name += "(" + params.join(",") + ")";                
            auto a = new QAction(name, &context_menu);
            a->setObjectName(name);
            connect(a, &QAction::triggered, this, &LabelDefinitionPropertiesWidget::AddCodeLine);
            actions.push_back(a);
        }
    }

    std::sort(actions.begin(), actions.end(), [](QAction *a, QAction* b) {return a->objectName().compare(b->objectName()) < 0; });
        
    for (auto a : actions) {
        context_menu.addAction(a);
    }    
    context_menu.exec(ui.widget->mapToGlobal(ui.add_code_line_pushButton->geometry().bottomRight()));
}

void LabelDefinitionPropertiesWidget::AddCodeLine() {
    ui.rendering_script_textEdit->insertPlainText(sender()->objectName());
}
