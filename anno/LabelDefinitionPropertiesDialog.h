#ifndef LABELDEFINITIONPROPERTIESDIALOG_H
#define LABELDEFINITIONPROPERTIESDIALOG_H

#include "implement_q_property.h"
#include "LabelDefinitionsTreeModel.h"
#include "ui_LabelDefinitionPropertiesDialog.h"
#include <QDialog>
#include <memory>

class LDProperties : public QObject {
    Q_OBJECT

public:
    LDProperties(QObject* parent) : QObject(parent) {}

    enum LabelType {
        circle = 0,
        oriented_circle,
        oriented_point,
        oriented_rect,
        point,
        polygon,
        polyline,
        rect,
        tool
    };

    Q_ENUM(LabelType)

    Q_PROPERTY(QString type_name READ get_type_name WRITE set_type_name NOTIFY type_name_changed);
    Q_PROPERTY(QString description READ get_description WRITE set_description NOTIFY description_changed);
    Q_PROPERTY(int line_width READ get_line_width WRITE set_line_width NOTIFY line_width_changed);
    Q_PROPERTY(LabelType value_type READ get_value_type WRITE set_value_type NOTIFY value_type_changed);

signals:
    void type_name_changed(QString);
    void description_changed(QString);
    void line_width_changed(int);
    void value_type_changed(LabelType);

public slots:
    IMPLEMENT_Q_PROPERTY_WRITE(QString, type_name);
    IMPLEMENT_Q_PROPERTY_WRITE(QString, description);
    IMPLEMENT_Q_PROPERTY_WRITE(int, line_width);
    IMPLEMENT_Q_PROPERTY_WRITE(LabelType, value_type);
    
private:
    QString type_name_;
    QString description_;
    int line_width_;
    LabelType value_type_;

public:
    IMPLEMENT_Q_PROPERTY_READ(type_name);
    IMPLEMENT_Q_PROPERTY_READ(description);
    IMPLEMENT_Q_PROPERTY_READ(line_width);
    IMPLEMENT_Q_PROPERTY_READ(value_type);
};

class LabelDefinitionPropertiesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LabelDefinitionPropertiesDialog(std::shared_ptr<LabelDefinition>, std::shared_ptr<LabelDefinitionsTreeModel>, QWidget *parent = nullptr);
    ~LabelDefinitionPropertiesDialog();

private slots:
    void onSharedLabelsCountChanged(int);
    void ApplyAndClose();

private:
    Ui::LabelDefinitionPropertiesDialog ui;
    std::shared_ptr<LabelDefinition> definition_;
    std::shared_ptr<LabelDefinitionsTreeModel> definitions_;

    LDProperties *properties_ = nullptr;
};

#endif // LABELDEFINITIONPROPERTIESDIALOG_H
