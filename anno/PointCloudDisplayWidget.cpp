// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
//
// Anno Labeling Tool
// 2020-2021 (c) urobots GmbH, https://urobots.io

#include "PointCloudDisplayWidget.h"
#include "ColoredVertexData.h"
#include "PointCloudDisplayShaders.h"
#include <QApplication>
#include <QFileInfo>
#include <QMouseEvent>
#include <QRegularExpression>
#include <math.h>
#include <vector>
#include <sstream>

using namespace std;

PointCloudDisplayWidget::PointCloudDisplayWidget(QWidget *parent) 
: QOpenGLWidget(parent)
, arc_ball_(640, 480)
{    
    setMouseTracking(true);

    connect(this, &PointCloudDisplayWidget::display_axis_changed, this, static_cast<void (QWidget::*)()>(&QWidget::update));

    ResetView();
}

PointCloudDisplayWidget::~PointCloudDisplayWidget() {
    makeCurrent();    
    
    image_buffer_.destroy();
    axis_buffer_.destroy();
    rotation_axis_buffer_.destroy();
    selection_buffer_.destroy();

    doneCurrent();
}

void PointCloudDisplayWidget::ResetView() {
    rotation_x_ = 0;
    rotation_y_ = 0;

    arcball_rotation_ = QQuaternion();
    center_ = QVector3D(0, 0, 0);

    update();
}

void PointCloudDisplayWidget::keyPressEvent(QKeyEvent*) {
}

void PointCloudDisplayWidget::keyReleaseEvent(QKeyEvent*) {
}

void PointCloudDisplayWidget::wheelEvent(QWheelEvent *event) {
    auto degrees = double(event->delta()) / 8;
    
    center_ += GetRotationMatrix() * QVector3D(0, 0, degrees * 0.05);

    update();
}

void PointCloudDisplayWidget::mousePressEvent(QMouseEvent *e) {
    mouse_position_ = QVector2D(e->localPos());
    mouse_presssed_ = true;
    arc_ball_.click(mouse_position_);
}

void PointCloudDisplayWidget::mouseMoveEvent(QMouseEvent *e) {
    mouse_rotated_ = false;

    if (mouse_presssed_) {
        QVector2D diff = QVector2D(e->localPos()) - mouse_position_;
        mouse_position_ = QVector2D(e->localPos());

        if (e->buttons() & Qt::LeftButton) {
            mouse_rotated_ = true;
            if (use_arcball_navigation_) {
                arcball_rotation_drag_ = arc_ball_.drag(mouse_position_).inverted();
            }
            else {
                const float scale = 0.5f;
                rotation_y_ += -diff.y() * scale;
                rotation_x_ += -diff.x() * scale;
            }
        }
        else if (e->buttons() & Qt::RightButton) {
            auto rotation = GetRotationMatrix();
            const float scale = 0.006f;
            QVector3D up = rotation * QVector3D(0, diff.y() * scale, 0);
            QVector3D side = rotation * QVector3D(-diff.x() * scale, 0, 0);
            center_ += side;
            center_ += up;
        }

        update();
    }
    else {
        UpdateSelection(QVector2D(e->localPos()));
    }
}

void PointCloudDisplayWidget::mouseReleaseEvent(QMouseEvent *) {
    mouse_presssed_ = false;

    if (mouse_rotated_ && use_arcball_navigation_) {
        arcball_rotation_ *= arcball_rotation_drag_;
        arcball_rotation_drag_ = QQuaternion();
    }

    mouse_rotated_ = false;
    update();
}

void PointCloudDisplayWidget::initializeGL() {
    initializeOpenGLFunctions();

    InitShaders();
    CreateBuffers();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    is_initialized_ = true;

    if (image_filepath_.length()) {
        SetupImage(image_filepath_, filesystem_);
    }
    else {
        SetupImage({}, nullptr);
    }
}

void PointCloudDisplayWidget::CreateBuffers() {
    image_buffer_.create();
    axis_buffer_.create();
    rotation_axis_buffer_.create();
    selection_buffer_.create();    
}

void PointCloudDisplayWidget::CreateAxis(const QVector3D &origin, const QVector3D &size, const QVector3D &selection_size) {
    ColoredVertexData vertices[] = {
        { origin, QVector3D(1.0f, 0.0f, 0.0f), {}},
        { origin + QVector3D(size.x(), 0, 0), QVector3D(1.0f, 0.0f, 0.0f), {} },
        
        { origin, QVector3D(0.0f, 1.0f, 0.0f), {} },
        { origin + QVector3D(0, size.y(), 0), QVector3D(0.0f, 1.0f, 0.0f), {} },

        { origin, QVector3D(0.0f, 0.0f, 1.0f), {} },
        { origin + QVector3D(0, 0, size.z()), QVector3D(0.0f, 0.0f, 1.0f), {} }
    };

    axis_buffer_.bind();
    axis_buffer_.allocate(vertices, sizeof(vertices));    

    ColoredVertexData cross_vertices[] = {
        { QVector3D(-selection_size.x(), 0.0f, 0.0f), QVector3D(1.0f, 0.0f, 0.0f), {} },
        { QVector3D(selection_size.x(), 0.0f, 0.0f), QVector3D(1.0f, 0.0f, 0.0f), {} },

        { QVector3D(0.0f, -selection_size.y(), 0.0f), QVector3D(0.0f, 1.0f, 0.0f), {} },
        { QVector3D(0.0f, selection_size.y(), 0.0f), QVector3D(0.0f, 1.0f, 0.0f), {} },

        { QVector3D(0.0f, 0.0f, -selection_size.z()), QVector3D(0.0f, 0.0f, 1.0f), {} },
        { QVector3D(0.0f, 0.0f, selection_size.z()), QVector3D(0.0f, 0.0f, 1.0f), {} }
    };

    selection_buffer_.bind();
    selection_buffer_.allocate(cross_vertices, sizeof(cross_vertices));

    ColoredVertexData rotation_axis_vertices[] = {
        { QVector3D(0, 0, 0), QVector3D(1.0f, 0.0f, 0.0f), {} },
        { QVector3D((image_scale_.x() > 0 ? 1 : -1), 0, 0), QVector3D(1.0f, 0.0f, 0.0f), {} },
        { QVector3D(0, 0, 0), QVector3D(0.0f, 1.0f, 0.0f), {} },
        { QVector3D(0, (image_scale_.y() > 0 ? 1 : -1), 0), QVector3D(0.0f, 1.0f, 0.0f), {} },
        { QVector3D(0, 0, 0), QVector3D(0.0f, 0.0f, 1.0f), {} },
        { QVector3D(0, 0, (image_scale_.z() > 0 ? 1 : -1)), QVector3D(0.0f, 0.0f, 1.0f), {} }
    };
    rotation_axis_buffer_.bind();
    rotation_axis_buffer_.allocate(rotation_axis_vertices, sizeof(rotation_axis_vertices));
}

void PointCloudDisplayWidget::InitShaders() {
    if (!program_.addShaderFromSourceCode(QOpenGLShader::Vertex, shaders::VertexShaderPoints()))
        throw std::exception();
    if (!program_.addShaderFromSourceCode(QOpenGLShader::Fragment, shaders::FragmentShaderPoints()))
        throw std::exception();
    if (!program_.link())
        throw std::exception();
    if (!program_.bind())
        throw std::exception();
}

void PointCloudDisplayWidget::resizeGL(int w, int h) {
    float aspect = float(w) / float(h ? h : 1);
    const float zNear = 0.1f, zFar = 70.0f, fov = 45.0f;
    projection_.setToIdentity();
    projection_.perspective(fov, aspect, zNear, zFar);
    arc_ball_.setBounds(w, h);
}

namespace {
void SetShaderAttributes(QOpenGLShaderProgram & program, bool rgb_color) {
    int vertexLocation = program.attributeLocation("a_position");
    program.enableAttributeArray(vertexLocation);
    program.setAttributeBuffer(vertexLocation, GL_FLOAT, 0, 3, sizeof(ColoredVertexData));

    int colorLocation = program.attributeLocation("a_color");
    program.enableAttributeArray(colorLocation);
    program.setAttributeBuffer(colorLocation, GL_FLOAT, sizeof(QVector3D) * (rgb_color ? 1 : 2), 3, sizeof(ColoredVertexData));
}
}

QMatrix4x4 PointCloudDisplayWidget::GetRotationMatrix() const {
    QMatrix4x4 matrix;
    if (use_arcball_navigation_) {
        matrix.rotate(arcball_rotation_ * arcball_rotation_drag_);
    }
    else {
        matrix.rotate(
            QQuaternion::fromAxisAndAngle(0, 0, 1, rotation_x_) *
            QQuaternion::fromAxisAndAngle(1, 0, 0, rotation_y_));
    }
    return matrix;
}

void PointCloudDisplayWidget::SetupProgramWithMVP(bool selected_vertex) {
    // Calculate model view transformation
    QMatrix4x4 rotation = GetRotationMatrix();
    QVector3D eye = rotation * QVector3D(0, 0, eye_offset_);
    QVector3D up = rotation * QVector3D(0, 1, 0);

    QMatrix4x4 matrix;
    
    matrix.lookAt(center_ + eye, center_, up);
    matrix.translate(image_offset_.x(), image_offset_.y(), image_offset_.z());
    matrix.scale(image_scale_.x(), image_scale_.y(), image_scale_.z());

    if (selected_vertex) {
        matrix.translate(selected_vertex_);
    }

    program_.bind();
    program_.setUniformValue("mvp_matrix", projection_ * matrix);
}

void PointCloudDisplayWidget::paintGL() {
#if (0)
    // selection rendering test
    UpdateSelection(QVector2D(0, 0));
    return;
#endif

    glClearColor(0.7f, 0.7f, 0.7f, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (mouse_rotated_) {
        QMatrix4x4 rotation = GetRotationMatrix();
        QVector3D eye = rotation * QVector3D(0, 0, eye_offset_);
        QVector3D up = rotation * QVector3D(0, 1, 0);
        QMatrix4x4 matrix;
        matrix.lookAt(eye, QVector3D(0, 0, 0), up);
        program_.bind();
        program_.setUniformValue("mvp_matrix", projection_ * matrix);
        rotation_axis_buffer_.bind();
        SetShaderAttributes(program_, true);
        glLineWidth(1.f);
        glDrawArrays(GL_LINES, 0, axis_buffer_.size() / sizeof(ColoredVertexData));
    }

    SetupProgramWithMVP();

    // Render axis
    if (get_display_axis()) {
        glLineWidth(2.f);
        axis_buffer_.bind();
        SetShaderAttributes(program_, true);
        glDrawArrays(GL_LINES, 0, axis_buffer_.size() / sizeof(ColoredVertexData));
    }

    // Render points
    image_buffer_.bind();
    SetShaderAttributes(program_, true);
    glDrawArrays(GL_POINTS, 0, image_buffer_.size() / sizeof(ColoredVertexData));

    // Render selected vertex
    if (selected_vertex_index_) {
        glLineWidth(1.f);
        SetupProgramWithMVP(true);
        selection_buffer_.bind();
        SetShaderAttributes(program_, true);
        glDrawArrays(GL_LINES, 0, selection_buffer_.size() / sizeof(ColoredVertexData));
    }    
}

namespace {
#ifdef ANNO_USE_OPENCV
bool TryLoadFile(QString filename, std::shared_ptr<FilesystemInterface> filesystem, int expected_format, cv::Mat & result) {
    auto file_data = filesystem->LoadFile(filename);
    if (!file_data.size())
        return false;

    cv::Mat rawData(1, int(file_data.size()), CV_8UC1, (void*)file_data.data());
    result = cv::imdecode(rawData, cv::IMREAD_UNCHANGED);
    auto read_format = result.type();
    if (read_format != expected_format) {
        result = cv::Mat();
        return false;
    }
    return true;
}
#endif

inline QVector3D IndexToColor(size_t index) {
    const float color_scale = 1.f / 255;
    return QVector3D(
        (index >> 16) * color_scale,
        ((index >> 8) & 0xff) * color_scale,
        (index & 0xff) * color_scale);
}

inline int ColorToIndex(const QVector3D& color) {
    return
        (int(color.x() * 255) << 16) +
        (int(color.y() * 255) << 8) +
        (int(color.z() * 255));
}
}

void PointCloudDisplayWidget::SetupImage(QString filename, std::shared_ptr<FilesystemInterface> filesystem) {
    makeCurrent();

    filesystem_ = filesystem;
    image_filepath_ = filename;
    point_cloud_filepath_.clear();

    if (!is_initialized_ || !filesystem)
        return;

    QFileInfo fi(filename);
    if (fi.suffix().toUpper() == "PCD") {
        LoadPCD(filename, filesystem);
        return;
    }
    
    auto file_data = filesystem->LoadFile(filename);
    
    QImage image;
    if (!image.loadFromData(file_data)) {
        // Clear buffer
        image_buffer_.bind();       
        image_buffer_.allocate(0);

        // Reset data
        image_offset_ = QVector3D(0, 0, 0);
        image_scale_ = QVector3D(1, 1, 1);
        CreateAxis(
            QVector3D(0, 0, 0),
            QVector3D(1, 1, 1),
            QVector3D(0, 0, 0));

        update();
        return;
    }

#ifdef ANNO_USE_OPENCV    
    cv::Mat depth, xyz;
    QRegularExpression re("(.*)_RGB(.*)");
    QRegularExpressionMatch match = re.match(fi.fileName());
    if (match.hasMatch()) {

        QString depth_filename = QString("%0/%1_D%2")
            .arg(fi.path())
            .arg(match.captured(1))
            .arg(match.captured(2));

        if (TryLoadFile(depth_filename, filesystem, CV_16UC1, depth)) {
            point_cloud_filepath_ = depth_filename;
        }
        else {
            QString xyz_filename = QString("%0/%1_XYZ%2")
                .arg(fi.path())
                .arg(match.captured(1))
                .arg(match.captured(2));

            if (TryLoadFile(xyz_filename, filesystem, CV_16UC3, xyz)) {
                point_cloud_filepath_ = xyz_filename;
            }            
        }
    }

    vector<ColoredVertexData> vertices;
    vertices.reserve(image.width() * image.height());

    if (depth.empty() && xyz.empty()) {
        // load only image file
        for (int y = 0, index = 0; y < image.height(); ++y) {
            for (int x = 0; x < image.width(); ++x) {
                auto c = image.pixelColor(x, y);
                ColoredVertexData cvd;
                cvd.position = QVector3D(x, y, 0);
                cvd.color = QVector3D(c.redF(), c.greenF(), c.blueF());
                cvd.color_index = IndexToColor(vertices.size() + 1);
                vertices.push_back(cvd);
            }
        }

        float scale = std::min(1.f / image.width(), 1.f / image.height());
        image_offset_ = QVector3D(-image.width() * scale, image.height() * scale, scale);
        image_scale_ = QVector3D(scale * 2, -scale * 2, scale);

        auto offset = min(10.f, 0.1f * min(image.width(), image.height()));
        CreateAxis(
            QVector3D(-offset, -offset, 0),
            QVector3D(image.width(), image.height(), (image.width() + image.height())/2),
            QVector3D(offset, offset, offset));
    }
    else if (!depth.empty()) {
        // load image with depth        
        float zmin = 65535, zmax = 0;        
        for (int y = 0, index = 0; y < image.height(); ++y) {
            for (int x = 0; x < image.width(); ++x) {
                auto c = image.pixelColor(x, y);
                float z = depth.at<unsigned short>(y, x);                
                if (z == 0) continue;                
                zmin = min(zmin, z);
                zmax = max(zmax, z);
                ColoredVertexData cvd;
                cvd.position = QVector3D(x, y, z);
                cvd.color = QVector3D(c.redF(), c.greenF(), c.blueF());
                cvd.color_index = IndexToColor(vertices.size() + 1);
                vertices.push_back(cvd);
            }
        }
        
        float scale = std::min(1.f / image.width(), 1.f / image.height());
        float z_scale = 0.00003f; // TODO(ap): make adjustable by the user
        image_offset_ = QVector3D(-image.width() * scale, image.height() * scale, zmax * z_scale);
        image_scale_ = QVector3D(scale * 2, -scale * 2, -z_scale);

        auto offset = min(10.f, 0.1f * max(image.width(), image.height()));
        CreateAxis(
            QVector3D(-offset, -offset, zmax),
            QVector3D(image.width(), image.height(), (zmax - zmin) * 10),
            QVector3D(offset, offset, offset));
    }
    else if (!xyz.empty()) {
        float zmin = std::numeric_limits<float>::max();
        float zmax = 0;
        float xmin = std::numeric_limits<float>::max();
        float xmax = 0;
        float ymin = std::numeric_limits<float>::max();
        float ymax = 0;

        for (int y = 0, index = 0; y < image.height(); ++y) {
            for (int x = 0; x < image.width(); ++x) {
                auto pos = xyz.at<cv::Vec3w>(y, x);

                if (!pos[2]) continue;                

                float fx = 0.01f * pos[0];
                float fy = 0.01f * pos[1];
                float fz = 0.01f * pos[2];

                xmin = min(xmin, fx);
                xmax = max(xmax, fx);
                ymin = min(ymin, fy);
                ymax = max(ymax, fy);
                zmin = min(zmin, fz);
                zmax = max(zmax, fz);

                auto c = image.pixelColor(x, y);

                ColoredVertexData cvd;
                cvd.position = QVector3D(fx, fy, fz);
                cvd.color = QVector3D(c.redF(), c.greenF(), c.blueF());
                cvd.color_index = IndexToColor(vertices.size() + 1);
                vertices.push_back(cvd);
            }
        }

        auto width = xmax - xmin;
        auto height = ymax - ymin;
        auto depth = zmax - zmin;

        float scale = min(1.f / width, 1.f / height);

        image_offset_ = QVector3D(-(xmin + xmax) * scale, (ymin + ymax) * scale, (zmin + zmax) * scale);
        image_scale_ = QVector3D(scale * 2, -scale * 2, -scale * 2);

        float s = 0.01f;
        float l = s * max(width, max(height, depth));
        CreateAxis(
            QVector3D(xmin, ymin, zmin),
            QVector3D(width, height, depth),
            QVector3D(l, l, l)
        );
    }
    
    image_buffer_.bind();
    image_buffer_.allocate(&vertices[0], int(sizeof(ColoredVertexData) * vertices.size()));

    update();
#endif
}

// point picking: http://www.lighthouse3d.com/tutorials/opengl-selection-tutorial/
void PointCloudDisplayWidget::UpdateSelection(QVector2D mouse_pos) {    
    if (!image_buffer_.size())
        return;

    makeCurrent();

    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    SetupProgramWithMVP();

    // render points
    image_buffer_.bind();
    SetShaderAttributes(program_, false);
    glDrawArrays(GL_POINTS, 0, image_buffer_.size() / sizeof(ColoredVertexData));

    // read pixels from the square around mouse cursor
    const int square_size = 9;
    const int square_size_2 = square_size / 2;
    float res[3 * square_size * square_size];
    int x = mouse_pos.x() - square_size_2;
    int y = height() - mouse_pos.y() - square_size_2;
    glReadPixels(x, y, square_size, square_size, GL_RGB, GL_FLOAT, &res);

    // lookup table of distances to the middle point
    static float dists[square_size * square_size];
    static bool dists_initialized = false;
    if (!dists_initialized) {
        dists_initialized = true;
        for (int y = 0; y < square_size; ++y) {
            for (int x = 0; x < square_size; ++x) {
                auto dx = x - square_size_2;
                auto dy = y - square_size_2;
                dists[x + y * square_size] = sqrtf(dx*dx + dy*dy);
            }
        }
    }
    
    // find nearest point    
    float best_distance = 0;
    int index = 0;

    float *p = res;
    float *d = dists;
    for (int i = 0; i < square_size * square_size; ++i, p += 3, ++d) {
        auto test = ColorToIndex(QVector3D(p[0], p[1], p[2]));
        if (test && (!index || *d < best_distance)) {
            index = test;
            best_distance = *d;
        }
    }
    
    QVector3D vertex;
    QVector3D color;
    auto count = int(image_buffer_.size() / sizeof(ColoredVertexData));
    if (index > 0 && index <= count) {
        ColoredVertexData data;
        image_buffer_.read((index - 1) * sizeof(data), &data, sizeof(data));
        vertex = data.position;
        color = data.color;
    }        

    if (selected_vertex_index_ != index) {
        selected_vertex_index_ = index;
        if (index) selected_vertex_ = vertex;               

        emit VertexSelected(index > 0, vertex, color);
    }

    update();
}

void PointCloudDisplayWidget::LoadPCD(QString filename, std::shared_ptr<FilesystemInterface> filesystem) {
    auto file_data = filesystem->LoadFile(filename);
    std::istringstream cin(file_data.toStdString());

    for (int i = 0; i < 11; ++i) {
        std::string buffer;
        if (!std::getline(cin, buffer)) break;
    }
    
    vector<ColoredVertexData> vertices;
    while (!cin.eof()) {
        float x, y, z, color, intencity;
        cin >> x >> y >> z >> color >> intencity;
        ColoredVertexData cvd;
        cvd.color = QVector3D(1, intencity, 0);
        cvd.color_index = IndexToColor(vertices.size() + 1);
        cvd.position = QVector3D(x, y, z);
        vertices.push_back(cvd);
    }

    float scale = 0.001f;
    image_scale_ = QVector3D(scale * 2, -scale * 2, scale);


    image_buffer_.bind();
    image_buffer_.allocate(&vertices[0], int(sizeof(ColoredVertexData) * vertices.size()));

    update();
}
