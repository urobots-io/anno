#ifndef STARTUPDIALOG_H
#define STARTUPDIALOG_H

#include <QDialog>
#include <QListWidgetItem>

namespace Ui {
class StartupDialog;
}

class StartupDialog : public QDialog
{
    Q_OBJECT

public:
    explicit StartupDialog(QWidget *parent = nullptr);
    ~StartupDialog();

    bool OpenProject() const { return open_project_; }
    bool NewProject() const { return new_project_; }
    QString SelectedProject() const { return selected_project_; }

public slots:
    void OnItemDoubleClicked(QListWidgetItem*);
    void OnSelectionChanged();
    void OnBrowse();
    void OnNewProject();
    void OnOk();

private:
    Ui::StartupDialog *ui;
    QString selected_project_;
    bool open_project_ = false;
    bool new_project_ = false;
};

#endif // STARTUPDIALOG_H
