// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
//
// Anno Labeling Tool
// 2020-2021 (c) urobots GmbH, https://urobots.io

#include "ModifyLabelCategoryFileModelCommand.h"

ModifyLabelCategoryFileModelCommand::ModifyLabelCategoryFileModelCommand(FileModel* file, std::shared_ptr<Label> label, int category)
    : file_(file)
    , label_(label)
    , category_(category)
{
}

void ModifyLabelCategoryFileModelCommand::undo() {
    ExchangeData();
    file_->NotifyUpdate(label_);
}

void ModifyLabelCategoryFileModelCommand::redo() {
    ExchangeData();
    file_->NotifyUpdate(label_);
}

void ModifyLabelCategoryFileModelCommand::ExchangeData() {
    auto current_category = label_->GetCategory()->value;    
    label_->SetCategory(label_->GetCategory()->definition->GetCategory(category_).get());
    category_ = current_category;
}
