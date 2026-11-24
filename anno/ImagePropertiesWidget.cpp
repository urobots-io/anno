// Anno Labeling Tool
// 2020-2024 (c) urobots GmbH, https://urobots.io/en/portfolio/anno/

#include "stdafx.h"
#include "ImagePropertiesWidget.h"
#include "ImagePropertiesTableModel.h"

ImagePropertiesWidget::ImagePropertiesWidget(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);

    // compact panel shown over the image
    layout()->setContentsMargins(0, 0, 0, 0);
    ui.tableView->horizontalHeader()->hide();
    ui.tableView->verticalHeader()->hide();
    ui.tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui.tableView->setSelectionMode(QAbstractItemView::NoSelection);
    ui.tableView->setFocusPolicy(Qt::NoFocus);
    ui.tableView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui.tableView->setWordWrap(false);

    // shown instead of the table when there are no properties
    empty_label_ = new QLabel(tr("No image selected"), this);
    empty_label_->setFrameShape(QFrame::StyledPanel);
    empty_label_->setBackgroundRole(QPalette::Base);
    empty_label_->setAutoFillBackground(true);
    empty_label_->setMargin(6);
    layout()->addWidget(empty_label_);

    setFixedWidth(320);
    setProperties({});
}

ImagePropertiesWidget::~ImagePropertiesWidget()
{
}

void ImagePropertiesWidget::setProperties(const ImagePropertiesList& props) {
    delete ui.tableView->model();
    ui.tableView->setModel(new ImagePropertiesTableModel(props, this));
    ui.tableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui.tableView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);

    const bool empty = props.isEmpty();
    ui.tableView->setVisible(!empty);
    empty_label_->setVisible(empty);
    if (empty) {
        setFixedHeight(empty_label_->sizeHint().height());
        return;
    }

    // fit the height to the rows, limited to max_height
    const int max_height = 400;
    ui.tableView->resizeRowsToContents();
    int height = 2 * ui.tableView->frameWidth();
    for (int row = 0; row < props.size(); ++row) {
        height += ui.tableView->rowHeight(row);
    }
    setFixedHeight(qMin(height, max_height));
}
