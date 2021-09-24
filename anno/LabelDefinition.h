/// this is a definition of the label type in the project
#pragma once
#include "CustomProperty.h"
#include "LabelType.h"
#include "SharedPropertyDefinition.h"
#include <QColor>
#include <QJsonObject>
#include <QObject>
#include <QString>
#include <memory>
#include <set>

class FileModel;
class Label;
class LabelDefinition;
class LabelProperty;

class LabelCategory : public QObject {
public:
    /// parent definition
    LabelDefinition *definition;

    /// id of the category
	int value;

    /// human readable name
	QString name;
	
    /// color
	QColor color;

    static QColor GetStandardColor(int index);
};

class LabelDefinition : public QObject
{
    Q_OBJECT

public:
    LabelDefinition(LabelType type, QObject *parent = nullptr);
    ~LabelDefinition();

    /// type name (key in the file)
    QString type_name;

    /// value type
    const LabelType value_type;

    /// categories
	std::vector<std::shared_ptr<LabelCategory>> categories;

    /// > 0 - width in picture pixels
    /// < 0 - width in screen pixels
    static const int default_line_width = -3;
	int line_width = default_line_width;

    /// Coordinate axis length, using for OrientedPointLabel.
    /// If value is 0 axis handle will not be rendered.
    std::vector<int>axis_length;

    /// creation using stamps
    bool is_stamp = false;

    /// stamp parameters
    QJsonObject stamp_parameters;
    
    /// shared labels, if any
    std::vector<std::shared_ptr<Label>> shared_labels;

    /// regex which defines if label is allowed on the image with given name
    /// for shared labels, each label uses filter with corresponding index  
    std::vector<std::string> filename_filter;

    /// shared properties
    std::map<std::string, std::shared_ptr<SharedPropertyDefinition>> shared_properties;

    /// custom label properties
    std::vector<CustomPropertyDefinition> custom_properties;
	
    /// custom source code to render the label
    QString get_rendering_script() const { return rendering_script_; }
    void set_rendering_script(const QString);

    /// text for user
    QString get_description() const { return description_; }
    void set_description(const QString);

    bool is_shared() const { return shared_labels.size() > 0; }

    void ConnectProperty(LabelProperty&, const std::string & name, bool inject_my_value) const;
    double GetSharedPropertyValue(const std::string & name, double default_value) const;

    std::set<int> GetMissingIndexes(const std::set<int>& existing_indexes) const;

    bool AllowedForFile(FileModel*, int shared_index = -1) const;

    std::shared_ptr<LabelCategory> GetCategory(int value) const;

signals:
    void Changed();
    
private:
    LabelDefinition(const LabelDefinition &);
    LabelDefinition & operator = (const LabelDefinition&);

    bool AllowedForFilename(QString filename, int shared_index = -1) const;

private:    
    QString rendering_script_;
    QString description_;
};

