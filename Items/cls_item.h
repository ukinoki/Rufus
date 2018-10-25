#ifndef CLS_ITEM_H
#define CLS_ITEM_H

#include <QJsonObject>
#include <QObject>
#include <QVariant>
#include <QDate>

class Item : public QObject
{
    Q_OBJECT
public: //STATIC
    static QMap<QString,QVariant> CalculAge(QDate datedenaissance);
    static QMap<QString,QVariant> CalculAge(QDate datedenaissance, QDate datedujour);
    static QMap<QString,QVariant> CalculAge(QDate datedenaissance, QString Sexe, QDate datedujour = QDate::currentDate());


public:
    explicit Item(QObject *parent = nullptr);


protected:

    void setDataString(QJsonObject data, QString key, QString &prop, bool useTrim=false);
    void setDataInt(QJsonObject data, QString key, int &prop);
    void setDataDouble(QJsonObject data, QString key, double &prop);
    void setDataBool(QJsonObject data, QString key, bool &prop);
    void setDataDateTime(QJsonObject data, QString key, QDateTime &prop);
    void setDataDate(QJsonObject data, QString key, QDate &prop);

private:

signals:

public slots:
};

#endif // CLS_ITEM_H
