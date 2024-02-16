// Anno Labeling Tool
// 2020-2024 (c) urobots GmbH, https://urobots.io/en/portfolio/anno/

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
    auto category = label_->GetCategory();
    auto definition = label_->GetDefinition();
    if (!category || !definition) {
        return;
    }
    
    auto new_category = definition->GetCategory(category_);
    if (!new_category) {
        return;
    }

    category_ = category->get_value();
    label_->SetCategory(new_category);        
}
