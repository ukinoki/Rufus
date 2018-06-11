#ifndef DataBase_H
#define DataBase_H

#include <QJsonObject>
#include <QObject>
#include <QSettings>
#include <QSqlDatabase>

#include "cls_user.h"

class DataBase : public QObject
{
    Q_OBJECT
private:
    DataBase();
    static DataBase *instance;

    User m_userConnected;


    QString m_mode;
        enum m_mode { Poste, ReseauLocal, Distant };
    QString m_base;
    QString m_server;
    int m_port;
    bool m_useSSL;

    QMap<QString, QSqlDatabase> *m_mapDB;


public:
    static DataBase *getInstance();

    void init(QSettings const &setting, int mode);

    QString getMode() const;
    QString getBase() const;
    QString getServer() const;
    QSqlDatabase getDataBase(QString basename) const;

    QString getInformations();
    User getUser() const;



    QString connectToDataBase(QString basename, QString login, QString password);
    QJsonObject login(QString login, QString password);

    bool RestaureBase();



signals:

public slots:
};

#endif // DataBase_H
