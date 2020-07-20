#include "ApplicationModel.h"
#include "LabelFactory.h"
#include "LocalFilesystem.h"
#include "messagebox.h"
#include "RestDatasetFilesystem.h"
#include "RestImageConverter.h"
#include "ProxyLabel.h"
#include "qjson_helpers.h"
#include "rest.h"
#include <QDir>
#include <QJsonArray>
#include <QJsonObject>

#define K_DEFINITIONS "definitions"
#define K_FILES_LOADER "files_loader"
#define K_FILES_ROOT_DIR "files_root_dir"
#define K_MARKER_TYPES "marker_types"
#define K_USER_DATA "user_data"
#define K_IMAGE_SCRIPT "image_script"
#define K_FILES "files"

#define K_DEFINITION_NAME "name"
#define K_DEFINITION_DESCRIPTION "description"
#define K_DEFINITION_VALUE_TYPE "value_type"
#define K_DEFINITION_LINE_WIDTH "line_width"
#define K_DEFINITION_IS_STAMP "stamp"
#define K_DEFINITION_AXIS_LENGTH "axis_length"
#define K_DEFINITION_RENDERING_SCRIPT "rendering_script"
#define K_DEFINITION_STAMP_PARAMETERS "stamp_parameters"
#define K_DEFINITION_CATEGORIES "categories"
#define K_DEFINITION_SHARED "shared"
#define K_DEFINITION_SHARED_COUNT "shared_count"
#define K_DEFINITION_FILENAME_FILTER "filename_filter"
#define K_DEFINITION_SHARED_PROPERTIES "shared_properties"
#define K_DEFINITION_CUSTOM_PROPERTIES "custom_properties"

#define K_DEFINITION_SHARED_PROPERTY_NAME "name"
#define K_DEFINITION_SHARED_PROPERTY_LINEAR_TRANSFORM_A "a"
#define K_DEFINITION_SHARED_PROPERTY_LINEAR_TRANSFORM_B "b"

#define K_CATEGORY_ID "id"
#define K_CATEGORY_NAME "name"
#define K_CATEGORY_COLOR "color"

#define K_FILE_NAME "name"
#define K_FILE_MARKERS "markers"

#define K_MARKER_TYPE_NAME "type"
#define K_MARKER_CATEGORY "category"
#define K_MARKER_TEXT "text"
#define K_MARKER_SHARED_INDEX "shared_index"
#define K_MARKER_VALUE "value"
#define K_MARKER_CHILDREN "children"
#define K_MARKER_CUSTOM_PROPERTIES "custom_properties"

#define K_CUSTOM_PROP_TYPE "type"
#define K_CUSTOM_PROP_DEFAULT_VALUE "default"
#define K_CUSTOM_PROP_CASES "cases"

using namespace urobots::qt_helpers;

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

namespace {
std::vector<std::shared_ptr<LabelDefinition>> LoadLabelDefinitions(const QJsonObject & types) {
    std::vector<std::shared_ptr<LabelDefinition>> definitions;
    for (auto key : types.keys()) {
        auto def = std::make_shared<LabelDefinition>();
        def->type_name = key;

        auto def_json = types[key].toObject();

        if (def_json.contains(K_DEFINITION_DESCRIPTION)) {
            def->set_description(def_json[K_DEFINITION_DESCRIPTION].toString());
        }
        else if (def_json.contains(K_DEFINITION_NAME)) {
            // backward compatibility - transform name into description
            def->set_description(def_json[K_DEFINITION_NAME].toString());
        }
        def->value_type = LabelTypeFromString(def_json[K_DEFINITION_VALUE_TYPE].toString());

        if (def_json.contains(K_DEFINITION_LINE_WIDTH))
            def->line_width = def_json[K_DEFINITION_LINE_WIDTH].toInt();

        if (def_json.contains(K_DEFINITION_IS_STAMP))
            def->is_stamp = def_json[K_DEFINITION_IS_STAMP].toBool();

        int num_shared = 0;
        if (def_json.contains(K_DEFINITION_SHARED))
            num_shared = def_json[K_DEFINITION_SHARED].toBool() ? 1 : 0;

        if (def_json.contains(K_DEFINITION_SHARED_COUNT))
            num_shared = def_json[K_DEFINITION_SHARED_COUNT].toInt();

        if (def_json.contains(K_DEFINITION_FILENAME_FILTER)) {
            auto filter = def_json[K_DEFINITION_FILENAME_FILTER];
            if (filter.isString()) {
                def->filename_filter.push_back(filter.toString().toStdString());
            }
            else if (filter.isArray()) {
                for (const auto &i : filter.toArray()) {
                    def->filename_filter.push_back(i.toString().toStdString());
                }                
            }
        }
        
        if (def_json.contains(K_DEFINITION_AXIS_LENGTH)) {
            def->axis_length.clear();
            for (const auto &i : def_json[K_DEFINITION_AXIS_LENGTH].toArray()) {
                def->axis_length.push_back(i.toInt());
            }
        }

        if (def_json.contains(K_DEFINITION_SHARED_PROPERTIES)) {
            auto shared_props = def_json[K_DEFINITION_SHARED_PROPERTIES].toObject();
            for (auto key : shared_props.keys()) {
                auto property_def = std::make_shared<SharedPropertyDefinition>();
                auto jnode = shared_props[key];
                if (!jnode.isObject()) {
                    property_def->name = jnode.toString().toStdString();
                }
                else {
                    auto jobj = jnode.toObject();
                    property_def->name = jobj[K_DEFINITION_SHARED_PROPERTY_NAME].toString().toStdString();
                    if (jobj.contains(K_DEFINITION_SHARED_PROPERTY_LINEAR_TRANSFORM_A)) {
                        property_def->a = jobj[K_DEFINITION_SHARED_PROPERTY_LINEAR_TRANSFORM_A].toDouble();
                    }
                    if (jobj.contains(K_DEFINITION_SHARED_PROPERTY_LINEAR_TRANSFORM_B)) {
                        property_def->b = jobj[K_DEFINITION_SHARED_PROPERTY_LINEAR_TRANSFORM_B].toDouble();
                    }
                }
                def->shared_properties[key.toStdString()] = property_def;
            }
        }

        if (def_json.contains(K_DEFINITION_CUSTOM_PROPERTIES)) {
            auto custom_properties = def_json[K_DEFINITION_CUSTOM_PROPERTIES].toObject();
            for (auto key : custom_properties.keys()) {            
                auto iobj = custom_properties[key].toObject();
                CustomPropertyDefinition p;
                p.id = key;
                p.default_value = iobj[K_CUSTOM_PROP_DEFAULT_VALUE].toVariant();
                p.type = CustomPropertyTypeFromString(iobj[K_CUSTOM_PROP_TYPE].toString());
                p.cases = ToStringList(iobj[K_CUSTOM_PROP_CASES]);
                def->custom_properties.push_back(p);
            }
        }
        def->stamp_parameters = def_json[K_DEFINITION_STAMP_PARAMETERS].toObject();
        def->set_rendering_script(ArrayToString(def_json[K_DEFINITION_RENDERING_SCRIPT]));

        auto categories = def_json[K_DEFINITION_CATEGORIES].toArray();
        for (int i = 0; i < categories.size(); ++i) {
            auto json = categories[i].toObject();
            auto category = std::make_shared<LabelCategory>();

            category->value = json[K_CATEGORY_ID].toInt();
            category->name = json[K_CATEGORY_NAME].toString();
            category->color = GetStandardColor(category->value);
            category->definition = def.get();

            auto jcolor = json[K_CATEGORY_COLOR];
            if (!jcolor.isNull())
                category->color.setNamedColor(jcolor.toString());

            //xxx def->categories[category->value] = category;
            def->categories_list.push_back(category);
        }

        if (categories.size()) {
            for (int i = 0; i < num_shared; ++i) {
                // create label shared between other labels
                auto shared_label = LabelFactory::CreateLabel(def->value_type);
                if (!shared_label) {
                    // TODO(ap): propagate this error
                    throw std::runtime_error("Failed to create shared label");
                    /*
                    errors << tr("Failed to create label with type \"%0\" for definition \"%1\"")
                    .arg(LabelTypeToString(definition->label_type))
                    .arg(definition->name);
                    file_content_ok = false;
                    continue;
                    */
                }

                // setup valid shared label index
                shared_label->SetSharedLabelIndex(i);

                // use first category
                shared_label->SetCategory(def->categories_list[0].get());

                // connect to the database
                shared_label->ConnectSharedProperties(true, false);
                
                def->shared_labels.push_back(shared_label);
            }
        }

        definitions.push_back(def);
    }

    return definitions;
}
}

// TODO(ap): return list of errors
bool ApplicationModel::ApplyHeader(QJsonObject json, QString & error) {
    auto definitions = LoadLabelDefinitions(json[K_MARKER_TYPES].toObject());

    // assignments, will be performed, if all checks will be OK
    std::map<Label*, std::shared_ptr<LabelCategory>> assignments;    

    // make sure all existing markers have definitions    
    for (auto & file : file_models_) {
        for (auto & label : file.second->labels_) {
            auto old_category = label->GetCategory();
            auto old_definition = old_category->definition;

            auto new_definition = std::find_if(definitions.begin(), definitions.end(),
                [&](std::shared_ptr<LabelDefinition> & def) {
                return def->type_name == old_definition->type_name;
            });

            if (new_definition == definitions.end()) {
                error = tr("Missing label definition: \"%0\"")
                    .arg(old_definition->type_name);
                return false;
            }

            if (!(*new_definition)->GetCategory(old_category->value)) {
                error = tr("Missing category \"%0\" of definition: \"%1\"")
                    .arg(old_category->value)
                    .arg(old_definition->type_name);
                return false;
            }

            auto value_type_old = old_definition->value_type;
            auto value_type_new = (*new_definition)->value_type;
            if (value_type_old != value_type_new) {
                error = tr("Cannot change label type of \"%0\" from \"%1\" to \"%2\"")
                    .arg(old_definition->type_name)                    
                    .arg(LabelTypeToString(value_type_old))
                    .arg(LabelTypeToString(value_type_new));
                return false;
            }

            auto is_shared_old = old_definition->is_shared();
            auto is_shared_new = (*new_definition)->is_shared();
            if (is_shared_new != is_shared_old) {
                error = tr("Cannot change shared type of \"%0\" from \"%1\" to \"%2\"")
                    .arg(old_definition->type_name)                    
                    .arg(is_shared_old)
                    .arg(is_shared_new);
                return false;
            }

            if (is_shared_new) {
                // make sure shared label includes our index
                auto index = label->GetSharedLabelIndex();
                auto shared_count_old = int(old_definition->shared_labels.size());
                auto shared_count_new = int((*new_definition)->shared_labels.size());
                if (index >= shared_count_new) {
                    error = tr("Cannot reduce shared_count type of \"%0\" from \"%1\" to \"%2\"")
                        .arg(old_definition->type_name)
                        .arg(shared_count_old)
                        .arg(shared_count_new);
                    return false;
                }
                else {
                    auto proxy = dynamic_cast<ProxyLabel*>(label.get());
                    if (!proxy) {
                        error = tr("Intenal error, cannot get cast to ProxyLabel label of \"%0\"")
                            .arg(old_definition->type_name);
                        return false;
                    }
                    (*new_definition)->shared_labels[index] = proxy->GetProxyClient();
                    assignments[proxy->GetProxyClient().get()] = (*new_definition)->categories_list[0];
                }
            }

            assignments[label.get()] = (*new_definition)->GetCategory(old_category->value);
        }
    }

    // apply new labels
    for (auto a : assignments) {
        a.first->SetCategory(a.second.get());
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
    set_image_script(ArrayToString(definitions[K_IMAGE_SCRIPT]));
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

    set_label_definitions(std::make_shared<LabelDefinitionsTreeModel>(
        this, LoadLabelDefinitions(definitions[K_MARKER_TYPES].toObject())));
    connect(get_label_definitions().get(), &LabelDefinitionsTreeModel::Changed, this, &ApplicationModel::SetModified);

    bool file_content_ok = true;

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
                file_content_ok = false;
                continue;
            }

            if (!definition->categories_list.size()) {
                errors << tr("Definition \"%0\" has no categories").arg(definition->type_name);
                file_content_ok = false;
                continue;
            }

            if (!definition->GetCategory(category)) {
                errors << tr("Definition \"%0\" has no category \"%1\"")
                    .arg(definition->type_name)
                    .arg(category);
                file_content_ok = false;
                continue;
            }

            auto shared_index = marker[K_MARKER_SHARED_INDEX].toInt(0);
            if (definition->is_shared() && shared_index >= int(definition->shared_labels.size())) {
                errors << tr("Definition \"%0\" shared count is less than label shared index \"%1\"")
                    .arg(definition->type_name)
                    .arg(shared_index);
                file_content_ok = false;
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
                        .arg(definition->type_name);
                    file_content_ok = false;
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
            label->SetCategory(definition->GetCategory(category).get());
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

    return file_content_ok;
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

    if (image_script_.length()) {
        header.insert(K_IMAGE_SCRIPT, ToJsonArray(image_script_));
    }

    QJsonObject types;
	if (label_definitions_) {
		for (auto def : label_definitions_->GetDefinitions()) {
			QJsonObject json;
			json.insert(K_DEFINITION_VALUE_TYPE, QJsonValue::fromVariant(LabelTypeToString(def->value_type)));

			if (def->line_width != LabelDefinition::default_line_width) {
				json.insert(K_DEFINITION_LINE_WIDTH, QJsonValue::fromVariant(def->line_width));
			}

			if (!def->get_description().isEmpty()) {
				json.insert(K_DEFINITION_DESCRIPTION, QJsonValue::fromVariant(def->get_description()));
			}

			QJsonArray categories;

			for (auto c : def->categories_list) {
				QJsonObject json;
				json.insert(K_CATEGORY_NAME, c->name);
				json.insert(K_CATEGORY_ID, c->value);
				json.insert(K_CATEGORY_COLOR, c->color.name());

				categories.push_back(json);
			}

			json.insert(K_DEFINITION_CATEGORIES, categories);

            if (def->get_rendering_script().size()) {
                json.insert(K_DEFINITION_RENDERING_SCRIPT, ToJsonArray(def->get_rendering_script()));
            }

            if (def->is_stamp) {
                json.insert(K_DEFINITION_IS_STAMP, QJsonValue::fromVariant(true));
            }

            if (def->shared_labels.size() == 1) {
                json.insert(K_DEFINITION_SHARED, QJsonValue::fromVariant(true));
            }
            else if (def->shared_labels.size() > 1) {
                json.insert(K_DEFINITION_SHARED_COUNT, QJsonValue::fromVariant(int(def->shared_labels.size())));
            }

			if (def->filename_filter.size()) {
				QJsonArray filename_filters_array;
				for (const auto &i : def->filename_filter)
					filename_filters_array.push_back(QJsonValue::fromVariant(QString::fromStdString(i)));
				json.insert(K_DEFINITION_FILENAME_FILTER, filename_filters_array);
			}

			if (def->custom_properties.size()) {
				QJsonObject custom_properties;
				for (auto p : def->custom_properties) {
					QJsonObject jp;
					jp.insert(K_CUSTOM_PROP_TYPE, CustomPropertyTypeToString(p.type));
					if (p.default_value.isValid()) {
						jp.insert(K_CUSTOM_PROP_DEFAULT_VALUE, QJsonValue::fromVariant(p.default_value));
					}
					if (p.cases.size()) {
						jp.insert(K_CUSTOM_PROP_CASES, ToJsonArray(p.cases));
					}
					custom_properties.insert(p.id, jp);
				}
				json.insert(K_DEFINITION_CUSTOM_PROPERTIES, custom_properties);
			}

            if (!def->stamp_parameters.isEmpty()) {
                json.insert(K_DEFINITION_STAMP_PARAMETERS, def->stamp_parameters);
            }

			if (def->shared_properties.size()) {
				QJsonObject shared_properties;
				for (auto p : def->shared_properties) {
					if (p.second->IsIdentity()) {
						shared_properties[QString::fromStdString(p.first)] = QString::fromStdString(p.second->name);
					}
					else {
						QJsonObject jprop;
						jprop.insert(K_DEFINITION_SHARED_PROPERTY_NAME, QString::fromStdString(p.second->name));
						jprop.insert(K_DEFINITION_SHARED_PROPERTY_LINEAR_TRANSFORM_A, QJsonValue::fromVariant(p.second->a));
						if (p.second->b != 0) {
							jprop.insert(K_DEFINITION_SHARED_PROPERTY_LINEAR_TRANSFORM_B, QJsonValue::fromVariant(p.second->b));
						}
						shared_properties[QString::fromStdString(p.first)] = jprop;
					}
				}
				json.insert(K_DEFINITION_SHARED_PROPERTIES, shared_properties);
			}

			if (!def->axis_length.empty()) {
				QJsonArray axis_len;
                for (const auto &i : def->axis_length) {
                    axis_len.push_back(QJsonValue::fromVariant(i));
                }
				json.insert(K_DEFINITION_AXIS_LENGTH, axis_len);
			}

			types.insert(def->type_name, json);
		}
	}
    header.insert(K_MARKER_TYPES, types);
    return header;
}

void ApplicationModel::UpdateSharedProperties() {
    for (auto i : file_models_) {
        for (auto l : i.second->labels_) {
            l->UpdateSharedProperties();
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
            marker.insert(K_MARKER_CATEGORY, QJsonValue::fromVariant(label->GetCategory()->value));
            marker.insert(K_MARKER_TYPE_NAME, QJsonValue::fromVariant(label->GetCategory()->definition->type_name));

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

void ApplicationModel::set_image_script(QString value) {
    if (image_script_ != value) {
        image_script_ = value;
        image_script_changed(value);
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

    std::set<QString> subdirs;
    std::vector<FileTreeItemInfo> result;
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
                else {
                    // remove leading "/"
                    file_id_rest.remove(0, 1);
                }
            }

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

void ApplicationModel::OnFileModifiedChanged(bool value) {
    if (value) {
        set_is_modified(true);
    }
}

std::set<int> ApplicationModel::GetExistingSharedIndexes(LabelDefinition *def) {
    std::set<int> result;
    for (auto i : file_models_) {
        auto file_set = i.second->GetExistingSharedIndexes(def);
        result.insert(file_set.begin(), file_set.end());
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
    if (!files_loader_.empty()) {
        auto converter_type = files_loader_["type"].toString();
        if (converter_type == "rest_converter") {
            return std::make_shared<RestImageConverter>(files_loader_["params"].toObject());
        }
    }
    return {};
}

void ApplicationModel::Delete(LabelDefinition* marker, bool delete_only_instances) {
    for (auto file : file_models_) {
        file.second->Delete(marker, nullptr);
    }

    if (delete_only_instances) {
        return;
    }

    label_definitions_->Delete(marker);
}

void ApplicationModel::Delete(LabelCategory* category, bool delete_only_instances) {
    for (auto file : file_models_) {
        file.second->Delete(nullptr, category);
    }

    if (delete_only_instances) {
        return;
    }

    label_definitions_->Delete(category);
}
