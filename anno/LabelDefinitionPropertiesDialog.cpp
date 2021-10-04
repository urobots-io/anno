// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
//
// Anno Labeling Tool
// 2020-2021 (c) urobots GmbH, https://urobots.io

#include "LabelDefinitionPropertiesDialog.h"
#include "CustomPropertiesEditorTableModel.h"
#include "PropertyTableItemDelegate.h"
#include "SharedPropertiesEditorTableModel.h"
#include "StampPropertiesEditorTableModel.h"

using namespace std;

LabelDefinitionPropertiesDialog::LabelDefinitionPropertiesDialog(shared_ptr<LabelDefinition> definition, shared_ptr<LabelDefinitionsTreeModel> definitions, QWidget *parent)
: QDialog(parent)
, definition_(definition)
, definitions_(definitions)
{
    ui.setupUi(this);

    // Main properties page.
    auto props = new LDProperties(this);
    props->set_type_name(definition_->type_name);
    props->set_description(definition_->get_description());
    props->set_line_width(definition_->line_width);
    props->set_value_type(LDProperties::LabelType(int(definition_->value_type)));
    PropertyTableItemDelegate::SetupTableView(ui.properties_tableView, props);

    auto shared = new SharedPropertiesEditorTableModel(definition_->shared_properties, definition_->value_type, this);
    ui.shared_properties_tableView->setModel(shared);
    ui.shared_properties_tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

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

    // Stamp page.
    auto stamp_properties = new StampPropertiesEditorTableModel(definition_->stamp_parameters, definition_->value_type, this);
    ui.stamp_parameters_tableView->setModel(stamp_properties);

    // Custom properties page.
    auto custom_properties = new CustomPropertiesEditorTableModel(definition_->custom_properties, this);
    ui.custom_properties_tableView->setModel(custom_properties);

    connect(ui.add_cp_pushButton, &QPushButton::clicked, custom_properties, &CustomPropertiesEditorTableModel::AddProperty);
}

LabelDefinitionPropertiesDialog::~LabelDefinitionPropertiesDialog()
{
}

void LabelDefinitionPropertiesDialog::onSharedLabelsCountChanged(int count) {
    ui.shared_status_label->setText(count == 0 ? tr(" - labels are not shared") : QString());
}