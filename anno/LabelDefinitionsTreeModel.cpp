// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
//
// Anno Labeling Tool
// 2020-2021 (c) urobots GmbH, https://urobots.io

#include "LabelDefinitionsTreeModel.h"
#include "ApplicationModel.h"
#include "Serialization.h"

LabelDefinitionsTreeModel::LabelDefinitionsTreeModel(ApplicationModel *parent, const std::vector<std::shared_ptr<LabelDefinition>> & definitions)
    : QAbstractItemModel(parent)
    , definitions_(definitions)    
{
    for (auto def : definitions) {
        connect(def.get(), &LabelDefinition::Changed, this, &LabelDefinitionsTreeModel::DefinitionChanged);
    }
}

LabelDefinitionsTreeModel::~LabelDefinitionsTreeModel() {
    for (auto def : definitions_) {
        def->disconnect(this);
    }
}

std::shared_ptr<LabelDefinition> LabelDefinitionsTreeModel::FindDefinition(QString type_name) const {
    for (auto def : definitions_) {
        if (def->get_type_name() == type_name) {
            return def;
        }
    }
    return{};
}

void LabelDefinitionsTreeModel::DefinitionChanged() {
    emit Changed();
}

std::pair<std::shared_ptr<LabelDefinition>, std::shared_ptr<LabelCategory>> LabelDefinitionsTreeModel::GetItem(const QModelIndex & index) const {
    if (index.isValid()) {
        if (index.parent().isValid()) {
            if (auto def = static_cast<LabelDefinition*>(index.parent().internalPointer())) {
                return { nullptr, def->categories[index.row()] };
            }
        }
        else {
            auto i = index.row() - 1;
            if (i >= 0 && i < int(definitions_.size())) {
                return { definitions_[i], nullptr };
            }
        }
    }
    return {};
}

std::shared_ptr<LabelDefinition> LabelDefinitionsTreeModel::GetDefinition(const QModelIndex & index) {
    return GetItem(index).first;
}

std::shared_ptr<LabelCategory> LabelDefinitionsTreeModel::GetCategory(const QModelIndex & index) {
    return GetItem(index).second;
}

QVariant LabelDefinitionsTreeModel::data(const QModelIndex & index, int role) const {
    if (!index.isValid())
        return QVariant();

    auto item = GetItem(index);
    if (role == Qt::DisplayRole) {
        if (item.second) return item.second->get_name();
        else if (item.first) {
            return item.first->get_type_name();
        }
        else return tr("Select");
    }
    else if (role == Qt::EditRole) {
        if (item.second) return item.second->get_name();
        else if (item.first) return item.first->get_type_name();
        else return QString();
    }
    else if (role == Qt::DecorationRole) {
        if (item.second) {
            int size = 14;
            QPixmap pixmap(size, size);
            pixmap.fill(item.second->get_color());
            return pixmap;
        }
        else if (!item.first) {
            return QPixmap(":/MainWindow/Resources/select.png");
        }
    }
    return QVariant();
}

bool LabelDefinitionsTreeModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    auto item = GetItem(index);

    if (role == Qt::EditRole) {
        if (item.second) {
            item.second->set_name(value.toString());
            DefinitionChanged();
            return true;
        }
        else if (item.first) {
            auto definition = item.first;
            auto new_name = value.toString();
            if (definition->get_type_name() == new_name) {
                return true;
            }

            for (auto d : definitions_) {
                if (d.get() != definition.get() && d->get_type_name() == new_name) {
                    emit Error(tr("Cannot rename marker to %0, marker with this name already exists").arg(new_name));
                    return false;
                }
            }

            definition->set_type_name(value.toString());
            return true;
        }
    }

    return false;
}

Qt::ItemFlags LabelDefinitionsTreeModel::flags(const QModelIndex & index) const {
    if (!index.isValid())
        return Qt::NoItemFlags;

    if (!index.internalPointer())
        return QAbstractItemModel::flags(index);

    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

QVariant LabelDefinitionsTreeModel::headerData(int section, Qt::Orientation orientation, int role) const {
    Q_UNUSED(section);
    Q_UNUSED(orientation);
    Q_UNUSED(role);    
    return QVariant();
}

QModelIndex LabelDefinitionsTreeModel::index(int row, int column, const QModelIndex & parent) const {
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    if (!parent.isValid()) {
        if (row == 0) {
            return createIndex(0, column);
        }
        else {
            return createIndex(row, column, definitions_[row - 1].get());
        }
    }
    else {
        if (auto def = dynamic_cast<LabelDefinition*>((QObject*)parent.internalPointer())) {
            return createIndex(row, column, def->categories[row].get());
        }
    }
    return QModelIndex();
}

QModelIndex LabelDefinitionsTreeModel::parent(const QModelIndex & index) const {
    if (!index.isValid())
        return QModelIndex();

    auto internal_pointer = (QObject*)index.internalPointer();
    if (internal_pointer) {
        if (auto def = dynamic_cast<LabelDefinition*>(internal_pointer)) {
            return QModelIndex();
        }

        if (auto category = dynamic_cast<LabelCategory*>(internal_pointer)) {
            return GetIndex(category->GetDefinition().get());
        }
    }
    return QModelIndex();
}

int LabelDefinitionsTreeModel::rowCount(const QModelIndex & parent) const {
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid()) {
        return int(definitions_.size()) + 1;
    }
    else if (auto def = dynamic_cast<LabelDefinition*>((QObject*)parent.internalPointer())) {
        return int(def->categories.size());
    }    

    return 0;
}

int LabelDefinitionsTreeModel::columnCount(const QModelIndex & parent) const {
    Q_UNUSED(parent);
    return 1;
}

QModelIndex LabelDefinitionsTreeModel::GetSelectModeIndex() {
    return createIndex(0, 0);
}

QModelIndex LabelDefinitionsTreeModel::CreateMarkerType(LabelType value_type) {    
    QStringList cased;
    for (auto word : LabelTypeToString(value_type).split("_", QString::SkipEmptyParts)) {
        cased << word.at(0).toUpper() + word.mid(1);
    }
    QString label_type_name = cased.join(" ");
    QString name;
    for (int i = 0; name.isEmpty(); ++i) {
        name = QString("%0 %1").arg(label_type_name).arg(i);
        for (auto d : definitions_) {
            if (d->get_type_name() == name) {
                name.clear();
                break;
            }
        }        
    }

    auto def = std::make_shared<LabelDefinition>(value_type);
    def->set_type_name(name);
    connect(def.get(), &LabelDefinition::Changed, this, &LabelDefinitionsTreeModel::DefinitionChanged);
    
    LabelDefinition::CreateCategory(def, 0, "Category 0", Qt::red);
        
    int pos = int(definitions_.size()) + 1;
    beginInsertRows(QModelIndex(), pos, pos);    
    definitions_.push_back(def);    
    endInsertRows();

    emit Changed();

    return createIndex(pos, 0, def.get());
}

QModelIndex LabelDefinitionsTreeModel::CreateCategory(const QModelIndex & index) {
    auto def = GetDefinition(index);
    if (!def) {
        return QModelIndex();
    }

    int value = 0;
    for (auto c : def->categories) {
        value = std::max(value, c->get_value() + 1);
    }

    int pos = int(def->categories.size());
    beginInsertRows(index, pos, pos);
    auto cat = LabelDefinition::CreateCategory(
        def,
        value,
        QString("Category %0").arg(value),
        LabelCategory::GetStandardColor(value));
    endInsertRows();

    emit Changed();

    return createIndex(pos, 0, cat.get());
}

QModelIndex LabelDefinitionsTreeModel::GetIndex(LabelDefinition *marker) const {    
    for (size_t i = 0; i < definitions_.size(); ++i) {
        if (definitions_[i].get() == marker) {
            return createIndex(int(i + 1), 0, marker);
        }        
    }
    return QModelIndex();
}

void LabelDefinitionsTreeModel::Delete(std::shared_ptr<LabelDefinition> marker) {
    auto index = GetIndex(marker.get());
    if (index.isValid()) {
        beginRemoveRows(QModelIndex(), index.row(), index.row());
        definitions_.erase(definitions_.begin() + index.row() - 1);
        endRemoveRows();
    }
}

void LabelDefinitionsTreeModel::Delete(std::shared_ptr<LabelCategory> category) {
    auto marker = category->GetDefinition().get();
    auto parent = GetIndex(marker);
    if (!parent.isValid()) {
        return;
    }

    auto& cats = marker->categories;
    int index = -1;
    for (auto i = cats.begin(); index < 0 && i != cats.end(); ++i) {
        if (*i == category) {
            index = i - cats.begin();
            break;
        }
    }
    if (index >= 0) {
        beginRemoveRows(parent, index, index);
        cats.erase(cats.begin() + index);
        endRemoveRows();
    }
}

QModelIndex LabelDefinitionsTreeModel::CloneDefinition(std::shared_ptr<LabelDefinition> marker) {
    auto index = GetIndex(marker.get());
    if (!index.isValid()) {
        return QModelIndex();
    }

    QString name = marker->get_type_name() + tr(" copy");
    int copy_index = 1;
    bool name_found = false;
    while (!name_found) {        
        for (auto d : definitions_) {
            if (d->get_type_name() == name) {
                name.clear();
                break;
            }
        }

        name_found = !name.isEmpty();
        if (!name_found) {
            name = marker->get_type_name() + tr(" copy(%0)").arg(++copy_index);
        }
    }

    QStringList errors;    
    if (auto new_marker = DeserializeLabelDefinition(Serialize(marker), errors)) {
        new_marker->set_type_name(name);

        int pos = index.row() + 1;
        beginInsertRows(QModelIndex(), pos, pos);
        definitions_.insert(definitions_.begin() + index.row(), new_marker);
        endInsertRows();

        emit Changed();

        return createIndex(pos, 0, new_marker.get());
    }

    return QModelIndex();
}

