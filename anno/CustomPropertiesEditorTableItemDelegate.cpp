// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
//
// Anno Labeling Tool
// 2020-2021 (c) urobots GmbH, https://urobots.io

#include "CustomPropertiesEditorTableItemDelegate.h"
#include "CustomPropertiesEditorTableModel.h"
#include <QComboBox>
#include <QMetaEnum>

QWidget *CustomPropertiesEditorTableItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    if (index.column() == 1) {
        auto current_value = index.model()->data(index, Qt::EditRole).toString();
        auto combo = new QComboBox(parent);


        static int enum_id = CustomPropertiesEditorTableModel::staticMetaObject.indexOfEnumerator("PropertyType");
        auto e = CustomPropertiesEditorTableModel::staticMetaObject.enumerator(enum_id);

        int current_index = 0;
        for (int i = 1; i < e.keyCount(); ++i) {
            combo->addItem(e.key(i));
            if (current_value == e.value(i))
                current_index = i;
        }
        combo->setCurrentIndex(current_index);
        connect(combo, QOverload<int>::of(&QComboBox::activated), this, &CustomPropertiesEditorTableItemDelegate::EditorValueChanged);

        return combo;
    }

    return QStyledItemDelegate::createEditor(parent, option, index);
}

void CustomPropertiesEditorTableItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const {
    if (editor != last_editor_) {
        QStyledItemDelegate::setEditorData(editor, index);
    }
}

void CustomPropertiesEditorTableItemDelegate::EditorValueChanged(int index) {
    last_editor_ = (QWidget*)sender();
    emit commitData(last_editor_);
    last_editor_ = nullptr;
}

