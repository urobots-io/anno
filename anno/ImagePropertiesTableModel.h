#pragma once
#include "ImageData.h"
#include <QAbstractTableModel>

class ImagePropertiesTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    ImagePropertiesTableModel(const ImagePropertiesList& properties, QObject *parent);
    ~ImagePropertiesTableModel();

    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;    
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

private:
    ImagePropertiesList properties_;
};
