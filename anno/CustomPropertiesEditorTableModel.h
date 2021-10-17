#pragma once
#include "CustomProperty.h"
#include "LabelDefinition.h"
#include <QAbstractTableModel>
#include <QObject>
#include <QStyledItemDelegate>

class CustomPropertiesEditorTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    CustomPropertiesEditorTableModel(const std::vector<CustomPropertyDefinition> & type, QObject *parent);
    ~CustomPropertiesEditorTableModel();

    enum PropertyType {
        Undefined = 0,
        Int,
        Double,
        String,
        Boolean,
        Selector
    };

    Q_ENUM(PropertyType)

    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    const std::vector<CustomPropertyDefinition> & GetProperties() const { return properties_; }
    const QStringList GetOriginalNames() const { return original_names_; }

public slots:
    void AddProperty();
    void DeleteProperty(QModelIndex index);

private:
    QStringList headers_;
    std::vector<CustomPropertyDefinition> properties_;
    QStringList original_names_;
};

Q_DECLARE_METATYPE(CustomPropertiesEditorTableModel::PropertyType)

class CustomPropertiesEditorTableItemDelegate : public QStyledItemDelegate {
    Q_OBJECT

public:
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;

public slots:
    void EditorValueChanged(int index);

private:
    QWidget *last_editor_ = nullptr;
};

