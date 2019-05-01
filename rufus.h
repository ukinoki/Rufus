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

#ifndef RUFUS_H
#define RUFUS_H

#include <ctime>
#include <QDesktopServices>
#include <QtGlobal>
#include <QMainWindow>
#include <QMenuBar>
#include <QNetworkAccessManager>
#include <QStackedWidget>
#include <QSystemTrayIcon>
#include <QApplication>

#include <poppler-qt5.h>

#include "dlg_actesprecedents.h"
#include "dlg_bilanortho.h"
#include "ui_dlg_bilanortho.h"
#include "dlg_comptes.h"
#include "dlg_depenses.h"
#include "ui_dlg_depenses.h"
#include "dlg_docsexternes.h"
#include "dlg_docsscanner.h"
#include "dlg_docsvideo.h"
#include "dlg_documents.h"
#include "ui_dlg_documents.h"
#include "dlg_gestionlieux.h"
#include "dlg_identificationcorresp.h"
#include "ui_dlg_identificationcorresp.h"
#include "dlg_identificationpatient.h"
#include "ui_dlg_identificationpatient.h"
#include "dlg_listecorrespondants.h"
#include "dlg_listemotscles.h"
#include "dlg_paiement.h"
#include "dlg_param.h"
#include "ui_dlg_param.h"
#include "dlg_refraction.h"
#include "dlg_bilanrecettes.h"
#include "dlg_recettesspeciales.h"
#include "ui_dlg_recettesspeciales.h"
#include "dlg_remisecheques.h"
#include "dlg_salledattente.h"
#include "ui_dlg_salledattente.h"
#include "dlg_autresmesures.h"
#include "dlg_paiementdirect.h"
#include "dlg_paiementtiers.h"
#include "tcpsocket.h"

#include "flags.h"
#include "importdocsexternesthread.h"
#include "log.h"

#include "conversionbase.h"
#include "pyxinterf.h"

#include "cls_item.h"
#include "cls_user.h"
#include "cls_motif.h"
#include "cls_cotation.h"

namespace Ui {
class Rufus;
}

class Rufus : public QMainWindow
{
    Q_OBJECT

public:
    explicit Rufus(QWidget *parent = Q_NULLPTR);
    ~Rufus();
    Ui::Rufus                       *ui;

private:
    bool                            UtiliserTcpSocket = false;

    dlg_actesprecedents             *Dlg_ActesPrecs;
    dlg_bilanortho                  *Dlg_BlOrtho;
    dlg_comptes                     *Dlg_Cmpt;
    dlg_depenses                    *Dlg_Deps;
    dlg_docsexternes                *Dlg_DocsExt;
    dlg_docsscanner                 *Dlg_DocsScan;
    dlg_docsvideo                   *Dlg_DocsVideo;
    dlg_documents                   *Dlg_Docs;
    dlg_identificationcorresp       *Dlg_IdentCorresp;
    dlg_identificationpatient       *Dlg_IdentPatient;
    dlg_listecorrespondants         *Dlg_ListCor;
    dlg_paiement                    *Dlg_PaimtDirect, *Dlg_PaimtTiers;
    dlg_paiementdirect              *Dlg_PmtDirect;
    dlg_paiementtiers               *Dlg_PmtTiers;
    dlg_param                       *Dlg_Param;
    dlg_autresmesures               *Dlg_AutresMes;
    dlg_bilanrecettes               *Dlg_BilanRec;
    dlg_recettesspeciales           *Dlg_RecSpec;
    dlg_refraction                  *Dlg_Refraction;
    dlg_remisecheques               *Dlg_RemCheq;
    dlg_salledattente               *Dlg_SalDat;
    UpLabel                         *lblnom;
    UpLineEdit                      *MGlineEdit;
    UpLineEdit                      *AutresCorresp1LineEdit, *AutresCorresp2LineEdit;
    DataBase                        *db;
    Flags                           *flags;
    bool                            ok;

    pyxinterf                       *pyxi;     // CZ001

    // anciens slots
    void        ChoixMenuContextuelIdentPatient();
    void        ChoixMenuContextuelMotsCles();
    void        EnregistreDocScanner();
    void        EnregistreVideo();
    void        ListeCorrespondants();



signals:
    void        EnregistrePaiement();

private:
    void        ActeMontantModifie();
    void        ActiveActeAccueil(int);                        // dans l'accueil secrétaire, quand on sélectionne un acte effectué pour un parent, ceux effectués pour d'autres parents sont déselectionnés
    void        AfficheDossiersRechercheParMotCle();
    void        AfficheCourriersAFaire();
    void        AfficheMenu(QMenu *menu);
    void        AfficheMotif(UpLabel *lbl);
    void        AfficheToolTip(Patient *pat);
    void        AppelPaiementDirect(QString Origin = "");
    void        AppelPaiementTiers();
    void        AppelPaiementTiers2();
    void        AutreDossier(Patient *pat);
    void        BilanRecettes();
    void        ChercheNomparID(QString id);
    void        ChoixCor(UpComboBox *box);
    void        BasculerMontantActe();
    void        ChoixMG();
    void        ChangeTabBureau();
    void        ConnectTimers(bool);
    void        CourrierAFaireChecked();
    void        CreerBilanOrtho();
    void        CreerDossierpushButtonClicked();
    void        DropPatient(QByteArray);
    void        EnableCreerDossierButton();
    void        ExporteDocs();                                  /* exporte les documents d'imagerie inscrits dans la base pra les postes idstants
                                                                pour les archiver en fichiers standards sur le HD du serveur*/
    void        FiltreAccueil(int idx);
    void        FiltreSalleDAttente();
    void        GestionComptes();
    void        ImportDocsExternes();                           /* vérifie si le poste est le poste responsable de l'import des documents d'imagerie et si oui
                                                                lance le thread impotrtdocsexternesthread qui va importer les documents d'imagerie
                                                                enregistrés sur les dossiers d'échanges par les appareils d'imagerie*/
    void        VerifImportateur();                             /* vérifie que le poste importateur des documents externes est valide et le remplace au besoin*/
    void        ImprimeDossier();
    void        ImprimeListPatients(QVariant var);
    void        ImprimeListActes(QList<Acte *> listeactes, bool toutledossier = true, bool queLePdf = false, QString  nomdossier = "");
    void        LireLaCV();       // CZ001
    void        LireLaCPS();      // CZ001
    void        MajusculeCreerNom();
    void        MajusculeCreerPrenom();
    void        ModfiCotationActe();
    void        ModifierTerrain();
    void        Moulinette();
    void        NouvelleMesureRefraction();                     // les connexions aux appareils de mesure
    void        OKModifierTerrain(bool recalclesdonnees = true);
    void        OuvrirActesPrecspushButtonClicked();
    void        OuvrirJournalDepenses();
    void        OuvrirParametres();
    void        RecettesSpeciales();
    void        RechercheParID();
    void        RechercheParMotCle();
    void        ResumeStatut();
    void        RetrouveMontantActe();
    void        SaisieFSE();
    void        SalleDAttente();
    void        SurbrillanceSalDat(UpLabel *lab);

    // gestion des patients vus dans la journée
    QTimer      *gTimerPatientsVus;
    void        AffichePatientsVusWidget();
    void        MAJPatientsVus();
    void        MasquePatientsVusWidget();

    // la messagerie
    void        AfficheMessageLimitDate(bool a);
    void        AfficheMessages(int idx = -1);
    void        AllusrChkBoxSendMsg(bool a);
    void        EnregMsgResp(int);
    void        MsgDone(UpCheckBox *chk);
    void        MsgModif(int);
    void        MsgRead(UpCheckBox *chk);
    void        MsgResp(int);
    void        OneusrChkBoxSendMsg(bool a);
    void        ReconstruitListeMessages();
    void        SupprimerMessageEmis(int idMsg);
    void        SupprimerMessageRecu(int idJoint);
    void        VerifSendMessage(int idMsg = -1);

    void        MenuContextuelBureaux(UpTextEdit *UpText);
    void        MenuContextuelCorrespondant(UpComboBox *box);
    void            ChoixMenuContextuelCorrespondant(QString choix);
    void        MenuContextuelIdentPatient();
    void        MenuContextuelListePatients();
    void            ChoixMenuContextuelListePatients(QString);
    void        MenuContextuelMedecin();
    void            ChoixMenuContextuelMedecin();
    void        MenuContextuelMotsCles();
    void        MenuContextuelSalDatPaiemt(UpLabel *labelCLicked);
    void        MenuContextuelSalDat(UpLabel *labelCLicked);
    void            ChoixMenuContextuelSalDat(QString);

    //fonctions lancées par des timers
    void        ActualiseDocsExternes();
    void        MetAJourUserConnectes();
    void        VerifCorrespondants();
    void        VerifMessages();
    void        VerifSalleDAttente();
    void        VerifVerrouDossier();

private:
    bool                    gAutorModifConsult, closeFlag;
    bool                    gIdentificationOK;
    int                     nbActes, noActe;
    int                     m_flagcorrespondants, m_flagsalledattente, m_flagmessages;
    int                     gNombreDossiers;
    int                     idRefraction;
    int                     gMode;
    int                     gTotalMessages, gTotalNvxMessages;
    int                     idAdministrateur;
    bool                    gAffichTotalMessages;
    enum gMode              {NouveauDossier, Liste, RechercheDDN};
    QDate                   gdateParDefaut;
    QDateTime               gUserDateDernierMessage;
    UpDialog                *gAsk;
    QMenu                   *gmenuContextuel;
    QString                 gActeMontant;
    QString                 gActeDate;
    QString                 grequeteSalDat;
    QString                 gDirSauv;
    QStandardItemModel      *gListePatientsModel;
    QStandardItemModel      *gListeSuperviseursModel, *gListeParentsModel;
    QTabBar                 *gSalDatTab, *gAccueilTab;
    QTimer                  *gTimerSalDat, *gTimerCorrespondants, *gTimerUserConnecte, *gTimerVerifVerrou, *gTimerVerifConnexion, *gTimerSupprDocs, *gTimerVerifImportateurDocs;
    QTimer                  *gTimerExportDocs, *gTimerActualiseDocsExternes, *gTimerImportDocsExternes, *gTimerVerifMessages;
    Procedures              *proc;

    Acte                    *m_currentact;
    User                    *gUserEnCours;
    Patient                 *m_currentpatient;
    Patient                 *m_dossierpatientaouvrir;
    QMap<QString,QVariant>  gMesureFronto, gMesureAutoref;
    UpDialog                *gAskRechParMotCleDialog,*gAskRechParIDDialog, *gAskListPatients;
    UpLabel                 *gAskinflabel;
    UpLineEdit              *gAsklinetitre;
    QDateEdit               *gAskeditdate;

    QDialog                 *gMsgRepons, *gMsgDialog;
    QSystemTrayIcon         *gMessageIcon;

    ImportDocsExternesThread*ImportDocsExtThread;
    bool                    isPosteImport();
    bool                    PosteImport;
    bool                    PasDExportPourLeMoment = false;
    UpSmallButton           *ModifIdentificationupSmallButton;
private slots:
    void                    AfficheMessageImport(QStringList listmsg, int pause, bool bottom=true);

private:
                    // Connexion objets ---------------------------------------------------------------------------------------------
    void                Connect_Slots();

                    // Fonctions ---------------------------------------------------------------------------------------------------
    void                closeEvent(QCloseEvent *);
    bool                eventFilter(QObject *obj, QEvent *event)  ;
    void                keyPressEvent ( QKeyEvent * event );
    void                AfficheActe(Acte *acte);
    void                AfficheActeCompta();
    void                AfficheDossier(Patient *pat, int idacte = 0);
    bool                AutorDepartConsult(bool ChgtDossier);
    bool                AutorSortieAppli();
    void                CalcHtmlIdentificationPatient();
    void                CalcMotsCles(Patient *pat);
    void                CalcNbDossiers();
    QString             CalcToolTipCorrespondant(int);
    bool                ChargeDataUser();
    void                ChercheNomFiltre(Patient *pat = Q_NULLPTR);
    void                ChoixDossier(Patient *pat, int idacte = 0);
    void                CreerActe(Patient *pat = Q_NULLPTR);
    void                ChercherDepuisListe();
    void                CreerDossier();
    void                CreerMenu();
    void                DescendUneLigne();
    void                Descend20Lignes();
    int                 EnregistreNouveauCorresp(QString Cor, QString Nom);
    void                ExporteActe();
    void                FermeDlgAnnexes();
    bool                FermeDossier();
    void                FlagMetAjourSalDat();
    Patient*            getSelectedPatientFromTable();                       //!> retrouve le patient sélectionné dans la liste des patients
    Patient*            getSelectedPatientFromCursorPositionInTable();       //!> retrouve le patient sous le curseur de la souris dans la liste des patients
    bool                IdentificationPatient(dlg_identificationpatient::Mode mode, Patient *pat);
    bool                Imprimer_Document(User *user, QString titre, QString Entete, QString text, QDate date, QString nom, QString prenom,
                                          bool Prescription, bool ALD, bool AvecPrintDialog, bool AvecDupli = false, bool AvecChoixImprimante = false, bool Administratif = true);
    void                InitWidgets();
    void                InitEventFilters();
    void                InitMenus();
    void                InitVariables();
    bool                InscritEnSalDat(Patient *pat);
    int                 LectureMesure(QString lIdPatient, QString lPatNom, QString lPatPrenom, QString lPatDDN, QString lPatCreeLe, QString lPatCreePar, QString MessageErreur);
    void                MAJActesPrecs();
    void                MAJDocsExternes();
    void                MAJCorrespondant(QObject*);
    void                MonteUneLigne();
    void                Monte20Lignes();
    QStringList         MotifRDV(QString Motif = "", QString Message = "", QTime heurerdv = QTime::currentTime());
    bool                NavigationConsult(int i);
    void                OuvrirActesPrecedents();
    void                OuvrirDocsExternes(Patient *pat, bool depuismenucontextuel = false);
    void                OuvrirDocuments(bool AffichDocsExternes = true);
    void                OuvrirListe();
    void                OuvrirNouveauDossier();
    void                RecopierDossier(Patient *patient = Q_NULLPTR);
    void                RecaleTableView(Patient *pat);
    void                Refraction();
    void                ReconstruitListesActes();
    void                ReconstruitCombosCorresp(bool reconstruireliste = true);
    void                RegleRefracteur(QString TypeMesure);
    void                RemiseCheques();
    bool                Remplir_ListePatients_TableView(QList<Patient*>  listpatients);
    QTabWidget*         Remplir_MsgTabWidget();
    void                Remplir_SalDat();
    bool                RetourSalleDattente(QString Titre);
    void                SendMessage(QMap<QString,QVariant>, int id = -1 , int idMsg = -1);
    void                setTitre();
    void                SupprimerActe();
    void                SupprimerDocsEtFactures();
    void                SupprimerDossier();
    void                SupprimerDossier(Patient *pat);
    void                Tonometrie();
    void                TrouverDDN();
    bool                ValideActeMontantLineEdit(QString NouveauMontant = "0,00", QString AncienMontant = "0.00");

    // TCPServer, TCPSocket
    bool                UtiliseTCP;
    QStringList         gListSockets;
    TcpSocket           *TcPConnect;
    void                envoieMessage(QString msg);                     /* envoi d'un message au serveur pour être redispatché vers tous les clients */
    void                envoieMessageA(QList<int> listidusr);           /* envoi d'un message à une liste d'utilisateurs */
    void                TraiteTCPMessage(QString msg);                  /* traitement des messages reçus par les clients */
    void                TesteConnexion();
    QString             gResumeStatut;

    // Les menus
private:
        QMenu           *menuActe, *menuComptabilite, *menuEdition, *menuDocuments, *menuDossier;
        QMenu           *menuEmettre, *menuPrecedentsActes;
        QMenu           *menuAide;
        QAction         *actionCreerDossier, *actionCreerActe, *actionOuvrirDossier, *actionEmettreDocument, *actionRecopierDossier;
        QAction         *actionParametres, *actionSupprimerActe, *actionSupprimerDossier, *actionRechercheParMotCle, *actionRechercheParID;
        QAction         *actionDossierPatient, *actionCorrespondants, *actionEnregistrerDocScanner, *actionEnregistrerVideo, *actionRechercheCourrier, *actionExportActe;
        QAction         *actionGestionComptesBancaires, *actionPaiementDirect, *actionPaiementTiers, *actionPaiementTiers2, *actionRecettesSpeciales, *actionResumeStatut;
        QAction         *actionBilanRecettes, *actionJournalDepenses, *actionRemiseCheques;
        QAction         *actionQuit;
};

#endif // RUFUS_H
