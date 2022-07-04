// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
//
// Anno Labeling Tool
// 2020-2021 (c) urobots GmbH, https://urobots.io

#include "PropertyTableItemDelegate.h"
#include "PropertyTableModel.h"
#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QHeaderView>
#include <QLineEdit>
#include <QMetaProperty>
#include <QSpinBox>
#include <QTableView>

PropertyTableItemDelegate* PropertyTableItemDelegate::SetupTableView(QTableView *table_view, QObject *object, bool readonly) {
    auto item_delegate = new PropertyTableItemDelegate(table_view);
    auto model = new PropertyTableModel(object, table_view);
    model->set_read_only(readonly);
    table_view->setModel(model);
    table_view->setItemDelegate(item_delegate);
    table_view->verticalHeader()->setVisible(false);
    table_view->horizontalHeader()->setStretchLastSection(true);
    table_view->resizeColumnsToContents();
    table_view->setEditTriggers(QAbstractItemView::AllEditTriggers);
    return item_delegate;
}

PropertyTableItemDelegate::PropertyTableItemDelegate(QObject * parent)
    : QStyledItemDelegate(parent) {
}

void PropertyTableItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const {
    if (editor != last_editor_) {
        QStyledItemDelegate::setEditorData(editor, index);
    }
}

void PropertyTableItemDelegate::EditorValueChanged(int) {
    last_editor_ = (QWidget*)sender();
    emit commitData(last_editor_);
    last_editor_ = nullptr;
}

void PropertyTableItemDelegate::EditorDoubleValueChanged(double) {
    last_editor_ = (QWidget*)sender();
    emit commitData(last_editor_);
    last_editor_ = nullptr;
}

void PropertyTableItemDelegate::EditorTextValueChanged(const QString &) {      
    last_editor_ = (QWidget*)sender();
    emit commitData(last_editor_);
    last_editor_ = nullptr;
}

void PropertyTableItemDelegate::paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const {
    auto p = static_cast<const PropertyTableModel*>(index.model())->GetProperty(index);
    if (index.column() == 1 && p.type() == QVariant::Bool) {
        QStyleOptionButton cbOpt;
        cbOpt.rect = option.rect;
        cbOpt.state = option.state;

        if (index.data().toBool()) cbOpt.state |= QStyle::State_On;
        else cbOpt.state |= QStyle::State_Off;
        QApplication::style()->drawControl(QStyle::CE_CheckBox, &cbOpt, painter);
    }
    else {
        QStyledItemDelegate::paint(painter, option, index);
    }
}

bool PropertyTableItemDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    auto p = static_cast<const PropertyTableModel*>(index.model())->GetProperty(index);
    if (p.type() == QVariant::Bool) {
        if (event->type() == QEvent::MouseButtonRelease) {
            bool value = index.data().toBool();
            model->setData(index, !value);
            return true;
        }
    }

    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

QWidget *PropertyTableItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    auto p = static_cast<const PropertyTableModel*>(index.model())->GetProperty(index);

    if (p.type() == QVariant::Bool) {
        QCheckBox *check_box = new QCheckBox(parent);
        auto palette = check_box->palette();
        palette.setColor(QPalette::ColorRole::Button, QColor(245, 245, 185));
        check_box->setPalette(palette);
        check_box->setAutoFillBackground(true);
        check_box->setChecked(index.data().toBool());
        connect(check_box, &QCheckBox::stateChanged, this, &PropertyTableItemDelegate::EditorValueChanged);
        return check_box;
    }

    if (p.isEnumType()) {
        int current_value = index.model()->data(index, Qt::EditRole).toInt();
        auto combo = new QComboBox(parent);
        auto e = p.enumerator();
        int current_index = 0;
        for (int i = 0; i < e.keyCount(); ++i) {
            combo->addItem(e.key(i));
            if (current_value == e.value(i))
                current_index = i;
        }
        combo->setCurrentIndex(current_index);        
        connect(combo, QOverload<int>::of(&QComboBox::activated), this, &PropertyTableItemDelegate::EditorValueChanged);
        
        return combo;
    }

    auto editor = QStyledItemDelegate::createEditor(parent, option, index);
    if (auto spinbox = dynamic_cast<QSpinBox*>(editor)) {
        connect(spinbox, QOverload<int>::of(&QSpinBox::valueChanged), this, &PropertyTableItemDelegate::EditorValueChanged);
    } 
    else if (auto spinbox = dynamic_cast<QDoubleSpinBox*>(editor)) {
        connect(spinbox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &PropertyTableItemDelegate::EditorDoubleValueChanged);
        spinbox->setDecimals(6);
    }
    else if (auto combo = dynamic_cast<QComboBox*>(editor)) {
        connect(combo, QOverload<int>::of(&QComboBox::activated), this, &PropertyTableItemDelegate::EditorValueChanged);
    }
    else if (auto line_edit = dynamic_cast<QLineEdit*>(editor)) {
        connect(line_edit, &QLineEdit::textChanged, this, &PropertyTableItemDelegate::EditorTextValueChanged);
    }

    return editor;
}
 
