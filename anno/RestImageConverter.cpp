#include "RestImageConverter.h"
#include "rest.h"

using namespace std;

ImageData RestImageConverter::ConvertImage(const ImageData & image, QString filename, std::shared_ptr<FilesystemInterface> filesystem, QString & error) {
    ImageData result_image;

#ifndef ANNO_USE_OPENCV
    Q_UNUSED(image)
    Q_UNUSED(filename)
    Q_UNUSED(filesystem)
    Q_UNUSED(error)
#else
    using namespace cv;

    auto url = params_["url"].toString();

    QString config;
    auto config_file = params_["config"].toString();
    if (config_file.length()) {
        config_file = QFileInfo(filename).path() + "/" + config_file;

        auto file_data = filesystem->LoadFile(config_file);
        if (file_data.size()) {
            config = QString(file_data);
        }
    }

    BlobPacker request_blobs;
    QJsonObject json;
    if (config.length())
        json.insert("config", config);

    json.insert("width", image.cols);
    json.insert("height", image.rows);

    vector<Mat> rgb(3);
    split(image, rgb);
    json.insert("r", request_blobs.AddBlob((const char*)rgb[0].data, image.cols * image.rows, false));
    json.insert("g", request_blobs.AddBlob((const char*)rgb[1].data, image.cols * image.rows, false));
    json.insert("b", request_blobs.AddBlob((const char*)rgb[2].data, image.cols * image.rows, false));
    auto json_text = QJsonDocument(json).toJson(QJsonDocument::Compact);    

    try {
        BlobPacker response_blobs;
        std::vector<char> response;

        rest::ExchangeData(url, json_text, request_blobs, response, response_blobs);

        auto json = QJsonDocument::fromJson(QString::fromLatin1(&response[0]).toUtf8()).object();
        int width = json["width"].toString().toInt();
        int height = json["height"].toString().toInt();

        if (width * height > 0) {
            Mat channelR(height, width, CV_8UC1, (void*)(response_blobs.GetBlob(json["r"].toString().toInt()).first));
            Mat channelG(height, width, CV_8UC1, (void*)(response_blobs.GetBlob(json["g"].toString().toInt()).first));
            Mat channelB(height, width, CV_8UC1, (void*)(response_blobs.GetBlob(json["b"].toString().toInt()).first));
            std::vector<Mat> channels{ channelB, channelG, channelR };
            merge(channels, result_image);
        }        
    }
    catch (std::exception & e) {
        error = e.what();        
    }
#endif

    return result_image;
}
