#pragma once
#include "Label.h"

class PolygonLabel : public CloneableLabel<PolygonLabel> {
public:
	PolygonLabel(const WorldInfo *);

	~PolygonLabel();

	void OnPaint(const PaintInfo &, PaintExtraFunctions*) override;

	bool IsCreationFinished() const override;

	bool OnCreateMove(const WorldInfo &) override;

	bool MoveBy(const QPointF & position, bool use_own_cs) override;

	void OnCreateClick(const WorldInfo &, bool) override;

	void CancelExtraAction() override;
    
	void HandlePositionChanged(LabelHandle* h, const QPointF & offset) override;

	void SetComputeVisualisationData(bool value) override;

	bool HitTest(const WorldInfo &) const override;

    double Area() const override;

	bool HasExtraAction(const WorldInfo &, QString & description) override;

	bool StartExtraAction(const WorldInfo &, QStringList & data) override;
	
	QStringList ToStringsList() const override;

	void FromStringsList(const QStringList &) override;

    QTransform GetTransform(bool scale, bool rotate) override;

private:
	bool IsNearStartPoint(const WorldInfo &) const;

	void Triangulate();

	enum class ExtraActionType {
		Nothing,
		DeleteHandle,
		CreateHandle,
		CutHole
	};

	typedef std::vector<std::shared_ptr<LabelHandle>> Handles;

    struct ExtraAction {
        ExtraActionType type = ExtraActionType::Nothing;
        int index = 0;
        std::shared_ptr<Handles> contour;
    };

	ExtraAction DetectExtraAction(const WorldInfo & wi);

	void RenderHandles(const PaintInfo &, const Handles &, QPointF *candidate);


private:
	enum class State {
		creation,
		ready,
		cutting_hole
	};

	State state_;
	
	std::list<std::shared_ptr<Handles>> contours_;

	QPointF next_point_;

	std::vector<QPointF> triangles_;

    double area_;

    QRectF aabb_;

    const int distance_to_finish_creation_ = 7; // in pixels
};
