#include "SourcePicturesTreeModel.h"
#include <QDebug>
#include <QDir>
#include <QIcon>
#include <QMimeData>
#include <QUrl>

using namespace std;

SourcePicturesTreeModel::SourcePicturesTreeModel(std::shared_ptr<FilesystemInterface> loader, FileModelProviderInterface *file_model_provider, QWidget *parent)
: QAbstractItemModel(parent)
, loader_(loader)
, file_model_provider_(file_model_provider)
{	
    // create root element
    root_ = new FileTreeElement(this);
    root_->is_folder = true;
    root_->name = "__root__";
    root_->state = FileTreeElement::ElementState::loaded;

    // create root for the filesystem
    auto files = new FileTreeElement(this);
    files->is_folder = true;
    files->name = tr("Files");
    root_->AddChild(files);

    parent_widget_ = parent;
}

SourcePicturesTreeModel::~SourcePicturesTreeModel() {
    RecursiveDelete(root_);
}

QModelIndex SourcePicturesTreeModel::GetFilesRootIndex() {
    return createIndex(0, 0, root_->children[0]);
}

void SourcePicturesTreeModel::RecursiveDelete(FileTreeElement * e) {
    for (auto c : e->children)
        RecursiveDelete(c);

    delete e;
}

std::shared_ptr<FileModel> SourcePicturesTreeModel::GetFileModel(const QModelIndex & index) {
    if (!index.isValid())
        return nullptr;

    auto item = static_cast<FileTreeElement*>(index.internalPointer());
    return item->GetFileModel();
}

FileTreeItemInfo SourcePicturesTreeModel::GetFileInfo(const QModelIndex & index) {
    if (!index.isValid())
        return {};

    auto item = static_cast<FileTreeElement*>(index.internalPointer());
    QStringList path;
    for (FileTreeElement* i = item; i != root_ && i->parent != root_; i = i->parent)
        path.insert(0, i->name);

    return { path.join("/"), item->is_folder };
}

QVariant SourcePicturesTreeModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid())
        return QVariant();

    auto item = static_cast<FileTreeElement*>(index.internalPointer());  
    bool is_modified = false;
    if (auto model = item->GetFileModel()) {
        is_modified = model->get_is_modified();
    }

    if (role == Qt::DisplayRole) {
        if (is_modified) return item->name + "*";
        else return item->name;
    }
    else if (role == Qt::DecorationRole) {        
        if (index.column() == 0) {
            if (item->is_folder) {
                return QIcon(":/MainWindow/Resources/folder.ico");
            }
            else {
                if(item->GetFileModel()->HaveLabels())
                    return QIcon(":/MainWindow/Resources/file_blue.ico");
                else
                    return QIcon(":/MainWindow/Resources/file.ico");
            }
        }
    } 
    else if (role == Qt::FontRole) {
        if (is_modified) {
            QFont font;
            font.setBold(true);
            return font;
        }
    }
    else if (role == Qt::ForegroundRole) {
        if (!item->is_valid) {
            return  QVariant(QColor(Qt::red));
        }
    }
    else if (role == Qt::EditRole) {
        return item->name;
    }

    return QVariant();
}

bool SourcePicturesTreeModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (role == Qt::EditRole) {
        auto item = static_cast<FileTreeElement*>(index.internalPointer());        
        auto source = item->GetPathList();
        auto destination = source;
        destination.last() = value.toString();

        if (loader_->Rename(source, destination)) {
            file_model_provider_->Rename(source, destination);            
            item->name = destination.last();
            return true;
        }
    }

    return false;
}

Qt::ItemFlags SourcePicturesTreeModel::flags(const QModelIndex &index) const {
    if (!index.isValid()) {
        return QAbstractItemModel::flags(index);
    }

    auto item = static_cast<FileTreeElement*>(index.internalPointer());
    return (item && item->is_folder ? Qt::ItemIsDropEnabled : Qt::NoItemFlags) 
        | QAbstractItemModel::flags(index)
        | ((item->parent && item->parent != root_) ? Qt::ItemIsEditable : Qt::NoItemFlags);
}

QVariant SourcePicturesTreeModel::headerData(int section, Qt::Orientation orientation, int role) const {
    Q_UNUSED(section)
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return tr("Name");

    return QVariant();
}

QModelIndex SourcePicturesTreeModel::index(int row, int column, const QModelIndex &parent) const {
    if (!hasIndex(row, column, parent))
        return QModelIndex();

     auto parent_item = parent.isValid()
        ? static_cast<FileTreeElement*>(parent.internalPointer())
        : root_;
    
     auto child_item = row < int(parent_item->children.size())
         ? parent_item->children[row]
         : nullptr;    

    if (child_item)
        return createIndex(row, column, child_item);
    else
        return QModelIndex();
}

QModelIndex SourcePicturesTreeModel::parent(const QModelIndex &index) const {
    if (!index.isValid())
        return QModelIndex();

    auto child_item = static_cast<FileTreeElement*>(index.internalPointer());
    auto parent_item = child_item->parent;

    if (parent_item == root_)
        return QModelIndex();

    return createIndex(parent_item->parent_index, 0, parent_item);
}

bool SourcePicturesTreeModel::canFetchMore(const QModelIndex &parent) const {
    if (parent.column() > 0)
        return false;

    auto item = parent.isValid()
        ? static_cast<FileTreeElement*>(parent.internalPointer())
        : root_;
    
    return item && item->is_folder && item->state == FileTreeElement::ElementState::created;
}

void SourcePicturesTreeModel::fetchMore(const QModelIndex &parent) {
    if (parent.column() > 0)
        return;    

    auto item = parent.isValid()
        ? static_cast<FileTreeElement*>(parent.internalPointer())
        : root_;

    if (item && item->is_folder && item->state == FileTreeElement::ElementState::created) {
        LoadChildren(parent, item);
    }
}

int SourcePicturesTreeModel::rowCount(const QModelIndex & parent) const {
    if (parent.column() > 0)
        return 0;

    auto item = parent.isValid()
        ? static_cast<FileTreeElement*>(parent.internalPointer())
        : root_;

    if (item && item->is_folder && item->state == FileTreeElement::ElementState::created) {
        return 1; // we do not know how many items we have
    }
    
    return int(item->children.size());
}

int SourcePicturesTreeModel::columnCount(const QModelIndex & parent) const {
    Q_UNUSED(parent)
    return 1;
}

void SourcePicturesTreeModel::RegisterFileModel(FileModel* file) {
    connect(file, &FileModel::is_modified_changed, this, &SourcePicturesTreeModel::OnFileModelModified);
    connect(file, &FileModel::file_updated, this, &SourcePicturesTreeModel::OnFileModelModified);
}

void SourcePicturesTreeModel::UnregisterFileModel(FileModel* file) {
    file->disconnect(this);
}

void SourcePicturesTreeModel::LoadCompletely() {
    if (!completely_loaded_) {
        RecursiveLoadChildren(root_);
        completely_loaded_ = true;
    }
}

void SourcePicturesTreeModel::RecursiveLoadChildren(FileTreeElement* e) {
    if (e->state == FileTreeElement::ElementState::created) {
        LoadChildren({}, e);
    }

    for (auto & c : e->children) {
        if (c->is_folder) {
            RecursiveLoadChildren(c);
        }
    }
}

void SourcePicturesTreeModel::LoadChildren(QModelIndex parentIndex, FileTreeElement* e) {
    if (!e->is_folder || e->state == FileTreeElement::ElementState::loading || !loader_)
        return;

    e->state = FileTreeElement::ElementState::loading;

    auto path = e->GetPathList();
    auto dir_files = loader_->LoadFolder(path);
    auto model_files = file_model_provider_->GetFolderInfo(path);
    int valid_files_count = int(dir_files.size());

    for (auto & m : model_files) {
        bool file_present = false;
        for (auto & f : dir_files) {
            if (f.name == m.name && f.is_folder == m.is_folder) {
                file_present = true;
                break;
            }
        }
        if (!file_present) {
            dir_files.push_back(m);
        }
    }

    if (dir_files.size()) {
        if (parentIndex.isValid()) {
            beginInsertRows(parentIndex, 0, int(dir_files.size()) - 1);
        }

        int index = 0;
        for (auto i : dir_files) {
            CreateChild(i, e, path, index++ < valid_files_count);
        }

        if (parentIndex.isValid()) {
            endInsertRows();
        }
    }

    e->state = FileTreeElement::ElementState::loaded;
}

bool SourcePicturesTreeModel::CreateSubfolder(const QModelIndex & index, QString name) {
    auto folder = static_cast<FileTreeElement*>(index.internalPointer());
    if (!folder || !folder->is_folder) {
        return false;
    }

    auto path = folder->GetPathList();
    if (!loader_->CreateSubfolder(path, name)) {
        return false;
    }

    if (folder->state == FileTreeElement::ElementState::loaded) {
        beginInsertRows(index, int(folder->children.size()), int(folder->children.size()));

        auto child = new FileTreeElement(this);
        child->is_folder = true;
        child->name = name;
        folder->AddChild(child);

        endInsertRows();
    }
    return true;
}

bool SourcePicturesTreeModel::Remove(const QModelIndex & index, QString & error) {
    auto parent_index = parent(index);

    auto file = static_cast<FileTreeElement*>(index.internalPointer());
    auto folder = static_cast<FileTreeElement*>(parent_index.internalPointer());
    if (!folder) {
        error = tr("Root folder cannot be deleted");
        return false;
    }
    
    int row = file->parent_index;
    auto file_model = file->GetFileModel();

    if (row >= int(folder->children.size()) || folder->children[row] != file) {
        error = tr("Unexpected tree state, please reload the tree");
        return false;
    }        
    
    auto file_path = file->GetPathList();
    if (!loader_->Remove(file_path.join("/"))) {
        error = tr("Failed to delete file or folder from the source location");
        return false;
    }
    
    if (file_model) {
        file_model->DeleteAllLabels();
    }
    else {
        file_model_provider_->DeleteAllLabels(file_path);
    }

    beginRemoveRows(parent_index, row, row);
    
    for (int i = row + 1; i < int(folder->children.size()); ++i) {
        --folder->children[i]->parent_index;
    }

    folder->children.erase(folder->children.begin() + row);
    
    endRemoveRows();

    return true;
}

void SourcePicturesTreeModel::ReloadFolder(const QModelIndex & index, int row, int column) {
    Q_UNUSED(row)
    Q_UNUSED(column)
    auto folder = static_cast<FileTreeElement*>(index.internalPointer());
    
    if (folder->children.size() > 0) {
        beginRemoveRows(index, 0, int(folder->children.size()) - 1);

        for (auto c : folder->children) {
            RecursiveDelete(c);
        }

        folder->children.clear();

        endRemoveRows();
    }

    folder->state = FileTreeElement::ElementState::created;

    LoadChildren(index, folder);
}

SourcePicturesTreeModel::CopiedObjectInfo * SourcePicturesTreeModel::CreateCopiedObjectInfoFromFileInfo(const QFileInfo &info, CopiedObjectInfo * parent) {
    CopiedObjectInfo* obj = new CopiedObjectInfo(parent);
    obj->is_folder = info.isDir();
    if (obj->is_folder) {
        QDir d(info.absoluteFilePath());
        obj->absolute_path = d.absolutePath();
        obj->file_name = d.dirName();
    }
    else {
        obj->absolute_path = info.absoluteFilePath();
        obj->file_name = info.fileName();
    }
    objects_to_copy_count_++;
    return obj;
}

void SourcePicturesTreeModel::InsertFiles(const QModelIndex & index, int row, int column, const QList<QUrl>& urls) {    
    if (!index.isValid())
        return;

    auto folder = static_cast<FileTreeElement*>(index.internalPointer());
    auto path = folder->GetPathList();

    QVector<CopiedObjectInfo*> root_elements_to_copy;
    root_elements_to_copy.reserve(urls.size());

    objects_to_copy_count_ = 0;
    for(auto url : urls) {
        if (!url.isLocalFile()) {
            continue;
        }
        //  Create an info about current url´s object first
        auto root_element = CreateCopiedObjectInfoFromFileInfo(QFileInfo(url.toLocalFile()), nullptr);
        root_elements_to_copy << root_element;
        //  Recursively create infos about all the children objects
        CreateCopiedObjectInfoRecursively(root_element);
    }

    // QDialog to indicate the progress and cancel it
    QProgressDialog progress(tr("Inserting files..."), tr("Cancel"), 0, objects_to_copy_count_, parent_widget_);
    progress.setWindowModality(Qt::WindowModality::WindowModal);
    current_progress_dialog_ = &progress;

    //  Using created infos copy all the objects
    copied_objects_count_ = 0;
    for (auto root_element: root_elements_to_copy) {
        InsertObjectsRecursively(root_element, path);
    }

    qDeleteAll(root_elements_to_copy);
    ReloadFolder(index,row, column);
}


void SourcePicturesTreeModel::CreateCopiedObjectInfoRecursively(CopiedObjectInfo * parent) {
    QDir directory(parent->absolute_path);
    auto entries = directory.entryInfoList({}, QDir::AllEntries | QDir::NoDotAndDotDot);
    for (int i = 0; i < entries.size(); ++i) {
        CopiedObjectInfo* info = CreateCopiedObjectInfoFromFileInfo(entries[i], parent);
        if (info && info->is_folder) {
            CreateCopiedObjectInfoRecursively(info);
        }
    }
}

void SourcePicturesTreeModel::InsertObjectsRecursively(CopiedObjectInfo * object, QStringList path) {
    if (current_progress_dialog_->wasCanceled())
        return;

    auto result = object->is_folder
        ? loader_->CreateSubfolder(path, object->file_name)
        : loader_->CopyLocalFile(path, object->absolute_path);

    // TODO(ap): handle result, show error to user.

    current_progress_dialog_->setValue(++copied_objects_count_);

    path.append(object->file_name);
    for (size_t i = 0; i < object->children.size(); ++i) {
        InsertObjectsRecursively(object->children[i], path);
    }
}

QStringList SourcePicturesTreeModel::mimeTypes() const {
    return QAbstractItemModel::mimeTypes();
}

bool SourcePicturesTreeModel::canDropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent) const {
    Q_UNUSED(action)
    Q_UNUSED(row)
    Q_UNUSED(column)
    Q_UNUSED(parent)
    QList<QUrl> urls = data->urls();
    return !urls.empty();
}

QMimeData * SourcePicturesTreeModel::mimeData(const QModelIndexList & indexes) const {
    return QAbstractItemModel::mimeData(indexes);
}

bool SourcePicturesTreeModel::dropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent) {
    Q_UNUSED(action)    
    InsertFiles(parent, row, column, data->urls());
    return true;
}

Qt::DropActions SourcePicturesTreeModel::supportedDragActions() const{
    return Qt::IgnoreAction;
}

Qt::DropActions SourcePicturesTreeModel::supportedDropActions() const{
    return QAbstractItemModel::supportedDropActions() | Qt::CopyAction;
}

FileTreeElement* SourcePicturesTreeModel::CreateChild(const FileTreeItemInfo &info, FileTreeElement * parent, const QStringList &path, bool is_valid) {
    auto child = new FileTreeElement(this);
    child->is_folder = info.is_folder;
    child->is_valid = is_valid;
    child->name = info.name;    

    if (!info.is_folder) {
        QStringList child_path = path;
        child_path.append(info.name);
        child->SetFileModel(file_model_provider_->GetFileModel(child_path));
    }

    parent->AddChild(child);
    return child;
}

void SourcePicturesTreeModel::OnFileModelModified(bool) {
    auto file_model = static_cast<FileModel*>(sender());
    auto parts = file_model->get_id().split('/');
    
    // search for the element startig from files
    int child_index = 0;
    FileTreeElement *child = root_->children[0];
    FileTreeElement *parent = nullptr;

    for (auto s : parts) {        
        parent = child;
        child = nullptr;
        child_index = -1;

        for (size_t t = 0; t < parent->children.size(); ++t) {
            if (parent->children[t]->name == s) {
                child_index = int(t);
                child = parent->children[t];
                break;
            }
        }

        if (!child)
            break;
    }

    if (child) {
        auto index = createIndex(child_index, 0, child);
        emit dataChanged(index, index);
    }
}

void SourcePicturesProxyModel::set_show_labeled_only(bool value) {
    if (value != show_labeled_only_) {
        show_labeled_only_ = value;
        emit show_labeled_only_changed(value);
        invalidateFilter();
    }
}


bool SourcePicturesProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const {
    if (!get_show_labeled_only()) {
        return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
    }

    if (!source_parent.isValid()) {
        return true;
    }

    auto result = QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
    
    auto source = (SourcePicturesTreeModel*)sourceModel();
    auto index = source->index(source_row, 0, source_parent);
    auto fi = source->GetFileInfo(index);

    if (fi.is_folder) {
        if (filterRegExp().isEmpty()) {
            return false;
        }
    }
    else {
        auto file = source->GetFileModel(index);
        if (!file || !file->HaveLabels()) {
            return false;
        }
    }

    return result;
}
