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

    LabelDefinition *GetDefinition(const QModelIndex &);
    LabelCategory *GetCategory(const QModelIndex &);

signals:
    void Changed();

public slots:
    void DefinitionChanged();

private:
	std::vector<std::shared_ptr<LabelDefinition>> definitions_;

	struct TreeItem {	
		TreeItem *parent = nullptr;

		/// !0 if item represents a category
		LabelCategory *category = nullptr;

		/// definition OR parent definition (if item is a category)
		/// OR nullptr if item is "select" node
		LabelDefinition *definition = nullptr;
		
		/// index inside the parent list
		int parent_index = 0;

		/// child nodes
		std::vector<TreeItem*> children;

		/// recursive cleanup
		void Clear();
	};

	/// TODO: this all is not needed in 2 level tree if ModelIndex internal pointer will code 2 indexes

	TreeItem *tree_;
};
