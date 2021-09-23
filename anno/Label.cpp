// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
//
// Anno Labeling Tool
// 2020-2021 (c) urobots GmbH, https://urobots.io

#include "Label.h"
#include <QTextStream>

using namespace std;

Label::Label()
{
}

Label::~Label() {
    DeleteHandles();
}

void Label::SetCategory(LabelCategory *category) {
    auto old_definition = category_ ? category_->definition : nullptr;
    auto new_definition = category ? category->definition : nullptr;

    assert(category);
    category_ = category;

    if (old_definition != new_definition) {
        OnNewDefinition();
    }
}

void Label::SetComputeVisualisationData(bool value) { 
    compute_visualisation_data_ = value; 
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

void Label::FromString(QString const & string, std::vector<std::shared_ptr<LabelHandle>>& handles) {
	QTextStream stream(&(QString&)string);
	while (!stream.atEnd()) {
		float x, y;
		stream >> x >> y;

		auto h = make_shared<LabelHandle>(QPointF(x, y), this);
		handles.push_back(h);
	}
}

QStringList Label::ToStringsList() {
	return QStringList() << ToString(handles_);
}

void Label::FromStringsList(QStringList const & str) {
    DeleteHandles();
	FromString(str[0], handles_);
}

LabelCategory* Label::GetCategory() const {
    assert(category_);
    return category_; 
}

void Label::SetText(const QString& text) {
	text_ = text;
}

QPen Label::GetOutlinePen(const PaintInfo & pi) const {
	QColor color(category_->color);
	int line_width = category_->definition->line_width;
	
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

void Label::CopyFrom(Label * other) {    
    text_ = other->text_;
    FromStringsList(other->ToStringsList());
    SetCategory(other->category_);
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



