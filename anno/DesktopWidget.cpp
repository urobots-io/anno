#include "DesktopWidget.h"
#include "ApplicationModel.h"
#include "geometry.h"
#include "LabelFactory.h"
#include "ProxyLabel.h"
#include "ScriptPainter.h"
#include <QApplication>
#include <QJSEngine>
#include <QMatrix3x3>
#include <QMetaMethod>
#include <QMouseEvent>
#include <QPointF>
#include <QTransform>
#include <algorithm>
#include <cmath>
#include <string>

DesktopWidget::DesktopWidget(QWidget *parent)
    : QWidget(parent)
{
    setMouseTracking(true);

    setBackgroundRole(QPalette::Mid);
    setAutoFillBackground(true);
    setFocusPolicy(Qt::ClickFocus);

    connect(&image_, &ImageModel::pixmap_changed, this, static_cast<void (QWidget::*)()>(&DesktopWidget::update));    
}

DesktopWidget::~DesktopWidget() {
}

void DesktopWidget::Init(ApplicationModel *model) {
    model_ = model;
    connect(model, &ApplicationModel::image_script_changed, this, &DesktopWidget::OnImageScriptChanged);
    connect(model, &ApplicationModel::label_definitions_changed, this, &DesktopWidget::OnLabelDefinitionsChanged);
}

void DesktopWidget::OnLabelDefinitionsChanged(std::shared_ptr<LabelDefinitionsTreeModel> model) {
    set_is_creation_mode(false);
    if (model) {
        connect(model.get(), &LabelDefinitionsTreeModel::Changed, this, &DesktopWidget::OnRerenderFile);
    }
}

void DesktopWidget::SetCursorMode(CursorMode mode) {
    cursor_mode_ = mode;
}

void DesktopWidget::set_world_scale(double scale) {
    world_scale_ = scale;
    RefreshWorldScalePower();
    emit world_scale_changed(scale);
}

void DesktopWidget::set_world_scale_power(int power) {
    world_scale_power_ = power;
    set_world_scale(pow(world_scale_base_, power));
    emit world_scale_power_changed(power);
}

void DesktopWidget::set_selected_label(std::shared_ptr<Label> label) {
    if (!file_)
        return;

    auto owner = file_->GetOwner(label);
    if (owner != selected_label_) {
        selected_label_ = owner;
        emit selected_label_changed(file_, owner);
    }
}

void DesktopWidget::OnRerenderFile() {
    if (stamp_label_) {
        stamp_label_->SetSharedLabelIndex(-1);
    }
    update();
}

void DesktopWidget::OnSelectLabel(std::shared_ptr<Label> label) {
    set_selected_label(label);
    update();
}

void DesktopWidget::AbortCreation() {
    if (cursor_mode_ == CursorMode::modifying) {
        selected_label_->CancelExtraAction();
        SetCursorMode(CursorMode::select);
        update();
    }
    else if (cursor_mode_ == CursorMode::creation_in_progress) {
        DeleteSelectedLabel();
    }
}

void DesktopWidget::SetFile(std::shared_ptr<FileModel> file) {
    AbortCreation();

    hovered_handle_ = nullptr;
    hovered_label_ = nullptr;
    selected_handle_ = nullptr;
    set_selected_label({});

    if (file_) {
        file_->disconnect(this);
    }

    image_.Clear();

    file_ = file;

    if (file_) {
        connect(file_.get(), &FileModel::rendering_required, this, &DesktopWidget::OnRerenderFile);
        connect(file_.get(), &FileModel::select_label_required, this, &DesktopWidget::OnSelectLabel);
    }

    if (stamp_label_) {
        stamp_label_->SetSharedLabelIndex(-1);
    }

    if (file_) {
        set_is_loading_image(true);
        set_status(tr("Loading %0...").arg(QFileInfo(file_->get_id()).fileName()));

        current_loader_ = new ImageLoader(this);
        loaders_.push_back(current_loader_);
        connect(current_loader_, &ImageLoader::finished, this, &DesktopWidget::ImageLoaded);

        current_loader_->StartLoading(file_->get_id(), model_->get_filesystem(), model_->GetImageConverter());

        for (auto l : file_->labels_) {
            l->UpdateSharedProperties();
            l->SetComputeVisualisationData(true);
        }
    }
    else {
        current_loader_ = nullptr;
    }

    update();

}

void DesktopWidget::ImageLoaded() {
    auto loader = (ImageLoader*)sender();
    if (loader == current_loader_) {
        set_is_loading_image(false);

        image_.Load(loader->GetLoadedImage());
        if (get_fit_to_view_on_load()) {            
            FitBackgroundToView();
        }
        update();

        auto error = loader->GetErrorText();
        if (error.length()) {
            set_status(tr("Load error: %0").arg(error));
        }
        else {
            set_status({});
        }
    }

    auto i = std::find(loaders_.begin(), loaders_.end(), loader);
    loaders_.erase(i);

    delete loader;
}

void DesktopWidget::resizeEvent(QResizeEvent *event) {
    if (fit_to_view_on_resize_) {
        FitBackgroundToView();
    }

    QWidget::resizeEvent(event);
}

void DesktopWidget::FitBackgroundToView() {
    auto &p = image_.GetPixmap();
    if (p.isNull()) {
        set_world_scale(1.0);
        world_offset_ = QPointF(0, 0);
    }
    else {
        auto sx = float(width()) / p.width();
        auto sy = float(height()) / p.height();

        set_world_scale(std::min(sx, sy));
        world_offset_ = QPointF(
            (width() / world_scale_ - p.width()) / 2,
            (height() / world_scale_ - p.height()) / 2
        );
    }

    fit_to_view_on_resize_ = true;

    update();
}

void DesktopWidget::ResetWorldTransform() {
    set_world_scale(1.f);
    world_offset_ = QPointF(0, 0);
}

QTransform DesktopWidget::GetWorldTransform() const {
    QTransform scale, translate;
    scale.scale(world_scale_, world_scale_);
    translate.translate(world_offset_.x(), world_offset_.y());
    return translate * scale;
}

void DesktopWidget::set_category_for_creation(std::shared_ptr<LabelCategory> value) {
    if (category_for_creation_ == value)
        return;

    category_for_creation_ = value;

    emit category_for_creation_changed(value);

    stamp_label_.reset();

    if (category_for_creation_ && category_for_creation_->definition->is_stamp) {
        CreateStampLabel();
    }

    if (cursor_mode_ == CursorMode::creation_start) {
        update();
    }
}

void DesktopWidget::set_is_creation_mode(bool value) {
    if (value == is_creation_mode_)
        return;

    is_creation_mode_ = value;

    if (is_creation_mode_) {
        if (cursor_mode_ == CursorMode::modifying) {
            selected_label_->CancelExtraAction();
        }

        SetCursorMode(CursorMode::creation_start);
    }
    else {
        if (cursor_mode_ == CursorMode::creation_in_progress) {
            DeleteSelectedLabel();
        }

        SetCursorMode(CursorMode::select);

        set_category_for_creation(nullptr);
    }

    update();

    emit is_creation_mode_changed(value);
}

void DesktopWidget::CreateStampLabel() {
    if (!category_for_creation_ || !category_for_creation_->definition)
        return;

    auto def = category_for_creation_->definition;
    if (def->is_shared()) {
        auto missing_shared_indexes = def->GetMissingIndexes(model_->GetExistingSharedIndexes(def));
        if (missing_shared_indexes.size()) {
            // create stamp for first missing index
            if (auto label = LabelFactory::CreateLabel(def->value_type)) {
                label->SetCategory(category_for_creation_.get());
                label->InitStamp();
                label->SetSharedLabelIndex(-1);
                stamp_label_ = label;
            }
        }
    }
    else {
        if (auto label = LabelFactory::CreateLabel(def->value_type)) {
            label->SetCategory(category_for_creation_.get());
            label->InitStamp();
            stamp_label_ = label;
        }
    }
}

void DesktopWidget::paintEvent(QPaintEvent *) {
    QPainter painter(this);

    if (get_is_loading_image() || (image_.GetPixmap().isNull() && file_)) {
        painter.drawText(rect(), Qt::AlignCenter, get_status());
    }
    else {
        // render pixmap with half pixel offset
        QTransform half_pixel;
        half_pixel.translate(-0.5, -0.5);
        painter.setTransform(half_pixel * GetWorldTransform());
        painter.drawPixmap(0, 0, image_.GetPixmap());
    }

    // Setup world transform matrix
    painter.setTransform(GetWorldTransform());

    if (file_ && !get_is_loading_image()) {
        if (PropertyDatabase::Instance().GetStateIndex() != props_db_state_on_paint_) {
            props_db_state_on_paint_ = PropertyDatabase::Instance().GetStateIndex();
            model_->UpdateSharedProperties();
        }

        /** Explicitely define parent to avoid transfer of pointer ownership
        another way to prevent PO transfer is to call QQmlEngine::setObjectOwnership        
        QQmlEngine::setObjectOwnership(&test, QQmlEngine::CppOwnership);
        */
        ScriptPainter xp(this);  
        
        xp.pi.painter = &painter;
        xp.pi.world_scale = world_scale_;
        xp.original_transform = painter.transform();

        QJSValue objectValue = js_engine_.newQObject(&xp);
        
        // register the whole object to support old scripts
        js_engine_.globalObject().setProperty("p", objectValue);

        // register script functions
        auto &meta = ScriptPainter::staticMetaObject;
        for (int i = 0; i < meta.methodCount(); ++i) {
            auto m = meta.method(i);
            if (m.methodType() == m.Slot) {
                auto name = QString::fromLatin1(m.name());
                js_engine_.globalObject().setProperty(name, objectValue.property(name));
            }
        }

        auto image_size = image_.GetPixmap().size();
        js_engine_.globalObject().setProperty("cols", image_size.width());
        js_engine_.globalObject().setProperty("rows", image_size.height());

		auto image_script = model_->get_image_script();
        if (!image_script.isEmpty()) {
            js_engine_.evaluate(image_script);
        }

        for (auto label : file_->labels_) {
            xp.pi.is_selected = (selected_label_ == label);
            xp.pi.is_highlighted = !xp.pi.is_selected && (hovered_label_ == label);
            xp.RenderLabel(js_engine_, label.get());

            auto label_selected = xp.pi.is_selected;
            auto label_highlighted = xp.pi.is_highlighted;

            // show handles of selected label OR hovered handle/label
            for (auto handle : label->GetHandles()) {
                if (!handle->Enabled()) {
                    continue;
                }

                if (label_selected && cursor_mode_ == CursorMode::move_handle && selected_handle_ == handle) {
                    continue;
                }

                if (label_selected || label_highlighted || hovered_handle_ == handle) {
                    xp.pi.is_selected = label_selected || (label_highlighted && hovered_handle_ == handle);
                    handle->OnPaint(xp.pi);
                }
            }
        }

        if (cursor_mode_ == CursorMode::creation_start && stamp_label_.get() && file_.get()) {
            auto def = stamp_label_->GetCategory()->definition;
            if (def->is_shared()) {
                // check if stamp label can be placed
                if (stamp_label_->GetSharedLabelIndex() == -1) {
                    auto missing_shared_indexes = def->GetMissingIndexes(model_->GetExistingSharedIndexes(def));
                    for (auto index : missing_shared_indexes) {
                        if (def->AllowedForFile(file_.get(), index)) {
                            stamp_label_->SetSharedLabelIndex(index);
                            break;
                        }
                    }
                }
            }

            if (!def->is_shared() || stamp_label_->GetSharedLabelIndex() != -1) {
                xp.pi.is_selected = true;
                stamp_label_->CenterTo(mouse_pos_, geometry::Deg2Rad(mouse_angle_));
                xp.RenderLabel(js_engine_, stamp_label_.get());
            }
        }
    }

    // Draw cursor
    painter.setTransform(QTransform().translate(mouse_pos_pixels_.x(), mouse_pos_pixels_.y()));

    switch (cursor_mode_) {
    default:
    case CursorMode::select:
        if (hovered_handle_) {
            RenderHoveredHandle(painter);
        }
        else if (hide_cursor_) {
            RenderSelectCross(painter);
        }
        break;
    case CursorMode::creation_start:
    case CursorMode::creation_in_progress:
        RenderCreationCross(painter);
        break;
    case CursorMode::move_handle_start:
    case CursorMode::move_handle:
        RenderMovingCross(painter);
        break;
    }
}

void DesktopWidget::RenderCross(QPainter & painter, int size, QColor color) {
    auto old_mode = painter.compositionMode();

    QPoint p[] = {
        QPoint(0, -size),
        QPoint(0, size),
        QPoint(-size, 0),
        QPoint(size, 0)
    };

    painter.setCompositionMode(QPainter::RasterOp_NotDestination);
    painter.setPen(Qt::black);
    painter.drawLine(p[0], p[1]);
    painter.drawLine(p[2], p[3]);

    painter.setCompositionMode(QPainter::CompositionMode_Source);
    QPen pen(Qt::DotLine);
    pen.setColor(color);
    painter.setPen(pen);
    painter.drawLine(p[0], p[1]);
    painter.drawLine(p[2], p[3]);

    painter.setCompositionMode(old_mode);
}

void DesktopWidget::RenderHoveredHandle(QPainter & painter) {
    auto color = QColor(Qt::white);
    if (file_ && hovered_handle_) {
        if (auto label = file_->GetHandleOwner(hovered_handle_)) {
            color = label->GetCategory()->color;
        }
    }
    RenderCross(painter, 15, color);
}

void DesktopWidget::RenderMovingCross(QPainter & painter) {
    RenderCross(painter, 15, selected_label_ ? selected_label_->GetCategory()->color : QColor(Qt::white));
}

void DesktopWidget::RenderSelectCross(QPainter & painter) {
    RenderCross(painter, 15, Qt::white);
}

void DesktopWidget::RenderCreationCross(QPainter & painter) {
    if (cursor_mode_ == CursorMode::creation_in_progress) {
        RenderCross(painter, is_creation_to_be_completed_ ? 10 : 20, selected_label_->GetCategory()->color);
    }
    else {
        int size = std::max(width(), height());

        bool draw_with_angle = false;
        switch (category_for_creation_->definition->value_type) {
        default:
        case LabelType::circle:
        case LabelType::point:
        case LabelType::polygon:
        case LabelType::polyline:
        case LabelType::rect:
            break;

        case LabelType::oriented_point:
        case LabelType::oriented_rect:
            draw_with_angle = true;
            break;
        }

        if (draw_with_angle)
            painter.setTransform(QTransform().rotate(mouse_angle_), true);

        RenderCross(painter, size, category_for_creation_->color);

        if (draw_with_angle) {
            QPen pen(Qt::SolidLine);
            pen.setWidth(3);

            pen.setColor(Qt::red);
            painter.setPen(pen);
            painter.drawLine(50, 0, size, 0);

            pen.setColor(Qt::green);
            painter.setPen(pen);
            painter.drawLine(0, 50, 0, size);
        }
    }
}

void DesktopWidget::SetMousePos(QMouseEvent* event) {
    set_mouse_pos(GetWorldTransform().inverted().map(QPointF(event->pos())));
}

bool DesktopWidget::AbortCreationMode() {
    if (cursor_mode_ == CursorMode::creation_start) {
        set_is_creation_mode(false);
        return true;
    }
    else if (cursor_mode_ == CursorMode::creation_in_progress) {
        if (selected_label_->ForceCompleteCreation(GetWorldInfo())) {
            selected_label_->ConnectSharedProperties(true, true);
            SetCursorMode(CursorMode::creation_start);
        }
        else {
            DeleteSelectedLabel();
        }

        return true;
    }
    else {
        return false;
    }
}

std::shared_ptr<Label> DesktopWidget::FindLabelUnderCursor() {
    double smallest_area = 0;
    std::shared_ptr<Label> label;

    auto wi = GetWorldInfo();
    for (auto l : file_->labels_) {
        if (l->HitTest(wi)) {
            auto area = l->Area();
            if (!label || area < smallest_area) {
                label = l;
                smallest_area = area;
            }
        }
    }

    return label;
}

void DesktopWidget::StartExtraActionUnderCursor() {
    auto wi = GetWorldInfo();
    std::shared_ptr<Label> label;
    QStringList data;

    // extra case - label with hovered handle shall be checked first
    if (hovered_handle_) {
        if (auto l = file_->GetHandleOwner(hovered_handle_)) {
            if (l->StartExtraAction(wi, data)) {
                label = l;
            }
        }
    }
    
    if (!label) {
        for (auto l : file_->labels_) {
            if (l->StartExtraAction(wi, data)) {
                label = l;
                break;
            }
        }
    }

    if (label) {
        set_selected_label(label);
        hovered_handle_ = FindClosestHandle(mouse_pos_);
        SetCursorMode(CursorMode::modifying);
        file_->ModifyLabelGeometry(get_selected_label(), data);
    }
}

void DesktopWidget::mousePressEvent(QMouseEvent *event) {
    SetMousePos(event);

    if (!file_ || !image_.get_loaded())
        return;

    if (event->button() == Qt::LeftButton && QApplication::keyboardModifiers() == Qt::NoModifier) {
        switch (cursor_mode_) {
        case CursorMode::select:
            if (hovered_handle_) {
                // start moving of the handle
                selected_handle_ = hovered_handle_;
                set_selected_label(file_->GetHandleOwner(hovered_handle_));
                SetCursorMode(CursorMode::move_handle_start);
            }
            else {
                // cleanup selection
                selected_handle_ = nullptr;
                set_selected_label(FindLabelUnderCursor());
            }
            break;

        case CursorMode::modifying:
            if (selected_label_) {
                selected_label_->OnCreateClick(GetWorldInfo(), true);
                if (selected_label_->IsCreationFinished()) {
                    SetCursorMode(CursorMode::select);
                }
            }
            break;

        case CursorMode::creation_start:
            if (stamp_label_.get()) {
                auto def = stamp_label_->GetCategory()->definition;
                if (def->is_shared()) {          
                    auto index = stamp_label_->GetSharedLabelIndex();
                    auto missing_shared_indexes = def->GetMissingIndexes(model_->GetExistingSharedIndexes(def));
                    if (missing_shared_indexes.count(index) && def->AllowedForFile(file_.get(), index)) {
                        // Create first instance of a shared label with this index in the project
                        def->shared_labels[index]->CopyFrom(stamp_label_.get());
                        auto label = std::make_shared<ProxyLabel>(def->shared_labels[index]);
                        label->SetSharedLabelIndex(index);
                        label->ConnectSharedProperties(true, true);
                        set_selected_label(file_->CreateLabel(std::shared_ptr<Label>(label)));
                        missing_shared_indexes.erase(index);
                    }  

                    // Invalidate shared label index - label will be hidden,
                    // if it is not possible to create any more shared labels for this
                    // label.
                    stamp_label_->SetSharedLabelIndex(-1);
                }
                else if (def->AllowedForFile(file_.get())) {
                    auto stamp_clone = stamp_label_->Clone();
                    stamp_clone->ConnectSharedProperties(true, true);
                    set_selected_label(file_->CreateLabel(std::shared_ptr<Label>(stamp_clone)));
                }                
            }
            else if (category_for_creation_) {
                auto wi = GetWorldInfo();
                auto def = category_for_creation_->definition;
                if (!def->is_shared() && def->AllowedForFile(file_.get())) {
                    if (auto label = LabelFactory::CreateLabel(def->value_type, &wi)) {
                        label->SetCategory(category_for_creation_.get());
                        label->SetComputeVisualisationData(true);
                        is_creation_to_be_completed_ = false;
                        set_selected_label(file_->CreateLabel(label));
                        SetCursorMode(CursorMode::creation_in_progress);
                    }
                }
            }
            break;

        case CursorMode::creation_in_progress:
            if (selected_label_) {
                selected_label_->OnCreateClick(GetWorldInfo(), true);
                if (selected_label_->IsCreationFinished()) {
                    selected_label_->ConnectSharedProperties(true, true);
                    SetCursorMode(CursorMode::creation_start);
                }
            }
            break;

        default:
        case CursorMode::move_handle_start:
        case CursorMode::move_handle:
            break;
        }
    }
    else if (event->button() == Qt::RightButton) {
        if (!AbortCreationMode() && cursor_mode_ == CursorMode::select) {
            StartExtraActionUnderCursor();            
        }
    }

    update();
}

void DesktopWidget::mouseReleaseEvent(QMouseEvent *event) {
    SetMousePos(event);

    switch (cursor_mode_) {
    default:
    case CursorMode::select:        
    case CursorMode::creation_start:
        break;

    case CursorMode::creation_in_progress:
    case CursorMode::modifying:
        selected_label_->OnCreateClick(GetWorldInfo(), false);
        if (selected_label_->IsCreationFinished()) {
            if (cursor_mode_ == CursorMode::modifying) {
                SetCursorMode(CursorMode::select);
            }
            else {
                selected_label_->ConnectSharedProperties(true, true);
                SetCursorMode(CursorMode::creation_start);
            }
        }
        break;

    case CursorMode::move_handle:
    case CursorMode::move_handle_start:
        SetCursorMode(CursorMode::select);
        break;    
    }

    update();
}

void DesktopWidget::mouseMoveEvent(QMouseEvent *event) {
    SetMousePos(event);

    auto old_pos = mouse_pos_pixels_;
    mouse_pos_pixels_ = event->pos();

    if (!file_) {
        // do nothing
    }
    else if (event->buttons() == Qt::MidButton || 
        (event->buttons() == Qt::LeftButton && (QApplication::keyboardModifiers() & Qt::ControlModifier))) {
        auto delta = old_pos - mouse_pos_pixels_;
        world_offset_ -= QPointF(delta.x() / world_scale_, delta.y() / world_scale_);
        fit_to_view_on_resize_ = false;
    }
    else {
        switch (cursor_mode_) {
        case CursorMode::select:
            if (event->buttons() == Qt::NoButton) {
                // detect handle under the cursor
                hovered_handle_ = FindClosestHandle(mouse_pos_);

                if (hovered_handle_) {
                    hovered_label_ = file_->GetHandleOwner(hovered_handle_);
                }
                else {
                    hovered_label_ = FindLabelUnderCursor();
                }
            }
            break;

        case CursorMode::move_handle_start:
            file_->ModifyLabelGeometry(get_selected_label());
            SetCursorMode(CursorMode::move_handle);
            // fall through is intended

        case CursorMode::move_handle:
            selected_handle_->SetPosition(mouse_pos_);
            break;

        case CursorMode::creation_in_progress:
        case CursorMode::modifying:
            is_creation_to_be_completed_ = selected_label_->OnCreateMove(GetWorldInfo());
            break;

        case CursorMode::creation_start:
            break;
        }
    }

    update();

    bool hide_cursor = (cursor_mode_ != CursorMode::select) || hovered_handle_ || hovered_label_;
    if (!hide_cursor) {
        if (!image_.GetPixmap().isNull()) {
            auto size = image_.GetPixmap().size();
            QRectF rect(0, 0, size.width(), size.height());
            hide_cursor = rect.contains(GetWorldInfo().position);
        }        
    }

    if (hide_cursor_ != hide_cursor) {
        hide_cursor_ = hide_cursor;
        setCursor(hide_cursor ? Qt::BlankCursor : Qt::ArrowCursor);
    }
}

std::shared_ptr<LabelHandle> DesktopWidget::FindClosestHandle(QPointF position) {
    std::shared_ptr<LabelHandle> closest_handle;
    double closest_dist = 7.0 / world_scale_;

    // TODO(ap): optimize (sort handles on change, etc)
    for (auto label : file_->labels_) {
        for (auto handle : label->GetHandles()) {
            auto length = (handle->GetPosition() - position).manhattanLength();
            if (length < closest_dist) {
                if (!handle->Enabled())
                    continue;
                closest_handle = handle;
                closest_dist = length;
            }
        }
    }
    return closest_handle;
}

WorldInfo DesktopWidget::GetWorldInfo() const {
    WorldInfo wi;
    wi.position = mouse_pos_;
    wi.angle = geometry::Deg2Rad(mouse_angle_);
    wi.world_scale = world_scale_;
    return wi;
}

void DesktopWidget::wheelEvent(QWheelEvent *event) {
    int numDegrees = event->delta() / 8;
    int numSteps = numDegrees / 15;

    if (QApplication::keyboardModifiers() & Qt::ControlModifier) {
        auto old_world_scale = world_scale_;

        set_world_scale_power(world_scale_power_ + numSteps);

        float dx = (world_offset_.x() * old_world_scale + mouse_pos_.x() * (old_world_scale - world_scale_)) / world_scale_;
        float dy = (world_offset_.y() * old_world_scale + mouse_pos_.y() * (old_world_scale - world_scale_)) / world_scale_;

        world_offset_.setX(dx);
        world_offset_.setY(dy);

        fit_to_view_on_resize_ = false;
    }
    else {
        double delta;
        if (QApplication::keyboardModifiers() & Qt::ShiftModifier) {
            delta = numSteps;
            if (QApplication::keyboardModifiers() & Qt::AltModifier)
                delta /= 8;
        }
        else
            delta = numDegrees / 2;

        if (cursor_mode_ == CursorMode::creation_start) {
            mouse_angle_ += delta;

            double amin = -180., amax = 180.;
            while (mouse_angle_ < amin) mouse_angle_ += 360.;
            while (mouse_angle_ > amax) mouse_angle_ -= 360.;
        }
        else if (cursor_mode_ == CursorMode::select && selected_label_) {
            file_->ModifyLabelGeometry(selected_label_);
            selected_label_->Rotate(delta * M_PI / 180.);
        }
    }

    update();
}

void DesktopWidget::DeleteSelectedLabel() {
    DeleteLabel(selected_label_);
}

void DesktopWidget::MoveSelectedLabel(QPointF offset) {
    if (!selected_label_)
        return;
    if (is_need_to_undostack_move_label_by_arrows_) {
        file_->ModifyLabelGeometry(selected_label_);
        is_need_to_undostack_move_label_by_arrows_ = false;
    }
    selected_label_->MoveBy(offset / world_scale_);
    update();
}

void DesktopWidget::SetWorldScaleAndUpdate(double value, bool is_num_steps) {
    auto old_world_scale = world_scale_;
    auto center_pos = GetWorldTransform().inverted().map(QPointF(this->width() / 2, this->height() / 2));

    if (is_num_steps)
        set_world_scale_power((int)value);
    else
        set_world_scale(value);

    auto dx = (world_offset_.x() * old_world_scale + center_pos.x() * (old_world_scale - world_scale_)) / world_scale_;
    auto dy = (world_offset_.y() * old_world_scale + center_pos.y() * (old_world_scale - world_scale_)) / world_scale_;

    world_offset_.setX(dx);
    world_offset_.setY(dy);

    fit_to_view_on_resize_ = false;

    update();
}

// TODO(ap): remove this function, use FileModel::DeleteLabel && react on model updates
void DesktopWidget::DeleteLabel(std::shared_ptr<Label> label) {
    if (!label || !file_)
        return;

    bool is_selected = (label == selected_label_);

    if (file_->GetHandleOwner(hovered_handle_) == label) {
        hovered_handle_ = nullptr;
    }

    if (hovered_label_ == label) {
        hovered_label_ = nullptr;
    }

    file_->DeleteLabel(label);

    if (is_selected) {
        set_selected_label(nullptr);
        selected_handle_ = nullptr;
        if (cursor_mode_ == CursorMode::creation_in_progress)
            SetCursorMode(CursorMode::creation_start);
        else if (cursor_mode_ == CursorMode::modifying)
            SetCursorMode(CursorMode::select);
    }

    update();
}

void DesktopWidget::SetCategoryValueForSelectedLabel(int category_value) {
    if (!selected_label_)
        return;

    auto def = selected_label_->GetCategory()->definition;
    if (def->GetCategory(category_value)) {
        file_->ModifyLabelCategory(selected_label_, category_value);
    }

    update();
}

void DesktopWidget::keyPressEvent(QKeyEvent * event) {
    switch (event->key()) {
    case Qt::Key_Delete:
        DeleteSelectedLabel();
        break;

    case Qt::Key_Escape:
        if (AbortCreationMode())
            update();
        break;
    case Qt::Key_0:
    case Qt::Key_1:
    case Qt::Key_2:
    case Qt::Key_3:
    case Qt::Key_4:
    case Qt::Key_5:
    case Qt::Key_6:
    case Qt::Key_7:
    case Qt::Key_8:
    case Qt::Key_9:
        ChangeCurrentCategory(event->key() - Qt::Key_0);
        break;
    case Qt::Key_Right:
        MoveSelectedLabel(QPointF(num_of_move_pixels_, 0));
        break;
    case Qt::Key_Down:
        MoveSelectedLabel(QPointF(0, num_of_move_pixels_));
        break;
    case Qt::Key_Left:
        MoveSelectedLabel(QPointF(-num_of_move_pixels_, 0));
        break;
    case Qt::Key_Up:
        MoveSelectedLabel(QPointF(0, -num_of_move_pixels_));
        break;
    }
}

void DesktopWidget::keyReleaseEvent(QKeyEvent *event) {
    if (event->isAutoRepeat())
        return;
    switch (event->key()) {
    case Qt::Key_Right:
    case Qt::Key_Down:
    case Qt::Key_Left:
    case Qt::Key_Up:
        is_need_to_undostack_move_label_by_arrows_ = true;
        break;
    }
}

void DesktopWidget::focusOutEvent(QFocusEvent*) {
    is_need_to_undostack_move_label_by_arrows_ = true;
}

void DesktopWidget::ChangeCurrentCategory(int category_value) {
    if (cursor_mode_ == CursorMode::creation_start && stamp_label_.get() && category_for_creation_) {
        // Change category for creation, it will also update stamp category
        auto def = category_for_creation_->definition;
        if (auto lci = def->GetCategory(category_value)) {
            set_category_for_creation(lci);
        }
    }
    else {
        SetCategoryValueForSelectedLabel(category_value);
    }
}

void DesktopWidget::OnImageScriptChanged(QString) {
    update();
}
