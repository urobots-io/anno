#include "stdafx.h"
#include "ImagePropertiesWidget.h"
#include "ImagePropertiesTableModel.h"

ImagePropertiesWidget::ImagePropertiesWidget(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
    ui.tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

ImagePropertiesWidget::~ImagePropertiesWidget()
{
}

void ImagePropertiesWidget::setProperties(const ImagePropertiesList& props) {
    delete ui.tableView->model();
    ui.tableView->setModel(new ImagePropertiesTableModel(props, this));
}
