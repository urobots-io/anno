#pragma once
#include "FileModel.h"
#include "ImageConverter.h"
#include "implement_q_property.h"
#include "LabelDefinitionsTreeModel.h"
#include "SourcePicturesTreeModel.h"
#include <QObject>

class ApplicationModel : public QObject, public FileModelProviderInterface
{
	Q_OBJECT

public:
	ApplicationModel(QObject *parent);
	~ApplicationModel();

    Q_PROPERTY(bool is_modified READ get_is_modified WRITE set_is_modified NOTIFY is_modified_changed);    
    Q_PROPERTY(QString project_filename READ get_project_filename WRITE set_project_filename NOTIFY project_filename_changed);	    
    Q_PROPERTY(QString project_script READ get_project_script WRITE set_project_script NOTIFY project_script_changed);
    Q_PROPERTY(std::shared_ptr<LabelDefinitionsTreeModel> label_definitions READ get_label_definitions WRITE set_label_definitions NOTIFY label_definitions_changed);
    Q_PROPERTY(std::shared_ptr<FilesystemInterface> filesystem READ get_filesystem WRITE set_filesystem NOTIFY filesystem_changed);
    Q_PROPERTY(QJsonObject user_data READ get_user_data WRITE set_user_data NOTIFY user_data_changed);

    void NewProject(QString images_folder);
    bool OpenProject(QString filename, QStringList &errors);
    bool SaveProject(QStringList & errors, QString filename = QString());
    void ClearProject();

    std::shared_ptr<FileModel> GetFileModel(QString file_id);

    std::shared_ptr<FileModel> GetFileModel(QStringList path) override;
    std::vector<FileTreeItemInfo> GetFolderInfo(QStringList path) override;
    void DeleteAllLabels(QStringList path) override;
    void Rename(QStringList source, QStringList destination) override;

    QJsonObject GenerateHeader();
    bool ApplyHeader(QJsonObject, QStringList & errors);

    /// Return shared indexes which are present in the document
    std::set<int> GetExistingSharedIndexes(std::shared_ptr<LabelDefinition> def);

    /// Returns count of existing labels for a definition
    int GetLabelsCount(std::shared_ptr<LabelDefinition> def);

    /// Test function
    std::shared_ptr<FileModel> GetFirstFileModel();

    /// Update all shared properties
    void UpdateSharedProperties(bool forced_update = false);

    /// Return object which converts images after loading.
    std::shared_ptr<ImageConverter> GetImageConverter();

    /// Delete label definition or its instances from all files
    void Delete(std::shared_ptr<LabelDefinition>, bool delete_only_instances);

    /// Delete label category or its instances from all files
    void Delete(std::shared_ptr<LabelCategory>, bool delete_only_instances);

    /// Change shared count of definition to a new number
    void UpdateDefinitionSharedCount(std::shared_ptr<LabelDefinition> def, int new_shared_count);

    /// Change shared properties.
    void UpdateDenitionSharedProperties(std::shared_ptr<LabelDefinition> def, std::map<QString, SharedPropertyDefinition>);

    /// Update custom properties.
    void UpdateDefinitionCustomProperties(std::shared_ptr<LabelDefinition> def, std::vector<CustomPropertyDefinition>, QStringList);

    /// Trigger update of labels internal data.
    void UpdateDefinitionInternalData(std::shared_ptr<LabelDefinition> def);

    /// Update all labels
    void BatchUpdate(std::shared_ptr<LabelDefinition> def, float dx, float dy, float da, bool use_label_cs);

public slots:
	void SetModified() { set_is_modified(true); }
    void OnFileModifiedChanged(bool value);
    void DetectPlates(std::shared_ptr<FileModel>, const ImageData &image, QPointF image_offset);

    DECLARE_Q_PROPERTY_WRITE(QString, project_script);
    IMPLEMENT_Q_PROPERTY_WRITE(bool, is_modified); 
    IMPLEMENT_Q_PROPERTY_WRITE_ALWAYS(QJsonObject, user_data);

private:
	IMPLEMENT_Q_PROPERTY_WRITE(QString, project_filename);	
	IMPLEMENT_Q_PROPERTY_WRITE(std::shared_ptr<LabelDefinitionsTreeModel>, label_definitions);
    IMPLEMENT_Q_PROPERTY_WRITE(std::shared_ptr<FilesystemInterface>, filesystem);

    std::vector<std::shared_ptr<LabelDefinition>> LoadLabelDefinitions(const QJsonObject & types, QStringList& errors);
        
signals:
    void project_script_changed(QString);
    void is_modified_changed(bool);
	void project_filename_changed(QString);		
	void pictures_path_changed(QString);	
	void label_definitions_changed(std::shared_ptr<LabelDefinitionsTreeModel>);
    void filesystem_changed(std::shared_ptr<FilesystemInterface>);
    void user_data_changed(QJsonObject);

    void ApplicationShutdown();

private:
    bool OpenProject(const QJsonObject& json, QString anno_filename, QStringList & errors);
    void ApplyBasicDefinitions(QJsonObject & definitions);

private:
    bool is_modified_ = false;
	QString project_filename_;
    QString pictures_path_original_;	    
    std::shared_ptr<LabelDefinitionsTreeModel> label_definitions_;
    std::shared_ptr<FilesystemInterface> filesystem_;    

    std::map<QString, std::shared_ptr<FileModel>> file_models_;
	
    QJsonObject user_data_;
    QJsonObject files_loader_;

    QString project_script_;

public:
    IMPLEMENT_Q_PROPERTY_READ(is_modified);
    IMPLEMENT_Q_PROPERTY_READ(user_data);
    IMPLEMENT_Q_PROPERTY_READ(project_script);
	IMPLEMENT_Q_PROPERTY_READ(project_filename);
	IMPLEMENT_Q_PROPERTY_READ(label_definitions);
    IMPLEMENT_Q_PROPERTY_READ(filesystem);
};
