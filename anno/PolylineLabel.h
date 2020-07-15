#pragma once
#include "Label.h"

class PolylineLabel : public CloneableLabel<PolylineLabel> {
public:
    PolylineLabel(const WorldInfo *);

    ~PolylineLabel();

    void OnPaint(const PaintInfo &) override;

    bool IsCreationFinished() const override;

    bool OnCreateMove(const WorldInfo &) override;

    void OnCreateClick(const WorldInfo &, bool) override;

    bool MoveBy(QPointF offset) override;

    bool ForceCompleteCreation(const WorldInfo &) override;

    void CancelExtraAction() override;  

    bool HasExtraAction(const WorldInfo &, QString & description) override;

    bool StartExtraAction(const WorldInfo &, QStringList & data) override;

    QStringList ToStringsList() override;

    void FromStringsList(QStringList const &) override;

    QTransform GetTransform(bool scale, bool rotate) override;

private:
    enum class ExtraActionType {
        Nothing,
        DeleteHandle,
        CreateHandle
    };

    typedef std::vector<LabelHandle*> Handles;

    ExtraActionType DetectExtraAction(const WorldInfo & wi, int & index);    

private:
    enum class State {
        creation,
        ready
    };

    State state_;

    QPointF next_point_;
    
    // TODO: create and maintain aabb
};
