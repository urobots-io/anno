#include "stdafx.h"
#include "ImagePropertiesWidget.h"
#include "ImagePropertiesTableModel.h"

ImagePropertiesWidget::ImagePropertiesWidget(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
}

ImagePropertiesWidget::~ImagePropertiesWidget()
{
}

void ImagePropertiesWidget::setProperties(QVariantMap map) {
    delete ui.tableView->model();
    ui.tableView->setModel(new ImagePropertiesTableModel(map, this));
}