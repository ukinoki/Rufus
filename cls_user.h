#ifndef CLS_USER_H
#define CLS_USER_H

#include <QMap>
#include <QObject>
#include <QVariant>
#include <QDate>
#include <QJsonObject>

class User : public QObject
{
    Q_OBJECT
public: //static
    static User create();
    static QMap<QString,QVariant> CalculAge(QDate datedenaissance);
    static QMap<QString,QVariant> CalculAge(QDate datedenaissance, QDate datedujour);
    static QMap<QString,QVariant> CalculAge(QDate datedenaissance, QString Sexe, QDate datedujour = QDate::currentDate());

private:
    QString m_login;
    QString m_password;

    int m_id;
    QString m_firstName;
    QString m_lastName;

public:
    explicit User(QObject *parent = nullptr);
    explicit User(QString login, QString password, QJsonObject data = {}, QObject *parent = nullptr);

    int id() const;
signals:

public slots:
};

#endif // CLS_USER_H
