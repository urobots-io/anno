// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
//
// Anno Labeling Tool
// 2020-2021 (c) urobots GmbH, https://urobots.io

#include "SharedPropertiesEditorTableModel.h"
#include "LabelFactory.h"

using namespace std;

SharedPropertiesEditorTableModel::SharedPropertiesEditorTableModel(const map<QString, shared_ptr<SharedPropertyDefinition>> & props, LabelType type, QObject *parent)
    : QAbstractTableModel(parent)
{
    headers_ << "Property" << "Shared" << "Id" << "a" << "b";

    auto label = LabelFactory::CreateLabel(type);
    for (auto name : label->GetPropertiesList()) {
        bool shared = props.count(name) > 0;

        SharedPropertyDefinition spd;
        if (shared) {            
            spd = *props.at(name);
        }
                
        property_names_.push_back(name);
        shared_[name] = shared;
        properties_.push_back(spd);        
    }
}

SharedPropertiesEditorTableModel::~SharedPropertiesEditorTableModel()
{
}

map<QString, SharedPropertyDefinition> SharedPropertiesEditorTableModel::GetProperties() const {
    map<QString, SharedPropertyDefinition> result;
    for (size_t i = 0; i < property_names_.size(); ++i) {
        auto name = property_names_[i];
        if (shared_.at(name)) {
            result[name] = properties_[i];
        }
    }
    return result;
}

int SharedPropertiesEditorTableModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return int(properties_.size());    
}

int SharedPropertiesEditorTableModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)
    return headers_.size();
}

QVariant SharedPropertiesEditorTableModel::data(const QModelIndex &index, int role) const {
    int row = index.row();
    if (row >= int(properties_.size()))
        return QVariant();

    int column = index.column();
    switch (column) {
    case 0:
        if (role == Qt::DisplayRole) {
            return property_names_[row];
        }
        break;

    case 1:
        if (role == Qt::DisplayRole) {
            return shared_.at(property_names_[row]);
        }       
        else if (role == Qt::CheckStateRole) {
            return shared_.at(property_names_[row]) ? Qt::Checked : Qt::Unchecked;
        }        
        break;

    case 2:
        if (role == Qt::DisplayRole || role == Qt::EditRole) {
            return properties_[row].name;
        }
        break;

    case 3:
        if (role == Qt::DisplayRole || role == Qt::EditRole) {
            return properties_[row].a;                
        }
        break;

    case 4:
        if (role == Qt::DisplayRole || role == Qt::EditRole) {
            return properties_[row].b;
        }
        break;


    default:
        break;
    }

    return QVariant();
}

bool SharedPropertiesEditorTableModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    int column = index.column();
    int row = index.row();
    if (row >= int(properties_.size()))
        return false;

    if (column == 1) {
        if (role == Qt::CheckStateRole) {
            shared_[property_names_[row]] = (value == Qt::Checked);
            return true;
        }
    }
    else if (column == 2 && (role == Qt::DisplayRole || role == Qt::EditRole)) {
        properties_[row].name = value.toString();
        if (!properties_[row].name.isEmpty() && !shared_[property_names_[row]]){
            shared_[property_names_[row]] = true;
            emit dataChanged(createIndex(row, 1), createIndex(row, 1));
        }
        return true;
    }
    else if (column == 3 && (role == Qt::DisplayRole || role == Qt::EditRole)) {
        properties_[row].a = value.toDouble();
        return true;        
    }
    else if (column == 4 && (role == Qt::DisplayRole || role == Qt::EditRole)) {
        properties_[row].b = value.toDouble();
        return true;
    }

    return false;
}

Qt::ItemFlags SharedPropertiesEditorTableModel::flags(const QModelIndex &index) const {
    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemNeverHasChildren;
    if (index.column() == 1) {
        flags |= Qt::ItemIsUserCheckable;        
    }
    else if (index.column() == 2 || index.column() == 3 || index.column() == 4) {
        flags |= Qt::ItemIsEditable;
    }
    return flags;
}

QVariant SharedPropertiesEditorTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            return headers_[section];
        }
        else if (orientation == Qt::Vertical) {
        }
    }

    return QVariant();
}

