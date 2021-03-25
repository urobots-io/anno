#pragma once
#include "LabelDefinition.h"
#include <QAbstractItemModel>
#include <QObject>

class ApplicationModel;

class LabelDefinitionsTreeModel : public QAbstractItemModel
{
	Q_OBJECT

public:
	LabelDefinitionsTreeModel(ApplicationModel *parent, const std::vector<std::shared_ptr<LabelDefinition>> & definitions);
	~LabelDefinitionsTreeModel();

	const std::vector<std::shared_ptr<LabelDefinition>> & GetDefinitions() const { return definitions_; }
    std::shared_ptr<LabelDefinition> FindDefinition(QString type_name) const;

	QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
	Qt::ItemFlags flags(const QModelIndex &index) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
	QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
	QModelIndex parent(const QModelIndex &index) const override;
	int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	int columnCount(const QModelIndex &parent = QModelIndex()) const override;

	QModelIndex GetSelectModeIndex();

    std::shared_ptr<LabelDefinition> GetDefinition(const QModelIndex &);
    std::shared_ptr<LabelCategory> GetCategory(const QModelIndex &);

    QModelIndex CreateMarkerType(LabelType value_type);
    QModelIndex CreateCategory(const QModelIndex &);

    void Delete(std::shared_ptr<LabelDefinition>);
    void Delete(std::shared_ptr<LabelCategory>);
    QModelIndex CloneDefinition(std::shared_ptr<LabelDefinition>);

    QModelIndex GetIndex(LabelDefinition*) const;

signals:
    void Changed();
    void Error(QString);

public slots:
    void DefinitionChanged();

private:
    std::pair<std::shared_ptr<LabelDefinition>, std::shared_ptr<LabelCategory>> GetItem(const QModelIndex & index) const;        

private:
	std::vector<std::shared_ptr<LabelDefinition>> definitions_;
};
