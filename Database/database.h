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
#include <QHostInfo>
#include <QObject>
#include <QSettings>
#include <QSqlDatabase>

#include "cls_acte.h"
#include "cls_patient.h"
#include "cls_user.h"
#include "cls_correspondant.h"
#include "cls_depense.h"
#include "cls_villes.h"
#include "log.h"
#include "utils.h"

class DataBase : public QObject
{
    Q_OBJECT
public:
    enum m_mode { Poste, ReseauLocal, Distant };
    enum m_comparateur { Egal = 0x0, Inf = 0x1, Sup = 0x2 };
    Q_DECLARE_FLAGS(Comparateurs, m_comparateur)

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
    //enum gComparateur {SUP, INF, EGAL};

    static DataBase *getInstance();

    void init(QSettings const &setting, int mode);
    void initFromFirstConnexion(QString mode, QString Server, int Port, bool SSL);

    int getMode() const;
    QString getBase() const;
    QString getBaseFromInt( int mode ) const;
    QString getServer() const;
    QSqlDatabase getDataBase() const;
    void getInformations();
    User* getUserConnected() const;

    bool traiteErreurRequete(QSqlQuery query, QString requete, QString ErrorMessage = "");

    QString connectToDataBase(QString basename, QString login, QString password);

    void    commit();
    void    rollback();
    bool    locktables(QStringList ListTables, QString ModeBlocage = "WRITE");
    bool    testconnexionbase();
    int     selectMaxFromTable(QString nomchamp, QString nomtable);
    void    SupprRecordFromTable(int id, QString nomChamp, QString nomtable);
    QList<QList<QVariant>> SelectRecordsFromTable(QStringList listselectChamp, QString nomtable, QString where = "", QString order="", bool distinct=false);
    void    UpdateTable(QString nomtable, QHash<QString, QString>, QString where);
    void    InsertIntoTable(QString nomtable,  QHash<QString, QString>);

    /*
     * Users
    */
    QJsonObject login(QString login, QString password);
    QJsonObject loadUserData(int idUser);
    QList<User*> loadUsersAll();

    /*
     * Correspondants
    */
    QList<Correspondant*> loadCorrespondants();
    void    SupprCorrespondant(int idcor);
    QList<Correspondant*> loadCorrespondantsALL();

    /*
     * Compta
    */
    QList<Compte*>  loadComptesByUser(int idUser);
    QList<Depense*> loadDepensesByUser(int idUser);
    void            loadDepenseArchivee(Depense *dep);
    QStringList     ListeRubriquesFiscales();
    QList<Depense*> VerifExistDepense(QMap<int, Depense*> m_listDepenses, QDate date, QString objet, double montant, int iduser, Comparateurs Comp = Egal);
    int             getMaxLigneBanque();

    /*
     * Sites
    */
    QList<Site*> loadSitesByUser(int idUser);
    QList<Site*> loadSitesAll();
private:
    QList<Site*> loadSites(QString req);

public:
    /*
     * Villes
    */
    Villes* loadVillesAll();

    /*
     * Gestion des Patients
    */
    QList<Patient*> loadPatientAll();
    Patient*        loadPatientById(int idPat);


    /*
     * Actes
    */
private:
    QString createActeRequest(int idActe, int idPat);
    QJsonObject extractActeData(QSqlQuery query);
public:
    Acte* loadActeById(int idActe);
    QMap<int, Acte*> loadActesByIdPat(int idPat);
    double getActeMontant(int idActe);


signals:

public slots:
};

#endif // DataBase_H
