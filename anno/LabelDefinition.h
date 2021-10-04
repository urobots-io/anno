/// this is a definition of the label type in the project
#pragma once
#include "CustomProperty.h"
#include "implement_q_property.h"
#include "LabelCategory.h"
#include "LabelType.h"
#include "SharedPropertyDefinition.h"
#include <QColor>
#include <QJsonObject>
#include <QObject>
#include <memory>
#include <set>

class FileModel;
class Label;
class LabelProperty;

class LabelDefinition : public QObject
{
    Q_OBJECT

public:
    LabelDefinition(LabelType type, QObject *parent = nullptr);
    ~LabelDefinition();

    static std::shared_ptr<LabelCategory> CreateCategory(std::shared_ptr<LabelDefinition>, int value, const QString & name, const QColor & color);

    /// type name (key in the file)
    Q_PROPERTY(QString type_name READ get_type_name WRITE set_type_name NOTIFY type_name_changed);
    /// description for humans
    Q_PROPERTY(QString description READ get_description WRITE set_description NOTIFY description_changed);
    /// custom rendering script
    Q_PROPERTY(QString rendering_script READ get_rendering_script WRITE set_rendering_script NOTIFY rendering_script_changed);
    /// rendering line width
    Q_PROPERTY(int line_width READ get_line_width WRITE set_line_width NOTIFY line_width_changed);
    /// creation using stamps
    Q_PROPERTY(bool is_stamp READ get_is_stamp WRITE set_is_stamp NOTIFY is_stamp_changed);

    /// value type
    const LabelType value_type;

    /// categories
	std::vector<std::shared_ptr<LabelCategory>> categories;

    /// > 0 - width in picture pixels
    /// < 0 - width in screen pixels
    static const int default_line_width = -3;

    /// Coordinate axis length, using for OrientedPointLabel.
    /// If value is 0 axis handle will not be rendered.
    std::vector<int> axis_length;

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
	
    bool is_shared() const { return shared_labels.size() > 0; }

    void ConnectProperty(LabelProperty&, const std::string & name, bool inject_my_value) const;
    double GetSharedPropertyValue(const std::string & name, double default_value) const;

    std::set<int> GetMissingIndexes(const std::set<int>& existing_indexes) const;

    bool AllowedForFile(FileModel*, int shared_index = -1) const;

    std::shared_ptr<LabelCategory> GetCategory(int value) const;    

public slots:    
    IMPLEMENT_Q_PROPERTY_WRITE(QString, description);
    IMPLEMENT_Q_PROPERTY_WRITE(bool, is_stamp);
    IMPLEMENT_Q_PROPERTY_WRITE(int, line_width);
    IMPLEMENT_Q_PROPERTY_WRITE(QString, rendering_script);
    IMPLEMENT_Q_PROPERTY_WRITE(QString, type_name);

signals:
    void Changed();
    void description_changed(QString);
    void is_stamp_changed(bool);
    void line_width_changed(int);
    void rendering_script_changed(QString);
    void type_name_changed(QString);
    
private:
    LabelDefinition(const LabelDefinition &);
    LabelDefinition & operator = (const LabelDefinition&);

    bool AllowedForFilename(QString filename, int shared_index = -1) const;

private:    
    QString description_;
    bool is_stamp_ = false;
    int line_width_ = default_line_width;
    QString rendering_script_;
    QString type_name_;

public:
    IMPLEMENT_Q_PROPERTY_READ(description);
    IMPLEMENT_Q_PROPERTY_READ(is_stamp);
    IMPLEMENT_Q_PROPERTY_READ(line_width);
    IMPLEMENT_Q_PROPERTY_READ(rendering_script);
    IMPLEMENT_Q_PROPERTY_READ(type_name);
};

