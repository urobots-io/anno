// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
//
// Anno Labeling Tool
// 2020-2021 (c) urobots GmbH, https://urobots.io

#include "CustomPropertyTableModel.h"

using namespace std;

CustomPropertyTableModel::CustomPropertyTableModel(std::shared_ptr<FileModel> file, std::shared_ptr<Label> label, QObject *parent)
    : QAbstractTableModel(parent) 
    , label_(label)
    , file_(file)    
{    
}

CustomPropertyTableModel::~CustomPropertyTableModel()
{
}

int CustomPropertyTableModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)

    return label_ ? int(label_->GetCategory()->definition->custom_properties.size()) : 0;
}

int CustomPropertyTableModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)

    // Only 2 columns: name and value
    return 2;
}

CustomPropertyDefinition CustomPropertyTableModel::GetProperty(const QModelIndex &index) const {
    int row = index.row();
    if (label_ && row >= 0) {
        auto &props_list = label_->GetCategory()->definition->custom_properties;
        if (row < int(props_list.size())) {
            return props_list[row];
        }
    }
    return {};
}

QVariant CustomPropertyTableModel::data(const QModelIndex &index, int role) const {
    if (!label_) return QVariant();

    auto &props_list = label_->GetCategory()->definition->custom_properties;
    int row = index.row();
    if (row >= int(props_list.size()))
        return QVariant();

    int column = index.column();
    switch (column) {
    case 0:
        if (role == Qt::DisplayRole) {
            auto p = GetProperty(index);
            return p.id;
        }
        else if (role == Qt::BackgroundColorRole) return QColor(240, 240, 240);
        break;

    case 1: 
        if (role == Qt::DisplayRole) {
            auto p = GetProperty(index);
            if (p.type != CustomPropertyType::p_boolean) {
                // do not display bool as text
                return label_->Read(p);
            }
        }
        else if (role == Qt::CheckStateRole) {
            auto p = GetProperty(index);
            if (p.type == CustomPropertyType::p_boolean) {
                auto value = label_->Read(p);
                return value.toBool() ? Qt::Checked : Qt::Unchecked;
            }
        }
        else if (role == Qt::EditRole) {
            auto p = GetProperty(index);
            auto value = label_->Read(p);
            switch (p.type) {
            case CustomPropertyType::p_boolean:
                // do not return value
                break;
            case CustomPropertyType::p_int:
                return value.toInt();
            case CustomPropertyType::p_double:
                return value.toDouble();
            default:
                return value;
            }
        }        
        break;


    default:
        break;
    }

    return QVariant();    
}

bool CustomPropertyTableModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    int column = index.column();

    if (label_ && file_ && column == 1) {
        auto p = GetProperty(index);
        if (role == Qt::CheckStateRole) {
            if (p.type == CustomPropertyType::p_boolean) {
                label_->Write(p, value == Qt::Checked);
                file_->NotifyUpdate(false);
                return true;
            }
        }
        else if (role == Qt::DisplayRole || role == Qt::EditRole) {
            label_->Write(p, value);
            file_->NotifyUpdate(false);
            return true;
        }
    }

    return false;
}

Qt::ItemFlags CustomPropertyTableModel::flags(const QModelIndex &index) const {
    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemNeverHasChildren;
    if (index.column() == 1) {
        auto p = GetProperty(index);
        if (p.type == CustomPropertyType::p_boolean) {
            flags |= Qt::ItemIsUserCheckable;
        }
        else {
            flags |= Qt::ItemIsEditable;
        }
    }
    return flags;    
}

QVariant CustomPropertyTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
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

/*
void CustomPropertyTableModel::SubscribeForObjectChanges() {
    QMetaMethod updateSlot = this->metaObject()->method(this->metaObject()->indexOfSlot("OnObjectPropertyChanged()"));
    auto meta = object_->metaObject();
    for (auto name : property_names_) {
        int index = meta->indexOfProperty(name.c_str());
        if (index == -1) continue;
        auto mp = meta->property(index);
        if (mp.hasNotifySignal()) {
            auto c = connect(object_, mp.notifySignal(), this, updateSlot);
            if (!c) {
                OutputDebugString(QString("Failed to connect to %0 notification\n").arg(name.c_str()).toStdWString().c_str());
            }
        }
    }
}

void CustomPropertyTableModel::OnObjectPropertyChanged() {
    if (sender() != object_) return;

    auto sender_index = senderSignalIndex();    

    auto meta = object_->metaObject();
    for (int i = 0; i < property_names_.size(); ++i) {
        auto index = meta->indexOfProperty(property_names_[i].c_str());
        auto p = meta->property(index);
        if (p.notifySignalIndex() == sender_index) {
            emit dataChanged(createIndex(i, 1), createIndex(i, 1));
            OutputDebugString(QString("%0 changed\n").arg(p.name()).toStdWString().c_str());
            emit object_property_changed(p.name());
            break;
        }
    }    
}
*/
