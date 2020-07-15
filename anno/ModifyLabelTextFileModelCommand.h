#pragma once
#include "FileModel.h"

class ModifyLabelTextFileModelCommand : public QUndoCommand {
public:
    ModifyLabelTextFileModelCommand(FileModel*, std::shared_ptr<Label> label, QString text);

    void undo() override;
    void redo() override;

    std::shared_ptr<Label> GetLabel() const { return label_; }

private:
    void ExchangeData();

private:
    FileModel *file_;
    std::shared_ptr<Label> label_;
    QString text_ = 0;
};