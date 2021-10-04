#pragma once
#include "Label.h"
#include "PropertyDatabase.h"

class OrientedPointLabel : public CloneableLabel<OrientedPointLabel> {
public:
    OrientedPointLabel(const WorldInfo *);

    QStringList GetPropertiesList() const override;

    void ConnectSharedProperties(bool connect, bool inject_my_values) override;
    
    void CenterTo(QPointF position, double angle) override;

	void OnPaint(const PaintInfo &, PaintExtraFunctions*) override;

    QTransform GetTransform(bool scale, bool rotate) override;

    QStringList ToStringsList() const override;

    void FromStringsList(const QStringList &) override;

    void HandlePositionChanged(LabelHandle* h, const QPointF & offset) override;

    bool Rotate(double angle) override;
	
	bool MoveBy(const QPointF & offset) override;
    
    void UpdateSharedProperties() override;

    void OnNewDefinition() override;

    LabelProperty *GetProperty(QString property_name) override;

private:
    void UpdateHandlesPositions();
    
private:
    LabelProperty angle_;

    static const int axis_length = 50;
};

