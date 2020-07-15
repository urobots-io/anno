#pragma once
#include "Label.h"

class PointLabel : public CloneableLabel<PointLabel> {
public:
	PointLabel(const WorldInfo *);		

    void InitStamp() override;

    void CenterTo(QPointF position, double angle) override;

	void OnPaint(const PaintInfo &) override;

    QTransform GetTransform(bool scale, bool rotate) override;

	bool MoveBy(QPointF offset) override;
};

