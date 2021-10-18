// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
//
// Anno Labeling Tool
// 2020-2021 (c) urobots GmbH, https://urobots.io

#include "ApplicationModel.h"
#include "LabelFactory.h"
#include "LocalFilesystem.h"
#include "messagebox.h"
#include "RestDatasetFilesystem.h"
#include "PropertyDatabase.h"
#include "ProxyLabel.h"
#include "qjson_helpers.h"
#include "Serialization.h"
#include "rest.h"
#include <QDir>
#include <QJsonArray>
#include <QJsonObject>

#define K_DEFINITIONS "definitions"
#define K_FILES_LOADER "files_loader"
#define K_FILES_ROOT_DIR "files_root_dir"
#define K_MARKER_TYPES "marker_types"
#define K_USER_DATA "user_data"
#define K_IMAGE_SCRIPT "image_script" // deprecated
#define K_PROJECT_SCRIPT "project_script"
#define K_FILES "files"

#define K_FILE_NAME "name"
#define K_FILE_MARKERS "markers"

#define K_MARKER_TYPE_NAME "type"
#define K_MARKER_CATEGORY "category"
#define K_MARKER_TEXT "text"
#define K_MARKER_SHARED_INDEX "shared_index"
#define K_MARKER_VALUE "value"
#define K_MARKER_CHILDREN "children"
#define K_MARKER_CUSTOM_PROPERTIES "custom_properties"

using namespace std;

ApplicationModel::ApplicationModel(QObject *parent)
    : QObject(parent) {
}

ApplicationModel::~ApplicationModel() {
}

void ApplicationModel::ClearProject() {
    set_project_filename(QString());
    set_filesystem({});
    set_label_definitions({});

    user_data_ = QJsonObject();
    files_loader_ = QJsonObject();
    file_models_.clear();

    PropertyDatabase::Instance().Clear();
    set_is_modified(false);
}

void ApplicationModel::NewProject(QString images_folder) {
    ClearProject();

    pictures_path_original_ = images_folder;

    set_filesystem(std::make_shared<LocalFilesystem>(images_folder));

    auto empty_definitions = std::vector<std::shared_ptr<LabelDefinition>>();
    set_label_definitions(std::make_shared<LabelDefinitionsTreeModel>(this, empty_definitions));
    connect(get_label_definitions().get(), &LabelDefinitionsTreeModel::Changed, this, &ApplicationModel::SetModified);
}

bool ApplicationModel::OpenProject(QString filename, QStringList & errors) {
    ClearProject();

    QJsonObject json;
    if (filename.startsWith("http://")) {
        // load anno from the remote dataset
        QByteArray json_content;
        try {
            json_content = rest::ExchangeData(filename + "/attrs/get_anno/call/", {}, rest::ContentType::json);
        }
        catch (std::exception & e) {
            errors << tr("Error while getting dataset content: %0").arg(QString::fromLatin1(e.what()));
            return false;
        }

        QString error;
        QJsonDocument document = LoadJsonFromText(json_content, error);
        if (document.isNull()) {
            errors << error;
            return false;
        }

        json = document.object()["retval"].toObject();
    }
    else {
        // load anno from the local file
        if (!QFileInfo(filename).exists()) {
            errors << tr("File does not exist: %0").arg(filename);
            return false;
        }

        QFile jsonFile(filename);
        jsonFile.open(QFile::ReadOnly);
        auto json_content = jsonFile.readAll();

        QString error;
        QJsonDocument document = LoadJsonFromText(json_content, error);
        if (document.isNull()) {
            errors << error;
            return false;
        }
        else {
            json = document.object();
        }
    }

    if (OpenProject(json, filename, errors)) {
        set_project_filename(filename);
        return true;
    }
    else {
        ClearProject();
        return false;
    }
}

std::vector<std::shared_ptr<LabelDefinition>> ApplicationModel::LoadLabelDefinitions(const QJsonObject & types, QStringList& errors) {
    std::vector<std::shared_ptr<LabelDefinition>> definitions;
    for (auto key : types.keys()) {
        auto def = DeserializeLabelDefinition(types[key].toObject(), errors);        
        if (def) {
            def->set_type_name(key);
            definitions.push_back(def);
        }
    }

    return definitions;
}

bool ApplicationModel::ApplyHeader(QJsonObject json, QStringList & errors) {
    auto definitions = LoadLabelDefinitions(json[K_MARKER_TYPES].toObject(), errors);
    if (errors.size()) {
        return false;
    }

    // assignments, will be performed, if all checks will be OK
    std::map<Label*, std::shared_ptr<LabelCategory>> assignments;    

    // make sure all existing markers have definitions    
    for (auto & file : file_models_) {
        for (auto & label : file.second->labels_) {
            auto old_category = label->GetCategory();
            auto old_definition = old_category->GetDefinition();

            auto new_definition = std::find_if(definitions.begin(), definitions.end(),
                [&](std::shared_ptr<LabelDefinition> & def) {
                return def->get_type_name() == old_definition->get_type_name();
            });

            if (new_definition == definitions.end()) {
                errors << tr("Missing label definition: \"%0\"").arg(old_definition->get_type_name());
                break;
            }

            if (!(*new_definition)->GetCategory(old_category->get_value())) {
                errors << tr("Missing category \"%0\" of definition: \"%1\"")
                    .arg(old_category->get_value())
                    .arg(old_definition->get_type_name());
                break;
            }

            auto value_type_old = old_definition->value_type;
            auto value_type_new = (*new_definition)->value_type;
            if (value_type_old != value_type_new) {
                errors << tr("Cannot change label type of \"%0\" from \"%1\" to \"%2\"")
                    .arg(old_definition->get_type_name())
                    .arg(LabelTypeToString(value_type_old))
                    .arg(LabelTypeToString(value_type_new));
                break;
            }

            auto is_shared_old = old_definition->is_shared();
            auto is_shared_new = (*new_definition)->is_shared();
            if (is_shared_new != is_shared_old) {
                errors << tr("Cannot change shared type of \"%0\" from \"%1\" to \"%2\"")
                    .arg(old_definition->get_type_name())
                    .arg(is_shared_old)
                    .arg(is_shared_new);
                break;
            }

            if (is_shared_new) {
                // make sure shared label includes our index
                auto index = label->GetSharedLabelIndex();
                auto shared_count_old = int(old_definition->shared_labels.size());
                auto shared_count_new = int((*new_definition)->shared_labels.size());
                if (index >= shared_count_new) {
                    errors << tr("Cannot reduce shared_count type of \"%0\" from \"%1\" to \"%2\"")
                        .arg(old_definition->get_type_name())
                        .arg(shared_count_old)
                        .arg(shared_count_new);
                    break;
                }
                else {
                    auto proxy = dynamic_cast<ProxyLabel*>(label.get());
                    if (!proxy) {
                        errors << tr("Intenal error, cannot get cast to ProxyLabel label of \"%0\"")
                            .arg(old_definition->get_type_name());
                        break;
                    }
                    (*new_definition)->shared_labels[index] = proxy->GetProxyClient();
                    assignments[proxy->GetProxyClient().get()] = (*new_definition)->categories[0];
                }
            }

            assignments[label.get()] = (*new_definition)->GetCategory(old_category->get_value());
        }
    }

    if (errors.size()) {
        return false;
    }

    // apply new labels
    for (auto a : assignments) {
        a.first->SetCategory(a.second);
    }

    // reconnect all labels to the database
    for (auto & file : file_models_) {
        for (auto & label : file.second->labels_) {
            label->ConnectSharedProperties(true, false);
        }
    }

    // replace label definitions tree
    set_label_definitions(std::make_shared<LabelDefinitionsTreeModel>(this, definitions));
    connect(get_label_definitions().get(), &LabelDefinitionsTreeModel::Changed, this, &ApplicationModel::SetModified);
        
    ApplyBasicDefinitions(json);

    // disable all undo commands, because undoing now is not safe
    for (auto & f : file_models_) {
        f.second->GetUndoStack()->clear();
    }

    SetModified();

    return true;
}

void ApplicationModel::ApplyBasicDefinitions(QJsonObject & definitions) {
    user_data_ = definitions[K_USER_DATA].toObject();
    pictures_path_original_ = definitions[K_FILES_ROOT_DIR].toString();
    files_loader_ = definitions[K_FILES_LOADER].toObject();
    QString project_script = ArrayToString(definitions[K_PROJECT_SCRIPT]);
    if (project_script.isEmpty()) {
        // try deprecated script name
        project_script = ArrayToString(definitions[K_IMAGE_SCRIPT]);
    }
    set_project_script(project_script);
}

bool ApplicationModel::OpenProject(const QJsonObject& json, QString anno_filename, QStringList & errors) {
    auto definitions = json[K_DEFINITIONS].toObject();
    ApplyBasicDefinitions(definitions);

    if (anno_filename.startsWith("http://")) {
        set_filesystem(std::make_shared<RestDatasetFilesystem>(anno_filename, pictures_path_original_));
    }
    else {
        auto files_root_dir_s = pictures_path_original_;
        QDir file_root_dir(files_root_dir_s);
        if (!file_root_dir.isAbsolute()) {
            // Get path relative to the file path.
            files_root_dir_s = QFileInfo(anno_filename).dir().filePath(files_root_dir_s);
        }
        set_filesystem(std::make_shared<LocalFilesystem>(files_root_dir_s));
    }

    auto label_definitions = LoadLabelDefinitions(definitions[K_MARKER_TYPES].toObject(), errors);
    if (errors.size()) {
        return false;
    }
    set_label_definitions(std::make_shared<LabelDefinitionsTreeModel>(this, label_definitions));

    connect(get_label_definitions().get(), &LabelDefinitionsTreeModel::Changed, this, &ApplicationModel::SetModified);    

    for (auto file_marker : json[K_FILES].toArray()) {
        auto filename = file_marker.toObject()[K_FILE_NAME].toString();
        auto file_model = GetFileModel(filename);

        for (auto m : file_marker.toObject()[K_FILE_MARKERS].toArray()) {
            auto marker = m.toObject();
            auto type_name = marker[K_MARKER_TYPE_NAME].toString();
            auto category = marker[K_MARKER_CATEGORY].toInt();

            auto definition = get_label_definitions()->FindDefinition(type_name);
            if (!definition) {
                errors << tr("Definition with name \"%0\" does not exist").arg(type_name);
                continue;
            }

            if (!definition->categories.size()) {
                errors << tr("Definition \"%0\" has no categories").arg(definition->get_type_name());
            }

            if (!definition->GetCategory(category)) {
                errors << tr("Definition \"%0\" has no category \"%1\"")
                    .arg(definition->get_type_name())
                    .arg(category);
                continue;
            }

            auto shared_index = marker[K_MARKER_SHARED_INDEX].toInt(0);
            if (definition->is_shared() && shared_index >= int(definition->shared_labels.size())) {
                errors << tr("Definition \"%0\" shared count is less than label shared index \"%1\"")
                    .arg(definition->get_type_name())
                    .arg(shared_index);
                continue;
            }

            std::shared_ptr<Label> label;
            if (definition->is_shared()) {
                label = std::make_shared<ProxyLabel>(definition->shared_labels[shared_index]);
                label->SetSharedLabelIndex(shared_index);
            }
            else {
                label = LabelFactory::CreateLabel(definition->value_type);

                if (!label) {
                    errors << tr("Failed to create label with type \"%0\" for definition \"%1\"")
                        .arg(LabelTypeToString(definition->value_type))
                        .arg(definition->get_type_name());
                    continue;
                }
            }

            // get value of a marker
            QStringList value = { marker[K_MARKER_VALUE].toString() };
            auto children = marker[K_MARKER_CHILDREN].toArray();
            for (auto child : children) {
                value.push_back(child.toObject()[K_MARKER_VALUE].toString());
            }
        
            label->SetText(marker[K_MARKER_TEXT].toString(""));
            label->SetCategory(definition->GetCategory(category));
            label->ConnectSharedProperties(true, false);

            if (marker.contains(K_MARKER_CUSTOM_PROPERTIES)) {
                auto &custom_properties = label->GetCustomProperties();
                auto jcustom_properties = marker[K_MARKER_CUSTOM_PROPERTIES].toObject();
                for (auto key : jcustom_properties.keys()) {
                    custom_properties[key] = jcustom_properties[key].toVariant();
                }
            }
        
            label->FromStringsList(value);
            file_model->labels_.push_back(label);
        }
    }

    return errors.size() == 0;
}

QJsonObject ApplicationModel::GenerateHeader() {
    QJsonObject header;

    header.insert(K_FILES_ROOT_DIR, QJsonValue::fromVariant(pictures_path_original_));

    if (!files_loader_.empty()) {
        header.insert(K_FILES_LOADER, files_loader_);
    }

    if (!user_data_.empty()) {
        header.insert(K_USER_DATA, user_data_);
    }

    if (project_script_.length()) {
        header.insert(K_PROJECT_SCRIPT, ToJsonArray(project_script_));
    }

    QJsonObject types;
	if (label_definitions_) {
		for (auto def : label_definitions_->GetDefinitions()) {
            types.insert(def->get_type_name(), Serialize(def));
		}
	}
    header.insert(K_MARKER_TYPES, types);
    return header;
}

void ApplicationModel::UpdateSharedProperties(bool forced_update) {
    for (auto i : file_models_) {
        for (auto l : i.second->labels_) {
            l->UpdateSharedProperties(forced_update);
        }
    }
}

bool ApplicationModel::SaveProject(QStringList & errors, QString filename) {
    if (get_project_filename().isEmpty() || !filename.isEmpty()) {
        if (filename.isEmpty()) {
            errors << tr("Project filename is not defined");
            return false;
        }

        auto pictures_path = pictures_path_original_;
        if (QFileInfo(pictures_path).isRelative()) {
            pictures_path = QFileInfo(get_project_filename()).dir().filePath(pictures_path);
        }

        QFileInfo pictures_path_info(pictures_path);
        QFileInfo project_file_info(filename);
        
        pictures_path_original_ = project_file_info.dir().relativeFilePath(pictures_path_info.absoluteFilePath());

        set_project_filename(filename);
    }

    QJsonObject root;
    root.insert(K_DEFINITIONS, GenerateHeader());

    // save all markers
    QJsonArray markers;
    for (auto i : file_models_) {
        if (!i.second->labels_.size())
            continue;

        QJsonObject file;
        file.insert(K_FILE_NAME, QJsonValue::fromVariant(i.first));

        QJsonArray file_markers;
        for (auto label : i.second->labels_) {
            QJsonObject marker;
            marker.insert(K_MARKER_CATEGORY, QJsonValue::fromVariant(label->GetCategory()->get_value()));
            marker.insert(K_MARKER_TYPE_NAME, QJsonValue::fromVariant(label->GetDefinition()->get_type_name()));

            // Save text only if not empty
            auto text = label->GetText();
            if (!text.isEmpty())
                marker.insert(K_MARKER_TEXT, QJsonValue::fromVariant(text));

            auto shared_index = label->GetSharedLabelIndex();
            if (shared_index)
                marker.insert(K_MARKER_SHARED_INDEX, QJsonValue::fromVariant(shared_index));

            auto value = label->ToStringsList();
            marker.insert(K_MARKER_VALUE, value[0]);

            if (value.size() > 1) {
                QJsonArray children;
                for (int i = 1; i < value.size(); ++i) {
                    QJsonObject child;
                    child.insert(K_MARKER_VALUE, QJsonValue::fromVariant(value[i]));
                    children.push_back(child);
                }
                marker.insert(K_MARKER_CHILDREN, children);
            }
            
            auto &custom_properties = label->GetCustomProperties();
            if (custom_properties.size()) {
                QJsonObject jcustom_properties;
                for (auto key : custom_properties.keys()) {
                    jcustom_properties.insert(key, QJsonValue::fromVariant(custom_properties[key]));
                }
                marker.insert(K_MARKER_CUSTOM_PROPERTIES, jcustom_properties);
            }

            file_markers.push_back(marker);
        }

        file.insert(K_FILE_MARKERS, file_markers);
        markers.push_back(file);
    }
    root.insert(K_FILES, markers);

    if (get_project_filename().startsWith("http://")) {
        // save project to the remote dataset
        QJsonObject parameters;
        parameters["anno"] = root;
        QByteArray response_json_content;
        try {
            response_json_content = rest::ExchangeData(get_project_filename() + "/attrs/set_anno/call/", parameters, rest::ContentType::json);
        }
        catch (std::exception & e) {
            errors << tr("Failed to save anno file to server: %0").arg(QString::fromLatin1(e.what()));
            return false;
        }
    }
    else {
        // save project to the local file
        QFile jsonFile(get_project_filename());
        if (jsonFile.open(QFile::WriteOnly)) {
            jsonFile.write(QJsonDocument(root).toJson());
        }
        else {
            errors << tr("Failed to save project into file: %0").arg(get_project_filename());
            return false;
        }
    }

    for (auto file : file_models_) {
        file.second->set_is_modified(false);
    }

    set_is_modified(false);
    return true;
}

void ApplicationModel::set_project_script(QString value) {
    if (project_script_ != value) {
        project_script_ = value;
        project_script_changed(value);
    }
}

std::shared_ptr<FileModel> ApplicationModel::GetFileModel(QString file_id) {
    auto it = file_models_.find(file_id);
    if (it != file_models_.end()) {
        return it->second;
    }

    auto model = std::make_shared<FileModel>(file_id);
    file_models_[file_id] = model;
    connect(model.get(), &FileModel::is_modified_changed, this, &ApplicationModel::OnFileModifiedChanged);
    return model;
}

std::shared_ptr<FileModel> ApplicationModel::GetFileModel(QStringList path) {
    return GetFileModel(path.join("/"));
}

void ApplicationModel::DeleteAllLabels(QStringList path) {
    auto path_id = path.join("/");
    for (auto & i : file_models_) {
        if (!i.second->labels_.size()) {
            continue;
        }

        auto file_id = i.first;
        if (file_id.startsWith(path_id)) {
            auto file_id_rest = file_id.right(file_id.length() - path_id.length());
            if (path_id.length() && file_id_rest.contains("/")) {
                if (file_id_rest[0] != '/') {
                    // i.e. path = "1/a" and file_id = "1/aAAA..."
                    continue;
                }
            }
            i.second->DeleteAllLabels();
        }
    }
}

std::vector<FileTreeItemInfo> ApplicationModel::GetFolderInfo(QStringList path) {
    auto path_id = path.join("/");
    if (path_id.length()) {
        path_id += "/";
    }

    std::set<QString> subdirs;
    std::vector<FileTreeItemInfo> result;
    for (auto & i : file_models_) {
        if (!i.second->labels_.size()) {
            continue;
        }

        auto file_id = i.first;
        if (file_id.startsWith(path_id)) {
            auto file_id_rest = file_id.right(file_id.length() - path_id.length());
            auto parts = file_id_rest.split("/");
            if (parts.length() == 1) {
                FileTreeItemInfo file;
                file.name = parts[0];
                file.is_folder = false;
                result.push_back(file);
            }
            else if (parts.length() > 1) {
                subdirs.insert(parts[0]);
            }
        }
    }

    for (auto & subdir: subdirs) {
        FileTreeItemInfo file;
        file.name = subdir;
        file.is_folder = true;
        result.push_back(file);
    }

    return result;
}

void ApplicationModel::Rename(QStringList source, QStringList destination) {
    auto source_path = source.join("/");
    auto destination_path = destination.join("/");  
    bool changed = false;

    if (file_models_.count(source_path) == 1) {
        // It is a file model, rename it.
        auto file = file_models_[source_path];
        file_models_.erase(source_path);
        file_models_[destination_path] = file;
        file->set_id(destination_path);   
        changed = true;
    }
    else {
        // This might be a folder.
        source_path += "/";
        destination_path += "/";
        QStringList files_to_rename;
        for (auto i : file_models_) {
            if (i.first.startsWith(source_path)) {
                files_to_rename.push_back(i.first);
            }
        }
        for (auto n : files_to_rename) {
            auto file = file_models_[n];
            file_models_.erase(n);
            
            auto n_new = destination_path + n.right(n.length() - source_path.length());
            file_models_[n_new] = file;
            file->set_id(n_new);
            changed = true;
        }
    }

    if (changed) {
        set_is_modified(true);
    }
}

void ApplicationModel::OnFileModifiedChanged(bool value) {
    if (value) {
        set_is_modified(true);
    }
}

std::set<int> ApplicationModel::GetExistingSharedIndexes(shared_ptr<LabelDefinition> def) {
    std::set<int> result;
    for (auto i : file_models_) {
        auto file_set = i.second->GetExistingSharedIndexes(def);
        result.insert(file_set.begin(), file_set.end());
    }
    return result;
}

int ApplicationModel::GetLabelsCount(std::shared_ptr<LabelDefinition> def) {
    int result = 0;
    for (auto i : file_models_) {
        for (auto l: i.second->labels_) {
            if (l->GetDefinition() == def) {
                result++;
            }
        }
    }
    return result;
}

std::shared_ptr<FileModel> ApplicationModel::GetFirstFileModel() {
    if (file_models_.size()) {
        return file_models_.begin()->second;
    }
    return nullptr;
}

std::shared_ptr<ImageConverter> ApplicationModel::GetImageConverter() {
    /*
    if (!files_loader_.empty()) {        
        For possible use in the future:
        auto converter_type = files_loader_["type"].toString();                
        if (converter_type == "MyType") {
            return std::make_shared<MyTypeConverter>(files_loader_["params"].toObject());
        }        
    }
    */
    return {};
}

void ApplicationModel::Delete(std::shared_ptr<LabelDefinition> marker, bool delete_only_instances) {
    for (auto file : file_models_) {
        file.second->Delete(marker, nullptr);
    }

    if (delete_only_instances) {
        return;
    }

    label_definitions_->Delete(marker);
}

void ApplicationModel::Delete(std::shared_ptr<LabelCategory> category, bool delete_only_instances) {
    for (auto file : file_models_) {
        file.second->Delete(nullptr, category);
    }

    if (delete_only_instances) {
        return;
    }

    label_definitions_->Delete(category);
}

void ApplicationModel::UpdateDefinitionSharedCount(std::shared_ptr<LabelDefinition> def, int new_shared_count) {
    if (int(def->shared_labels.size()) == new_shared_count || def->categories.size() == 0) {
        // nothing to update
        return;
    }

    // Create new shared labels
    vector<shared_ptr<Label>> shared_labels;
    for (int i = 0; i < new_shared_count; ++i) {
        auto shared_label = LabelFactory::CreateLabel(def->value_type);

        // setup valid shared label index
        shared_label->SetSharedLabelIndex(i);

        // use first category
        shared_label->SetCategory(def->categories[0]);

        // connect to the database
        shared_label->ConnectSharedProperties(true, false);

        shared_labels.push_back(shared_label);
    }

    for (auto file: file_models_) {
        file.second->UpdateDefinitionSharedLabels(def, shared_labels);
    }

    def->shared_labels = shared_labels;
}

void ApplicationModel::UpdateDenitionSharedProperties(std::shared_ptr<LabelDefinition> def, std::map<std::string, SharedPropertyDefinition> props) {
    set<string> to_remove;
    set<string> to_add;
    set<string> to_modify;

    for (auto p: def->shared_properties) {
        if (props.count(p.first)) {
            auto p1 = p.second;
            auto p2 = props[p.first];
            if (p1->name != p2.name ||
                fabs(p1->a - p2.a) > SharedPropertyDefinition::eps() ||
                fabs(p1->b - p2.b) > SharedPropertyDefinition::eps()) {
                to_modify.insert(p.first);
            }
        }
        else {
            to_remove.insert(p.first);
        }
    }

    for (auto p: props) {
        if (def->shared_properties.count(p.first) == 0) {
            to_add.insert(p.first);
        }
    }

    if (to_add.empty() && to_remove.empty() && to_modify.empty()) {
        return;
    }

    // property changes from shared -> not shared
    for (auto p: to_remove) {
        def->shared_properties.erase(def->shared_properties.find(p));
    }
    // property changes from not shared -> shared
    for (auto p: to_add) {
        def->shared_properties[p] = make_shared<SharedPropertyDefinition>();
        *def->shared_properties[p] = props[p];
    }
    // property changes from one shared -> another shared
    for (auto p: to_modify) {
        *def->shared_properties[p] = props[p];
    }

    for (auto file : file_models_) {
        file.second->ReconnectSharedProperties(def);
    }
}

void ApplicationModel::UpdateDefinitionCustomProperties(std::shared_ptr<LabelDefinition> def, std::vector<CustomPropertyDefinition> props, QStringList original_names) {
    for (auto file : file_models_) {
        file.second->UpdateDefinitionCustomProperties(def, props, original_names);
    }
    def->custom_properties = props;
}
