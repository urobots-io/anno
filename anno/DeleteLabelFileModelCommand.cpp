#include "DeleteLabelFileModelCommand.h"

DeleteLabelFileModelCommand::DeleteLabelFileModelCommand(FileModel* file, std::shared_ptr<Label> label)
    : file_(file)
    , label_(label) {
}

void DeleteLabelFileModelCommand::undo() {
    file_->labels_.push_back(label_);
    file_->NotifyUpdate(label_);
}

void DeleteLabelFileModelCommand::redo() {
    auto it = std::find_if(
        file_->labels_.begin(),
        file_->labels_.end(),
        [&](std::shared_ptr<Label> & label) { return label.get() == label_.get(); });

    if (it == file_->labels_.end())
        return;

    file_->labels_.erase(it);
    file_->NotifyUpdate(true);
}