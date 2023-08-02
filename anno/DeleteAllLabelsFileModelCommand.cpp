// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
//
// Anno Labeling Tool
// 2020-2023 (c) urobots GmbH, https://urobots.io

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