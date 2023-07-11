// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
//
// Anno Labeling Tool
// 2020-2021 (c) urobots GmbH, https://urobots.io

#include "stdafx.h"
#include "ImagePropertiesTableModel.h"
#include "migration_helpers.h"

ImagePropertiesTableModel::ImagePropertiesTableModel(const ImagePropertiesList& properties, QObject *parent)
    : QAbstractTableModel(parent)
    , properties_(properties)
{
}

ImagePropertiesTableModel::~ImagePropertiesTableModel()
{
}

int ImagePropertiesTableModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)

    return properties_.size();
}

int ImagePropertiesTableModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)

    // Only 2 columns: name and value
    return 2;
}

QVariant ImagePropertiesTableModel::data(const QModelIndex &index, int role) const {
    int row = index.row();
    if (row >= properties_.size())
        return QVariant();

    int column = index.column();
    switch (column) {
    case 0:
        if (role == Qt::DisplayRole) {
            return properties_[row].name;
        }
        else if (role == QT_BACKGROUND_COLOR_ROLE) {
            return QApplication::palette().alternateBase();
        }
        break;

    case 1:
        if (role == Qt::DisplayRole) {
            return properties_[row].value;
        }         
        break;


    default:
        break;
    }

    return QVariant();
}

Qt::ItemFlags ImagePropertiesTableModel::flags(const QModelIndex &index) const {
    Q_UNUSED(index);
    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemNeverHasChildren | Qt::ItemIsSelectable;
    return flags;
}

QVariant ImagePropertiesTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            if (section == 0) return tr("Name");
            else if (section == 1) return tr("Value");
        }
        else if (orientation == Qt::Vertical) {
        }
    }

    return QVariant();
}

