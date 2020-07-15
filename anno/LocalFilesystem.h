#pragma once
#include "FilesystemInterface.h"
#include <QDir>

struct LocalFilesystem final : public FilesystemInterface {
    LocalFilesystem(QString root_path) : root_path_(root_path) {}

    QString GetLocalPath(QString filename) const override;
    std::vector<FileTreeItemInfo> LoadFolder(QStringList path) override;
    std::vector<FileTreeItemInfo> LoadFolder(QDir directory);
    QByteArray LoadFile(QString filename) override;
    bool Remove(QString filename) override;
    bool CreateSubfolder(const QStringList destination, QString name) override;
    bool CopyLocalFile(const QStringList destination, QString source_path) override;

private:
    QString root_path_;
};

