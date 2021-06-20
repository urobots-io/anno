#include "StartupDialog.h"
#include "ui_StartupDialog.h"
#include <QDir>
#include <QScrollBar>
#include <QSettings>
#include <QUrl>

StartupDialog::StartupDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::StartupDialog)
{
    ui->setupUi(this);
    ui->listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // Load list of recent projects
    QSettings settings;
    auto values = settings.value("recentProjectsList").toStringList();
    for (auto project: values) {
        auto icon = QUrl(project).scheme().startsWith("http")
            ? QIcon(":/MainWindow/Resources/folder_network.ico")
            : QIcon(":/MainWindow/Resources/anno.ico");

        
        auto parts = QDir::toNativeSeparators(project).split(QDir::separator(),
                QString::SkipEmptyParts);

        auto project_name = parts.last();
        auto size = parts.size();
        if (size >= 3 && parts[size - 2] == "dataset" && parts[size - 1] == "annotations.anno") {
            project_name = parts[size - 3];
        }
        else if (project_name.endsWith(".anno")) {
            project_name = project_name.left(project_name.length() - 5);
        }

        auto item = new QListWidgetItem(icon, project_name + " - " + project, ui->listWidget);
        item->setData(Qt::UserRole, project);
    }

    ui->ok_pushButton->setEnabled(false);

    connect(ui->listWidget, &QListWidget::itemDoubleClicked, this, &StartupDialog::OnItemDoubleClicked);
    connect(ui->listWidget, &QListWidget::itemSelectionChanged, this, &StartupDialog::OnSelectionChanged);
    connect(ui->browse_pushButton, &QPushButton::clicked, this, &StartupDialog::OnBrowse);
    connect(ui->new_pushButton, &QPushButton::clicked, this, &StartupDialog::OnNewProject);
    connect(ui->ok_pushButton, &QPushButton::clicked, this, &StartupDialog::OnOk);
}

StartupDialog::~StartupDialog()
{
    delete ui;
}

void StartupDialog::OnItemDoubleClicked(QListWidgetItem* item) {
    if (item) {
        selected_project_ = item->data(Qt::UserRole).toString();
        close();
    }
}

void StartupDialog::OnSelectionChanged() {
    auto items = ui->listWidget->selectedItems();
    auto item = items.size() ? items.at(0) : nullptr;
    if (item) {
        selected_project_ = item->data(Qt::UserRole).toString();
        ui->ok_pushButton->setEnabled(true);
    }
}

void StartupDialog::OnBrowse() {
    open_project_ = true;
    close();
}

void StartupDialog::OnNewProject() {
    new_project_ = true;
    close();
}

void StartupDialog::OnOk() {
    close();
}
