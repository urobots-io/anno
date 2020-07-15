#pragma once
#include "Label.h"
#include "PropertyDatabase.h"

class RectLabel : public CloneableLabel<RectLabel> {
public:
	RectLabel(const WorldInfo *);    

    void InitStamp() override;

    void ConnectSharedProperties(bool connect, bool inject_my_values) override;

    void CenterTo(QPointF position, double angle) override;

	void OnPaint(const PaintInfo &) override;

    bool HitTest(const WorldInfo &) const override;

    double Area() const override;

	bool IsCreationFinished() const override;

	bool OnCreateMove(const WorldInfo &) override;

	void OnCreateClick(const WorldInfo &, bool) override;

    QTransform GetTransform(bool scale, bool rotate) override;

	bool MoveBy(QPointF offset) override;

    void HandlePositionChanged(LabelHandle* h, QPointF offset) override;

    QStringList ToStringsList() override;

    void FromStringsList(QStringList const &) override;

    void UpdateSharedProperties() override;

    LabelProperty *GetProperty(QString property_name) override;

    QString GetComment() override;

private:
    void UpdateHandlesPositions();
	bool creation_completed_;

    LabelProperty width_;
    LabelProperty height_;
};
