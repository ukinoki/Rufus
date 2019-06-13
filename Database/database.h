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
#include "cls_lignepaiement.h"
#include "cls_motif.h"
#include "cls_paiementtiers.h"
#include "cls_patient.h"
#include "cls_patientencours.h"
#include "cls_recette.h"
#include "cls_tiers.h"
#include "cls_user.h"
#include "cls_userconnecte.h"
#include "cls_site.h"
#include "cls_ville.h"
#include "cls_compte.h"
#include "cls_parametressysteme.h"

#include "log.h"
#include "utils.h"

class DataBase : public QObject
{
    Q_OBJECT
public:
    enum m_mode { Poste, ReseauLocal, Distant };
    int comparateur;
    enum comparateur { Egal = 0x0, Inf = 0x1, Sup = 0x2 };
    enum typecorrespondant {MG, Spe1, Spe2};                    //! les 3 types de correspondants MG = médecin traitant, Spe1 ou Spe2 = tout autre type de correpondant

private:
    DataBase();
    static DataBase *instance;

    User *m_userConnected = Q_NULLPTR;
    ParametresSysteme *m_parametres = Q_NULLPTR;


    int m_mode;
    QString m_base;
    QString m_server;
    int m_port;
    bool m_useSSL;
    bool ok;

    QSqlDatabase m_db;

public:
    static DataBase*        I();

    /*
     * SQL
    */
    //     ACTION SUR LES PARAMETRES DE CONNEXION A LA BASE ------------------------------------------------------------------------------------------------------------------------------------------------------------------
    void                    init(QSettings const &setting, int mode);
                                                                /*! initialise les paramètres de connexion au serveur (adresse, port, SSL)
                                                                 *  à partir des valeurs enregistrées dans dans rufus.ini
                                                                 *  en fonction du mode de connexion*/
    void                    initFromFirstConnexion(QString mode, QString Server, int Port, bool SSL);
                                                                /*! initialise les paramètres de connexion au serveur (adresse, port, SSL)
                                                                 *  à partir des valeurs sasies dans la fiche paramconnexion utilisée au premier lancement du programme*/

    int                     getMode() const;                    //!> renvoie le mode d'accès au serveur monoposte = Mono, reseau local = ReseauLocal, distant = Distant
    QString                 getBase() const;                    //!> renvoie le mode d'accès au serveur tel qu'il est inscrit dans le fichier rufus.ini - monoposte = BDD_POSTE, reseau local = BDD_LOCAL, distant = BDD_DISTANT
    QString                 getBaseFromInt( int mode ) const;   //!> renvoie le mode d'accès au serveur tel qu'il est inscrit dans le fichier rufus.ini - monoposte = BDD_POSTE, reseau local = BDD_LOCAL, distant = BDD_DISTANT, \param le mode d'accès
    QString                 getServer() const;                  //!> l'adresse SQL du serveur - localhost ou adresse IP ou DynDNS
    QSqlDatabase            getDataBase() const;
    void                    getInformations();                  //! renvoie les infos de connexions SQL : host, database, login, mdp
    User*                   getUserConnected() const;           //!> le user connecté sur ce poste
    void                    setUserConnected(User*);            /*! utilisé uniquement lors du premier démarrage pour définir le premier user
                                                                 * normalement effectuéé par DataBase::login() mais pas possible dans ce cas
                                                                 * parce que login() utilise la table des utilisateurs connectés qui n'a pas encore été remplie à ce stade */



    QString                 connectToDataBase(QString basename, QString login, QString password);   //!> idem
    QDateTime               ServerDateTime();                   /*! renvoie le QDateTime du serveur
                                                                 * - utilisé pour le remplissage de certains champs pour éviter une erreur de date
                                                                 * au cas où le poste client ne serait pas correctement mis à l'heure */

    //     TRANSACTIONS, VERROUS ------------------------------------------------------------------------------------------------------------------------------------------------------------------
    bool                    createtransaction(QStringList ListTables, QString ModeBlocage = "WRITE");   //!> crée une transaction SQL
    void                    commit();                                                                   //!> commit de la transaction
    void                    rollback();                                                                 //!> rollback de la transaction

    bool                    locktables(QStringList ListTables, QString ModeBlocage = "WRITE");          //!> comme son nom l'indique
    void                    unlocktables();                                                             //!> comme son nom l'indique
    bool                    testconnexionbase();                                                        //!> comme son nom l'indique

    //     REQUETES ------------------------------------------------------------------------------------------------------------------------------------------------------------------
    bool                    erreurRequete(QSqlError type, QString requete, QString ErrorMessage = "");
                                                                //!> comme son nom l'indique
    int                     selectMaxFromTable(QString nomchamp, QString nomtable, bool &ok, QString errormsg="");
                                                                //!> la valeur maximale d'un champ int
    bool                    SupprRecordFromTable(int id, QString nomChamp, QString nomtable, QString errormsg="");
                                                                //!> supprime l' enregistrement  correspondant à l'id, dans le champ nomchamp, dans la table nomtable
    QList<QVariantList>     SelectRecordsFromTable(QStringList listselectChamp, QString nomtable, bool &OK, QString where = "", QString orderby="", bool distinct=false, QString errormsg="");
                                                                /*! renvoie sous forme de QList<QVariantList> les select SQL des champs listselectchamp de la table nomtable
                                                                *  avec la clause where et les modulations order by et distinct
                                                                * affiche le message errormsg en cas de pb */
    bool                    UpdateTable(QString nomtable, QHash<QString, QString> hash, QString where, QString errormsg="");
                                                                /*! Update la table nomtable
                                                                * le hash énumère les couples nomchamp, valeur à écrire
                                                                *  avec la clause where
                                                                * affiche le message errormsg en cas de pb */
    bool                    InsertIntoTable(QString nomtable,  QHash<QString, QString>, QString errormsg="");
                                                                /*! Insertion dans la table nomtable
                                                                * le hash énumère les couples nomchamp, valeur à écrire
                                                                *  avec la clause where
                                                                * affiche le message errormsg en cas de pb */
    bool                    InsertSQLByBinds(QString nomtable,  QHash<QString, QVariant>, QString errormsg="");
                                                                /*! Insertion dans la table nomtable en utilisant la fonction bindvalue de Qt
                                                                * - obligatoire pour insérer un QByteArray - ça ne fonctionne pas sinon
                                                                * le hash énumère les couples nomchamp, valeur à écrire
                                                                *  avec la clause where
                                                                * affiche le message errormsg en cas de pb */
    bool                    StandardSQL(QString req , QString errormsg="");
                                                                //!> éxécute la requête req et affiche le message d'erreur errormsg en cas d'échec
    QList<QVariantList>     StandardSelectSQL(QString req, bool &ok, QString errormsg="");
                                                                /*! éxécute le SELECT req et affiche le message d'erreur errormsg en cas d'échec
                                                                * renvoie la réponse sous forme de QList<QVariantList>
                                                                * la variable ok sert à pointer les erreurs sur requête pour les différencier des réponses vides */
    QVariantList            getFirstRecordFromStandardSelectSQL(QString req, bool &ok, QString errormsg="");
                                                                /*! renvoie la première réponse de la requête SQL SELECT req et affiche le message d'erreur errormsg en cas d'échec
                                                                * renvoie la réponse sous forme de QVariantList
                                                                * la variable ok sert à pointer les erreurs sur requête pour les différencier des réponses vides */
    /*
     * Parametres système
    */
    void initParametres();                           //! charge les parametres susteme
    ParametresSysteme*      parametres();                               //! renvoie les parametres système
    void setmdpadmin(QString mdp = "");
    void setnumcentre(int id);
    void setidlieupardefaut(int id);
    void setdocscomprimes(bool one);
    void setversionbase(int version);
    void setaveccompta(bool one);
    void setadresseserveurlocal(QString  adress = "");
    void setadresseserveurdistant(QString adress = "");
    void setadresseserveurtcp(QString adress = "");
    void setporttcp(int port);
    void setdirimagerie(QString adress = "");
    void setlundibkup(bool one);
    void setmardibkup(bool one);
    void setmercredibkup(bool one);
    void setjeudibkup(bool one);
    void setvendredibkup(bool one);
    void setsamedibkup(bool one);
    void setdimanchebkup(bool one);
    void setheurebkup(QTime time = QTime());
    void setdirbkup(QString adress = "");

    /*
     * Users
    */
    QJsonObject             login(QString login, QString password);     /*! connecte à la base mYSQL SQL avec le login login et le password password
                                                                        * crée l'utilisateur en cours m_userconnected  et complète tous les renseignements concernant cet utilisateur
                                                                        * renvoie un QJsonObject contenant les id d la réussite ou l'échec de la connection */
    QList<User*>            loadUsers();                                //! charge tous les utilisateurs Rufus référencés dans la table Utilisateurs avec des renseignements succincts
    QJsonObject             loadUserData(int idUser);                   //! complète tous les renseignements concernant l'utilisateur défini par l'id sauf la liste des comptes
    QJsonObject             loadAdminData();                            //! complète tous les renseignements concernant l'utilisateur admin

    /*
     * Users connectés
    */
    QList<UserConnecte*>    loadUsersConnectes();                                           //! charge tous les couples poste-utilisateurs connectés à la base
    QJsonObject             loadUserConnecteData(int idUser, QString macadress);            //! complète tous les renseignements concernant le couple poste-utilisateur défini par l'id

    /*
     * Correspondants
    */
    QList<Correspondant *>  loadCorrespondants();                       //! charge tous les correspondants référencés dans la table correspondants avec des renseignements succincts
    QList<Correspondant*>   loadCorrespondantsALL();                    //! charge tous les correspondants référencés dans la table correspondants avec tous les renseignements
    QJsonObject             loadCorrespondantData(int idcor);           //! complète tous les renseignements concernant le correspondant défini par l'id
    void                    SupprCorrespondant(int idcor);              //! supprime un correspondant défini par l'id

    /*
     * DocsExternes
    */
    QList<DocExterne*>      loadDoscExternesByPatient(Patient *pat);    //! charge tous les documents externes d'un patient référencés dans la table impressionss avec des renseignements succincts
    QJsonObject             loadDocExterneData(int idDoc);              //! complète tous les renseignements concernant le document externe défini par l'id
    void                    SupprDocExterne(DocExterne* doc);           //! supprime un document externe défini par l'id

    /*
     * Documents émis
    */
    QList<Document*>        loadDocuments();                            //! charge tous les documents imprimables de la  table courriers
    void                    SupprDocument(Document *doc);               //! supprime un document défini par l'id

    /*
     * MetaDocuments
    */
    QList<MetaDocument*>    loadMetaDocuments();                        //! charge tous les dossiers de documents imprimables de la  table courriers
    void                    SupprMetaDocument(Document *doc);           //! supprime un dossier de document défini par l'id

    /*
     * Compta
    */
    QList<Compte*>          loadComptesAll();                           //! charge tous les comptes bancaires sans exception
    QJsonObject             loadCompteById(int id);                     //! charge les datas d'un compte bancaire défini par son id
    QList<Depense*>         loadDepensesByUser(int idUser);             //! charge toutes les dépenses d'un utilisateur à partir de la table depenses
    void                    loadDepenseArchivee(Depense *dep);          //! charge tous renseignements sur une dépense archivée
    QStringList             ListeRubriquesFiscales();                   //! charge la liste de toutes les rubriques fiscales à partir de la table rubriques2035
    QList<Depense*>         VerifExistDepense(QMap<int, Depense *> m_listDepenses, QDate date, QString objet, double montant, int iduser, enum comparateur = Egal);
                                                                        //! comme son nom l'indique
    int                     getIdMaxTableComptesTableArchives();        //! comme son nom l'indique
    QList<Archive*>         loadArchiveByDate(QDate date, Compte *compte, int intervalle);
                                                                        //! charge les archives contenues entre 6 mois avant date et date pour le compte donné
    QList<Banque*>          loadBanques();                              //! charge toutes les banques ç partir de la  table banques
    QList<Tiers*>           loadTiersPayants();                         //! charge tous les organismes de tiers payants de la table tiers
    QList<TypeTiers*>       loadTypesTiers();                           //! charge tous les types de tiers payants (AME, CMU, AT...) à partir de la table rufus.listetiers

    QList<Recette*>         loadRecettesByDate(QDate datedebut, QDate datefin);
                                                                        //! charge toutes les recettes pour la période spécifiée

    QList<PaiementTiers *>  loadPaiementTiersByUser(User *usr);    //! charge tous les paiements par tiers pour un utilisateur pour la période spécifiée

    QList<LignePaiement*>   loadlignespaiementsByPatient(Patient *pat); //!> charge toutes les lignes de paiements des actes d'un patient

    /*
     * Cotations
    */
    QStringList             loadTypesCotations();                       //! charge toutes les cotations possibles à partir des tables cotations et ccam
    QList<Cotation*>        loadCotationsByUser(int iduser);            //! charge toutes les cotations utilisées par un user à partir des tables cotations et ccam

    /*
     * Motifs
    */
    QList<Motif*>           loadMotifs();                               //! charge tous les motifs d'actes possibles à partir de la table motifsRDV

    /*
     * Sites
    */
private:
    QList<Site*>            loadSites(QString req);                 //! charge les sites de travail à partir de la requete req
public:
    QList<Site*>            loadSitesByUser(int idUser);            //! élabore la requête qui charge tous les sites de travail utilisés par un user
    QList<Site*>            loadSitesAll();                         //! élabore la requête qui charge tous les sites de travail sans exception

public:
    /*
     * Villes
    */
    QList<Ville *>          loadVilles();                                //! charge toutes les villes et leur code postal à partir de la table villes

    /*
     * PatientsEnCours r
    */
    PatientEnCours*             loadPatientEnCoursById(int idPat);                                          //! charge toutes les données d'un patient défini par son id - utilisé pour renouveler les données en cas de modification
    QJsonObject                 loadPatientEnCoursData(QVariantList patdata);                               //! crée le QJsonObject des data d'un patient à partir des résultats de la requête en BDD
    QJsonObject                 loadPatientEnCoursDataById(int idPat);                                   //! charge toutes les données d'un patient défini par son id - utilisé pour renouveler les données en cas de modification
    QList<PatientEnCours *>     loadPatientsenCoursAll();                                                   /*! charge la liste de tous les patients à partir de la table salledattente*/

    /*
     * Patients
    */
    QJsonObject             loadPatientAllData(int idPat);                                                  //! charge toutes les données d'un patient défini par son id - utilisé pour renouveler les données en cas de modification
    void                    loadSocialDataPatient(QJsonObject &jData, bool &ok);                            //! charge les donnéess sociales d'un patient à partir de la table donneessocialespatients
    void                    loadMedicalDataPatient(QJsonObject &jData, bool &ok);                           //! charge les donnéess médicales d'un patient à partir de la table renseignementsmedicauxpatients
    Patient*                loadPatientById(int idPat, Patient *pat = Q_NULLPTR, bool all = false);         //! charge un patient par son id à partir de la table patients

    qint64                  countPatientsAll(QString nom = "", QString prenom = "");                        /*! compte le nombre de patients
                                                                                                            * \param patnom filtrer sur le nom de patient
                                                                                                            * \param patprenom filtrer sur le prénom de patient */

    QList<Patient*>         loadPatientsAll(QString nom = "", QString prenom = "", bool filtre = false);    /*! charge la liste de tous les patients à partir de la table patients
                                                                                                            * \param patnom filtrer sur le nom de patient
                                                                                                            * \param patprenom filtrer sur le prénom de patient
                                                                                                            * \param le filtre se fait sur des valeurs aprrochantes */

    QList<Patient *>        loadPatientsByDDN(QDate DDN);                                                   /*! charge la liste de tous les patients pour une date de naissance
                                                                                                             * \param DDN la date de naissance */

    /*
     * Mots de passe
    */
    //Pas normal, les mots de passes doivent etre chiffrés
    QString                 getMDPAdmin();                                      //! retrouve le mdp de l'administratuer

    /*
     * Actes
    */
private:
    QJsonObject             loadActeData(QVariantList actdata);                 //! attribue le liste des datas à un acte
public:
    Acte*                   loadActeById(int idActe);                           //! charge un Acte à partir de son id
    QJsonObject             loadActeAllData(int idActe);                        //! charge toutes les données d'un acte défini par son id - utilisé pour renouveler les données en cas de modification
    QList<Acte*>            loadActesByPat(Patient *pat);                       //! chrage les actes d'un patient
    QList<Acte*>            loadIdActesByPat(Patient *pat);                     //! chrage les actes d'un patient en ne retenant que les id
    double                  getActePaye(int idActe);                            //! retrouve le total des paiements pour un acte

};

#endif // DataBase_H
