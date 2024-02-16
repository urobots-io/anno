// Anno Labeling Tool
// 2020-2024 (c) urobots GmbH, https://urobots.io/en/portfolio/anno/

#include "DeleteAllLabelsFileModelCommand.h"

DeleteAllLabelsFileModelCommand::DeleteAllLabelsFileModelCommand(FileModel * file)
    : file_(file) {
}

void DeleteAllLabelsFileModelCommand::undo() {
    file_->labels_ = labels_;
    labels_.clear();
    file_->NotifyUpdate(true);
}

void DeleteAllLabelsFileModelCommand::redo() {
    labels_ = file_->labels_;
    file_->labels_.clear();
    file_->NotifyUpdate(true);
}