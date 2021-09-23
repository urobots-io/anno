// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
//
// Anno Labeling Tool
// 2020-2021 (c) urobots GmbH, https://urobots.io

#include "LocalFilesystem.h"
#include <QStringBuilder>
#include <fstream>

using namespace std;

LocalFilesystem::LocalFilesystem(QString root_path)
    : root_path_(root_path)
{
}

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

    QFileInfo fi(destination_abs + name);
    if (fi.exists()) {
        if (fi.isDir()) {
            return false;
        }
    }
    
    return QDir(destination_abs).mkpath(name);
}

bool LocalFilesystem::CopyLocalFile(const QStringList destination, QString source_path) {
    QString new_path = 
        root_path_ % "/" % 
        destination.join("/") % "/" % 
        QFileInfo(source_path).fileName();

    //qDebug(source_path.toLatin1());
    //qDebug(new_path.toLatin1());

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

bool LocalFilesystem::Rename(const QStringList source, const QStringList destination) {
    auto abs_source = GetLocalPath(source.join("/"));
    auto abs_destination = GetLocalPath(destination.join("/"));
    QFileInfo fi(abs_source);
    if (fi.exists()) {
        if (fi.isDir()) {
            return QDir().rename(abs_source, abs_destination);
        }
        else {
            return QFile().rename(abs_source, abs_destination);
        }
    }
    return false;
}



