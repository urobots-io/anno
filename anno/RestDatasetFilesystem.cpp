// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
//
// Anno Labeling Tool
// 2020-2021 (c) urobots GmbH, https://urobots.io

#include "RestDatasetFilesystem.h"
#include "rest.h"
#include <QFileInfo>
#include <QVariant>

using namespace std;
using namespace rest;

RestDatasetFilesystem::RestDatasetFilesystem(QString dataset_object_address, QString root_path)
    : dataset_object_address_(dataset_object_address)
    , root_path_(root_path) {
    
    if (!root_path_.endsWith("/"))
        root_path_ += "/";
}

QJsonObject RestDatasetFilesystem::Params(QString path) {    
    QJsonObject json;
    json.insert("path", QJsonValue::fromVariant(root_path_ + path));
    return json;
}

QString RestDatasetFilesystem::FunctionUrl(QString function) {
    return dataset_object_address_ + "/attrs/" + function + "/call/";
}

QByteArray RestDatasetFilesystem::CallDatasetFunction(QString function, QJsonObject params, QString accept_type) {        
    return ExchangeData(FunctionUrl(function), params, accept_type);
}

vector<FileTreeItemInfo> RestDatasetFilesystem::LoadFolder(QStringList path) {
    QByteArray json_content;
    try {
        json_content = CallDatasetFunction("dir", Params(path.join("/")), ContentType::json);
    }
    catch (exception&) {
        return {};
    }
    
    QJsonDocument json = QJsonDocument().fromJson(json_content);
    if (json.isNull()) {
        return{};
    }

    auto retval = json.object()["retval"].toObject();

    vector<FileTreeItemInfo> result;
    int ifolder = 0;

    foreach(const QString& key, retval.keys()) {
        auto type = retval.value(key).toString();
        if (type == "file") {
            result.push_back({ key, false });
        }
        else {
            result.insert(result.begin() + ifolder, { key, true });
            ++ifolder;
        }
    }
    return result;
}

QByteArray RestDatasetFilesystem::LoadFile(QString filename) {
    QByteArray bytes;
    try {
        bytes = CallDatasetFunction("read_file", Params(filename), ContentType::binary);
    }
    catch (exception&) {
        return{};
    }
    return bytes;
}

bool RestDatasetFilesystem::Remove(QString filename) {
    QByteArray bytes;
    try {
        bytes = CallDatasetFunction("remove", Params(filename), ContentType::json);
    }
    catch (exception&) {
        return false;
    }
    return true;
}

bool RestDatasetFilesystem::CreateSubfolder(const QStringList destination, QString name) {
    auto dir = (QStringList() << destination << name).join("/");    
    auto params = Params(dir);
    params.insert("exist_ok", QJsonValue::fromVariant(false));
    QByteArray bytes;
    try {
        bytes = CallDatasetFunction("makedirs", params, ContentType::json);
    }
    catch (exception&) {
        return false;
    }
    return true;
}

bool RestDatasetFilesystem::CopyLocalFile(const QStringList destination, QString source_path) {
    QString file_name = QFileInfo(source_path).fileName();
    auto dir = (QStringList() << destination << file_name).join("/");
    auto params = Params(dir);

    QFile file(source_path);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QByteArray file_data = file.readAll();
    QByteArray bytes;
    try {
        bytes = ExchangeData(FunctionUrl("write_file"), params, "data", file_name, file_data, ContentType::json);
    }
    catch (exception&) {
        return false;
    }
    return false;
}

bool RestDatasetFilesystem::Rename(const QStringList path, const QStringList new_path) {    
    auto params = Params(path.join("/"));
    params.insert("new_path", QJsonValue::fromVariant(root_path_ + new_path.join("/")));
    QByteArray bytes;
    try {
        bytes = CallDatasetFunction("move", params, ContentType::json);
    }
    catch (exception&) {
        return false;
    }
    return true;
}
