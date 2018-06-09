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
    void        EnregistrePaiement(QString);


private slots:
    void        Slot_ActiveActeAccueil(int);                        // dans l'accueil secrétaire, quand on sélectionne un acte effectué pour un parent, ceux effectués pour d'autres parents sont déselectionnés
    void        Slot_AfficheMessages(int idx = -1);
    void        Slot_ActeMontantModifie();
    void        Slot_AfficheMotif(int);
    void        Slot_AfficheToolTip(QModelIndex);
    void        Slot_AfficheToolTip(int);
    void        Slot_AfficheMenu();
    void        Slot_AppelPaiementDirect(QString Origin = "");
    void        Slot_AppelPaiementTiers();
    void        Slot_AutreDossier(int idPat);
    void        Slot_BasculerMontantActe();
    void        Slot_BilanRecettes();
    void        Slot_CCAM(QString);
    void        Slot_ChangeTabBureau();
    void        Slot_ChercheNomparID(QString id);
    void        Slot_ChoixMG();
    void        Slot_ChoixCor();
    void        Slot_ChoixDossier();
    void        Slot_ChoixDossier(int idPat);
    void        Slot_ConnectTimers(bool = true);
    void        Slot_CourrierAFaireChecked();
    void        Slot_CreerActe();
    void        Slot_CreerBilanOrtho();
    void        Slot_ChercherDepuisListepushButtonClicked();
    void        Slot_CreerDossierpushButtonClicked();
    void        Slot_EnableCreerDossierButton();
    void        Slot_ExporteDocs();
    void        Slot_FiltrecheckBoxClicked();
    void        Slot_FiltreSalleDAttente(int);
    void        Slot_FiltreAccueil(int idx);
    void        Slot_GestionComptes();
    void        Slot_IdentificationUser();   // CZ001
    void        Slot_ImportDocsExternes();
    void        Slot_ImprimeDossier();
    bool        Slot_InscritEnSalDat(int);
    void        Slot_DropPatient(QByteArray);
    void        Slot_LireLaCPSpushButtonClicked();      // CZ001
    void        Slot_LireLaCVpushButtonClicked();       // CZ001
    void        Slot_MetAJourLaConnexion();
    void        Slot_MajusculeCreerNom();
    void        Slot_MajusculeCreerPrenom();
    void        Slot_MAJ_SalleDAttente();
    void        Slot_MenuContextuelIdentPatient();
    void        Slot_MenuContextuelListePatients(QPoint point);
    void        Slot_MenuContextuelMotsCles();
        void        Slot_RechercheParMotCle();
    void        Slot_RechercheParID();
    void        Slot_AfficheDossiersRechercheParMotCle();
    void        Slot_ImprimeListPatients(QVariant var);
    void        Slot_MenuContextuelBureaux();
        void        Slot_ChoixMenuContextuelListePatients(QString);
    void        Slot_MenuContextuelMedecin();
        void        Slot_ChoixMenuContextuelMedecin();
    void        Slot_MenuContextuelCorrespondant();
        void        Slot_ChoixMenuContextuelCorrespondant(QString choix);
    void        Slot_MenuContextuelSalDat();
    void        Slot_MenuContextuelSalDatPaiemt();
        void        Slot_ChoixMenuContextuelSalDat(QString);
    void        Slot_MenuContextuelUptextEdit();
        void        Slot_ChoixMenuContextuelUptextEdit(QString);
    void        Slot_ModifActeDate();
    void        Slot_ModifierTerrain();
    void        Slot_MsgModif(int);
    void        Slot_MsgResp(int);
    void        Slot_EnregMsgResp(int);
    void        Slot_MsgDone(bool);
    void        Slot_MsgRead(bool);
    void        Slot_OKModifierTerrain();
    void        Slot_ModfiCotationActe();
    void        Slot_NavigationActePrecpushButtonClicked();
    void        Slot_NavigationActeSuivpushButtonClicked();
    void        Slot_NavigationPremierActepushButtonClicked();
    void        Slot_NavigationDernierActepushButtonClicked();
    void        Slot_NavigationDossierPrecedentListe();
    void        Slot_NavigationDossierSuivantListe();
    void        Slot_OuvrirActesPrecspushButtonClicked();
    void        Slot_OuvrirDocsExternes();
    void        Slot_OuvrirDocuments();
    void        Slot_OuvrirJournalDepenses();
    void        Slot_OuvrirListepushButtonClicked();
    void        Slot_OuvrirNouveauDossierpushButtonClicked();
    void        Slot_OuvrirParametres();
    void        Slot_RecopierDossierpushButtonClicked();
    void        Slot_SendMessage();
    void        Slot_AllusrChkBoxSendMsg(bool a);
    void        Slot_OneusrChkBoxSendMsg(bool a);
    void        Slot_VerifSendMessage(int idMsg = -1);
    void        Slot_AfficheMessageLimitDate(bool a);
    void        Slot_SurbrillanceSalDat();
    void        Slot_SupprimerActepushButtonClicked();
    void        Slot_RecettesSpeciales();
    void        Slot_Refraction();
    void        Slot_RemiseCheques();
    void        Slot_RetrouveiDDepuisTab(int x, int y);
    void        Slot_RetrouveMontantActe();
    void        Slot_SalleDAttente();
    void        Slot_SaisieFSE();           // CZ001
    void        Slot_SupprimerDocs();
    void        Slot_SupprimerDossier();
    void        Slot_SupprimerMessageEmis(int idMsg);
    void        Slot_SupprimerMessageRecu(int idJoint);
    void        Slot_Tonometrie();
    void        Slot_TrouverDDN();
    void        Slot_VerifMessages();
    void        Slot_VerifSalleDAttente();
    void        Slot_VerifVerrouDossier();
    void        Slot_ActualiseDocsExternes();

    void        Slot_Moulinette();
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
    QTimer                  *gTimerSalleDAttente, *gTimerUserConnecte, *gTimerVerifVerrou, *gTimerVerifGestDocs, *gTimerSupprDocs;
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
    int                 LectureMesure(QString lIdPatient, QString lPatNom, QString lPatPrenom, QString lPatDDN, QString lPatCreeLe, QString lPatCreePar, QString MessageErreur);
    void                MAJActesPrecs();
    void                MAJDocsExternes();
    void                MAJMG(QObject*);
    void                MonteUneLigne();
    void                Monte20Lignes();
    QStringList         MotifMessage(QString Motif = "", QString Message = "", QTime heurerdv = QTime::currentTime());
    bool                NavigationConsult(int i);
    void                NavigationDossier(int i);
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
    bool                SalleDattente(QString Titre);
    void                SendMessage(QMap<QString,QVariant>, int id = -1 , int idMsg = -1);
    void                SupprimerActe();
    void                SupprimerDossier(int);
    void                Tonometrie();
    void                TrouverDDN();
    bool                ValideActeMontantLineEdit(QString NouveauMontant = "0,00", QString AncienMontant = "0.00");
    void                VerifImportateur();

    // les connexions aux appareils de mesure
    private slots:
        void                    Slot_NouvMesureRefraction();

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
