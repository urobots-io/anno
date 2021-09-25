#include "implement_q_property.h"
#include <QObject>
#include <QString>
#include <memory>

class LabelDefinition;

class LabelCategory : public QObject {
    Q_OBJECT

protected:
    friend class LabelDefinition;

    LabelCategory(std::shared_ptr<LabelDefinition> definition, int value, const QString & name, const QColor & color);

public:
    Q_PROPERTY(int value READ get_value WRITE set_value NOTIFY value_changed);
    Q_PROPERTY(QString name READ get_name WRITE set_name NOTIFY name_changed);
    Q_PROPERTY(QColor color READ get_color WRITE set_color NOTIFY color_changed);

    std::shared_ptr<LabelDefinition> GetDefinition() const { return definition_.lock(); };

    static QColor GetStandardColor(int index);

    IMPLEMENT_Q_PROPERTY_WRITE(int, value);
    IMPLEMENT_Q_PROPERTY_WRITE(QString, name);
    IMPLEMENT_Q_PROPERTY_WRITE(QColor, color);

signals:
    void value_changed(int);
    void name_changed(QString);
    void color_changed(QColor);

private:
    /// parent definition
    std::weak_ptr<LabelDefinition> definition_;

    /// id of the category
    int value_;

    /// human readable name
    QString name_;

    /// color
    QColor color_;

public:
    IMPLEMENT_Q_PROPERTY_READ(value);
    IMPLEMENT_Q_PROPERTY_READ(name);
    IMPLEMENT_Q_PROPERTY_READ(color);
};
