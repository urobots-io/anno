#pragma once
#include "implement_q_property.h"
#include <QAbstractTableModel>
#include <QObject>

class PropertyTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    PropertyTableModel(QObject *parent = nullptr);
    PropertyTableModel(QObject *object, QObject *parent);
    ~PropertyTableModel();    

    Q_PROPERTY(QObject* object READ get_object WRITE set_object NOTIFY object_changed);
    Q_PROPERTY(bool suppress_object_properties READ get_suppress_object_properties WRITE set_suppress_object_properties NOTIFY suppress_object_properties_changed);

    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    QMetaProperty GetProperty(const QModelIndex &index) const;

    void set_read_only(bool value) { read_only_ = value; }

signals:
    void object_changed(QObject*);
    void suppress_object_properties_changed(bool);
    void object_property_changed(QString property_name);

public slots:
    void set_object(QObject*);
    void set_suppress_object_properties(bool);
    void OnObjectPropertyChanged();

private:    
    void SubscribeForObjectChanges();

private:
    QObject *object_ = nullptr;
    bool suppress_object_properties_ = true;
    bool read_only_ = false;
    std::vector<QString> property_names_;

public:
    IMPLEMENT_Q_PROPERTY_READ(object);
    IMPLEMENT_Q_PROPERTY_READ(suppress_object_properties)
};

