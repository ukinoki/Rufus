#ifndef CLS_ITEM_H
#define CLS_ITEM_H

#include <QJsonObject>
#include <QObject>
#include <QVariant>
#include <QDate>

class Item : public QObject
{
    Q_OBJECT
public:
    explicit Item(QObject *parent = nullptr);

protected:
    void setStringProperty(QJsonObject data, QList<QString> props);
    void setDataString(QJsonObject data, QString key);

    void setIntProperty(QJsonObject data, QList<QString> props);
    void setDataInt(QJsonObject data, QString key);

    void setDataDouble(QJsonObject data, QString key);

    void setBoolProperty(QJsonObject data, QList<QString> props);
    void setDataBool(QJsonObject data, QString key);

    void setDataDateTime(QJsonObject data, QString key);

private:

signals:

public slots:
};

#endif // CLS_ITEM_H
