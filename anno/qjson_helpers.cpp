// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
//
// Anno Labeling Tool
// 2020-2021 (c) urobots GmbH, https://urobots.io

#include "qjson_helpers.h"
#include <QJsonArray>
#include <QStringList>
#include <QTextCodec>
#include <QVariant>

QStringList ToStringList(const QJsonValue & value) {
    QStringList result;
    if (!value.isNull() && value.isArray()) {
        for (auto i : value.toArray()) {
            result << i.toString();
        }
    }
    return result;
}

QString ArrayToString(const QJsonValue & value) {
    QStringList result;
    if (!value.isNull() && value.isArray()) {
        for (auto i : value.toArray()) {
            result << i.toString();
        }
    }
    return result.join("\n");
}

QJsonArray ToJsonArray(const QStringList & value) {
    QJsonArray result;
    for (QString s : value) {
        result.push_back(QJsonValue::fromVariant(s));
    }
    return result;
}

QJsonArray ToJsonArray(const QString & value) {
    return ToJsonArray(value.split("\n"));
}

QJsonDocument LoadJsonFromText(const QByteArray& json_content, QString & error_text) {
    QJsonParseError error;
    QJsonDocument json = QJsonDocument().fromJson(json_content, &error);
    if (json.isNull()) {
        auto utf_8_codec = 106;
        auto lines_before_error = QTextCodec::codecForMib(utf_8_codec)->toUnicode(
            json_content.constBegin(), error.offset + 80).count('\n');

        auto string = QTextCodec::codecForMib(utf_8_codec)->toUnicode(
            json_content.constBegin() + std::max(0, error.offset - 1),
            std::min<int>(error.offset + 80, json_content.size()) - error.offset);

        error_text = 
            QString::fromLatin1("Format error: %0, error at line %1:\n\n%2...")
            .arg(error.errorString())
            .arg(lines_before_error + 1)
            .arg(string);
    }
    return json;
}
