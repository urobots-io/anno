#pragma once
#include "Label.h"
#include "PropertyDatabase.h"

class OrientedRectLabel : public CloneableLabel<OrientedRectLabel> {
public:
	OrientedRectLabel(const WorldInfo *);	

    void InitStamp() override;

    void ConnectSharedProperties(bool connect, bool inject_my_values) override;

    void CenterTo(QPointF position, double angle) override;

	void OnPaint(const PaintInfo &) override;

    bool HitTest(const WorldInfo &) const override;

    double Area() const override;

	bool IsCreationFinished() const override;

	bool OnCreateMove(const WorldInfo &) override;

	void OnCreateClick(const WorldInfo &, bool) override;

	void CancelExtraAction() override;	

	void HandlePositionChanged(LabelHandle* h, QPointF offset) override;

	bool StartExtraAction(const WorldInfo &, QStringList &) override;

	QStringList ToStringsList() override;

	void FromStringsList(QStringList const &) override;

    QTransform GetTransform(bool scale, bool rotate) override;

    bool Rotate(double angle) override;

	bool MoveBy(QPointF offset) override;

    void UpdateSharedProperties() override;

    LabelProperty *GetProperty(QString property_name) override;

private:
	void UpdateHandlesPositions();

private:
	enum class State {
		creation_dimensions,
		ready,
		rotating
	};
	
	State state_;

    LabelProperty angle_;
    LabelProperty width_;
    LabelProperty height_;

	int rotating_index_;

	QPointF rotating_point_;
};
