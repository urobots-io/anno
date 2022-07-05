#pragma once
#ifdef ANNO_USE_OPENCV
#include "ImageSettingsWidgetOpenCV.h"
typedef ImageSettingsWidgetOpenCV ImageSettingsWidget;
#else
#include "ImageSettingsWidgetQt.h"
typedef ImageSettingsWidgetQt ImageSettingsWidget;
#endif