#pragma once
#include "FileModel.h"

class ModifyLabelCategoryFileModelCommand : public QUndoCommand {
public:
    ModifyLabelCategoryFileModelCommand(FileModel*, std::shared_ptr<Label> label, int category);

    void undo() override;
    void redo() override;

    std::shared_ptr<Label> GetLabel() const { return label_; }

private:
    void ExchangeData();

private:
    FileModel *file_;
    std::shared_ptr<Label> label_;
    int category_ = 0;
};