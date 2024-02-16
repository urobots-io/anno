// Anno Labeling Tool
// 2020-2024 (c) urobots GmbH, https://urobots.io/en/portfolio/anno/

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
