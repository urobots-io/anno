#pragma once
#include "FileModel.h"

/// Undoable change of the label geometry. Stores the complete geometry state
/// (including the keyframes of dynamic video labels) before the change; the state
/// after the change is captured when the command is undone.
class ModifyLabelGeometryFileModelCommand : public QUndoCommand {
public:
    /// <data>: geometry before the change, if the label is already modified
    ModifyLabelGeometryFileModelCommand(FileModel*, std::shared_ptr<Label> label, QStringList data = {});

    void undo() override;
    void redo() override;

    std::shared_ptr<Label> GetLabel() const { return label_; }

private:
    FileModel *file_;
    std::shared_ptr<Label> label_;
    Label::GeometryState before_;
    Label::GeometryState after_;
    bool just_created_ = true;
};
