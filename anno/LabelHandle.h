#pragma once
#include "PaintInfo.h"

class Label;

class LabelHandle {
public:
	LabelHandle(Label *parent);
    LabelHandle(QPointF pos, Label *parent);
	virtual ~LabelHandle();

	virtual void OnPaint(const PaintInfo &);

	QPointF GetPosition() const { return pos_; }

	// TODO(ap): source (default, left_mouse, right_mouse)
	virtual void SetPosition(QPointF pos, bool notify_parent = true);

	Label* GetParentLabel() const { return parent_; }

    void ClearParent() { parent_ = nullptr; }

    void SetEnabled(bool enabled) { enabled_ = enabled; };

    bool Enabled() { return enabled_; };

private:
    bool enabled_ = true;
	QPointF pos_;
	Label *parent_;
};