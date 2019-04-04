/* (C) 2018 LAINE SERGE
This file is part of RufusAdmin or Rufus.

RufusAdmin and Rufus are free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License,
or any later version.

RufusAdmin and Rufus are distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with RufusAdmin and Rufus.  If not, see <http://www.gnu.org/licenses/>.
*/

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
#include <QSqlRecord>

#include "cls_acte.h"
#include "cls_archivebanque.h"
#include "cls_banque.h"
#include "cls_correspondant.h"
#include "cls_cotation.h"
#include "cls_depense.h"
#include "cls_docexterne.h"
#include "cls_document.h"
#include "cls_motif.h"
#include "cls_patient.h"
#include "cls_tiers.h"
#include "cls_user.h"
#include "cls_villes.h"

#include "log.h"
#include "utils.h"

class DataBase : public QObject
{
    Q_OBJECT
public:
    enum m_mode { Poste, ReseauLocal, Distant };
    int comparateur;
    enum comparateur { Egal = 0x0, Inf = 0x1, Sup = 0x2 };

private:
    DataBase();
    static DataBase *instance;

    User *m_userConnected = nullptr;


    int m_mode;
    QString m_base;
    QString m_server;
    int m_port;
    bool m_useSSL;
    bool ok;

    QSqlDatabase m_db;

public:
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
    void setUserConnected(User*);           // utilisé uniquement lors du premier démarrage pour définir le premier user
                                            // normalement effectuéé par DataBase::login() mais pas possible dans ce cas
                                            // parce que login() utilise la table des utilisateurs connectés qui n'a pas encore été remplie à ce stade

    bool                    erreurRequete(QSqlError type, QString requete, QString ErrorMessage = "");

    QString connectToDataBase(QString basename, QString login, QString password);

    bool                    createtransaction(QStringList ListTables, QString ModeBlocage = "WRITE");
    void                    commit();
    void                    rollback();
    bool                    locktables(QStringList ListTables, QString ModeBlocage = "WRITE");
    void                    unlocktables();
    bool                    testconnexionbase();
    int                     selectMaxFromTable(QString nomchamp, QString nomtable, bool &ok, QString errormsg="");
    bool                    SupprRecordFromTable(int id, QString nomChamp, QString nomtable, QString errormsg="");
    QList<QVariantList>     SelectRecordsFromTable(QStringList listselectChamp, QString nomtable, bool &OK, QString where = "", QString orderby="", bool distinct=false, QString errormsg="");
    bool                    UpdateTable(QString nomtable, QHash<QString, QString>, QString where, QString errormsg="");
    bool                    InsertIntoTable(QString nomtable,  QHash<QString, QString>, QString errormsg="");
    bool                    InsertSQLByBinds(QString nomtable,  QHash<QString, QVariant>, QString errormsg="");
    bool                    StandardSQL(QString req , QString errormsg="");
    QList<QVariantList>     StandardSelectSQL(QString req, bool &ok, QString errormsg="");  // la variable ok sert à pointer les erreurs sur requête pour les différencier des réponses vides
    QVariantList           getFirstRecordFromStandardSelectSQL(QString req, bool &ok, QString errormsg="");  // la variable ok sert à pointer les erreurs sur requête pour les différencier des réponses vides
    /*
     * Users
    */
    QJsonObject login(QString login, QString password);
    QJsonObject loadUserData(int idUser);
    QJsonObject loadUserDatabyLogin(QString login);
    QList<User*> loadUsersAll();

    /*
     * Correspondants
    */
    QList<Correspondant *> loadCorrespondants();
    void    SupprCorrespondant(int idcor);
    QList<Correspondant*> loadCorrespondantsALL();

    /*
     * DocsExternes
    */
    QList<DocExterne*> loadDoscExternesByPatientAll(int idpatient);
    QJsonObject loadDocExterneData(int idDoc);
    void    SupprDocExterne(DocExterne* doc);

    /*
     * Documents émis
    */
    QList<Document*> loadDocuments();
    QJsonObject loadDocumentData(int idDoc);
    void    SupprDocument(Document *doc);

    /*
     * MetaDocuments
    */
    QList<MetaDocument*> loadMetaDocuments();
    QJsonObject loadMetaDocumentData(int idDoc);
    void    SupprMetaDocument(Document *doc);

    /*
     * Compta
    */
    QList<Compte*>  loadComptesAllUsers();
    QList<Compte*>  loadComptesByUser(int idUser);
    QList<Depense*> loadDepensesByUser(int idUser);
    void            loadDepenseArchivee(Depense *dep);
    QStringList     ListeRubriquesFiscales();
    QList<Depense*> VerifExistDepense(QMap<int, Depense *> m_listDepenses, QDate date, QString objet, double montant, int iduser, enum comparateur = Egal);
    int             getMaxLigneBanque();
    QList<Archive*> loadArchiveByDate(QDate date, Compte *compte, int intervalle); //! charge les archives contenues entre 6 mois avant date et date pour le compte donné
    QList<Banque*>  loadBanques();
    QList<Tiers*>   loadTiersPayants();
    QList<TypeTiers*>  loadTypesTiers();

    /*
     * Cotations
    */
    QStringList loadTypesCotations();
    QList<Cotation*> loadCotationsByUser(int iduser);

    /*
     * Motifs
    */
    QList<Motif*> loadMotifs();

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
     * Mots de passe
    */
    //Pas normal, les mots de passes doivent etre chiffrés
    QString getMDPAdmin();

    /*
     * Actes
    */
private:
    QString createActeRequest(int idActe, int idPat);
    QJsonObject extractActeData(QVariantList actdata);
public:
    Acte* loadActeById(int idActe);
    QMap<int, Acte*> loadActesByIdPat(int idPat);
    double getActeMontant(int idActe);


signals:

public slots:
};

#endif // DataBase_H
