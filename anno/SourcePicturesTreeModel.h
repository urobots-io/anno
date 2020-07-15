#pragma once
#include "implement_q_property.h"
#include "FilesystemInterface.h"
#include "FileModel.h"
#include <QAbstractItemModel>
#include <QFileInfo>
#include <QObject>
#include <QProgressDialog>
#include <QSortFilterProxyModel>

struct FileModelProviderInterface {
    /// get file model defined by the path
    virtual std::shared_ptr<FileModel> GetFileModel(QStringList path) = 0;

    /// get names of files and folders in the path
    virtual std::vector<FileTreeItemInfo> GetFolderInfo(QStringList path) = 0;

    /// remove all markers for files in this folder, recursively
    virtual void DeleteAllLabels(QStringList path) = 0;
};

class SourcePicturesTreeModel : public QAbstractItemModel
{
	Q_OBJECT

public:
	SourcePicturesTreeModel(std::shared_ptr<FilesystemInterface> loader, FileModelProviderInterface *file_model_provider, QWidget *parent);
	~SourcePicturesTreeModel();

    bool canFetchMore(const QModelIndex &parent) const override;
    void fetchMore(const QModelIndex &parent) override;

    QVariant data(const QModelIndex &index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;    

    std::shared_ptr<FileModel> GetFileModel(const QModelIndex &);
    FileTreeItemInfo GetFileInfo(const QModelIndex &);

    bool Remove(const QModelIndex &, QString & error);
    bool CreateSubfolder(const QModelIndex &, QString);
    void ReloadFolder(const QModelIndex &, int, int);    
    //  TODO: make use of name_filters
    void InsertFiles(const QModelIndex & index, int row, int column, const QList<QUrl> &urls, const QStringList &name_filters = QStringList());

    std::shared_ptr<FilesystemInterface> GetFileSystem() const { return loader_; }

    bool IsCompletelyLoaded() const { return completely_loaded_; }
    void LoadCompletely();

public slots:
    void OnFileModelModified(bool);

private:
    enum class ElementState {
        /// element created, but not loaded
        created,

        /// loading is in progress
        loading,

        /// element loaded
        loaded
    };

    struct FileTreeElement {
        ElementState state = ElementState::created;

        FileTreeElement *parent = nullptr;
        
        int parent_index = 0;

        QString name;
        
        bool is_folder = false;

        bool is_valid = true;

        QPixmap thumbnail;
        
        std::vector<FileTreeElement*> children;

        FileTreeElement(SourcePicturesTreeModel* parent) : parent_(parent) {

        }

        ~FileTreeElement() {
            if (file_model_ && parent_)
                parent_->UnregisterFileModel(file_model_.get());
        }

        std::shared_ptr<FileModel> GetFileModel() {
            return file_model_;
        }

        void SetFileModel(std::shared_ptr<FileModel> file_model) {
            if (file_model_ && parent_)
                parent_->UnregisterFileModel(file_model_.get());

            file_model_ = file_model;

            if (file_model_ && parent_)
                parent_->RegisterFileModel(file_model_.get());
        }
        
        void AddChild(FileTreeElement *child) {
            child->parent = this;
            child->parent_index = int(children.size());
            children.push_back(child);
        }

    private:
        std::shared_ptr<FileModel> file_model_;
        SourcePicturesTreeModel* parent_ = nullptr;
    };

    //  A structure to store info and pointers to a parent and children. It is a mix of the classes QFileInfo and FileTreeElement
    struct CopiedObjectInfo
    {
        CopiedObjectInfo(CopiedObjectInfo* parent = nullptr) : parent_(parent) { if (parent != nullptr) parent->children.push_back(this); }
        ~CopiedObjectInfo() { qDeleteAll(children); }
        CopiedObjectInfo* parent_;
        std::vector<CopiedObjectInfo*> children;
        QString file_name;
        QString absolute_path;
        bool is_folder;
    };

    void LoadChildren(QModelIndex index, FileTreeElement*);
    void RecursiveDelete(FileTreeElement*);

    void RegisterFileModel(FileModel*);
    void UnregisterFileModel(FileModel*);

    FileTreeElement* CreateChild(const FileTreeItemInfo &, FileTreeElement* parent,const QStringList &path, bool is_valid);
    QStringList GetPathList(FileTreeElement*) const;

    CopiedObjectInfo* CreateCopiedObjectInfoFromFileInfo(const QFileInfo &info, CopiedObjectInfo* parent);
    void CreateCopiedObjectInfoRecursively(CopiedObjectInfo* parent);
    void InsertObjectsRecursively(CopiedObjectInfo* object, QStringList path);

    QStringList mimeTypes() const override;
    bool canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const override;
    QMimeData *mimeData(const QModelIndexList &indexes) const override;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;
    Qt::DropActions supportedDragActions() const override;
    Qt::DropActions supportedDropActions() const override;

    void RecursiveLoadChildren(FileTreeElement*);

private:
    FileTreeElement *root_ = nullptr;        
    std::shared_ptr<FilesystemInterface> loader_;
    FileModelProviderInterface *file_model_provider_ = nullptr;
    QWidget* parent_widget_;
    int initial_copied_objects_count, already_copied_objects_count;
    QProgressDialog* current_progress_dialog_;
    bool completely_loaded_ = false;
};


class SourcePicturesProxyModel : public QSortFilterProxyModel {
    Q_OBJECT

public:
    Q_PROPERTY(bool show_labeled_only READ get_show_labeled_only WRITE set_show_labeled_only NOTIFY show_labeled_only_changed);

    SourcePicturesProxyModel(QObject *parent) : QSortFilterProxyModel(parent) 
    {
    }

    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

public slots:
    DECLARE_Q_PROPERTY_WRITE(bool, show_labeled_only);

signals:    
    void show_labeled_only_changed(bool);

private:
    bool show_labeled_only_ = false;

public:
    IMPLEMENT_Q_PROPERTY_READ(show_labeled_only)
};

