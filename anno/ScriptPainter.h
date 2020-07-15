#pragma once
#include "Label.h"
#include <QObject>

class QJSEngine;

struct SharedPropertyDefinitionGadget {
    Q_GADGET

public:
    double a = 1.0;
    double b = 0;
    
    Q_PROPERTY(double a MEMBER a)
    Q_PROPERTY(double b MEMBER b)
};

Q_DECLARE_METATYPE(SharedPropertyDefinitionGadget)

class ScriptPainter : public QObject {
    Q_OBJECT

public:
    ScriptPainter(QObject* parent = nullptr);

    void RenderLabel(QJSEngine & engine, Label *label);

public slots:
    /// return standard property value
    double Property(QString property_name);
    
    /// return custom property definition
    SharedPropertyDefinitionGadget PropertyDefinition(QString property_name);

    /// return custom property value
    QVariant Custom(QString property_name);
    
    /// render native label class graphics
    void RenderBase() {
        if (label) label->OnPaint(pi);
    }

    /// set label coordinate system
    void SetBaseTransform(bool scale, bool rotate) {
        if (!label) return;
        auto l_transform = label->GetTransform(scale, rotate);
        painter()->setTransform(original_transform);
        painter()->setTransform(l_transform, true);
    }

    /// set label coordinate system cosmetic
    // todo(ap): add Scale(), remove this
    void SetBaseTransformCosmetic(bool rotate) {
        if (!label) return;
        auto l_transform = label->GetTransform(false, rotate);
        painter()->setTransform(original_transform);
        painter()->setTransform(l_transform, true);
        double s = 1. / pi.world_scale;
        painter()->setTransform(QTransform().scale(s, s), true);
    }

    /// setup original picture coordinate system
    void ResetTransform() {
        painter()->setTransform(original_transform);
    }

    /// translate 
    void Translate(double dx, double dy) {
        painter()->setTransform(QTransform().translate(dx, dy), true);
    }

    /// rotate
    void Rotate(double angle) {
        painter()->setTransform(QTransform().rotate(angle), true);
    }

    /// set default outline pen
    void SetDefaultPen() {
        if (label) painter()->setPen(label->GetOutlinePen(pi));
    }

    /// set default outline pen
    void SetDefaultPen(int width) {
        if (label) {
            QPen pen = label->GetOutlinePen(pi);
            pen.setWidth(abs(width));
            pen.setCosmetic(width < 0);
            painter()->setPen(pen);
        }
    }
    
    /// set custom pen
    void SetPen(int r, int g, int b, int width) {
        QPen pen(Qt::SolidLine);
        pen.setColor(QColor(r, g, b));
        pen.setWidth(abs(width));
        if (width < 0) pen.setCosmetic(true);
        painter()->setPen(pen);
    }

    /// draw line
    void DrawLine(double x0, double y0, double x1, double y1) {
        painter()->drawLine(QPointF(x0, y0), QPointF(x1, y1));
    }

    /// draw rectangle
    void DrawRect(double left, double top, double width, double height) {
        painter()->drawRect(QRectF(left, top, width, height));
    }

    /// draw ellipse
    void DrawEllipse(double left, double top, double width, double height) {
        painter()->drawEllipse(QRectF(left, top, width, height));
    }

    /// draw arc
    void DrawArc(double left, double top, double width, double height, double startAngle, double endAngle) {
        auto start = int(startAngle * 16);
        auto span = int((endAngle - startAngle) * 16);
        painter()->drawArc(QRectF(left, top, width, height), start, span);
    }

    /// draw circle
    void DrawCircle(double x, double y, double radius) {
        painter()->drawEllipse(QPointF(x, y), radius, radius);
    }

    /// draw text
    void DrawText(double x, double y, QString text, int font_size) {
        auto font = painter()->font();
        if (!font.bold() || font.pointSize() != font_size) {
            font.setBold(true);
            font.setPointSize(font_size);
            painter()->setFont(font);
        }
        painter()->drawText(QPointF(x, y), text);
    }

private:
    inline QPainter* painter() const { return pi.painter; }

public:
    PaintInfo pi;
    Label *label = nullptr;
    QTransform original_transform;
};
