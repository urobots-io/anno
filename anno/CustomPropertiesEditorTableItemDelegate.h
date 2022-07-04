#pragma once
#include <QStyledItemDelegate>

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

