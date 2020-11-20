#pragma once
#include "PolylineLabel.h"

class ToolLabel : public PolylineLabel {
public:
    ToolLabel(const WorldInfo *);

    ~ToolLabel();

    void OnPaint(const PaintInfo &) override;

private:
    QPen GetHelperPen(const PaintInfo & pi) const;
};
