#pragma once
#include <QMessageBox>

namespace messagebox {

/// Displays information notification.
void Info(QString text);

/// Displays critical notification.
void Critical(QString text);

/// Asks questions with Yes/No options. Returns true if "Yes" answer was selected.
bool Question(QString text, QString caption, QMessageBox::StandardButton negative_answer = QMessageBox::No);

/// Asks Yes/No/Cancel question. Returns one of these values (close message box = Cancel)
QMessageBox::StandardButton Question3(QString text, QString caption);

}
