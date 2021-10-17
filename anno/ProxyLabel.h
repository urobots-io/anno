#pragma once
#include "Label.h"

/// Label which delegates all calls to the proxy object
class ProxyLabel : public Label
{
public:
    ProxyLabel(std::shared_ptr<Label> client) : client_(client) {
        SetCategory(client_->GetCategory());
    }

	~ProxyLabel() {}

    bool IsProxyLabel() const override { return true; }
     
    void CenterTo(QPointF position, double angle) override {
        client_->CenterTo(position, angle); 
    }

    QStringList ToStringsList() const override { 
        return client_->ToStringsList(); 
    }

    QStringList GetPropertiesList() const override {
        return client_->GetPropertiesList();
    }

    void ConnectSharedProperties(bool connect, bool inject_my_values) override {
        client_->ConnectSharedProperties(connect, inject_my_values);
    }

    void FromStringsList(QStringList const & value) override { 
        client_->FromStringsList(value); 
    }

    Label *Clone() override { 
        auto copy = new ProxyLabel(client_);
        copy->SetCategory(category_);
        copy->SetText(text_);
        return copy;
    }
	
    void OnPaint(const PaintInfo & pi, PaintExtraFunctions* pf) override {
        client_->category_ = category_; 
        client_->OnPaint(pi, pf); 
    }
	
	bool HitTest(const WorldInfo & wi) const override { 
        return client_->HitTest(wi);  
    }

    double Area() const override {
        return client_->Area();
    }

	bool HasExtraAction(const WorldInfo & wi, QString & description) override { 
        return client_->HasExtraAction(wi, description); 
    }
	
	bool StartExtraAction(const WorldInfo & wi, QStringList & data) override { 
        return client_->StartExtraAction(wi, data); 
    }
	
    void CancelExtraAction() override { 
        client_->CancelExtraAction(); 
    }

	bool OnCreateMove(const WorldInfo & wi) override { 
        return client_->OnCreateMove(wi); 
    }

    void OnCreateClick(const WorldInfo & wi, bool is_down) override { 
        client_->OnCreateClick(wi, is_down); 
    }
    
    bool ForceCompleteCreation(const WorldInfo & wi) override { 
        return client_->ForceCompleteCreation(wi); 
    }

	bool IsCreationFinished() const override { 
        return client_->IsCreationFinished(); 
    }
     
    void HandlePositionChanged(LabelHandle* h, const QPointF & offset) override { 
        client_->HandlePositionChanged(h, offset); 
    }
    
    bool Rotate(double angle) override { 
        return client_->Rotate(angle); 
    }

    bool MoveBy(const QPointF & offset) override { 
        return client_->MoveBy(offset); 
    }

    void SetComputeVisualisationData(bool value) override { 
        client_->SetComputeVisualisationData(value); 
    }

    QTransform GetTransform(bool scale, bool rotate) override { 
        return client_->GetTransform(scale, rotate); 
    }

    const std::vector<std::shared_ptr<LabelHandle>> & GetHandles() const override { 
        return client_->GetHandles(); 
    }

    std::shared_ptr<Label> GetProxyClient() const { 
        return client_; 
    }

    void UpdateSharedProperties(bool forced_update) override {
        client_->UpdateSharedProperties(forced_update); 
    }

    LabelProperty* GetProperty(QString property_name) override {
        return client_->GetProperty(property_name); 
    }

    QString GetComment() override {
        return client_->GetComment();
    }
    
protected:
    std::shared_ptr<Label> client_;
};
