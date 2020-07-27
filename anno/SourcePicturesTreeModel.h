#pragma once
#include "implement_q_property.h"
#include "FilesystemInterface.h"
#include "FileModel.h"
#include "FileTreeElement.h"
#include <QAbstractItemModel>
#include <QFileInfo>
#include <QObject>
#include <QProgressDialog>
#include <QSortFilterProxyModel>

struct FileModelProviderInterface {
    /// Get file model defined by the path.
    virtual std::shared_ptr<FileModel> GetFileModel(QStringList path) = 0;

    /// Get names of files and folders in the path.
    virtual std::vector<FileTreeItemInfo> GetFolderInfo(QStringList path) = 0;

    /// Remove all markers for files in this folder, recursively.
    virtual void DeleteAllLabels(QStringList path) = 0;

    /// Rename file model. If source is a folder, rename all included models, recursively.
    virtual void Rename(QStringList source, QStringList destination) = 0;
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
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
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

    QModelIndex GetFilesRootIndex();


    void RegisterFileModel(FileModel*);
    void UnregisterFileModel(FileModel*);

public slots:
    void OnFileModelModified(bool);

private:
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

    FileTreeElement* CreateChild(const FileTreeItemInfo &, FileTreeElement* parent,const QStringList &path, bool is_valid);

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

