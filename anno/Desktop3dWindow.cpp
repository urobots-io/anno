// Anno Labeling Tool
// 2020-2024 (c) urobots GmbH, https://urobots.io/en/portfolio/anno/

#include "Desktop3dWindow.h"
#include "ApplicationModel.h"
#include <QStringBuilder>

Desktop3dWindow::Desktop3dWindow(ApplicationModel *model, QWidget *parent)
    : QWidget(parent)
    , model_(model)
{
    ui.setupUi(this);

    ui.actionDisplay_axis->setChecked(ui.point_cloud_display_widget->get_display_axis());
    ui.actionTrackball_Navigation->setChecked(ui.point_cloud_display_widget->get_use_arcball_navigation());

    connect(ui.actionReset_3d_View, &QAction::triggered, ui.point_cloud_display_widget, &PointCloudDisplayWidget::ResetView);
    connect(ui.actionDisplay_axis, &QAction::toggled, ui.point_cloud_display_widget, &PointCloudDisplayWidget::set_display_axis);
    connect(ui.actionTrackball_Navigation, &QAction::toggled, ui.point_cloud_display_widget, &PointCloudDisplayWidget::set_use_arcball_navigation);

    connect(ui.point_cloud_display_widget, &PointCloudDisplayWidget::VertexSelected, this, &Desktop3dWindow::OnVertexSelected);

    setAttribute(Qt::WA_DeleteOnClose);

    setWindowIcon(QIcon(":/MainWindow/Resources/3d.ico"));

    UpdateWindowTitle();
}

Desktop3dWindow::~Desktop3dWindow()
{
}

void Desktop3dWindow::OnSelectedImageFileChanged(std::shared_ptr<FileModel> file_model) {
    ui.point_cloud_display_widget->SetupImage(file_model ? file_model->get_id() : QString(), model_->get_filesystem());
    UpdateWindowTitle();
}

void Desktop3dWindow::UpdateWindowTitle() {
    setWindowTitle(tr("anno 3d View ") % ui.point_cloud_display_widget->PointCloudFilepath());
}

void Desktop3dWindow::OnVertexSelected(bool selected, QVector3D position, QVector3D color) {
    Q_UNUSED(color)
    if (selected) {
        ui.x_label->setText(QString::number(position.x()));
        ui.y_label->setText(QString::number(position.y()));
        ui.z_label->setText(QString::number(position.z()));
    }
    else {
        ui.x_label->setText("");
        ui.y_label->setText("");
        ui.z_label->setText("");
    }
}
