// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
//
// Anno Labeling Tool
// 2020-2021 (c) urobots GmbH, https://urobots.io

#include "PropertyTableModel.h"
#include "migration_helpers.h"
#include <QColor>
#include <QDebug>
#include <QMetaProperty>

using namespace std;

PropertyTableModel::PropertyTableModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

PropertyTableModel::PropertyTableModel(QObject *object, QObject *parent)
    : QAbstractTableModel(parent) {
    set_object(object);
}

PropertyTableModel::~PropertyTableModel()
{
}

void PropertyTableModel::set_object(QObject* object) {
    if (object == object_) return;

    emit beginResetModel();    
    
    property_names_.clear();

    if (object_) object_->disconnect(this);
    
    object_ = object;    
    
    if (object_) {
        vector<QString> property_names;
        auto meta = object_->metaObject();
        for (int i = 0; i < meta->propertyCount(); ++i) {
            auto p = meta->property(i);
            auto name = p.name();
            if (name && p.isValid()) {
                if (!get_suppress_object_properties() || strcmp(name, "objectName"))
                    property_names.push_back(QString::fromLatin1(name));
            }
        }

        property_names_ = property_names;    

        SubscribeForObjectChanges();
    }

    emit endResetModel();

    emit object_changed(object_);
}

void PropertyTableModel::set_suppress_object_properties(bool value) {
    if (suppress_object_properties_ != value) {
        suppress_object_properties_ = value;
        emit suppress_object_properties_changed(value);
        auto obj = get_object();
        set_object(nullptr);
        set_object(obj);
    }
}

int PropertyTableModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return int(property_names_.size());
}

int PropertyTableModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return 2; // Name and Value
}

QMetaProperty PropertyTableModel::GetProperty(const QModelIndex &index) const {
    int row = index.row();
    if (object_ && row >= 0 && row < int(property_names_.size())) {
        int property_index = object_->metaObject()->indexOfProperty(property_names_[row].toLatin1());
        return object_->metaObject()->property(property_index);
    }
    return QMetaProperty();
}

QVariant PropertyTableModel::data(const QModelIndex &index, int role) const {
    if (!object_) return QVariant();

    int row = index.row();
    if (row >= int(property_names_.size()))
        return QVariant();

    int column = index.column();
    switch (column) {
    case 0:
        if (role == Qt::DisplayRole) {
            auto p = GetProperty(index);
            return p.name();
        }
        else if (role == QT_BACKGROUND_COLOR_ROLE) {
            return QColor(240, 240, 240);
        }
        break;

    case 1: 
        if (role == Qt::DisplayRole || role == Qt::EditRole) {            
            auto p = GetProperty(index);
            auto value = p.read(object_);

            if (p.isEnumType() && role == Qt::DisplayRole) {
                auto key = p.enumerator().valueToKey(value.toInt());
                if (key)
                    return key;                
            }
                            
            if (value.canConvert<QString>()) {
                return value;
            }
            else {
                // TODO: Extra converters, i.e. for the rectangle                   
            }                        
        }
        else if (role == QT_BACKGROUND_COLOR_ROLE) {
            auto p = GetProperty(index);
            if (p.isConstant()) return QColor(200, 240, 200);
            else if (!p.hasNotifySignal()) return QColor(240, 210, 200);
            else if (!p.isWritable()) return QColor(240, 240, 240);
        }
        break;


    default:
        break;
    }

    return QVariant();    
}

bool PropertyTableModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    int column = index.column();
    if (column == 1 && (role == Qt::DisplayRole || role == Qt::EditRole)) {
        auto p = GetProperty(index);
        p.reset(object_);
        return p.write(object_, value);
    }

    return false;
}

Qt::ItemFlags PropertyTableModel::flags(const QModelIndex &index) const {
    if (!read_only_ && index.column() == 1) {
        auto p = GetProperty(index);
        if (p.isWritable()) {
            return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemNeverHasChildren | Qt::ItemIsEditable;
        }
    }
    
    return Qt::ItemIsEnabled;
}

QVariant PropertyTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
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

void PropertyTableModel::SubscribeForObjectChanges() {
    QMetaMethod updateSlot = this->metaObject()->method(this->metaObject()->indexOfSlot("OnObjectPropertyChanged()"));
    auto meta = object_->metaObject();
    for (auto name : property_names_) {
        int index = meta->indexOfProperty(name.toLatin1());
        if (index == -1) continue;
        auto mp = meta->property(index);
        if (mp.hasNotifySignal()) {                        
            auto c = connect(object_, mp.notifySignal(), this, updateSlot);
            if (!c) {
                qDebug() << QString("Failed to connect to %0 notification\n").arg(name);
            }
        }
    }
}

void PropertyTableModel::OnObjectPropertyChanged() {
    if (sender() != object_) return;

    auto sender_index = senderSignalIndex();    

    auto meta = object_->metaObject();
    for (int i = 0; i < int(property_names_.size()); ++i) {
        auto index = meta->indexOfProperty(property_names_[i].toLatin1());
        auto p = meta->property(index);
        if (p.notifySignalIndex() == sender_index) {
            emit dataChanged(createIndex(i, 1), createIndex(i, 1));
            qDebug() << QString("%0 changed\n").arg(p.name());
            emit object_property_changed(p.name());
            break;
        }
    }    
}
