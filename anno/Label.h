#pragma once
#include "LabelDefinition.h"
#include "LabelHandle.h"
#include "LabelType.h"
#include "WorldInfo.h"

/// base class for different label types
class Label
{
protected:
    Label();

public:	
	virtual ~Label();

    void SetCategory(LabelCategory *category);
    LabelCategory* GetCategory() const;

    /// Create geometry for stamp label
    virtual void InitStamp() {}

    /// Connect / disconnect shared properties to the database
    /// If (connect && inject_my_values), then database will be overwritten with 
    /// new values.
    virtual void ConnectSharedProperties(bool connect, bool inject_my_values) { Q_UNUSED(connect) Q_UNUSED(inject_my_values) }

	void SetText(const QString& text);
	const QString& GetText() const { return text_; }

    QVariantMap& GetCustomProperties() { return custom_properties_; }
    QVariant Read(const CustomPropertyDefinition &) const;
    void Write(const CustomPropertyDefinition &, QVariant value);
    
    virtual bool IsProxyLabel() const { return false; }

    void SetSharedLabelIndex(int value) { shared_label_index_ = value; }
    int GetSharedLabelIndex() const { return shared_label_index_; }    

    /// transform label - move its center to the <position>, rotate to the <angle>
    virtual void CenterTo(QPointF position, double angle) { Q_UNUSED(position) Q_UNUSED(angle) }

	/// serialize label data into strings
	virtual QStringList ToStringsList();

	/// deserialize label data from string
	virtual void FromStringsList(QStringList const &);

	/// make a copy, todo(ap) - use CopyFrom function instead (?)
	virtual Label *Clone() = 0;

	/// render a label
	virtual void OnPaint(const PaintInfo &) = 0;

	/// returns true if position is above the label
	virtual bool HitTest(const WorldInfo &) const { return false;  }

    /// returns area of the label, used for sorting
    virtual double Area() const { return 0; }

	/// returns true if label can do an extra action after click in this position
	/// description shall contain a name of the action
    virtual bool HasExtraAction(const WorldInfo &, QString & description) { Q_UNUSED(description) return false; }

	/// start extra action in the position
	/// returns true if action started
    virtual bool StartExtraAction(const WorldInfo &, QStringList & data) { Q_UNUSED(data) return false; }

	/// force finish of extra action; revert last changes if possible
	virtual void CancelExtraAction() { }

	/// mouse move in creation mode handler
	/// returns true if object creation will be completed with a click in this position
	virtual bool OnCreateMove(const WorldInfo &) { return false; }

	/// mouse click in creation mode handler
    virtual void OnCreateClick(const WorldInfo &, bool is_down) { Q_UNUSED(is_down) }

    /// returns true if creation can be completed right now
    virtual bool ForceCompleteCreation(const WorldInfo &) { return false; }

	/// returns true if object creation is completed
	virtual bool IsCreationFinished() const { return true; }

	/// notification called by handle 
    virtual void HandlePositionChanged(LabelHandle*, QPointF offset) { Q_UNUSED(offset) }

    /// rotate label - returns true if rotation was done
    virtual bool Rotate(double angle) { Q_UNUSED(angle) return false; }

	/// offset the label - returns true if move was done
    virtual bool MoveBy(QPointF offset) { Q_UNUSED(offset) return false; }

	/// shall be called for labels to display on gui
    virtual void SetComputeVisualisationData(bool value);

	/// get label handles
	virtual const std::vector<std::shared_ptr<LabelHandle>> & GetHandles() const { return handles_; }

    /// get transform from image CS to label CS
    /// translate transform + 
    /// if <scale> then scale to bounding box (-1, -1, 1, 1)
    /// if <rotate> then also rotate
    virtual QTransform GetTransform(bool scale, bool rotate) { Q_UNUSED(scale) Q_UNUSED(rotate) return QTransform(); }

    /// get outline pen based on current state (via PaintInfo)
	QPen GetOutlinePen(const PaintInfo &) const;

    /// copy from another label
    void CopyFrom(Label *);

    /// Update content from the database
    virtual void UpdateSharedProperties() {}

    /// Return pointer to the standard property
    virtual LabelProperty* GetProperty(QString property_name) { Q_UNUSED(property_name); return nullptr; }

    /// Generate comment which might be helpful to understand label content
    virtual QString GetComment() { return QString(); }

protected:	
    virtual void OnNewDefinition() {}

	/// save vector of handles into string
	static QString ToString(const std::vector<std::shared_ptr<LabelHandle>> & handles);

	/// load handles from string, add them into <handles>
    void FromString(QString const & string, std::vector<std::shared_ptr<LabelHandle>> & handles);

    /// delete all handles
    void DeleteHandles();

protected:
    friend class ProxyLabel;

	/// handles of the label, will be deleted in Label ~tor
	std::vector<std::shared_ptr<LabelHandle>> handles_;

	/// label category
	LabelCategory* category_ = nullptr;

    /// additional label information, editable by the user
	QString text_;

    /// index of the shared label, used only by ProxyLabels
    int shared_label_index_ = 0;

	/// true, if label shall compute data for visualisation
	bool compute_visualisation_data_ = false;

    /// custom properties
    QVariantMap custom_properties_;

    /// default dimension of the label
    static const int default_dimension_ = 100;
};

template<class T>
class CloneableLabel : public Label {
public:
    Label *Clone() override {
        auto clone = new T(nullptr);
        clone->CopyFrom(this);
        return clone;
    }
};

