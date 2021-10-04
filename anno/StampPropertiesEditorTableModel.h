#pragma once
#include "LabelType.h"
#include <QAbstractTableModel>
#include <QJsonObject>
#include <QObject>

class StampPropertiesEditorTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    StampPropertiesEditorTableModel(QJsonObject props, LabelType type, QObject *parent);
    ~StampPropertiesEditorTableModel();

    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    QJsonObject GetStampProperties() const;

private:
    QStringList headers_;
    std::vector<QString> names_;
    std::vector<QString> values_;
};
