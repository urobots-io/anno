#pragma once
#include "Label.h"
#include "PropertyDatabase.h"

class OrientedCircleLabel : public CloneableLabel<OrientedCircleLabel> {
public:
    OrientedCircleLabel(const WorldInfo *);

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

    QTransform GetTransform(bool scale, bool rotate) override;

    QStringList ToStringsList() const override;

    void FromStringsList(const QStringList &) override;

    void HandlePositionChanged(LabelHandle* h, const QPointF & offset) override;

    bool Rotate(double angle) override;
	
	bool MoveBy(const QPointF & offset) override;
    
    void UpdateSharedProperties(bool forced_update = false) override;

    void OnNewDefinition() override;

    LabelProperty *GetProperty(QString property_name) override;

private:
    void UpdateHandlesPositions();
    
private:
    LabelProperty angle_;
    LabelProperty radius_;

    bool creation_completed_;    

    static const int axis_length = 50;
};

