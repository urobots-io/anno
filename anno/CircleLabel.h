#pragma once
#include "Label.h"
#include "PropertyDatabase.h"

class CircleLabel : public CloneableLabel<CircleLabel> {
public:
	CircleLabel(const WorldInfo * wi);	

    void InitStamp() override;

    QStringList GetPropertiesList() const override;

    void ConnectSharedProperties(bool connect, bool inject_my_values) override;

    void CenterTo(QPointF position, double angle) override;

	void OnPaint(const PaintInfo &, PaintExtraFunctions*) override;

    bool HitTest(const WorldInfo &) const override;

    double Area() const override;

	bool IsCreationFinished() const override;

	bool OnCreateMove(const WorldInfo &) override;

	void OnCreateClick(const WorldInfo &, bool) override;
    
	void HandlePositionChanged(LabelHandle* h, const QPointF & offset) override;

	QStringList ToStringsList() const override;

	void FromStringsList(const QStringList &) override;

    QTransform GetTransform(bool scale, bool rotate) override;

	bool MoveBy(const QPointF & offset) override;

    void UpdateSharedProperties(bool forced_update = false) override;

    LabelProperty *GetProperty(QString property_name) override;

private:
	bool creation_completed_;
    LabelProperty radius_;
};