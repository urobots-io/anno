#include "FileModel.h"
#include "CreateLabelFileModelCommand.h"
#include "DeleteAllLabelsFileModelCommand.h"
#include "DeleteLabelFileModelCommand.h"
#include "ModifyLabelCategoryFileModelCommand.h"
#include "ModifyLabelGeometryFileModelCommand.h"
#include "ModifyLabelTextFileModelCommand.h"
#include "ProxyLabel.h"

using namespace std;

FileModel::FileModel(QString id)
: id_(id)
{
}

FileModel::~FileModel()
{
}

void FileModel::DeleteAllLabels() {
    if (labels_.size()) {
        PushCommand(new DeleteAllLabelsFileModelCommand(this));
    }
}


void FileModel::PushCommand(QUndoCommand * command) {
    undo_stack_.push(command);    
}

void FileModel::ModifyLabelGeometry(std::shared_ptr<Label> label, QStringList data) {
    PushCommand(new ModifyLabelGeometryFileModelCommand(this, label, data));
}

void FileModel::ModifyLabelCategory(std::shared_ptr<Label> label, int category) {
    PushCommand(new ModifyLabelCategoryFileModelCommand(this, label, category));
}

void FileModel::ModifyLabelText(std::shared_ptr<Label> label, QString text) {
    if (undo_stack_.count() > 0) {
        auto modify_text_command = dynamic_cast<const ModifyLabelTextFileModelCommand*>(undo_stack_.command(undo_stack_.count() - 1));
        if (modify_text_command && modify_text_command->GetLabel() == label) {
            // we already saved previous text, do not add a new command to the stack
            label->SetText(text);
            NotifyUpdate(label);
            return;
        }
    }
    PushCommand(new ModifyLabelTextFileModelCommand(this, label, text));
}

std::shared_ptr<Label> FileModel::CreateLabel(std::shared_ptr<Label> label) {
    PushCommand(new CreateLabelFileModelCommand(this, label));
    return label;
}

std::shared_ptr<Label> FileModel::DeleteLabel(std::shared_ptr<Label> label) {
    if (!label->IsCreationFinished() && undo_stack_.count() > 0) {
        // disable previos creation command
        auto create_command = dynamic_cast<const CreateLabelFileModelCommand*>(undo_stack_.command(undo_stack_.count() - 1));
        if (create_command && create_command->GetLabel() == label) {
            undo_stack_.undo();
            undo_stack_.push(new QUndoCommand());
            undo_stack_.undo();
            return label;
        }
    }
    
    PushCommand(new DeleteLabelFileModelCommand(this, label));
    return label;
}

void FileModel::NotifyUpdate(bool clear_selection, std::shared_ptr<Label> label_to_select) {
    set_is_modified(true);
    file_updated(true);
    emit rendering_required();
    if (clear_selection) {
        emit select_label_required({});
    }
    else if (label_to_select) {
        emit select_label_required(label_to_select);
    }
}

void FileModel::NotifyUpdate(std::shared_ptr<Label> label_to_select) {
    NotifyUpdate(false, label_to_select);
}

std::shared_ptr<Label> FileModel::GetOwner(std::shared_ptr<Label> label) {
    if (!label)
        return nullptr;

    auto def = label->GetCategory()->definition;
    if (!def->is_shared())
        return label;

    // find proxy of this label
    for (auto l : labels_) {
        if (l->GetCategory()->definition == def && l->GetSharedLabelIndex() == label->GetSharedLabelIndex()) {
            return l;
        }
    }

    return{};
}

std::shared_ptr<Label> FileModel::GetHandleOwner(std::shared_ptr<LabelHandle> handle) {
    if (!handle)
        return nullptr;

    auto label_ptr = handle->GetParentLabel();
    if (!label_ptr) {
        return nullptr;
    }

    auto def = label_ptr->GetCategory()->definition;
    if (!def->is_shared()) {
        // this is needed, because there is currently a mix between shared_ptr to Label everywhere
        // and pointer to Label in Handle class.
        for (auto l : labels_) {
            if (l.get() == label_ptr) {
                return l;
            }
        }
    }

    // find proxy of this label
    for (auto l : labels_) {
        if (l->GetCategory()->definition == def && l->GetSharedLabelIndex() == label_ptr->GetSharedLabelIndex()) {
            return l;
        }
    }

    return{};
}

void FileModel::CreateDefaultSharedLabel(LabelCategory * category) {
    auto def = category->definition;

    assert(def->is_shared());
    if (!def->is_shared())
        return;

    auto missing_indexes = def->GetMissingIndexes(GetExistingSharedIndexes(def));
    for (auto i : missing_indexes) {
        if (def->AllowedForFile(this, i)) {
            auto shared_label = def->shared_labels[i];
            // make sure label property values are actual
            shared_label->UpdateSharedProperties();
            // create proxies
            auto proxy = std::make_shared<ProxyLabel>(shared_label);
            proxy->SetCategory(category);
            proxy->SetSharedLabelIndex(i);
            CreateLabel(proxy);
        }
    }
}

set<int> FileModel::GetExistingSharedIndexes(LabelDefinition *def) {
    assert(def->is_shared());

    set<int> result;    
    if (def->is_shared()) {
        for (auto label : labels_) {
            if (label->GetCategory()->definition == def) {
                result.insert(label->GetSharedLabelIndex());
            }
        }
    }
    return result;
}

