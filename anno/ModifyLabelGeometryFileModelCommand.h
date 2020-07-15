#pragma once
#include "FileModel.h"

class ModifyLabelGeometryFileModelCommand : public QUndoCommand {
public:
    ModifyLabelGeometryFileModelCommand(FileModel*, std::shared_ptr<Label> label, QStringList data = {});

    void undo() override;
    void redo() override;

    std::shared_ptr<Label> GetLabel() const { return label_; }

private:
    void ExchangeData();

private:
    FileModel *file_;
    std::shared_ptr<Label> label_;
    QStringList data_;
    bool just_created_ = true;
};