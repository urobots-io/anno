#include "ErrorsListDialog.h"
#include <QSortFilterProxyModel>
#include <QStandardItemModel>

ErrorsListDialog::ErrorsListDialog(QStringList errors, QWidget *parent)
    : QDialog(parent)
{
    ui.setupUi(this);

    auto model = new QStandardItemModel(errors.size(), 1);
    model->setHorizontalHeaderLabels(QStringList()
        << tr("Error text"));

    int row = 0;
    for (auto i : errors) {
        model->setItem(row, 0, new QStandardItem(i));
        row++;        
    }

    auto proxy = new QSortFilterProxyModel(this);
    proxy->setSourceModel(model);
    ui.tableView->setModel(proxy);
    ui.tableView->resizeColumnsToContents();
    ui.tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

ErrorsListDialog::~ErrorsListDialog()
{
}
