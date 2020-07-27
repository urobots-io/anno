#pragma once
#include "FileModel.h"

class SourcePicturesTreeModel;

struct FileTreeElement {
    enum class ElementState {
        /// element created, but not loaded
        created,

        /// loading is in progress
        loading,

        /// element loaded
        loaded
    };

    ElementState state = ElementState::created;

    FileTreeElement *parent = nullptr;

    int parent_index = 0;

    QString name;

    bool is_folder = false;

    bool is_valid = true;

    QPixmap thumbnail;

    std::vector<FileTreeElement*> children;

    FileTreeElement(SourcePicturesTreeModel* parent);
    ~FileTreeElement();

    std::shared_ptr<FileModel> GetFileModel();

    void SetFileModel(std::shared_ptr<FileModel> file_model);
    void AddChild(FileTreeElement *child);

    QStringList GetPathList() const;

private:
    std::shared_ptr<FileModel> file_model_;
    SourcePicturesTreeModel* parent_ = nullptr;
};

