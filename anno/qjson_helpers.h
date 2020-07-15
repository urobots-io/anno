#pragma once 
#include <QString>
#include <QJsonDocument>

QStringList ToStringList(const QJsonValue & value);
QString ArrayToString(const QJsonValue & value);
QJsonArray ToJsonArray(const QStringList & value);
QJsonArray ToJsonArray(const QString & value);

QJsonDocument LoadJsonFromText(const QByteArray& json_content, QString & error);