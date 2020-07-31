#pragma once
#include <QByteArray>
#include <QJsonDocument>

namespace rest {

struct ContentType {
    static const char* binary;
    static const char* json;
};

/// Exchange data in json format
QByteArray ExchangeData(const QString url, const QJsonObject & json, const QString accept);

/// Exchange data in json format, attach binary multipart
QByteArray ExchangeData(const QString url, const QJsonObject & json, QString data_name, QString file_name, QByteArray data, const QString accept);

}
