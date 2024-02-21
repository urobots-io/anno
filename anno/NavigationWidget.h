#pragma once
#include "NavigationModel.h"
#include "ui_NavigationWidget.h"
#include <QWidget>


class NavigationWidget : public QWidget
{
    Q_OBJECT

public:
    NavigationWidget(NavigationModel *model, QWidget *parent = nullptr);
    ~NavigationWidget();

signals:

public slots:


private:
    Ui::NavigationWidget ui;
    NavigationModel *model_;
};
