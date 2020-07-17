#include "LabelDefinitionsTreeModel.h"
#include "ApplicationModel.h"

void LabelDefinitionsTreeModel::TreeItem::Clear()
{
    for (auto c : children) {
        c->Clear();
        delete c;
    }
    children.clear();
}


LabelDefinitionsTreeModel::LabelDefinitionsTreeModel(ApplicationModel *parent, const std::vector<std::shared_ptr<LabelDefinition>> & definitions)
    : QAbstractItemModel(parent)
    , definitions_(definitions)
    , tree_(new TreeItem())
{
    int index = 0;
    auto def_node = new TreeItem();
    def_node->parent = tree_;
    def_node->parent_index = index++;
    tree_->children.push_back(def_node);

    for (auto def : definitions) {
        auto def_node = new TreeItem();
        def_node->parent = tree_;
        def_node->definition = def.get();
        def_node->parent_index = index++;
        tree_->children.push_back(def_node);

        int cat_index = 0;
        for (auto & cat : def->categories) {
            auto cat_node = new TreeItem();
            cat_node->parent = def_node;
            cat_node->category = cat.second.get();
            cat_node->definition = def.get();
            cat_node->parent_index = cat_index++;
            def_node->children.push_back(cat_node);
        }

        connect(def.get(), &LabelDefinition::Changed, this, &LabelDefinitionsTreeModel::DefinitionChanged);
    }
}

LabelDefinitionsTreeModel::~LabelDefinitionsTreeModel() {
    for (auto def : definitions_) {
        def->disconnect(this);
    }

    tree_->Clear();

    delete tree_;
    tree_ = nullptr;
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

QVariant LabelDefinitionsTreeModel::data(const QModelIndex & index, int role) const {
    if (!index.isValid())
        return QVariant();

    TreeItem *item = static_cast<TreeItem*>(index.internalPointer());

    if (role == Qt::DisplayRole) {
        if (item->category) return item->category->name;
        else if (item->definition) {
            return item->definition->type_name;
        }
        else return tr("Select");
    }
    else if (role == Qt::EditRole) {
        if (item->category) return item->category->name;
        else if (item->definition) return item->definition->type_name;
        else return QString();
    }
    else if (role == Qt::DecorationRole) {
        if (item->category) {
            int size = 14;
            QPixmap pixmap(size, size);
            pixmap.fill(item->category->color);
            return pixmap;
        }
        else if (!item->definition) {
            return QPixmap(":/MainWindow/Resources/select.png");
        }
    }

    return QVariant();
}

bool LabelDefinitionsTreeModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (!index.isValid())
        return false;

    TreeItem *item = static_cast<TreeItem*>(index.internalPointer());

    if (role == Qt::EditRole) {
        if (item->category) {
            item->category->name = value.toString();
            DefinitionChanged();
            return true;
        }
        else if (item->definition) {
            item->definition->type_name = value.toString();
            DefinitionChanged();
            return true;
        }
    }

    return false;
}

Qt::ItemFlags LabelDefinitionsTreeModel::flags(const QModelIndex & index) const {
    if (!index.isValid())
        return Qt::NoItemFlags;

    TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
    if (!item->category && !item->definition)
        return QAbstractItemModel::flags(index);

    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

QVariant LabelDefinitionsTreeModel::headerData(int section, Qt::Orientation orientation, int role) const {
    Q_UNUSED(section)
        if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
            return tr("Name");

    return QVariant();
}

QModelIndex LabelDefinitionsTreeModel::index(int row, int column, const QModelIndex & parent) const {
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    TreeItem *parent_item;
    if (!parent.isValid())
        parent_item = tree_;
    else
        parent_item = static_cast<TreeItem*>(parent.internalPointer());

    TreeItem *child_item = nullptr;
    if (row < int(parent_item->children.size()))
        child_item = parent_item->children[row];

    if (child_item)
        return createIndex(row, column, child_item);
    else
        return QModelIndex();
}

QModelIndex LabelDefinitionsTreeModel::parent(const QModelIndex & index) const
{
    if (!index.isValid())
        return QModelIndex();

    TreeItem *child_item = static_cast<TreeItem*>(index.internalPointer());
    TreeItem *parent_item = child_item->parent;

    if (parent_item == tree_)
        return QModelIndex();

    return createIndex(parent_item->parent_index, 0, parent_item);
}

int LabelDefinitionsTreeModel::rowCount(const QModelIndex & parent) const {
    if (parent.column() > 0)
        return 0;

    TreeItem *parent_item;
    if (!parent.isValid())
        parent_item = tree_;
    else
        parent_item = static_cast<TreeItem*>(parent.internalPointer());

    return int(parent_item->children.size());
}

int LabelDefinitionsTreeModel::columnCount(const QModelIndex & parent) const {
    Q_UNUSED(parent)
        return 1;
}

QModelIndex LabelDefinitionsTreeModel::GetSelectModeIndex()
{
    return createIndex(0, 0, tree_->children[0]);
}

LabelDefinition *LabelDefinitionsTreeModel::GetDefinition(const QModelIndex & index) {
    if (index.isValid()) {
        TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
        if (item && item->definition && !item->category) {
            return item->definition;
        }
    }
    return nullptr;
}

LabelCategory *LabelDefinitionsTreeModel::GetCategory(const QModelIndex & index) {
    if (index.isValid()) {
        TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
        if (item) {
            return item->category;
        }
    }
    return nullptr;
}

void LabelDefinitionsTreeModel::CreateMarkerType(LabelType value_type) {
    QString name;
    for (int i = 0; name.isEmpty(); ++i) {
        name = QString("Marker %0").arg(i);
        for (auto d : definitions_) {
            if (d->type_name == name) {
                name.clear();
                break;
            }
        }        
    }

    int pos = int(definitions_.size());
    beginInsertRows(QModelIndex(), pos, pos);

    auto def = std::make_shared<LabelDefinition>();
    def->type_name = name;
    def->value_type = value_type;
    definitions_.push_back(def);
    
    auto def_node = new TreeItem();
    def_node->parent = tree_;
    def_node->definition = def.get();
    def_node->parent_index = definitions_.size();
    tree_->children.push_back(def_node);

    auto cat = def->categories[0] = std::make_shared<LabelCategory>();
    cat->color = Qt::red;
    cat->name = "ok";
    cat->value = 0;
    cat->definition = def.get();

    auto cat_node = new TreeItem();
    cat_node->parent = def_node;
    cat_node->category = cat.get();
    cat_node->definition = def.get();
    cat_node->parent_index = 0;
    def_node->children.push_back(cat_node);    

    connect(def.get(), &LabelDefinition::Changed, this, &LabelDefinitionsTreeModel::DefinitionChanged);

    endInsertRows();
}
