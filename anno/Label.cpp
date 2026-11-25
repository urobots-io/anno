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

    Q_ASSERT(category);
    category_ = category;

    if (old_definition != new_definition) {
        OnNewDefinition();
    }
}

shared_ptr<LabelCategory> Label::GetCategory() const {
    Q_ASSERT(category_);
    return category_;
}

shared_ptr<LabelDefinition> Label::GetDefinition() const {
    Q_ASSERT(category_);
    if (category_) {
        auto definition = category_->GetDefinition();
        Q_ASSERT(definition);
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

bool Label::IsActiveAt(double time) const {
    // tolerance for rounding of frame times
    const double eps = 1e-6;
    if (time < start_time_ - eps) {
        return false;
    }
    return duration_ < 0 || time < start_time_ + duration_ - eps;
}

bool Label::CanBeDynamic() const {
    if (IsProxyLabel()) {
        return false;
    }
    auto def = GetDefinition();
    return def && !def->is_shared() && def->shared_properties.empty();
}

namespace {
// keyframe at <time> (with tolerance for rounding of frame times)
std::map<double, QStringList>::iterator FindKeyframe(std::map<double, QStringList> &keyframes, double time) {
    const double eps = 1e-6;
    auto it = keyframes.lower_bound(time - eps);
    return (it != keyframes.end() && it->first <= time + eps) ? it : keyframes.end();
}

// geometry of <time>: keyframe at this time or the last one before, the first one for earlier times
const QStringList *KeyframeGeometry(const std::map<double, QStringList> &keyframes, double time) {
    if (keyframes.empty()) {
        return nullptr;
    }
    auto it = keyframes.upper_bound(time + 1e-6);
    if (it == keyframes.begin()) {
        return &it->second;
    }
    return &std::prev(it)->second;
}

void SetKeyframe(std::map<double, QStringList> &keyframes, double time, const QStringList &geometry) {
    auto it = FindKeyframe(keyframes, time);
    if (it != keyframes.end()) {
        it->second = geometry;
    }
    else {
        keyframes[time] = geometry;
    }
}
}

void Label::SetDynamic(bool value) {
    if (value == dynamic_) {
        return;
    }
    dynamic_ = value;
    keyframes_.clear();
    keyframe_applied_ = false;
    if (dynamic_) {
        applied_geometry_ = ToStringsList();
        keyframes_[start_time_] = applied_geometry_;
        keyframe_applied_ = true;
    }
}

void Label::ApplyKeyframe(double time) {
    if (!dynamic_) {
        return;
    }
    if (auto geometry = KeyframeGeometry(keyframes_, time)) {
        if (*geometry != ToStringsList()) {
            FromStringsList(*geometry);
        }
        applied_geometry_ = *geometry;
        keyframe_applied_ = true;
    }
}

void Label::CommitKeyframe(double time) {
    if (!dynamic_ || !keyframe_applied_) {
        return;
    }
    auto geometry = ToStringsList();
    if (geometry != applied_geometry_) {
        SetKeyframe(keyframes_, time, geometry);
        applied_geometry_ = geometry;
    }
}

Label::GeometryState Label::CaptureState(double time, const QStringList &geometry) const {
    GeometryState state;
    state.geometry = geometry.isEmpty() ? ToStringsList() : geometry;
    state.dynamic = dynamic_;
    state.keyframes = keyframes_;
    if (dynamic_ && keyframe_applied_ && state.geometry != applied_geometry_) {
        // not yet committed change of the current frame
        SetKeyframe(state.keyframes, time, state.geometry);
    }
    return state;
}

void Label::RestoreState(const GeometryState &state, double time) {
    dynamic_ = state.dynamic;
    keyframes_ = state.keyframes;
    keyframe_applied_ = false;
    if (dynamic_ && !keyframes_.empty()) {
        ApplyKeyframe(time);
    }
    else if (state.geometry != ToStringsList()) {
        FromStringsList(state.geometry);
    }
}

void Label::CopyFrom(const Label & other) {
    text_ = other.text_;
    start_time_ = other.start_time_;
    duration_ = other.duration_;
    dynamic_ = other.dynamic_;
    keyframes_ = other.keyframes_;
    keyframe_applied_ = false;
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



