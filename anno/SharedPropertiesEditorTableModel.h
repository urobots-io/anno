#pragma once
#include "LabelDefinition.h"
#include <QAbstractTableModel>
#include <QObject>

class SharedPropertiesEditorTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    SharedPropertiesEditorTableModel(const std::map<QString, std::shared_ptr<SharedPropertyDefinition>> & properties, LabelType type, QObject *parent);
    ~SharedPropertiesEditorTableModel();

    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    std::map<QString, SharedPropertyDefinition> GetProperties() const;
    
private:
    std::map<QString, bool> shared_;
    std::vector<QString> property_names_;
    std::vector<SharedPropertyDefinition> properties_;
    QStringList headers_;
};
