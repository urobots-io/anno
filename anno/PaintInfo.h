#pragma once
#include <qpainter.h>

struct PaintInfo {
    QPainter *painter = nullptr;
    float world_scale = 1.f;
    bool is_selected = false;
    bool is_highlighted = false;
};
