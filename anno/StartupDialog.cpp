#include "StartupDialog.h"
#include "ui_StartupDialog.h"
#include <QScrollBar>
#include <QSettings>

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
        auto item = new QListWidgetItem(project, ui->listWidget);
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
        selected_project_ = item->text();
        close();
    }
}

void StartupDialog::OnSelectionChanged() {
    auto items = ui->listWidget->selectedItems();
    auto item = items.size() ? items.at(0) : nullptr;
    if (item) {
        selected_project_ = item->text();
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
