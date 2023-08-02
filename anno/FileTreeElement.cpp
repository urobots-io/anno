// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
//
// Anno Labeling Tool
// 2020-2023 (c) urobots GmbH, https://urobots.io

#include "FileTreeElement.h"
#include "SourcePicturesTreeModel.h"

FileTreeElement::FileTreeElement(SourcePicturesTreeModel* parent) : parent_(parent) {

}

FileTreeElement::~FileTreeElement() {
    if (file_model_ && parent_)
        parent_->UnregisterFileModel(file_model_.get());
}

std::shared_ptr<FileModel> FileTreeElement::GetFileModel() {
    return file_model_;
}

void FileTreeElement::SetFileModel(std::shared_ptr<FileModel> file_model) {
    if (file_model_ && parent_)
        parent_->UnregisterFileModel(file_model_.get());

    file_model_ = file_model;

    if (file_model_ && parent_)
        parent_->RegisterFileModel(file_model_.get());
}

void FileTreeElement::AddChild(FileTreeElement *child) {
    child->parent = this;
    child->parent_index = int(children.size());
    children.push_back(child);
}

QStringList FileTreeElement::GetPathList() const {
    // Traverse up until 2nd level of the tree.
    QStringList path;
    for (const FileTreeElement* i = this; i->parent && i->parent->parent; i = i->parent)
        path.insert(0, i->name);
    return path;
}

