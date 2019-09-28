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

#include <poppler-qt5.h>

#include "dlg_actesprecedents.h"
#include "dlg_autresmesures.h"
#include "dlg_bilanortho.h"
#include "dlg_bilanrecettes.h"
#include "ui_dlg_bilanortho.h"
#include "dlg_comptes.h"
#include "dlg_depenses.h"
#include "ui_dlg_depenses.h"
#include "dlg_docsexternes.h"
#include "dlg_docsscanner.h"
#include "dlg_docsvideo.h"
#include "dlg_impressions.h"
#include "ui_dlg_impressions.h"
#include "dlg_gestionlieux.h"
#include "dlg_identificationcorresp.h"
#include "ui_dlg_identificationcorresp.h"
#include "dlg_identificationpatient.h"
#include "ui_dlg_identificationpatient.h"
#include "dlg_listecorrespondants.h"
#include "dlg_listemotscles.h"
#include "dlg_paiementdirect.h"
#include "dlg_paiementtiers.h"
#include "dlg_param.h"
#include "ui_dlg_param.h"
#include "dlg_refraction.h"
#include "dlg_recettesspeciales.h"
#include "ui_dlg_recettesspeciales.h"
#include "dlg_remisecheques.h"
#include "dlg_salledattente.h"
#include "ui_dlg_salledattente.h"

#include "updoublevalidator.h"
#include "upstandarditem.h"
#include "conversionbase.h"
#include "database.h"
#include "flags.h"
#include "gbl_datas.h"
#include "icons.h"
#include "importdocsexternesthread.h"
#include "log.h"
#include "pyxinterf.h"
#include "styles.h"
#include "tcpsocket.h"


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
    enum Origin {BoutonPaiement, Accueil, Menu};    Q_ENUM(Origin)

private:
    bool                            UtiliserTcpSocket = false;

    dlg_actesprecedents             *Dlg_ActesPrecs;
    dlg_bilanortho                  *Dlg_BlOrtho;
    dlg_comptes                     *Dlg_Cmpt;
    dlg_depenses                    *Dlg_Deps;
    dlg_docsexternes                *Dlg_DocsExt;
    dlg_docsscanner                 *Dlg_DocsScan;
    dlg_docsvideo                   *Dlg_DocsVideo;
    dlg_impressions                   *Dlg_Imprs;
    dlg_identificationcorresp       *Dlg_IdentCorresp;
    dlg_identificationpatient       *Dlg_IdentPatient;
    dlg_listecorrespondants         *Dlg_ListCor;
    dlg_paiementdirect              *Dlg_PmtDirect;
    dlg_paiementtiers               *Dlg_PmtTiers;
    dlg_param                       *Dlg_Param;
    dlg_autresmesures               *Dlg_AutresMes;
    dlg_bilanrecettes               *Dlg_BilanRec;
    dlg_recettesspeciales           *Dlg_RecSpec;
    dlg_refraction                  *Dlg_Refraction;
    dlg_remisecheques               *Dlg_RemCheq;
    dlg_salledattente               *Dlg_SalDat;
    Procedures                      *proc;
    DataBase                        *db;
    ParametresSysteme               *m_parametres;
    PatientsEnCours                 *m_listepatientsencours = Datas::I()->patientsencours;
    UpLabel                         *wdg_nomlbl;
    UpLineEdit                      *wdg_MGlineEdit;
    UpLineEdit                      *wdg_autresCorresp1LineEdit, *wdg_autresCorresp2LineEdit;
    bool                            m_ok;
    upDoubleValidator               *m_val;

    pyxinterf                       *m_pyxi;     // CZ001

    void        ChoixMenuContextuelIdentPatient();
    void        ChoixMenuContextuelMotsCles();
    void        EnregistreDocScanner(Patient *pat);
    void        EnregistreVideo(Patient *pat);
    void        ListeCorrespondants();

private:
    void        ActeMontantModifie();
    void        ActiveActeAccueil(int);                        // dans l'accueil secrétaire, quand on sélectionne un acte effectué pour un parent, ceux effectués pour d'autres parents sont déselectionnés
    void        AfficheDossiersRechercheParMotCle();
    void        AfficheCourriersAFaire();
    void        AfficheMenu(QMenu *menu);
    void        AfficheMotif(UpLabel *lbl);
    void        AfficheToolTip(Patient *pat);
    void        AppelPaiementDirect(Origin origin);
    void        AppelPaiementTiers();
    void        AutreDossier(Patient *pat);
    void        BilanRecettes();
    void        CherchePatientParID(int id);
    void        ChoixCor(UpComboBox *box);
    void        BasculerMontantActe();
    void        ChoixMG();
    void        ChangeTabBureau();
    void        ConnectTimers(bool);
    void        CourrierAFaireChecked();
    void        CreerBilanOrtho();
    void        CreerDossierpushButtonClicked();
    void        DropPatient(QByteArray);
    void        EnableButtons();
    void        ExporteDocs();                                  /* exporte les documents d'imagerie inscrits dans la base pra les postes idstants
                                                                pour les archiver en fichiers standards sur le HD du serveur*/
    void        FiltreAccueil(int idx);
    void        FiltreSalleDAttente();
    void        GestionComptes();
    void        ImportDocsExternes();                           /* vérifie si le poste est le poste responsable de l'import des documents d'imagerie et si oui
                                                                lance le thread impotrtdocsexternesthread qui va importer les documents d'imagerie
                                                                enregistrés sur les dossiers d'échanges par les appareils d'imagerie*/
    void        VerifImportateur();                             /* vérifie que le poste importateur des documents externes est valide et le remplace au besoin*/
    void        ImprimeDossier(Patient *pat);
    void        ImprimeListPatients(QVariant var);
    void        ImprimeListActes(QList<Acte *> listeactes, bool toutledossier = true, bool queLePdf = false, QString  nomdossier = "");
    void        LireLaCV();       // CZ001
    void        LireLaCPS();      // CZ001
    void        MajusculeCreerNom();
    void        MajusculeCreerPrenom();
    void        ModifCotationActe();
    void        ModifierTerrain();
    void        Moulinette();
    void        NouvelleMesureRefraction(Procedures::TypeMesure TypeMesure);                     // les connexions aux appareils de mesure
    void        OKModifierTerrain(Patient *pat, bool recalclesdonnees = true);                  //!> recalcule le TreeWidgtet résumant le terrain
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
    void        MAJPosteConnecte();
    void        VerifCorrespondants();
    void        VerifMessages();
    void        VerifSalleDAttente();
    void        VerifVerrouDossier();

private:

    MesureRefraction *shortref_acuite       = Q_NULLPTR;
    bool                    m_autorModifConsult, m_closeflag;
    int                     m_flagcorrespondants, m_flagsalledattente, m_flagmessages;
    enum Mode               {NullMode, NouveauDossier, Liste, RechercheDDN};
    Mode                    m_mode;
    int                     m_totalMessages, m_totalNvxMessages;
    bool                    m_isTotalMessagesAffiche;
    QDate                   m_datepardefaut;
    QDateTime               m_datederniermessageuser;
    UpDialog                *dlg_ask;
    QMenu                   *m_menuContextuel;
    QString                 m_montantActe;
    QString                 m_dateActe;

    QStandardItemModel      *m_listepatientsmodel           = Q_NULLPTR;
    QStandardItemModel      *m_listepatientsencoursmodel    = Q_NULLPTR;
    QStandardItemModel      *m_listesuperviseursmodel       = Q_NULLPTR;
    QStandardItemModel      *m_listeparentsmodel            = Q_NULLPTR;
    QSortFilterProxyModel   *m_listepatientsproxymodel      = Q_NULLPTR;
    QSortFilterProxyModel   *m_DDNsortmodel                 = Q_NULLPTR;
    QSortFilterProxyModel   *m_prenomfiltersortmodel        = Q_NULLPTR;
    QTabBar                 *wdg_salledattenteTab, *wdg_accueilTab;
    QTimer                  *t_timerSalDat, *t_timerCorrespondants, *t_timerPosteConnecte, *t_timerVerifVerrou, *t_timerSupprDocs, *t_timerVerifImportateurDocs;
    QTimer                  *t_timerExportDocs, *t_timerActualiseDocsExternes, *t_timerImportDocsExternes, *t_timerVerifMessages;

    Acte                    *m_currentact                   = Q_NULLPTR;
    User                    *m_currentuser                  = Q_NULLPTR;
    Patients                *m_patients                     = Q_NULLPTR;
    QMap<int, Patient*>     *map_patientstable              = Q_NULLPTR;
    QMap<int, Patient*>     *map_patientssaldat             = Q_NULLPTR;
    Actes                   *m_listeactes                   = Q_NULLPTR;
    LignesPaiements         *m_lignespaiements              = Q_NULLPTR;

    UpDialog                *dlg_rechParMotCle,*dlg_rechParId, *dlg_listPatients;

    QDialog                 *dlg_msgRepons, *dlg_msgDialog;
    QSystemTrayIcon         *ict_messageIcon;

    ImportDocsExternesThread*ImportDocsExtThread;
    bool                    isPosteImport();
    bool                    m_isposteImport;
    bool                    m_pasDExportPourLeMoment = false;
    UpSmallButton           *wdg_modifIdentificationupSmallButton;

private:
                    // Connexion objets ---------------------------------------------------------------------------------------------
    void                ConnectSignals();

                    // Fonctions ---------------------------------------------------------------------------------------------------
    bool                eventFilter(QObject *obj, QEvent *event)  ;
    void                keyPressEvent ( QKeyEvent * event );
    void                AfficheActe(Acte *acte);
    void                AfficheActeCompta(Acte *acte);
    void                AfficheDossier(Patient *pat, int idacte = 0);
    void                AfficheMessageImport(QStringList listmsg, int pause, bool bottom=true);
    bool                AutorDepartConsult(bool ChgtDossier);
    QString             CalcHtmlIdentificationPatient(Patient *pat);
    QIcon               CalcIconPatient(Patient *pat);                              //!> renvoie l'icone qui représente le patient dans le html et sur le tab
    void                CalcMotsCles(Patient *pat);
    void                CalcNbDossiers();
    QString             CalcToolTipCorrespondant(int);
    void                FiltreTable(QString nom = "", QString prenom = "");         //!> filtrage de la liste des patients en fonction des valeurs correspondant aux zones de saisie
    void                ChoixDossier(Patient *pat, int idacte = 0);
    void                CreerActe(Patient *pat = Q_NULLPTR);
    void                ChercherDepuisListe();
    void                CreerDossier();
    void                CreerMenu();
    void                DescendUneLigne();
    void                Descend20Lignes();
    int                 EnregistreNouveauCorresp(QString Cor, QString Nom);
    void                ExporteActe(Acte *act);
    void                FermeDlgActesPrecedentsEtDocsExternes();
    bool                FermeDossier(Patient *pat);
    Patient*            getPatientFromRow(int row);                         //!> retrouve le patient correspondant à la rangée row
    Patient*            getPatientFromSelectionInTable();                   //!> retrouve le patient sélectionné dans la liste des patients
    Patient*            getPatientFromCursorPositionInTable();              //!> retrouve le patient sous le curseur de la souris dans la liste des patients
    Patient*            getPatientFromIndex(QModelIndex idx);               //!> retrouve le patient à partir du modelindex dans la table
    int                 getRowFromPatient(Patient *pat);                    //!> retrouve la rangée où se trouve un patient dans la table;
    bool                IdentificationPatient(dlg_identificationpatient::Mode mode, Patient *pat);
    bool                Imprimer_Document(User *user, QString titre, QString Entete, QString text, QDate date, QString nom, QString prenom,
                                          bool Prescription, bool ALD, bool AvecPrintDialog, bool AvecDupli = false, bool AvecChoixImprimante = false, bool Administratif = true);
    void                InitWidgets();
    void                InitEventFilters();
    void                InitMenus();
    void                InitVariables();
    bool                InscritEnSalDat(Patient *pat);
    void                MAJActesPrecs();
    void                MAJDocsExternes();
    void                MAJCorrespondant(QObject*);
    void                MonteUneLigne();
    void                Monte20Lignes();
    QStringList         MotifRDV(QString Motif = "", QString Message = "", QTime heurerdv = QTime::currentTime());
    bool                NavigationConsult(ItemsList::POSITION i);
    void                OuvrirActesPrecedents();
    void                OuvrirDocsExternes(Patient *pat);
    void                OuvrirImpressions(bool AffichDocsExternes = true);
    void                ModeSelectDepuisListe();                                                    //!> Passe en mode sélection depuis la liste de patients
    void                ModeCreationDossier();                                                      //!> Passe en mode création de dossier
    void                RecopierDossier(Patient *patient = Q_NULLPTR);
    void                RecaleTableView(Patient *pat, QAbstractItemView::ScrollHint scrollhint = QAbstractItemView::PositionAtCenter);
    int                 RecherchePatient(QString lPatNom, QString lPatPrenom, QString lPatDDN, QString MessageErreur);
    void                RefractionMesure(dlg_refraction::ModeOuverture mode);
    void                ConnectCotationComboBox();  //!> reconnecte la box des cotations à 2 signaux
                                                    //!> si une cotation est choisie, le montant de l'acte est recherché est affiché dans la ligne MontantLineEdit
                                                    //!> un tooltip est affiché décrivant le descriptif de la cotation mise en surbrillance dans la liste déroulante
    void                ReconstruitListesCotations();
    void                ReconstruitCombosCorresp(bool reconstruireliste = true);
    void                SetDatasRefractionKerato();
    void                RemiseCheques();
    bool                Remplir_ListePatients_TableView();
    QTabWidget*         Remplir_MsgTabWidget();
    void                Remplir_SalDat();
    bool                RetourSalleDattente(QString Titre);
    void                SendMessage(QMap<QString,QVariant>, int id = -1 , int idMsg = -1);
    void                setTitre();
    void                SortieAppli();
    void                SupprimerActe(Acte *act);
    void                SupprimerDocsEtFactures();
    void                SupprimerDossier(Patient *pat);
    void                Tonometrie();
    void                FiltreTableparDDN();
    bool                ValideActeMontantLineEdit(QString NouveauMontant = "0,00", QString AncienMontant = "0.00");
    bool                VerifCoherenceMontantPaiement();        /*! Vérifie que le montant facturé pour l'acte en cours n'est pas inférieur à la somme des paiements déjà enregistrés */

    // TCPServer, TCPSocket
    bool                m_utiliseTCP;
    QStringList         m_listesockets;
    TcpSocket           *TcPConnect;
    void                envoieMessage(QString msg);                     /* envoi d'un message au serveur pour être redispatché vers tous les clients */
    void                envoieMessageA(QList<int> listidusr);           /* envoi d'un message à une liste d'utilisateurs */
    void                TraiteTCPMessage(QString msg);                  /* traitement des messages reçus par les clients */
    void                TesteConnexion();
    QString             m_resumeStatut;

    // Les menus
private:
        QMenu           *menuActe, *menuComptabilite, *menuEdition, *menuDocuments, *menuDossier;
        QMenu           *menuEmettre, *menuPrecedentsActes;
        QMenu           *menuAide;
        QAction         *actionCreerDossier, *actionCreerActe, *actionOuvrirDossier, *actionEmettreDocument, *actionRecopierDossier;
        QAction         *actionParametres, *actionSupprimerActe, *actionSupprimerDossier, *actionRechercheParMotCle, *actionRechercheParID;
        QAction         *actionDossierPatient, *actionCorrespondants, *actionEnregistrerDocScanner, *actionEnregistrerVideo, *actionRechercheCourrier, *actionExportActe;
        QAction         *actionGestionComptesBancaires, *actionPaiementDirect, *actionPaiementTiers, *actionRecettesSpeciales, *actionResumeStatut;
        QAction         *actionBilanRecettes, *actionJournalDepenses, *actionRemiseCheques;
        QAction         *actionQuit;
};

#endif // RUFUS_H
