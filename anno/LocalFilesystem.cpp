#include "LocalFilesystem.h"
#include <QStringBuilder>
#include <fstream>

using namespace std;

vector<FileTreeItemInfo> LocalFilesystem::LoadFolder(QStringList path) {
    QDir directory(root_path_ + "/" + path.join("/"));

    return LoadFolder(directory);
}

std::vector<FileTreeItemInfo> LocalFilesystem::LoadFolder(QDir directory) {
    vector<FileTreeItemInfo> result;

    for (auto dir : directory.entryList({}, QDir::Dirs | QDir::NoDotAndDotDot)) {
        result.push_back({ dir,  true });
    }

    for (auto file : directory.entryList({}, QDir::Files)) {
        result.push_back({ file,  false });
    }

    return result;
}

QString LocalFilesystem::GetLocalPath(QString filename) const {
    return root_path_ + "/" + filename;
}

bool LocalFilesystem::CreateSubfolder(const QStringList destination, QString name) {
    QString destination_abs = root_path_ + "/" + destination.join("/");
    return QDir(destination_abs).mkpath(name);
}

bool LocalFilesystem::CopyLocalFile(const QStringList destination, QString source_path) {
    QString new_path = 
        root_path_ % "/" % 
        destination.join("/") % "/" % 
        QFileInfo(source_path).fileName();
    return QFile::copy(source_path, new_path);
}

QByteArray LocalFilesystem::LoadFile(QString filename) {
    QFile file(GetLocalPath(filename));
    if (file.open(QIODevice::ReadOnly)) {
        return file.readAll();
    }
    return {};
}

bool LocalFilesystem::Remove(QString filename) {
    auto abs_path = GetLocalPath(filename);    
    QFileInfo fi(abs_path);
    if (fi.exists()) {
        if (fi.isDir()) {
            return QDir(abs_path).removeRecursively();
        }
        else {
            return QFile(abs_path).remove();
        }
    }
    else {
        return true;
    }
}



