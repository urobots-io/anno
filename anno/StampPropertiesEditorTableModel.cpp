// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
//
// Anno Labeling Tool
// 2020-2021 (c) urobots GmbH, https://urobots.io

#include "StampPropertiesEditorTableModel.h"
#include "LabelFactory.h"

using namespace std;

StampPropertiesEditorTableModel::StampPropertiesEditorTableModel(QJsonObject props, LabelType type, QObject *parent)
    : QAbstractTableModel(parent)    
{
    headers_ << "Name" << "Value";

    auto label = LabelFactory::CreateLabel(type);
    for (auto name : label->GetPropertiesList()) {
        names_.push_back(name);
        qDebug() << props[name];
        values_.push_back(props[name].toVariant().toString());        
    }
}

StampPropertiesEditorTableModel::~StampPropertiesEditorTableModel()
{
}

int StampPropertiesEditorTableModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return int(names_.size());
}

int StampPropertiesEditorTableModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)
        return headers_.size();
}

QVariant StampPropertiesEditorTableModel::data(const QModelIndex &index, int role) const {
    int row = index.row();
    if (row >= int(names_.size()))
        return QVariant();

    int column = index.column();
    switch (column) {
    case 0:
        if (role == Qt::DisplayRole) {
            return names_[row];
        }
        break;

    case 1:
        if (role == Qt::DisplayRole || role == Qt::EditRole) {
            return values_[row];
        }
        break;

    default:
        break;
    }

    return QVariant();
}

bool StampPropertiesEditorTableModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    int column = index.column();
    int row = index.row();
    if (row >= int(names_.size()))
        return false;

    if (column == 1 && (role == Qt::DisplayRole || role == Qt::EditRole)) {
        values_[row] = value.toString();
        return true;
    }
    
    return false;
}

Qt::ItemFlags StampPropertiesEditorTableModel::flags(const QModelIndex &index) const {
    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemNeverHasChildren;
    if (index.column() == 1) {
        flags |= Qt::ItemIsEditable;
    }
    return flags;
}

QVariant StampPropertiesEditorTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            return headers_[section];
        }
        else if (orientation == Qt::Vertical) {
        }
    }

    return QVariant();
}

