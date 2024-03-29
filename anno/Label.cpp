// Anno Labeling Tool
// 2020-2024 (c) urobots GmbH, https://urobots.io/en/portfolio/anno/

#include "Label.h"
#include <QTextStream>

using namespace std;

Label::Label()
{
}

Label::~Label() {
    DeleteHandles();
}

void Label::SetCategory(shared_ptr<LabelCategory> category) {
    auto old_definition = category_ ? category_->GetDefinition() : nullptr;
    auto new_definition = category ? category->GetDefinition() : nullptr;

    assert(category);
    category_ = category;

    if (old_definition != new_definition) {
        OnNewDefinition();
    }
}

shared_ptr<LabelCategory> Label::GetCategory() const {
    assert(category_);
    return category_;
}

shared_ptr<LabelDefinition> Label::GetDefinition() const {
    assert(category_);
    if (category_) {
        auto definition = category_->GetDefinition();
        assert(definition);
        return definition;
    }
    return {};
}

void Label::SetComputeVisualisationData(bool value) { 
    compute_visualisation_data_ = value; 
}

void Label::DeleteHandle(std::shared_ptr<LabelHandle> handle) {
    if (handle) {
        auto it = std::find(handles_.begin(), handles_.end(), handle);
        if (it != handles_.end()) {
            handles_.erase(it);
        }
        handle->ClearParent();
    }
}

void Label::DeleteHandles() {
    for (auto h : handles_) {
        h->ClearParent();
    }
    handles_.clear();
}

QString Label::ToString(const std::vector<std::shared_ptr<LabelHandle>> & handles) {
	QStringList values;
	for (auto h : handles) {
		values << QString("%0 %1").arg(h->GetPosition().x()).arg(h->GetPosition().y());
	}
	return values.join(' ');
}

void Label::FromString(const QString & string, std::vector<std::shared_ptr<LabelHandle>>& handles) {
	QTextStream stream(&(QString&)string);
	while (!stream.atEnd()) {
		float x, y;
		stream >> x >> y;

		auto h = make_shared<LabelHandle>(QPointF(x, y), this);
		handles.push_back(h);
	}
}

QStringList Label::ToStringsList() const {
	return QStringList() << ToString(handles_);
}

void Label::FromStringsList(QStringList const & str) {
    DeleteHandles();
	FromString(str[0], handles_);
}

void Label::SetText(const QString& text) {
	text_ = text;
}

QPen Label::GetOutlinePen(const PaintInfo & pi) const {
    auto def = GetDefinition();
    if (!def)
        return {};

	QColor color(category_->get_color());
    int line_width = def->get_line_width();
	
	auto style = Qt::SolidLine;

    if (pi.is_selected) {
        color.setAlpha(150);
    }

    if (pi.is_highlighted) {
        style = Qt::DotLine;
    }
	
	QPen pen (style);
	pen.setColor(color);
	pen.setWidth(abs(line_width));
	pen.setCosmetic(line_width < 0);
	return pen;	
}

void Label::CopyFrom(const Label & other) {    
    text_ = other.text_;
    SetCategory(other.category_);
    FromStringsList(other.ToStringsList());
}

QVariant Label::Read(const CustomPropertyDefinition & prop_def) const {
    if (custom_properties_.contains(prop_def.id)) {
        return custom_properties_[prop_def.id];
    }
    return prop_def.default_value;
}

void Label::Write(const CustomPropertyDefinition & prop_def, QVariant value) {
    custom_properties_[prop_def.id] = value;
}



