// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
//
// Anno Labeling Tool
// 2020-2021 (c) urobots GmbH, https://urobots.io

#include "LabelDefinitionPropertiesDialog.h"
#include "ApplicationModel.h"
#include "CustomPropertiesEditorTableItemDelegate.h"
#include "CustomPropertiesEditorTableModel.h"
#include "messagebox.h"
#include "PropertyTableItemDelegate.h"
#include "SharedPropertiesEditorTableModel.h"
#include "StampPropertiesEditorTableModel.h"

using namespace std;

class SharedPropertiesTableDelegate : public QStyledItemDelegate
{
public:
    QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override {
            auto w = QStyledItemDelegate::createEditor(parent, option, index);

            auto sp = qobject_cast<QDoubleSpinBox*>(w);
            if (sp) {
                sp->setDecimals(4);
            }
            return w;
        }
};

LabelDefinitionPropertiesDialog::LabelDefinitionPropertiesDialog(shared_ptr<LabelDefinition> definition, shared_ptr<LabelDefinitionsTreeModel> definitions, QWidget *parent)
: QDialog(parent)
, definition_(definition)
, definitions_(definitions)
{
    ui.setupUi(this);

    ui.shared_properties_tableView->setItemDelegate(new SharedPropertiesTableDelegate());
    ui.custom_properties_tableView->setItemDelegate(new CustomPropertiesEditorTableItemDelegate());

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
    connect(ui.delete_cp_pushButton, &QPushButton::clicked, this, &LabelDefinitionPropertiesDialog::DeleteCustomProperty);

    // Button box
    connect(ui.buttonBox->button(QDialogButtonBox::Cancel), &QPushButton::clicked, this, &QDialog::close);
    connect(ui.buttonBox->button(QDialogButtonBox::Ok), &QPushButton::clicked, this, &LabelDefinitionPropertiesDialog::ApplyAndClose);
    connect(ui.buttonBox->button(QDialogButtonBox::Ok), &QPushButton::pressed, this, &LabelDefinitionPropertiesDialog::CloseEditors);
}

LabelDefinitionPropertiesDialog::~LabelDefinitionPropertiesDialog()
{
}

void LabelDefinitionPropertiesDialog::onSharedLabelsCountChanged(int count) {
    ui.shared_status_label->setText(count == 0 ? tr(" - labels are not shared") : QString());
}

void LabelDefinitionPropertiesDialog::CloseEditors() {
    ui.shared_properties_tableView->setCurrentIndex(QModelIndex());
    ui.properties_tableView->setCurrentIndex(QModelIndex());
    ui.stamp_parameters_tableView->setCurrentIndex(QModelIndex());
    ui.custom_properties_tableView->setCurrentIndex(QModelIndex());
}

void LabelDefinitionPropertiesDialog::DeleteCustomProperty() {
    auto model = (CustomPropertiesEditorTableModel*)ui.custom_properties_tableView->model();
    model->DeleteProperty(ui.custom_properties_tableView->currentIndex());
}

QString LabelDefinitionPropertiesDialog::GetSharedLabelsCountUpdateNotification() {
    // Test changes in shared properties:
    // 1. Change from not_shared -> shared: question to remove already available labels.
    // 2. Warn if sharing is to be removed.
    // 3. Warn/Question if shared labels number decreased and some labels will be removed.
    auto def = definition_;
    auto new_count = ui.spinBox->value();
    auto app = definitions_->GetApplicationModel();

    set<int> existing_indexes = def->is_shared()
            ? app->GetExistingSharedIndexes(def)
            : set<int>();
    auto existing_labels_count = app->GetLabelsCount(def);
    if (!existing_indexes.size() && !existing_labels_count) {
        // nothing to worry about, no labels are created yet.
        return QString();
    }

    if (!def->is_shared()) {
        if (new_count > 0) {
            return "You want to make marker shared, but there are existing labels which might be deleted or modified via this change. Dou you want to continue?";
        }
    }
    else {
        if (new_count == 0) {
            return "Do you want to disable labels sharing?";
        }
        else {
            set<int> indexes_to_delete;
            for (auto i: existing_indexes) {
                if (i >= new_count) {
                    indexes_to_delete.insert(i);
                }
            }
            if (indexes_to_delete.size()) {
                return "You want to decrease number of shared instances of the marker. Some labels will be deleted. Do you want to continue?";
            }
        }
    }

    return QString();
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

    auto notification = GetSharedLabelsCountUpdateNotification();
    if (!notification.isEmpty()) {
        if (!messagebox::Question(notification +
                                  "\nReverting changes will not be possible, undo stack will be cleared.",
                                  tr("Shared Instances Number Modified"))) {
            return;
        }
    }

    // Apply changes in main properties.
    definition_->set_type_name(properties_->get_type_name());
    definition_->set_line_width(properties_->get_line_width());
    definition_->set_description(properties_->get_description());
    definition_->set_is_stamp(ui.is_stamp_checkBox->isChecked());

    // Apply stamp properties.
    auto stamp_properties = (StampPropertiesEditorTableModel*)ui.stamp_parameters_tableView->model();
    definition_->stamp_parameters = stamp_properties->GetStampProperties();

    // Apply filename filter.
    vector<string> filename_filter;
    for (auto s : ui.plainTextEdit->toPlainText().split('\n')) {
        if (!s.trimmed().isEmpty()) {
            filename_filter.push_back(s.toStdString());
        }
    }
    definition_->filename_filter = filename_filter;

    auto model = definitions_->GetApplicationModel();

    // Apply shared labels.
    model->UpdateDefinitionSharedCount(definition_, ui.spinBox->value());

    // Apply shared properties.
    auto props = ((SharedPropertiesEditorTableModel*)ui.shared_properties_tableView->model())->GetProperties();
    model->UpdateDenitionSharedProperties(definition_, props);


    close();
}
