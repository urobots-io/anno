#pragma once
#include "Label.h"

class PointLabel : public CloneableLabel<PointLabel> {
public:
	PointLabel(const WorldInfo *);		

    void InitStamp() override;

    void CenterTo(QPointF position, double angle) override;

	void OnPaint(const PaintInfo &, PaintExtraFunctions*) override;

    QTransform GetTransform(bool scale, bool rotate) override;

	bool MoveBy(const QPointF & offset) override;
};

