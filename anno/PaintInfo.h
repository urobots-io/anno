#pragma once
#include <qpainter.h>

struct PaintExtraFunctions {
    virtual void AddHint(QString text, QPointF position, QColor color) = 0;
};

struct PaintInfo {
    QPainter *painter = nullptr;
    float world_scale = 1.f;
    bool is_selected = false;
    bool is_highlighted = false;
};
