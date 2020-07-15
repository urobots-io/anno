#pragma once
#include "ArcBall.h"
#include "FilesystemInterface.h"
#include "implement_q_property.h"
#include <QMatrix4x4>
#include <QObject>
#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLWidget>
#include <QQuaternion>
#include <QVector2D>
#include <memory>

class PointCloudDisplayWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    PointCloudDisplayWidget(QWidget *parent = nullptr);
    ~PointCloudDisplayWidget();

    Q_PROPERTY(bool display_axis READ get_display_axis WRITE set_display_axis NOTIFY display_axis_changed);
    Q_PROPERTY(bool use_arcball_navigation READ get_use_arcball_navigation WRITE set_use_arcball_navigation NOTIFY use_arcball_navigation_changed);

    void SetupImage(QString image_path, std::shared_ptr<FilesystemInterface> filesystem);

    // TODO: change into properties
    QString ImageFilepath() const { return image_filepath_; }
    QString PointCloudFilepath() const { return point_cloud_filepath_.length() ? point_cloud_filepath_ : image_filepath_; }

protected:
    // qt event handlers
    void keyPressEvent(QKeyEvent *e) override;
    void keyReleaseEvent(QKeyEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void wheelEvent(QWheelEvent *event) override;
    // qt event end

    // QOpenGLWidget overrides
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
    // QOpenGLWidget overrides end

    void InitShaders();  
    void CreateBuffers();

    void CreateAxis(QVector3D origin, QVector3D size, QVector3D selection_size);

    void SetupProgramWithMVP(bool selected_vertex = false);

    void UpdateSelection(QVector2D mouse_pos);

    QMatrix4x4 GetRotationMatrix() const;

signals:
    void display_axis_changed(bool);
    void use_arcball_navigation_changed(bool);

    /// emitted when vertex is selected or if selection is cleared
    void VertexSelected(bool selected, QVector3D position, QVector3D color);

public slots:
    IMPLEMENT_Q_PROPERTY_WRITE(bool, display_axis);
    IMPLEMENT_Q_PROPERTY_WRITE(bool, use_arcball_navigation);

    /// set initial view transform
    void ResetView();

private:    
    QOpenGLShaderProgram program_;
    QMatrix4x4 projection_;
    QVector2D mouse_position_;
    bool mouse_presssed_ = false;
    bool mouse_rotated_ = false;

    bool is_initialized_ = false;

    /// View mode
    bool use_arcball_navigation_ = true;
    /// LookAt center point
    QVector3D center_;
    /// Offset of the eye
    const float eye_offset_ = 4;
    /// z rotation of the eye vector
    float rotation_x_;
    /// y rotation of the eye vector
    float rotation_y_;
    /// rotation in the free mode
    QQuaternion arcball_rotation_;
    /// additional rotation during drag motion
    QQuaternion arcball_rotation_drag_;
    /// arcball
    ArcBall arc_ball_;

    /// Scale of the image to fit in 2x2 square
    QVector3D image_scale_;
    /// Offset to center the image
    QVector3D image_offset_;

    QOpenGLBuffer image_buffer_;
    QOpenGLBuffer axis_buffer_;
    QOpenGLBuffer rotation_axis_buffer_;
    QOpenGLBuffer selection_buffer_;

    std::shared_ptr<FilesystemInterface> filesystem_;
    QString image_filepath_;
    QString point_cloud_filepath_;

    int selected_vertex_index_ = 0;
    QVector3D selected_vertex_;

    bool display_axis_ = false;
    bool display_rotation_axis_ = true;

public:
    IMPLEMENT_Q_PROPERTY_READ(display_axis);
    IMPLEMENT_Q_PROPERTY_READ(use_arcball_navigation);
};
