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

#ifndef RUFUS_H
#define RUFUS_H

#include <ctime>
#include <QDesktopServices>
#include <QFileSystemWatcher>
#include <QtGlobal>
#include <QMainWindow>
#include <QMenuBar>
#include <QNetworkAccessManager>
#include <QStackedWidget>
#include <QSystemTrayIcon>
#include <QPdfDocument>

//#include <poppler-qt6.h>

#include <dlg_actesprecedents.h>
#include <dlg_autresmesures.h>
#include <dlg_bilanortho.h>
#include <dlg_bilanrecettes.h>
#include <ui_dlg_bilanortho.h>
#include <dlg_comptes.h>
#include <dlg_depenses.h>
#include <ui_dlg_depenses.h>
#include <dlg_docsexternes.h>
#include <dlg_docsscanner.h>
#include <dlg_docsvideo.h>
#include <dlg_impressions.h>
#include <ui_dlg_impressions.h>
#include <dlg_listelieux.h>
#include <dlg_identificationcorresp.h>
#include <ui_dlg_identificationcorresp.h>
#include <ui_dlg_identificationmanufacturer.h>
#include <dlg_identificationpatient.h>
#include <ui_dlg_identificationpatient.h>
#include <dlg_listecorrespondants.h>
#include <dlg_listeiols.h>
#include <dlg_listemanufacturers.h>
#include <dlg_listetiers.h>
#include <dlg_listemotscles.h>
#include <dlg_paiementdirect.h>
#include <dlg_paiementtiers.h>
#include <dlg_param.h>
#include <ui_dlg_param.h>
#include <dlg_programmationinterventions.h>
#include <dlg_refraction.h>
#include <dlg_recettesspeciales.h>
#include <ui_dlg_recettesspeciales.h>
#include <dlg_remisecheques.h>
#include <dlg_salledattente.h>
#include <ui_dlg_salledattente.h>

#include <upcombobox.h>
#include <updoublevalidator.h>
#include <upstandarditem.h>
#include <conversionbase.h>
#include <database.h>
#include <flags.h>
#include <gbl_datas.h>
#include <icons.h>
#include <importdocsexternesthread.h>
#include <log.h>
#include <pyxinterf.h>
#include <styles.h>
#include <tcpsocket.h>
#include <upsystemtrayicon.h>
#include <cls_rendezvous.h>


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

    Procedures                      *proc = Procedures::I();
    DataBase                        *db = DataBase::I();
    ParametresSysteme               *m_parametres;
    UpLabel                         *wdg_nomlbl;
    UpLineEdit                      *wdg_MGlineEdit;
    UpLineEdit                      *wdg_autresCorresp1LineEdit, *wdg_autresCorresp2LineEdit;
    bool                            m_ok;
    upDoubleValidator               *m_val;
    QAction         *actionFabricants               = Q_NULLPTR; // si on met ces 3 QAction avec les autres en fin de fichier le programme plante dans la fonction RapatrieDocsExternes de ImportsDocsExetrnes (????)
    QAction         *actionIOLs                     = Q_NULLPTR;
    QAction         *actionTiers                    = Q_NULLPTR;
    QDate m_currentdate;
    QTime m_currenttime;

    void RecalcCurrentDateTime() {
        QDateTime dt = db->ServerDateTime();
        m_currentdate = dt.date();
        m_currenttime = dt.time();}

    /*! IMPORT DES FICIERS D'IMAGERIE */
    QTimer                          t_timerfilewatcher;             /*! utilisé à la place du QfileSystemWatcher dont le signal directorychanged bugue trop */
    QFileSystemWatcher              m_filewatcher;                  /*! le QFilesystemwatcher surveille les dossiers où sont enregistrés les nouveaux documents d'imagerie */

    pyxinterf                       *m_pyxi;     // CZ001

private:
    void        ActeGratuit();
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
    void        EnregistreDocScanner(Patient *pat);
    void        EnregistreVideo(Patient *pat);
    void        ExporteDocs();                                  /*! exporte les documents d'imagerie inscrits dans la base par les postes distants
                                                                pour les archiver en fichiers standards sur le HD du serveur
                                                       les fichiers d'imagerie ou les factures enregistrés par des utilisateurs distants sont stockés
                                                       dans les champs pdf ou jpg de la table Rufus.Impressions pour les imageries et ComptaMedicale.Factures pour les factures.
                                                       Cette fonction, appelée par le timer t_timerUserConnecte ou par le bouton ui->ExportImagespushButton,
                                                       permet de récupérer le contenu blob de ces fichiers
                                                       et de recréer un fichier d'imagerie stocké dans le système de fichiers du serveur */

    void        FiltreAccueil(int idx);
    void        FiltreSalleDAttente();
    void        GestionComptes();
    void        ImportNouveauDocExterne(AppareilImagerie *appareil);    /*! importe le document d'imagerie qui vient d'être émis par l'appareil nomapp
                                                                       * importe les fichiers d'imagerie au lancement du programme et quand on utilise le QFileSystemWatcher m_filewatcher*/
    void        VerifDocsDossiersEchanges();                          /*! utilisé à la place du QFileSystemWatcher dont le signal directorychanged bugue trop
                                                                       * importe les fichiers d'imagerie quand on utilise le QTimer t_timerfilewatcher */
    void        VerifDossiersImagerie();
    void        VerifImportateur();                             /*! vérifie que le poste importateur des documents externes est valide et le remplace au besoin*/
    void        ImprimeDossier(Patient *pat);
    void        ImprimeListPatients(QVariant var);
    void        ImprimeListActes(QList<Acte *> listeactes, bool toutledossier = true, bool queLePdf = false, QString  nomdossier = "");
    void        LireLaCV();       // CZ001
    void        LireLaCPS();      // CZ001
    void        ListeCorrespondants();
    void        ListeIOLs();
    void        ListeManufacturers();
    void        ListeTiersPayants();
    void        MajusculeCreerNom();
    void        MajusculeCreerPrenom();
    void        ModifCotationActe();
    void        ModifierTerrain();
    void        Moulinette();
    void        NouvelleMesure(Procedures::TypeMesure TypeMesure);
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
    void        Apropos();


    // la liste de cotations en cours d'utilisation
    Cotations*  m_currencotation = Q_NULLPTR;
    Cotations*  currentlistecotations()                     { return m_currencotation; };
    void        setcurrentlistecotations(Cotations* cot)    { m_currencotation = cot; };

    // gestion des patients vus dans la journée
    QTimer      *gTimerPatientsVus;
    void        AffichePatientsVusWidget();
    void        MAJPatientsVus();
    void        MasquePatientsVusWidget();

    // la messagerie

    void        AfficheBAL(int idx = -1);                                                   /*! Affiche la BAL des messages reçus et envoyés sous la barre des menus - dlg_msgDialog */
    void        MsgModif(int);                                                              /*! Gère la modification d'un message émis */
    void        MsgResp(int);                                                               /*! Affiche la fiche de rédaction des réponses - dlg_msgRepons */
    void        ReconstruitListeMessages();                                                 /*! initialise la liste des messages reçus et émis de l'utilisteur */
    void        SendMessage(QMap<QString,QVariant>, int id = -1 , int idMsg = -1);          /*! Affiche la fiche de rédaction des messages - dlg_sendMessage */
    void        SupprimerMessageEmis(int idMsg);
    void        SupprimerMessageRecu(int idJoint);

    QDialog     *dlg_msgBAL = Q_NULLPTR;                                                    /*! la BAL des messages, affichée sous la barre de menus - appelée par AfficheMessages() */
    void        ChoixMenuContextuelIdentPatient();
    void        ChoixMenuContextuelMotsCles();
    void        MenuContextuelBureaux(UpTextEdit *UpText);
    void        MenuContextuelCorrespondant(UpComboBox *box);
    void            ChoixMenuContextuelCorrespondant(QString choix);
    void        MenuContextuelIdentPatient();
    void        MenuContextuelListePatients();
    void            ChoixMenuContextuelListePatients(int idpat, QString);
    void        MenuContextuelMedecin();
    void            ChoixMenuContextuelMedecin();
    void        MenuContextuelMotsCles();
    void        MenuContextuelAccueil(UpLabel *labelCLicked);
    void        MenuContextuelSalDat(UpLabel *labelCLicked);
    void            ChoixMenuContextuelSalDat(int idpat, QString);

    //fonctions lancées par des timers
    void        ActualiseDocsExternes();
    void        MAJPosteConnecte();
    void        VerifCorrespondants();
    void        VerifMessages();
    void        VerifSalleDAttente();
    void        VerifVerrouDossier();

private:

    MesureRefraction        *shortref_acuite       = Q_NULLPTR;
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
    QStandardItemModel      *m_listesuperviseursmodel       = Q_NULLPTR;
    QStandardItemModel      *m_listesuperviseursaccueilmodel= Q_NULLPTR;
    QSortFilterProxyModel   *m_listepatientsproxymodel      = Q_NULLPTR;
    QSortFilterProxyModel   *m_DDNsortmodel                 = Q_NULLPTR;
    QSortFilterProxyModel   *m_prenomfiltersortmodel        = Q_NULLPTR;
    QTabBar                 *wdg_salledattenteTab, *wdg_accueilTab;
    QTimer                  *t_timerSalDat, *t_timerCorrespondants, *t_timerPosteConnecte, *t_timerVerifVerrou, *t_timerSupprDocs, *t_timerVerifImportateurDocs;
    QTimer                  *t_timerExportDocs, *t_timerActualiseDocsExternes, *t_timerVerifMessages;

    Patient*                dossierpatientaouvrir() const           { return Datas::I()->patients->dossierpatientaouvrir(); }
    Patient*                currentpatient() const                  { return Datas::I()->patients->currentpatient(); }
    Acte*                   currentacte()                           { return Datas::I()->actes->currentacte(); }
    User*                   currentuser()                           { return Datas::I()->users->userconnected(); }
    PosteConnecte*          currentpost()                           { return Datas::I()->postesconnectes->currentpost(); }
    Actes                   *m_listeactes                   = Q_NULLPTR;
    LignesPaiements         *m_lignespaiements              = Q_NULLPTR;

    UpDialog                *dlg_rechParMotCle, *dlg_listPatients;

    QDialog                 *dlg_msgRepons = Q_NULLPTR;
    QSystemTrayIcon         *ict_messageIcon;

    ImportDocsExternesThread *m_importdocsexternesthread = Q_NULLPTR;
    bool                    isPosteImport();
    bool                    m_isposteImport = false;
    bool                    m_pasDExportPourLeMoment = false;
    UpSmallButton           *wdg_modifIdentificationupSmallButton;

private:
                    // Connexion objets ---------------------------------------------------------------------------------------------
    void                ConnectSignals();

                    // Fonctions ---------------------------------------------------------------------------------------------------
    void                closeEvent(QCloseEvent *);
    bool                eventFilter(QObject *obj, QEvent *event)  ;
    void                keyPressEvent ( QKeyEvent * event );
    void                AfficheActe(Acte *acte);
    void                AfficheActeCompta(Acte *acte);
    void                AfficheDossier(Patient *pat, int idacte = 0);
    void                AfficheMessageImport(QStringList listmsg, int pause);
    bool                AutorDepartConsult(bool ChgtDossier);
    QString             CalcHtmlIdentificationPatient(Patient *pat);
    QIcon               CalcIconPatient(Patient *pat);                              //!> renvoie l'icone qui représente le patient dans le html et sur le tab
    void                CalcMotsCles(Patient *pat);
    void                CalcNbDossiers();
    QString             CalcToolTipCorrespondant(int);
    void                FiltreTable(QString nom = "", QString prenom = "");         //!> filtrage de la liste des patients en fonction des valeurs correspondant aux zones de saisie
    void                OuvrirDossier(Patient *pat, int idacte = 0);
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
    Cotations*          getListeCotationdByUser(User *usr);                 //! retrouve la liste des cotations d'un utilisateur et l''ajoute à la map des listes de cotations Datas::I()->listecotations
    Patient*            getPatientFromRow(int row);                         //!> retrouve le patient correspondant à la rangée row
    Patient*            getPatientFromSelectionInTable();                   //!> retrouve le patient sélectionné dans la liste des patients
    Patient*            getPatientFromCursorPositionInTable();              //!> retrouve le patient sous le curseur de la souris dans la liste des patients
    Patient*            getPatientFromIndex(QModelIndex idx);               //!> retrouve le patient à partir du modelindex dans la table
    int                 getRowFromPatient(Patient *pat);                    //!> retrouve la rangée où se trouve un patient dans la table;
    bool                IdentificationPatient(dlg_identificationpatient::Mode mode, Patient *pat);
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
    QMap<QString, QVariant>
                        MotifRDV(QString Motif = "", QString ShowMessage = "", QTime heurerdv = QTime::currentTime());
    bool                NavigationConsult(ItemsList::POSITION i);
    void                OuvrirActesPrecedents();
    void                OuvrirDocsExternes(DocsExternes *docs);
    void                ImprimeDocument(Patient *pat);                                             //! ouvre la fiche dlg_impressions et prépare la liste de documents à imprimer
    void                ModeSelectDepuisListe();                                                    //!> Passe en mode sélection depuis la liste de patients
    void                ModeCreationDossier();                                                      //!> Passe en mode création de dossier
    void                ProgrammationIntervention(Patient *pat, Acte *act = Q_NULLPTR);
    void                RecopierDossier(Patient *patient = Q_NULLPTR);
    void                RecaleTableView(Patient *pat, QAbstractItemView::ScrollHint scrollhint = QAbstractItemView::PositionAtCenter);
    int                 RecherchePatient(QString lPatNom, QString lPatPrenom, QString lPatDDN, QString MessageErreur);
    void                RefractionMesure(dlg_refraction::ModeOuverture mode);
    void                ConnectCotationComboBox();  //!> reconnecte la box des cotations à 2 signaux
                                                    //!> si une cotation est choisie, le montant de l'acte est recherché est affiché dans la ligne MontantLineEdit
                                                    //!> un tooltip est affiché décrivant le descriptif de la cotation mise en surbrillance dans la liste déroulante
    void                ReconstruitListesCotations(User* = Datas::I()->users->userconnected());
    void                ReconstruitCombosCorresp(bool reconstruireliste = true);
    void                SetDatasRefractionKerato();
    void                RemiseCheques();
    bool                Remplir_ListePatients_TableView();
    QTabWidget*         Remplir_MsgTabWidget();
    void                Remplir_SalDat();
    bool                RetourSalleDattente(QString Titre);
    void                setTitre();
    void                SortieAppli();
    void                SupprimerActe(Acte *act);
    void                SupprimerDocsEtFactures();
    void                SupprimerDossier(Patient *pat);
    void                AffichePachymetrie();
    void                Pachymetrie();
    void                Tonometrie();
    void                FiltreTableparDDN();
    bool                ValideActeMontantLineEdit(QString NouveauMontant = "0,00", QString AncienMontant = "0.00");
    bool                VerifCoherenceMontantPaiement();        /*! Vérifie que le montant facturé pour l'acte en cours n'est pas inférieur à la somme des paiements déjà enregistrés */

    // TCPServer, TCPSocket
    bool                m_utiliseTCP;
    QStringList         m_listesockets;
    TcpSocket           *TcPConnect;
    void                envoieTCPMessage(QString msg);                  /* envoi d'un message au serveur pour être redispatché vers tous les clients */
    void                envoieTCPMessageA(QList<int> listidusr);        /* envoi d'un message à une liste d'utilisateurs */
    void                TraiteTCPMessage(QString msg);                  /* traitement des messages reçus par les clients */
    void                TesteConnexion();
    QString             m_resumeStatut;

    // Les menus
    QMenu           *menuActe                       = Q_NULLPTR;
    QMenu           *menuComptabilite               = Q_NULLPTR;
    QMenu           *menuEdition                    = Q_NULLPTR;
    QMenu           *menuDocuments                  = Q_NULLPTR;
    QMenu           *menuDossier                    = Q_NULLPTR;
    QMenu           *menuEmettre                    = Q_NULLPTR;
    QMenu           *menuPrecedentsActes            = Q_NULLPTR;
    QMenu           *menuAide                       = Q_NULLPTR;
    QAction         *actionCreerDossier             = Q_NULLPTR;
    QAction         *actionCreerActe                = Q_NULLPTR;
    QAction         *actionOuvrirDossier            = Q_NULLPTR;
    QAction         *actionEmettreDocument          = Q_NULLPTR;
    QAction         *actionRecopierDossier          = Q_NULLPTR;
    QAction         *actionParametres               = Q_NULLPTR;
    QAction         *actionSupprimerActe            = Q_NULLPTR;
    QAction         *actionSupprimerDossier         = Q_NULLPTR;
    QAction         *actionRechercheParMotCle       = Q_NULLPTR;
    QAction         *actionRechercheParID           = Q_NULLPTR;
    QAction         *actionDossierPatient           = Q_NULLPTR;
    QAction         *actionCorrespondants           = Q_NULLPTR;
    QAction         *actionEnregistrerDocScanner    = Q_NULLPTR;
    QAction         *actionEnregistrerVideo         = Q_NULLPTR;
    QAction         *actionRechercheCourrier        = Q_NULLPTR;
    QAction         *actionExportActe               = Q_NULLPTR;
    QAction         *actionGestionComptesBancaires  = Q_NULLPTR;
    QAction         *actionPaiementDirect           = Q_NULLPTR;
    QAction         *actionPaiementTiers            = Q_NULLPTR;
    QAction         *actionRecettesSpeciales        = Q_NULLPTR;
    QAction         *actionResumeStatut             = Q_NULLPTR;
    QAction         *actionBilanRecettes            = Q_NULLPTR;
    QAction         *actionJournalDepenses          = Q_NULLPTR;
    QAction         *actionRemiseCheques            = Q_NULLPTR;
    QAction         *actionApropos                  = Q_NULLPTR;
    QAction         *actionQuit                     = Q_NULLPTR;
};

#endif // RUFUS_H
