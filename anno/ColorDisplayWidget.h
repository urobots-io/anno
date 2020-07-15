#pragma once
#include "implement_q_property.h"
#include <QWidget>

class ColorDisplayWidget : public QWidget
{
    Q_OBJECT

public:
    ColorDisplayWidget(QWidget *parent);
    ~ColorDisplayWidget();

    void paintEvent(QPaintEvent*) override;

    void SetColor(const std::vector<float> values);

private:
    QColor color_ = QColor(0, 0, 0, 0);
};
