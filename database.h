#ifndef DataBase_H
#define DataBase_H

/**
* \file DataBase.h
* \brief Cette classe gére l'ensemble des requetes SQL
* \author Alexanre.D
* \version 0.1
* \date 6 juin 2018
*
*
*/

#include <QJsonObject>
#include <QObject>
#include <QSettings>
#include <QSqlDatabase>

#include "cls_acte.h"
#include "cls_user.h"
#include "cls_villes.h"
#include "log.h"

class DataBase : public QObject
{
    Q_OBJECT
public:
    enum m_mode { Poste, ReseauLocal, Distant };
private:
    DataBase();
    static DataBase *instance;

    User *m_userConnected = nullptr;


    int m_mode;
    QString m_base;
    QString m_server;
    int m_port;
    bool m_useSSL;

    QSqlDatabase m_db;


public:
    static DataBase *getInstance();

    void init(QSettings const &setting, int mode);

    int getMode() const;
    QString getBase() const;
    QString getBaseFromInt( int mode ) const;
    QString getServer() const;
    QSqlDatabase getDataBase() const;
    void getInformations();
    User* getUserConnected() const;
    void setUserConnected(User *user);

    bool traiteErreurRequete(QSqlQuery query, QString requete, QString ErrorMessage = "");

    QString connectToDataBase(QString basename, QString login, QString password);

    QJsonObject login(QString login, QString password);
    QJsonObject loadUser(int idUser);

    /*
     * Sites
    */
    QList<Site*> loadUserSites(int idUser);
    QList<Site*> loadAllSites();
private:
    QList<Site*> loadSites(QString req);

public:
    /*
     * Villes
    */
    Villes* loadAllVilles();

    /*
     * Actes
    */
    Acte* loadActeById(int idActe);
    QMap<int, Acte*> loadActeByIdPat(int idPat);



signals:

public slots:
};

#endif // DataBase_H
