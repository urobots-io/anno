// Anno Labeling Tool
// 2020-2024 (c) urobots GmbH, https://urobots.io/en/portfolio/anno/

#include "CustomPropertyTableModel.h"
#include "migration_helpers.h"
#include <QApplication>
#include <QPalette>

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

int CustomPropertyTableModel::TimeRows() const {
    return (label_ && file_ && file_->IsVideo()) ? 2 : 0;
}

int CustomPropertyTableModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    if (label_) {
        if (auto def = label_->GetDefinition()) {
            return TimeRows() + int(def->custom_properties.size());
        }
    }
    return 0;
}

int CustomPropertyTableModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)

    // Only 2 columns: name and value
    return 2;
}

CustomPropertyDefinition CustomPropertyTableModel::GetProperty(const QModelIndex &index) const {
    int row = index.row();
    if (row < TimeRows()) {
        CustomPropertyDefinition time_property;
        time_property.id = (row == start_row) ? tr("start (s)") : tr("duration (s)");
        time_property.type = CustomPropertyType::p_double;
        return time_property;
    }
    row -= TimeRows();
    if (label_ && row >= 0) {
        if (auto def = label_->GetDefinition()) {
            auto &props_list = def->custom_properties;
            if (row < int(props_list.size())) {
                return props_list[row];
            }
        }
    }
    return {};
}

QVariant CustomPropertyTableModel::data(const QModelIndex &index, int role) const {
    if (!label_) return QVariant();

    auto def = label_->GetDefinition();
    if (!def) return QVariant();

    auto &props_list = def->custom_properties;
    int row = index.row();
    int column = index.column();

    if (row < TimeRows()) {
        if (column == 0 && role == Qt::DisplayRole) {
            return GetProperty(index).id;
        }
        if (column == 0 && role == QT_BACKGROUND_COLOR_ROLE) {
            return QApplication::palette().alternateBase();
        }
        if (column == 1) {
            double value = (row == start_row) ? label_->GetStartTime() : label_->GetDuration();
            if (role == Qt::DisplayRole) {
                return (row == duration_row && value < 0) ? tr("until end (-1)") : QString::number(value, 'f', 3);
            }
            if (role == Qt::EditRole) {
                return value;
            }
        }
        return QVariant();
    }

    if (row - TimeRows() >= int(props_list.size()))
        return QVariant();

    switch (column) {
    case 0:
        if (role == Qt::DisplayRole) {
            auto p = GetProperty(index);
            return p.id;
        }
        else if (role == QT_BACKGROUND_COLOR_ROLE) {
            return QApplication::palette().alternateBase();
        }
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

    if (label_ && file_ && column == 1 && index.row() < TimeRows()) {
        if (role == Qt::DisplayRole || role == Qt::EditRole) {
            double time = value.toDouble();
            if (index.row() == start_row) {
                label_->SetStartTime(std::max(0.0, time));
            }
            else {
                // negative duration: until the end of the video
                label_->SetDuration(time < 0 ? -1.0 : time);
            }
            file_->NotifyUpdate(false);
            return true;
        }
        return false;
    }

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

