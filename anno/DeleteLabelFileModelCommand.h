#pragma once
#include "FileModel.h"

class DeleteLabelFileModelCommand : public QUndoCommand {
public:
    DeleteLabelFileModelCommand(FileModel*, std::shared_ptr<Label> label);

    void undo() override;
    void redo() override;

private:
    FileModel *file_;
    std::shared_ptr<Label> label_;
};
