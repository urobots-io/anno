#pragma once
#include "FilesystemInterface.h"
#include "ImageData.h"
#include <memory>

//
// Interface to convert images before showing them on the desktop.
// 
struct ImageConverter {
    virtual ImageData ConvertImage(const ImageData &, QString filename, std::shared_ptr<FilesystemInterface> filesystem, QString & error) = 0;
};
