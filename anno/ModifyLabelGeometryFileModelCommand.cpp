// Anno Labeling Tool
// 2020-2024 (c) urobots GmbH, https://urobots.io/en/portfolio/anno/

#include "ModifyLabelGeometryFileModelCommand.h"

ModifyLabelGeometryFileModelCommand::ModifyLabelGeometryFileModelCommand(FileModel* file, std::shared_ptr<Label> label, QStringList data)
    : file_(file)
    , label_(label)
{
    before_ = label_->CaptureState(file_->GetVideoTime(), data);
}

void ModifyLabelGeometryFileModelCommand::undo() {
    // dynamic labels: the state is restored for the current frame of the video
    after_ = label_->CaptureState(file_->GetVideoTime());
    label_->RestoreState(before_, file_->GetVideoTime());
    file_->NotifyUpdate(label_);
}

void ModifyLabelGeometryFileModelCommand::redo() {
    if (just_created_) {
        just_created_ = false;
    }
    else {
        label_->RestoreState(after_, file_->GetVideoTime());
    }
    file_->NotifyUpdate(label_);
}
