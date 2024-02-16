// Anno Labeling Tool
// 2020-2024 (c) urobots GmbH, https://urobots.io/en/portfolio/anno/

#include "ModifyLabelTextFileModelCommand.h"

ModifyLabelTextFileModelCommand::ModifyLabelTextFileModelCommand(FileModel* file, std::shared_ptr<Label> label, QString text)
    : file_(file)
    , label_(label)
    , text_(text)
{
}

void ModifyLabelTextFileModelCommand::undo() {
    ExchangeData();
    file_->NotifyUpdate(label_);
}

void ModifyLabelTextFileModelCommand::redo() {
    ExchangeData();
    file_->NotifyUpdate(label_);
}

void ModifyLabelTextFileModelCommand::ExchangeData() {
    auto current_text = label_->GetText();
    label_->SetText(text_);
    text_ = current_text;
}