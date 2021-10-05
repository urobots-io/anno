// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
//
// Anno Labeling Tool
// 2020-2021 (c) urobots GmbH, https://urobots.io

#include "LabelDefinitionPropertiesDialog.h"
#include "CustomPropertiesEditorTableModel.h"
#include "messagebox.h"
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
    properties_ = new LDProperties(this);
    properties_->set_type_name(definition_->get_type_name());
    properties_->set_description(definition_->get_description());
    properties_->set_line_width(definition_->get_line_width());
    properties_->set_value_type(LDProperties::LabelType(int(definition_->value_type)));
    PropertyTableItemDelegate::SetupTableView(ui.properties_tableView, properties_);

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
    ui.is_stamp_checkBox->setChecked(definition_->get_is_stamp());

    // Custom properties page.
    auto custom_properties = new CustomPropertiesEditorTableModel(definition_->custom_properties, this);
    ui.custom_properties_tableView->setModel(custom_properties);

    connect(ui.add_cp_pushButton, &QPushButton::clicked, custom_properties, &CustomPropertiesEditorTableModel::AddProperty);

    // Button box
    connect(ui.buttonBox->button(QDialogButtonBox::Cancel), &QPushButton::clicked, this, &QDialog::close);
    connect(ui.buttonBox->button(QDialogButtonBox::Ok), &QPushButton::clicked, this, &LabelDefinitionPropertiesDialog::ApplyAndClose);
}

LabelDefinitionPropertiesDialog::~LabelDefinitionPropertiesDialog()
{
}

void LabelDefinitionPropertiesDialog::onSharedLabelsCountChanged(int count) {
    ui.shared_status_label->setText(count == 0 ? tr(" - labels are not shared") : QString());
}

void LabelDefinitionPropertiesDialog::ApplyAndClose() {
    // Test changes to be done.
    auto existind_definition = definitions_->FindDefinition(properties_->get_type_name());
    if (existind_definition && existind_definition != definition_) {
        messagebox::Critical(tr("Marker named %0 already exists. Please use another name.")
                             .arg(properties_->get_type_name()));
        return;
    }

    if (LabelType(properties_->get_value_type()) != definition_->value_type) {
        messagebox::Critical(tr("Changing of the value type is not yet supported."));
        return;
    }

    // Apply changes in main properties.
    definition_->set_type_name(properties_->get_type_name());
    definition_->set_line_width(properties_->get_line_width());
    definition_->set_description(properties_->get_description());
    definition_->set_is_stamp(ui.is_stamp_checkBox->isChecked());

    // Apply stamp properties.
    auto stamp_properties = (StampPropertiesEditorTableModel*)ui.stamp_parameters_tableView->model();
    definition_->stamp_parameters = stamp_properties->GetStampProperties();

    // Apply shared instances and filename filter.
    vector<string> filename_filter;
    for (auto s : ui.plainTextEdit->toPlainText().split('\n')) {
        if (!s.trimmed().isEmpty()) {
            filename_filter.push_back(s.toStdString());
        }
    }
    definition_->filename_filter = filename_filter;


    close();
}
