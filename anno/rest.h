#pragma once
#include "BlobPacker.h"
#include <QByteArray>
#include <QJsonDocument>

namespace rest {

struct ContentType {
    static const char* binary;
    static const char* json;
};

/// Exchange data using BlobPacker format
// TODO: remove or refactor, this feature was used only in ids_experiemnta project
void ExchangeData(const QString url,
    const QString request_json, const BlobPacker & request_blobs,
    std::vector<char> & response, BlobPacker & response_blobs);

/// Exchange data in json format
QByteArray ExchangeData(const QString url, const QJsonObject & json, const QString accept);

/// Exchange data in json format, attach binary multipart
QByteArray ExchangeData(const QString url, const QJsonObject & json, QString data_name, QString file_name, QByteArray data, const QString accept);

}
