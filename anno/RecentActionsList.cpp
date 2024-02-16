// Anno Labeling Tool
// 2020-2024 (c) urobots GmbH, https://urobots.io/en/portfolio/anno/

#include "RecentActionsList.h"
#include <QFileInfo>
#include <QSettings>

RecentActionsList::RecentActionsList(QObject *parent)
    : QObject(parent)
{
}

RecentActionsList::~RecentActionsList()
{
}

void RecentActionsList::Init(QString settings_key, QMenu *parent_menu, int max_actions, bool is_fileslist) {
    settings_key_ = settings_key;
    is_fileslist_ = is_fileslist;
    menu_ = parent_menu;

    actions_.resize(max_actions);
    for (auto& recent_action : actions_) {
        recent_action = new QAction(this);
        recent_action->setVisible(false);
        parent_menu->addAction(recent_action);
        connect(recent_action, &QAction::triggered, this, &RecentActionsList::OnActionTriggered);
    }

    UpdateActions();
}

void RecentActionsList::AddValue(QString value) {
    if (is_fileslist_ && !value.length())
        return;

    QSettings settings;

    if (is_fileslist_) {
        // use absolute filename
        QFileInfo fi(value);
        if (fi.isAbsolute()) {
            value = fi.absoluteFilePath();
        }
    }

    QStringList values = settings.value(settings_key_).toStringList();
    values.removeAll(value);
    values.prepend(value);
    while (values.size() > int(actions_.size())) {
        values.removeLast();
    }

    settings.setValue(settings_key_, values);

    UpdateActions();
}

void RecentActionsList::OnActionTriggered() {
    QAction *action = qobject_cast<QAction *>(sender());
    if (action) {
        auto data = action->data().toString();

        AddValue(data);

        emit ActionSelected(data);
    }
}

void RecentActionsList::UpdateActions() {
    QSettings settings;
    QStringList values = settings.value(settings_key_).toStringList();
    int num_values = qMin(values.size(), static_cast<int>(actions_.size()));
    for (int i = 0; i < num_values; ++i) {
        QString text = tr("&%1 %2").arg(i + 1).arg(values[i]);
        actions_[i]->setText(text);
        actions_[i]->setData(values[i]);
        actions_[i]->setVisible(true);
    }

    menu_->setEnabled(num_values > 0);

    for (size_t j = num_values; j < actions_.size(); ++j) {
        actions_[j]->setVisible(false);
    }
}

QStringList RecentActionsList::GetActions() {
    return QSettings().value(settings_key_).toStringList();
}
