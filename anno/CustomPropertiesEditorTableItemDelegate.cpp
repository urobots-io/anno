// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
//
// Anno Labeling Tool
// 2020-2023 (c) urobots GmbH, https://urobots.io

#include "CustomPropertiesEditorTableItemDelegate.h"
#include "CustomPropertiesEditorTableModel.h"
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QMetaEnum>

QWidget *CustomPropertiesEditorTableItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    if (index.column() == 1) {
        auto current_value = index.model()->data(index, Qt::EditRole).toString();
        auto combo = new QComboBox(parent);

        static int enum_id = CustomPropertiesEditorTableModel::staticMetaObject.indexOfEnumerator("PropertyType");
        auto e = CustomPropertiesEditorTableModel::staticMetaObject.enumerator(enum_id);

        int current_index = 0;
        for (int i = 1; i < e.keyCount(); ++i) { // 1-to skip first (Unknown) value
            combo->addItem(e.key(i));
            if (current_value == QChar(e.value(i)))
                current_index = i;
        }
        combo->setCurrentIndex(current_index);
        connect(combo, QOverload<int>::of(&QComboBox::activated), this, &CustomPropertiesEditorTableItemDelegate::EditorValueChanged);

        return combo;
    }
    else if (index.column() == 2) {
        auto model = (CustomPropertiesEditorTableModel*)index.model();
        auto &p = model->GetProperties()[index.row()];
        if (p.type == CustomPropertyType::p_selector) {
            auto current_value = model->data(index, Qt::EditRole).toString();
            auto combo = new QComboBox(parent);
            auto cases = p.cases;

            int current_index = 0;
            for (int i = 0; i < cases.size(); ++i) {
                combo->addItem(cases.at(i));
                if (current_value == cases.at(i))
                    current_index = i;
            }
            combo->setCurrentIndex(current_index);
            connect(combo, QOverload<int>::of(&QComboBox::activated), this, &CustomPropertiesEditorTableItemDelegate::EditorValueChanged);

            return combo;
        }
    }

    auto w = QStyledItemDelegate::createEditor(parent, option, index);
    auto sp = qobject_cast<QDoubleSpinBox*>(w);
    if (sp) {
        sp->setDecimals(6);
    }
    return w;
}

void CustomPropertiesEditorTableItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const {
    if (editor != last_editor_) {
        QStyledItemDelegate::setEditorData(editor, index);
    }
}

void CustomPropertiesEditorTableItemDelegate::EditorValueChanged(int index) {
    Q_UNUSED(index);
    last_editor_ = (QWidget*)sender();
    emit commitData(last_editor_);
    last_editor_ = nullptr;
}

