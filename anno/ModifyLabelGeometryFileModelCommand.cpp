// Anno Labeling Tool
// 2020-2024 (c) urobots GmbH, https://urobots.io/en/portfolio/anno/

#include "ModifyLabelGeometryFileModelCommand.h"

ModifyLabelGeometryFileModelCommand::ModifyLabelGeometryFileModelCommand(FileModel* file, std::shared_ptr<Label> label, QStringList data)
    : file_(file)
    , label_(label)
    , data_(data)
{
    if (!data_.length())
        data_ = label_->ToStringsList();
}

void ModifyLabelGeometryFileModelCommand::undo() {
    ExchangeData();
    file_->NotifyUpdate(label_);
}

void ModifyLabelGeometryFileModelCommand::redo() {
    if (just_created_) {
        just_created_ = false;
    }
    else {
        ExchangeData();
    }
    file_->NotifyUpdate(label_);
}

void ModifyLabelGeometryFileModelCommand::ExchangeData() {
    auto current_data = label_->ToStringsList();
    label_->FromStringsList(data_);
    data_ = current_data;
}
