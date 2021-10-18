// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
//
// Anno Labeling Tool
// 2020-2021 (c) urobots GmbH, https://urobots.io

#include "FileModel.h"
#include "CreateLabelFileModelCommand.h"
#include "DeleteAllLabelsFileModelCommand.h"
#include "DeleteLabelFileModelCommand.h"
#include "LabelFactory.h"
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

void FileModel::Delete(std::shared_ptr<LabelDefinition> marker, std::shared_ptr<LabelCategory> category) {
    bool file_updated = false;
    for (auto i = labels_.begin(); i != labels_.end();) {
        auto label_category = (*i)->GetCategory();
        auto label_marker = label_category->GetDefinition();
        if ((category && label_category == category) || (marker && label_marker == marker)) {
            file_updated = true;
            i = labels_.erase(i);
        }
        else {
            ++i;
        }
    }

    if (file_updated) {
        GetUndoStack()->clear();
        set_is_modified(true);
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

    auto def = label->GetDefinition();
    if (!def->is_shared())
        return label;

    // find proxy of this label
    for (auto l : labels_) {
        if (l->GetDefinition() == def && l->GetSharedLabelIndex() == label->GetSharedLabelIndex()) {
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

    if (auto def = label_ptr->GetDefinition()) {
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
            if (l->GetDefinition() == def && l->GetSharedLabelIndex() == label_ptr->GetSharedLabelIndex()) {
                return l;
            }
        }
    }

    return{};
}

void FileModel::CreateDefaultSharedLabel(shared_ptr<LabelCategory> category) {
    auto def = category->GetDefinition();
    assert(def);
    if (!def)
        return;

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

set<int> FileModel::GetExistingSharedIndexes(std::shared_ptr<LabelDefinition> def) {
    assert(def->is_shared());

    set<int> result;    
    if (def->is_shared()) {
        for (auto label : labels_) {
            if (label->GetDefinition() == def) {
                result.insert(label->GetSharedLabelIndex());
            }
        }
    }
    return result;
}

void FileModel::ReconnectSharedProperties(std::shared_ptr<LabelDefinition> def) {
    bool file_updated = false;
    for (auto l: labels_) {
        if (l->GetDefinition() == def) {
            file_updated = true;
            l->ConnectSharedProperties(false, false);
            l->ConnectSharedProperties(true, false);
            l->UpdateSharedProperties(true);
        }
    }

    if (file_updated) {
        GetUndoStack()->clear();
        set_is_modified(true);
    }
}

void FileModel::UpdateDefinitionSharedLabels(std::shared_ptr<LabelDefinition> def, std::vector<std::shared_ptr<Label>>& shared_labels) {
    bool file_updated = false;
    auto old_number = int(def->shared_labels.size());
    auto new_number = int(shared_labels.size());
    if (old_number && !new_number) {
        // sharing is disabled -> convert from shared to not shared
        auto it = labels_.begin();
        while (it != labels_.end()) {
            auto l = *it;
            if (l->GetDefinition() == def) {
                it = labels_.erase(it);
                auto new_label = LabelFactory::CreateLabel(def->value_type);
                new_label->SetCategory(l->GetCategory());
                new_label->FromStringsList(l->ToStringsList());
                it = labels_.insert(it, new_label) + 1;
                file_updated = true;
            }
            else {
                ++it;
            }
        }
    }
    else if (new_number < old_number) {
        // shared count decreased - remove labels with shared index > new max index
        auto it = labels_.begin();
        while (it != labels_.end()) {
            auto l = *it;
            if (l->GetDefinition() == def && l->GetSharedLabelIndex() >= new_number) {
                it = labels_.erase(it);
                file_updated = true;
            }
            else {
                ++it;
            }
        }
    }
    else if (!old_number && new_number) {
        // sharing is enabled - try to convert from non-shared to shared if possible, delete rest of labels.
        int index = 0;
        auto it = labels_.begin();
        while (it != labels_.end()) {
            auto l = *it;
            if (l->GetDefinition() == def) {
                it = labels_.erase(it);
                if (index < new_number) {
                    auto new_label = make_shared<ProxyLabel>(shared_labels[index]);
                    new_label->FromStringsList(l->ToStringsList());
                    new_label->SetSharedLabelIndex(index);
                    it = labels_.insert(it, new_label) + 1;
                    ++index;
                    file_updated = true;
                }
            }
            else {
                ++it;
            }
        }
    }
    else  {
        // --> new_number >= old_number
        // shared count increased - do nothing
    }

    if (file_updated) {
        GetUndoStack()->clear();
        set_is_modified(true);
    }
}

void FileModel::UpdateDefinitionCustomProperties(std::shared_ptr<LabelDefinition> def, std::vector<CustomPropertyDefinition> props, QStringList original_names) {
    bool file_updated = false;

    for (auto l : labels_) {
        if (l->GetDefinition() != def) {
            continue;
        }

        auto &label_properties = l->GetCustomProperties();
        // TODO
    }

    if (file_updated) {
        GetUndoStack()->clear();
        set_is_modified(true);
    }
}