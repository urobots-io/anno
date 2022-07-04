#pragma once
#include <QObject>
#include <QtWidgets/QStyledItemDelegate>

class QTableView;

class PropertyTableItemDelegate : public QStyledItemDelegate {
    Q_OBJECT

public:
    PropertyTableItemDelegate(QObject * parent = nullptr);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const override;
    
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    // void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;

    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;

    static PropertyTableItemDelegate *SetupTableView(QTableView *table_view, QObject *object, bool readonly = false);

public slots:
    void EditorValueChanged(int index);
    void EditorDoubleValueChanged(double index);
    void EditorTextValueChanged(const QString & index);

private:
    QWidget *last_editor_ = nullptr;
};
