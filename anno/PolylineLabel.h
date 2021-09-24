#pragma once
#include "Label.h"

class PolylineLabel : public CloneableLabel<PolylineLabel> {
public:
    PolylineLabel(const WorldInfo *);

    ~PolylineLabel();

    void OnPaint(const PaintInfo &, PaintExtraFunctions*) override;

    bool IsCreationFinished() const override;

    bool OnCreateMove(const WorldInfo &) override;

    void OnCreateClick(const WorldInfo &, bool) override;

    bool MoveBy(const QPointF & offset) override;

    bool ForceCompleteCreation(const WorldInfo &) override;

    void CancelExtraAction() override;  

    bool HasExtraAction(const WorldInfo &, QString & description) override;

    bool StartExtraAction(const WorldInfo &, QStringList & data) override;

    QStringList ToStringsList() const override;

    void FromStringsList(const QStringList &) override;

    QTransform GetTransform(bool scale, bool rotate) override;

    void HandlePositionChanged(LabelHandle *, const QPointF &) override;

    bool HitTest(const WorldInfo & wi) const override;

protected:
    enum class ExtraActionType {
        Nothing,
        DeleteHandle,
        CreateHandle
    };

    typedef std::vector<LabelHandle*> Handles;

    struct ExtraAction {
        int index = 0;
        ExtraActionType type = ExtraActionType::Nothing;
    };

    ExtraAction DetectExtraAction(const WorldInfo & wi) const;    

    virtual void UpdateInternalData();

protected:
    enum class State {
        creation,
        ready
    };

    State state_;

    QPointF next_point_;   

    QRectF aabb_;
};
