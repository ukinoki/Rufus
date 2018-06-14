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

#ifndef RUFUS_H
#define RUFUS_H

#include <ctime>
#include <QDesktopServices>
#include <QtGlobal>
#include <QMainWindow>
#include <QMenuBar>
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
#include "dlg_recettes.h"
#include "dlg_remisecheques.h"
#include "dlg_salledattente.h"
#include "ui_dlg_salledattente.h"
#include "dlg_autresmesures.h"

#include "importdocsexternesthread.h"

#include "conversionbase.h"
#include "pyxinterf.h"

namespace Ui {
class Rufus;
}

namespace Ui {
class Rufuslow;
}

class Rufus : public QMainWindow
{
    Q_OBJECT

public:
    explicit Rufus(QWidget *parent = Q_NULLPTR);
    ~Rufus();
    Ui::Rufus                       *ui;

private:
    dlg_actesprecedents             *Dlg_ActesPrecs;
    dlg_bilanortho                  *Dlg_BlOrtho;
    Ui_dlg_bilanortho               *UiDLg_BlOrtho;
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
    dlg_param                       *Dlg_Param;
    dlg_autresmesures               *Dlg_AutresMes;
    dlg_recettes                    *Dlg_Rec;
    dlg_refraction                  *Dlg_Refraction;
    dlg_remisecheques               *Dlg_RemCheq;
    dlg_salledattente               *Dlg_SalDat;
    UpLabel                         *lblnom;
    UpLineEdit                      *MGlineEdit, *Cotation;
    UpLineEdit                      *AutresCorresp1LineEdit, *AutresCorresp2LineEdit;

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
    void        AfficheMenu(QMenu *menu);
    void        AfficheMotif(UpLabel *lbl);
    void        AfficheToolTip(int);
    void        AppelPaiementDirect(QString Origin = "");
    void        AppelPaiementTiers();
    void        AutreDossier(int idPat);
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
    void        ExporteDocs();
    void        FiltreAccueil(int idx);
    void        FiltrecheckBoxClicked();
    void        FiltreSalleDAttente(int);
    void        GestionComptes();
    void        IdentificationUser();
    void        ImportDocsExternes();
    void        ImprimeDossier();
    void        ImprimeListPatients(QVariant var);
    void        LireLaCV();       // CZ001
    void        LireLaCPS();      // CZ001
    void        MajusculeCreerNom();
    void        MajusculeCreerPrenom();
    void        ModfiCotationActe();
    void        ModifierTerrain();
    void        Moulinette();
    void        NouvelleMesureRefraction();    // les connexions aux appareils de mesure
    void        OKModifierTerrain();
    void        OuvrirActesPrecspushButtonClicked();
    void        OuvrirJournalDepenses();
    void        OuvrirParametres();
    void        RecettesSpeciales();
    void        RechercheParID();
    void        RechercheParMotCle();
    void        RetrouveiDDepuisTab(int x, int y, QTableWidget *);
    void        RetrouveMontantActe();
    void        SaisieFSE();           // CZ001
    void        SalleDAttente();
    void        SupprimerDocs();
    void        SupprimerDossier();
    void        SurbrillanceSalDat(UpLabel *lab);

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
    void        MenuContextuelUptextEdit(UpTextEdit *TxtEdit);
    void            ChoixMenuContextuelUptextEdit(QString);

    //fonctions lancées par des timers
    void        ActualiseDocsExternes();
    void        MetAJourLaConnexion();
    void        VerifMessages();
    void        VerifSalleDAttente();
    void        VerifVerrouDossier();

private slots:
    void        Slot_CalcIP(const QHostInfo &);

private:
    QString                 gIPadr;

    bool                    ophtalmo;
    bool                    orthoptist;
    bool                    autresoignant;
    bool                    secretaire;
    bool                    assistant;
    bool                    liberal;
    bool                    salarie;
    bool                    remplacant;
    bool                    pasremplacant;
    bool                    soignant;

    bool                    gAutorModifConsult, closeFlag;
    bool                    gIdentificationOK;
    bool                    ListeRestreinte;
    bool                    gCMUPatient;
    int                     gidPatient, gidActe, nbActes, noActe, gidARecopier, gAgePatient;
    int                     gflagPatients, gflagMG;
    int                     idPatientAuHasard, gNombreDossiers, gdossierAOuvrir;
    int                     idRefraction;
    int                     gMode;
    int                     gTotalMessages, gTotalNvxMessages;
    bool                    gAffichTotalMessages;
    enum gMode              {NouveauDossier, Liste, RechercheDDN};
    QDate                   gdateParDefaut, gDDNPatient;
    QDateTime               gUserdateDerniereConnexion, gUserDateDernierMessage;
    UpDialog                *gAsk;
    QMenu                   *gmenuContextuel;
    QString                 gNomPatient, gPrenomPatient;
    QString                 gActeMontant;
    QString                 gActeDate;
    QString                 grequeteListe, grequeteSalDat;
    QString                 gMonnaie;
    QString                 gSexePat, gNNIPat;
    QString                 gDirSauv;
    QStandardItemModel      *gListePatientsModel;
    QStandardItemModel      *gListeSuperviseursModel, *gListeParentsModel;
    QTabBar                 *gSalDatTab, *gAccueilTab;
    QTimer                  *gTimerSalleDAttente, *gTimerUserConnecte, *gTimerVerifVerrou, *gTimerSupprDocs;
    QTimer                  *gTimerExportDocs, *gTimerActualiseDocsExternes, *gTimerImportDocsExternes, *gTimerVerifMessages;
    Procedures              *proc;
    QSqlDatabase            db;

    QMap<QString,QVariant>  gDataUser;
    QMap<QString,QVariant>  gMesureFronto, gMesureAutoref;
    QString                 gUserDroits, gUserLogin, gUserFonction,
                            gUserTitre, gUserNom, gUserPrenom, gUserAdresse1, gUserAdresse2, gUserAdresse3,
                            gUserCodePostal, gUserVille, gUserTelephone, gUserMail, gUserSpecialite, gUserNumCO, gUserNumPS,
                            gUserMDP, gUserPortable, gUserPoste, gUserFax, gUserWeb,
                            gUserMemo, gUserPoliceEcran, gUserPoliceAttribut, gNomCompteEncaissHonoraires;
    int                     gidUser, gidUserSuperviseur, gidUserParent, gidUserComptable;
    int                     gUserNoSpecialite, gidCompteParDefaut, gidCompteEncaissHonoraires, gUserSecteur;
    bool                    gUserLiberal, gUserDesactive, gUserAGA, gMedecin, gUserOPTAM;
    UpDialog                *gAskRechParMotCleDialog,*gAskRechParIDDialog, *gAskListPatients;
    UpLabel                 *gAskinflabel;
    UpLineEdit              *gAsklinetitre;
    QDateEdit               *gAskeditdate;

    QDialog                 *gMsgRepons, *gMsgDialog;
    QSystemTrayIcon         *gMessageIcon;

    ImportDocsExternesThread*ImportDocsExtThread;
    QTreeWidget             *TerraintreeWidget;
    UpSmallButton           *ModifTerrainupSmallButton;


                    // Connexion objets ---------------------------------------------------------------------------------------------
    void                Connect_Slots();

                    // Fonctions ---------------------------------------------------------------------------------------------------
    void                closeEvent(QCloseEvent *);
    bool                eventFilter(QObject *obj, QEvent *event)  ;
    void                keyPressEvent ( QKeyEvent * event );
    void                AfficheActe(int idActe);
    void                AfficheActeCompta();
    void                AfficheDossier(int IdPat);
    bool                AutorDepartConsult(bool ChgtDossier);
    bool                AutorSortieAppli();
    void                CalcMotsCles(int idpt);
    void                CalcNbDossiers();
    QString             CalcToolTipCorrespondant(int);
    bool                ChargeDataUser();
    void                ChercheNom(int idpat = 0);
    void                ChercheNomFiltre(int idpat = 0);
    void                ChoixDossier(int idpat = 0);
    void                CreerActe(int idPat);
    void                ChercherDepuisListe();
    void                CreerDossier();
    void                CreerMenu();
    void                DescendUneLigne();
    void                Descend20Lignes();
    int                 EnregistreNouveauCorresp(QString Cor, QString Nom);
    void                FermeDlgAnnexes();
    bool                FermeDossier();
    void                FlagMetAjourMG();
    void                FlagMetAjourTreeView();
    bool                IdentificationPatient(QString CreationModification, int idPat);
    bool                Imprimer_Document(QString idUser, QString titre, QString Entete, QString text, QDate date, QString nom, QString prenom,
                                          bool Prescription, bool ALD, bool AvecPrintDialog, bool AvecDupli = false, bool AvecChoixImprimante = false);
    void                InitDivers();
    void                InitEventFilters();
    void                InitMenus();
    void                InitTables();
    void                InitVariables();
    bool                InscritEnSalDat(int);
    int                 LectureMesure(QString lIdPatient, QString lPatNom, QString lPatPrenom, QString lPatDDN, QString lPatCreeLe, QString lPatCreePar, QString MessageErreur);
    void                MAJActesPrecs();
    void                MAJDocsExternes();
    void                MAJMG(QObject*);
    void                MonteUneLigne();
    void                Monte20Lignes();
    QStringList         MotifMessage(QString Motif = "", QString Message = "", QTime heurerdv = QTime::currentTime());
    bool                NavigationConsult(int i);
    void                OuvrirActesPrecedents(int idActeEnCours);
    void                OuvrirDocsExternes(int idpat, bool depuismenu = false);
    void                OuvrirDocuments(bool AffichDocsExternes = true);
    void                OuvrirListe(bool AvecRecalcul = true);
    void                OuvrirNouveauDossier();
    void                RecopierDossier(int idARecopier = 0);
    void                RecaleTableView(int idPat);
    void                Refraction();
    void                ReconstruitListesActes();
    void                ReconstruitListesCorresp();
    void                RegleRefracteur(QString TypeMesure);
    void                RemiseCheques();
    bool                Remplir_ListePatients_TableView(QString requete, QString PatNom, QString PatPrenom);
    QTabWidget*         Remplir_MsgTabWidget();
    void                Remplir_SalDat();
    bool                RetourSalleDattente(QString Titre);
    void                SendMessage(QMap<QString,QVariant>, int id = -1 , int idMsg = -1);
    void                SupprimerActe();
    void                SupprimerDossier(int);
    void                Tonometrie();
    void                TrouverDDN();
    bool                ValideActeMontantLineEdit(QString NouveauMontant = "0,00", QString AncienMontant = "0.00");
    void                VerifImportateur();


    // Les menus
private:
        QMenu           *menuActe, *menuComptabilite, *menuEdition, *menuDocuments, *menuDossier;
        QMenu           *menuEmettre, *menuPrecedentsActes;
        QMenu           *menuAide;
        QAction         *actionCreerDossier, *actionCreerActe, *actionOuvrirDossier, *actionEmettreDocument, *actionRecopierDossier;
        QAction         *actionParametres, *actionSupprimerActe, *actionSupprimerDossier, *actionRechercheParMotCle, *actionRechercheParID;
        QAction         *actionDossierPatient, *actionCorrespondants, *actionEnregistrerDocScanner, *actionEnregistrerVideo;
        QAction         *actionGestionComptesBancaires, *actionPaiementDirect, *actionPaiementTiers, *actionRecettesSpeciales;
        QAction         *actionBilanRecettes, *actionJournalDepenses, *actionRemiseCheques, *actionImpayes;
        QAction         *actionQuit;
};

#endif // RUFUS_H
