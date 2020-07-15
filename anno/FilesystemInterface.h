#pragma once
#include "FileTreeItemInfo.h"

struct FilesystemInterface {
    /// Returns path of the root folder. Returns empty string if root folder is not local.
    virtual QString GetLocalPath(QString filename) const = 0;

    /// Returns content of the folder, non-recursive.
    virtual std::vector<FileTreeItemInfo> LoadFolder(QStringList path) = 0;
    
    /// Returns content of the file.
    virtual QByteArray LoadFile(QString filename) = 0;
    
    /// Deletes file or folder.
    virtual bool Remove(QString filename) = 0;

    /// Create a subfolder in the destination folder.
    virtual bool CreateSubfolder(const QStringList destination, QString name) = 0;

    /// Copy local file to the destination folder.
    virtual bool CopyLocalFile(const QStringList destination, QString source_path) = 0;
};
