/* (C) 2020 LAINE SERGE
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

#include <QBuffer>
#include <QJsonObject>
#include <QHostInfo>
#include <QObject>
#include <QSettings>
#include <QSqlDatabase>
#include <QSqlRecord>
#include <QJsonDocument>

#include "cls_acte.h"
#include "cls_archivebanque.h"
#include "cls_banque.h"
#include "cls_commentlunet.h"
#include "cls_commercial.h"
#include "cls_correspondant.h"
#include "cls_cotation.h"
#include "cls_depense.h"
#include "cls_docexterne.h"
#include "cls_impression.h"
#include "cls_intervention.h"
#include "cls_iol.h"
#include "cls_lignepaiement.h"
#include "cls_manufacturer.h"
#include "cls_message.h"
#include "cls_motcle.h"
#include "cls_motif.h"
#include "cls_paiementtiers.h"
#include "cls_patient.h"
#include "cls_patientencours.h"
#include "cls_recette.h"
#include "cls_tiers.h"
#include "cls_user.h"
#include "cls_posteconnecte.h"
#include "cls_recettecomptable.h"
#include "cls_refraction.h"
#include "cls_session.h"
#include "cls_site.h"
#include "cls_ville.h"
#include "cls_compte.h"
#include "cls_lignecompte.h"
#include "cls_parametressysteme.h"
#include "cls_donneesophtapatient.h"

#include "log.h"
#include "utils.h"

class DataBase : public QObject
{
    Q_OBJECT
public:
    enum comparateur { Egal = 0x0, Inf = 0x1, Sup = 0x2 };  Q_ENUM(comparateur)
    enum QueryResult { Error, Empty, OK}; Q_ENUM(QueryResult)

private:
    DataBase();
    static DataBase *instance;

    int m_iduserConnected = 0;
    ParametresSysteme *m_parametres = Q_NULLPTR;
    DonneesOphtaPatient *m_donneesophtapatient = new DonneesOphtaPatient();

    Utils::ModeAcces m_modeacces;
    QString m_base;
    QString m_server;
    int m_port = 3306;
    bool ok;
    QSqlDatabase m_db = QSqlDatabase();

public:
    static DataBase*        I();

    /*
     * SQL
    */
    //     ACTION SUR LES PARAMETRES DE CONNEXION A LA BASE ------------------------------------------------------------------------------------------------------------------------------------------------------------------
    void                    initParametresConnexionSQL(
                                QString Server,
                                int Port);                      /*! initialise les paramètres de connexion au serveur (adresse, port, SSL) */

    Utils::ModeAcces        ModeAccesDataBase() const;          /*! le mode d'accès au serveur
                                                                    \result monoposte = Utils::Poste, reseau local = Utils::ReseauLocal, distant = Utils::Distant */
    void                    setModeacces(const Utils::ModeAcces &modeacces);
    QString                 AdresseServer() const;              /*! l'adresse SQL du serveur - localhost ou adresse IP ou DynDNS */
    int                     port() const;                       /*! le port SQL */
    void                    InfosConnexionSQL();                /*! les infos de connexions SQL : host, database, login, mdp */
    int                     idUserConnected() const             { return m_iduserConnected; }


    QString                 connectToDataBase(QString basename, QString login = LOGIN_SQL, QString password = MDP_SQL);   //!> idem
    QDateTime               ServerDateTime();                   /*! renvoie le QDateTime du serveur
                                                                 * - utilisé pour le remplissage de certains champs pour éviter une erreur de date
                                                                 * au cas où le poste client ne serait pas correctement mis à l'heure */

    //     TRANSACTIONS, VERROUS ------------------------------------------------------------------------------------------------------------------------------------------------------------------
    bool                    createtransaction(QStringList ListTables, QString ModeBlocage = "WRITE");   //!> crée une transaction SQL
    void                    commit();                                                                   //!> commit de la transaction
    void                    rollback();                                                                 //!> rollback de la transaction

    bool                    locktables(QStringList ListTables, QString ModeBlocage = "WRITE");          //!> comme son nom l'indique
    bool                    locktable(QString NomTable, QString ModeBlocage = "WRITE");                 //!> comme son nom l'indique
    void                    unlocktables();                                                             //!> comme son nom l'indique

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
    bool                    UpdateTable(QString nomtable, QHash<QString, QVariant> sets, QString where, QString errormsg="");
                                                                /*! Update la table nomtable
                                                                * le hash énumère les couples nomchamp, valeur à écrire
                                                                *  avec la clause where
                                                                * affiche le message errormsg en cas de pb */
    bool                    InsertIntoTable(QString nomtable,  QHash<QString, QString>, QString errormsg="");
                                                                /*! Insertion dans la table nomtable
                                                                * le hash énumère les couples nomchamp, valeur à écrire
                                                                *  avec la clause where
                                                                * affiche le message errormsg en cas de pb */
    bool                    InsertSQLByBinds(QString nomtable,  QHash<QString, QVariant> sets, QString errormsg="");
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
    void                    VideDatabases();                    /*! vide la base de données */
    /*
     * Parametres système
    */
    void initParametresSysteme();                               //! charge les parametres susteme
    ParametresSysteme*      parametres();                       //! renvoie les parametres système
    void setmdpadmin(QString mdp);
    void setnumcentre(int id);
    void setidlieupardefaut(int id);
    void setdocscomprimes(bool one);
    void setversionbase(int version);
    void setsanscompta(bool one);
    void setadresseserveurlocal(QString  adress = "");
    void setadresseserveurdistant(QString adress = "");
    void setporttcp(int port);
    void setdirimagerie(QString adress = "");
    void setdaysbkup(Utils::Days days);
    void setheurebkup(QTime time = QTime());
    void setdirbkup(QString adress = "");

    /*
     * Donnees ophta patient
    */
    void initDonnesOphtaPatient(int idpat);                     //! charge les données optha d'un patient
    DonneesOphtaPatient* donneesOphtaPatient();
    /*
     * Users
    */
    void                    setidUserConnected(int id)                  { m_iduserConnected = id; }
    QueryResult             verifExistUser(QString login, QString password);        /*! verifie que le login-mdp existe dans la base */
    QueryResult             calcidUserConnected(QString login, QString password);   /*! retrouve l'id correspondant au mdp_login, modifie au besoin le mdp en sha1(mdp) */
    QList<User*>            loadUsers();                                            //! charge tous les utilisateurs Rufus référencés dans la table Utilisateurs avec des renseignements complets
    QList<User*>            loadUsersShortListe();                                  //! charge tous les utilisateurs Rufus référencés dans la table Utilisateurs avec des renseignements succincts
    QJsonObject             loadUserData(int idUser);                               //! complète tous les renseignements concernant l'utilisateur défini par l'id sauf la liste des comptes
    QJsonObject             loadAdminData();                                        //! complète tous les renseignements concernant l'utilisateur admin

    /*
     * Postes connectés
    */
    QList<PosteConnecte*>   loadPostesConnectes();                                              //! charge tous les postes connectés à la base
    QJsonObject             loadPosteConnecteData(int idUser, QString macadress);               //! complète tous les renseignements concernant le poste défini par l'id

    /*
     * Correspondants
    */
    QList<Correspondant *>  loadCorrespondants();                       //! charge tous les correspondants référencés dans la table correspondants avec des renseignements succincts
    QList<Correspondant*>   loadCorrespondantsALL();                    //! charge tous les correspondants référencés dans la table correspondants avec tous les renseignements
    QJsonObject             loadCorrespondantData(int idcor);           //! complète tous les renseignements concernant le correspondant défini par l'id

    /*
     * DocsExternes
    */
    QList<DocExterne*>      loadDoscExternesByPatient(Patient *pat);    //! charge tous les documents externes d'un patient référencés dans la table impressionss avec des renseignements succincts
    QJsonObject             loadDocExterneData(int idDoc);              //! complète tous les renseignements concernant le document externe défini par l'id

    /*
     * Impressions
    */
private:
    QJsonObject                 loadImpressionData(QVariantList Impressiondata);    //! attribue la liste des datas à une impression
public:
    QList<Impression*>          loadImpressions();                                  //! charge tous les documents imprimables de la  table courriers
    Impression*                 loadImpressionById(int id);                         //! charge un document d'imprssion par son id

    /*
     * Dossiers d'impressions
    */
private:
    QJsonObject                 loadDossierImpressionData(QVariantList dossierdata);    //! attribue la liste des datas à un dossier de documents imprimables
public:
    QList<DossierImpression*>   loadDossiersImpressions();                              //! charge tous les dossiers de documents imprimables de la  table courriers
    DossierImpression*          loadDossierImpressionById(int id);                      //! charge un dossier de documents imprimables à partir de son id

    /*
     * Compta
    */
private:
    QJsonObject             loadCompteData(QVariantList listdata);                  //! attribue le liste des datas à un compte
public:
    QList<Compte*>          loadComptesAll();                                       //! charge tous les comptes bancaires sans exception
    Compte*                 loadCompteById(int id);                                 //! chargeun compte bancaire défini par son id

    QList<LigneCompte*>     loadLignesComptesByCompte(int idcompte);                //! charge tous les lignes de comptes bancaires pas encore archivées
    QJsonObject             loadLigneCompteDataById(int id);                        //! charge les datas d'une ligne de compte bancaire défini par son id

    QList<Depense*>         loadDepensesByUser(int idUser);                         //! charge toutes les dépenses d'un utilisateur à partir de la table depenses
    void                    loadDepenseArchivee(Depense *dep);                      //! charge tous renseignements sur une dépense archivée
    QStringList             ListeRubriquesFiscales();                               //! charge la liste de toutes les rubriques fiscales à partir de la table rubriques2035
    QList<Depense*>         VerifExistDepense(QMap<int, Depense *> m_listDepenses, QDate date, QString objet, double montant, int iduser, enum comparateur = Egal);
                                                                                    //! comme son nom l'indique
    int                     GetidRubriqueFiscale(Depense *dep);                     //! retrouve l'id de la rubrique fisclae d'une dépense

    int                     getIdMaxTableComptesTableArchives();                    //! comme son nom l'indique
    QList<Archive*>         loadArchiveByDate(QDate date, Compte *compte, int intervalle);
                                                                                    //! charge les archives contenues entre 6 mois avant date et date pour le compte donné
    QList<Banque*>          loadBanques();                                          //! charge toutes les banques ç partir de la  table banques
    QList<Tiers*>           loadTiersPayants();                                     //! charge tous les organismes de tiers payants de la table tiers
    QList<TypeTiers*>       loadTypesTiers();                                       //! charge tous les types de tiers payants (AME, CMU, AT...) à partir de la table rufus.listetiers

    QList<Recette*>         loadRecettesByPeriod(QDate datedebut, QDate datefin);   //! charge toutes les recettes pour la période spécifiée
    RecetteComptable*       loadRecetteComptablebyId(int id);                       //! charge une recette comptable par son id

    QList<PaiementTiers *>  loadPaiementTiersByUser(User *usr);                     //! charge tous les paiements par tiers pour un utilisateur pour la période spécifiée

    QList<LignePaiement*>   loadlignespaiementsByPatient(Patient *pat);             //!> charge toutes les lignes de paiements des actes d'un patient

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
    QJsonObject             loadSiteData(QVariantList sitdata);
public:
    QList<Site*>            loadSites();                                //! charge tous les sites de travail
    Site*                   loadSiteById(int id);                       //! charge un sites de travail à partir son id
    QList<int>              loadidSitesByUser(int idUser);              //! élabore la requête qui charge tous les sites de travail utilisés par un user

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
    QJsonObject                 loadPatientEnCoursDataById(int idPat);                                      //! charge toutes les données d'un patient défini par son id - utilisé pour renouveler les données en cas de modification
    QList<PatientEnCours *>     loadPatientsenCoursAll();                                                   /*! charge la liste de tous les patients à partir de la table salledattente*/

/*
 * Patients
 */
    QJsonObject             loadPatientAllData(int idPat);                                                  //! charge toutes les données d'un patient défini par son id - utilisé pour renouveler les données en cas de modification
    void                    loadSocialDataPatient(QJsonObject &jData, bool &ok);                            //! charge les donnéess sociales d'un patient à partir de la table donneessocialespatients
    void                    loadMedicalDataPatient(QJsonObject &jData, bool &ok);                           //! charge les donnéess médicales d'un patient à partir de la table renseignementsmedicauxpatients
    Patient*                loadPatientById(int idPat, Patient *pat = Q_NULLPTR,
                                            Item::LOADDETAILS details = Item::NoLoadDetails);               //! charge un patient par son id à partir de la table patients

    qint64                  countPatientsAll(QString nom = "", QString prenom = "");                        /*! compte le nombre de patients
                                                                                                            * \param patnom filtrer sur le nom de patient
                                                                                                            * \param patprenom filtrer sur le prénom de patient */

    QList<Patient*>         loadPatientsAll(QString nom = "", QString prenom = "", bool filtre = false);    /*! charge la liste de tous les patients à partir de la table patients
                                                                                                            * \param patnom filtrer sur le nom de patient
                                                                                                            * \param patprenom filtrer sur le prénom de patient
                                                                                                            * \param le filtre se fait sur des valeurs aprrochantes */

    QList<Patient*>         loadPatientsByListId(QList<int> listid);                                        /*! charge la liste des patients à partir d'e la table patients'une liste d'id
                                                                                                            * \param QList<int> la liste des id */

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
    int                     getidActeCorrespondant(int idpat, QDate date);      //! recherche s'il existe un acte correspondant à cet idpatient et à cette date

    /*
     * Refractions
    */
private:
    QJsonObject             loadRefractionData(QVariantList refdata);           //! attribue le liste des datas à une refraction
public:
    QList<Refraction*>      loadRefractionsByPatId(int id);                     //! charge toutes les refractions d'un patient
    Refraction*             loadRefractionById(int idref);                      //! charge une refraction définie par son id - utilisé pour renouveler les données en cas de modification

    /*
     * Commentaires lunettes
    */
private:
    QJsonObject             loadCommentLunetData(QVariantList refdata);         //! attribue le liste des datas à un commentaire lunette
public:
    QList<CommentLunet*>    loadCommentsLunetsByListidUser(QList<int> listid);  //! charge tous les commentaires lunettes
    CommentLunet*           loadCommentLunetById(int idref);                    //! charge un commentaire défini par son id - utilisé pour renouveler les données en cas de modification

    /*
     * Sessions opératoires
    */
private:
    QJsonObject             loadSessionOpData(QVariantList refdata);            //! attribue le liste des datas à une session
public:
    QList<SessionOperatoire *> loadSessionsOpByUserId(int id);                  //! charge toutes les sessions opératoires d'un user
    SessionOperatoire*      loadSessionOpById(int idintervention, SessionOperatoire *session = Q_NULLPTR);              //! charge une session définie par son id - utilisé pour renouveler les données en cas de modification

    /*
     * Interventions
    */
private:
    QJsonObject             loadInterventionData(QVariantList refdata);                 //! attribue le liste des datas à une Intervention
public:
    QList<Intervention*>    loadInterventionsBySessionId(int id);                       //! charge toutes les Interventions d'une session
    QList<QPair<int, int> > loadIdInterventionsByPatientId(int id);                     //! charge tous les id d'interventions d'un patient
    Intervention*           loadInterventionById(int idintervention, Intervention* interv = Q_NULLPTR);                   //! charge une Intervention définie par son id - utilisé pour renouveler les données en cas de modification
    Intervention*           loadInterventionByDateIdPatient(QDate date, int idpatient); //! charge une Intervention définie par sa sate et l'id du patient

    /*
     * IOLs
    */
private:
    QJsonObject             loadIOLData(QVariantList refdata);                      //! attribue la liste des datas à un IOL
public:
    QList<IOL*>             loadIOLs();                                             //! charge tous les IOLs
    QList<IOL*>             loadIOLsByManufacturerId(int id);                       //! charge tous les IOLs d'un fabricant
    IOL*                    loadIOLById(int idintervention);                        //! charge les datas d'un IOL défini par son id - utilisé pour renouveler les données en cas de modification
    void                    UpDateIOL(int id, QHash<QString, QVariant> sets);

    /*
     * TypeInterventions
    */
private:
    QJsonObject                 loadTypeInterventionData(QVariantList typeinterventiondata);    //! attribue la liste des datas à un TypeIntervention
public:
    QList<TypeIntervention*>    loadTypeInterventions();                                        //! charge tous les TypeInterventionS
    TypeIntervention*           loadTypeInterventionById(int idtypeintervention);               //! charge les datas d'un TypeIntervention défini par son id - utilisé pour renouveler les données en cas de modification

    /*
     * Manufacturers
    */
private:
    QJsonObject             loadManufacturerData(QVariantList Mandata);             //! attribue le liste des datas à un fabricant
public:
    QList<Manufacturer*>    loadManufacturers();                                    //! charge toutes les fabricants
    Manufacturer*           loadManufacturerById(int idManufacturer);               //! charge un fabricant défini par son id - utilisé pour renouveler les données en cas de modification

    /*
     * Commerciaux
    */
private:
    QJsonObject             loadCommercialData(QVariantList Mandata);               //! attribue le liste des datas à un commercial
public:
    QList<Commercial*>      loadCommercials();                                      //! charge tous les commerciaux
    Commercial*             loadCommercialById(int idCommercial);                   //! charge un commercial défini par son id - utilisé pour renouveler les données en cas de modification
    QList<Commercial*>      loadCommercialsByIdManufacturer(int idmanufacturer);     //! charge tous les commerciaux d'un fabricant

    /*
     * MotsClés
    */
private:
    QJsonObject             loadMotCleData(QVariantList Motcledata);                //! attribue le liste des datas à un motcle
public:
    QList<MotCle*>          loadMotsCles();                                         //! charge tous les mots clés
    MotCle*                 loadMotCleById(int idMotcle);                           //! charge un motclé défini par son id - utilisé pour renouveler les données en cas de modification
    QList<int>              loadListIdMotsClesByPat(int idpat);                     //! chagre les id des mots clés utilisés par un patient

    /*
     * Messages
    */
private:
    QJsonObject             loadMessageData(QVariantList msgdata);                  //! attribue le liste des datas à un message
public:
    QList<Message*>         loadMessagesRecusByIdUser(int id);                      //! charge tous les messages reçus par un utilisateur
    QList<Message*>         loadMessagesEnvoyesByIdUser(int id);                    //! charge tous les messages envoyes par un utilisateur
    QList<Message*>         loadAllMessagesByIdUser(int id);                        //! charge tous les messages reçus ou envoyes par un utilisateur
    Message*                loadMessageById(int idMotcle);                          //! charge un motclé défini par son id - utilisé pour renouveler les données en cas de modification
//    QList<int>              loadListIdMotsClesByPat(int idpat);                     //! chagre les id des mots clés utilisés par un patient

    /*
     * Sessions
    */
private:
    QJsonObject             loadSessionData(QVariantList refdata);                  //! attribue la liste des datas d'une session
public:
    Session*                loadSessionById(int idsession);                         //! charge une session définie par son id - utilisé pour renouveler les données en cas de modification

};

#endif // DataBase_H
