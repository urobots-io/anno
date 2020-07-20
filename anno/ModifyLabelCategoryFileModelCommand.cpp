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
