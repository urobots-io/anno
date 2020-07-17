#include "LabelDefinitionsTreeModel.h"
#include "ApplicationModel.h"

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
        if (def->type_name == type_name) {
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
            if (i >= 0 && i < definitions_.size()) {
                return { definitions_[i], nullptr };
            }
        }
    }
    return {};
}

QVariant LabelDefinitionsTreeModel::data(const QModelIndex & index, int role) const {
    if (!index.isValid())
        return QVariant();

    auto item = GetItem(index);
    if (role == Qt::DisplayRole) {
        if (item.second) return item.second->name;
        else if (item.first) {
            return item.first->type_name;
        }
        else return tr("Select");
    }
    else if (role == Qt::EditRole) {
        if (item.second) return item.second->name;
        else if (item.first) return item.first->type_name;
        else return QString();
    }
    else if (role == Qt::DecorationRole) {
        if (item.second) {
            int size = 14;
            QPixmap pixmap(size, size);
            pixmap.fill(item.second->color);
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
            item.second->name = value.toString();
            DefinitionChanged();
            return true;
        }
        else if (item.first) {
            item.first->type_name = value.toString();
            DefinitionChanged();
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
    if (!internal_pointer) {
        return QModelIndex();
    }

    if (auto def = dynamic_cast<LabelDefinition*>(internal_pointer)) {
        return QModelIndex();
    }

    if (auto category = dynamic_cast<LabelCategory*>(internal_pointer)) {
        for (int i = 0; i < definitions_.size(); ++i) {
            if (definitions_[i].get() == category->definition) {
                return createIndex(i + 1, 0, category->definition);
            }
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

LabelDefinition *LabelDefinitionsTreeModel::GetDefinition(const QModelIndex & index) {
    return GetItem(index).first.get();    
}

LabelCategory *LabelDefinitionsTreeModel::GetCategory(const QModelIndex & index) {
    return GetItem(index).second.get();
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
            if (d->type_name == name) {
                name.clear();
                break;
            }
        }        
    }

    auto def = std::make_shared<LabelDefinition>();
    def->type_name = name;
    def->value_type = value_type;
    connect(def.get(), &LabelDefinition::Changed, this, &LabelDefinitionsTreeModel::DefinitionChanged);
    
    auto cat = def->categories[0] = std::make_shared<LabelCategory>();
    cat->color = Qt::red;
    cat->name = "ok";
    cat->value = 0;
    cat->definition = def.get();    

    int pos = int(definitions_.size()) + 1;
    beginInsertRows(QModelIndex(), pos, pos);    
    definitions_.push_back(def);    
    endInsertRows();

    return createIndex(pos, 0, def.get());
}

QModelIndex LabelDefinitionsTreeModel::CreateCategory(const QModelIndex & index) {
    auto def = GetDefinition(index);
    if (!def) {
        return QModelIndex();
    }

    int value = 0;
    for (auto c : def->categories) {
        value = std::max(value, c.first + 1);
    }

    auto cat = std::make_shared<LabelCategory>();
    cat->color = GetStandardColor(value);
    cat->name = QString("Category %0").arg(value);
    cat->value = value;
    cat->definition = def;    

    int pos = int(def->categories.size());
    beginInsertRows(index, pos, pos);
    def->categories[value] = cat;
    endInsertRows();

    return createIndex(pos, 0, cat.get());
}
