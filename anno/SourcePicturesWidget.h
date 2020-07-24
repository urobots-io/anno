#pragma once
#include "ApplicationModel.h"
#include "ui_SourcePicturesWidget.h"
#include <QWidget>

class SourcePicturesWidget : public QWidget
{
	Q_OBJECT

public:
    SourcePicturesWidget(QWidget *parent);
	~SourcePicturesWidget();

    void Init(ApplicationModel *model);

public slots:
	void OnCurrentChanged(const QModelIndex &current, const QModelIndex &previous);
	void OnNewFileSystem(std::shared_ptr<FilesystemInterface>);

    void OnCustomContextMenu(const QPoint &point);

    void SelectNextFile();
    void SelectPreviousFile();

    void OnRenameFile();
    void OnDeleteFile();
    void OnReloadFolder();
    void OnRevealInExplorer();
    void OnDeleteFileMarkers();
    void OnCreateFolder();

private:
    void SelectFile(int offset);

    template<class T>
    QAction* AddAction(QMenu *menu, QString icon, QString text, T callback);

private slots:
    void OnTextFilterChanged();
    void OnLoadedFilterChanged(bool);

signals:
    void FileModelSelected(std::shared_ptr<FileModel>);

private:
	Ui::SourcePicturesWidget ui;
	ApplicationModel *model_ = nullptr;
    SourcePicturesTreeModel *tree_model_ = nullptr;
    SourcePicturesProxyModel *sort_filter_model_ = nullptr;

    QMenu *file_menu_ = nullptr;
    QMenu *folder_menu_ = nullptr;

    QModelIndex menu_index_;

    QAction* delete_folder_action_ = nullptr;
    QAction* rename_folder_action_ = nullptr;
};
