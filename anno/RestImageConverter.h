#pragma once
#include "ImageConverter.h"
#include <QJsonObject>

struct RestImageConverter final : public ImageConverter {
    RestImageConverter(QJsonObject params) : params_(params) {}

    ImageData ConvertImage(const ImageData&, QString filename, std::shared_ptr<FilesystemInterface> filesystem, QString & error) override;

private:
    QJsonObject params_;
};

