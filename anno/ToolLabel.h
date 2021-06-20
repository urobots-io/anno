#pragma once
#include "PolylineLabel.h"

class ToolLabel : public PolylineLabel {
public:
    ToolLabel(const WorldInfo *);

    ~ToolLabel();

    void OnPaint(const PaintInfo &, PaintExtraFunctions*) override;

private:
    void Paint2(const PaintInfo &, PaintExtraFunctions*);
    void Paint3(const PaintInfo &, PaintExtraFunctions*);
    void Paint4(const PaintInfo &, PaintExtraFunctions*);

private:
    QPen GetHelperPen(const PaintInfo & pi) const;
};
