#pragma once
#include "FilesystemInterface.h"
#include <QJsonObject>

struct RestDatasetFilesystem final : public FilesystemInterface {
    // local path does not exist
    QString GetLocalPath(QString filename) const override { Q_UNUSED(filename); return QString(); }
    RestDatasetFilesystem(QString dataset_object_address, QString root_path);
    std::vector<FileTreeItemInfo> LoadFolder(QStringList path) override;    
    QByteArray LoadFile(QString filename) override;
    bool Remove(QString filename) override;    
    bool CreateSubfolder(const QStringList destination, QString name) override;
    bool CopyLocalFile(const QStringList destination, QString source_path) override;

private:
    QJsonObject Params(QString path);
    QString FunctionUrl(QString function);
    QByteArray CallDatasetFunction(QString function, QJsonObject params, QString accept_type);
    
private:
    QString dataset_object_address_;
    QString root_path_;
};

