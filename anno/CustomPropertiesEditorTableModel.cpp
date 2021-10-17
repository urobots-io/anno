// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
//
// Anno Labeling Tool
// 2020-2021 (c) urobots GmbH, https://urobots.io

#include "CustomPropertiesEditorTableModel.h"
#include "LabelFactory.h"
#include <QDebug>
#include <QMetaEnum>

using namespace std;

CustomPropertiesEditorTableModel::CustomPropertiesEditorTableModel(const vector<CustomPropertyDefinition> & props, QObject *parent)
: QAbstractTableModel(parent)
, properties_(props)
{
    headers_ << "Name" << "Type" << "Default value" << "Cases";
    for (auto &p : props) {
        original_names_ << p.id;
    }
}

CustomPropertiesEditorTableModel::~CustomPropertiesEditorTableModel()
{
}

void CustomPropertiesEditorTableModel::AddProperty() {
    emit beginResetModel();
    properties_.push_back(CustomPropertyDefinition());    
    original_names_.push_back(QString());
    emit endResetModel();
}

void CustomPropertiesEditorTableModel::DeleteProperty(QModelIndex index) {
    if (index.row() < properties_.size()) {
        emit beginResetModel();
        properties_.erase(properties_.begin() + index.row());
        original_names_.removeAt(index.row());
        emit endResetModel();
    }
}

int CustomPropertiesEditorTableModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return int(properties_.size());
}

int CustomPropertiesEditorTableModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)
        return headers_.size();
}

QVariant CustomPropertiesEditorTableModel::data(const QModelIndex &index, int role) const {
    int row = index.row();
    if (row >= int(properties_.size()))
        return QVariant();

    int column = index.column();
    switch (column) {
    case 0:
        if (role == Qt::DisplayRole || role == Qt::EditRole) {
            return properties_[row].id;
        }
        break;

    case 1:
        if (role == Qt::DisplayRole || role == Qt::EditRole) {
            auto result = QVariant::fromValue(PropertyType(properties_[row].type));
            qDebug() << result;
            return result;
        }
        break;

    case 2:
        if (role == Qt::DisplayRole || role == Qt::EditRole) {
            return properties_[row].default_value.toString();
        }
        break;

    case 3:
        if (role == Qt::DisplayRole || role == Qt::EditRole) {
            return properties_[row].cases.join(", ");
        }
        break;

    default:
        break;
    }

    return QVariant();
}

bool CustomPropertiesEditorTableModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    int column = index.column();
    int row = index.row();
    if (row >= int(properties_.size()))
        return false;

    if (column == 0 && (role == Qt::DisplayRole || role == Qt::EditRole)) {
        properties_[row].id = value.toString();
        return true;
    }
    else if (column == 1) {
        static int enum_id = staticMetaObject.indexOfEnumerator("PropertyType");
        auto e = staticMetaObject.enumerator(enum_id);
        properties_[row].type = (CustomPropertyType)e.keyToValue(value.toString().toLatin1());
        return true;
    }
    else if (column == 2 && (role == Qt::DisplayRole || role == Qt::EditRole)) {
        properties_[row].default_value = value;
        return true;
    }
    else if (column == 3 && (role == Qt::DisplayRole || role == Qt::EditRole)) {
        properties_[row].cases = value.toString().split(", ");
        return true;
    }

    return false;
}

Qt::ItemFlags CustomPropertiesEditorTableModel::flags(const QModelIndex &index) const {
    Q_UNUSED(index)
    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemNeverHasChildren | Qt::ItemIsEditable | Qt::ItemIsSelectable;
    return flags;
}

QVariant CustomPropertiesEditorTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            return headers_[section];
        }
        else if (orientation == Qt::Vertical) {
        }
    }

    return QVariant();
}

