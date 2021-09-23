// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
//
// Anno Labeling Tool
// 2020-2021 (c) urobots GmbH, https://urobots.io

#include "rest.h"
#include <QEventLoop>
#include <QHttpMultiPart>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <memory>

using namespace std;

namespace rest {

const char* ContentType::binary = "application/octet-stream";
const char* ContentType::json = "application/json";

QByteArray ExchangeData(const QString url, const QJsonObject & json, const QString accept){
    QByteArray json_string = QJsonDocument(json).toJson(QJsonDocument::Compact);

    QNetworkRequest request;
    request.setUrl(QUrl(url));
    request.setRawHeader("Content-Type", ContentType::json);
    request.setRawHeader("Content-Length", QByteArray::number(json_string.size()));
    request.setRawHeader("Accept", accept.toUtf8());

    QNetworkAccessManager netman;
    unique_ptr<QNetworkReply> reply(netman.post(request, json_string));

    QEventLoop loop;
    QObject::connect(reply.get(), SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    if (reply->error()) {
        throw std::runtime_error(reply->errorString().toStdString().c_str());
    }

    return reply->readAll();
}

QByteArray ExchangeData(const QString url, const QJsonObject & json, const QString data_name, QString file_name, QByteArray data, const QString accept) {
    QByteArray json_string = QJsonDocument(json).toJson(QJsonDocument::Compact);

    QNetworkRequest request;
    request.setUrl(QUrl(url));       
    request.setRawHeader("Accept", accept.toUtf8());

    QHttpMultiPart multipart(QHttpMultiPart::FormDataType);

    QHttpPart kwargs_part;    
    kwargs_part.setHeader(QNetworkRequest::ContentTypeHeader, ContentType::json);
    kwargs_part.setHeader(QNetworkRequest::ContentDispositionHeader, "form-data; name = \"_kwargs_\"");
    kwargs_part.setBody(json_string);
    multipart.append(kwargs_part);

    QHttpPart data_part;
    data_part.setHeader(QNetworkRequest::ContentTypeHeader, ContentType::binary);
    data_part.setHeader(QNetworkRequest::ContentDispositionHeader, 
        "form-data; name = \"" + data_name + "\"; filename=\"" + file_name + "\"");
    data_part.setBody(data);
    multipart.append(data_part);
       

    QNetworkAccessManager netman;
    unique_ptr<QNetworkReply> reply(netman.post(request, &multipart));

    QEventLoop loop;
    QObject::connect(reply.get(), SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    if (reply->error()) {
        throw std::runtime_error(reply->errorString().toStdString().c_str());
    }

    return reply->readAll();
}

}
