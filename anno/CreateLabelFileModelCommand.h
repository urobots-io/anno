#pragma once
#include "FileModel.h"

class CreateLabelFileModelCommand : public QUndoCommand {
public:
    CreateLabelFileModelCommand(FileModel*, std::shared_ptr<Label> label);

    void undo() override;
    void redo() override;

    std::shared_ptr<Label> GetLabel() const { return label_; }

private:
    FileModel *file_;
    std::shared_ptr<Label> label_;
};

