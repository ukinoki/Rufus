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

// #include <QWebEngineView>    /* abandonné parce QWebEngine pèse beaucoup trop lourd


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
    static Procedures *I();
    bool                gdbOK;
    bool                dlgrefractionouverte;
    int                 gMode2;
    DataBase            *db;
    bool                ok;

    QSettings           *gsettingsIni;
    QString             CouleurTitres;

    void                    ab(int i = 1);

    QMap<QString,QVariant>  CalcImage(int idimpression, QString typedoc, bool imagerie, bool afficher = true);
    QMap<QString, QDate>    ChoixDate(QWidget *parent=Q_NULLPTR);
    void                    DisplayWebPage(QUrl);    /* abandonné parce QWebEngine pèse beaucoup trop lourd */
    QString                 Edit(QString txt, QString titre = "", bool editable = true, bool ConnectAuSignal = false);
    void                    EditHtml(QString txt);
    void                    EditDocument(QMap<QString, QVariant> doc, QString label = "", QString titre = "", UpDialog::Buttons Button=UpDialog::ButtonOK);
    bool                    PrintDocument(QMap<QString, QVariant> doc);
    bool                    FicheChoixConnexion();
    QString                 getDossierDocuments(QString Appareil, int mod = DataBase::ReseauLocal);
    void                    EnChantier(bool avecMsg = false);
    void                    Message(QStringList listmsg, int pause = 1000, bool bottom = true);
    void                    Message(QString mess, int pause = 1000, bool bottom = true);

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
              };
    Q_DECLARE_FLAGS(Days, Day)
    QTimer                  gTimerBackup;
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

    //bool                    VerifParamBackup();
    //--------------------------------------------------------------------------------------------------------
    // fin sauvegardes
    //--------------------------------------------------------------------------------------------------------

private:
    //--------------------------------------------------------------------------------------------------------
    // definition du superviseur, de l'utilisateur qui enregistre la commpta et de l'utilistaion de la compta
    //--------------------------------------------------------------------------------------------------------
    //TODO : ICI info pour le rôle
    int                     gidCentre;
    bool                    gUseCotation;
    bool                    avecLaComptaProv;
    bool                    gisPosteImpotDocs;                       // le poste est celui qui importe les documents
    bool                    DefinitRoleUser();                       /* definit les iduser pour lequel le user travaille
                                                                        . iduser superviseur des actes                      (int gidUserSuperViseur)
                                                                            . lui-même s'il est responsable de ses actes
                                                                            . un autre user s'il est assistant
                                                                        . iduser qui enregistrera la comptabilité des actes (int gidUserComptable)
                                                                        . idUser soignant remplacé si le superviseur est remplaçant (int gidUserParent)
                                                                        . s'il cote les actes                            (bool gUseCotation)
                                                                        . s'il enregistre une compta                     (bool AvecLaComptaProv)
                                                                       */
    Site*                   DetermineLieuExercice();
    void                    RestoreFontAppliAndGeometry();
private slots:
    void                    Slot_CalcUserSuperviseur();
    void                    Slot_CalcUserParent();
public:
    int                     idCentre();
    int                     idLieuExercice();
    QString                 getSessionStatus();
    //--------------------------------------------------------------------------------------------------------


public:
    bool                    VerifRessources(QString Nomfile = "");

    bool                    Connexion_A_La_Base();
    bool                    ReinitBase();
    bool                    RestaureBase(bool BaseVierge = false, bool PremierDemarrage = false, bool VerifUserConnectes = true);
    bool                    VerifBaseEtRessources();
    void                    VideDatabases();
    qint64                  BaseSize, ImagesSize, VideosSize, FacturesSize, FreeSpace;
    UpDialog                *gAskBupRestore;
    UpLabel                 *labelResume, *labelVolumeLibre, *inflabel;
    QList<QImage>           listimage;
    qint64                  CalcBaseSize();

    bool                    Init();
    bool                    ApercuAvantImpression();
    // Les accesseurs
    QString                 getCodePostalParDefaut();
    void                    setCodePostalParDefaut(QString CPParDefaut);
    void                    setDirImagerie();
    QString                 DirImagerie();
    QString                 DirImagerieServeur();
    void                    setFicheRefractionOuverte(bool a);
    bool                    FicheRefractionOuverte();
    QStandardItemModel*     getListeComptesEncaissmtUser();
    QStandardItemModel*     getListeComptesEncaissmtUserAvecDesactive();
    void                    setListeComptesEncaissmtUser(int);

    bool                    isPosteImportDocs();

    void                    ReconstruitComboCorrespondants(QComboBox* box, bool All = true); // si all = false => que les generalistes
    void                    setmg(Patient* pat, int idcor = 0);
    void                    setspe1(Patient* pat, int idcor = 0);
    void                    setspe2(Patient* pat, int idcor = 0);

    QString                 getMDPAdmin();
    void                    setNomImprimante(QString NomImprimante);
    QString                 getNomImprimante();
    int                     TailleEnTete();
    int                     TailleEnTeteALD();
    int                     TaillePieddePage();
    int                     TaillePieddePageOrdoLunettes();
    int                     TailleTopMarge();
    QString                 getVilleParDefaut();

    void                    setPosteImportDocs(bool a = true);
    QString                 PosteImportDocs();
    bool                    VerifAutresPostesConnectes(bool msg = true);
    bool                    Verif_secure_file_priv();
    QString                 Var_secure_file_priv();

    bool                    Connexion();

    void                    TestAdminPresent();
    bool                    isadminpresent();
    void                    setoktcp(bool  ok);

signals:
    void                    UpdDocsExternes();
    void                    ModifEdit(QString txt);
    void                    ConnectTimers();
    void                    DelImage();
    void                    CloseEditDocument();

private:
    bool                    initOK;
    bool                    connexion;
    bool                    OKTCP, OKAdmin;
    dlg_choixdate           *Dlg_ChxDate;
    dlg_gestionusers        *Dlg_GestUsr;
    dlg_paramconnexion      *Dlg_ParamConnex;
    QFont                   gAppFont;
    User *m_userConnected = Q_NULLPTR; //user connected //TODO : DEPLACER DANS DATAS
    QStandardItemModel      *ListeComptesEncaissUser;
    QStandardItemModel      *ListeComptesEncaissUserAvecDesactive;

    QString                 DirStockageImages, DirStockageImagesServeur;
    QString                 lCPParDefaut, lVilleParDefaut;
    QPlainTextEdit          *gEtat;         // CZ 27082015
    QString                 gnomFichIni;
    QString                 gnomImprimante;
    Villes                  *m_villes = Q_NULLPTR;
    UpDialog                *gAskLogin, *gAskUser;
    UpTableWidget           *uptable;
    QPrinter                *printer;
    QRectF                  rect;
    void                    Print(QPrinter*, QImage image);
    void                    PrintPdf(QPrinter*, Poppler::Document* document, bool &printok);
    bool                    VerifParamConnexion(bool OKAccesDistant = true, QString nomtblutilisateurs = NOM_TABLE_UTILISATEURS);
    bool                    CreerPremierUser(QString Login, QString MDP);
    void                    CreerUserFactice(int idusr, QString login, QString mdp);
    QString                 gLogin, gConfirmMDP, gNouvMDP;

private slots:
    void                    Slot_CalcTimeBupRestore();
private:
    bool                    PremierDemarrage();
    void                    PremierParametrageMateriel();
    void                    PremierParametrageRessources();

public slots:
    void                    Slot_printPreview(QPrinter *printer);   // CZ 27082015

/* ------------------------------------------------------------------------------------------------------------------------------------------
     GESTION DES PORTS SERIES -------------------------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------------------------------------------------*/
private:
    bool                    gAutorefParametre, gRefracteurParametre, gFrontoParametre, gTonoParametre;
    QString                 gPortAutoref, gPortFronto, gPortRefracteur, gPortTono;
    QSerialPort             *lPortAutoref, *lPortRefracteur, *lPortTono, *lPortFronto;
    struct Settings {
        qint32 baudRate;
        QSerialPort::DataBits dataBits;
        QSerialPort::Parity parity;
        QSerialPort::StopBits stopBits;
        QSerialPort::FlowControl flowControl;
    };
    Settings                ParamPortSerieAutoref;
    Settings                ParamPortSerieFronto;
    Settings                ParamPortSerieRefracteur;
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
                };

    SerialThread            *ThreadFronto, *ThreadRefracteur, *ThreadAutoref;
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
    QString                 gMesureSerie;
    TypeMesure              MesureRef;                              // le type de mesure effectuée: Fronto, Autoref ou Refracteur
    void                    ClearMesures();
    void                    ClearHtmlMesures();
    void                    debugformule(QMap<QString,QVariant>  Data, QString type);
                                                                    // qdebug de la formule à partir du QMap<QString,QVariant>  Data des données de refraction
    //LE FRONTO ----------------------------------------------------
    QMap<QString,QVariant>  MesureFronto;
    QString                 HtmlMesureFronto;
    void                    setDonneesFronto(QString Mesure);       // détermine le QMap MesureFronto à partir de la mesure relevée sur le port série du fronto
    void                    setHtmlFronto();                        // détermine le html à inscrire dans la fiche observation à partir du QMap MesureFronto
    bool                    NouvMesureFronto;                       // détermine si la mesure provient du fronto ou du dossier
    //L'AUTOREF ----------------------------------------------------
    QMap<QString,QVariant>  MesureAutoref;
    QMap<QString,QVariant>  MesureKerato;
    QMap<QString,QVariant>  MesureTono;
    QMap<QString,QVariant>  MesurePachy;
    QMap<QString,QVariant>  DataAEnvoyerAuRefracteur;
    QString                 HtmlMesureAutoref;
    QString                 HtmlMesureKerato;
    QString                 HtmlMesureTono;
    QString                 HtmlMesurePachy;
    void                    setDonneesAutoref(QString Mesure);              // détermine les QMap MesureAutoref, MesureKerato et MesureTono à partir de la mesure relevée sur le port série du fronto
    void                    setHtmlAutoref();                               // détermine le html à inscrire dans la fiche observation à partir du QMap MesureAutoref
    void                    setHtmlKerato(QMap<QString,QVariant>  MKer);    // détermine le html à inscrire dans la fiche observation à partir du QMap MesureKerato
    void                    setHtmlTono();                                  // détermine le html à inscrire dans la fiche observation à partir du QMap MesureTono
    void                    setHtmlPachy();                                 // détermine le html à inscrire dans la fiche observation à partir du QMap MesurePachy
    bool                    NouvMesureAutoref;                              // détermine si la mesure provient de l'autoref ou du dossier
    //LE REFRACTEUR ------------------------------------------------
    QMap<QString,QVariant>  MesureRefracteurSubjectif;
    QString                 HtmlMesureRefracteurSubjectif;
    QMap<QString,QVariant>  MesureRefracteurFinal;
    QString                 HtmlMesureRefracteurFinal;
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
