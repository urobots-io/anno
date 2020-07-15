#include "CustomPropertyTableItemDelegate.h"
#include "CustomPropertyTableModel.h"
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QHeaderView>
#include <QLineEdit>
#include <QSpinBox>
#include <QTableView>

CustomPropertyTableItemDelegate* CustomPropertyTableItemDelegate::SetupTableView(QTableView *table_view, std::shared_ptr<FileModel> file, std::shared_ptr<Label> label, bool readonly) {
    if (auto model = table_view->model()) {
        delete model;
    }

    auto item_delegate = new CustomPropertyTableItemDelegate(table_view);
    auto model = new CustomPropertyTableModel(file, label, table_view);
    model->set_read_only(readonly);
    table_view->setModel(model);
    table_view->setItemDelegate(item_delegate);
    table_view->verticalHeader()->setVisible(false);
    table_view->horizontalHeader()->setStretchLastSection(true);
    table_view->resizeColumnsToContents();
    table_view->setEditTriggers(QAbstractItemView::AllEditTriggers);
    return item_delegate;
}

CustomPropertyTableItemDelegate::CustomPropertyTableItemDelegate(QObject * parent)
    : QStyledItemDelegate(parent) {
}

void CustomPropertyTableItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const {
    if (editor != last_editor_) {
        QStyledItemDelegate::setEditorData(editor, index);
    }
}

void CustomPropertyTableItemDelegate::EditorValueChanged(int) {
    last_editor_ = (QWidget*)sender();
    emit commitData(last_editor_);
    last_editor_ = nullptr;
}

void CustomPropertyTableItemDelegate::EditorDoubleValueChanged(double) {
    last_editor_ = (QWidget*)sender();
    emit commitData(last_editor_);
    last_editor_ = nullptr;
}

void CustomPropertyTableItemDelegate::EditorTextValueChanged(const QString &) {
    last_editor_ = (QWidget*)sender();
    emit commitData(last_editor_);
    last_editor_ = nullptr;
}

QWidget *CustomPropertyTableItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    auto p = static_cast<const CustomPropertyTableModel*>(index.model())->GetProperty(index);
    if (p.type == CustomPropertyType::p_selector) {
        QString current_value = index.model()->data(index, Qt::EditRole).toString();
        auto combo = new QComboBox(parent);
        int current_index = 0;
        for (int i = 0; i < p.cases.size(); ++i) {
            combo->addItem(p.cases[i]);
            if (current_value == p.cases[i]) {
                current_index = i;
            }
        }
        combo->setCurrentIndex(current_index);
        connect(combo, QOverload<int>::of(&QComboBox::activated), this, &CustomPropertyTableItemDelegate::EditorValueChanged);
        
        return combo;
    }

    auto editor = QStyledItemDelegate::createEditor(parent, option, index);
    if (auto spinbox = dynamic_cast<QSpinBox*>(editor)) {
        connect(spinbox, QOverload<int>::of(&QSpinBox::valueChanged), this, &CustomPropertyTableItemDelegate::EditorValueChanged);
    } 
    else if (auto doubleSpinbox = dynamic_cast<QDoubleSpinBox*>(editor)) {
        connect(doubleSpinbox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CustomPropertyTableItemDelegate::EditorDoubleValueChanged);
        doubleSpinbox->setDecimals(6);
    }
    else if (auto combo = dynamic_cast<QComboBox*>(editor)) {
        connect(combo, QOverload<int>::of(&QComboBox::activated), this, &CustomPropertyTableItemDelegate::EditorValueChanged);
    }
    else if (auto line_edit = dynamic_cast<QLineEdit*>(editor)) {
        connect(line_edit, &QLineEdit::textChanged, this, &CustomPropertyTableItemDelegate::EditorTextValueChanged);
    }

    return editor;
}
