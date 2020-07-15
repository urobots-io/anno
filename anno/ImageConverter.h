#pragma once
#include "FilesystemInterface.h"
#include "ImageData.h"
#include <memory>

struct ImageConverter {
    virtual ImageData ConvertImage(const ImageData &, QString filename, std::shared_ptr<FilesystemInterface> filesystem, QString & error) = 0;
};
