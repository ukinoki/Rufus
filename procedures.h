/* (C) 2016 LAINE SERGE
This file is part of Rufus.

Rufus is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Rufus is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Rufus.  If not, see <http://www.gnu.org/licenses/>.
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
#include "upmessagebox.h"

#include "cls_correspondant.h"
#include "cls_correspondants.h"
#include "cls_cotation.h"
#include "cls_depense.h"
#include "cls_depenses.h"
#include "cls_motif.h"
#include "cls_patient.h"
#include "cls_user.h"
#include "cls_users.h"
#include "database.h"

class Procedures : public QObject
{
    Q_OBJECT

private:
    explicit Procedures(QObject *parent = Q_NULLPTR);
    static Procedures *instance;

public:
    static Procedures *I();
    bool                gdbOK;
    bool                dlgrefractionouverte;
    int                 gMode2;

    QSettings           *gsettingsIni;
    QString             CouleurTitres;

    void                    ab(int i = 1);

    QFont                   AppFont();
    QString                 ConvertitEnHtml(QString Texte);
    QMap<QString, QDate>    ChoixDate(QWidget *parent=Q_NULLPTR);
    QString                 Edit(QString txt, QString titre = "", bool editable = true, bool ConnectAuSignal = false);
    void                    EditHtml(QString txt);
    bool                    FicheChoixConnexion();
    QString                 getDossierDocuments(QString Appareil, int mod = DataBase::ReseauLocal);
    int                     GetflagMG();
    int                     GetflagSalDat();
    void                    MAJflagMG();
    void                    MAJTcpMsgEtFlagSalDat();
    void                    MAJflagMessages();
    int                     getMAXligneBanque();
    void                    EnChantier(bool avecMsg = false);
    void                    Message(QStringList listmsg, int pause = 1000, bool bottom = true);
    void                    Message(QString mess, int pause = 1000, bool bottom = true);

    void                    ModifTailleFont(QObject *obj, int siz, QFont font=qApp->font());
    int                     Nombre_Mesure_Selected(QTreeWidget *Tree);
    int                     Nombre_Mesure_Selected(QTableWidget *Table, int col);
    QString                 RecupPartie(QString ficModele, QString partie);
    QString                 ImpressionCorps(QString text, bool ALD = false);
    QMap<QString,QString>   ImpressionEntete(QDate date, User* user);
    User*                   setDataOtherUser (int id);
    QString                 ImpressionPied(bool lunettes = false, bool ALD = false);
    bool                    Imprime_Etat(QTextEdit *Etat, QString EnTete, QString Pied,
                                     int TaillePieddePage, int TailleEnTete, int TailleTopMarge,
                                     bool AvecDupli = false, bool AvecPrevisu = false, bool AvecNumPage = false,
                                     bool AvecChoixImprimante = true);
    void                    Imprimer_Etat(QWidget *Formu, QPlainTextEdit *Etat);                                  // CZ 27082015
    QString                 RetireCaracteresAccentues(QString mot);
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
    // definition du superviseur, de l'utilisateur qui enregistre la commpta et de l'utilistaion de la compta
    //--------------------------------------------------------------------------------------------------------
private:
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
    int                     UserSuperviseur(); //TODO : DELETE
    int                     UserParent(); //TODO : DELETE
    bool                    ChargeDataUser(int iduser);
    QString                 getsSessionStatus();
    //--------------------------------------------------------------------------------------------------------


public:
    bool                    VerifRessources(QString Nomfile = "");

    bool                    Connexion_A_La_Base();
    QStringList             DecomposeScriptSQL(QString nomficscript);
    void                    InfoBase();
    bool                    ReinitBase();
    bool                    RestaureBase(bool BaseVierge = false, bool PremierDemarrage = false, bool VerifUserConnectes = true);
    bool                    VerifBaseEtRessources();
    void                    VideDatabases();
    double                  BaseSize, ImagesSize, VideosSize, FreeSpace;
    UpDialog                *gAskBupRestore;
    UpLabel                 *labelResume, *labelVolumeLibre;
    void                    AskBupRestore(bool restore, QString pathorigin, QString pathdestination, bool OKini = true, bool OKRessces = true, bool OKimages = true, bool OKvideos = true);
    void                    DefinitScriptBackup(QString path, bool AvecImages= true, bool AvecVideos = true);
    bool                    ImmediateBackup(bool full=false);
    QString                 getExpressionSize(double size);
    void                    ModifParamBackup();
    double                  CalcBaseSize();

    // Les accesseurs
    bool                    Init();
    bool                    ApercuAvantImpression();
    QString                 getCodePostalParDefaut();
    void                    setCodePostalParDefaut(QString CPParDefaut);
    User*                   getUserConnected();
    Patient*                getPatientById(int id);
    void                    setDirImagerie();
    QString                 DirImagerie();
    void                    setFicheRefractionOuverte(bool a);
    bool                    FicheRefractionOuverte();
    QStandardItemModel*     getListeComptesEncaissmtUser();
    QStandardItemModel*     getListeComptesEncaissmtUserAvecDesactive();
    void                    setListeComptesEncaissmtUser(int);
    Villes*                 getVilles();
    QString                 getLogin(int idUser);

    void                    initListeUsers();
    void                    initListeBanques();
    void                    initListeComptes();
    void                    initListeCorrespondants();
    void                    initListeCorrespondantsAll();
    void                    initListeCotationsByUser(int iduser);
    void                    initListeDepenses(int iduser);
    void                    initListeMotifs();
    void                    initListeTiers();
    void                    initTypesTiers();
    bool                    isPosteImportDocs();

    void                    ReconstruitComboCorrespondants(QComboBox* box, bool All = true); // si all = false => que les generalistes

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
    bool                    Verif_secure_file_priv();
    QString                 Var_secure_file_priv();

    bool                    Connexion();

    QString                 gLoginUser() { return m_userConnected->getLogin(); }
    QString                 gMDPUser() { return m_userConnected->getPassword(); }
    bool                    UtiliseTCP();

signals:
    void                    UpdSalDat();
    void                    UpdCorrespondants();
    void                    UpdDocsExternes();
    void                    ModifEdit(QString txt);
    void                    ConnectTimers(bool);

private:
    bool                    initOK;
    bool                    connexion;
    bool                    OKTCP;
    dlg_choixdate           *Dlg_ChxDate;
    dlg_gestionusers        *Dlg_GestUsr;
    dlg_paramconnexion      *Dlg_ParamConnex;
    QFont                   gAppFont;
    User *m_userConnected = nullptr; //user connected //TODO : DEPLACER DANS DATAS
    User *OtherUser = nullptr; //TODO créer QMap<int, User> iduser, user
    QStandardItemModel      *ListeComptesEncaissUser;
    QStandardItemModel      *ListeComptesEncaissUserAvecDesactive;

    QString                 DirStockageImages;
    QString                 lCPParDefaut, lVilleParDefaut;
    QPlainTextEdit          *gEtat;         // CZ 27082015
    QString                 gnomFichIni;
    QString                 gnomImprimante;
    Villes                  *m_villes = nullptr;
    UpDialog                *gAskLogin, *gAskUser;//, *gAskLieux;
    bool                    VerifParamConnexion(bool OKAccesDistant = true, QString nomtblutilisateurs = NOM_TABLE_UTILISATEURS);
    bool                    CreerPremierUser(QString Login, QString MDP);
    QString                 CreerUserFactice(User &user);
    QStringList             ChoisirUnLogin();
    QString                 gLogin, gConfirmMDP, gNouvMDP;
private slots:
    void                    Slot_VerifLogin();
    void                    Slot_CalcTimeBupRestore();
private:
    bool                    PremierDemarrage();
    void                    PremierParametrageMateriel();
    void                    PremierParametrageRessources();
    void                    Test();

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
    QString                 PrefixePlus(QString);                          // convertit en QString signé + ou - les valeurs QDouble de dioptries

signals:
    void                    NouvMesureRefraction();

public:
    SerialThread            *ThreadFronto, *ThreadRefracteur, *ThreadAutoref;
    QSerialPort*            PortAutoref();
    QSerialPort*            PortFronto();
    QSerialPort*            PortRefracteur();
    QSerialPort*            PortTono();
    QString                 NomPortAutoref();
    QString                 NomPortRefracteur();
    QString                 NomPortFronto();
    QString                 NomPortTono();
    QString                 TypeMesureRefraction();                     // accesseur pour le type de mesure effectuée: Fronto, Autoref ou Refracteur
    void                    setTypeMesureRefraction(QString);           // détermine le type de mesure effectuée: Fronto, Autoref ou Refracteur
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
    QString                 CalculeFormule(
                                QMap<QString,QVariant>  Donnees,
                                QString Cote);                      // comme son nom l'indique
    void                    InsertRefraction(
                                int idPatient,
                                int idActe,
                                QString Mesure = "All");            // enregistre la mesure de réfraction
    void                    SetDataAEnvoyerAuRefracteur(QMap<QString, QVariant> DataFronto, QMap<QString,QVariant> DataAutoref);


private:
    QString                 gMesureSerie;
    QString                 MesureRef;                              // le type de mesure effectuée: Fronto, Autoref ou Refracteur
    void                    ClearMesures();
    void                    ClearHtmlMesures();
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

#endif // PROCEDURES_H
