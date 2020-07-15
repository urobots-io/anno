#pragma once
#include "FileModel.h"

class DeleteAllLabelsFileModelCommand : public QUndoCommand {
public:
    DeleteAllLabelsFileModelCommand(FileModel*);

    void undo() override;
    void redo() override;

private:
    FileModel *file_;
    std::vector<std::shared_ptr<Label>> labels_;
};


