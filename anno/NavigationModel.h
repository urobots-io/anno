#pragma once
#include <QObject>
#include <QString>
#include "implement_q_property.h"

class NavigationModel : public QObject {
    Q_OBJECT

public:
    NavigationModel(QObject *parent) : QObject(parent) {}
    ~NavigationModel() {}

    Q_PROPERTY(QString current_path READ get_current_path WRITE set_current_path NOTIFY current_path_changed);
    Q_PROPERTY(bool can_back READ get_can_back WRITE set_can_back NOTIFY can_back_changed);
    Q_PROPERTY(bool can_forward READ get_can_forward WRITE set_can_forward NOTIFY can_forward_changed);

    void SetPath(const QString & path);

signals:
    void current_path_changed(QString);
    void can_back_changed(bool);
    void can_forward_changed(bool);

public slots:
    void Back();
    void Forward();
    void Clear();

private:
    IMPLEMENT_Q_PROPERTY_WRITE(QString, current_path);
    IMPLEMENT_Q_PROPERTY_WRITE(bool, can_back);
    IMPLEMENT_Q_PROPERTY_WRITE(bool, can_forward);

private:
    QString current_path_;
    
    bool can_back_ = false;
    bool can_forward_ = false;

    int index_ = 0;

    QStringList paths_;

public:
    IMPLEMENT_Q_PROPERTY_READ(current_path);
    IMPLEMENT_Q_PROPERTY_READ(can_back);
    IMPLEMENT_Q_PROPERTY_READ(can_forward);
};
