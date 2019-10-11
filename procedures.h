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

#ifndef PROCEDURES_H
#define PROCEDURES_H


#include <QCompleter>
#include <QFileDialog>
#include <QFontDialog>
#include <QDesktopWidget>
#include <QHostInfo>
#include <QInputDialog>
#include <QKeyEvent>
#include <QListWidget>
#include <QMenu>
#include <QNetworkInterface>
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPlainTextEdit>
#include <QPainter>
#include <QPrinter>
#include <QPrintPreviewDialog>
#include <QPrintDialog>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QSettings>
#include <QSound>
#include <QSplashScreen>
#include <QStandardItemModel>
#include <QtSql>
#include <QTableWidget>
#include <QTreeWidget>
#include <QTreeWidgetItem>

#include "uppushbutton.h"
#include "upcheckbox.h"
#include "textprinter.h"
#include "serialthread.h"
#include "dlg_paramconnexion.h"
#include "ui_dlg_paramconnexion.h"
#include "dlg_choixdate.h"
#include "ui_dlg_choixdate.h"
#include "dlg_gestionusers.h"
#include "ui_dlg_gestionusers.h"
#include "dlg_identificationuser.h"
#include "ui_dlg_identificationuser.h"
#include "dlg_message.h"
#include "uptextedit.h"
#include "uptoolbar.h"
#include "upmessagebox.h"

#include "database.h"
#include "gbl_datas.h"

class Procedures : public QObject
{
    Q_OBJECT

private:
    explicit                Procedures(QObject *parent = Q_NULLPTR);
    static Procedures*      instance;
    bool                    eventFilter(QObject *obj, QEvent *event)  ;
    QSettings               *m_settings;
    ParametresSysteme       *m_parametres;

public:
    static Procedures       *I();
    QSettings*              settings() const { return m_settings; }





/*! --------------------------------------------------------------------------------------------------------
* opérations sur la base de données, le système et les datas
* -------------------------------------------------------------------------------------------------------- */
private:
    DataBase                *db;
    Utils::ModeAcces        m_modeacces;
    bool                    m_connexionbaseOK;
    bool                    m_ok;
    qint64                  m_basesize, m_imagessize, m_videossize, m_facturessize, m_freespace;
    QString                 m_nomFichierIni;
    UpDialog                *dlg_buprestore;
    UpLabel                 *wdg_resumelbl, *wdg_volumelibrelbl, *wdg_inflabel;
    dlg_paramconnexion      *Dlg_ParamConnex;
    bool                    VerifBaseEtRessources();
    bool                    VerifIni(QString msg = "",                          //! Récupère ou reconstruit le fichier d'initialisaton Rufus.ini et/ou la base
                                     QString msgInfo = "",
                                     bool DetruitIni = true,
                                     bool RecupIni = true,
                                     bool ReconstruitIni = true,
                                     bool ReconstruitBase = true,
                                     bool PremDemarrage = false,
                                     bool BaseVierge = false);
    bool                    VerifParamConnexion(bool OKAccesDistant = true, QString nomtblutilisateurs = TBL_UTILISATEURS);
    bool                    VerifRessources(QString Nomfile = "");
    bool                    Verif_secure_file_priv();
    int                     VerifUserBase(QString Login, QString MDP);          //! Vérifie que l'utilisateur existe dans la base

public:
    bool                    AutresPostesConnectes(bool msg = true);
    bool                    FicheChoixConnexion();
    bool                    Connexion_A_La_Base();
    bool                    ConnexionBaseOK() const { return m_connexionbaseOK; }
    void                    ProgrammeSQLVideImagesTemp(QTime timebackup);   /*! programme l'effacement des données temporaires d'imagerie
                                                                             * vide la table EchangeImages
                                                                             * purge les champs jpg et pdf de la table Factures  */
private:
    User                    *m_currentuser = Q_NULLPTR; //user connected //TODO : DEPLACER DANS DATAS
    QString                 m_absolutepathDirStockageImage, m_pathDirStockageImagesServeur;
    QString                 m_CPpardefaut, m_Villepardefaut;
public:
    static QString          CodePostalParDefaut();
    static QString          VilleParDefaut();

    void                    setAbsolutePathDirImagerie();
    QString                 AbsolutePathDirImagerie();
    QString                 DirImagerieServeur();

/*! fin opérations sur la base de données, le système et les datas -------------------------------------------------------------------------------------------------------- */






/*! --------------------------------------------------------------------------------------------------------
* première connection
* -------------------------------------------------------------------------------------------------------- */
private:
    dlg_gestionusers        *Dlg_GestUsr;
    bool                    CreerPremierUser(QString Login, QString MDP);
    void                    CreerUserFactice(int idusr, QString login, QString mdp);
    bool                    PremierDemarrage();
    void                    PremierParametrageMateriel();
    void                    PremierParametrageRessources();

/*! fin première connection -------------------------------------------------------------------------------------------------------- */





/*! --------------------------------------------------------------------------------------------------------
* identification et definition des datas du user (superviseur, utilisateur qui enregistre la commpta et utilistaion de la compta)
* -------------------------------------------------------------------------------------------------------- */
private:
    bool                    m_aveccomptaprovisoire;
    bool                    m_initok;
    bool                    m_usecotation;
    int                     m_idcentre;
    bool                    IdentificationUser(bool ChgtUtilisateur = false);   /*! la fiche d'identification de l'utilisateur au lancement du programme
                                                                                 * suivie de l'initialisation de tout
                                                                                 * et de la définition du rôle de l'utilisateur */
    void                    CalcLieuExercice();
    void                    CalcUserParent();
    void                    CalcUserSuperviseur();
    bool                    DefinitRoleUser();                       /*! definit les iduser pour lequel le user travaille
                                                                        . iduser superviseur des actes                      (int gidUserSuperViseur)
                                                                            . lui-même s'il est responsable de ses actes
                                                                            . un autre user s'il est assistant
                                                                        . iduser qui enregistrera la comptabilité des actes (int gidUserComptable)
                                                                        . idUser soignant remplacé si le superviseur est remplaçant (int gidUserParent)
                                                                        . s'il cote les actes                            (bool gUseCotation)
                                                                        . s'il enregistre une compta                     (bool AvecLaComptaProv)
                                                                      */
public:
    QString                 MDPAdmin();
    QString                 SessionStatus();                                    /*! statut de l'utilisateur pour cette session */

    int                     idCentre();
    bool                    Init();
    void                    ReconstruitListeComptes (User *usr, QList<Compte*>* listcomptes);
    bool                    SetUserAllData(User* usr, Item::UPDATE upd = Item::Update);

/*! fin definition des datas du user (superviseur, utilisateur qui enregistre la commpta et utilistaion de la compta) -------------------------------------------------------------------------------------------------------- */




/*! --------------------------------------------------------------------------------------------------------
* les impressions
* -------------------------------------------------------------------------------------------------------- */
private:
    QString                 m_nomImprimante;
    QPrinter                *p_printer;
    QRectF                  m_rect;
    void                    Print(QPrinter*, QImage image);
    void                    PrintPdf(QPrinter*, Poppler::Document* document, bool &printok);

public:
    bool                    ApercuAvantImpression();                                                /*! les impressions passent par un aperçu avant d'être lancées */
    void                    setNomImprimante(QString NomImprimante);
    QString                 nomImprimante();

  /*! 1 - Impression avec textprinter.h */
    int                     TailleEnTete();
    int                     TailleEnTeteALD();
    int                     TaillePieddePage();
    int                     TaillePieddePageOrdoLunettes();
    int                     TailleTopMarge();
            /*! a - Impression d'un texte */
    QString                 CalcCorpsImpression(QString text, bool ALD = false);
    QMap<QString,QString>   CalcEnteteImpression(QDate date, User* user);
    QString                 CalcPiedImpression(User* user, bool lunettes = false, bool ALD = false);
    bool                    Imprime_Etat(QTextEdit *Etat, QString EnTete, QString Pied,
                                     int TaillePieddePage, int TailleEnTete, int TailleTopMarge,
                                     bool AvecDupli = false, bool AvecPrevisu = false, bool AvecNumPage = false,
                                     bool AvecChoixImprimante = true);
            /*! b - Création d'un pdf */
    bool                    Cree_pdf(QTextEdit *Etat, QString EnTete, QString Pied, QString nomfichier, QString nomdossier = "");

  /*! 2 - Impression directe d'un jpg ou d'un pdf sans utiliser textprinter.h */
    bool                    PrintDocument(QMap<QString, QVariant> doc);

/*! fin impressions -------------------------------------------------------------------------------------------------------- */





/*! --------------------------------------------------------------------------------------------------------
* l'import des documents
* -------------------------------------------------------------------------------------------------------- */
private:
    bool                    m_isposteimportdocs;                                /*! le poste est celui qui importe les documents */
public:
    bool                    isPosteImportDocs();
    QString                 pathDossierDocuments(QString Appareil, Utils::ModeAcces mod = Utils::ReseauLocal);
    QString                 PosteImportDocs();
    void                    setPosteImportDocs(bool a = true);

/*! fin import des documents -------------------------------------------------------------------------------------------------------- */





/*! --------------------------------------------------------------------------------------------------------
* en vrac
* -------------------------------------------------------------------------------------------------------- */
private:
    QFont                   m_applicationfont;
    QList<QImage>           m_listeimages;
    UpDialog                *dlg_askLogin, *dlg_askUser;
    UpTableWidget           *wdg_tablewidget;
    dlg_choixdate           *Dlg_ChxDate;

public:
    void                    ab(int i = 1);
    void                    CalcImage(Item *item, bool imagerie, bool afficher = true);
    QMap<QString, QDate>    ChoixDate(QWidget *parent=Q_NULLPTR);
    QString                 Edit(QString txt, QString titre = "", bool editable = true, bool ConnectAuSignal = false);
    void                    EditHtml(QString txt);
    void                    EditDocument(QMap<QString, QVariant> doc, QString label = "", QString titre = "", UpDialog::Buttons Button=UpDialog::ButtonOK);
    void                    EnChantier(bool avecMsg = false);
    void                    EpureLogs(int anciennete = 7);          //!> supprime les fichiers de logs antérieurs à J - anciennete jours
    void                    ModifTailleFont(QObject *obj, int siz, QFont font=qApp->font());
    static void             ReconstruitComboCorrespondants(QComboBox* box, Correspondants::TYPECORRESPONDANT type = Correspondants::TousLesCorrespondants);

signals:
    void                    UpdDocsExternes();
    void                    ModifEdit(QString txt);
    void                    ConnectTimers(bool connect);
    void                    DelImage();
    void                    CloseEditDocument();

/*! fin en vrac -------------------------------------------------------------------------------------------------------- */






/*! --------------------------------------------------------------------------------------------------------
* les sauvegardes
* -------------------------------------------------------------------------------------------------------- */

        /*! LA SAUVEGARDE DE LA BASE DE DONNEES

          La sauvegarde de la BDD peut-être planifiée dans le Qframe ui->Sauvegardeframe de la fiche dlg_param
          On peut planifier l'emplacement du fichier de sauvegarde, l'heure de la sauvegarde, et les jours de la sauvegarde.
          La sauvegarde ne peut se programmer que sur le serveur et pas ailleurs. Il faut donc installer une instance de Rufus sur le serveur.
          Les éléments du cadre ui->Sauvegardeframe sont donc désactivés si on n'est pas en mode Poste, autrement dit, sur le serveur.

          Les paramètres de programmation de la sauvegarde sont sauvegardés dans la base de données dans la table ParametresSyteme

          La sauvegarde se fait par le script RufusBackupScript.sh qui lance le prg mysqldump de sauvegarde des données et recopie les fichiers d'imagerie, les factures et les videos vers l'emplacement de sauvegarde.
          Ce script définit l'emplacement de la sauvegarde, le nom de la sauvegarde et détruit les sauvegardes datant de plus de 14 jours
          Le script RufusBackupScript.sh est situé dans le dossier /Users/nomdutilisateur/Documents/Rufus

          Le lancement de la sauvegarde au moment programmé se fait
            . Sous Mac,  par un autre script -> c'est le fichier xml rufus.bup.plist situé dans /Users/nomutilisateur/Library/LaunchAgents.
              Ce fichier est chargé au démarrage par le launchd Apple.
              Il est donc éxécuté même quand Rufus ne tourne pas
            . Sous Linux, c'est un timer t_timerbackup qui lance la sauvegarde et la fonction BackupWakeUp()

          Au chargement de Rufus, les données de Rufus.ini sont récupérées pour régler l'affichage des données dans  ui->Sauvegardeframe.

          Une modification de l'emplacement de sauvegarde se fait par un clic sur le bouton ui->DirBackuppushButton qui va lancer la fonction ModifDirBachup()
                * ce slot va créer le fichier RufusScriptBackup.sh et enregistrer l'emplacement de sauvegarde dans rufus.ini
          Le bouton ui->EffacePrgSauvupPushButton réinitialise la programmation en déclenchant la fonction EffaceAutoBackup():
                * annule les données de programmation dans rufus.ini,`
                * réinitialise l'affichage dans ui->Sauvegardeframe,`
                * supprime le script de sauvegarde RufusBackupScript.sh
                * sous Mac, supprime le script de programmation rufus.bup.plist et le décharge du launchd
                * sous Linux, arrête le timer t_timerbackup

          Au lancement du programme, depuis rufus.cpp,
                * les paramètres de sauvegarde automatique sont récupérés pour régler l'affichage des données dans  ui->Sauvegardeframe.
                * si le programme est utilisé sur le  serveur, et s'il y a une programmation valide des sauvegardes automatiques
                * la programmation de sauvegarde va lancer la fonction ParamAutoBackup()
                    * en recréant le fichier rufus.bup.plist sous MacOS
                    * en lançant le timer t_timerbackup sous Linux
                * dans le cas contraire, la fonction EffaceProgrammationBackup() efface le fichier rufus.bup.plist sous MacOS

          Si le programme s'éxécute sur le serveur le QFrame ui->Sauvegardeframe de dlg_param.cpp est enabled, pas dans le cas contraire

          Une modification de l'heure ou du jour du backup dans dlg_param.cpp
          lance la fonction ModifDateHeureBackup() qui va modifier les paramètres de backup en BDD
          puis lancer ParamAutoBackup()

          Une modification de jour de backup dans dlg.param.cpp
          lance la fonction ModifDateBackup() de dlg_param.cpp qui va modifier les paramètres de backup en BDD
          puis lancer ParamBackupAuto()

          Une modification du dossier de destination du backup dans dlg.param.cpp
          lance la fonction ModifDirBackup() qui va modifier les paramètres de backup en BDD
          puis lancer ParamAutoBackup()

          Le  bouton ui->ImmediatBackupupPushButton de dlg_param.cpp lance la fonction startImmediateBackup() qui va
            * vérifier qu'il n'y a pas d'autres postes connectés
            * créer les paramètres de la fonction ImmediateBackup() et la lancer

         La fonction ImmediateBackup() est lancée par
            * la fonction startImmediateBackup déclenchée par un click sur le bouton ui->ImmediatBackupupPushButton de dlg_param.cpp
            * une demande de réintialisation de la BDD
            * une mise à jour de la BDD

         La fonction DefinitScriptBackup() crée le fichier RufusScriptBackup.sh qui va éxécuter la sauvegarde.
         Elle est lancée par
            * ParamAutoBackup() sous Mac
            * Backup() utilisée pour un backup immédiat de la base (ImmediateBackup()) ou pour un backup auto sous Linux (BackupWakeUp())
         */

    public:
       void                    EffaceBDDDataBackup();
                                /*! efface le paramétrage de la sauvegarde (moment et emplacement) dans la base de données */
        void                    EffaceProgrammationBackup();
                                /*! efface la programmation de la sauvegarde qui a été créé sur le poste à partir du paramètrage enregistré dans la base de données
                                * n'efface pas le paramètrage de sauvegarde (moment et emplacement) dans la base de données
                                * suppression de rufus.bup.plist sous Mac et arrêt du timer t_timerbackup sous Linux
                                */
        bool                    ImmediateBackup(QString dirdestination = "", bool verifposteconnecte = true, bool full = false);
                                /*! lance un backup immédiat */
        void                    ParamAutoBackup();
                                /*! paramètre le moment et l'emplacement de la sauvegarde
                                 * sous Mac, crée le fichier xml rufus.bup.plist
                                 * sous Linux, lance le timer t_timerbackup
                                */
        bool                    RestaureBase(bool BaseVierge = false, bool PremierDemarrage = false, bool VerifPostesConnectes = true);
        bool                    ReinitBase();

    private:
        QTimer                  t_timerbackup;
        void                    AskBupRestore(bool restore, QString pathorigin, QString pathdestination, bool OKini = true, bool OKRessces = true, bool OKimages = true, bool OKvideos = true, bool OKfactures = true);
                                /*! fiche utilisée par ImmediateBackup ou DefinitScriptRestore() pour choisir ce qu'on va sauvegarder ou restaurer */
        bool                    Backup(QString dirSauv, bool OKBase, bool OKImages, bool OKVideos, bool OKFactures, bool isbkupauto);
                                /*! utilisée par ImmediateBackup() pour sauvegarder la base et/ou les fichiers d'imagerie suivant le choix fait dans AskBackupRestore()
                                * et par le timer t_timerbackup sous Linux pour effectuer une sauvegarde automatique et sans choix des options dans ce cas */
        void                    BackupWakeUp();
                                /*! sous Linux, déclenche le backup au moment programmé */
        qint64                  CalcBaseSize();
                                /*! calcule le volume de la base */
        void                    CalcTimeBupRestore();
                                /*! calcule la durée approximative du backup */
        void                    DefinitScriptBackup(QString NomDirDestination, bool AvecImages= true, bool AvecVideos = true);
                                /*! crée le script RufusScriptBackup.sh qui va éxécuter la sauvegarde */
        void                    DefinitScriptRestore(QStringList ListNomFiles);
                                /*! crée le script RufusScriptRestore.sh qui va éxécuter la restauration de la base MySQL */

    /*! fin sauvegardes -------------------------------------------------------------------------------------------------------- */




/*! ------------------------------------------------------------------------------------------------------------------------------------------
     GESTION DES PORTS SERIES -------------------------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------------------------------------------------*/
public:
    void                    setFicheRefractionOuverte(bool a);
    bool                    FicheRefractionOuverte();
private:
    bool                    m_dlgrefractionouverte;
    QString                 m_portAutoref, m_portFronto, m_portRefracteur, m_portTono;
    QSerialPort             *sp_portAutoref, *sp_portRefracteur, *sp_portTono, *sp_portFronto;
    struct Settings {
        qint32 baudRate;
        QSerialPort::DataBits dataBits;
        QSerialPort::Parity parity;
        QSerialPort::StopBits stopBits;
        QSerialPort::FlowControl flowControl;
    };
    Settings                s_paramPortSerieAutoref;
    Settings                s_paramPortSerieFronto;
    Settings                s_paramPortSerieRefracteur;
    SerialThread            *t_threadFronto;
    SerialThread            *t_threadRefracteur;
    SerialThread            *t_threadAutoref;
    bool                    ReglePortAutoref();
    bool                    ReglePortFronto();
    bool                    ReglePortRefracteur();

public:
    enum TypeMesure {
                None        = 0x0,
                All         = 0x1,
                Fronto      = 0x2,
                Autoref     = 0x4,
                Kerato      = 0x8,
                Subjectif   = 0x10,
                Final       = 0x20,
                Tono        = 0x40,
                Pachy       = 0x80
                };  Q_ENUM(TypeMesure)
    Q_DECLARE_FLAGS(TypesMesures, TypeMesure)
signals:
    void                    NouvMesureRefraction(TypeMesure);

public:
    QSerialPort*            PortAutoref();
    QSerialPort*            PortFronto();
    QSerialPort*            PortRefracteur();
    QSerialPort*            PortTono();
    void                    debugMesure(QObject *mesure, QString titre = "");
    //LE FRONTO ----------------------------------------------------
    QString                 HtmlFronto();                           // accesseur pour le html de mesure fronto à afficher;
    //L'AUTOREF ----------------------------------------------------
    QString                 HtmlAutoref();                          // accesseur pour le html de mesure fronto à afficher;
    QString                 HtmlKerato();                           // accesseur pour le html de mesure kerato à afficher;
    QString                 HtmlTono();                             // accesseur pour le html de mesure tonométrie à afficher;
    QString                 HtmlPachy();                            // accesseur pour le html de mesure pachy à afficher;
   //LE REFRACTEUR ------------------------------------------------
    QString                 HtmlRefracteur();                       // accesseur pour le html de mesure refracteur à afficher;
    void                    InsertRefraction(TypeMesure = All);     // enregistre la mesure de réfraction
    void                    EnvoiDataPatientAuRefracteur();
    static TypeMesure       ConvertMesure(QString Mesure);
    void                    setFlagReglageRefracteur(TypesMesures mesures)  { m_flagreglagerefracteur = mesures; }
    TypesMesures            FlagReglageRefracteur()                         { return m_flagreglagerefracteur; }
    static QString          ConvertMesure(Procedures::TypeMesure Mesure);

private:
    QString                 m_mesureSerie;
    TypeMesure              m_typemesureRefraction;                // le type de mesure effectuée: Fronto, Autoref ou Refracteur
    TypesMesures            m_flagreglagerefracteur = None;
    QString                 CalculeFormule(MesureRefraction *ref, QString Cote);
                                                                    //! calcule la forumle de réfraction à partir des data sphere, cylindre, axe, addVP
    void                    ClearMesures();
    void                    ClearHtmlMesures();
    bool                    Ouverture_Ports_Series();

    //LE FRONTO ----------------------------------------------------
    QString                 m_htmlMesureFronto;
    bool                    LectureDonneesFronto(QString Mesure);           // lit les données envoyées sur le port série du fronto
    void                    setHtmlFronto();                                // détermine le html à inscrire dans la fiche observation à partir du QMap MesureFronto
    bool                    m_isnewMesureFronto;                            // détermine si la mesure provient du fronto ou du dossier
    //L'AUTOREF ----------------------------------------------------
    QMap<QString,QVariant>  map_dataAEnvoyerAuRefracteur;
    QString                 m_htmlMesureAutoref;
    QString                 m_htmlMesureKerato;
    QString                 m_htmlMesureTono;
    QString                 m_htmlMesurePachy;
    bool                    LectureDonneesAutoref(QString Mesure);          // lit les données envoyées sur le port série du fronto
    void                    setHtmlAutoref();                               // détermine le html à inscrire dans la fiche observation à partir du QMap MesureAutoref
    void                    setHtmlKerato();                                // détermine le html à inscrire dans la fiche observation à partir du QMap MesureKerato
    void                    setHtmlTono();                                  // détermine le html à inscrire dans la fiche observation à partir du QMap MesureTono
    void                    setHtmlPachy();                                 // détermine le html à inscrire dans la fiche observation à partir du QMap MesurePachy
    bool                    m_isnewMesureAutoref;                           // détermine si la mesure d'autoref provient de l'autoref ou du dossier
    bool                    m_isnewMesureKerato;                            // détermine si la mesure de kerato provient de l'autoref ou du dossier
    bool                    m_isnewMesureTono;                              // détermine si la mesure de tono provient de l'autoref ou du dossier
    //LE REFRACTEUR ------------------------------------------------
    QString                 m_htmlMesureRefracteurSubjectif;
    QMap<QString,QVariant>  map_mesureRefracteurFinal;
    QString                 m_htmlMesureRefracteurFinal;
    bool                    LectureDonneesRefracteur(QString Mesure);       // lit les données envoyées sur le port série du refracteur
    void                    setHtmlRefracteur();                            // détermine le html à inscrire dans la fiche observation à partir des QMap MesureFronto, MesureAutoref et MesureRefracteurSubjectif
    void                    RegleRefracteur();
    void                    ReponsePortSerie_Autoref(const QString &s);
    void                    ReponsePortSerie_Fronto(const QString &s);
    void                    ReponsePortSerie_Refracteur(const QString &s);
    QByteArray              RequestToSendNIDEK();
    QByteArray              SendDataNIDEK(QString mesure);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Procedures::TypesMesures)

#endif // PROCEDURES_H
