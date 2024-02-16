// Anno Labeling Tool
// 2020-2024 (c) urobots GmbH, https://urobots.io/en/portfolio/anno/

#include "ColorDisplayWidget.h"
#include <QPainter>

ColorDisplayWidget::ColorDisplayWidget(QWidget *parent)
    : QWidget(parent)
{
}

ColorDisplayWidget::~ColorDisplayWidget() {
}

void ColorDisplayWidget::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.fillRect(rect(), color_);
}

void ColorDisplayWidget::SetColor(const std::vector<float> values) {
    QColor color;
    if (values.size() == 1) {
        color.setRgb(values[0], values[0], values[0], 255);
    } else if (values.size() == 3) {
        color.setRgb(values[0], values[1], values[2], 255);
    } else {
        color.setAlpha(0);
    }
    if (color != color_) {
        color_ = color;
        update();
    }
}
