// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
//
// Anno Labeling Tool
// 2020-2021 (c) urobots GmbH, https://urobots.io

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
