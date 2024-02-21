#pragma once
#include "ui_NavigationWidget.h"
#include <QWidget>


class NavigationWidget : public QWidget
{
    Q_OBJECT

public:
    NavigationWidget(QWidget *parent = nullptr);
    ~NavigationWidget();

signals:

public slots:


private:
    Ui::NavigationWidget ui;
};
