#pragma once
#include "FileModel.h"
#include <QAbstractTableModel>
#include <QObject>

class CustomPropertyTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:    
    CustomPropertyTableModel(std::shared_ptr<FileModel> file, std::shared_ptr<Label> label, QObject *parent);
    ~CustomPropertyTableModel();    

    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    CustomPropertyDefinition GetProperty(const QModelIndex &index) const;

    void set_read_only(bool value) { read_only_ = value; }

private:
    std::shared_ptr<Label> label_;
    std::shared_ptr<FileModel> file_;
    bool read_only_ = false;    
};
