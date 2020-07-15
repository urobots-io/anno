#pragma once
#include "FileModel.h"
#include <QObject>
#include <QtWidgets/QStyledItemDelegate>

class QTableView;

class CustomPropertyTableItemDelegate : public QStyledItemDelegate {
    Q_OBJECT

public:
    CustomPropertyTableItemDelegate(QObject * parent = nullptr);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    void setEditorData(QWidget *editor, const QModelIndex &index) const override;

    static CustomPropertyTableItemDelegate *SetupTableView(QTableView *table_view, std::shared_ptr<FileModel> file, std::shared_ptr<Label> label, bool readonly = false);

public slots:
    void EditorValueChanged(int index);
    void EditorDoubleValueChanged(double index);
    void EditorTextValueChanged(const QString & index);

private:
    QWidget *last_editor_ = nullptr;
};
