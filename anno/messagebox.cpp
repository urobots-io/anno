// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
//
// Anno Labeling Tool
// 2020-2023 (c) urobots GmbH, https://urobots.io

#include <QMessageBox>

namespace messagebox {

void Info(QString text) {
	QMessageBox box(QMessageBox::Information, "Info", text, QMessageBox::Ok);
	box.exec();
}

void Critical(QString text) {
    QMessageBox box(QMessageBox::Critical, "Application Error", text, QMessageBox::Ok);
    box.exec();
}

bool Question(QString text, QString caption, QMessageBox::StandardButton negative_answer) {
    QMessageBox box(
        QMessageBox::Question,
        caption,
        text, 
        QMessageBox::Yes | negative_answer);

    return box.exec() == int(QMessageBox::Yes);
}

QMessageBox::StandardButton Question3(QString text, QString caption) {
    QMessageBox box(
        QMessageBox::Question,
        caption,
        text, 
        QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

    return (QMessageBox::StandardButton)box.exec();
}

}
