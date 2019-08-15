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
    explicit Procedures(QObject *parent = Q_NULLPTR);
    static Procedures *instance;
    bool                eventFilter(QObject *obj, QEvent *event)  ;

public:
    static Procedures   *I();
    DataBase            *db;
    bool                m_connexionbaseOK;
    bool                m_dlgrefractionouverte;
    DataBase::ModeAcces m_modeacces;
    bool                m_ok;

    QSettings               *m_settings;

    void                    ab(int i = 1);

    void                    CalcImage(Item *item, bool imagerie, bool afficher = true);
    QMap<QString, QDate>    ChoixDate(QWidget *parent=Q_NULLPTR);
    QString                 Edit(QString txt, QString titre = "", bool editable = true, bool ConnectAuSignal = false);
    void                    EditHtml(QString txt);
    void                    EditDocument(QMap<QString, QVariant> doc, QString label = "", QString titre = "", UpDialog::Buttons Button=UpDialog::ButtonOK);
    bool                    PrintDocument(QMap<QString, QVariant> doc);
    bool                    FicheChoixConnexion();
    QString                 pathDossierDocuments(QString Appareil, int mod = DataBase::ReseauLocal);
    void                    EnChantier(bool avecMsg = false);

    void                    ModifTailleFont(QObject *obj, int siz, QFont font=qApp->font());
    QString                 ImpressionCorps(QString text, bool ALD = false);
    QMap<QString,QString>   ImpressionEntete(QDate date, User* user);
    QString                 ImpressionPied(User* user, bool lunettes = false, bool ALD = false);
    bool                    Imprime_Etat(QTextEdit *Etat, QString EnTete, QString Pied,
                                     int TaillePieddePage, int TailleEnTete, int TailleTopMarge,
                                     bool AvecDupli = false, bool AvecPrevisu = false, bool AvecNumPage = false,
                                     bool AvecChoixImprimante = true);
    void                    Imprimer_Etat(QWidget *Formu, QPlainTextEdit *Etat);
    bool                    Imprime_pdf(QTextEdit *Etat, QString EnTete, QString Pied, QString nomfichier, QString nomdossier = "");
    bool                    VerifIni(QString msg = "",
                                     QString msgInfo = "",
                                     bool DetruitIni = true,
                                     bool RecupIni = true,
                                     bool ReconstruitIni = true,
                                     bool ReconstruitBase = true,
                                     bool PremDemarrage = false,
                                     bool BaseVierge = false);              // Récupère ou reconstruit le fichier d'initialisaton Rufus.ini et/ou la base
    int                     VerifUserBase(QString Login, QString MDP);      // Vérifie que l'utilisateur existe dans la base
    bool                    IdentificationUser(bool ChgtUtilisateur = false);

    //--------------------------------------------------------------------------------------------------------
    // les sauvegardes
    //--------------------------------------------------------------------------------------------------------
    enum Day {
                Lundi       = 0x1,
                Mardi       = 0x2,
                Mercredi    = 0x4,
                Jeudi       = 0x8,
                Vendredi    = 0x10,
                Samedi      = 0x20,
                Dimanche    = 0x40
              };    Q_ENUM(Day)
    Q_DECLARE_FLAGS(Days, Day)
    QTimer                  t_timerbackup;
    void                    AskBupRestore(bool restore, QString pathorigin, QString pathdestination, bool OKini = true, bool OKRessces = true, bool OKimages = true, bool OKvideos = true, bool OKfactures = true);
    bool                    Backup(QString dirSauv, bool OKBase, QString NomDirStockageImagerie = "", bool OKImages = false, bool OKVideos = false, bool OKFactures = false);
    void                    BackupWakeUp(QString NomDirDestination, QTime timebkup, Days days);                     // déclenche le backup à l'heure programmée
    void                    DefinitScriptBackup(QString NomDirDestination, QString NomDirStockageImagerie, bool AvecImages= true, bool AvecVideos = true);
    void                    DefinitScriptRestore(QStringList ListNomFiles);
    void                    EffaceAutoBackup();
    void                    EffaceScriptsBackup();
    bool                    ImmediateBackup(QString dirSauv = "", bool verifposteconnecte = true, bool full=false);
    void                    InitBackupAuto();
    void                    ParamAutoBackup(QString dirdestination, QString dirimagerie, QTime timebackup, Days days);
    bool                    VerifParamBackup(QString dirdestination, QTime time, Days days);

private slots:
    void                    Slot_CalcTimeBupRestore();
    //--------------------------------------------------------------------------------------------------------
    // fin sauvegardes
    //--------------------------------------------------------------------------------------------------------

private:
    //--------------------------------------------------------------------------------------------------------
    // definition du superviseur, de l'utilisateur qui enregistre la commpta et de l'utilistaion de la compta
    //--------------------------------------------------------------------------------------------------------
    //TODO : ICI info pour le rôle
    int                     m_idcentre;
    bool                    m_usecotation;
    bool                    m_aveccomptaprovisoire;
    bool                    m_isposteimportdocs;                      //! le poste est celui qui importe les documents
    bool                    DefinitRoleUser();                       /*! definit les iduser pour lequel le user travaille
                                                                        . iduser superviseur des actes                      (int gidUserSuperViseur)
                                                                            . lui-même s'il est responsable de ses actes
                                                                            . un autre user s'il est assistant
                                                                        . iduser qui enregistrera la comptabilité des actes (int gidUserComptable)
                                                                        . idUser soignant remplacé si le superviseur est remplaçant (int gidUserParent)
                                                                        . s'il cote les actes                            (bool gUseCotation)
                                                                        . s'il enregistre une compta                     (bool AvecLaComptaProv)
                                                                      */
    Site*                   DetermineLieuExercice();
    void                    CalcUserSuperviseur();
    void                    CalcUserParent();
public:
    bool                    SetUserAllData(User* usr, Item::UPDATE upd = Item::ForceUpdate);
    void                    ReconstruitListeComptes (User *usr, QList<Compte*>* listcomptes);
    int                     idCentre();
    int                     idLieuExercice();
    QString                 getSessionStatus();
    //--------------------------------------------------------------------------------------------------------


public:
    bool                    Connexion_A_La_Base();
    bool                    ReinitBase();
    bool                    RestaureBase(bool BaseVierge = false, bool PremierDemarrage = false, bool VerifPostesConnectes = true);

private:
    qint64                  m_basesize, m_imagessize, m_videossize, m_facturessize, m_freespace;
    UpDialog                *dlg_buprestore;
    UpLabel                 *wdg_resumelbl, *wdg_volumelibrelbl, *wdg_inflabel;
    QList<QImage>           m_listeimages;
    qint64                  CalcBaseSize();
    bool                    VerifBaseEtRessources();
    bool                    VerifRessources(QString Nomfile = "");
    void                    VideDatabases();

public:
    bool                    Init();
    bool                    ApercuAvantImpression();

    static QString          CodePostalParDefaut();
    static QString          VilleParDefaut();

    void                    setDirImagerie();
    QString                 DirImagerie();
    QString                 DirImagerieServeur();
    void                    setFicheRefractionOuverte(bool a);
    bool                    FicheRefractionOuverte();

    bool                    isPosteImportDocs();

    static void             ReconstruitComboCorrespondants(QComboBox* box, Correspondants::TYPECORRESPONDANT type = Correspondants::TousLesCorrespondants);

    QString                 getMDPAdmin();
    void                    setNomImprimante(QString NomImprimante);
    QString                 getNomImprimante();
    int                     TailleEnTete();
    int                     TailleEnTeteALD();
    int                     TaillePieddePage();
    int                     TaillePieddePageOrdoLunettes();
    int                     TailleTopMarge();

    void                    setPosteImportDocs(bool a = true);
    QString                 PosteImportDocs();
    bool                    AutresPostesConnectes(bool msg = true);
    bool                    Verif_secure_file_priv();

signals:
    void                    UpdDocsExternes();
    void                    ModifEdit(QString txt);
    void                    ConnectTimers(bool connect);
    void                    DelImage();
    void                    CloseEditDocument();

private:
    bool                    m_initok;
    QFont                   m_applicationfont;
    User                    *m_currentuser = Q_NULLPTR; //user connected //TODO : DEPLACER DANS DATAS
    ParametresSysteme       *m_parametres;
    dlg_choixdate           *Dlg_ChxDate;
    dlg_gestionusers        *Dlg_GestUsr;
    dlg_paramconnexion      *Dlg_ParamConnex;

    QString                 m_pathDirStockageImage, m_pathDirStockageImagesServeur;
    QString                 m_CPpardefaut, m_Villepardefaut;
    QString                 m_nomFichierIni;
    QString                 m_nomImprimante;
    UpDialog                *dlg_askLogin, *dlg_askUser;
    UpTableWidget           *wdg_tablewidget;
    QPrinter                *p_printer;
    QRectF                  m_rect;
    void                    Print(QPrinter*, QImage image);
    void                    PrintPdf(QPrinter*, Poppler::Document* document, bool &printok);
    bool                    VerifParamConnexion(bool OKAccesDistant = true, QString nomtblutilisateurs = TBL_UTILISATEURS);
    bool                    CreerPremierUser(QString Login, QString MDP);
    void                    CreerUserFactice(int idusr, QString login, QString mdp);

private:
    bool                    PremierDemarrage();
    void                    PremierParametrageMateriel();
    void                    PremierParametrageRessources();

/* ------------------------------------------------------------------------------------------------------------------------------------------
     GESTION DES PORTS SERIES -------------------------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------------------------------------------------*/
private:
    bool                    m_isAutorefParametre, m_isRefracteurParametre, m_isFrontoParametre, m_isTonoParametre;
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

signals:
    void                    NouvMesureRefraction();

public:
    enum TypeMesure {
                None,
                All,
                Fronto,
                Autoref,
                Kerato,
                Subjectif,
                Final,
                Tono,
                Pachy
                };  Q_ENUM(TypeMesure)

    QSerialPort*            PortAutoref();
    QSerialPort*            PortFronto();
    QSerialPort*            PortRefracteur();
    QSerialPort*            PortTono();
    TypeMesure              TypeMesureRefraction();                     // accesseur pour le type de mesure effectuée: Fronto, Autoref ou Refracteur
    void                    setTypeMesureRefraction(TypeMesure = None); // détermine le type de mesure effectuée: Fronto, Autoref ou Refracteur
    //LE FRONTO ----------------------------------------------------
    QMap<QString,QVariant>  DonneesFronto();                        // accesseur pour MesureFronto
    QString                 HtmlFronto();                           // accesseur pour le html de mesure fronto à afficher;
    //L'AUTOREF ----------------------------------------------------
    QMap<QString,QVariant>  DonneesAutoref();                       // accesseur pour MesureAutoref
    QMap<QString,QVariant>  DonneesKerato();                        // accesseur pour MesureKerato
    QString                 HtmlAutoref();                          // accesseur pour le html de mesure fronto à afficher;
    QString                 HtmlKerato();                           // accesseur pour le html de mesure kerato à afficher;
    QString                 HtmlTono();                             // accesseur pour le html de mesure tonométrie à afficher;
    QString                 HtmlPachy();                            // accesseur pour le html de mesure pachy à afficher;
   //LE REFRACTEUR ------------------------------------------------
    QMap<QString,QVariant>  DonneesRefracteurSubj();                // accesseur pour MesureRefracteurSubjectif
    QMap<QString,QVariant>  DonneesRefracteurFin();                 // accesseur pour MesureRefracteurFinal
    QString                 HtmlRefracteur();                       // accesseur pour le html de mesure refracteur à afficher;
    void                    InsertRefraction(
                                int idPatient,
                                int idActe,
                                TypeMesure = All);                  // enregistre la mesure de réfraction
    void                    SetDataAEnvoyerAuRefracteur(QMap<QString, QVariant> DataFronto, QMap<QString,QVariant> DataAutoref);

private:
    QString                 m_mesureSerie;
    TypeMesure              m_typemesureRefraction;                              // le type de mesure effectuée: Fronto, Autoref ou Refracteur
    void                    ClearMesures();
    void                    ClearHtmlMesures();
    void                    debugformule(QMap<QString,QVariant>  Data, QString type);
                                                                    // qdebug de la formule à partir du QMap<QString,QVariant>  Data des données de refraction
    //LE FRONTO ----------------------------------------------------
    QMap<QString,QVariant>  map_mesureFronto;
    QString                 m_htmlMesureFronto;
    void                    setDonneesFronto(QString Mesure);       // détermine le QMap MesureFronto à partir de la mesure relevée sur le port série du fronto
    void                    setHtmlFronto();                        // détermine le html à inscrire dans la fiche observation à partir du QMap MesureFronto
    bool                    m_isnewMesureFronto;                    // détermine si la mesure provient du fronto ou du dossier
    //L'AUTOREF ----------------------------------------------------
    QMap<QString,QVariant>  map_mesureAutoref;
    QMap<QString,QVariant>  map_mesureKerato;
    QMap<QString,QVariant>  map_mesureTono;
    QMap<QString,QVariant>  map_mesurePachy;
    QMap<QString,QVariant>  map_dataAEnvoyerAuRefracteur;
    QString                 m_htmlMesureAutoref;
    QString                 m_htmlMesureKerato;
    QString                 m_htmlMesureTono;
    QString                 m_htmlMesurePachy;
    void                    setDonneesAutoref(QString Mesure);              // détermine les QMap MesureAutoref, MesureKerato et MesureTono à partir de la mesure relevée sur le port série du fronto
    void                    setHtmlAutoref();                               // détermine le html à inscrire dans la fiche observation à partir du QMap MesureAutoref
    void                    setHtmlKerato(QMap<QString,QVariant>  MKer);    // détermine le html à inscrire dans la fiche observation à partir du QMap MesureKerato
    void                    setHtmlTono();                                  // détermine le html à inscrire dans la fiche observation à partir du QMap MesureTono
    void                    setHtmlPachy();                                 // détermine le html à inscrire dans la fiche observation à partir du QMap MesurePachy
    bool                    m_isnewMesureAutoref;                           // détermine si la mesure provient de l'autoref ou du dossier
    //LE REFRACTEUR ------------------------------------------------
    QMap<QString,QVariant>  map_mesureRefracteurSubjectif;
    QString                 m_htmlMesureRefracteurSubjectif;
    QMap<QString,QVariant>  map_mesureRefracteurFinal;
    QString                 m_htmlMesureRefracteurFinal;
    void                    setDonneesRefracteur(QString Mesure);   // détermine le QMap MesureRefracteur à partir de la mesure relevée sur le port série du refracteur
    void                    setHtmlRefracteur();                    // détermine le html à inscrire dans la fiche observation à partir des QMap MesureFronto, MesureAutoref et MesureRefracteurSubjectif
    void                    RegleRefracteur();

private slots:
    bool                    Ouverture_Ports_Series();
    void                    Slot_ReponsePortSerie_Autoref(const QString &s);
    void                    Slot_ReponsePortSerie_Fronto(const QString &s);
    void                    Slot_ReponsePortSerie_Refracteur(const QString &s);
};
Q_DECLARE_OPERATORS_FOR_FLAGS(Procedures::Days)

#endif // PROCEDURES_H
