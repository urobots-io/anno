#include "rest.h"
#include <QEventLoop>
#include <QHttpMultiPart>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QNetworkAccessManager>

using namespace std;

namespace rest {

const char* ContentType::binary = "application/octet-stream";
const char* ContentType::json = "application/json";

void ExchangeData(const QString url,
    const QString request_json, const BlobPacker & request_blobs,
    std::vector<char> & response,
    BlobPacker & response_blobs)
{
	auto request_json_string = request_json.toStdString();

    vector<char> data_to_send(request_json_string.size() + 1 + request_blobs.GetSize());
    // copy json text including trailing \0
    memcpy(&data_to_send[0], request_json_string.c_str(), request_json_string.size() + 1);
    // copy bytes
    request_blobs.WriteTo(&data_to_send[request_json_string.size() + 1]);

    QNetworkRequest request;
    request.setUrl(QUrl(url));
    request.setRawHeader("Content-Type", ContentType::binary);

    QNetworkAccessManager netman;
    unique_ptr<QNetworkReply> reply(netman.post(request, QByteArray::fromRawData(&data_to_send[0], int(data_to_send.size()))));

    QEventLoop loop;
    QObject::connect(reply.get(), SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    if (reply->error()) {
        throw std::runtime_error(reply->errorString().toStdString().c_str());
    }

    // read data from response
    const int initial_size = 1024 * 1024;
    response.resize(initial_size);
    for (size_t offset = 0;;) {
        auto bytes_read = reply->read(&response[offset], response.size() - offset);
        if (bytes_read <= 0) {
            response.resize(offset);
            break;
        }
        else {
            offset += bytes_read;
            if (response.size() < offset * 2) {
                response.resize(response.size() * 2);
            }
        }
    }

    auto data_start_it = std::find(response.begin(), response.end(), '\0');
    if (data_start_it == response.end()) {
        throw std::runtime_error("Wrong response: json end not found");
    }
    auto data_start = data_start_it - response.begin();

    response_blobs = BlobPacker();
    response_blobs.ReadFrom(
        &response[data_start + 1],
        int(response.size()) - data_start - 1,
        false);
}

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
