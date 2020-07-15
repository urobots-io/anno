#pragma once
#include <QMenu>
#include <QObject>

class RecentActionsList : public QObject
{
    Q_OBJECT

public:
    RecentActionsList(QObject *parent = nullptr);
    ~RecentActionsList();

    void Init(QString settings_key, QMenu *parent_menu, int max_actions = 15, bool is_fileslist = true);
    QStringList GetActions();

signals:
    void ActionSelected(QString);

public slots:
    void OnActionTriggered();
    void AddValue(QString);

private:
    void UpdateActions();

private:
    QString settings_key_;
    std::vector<QAction*> actions_;
    QMenu *menu_ = nullptr;
    bool is_fileslist_ = true;
};
