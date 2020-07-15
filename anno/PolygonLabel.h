#pragma once
#include "Label.h"

class PolygonLabel : public CloneableLabel<PolygonLabel> {
public:
	PolygonLabel(const WorldInfo *);

	~PolygonLabel();

	void OnPaint(const PaintInfo &) override;

	bool IsCreationFinished() const override;

	bool OnCreateMove(const WorldInfo &) override;

	bool MoveBy(QPointF position) override;

	void OnCreateClick(const WorldInfo &, bool) override;

	void CancelExtraAction() override;
    
	void HandlePositionChanged(LabelHandle* h, QPointF offset) override;

	void SetComputeVisualisationData(bool value) override;

	bool HitTest(const WorldInfo &) const override;

    double Area() const override;

	bool HasExtraAction(const WorldInfo &, QString & description) override;

	bool StartExtraAction(const WorldInfo &, QStringList & data) override;
	
	QStringList ToStringsList() override;

	void FromStringsList(QStringList const &) override;

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

	ExtraActionType DetectExtraAction(const WorldInfo & wi, int & index, Handles *&contour);

	void RenderHandles(const PaintInfo &, const Handles &, QPointF *candidate);


private:
	enum class State {
		creation,
		ready,
		cutting_hole
	};

	State state_;
	
	std::list<Handles*> contours_;

	QPointF next_point_;

	std::vector<QPointF> triangles_;

    double area_;

    QRectF aabb_;
};
