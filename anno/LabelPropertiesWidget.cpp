// Anno Labeling Tool
// 2020-2024 (c) urobots GmbH, https://urobots.io/en/portfolio/anno/

#include "LabelPropertiesWidget.h"
#include "CustomPropertyTableItemDelegate.h"
#include <QClipboard>

LabelPropertiesWidget::LabelPropertiesWidget(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

    connect(ui.delete_pushButton, &QPushButton::clicked, this, &LabelPropertiesWidget::OnDeleteLabel);
    connect(ui.copy_label_data_pushButton, &QPushButton::clicked, this, &LabelPropertiesWidget::CopyLabelProperties);
	connect(ui.text_lineEdit, &QLineEdit::textEdited, this, &LabelPropertiesWidget::SetLabelText);
    
    OnSelectedLabelChanged({}, {});
}

LabelPropertiesWidget::~LabelPropertiesWidget()
{
}

void LabelPropertiesWidget::UpdateLabel(std::shared_ptr<Label> label) {
    if (label && selected_label_ == label) {
        ui.text_lineEdit->setText(label->GetText());
    }
}

void LabelPropertiesWidget::CopyLabelProperties() {
    if (selected_label_) {
        if (auto clipboard = QGuiApplication::clipboard()) {
            auto data = selected_label_->ToStringsList();
            auto comment = selected_label_->GetComment();
            if (!comment.isEmpty()) {
                data.insert(0, "/** " + comment + " */");
            }
            clipboard->setText(data.join("\n"));
        }
    }
}

void LabelPropertiesWidget::OnSelectedLabelChanged(std::shared_ptr<FileModel> file_model, std::shared_ptr<Label> label) {
    bool file_model_changed = file_model != selected_file_;

    // disconnect from the old file
    if (file_model_changed && selected_file_) {
        selected_file_->disconnect(this);
    }

	selected_label_ = label;
    selected_file_ = file_model;
	
    // connect to the new file
    if (file_model_changed && selected_file_) {
        connect(selected_file_.get(), &FileModel::select_label_required, 
            this, &LabelPropertiesWidget::UpdateLabel);
    }

    // update displayed values
    ui.delete_pushButton->setEnabled(label != nullptr);
    ui.text_lineEdit->setEnabled(label != nullptr);

    if (!label) {
        ui.type_name_lineEdit->setText(QString());
        ui.category_lineEdit->setText(QString());
        ui.text_lineEdit->setText(QString());
        ui.shared_index_lineEdit->setText(QString());

        delete ui.custom_properties_tableView->model();
        ui.custom_properties_tableView->setModel(nullptr);
    }
    else {
        auto cat = label->GetCategory();
        if (auto def = cat->GetDefinition()) {
            ui.type_name_lineEdit->setText(def->get_type_name());
        }
        ui.category_lineEdit->setText(cat->get_name());
        ui.text_lineEdit->setText(label->GetText());

        if (label->IsProxyLabel()) {
            ui.shared_index_lineEdit->setText(QString("%0").arg(label->GetSharedLabelIndex()));
        }
        else {
            ui.shared_index_lineEdit->setText("-");
        }

        CustomPropertyTableItemDelegate::SetupTableView(ui.custom_properties_tableView, selected_file_, label);
        ui.custom_properties_tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    }
}

void LabelPropertiesWidget::SetLabelText(QString text) {
    if (selected_label_ && selected_file_ && selected_label_->GetText() != text) {
        selected_file_->ModifyLabelText(selected_label_, text);        
    }
}


