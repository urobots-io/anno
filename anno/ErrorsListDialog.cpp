// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
//
// Anno Labeling Tool
// 2020-2021 (c) urobots GmbH, https://urobots.io

#include "ErrorsListDialog.h"
#include <QSortFilterProxyModel>
#include <QStandardItemModel>

ErrorsListDialog::ErrorsListDialog(QString title, QString message, QStringList errors, QWidget *parent)
    : QDialog(parent)
{
    ui.setupUi(this);

    setWindowTitle(title);

    if (!message.isEmpty()) {
        ui.label->setText(message);
    }
    else {
        ui.label->hide();
    }

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
    ui.tableView->resizeRowsToContents();
    ui.tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);

}

ErrorsListDialog::~ErrorsListDialog()
{
}
