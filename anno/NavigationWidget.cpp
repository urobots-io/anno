// Anno Labeling Tool
// 2020-2024 (c) urobots GmbH, https://urobots.io/en/portfolio/anno/
#include "NavigationWidget.h"

NavigationWidget::NavigationWidget(NavigationModel *model, QWidget *parent)
    : QWidget(parent) 
    , model_(model)
{
    ui.setupUi(this);

    connect(model_, &NavigationModel::current_path_changed, ui.filename_lineEdit, &QLineEdit::setText);
    connect(model_, &NavigationModel::can_back_changed, ui.back_toolButton, &QToolButton::setEnabled);
    connect(model_, &NavigationModel::can_forward_changed, ui.forward_toolButton, &QToolButton::setEnabled);

    connect(ui.back_toolButton, &QToolButton::clicked, model_, &NavigationModel::Back);
    connect(ui.forward_toolButton, &QToolButton::clicked, model_, &NavigationModel::Forward);

    ui.back_toolButton->setEnabled(false);
    ui.forward_toolButton->setEnabled(false);
}

NavigationWidget::~NavigationWidget() {
}