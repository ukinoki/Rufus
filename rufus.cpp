/* (C) 2016 LAINE SERGE
This file is part of Rufus.

Rufus is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Rufus is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Rufus. If not, see <http://www.gnu.org/licenses/>.
*/

#include "rufus.h"
#include "ui_rufus.h"

Rufus::Rufus(QWidget *parent) : QMainWindow(parent)//, ui(new Ui::Rufus)

/*--------------------------------------------------------------------------------------------------------------
-- Création de la fiche ----------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------------------------------*/
{
    // la version du programme correspond à la date de publication, suivie de "/" puis d'un sous-n° - p.e. "23-6-2017/3"
    qApp->setApplicationVersion("29-05-2018/1");       // doit impérativement être composé de date version / n°version;

    ui = new Ui::Rufus;
    ui->setupUi(this);
    setWindowFlags(Qt::Window | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint);

    QString border = "border-image: url(://wallpaper.jpg)";
    qApp->setStyleSheet(
        "QDialog{" + border + "}"
        "QGroupBox{font:bold;}"
        "QLineEdit {background-color:white; border: 1px solid rgb(150,150,150);border-radius: 5px;}"
        "QLineEdit:focus {border: 1px solid rgb(164, 205, 255);border-radius: 5px;}"
        "QRadioButton::indicator {width: 18px; height: 18px;}"
        "QRadioButton::indicator::checked {image: url(://blueball.png);}"
        "QScrollArea {background-color:rgb(237, 237, 237); border: 1px solid rgb(150,150,150);}"
        "QTabBar::tab:selected {background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #fafafa, stop: 1.0 rgb(164, 205, 255));}"
        "QTabBar::tab:hover {background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #fafafa, stop: 0.4 #f4f4f4,stop: 0.5 #e7e7e7, stop: 1.0 #fafafa);}"
        "QTabBar::tab:selected {border-color: #9B9B9B; border-bottom-color: #C2C7CB;}"
        "QTabBar::tab:!selected {margin-top: 4px;}"
        "QTabBar::tab {background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #E0E0E0, stop: 1 #FFFFFF);"
                 "border: 1px solid #C4C4C3;"
                 "border-bottom-color: #C2C7CB;"
                 "border-top-left-radius: 4px; border-top-right-radius: 4px;"
                 "min-width: 8ex;"
                 "padding: 2px;}"
        "QTabWidget::pane {border-top: 1px solid #C2C7CB;}"
        "QTabWidget::tab-bar {left: 20px;}"
        "QTextEdit {background-color:white; border: 1px solid rgb(150,150,150); border-radius: 10px;}"
        "QTextEdit:focus {border: 2px solid rgb(164, 205, 255);border-radius: 10px;}"
        "QTreeWidget{background-color: white;}"
        "QToolBar {border-radius: 5px; margin-left: 5px; margin-right: 5px;  margin-top: 3px; margin-bottom: 3px;}"
        "QWidget#centralWidget{" + border + "}"
        "QSlider::groove:horizontal {"
                 "border: 1px solid #999999;"
                 "height: 8px;" /* the groove expands to the size of the slider by default. by giving it a height, it has a fixed size */
                 "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #B1B1B1, stop:1 #c4c4c4);"
                 "margin: 2px 0;}"
        "QSlider::handle:horizontal {"
                 "background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #b4b4b4, stop:1 #8f8f8f);"
                 "border: 1px solid #5c5c5c;"
                 "width: 18px;"
                 "margin: -2px 0;" /* handle is placed by default on the contents rect of the groove. Expand outside the groove */
                 "border-radius: 3px;}");

    proc = new Procedures(this);

    if (!proc->Init())
        exit(0);

    //0. Connexion à la base et récupération des données utilisateur
    if (!proc->gdbOK)
    {
        bool    a;
        int     b = 0;
        if (proc->gsettingsIni->value("BDD_POSTE/Active").toString()    == "YES")       b += 1;
        if (proc->gsettingsIni->value("BDD_LOCAL/Active").toString()    == "YES")       b += 1;
        if (proc->gsettingsIni->value("BDD_DISTANT/Active").toString()  == "YES")       b += 1;
        a = (b>1);
        if (b==1)
            if (!proc->Connexion_A_La_Base())
                exit(0);
        while (a)
        {
            if (proc->Connexion_A_La_Base()) break;
            if (!proc->FicheChoixConnexion())
                exit(0);
        }
    }
    db      = proc->getDataBase();

    proc->setDirImagerie();
    proc->getAvecCompta();
    proc->setListeVilles();
    proc->setListeCP();
    setWindowIcon(proc->giconSunglasses);
    //1. Restauration de la position de la fenetre et de la police d'écran
    restoreGeometry(proc->gsettingsIni->value("PositionsFiches/Rufus").toByteArray());

    gidUser = proc->getidUser();
    if (gidUser == -1)
    {
        //Slot_OuvrirIdentification();
        gidUser = proc->gsettingsIni->value("idUtilisateur/idUser").toInt();
    }

    gIdentificationOK = (gidUser > -1);
    if (!gIdentificationOK)
    {
        UpMessageBox::Watch(this, tr("Pas d'utilisateur identifié!\nSortie du programme"));
        exit(0);
    }


    if (!ChargeDataUser())
    {
        UpMessageBox::Watch(this, tr("Impossible de charger les données de l'utilisateur!\nSortie du programme"));
        exit(0);
    }

    qApp->setFont(proc->AppFont());
    proc->Message(gDataUser["Statut"].toString(), 6000);

    //dlg_GestionLieux(db, NOM_TABLE_LIEUXEXERCICE, NOM_TABLE_JOINTURESLIEUX, QDir::homePath() + NOMFIC_INI).exec();

    //2 Initialisation de tout
    gUserDateDernierMessage = QDateTime(gUserdateDerniereConnexion);
    InitDivers();
    InitEventFilters();
    InitVariables();
    InitTables();
    CreerMenu();
    InitMenus();

    QString modeconnexion = "";
    if (proc->getModeConnexion() == Procedures::Poste)
        modeconnexion = tr("monoposte");
    else if (proc->getModeConnexion() == Procedures::ReseauLocal)
        modeconnexion = tr("réseau local");
    if (proc->getModeConnexion() == Procedures::Distant)
    {
        modeconnexion = tr("accès distant - connexion ");
        if (proc->gsettingsIni->value("BDD_DISTANT/SSL").toString() != "NO")
            modeconnexion += tr("cryptée (SSL)");
        else
            modeconnexion += tr("non cryptée");
    }

    setWindowTitle("Rufus - " + gUserLogin + " - " + gUserFonction + " - " + modeconnexion + " - " + qApp->applicationVersion());

    gidPatient = idPatientAuHasard;

    OuvrirListe();
    for (int abc = 1; abc < ui->tabWidget->count(); abc++)
        ui->tabWidget->removeTab(abc);

    //    Lancement du timer de scrutation des modifications de la salle d'attente
    gTimerSalleDAttente = new QTimer(this);
    if (proc->getModeConnexion() == Procedures::Distant)
        gTimerSalleDAttente->start(10000);
    else
        gTimerSalleDAttente->start(1000);

    // Lancement du timer de mise à jour des utilisateurs connectés
    gTimerUserConnecte = new QTimer(this);
    gTimerUserConnecte->start(10000);// "toutes les 10 secondes"

    // Lancement du timer de vérification du gestionnaire de documents
    gTimerVerifGestDocs = new QTimer(this);
    if (proc->getModeConnexion() == Procedures::Distant)
        gTimerVerifGestDocs->start(60000);// "toutes les 60 secondes"
    else
        gTimerVerifGestDocs->start(10000);// "toutes les 10 secondes"

    // Lancement du timer de vérification des verrous - +++ à lancer après le timer gTimerVerifGestDocs puisqu'il l'utilise
    gTimerVerifVerrou = new QTimer(this);
    gTimerVerifVerrou->start(60000);// "toutes les 60 secondes"

    // Lancement du timer de suppression des fichiers documents inutiles
    gTimerSupprDocs = new QTimer(this);
    if (proc->getModeConnexion() != Procedures::Distant)
    {
        gTimerSupprDocs->start(60000);// "toutes les 60 secondes"
        connect(gTimerSupprDocs, SIGNAL(timeout()), this, SLOT(Slot_SupprimerDocs()));
    }

    Slot_MetAJourLaConnexion();
    Slot_VerifSalleDAttente();
    Slot_VerifVerrouDossier();
    Slot_ActualiseDocsExternes();
    proc->UpdVerrouSalDat();
    setFixedWidth(LARGEURLISTE);
    ui->tabWidget->setGeometry(5,10,-10,920);
    ToolBarListe();

    //Nettoyage des erreurs éventuelles de la salle d'attente
    QString blabla              = ENCOURSEXAMEN;
    int length                  = blabla.size();
    QString req = "UPDATE " NOM_TABLE_SALLEDATTENTE
                    " SET Statut = '" ARRIVE "', idUserEnCoursExam = null, PosteExamen = null"
                    " WhERE idUser = " + QString::number(gidUser) +
                    " AND Left(Statut," + QString::number(length) + ") = '" ENCOURSEXAMEN "'";
    QSqlQuery (req,db);

    TerraintreeWidget         = new QTreeWidget(ui->AtcdstMedxframe);
    ModifTerrainupSmallButton = new UpSmallButton(ui->AtcdstMedxframe);
    ModifTerrainupSmallButton ->setUpButtonStyle(UpSmallButton::EDITBUTTON);
    ModifTerrainupSmallButton ->setGeometry(330,640,30,30);
    ModifTerrainupSmallButton ->setIconSize(QSize(30,30));

    // les slots
    Connect_Slots();

    //libération des verrous de la compta
    req = " delete from " NOM_TABLE_VERROUCOMPTAACTES " where PosePar = " + QString::number(gidUser);
    QSqlQuery lib(req,db);
    proc->TraiteErreurRequete(lib,req,tr("impossible de libérer les verrous de la comptabilité"));

    if (gListePatientsModel->rowCount() == 0)
    {
        OuvrirNouveauDossier();
        ui->LListepushButton->setEnabled(false);
        UpMessageBox::Watch(this,tr("Vous n'avez aucun dossier de patient enregistré!"), tr("Vous devez d'abord en créer un."));
    }

    closeFlag = false;
    gIPadr = "";
    QHostInfo::lookupHost(QHostInfo::localHostName(), this, SLOT(Slot_CalcIP(QHostInfo)));
    connect(gTimerVerifGestDocs, SIGNAL(timeout()), this, SLOT(Slot_ImportDocsExternes()));
    if (proc->getModeConnexion() != Procedures::Distant)
        connect(gTimerVerifGestDocs, SIGNAL(timeout()), this, SLOT(Slot_ExporteDocs()));
    if (QSystemTrayIcon::isSystemTrayAvailable())
    {
        Slot_VerifMessages();
        connect(gTimerVerifGestDocs, SIGNAL(timeout()), this, SLOT(Slot_VerifMessages()));
    }
}

Rufus::~Rufus()
{
    delete ui;
}

void Rufus::closeEvent(QCloseEvent *event)
{
    if (proc->gdbOK)
    {
        if (AutorSortieAppli())
            exit(0);
        else
            event->ignore();
    }
    else
        exit(0);
}

/*-----------------------------------------------------------------------------------------------------------------
-- Connexion des actions associees a chaque objet du formulaire et aux menus --------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void Rufus::Connect_Slots()
{
    // Les objets -------------------------------------------------------------------------------------------------
    QList<UpTextEdit*> listuptextedits = findChildren<UpTextEdit*>();
    for (int i=0; i<listuptextedits.size(); i++)
        if (listuptextedits.at(i)->contextMenuPolicy() == Qt::CustomContextMenu)
            connect (listuptextedits.at(i),                         SIGNAL(customContextMenuRequested(QPoint)),         this,       SLOT (Slot_MenuContextuelUptextEdit()));

    connect (ui->AccueilupTableWidget,                              SIGNAL(currentCellChanged(int,int,int,int)),        this,       SLOT (Slot_ActiveActeAccueil(int)));
    connect (ui->ActePrecedentpushButton,                           SIGNAL(clicked()),                                  this,       SLOT (Slot_NavigationActePrecpushButtonClicked()));
    connect (ui->ActeSuivantpushButton,                             SIGNAL(clicked()),                                  this,       SLOT (Slot_NavigationActeSuivpushButtonClicked()));
    connect (ui->ChgUserpushButton,                                 SIGNAL(clicked()),                                  this,       SLOT (Slot_IdentificationUser()));
    connect (ui->CourrierAFairecheckBox,                            SIGNAL(clicked()),                                  this,       SLOT (Slot_CourrierAFaireChecked()));
    connect (ui->CreerActepushButton,                               SIGNAL(clicked()),                                  this,       SLOT (Slot_CreerActe()));
    connect (ui->CreerActepushButton_2,                             SIGNAL(clicked()),                                  this,       SLOT (Slot_CreerActe()));
    connect (ui->CreerBOpushButton,                                 SIGNAL(clicked()),                                  this,       SLOT (Slot_CreerBilanOrtho()));
    connect (ui->CreerBOpushButton_2,                               SIGNAL(clicked()),                                  this,       SLOT (Slot_CreerBilanOrtho()));
    connect (ui->CreerDDNdateEdit,                                  SIGNAL(dateChanged(QDate)),                         this,       SLOT (Slot_TrouverDDN()));
    connect (ui->ChercherDepuisListepushButton,                     SIGNAL(clicked()),                                  this,       SLOT (Slot_ChercherDepuisListepushButtonClicked()));
    connect (ui->CreerNomlineEdit,                                  SIGNAL(textEdited(QString)),                        this,       SLOT (Slot_MajusculeCreerNom()));
    connect (ui->CreerPrenomlineEdit,                               SIGNAL(textEdited(QString)),                        this,       SLOT (Slot_MajusculeCreerPrenom()));
    connect (ui->CreerDossierpushButton,                            SIGNAL(clicked()),                                  this,       SLOT (Slot_CreerDossierpushButtonClicked()));
    connect (ui->DernierActepushButton,                             SIGNAL(clicked()),                                  this,       SLOT (Slot_NavigationDernierActepushButtonClicked()));
    connect (ui->FermepushButton,                                   SIGNAL(clicked()),                                  this,       SLOT (close()));
    connect (ui->FiltrecheckBox,                                    SIGNAL(clicked()),                                  this,       SLOT (Slot_FiltrecheckBoxClicked()));
    connect (ui->FSEpushButton,                                     SIGNAL(clicked()),                                  this,       SLOT (Slot_SaisieFSE()));       // CZ001
    connect (ui->IdentPatienttextEdit,                              SIGNAL(customContextMenuRequested(QPoint)),         this,       SLOT (Slot_MenuContextuelIdentPatient()));
    connect (ui->LFermepushButton,                                  SIGNAL(clicked()),                                  this,       SLOT (close()));
    connect (ui->ListepushButton,                                   SIGNAL(clicked()),                                  this,       SLOT (Slot_OuvrirListepushButtonClicked()));
    connect (ui->LListepushButton,                                  SIGNAL(clicked()),                                  this,       SLOT (Slot_OuvrirListepushButtonClicked()));
    connect (ui->LNouvDossierpushButton,                            SIGNAL(clicked()),                                  this,       SLOT (Slot_OuvrirNouveauDossierpushButtonClicked()));
    connect (ui->LRecopierpushButton,                               SIGNAL(clicked()),                                  this,       SLOT (Slot_RecopierDossierpushButtonClicked()));
    connect (ui->SendMessagepushButton,                             SIGNAL(clicked()),                                  this,       SLOT (Slot_SendMessage()));
    connect (ui->LSendMessagepushButton,                            SIGNAL(clicked()),                                  this,       SLOT (Slot_SendMessage()));
    connect (ui->MGupComboBox,                                      SIGNAL(activated(int)),                             this,       SLOT (Slot_ChoixMG()));
    connect (ui->AutresCorresp1upComboBox,                          SIGNAL(activated(int)),                             this,       SLOT (Slot_ChoixCor()));
    connect (ui->AutresCorresp2upComboBox,                          SIGNAL(activated(int)),                             this,       SLOT (Slot_ChoixCor()));
    connect (ui->MGupComboBox,                                      SIGNAL(customContextMenuRequested(QPoint)),         this,       SLOT (Slot_MenuContextuelMedecin()));
    connect (ui->AutresCorresp1upComboBox,                          SIGNAL(customContextMenuRequested(QPoint)),         this,       SLOT (Slot_MenuContextuelCorrespondant()));
    connect (ui->AutresCorresp2upComboBox,                          SIGNAL(customContextMenuRequested(QPoint)),         this,       SLOT (Slot_MenuContextuelCorrespondant()));
    connect (ui->ModifDatepushButton,                               SIGNAL(clicked()),                                  this,       SLOT (Slot_ModifActeDate()));
    connect (ui->ModifIdentificationSmallButton,                    &QPushButton::clicked,                              [=] {ChoixMenuContextuelIdentPatient();});
    connect (ModifTerrainupSmallButton,                             SIGNAL(clicked()),                                  this,       SLOT (Slot_ModifierTerrain()));
    connect (ui->MotsClesLabel,                                     SIGNAL(customContextMenuRequested(QPoint)),         this,       SLOT (Slot_MenuContextuelMotsCles()));
    connect (ui->MotsClesupSmallButton,                             &QPushButton::clicked,                              [=] {ChoixMenuContextuelMotsCles();});
    connect (ui->OKModifTerrainupSmallButton,                       SIGNAL(clicked()),                                  this,       SLOT (Slot_OKModifierTerrain()));
    connect (ui->NouvDossierpushButton,                             SIGNAL(clicked()),                                  this,       SLOT (Slot_OuvrirNouveauDossierpushButtonClicked()));
    connect (ui->OuvreActesPrecspushButton,                         SIGNAL(clicked()),                                  this,       SLOT (Slot_OuvrirActesPrecspushButtonClicked()));
    connect (ui->OuvreDocsExternespushButton,                       SIGNAL(clicked()),                                  this,       SLOT (Slot_OuvrirDocsExternes()));
    connect (ui->OuvrirDocumentpushButton,                          SIGNAL(clicked()),                                  this,       SLOT (Slot_OuvrirDocuments()));
    connect (ui->PatientsListeTableView,                            SIGNAL(customContextMenuRequested(QPoint)),         this,       SLOT (Slot_MenuContextuelListePatients(QPoint)));
    connect (ui->PatientsListeTableView,                            SIGNAL(doubleClicked(QModelIndex)),                 this,       SLOT (Slot_ChoixDossier()));
    connect (ui->PatientsListeTableView,                            SIGNAL(entered(QModelIndex)),                       this,       SLOT (Slot_AfficheToolTip(QModelIndex)));
    connect (ui->PatientsListeTableView->selectionModel(),          SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this,  SLOT (Slot_EnableCreerDossierButton()));
    connect (ui->SalleDAttenteupTableWidget,                        SIGNAL(cellDoubleClicked(int,int)),                 this,       SLOT (Slot_RetrouveiDDepuisTab(int,int)));
    connect (ui->PatientsVusupTableWidget,                          SIGNAL(cellDoubleClicked(int,int)),                 this,       SLOT (Slot_RetrouveiDDepuisTab(int,int)));
    connect (ui->PremierActepushButton,                             SIGNAL(clicked()),                                  this,       SLOT (Slot_NavigationPremierActepushButtonClicked()));
    connect (ui->RefractionpushButton,                              SIGNAL(clicked()),                                  this,       SLOT (Slot_Refraction()));
    connect (ui->SalleDAttentepushButton,                           SIGNAL(clicked()),                                  this,       SLOT (Slot_SalleDAttente()));
    connect (ui->SalleDAttenteupTableWidget,                        SIGNAL(dropsignal(QByteArray)),                     this,       SLOT (Slot_DropPatient(QByteArray)));
    connect (ui->SupprimeActepushButton,                            SIGNAL(clicked()),                                  this,       SLOT (Slot_SupprimerActepushButtonClicked()));
    connect (ui->TonometriepushButton,                              SIGNAL(clicked()),                                  this,       SLOT (Slot_Tonometrie()));
    connect (ui->VitaleupPushButton,                                SIGNAL(clicked()),                                  this,       SLOT (Slot_LireLaCVpushButtonClicked())); // CZ001

    connect (ui->ActeCotationcomboBox->lineEdit(),                  SIGNAL(textEdited(QString)),                        this,       SLOT (Slot_RetrouveMontantActe()));
    connect (ui->ActeCotationcomboBox,                              SIGNAL(currentIndexChanged(QString)),               this,       SLOT (Slot_RetrouveMontantActe()));
    connect (ui->ActeMontantlineEdit,                               SIGNAL(TextModified(QString)),                      this,       SLOT (Slot_ActeMontantModifie()));
    connect (ui->BasculerMontantpushButton,                         SIGNAL(clicked(bool)),                              this,       SLOT (Slot_BasculerMontantActe()));
    connect (ui->CCAMlinklabel,                                     SIGNAL(linkActivated(QString)),                     this,       SLOT (Slot_CCAM(QString)));
    connect (ui->ModifierCotationActepushButton,                    SIGNAL(clicked()),                                  this,       SLOT (Slot_ModfiCotationActe()));
    // Les tabs --------------------------------------------------------------------------------------------------
    connect (ui->tabWidget,                                         SIGNAL(currentChanged(int)),                        this,       SLOT (Slot_ChangeTabBureau()));

    // les timers -------------------------------------------------------------------------------------------------
    connect (gTimerSalleDAttente,                                   SIGNAL(timeout()),                                  this,       SLOT(Slot_VerifSalleDAttente()));
    connect (gTimerUserConnecte,                                    SIGNAL(timeout()),                                  this,       SLOT(Slot_MetAJourLaConnexion()));
    connect (gTimerVerifVerrou,                                     SIGNAL(timeout()),                                  this,       SLOT(Slot_VerifVerrouDossier()));
    connect (gTimerVerifGestDocs,                                   SIGNAL(timeout()),                                  this,       SLOT(Slot_ActualiseDocsExternes()));
    connect (proc,                                                  SIGNAL(ConnectTimers(bool)),                        this,       SLOT(Slot_ConnectTimers(bool)));

    // les signaux personnalisés ----------------------------------------------------------------------------------
    connect(this,                                                   SIGNAL(EnregistrePaiement(QString)),                this,       SLOT(Slot_AppelPaiementDirect(QString)));

    // MAJ Salle d'attente ----------------------------------------------------------------------------------
    connect(proc,                                                   SIGNAL(UpdSalDat()),                                this,       SLOT(Slot_MAJ_SalleDAttente()));

    // Nouvelle mesure d'appareil de refraction ----------------------------------------------------------------------------------
    if (proc->PortFronto()!=NULL || proc->PortAutoref()!=NULL || proc->PortRefracteur()!=NULL)
        connect(proc,                                               SIGNAL(NouvMesureRefraction()),                     this,       SLOT(Slot_NouvMesureRefraction()));

    connect (ui->MoulinettepushButton,                              SIGNAL(clicked()),                                  this,       SLOT(Slot_Moulinette()));
    ui->MoulinettepushButton->setVisible(false);
    ui->ChgUserpushButton   ->setVisible(false);
}

void Rufus::Slot_OuvrirDocsExternes()
{
    OuvrirDocsExternes(gidPatient);  //depuis OuvreDocsExternespushbutton
}
void Rufus::OuvrirDocsExternes(int idpat, bool depuismenu)
{
    if (!depuismenu)  // -> depuis gTimerVerifGestDocs, AfficheDossier() ou ui->OuvreDocsExternespushButton
    {
        QList<dlg_docsexternes *> ListDialogDocs = this->findChildren<dlg_docsexternes *>();
        if (ListDialogDocs.size()>0)
            return;
    }
    QString req = "Select idImpression from " NOM_TABLE_IMPRESSIONS " where idpat = " + QString::number(idpat);
    QSqlQuery quer(req,proc->getDataBase());
    if (quer.size()>0)
    {
        Dlg_DocsExt = new dlg_docsexternes(proc,idpat, this);
        ui->OuvreDocsExternespushButton->setEnabled(true);
        QString patNom;
        QString patPrenom;
        if (depuismenu)
        {
            QString autrerequete = "select PatNom, PatPrenom from " NOM_TABLE_PATIENTS " where idPat = " + QString::number(idpat);
            QSqlQuery autrequery (autrerequete, db);
            if (proc->TraiteErreurRequete(autrequery,autrerequete,""))
                return;
            if (autrequery.size() == 0)
            {
                UpMessageBox::Watch(this, tr("Pas de consultation enregistrée pour ce patient"));
                return;
            }
            autrequery.first();
            patNom      = autrequery.value(0).toString();
            patPrenom   = autrequery.value(1).toString();
        }
        else
        {
            patNom      = gNomPatient;
            patPrenom   = gPrenomPatient;
        }
        Dlg_DocsExt->setWindowTitle(tr("Documents de ") + patPrenom + " " + patNom);
        Dlg_DocsExt->show();
        Dlg_DocsExt->Slot_AfficheDoc(Dlg_DocsExt->ListDocsTreeView->selectionModel()->currentIndex());
        if (depuismenu)
            Dlg_DocsExt->setModal(true); //quand la fiche est ouverte depuis le menu contectuel de la liste des patients
    }
    else
        if (!depuismenu)
            ui->OuvreDocsExternespushButton->setEnabled(false);
}

/*-----------------------------------------------------------------------------------------------------------------
-- // mettre à jour dlg_actesprecedents aund le contenu de l'acte affiché a été modifié  --------------------------
-----------------------------------------------------------------------------------------------------------------*/
void Rufus::MAJActesPrecs()
{
    QList<dlg_actesprecedents *> ListDialog = this->findChildren<dlg_actesprecedents *>();
    for (int n = 0; n <  ListDialog.size(); n++)
        if (ListDialog.at(n)->getActeAffiche() == gidActe)
        {
            ListDialog.at(n)->ActesPrecsAfficheActe(gidActe);
            break;
        }
}

/*-----------------------------------------------------------------------------------------------------------------
-- // mettre à jour dlg_docsexternes   ----------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void Rufus::MAJDocsExternes()
{
    QList<dlg_docsexternes *> ListDialogDocs = this->findChildren<dlg_docsexternes *>();
    if (ListDialogDocs.size()>0)
        ListDialogDocs.at(0)->RemplirTreeView();
    else
    {
        QString req = "Select idImpression from " NOM_TABLE_IMPRESSIONS " where idpat = " + QString::number(gidPatient);
        QSqlQuery quer(req,proc->getDataBase());
        if (quer.size()>0)
        {
            Dlg_DocsExt = new dlg_docsexternes(proc,gidPatient, this);
            ui->OuvreDocsExternespushButton->setEnabled(true);
            Dlg_DocsExt->setWindowTitle(tr("Documents de ") + gPrenomPatient + " " + gNomPatient);
            Dlg_DocsExt->show();
            Dlg_DocsExt->Slot_AfficheDoc(Dlg_DocsExt->ListDocsTreeView->selectionModel()->currentIndex());
        }
        else
            ui->OuvreDocsExternespushButton->setEnabled(false);
    }
}


/*-----------------------------------------------------------------------------------------------------------------
-- La moulinette ne sert qu'à tester des fonctions et n'est pas utilisée par le programe        ------------------
-----------------------------------------------------------------------------------------------------------------*/
void Rufus::Slot_Moulinette()
{
    /*QString req, str;
    QTextEdit txt;
    req = "select idimpression, Titre from " NOM_TABLE_IMPRESSIONS " where soustypedoc = null and typedoc = 'DocRecu'";
    //qDebug() << req;
    QSqlQuery idquer (req,db);
    idquer.first();
    for (int i=0; i<idquer.size(); i++)
    {
        txt.setHtml(idquer.value(1).toString());
        str = txt.toPlainText();
        req = "update " NOM_TABLE_IMPRESSIONS " set soustypedoc = '" + str + "', Formatdoc = 'Prescription', emisrecu = 1 where idimpression = " + idquer.value(0).toString();
        //qDebug() << req;
        QSqlQuery (req,db);
        idquer.next();
    }*/
    /*// REGROUPEMENT DES OCT DANS UN SEUL DOSSIER ============================================================================================================================================================
    QString StartDir, DestDir, List;
    QStringList ListDir;
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setViewMode(QFileDialog::List);
    if (dialog.exec()>0)
    {
        QDir dockdir = dialog.directory();
        StartDir = dockdir.absolutePath();
        QStringList flist = dockdir.entryList(QDir::AllDirs);
        for (int i=0; i<flist.size(); i++)
        {
            if (!flist.at(i).contains("."))
            {
                ListDir << StartDir + "/" + flist.at(i);
                List += StartDir + "/" + flist.at(i) + "\n";
            }
        }
    }
    if (dialog.exec()>0)
    {
        QDir dockdir = dialog.directory();
        DestDir = dockdir.absolutePath();
    }
    for (int i = 0; i<ListDir.size(); i++)
    {
        QStringList listfile = QDir(ListDir.at(i)).entryList(QDir::Files);
        for (int j=0; j<listfile.size();j++)
        {
            QFile source(ListDir.at(i) + "/" + listfile.at(j));
            QString newname = listfile.at(j).toUpper();
            newname.replace("- ","-");
            QString destination = DestDir + "/" + newname;
            if (source.exists())
            {
                //proc->Edit("Source = " + ListDir.at(i) + "/" + listfile.at(j) + "\nDestination = " + destination);
                source.copy(destination);
            }
        }
    }
    QStringList list2 = QDir(StartDir).entryList(QDir::Files);
    for (int k=0; k<list2.size();k++)
    {
        QFile source(StartDir + "/" + list2.at(k));
        QString newname = list2.at(k).toUpper();
        newname.replace("- ","-");
        QString destination = DestDir + "/" + newname;
        if (source.exists())
        {
            //proc->Edit("Source = " + ListDir.at(i) + "/" + listfile.at(j) + "\nDestination = " + destination);
            source.copy(destination);
        }
    }*/

    /*
    //CONVERSION DES BASES OPHTALOGIC ============================================================================================================================================================
    int max = 1000;
    QString req;
    conversionbase *convbase = new conversionbase(proc,"");
    delete convbase;
    Remplir_ListePatients_TableView(grequeteListe,"","");
    */

    /*
    // SUPPRESSION DES RETOURS A LA LIGNE DANS LES CHAMPS TEXTE DE ACTES ET IMPRESSIONS ============================================================================================================================================================
    QString req = "select idActe, ActeTexte from " NOM_TABLE_ACTES " order by idacte desc";
    QSqlQuery imp(req,db);
    imp.first();
    int b = 0;
    int max = imp.size();
    for (int i = 0; i<max; i++)
    {
        QString texte ="";
        texte = imp.value(1).toString();
        //proc->Edit(texte);
        QString texte2 = texte.remove("<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><br /></p>");
        texte2.replace("<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">","<p style=\" margin-top:0px; margin-bottom:0px;\">");
        texte2.remove("border=\"0\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px;\" ");
        //proc->Edit(texte + "\n-\n" + texte2 + "\nfin");
        req = "update " NOM_TABLE_ACTES " set actetexte = '" + proc->CorrigeApostrophe(texte2) + "' where idacte = " + imp.value(0).toString();
        QSqlQuery modif(req,db);
        proc->TraiteErreurRequete(modif, req,"");
        imp.next();
        if (b==10)
            b=0;
        if (b==0)
        {
            QTime dieTime= QTime::currentTime().addMSecs(1);
            while (QTime::currentTime() < dieTime)
                QCoreApplication::processEvents(QEventLoop::AllEvents, 1);
            proc->Message("correction des actes - acte n° " + imp.value(0).toString());
        }
        b+=1;
    }
    req = "select idimpression, textCorps from " NOM_TABLE_IMPRESSIONS " order by idimpression desc";
    QSqlQuery imp2(req,db);
    imp2.first();
    max = imp2.size();
    for (int i = 0; i<max; i++)
    {
        QString texte ="";
        texte = imp2.value(1).toString();
        //proc->Edit(texte);
        QString texte2 = texte.remove("<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><br /></p>");
        texte2.replace("<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">","<p style=\" margin-top:0px; margin-bottom:0px;\">");
        texte2.remove("border=\"0\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px;\" ");
        //proc->Edit(texte + "\n-\n" + texte2 + "\nfin");
        req = "update " NOM_TABLE_IMPRESSIONS " set textcorps = '" + proc->CorrigeApostrophe(texte2) + "' where idimpression = " + imp2.value(0).toString();
        QSqlQuery modif(req,db);
        proc->TraiteErreurRequete(modif, req,"");
        imp2.next();
        if (b==10)
            b=0;
        if (b==0)
        {
            QTime dieTime= QTime::currentTime().addMSecs(1);
            while (QTime::currentTime() < dieTime)
                QCoreApplication::processEvents(QEventLoop::AllEvents, 1);
            proc->Message("correction des impressions - impression n° " + imp2.value(0).toString());
        }
        b+=1;
    }
    */

    /*
    // MODIFICATION DES TABLES CCAM ============================================================================================================================================================
    QString req= "select codeCCAM, modificateur, montant from ccam.ccamd";
    QSqlQuery quer(req,db);
    for (int i=0; i< quer.size(); i++)
    {
        quer.seek(i);
        if (quer.value(1).toInt()==1)
            req = "update ccam.ccamd set OPTAM = " + quer.value(2).toString().replace(",",".") + " where codeCCAM = '" + quer.value(0).toString() + "'";
        else if (quer.value(1).toInt()==2)
            req = "update ccam.ccamd set NoOPTAM = " + quer.value(2).toString().replace(",",".") + " where codeCCAM = '" + quer.value(0).toString() + "'";
        //proc->Edit(req);
        QSqlQuery(req,db);
    }
    */

    /*
    // CORRECTION DE LA BASE VILLES - ELIMINATION DES TIRETS ============================================================================================================================================================
    QString req = "select patville from " NOM_TABLE_DONNEESSOCIALESPATIENTS;
    QSqlQuery quer(req,db);
    quer.first();
    for (int i=0;i<quer.size();i++)
    {
        QString ville = quer.value(0).toString();
        if (ville.contains("-"))
        {
        req = "update " NOM_TABLE_DONNEESSOCIALESPATIENTS " set patVille = '" + proc->CorrigeApostrophe(ville.replace("-"," ")) + "' where patville = '" + proc->CorrigeApostrophe(quer.value(0).toString()) + "'";
        QSqlQuery(req,db);
        }
        quer.next();
    }
    req = "select corville from " NOM_TABLE_CORRESPONDANTS;
    QSqlQuery quer1(req,db);
    quer1.first();
    for (int i=0;i<quer1.size();i++)
    {
        QString ville = quer1.value(0).toString();
        if (ville.contains("-"))
        {
        req = "update " NOM_TABLE_CORRESPONDANTS " set corVille = '" + proc->CorrigeApostrophe(ville.replace("-"," ")) + "' where corville = '" + proc->CorrigeApostrophe(quer1.value(0).toString()) + "'";
        QSqlQuery(req,db);
        }
        quer1.next();
    }
    proc->Edit("OK pour villes");*/


    // CREATION D'UNE BASE FACTICE ============================================================================================================================================================
    //Mélange les noms, et 1ère ligne d'adresse dans la base
    int idauhasard;
    QString copierequete = "drop table if exists rufus.patients2;\n";
    copierequete += "create table rufus.patients2 like rufus.patients;\n";
    copierequete += "insert into rufus.patients2 (select * from " NOM_TABLE_PATIENTS ");";
    QSqlQuery copiequery (copierequete,db);
    proc->TraiteErreurRequete(copiequery,copierequete,"");

    QStringList listNoms;
    copierequete = "select idPat, patNom from rufus.patients2;";
    QSqlQuery copie(copierequete,db);
    int s = copie.size();
    copie.first();
    for (int i = 0; i < copie.size(); i++)
    {
        listNoms << copie.value(1).toString();
        copie.next();
    }
    for (int k = 0; k < s ; k++)
    {
        copie.seek(k);
        QString idpat = copie.value(0).toString();
        idauhasard = rand() % (listNoms.size());
        QString AncNom (""), NouvNom;
        NouvNom = listNoms.at(idauhasard);
        QSqlQuery quernom("select patnom, patprenom from " NOM_TABLE_PATIENTS " where idPat = " + idpat, db);
        if (quernom.size()>0)
        {
            quernom.first();
            AncNom = quernom.value(0).toString();
            //proc->Message(quernom.value(1).toString() + " " + AncNom + " - " + QString::number(k) + "/" + QString::number(s), 1);
            //qDebug() << quernom.value(1).toString() + " " + AncNom + " - " + QString::number(k) + "/" + QString::number(s);
        }
        copierequete = "update rufus.patients2 set patnom = '" + proc->CorrigeApostrophe(listNoms.at(idauhasard)) + "' where idPat = " + idpat;
        QSqlQuery modif (copierequete,db);
        proc->TraiteErreurRequete(modif,copierequete,"");
        listNoms.removeAt(idauhasard);

        QSqlQuery modifactesquer("select idacte, actemotif, acteconclusion, actetexte from " NOM_TABLE_ACTES " where idpat = " + idpat, db);
        for (int m=0; m<modifactesquer.size(); m++)
        {
            modifactesquer.seek(m);
            QString nouvmotif   = modifactesquer.value(1).toString();
            QString nouvconcl   = modifactesquer.value(2).toString();
            QString nouvtxt     = modifactesquer.value(3).toString();
            //qDebug() << nouvtxt;
            bool b = false;
            if (nouvmotif.contains(AncNom, Qt::CaseInsensitive))
            {
                nouvmotif   = nouvmotif.replace(AncNom,NouvNom, Qt::CaseInsensitive);
                b = true;
            }
            if (nouvconcl.contains(AncNom, Qt::CaseInsensitive))
            {
                nouvconcl   = nouvconcl.replace(AncNom,NouvNom, Qt::CaseInsensitive);
                b = true;
            }
            if (nouvtxt.contains(AncNom, Qt::CaseInsensitive))
            {
                nouvtxt   = nouvtxt.replace(AncNom,NouvNom, Qt::CaseInsensitive);
                b = true;
            }
            if (b)
            {
                QString req1 = "update " NOM_TABLE_ACTES " set"
                       " actemotif = '"         + proc->CorrigeApostrophe(nouvmotif) + "',"
                       " actetexte = '"         + proc->CorrigeApostrophe(nouvtxt)   + "',"
                       " acteconclusion = '"    + proc->CorrigeApostrophe(nouvconcl) + "'"
                       " where idacte = " + modifactesquer.value(0).toString();
                //qDebug() << req1;
                QSqlQuery (req1, db);
            }
        }
        QSqlQuery modifimprquer("select idimpression, textentete, textcorps, textorigine from " NOM_TABLE_IMPRESSIONS " where idpat = " + idpat, db);
        for (int m=0; m<modifimprquer.size(); m++)
        {
            modifimprquer.seek(m);
            QString nouventete   = modifimprquer.value(1).toString();
            QString nouvcorps   = modifimprquer.value(2).toString();
            QString nouvorigine     = modifimprquer.value(3).toString();
            bool b = false;
            if (nouventete.contains(AncNom, Qt::CaseInsensitive))
            {
                nouventete   = nouventete.replace(AncNom,NouvNom.toUpper(), Qt::CaseInsensitive);
                b = true;
            }
            if (nouvcorps.contains(AncNom, Qt::CaseInsensitive))
            {
                nouvcorps   = nouvcorps.replace(AncNom,NouvNom.toUpper(), Qt::CaseInsensitive);
                b = true;
            }
            if (nouvorigine.contains(AncNom, Qt::CaseInsensitive))
            {
                nouvorigine   = nouvorigine.replace(AncNom,NouvNom.toUpper(), Qt::CaseInsensitive);
                b = true;
            }
            if (b)
            {
                QString req1 = "update " NOM_TABLE_IMPRESSIONS " set"
                       " textentete = '"         + proc->CorrigeApostrophe(nouventete) + "',"
                       " textcorps = '"         + proc->CorrigeApostrophe(nouvcorps)   + "',"
                       " textorigine = '"    + proc->CorrigeApostrophe(nouvorigine) + "'"
                       " where idimpression = " + modifimprquer.value(0).toString();
                //qDebug() << req1;
                QSqlQuery (req1, db);
            }
        }
    }
    copierequete = "delete from rufus.patients;\n";
    copierequete += "insert into rufus.patients (select * from rufus.patients2);\n";
    copierequete += "drop table if exists rufus.patients2;\n";
    QSqlQuery (copierequete,db);

    UpMessageBox::Watch(this,"OK pour nom");

        copierequete = "drop table if exists rufus.donneessocialespatients2;\n";
    copierequete += "create table rufus.donneessocialespatients2 like rufus.donneessocialespatients;\n";
    copierequete += "insert into rufus.donneessocialespatients2 (select * from rufus.donneessocialespatients);";
    QSqlQuery copieAquery (copierequete,db);
    proc->TraiteErreurRequete(copieAquery,copierequete,"");

    QStringList listAdresses;
    copierequete = "select idPat, patAdresse1 from rufus.donneessocialespatients2 order by patAdresse1;";
    QSqlQuery copieA (copierequete,db);
    copieA.first();
    s = copieA.size();
    for (int i = 0; i < copieA.size(); i++)
    {
        listAdresses << copieA.value(1).toString();
        copieA.next();
    }
    for (int j = 0; j < s ; j++)
    {
        copieA.seek(j);
        QString idpat = copieA.value(0).toString();
        idauhasard = rand() % (listAdresses.size());
        copierequete = "update rufus.donneessocialespatients2 set patAdresse1 = '" + proc->CorrigeApostrophe(listAdresses.at(idauhasard))
                + "' where idPat = " + idpat;
        QSqlQuery modif3 (copierequete,db);
        proc->TraiteErreurRequete(modif3,copierequete,"");
        listAdresses.removeAt(idauhasard);
    }
    listAdresses.clear();
    copierequete = "select idPat, patAdresse2 from rufus.donneessocialespatients2 order by patAdresse2;";
    QSqlQuery copieA2 (copierequete,db);
    copieA2.first();
    s = copieA2.size();
    for (int i = 0; i < copieA2.size(); i++)
    {
        listAdresses << copieA2.value(1).toString();
        copieA2.next();
    }
    for (int j = 0; j < s ; j++)
    {
        copieA2.seek(j);
        QString idpat = copieA2.value(0).toString();
        idauhasard = rand() % (listAdresses.size());
        copierequete = "update rufus.donneessocialespatients2 set patAdresse2 = '" + proc->CorrigeApostrophe(listAdresses.at(idauhasard))
                + "' where idPat = " + idpat;
        QSqlQuery modif4 (copierequete,db);
        proc->TraiteErreurRequete(modif4,copierequete,"");
        listAdresses.removeAt(idauhasard);
    }
    copierequete = "delete from rufus.donneessocialespatients;\n";
    copierequete += "insert into rufus.donneessocialespatients (select * from rufus.donneessocialespatients2);\n";
    copierequete += "drop table if exists rufus.donneessocialespatients2;\n";
    QSqlQuery (copierequete,db);
    UpMessageBox::Watch(this,"OK pour adresse1 et 2");
    Remplir_ListePatients_TableView(grequeteListe,"","");

    //Melange des noms des correspondants
    QSqlQuery("update " NOM_TABLE_CORRESPONDANTS " set CorNom = 'Porteix' where CorNom = 'Porte'",db);
    QSqlQuery("update " NOM_TABLE_CORRESPONDANTS " set CorNom = 'Longeix' where CorNom = 'Long'",db);
    QString Corcopierequete = "select idCor, CorNom from " NOM_TABLE_CORRESPONDANTS;
    QSqlQuery Corcopie(Corcopierequete,db);
    QStringList listnomcor;
    QSqlQuery nompatquery("select patnom from " NOM_TABLE_PATIENTS " where patnom not in (select Cornom from " NOM_TABLE_CORRESPONDANTS ") order by rand() limit " + QString::number(Corcopie.size()), db);
    for (int e=0; e<Corcopie.size();e++)
    {
        nompatquery.seek(e);
        listnomcor <<  nompatquery.value(0).toString();
    }
    QString Corimpr = "select idimpression, textcorps, textorigine from " NOM_TABLE_IMPRESSIONS " where textcorps is not null";
    QSqlQuery Corimprquery(Corimpr, db);
    QString CorAct = "select idacte, actemotif, actetexte from " NOM_TABLE_ACTES;
    QSqlQuery CorActquery(CorAct, db);
    proc->TraiteErreurRequete(CorActquery, CorAct,"");
    for (int k = 0; k< Corcopie.size(); k++)
    {
        QString AncNom, NouvNom, NouvAdresse, idCor;
        Corcopie.seek(k);
        idCor       = Corcopie.value(0).toString();
        AncNom      = Corcopie.value(1).toString();
        NouvNom     = listnomcor.at(k);
        Corcopierequete = "update " NOM_TABLE_CORRESPONDANTS " set Cornom = '" + proc->CorrigeApostrophe(NouvNom) + "' where idCor = " + idCor;
        QSqlQuery modifnom (Corcopierequete,db);
        proc->TraiteErreurRequete(modifnom,Corcopierequete,"");


        for (int p=0; p<Corimprquery.size(); p++)
        {
            Corimprquery.seek(p);
            if (Corimprquery.value(1).toString().contains(AncNom, Qt::CaseInsensitive))
            {
                //qDebug() << AncNom + " - " + QString::number(k) + "/" + QString::number(Corcopie.size()) + " // " + QString::number(p) + "/" + QString::number(Corimprquery.size());
                QSqlQuery ("update " NOM_TABLE_IMPRESSIONS " set textcorps = '" + proc->CorrigeApostrophe(Corimprquery.value(1).toString().replace(AncNom,NouvNom))
                           + "' where idimpression = " + Corimprquery.value(0).toString(), db);
            }
            if (Corimprquery.value(2).toString().contains(AncNom, Qt::CaseInsensitive))
            {
                QSqlQuery ("update " NOM_TABLE_IMPRESSIONS " set textorigine = '" + proc->CorrigeApostrophe(Corimprquery.value(2).toString().replace(AncNom,NouvNom))
                           + "' where idimpression = " + Corimprquery.value(0).toString(), db);
            }
        }

        for (int q=0; q<CorActquery.size(); q++)
        {
            CorActquery.seek(q);
            if (CorActquery.value(1).toString().contains(AncNom, Qt::CaseInsensitive))
            {
                //qDebug() << AncNom + " - " + QString::number(k) + "/" + QString::number(Corcopie.size()) + " // " + QString::number(q) + "/" + QString::number(CorActquery.size());
                QSqlQuery ("update " NOM_TABLE_ACTES " set actemotif = 'Courrier efffacé' where idacte = " + CorActquery.value(0).toString(), db);
            }
            if (CorActquery.value(2).toString().contains(AncNom, Qt::CaseInsensitive))
            {
                QSqlQuery("update " NOM_TABLE_ACTES " set actetexte = 'Courrier effacé' where idacte = " + CorActquery.value(0).toString(), db);
            }
        }

        QSqlQuery adresspatquery("select patadresse1 from " NOM_TABLE_DONNEESSOCIALESPATIENTS " order by rand() limit 1", db);
        adresspatquery.first();
        NouvAdresse = adresspatquery.value(0).toString();
        Corcopierequete = "update rufus.correspondants set Coradresse1 = '" + proc->CorrigeApostrophe(NouvAdresse) + "' where idCor = " + idCor;
        QSqlQuery modif (Corcopierequete,db);
        proc->TraiteErreurRequete(modif,Corcopierequete,"");
    }
    UpMessageBox::Watch(this,"OK pour Correspondants");
}

void Rufus::Slot_ActeMontantModifie()
{
    QString b = QLocale().toString(QLocale().toDouble(ui->ActeMontantlineEdit->text()),'f',2);
    if (b != gActeMontant)
        ValideActeMontantLineEdit(b, gActeMontant);
}

/*------------------------------------------------------------------------------------------------------------------------------------
-- Afficher le motif de l'acte ----------------------------------------------------------------
------------------------------------------------------------------------------------------------------------------------------------*/
void Rufus::Slot_AfficheMotif(int idpat)
{
    QString Msg("");
    QString req = "select saldat.Motif, Message, rdv.motif from " NOM_TABLE_SALLEDATTENTE " saldat left outer join " NOM_TABLE_MOTIFSRDV " rdv on saldat.motif = rdv.raccourci "
                  "where idpat = " + QString::number(idpat);
    //qDebug() << req;
    QSqlQuery quer(req,db);
    if (quer.size()>0)
    {
        quer.first();
        UpLabel* lbl = dynamic_cast<UpLabel*>(sender());
        if (lbl)
            if (ui->SalleDAttenteupTableWidget->isAncestorOf(lbl))
            {
                req = "SELECT PatDDN FROM " NOM_TABLE_PATIENTS " WHERE idPat = " + QString::number(idpat);
                QSqlQuery quer1 (req,db);
                proc->TraiteErreurRequete(quer1,req,"");
                if (quer1.size() > 0)
                {
                    quer1.first();
                    if (quer1.value(0).toString() != "")
                        Msg += proc->CalculAge(quer1.value(0).toDate(), QDate::currentDate())["Total"].toString() + " - ";
                }
                if (quer.value(0).toString() == "URG")
                    Msg += tr("Urgence");
                else
                    Msg += quer.value(2).toString();
            }
        if (quer.value(1).toString()!= "")
        {
            if (Msg != "") Msg += "\n";
            Msg += quer.value(1).toString();
        }
    }
    if (Msg!="")
        QToolTip::showText(cursor().pos(),Msg);
}

/*------------------------------------------------------------------------------------------------------------------------------------
    -- Afficher l'adresse du patient en toolTip ----------------------------------------------------------------
    ------------------------------------------------------------------------------------------------------------------------------------*/
void Rufus::Slot_AfficheToolTip(QModelIndex pindx)
{
    int row = gListePatientsModel->itemFromIndex(pindx)->row();
    QString idpat = gListePatientsModel->item(row)->text();
    QString req = "SELECT PatAdresse1, PatAdresse2, PatAdresse3, PatVille, PatDDN FROM " NOM_TABLE_DONNEESSOCIALESPATIENTS " dos, " NOM_TABLE_PATIENTS " pat"
                  "  WHERE pat.idPat = " + idpat + " and pat.idpat = dos.idpat";
    QSqlQuery quer (req,db);
    proc->TraiteErreurRequete(quer,req,"");
    QString Msg = "";
    if (quer.size() > 0)
    {
        quer.first();
        if (quer.value(4).toString() != "")
            Msg += proc->CalculAge(quer.value(4).toDate(), QDate::currentDate())["Total"].toString();
        if (quer.value(3).toString() != "")
        {
            if (Msg!="") Msg = "\n" + Msg;
            Msg = quer.value(3).toString() + Msg;
        }
        if (quer.value(2).toString() != "")
        {
            if (Msg!="") Msg = "\n" + Msg;
            Msg = quer.value(2).toString() + Msg;
        }
        if (quer.value(1).toString() != "")
        {
            if (Msg!="") Msg = "\n" + Msg;
            Msg = quer.value(1).toString() + Msg;
        }
        if (quer.value(0).toString() != "")
        {
            if (Msg!="") Msg = "\n" + Msg;
            Msg = quer.value(0).toString() + Msg;
        }
        if (Msg != "")
            QToolTip::showText(cursor().pos(),Msg);
    }
}

void Rufus::Slot_AfficheToolTip(int id)
{
    UpLabel* lab = dynamic_cast<UpLabel*>(sender());
    if (lab != NULL)
    {
        QString req = "SELECT PatAdresse1, PatAdresse2, PatAdresse3, PatVille, PatDDN FROM " NOM_TABLE_DONNEESSOCIALESPATIENTS " dos, " NOM_TABLE_PATIENTS " pat"
                      "  WHERE pat.idPat = " + QString::number(id) + " and pat.idpat = dos.idpat";
        QSqlQuery quer (req,db);
        proc->TraiteErreurRequete(quer,req,"");
        QString Msg = "";
        if (quer.size() > 0)
        {
            quer.first();
            if (quer.value(4).toString() != "")
                Msg += proc->CalculAge(quer.value(4).toDate(), QDate::currentDate())["Total"].toString();
            if (quer.value(3).toString() != "")
            {
                if (Msg!="") Msg = "\n" + Msg;
                Msg = quer.value(3).toString() + Msg;
            }
            if (quer.value(2).toString() != "")
            {
                if (Msg!="") Msg = "\n" + Msg;
                Msg = quer.value(2).toString() + Msg;
            }
            if (quer.value(1).toString() != "")
            {
                if (Msg!="") Msg = "\n" + Msg;
                Msg = quer.value(1).toString() + Msg;
            }
            if (quer.value(0).toString() != "")
            {
                if (Msg!="") Msg = "\n" + Msg;
                Msg = quer.value(0).toString() + Msg;
            }
        }
        if (Msg != "")
            QToolTip::showText(cursor().pos(),Msg);
    }
}

/*-----------------------------------------------------------------------------------------------------------------
-- Gère l'affichage des menus -------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void Rufus::Slot_AfficheMenu()
{
    actionRechercheParID->setVisible(gMode != NouveauDossier);
    bool a = ui->tabWidget->indexOf(ui->tabDossier) < 0;
        actionBilanRecettes->setEnabled(a);
        actionRemiseCheques->setEnabled(a);
        actionRecettesSpeciales->setEnabled(a);
        actionImpayes->setEnabled(a);
        actionParametres->setEnabled(a);

    bool b = (ui->tabWidget->currentWidget() == ui->tabDossier);
        actionSupprimerActe->setVisible(b);
        actionCreerActe->setVisible(b);
        actionEmettreDocument->setVisible(b);
        actionDossierPatient->setVisible(b);
        menuEmettre->setEnabled(b);

    if (sender() == menuDossier)
    {
        actionSupprimerDossier->setEnabled(true);
        if (ui->tabWidget->currentWidget() == ui->tabList)
        {
            if (ui->tabWidget->indexOf(ui->tabDossier) < 0)
            {
               QModelIndexList listitems = ui->PatientsListeTableView->selectionModel()->selectedIndexes();
               if (listitems.size() > 0)
               {
                   int idpat = gListePatientsModel->itemFromIndex(listitems.at(0))->text().toInt();
                   QSqlQuery quer("select idacte from " NOM_TABLE_ACTES " where idpat = " + QString::number(idpat),db);
                   actionSupprimerDossier->setVisible(quer.size() == 0);
               }
                else
                   actionSupprimerDossier->setVisible(false);
            }
            else
            {
                actionSupprimerDossier->setVisible(gUserFonction == tr("Médecin") || gUserFonction == tr("Orthoptiste"));
                actionSupprimerDossier->setEnabled(false);
            }
        }
        else if (ui->tabWidget->currentWidget() == ui->tabDossier) {
            QSqlQuery quer("select idacte from " NOM_TABLE_ACTES " where idpat = " + QString::number(gidPatient),db);
            actionSupprimerDossier->setVisible(quer.size() == 0 || gUserFonction == tr("Médecin") || gUserFonction == tr("Orthoptiste"));
        }
    }
    if (sender() == menuDocuments)
    {
        bool a = false;
        if (ui->tabWidget->currentWidget() == ui->tabList)
            a = (ui->PatientsListeTableView->selectionModel()->selectedIndexes().size() > 0);
        else
            a = (ui->tabWidget->currentWidget() == ui->tabDossier);
        actionEnregistrerVideo      ->setVisible(a);
        actionEnregistrerDocScanner ->setVisible(a);
    }
}

void Rufus::Slot_AppelPaiementDirect(QString Origin)
{
    QList<int> ListidActeAPasser;
    int Mode = 1;
    ListidActeAPasser << 0;

    if (Origin == "AttentePaiement")                                        // l'appel est fait par un clic dans le menu contextuel de la salle d'attente des paiements en attente
    {
        ListidActeAPasser.clear();
        QList<QTableWidgetSelectionRange> ListItems = ui->AccueilupTableWidget->selectedRanges();
        if (ListItems.size() == 0) return;
        for (int i = 0; i < ListItems.size(); i++)
        {
            int debut = ListItems.at(i).topRow();
            int hauteur = ListItems.at(i).rowCount();
            for (int k=0;k<hauteur;k++)
            {
                // On vérifie que chaque acte sélectionné n'est pas déjà en cours d'enregistrement sur un autre poste
                QString req = "SELECT idActe FROM "  NOM_TABLE_VERROUCOMPTAACTES
                              " WHERE idActe = "  + ui->AccueilupTableWidget->item(debut+k,5)->text();
                QSqlQuery ChercheVerrouQuery (req,db);
                proc->TraiteErreurRequete(ChercheVerrouQuery, req,"");
                if (ChercheVerrouQuery.size() == 0)
                    ListidActeAPasser << ui->AccueilupTableWidget->item(debut+k,5)->text().toInt();
            }
            if (ListidActeAPasser.size() == 0)
            {
                QSound::play(NOM_ALARME);
                UpMessageBox::Watch(this,tr("Le ou les actes que vous avez sélectionnés\nsont déjà en cours d'enregistrement!"));
                return;
            }
        }
    }
    if (Origin == "Bouton")                                                 // l'appel est fait par un clic sur le bouton enregistrepaiement
    {
        // On vérifie que la cotation est complète
        QString Titre = "";
        if (ui->ActeCotationcomboBox->currentText() == "")
            Titre = tr("Il manque la cotation!");
        else if (ui->ActeMontantlineEdit->text() == "")
            Titre = tr("Il manque le montant!");
        if (Titre != "")
        {
            UpMessageBox::Watch(this,tr("Vous ne pouvez pas enregistrer le paiement de cet acte !"), Titre);
            return;
        }
        // On vérifie que cet acte n'est pas déjà en cours d'enregistrement sur un autre poste
        QString req = "SELECT UserLogin FROM " NOM_TABLE_VERROUCOMPTAACTES ", " NOM_TABLE_UTILISATEURS
                      " WHERE idActe = "  + QString::number(gidActe) +
                      " AND PosePar = idUser";
        QSqlQuery ChercheVerrouQuery (req,db);
        proc->TraiteErreurRequete(ChercheVerrouQuery, req,"");
        if (ChercheVerrouQuery.size() > 0)
        {
            ChercheVerrouQuery.first();
            UpMessageBox::Watch(this,tr("Vous ne pouvez paz enregistrer le paiement de cet acte !"),
                                tr("Il est déjà en cours d'enregistrement par ") + ChercheVerrouQuery.value(0).toString());
            return;
        }
        // il s'agit d'un acte gratuit - on propose de le classer
        if (QLocale().toDouble(ui->ActeMontantlineEdit->text()) == 0 && ui->ActeCotationcomboBox->currentText() != "")
        {
            UpMessageBox msgbox;
            UpSmallButton *OKBouton = new UpSmallButton();
            msgbox.setText(tr("Vous avez entré un montant nul !"));
            msgbox.setInformativeText(tr("Enregistrer cette consultation comme gratuite?"));
            OKBouton->setText(tr("Consultation gratuite"));
            UpSmallButton *NoBouton = new UpSmallButton();
            NoBouton->setText(tr("Annuler"));
            msgbox.addButton(NoBouton, UpSmallButton::CANCELBUTTON);
            msgbox.addButton(OKBouton, UpSmallButton::STARTBUTTON);
            msgbox.exec();
            if (msgbox.clickedButton() != OKBouton)
                ui->ActeMontantlineEdit->setFocus();
            else
            {
                QString enreggratuit = "INSERT INTO " NOM_TABLE_TYPEPAIEMENTACTES " (idActe, TypePaiement) VALUES (" + QString::number(gidActe) + ",'G')";
                QSqlQuery InsertGratuitQuery (enreggratuit,db);
                proc->TraiteErreurRequete(InsertGratuitQuery,enreggratuit,tr("Impossible d'enregister cet acte comme gratuit"));
                AfficheActeCompta();
            }
            delete OKBouton;
            delete NoBouton;
            return;
        }

        QString ActeSal = QString::number(gidActe);
        QString Msg;
        QString requete =   "SELECT idPat FROM " NOM_TABLE_SALLEDATTENTE
                    " WHERE idPat = " + QString::number(gidPatient);
        QSqlQuery SalDatQuery(requete,db);
        proc->TraiteErreurRequete(SalDatQuery,requete,tr("Impossible de trouver le dossier dans la salle d'attente!"));

        if (SalDatQuery.size() == 0)
        {

            requete = "INSERT INTO " NOM_TABLE_SALLEDATTENTE
                    " (idPat, idUser, Statut, HeureStatut, idUserEnCoursExam, idActeAPayer, PosteExamen)"
                    " VALUES (" + QString::number(gidPatient) + "," + QString::number(gidUserSuperviseur) + ",'" RETOURACCUEIL "',"
                                + QTime::currentTime().toString("hh:mm") +", null," + ActeSal + ", null)";
            Msg = tr("Impossible de mettre ce dossier en salle d'attente");
        }
        else
        {
            requete = "UPDATE " NOM_TABLE_SALLEDATTENTE
                    " SET Statut = '" RETOURACCUEIL
                    "', HeureStatut = '" + QTime::currentTime().toString("hh:mm") +
                    "', idUserEnCoursExam = null"
                    " , PosteExamen = null";
            if (ActeSal != "null")
                requete += ", idActeAPayer = " + ActeSal;
            requete += " WHERE idPat = '" + QString::number(gidPatient) + "'";
            Msg = tr("Impossible de modifier les statuts du dossier en salle d'attente!");
        }
        proc->UpdVerrouSalDat();
        QSqlQuery ModifSalDatQuery(requete,db);
        proc->TraiteErreurRequete(ModifSalDatQuery,requete,Msg);
        ListidActeAPasser.clear();
        ListidActeAPasser << gidActe;
    }

    Dlg_PaimtDirect           = new dlg_paiement(&ListidActeAPasser, Mode, proc, 0, 0, this);
    if(Dlg_PaimtDirect->getInitOK())
    {
        Dlg_PaimtDirect->setWindowTitle(tr("Gestion des paiements directs"));
        Dlg_PaimtDirect->exec();
    }
    if (Origin == "Bouton")  // on redonne le statut en cours d'examen au dossier
    {
        QString Msg;
        QString req =   "SELECT idPat FROM " NOM_TABLE_SALLEDATTENTE
                " WHERE idPat = " + QString::number(gidPatient);
        QSqlQuery SalDatQuery(req,db);
        proc->TraiteErreurRequete(SalDatQuery,req, tr("Impossible de trouver le dossier dans la salle d'attente!"));
        if (SalDatQuery.size() == 0)
        {
            req =   "INSERT INTO " NOM_TABLE_SALLEDATTENTE
                    " (idPat, idUser, Statut, HeureStatut, idUserEnCoursExam, PosteExamen)"
                    " VALUES ('" + QString::number(gidPatient) + "','" + QString::number(gidUserSuperviseur) + "','" ENCOURSEXAMEN + gUserLogin + "','" + QTime::currentTime().toString("hh:mm")
                    + "'," + QString::number(gidUser) + ",'" + QHostInfo::localHostName().left(60) + "')";
            Msg = tr("Impossible de modifier le statut du dossier");
        }
        else
        {
            req =   "UPDATE " NOM_TABLE_SALLEDATTENTE
                    " SET Statut = '" ENCOURSEXAMEN + gUserLogin +
                    "', HeureStatut = '" + QTime::currentTime().toString("hh:mm") +
                    "', idUserEnCoursExam = " + QString::number(gidUser) +
                    ", PosteExamen = '" + QHostInfo::localHostName().left(60) +
                    "' WHERE idPat = " + QString::number(gidPatient);
            Msg = tr("Impossible de modifier le statut du dossier");
        }
        QSqlQuery ModifSalDatQuery (req,db);
        proc->TraiteErreurRequete(ModifSalDatQuery,req,Msg);
        proc->UpdVerrouSalDat();
    }
    AfficheActeCompta();
    delete Dlg_PaimtDirect;
}

void Rufus::Slot_AppelPaiementTiers()
{
    QList<dlg_paiement *> PaimtList = findChildren<dlg_paiement*>();
    if (PaimtList.size()>0)
        for (int i=0; i<PaimtList.size();i++)
            if (PaimtList.at(i)->isVisible())
            {
                QSound::play(NOM_ALARME);
                PaimtList.at(i)->raise();
                return;
            }
    int Mode = 3;
    QList<int> ListidActeAPasser;
    ListidActeAPasser << 0;
    Dlg_PaimtTiers = new dlg_paiement(&ListidActeAPasser, Mode, proc, 0, 0, this);
    if(Dlg_PaimtTiers->getInitOK())
    {
        Dlg_PaimtTiers->setWindowTitle(tr("Gestion des tiers payants"));
        Dlg_PaimtTiers->show();
    }
}

void Rufus::Slot_AutreDossier(int idPat)
{
    gdossierAOuvrir = idPat;
    Slot_ChoixMenuContextuelListePatients(tr("Autre Dossier"));
}

void Rufus::Slot_BasculerMontantActe()
{
    int idx = ui->ActeCotationcomboBox->findText(ui->ActeCotationcomboBox->currentText());
    if (idx>0)
    {
        QStringList listMontantActe = ui->ActeCotationcomboBox->itemData(idx).toStringList();
        double MontantConv, MontantPrat, MontantActe;
        MontantActe = QLocale().toDouble(ui->ActeMontantlineEdit->text());
        MontantConv = listMontantActe.at(0).toDouble();
        MontantPrat = listMontantActe.at(1).toDouble();
        if (MontantActe!=MontantPrat)
        {
            ui->ActeMontantlineEdit->setText(QLocale().toString(MontantPrat,'f',2));
            ui->BasculerMontantpushButton->setImmediateToolTip(tr("Revenir au tarif conventionnel"));
        }
        else
        {

            ui->ActeMontantlineEdit->setText(QLocale().toString(MontantConv,'f',2));
            ui->BasculerMontantpushButton->setImmediateToolTip(tr("Revenir au tarif habituellement pratiqué"));
        }
        ValideActeMontantLineEdit(ui->ActeMontantlineEdit->text(),QLocale().toString(MontantActe,'f',2));
    }
}

void Rufus::Slot_BilanRecettes()
{
    if (proc->ChoixDate())
    {
        QString req =
        "select res1.idActe, res1.actedate, res1.nom, res1.actecotation, res1.acteMontant, res1.actemonnaie, res1.TypePaiement, res1.Tiers, Paye, res1.iduser, res1.userparent from\n "
        "(\n"
            "select\n"
            " act.idActe, actedate, concat(patnom, ' ', patprenom) as nom, actecotation, acteMontant, acteMonnaie, TypePaiement, Tiers, iduser, userparent from \n"
            NOM_TABLE_ACTES " act, " NOM_TABLE_PATIENTS " pat, " NOM_TABLE_TYPEPAIEMENTACTES " typ\n"
            " where act.idPat = pat.idpat\n"
            " and act.idActe = typ.idacte\n"
            " and actedate >= '" + proc->DateDebut.toString("yyyy-MM-dd") + "'\n"
            " and actedate <= '" + proc->DateFin.toString("yyyy-MM-dd") + "'\n"
            " order by actedate, nom\n"
        ")\n"
        " as res1\n"
        " left outer join\n"
        " (\n"
            "select rec.idrecette, paye, lig.idActe from \n"
            NOM_TABLE_LIGNESPAIEMENTS " lig, " NOM_TABLE_RECETTES " rec, " NOM_TABLE_TYPEPAIEMENTACTES " typ2\n"
            " where lig.idrecette = rec.idrecette\n"
            " and lig.idActe = typ2.idacte\n"
            " and TypePaiement <> 'T'\n"
            " and TypePaiement <> 'G'\n"
            " and datepaiement >= '" + proc->DateDebut.toString("yyyy-MM-dd") + "'\n"
            " and datepaiement <= '" + proc->DateFin.toString("yyyy-MM-dd") + "'\n"
        ")\n"
        " as res3 on res1.idacte = res3.idActe\n"

        " union\n"

        " select null as idActe, DatePaiement as actedate, NomTiers as nom, null as actecotation, null as acteMontant, Monnaie as acteMonnaie, ModePaiement as TypePaiement, null as Tiers, Montant as paye, iduser, iduser as userparent from \n"
        NOM_TABLE_RECETTES
        "\n where TiersPayant = 'O'\n"
        " and DatePaiement >= '" + proc->DateDebut.toString("yyyy-MM-dd") + "'\n"
        " and DatePaiement <= '" + proc->DateFin.toString("yyyy-MM-dd") + "'\n"
        " order by actedate, nom";

        //proc->Edit(req);

        QSqlQuery EnumRecettesQuery (req,proc->getDataBase());
        if (proc->TraiteErreurRequete(EnumRecettesQuery,req,tr("Impossible de construire la table des recettes")))
            return;

        if (EnumRecettesQuery.size() == 0)
        {
            UpMessageBox::Watch(this,tr("Pas de recette enregistrée pour cette période"));
            return;
        }

        Dlg_Rec            = new dlg_recettes(&proc->DateDebut, &proc->DateFin, proc, EnumRecettesQuery, this);
        Dlg_Rec->exec();
        Dlg_Rec->close(); // nécessaire pour enregistrer la géométrie
        delete Dlg_Rec;
    }
}

void Rufus::Slot_CCAM(QString link)
{
    QDesktopServices::openUrl(QUrl(link));
}

void Rufus::Slot_ChangeTabBureau()
{
    if(ui->tabWidget->currentWidget() == ui->tabList)
    {
        setFixedWidth(LARGEURLISTE);
        ToolBarListe();
        ui->CreerNomlineEdit->setFocus();
        CalcNbDossiers();
        //ui->ChgUserpushButton->setEnabled(ui->tabWidget->indexOf(ui->tabDossier) < 0);
    }
    else
    {
        setFixedWidth(LARGEURNORMALE);
        ToolBarDossier();
    }
}

void Rufus::Slot_ChoixDossier()  // double clic dans Liste
{
    ChoixDossier(gListePatientsModel->itemFromIndex(ui->PatientsListeTableView->selectionModel()->selectedIndexes().at(0))->text().toInt());
}

void Rufus::Slot_ChoixDossier(int idPat) // double clic dans table salle d'attente ou accueil
{
    if (gUserFonction != tr("Secrétaire"))
        ChoixDossier(idPat);
}

void Rufus::Slot_ChoixMG()
{
    QString req = "select idpat from " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS " where idpat = " + QString::number(gidPatient);
    QSqlQuery quer(req,db);
    if (quer.size() == 0)
        req =   "INSERT INTO " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS
                " (idPat, idCorMedMG) VALUES (" + QString::number(gidPatient) + "," + ui->MGupComboBox->currentData().toString() + ")";
    else
        req = "update " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS " set idcormedmg = " + ui->MGupComboBox->currentData().toString() + " where idpat = " + QString::number(gidPatient);
    //qDebug() << req;
    QSqlQuery (req,db);
    Slot_OKModifierTerrain();
    ui->MGupComboBox->setImmediateToolTip(CalcToolTipCorrespondant(ui->MGupComboBox->currentData().toInt()));
}

/*-----------------------------------------------------------------------------------------------------------------
-- Rechercher le nom dans la listePatients par l'id ---------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void    Rufus::Slot_ChercheNomparID(QString id)  // ce mode de recherche se fait sans filtrage de la liste qui est vue dans son intégralité
{
    QSqlQuery quer("SELECT PatNom, PatPrenom FROM "  NOM_TABLE_PATIENTS " WHERE idPat = " + id, db);
    if (quer.size()>0)
    {
        quer.first();
        ui->CreerNomlineEdit->setText(quer.value(0).toString());
        ui->CreerPrenomlineEdit->clear();
        lblnom->setText(quer.value(0).toString().toUpper() + " " + quer.value(1).toString());
        if (!ui->FiltrecheckBox->isChecked())
            ChercheNom(id.toInt());
        else
            ChercheNomFiltre(id.toInt());
    }
}

void Rufus::Slot_ChoixCor()
{
    QString idcor;
    UpComboBox *box = dynamic_cast<UpComboBox*>(sender());
    if (sender()==ui->AutresCorresp1upComboBox)
        idcor = "idcormedspe1";
    else if (sender()==ui->AutresCorresp2upComboBox)
        idcor = "idcormedspe2";
    QString req = "select idpat from " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS " where idpat = " + QString::number(gidPatient);
    QSqlQuery quer(req,db);
    if (quer.size() == 0)
        req =   "INSERT INTO " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS
                " (idPat, " + idcor + ") VALUES (" + QString::number(gidPatient) + "," + box->currentData().toString() + ")";
    else
        req = "update " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS " set " + idcor + " = " + box->currentData().toString() + " where idpat = " + QString::number(gidPatient);
    QSqlQuery (req,db);
    Slot_OKModifierTerrain();
    box->setImmediateToolTip(CalcToolTipCorrespondant(box->currentData().toInt()));
}

void Rufus::Slot_ConnectTimers(bool a)
{
    if (a)
    {
        if (proc->getModeConnexion() == Procedures::Distant)
        {
            gTimerSalleDAttente->start(10000);
            gTimerVerifGestDocs->start(60000);
        }
        else
        {
            gTimerSalleDAttente->start(1000);
            gTimerVerifGestDocs->start(10000);
        }
        gTimerUserConnecte  ->start(10000);
        gTimerVerifVerrou   ->start(60000);
        gTimerSupprDocs     ->start(60000);
        connect (gTimerSalleDAttente,       SIGNAL(timeout()),  this,   SLOT(Slot_VerifSalleDAttente()));
        connect (gTimerUserConnecte,        SIGNAL(timeout()),  this,   SLOT(Slot_MetAJourLaConnexion()));
        connect (gTimerVerifVerrou,         SIGNAL(timeout()),  this,   SLOT(Slot_VerifVerrouDossier()));
        connect (gTimerVerifGestDocs,       SIGNAL(timeout()),  this,   SLOT(Slot_ActualiseDocsExternes()));
        connect (gTimerVerifGestDocs,       SIGNAL(timeout()),  this,   SLOT(Slot_ImportDocsExternes()));
        if (proc->getModeConnexion() != Procedures::Distant)
           connect(gTimerVerifGestDocs, SIGNAL(timeout()), this, SLOT(Slot_ExporteDocs()));
        if (QSystemTrayIcon::isSystemTrayAvailable())
            connect(gTimerVerifGestDocs,    SIGNAL(timeout()),  this,   SLOT(Slot_VerifMessages()));
        if (proc->getModeConnexion() == Procedures::Poste)
            connect(gTimerSupprDocs,        SIGNAL(timeout()),  this,   SLOT(Slot_SupprimerDocs()));
    }
    else
    {
        disconnect (gTimerSalleDAttente,    SIGNAL(timeout()),  this,   SLOT(Slot_VerifSalleDAttente()));
        disconnect (gTimerUserConnecte,     SIGNAL(timeout()),  this,   SLOT(Slot_MetAJourLaConnexion()));
        disconnect (gTimerVerifVerrou,      SIGNAL(timeout()),  this,   SLOT(Slot_VerifVerrouDossier()));
        disconnect (gTimerVerifGestDocs,    SIGNAL(timeout()),  this,   SLOT(Slot_ActualiseDocsExternes()));
        disconnect (gTimerVerifGestDocs,    SIGNAL(timeout()),  this,   SLOT(Slot_ImportDocsExternes()));
        disconnect (gTimerVerifGestDocs,    SIGNAL(timeout()), this, SLOT(Slot_ExporteDocs()));
        disconnect (gTimerVerifGestDocs,    SIGNAL(timeout()),  this,   SLOT(Slot_VerifMessages()));
        disconnect(gTimerSupprDocs,         SIGNAL(timeout()),  this,   SLOT(Slot_SupprimerDocs()));
        gTimerSalleDAttente ->stop();
        gTimerVerifGestDocs ->stop();
        gTimerUserConnecte  ->stop();
        gTimerVerifVerrou   ->stop();
        gTimerSupprDocs     ->stop();
    }
}

void Rufus::Slot_CourrierAFaireChecked()
{
    QString CourrierAFaire = "";
    if (ui->CourrierAFairecheckBox->isChecked())
        CourrierAFaire = "T";
    QString requete =   "UPDATE " NOM_TABLE_ACTES " SET ActeCourrieraFaire = '" + CourrierAFaire + "' WHERE idActe = " + QString::number(gidActe);
    QSqlQuery UpdateCourrierAFaireQuery (requete,db);
    proc->TraiteErreurRequete(UpdateCourrierAFaireQuery,requete,tr("Impossible d'écrire dans la table des Actes"));
    MAJActesPrecs();
}

void Rufus::Slot_CreerActe()
{
    CreerActe(gidPatient);
}

/*------------------------------------------------------------------------------------------------------------------
-- création - gestion des bilans orthoptiques ----------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void Rufus::Slot_CreerBilanOrtho()
{
    bool    nouveauBO       = true;
    bool    ConversionEnBO  = false;
    if (ui->Acteframe->isVisible())
    {
        QString chborequete = "select idbilanortho, ActeDate from " NOM_TABLE_BILANORTHO ", " NOM_TABLE_ACTES
                " where idActe = idbilanortho and idbilanortho = " + ui->idActelineEdit->text();
        //UpMessageBox::Watch(this,chborequete);
        QSqlQuery chboquery(chborequete,proc->getDataBase());
        QDate DateBl;
        nouveauBO = (chboquery.size()<1);
        if (chboquery.size() > 0)
        {
            chboquery.first();
            DateBl = chboquery.value(1).toDate();
            if (DateBl != QDate::currentDate())
            {
                UpMessageBox msgbox;
                UpSmallButton *OKBouton = new UpSmallButton();
                OKBouton->setText(tr("Reprendre"));
                UpSmallButton *NoBouton = new UpSmallButton();
                NoBouton->setText(tr("Créer un nouveau\nbilan orthoptique"));
                UpSmallButton *AnnulBouton = new UpSmallButton();
                NoBouton->setText(tr("Annuler"));
                msgbox.setText("Euuhh... " + gUserLogin);
                msgbox.setInformativeText(tr("Voulez-vous reprendre le bilan affiché\nou créer un nouveau bilan à la date d'aujourd'hui?"));
                msgbox.setIcon(UpMessageBox::Quest);
                msgbox.addButton(AnnulBouton, UpSmallButton::CANCELBUTTON);
                msgbox.addButton(NoBouton, UpSmallButton::COPYBUTTON);
                msgbox.addButton(OKBouton, UpSmallButton::STARTBUTTON);
                msgbox.exec();
                if (msgbox.clickedButton() != OKBouton && msgbox.clickedButton() != NoBouton)
                    return;
                nouveauBO = (msgbox.clickedButton() == NoBouton);
                delete OKBouton;
                delete NoBouton;
                delete AnnulBouton;
                msgbox.close();
            }
        }
        else
        {
            if (ui->ActeDatedateEdit->date() == QDate::currentDate())
            {
                UpMessageBox msgbox;
                UpSmallButton *NoBouton = new UpSmallButton();
                NoBouton->setText(tr("Annuler"));
                UpSmallButton *OKBouton = new UpSmallButton();
                OKBouton->setText(tr("Convertir en\nbilan orthoptique"));
                msgbox.setText("Euuhh... " + gUserLogin);
                msgbox.setInformativeText(tr("Voulez-vous transformer l'acte en cours en bilan orthoptique?\nToutes les données saisies seront perdues!"));
                msgbox.setIcon(UpMessageBox::Warning);
                msgbox.addButton(NoBouton, UpSmallButton::CANCELBUTTON);
                msgbox.addButton(OKBouton, UpSmallButton::STARTBUTTON);
                msgbox.exec();
                nouveauBO = (msgbox.clickedButton() == OKBouton);
                if (nouveauBO)
                {
                    ui->ActeMotiftextEdit->clear();
                    ui->ActeTextetextEdit->clear();
                    ui->ActeConclusiontextEdit->clear();
                    ConversionEnBO = true;
                }
                delete OKBouton;
                delete NoBouton;
                msgbox.close();
                if (!nouveauBO) return;
            }

        }
        if (!nouveauBO)
        {
            Dlg_BlOrtho             = new dlg_bilanortho(proc, ui->idActelineEdit->text().toInt(), chboquery.value(0).toInt(),this, gidPatient);
            UiDLg_BlOrtho           = Dlg_BlOrtho->ui;
            QString Titre           = tr("Bilan orthoptique - ") + gPrenomPatient + " " + gNomPatient;
            UiDLg_BlOrtho->OcclAlternlabel->setVisible(gDDNPatient.daysTo(DateBl) < 730);
            UiDLg_BlOrtho->OcclAlterncomboBox->setVisible(gDDNPatient.daysTo(DateBl) < 730);
            Dlg_BlOrtho             ->setWindowTitle(Titre);
            Dlg_BlOrtho             ->setDateBO(QDate::fromString(ui->ActeDatedateEdit->text(),"dd/MM/yyyy"));
            nouveauBO               = false;
        }
    }
    if (nouveauBO)
    {
        if (ui->Acteframe->isVisible() && !ConversionEnBO)
            if (!AutorDepartConsult(false)) return;
        if (!ConversionEnBO)
            CreerActe(gidPatient);
        Dlg_BlOrtho             = new dlg_bilanortho(proc, ui->idActelineEdit->text().toInt(), 0,this, gidPatient);
        UiDLg_BlOrtho           = Dlg_BlOrtho->ui;
        QString Titre           = tr("Bilan orthoptique - ") + gPrenomPatient + " " + gNomPatient;
        Dlg_BlOrtho             ->setWindowTitle(Titre);
        QString RefractionOD    = "";
        QString RefractionOG    = "";
        Dlg_BlOrtho             ->setDateBO(QDate::currentDate());

        QString RefODrequete    = "select max(idrefraction), formuleOD from " NOM_TABLE_REFRACTION " where quelleMesure = 'R' and quelleDistance = '2' and idPat = " + QString::number(gidPatient);
        QSqlQuery RefODquery(RefODrequete,proc->getDataBase());
        RefODquery.last();
        if (RefODquery.value(1).toString() != "")  // On ne peut pas utiliser if (query.size() > 0) dans ce cas car l'opérateur max retourne toujours un size() = 1, même quand il n'y a pas de réponse...
            RefractionOD += RefODquery.value(1).toString();
        else
        {
            QString RefODVLrequete = "select max(idrefraction), formuleOD from " NOM_TABLE_REFRACTION " where quelleMesure = 'R' and quelleDistance = 'L' and idPat = " + QString::number(gidPatient);
            QSqlQuery RefODVLquery(RefODVLrequete,proc->getDataBase());
            RefODVLquery.last();
            if (RefODVLquery.value(1).toString() != "")
                RefractionOD += RefODVLquery.value(1).toString();
        }

        QString RefOGrequete    = "select max(idrefraction), formuleOG from " NOM_TABLE_REFRACTION " where quelleMesure = 'R' and quelleDistance = '2' and idPat = " + QString::number(gidPatient);
        QSqlQuery RefOGquery(RefOGrequete,proc->getDataBase());
        RefOGquery.last();
        if (RefOGquery.value(1).toString() != "")
            RefractionOG += RefOGquery.value(1).toString();
        else
        {
            QString RefOGVLrequete = "select max(idrefraction), formuleOG from " NOM_TABLE_REFRACTION " where quelleMesure = 'R' and quelleDistance = 'L' and idPat = " + QString::number(gidPatient);
            QSqlQuery RefOGVLquery(RefOGVLrequete,proc->getDataBase());
            RefOGVLquery.last();
            if (RefOGVLquery.value(1).toString() != "")
                RefractionOG += RefOGVLquery.value(1).toString();
        }
        if (RefractionOD != "")     UiDLg_BlOrtho->AVODlineEdit->setText(RefractionOD);
        if (RefractionOG != "")     UiDLg_BlOrtho->AVOGlineEdit->setText(RefractionOG);
        UiDLg_BlOrtho->OcclAlternlabel->setVisible(gDDNPatient.daysTo(QDate::currentDate()) < 730);
        UiDLg_BlOrtho->OcclAlterncomboBox->setVisible(gDDNPatient.daysTo(QDate::currentDate()) < 730);
    }
    if (Dlg_BlOrtho->exec()> 0)
    {
        if (!nouveauBO)
        {
            ui->ActeMotiftextEdit->setText("");
            ui->ActeTextetextEdit->setText("");
            ui->ActeConclusiontextEdit->setText("");
        }
        QString Reponse = Dlg_BlOrtho->calcReponsehTml();

        // Compléter le champ Motif et mettre à jour l'affichage de ActeMotiftextEdit --------------------------------------------------------------------------------
        QString texte = ui->ActeMotiftextEdit->toHtml();
        QString plaintext = ui->ActeMotiftextEdit->toPlainText();
        QString Motif= UiDLg_BlOrtho->MotiftextEdit->toPlainText();
        if (Motif != "")
        {
            for (int i= 0; i<Motif.size();i++)
                if (Motif.at(i).unicode() == 10) Motif.replace(Motif.at(i),"<br>");
            Motif = "<p style = \"margin-top:0px; margin-bottom:0px;margin-left: 0px\"><td width=\"70\"><font color = \""
                    + proc->CouleurTitres + "\">" + tr("Motif:") + "</font></td><td width=\"300\">" + Motif + "</td></p>";
        }

        if (plaintext.size() == 0)
            ui->ActeMotiftextEdit->setText("<p style = \"margin-top:0px; margin-bottom:0px;margin-left: 0px\">" + tr("Bilan orthoptique") + "</p>" + Motif);
        else
        {
            bool a = true;
            while (a) {
                // il faut retirer la dernière ligne du html qui contient le retour à la ligne
                int debut = texte.lastIndexOf("<p");
                int fin   = texte.lastIndexOf("</p>");
                int longARetirer = fin - debut + 4;
                if (texte.mid(debut,longARetirer).contains("-qt-paragraph-type:empty;"))
                    texte.remove(debut,longARetirer);
                else a = false;
            }
            //Motif
            QString Motif = UiDLg_BlOrtho->MotiftextEdit->toPlainText();
            texte += "<p style = \"margin-top:0px; margin-bottom:0px;margin-left: 0px\">" + tr("Bilan orthoptique") + "</p>";
            texte += Motif;
            ui->ActeMotiftextEdit->setText(texte);
        }
        QString updaterequete = "UPDATE " NOM_TABLE_ACTES " SET ActeMotif = '" + proc->CorrigeApostrophe(ui->ActeMotiftextEdit->toHtml()) +
                                "' where idActe = " + ui->idActelineEdit->text();
        QSqlQuery UpdateUpMotifTextEditQuery (updaterequete,db);
        proc->TraiteErreurRequete(UpdateUpMotifTextEditQuery,updaterequete,tr("Impossible de mettre à jour le champ Motif !"));

        // Compléter le Champ Texte et mettre à jour l'affichage de ActeTextetextEdit ----------------------------------------------------------------------------------
        texte = ui->ActeTextetextEdit->toHtml();
        plaintext = ui->ActeTextetextEdit->toPlainText();
        if (plaintext.size() == 0)
            ui->ActeTextetextEdit->setText(Reponse);
        else
        {
            bool a = true;
            while (a) {
                // il faut retirer la dernière ligne du html qui contient le retour à la ligne
                int debut = texte.lastIndexOf("<p");
                int fin   = texte.lastIndexOf("</p>");
                int longARetirer = fin - debut + 4;
                if (texte.mid(debut,longARetirer).contains("-qt-paragraph-type:empty;"))
                    texte.remove(debut,longARetirer);
                else a = false;
            }
            texte += Reponse;
            ui->ActeTextetextEdit->setText(texte);
        }
        updaterequete =  "UPDATE " NOM_TABLE_ACTES " SET ActeTexte = '" + proc->CorrigeApostrophe(ui->ActeTextetextEdit->toHtml()) +
                "' where idActe = " + ui->idActelineEdit->text();
        QSqlQuery UpdateUpTextEditQuery (updaterequete,db);
        proc->TraiteErreurRequete(UpdateUpTextEditQuery,updaterequete, tr("Impossible de mettre à jour le champ Texe !"));

        // Compléter le Champ Conclusion et mettre à jour l'affichage de ActeConclusiontextEdit ----------------------------------------------------------------------------------
        QString Concl = UiDLg_BlOrtho->ConclusiontextEdit->toPlainText();
        if (Concl != "")
        {
            Concl       = "<p style = \"margin-top:0px; margin-bottom:0px;margin-left: 0px\">" + Concl + "</p>";
            texte       = ui->ActeConclusiontextEdit->toHtml();
            plaintext   = ui->ActeConclusiontextEdit->toPlainText();
            if (plaintext.size() == 0)
                ui->ActeConclusiontextEdit->setText(Concl);
            else
            {
                bool a = true;
                while (a) {
                    // il faut retirer la dernière ligne du html qui contient le retour à la ligne
                    int debut = texte.lastIndexOf("<p");
                    int fin   = texte.lastIndexOf("</p>");
                    int longARetirer = fin - debut + 4;
                    if (texte.mid(debut,longARetirer).contains("-qt-paragraph-type:empty;"))
                        texte.remove(debut,longARetirer);
                    else a = false;
                }
                texte += Concl;
                ui->ActeConclusiontextEdit->setText(texte);
            }
            updaterequete =  "UPDATE " NOM_TABLE_ACTES " SET ActeConclusion = '" + proc->CorrigeApostrophe(ui->ActeConclusiontextEdit->toHtml()) +
                    "' where idActe = " + ui->idActelineEdit->text();
            QSqlQuery UpdateUpTextEditQuery (updaterequete,db);
            proc->TraiteErreurRequete(UpdateUpTextEditQuery,updaterequete, tr("Impossible de mettre à jour le champ Texte !"));
        }

        //Mettre à jour la table bilanorrtho
        QString deleteblorthorequete = "delete from " NOM_TABLE_BILANORTHO " where idBilanOrtho = " + ui->idActelineEdit->text();
        QString bilanorthorequete = "insert into " NOM_TABLE_BILANORTHO
                " (idBilanOrtho, Motif, AVOD, AVOG, OcclAltern"                                 // 0,1,2,3,4
                ", Wirt, WirtAnimaux, Lang, ODirecteur, Orientation"                            // 5,6,7,8,9
                ", EcranVLSC, EcranVLSCD, EcranfixresVLSC, EcranVPSC, EcranVPSCD"               // 10,11,12,13,14
                ", EcranfixresVPSC, EcranVLASC, EcranVLASCD, EcranfixresVLASC, EcranVPASC"      // 15,16,17,18,19
                ", EcranVPASCD, EcranfixresVPASC, MaddoxVLSC, MaddoxVLSCD, MaddoxVPSC"          // 20,21,22,23,24
                ", MaddoxVPSCD, MaddoxVLASC, MaddoxVLASCD, MaddoxVPASC, MaddoxVPASCD"           // 25,26,27,28,29
                ", Motilite, PPC, PPCComment, Saccades, Poursuite"                              // 30,31,32,33,34
                ", Worth1, Worth2, Bagolini1, Bagolini2, VergenceVLD"                           // 35,36,37,38,39
                ", VergenceVLC, VergenceVPD, VergenceVPC, Synopt1, Synopt2"                     // 40,41,42,43,44
                ", Synopt3, Conclusion, TNO, VergenceRestVLD, VergenceRestVPD"                  // 45,46,47,48,49
                ", HEcranVLSC, HEcranVLSCD, HEcranfixresVLSC, HEcranVPSC, HEcranVPSCD"          // 50,51,52,53,54
                ", HEcranfixresVPSC, HEcranVLASC, HEcranVLASCD, HEcranfixresVLASC, HEcranVPASC" // 55,56,57,58,59
                ", HEcranVPASCD, HEcranfixresVPASC, HMaddoxVLSC, HMaddoxVLSCD, HMaddoxVPSC"     // 60,61,62,63,64
                ", HMaddoxVPSCD, HMaddoxVLASC, HMaddoxVLASCD, HMaddoxVPASC, HMaddoxVPASCD"      // 65,66,67,68,69
                ") \nVALUES \n(";
        bilanorthorequete += ui->idActelineEdit->text();                                                                        //0
        bilanorthorequete += ", '" + proc->CorrigeApostrophe(UiDLg_BlOrtho->MotiftextEdit->toHtml()) + "'\n";                   //1
        bilanorthorequete += ", '" + proc->CorrigeApostrophe(UiDLg_BlOrtho->AVODlineEdit->text()) + "'\n";                      //2
        bilanorthorequete += ", '" + proc->CorrigeApostrophe(UiDLg_BlOrtho->AVOGlineEdit->text()) + "'\n";                      //3
        bilanorthorequete += ", '" + proc->CorrigeApostrophe(UiDLg_BlOrtho->OcclAlterncomboBox->currentText()) + "'\n";         //4
        bilanorthorequete += ", '" + proc->CorrigeApostrophe(UiDLg_BlOrtho->WirtcomboBox->currentText()) + "'\n";               //5
        bilanorthorequete += ", '" + proc->CorrigeApostrophe(UiDLg_BlOrtho->AnimauxWirtcomboBox->currentText()) + "'\n";        //6
        bilanorthorequete += ", '" + proc->CorrigeApostrophe(UiDLg_BlOrtho->LangcomboBox->currentText()) + "'\n";               //7 Lang
        if (UiDLg_BlOrtho->ODdirecteurradioButton->isChecked())                                                                 //8 ODirecteur
            bilanorthorequete += ", 'D'";
        else if (UiDLg_BlOrtho->OGdirecteurradioButton->isChecked())
            bilanorthorequete += ", 'G'";
        else
            bilanorthorequete += ", ''";
        if (UiDLg_BlOrtho->ODOrientationradioButton->isChecked())                                                               //9 Orientation
            bilanorthorequete += ", 'D'";
        else if (UiDLg_BlOrtho->OGOrientationradioButton->isChecked())
            bilanorthorequete += ", 'G'";
        else
            bilanorthorequete += ", ''";
        bilanorthorequete += ", '" + proc->CorrigeApostrophe(UiDLg_BlOrtho->EcranVLSCcomboBox->currentText()) + "'\n";          //10 EcranVLSC
        if (UiDLg_BlOrtho->EcranVLSCDcomboBox->currentText() != "-")                                                            //11 EcranVLSCD
            bilanorthorequete += ", " + proc->CorrigeApostrophe(UiDLg_BlOrtho->EcranVLSCDcomboBox->currentText()) + "\n";
        else
            bilanorthorequete += ", null";
        if (UiDLg_BlOrtho->fixSCVLcomboBox->currentText() != "-")                                                               //12 EcranfixresVLSC
            bilanorthorequete += ", '" + proc->CorrigeApostrophe(UiDLg_BlOrtho->fixSCVLcomboBox->currentText()) + "'\n";
        else
            bilanorthorequete += ", null";
        bilanorthorequete += ", '" + proc->CorrigeApostrophe(UiDLg_BlOrtho->EcranVPSCcomboBox->currentText()) + "'\n";          //13 EcranVPSC
        if (UiDLg_BlOrtho->EcranVPSCDcomboBox->currentText() != "-")                                                            //14 ECranVPSCD
            bilanorthorequete += ", " + proc->CorrigeApostrophe(UiDLg_BlOrtho->EcranVPSCDcomboBox->currentText()) + "\n";
        else
            bilanorthorequete += ", null";
        if (UiDLg_BlOrtho->fixSCVPcomboBox->currentText() != "-")                                                               //15 EcranfixresVPSC
            bilanorthorequete += ", '" + proc->CorrigeApostrophe(UiDLg_BlOrtho->fixSCVPcomboBox->currentText()) + "'\n";
        else
            bilanorthorequete += ", null";
        bilanorthorequete += ", '" + proc->CorrigeApostrophe(UiDLg_BlOrtho->EcranVLASCcomboBox->currentText()) + "'\n";         //16 EcranVLASC
        if (UiDLg_BlOrtho->EcranVLASCDcomboBox->currentText() != "-")                                                           //17 EcranVLASCD
            bilanorthorequete += ", " + proc->CorrigeApostrophe(UiDLg_BlOrtho->EcranVLASCDcomboBox->currentText()) + "\n";
        else
            bilanorthorequete += ", null";
        if (UiDLg_BlOrtho->fixASCVLcomboBox->currentText() != "-")                                                              //18 EcranfixresVLASC
            bilanorthorequete += ", '" + proc->CorrigeApostrophe(UiDLg_BlOrtho->fixASCVLcomboBox->currentText()) + "'\n";
        else
            bilanorthorequete += ", null";
        bilanorthorequete += ", '" + proc->CorrigeApostrophe(UiDLg_BlOrtho->EcranVPASCcomboBox->currentText()) + "'\n";         //19 EcranVPASC
        if (UiDLg_BlOrtho->EcranVPASCDcomboBox->currentText() != "-")                                                           //20 EcranVPASCD
            bilanorthorequete += ", " + proc->CorrigeApostrophe(UiDLg_BlOrtho->EcranVPASCDcomboBox->currentText()) + "\n";
        else
            bilanorthorequete += ", null";
        if (UiDLg_BlOrtho->fixASCVPcomboBox->currentText() != "-")                                                              //21 EcranfixresVPASC
            bilanorthorequete += ", '" + proc->CorrigeApostrophe(UiDLg_BlOrtho->fixASCVPcomboBox->currentText()) + "'\n";
        else
            bilanorthorequete += ", null";
        bilanorthorequete += ", '" + proc->CorrigeApostrophe(UiDLg_BlOrtho->MaddoxVLSCcomboBox->currentText()) + "'\n";
        if (UiDLg_BlOrtho->MaddoxVLSCDcomboBox->currentText() != "-")
            bilanorthorequete += ", '" + proc->CorrigeApostrophe(UiDLg_BlOrtho->MaddoxVLSCDcomboBox->currentText()) + "'\n";
        else
            bilanorthorequete += ", null";
        bilanorthorequete += ", '" + proc->CorrigeApostrophe(UiDLg_BlOrtho->MaddoxVPSCcomboBox->currentText()) + "'\n";
        if (UiDLg_BlOrtho->MaddoxVPSCDcomboBox->currentText() != "-")
            bilanorthorequete += ", '" + proc->CorrigeApostrophe(UiDLg_BlOrtho->MaddoxVPSCDcomboBox->currentText()) + "'\n";
        else
            bilanorthorequete += ", null";
        bilanorthorequete += ", '" + proc->CorrigeApostrophe(UiDLg_BlOrtho->MaddoxVLASCcomboBox->currentText()) + "'\n";
        if (UiDLg_BlOrtho->MaddoxVLASCDcomboBox->currentText() != "-")
            bilanorthorequete += ", '" + proc->CorrigeApostrophe(UiDLg_BlOrtho->MaddoxVLASCDcomboBox->currentText()) + "'\n";
        else
            bilanorthorequete += ", null";
        bilanorthorequete += ", '" + proc->CorrigeApostrophe(UiDLg_BlOrtho->MaddoxVPASCcomboBox->currentText()) + "'\n";
        if (UiDLg_BlOrtho->MaddoxVPASCDcomboBox->currentText() != "-")
            bilanorthorequete += ", '" + proc->CorrigeApostrophe(UiDLg_BlOrtho->MaddoxVPASCDcomboBox->currentText()) + "'\n";
        else
            bilanorthorequete += ", null";
        bilanorthorequete += ", '" + proc->CorrigeApostrophe(UiDLg_BlOrtho->MotilitetextEdit->toHtml()) + "'\n";
        bilanorthorequete += ", '" + proc->CorrigeApostrophe(UiDLg_BlOrtho->PPCcomboBox->currentText()) + "'\n";
        bilanorthorequete += ", '" + proc->CorrigeApostrophe(UiDLg_BlOrtho->PPClineEdit->text()) + "'\n";
        bilanorthorequete += ", '" + proc->CorrigeApostrophe(UiDLg_BlOrtho->SaccadeslineEdit->text()) + "'\n";
        bilanorthorequete += ", '" + proc->CorrigeApostrophe(UiDLg_BlOrtho->PoursuitelineEdit->text()) + "'\n";
        bilanorthorequete += ", '" + proc->CorrigeApostrophe(UiDLg_BlOrtho->Worth1lineEdit->text()) + "'\n";
        bilanorthorequete += ", '" + proc->CorrigeApostrophe(UiDLg_BlOrtho->Worth2lineEdit->text()) + "'\n";
        bilanorthorequete += ", '" + proc->CorrigeApostrophe(UiDLg_BlOrtho->Bagolini1lineEdit->text()) + "'\n";
        bilanorthorequete += ", '" + proc->CorrigeApostrophe(UiDLg_BlOrtho->Bagolini2lineEdit->text()) + "'\n";
        bilanorthorequete += ", '" + proc->CorrigeApostrophe(UiDLg_BlOrtho->VergenceDLcomboBox->currentText()) + "'\n";
        bilanorthorequete += ", '" + proc->CorrigeApostrophe(UiDLg_BlOrtho->VergenceCLcomboBox->currentText()) + "'\n";
        bilanorthorequete += ", '" + proc->CorrigeApostrophe(UiDLg_BlOrtho->VergenceDPcomboBox->currentText()) + "'\n";
        bilanorthorequete += ", '" + proc->CorrigeApostrophe(UiDLg_BlOrtho->VergenceCPcomboBox->currentText()) + "'\n";
        bilanorthorequete += ", '" + proc->CorrigeApostrophe(UiDLg_BlOrtho->Degre1lineEdit->text()) + "'\n";
        bilanorthorequete += ", '" + proc->CorrigeApostrophe(UiDLg_BlOrtho->Degre2lineEdit->text()) + "'\n";
        bilanorthorequete += ", '" + proc->CorrigeApostrophe(UiDLg_BlOrtho->Degre3lineEdit->text()) + "'\n";
        bilanorthorequete += ", '" + proc->CorrigeApostrophe(UiDLg_BlOrtho->ConclusiontextEdit->toHtml()) + "'\n";
        bilanorthorequete += ", '" + proc->CorrigeApostrophe(UiDLg_BlOrtho->TNOcomboBox->currentText()) + "'\n";
        bilanorthorequete += ", '" + proc->CorrigeApostrophe(UiDLg_BlOrtho->VergenceRestDLcomboBox->currentText()) + "'\n";
        bilanorthorequete += ", '" + proc->CorrigeApostrophe(UiDLg_BlOrtho->VergenceRestDPcomboBox->currentText()) + "'\n";
        bilanorthorequete += ", '" + proc->CorrigeApostrophe(UiDLg_BlOrtho->HEcranVLSCcomboBox->currentText()) + "'\n";         //50 HEcranVLSC
        if (UiDLg_BlOrtho->HEcranVLSCDcomboBox->currentText() != "-")                                                           //51 HEcranVLSCD
            bilanorthorequete += ", " + proc->CorrigeApostrophe(UiDLg_BlOrtho->HEcranVLSCDcomboBox->currentText()) + "\n";
        else
            bilanorthorequete += ", null";
        if (UiDLg_BlOrtho->HfixSCVLcomboBox->currentText() != "-")                                                              //52 HEcranfixresVLSC
            bilanorthorequete += ", '" + proc->CorrigeApostrophe(UiDLg_BlOrtho->HfixSCVLcomboBox->currentText()) + "'\n";
        else
            bilanorthorequete += ", null";
        bilanorthorequete += ", '" + proc->CorrigeApostrophe(UiDLg_BlOrtho->HEcranVPSCcomboBox->currentText()) + "'\n";         //53 HEcranVPSC
        if (UiDLg_BlOrtho->HEcranVPSCDcomboBox->currentText() != "-")                                                           //54 HECranVPSCD
            bilanorthorequete += ", " + proc->CorrigeApostrophe(UiDLg_BlOrtho->HEcranVPSCDcomboBox->currentText()) + "\n";
        else
            bilanorthorequete += ", null";
        if (UiDLg_BlOrtho->HfixSCVPcomboBox->currentText() != "-")                                                              //55 HEcranfixresVPSC
            bilanorthorequete += ", '" + proc->CorrigeApostrophe(UiDLg_BlOrtho->HfixSCVPcomboBox->currentText()) + "'\n";
        else
            bilanorthorequete += ", null";
        bilanorthorequete += ", '" + proc->CorrigeApostrophe(UiDLg_BlOrtho->HEcranVLASCcomboBox->currentText()) + "'\n";        //56 HEcranVLASC
        if (UiDLg_BlOrtho->HEcranVLASCDcomboBox->currentText() != "-")                                                          //57 HEcranVLASCD
            bilanorthorequete += ", " + proc->CorrigeApostrophe(UiDLg_BlOrtho->HEcranVLASCDcomboBox->currentText()) + "\n";
        else
            bilanorthorequete += ", null";
        if (UiDLg_BlOrtho->HfixASCVLcomboBox->currentText() != "-")                                                             //58 HEcranfixresVLASC
            bilanorthorequete += ", '" + proc->CorrigeApostrophe(UiDLg_BlOrtho->HfixASCVLcomboBox->currentText()) + "'\n";
        else
            bilanorthorequete += ", null";
        bilanorthorequete += ", '" + proc->CorrigeApostrophe(UiDLg_BlOrtho->HEcranVPASCcomboBox->currentText()) + "'\n";        //59 HEcranVPASC
        if (UiDLg_BlOrtho->HEcranVPASCDcomboBox->currentText() != "-")                                                          //60 HEcranVPASCD
            bilanorthorequete += ", " + proc->CorrigeApostrophe(UiDLg_BlOrtho->HEcranVPASCDcomboBox->currentText()) + "\n";
        else
            bilanorthorequete += ", null";
        if (UiDLg_BlOrtho->HfixASCVPcomboBox->currentText() != "-")                                                             //61 HEcranfixresVPASC
            bilanorthorequete += ", '" + proc->CorrigeApostrophe(UiDLg_BlOrtho->HfixASCVPcomboBox->currentText()) + "'\n";
        else
            bilanorthorequete += ", null";
        bilanorthorequete += ", '" + proc->CorrigeApostrophe(UiDLg_BlOrtho->HMaddoxVLSCcomboBox->currentText()) + "'\n";        //62 HMaddoxVLSC
        if (UiDLg_BlOrtho->HMaddoxVLSCDcomboBox->currentText() != "-")                                                          //63 HMaddoxVLSCD
            bilanorthorequete += ", '" + proc->CorrigeApostrophe(UiDLg_BlOrtho->HMaddoxVLSCDcomboBox->currentText()) + "'\n";
        else
            bilanorthorequete += ", null";
        bilanorthorequete += ", '" + proc->CorrigeApostrophe(UiDLg_BlOrtho->HMaddoxVPSCcomboBox->currentText()) + "'\n";        //64 HMaddoxVPSC
        if (UiDLg_BlOrtho->HMaddoxVPSCDcomboBox->currentText() != "-")                                                          //65 HMaddoxVPSCD
            bilanorthorequete += ", '" + proc->CorrigeApostrophe(UiDLg_BlOrtho->HMaddoxVPSCDcomboBox->currentText()) + "'\n";
        else
            bilanorthorequete += ", null";
        bilanorthorequete += ", '" + proc->CorrigeApostrophe(UiDLg_BlOrtho->HMaddoxVLASCcomboBox->currentText()) + "'\n";       //66 HMaddoxVLASC
        if (UiDLg_BlOrtho->HMaddoxVLASCDcomboBox->currentText() != "-")                                                         //67 HMaddoxVLASCD
            bilanorthorequete += ", '" + proc->CorrigeApostrophe(UiDLg_BlOrtho->HMaddoxVLASCDcomboBox->currentText()) + "'\n";
        else
            bilanorthorequete += ", null";
        bilanorthorequete += ", '" + proc->CorrigeApostrophe(UiDLg_BlOrtho->HMaddoxVPASCcomboBox->currentText()) + "'\n";       //68 HMaddoxVPASC
        if (UiDLg_BlOrtho->HMaddoxVPASCDcomboBox->currentText() != "-")                                                         //69 HMaddoxVPASCD
            bilanorthorequete += ", '" + proc->CorrigeApostrophe(UiDLg_BlOrtho->HMaddoxVPASCDcomboBox->currentText()) + "'\n";
        else
            bilanorthorequete += ", null";
        bilanorthorequete += ");";

        QSqlQuery ("SET AUTOCOMMIT = 0;", db);
        QString lockrequete = "LOCK TABLES " NOM_TABLE_BILANORTHO " WRITE;";
        QSqlQuery lockquery (lockrequete, db);
        if (proc->TraiteErreurRequete(lockquery,lockrequete, tr("Impossible de verrouiller ") + NOM_TABLE_BILANORTHO)){
            proc->rollback(db);    return;}
        QSqlQuery deleteblorthoquery(deleteblorthorequete,db);
        if (proc->TraiteErreurRequete(deleteblorthoquery,deleteblorthorequete,"")){
            proc->rollback(db); return;}
        QSqlQuery blorthoquery(bilanorthorequete,db);
        if (proc->TraiteErreurRequete(blorthoquery,bilanorthorequete,"")){
            proc->rollback(db); return;}
        proc->commit(db);

        ui->ActeTextetextEdit->setFocus();
        ui->ActeTextetextEdit->moveCursor(QTextCursor::End);
        MAJDocsExternes();  //CreerBilanOrtho()
    }
    else
        if (nouveauBO) SupprimerActe();

    Dlg_BlOrtho->close();
    delete Dlg_BlOrtho;
    MAJDocsExternes();  //CreerBilanOrtho()
}

void Rufus::Slot_ChercherDepuisListepushButtonClicked()
{
    ChercherDepuisListe();
}

void Rufus::Slot_CreerDossierpushButtonClicked()
{
    if (gMode == NouveauDossier)
    {
        gSexePat ="";
        CreerDossier();
    }
    else
        if (ui->PatientsListeTableView->selectionModel()->selectedIndexes().size() > 0)
            ChoixDossier(gListePatientsModel->itemFromIndex(ui->PatientsListeTableView->selectionModel()->selectedIndexes().at(0))->text().toInt());
}

void Rufus::Slot_EnableCreerDossierButton()
{
    if (gMode != NouveauDossier)
        ui->CreerDossierpushButton->setEnabled(ui->PatientsListeTableView->selectionModel()->selectedIndexes().size()>0);
    else
        ui->CreerDossierpushButton->setEnabled(ui->CreerNomlineEdit->text() != "" && ui->CreerPrenomlineEdit->text() != "");
}

void Rufus::EnregistreDocScanner()
{
    int idpat = 0;
    QString nomprenompat;
    if (ui->tabWidget->currentWidget() == ui->tabList) {
        QModelIndex idx     = ui->PatientsListeTableView->selectionModel()->selectedIndexes().at(0);
        idpat               = gListePatientsModel->itemFromIndex(idx)->text().toInt();
        QModelIndex idx2    = ui->PatientsListeTableView->selectionModel()->selectedIndexes().at(1);
        nomprenompat        = gListePatientsModel->itemFromIndex(idx2)->text();
    }
    else if (ui->tabWidget->currentWidget() == ui->tabDossier)
    {
        idpat           = gidPatient;
        nomprenompat    = gNomPatient.toUpper() + " " + gPrenomPatient;
    }
    if (idpat == 0)
    {
        UpMessageBox::Watch(this,tr("Pas de dossier patient en cours"));
        return;
    }

    Dlg_DocsScan = new dlg_docsscanner(proc,idpat, this);
    Dlg_DocsScan->setWindowTitle(tr("Enregistrer un document issu du scanner pour ") + nomprenompat);
    Dlg_DocsScan->show();
    Dlg_DocsScan->Slot_NavigueVers("Fin");
}

void Rufus::EnregistreVideo()
{
    int idpat = 0;
    QString nomprenompat;
    if (ui->tabWidget->currentWidget() == ui->tabList) {
        QModelIndex idx     = ui->PatientsListeTableView->selectionModel()->selectedIndexes().at(0);
        idpat               = gListePatientsModel->itemFromIndex(idx)->text().toInt();
        QModelIndex idx2    = ui->PatientsListeTableView->selectionModel()->selectedIndexes().at(1);
        nomprenompat        = gListePatientsModel->itemFromIndex(idx2)->text();
    }
    else if (ui->tabWidget->currentWidget() == ui->tabDossier)
    {
        idpat           = gidPatient;
        nomprenompat    = gNomPatient.toUpper() + " " + gPrenomPatient;
    }
    if (idpat == 0)
    {
        UpMessageBox::Watch(this,tr("Pas de dossier patient en cours"));
        return;
    }
    Dlg_DocsVideo = new dlg_docsvideo(proc,idpat, this);
    Dlg_DocsVideo->setWindowTitle(tr("Enregistrer une video dans le dossier de ") + nomprenompat);
    Dlg_DocsVideo->show();
    Dlg_DocsVideo->Slot_NavigueVers("Fin");
}

void Rufus::Slot_FiltrecheckBoxClicked()
{
    if (ui->FiltrecheckBox->isChecked() && (ui->CreerNomlineEdit->text() != "" || ui->CreerPrenomlineEdit->text() != ""))
        ChercheNomFiltre();
    else
    {
        QModelIndexList Listitems;
        Listitems = ui->PatientsListeTableView->selectionModel()->selectedIndexes();
        int idpat = -1;
        if (Listitems.size() > 0)
            idpat =  gListePatientsModel->itemFromIndex(Listitems.at(0))->text().toInt();

        if (ListeRestreinte)     Remplir_ListePatients_TableView(grequeteListe,"","");
        QList<QStandardItem*> listits = gListePatientsModel->findItems(QString::number(idpat));
        if (!listits.isEmpty())
        {
            ui->PatientsListeTableView->selectRow(0);
            ui->PatientsListeTableView->scrollTo(listits.at(0)->index(),QAbstractItemView::PositionAtTop);
            return;
        }
        CalcNbDossiers();
    }
    if (ui->PatientsListeTableView->selectionModel()->selectedIndexes().size()>0)
        ui->PatientsListeTableView->scrollTo(ui->PatientsListeTableView->selectionModel()->selectedIndexes().at(0),QAbstractItemView::PositionAtTop);
}

void Rufus::Slot_FiltreSalleDAttente(int index)
{
    int idx         = gSalDatTab->tabData(index).toInt();
    QString usrlog  = gSalDatTab->tabText(index);
    if (idx==-1)
        for(int i=0; i<ui->SalleDAttenteupTableWidget->rowCount(); i++)
            ui->SalleDAttenteupTableWidget->setRowHidden(i,false);
    else
        for(int i=0; i<ui->SalleDAttenteupTableWidget->rowCount(); i++)
        {
            UpLabel *lbl = dynamic_cast<UpLabel*>(ui->SalleDAttenteupTableWidget->cellWidget(i,6));
            ui->SalleDAttenteupTableWidget->setRowHidden(i,lbl->text() != usrlog);
        }
}

void Rufus::Slot_ActiveActeAccueil(int row)
{
    UpLabel *lblr = dynamic_cast<UpLabel*>(ui->AccueilupTableWidget->cellWidget(row,6));
    if (lblr == Q_NULLPTR)
        return;
    int idparent = lblr->text().toInt();
    for (int i=0; i<ui->AccueilupTableWidget->rowCount(); i++)
    {
        UpLabel *lbl = dynamic_cast<UpLabel*>(ui->AccueilupTableWidget->cellWidget(i,6));
        if (lbl != Q_NULLPTR)
            if (lbl->text().toInt() != idparent)
                ui->AccueilupTableWidget->setRangeSelected(QTableWidgetSelectionRange(i,0,i,6),false);
    }
}

void Rufus::Slot_FiltreAccueil(int idx)
{
    int idparent        = gAccueilTab->tabData(idx).toInt();
    for(int i=0; i<ui->AccueilupTableWidget->rowCount(); i++)
    {
        UpLabel *lbl = dynamic_cast<UpLabel*>(ui->AccueilupTableWidget->cellWidget(i,6));
        ui->AccueilupTableWidget->setRowHidden(i,lbl->text() != QString::number(idparent));
    }
}

void Rufus::Slot_GestionComptes()
{
    Dlg_Cmpt = new dlg_comptes(proc, this);
    if(Dlg_Cmpt->getInitOK())
        Dlg_Cmpt->exec();
}

/*-----------------------------------------------------------------------------------------------------------------
-- Identification de l'utilisateur --------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void    Rufus::Slot_IdentificationUser()
{
    QList<dlg_paiement *> PaimtList = findChildren<dlg_paiement*>();
    if (PaimtList.size()>0)
        for (int i=0; i<PaimtList.size();i++)
            if (PaimtList.at(i)->isVisible())
            {
                QSound::play(NOM_ALARME);
                PaimtList.at(i)->raise();
                return;
            }

    QString Base;
    switch (proc->gMode) {
    case Procedures::Poste:
        Base = "BDD_POSTE";
        break;
    case Procedures::ReseauLocal:
        Base = "BDD_LOCAL";
        break;
    case Procedures::Distant:
        Base = "BDD_DISTANT";
        break;
    default:
        Base = "BDD_POSTE";
        break;
    }
    QString Serveur  = proc->gsettingsIni->value(Base + "/Serveur").toString();
    int     Port     = proc->gsettingsIni->value(Base + "/Port").toInt();
    bool    SSL      = (proc->gMode == Procedures::Distant);
    bool    ChgUsr   = true;
    if (!proc->IdentificationUser(Serveur, Port, SSL, Base, ChgUsr))
        gIdentificationOK = false;
    if (gidUser == proc->getidUser())
        return;

    gidUser = proc->getidUser();
    proc->ChargeDataUser(gidUser);
    ChargeDataUser();
    proc->UpdVerrouSalDat();

    QString modeconnexion = "";
    if (proc->getModeConnexion() == Procedures::Poste)
        modeconnexion = tr("monoposte");
    else if (proc->getModeConnexion() == Procedures::ReseauLocal)
        modeconnexion = tr("réseau local");
    if (proc->getModeConnexion() == Procedures::Distant)
    {
        modeconnexion = tr("accès distant - connexion ");
        if (proc->gsettingsIni->value("BDD_DISTANT/SSL").toString() != "NO")
            modeconnexion += tr("cryptée (SSL)");
        else
            modeconnexion += tr("non cryptée");
    }

    setWindowTitle("Rufus - " + gUserLogin + " - " + gUserFonction + " - " + modeconnexion);
    if (gUserFonction == tr("Médecin") || gUserFonction == tr("Orthoptiste"))
        ReconstruitListesActes();
    InitMenus();
    gIdentificationOK = true;
    if (gIdentificationOK)
    {
        if (gMsgDialog != NULL)
            if (gMsgDialog->isVisible())
                gMsgDialog->close();
        Slot_VerifMessages();
    }
}

void Rufus::Slot_ExporteDocs()
{
    if (proc->getModeConnexion() == Procedures::Distant)
        return;
    QString B = proc->gsettingsIni->value("BDD_LOCAL/PrioritaireGestionDocs").toString();
    QString IpAdr = "";
    if (B=="YES")
        IpAdr = QHostInfo::localHostName() + " - prioritaire";
    else if (B=="NORM")
        IpAdr = QHostInfo::localHostName();
    else
        return;
    if (IpAdr !=  proc->PosteImportDocs())
        return;
    QString NomDirStockageImagerie;
    if (proc->getModeConnexion() == Procedures::Poste)
    {
        QSqlQuery dirquer("select dirimagerie from " NOM_TABLE_PARAMSYSTEME, db);
        dirquer.first();
        NomDirStockageImagerie = dirquer.value(0).toString();
    }
    else
        NomDirStockageImagerie = proc->gsettingsIni->value("BDD_LOCAL/DossierImagerie").toString();

    if (!QDir(NomDirStockageImagerie).exists() || NomDirStockageImagerie == "")
    {
        QString msg = tr("Le dossier de sauvegarde d'imagerie") + " <font color=\"red\"><b>" + NomDirStockageImagerie + "</b></font>" + tr(" n'existe pas");
        msg += "<br />" + tr("Renseignez un dossier valide dans") + " <font color=\"green\"><b>" + tr("Emplacement de stockage des documents archivés") + "</b></font>";
        QStringList listmsg;
        listmsg << msg;
        dlg_message(listmsg, 6000, false);
        return;
    }
    QString CheminEchecTransfrDir   = NomDirStockageImagerie + NOMDIR_ECHECSTRANSFERTS;
    QDir DirTrsferEchec;
    if (!QDir(CheminEchecTransfrDir).exists())
        if (!DirTrsferEchec.mkdir(CheminEchecTransfrDir))
        {
            QString msg = tr("Dossier de sauvegarde ") + "<font color=\"red\"><b>" + CheminEchecTransfrDir + "</b></font>" + tr(" invalide");
            QStringList listmsg;
            listmsg << msg;
            dlg_message(listmsg, 3000, false);
            return;
        }

    QString totreq = "SELECT idimpression FROM " NOM_TABLE_IMPRESSIONS " where jpg is not null or pdf is not null";
    //qDebug() << totreq;
    QSqlQuery totquer(totreq,db);
    int total = totquer.size();
    if (total>100)
    {
        int min = total/180;
        int hour = min/60;
        min = min - (hour*60);
        QString tps = QTime(hour,min).toString("H'H'mm'mn'");
        if (UpMessageBox::Question(this, tr("Nombreux documents à convertir"),
                                   tr("Il y a ") + QString::number(total) + tr(" documents à convertir.") +"\n" +
                                   tr("Cette procédure devrait durer environ ") + tps + "\n" +
                                   tr("et figera l'éxécution du programme durant ce temps") + tps + "\n" +
                                   tr("Voulez vous le faire maintenant?"))
                                   !=UpSmallButton::STARTBUTTON)
        {
            disconnect (gTimerVerifGestDocs,                SIGNAL(timeout()),                  this,   SLOT(Slot_ExporteDocs()));
            return;
        }
    }
    if (total==0)
        return;
    int faits = 0;
    QTime debut = QTime::currentTime();
    QStringList listmsg;
    QString duree;

    //-----------------------------------------------------------------------------------------------------------------------------------------
    //              LES JPG
    //-----------------------------------------------------------------------------------------------------------------------------------------
    QString req = "SELECT idimpression, idpat, SousTypeDoc, Dateimpression, jpg, lienversfichier, typedoc FROM " NOM_TABLE_IMPRESSIONS " where jpg is not null";
    //qDebug() << req;
    QSqlQuery exportjpgquer (req,db);
    for (int i=0; i<exportjpgquer.size(); i++)
    {
        exportjpgquer.seek(i);
        if (exportjpgquer.value(5).toString() != "")
        {
            QString CheminFichier = NomDirStockageImagerie + NOMDIR_IMAGES + exportjpgquer.value(5).toString();
            if (QFile(CheminFichier).exists())
            {
                QSqlQuery ("update " NOM_TABLE_IMPRESSIONS " set jpg = null where idimpression = " + exportjpgquer.value(0).toString(),db);
                continue;
            }
        }
        QString datetransfer            = exportjpgquer.value(3).toDate().toString("yyyy-MM-dd");
        QString CheminOKTransfrDir      = NomDirStockageImagerie + NOMDIR_IMAGES;
        QDir DirTrsferOK;
        if (!QDir(CheminOKTransfrDir).exists())
            if (!DirTrsferOK.mkdir(CheminOKTransfrDir))
            {
                QString msg = tr("Dossier de sauvegarde ") + "<font color=\"red\"><b>" + CheminOKTransfrDir + "</b></font>" + tr(" invalide");
                QStringList listmsg;
                listmsg << msg;
                dlg_message(listmsg, 3000, false);
                return;
            }
        CheminOKTransfrDir      = CheminOKTransfrDir + "/" + datetransfer;
        if (!QDir(CheminOKTransfrDir).exists())
            if (!DirTrsferOK.mkdir(CheminOKTransfrDir))
            {
                QString msg = tr("Dossier de sauvegarde ") + "<font color=\"red\"><b>" + CheminOKTransfrDir + "</b></font>" + tr(" invalide");
                QStringList listmsg;
                listmsg << msg;
                dlg_message(listmsg, 3000, false);
                return;
            }
        QString NomFileDoc = exportjpgquer.value(1).toString() + "_" + exportjpgquer.value(6).toString() + "-"
                + exportjpgquer.value(2).toString() + "_"
                + exportjpgquer.value(3).toDate().toString("yyyyMMdd") + "-" + QTime::currentTime().toString("HHmmss")
                + "-" + exportjpgquer.value(0).toString()  + ".jpg";
        QString CheminOKTransfrDoc      = CheminOKTransfrDir + "/" + NomFileDoc;
        QFile CC(CheminOKTransfrDoc);
        QImage  img;
        img.loadFromData(exportjpgquer.value(4).toByteArray());
        QPixmap pixmap;
        pixmap = pixmap.fromImage(img.scaledToWidth(2560,Qt::SmoothTransformation));
        if (!pixmap.save(CheminOKTransfrDoc, "jpeg"))
        {
            QString echectrsfername         = CheminEchecTransfrDir + "/0EchecTransferts - " + datetransfer + ".txt";
            QFile   echectrsfer(echectrsfername);
            if (echectrsfer.open(QIODevice::Append))
            {
                QTextStream out(&echectrsfer);
                out << NomFileDoc << "\n" ;
                echectrsfer.close();
                QFile CD(CheminEchecTransfrDir + "/" +NomFileDoc);
                if (CD.open(QIODevice::Append))
                {
                    QTextStream out(&CD);
                    out << exportjpgquer.value(4).toByteArray() ;
                }
            }
            QSqlQuery ("delete from  " NOM_TABLE_IMPRESSIONS " where idimpression = " + exportjpgquer.value(0).toString(),db);
            continue;
        }
        int     tauxcompress = 100;
        int sz = exportjpgquer.value(4).toByteArray().size();
        bool OK = true;
        while (sz > TAILLEMAXIIMAGES && tauxcompress > 10)
        {
            pixmap = pixmap.fromImage(img.scaledToWidth(2560,Qt::SmoothTransformation));
            if (CC.exists())
                CC.remove();
            if (!pixmap.save(CheminOKTransfrDoc, "jpeg",tauxcompress))
            {
                QString echectrsfername         = CheminEchecTransfrDir + "/0EchecTransferts - " + datetransfer + ".txt";
                QFile   echectrsfer(echectrsfername);
                if (echectrsfer.open(QIODevice::Append))
                {
                    QTextStream out(&echectrsfer);
                    out << NomFileDoc << "\n" ;
                    echectrsfer.close();
                    QFile CD(CheminEchecTransfrDir + "/" +NomFileDoc);
                    if (CD.open(QIODevice::Append))
                    {
                        QTextStream out(&CD);
                        out << exportjpgquer.value(4).toByteArray() ;
                    }
                }
                OK = false;
                break;
            }
            CC.open(QIODevice::ReadWrite);
            sz = CC.size();
            if (sz > TAILLEMAXIIMAGES)
                tauxcompress -= 10;
            CC.close();
        }
        if (!OK)
        {
            QSqlQuery ("delete from  " NOM_TABLE_IMPRESSIONS " where idimpression = " + exportjpgquer.value(0).toString(),db);
            continue;
        }
        CC.open(QIODevice::ReadWrite);
        CC.setPermissions(QFileDevice::ReadOther
                          | QFileDevice::ReadGroup
                          | QFileDevice::ReadOwner  | QFileDevice::WriteOwner
                          | QFileDevice::ReadUser   | QFileDevice::WriteUser);
        CC.close();
        QSqlQuery ("update " NOM_TABLE_IMPRESSIONS " set jpg = null, lienversfichier = '/" + datetransfer + "/" + NomFileDoc + "' where idimpression = " + exportjpgquer.value(0).toString(),db);
        faits ++;
        int nsec = debut.secsTo(QTime::currentTime());
        int min = nsec/60;
        int hour = min/60;
        min = min - (hour*60);
        nsec = nsec - (hour*3600) - (min*60);
        listmsg.clear();
        duree = QTime(hour,min,nsec).toString("HH:mm:ss");
        listmsg << "JPG - " + NomFileDoc + " - " + QString::number(faits) + "/" + QString::number(total) + " - "  + duree;
        QTime dieTime= QTime::currentTime().addMSecs(2);
        while (QTime::currentTime() < dieTime)
            QCoreApplication::processEvents(QEventLoop::AllEvents, 1);
        dlg_message(listmsg, 10);
    }

    //-----------------------------------------------------------------------------------------------------------------------------------------
    //              LES PDF
    //-----------------------------------------------------------------------------------------------------------------------------------------
    QString reqpdf = "SELECT idimpression, idpat, SousTypeDoc, Dateimpression, pdf, lienversfichier, compression, typedoc FROM " NOM_TABLE_IMPRESSIONS " where pdf is not null";
    QSqlQuery exportpdfquer (reqpdf,db);
    for (int i=0; i<exportpdfquer.size(); i++)
    {
        exportpdfquer.seek(i);
        if (exportpdfquer.value(5).toString() != "")
        {
            QString CheminFichier = NomDirStockageImagerie + NOMDIR_IMAGES + exportpdfquer.value(5).toString();
            if (QFile(CheminFichier).exists())
            {
                QSqlQuery ("update " NOM_TABLE_IMPRESSIONS " set pdf = null where idimpression = " + exportpdfquer.value(0).toString(),db);
                continue;
            }
        }
        QString datetransfer            = exportpdfquer.value(3).toDate().toString("yyyy-MM-dd");
        QString CheminOKTransfrDir      = NomDirStockageImagerie + NOMDIR_IMAGES;
        QDir DirTrsferOK;
        if (!QDir(CheminOKTransfrDir).exists())
            if (!DirTrsferOK.mkdir(CheminOKTransfrDir))
            {
                QString msg = tr("Dossier de sauvegarde ") + "<font color=\"red\"><b>" + CheminOKTransfrDir + "</b></font>" + tr(" invalide");
                QStringList listmsg;
                listmsg << msg;
                dlg_message(listmsg, 3000, false);
                return;
            }
        CheminOKTransfrDir      = CheminOKTransfrDir + "/" + datetransfer;
        if (!QDir(CheminOKTransfrDir).exists())
            if (!DirTrsferOK.mkdir(CheminOKTransfrDir))
            {
                QString msg = tr("Dossier de sauvegarde ") + "<font color=\"red\"><b>" + CheminOKTransfrDir + "</b></font>" + tr(" invalide");
                QStringList listmsg;
                listmsg << msg;
                dlg_message(listmsg, 3000, false);
                return;
            }
        QString NomFileDoc = exportpdfquer.value(1).toString() + "_" + exportpdfquer.value(7).toString() + "-"
                + exportpdfquer.value(2).toString() + "_"
                + exportpdfquer.value(3).toDate().toString("yyyyMMdd") + "-" + QTime::currentTime().toString("HHmmss")
                + "-" + exportpdfquer.value(0).toString()  + ".pdf";
        QString CheminOKTransfrDoc      = CheminOKTransfrDir + "/" + NomFileDoc;

        QByteArray bapdf;
        if (exportpdfquer.value(6).toString()=="1")
            bapdf.append(qUncompress(exportpdfquer.value(4).toByteArray()));
        else
            bapdf.append(exportpdfquer.value(4).toByteArray());

        Poppler::Document* document = Poppler::Document::loadFromData(bapdf);
        if (!document || document->isLocked() || document == 0)
        {
            QStringList listmsg;
            listmsg << tr("Impossible de charger le document ") + NomFileDoc;
            dlg_message(listmsg, 3000, false);
            QString echectrsfername         = CheminEchecTransfrDir + "/0EchecTransferts - " + datetransfer + ".txt";
            QFile   echectrsfer(echectrsfername);
            if (echectrsfer.open(QIODevice::Append))
            {
                QTextStream out(&echectrsfer);
                out << NomFileDoc << "\n" ;
                echectrsfer.close();
                QFile CD(CheminEchecTransfrDir + "/" + NomFileDoc);
                if (CD.open(QIODevice::Append))
                {
                    QTextStream out(&CD);
                    out << exportpdfquer.value(4).toByteArray() ;
                }
            }
            QString delreq = "delete from  " NOM_TABLE_IMPRESSIONS " where idimpression = " + exportpdfquer.value(0).toString();
            //qDebug() << delreq;
            QSqlQuery (delreq,db);
            delete document;
            continue;
        }
        Poppler::PDFConverter *doctosave = document->pdfConverter();
        doctosave->setOutputFileName(CheminOKTransfrDoc);
        doctosave->convert();

        QFile CC(CheminOKTransfrDoc);
        CC.open(QIODevice::ReadWrite);
        CC.setPermissions(QFileDevice::ReadOther
                          | QFileDevice::ReadGroup
                          | QFileDevice::ReadOwner  | QFileDevice::WriteOwner
                          | QFileDevice::ReadUser   | QFileDevice::WriteUser);
        CC.close();
        QSqlQuery ("update " NOM_TABLE_IMPRESSIONS " set pdf = null, compression = null, lienversfichier = '/" + datetransfer + "/" + NomFileDoc  + "' where idimpression = " + exportpdfquer.value(0).toString(),db);
        faits ++;
        int nsec = debut.secsTo(QTime::currentTime());
        int min = nsec/60;
        int hour = min/60;
        min = min - (hour*60);
        nsec = nsec - (hour*3600) - (min*60);
        listmsg.clear();
        duree = QTime(hour,min,nsec).toString("HH:mm:ss");
        listmsg << "PDF - " + NomFileDoc + " - " + QString::number(faits) + "/" + QString::number(total) + " - "  + duree;
        QTime dieTime= QTime::currentTime().addMSecs(2);
        while (QTime::currentTime() < dieTime)
            QCoreApplication::processEvents(QEventLoop::AllEvents, 1);
        dlg_message(listmsg, 10);
    }
    listmsg <<  tr("export terminé"), QString::number(total) + " documents exportés en "  + duree;
    dlg_message(listmsg, 3000);
}

void Rufus::Slot_ImportDocsExternes()
{
    if (proc->getModeConnexion() != Procedures::Distant)
    {
        QString B = proc->gsettingsIni->value("BDD_LOCAL/PrioritaireGestionDocs").toString();
        QString IpAdr = "";
        if (B=="YES")
            IpAdr = QHostInfo::localHostName() + " - prioritaire";
        else if (B=="NORM")
            IpAdr = QHostInfo::localHostName();
        else
            return;
        if (IpAdr ==  proc->PosteImportDocs())
            ImportDocsExtThread = new ImportDocsExternesThread(proc);
    }
    else if (proc->gsettingsIni->value("BDD_LOCAL/PrioritaireGestionDocs").toString() == "YES"
             || proc->gsettingsIni->value("BDD_LOCAL/PrioritaireGestionDocs").toString() == "NORM")
        ImportDocsExtThread = new ImportDocsExternesThread(proc);
}

void Rufus::Slot_ImprimeDossier()
{
    QString listactreq = "select idActe, ActeDate, ActeMotif, ActeTexte, ActeConclusion, usernom, userprenom, usertitre from " NOM_TABLE_ACTES
                         " as act left outer join " NOM_TABLE_UTILISATEURS " as usr on usr.iduser = act.iduser"
                         " where idPat = " + QString::number(gidPatient);
    //UpMessageBox::Watch(this,listactreq);
    QSqlQuery listactquer (listactreq,db);
    if (listactquer.size()==0)
    {
        UpMessageBox::Watch(this,tr("Pas d'actes enregistré pour ce patient!"));
        return;
    }

    QString Reponse =        "<html><head><meta name=\"qrichtext\" content=\"1\" />"
                             "<style type=\"text/css\">"
                             "p {margin-top:0px; margin-bottom:0px;margin-left: 0px; font-size:9pt}, li { white-space: pre-wrap; }"
                             "</style>"
                             "</head><body>";

    UpTextEdit textprov;

    // collecte des antécédents
    QString AtcdtsGenx = "", AtcdtsOphs = "", idCorMedMG = "";
    QString req = "select RMPAtcdtsPersos, RMPAtcdtsOphs, idcorMedMG from " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS " where idPat = " + QString::number(gidPatient);
    QSqlQuery atcdtsquer (req, db);
    if (atcdtsquer.size()>0)
    {
        atcdtsquer.first();
        AtcdtsGenx = atcdtsquer.value(0).toString();
        AtcdtsOphs = atcdtsquer.value(1).toString();
        idCorMedMG = atcdtsquer.value(2).toString();
    }
    if (AtcdtsOphs != "")
    {
        textprov.setText(AtcdtsOphs);
        Reponse += "<p><td width=\"20\"></td><td width=\"480\"><font color = \"" + proc->CouleurTitres + "\">" + tr("Antécédents ophtalmologiques: ") + "</font>" + textprov.toHtml() + "</td></p>";
    }
    if (AtcdtsGenx != "")
    {
        textprov.setText(AtcdtsGenx);
        Reponse += "<p><td width=\"20\"></td><td width=\"480\"><font color = \"" + proc->CouleurTitres + "\">" + tr("Antécédents généraux: ") + "</font>" + textprov.toHtml() + "</td></p>";
    }
    if (idCorMedMG != "")
    {
        req = "select CorNom, CorPrenom, CorVille from " NOM_TABLE_CORRESPONDANTS " where idCor = " + idCorMedMG;
        QSqlQuery MGQuery (req,db);
        if (MGQuery.size()>0)
        {
            MGQuery.first();
            if (MGQuery.value(0).toString() != "" || MGQuery.value(1).toString() != "")
            {
                idCorMedMG = "Dr " + MGQuery.value(1).toString() + " " + MGQuery.value(0).toString();
                if (MGQuery.value(2).toString() != "")
                    idCorMedMG += " - " + MGQuery.value(2).toString();
                Reponse += "<p><td width=\"20\"></td><td width=\"640\"><font color = \"" + proc->CouleurTitres + "\">" + tr("Médecin traitant: ") + "</font>" + idCorMedMG + "</td></p>";
            }
        }
    }
    if (AtcdtsOphs != "" || AtcdtsGenx != "" || idCorMedMG != "")
    {
        Reponse += "<br>";
    }

    bool reponsevide = true;
    listactquer.first();
    for (int i=0;i<listactquer.size();i++)
    {
        if (listactquer.value(2).toString() != ""
            || listactquer.value(3).toString() != ""
            || listactquer.value(4).toString() != "")
        {
            reponsevide = false;
            Reponse += "<p><td width=\"140\"><font color = \"" + proc->CouleurTitres + "\" ><u><b>" + listactquer.value(1).toDate().toString(tr("d MMMM yyyy")) +"</b></u></font></td>"
                    "<td width=\"400\">"
                    + listactquer.value(7).toString() + " " + listactquer.value(6).toString() + " " + listactquer.value(5).toString() + "</td></p>";
            if (listactquer.value(2).toString() != "")
            {
                textprov.setText(listactquer.value(2).toString());
                // on retire la dernière ligne si elle est vide
                QString texte = textprov.toHtml();
                bool a = true;
                while (a) {
                         // il faut retirer la dernière ligne du html qui contient le retour à la ligne
                    int debut = texte.lastIndexOf("<p");
                    int fin   = texte.lastIndexOf("</p>");
                    int longARetirer = fin - debut + 4;
                    if (texte.mid(debut,longARetirer).contains("-qt-paragraph-type:empty;"))
                        texte.remove(debut,longARetirer);
                    else a = false;}
                Reponse += "<p><td width=\"10\"></td><td width=\"450\"><font color = \"" + proc->CouleurTitres + "\"" + tr("Motif:") + "</font>" + texte + "</td></p>";
            }
            if (listactquer.value(3).toString() != "")
            {
                textprov.setText(listactquer.value(3).toString());
                // on retire la dernière ligne si elle est vide
                QString texte = textprov.toHtml();
                bool a = true;
                while (a) {
                         // il faut retirer la dernière ligne du html qui contient le retour à la ligne
                    int debut = texte.lastIndexOf("<p");
                    int fin   = texte.lastIndexOf("</p>");
                    int longARetirer = fin - debut + 4;
                    if (texte.mid(debut,longARetirer).contains("-qt-paragraph-type:empty;"))
                        texte.remove(debut,longARetirer);
                    else a = false;}
                Reponse += "<p><td width=\"10\"></td><td width=\"450\"><font color = \"" + proc->CouleurTitres + "\">" + tr("Examen:") + "</font>" + texte + "</td></p>";
            }
            if (listactquer.value(4).toString() != "")
            {
                textprov.setText(listactquer.value(4).toString());
                // on retire la dernière ligne si elle est vide
                QString texte = textprov.toHtml();
                bool a = true;
                while (a) {
                         // il faut retirer la dernière ligne du html qui contient le retour à la ligne
                    int debut = texte.lastIndexOf("<p");
                    int fin   = texte.lastIndexOf("</p>");
                    int longARetirer = fin - debut + 4;
                    if (texte.mid(debut,longARetirer).contains("-qt-paragraph-type:empty;"))
                        texte.remove(debut,longARetirer);
                    else a = false;}
                if (listactquer.value(2).toString() != "" || listactquer.value(3).toString() != "")
                    Reponse += "<p><td width=\"10\"></td><td width=\"450\"><font color = \"" + proc->CouleurTitres + "\">" + tr("Conclusion:") + "</font>" + texte + "</td></p>";
                else
                    Reponse += "<p><td width=\"10\"></td><td width=\"450\">" + texte + "</td></p>";
            }
        }
        listactquer.next();
    }
   if (reponsevide)
   {
       UpMessageBox::Watch(this,tr("rien à imprimer"));
       return;
   }

   //Impression du dossier
   QString  Corps, Entete, Pied;
   bool     AvecPrevisu = true;
   bool     AvecDupli   = false;
   bool     AvecNumPage = true;

   //création de l'entête
   if (!proc->setDataOtherUser(gidUserParent).value("Success").toBool())
   {
       UpMessageBox::Watch(this, tr("Impossible de retrouver les données de l'en-tête"), tr("Annulation de l'impression"));
       return;
   }
   Entete = proc->ImpressionEntete(QDate::currentDate()).value("Norm");
   if (Entete == "") return;
   Entete.replace("{{TITRE1}}"             , "");
   Entete.replace("{{TITRE}}"              , "<font color = \"" + proc->CouleurTitres + "\">" + tr("COMPTE RENDU DE DOSSIER") + "</font>");
   Entete.replace("{{PRENOM PATIENT}}"     , gPrenomPatient);
   Entete.replace("{{NOM PATIENT}}"        , gNomPatient.toUpper());
   Entete.replace("{{DDN}}"                , "(" + gDDNPatient.toString(tr("d MMM yyyy")) + ")");


   // création du pied
   Pied = proc->ImpressionPied();
   if (Pied == "") return;

   // creation du corps de l'impression
   Corps = "<html>"
           "<body LANG=\"fr-FR\" DIR=\"LTR\">"
           "<p><div align=\"justify\">"
           "<span style=\"font-size:9pt\">"
           "{{TEXTE ORDO}}"
           "</span>"
           "</div></p>"
           "</body></html>";

   Reponse.replace(QRegExp("font-size( *: *[\\d]{1,2} *)pt"),"font-size:9pt");
   Corps.replace("{{TEXTE ORDO}}",Reponse);

   QTextEdit *Etat_textEdit = new QTextEdit;
   Etat_textEdit->setHtml(Corps);

   bool aa =proc->Imprime_Etat(Etat_textEdit, Entete, Pied,
                      proc->TaillePieddePage(), proc->TailleEnTete(), proc->TailleTopMarge(),
                      AvecDupli, AvecPrevisu, AvecNumPage);
   if (aa)
   {
       QSqlQuery query = QSqlQuery(db);
       query.prepare("insert into " NOM_TABLE_IMPRESSIONS " (idUser, idpat, TypeDoc, SousTypeDoc, Titre, TextEntete,"
                                                          " TextCorps,"
                                                          " TextPied, Dateimpression, UserEmetteur, EmisRecu, FormatDoc, idLieu)"
                                                          " values(:iduser, :idpat, :typeDoc, :soustypedoc, :titre,"
                                                          " :textEntete,"
                                                          " :textCorps,"
                                                          " :textPied, :dateimpression, :useremetteur, :emisrecu, :formatdoc, :idlieu)");
       query.bindValue(":iduser", QString::number(gidUser));
       query.bindValue(":idpat", QString::number(gidPatient));
       query.bindValue(":typeDoc", COURRIER);
       query.bindValue(":soustypedoc", "Impression dossier");
       query.bindValue(":titre", "Impression dossier");
       query.bindValue(":textEntete",Entete);
       query.bindValue(":textCorps", Corps);
       query.bindValue(":textPied", Pied);
       query.bindValue(":dateimpression", QDate::currentDate().toString("yyyy-MM-dd") + " " + QTime::currentTime().toString("HH:mm:ss"));
       query.bindValue(":useremetteur", QString::number(gidUser));
       query.bindValue(":emisrecu", "0");
       query.bindValue(":formatdoc", COURRIER);
       query.bindValue(":idlieu", gDataUser["idLieu"].toString());
       if(!query.exec())
           UpMessageBox::Watch(this,tr("Impossible d'enregistrer ce document dans la base!"));
       ui->OuvreDocsExternespushButton->setEnabled(true);
   }
   delete Etat_textEdit;
}

void Rufus::Slot_DropPatient(QByteArray data)
{
    QDataStream stream(&data, QIODevice::ReadOnly);
    int id = 0;
    bool a = true;
    while (!stream.atEnd() && a)
    {
        int                 row, col;
        QMap<int,QVariant>  roleDataMap;
        stream >> row >> col >> roleDataMap;
        QString ab;
        ab = "col = " + QString::number(col) + " row = " + QString::number(row);
        for (int i=0;i<roleDataMap.size();i++)
        {
            ab += " roledatamap[" + QString::number(i) + "]= " + roleDataMap[i].toString();
            id = roleDataMap[0].toInt();
            if (id != 0) {a = false; break;}
        }
        //qDebug() << ab;
    }
    //qDebug() << QString::number(id);
    Slot_InscritEnSalDat(id);
}

bool Rufus::Slot_InscritEnSalDat(int idpat)
{
    QString choixreq = " select idPat from " NOM_TABLE_SALLEDATTENTE " where idpat = " + QString::number(idpat);
    QSqlQuery choixquery(choixreq,db);
    if (choixquery.size() > 0)
    {
        UpMessageBox::Watch(this, tr("Patient déjà inscrit en salle d'attente"));
        return false;
    }
    else
    {
        //créer une fiche avec la liste des checkbox
        QStringList llist = MotifMessage();
        if (llist.isEmpty())
            return false;
        QString Arriverequete =   "INSERT INTO " NOM_TABLE_SALLEDATTENTE
                " (idPat, Statut, HeureArrivee, Motif, Message, HeureRDV, idUser)"
                " VALUES (" + QString::number(idpat) + ",'" ARRIVE "','" + QTime::currentTime().toString("hh:mm") + "','" +
                            llist.at(0) + "','" + proc->CorrigeApostrophe(llist.at(1)) + "','" + llist.at(2) + "', " + llist.at(3) + ")";
        proc->TraiteErreurRequete(QSqlQuery (Arriverequete, db),Arriverequete,"");
        proc->UpdVerrouSalDat();
        RecaleTableView(idpat);
    }
    return true;
}


void Rufus::Slot_ListeCorrespondants()
{
    QString req = "SELECT idCor, CorNom, CorPrenom, nomspecialite as metier, CorAdresse1, CorAdresse2, CorAdresse3, CorCodepostal, CorVille, CorTelephone FROM " NOM_TABLE_CORRESPONDANTS ", " NOM_TABLE_SPECIALITES
            " where cormedecin = 1 and corspecialite = idspecialite"
            " union"
            " SELECT idCor, CorNom, CorPrenom, corautreprofession as metier, CorAdresse1, CorAdresse2, CorAdresse3, CorCodepostal, CorVille, CorTelephone FROM " NOM_TABLE_CORRESPONDANTS
            " where cormedecin <> 1 or cormedecin is null"
            " order by metier, cornom, corprenom";
    QSqlQuery quer(req,db);
    //qDebug() << req;
    if (proc->TraiteErreurRequete(quer,req,"")) return;
    if (quer.size() == 0) return;

    Dlg_ListCor = new dlg_listecorrespondants(proc, quer, this);

    Dlg_ListCor->exec();
    if (Dlg_ListCor->getListeModifiee())
        FlagMetAjourMG();
    delete Dlg_ListCor;
}

void Rufus::Slot_MajusculeCreerNom()
{
    QString nom;
    QLineEdit *UiNom;
    UiNom = ui->CreerNomlineEdit;
    nom = proc->MajusculePremiereLettre(UiNom->text(),false);
    UiNom->setText(nom);
    if (gMode == NouveauDossier) ChercheNomFiltre();
    if (!ui->FiltrecheckBox->isChecked())
        ChercheNom();
    else
        ChercheNomFiltre();
}

void Rufus::Slot_MajusculeCreerPrenom()
{
    QString nom;
    QLineEdit *UiNom;
    UiNom = ui->CreerPrenomlineEdit;
    nom = proc->MajusculePremiereLettre(UiNom->text(),false);
    UiNom->setText(nom);
    if (gMode == NouveauDossier) ChercheNomFiltre();
    if (!ui->FiltrecheckBox->isChecked())
        ChercheNom();
    else
        ChercheNomFiltre();
}

void Rufus::Slot_MenuContextuelIdentPatient()
{
    gmenuContextuel = new QMenu(this);

    QAction *pAction_IdentPatient = gmenuContextuel->addAction("Modifier les données patients") ;
    connect (pAction_IdentPatient, &QAction::triggered, [=] {ChoixMenuContextuelIdentPatient();});

    // ouvrir le menu
    gmenuContextuel->exec(QCursor::pos());
    delete gmenuContextuel;
}

void Rufus::ChoixMenuContextuelIdentPatient()
{
    IdentificationPatient("Modification",gidPatient);  // aussi appelé depuis le bouton ui->ModifIdentificationSmallButton
}

void Rufus::Slot_MenuContextuelMotsCles()
{
    gmenuContextuel = new QMenu(this);

    QAction *pAction_ModifMotCle = gmenuContextuel->addAction(tr("Modifier les mots clés"));
    connect (pAction_ModifMotCle, &QAction::triggered, [=] {ChoixMenuContextuelMotsCles();});

    // ouvrir le menu
    gmenuContextuel->exec(QCursor::pos());
    delete gmenuContextuel;
}

void Rufus::ChoixMenuContextuelMotsCles()
{
    dlg_listemotscles *ListMCDialog = new dlg_listemotscles(proc, gidPatient, this);
    if (ListMCDialog->exec()==0)
    {
        QStringList listMC = ListMCDialog->listMCDepart();
        if (listMC.size()>0)
        {
            QSqlQuery ("delete from " NOM_TABLE_MOTSCLESJOINTURES " where idpat = " + QString::number(gidPatient), db);
            QString req = "insert into " NOM_TABLE_MOTSCLESJOINTURES " (idpat, idmotcle) values ";
            req += "(" + QString::number(gidPatient) + ", " + listMC.at(0) + ")";
            for (int j=1; j<listMC.size(); j++)
                req += ", (" + QString::number(gidPatient) + ", " + listMC.at(j) + ")";
            QSqlQuery (req,db);
            QSqlQuery ("delete from " NOM_TABLE_MOTSCLESJOINTURES " where idMotCle not in (select idmotcle from " NOM_TABLE_MOTSCLES ")", db);
        }
    }
    CalcMotsCles(gidPatient);
    ListMCDialog->close(); // nécessaire pour enregistrer la géométrie
    delete ListMCDialog;
}

void Rufus::Slot_RechercheParID()
{
    gAskRechParIDDialog     = new UpDialog(this);
    gAskRechParIDDialog     ->setAttribute(Qt::WA_DeleteOnClose);
    QVBoxLayout     *globallay  = dynamic_cast<QVBoxLayout*>(gAskRechParIDDialog->layout());
    UpLabel         *idlabel    = new UpLabel(gAskRechParIDDialog, tr("id du patient"));
    UpLineEdit      *idLine     = new UpLineEdit(gAskRechParIDDialog);
    lblnom                      = new UpLabel(gAskRechParIDDialog);
    idLine              ->setMaxLength(8);
    idLine              ->setValidator((new QIntValidator(1,99999999)));
    globallay           ->insertWidget(0,idlabel);
    globallay           ->insertWidget(1,idLine);
    globallay           ->insertWidget(2,lblnom);
    gAskRechParIDDialog ->AjouteLayButtons();
    connect(idLine,                         SIGNAL(textEdited(QString)),    this,                   SLOT(Slot_ChercheNomparID(QString)));
    connect(gAskRechParIDDialog->OKButton,  SIGNAL(clicked(bool)),          gAskRechParIDDialog,    SLOT(close()));
    gAskRechParIDDialog->exec();
}

void Rufus::Slot_RechercheParMotCle()
{
    QString req = "select idmotcle, motcle from " NOM_TABLE_MOTSCLES " order by motcle";
    QSqlQuery quer(req,db);
    if (quer.size()==0)
    {
        UpMessageBox::Watch(this, tr("Aucun mot clé défini dans la base"), tr("Recherche impossible"));
        return;
    }

    gAskRechParMotCleDialog     = new UpDialog(this);
    QVBoxLayout     *globallay  = dynamic_cast<QVBoxLayout*>(gAskRechParMotCleDialog->layout());
    QTableView      *tabMC      = new QTableView(gAskRechParMotCleDialog);
    globallay               ->insertWidget(0,tabMC);
    gAskRechParMotCleDialog ->AjouteLayButtons();
    connect(gAskRechParMotCleDialog->OKButton,  SIGNAL(clicked(bool)),  this,   SLOT(Slot_AfficheDossiersRechercheParMotCle()));

    gAskRechParMotCleDialog ->setModal(true);
    gAskRechParMotCleDialog ->setWindowTitle(tr("Recherche de patients par mots clés"));

    tabMC                   ->verticalHeader()->setVisible(false);
    tabMC                   ->horizontalHeader()->setVisible(false);
    tabMC                   ->setFocusPolicy(Qt::StrongFocus);
    tabMC                   ->setSelectionMode(QAbstractItemView::SingleSelection);
    tabMC                   ->setGridStyle(Qt::NoPen);
    tabMC                   ->setEditTriggers(QAbstractItemView::NoEditTriggers);
    QFontMetrics fm(qApp->font());
    int hauteurligne        = fm.height()*1.1;
    tabMC                   ->setMinimumHeight(hauteurligne*20);
    tabMC                   ->setSizeIncrement(0,hauteurligne);
    tabMC                   ->setMouseTracking(true);

    QStandardItem      *pitem;
    QStandardItemModel *modele =  new QStandardItemModel;
    quer.first();
    for (int i=0; i<quer.size(); i++)
    {
        pitem   = new QStandardItem(quer.value(1).toString());
        pitem   ->setAccessibleDescription(quer.value(0).toString());
        pitem   ->setCheckable(true);
        modele  ->appendRow(pitem);
        quer    .next();
    }
    tabMC       ->setModel(modele);
    for (int i=0; i<modele->rowCount(); i++)
        tabMC   ->setRowHeight(i,hauteurligne);
    tabMC       ->setColumnWidth(0,300);
    tabMC       ->setFixedWidth(tabMC->columnWidth(0)+2);
    gAskRechParMotCleDialog->setFixedWidth(tabMC->width()
                        + globallay->contentsMargins().left()*2);
    gAskRechParMotCleDialog->exec();
    delete gAskRechParMotCleDialog;
}

void Rufus::Slot_AfficheDossiersRechercheParMotCle()
{
    QStringList listidMc;
    QStandardItemModel *model = dynamic_cast<QStandardItemModel*>(gAskRechParMotCleDialog->findChildren<QTableView *>().at(0)->model());
    if (model==NULL)
        return;
    for (int i=0; i< model->rowCount(); i++)
        if(model->item(i,0)->checkState() == Qt::Checked)
            listidMc << model->item(i,0)->accessibleDescription();
    if (listidMc.size()==0)
    {
        UpMessageBox::Watch(this, tr("Vous n'avez sélectionné aucun mot clé"));
        return;
    }
    QString req = "select mcjoin.idPat, PatNom, PatPrenom, PatDDN, motcle"
                  " from " NOM_TABLE_MOTSCLESJOINTURES " as mcjoin"
                  " left outer join " NOM_TABLE_PATIENTS " pat"
                  " on mcjoin.idPat = pat.IDPAT"
                  " left outer join " NOM_TABLE_MOTSCLES " msc"
                  " on mcjoin.idmotcle = msc.idmotcle"
                  " where mcjoin.idmotcle in (";
    req += listidMc.at(0);
    for (int i=1; i<listidMc.size(); i++)
        req += ", " +listidMc.at(i);
    req += ") order by patnom, patprenom";
    QSqlQuery quer(req,db);
    if (quer.size()==0)
    {
        UpMessageBox::Watch(this, tr("Aucun patient retrouvé pour ces critères"));
        return;
    }
    gAskListPatients = new UpDialog(this);
    QVBoxLayout     *globallay          = dynamic_cast<QVBoxLayout*>(gAskListPatients->layout());
    QTableView      *tabMC              = new QTableView(gAskListPatients);
    globallay       ->insertWidget(0,tabMC);
    gAskListPatients->AjouteLayButtons(UpDialog::ButtonPrint | UpDialog::ButtonOK);
    gAskListPatients->PrintButton   ->setLuggage(listidMc);
    connect(gAskListPatients->OKButton,     SIGNAL(clicked(bool)),      gAskListPatients,   SLOT(accept()));
    connect(gAskListPatients->PrintButton,  SIGNAL(clicked(QVariant)),  this,               SLOT(Slot_ImprimeListPatients(QVariant)));

    gAskListPatients->setModal(true);
    gAskListPatients->setSizeGripEnabled(false);
    gAskListPatients->setWindowTitle(tr("Recherche de patients par mots clés"));

    tabMC->verticalHeader()->setVisible(false);
    tabMC->horizontalHeader()->setVisible(false);
    tabMC->setFocusPolicy(Qt::StrongFocus);
    tabMC->setSelectionMode(QAbstractItemView::SingleSelection);
    tabMC->setGridStyle(Qt::NoPen);
    tabMC->setEditTriggers(QAbstractItemView::NoEditTriggers);
    QFontMetrics fm(qApp->font());
    int hauteurligne = fm.height()*1.1;
    tabMC->setMinimumHeight(hauteurligne*20);
    tabMC->setSizeIncrement(0,hauteurligne);
    tabMC->setMouseTracking(true);

    QStandardItem       *pitem, *pitem1, *pitem2;
    QStandardItemModel *modele =  new QStandardItemModel;
    quer.first();
    for (int i=0; i<quer.size(); i++)
    {
        pitem   = new QStandardItem(quer.value(1).toString() + " " + quer.value(2).toString());
        pitem1  = new QStandardItem(quer.value(3).toDate().toString(tr("dd-MMM-yyyy")));
        pitem2  = new QStandardItem(quer.value(4).toString());
        pitem   ->setAccessibleDescription(quer.value(0).toString());
        pitem1  ->setAccessibleDescription(quer.value(0).toString());
        pitem2  ->setAccessibleDescription(quer.value(0).toString());
        QList <QStandardItem*> listitems;
        listitems << pitem << pitem1 << pitem2;
        modele  ->appendRow(listitems);
        quer    .next();
    }
    tabMC->setModel(modele);
    for (int i=0; i<modele->rowCount(); i++)
        tabMC->setRowHeight(i,hauteurligne);
    tabMC->setColumnWidth(0,200);
    tabMC->setColumnWidth(1,100);
    tabMC->setColumnWidth(2,250);
    tabMC->setFixedWidth(tabMC->columnWidth(0)+tabMC->columnWidth(1)+tabMC->columnWidth(2)+2);
    gAskListPatients->setFixedWidth(tabMC->width()
                        + globallay->contentsMargins().left()*2);
    gAskListPatients->exec();
    delete gAskListPatients;
}

void Rufus::Slot_ImprimeListPatients(QVariant var)
{
    UpSmallButton *button = dynamic_cast<UpSmallButton*>(sender());
    if (button !=NULL)
        if (button->parent() == gAskListPatients)
        {
            QStringList listidMc = var.toStringList();
            bool AvecDupli   = false;
            bool AvecPrevisu = proc->ApercuAvantImpression();
            bool AvecNumPage = true;

            //--------------------------------------------------------------------
            // Préparation de l'état "liste patients" dans un QplainTextEdit
            //--------------------------------------------------------------------
            QString req1 = "select motcle from " NOM_TABLE_MOTSCLES " where idmotcle in(";
            req1 += listidMc.at(0);
            for (int i=1; i<listidMc.size(); i++)
                req1 += ", " +listidMc.at(i);
            req1 += ")";
            QSqlQuery titrquer(req1,db);
            titrquer.first();
            QString titre = tr("recherche de patients sur ");
            titre += titrquer.value(0).toString();
            for (int i=1; i<titrquer.size(); i++)
            {
                titrquer.seek(i);
                titre += ", " + titrquer.value(0).toString();
            }
            QString req = "select mcjoin.idPat, PatNom, PatPrenom, PatDDN, motcle"
                          " from " NOM_TABLE_MOTSCLESJOINTURES " as mcjoin"
                          " left outer join " NOM_TABLE_PATIENTS " pat"
                          " on mcjoin.idPat = pat.IDPAT"
                          " left outer join " NOM_TABLE_MOTSCLES " msc"
                          " on mcjoin.idmotcle = msc.idmotcle"
                          " where mcjoin.idmotcle in (";
            req += listidMc.at(0);
            for (int i=1; i<listidMc.size(); i++)
                req += ", " +listidMc.at(i);
            req += ") order by patnom, patprenom";
            QSqlQuery quer(req,db);
            int     gtotalNbreDossiers    = quer.size();
            QDate   date = QDate::currentDate();;

            //création de l'entête
            QString EnTete;
            if (!proc->setDataOtherUser(gidUserParent).value("Success").toBool())
                return;
            EnTete = proc->ImpressionEntete(date).value("Norm");
            if (EnTete == "") return;
            EnTete.replace("{{TITRE1}}"            , "");
            EnTete.replace("{{PRENOM PATIENT}}"    , "");
            EnTete.replace("{{NOM PATIENT}}"       , "");
            EnTete.replace("{{TITRE}}"             , titre);
            EnTete.replace("{{DDN}}"               , "<font color = \"" + proc->CouleurTitres + "\">" + QString::number(gtotalNbreDossiers)
                                                    + " " + (gtotalNbreDossiers>1? tr("dossiers") : tr("dosssier")) + "</font>");
            // création du pied
            QString Pied = proc->ImpressionPied();
            if (Pied == "") return;

            // creation du corps
            QString ligne = "<table width=\"490\" border=\"0\" cellspacing=\"0\" cellpadding=\"5\">"
                    "<tr>"
                      "<td width=\"150\" ><span style=\"font-size:9pt;\">{{NOM PATIENT}}</span></td>"
                      "<td width=\"100\" ><span style=\"font-size:9pt;\">{{DDN}}</span></td>"
                      "<td width=\"210\" ><span style=\"font-size:9pt;\">{{MOTIF}}</span></td>"
                  "</tr>"
                  "</table>";

            QString texte, lignepat;
            quer.first();
            for (int k = 0; k < quer.size(); k++)
            {
                // Remplacement des variables par les valeurs lues.
                lignepat = ligne;
                lignepat.replace("{{NOM PATIENT}}", quer.value(1).toString() + " " + quer.value(2).toString());
                lignepat.replace("{{DDN}}", quer.value(3).toDate().toString(tr("dd-MMM-yyyy")));
                lignepat.replace("{{MOTIF}}", quer.value(4).toString());
                texte += lignepat;
                quer.next();
            }
            QTextEdit *textEdit = new QTextEdit;
            textEdit->setHtml(texte);

            proc->Imprime_Etat(textEdit, EnTete, Pied,
                               proc->TaillePieddePage(), proc->TailleEnTete(), proc->TailleTopMarge(),
                               AvecDupli, AvecPrevisu, AvecNumPage);
            delete textEdit;
        }
}

void Rufus::Slot_MenuContextuelBureaux()
{
    UpTextEdit *UpText = dynamic_cast<UpTextEdit *> (sender());
    if (UpText == 0)
        return;
    else
    {
        if (UpText->getId() == 0)
            return;
        else
        {
            gdossierAOuvrir = UpText->getId();
            gmenuContextuel = new QMenu(this);

            if (gUserDroits != SECRETAIRE)
            {
                QAction *pAction_ReprendreDossier = gmenuContextuel->addAction(tr("Visualiser le dossier"));
                connect (pAction_ReprendreDossier, &QAction::triggered,    [=] {Slot_ChoixMenuContextuelListePatients("Autre Dossier");});
            }

            // ouvrir le menu
            gmenuContextuel->exec(QCursor::pos());
            delete gmenuContextuel;
        }
    }
}
void Rufus::Slot_MenuContextuelListePatients(QPoint point)
{
    QModelIndex pindx = ui->PatientsListeTableView->indexAt(point);
    if (gListePatientsModel->itemFromIndex(pindx) == 0)
        return;
    int row = gListePatientsModel->itemFromIndex(pindx)->row();
    gdossierAOuvrir = gListePatientsModel->item(row)->text().toInt();

    gmenuContextuel = new QMenu(this);

    QAction *pAction_MettreEnSalDat = gmenuContextuel->addAction(tr("Inscrire ce patient en salle d'attente"));
    connect (pAction_MettreEnSalDat, &QAction::triggered,    [=] {Slot_ChoixMenuContextuelListePatients("SalDat");});

    QAction *pAction_ModifierDossier = gmenuContextuel->addAction(tr("Modifier les données de ce patient"));
    connect (pAction_ModifierDossier, &QAction::triggered,    [=] {Slot_ChoixMenuContextuelListePatients("Modifier");});

    QAction *pAction_Copier = gmenuContextuel->addAction(tr("Créer un dossier de la même famille"));
    connect (pAction_Copier, &QAction::triggered,    [=] {Slot_ChoixMenuContextuelListePatients("Copie");});

    if (gUserDroits != SECRETAIRE)
    {
        QAction *pAction_ReprendreDossier = gmenuContextuel->addAction(tr("Visualiser le dossier"));
        connect (pAction_ReprendreDossier, &QAction::triggered,    [=] {Slot_ChoixMenuContextuelListePatients("Autre Dossier");});
    }
    QAction *pAction_EmettreDoc = gmenuContextuel->addAction(tr("Emettre un document"));
    connect (pAction_EmettreDoc, &QAction::triggered,    [=] {Slot_ChoixMenuContextuelListePatients("Document");});

    QString req = "Select idImpression from " NOM_TABLE_IMPRESSIONS " where idpat = " + QString::number(gdossierAOuvrir);
    QSqlQuery quer(req,db);
    if (quer.size() > 0){
        QAction *pAction_ImprimeDoc = gmenuContextuel->addAction(tr("Réimprimer un document"));
        connect (pAction_ImprimeDoc, &QAction::triggered,    [=] {Slot_ChoixMenuContextuelListePatients("ImprimeAncienDoc");});
    }
    QAction *pAction_EnregDoc = gmenuContextuel->addAction(tr("Enregistrer un document scanné"));
    connect (pAction_EnregDoc, &QAction::triggered,    [=] {Slot_ChoixMenuContextuelListePatients("EnregDocScan");});

    QAction *pAction_EnregVideo = gmenuContextuel->addAction(tr("Enregistrer une video"));
    connect (pAction_EnregVideo, &QAction::triggered,    [=] {Slot_ChoixMenuContextuelListePatients("EnregVideo");});

    QAction *pAction_SendMess = gmenuContextuel->addAction(tr("Envoyer un message"));
    connect (pAction_SendMess, &QAction::triggered,    [=] {Slot_ChoixMenuContextuelListePatients("SendMess");});

    // ouvrir le menu
    gmenuContextuel->exec(QCursor::pos());
    delete gmenuContextuel;
}

void Rufus::Slot_ChoixMenuContextuelListePatients(QString choix)
{
    if (choix == "Autre Dossier")
    {
        QString autrerequete = "select PatNom, PatPrenom from " NOM_TABLE_PATIENTS " where idPat = " + QString::number(gdossierAOuvrir);
        QSqlQuery autrequery (autrerequete, db);
        if (proc->TraiteErreurRequete(autrequery,autrerequete,""))
            return;
        if (autrequery.size() == 0)
        {
            UpMessageBox::Watch(this, tr("Pas de consultation enregistrée pour ce patient"));
            return;
        }
        else
        {
            autrequery.first();
            QString patNom = autrequery.value(0).toString();
            QString patPrenom = autrequery.value(1).toString();
            QString autreactes = "select idActe, ActeDate from " NOM_TABLE_ACTES " where idPat = " + QString::number(gdossierAOuvrir) + " ORDER BY ActeDate DESC";
            QSqlQuery autreactequery (autreactes, db);
            if (proc->TraiteErreurRequete(autreactequery,autreactes,""))
               return;
            if (autreactequery.size() == 0)
            {
                UpMessageBox::Watch(this, tr("Pas de consultation enregistrée pour ") + patPrenom + " " + patNom);
                return;
            }
            else
            {
                autreactequery  .first();
                int idActe      = autreactequery.value(0).toInt();
                Dlg_ActesPrecs  = new dlg_actesprecedents(gdossierAOuvrir, idActe, proc, false, this);
                Dlg_ActesPrecs  ->setWindowTitle(tr("Consultations précédentes de ") + patNom + " " + patPrenom);
                Dlg_ActesPrecs  ->setWindowIcon(proc->giconLoupe);
                Dlg_ActesPrecs  ->exec();
            }
        }
    }
    else if (choix == "SalDat")
        Slot_InscritEnSalDat(gdossierAOuvrir);
    else if (choix == "Copie")
        RecopierDossier(gdossierAOuvrir);
    else if (choix == "Modifier")
        IdentificationPatient("Modification",gdossierAOuvrir);  //depuis menu contextuel de la table liste
    else if (choix == "Document")
        OuvrirDocuments(false);
    else if (choix == "ImprimeAncienDoc")
        OuvrirDocsExternes(gdossierAOuvrir, true);              //depuis menu contextuel ListePatients
    else if (choix == "EnregDocScan")
        EnregistreDocScanner();                            //depuis menu contextuel ListePatients
    else if (choix == "EnregVideo")
        EnregistreVideo();                                 //depuis menu contextuel ListePatients
    else if (choix == "SendMess")
    {
        QMap<QString, QVariant> map;
        map["null"] = true;
        SendMessage(map, gdossierAOuvrir);                           //depuis menu contextuel ListePatients
    }
}

void Rufus::Slot_MenuContextuelMedecin()
{
    if (ui->MGupComboBox->findText(ui->MGupComboBox->currentText()))
    {
        gmenuContextuel = new QMenu(this);
        QAction *pAction_IdentPatient = gmenuContextuel->addAction(tr("Modifier les coordonnées de ce médecin"));
        connect (pAction_IdentPatient, &QAction::triggered,    [=] {Slot_ChoixMenuContextuelMedecin();});

        // ouvrir le menu
        gmenuContextuel->exec(QCursor::pos());
        delete gmenuContextuel;
    }
}

void Rufus::Slot_ChoixMenuContextuelMedecin()
{
    int id = ui->MGupComboBox->currentData().toInt();
    int idxMG = ui->MGupComboBox->currentIndex();
    Dlg_IdentCorresp          = new dlg_identificationcorresp("Modification",true,id, proc, this);
    if (Dlg_IdentCorresp->exec()>0)
    {
        FlagMetAjourMG();
        if (Dlg_IdentCorresp->IdentModified())
            ui->MGupComboBox->setCurrentIndex(idxMG);
        Slot_OKModifierTerrain();
    }
    delete Dlg_IdentCorresp;
}

void Rufus::Slot_MenuContextuelCorrespondant()
{
    QString choix = "";
    if (sender() == ui->AutresCorresp1upComboBox) choix = "Modifier1";
    else if (sender() == ui->AutresCorresp2upComboBox) choix = "Modifier2";
    else return;

    UpComboBox *box = dynamic_cast<UpComboBox*>(sender());
    if (box->findText(box->currentText()))
    {
        gmenuContextuel = new QMenu(this);
        QAction *pAction_IdentPatient = gmenuContextuel->addAction(tr("Modifier les coordonnées de ce correspondant"));
        connect (pAction_IdentPatient, &QAction::triggered,    [=] {Slot_ChoixMenuContextuelCorrespondant(choix);});

        // ouvrir le menu
        gmenuContextuel->exec(QCursor::pos());
        delete gmenuContextuel;
    }
}

void Rufus::Slot_ChoixMenuContextuelCorrespondant(QString choix)
{
    int id = -1;
    if (choix == "Modifier1")
        id = ui->AutresCorresp1upComboBox->currentData().toInt();
    else if (choix == "Modifier2")
        id = ui->AutresCorresp2upComboBox->currentData().toInt();
    if (id==-1) return;
    Dlg_IdentCorresp          = new dlg_identificationcorresp("Modification", false, id, proc, this);
    if (Dlg_IdentCorresp->exec()>0)
    {
        int idCor = Dlg_IdentCorresp->gidCor;
        FlagMetAjourMG();
        int a = -1;
        if (choix == "Modifier1")
            id = ui->AutresCorresp1upComboBox->findData(idCor);
        else if (choix == "Modifier2")
            id = ui->AutresCorresp2upComboBox->findData(idCor);
        if (choix == "Modifier1")
            ui->AutresCorresp1upComboBox->setCurrentIndex(a);
        else if (choix == "Modifier2")
            ui->AutresCorresp2upComboBox->setCurrentIndex(a);
        Slot_OKModifierTerrain();
    }
    delete Dlg_IdentCorresp;
}

void Rufus::Slot_MenuContextuelSalDat()
{
    UpLabel *labelClicked = dynamic_cast<UpLabel *> (sender());
    if (labelClicked == 0) return;

    gdossierAOuvrir = labelClicked->getId();
    int row = labelClicked->getRow();

    gmenuContextuel = new QMenu(this);

    if (sender()->parent()->parent() == ui->SalleDAttenteupTableWidget)
    {
        UpLabel *StatutClicked = dynamic_cast<UpLabel *> (ui->SalleDAttenteupTableWidget->cellWidget(row,1));
        if (StatutClicked != NULL)
        {
            if (StatutClicked->text() == ARRIVE)
            {
                QAction *pAction_RetirerDossier = gmenuContextuel->addAction(tr("Retirer ce dossier de la salle d'attente"));
                connect (pAction_RetirerDossier, &QAction::triggered,    [=] {Slot_ChoixMenuContextuelSalDat("Retirer");});
            }
        }
    }
    QAction *pAction_ModifierDossier = gmenuContextuel->addAction(tr("Modifier les données de ce patient"));
    connect (pAction_ModifierDossier, &QAction::triggered,    [=] {Slot_ChoixMenuContextuelSalDat("Modifier");});

    QAction *pAction_ModifierMotif = gmenuContextuel->addAction(tr("Modifier le motif de l'acte"));
    connect (pAction_ModifierMotif, &QAction::triggered,    [=] {Slot_ChoixMenuContextuelSalDat("Motif");});

    QAction *pAction_Copier = gmenuContextuel->addAction(tr("Créer un dossier de la même famille"));
    connect (pAction_Copier, &QAction::triggered,    [=] {Slot_ChoixMenuContextuelSalDat("Copie");});
    if (gUserDroits != SECRETAIRE)
    {
        QAction *pAction_OuvrirDossier = gmenuContextuel->addAction(tr("Ouvrir le dossier"));
        connect (pAction_OuvrirDossier, &QAction::triggered,    [=] {Slot_ChoixMenuContextuelSalDat("Ouvrir");});
    }
    QAction *pAction_EmettreDoc = gmenuContextuel->addAction(tr("Emettre un document"));
    connect (pAction_EmettreDoc, &QAction::triggered,    [=] {Slot_ChoixMenuContextuelSalDat("Document");});

    // ouvrir le menu
    gmenuContextuel->exec(QCursor::pos());
    delete gmenuContextuel;
}

void Rufus::Slot_MenuContextuelSalDatPaiemt()
{
    QList<QTableWidgetSelectionRange> listRange = ui->AccueilupTableWidget->selectedRanges();
    UpLabel *labelClicked = dynamic_cast<UpLabel *> (sender());
    if (labelClicked == 0) return;
    bool a = false;
    // si le label qui émet la demande de menu n'est pas dans la plage sélectionnée, on n'affiche pas de menu
    for (int i = 0; i< listRange.size();i++)
        if (listRange.at(i).topRow() <= labelClicked->getRow() && listRange.at(i).bottomRow() >= labelClicked->getRow())
        {a = true;  break;}
    if (a == false) return;

    gdossierAOuvrir = labelClicked->getId();
    gmenuContextuel = new QMenu(this);

    if (listRange.size() == 1 && listRange.at(0).rowCount()== 1)
    {
        if (gUserDroits != SECRETAIRE)
        {
            QAction *pAction_ReprendreDossier = gmenuContextuel->addAction(tr("Reprendre le dossier"));
            connect (pAction_ReprendreDossier, &QAction::triggered,    [=] {Slot_ChoixMenuContextuelSalDat("Reprendre");});
        }
        QAction *pAction_EmettreDoc = gmenuContextuel->addAction(tr("Emettre un document"));
        connect (pAction_EmettreDoc, &QAction::triggered,    [=] {Slot_ChoixMenuContextuelSalDat("Document");});
    }
    QAction *pAction_EnregistrePaiement = gmenuContextuel->addAction(tr("Enregistrer le paiement"));
    connect (pAction_EnregistrePaiement, &QAction::triggered,    [=] {Slot_ChoixMenuContextuelSalDat("Payer");});

    // ouvrir le menu
    gmenuContextuel->exec(QCursor::pos());
    delete gmenuContextuel;
}

void Rufus::Slot_ChoixMenuContextuelSalDat(QString choix)
{
    if (choix == "Reprendre")
        ChoixDossier(gdossierAOuvrir);
    else if (choix == "Payer")
        Slot_AppelPaiementDirect("AttentePaiement");
    else if (choix == "Modifier")
        IdentificationPatient("Modification",gdossierAOuvrir);  //appelé depuis le menu contextuel de la table salle d'attente

    else if (choix == "Ouvrir")
        ChoixDossier(gdossierAOuvrir);
    else if (choix == "Retirer" || choix == "Fermer")
    {
        QSqlQuery ("delete from " NOM_TABLE_SALLEDATTENTE " where idPat = " + QString::number(gdossierAOuvrir),db);
        proc->UpdVerrouSalDat();
    }
    else if (choix == "Copie")
        RecopierDossier(gdossierAOuvrir);
    else if (choix == "Document")
        OuvrirDocuments(false);
    else if (choix == "Motif")
    {
        QString Message(""), Motif("");
        QString req = "select Motif, Message, HeureRDV from " NOM_TABLE_SALLEDATTENTE " where idPat = " + QString::number(gdossierAOuvrir);
        QSqlQuery quer(req,db);
        if (quer.size()>0)
        {
            quer.first();
            Motif = quer.value(0).toString();
            Message = quer.value(1).toString();
            QTime heurerdv = quer.value(2).toTime();
            QStringList llist = MotifMessage(Motif, Message, heurerdv);
            if (llist.isEmpty())
                return;
            QString saldatrequete =   "SELECT idPat FROM " NOM_TABLE_SALLEDATTENTE " WHERE idPat = " + QString::number(gdossierAOuvrir);
            QSqlQuery SalDatQuery(saldatrequete,db);
            proc->TraiteErreurRequete(SalDatQuery,saldatrequete,"Impossible de trouver le dossier dans la salle d'attente!");
            if (SalDatQuery.size() == 0)
            {
                saldatrequete =   "INSERT INTO " NOM_TABLE_SALLEDATTENTE
                        " (idPat, Statut, HeureArrivee, Motif, Message, HeureRDV, idUser)"
                        " VALUES (" + QString::number(gdossierAOuvrir) + ",'" ARRIVE "','" + QTime::currentTime().toString("hh:mm") + "','" +
                                    llist.at(0) + "','" + proc->CorrigeApostrophe(llist.at(1)) + "','" + llist.at(2) + "', " + llist.at(3) + ")";
            }
            else
            {
                saldatrequete =   "UPDATE " NOM_TABLE_SALLEDATTENTE
                        " SET Motif = '" + llist.at(0) + "',"
                        " Message = '" + proc->CorrigeApostrophe(llist.at(1)) + "'," +
                        " HeureRDV = '" + llist.at(2) + "'," +
                        " idUser = " + llist.at(3) +
                        " WHERE idPat = " + QString::number(gdossierAOuvrir);
            }
            //proc->Edit(saldatrequete);
            QSqlQuery ModifSalDatQuery(saldatrequete,db);
            proc->TraiteErreurRequete(ModifSalDatQuery,saldatrequete,"");
            proc->UpdVerrouSalDat();
        }
    }
}


QStringList Rufus::MotifMessage(QString Motif, QString Message, QTime heurerdv)
{
    //créer une fiche avec 8 checkbox : Cs, OCT, CV, BO, Biométrie, Urgence, Angio Autre
    gAsk            = new UpDialog(this);
    QVBoxLayout     *globallayout   = dynamic_cast<QVBoxLayout*>(gAsk->layout());
    QVBoxLayout     *motiflayout    = new QVBoxLayout();
    UpLabel         *lbltitre       = new UpLabel(gAsk);
    UpLabel         *lblsoignt      = new UpLabel(gAsk);
    QTextEdit       *MsgText        = new QTextEdit(gAsk);
    QGroupBox       *grpBox         = new QGroupBox(gAsk);
    QTimeEdit       *HeureRDV       = new QTimeEdit(gAsk);
    UpComboBox *ComboSuperviseurs   = new UpComboBox(gAsk);
    UpLabel         *HeureTitre     = new UpLabel(gAsk);
    QStringList     llist;
    grpBox      ->setTitle(tr("Motif de l'acte"));
    for (int i=0; i<proc->getListeSuperviseurs()->rowCount(); i++)
        ComboSuperviseurs          ->addItem(proc->getListeSuperviseurs()->item(i,1)->text(), proc->getListeSuperviseurs()->item(i,0)->text());
    ComboSuperviseurs->setFixedWidth(100);

    QHBoxLayout *soignantlayout     = new QHBoxLayout();
    lblsoignt->setText(tr("RDV avec"));
    soignantlayout->addWidget(lblsoignt);
    soignantlayout->addWidget(ComboSuperviseurs);
    soignantlayout->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding));


    QString req = "select motif, raccourci, couleur, ParDefaut from " NOM_TABLE_MOTIFSRDV " where utiliser = 1 order by NoOrdre";
    QSqlQuery motifQuery(req,db);
    if (motifQuery.size()==0)
    {
        llist << "" << "";
        return llist;
    }
    motifQuery.first();
    int defaut = -1;
    for (int k=0; k<motifQuery.size();k++)
    {
        QRadioButton *radiobut = new QRadioButton(grpBox);
        radiobut->setAutoExclusive(true);
        radiobut->setText(motifQuery.value(0).toString());
        radiobut->setChecked(Motif==motifQuery.value(1).toString());
        if (motifQuery.value(3).toInt()==1)
            defaut = k;
        motifQuery.next();
    }
    QRadioButton    *UrgButton      = new QRadioButton(grpBox);
    UrgButton   ->setAutoExclusive(true);
    UrgButton   ->setText(tr("Urgence"));
    UrgButton   ->setChecked(Motif=="URG");
    bool b = false;
    for (int l=0; l<grpBox->findChildren<QRadioButton*>().size(); l++)
    {
        if (grpBox->findChildren<QRadioButton*>().at(l)->isChecked())
        {
            b = true;
            break;
        }
    }
    if (!b)
        grpBox->findChildren<QRadioButton*>().at(defaut)->setChecked(true);

    int n       = grpBox->findChildren<QRadioButton*>().size();
    int a       = n/2;
    if ((a*2)<n)
        a       += 1;
    for (int i=0;i<a;i++)
        grpBox->findChildren<QRadioButton*>().at(i)->setGeometry(5,30+(25*i),130,20);
    for (int j=a;j<n;j++)
        grpBox->findChildren<QRadioButton*>().at(j)->setGeometry(150,30+(25*(j-a)),130,20);

    grpBox      ->setFixedSize(300,(a*30)+20);

    lbltitre->setText(tr("Message"));
    MsgText->setFixedSize(300,40);
    HeureTitre->setText(tr("Heure de RDV"));
    HeureRDV->setTime(heurerdv);
    HeureRDV->setCurrentSection(QDateTimeEdit::MinuteSection);
    gAsk->AjouteWidgetLayButtons(HeureRDV, false);
    gAsk->AjouteWidgetLayButtons(HeureTitre, false);
    gAsk->AjouteLayButtons(UpDialog::ButtonOK);

    motiflayout->addLayout(soignantlayout);
    motiflayout->addWidget(grpBox);
    motiflayout->addWidget(lbltitre);
    motiflayout->addWidget(MsgText);
    globallayout->insertLayout(0,motiflayout);

    connect(gAsk->OKButton,   SIGNAL(clicked(bool)),  gAsk,   SLOT(accept()));

    gAsk->setWindowTitle(tr("Enregistrer le motif de l'acte"));

    gAsk->setModal(true);
    gAsk->setFixedWidth(320);
    globallayout->setSizeConstraint(QLayout::SetFixedSize);
    MsgText->setText(Message);
    if (gAsk->exec()>0)
    {
        Message = MsgText->toPlainText();
        for (int m=0; m<grpBox->findChildren<QRadioButton*>().size(); m++)
        {
            if (grpBox->findChildren<QRadioButton*>().at(m)->isChecked())
            {
                Motif = grpBox->findChildren<QRadioButton*>().at(m)->text();
                break;
            }
        }
        if (Motif==tr("Urgence"))
            Motif= "URG";
        else
        {
            for (int  o=0; o<motifQuery.size(); o++)
            {
                motifQuery.seek(o);
                if (motifQuery.value(0).toString()==Motif)
                {
                    Motif = motifQuery.value(1).toString();
                    break;
                }
            }
        }
        llist << Motif << Message << HeureRDV->time().toString("HH:mm") << ComboSuperviseurs->currentData().toString();
    }
    delete gAsk;
    return llist;
}

void Rufus::Slot_MenuContextuelUptextEdit()
{
    UpTextEdit *TxtEdit = dynamic_cast<UpTextEdit*>(sender());
    if (!TxtEdit) return;
    gmenuContextuel          = new QMenu();
    QAction *pAction_ModifPolice    = new QAction(this);
    QAction *pAction_Fontbold       = new QAction(this);
    QAction *pAction_Fontitalic     = new QAction(this);
    QAction *pAction_Fontunderline  = new QAction(this);
    QAction *pAction_Fontnormal     = new QAction(this);
    QAction *pAction_Copier         = new QAction(this);
    QAction *pAction_Cut            = new QAction(this);
    QAction *pAction_Coller         = new QAction(this);
    QAction *pAction_Blockcentr     = new QAction(this);
    QAction *pAction_Blockjust      = new QAction(this);
    QAction *pAction_Blockright     = new QAction(this);
    QAction *pAction_Blockleft      = new QAction(this);

    if (TxtEdit->textCursor().selectedText().size() > 0)   {
        pAction_ModifPolice    = gmenuContextuel->addAction(proc->giconFont,           tr("Modifier la police"));
        pAction_Fontbold       = gmenuContextuel->addAction(proc->giconFontbold,       tr("Gras"));
        pAction_Fontitalic     = gmenuContextuel->addAction(proc->giconFontitalic,     tr("Italique"));
        pAction_Fontunderline  = gmenuContextuel->addAction(proc->giconFontunderline,  tr("Souligné"));
        pAction_Fontnormal     = gmenuContextuel->addAction(proc->giconFontnormal,     tr("Normal"));
        gmenuContextuel->addSeparator();
    }
    pAction_Blockleft           = gmenuContextuel->addAction(proc->giconBlockLeft,      tr("Aligné à gauche"));
    pAction_Blockright          = gmenuContextuel->addAction(proc->giconBlockRight,     tr("Aligné à droite"));
    pAction_Blockcentr          = gmenuContextuel->addAction(proc->giconBlockCenter,    tr("Centré"));
    pAction_Blockjust           = gmenuContextuel->addAction(proc->giconBlockJustify,   tr("Justifié"));
    gmenuContextuel->addSeparator();
    if (TxtEdit->textCursor().selectedText().size() > 0)   {
        pAction_Copier         = gmenuContextuel->addAction(proc->giconCopy,            tr("Copier"));
        pAction_Cut            = gmenuContextuel->addAction(proc->giconCut,             tr("Couper"));
    }
    const QClipboard *clipboard = qApp->clipboard();
    const QMimeData *mimeData = clipboard->mimeData();
    if (mimeData->hasText() || mimeData->hasUrls() || mimeData->hasImage() || mimeData->hasHtml())
        pAction_Coller         = gmenuContextuel->addAction(proc->giconPaste,  tr("Coller"));

    connect (pAction_Fontbold,      &QAction::triggered,    [=] {Slot_ChoixMenuContextuelUptextEdit("Gras");});
    connect (pAction_Fontitalic,    &QAction::triggered,    [=] {Slot_ChoixMenuContextuelUptextEdit("Italique");});
    connect (pAction_Fontunderline, &QAction::triggered,    [=] {Slot_ChoixMenuContextuelUptextEdit("Souligne");});
    connect (pAction_Fontnormal,    &QAction::triggered,    [=] {Slot_ChoixMenuContextuelUptextEdit("Normal");});
    connect (pAction_ModifPolice,   &QAction::triggered,    [=] {Slot_ChoixMenuContextuelUptextEdit("Police");});
    connect (pAction_Blockcentr,    &QAction::triggered,    [=] {Slot_ChoixMenuContextuelUptextEdit("Centre");});
    connect (pAction_Blockright,    &QAction::triggered,    [=] {Slot_ChoixMenuContextuelUptextEdit("Droite");});
    connect (pAction_Blockleft,     &QAction::triggered,    [=] {Slot_ChoixMenuContextuelUptextEdit("Gauche");});
    connect (pAction_Blockjust,     &QAction::triggered,    [=] {Slot_ChoixMenuContextuelUptextEdit("Justifie");});
    connect (pAction_Copier,        &QAction::triggered,    [=] {Slot_ChoixMenuContextuelUptextEdit("Copier");});
    connect (pAction_Coller,        &QAction::triggered,    [=] {Slot_ChoixMenuContextuelUptextEdit("Coller");});
    connect (pAction_Cut,           &QAction::triggered,    [=] {Slot_ChoixMenuContextuelUptextEdit("Couper");});

    // ouvrir le menu
    gmenuContextuel->exec(QCursor::pos());
    delete gmenuContextuel;
}

void Rufus::Slot_ChoixMenuContextuelUptextEdit(QString choix)
{
    UpTextEdit *TextWidget = dynamic_cast<UpTextEdit*>(focusWidget());
    if (!TextWidget) return;
    if (choix       == "Coller")    TextWidget->paste();
    else if (choix  == "Copier")    TextWidget->copy();
    else if (choix  == "Couper")    TextWidget->cut();
    else if (choix  == "Police")    {
        bool ok = false;
        QFont police = QFontDialog::getFont(&ok, qApp->font(), this, tr("Choisissez une police"));
        if (ok){
            QTextCharFormat format;
            format.setFont(police);
            TextWidget->textCursor().setCharFormat(format);
        }
    }
    else if (choix  == "Gras")    {
        QTextCharFormat format  = TextWidget->textCursor().charFormat();
        format.setFontWeight(QFont::Bold);
        format.setFontUnderline(format.fontUnderline());
        format.setFontItalic(format.fontItalic());
        TextWidget->textCursor().setCharFormat(format);
    }
    else if (choix  == "Italique")    {
        QTextCharFormat format  = TextWidget->textCursor().charFormat();
        format.setFontItalic(true);
        format.setFontUnderline(format.fontUnderline());
        format.setFontWeight(format.fontWeight());
        TextWidget->textCursor().setCharFormat(format);
    }
    else if (choix  == "Souligne")    {
        QTextCharFormat format  = TextWidget->textCursor().charFormat();
        format.setFontUnderline(true);
        format.setFontItalic(format.fontItalic());
        format.setFontWeight(format.fontWeight());
        TextWidget->textCursor().setCharFormat(format);
    }
    else if (choix  == "Normal")    {
        QTextCharFormat format  = TextWidget->textCursor().charFormat();
        format.setFont(qApp->font());
        TextWidget->textCursor().setCharFormat(format);
    }
    else if (choix  == "Gauche")    {
        QTextCursor curs = TextWidget->textCursor();
        TextWidget->moveCursor(QTextCursor::StartOfBlock);
        TextWidget->moveCursor(QTextCursor::EndOfBlock,QTextCursor::KeepAnchor);
        QTextBlockFormat blockformat  = TextWidget->textCursor().blockFormat();
        blockformat.setAlignment(Qt::AlignLeft);
        TextWidget->textCursor().setBlockFormat(blockformat);
        TextWidget->setTextCursor(curs);
    }
    else if (choix  == "Justifie")    {
        QTextCursor curs = TextWidget->textCursor();
        TextWidget->moveCursor(QTextCursor::StartOfBlock);
        TextWidget->moveCursor(QTextCursor::EndOfBlock,QTextCursor::KeepAnchor);
        QTextBlockFormat blockformat  = TextWidget->textCursor().blockFormat();
        blockformat.setAlignment(Qt::AlignJustify);
        TextWidget->textCursor().setBlockFormat(blockformat);
        TextWidget->setTextCursor(curs);
    }
    else if (choix  == "Droite")    {
        QTextCursor curs = TextWidget->textCursor();
        TextWidget->moveCursor(QTextCursor::StartOfBlock);
        TextWidget->moveCursor(QTextCursor::EndOfBlock,QTextCursor::KeepAnchor);
        QTextBlockFormat blockformat  = TextWidget->textCursor().blockFormat();
        blockformat.setAlignment(Qt::AlignRight);
        TextWidget->textCursor().setBlockFormat(blockformat);
        TextWidget->setTextCursor(curs);
    }
    else if (choix  == "Centre")    {
        QTextCursor curs = TextWidget->textCursor();
        TextWidget->moveCursor(QTextCursor::StartOfBlock);
        TextWidget->moveCursor(QTextCursor::EndOfBlock,QTextCursor::KeepAnchor);
        QTextBlockFormat blockformat  = TextWidget->textCursor().blockFormat();
        blockformat.setAlignment(Qt::AlignCenter);
        TextWidget->textCursor().setBlockFormat(blockformat);
        TextWidget->setTextCursor(curs);
    }
}

void Rufus::Slot_MetAJourLaConnexion()
{
    QString lockrequete = "LOCK TABLES " NOM_TABLE_USERSCONNECTES " WRITE;";
    QSqlQuery lockquery (lockrequete, db);
    if (proc->TraiteErreurRequete(lockquery,lockrequete,"Impossible de verrouiller " NOM_TABLE_USERSCONNECTES))
        return;

    QString MAJConnexionRequete;
    QSqlQuery usrquer("select iduser from " NOM_TABLE_USERSCONNECTES " where NomPosteConnecte = '" + QHostInfo::localHostName().left(60) + "'", db);
    if (usrquer.size()>0)
        MAJConnexionRequete = "UPDATE " NOM_TABLE_USERSCONNECTES " SET HeureDerniereConnexion = NOW(), "
                              " idUser = " + QString::number(gidUser) +
                              " where NomPosteConnecte = '" + QHostInfo::localHostName().left(60) + "'";
    else
       MAJConnexionRequete = "insert into " NOM_TABLE_USERSCONNECTES "(HeureDerniereConnexion, idUser,UserSuperviseur,UserComptable,UserParent,NomPosteConnecte,LastidModifSalDat)"
                               " VALUES(NOW()," +
                               QString::number(gidUser) + "," +
                               QString::number(gidUserSuperviseur) + "," +
                               QString::number(gidUserComptable) + "," +
                               QString::number(gidUserParent) +",'" +
                               QHostInfo::localHostName().left(60) +
                               "', 0)";
    //qDebug() << MAJConnexionRequete;
    QSqlQuery MAJConnexionQuery (MAJConnexionRequete, db);
    proc->TraiteErreurRequete(MAJConnexionQuery, MAJConnexionRequete,"");
    QSqlQuery("unlock tables",db);
}

void Rufus::Slot_ModifActeDate()
{
        ui->ActeDatedateEdit->setEnabled(true);
        ui->ActeDatedateEdit->setFocus();
}

void Rufus::Slot_ModfiCotationActe()
{
    gAutorModifConsult = true;
    ui->Cotationframe->setEnabled(true);
    ui->CCAMlinklabel->setVisible(true);
}

void Rufus::Slot_ModifierTerrain()
{
    TerraintreeWidget->setVisible(false);
    ModifTerrainupSmallButton->setVisible(false);
    ui->ModifTerrainframe->setVisible(true);
    ui->ModifTerrainframe->setEnabled(true);
    ui->OKModifTerrainupSmallButton->setVisible(true);
}

void Rufus::Slot_OKModifierTerrain()
{
    TerraintreeWidget->clear();
    bool a = false;
    TerraintreeWidget->setVisible(a);
    ModifTerrainupSmallButton->setVisible(a);
    ui->OKModifTerrainupSmallButton->setVisible(!a);
    QString requete = "SELECT idPat, idCorMedMG, idCorMedSpe1, idCorMedSpe2, idCorMedSpe3, idCorNonMed, RMPAtcdtsPersos, RMPTtGeneral, RMPAtcdtsFamiliaux"
              ", RMPAtcdtsOphs, Tabac, Autrestoxiques, Gencorresp, Important, Resume, RMPTtOphs,"
              " CorNom, CorPrenom,CorAdresse1,CorAdresse2,CorAdresse3,CorVille,CorTelephone"
              " FROM " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS " AS RMP"
              " left outer join " NOM_TABLE_CORRESPONDANTS " on idcormedMG = idcor"
              " WHERE idPat = " + QString::number(gidPatient);
    QSqlQuery DonneesMedicalesQuery (requete,db);
    if (!proc->TraiteErreurRequete(DonneesMedicalesQuery,requete,tr("Impossible de retrouver les données médicales")))
        if (DonneesMedicalesQuery.size() > 0)           // Il y a des renseignements medicaux
        {
            TerraintreeWidget->setGeometry(5,259,360,420);
            TerraintreeWidget->setColumnCount(2);
            TerraintreeWidget->setColumnWidth(0,70);        //IdPat
            TerraintreeWidget->setColumnWidth(1,180 );     //
            TerraintreeWidget->setStyleSheet("QTreeWidget {selection-color: rgb(0,0,0);"
                                                 " selection-background-color: rgb(164, 205, 255);"
                                                 " background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #f6f7fa, stop: 1 rgba(200, 230, 200, 50));"
                                                 " border: 1px solid rgb(150,150,150); border-radius: 10px;}");
            TerraintreeWidget->setIconSize(QSize(25,25));
            TerraintreeWidget->header()->setVisible(false);
            QTreeWidgetItem *pItem0, *pItem1, *pItem2, *pItem3, *pItem4, *pItem5;
            pItem0 = new QTreeWidgetItem() ;
            pItem0->setText(1,QString::number(gidPatient));                                                             // IdPatient
            pItem0->setText(0,tr("ANTÉCÉDENTS GÉNÉRAUX"));
            pItem0->setIcon(0,proc->giconStetho);
            pItem0->setTextAlignment(1,Qt::AlignLeft);
            pItem1 = new QTreeWidgetItem() ;
            pItem1->setText(1,QString::number(gidPatient));                                                             // IdPatient
            pItem1->setText(0,tr("TRAITEMENTS EN COURS"));
            pItem1->setIcon(0,proc->giconMedoc);
            pItem1->setTextAlignment(1,Qt::AlignLeft);
            pItem2 = new QTreeWidgetItem();
            pItem2->setText(1,QString::number(gidPatient));                                                             // IdPatient
            pItem2->setText(0,tr("ATCDTS OPHTALMOLOGIQUES FAMILIAUX"));
            pItem2->setIcon(0,proc->giconFamily);
            pItem2->setTextAlignment(1,Qt::AlignLeft);
            pItem3 = new QTreeWidgetItem();
            pItem3->setText(1,QString::number(gidPatient));                                                             // IdPatient
            pItem3->setIcon(0,proc->giconSmoking);
            pItem3->setTextAlignment(1,Qt::AlignLeft);
            pItem4 = new QTreeWidgetItem();
            pItem4->setText(1,QString::number(gidPatient));                                                             // IdPatient
            pItem4->setText(0,tr("AUTRES"));
            pItem4->setIcon(0,proc->giconAlcool);
            pItem4->setTextAlignment(1,Qt::AlignLeft);
            pItem5 = new QTreeWidgetItem() ;
            pItem5->setText(0,tr("MÉDECIN GÉNÉRALISTE"));
            pItem5->setText(1,QString::number(gidPatient));                                                             // IdPatient
            pItem5->setIcon(0,proc->giconDoctor);
            pItem5->setTextAlignment(1,Qt::AlignLeft);

            pItem0->setForeground(0,QBrush(QColor(Qt::red)));
            pItem1->setForeground(0,QBrush(QColor(Qt::blue)));
            pItem2->setForeground(0,QBrush(QColor(Qt::darkGreen)));
            pItem3->setForeground(0,QBrush(QColor(Qt::darkMagenta)));
            pItem4->setForeground(0,QBrush(QColor(Qt::darkYellow)));
            pItem5->setForeground(0,QBrush(QColor(Qt::darkBlue)));

            DonneesMedicalesQuery.first();
            QString hash;
            QStringList listhash;
            QFontMetrics fm(qApp->font());
            hash = proc->MajusculePremiereLettre(DonneesMedicalesQuery.value(6).toString(), true, false, false);
            if (hash != "")
            {
                a = true;
                TerraintreeWidget->addTopLevelItem(pItem0);
                pItem0->setFirstColumnSpanned(true);
                listhash = hash.split("\n");
                pItem0->setExpanded(listhash.size() > 0);
                for (int i=0;i<listhash.size();i++)
                {
                    QTreeWidgetItem *pit = new QTreeWidgetItem(pItem0);
                    pit->setText(0,"");
                    pit->setText(1,listhash.at(i));
                    if (fm.width(listhash.at(i)) > (TerraintreeWidget->width() - TerraintreeWidget->columnWidth(0)))
                        pit->setToolTip(1, listhash.at(i));
                }
            }
            listhash.clear();
            hash = proc->MajusculePremiereLettre(DonneesMedicalesQuery.value(7).toString(), true, false, false);
            if (hash != "")
            {
                a = true;
                TerraintreeWidget->addTopLevelItem(pItem1);
                pItem1->setFirstColumnSpanned(true);
                listhash = hash.split("\n");
                pItem1->setExpanded(listhash.size() > 0);
                for (int i=0;i<listhash.size();i++)
                {
                    QTreeWidgetItem *pit = new QTreeWidgetItem(pItem1);
                    pit->setText(0,"");
                    pit->setText(1,listhash.at(i));
                    if (fm.width(listhash.at(i)) > (TerraintreeWidget->width() - TerraintreeWidget->columnWidth(0)))
                        pit->setToolTip(1, listhash.at(i));
                }
            }
            listhash.clear();
            hash = proc->MajusculePremiereLettre(DonneesMedicalesQuery.value(8).toString(), true, false, false);
            if (hash != "")
            {
                a = true;
                TerraintreeWidget->addTopLevelItem(pItem2);
                pItem2->setFirstColumnSpanned(true);
                listhash = hash.split("\n");
                pItem2->setExpanded(listhash.size() > 0);
                for (int i=0;i<listhash.size();i++)
                {
                    QTreeWidgetItem *pit = new QTreeWidgetItem(pItem2);
                    pit->setText(0,"");
                    pit->setText(1,listhash.at(i));
                    if (fm.width(listhash.at(i)) > (TerraintreeWidget->width() - TerraintreeWidget->columnWidth(0)))
                        pit->setToolTip(1, listhash.at(i));
                }
            }
            hash = proc->MajusculePremiereLettre(DonneesMedicalesQuery.value(10).toString(), true, false, false);
            if (hash != "")
            {
                a = true;
                if (hash == "0")
                {
                    pItem3->setForeground(0,QBrush(QColor(Qt::darkGreen)));
                    pItem3->setText(0,tr("NON FUMEUR"));
                }
                else
                    pItem3->setText(0,tr("TABAC : ") + hash + tr(" cig/j"));
                TerraintreeWidget->addTopLevelItem(pItem3);
                pItem3->setFirstColumnSpanned(true);
            }
            listhash.clear();
            hash = proc->MajusculePremiereLettre(DonneesMedicalesQuery.value(11).toString(), true, false, false);
            if (hash != "")
            {
                a = true;
                TerraintreeWidget->addTopLevelItem(pItem4);
                pItem4->setFirstColumnSpanned(true);
                listhash = hash.split("\n");
                pItem4->setExpanded(listhash.size() > 0);
                for (int i=0;i<listhash.size();i++)
                {
                    QTreeWidgetItem *pit = new QTreeWidgetItem(pItem4);
                    pit->setText(0,"");
                    pit->setText(1,listhash.at(i));
                    if (fm.width(listhash.at(i)) > (TerraintreeWidget->width() - TerraintreeWidget->columnWidth(0)))
                        pit->setToolTip(1, listhash.at(i));
                }
            }
            hash = DonneesMedicalesQuery.value(1).toString();
            if (DonneesMedicalesQuery.value(1).toInt()>0)
            {
                QString tooltp ="";
                if (DonneesMedicalesQuery.value(18).toString() != "")
                    tooltp += DonneesMedicalesQuery.value(18).toString();
                if (DonneesMedicalesQuery.value(19).toString() != "")
                    tooltp += "\n" + DonneesMedicalesQuery.value(19).toString();
                if (DonneesMedicalesQuery.value(20).toString() != "")
                    tooltp += "\n" + DonneesMedicalesQuery.value(20).toString();
                if (DonneesMedicalesQuery.value(21).toString() != "")
                    tooltp += "\n" + DonneesMedicalesQuery.value(21).toString();
                if (DonneesMedicalesQuery.value(22).toString() != "")
                    tooltp += "\n" + DonneesMedicalesQuery.value(22).toString();
                hash = "Dr " + DonneesMedicalesQuery.value(17).toString() + " " + DonneesMedicalesQuery.value(16).toString();
                a = true;
                TerraintreeWidget->addTopLevelItem(pItem5);
                pItem5->setFirstColumnSpanned(true);
                pItem5->setExpanded(true);
                QTreeWidgetItem *pit = new QTreeWidgetItem(pItem5);
                pit->setText(0,"");
                pit->setText(1,hash);
                if (fm.width(hash) > (TerraintreeWidget->width() - TerraintreeWidget->columnWidth(0)))
                    pit->setToolTip(1, hash);
            }
        }
    TerraintreeWidget->setVisible(a);
    ModifTerrainupSmallButton->setVisible(a);
    ui->ModifTerrainframe->setVisible(!a);
    ui->ModifTerrainframe->setEnabled(!a);
    ui->OKModifTerrainupSmallButton->setVisible(!a);
}

void Rufus::Slot_NavigationActePrecpushButtonClicked()
{
    NavigationConsult(-1);
}

void Rufus::Slot_NavigationActeSuivpushButtonClicked()
{
    NavigationConsult(1);
}
void Rufus::Slot_NavigationPremierActepushButtonClicked()
{
    NavigationConsult(0);
}

void Rufus::Slot_NavigationDernierActepushButtonClicked()
{
    NavigationConsult(100);
}

void Rufus::Slot_NavigationDossierPrecedentListe()
{
    NavigationDossier(-1);
}

void Rufus::Slot_NavigationDossierSuivantListe()
{
    NavigationDossier(1);
}

void Rufus::Slot_OuvrirActesPrecspushButtonClicked()
{
    QList<dlg_actesprecedents *> listres = findChildren<dlg_actesprecedents *>();
    if (listres.size()> 0)
    {
        if (listres.at(0)->isVisible())
            listres.at(0)->raise();
        else
            listres.at(0)->show();
    }
    else
        OuvrirActesPrecedents(gidActe);
}

void Rufus::Slot_OuvrirDocuments()
{
    OuvrirDocuments();
}

void Rufus::Slot_OuvrirJournalDepenses()
{
    Dlg_Deps           = new dlg_depenses(proc, this);
    if(Dlg_Deps->getInitOK())
    {
        Dlg_Deps->setWindowTitle(tr("Journal des dépenses"));
        Dlg_Deps->ui->GestionComptesupPushButton->setVisible(actionGestionComptesBancaires->isVisible());
        Dlg_Deps->exec();
    }
    delete Dlg_Deps;
}

void Rufus::Slot_OuvrirParametres()
{
    Dlg_Param = new dlg_param(gidUser, proc, this);
    Dlg_Param->setWindowTitle(tr("Paramètres"));
    Dlg_Param->exec();
    if (Dlg_Param->DataUserModifiees())
    {
        ChargeDataUser();
        setWindowTitle("Rufus - " + gUserLogin + " - " + gUserFonction);
    }
    if (Dlg_Param->CotationsModifiees())
    {
        QString req = "insert into " NOM_TABLE_COTATIONS " (typeacte, MontantOPTAM, MontantNonOPTAM, montantpratique, CCAM, iduser) values \n";
        for (int i=0; i<Dlg_Param->ui->ActesCCAMupTableWidget->rowCount(); i++)
        {
            UpCheckBox *check = dynamic_cast<UpCheckBox*>(Dlg_Param->ui->ActesCCAMupTableWidget->cellWidget(i,0));
            if (check != Q_NULLPTR)
                if (check->isChecked())
                {
                    QString codeCCAM, montantOPTAM, montantNonOPTAM, montantprat("");
                    codeCCAM        = Dlg_Param->ui->ActesCCAMupTableWidget->item(i,1)->text();
                    montantOPTAM    = QString::number(QLocale().toDouble(Dlg_Param->ui->ActesCCAMupTableWidget->item(i,2)->text()));
                    montantNonOPTAM = QString::number(QLocale().toDouble(Dlg_Param->ui->ActesCCAMupTableWidget->item(i,3)->text()));
                    QString mtconv  = (gUserOPTAM? montantOPTAM : montantNonOPTAM);
                    if (Dlg_Param->ui->ActesCCAMupTableWidget->columnCount()==6)
                    {
                        UpLineEdit *line = dynamic_cast<UpLineEdit*>(Dlg_Param->ui->ActesCCAMupTableWidget->cellWidget(i,5));
                        if (line != Q_NULLPTR)
                            montantprat = (line->text()!=""? line->text() : mtconv);
                        else
                            montantprat = mtconv;
                    }
                    QString mtprat = (gUserSecteur>1? montantprat : mtconv);
                    QString montantpratique = QString::number(QLocale().toDouble(mtprat));
                    req += "('" + codeCCAM +  "', " + montantOPTAM + "," + montantNonOPTAM + "," + montantpratique + ", 1, " + QString::number(gidUser) + "),\n";
                }
        }
        for (int i=0; i<Dlg_Param->ui->AssocCCAMupTableWidget->rowCount(); i++)
        {
            UpCheckBox *check = dynamic_cast<UpCheckBox*>(Dlg_Param->ui->AssocCCAMupTableWidget->cellWidget(i,0));
            if (check != Q_NULLPTR)
                if (check->isChecked())
                {
                    QString codeCCAM, montantOPTAM(""), montantNonOPTAM(""), montantprat("");
                    codeCCAM        = Dlg_Param->ui->AssocCCAMupTableWidget->item(i,1)->text();
                    UpLineEdit *lineOPTAM = dynamic_cast<UpLineEdit*>(Dlg_Param->ui->AssocCCAMupTableWidget->cellWidget(i,2));
                    if (lineOPTAM != Q_NULLPTR)
                        montantOPTAM    = QString::number(QLocale().toDouble(lineOPTAM->text()));
                    UpLineEdit *lineNonOPTAM = dynamic_cast<UpLineEdit*>(Dlg_Param->ui->AssocCCAMupTableWidget->cellWidget(i,3));
                    if (lineNonOPTAM != Q_NULLPTR)
                        montantNonOPTAM    = QString::number(QLocale().toDouble(lineNonOPTAM->text()));
                    QString mtconv  = (gUserOPTAM? montantOPTAM : montantNonOPTAM);
                    if (Dlg_Param->ui->AssocCCAMupTableWidget->columnCount()==5)
                    {
                        UpLineEdit *line = dynamic_cast<UpLineEdit*>(Dlg_Param->ui->AssocCCAMupTableWidget->cellWidget(i,4));
                        if (line != Q_NULLPTR)
                            montantprat = (line->text()!=""? line->text() : mtconv);
                        else
                            montantprat = mtconv;
                    }
                    QString mtprat = (gUserSecteur>1? montantprat : mtconv);
                    QString montantpratique = QString::number(QLocale().toDouble(mtprat));
                    req += "('" + codeCCAM +  "', " + montantOPTAM + "," + montantNonOPTAM + "," + montantpratique + ", 2, " + QString::number(gidUser) + "),\n";
                }
        }
        for (int i=0; i<Dlg_Param->ui->HorsNomenclatureupTableWidget->rowCount(); i++)
        {
            UpCheckBox *check = dynamic_cast<UpCheckBox*>(Dlg_Param->ui->HorsNomenclatureupTableWidget->cellWidget(i,0));
            if (check != Q_NULLPTR)
                if (check->isChecked())
                {
                    QString codeCCAM, mtconv(""), montantconv;
                    codeCCAM = Dlg_Param->ui->HorsNomenclatureupTableWidget->item(i,1)->text();
                    UpLineEdit *lineconv = dynamic_cast<UpLineEdit*>(Dlg_Param->ui->HorsNomenclatureupTableWidget->cellWidget(i,2));
                    if (lineconv != Q_NULLPTR)
                        mtconv = lineconv->text();
                    montantconv = QString::number(QLocale().toDouble(mtconv));
                    req += "('" + codeCCAM +  "', " + montantconv + "," + montantconv + "," + montantconv + ", 3, " + QString::number(gidUser) + "), \n";
                }
        }
        req = req.left(req.lastIndexOf(")")+1);
        //proc->Edit(req);
        proc->TraiteErreurRequete(QSqlQuery ("delete from " NOM_TABLE_COTATIONS " where idUser = " + QString::number(gidUser),db), "delete from " NOM_TABLE_COTATIONS " where idUser = " + QString::number(gidUser));
        proc->TraiteErreurRequete(QSqlQuery (req,db), req);
        ReconstruitListesActes();
    }
    delete Dlg_Param;
}

void Rufus::Slot_OuvrirListepushButtonClicked()
{
    OuvrirListe(false);
}

void Rufus::Slot_OuvrirNouveauDossierpushButtonClicked()
{
    OuvrirNouveauDossier();
}

void Rufus::Slot_RecopierDossierpushButtonClicked()
{
    RecopierDossier();
}

void Rufus::Slot_RecettesSpeciales()
{
    proc->EnChantier();
}

void Rufus::Slot_Refraction()
{
    Refraction();
}

void Rufus::Slot_RemiseCheques()
{
    RemiseCheques();
}

void Rufus::Slot_RetrouveiDDepuisTab(int x, int y)
{
    if (gUserDroits == SECRETAIRE) return;
    QTableWidget *tableClicked = dynamic_cast<QTableWidget *> (sender());
    if (tableClicked == 0) return;
    UpLabel *labelClicked = dynamic_cast<UpLabel *> (tableClicked->cellWidget(x,y));
    if (labelClicked == 0) return;
    gdossierAOuvrir = labelClicked->getId();
    ChoixDossier(gdossierAOuvrir);
}

void Rufus::Slot_RetrouveMontantActe()
{
    QString Cotation = ui->ActeCotationcomboBox->currentText();
    // On recherche s'il y a un montant enregistré pour cette cotation
    int idx = ui->ActeCotationcomboBox->findText(Cotation);
    if (idx>-1)
    {
        QStringList listMontantActe = ui->ActeCotationcomboBox->itemData(idx).toStringList();
        QString MontantActe;
        if (gUserSecteur>1 && !gCMUPatient)
            MontantActe = QLocale().toString(listMontantActe.at(1).toDouble(),'f',2);
        else
            MontantActe = QLocale().toString(listMontantActe.at(0).toDouble(),'f',2);
        if (MontantActe !=  ui->ActeMontantlineEdit->text())
        {
            gActeMontant = QLocale().toString(QLocale().toDouble(ui->ActeMontantlineEdit->text()),'f',2);
            ValideActeMontantLineEdit(MontantActe, gActeMontant);    // On vérifie que le montant entré n'est pas inférieur au montant déjà payé et on invalide dans ce cas
        }
        if (listMontantActe.at(0) != listMontantActe.at(1))
        {
            ui->BasculerMontantpushButton->setVisible(true);
            ui->BasculerMontantpushButton->setImmediateToolTip(tr("Revenir au tarif conventionnel"));
        }
    }
    else
    {
        QString tarifconventionne = (gUserOPTAM? "OPTAM" : "NoOPTAM");
        QString req =
                "SELECT " + tarifconventionne + ", montantpratique FROM " NOM_TABLE_COTATIONS " cot, " NOM_TABLE_CCAM " cc"
                " where Typeacte = codeccam"
                " and iduser = " + QString::number(gidUserParent) +
                " and codeccam like '" + proc->CorrigeApostrophe(Cotation) + "%'";
        //qDebug() << req;
        QSqlQuery ListCotationsQuery(req,db);
        proc->TraiteErreurRequete(ListCotationsQuery,req,"");
        if (ListCotationsQuery.size()>0)
        {
            ListCotationsQuery.first();
            QString MontantActe;
            if (gUserSecteur>1 && !gCMUPatient)
                MontantActe = QLocale().toString(ListCotationsQuery.value(1).toDouble(),'f',2);
            else
                MontantActe = QLocale().toString(ListCotationsQuery.value(0).toDouble(),'f',2);
            if (MontantActe !=  ui->ActeMontantlineEdit->text())
            {
                gActeMontant = QLocale().toString(QLocale().toDouble(ui->ActeMontantlineEdit->text()),'f',2);
                ValideActeMontantLineEdit(MontantActe, gActeMontant);    // On vérifie que le montant entré n'est pas inférieur au montant déjà payé et on invalide dans ce cas
            }
            if (ListCotationsQuery.value(0) != ListCotationsQuery.value(1))
            {
                ui->BasculerMontantpushButton->setVisible(true);
                ui->BasculerMontantpushButton->setImmediateToolTip(tr("Revenir au tarif conventionnel"));
            }
        }
        else
        {
            QString req = "SELECT OPTAM, NoOPTAM FROM " NOM_TABLE_CCAM " where codeccam like '" + proc->CorrigeApostrophe(Cotation) + "%'";
            QSqlQuery ListCotationsQuery (req,db);
            proc->TraiteErreurRequete(ListCotationsQuery,req,"");
            if (ListCotationsQuery.size()>0)
            {
                ListCotationsQuery.first();
                QString MontantActe;
                if (gUserSecteur>1 && !gCMUPatient && !gUserOPTAM)
                    MontantActe = QLocale().toString(ListCotationsQuery.value(1).toDouble(),'f',2);
                else
                    MontantActe = QLocale().toString(ListCotationsQuery.value(0).toDouble(),'f',2);
                if (MontantActe !=  ui->ActeMontantlineEdit->text())
                {
                    gActeMontant = QLocale().toString(QLocale().toDouble(ui->ActeMontantlineEdit->text()),'f',2);
                    ValideActeMontantLineEdit(MontantActe, gActeMontant);    // On vérifie que le montant entré n'est pas inférieur au montant déjà payé et on invalide dans ce cas
                }
            }
        }
    }
        /*  else
   {
        UpMessageBox msgbox;
        UpSmallButton *OKBouton = new UpSmallButton;
        UpSmallButton *AnnulBouton = new UpSmallButton;
        msgbox.setText("Acte non répertorié.");
        msgbox.setInformativeText("Cette cotation ne fait pas partie de vos habitudes."
                                  "Voulez vous l'intégrer à la liste de vos cotations habituelles?\n");
        msgbox.setIcon(UpMessageBox::Information);
        OKBouton->setText("Oui, intégrer");
        AnnulBouton->setText("Non");
        msgbox.addButton(AnnulBouton, QMessageBox::AcceptRole);
        msgbox.addButton(OKBouton, QMessageBox::AcceptRole);
        msgbox.exec();
        if (msgbox.clickedButton()==OKBouton)
        {
            QString req = "select codeccam, OPTAM, NoOPTAM from " NOM_TABLE_CCAM " where codeccam = " + ui->ActeCotationcomboBox->currentText().toUpper();
            QSqlQuery quer(req,db);
            if (quer.size() == 0)
            {
                proc->EnChantier();
                UpMessageBox msgbox;
                UpSmallButton *OKBouton = new UpSmallButton;
                UpSmallButton *AnnulBouton = new UpSmallButton;
                msgbox.setText("Acte hors convention?");
                msgbox.setInformativeText("Cet acte est-il hors convention?\n");
                msgbox.setIcon(QMessageBox::Information);
                OKBouton->setText("OK, enregistrer");
                AnnulBouton->setText("Non, annnuler");
                msgbox.addButton(AnnulBouton, QMessageBox::AcceptRole);
                msgbox.addButton(OKBouton, QMessageBox::AcceptRole);
                msgbox.exec();
                if (msgbox.clickedButton()==OKBouton)
                    return;

            }
            else
            {
                quer.first();
                if (gUserSecteur == 1 || gUserSecteur == 2)
                {
                    req = "insert into " NOM_TABLE_COTATIONS " (TypeActe, MontantConv, MontantPratique, CCAM, idUser) VALUES ('" +
                            quer.value(0).toString() + "', " +
                            quer.value(1).toString() + ", "  +
                            quer.value(1).toString() + ", "  +
                            "1, " +
                            QString::number(gidUser) + ")";
                    QSqlQuery (req,db);
                }
                else
                {
                    req = "insert into " NOM_TABLE_COTATIONS " (TypeActe, MontantConv, MontantPratique, CCAM, idUser) VALUES ('" +
                            quer.value(0).toString() + "', " +
                            quer.value(2).toString() + ", "  +
                            quer.value(2).toString() + ", "  +
                            "1, " +
                            QString::number(gidUser) + ")";
                    QSqlQuery (req,db);
                }
            }
        }
    }*/
    //on modifie la table Actes avec la nouvelle cotation
    QString requete = "UPDATE " NOM_TABLE_ACTES " SET ActeCotation = '" + Cotation + "' WHERE idActe = " + QString::number(gidActe);
    QSqlQuery UpdateCotationQuery (requete,db);
    proc->TraiteErreurRequete(UpdateCotationQuery,requete,"");
    ui->EnregistrePaiementpushButton->setEnabled(ui->ActeCotationcomboBox->currentText()!="");
}


void Rufus::Slot_SalleDAttente()
{
    if (AutorDepartConsult(true))
    {
        ui->tabWidget->removeTab(ui->tabWidget->indexOf(ui->tabDossier));
        FermeDlgAnnexes();
        OuvrirListe(false);
    }
}

void Rufus::Slot_SendMessage()
{
    QMap<QString, QVariant> map;
    map["null"] = true;
    UpSmallButton *send = dynamic_cast<UpSmallButton*>(sender());
    if (send == ui->SendMessagepushButton)
        SendMessage(map, gidPatient);
    else
        SendMessage(map);
}

void Rufus::Slot_AllusrChkBoxSendMsg(bool a)
{
    for (int i=0; i< gAsk->findChildren<UpCheckBox*>().size(); i++)
        if (gAsk->findChildren<UpCheckBox*>().at(i)->getRowTable() == 1)
            gAsk->findChildren<UpCheckBox*>().at(i)->setChecked(a);
}

void Rufus::Slot_OneusrChkBoxSendMsg(bool a)
{
    if (a)
    {
        bool allchk = true;
        for (int i=0; i< gAsk->findChildren<UpCheckBox*>().size(); i++)
            if (gAsk->findChildren<UpCheckBox*>().at(i)->getRowTable() == 1)
            {
                allchk = gAsk->findChildren<UpCheckBox*>().at(i)->isChecked();
                if (!allchk)
                    break;
            }
        for (int j=0; j< gAsk->findChildren<UpCheckBox*>().size(); j++)
            if (gAsk->findChildren<UpCheckBox*>().at(j)->getRowTable() == 2)
                gAsk->findChildren<UpCheckBox*>().at(j)->setChecked(allchk);
    }
    else
        for (int j=0; j< gAsk->findChildren<UpCheckBox*>().size(); j++)
            if (gAsk->findChildren<UpCheckBox*>().at(j)->getRowTable() == 2)
                gAsk->findChildren<UpCheckBox*>().at(j)->setChecked(false);
}

void Rufus::SendMessage(QMap<QString, QVariant> map, int id, int idMsg)
{
    gAsk            = new UpDialog(this);
    QHBoxLayout     *tasklayout;
    QHBoxLayout     *totallayout    = new QHBoxLayout();
    QVBoxLayout     *destlayout     = new QVBoxLayout();
    QVBoxLayout     *msglayout      = new QVBoxLayout();
    QVBoxLayout     *vbox           = new QVBoxLayout();

    QGroupBox       *UsrGroupBox;
    UpCheckBox      *checkpat, *checktask, *checkurg;
    UpTextEdit      *MsgText;
    QDateEdit       *limitdate;

    gAsk->AjouteLayButtons(UpDialog::ButtonOK);
    QVBoxLayout *globallay = dynamic_cast<QVBoxLayout*>(gAsk->layout());

    /* on prépare 2 layout verticaux et une ligne
     *  detslayout qui va comporter un groupbox où on choisit le destinataire
     *  une ligne verticale pour séparer les 2 layouts
     *  msglayout qui comportera les éléments du message
     *      . checkpat qui indique que ce message est à propos d'un patient
     *      . Msgtext , qtextedit qui affiche le corps du messge
     *      . un layout horizontal quiindique si le msg correpond à une tâche et s'il y a une date limite
     *      . un checkbox pour le caractère urgent
     * on incorpore les 2 layout et la ligne dans un layout horizontal, totallayout
     * on positionne le layout horizontal totallayout dans le layout global de la fiche
    */

    if (id>-1)
    {
        QString req = "select patnom, patprenom from " NOM_TABLE_PATIENTS " where idpat = " + QString::number(id);
        QSqlQuery quer(req,db);
        if (quer.size()>0)
        {
            checkpat       = new UpCheckBox(gAsk);
            checkpat        ->setObjectName("AboutPatupCheckBox");
            quer.first();
            checkpat        ->setText(tr("A propos de ") + quer.value(0).toString().toUpper() + " " + quer.value(1).toString());
            checkpat        ->setChecked(true);
            checkpat        ->setiD(id);
            msglayout       ->addWidget(checkpat);
        }
    }

    QString req1 = "select idUser, UserLogin from " NOM_TABLE_UTILISATEURS " where UserDesactive is NULL and userlogin is not null";
    QSqlQuery usrquer(req1,db);
    if (usrquer.size()>0)
    {
        usrquer.first();
        UsrGroupBox = new QGroupBox(gAsk);
        UsrGroupBox->setTitle(tr("Destinataire"));
        UpCheckBox *Allusrchk = new UpCheckBox();
        Allusrchk->setText(tr("Tout le monde"));
        Allusrchk->setAutoExclusive(false);
        Allusrchk->setRowTable(2);
        connect(Allusrchk,  SIGNAL(clicked(bool)),  this,   SLOT(Slot_AllusrChkBoxSendMsg(bool)));
        vbox->addWidget(Allusrchk);
        QLabel *line = new QLabel(gAsk);
        line->setFrameShape(QFrame::HLine);
        line->setFixedHeight(1);
        vbox->addWidget(line);
        usrquer.first();
        for (int i=0; i<usrquer.size(); i++)
        {
            UpCheckBox *chk0 = new UpCheckBox();
            chk0->setText(usrquer.value(1).toString());
            chk0->setiD(usrquer.value(0).toInt());
            if (idMsg>-1)
                chk0->setChecked(map["listdestinataires"].toStringList().contains(usrquer.value(0).toString()));
            chk0->setRowTable(1);
            connect(chk0,  SIGNAL(clicked(bool)),  this,   SLOT(Slot_OneusrChkBoxSendMsg(bool)));
            vbox->addWidget(chk0);
            chk0->setAutoExclusive(false);
            usrquer.next();
        }
        UsrGroupBox->setLayout(vbox);
        destlayout ->addWidget(UsrGroupBox);
        destlayout ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Minimum,QSizePolicy::Expanding));
    }
    else
    {
        delete gAsk;
        return;
    }

    MsgText        = new UpTextEdit(gAsk);
    MsgText         ->setFixedHeight(140);
    if (idMsg>-1)
        MsgText->setText(map["TexteMessage"].toString());
    msglayout       ->setContentsMargins(5,0,5,0);

    tasklayout     = new QHBoxLayout();
    limitdate      = new QDateEdit(gAsk);
    checktask      = new UpCheckBox(gAsk);
    checkurg       = new UpCheckBox(gAsk);
    checktask       ->setObjectName("TaskupCheckBox");
    checkurg        ->setObjectName("UrgeupCheckBox");
    limitdate       ->setFixedHeight(26);
    limitdate       ->setEnabled(false);
    limitdate       ->setDate(QDate::currentDate());
    if (idMsg>-1)
    {
        if (map["Tache"].toInt()==1)
        {
            checktask->setChecked(true);
            limitdate->setDate(map["DateLimite"].toDate());
            limitdate->setEnabled(true);
        }
        checkurg->setChecked(map["Urge"].toInt()==1);
    }
    checktask       ->setText(tr("Tâche à accomplir avant le "));
    checkurg        ->setText(tr("Urgent"));
    connect(checktask,  SIGNAL(clicked(bool)),  this,   SLOT(Slot_AfficheMessageLimitDate(bool)));
    tasklayout      ->addWidget(checktask);
    tasklayout      ->addWidget(limitdate);
    tasklayout      ->addSpacerItem(new QSpacerItem(10,10,QSizePolicy::Expanding));
    tasklayout      ->setContentsMargins(5,0,0,0);

    msglayout       ->addWidget(checkurg);
    msglayout       ->addWidget(MsgText);
    msglayout       ->addLayout(tasklayout);

    gAsk->OKButton   ->setId(idMsg);
    connect(gAsk->OKButton,   SIGNAL(clicked(int)),  this,   SLOT(Slot_VerifSendMessage(int)));

    totallayout->addLayout(destlayout);
    QLabel *Vline = new QLabel(gAsk);
    Vline->setFrameShape(QFrame::VLine);
    Vline->setFixedWidth(1);
    totallayout->addWidget(Vline);
    totallayout->addLayout(msglayout);
    globallay->insertLayout(0,totallayout);

    gAsk            ->setWindowTitle(tr("Envoyer un message"));
    gAsk            ->setFixedWidth(510);
    globallay       ->setSizeConstraint(QLayout::SetFixedSize);

    if (map["null"].toBool())
    {
        gAsk->exec();
        delete gAsk;
    }
}

void Rufus::Slot_VerifSendMessage(int idMsg)
{
    if (gAsk->findChildren<UpTextEdit*>().at(0)->toPlainText()=="")
    {
        proc->Message(tr("Vous avez oublié de rédiger le texte de votre message!"),2000,false);
        return;
    }
    bool checkusr = false;
    for (int j=0; j< gAsk->findChildren<UpCheckBox*>().size(); j++)
        if (gAsk->findChildren<UpCheckBox*>().at(j)->getRowTable() == 1)
        {
            checkusr = gAsk->findChildren<UpCheckBox*>().at(j)->isChecked();
            if (checkusr)
                break;
        }
    if (!checkusr)
    {
        proc->Message(tr("Vous avez oublié de choisir un destinataire!"),2000,false);
        return;
    }
    QSqlQuery ("SET AUTOCOMMIT = 0;", proc->getDataBase());
    QString lockrequete = "LOCK TABLES " NOM_TABLE_MESSAGES " WRITE, " NOM_TABLE_MESSAGESJOINTURES " WRITE;";
    QSqlQuery (lockrequete, db);
    if (idMsg<0)  // Enregistrement d'un nouveau message
    {
        QString req = "insert into " NOM_TABLE_MESSAGES " (idEmetteur, TexteMessage, idPatient, Tache, DateLimite, Urge, CreeLe)\n values(";
        req += QString::number(gidUser) + ", ";
        req += "'" + proc->CorrigeApostrophe(gAsk->findChildren<UpTextEdit*>().at(0)->toHtml()) + "', ";
        int ncheck = gAsk->findChildren<UpCheckBox*>().size();
        QString idpat = "NULL";
        for (int i=0; i<ncheck; i++)
            if (gAsk->findChildren<UpCheckBox*>().at(i)->objectName()=="AboutPatupCheckBox")
            {
                if (gAsk->findChildren<UpCheckBox*>().at(i)->isChecked())
                {
                    idpat = QString::number(gAsk->findChildren<UpCheckBox*>().at(i)->iD());
                    break;
                }
            }
        req += idpat + ", ";
        QString task = "NULL, NULL";
        for (int i=0; i<ncheck; i++)
            if (gAsk->findChildren<UpCheckBox*>().at(i)->objectName()=="TaskupCheckBox")
            {
                if (gAsk->findChildren<UpCheckBox*>().at(i)->isChecked())
                {
                    task = "1, '" + gAsk->findChildren<QDateTimeEdit*>().at(0)->date().toString("yyyy-MM-dd")  + "'";
                    break;
                }
            }
        req += task + ", ";
        QString urge = "NULL";
        for (int i=0; i<ncheck; i++)
            if (gAsk->findChildren<UpCheckBox*>().at(i)->objectName()=="UrgeupCheckBox")
            {
                if (gAsk->findChildren<UpCheckBox*>().at(i)->isChecked())
                {
                    urge = "1";
                    break;
                }
            }
        req += urge + ", ";
        req += "'" + QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") + "')";
        QSqlQuery quer(req,db);
        //qDebug() << req;
        if (proc->TraiteErreurRequete(quer,req,tr("Impossible d'enregistrer ce message")))
            proc->rollback(db);

        QString ChercheMaxrequete = "SELECT Max(idMessage) FROM " NOM_TABLE_MESSAGES;
        QSqlQuery ChercheMaxidMsgQuery (ChercheMaxrequete,db);
        if (proc->TraiteErreurRequete(ChercheMaxidMsgQuery, ChercheMaxrequete,""))
        {
            proc->rollback(db);
            return;
        }
        if (ChercheMaxidMsgQuery.size()==0)
        {
            proc->rollback(db);
            return;
        }
        ChercheMaxidMsgQuery.first();
        int idmsg = ChercheMaxidMsgQuery.value(0).toInt();
        QList<int> listidusr;
        for (int j=0; j< gAsk->findChildren<UpCheckBox*>().size(); j++)
            if (gAsk->findChildren<UpCheckBox*>().at(j)->getRowTable() == 1)       // c'est le checkbox d'un user
                if (gAsk->findChildren<UpCheckBox*>().at(j)->isChecked())
                    listidusr << gAsk->findChildren<UpCheckBox*>().at(j)->iD();
        if (listidusr.size()==0)
        {
            proc->rollback(db);
            return;
        }
        req = "insert into " NOM_TABLE_MESSAGESJOINTURES " (idMessage, idDestinataire) Values ";
        for (int i=0; i<listidusr.size(); i++)
        {
            req += "(" + QString::number(idmsg) + "," + QString::number(listidusr.at(i)) + ")";
            if (i<listidusr.size()-1)
                req += ",";
        }
        QSqlQuery isnquer(req,db);

        if (proc->TraiteErreurRequete(isnquer, req, tr("Impossible d'enregistrer le message")))
        {
            proc->rollback(db);
            return;
        }
    }
    else  //    modification d'un message existant
    {
        QString req = "update " NOM_TABLE_MESSAGES " set ";
        req += "textemessage = '" + proc->CorrigeApostrophe(gAsk->findChildren<UpTextEdit*>().at(0)->toHtml()) + "', ";
        int ncheck = gAsk->findChildren<UpCheckBox*>().size();
        QString idpat = "idpatient = null, ";
        for (int i=0; i<ncheck; i++)
            if (gAsk->findChildren<UpCheckBox*>().at(i)->objectName()=="AboutPatupCheckBox")
            {
                if (gAsk->findChildren<UpCheckBox*>().at(i)->isChecked())
                {
                    idpat= "idpatient = " + QString::number(gAsk->findChildren<UpCheckBox*>().at(i)->iD()) + ", ";
                    break;
                }
            }
        req += idpat;
        QString task = " Tache = null,  Datelimite = null, ";
        for (int i=0; i<ncheck; i++)
            if (gAsk->findChildren<UpCheckBox*>().at(i)->objectName()=="TaskupCheckBox")
            {
                if (gAsk->findChildren<UpCheckBox*>().at(i)->isChecked())
                {
                    task = "Tache = 1, DateLimite = '" + gAsk->findChildren<QDateTimeEdit*>().at(0)->date().toString("yyyy-MM-dd")  + "', ";
                    break;
                }
            }
        req += task;
        QString urge = "Urge = null ";
        for (int i=0; i<ncheck; i++)
            if (gAsk->findChildren<UpCheckBox*>().at(i)->objectName()=="UrgeupCheckBox")
            {
                if (gAsk->findChildren<UpCheckBox*>().at(i)->isChecked())
                {
                    urge = "Urge = 1 ";
                    break;
                }
            }
        req += urge;
        req += "where idmessage = " + QString::number(idMsg);
        QSqlQuery quer(req,db);
        //qDebug() << req;
        if (proc->TraiteErreurRequete(quer,req,tr("Impossible d'enregistrer ce message")))
            proc->rollback(db);
        QSqlQuery ("delete from " NOM_TABLE_MESSAGESJOINTURES " where idmessage = " + QString::number(idMsg),db);
        QList<int> listidusr;
        for (int j=0; j< gAsk->findChildren<UpCheckBox*>().size(); j++)
            if (gAsk->findChildren<UpCheckBox*>().at(j)->getRowTable() == 1)       // c'est le checkbox d'un user
                if (gAsk->findChildren<UpCheckBox*>().at(j)->isChecked())
                    listidusr << gAsk->findChildren<UpCheckBox*>().at(j)->iD();
        if (listidusr.size()==0)
        {
            proc->rollback(db);
            return;
        }
        req = "insert into " NOM_TABLE_MESSAGESJOINTURES " (idMessage, idDestinataire) Values ";
        for (int i=0; i<listidusr.size(); i++)
        {
            req += "(" + QString::number(idMsg) + "," + QString::number(listidusr.at(i)) + ")";
            if (i<listidusr.size()-1)
                req += ",";
        }
        QSqlQuery isnquer(req,db);
        if (proc->TraiteErreurRequete(isnquer, req, tr("Impossible d'enregistrer le message")))
        {
            proc->rollback(db);
            return;
        }
    }
    proc->Message(tr("Message enregistré"),1000,false);
    proc->commit(db);
    Slot_AfficheMessages(1);
    gAsk->accept();
}

void Rufus::Slot_AfficheMessageLimitDate(bool a)
{
    gAsk->findChildren<QDateEdit*>().at(0)->setEnabled(a);
}

void Rufus::Slot_SurbrillanceSalDat()
{
    UpLabel *lab    = dynamic_cast<UpLabel*>(sender());
    QString styleurg = "background:#EEFFFF ; color: red";
    QString Msg, background;
    QString backgroundsurbrill = "background:#B2D7FF";
    if (lab==NULL)
        return;
    int idpat       = lab->getId();
    int row         = lab->getRow();
    QString color   = "color: black";
    QString colorRDV= "color: black";
    UpLabel *lab0   = dynamic_cast<UpLabel*>(ui->SalleDAttenteupTableWidget->cellWidget(row,0));
    UpLabel *lab1   = dynamic_cast<UpLabel*>(ui->SalleDAttenteupTableWidget->cellWidget(row,1));
    UpLabel *lab2   = dynamic_cast<UpLabel*>(ui->SalleDAttenteupTableWidget->cellWidget(row,2));
    UpLabel *lab3   = dynamic_cast<UpLabel*>(ui->SalleDAttenteupTableWidget->cellWidget(row,3));
    UpLabel *lab4   = dynamic_cast<UpLabel*>(ui->SalleDAttenteupTableWidget->cellWidget(row,4));
    UpLabel *lab5   = dynamic_cast<UpLabel*>(ui->SalleDAttenteupTableWidget->cellWidget(row,5));
    UpLabel *lab6   = dynamic_cast<UpLabel*>(ui->SalleDAttenteupTableWidget->cellWidget(row,6));
    QString req = "select saldat.Motif, HeureArrivee, HeureRDV, Couleur from " NOM_TABLE_SALLEDATTENTE " saldat left outer join " NOM_TABLE_MOTIFSRDV " rdv on saldat.motif = rdv.raccourci "
                  "where idpat = " + QString::number(idpat);
    //qDebug() << req;
    QSqlQuery quer(req,db);
    if (quer.size()>0)
    {
        quer.first();
        Msg = quer.value(0).toString();
        if (quer.value(1).toTime().toString("HH:mm") != "")
        {
            QTime heureArriv = quer.value(1).toTime();
            if (heureArriv.secsTo(QTime::currentTime())/60 < 15)
                color = "color: green";
            else if (heureArriv.secsTo(QTime::currentTime())/60 < 30)
               color = "color: orange";
            else
               color = "color: red";
        }
        if (quer.value(2).toTime().toString("HH:mm") != "")
        {
            QTime heureRDV = quer.value(2).toTime();
            if (heureRDV.secsTo(QTime::currentTime())/60 < 15)
                colorRDV = "color: green";
            else if (heureRDV.secsTo(QTime::currentTime())/60 < 30)
               colorRDV = "color: orange";
            else
               colorRDV = "color: red";
        }
    }
    if (lab->styleSheet().contains(backgroundsurbrill))       // l'enregistrement est en surbrillance, on le remet à la normale
    {
        if (Msg=="URG")
        {
            lab0->setStyleSheet(styleurg);
            lab1->setStyleSheet(styleurg);
            lab2->setStyleSheet(styleurg);
            lab3->setStyleSheet(styleurg);
            lab4->setStyleSheet(styleurg);
            lab5->setStyleSheet(styleurg);
            lab6->setStyleSheet(styleurg);
        }
        else
        {
            QString background = "background:#FFFFEE";
            if (quer.value(3).toString() != "")
                background = "background:#" + quer.value(3).toString();
            lab0->setStyleSheet(background + "; color: black");
            lab1->setStyleSheet(background + "; color: black");
            lab2->setStyleSheet(background + "; color: black");
            lab3->setStyleSheet(background + ";" + colorRDV);
            lab4->setStyleSheet(background + "; color: black");
            lab5->setStyleSheet(background + ";" + color);
            lab6->setStyleSheet(background + "; color: black");
        }
    }
    else
    {
        for (int i=0; i<ui->SalleDAttenteupTableWidget->rowCount(); i++)  // on remet à la normale ceux qui étaient en surbrillance et on met l'enregistrement en surbrillance
        {
            UpLabel *labi0   = dynamic_cast<UpLabel*>(ui->SalleDAttenteupTableWidget->cellWidget(i,0));
            if (labi0->styleSheet().contains(backgroundsurbrill))       // l'enregistrement est en surbrillance, on le remet à la normale
            {
                QString Msgi;
                UpLabel *labi0   = dynamic_cast<UpLabel*>(ui->SalleDAttenteupTableWidget->cellWidget(i,0));
                UpLabel *labi1   = dynamic_cast<UpLabel*>(ui->SalleDAttenteupTableWidget->cellWidget(i,1));
                UpLabel *labi2   = dynamic_cast<UpLabel*>(ui->SalleDAttenteupTableWidget->cellWidget(i,2));
                UpLabel *labi3   = dynamic_cast<UpLabel*>(ui->SalleDAttenteupTableWidget->cellWidget(i,3));
                UpLabel *labi4   = dynamic_cast<UpLabel*>(ui->SalleDAttenteupTableWidget->cellWidget(i,4));
                UpLabel *labi5   = dynamic_cast<UpLabel*>(ui->SalleDAttenteupTableWidget->cellWidget(i,5));
                UpLabel *labi6   = dynamic_cast<UpLabel*>(ui->SalleDAttenteupTableWidget->cellWidget(i,6));
                QString req = "select saldat.Motif, HeureArrivee, HeureRDV, Couleur from " NOM_TABLE_SALLEDATTENTE " saldat left outer join " NOM_TABLE_MOTIFSRDV " rdv on saldat.motif = rdv.raccourci "
                              "where idpat = " + QString::number(labi0->getId());
                QSqlQuery queri(req,db);
                QString color2, colorRDV2;
                if (queri.size()>0)
                {
                    queri.first();
                    Msgi = queri.value(0).toString();
                    if (queri.value(1).toTime().toString("HH:mm") != "")
                    {
                        QTime heureArriv = queri.value(1).toTime();
                        if (heureArriv.secsTo(QTime::currentTime())/60 < 15)
                            color2 = "color: green";
                        else if (heureArriv.secsTo(QTime::currentTime())/60 < 30)
                           color2 = "color: orange";
                        else
                           color2 = "color: red";
                    }
                    if (queri.value(2).toTime().toString("HH:mm") != "")
                    {
                        QTime heureRDV = queri.value(2).toTime();
                        if (heureRDV.secsTo(QTime::currentTime())/60 < 15)
                            colorRDV2 = "color: green";
                        else if (heureRDV.secsTo(QTime::currentTime())/60 < 30)
                           colorRDV2 = "color: orange";
                        else
                           colorRDV2 = "color: red";
                    }
                }
                if (Msgi=="URG")
                {
                    labi0->setStyleSheet(styleurg);
                    labi1->setStyleSheet(styleurg);
                    labi2->setStyleSheet(styleurg);
                    labi3->setStyleSheet(styleurg);
                    labi4->setStyleSheet(styleurg);
                    labi5->setStyleSheet(styleurg);
                    labi6->setStyleSheet(styleurg);
                }
                else
                {
                    background = "background:#" + queri.value(3).toString();
                    labi0->setStyleSheet(background + "; color: black");
                    labi1->setStyleSheet(background + "; color: black");
                    labi2->setStyleSheet(background + "; color: black");
                    labi3->setStyleSheet(background + ";" + colorRDV2);
                    labi4->setStyleSheet(background + "; color: black");
                    labi5->setStyleSheet(background + ";" + color2);
                    labi6->setStyleSheet(background + "; color: black");
                }
            }
        }
        if (Msg=="URG")
        {
            lab0->setStyleSheet(backgroundsurbrill + "; color:red");
            lab1->setStyleSheet(backgroundsurbrill + "; color:red");
            lab2->setStyleSheet(backgroundsurbrill + "; color:red");
            lab3->setStyleSheet(backgroundsurbrill + "; color:red");
            lab4->setStyleSheet(backgroundsurbrill + "; color:red");
            lab5->setStyleSheet(backgroundsurbrill + "; color:red");
            lab6->setStyleSheet(backgroundsurbrill + "; color:red");
        }
        else
        {
            lab0->setStyleSheet(backgroundsurbrill + "; color: black");
            lab1->setStyleSheet(backgroundsurbrill + "; color: black");
            lab2->setStyleSheet(backgroundsurbrill + "; color: black");
            lab3->setStyleSheet(backgroundsurbrill + ";" + colorRDV);
            lab4->setStyleSheet(backgroundsurbrill + "; color: black");
            lab5->setStyleSheet(backgroundsurbrill + ";" + color);
            lab6->setStyleSheet(backgroundsurbrill + "; color: black");
        }
    }
}

void Rufus::Slot_SupprimerActepushButtonClicked()
{
    SupprimerActe();
}

void Rufus::Slot_SupprimerDocs()
{
    QString B = proc->gsettingsIni->value("BDD_LOCAL/PrioritaireGestionDocs").toString();
    QString IpAdr = "";
    if (B=="YES")
        IpAdr = QHostInfo::localHostName() + " - prioritaire";
    else if (B=="NORM")
        IpAdr = QHostInfo::localHostName();
    else
        return;
    if (IpAdr ==  proc->PosteImportDocs())
    {
        QSqlQuery ("lock tables '" NOM_TABLE_DOCSASUPPRIMER "' write", db);
        QString req = "Select filepath from " NOM_TABLE_DOCSASUPPRIMER;
        //qDebug() << req;
        QSqlQuery delreq (req, db);
        for (int i=0; i<delreq.size(); i++)
        {
            delreq.seek(i);
            QString CheminFichier ("");
            if (proc->getModeConnexion() == Procedures::ReseauLocal)
                CheminFichier = proc->gsettingsIni->value("BDD_LOCAL/DossierImagerie").toString();
            if (proc->getModeConnexion() == Procedures::Poste)
                CheminFichier = proc->DirImagerie();
            CheminFichier += delreq.value(0).toString();
            if (!QFile(CheminFichier).remove())
                UpMessageBox::Watch(this, tr("Fichier introuvable!"), CheminFichier);
            QSqlQuery("delete from " NOM_TABLE_DOCSASUPPRIMER " where filepath = '" + delreq.value(0).toString() + "'", db);
        }
        QSqlQuery("unlock tables", db);
    }
}

void Rufus::Slot_SupprimerDossier()
{
    int idpat = 0;
    if (ui->tabWidget->currentWidget() == ui->tabList)
    {
        if (ui->PatientsListeTableView->selectionModel()->selectedIndexes().size() > 0)
            idpat = gListePatientsModel->itemFromIndex(ui->PatientsListeTableView->selectionModel()->selectedIndexes().at(0))->text().toInt();
    }
    else idpat = gidPatient;
    if (idpat > 0)
        SupprimerDossier(idpat);
}

void Rufus::Slot_Tonometrie()
{
    Tonometrie();
}

void Rufus::Slot_TrouverDDN()
{
    if (gMode == RechercheDDN) TrouverDDN();
}

void Rufus::Slot_AfficheMessages(int idx)
{
    QTabWidget* Tabw = Remplir_MsgTabWidget();
    if (Tabw->count()>idx)
        Tabw->setCurrentIndex(idx);
    QVBoxLayout *globallay = new QVBoxLayout();
    if (gMsgDialog != NULL)
        if (gMsgDialog->isVisible())
            gMsgDialog->close();
    gMsgDialog = new QDialog();
    int x = qApp->desktop()->availableGeometry().width();
    int y = qApp->desktop()->availableGeometry().height();
    gMsgDialog->setStyleSheet("border-image: none; background-color:#FAFAFA;");
    Tabw->setParent(gMsgDialog);
    globallay->addWidget(Tabw);
    gMsgDialog->setLayout(globallay);
    gMsgDialog->setSizeGripEnabled(false);
    gMsgDialog->setWindowFlags(Qt::Tool | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::CustomizeWindowHint);
    gMsgDialog->setWindowTitle(tr("Messagerie"));
    gMsgDialog->setMaximumHeight(y-30);
    gMsgDialog->setWindowIcon(proc->giconSunglasses);
    gMsgDialog->move(x-470,30);
    gMsgDialog->setFixedWidth(500);
    gMsgDialog->show();
}

QTabWidget* Rufus::Remplir_MsgTabWidget()
{
    QTabWidget* tabw = new QTabWidget();
    tabw->setIconSize(QSize(25,25));
    // I - Les messages reçus
    QString req =
        "select Distinct mess.idMessage, idEmetteur, TexteMessage, idPatient, Tache, DateLimite, CreeLe, Urge, lu, Fait, idJointure from "
        NOM_TABLE_MESSAGES " mess left outer join " NOM_TABLE_MESSAGESJOINTURES " joint on mess.idmessage = joint.idmessage \n"
        " where \n"
        " iddestinataire = " + QString::number(gidUser) + "\n"
        " order by urge desc, CreeLe desc";
    //proc->Edit(req);
    QSqlQuery querdest(req,db);
    if (querdest.size()>0)
    {
        QScrollArea *Scroll = new QScrollArea();
        Scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        Scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        Scroll->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        Scroll->setFrameShape(QFrame::NoFrame);
        Scroll->setWidgetResizable(true);
        QVBoxLayout *lay = new QVBoxLayout();
        QWidget *widg = new QWidget();
        widg->setLayout(lay);
        Scroll->setWidget(widg);
        tabw->addTab(Scroll, proc->giconPostit, tr("Reçu"));

        querdest.first();

        for (int i=0; i<querdest.size(); i++)
        {
            /*
             * Par enregistrement:
             * un QFrame framerec pour chaque enregistrement
             * un QVBoxlayout reclay pour le contenu de ce QFrame
             *      un QHboxLayout titre lay pour le titre du message (date et expéditeur) et un checkbox lu ou pas lu
             *      un QHBoxLayout Msglay pour le message lui même
             *          un QVBoxLayout Droplay pour les ic^nes poubelle et répondre
             *          une QLine verticale
             *          un QVBoxLayout SMlay pour le corps du message, contenant
             *              un QHboxLayout Tasklay avec une checkbox    -> tâche à effectuer avant le
             *              un QHBoxLayout aboutlay                     -> à propos de qui?
             *              un UpTextEdit Msgtext                       -> le message
             */
            QFrame *framerec = new QFrame();
            //framerec->setFrameShape(QFrame::Panel);

            QVBoxLayout *reclay = new QVBoxLayout(framerec);

            QHBoxLayout *titrelay = new QHBoxLayout();
            UpLabel *Titredoc = new UpLabel();
            Titredoc->setStyleSheet("color: green");
            if (querdest.value(7).toInt()==1)
                Titredoc->setStyleSheet("color: red");
            QString txt = querdest.value(6).toDate().toString(tr("d-MMM-yy")) + " " + querdest.value(6).toTime().toString("h:mm");
            if (querdest.value(1).toInt()>0)
                txt += tr(" de ") + proc->getLogin(querdest.value(1).toInt());
            Titredoc->setText(txt);
            titrelay->addWidget(Titredoc);
            UpCheckBox *Rdchk = new UpCheckBox();
            Rdchk->setChecked(querdest.value(8).toInt()==1);
            Rdchk->setiD(querdest.value(10).toInt());
            Rdchk->setText(tr("lu"));
            Rdchk->setFixedWidth(45);
            connect(Rdchk, SIGNAL(clicked(bool)), this, SLOT(Slot_MsgRead(bool)));
            titrelay->addSpacerItem(new QSpacerItem(10,10,QSizePolicy::Expanding));
            titrelay->addWidget(Rdchk);
            reclay->addLayout(titrelay);

            QHBoxLayout *Msglay = new QHBoxLayout();

            QVBoxLayout *Droplay = new QVBoxLayout();
            Droplay->addSpacerItem(new QSpacerItem(10,10, QSizePolicy::Expanding, QSizePolicy::Expanding));
            if (querdest.value(1).toInt() != gidUser)
            {
                UpLabel *Respondlbl = new UpLabel();
                Respondlbl->setId(querdest.value(0).toInt());
                Respondlbl->setPixmap(QPixmap("://answer.png").scaled(20,20));
                Respondlbl->setImmediateToolTip(tr("Répondre"));
                connect(Respondlbl, SIGNAL(clicked(int)), this, SLOT(Slot_MsgResp(int)));
                Respondlbl->setFixedWidth(25);
                Droplay->addWidget(Respondlbl);
            }

            UpLabel *Dellbl = new UpLabel();
            Dellbl->setId(querdest.value(10).toInt());
            Dellbl->setPixmap(QPixmap("://trash.png").scaled(20,20));
            Dellbl->setFixedWidth(25);
            Dellbl->setMinimumWidth(25);
            connect(Dellbl, SIGNAL(clicked(int)), this, SLOT(Slot_SupprimerMessageRecu(int)));
            Droplay->addWidget(Dellbl);

            Msglay->addLayout(Droplay);

            QLabel *line = new QLabel();
            line->setFrameShape(QFrame::VLine);
            line->setFixedWidth(1);
            Msglay->addWidget(line);

            QVBoxLayout *SMlay = new QVBoxLayout();
            SMlay->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Minimum,QSizePolicy::Expanding));
            if (querdest.value(4).toInt()==1)                                                                           // le message est une tâche
            {
                QHBoxLayout *Tasklay = new QHBoxLayout();
                UpLabel *Todolbl = new UpLabel();
                Todolbl->setText(tr("A effectuer avant le ") + querdest.value(5).toDate().toString(tr("d-MMM-yy")));
                if (querdest.value(9).toInt()!=1)
                {
                    if (QDate::currentDate() >= querdest.value(5).toDate())
                        Todolbl->setStyleSheet("color: red");
                    else if (QDate::currentDate().addDays(3) > querdest.value(5).toDate())
                        Todolbl->setStyleSheet("color: orange");
                }
                Tasklay->addWidget(Todolbl);
                Tasklay->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding));
                UpCheckBox *Dnchk = new UpCheckBox();
                Dnchk->setChecked(querdest.value(9).toInt()==1);
                Dnchk->setiD(querdest.value(10).toInt());
                Dnchk->setText(tr("fait"));
                Dnchk->setFixedWidth(45);
                connect(Dnchk, SIGNAL(clicked(bool)), this, SLOT(Slot_MsgDone(bool)));
                Tasklay->addWidget(Dnchk);
                SMlay->addLayout(Tasklay);
            }
            if (querdest.value(3).toInt()>0)                                                                            // le message concerne un patient
            {
                QHBoxLayout *aboutlay = new QHBoxLayout();
                UpLabel *aboutdoc = new UpLabel();
                QString nomprenom = "";
                QString reqq = "select patprenom, patnom from " NOM_TABLE_PATIENTS " where idpat = " + QString::number(querdest.value(3).toInt());
                QSqlQuery querr(reqq,db);
                if (querr.size()>0)
                {
                    querr.first();
                    nomprenom = tr("à propos de ") + "<b>" + querr.value(0).toString() + " " + querr.value(1).toString() + "</b>";
                }
                if (nomprenom != "")
                {
                    aboutdoc->setText(nomprenom);
                    aboutdoc->setId(querdest.value(3).toInt());
                    aboutlay->addWidget(aboutdoc);
                    aboutlay->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding));
                    SMlay->addLayout(aboutlay);
                }
            }

            UpTextEdit *Msgtxt = new UpTextEdit();
            Msgtxt->setStyleSheet("border: 1px solid rgb(164, 205, 255);border-radius: 5px; background-color:#FFFFFF;");
            if  (querdest.value(7).toInt() == 1)
                Msgtxt->setStyleSheet("border: 2px solid rgb(251, 51, 61);border-radius: 5px; background-color:#FFFFFF;");
            Msgtxt->setText(querdest.value(2).toString());
            QString txt1 = Msgtxt->toHtml();
            Msgtxt->setText(txt1);
            Msgtxt->setReadOnly(true);
            Msgtxt->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            Msgtxt->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

            Msgtxt->document()->setTextWidth(370);
            Msgtxt->setFixedSize(380,Msgtxt->document()->size().height()+2);

            SMlay->addWidget(Msgtxt);

            Msglay->addLayout(SMlay);

            reclay->addLayout(Msglay);
            lay->addWidget(framerec);
            lay->setSpacing(3);
            reclay->setSpacing(3);
            querdest.next();
        }
        lay->addSpacerItem(new QSpacerItem(10,10,QSizePolicy::Minimum,QSizePolicy::Expanding));
    }

    // I - Les messages emis
    req =
        "select Distinct mess.idMessage, iddestinataire, TexteMessage, idPatient, Tache, DateLimite, CreeLe, Urge, lu, Fait, idJointure from \n"
        NOM_TABLE_MESSAGES " mess left outer join " NOM_TABLE_MESSAGESJOINTURES " joint \non mess.idmessage = joint.idmessage \n"
        " where \n"
        " idemetteur = " + QString::number(gidUser) + "\n"
        " and asupprimer is null\n"
        " order by urge desc, CreeLe desc";
    /*
    select Distinct mess.idMessage, iddestinataire, TexteMessage, idPatient, Tache, DateLimite, CreeLe, Urge, lu, Fait, idJointure from
    Rufus.Messagerie mess left outer join Rufus.MessagerieJointures joint
    on mess.idmessage = joint.idmessage
    where
    idemetteur = 1
    and asupprimer is null
    order by urge desc, CreeLe desc
    */
    QSqlQuery queremet(req,db);
    if (queremet.size()>0)
    {
        QScrollArea *Scroll = new QScrollArea();
        Scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        Scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        Scroll->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        Scroll->setFrameShape(QFrame::NoFrame);
        Scroll->setWidgetResizable(true);
        QVBoxLayout *lay = new QVBoxLayout();
        QWidget *widg = new QWidget();
        widg->setLayout(lay);
        Scroll->setWidget(widg);
        tabw->addTab(Scroll, proc->giconMessage, tr("Envoyé"));

        queremet.first();

        for (int i=0; i<queremet.size(); i++)
        {
            QFrame *framerec = new QFrame();
            //framerec->setFrameShape(QFrame::Panel);
            QVBoxLayout *reclay = new QVBoxLayout(framerec);
            QHBoxLayout *titrelay = new QHBoxLayout();
            UpLabel *Titredoc = new UpLabel();
            Titredoc->setStyleSheet("color: green");
            if (queremet.value(7).toInt()==1)
                Titredoc->setStyleSheet("color: red");
            QString txt = queremet.value(6).toDate().toString(tr("d-MMM-yy")) + " " + queremet.value(6).toTime().toString("h:mm");
            if (queremet.value(1).toInt()>0)
                txt += tr(" pour ") + proc->getLogin(queremet.value(1).toInt());
            Titredoc->setText(txt);
            titrelay->addWidget(Titredoc);
            UpCheckBox *Rdchk = new UpCheckBox();
            Rdchk->setChecked(queremet.value(8).toInt()==1);
            Rdchk->setEnabled(false);
            Rdchk->setText(tr("lu"));
            Rdchk->setFixedWidth(45);
            titrelay->addSpacerItem(new QSpacerItem(40,10,QSizePolicy::Expanding, QSizePolicy::Minimum));
            titrelay->addWidget(Rdchk);
            reclay->addLayout(titrelay);

            QHBoxLayout *Msglay = new QHBoxLayout();
            QVBoxLayout *Droplay = new QVBoxLayout();
            Droplay->addSpacerItem(new QSpacerItem(10,10, QSizePolicy::Expanding, QSizePolicy::Expanding));

            UpLabel *Modiflbl = new UpLabel();
            Modiflbl->setId(queremet.value(0).toInt());
            Modiflbl->setPixmap(QPixmap("://edit.png").scaled(20,20));
            Modiflbl->setImmediateToolTip(tr("Modifier"));
            Modiflbl->setFixedWidth(25);
            connect(Modiflbl, SIGNAL(clicked(int)), this, SLOT(Slot_MsgModif(int)));
            Droplay->addWidget(Modiflbl);

            UpLabel *Dellbl = new UpLabel();
            Dellbl->setId(queremet.value(0).toInt());
            Dellbl->setPixmap(QPixmap("://trash.png").scaled(20,20));
            Dellbl->setFixedWidth(25);
            connect(Dellbl, SIGNAL(clicked(int)), this, SLOT(Slot_SupprimerMessageEmis(int)));
            Droplay->addWidget(Dellbl);
            Msglay->addLayout(Droplay);

            QLabel *line = new QLabel();
            line->setFrameShape(QFrame::VLine);
            line->setFixedWidth(1);
            Msglay->addWidget(line);

            QVBoxLayout *SMlay = new QVBoxLayout();
            SMlay->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Minimum,QSizePolicy::Expanding));
            if (queremet.value(4).toInt()==1)                                                                           // le message est une tâche
            {
                QHBoxLayout *Tasklay = new QHBoxLayout();
                UpLabel *Todolbl = new UpLabel();
                Tasklay->setSpacing(0);
                Todolbl->setText(tr("A effectuer avant le ") + queremet.value(5).toDate().toString(tr("d-MMM-yy")));
                if (queremet.value(9).toInt()!=1)
                {
                    if (QDate::currentDate() >= queremet.value(5).toDate())
                        Todolbl->setStyleSheet("color: red");
                    else if (QDate::currentDate().addDays(3) > queremet.value(5).toDate())
                        Todolbl->setStyleSheet("color: orange");
                }
                Tasklay->addWidget(Todolbl);
                Tasklay->addSpacerItem(new QSpacerItem(10,10,QSizePolicy::Expanding,QSizePolicy::Minimum));
                UpCheckBox *Dnchk = new UpCheckBox();
                Dnchk->setChecked(queremet.value(9).toInt()==1);
                Dnchk->setEnabled(false);
                Dnchk->setText(tr("fait"));
                Dnchk->setFixedWidth(45);
                Tasklay->addWidget(Dnchk);
                SMlay->addLayout(Tasklay);
            }
            if (queremet.value(3).toInt()>0)                                                                            // le message concerne un patient
            {
                QHBoxLayout *aboutlay = new QHBoxLayout();
                UpLabel *aboutdoc = new UpLabel();
                QString nomprenom = "";
                QString reqq = "select patprenom, patnom from " NOM_TABLE_PATIENTS " where idpat = " + QString::number(queremet.value(3).toInt());
                QSqlQuery querr(reqq,db);
                if (querr.size()>0)
                {
                    querr.first();
                    nomprenom = tr("à propos de ") + "<b>" + querr.value(0).toString() + " " + querr.value(1).toString() + "</b>";
                }
                if (nomprenom != "")
                {
                    aboutdoc->setText(nomprenom);
                    aboutlay->addWidget(aboutdoc);
                    aboutlay->addSpacerItem(new QSpacerItem(40,10,QSizePolicy::Expanding, QSizePolicy::Minimum));
                    SMlay->addLayout(aboutlay);
                }
            }

            UpTextEdit *Msgtxt = new UpTextEdit();
            Msgtxt->setStyleSheet("border: 1px solid rgb(164, 205, 255);border-radius: 5px; background-color:#FFFFDD;");
            Msgtxt->setText(queremet.value(2).toString());
            QString txt1 = Msgtxt->toHtml();
            Msgtxt->setText(txt1);
            Msgtxt->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            Msgtxt->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

            Msgtxt->document()->setTextWidth(370);
            Msgtxt->setFixedSize(380,Msgtxt->document()->size().height()+2);
            Msgtxt->setTableCorrespondant(NOM_TABLE_MESSAGES);
            Msgtxt->setChampCorrespondant("TexteMessage");
            Msgtxt->setId(queremet.value(0).toInt());
            Msgtxt->installEventFilter(this);
            Msgtxt->setReadOnly(true);

            SMlay->addWidget(Msgtxt);

            Msglay->addLayout(SMlay);

            reclay->addLayout(Msglay);
            lay->addWidget(framerec);
            lay->setSpacing(3);
            reclay->setSpacing(3);
            queremet.next();
        }
        lay->addSpacerItem(new QSpacerItem(10,10,QSizePolicy::Minimum,QSizePolicy::Expanding));
    }

    if (querdest.size()==0 && queremet.size()==0)
    {
        QScrollArea *Scroll = new QScrollArea();
        Scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        Scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        Scroll->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        Scroll->setFrameShape(QFrame::NoFrame);
        Scroll->setWidgetResizable(true);
        QVBoxLayout *lay = new QVBoxLayout();
        QWidget *widg = new QWidget();
        widg->setLayout(lay);
        Scroll->setWidget(widg);
        tabw->addTab(Scroll,tr("Pas de messages"));
        UpLabel *Msg= new UpLabel();
        Msg->setText(tr("PAS DE MESSAGES"));
        Msg->setAlignment(Qt::AlignCenter);
        lay->addWidget(Msg);
    }
    return tabw;
}

void Rufus::Slot_MsgResp(int idmsg)
{
    QVBoxLayout *globallay = new QVBoxLayout();
    gMsgRepons = new QDialog();

    QString req = "select userlogin from " NOM_TABLE_UTILISATEURS " where iduser in (select idemetteur from " NOM_TABLE_MESSAGES " where idmessage = " + QString::number(idmsg) +  ")";
    QSqlQuery quer(req,db);
    if (quer.size()==0)
    {
        UpMessageBox::Watch(this,tr("Impossible de retrouver l'expéditeur du message"));
        return;
    }
    QLabel *lbl = new QLabel(gMsgRepons);
    quer.first();
    lbl->setText(tr("Réponse au message de ") + "<font color=\"green\"><b>" + quer.value(0).toString() + "</b></font>");
    globallay->addWidget(lbl);
    req = "select textemessage, idpatient from " NOM_TABLE_MESSAGES " where idmessage = " + QString::number(idmsg);
    QSqlQuery txtquer(req,db);
    if (txtquer.size()>0)
    {
        QHBoxLayout *lbllayout  = new QHBoxLayout();
        UpLabel     *msglbl     = new UpLabel(gMsgRepons);
        txtquer     .first();
        QString nomprenom = "";
        if (txtquer.value(1).toInt()>0)
        {
            QString reqq = "select patprenom, patnom from " NOM_TABLE_PATIENTS " where idpat = " + QString::number(txtquer.value(1).toInt());
            QSqlQuery querr(reqq,db);
            if (querr.size()>0)
            {
                querr.first();
                nomprenom = tr("à propos de ") + "<b>" + querr.value(0).toString() + " " + querr.value(1).toString() + "</b>";
            }
        }
        if (nomprenom != "")
            msglbl->setText(nomprenom + "\n");
        msglbl      ->setText(msglbl->text() + txtquer.value(0).toString());
        lbllayout   ->addSpacerItem(new QSpacerItem(30,1));
        lbllayout   ->addWidget(msglbl);
        globallay   ->addLayout(lbllayout);
    }

    UpTextEdit* rponstxt = new UpTextEdit(gMsgRepons);
    rponstxt->setStyleSheet("border: 1px solid rgb(164, 205, 255);border-radius: 5px; background-color:#FFFFFF;");
    rponstxt->setParent(gMsgRepons);

    QHBoxLayout     *buttonlayout;
    buttonlayout   = new QHBoxLayout();
    UpSmallButton *OKbutton       = new UpSmallButton("", gMsgRepons);
    OKbutton        ->setUpButtonStyle(UpSmallButton::STARTBUTTON);
    OKbutton        ->setId(idmsg);
    connect(OKbutton, SIGNAL(clicked(int)), this,   SLOT(Slot_EnregMsgResp(int)));
    buttonlayout    ->addSpacerItem(new QSpacerItem(10,10,QSizePolicy::Expanding));
    buttonlayout    ->addWidget(OKbutton);
    buttonlayout    ->setContentsMargins(0,0,5,5);

    globallay->addWidget(rponstxt);
    globallay->addLayout(buttonlayout);

    gMsgRepons->setLayout(globallay);
    gMsgRepons->setSizeGripEnabled(false);
    gMsgRepons->setWindowFlags(Qt::Tool | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::CustomizeWindowHint);
    gMsgRepons->setWindowTitle(tr("Messagerie"));
    int y = qApp->desktop()->availableGeometry().height();
    gMsgRepons->setMaximumHeight(y-30);
    gMsgRepons->setWindowIcon(proc->giconSunglasses);
    gMsgRepons->setFixedWidth(450);

    gMsgRepons->exec();
    delete gMsgRepons;
}

void Rufus::Slot_EnregMsgResp(int idmsg)
{
     if (gMsgRepons->findChildren<UpTextEdit*>().at(0)->toPlainText()=="")
    {
        proc->Message(tr("Vous avez oublié de rédiger le texte de votre message!"),2000,false);
        return;
    }
    QString req = "select idemetteur, tache, datelimite, urge from " NOM_TABLE_MESSAGES " where idmessage = " + QString::number(idmsg);
    QSqlQuery quer(req,db);
    if (quer.size() == 0)
        return;
    quer.first();
    int iddest          = quer.value(0).toInt();
    QString tache       = ((quer.value(1).toInt()==1)? "1" : "null");
    QString DateLimit   = ((quer.value(2).toDate().isValid())? "'" + quer.value(2).toDate().toString("yyyy-MM-dd") + "'" : "null");
    QString Urg         = ((quer.value(3).toInt()==1)? "1" : "null");
    QSqlQuery ("SET AUTOCOMMIT = 0;", proc->getDataBase());
    QString lockrequete = "LOCK TABLES " NOM_TABLE_MESSAGES " WRITE, " NOM_TABLE_MESSAGESJOINTURES " WRITE;";
    QSqlQuery (lockrequete, db);
    req  = "insert into " NOM_TABLE_MESSAGES " (idEmetteur, TexteMessage, CreeLe, ReponseA, Tache, Datelimite, Urge)\n values(";
    req += QString::number(gidUser) + ", ";
    QString Reponse = "<font color = " + proc->CouleurTitres + ">" + gMsgRepons->findChildren<UpLabel*>().at(0)->text() + "</font>"
            + "------<br><b>" + gUserLogin + ":</b> " + gMsgRepons->findChildren<UpTextEdit*>().at(0)->toPlainText().replace("\n","<br>");
    UpTextEdit txt;
    txt.setText(Reponse);
    req += "'" + proc->CorrigeApostrophe(txt.toHtml()) + "', ";
    req += "'" + QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") + "', ";
    req += QString::number(idmsg) + ", ";
    req += tache + ", ";
    req += DateLimit + ", ";
    req += Urg + ")";
    //qDebug() << req;
    QSqlQuery insquer(req,db);

    if (proc->TraiteErreurRequete(insquer,req,tr("Impossible d'enregistrer ce message")))
        proc->rollback(db);

    QString ChercheMaxrequete = "SELECT Max(idMessage) FROM " NOM_TABLE_MESSAGES;
    QSqlQuery ChercheMaxidMsgQuery (ChercheMaxrequete,db);
    if (proc->TraiteErreurRequete(ChercheMaxidMsgQuery, ChercheMaxrequete,""))
    {
        proc->rollback(db);
        return;
    }
    if (ChercheMaxidMsgQuery.size()==0)
    {
        proc->rollback(db);
        return;
    }
    ChercheMaxidMsgQuery.first();
    int idrep = ChercheMaxidMsgQuery.value(0).toInt();
    req = "insert into " NOM_TABLE_MESSAGESJOINTURES " (idMessage, idDestinataire) Values ";
    req += "(" + QString::number(idrep) + "," + QString::number(iddest) + ")";
    QSqlQuery isnquer(req,db);

    if (proc->TraiteErreurRequete(isnquer, req, tr("Impossible d'enregistrer le message")))
    {
        proc->rollback(db);
        return;
    }
    else
    {
        proc->Message(tr("Message enregistré"),1000,false);
        proc->commit(db);
    }
    gMsgRepons->accept();
}

void Rufus::Slot_MsgModif(int idmsg)
{
    QList<UpTextEdit*> listtxt = gMsgDialog->findChildren<UpTextEdit*>();
    if (listtxt.size()>0)
        for (int i=0; i<listtxt.size();i++)
        {
            if (listtxt.at(i)->getId()==idmsg)
            {
                QString req = "select TexteMessage, idPatient, Tache, DateLimite, CreeLe, Urge from " NOM_TABLE_MESSAGES
                              " where idMessage = " + QString::number(idmsg);
                QSqlQuery quer(req,db);
                quer.first();
                QMap<QString, QVariant> map;
                map["TexteMessage"]     = quer.value(0).toString();
                map["idPatient"]        = quer.value(1).toInt();
                map["Tache"]            = quer.value(2).toInt();
                map["DateLimite"]       = quer.value(3).toDate();
                map["CreeLe"]           = quer.value(4).toDateTime();
                map["Urge"]             = quer.value(5).toInt();
                map["null"]             = false;

                QStringList listdestinataires;
                req = "select iddestinataire from " NOM_TABLE_MESSAGESJOINTURES " where idmessage = " + QString::number(idmsg);
                QSqlQuery jointquer(req,db);
                jointquer.first();
                for (int i=0; i<jointquer.size();i++)
                {
                    listdestinataires << jointquer.value(0).toString();
                    jointquer.next();
                }
                map["listdestinataires"] = listdestinataires;

                SendMessage(map, map["idPatient"].toInt(), idmsg);                           //depuis gMsgDialog
                gAsk->exec();
                delete gAsk;
                i =listtxt.size();
            }
        }
}

void Rufus::Slot_MsgDone(bool a)
{
    UpCheckBox *chk = dynamic_cast<UpCheckBox*>(sender());
    int idjoin = chk->iD();
    QString res = (a? "1" : "NULL");
    QSqlQuery("update " NOM_TABLE_MESSAGESJOINTURES " set Fait = " + res + " where idjointure = " + QString::number(idjoin), db);
}

void Rufus::Slot_MsgRead(bool a)
{
    UpCheckBox *chk = dynamic_cast<UpCheckBox*>(sender());
    int idjoin = chk->iD();
    QString res = (a? "1" : "NULL");
    QSqlQuery("update " NOM_TABLE_MESSAGESJOINTURES " set Lu = " + res + " where idjointure = " + QString::number(idjoin), db);
}

void Rufus::Slot_SupprimerMessageEmis(int idMsg)
{
    QString req = "update " NOM_TABLE_MESSAGES " set ASupprimer = 1 where idmessage = " + QString::number(idMsg);
    QSqlQuery (req,db);
    req = "delete from " NOM_TABLE_MESSAGESJOINTURES " where "
          "idmessage = " + QString::number(idMsg) +
          " and iddestinataire = " + QString::number(gidUser);
    QSqlQuery (req,db);
    if (gMsgDialog->findChildren<QScrollArea*>().size()>0)
        Slot_AfficheMessages(1);
}

void Rufus::Slot_SupprimerMessageRecu(int idJoint)
{
    QString req = "select idmessage from " NOM_TABLE_MESSAGESJOINTURES  " where idjointure = " + QString::number(idJoint);
    //qDebug()<<req;
    QSqlQuery idmsgquer(req,db);
    idmsgquer.first();
    int idmsg = idmsgquer.value(0).toInt();
    req = "select idemetteur from " NOM_TABLE_MESSAGES  " where idmessage = " + QString::number(idmsg);
    QSqlQuery idemetquer(req,db);
    idemetquer.first();
    int idusr = idemetquer.value(0).toInt();
    if (idusr==gidUser)
        QSqlQuery ("update " NOM_TABLE_MESSAGES " set ASupprimer = 1 where idmessage = " + QString::number(idmsg),db);
    QSqlQuery ("delete from " NOM_TABLE_MESSAGESJOINTURES " where idjointure = " + QString::number(idJoint),db);
    req = "delete from " NOM_TABLE_MESSAGES " where "
          "idmessage not in (select idmessage from " NOM_TABLE_MESSAGESJOINTURES ") "
          " and ASupprimer = 1";
    QSqlQuery (req,db);
    if (gMsgDialog->findChildren<QScrollArea*>().size()>0)
        Slot_AfficheMessages();
}

void Rufus::Slot_VerifMessages()
{
    QDateTime DateMsg;
    gTotalNvxMessages = 0;
    QString req =
        "select Distinct mess.idMessage, Creele, ReponseA from "
        NOM_TABLE_MESSAGES " mess left outer join " NOM_TABLE_MESSAGESJOINTURES " joint on mess.idmessage = joint.idmessage \n"
        " where \n"
        " iddestinataire = " + QString::number(gidUser) + "\n"
        " or (idemetteur = " + QString::number(gidUser) + " and asupprimer is null)"
        " order by CreeLe";
    /*
    select Distinct mess.idMessage, Creele, ReponseA from Rufus.Messagerie mess left outer join Rufus.MessagerieJointures joint on mess.idmessage = joint.idmessage
    where iddestinataire = 1
    or (idemetteur = 1 and asupprimer is null)
    order by CreeLe
    */
    QSqlQuery quer(req,proc->getDataBase());
    gTotalMessages = quer.size();
    gMessageIcon->setVisible(gTotalMessages>0);
    if (gTotalMessages>0)
    {
        quer.first();
        for (int i=0; i<gTotalMessages; i++)
        {
            DateMsg = QDateTime(quer.value(1).toDate(), quer.value(1).toTime());
            if (DateMsg > QDateTime(gUserDateDernierMessage))
                gTotalNvxMessages += 1;
            quer.next();
        }
        gUserDateDernierMessage = QDateTime(DateMsg);
    }
    else if (gMsgDialog !=NULL)
    {
        if (gMsgDialog->isVisible())
            gMsgDialog->close();
    }

    QString msg = "";
    if (gAffichTotalMessages)
    {
        if (gTotalMessages==gTotalNvxMessages)
        {
            if (gTotalMessages>1)
                msg = tr("Vous avez ") + QString::number(gTotalMessages) + tr(" nouveaux messages");
            else if (gTotalMessages>0)
                msg = tr("Vous avez 1 nouveau message");
        }
        else if (gTotalMessages>gTotalNvxMessages)
        {
            if (gTotalMessages>1)
            {
                msg = tr("Vous avez ") + QString::number(gTotalMessages) + tr(" messages");
                if (gTotalNvxMessages>1)
                    msg += " dont " + QString::number(gTotalNvxMessages) + tr(" nouveaux");
                else if (gTotalNvxMessages>0)
                    msg += tr(" dont 1 nouveau");
            }
            else if (gTotalMessages>0)
                msg = tr("Vous avez 1 message");
        }
    }
    else if (gTotalNvxMessages>1)
        msg = tr("Vous avez ") + QString::number(gTotalNvxMessages) + tr(" nouveaux messages");
    else if (gTotalNvxMessages>0)
        msg = tr("Vous avez 1 nouveau message");
    if (msg!="")
    {
        QSound::play(NOM_ALARME);
        gMessageIcon->showMessage(tr("Messages"), msg, proc->giconPostit, 10000);
        if (gMsgDialog !=NULL)
            if (gMsgDialog->isVisible())
                Slot_AfficheMessages();
    }
    gAffichTotalMessages = false;
}

void Rufus::Slot_VerifSalleDAttente()
{

    if (ui->tabWidget->currentIndex() == ui->tabWidget->indexOf(ui->tabList))
    {
        QString SalDatrequete = "SELECT idUser From " NOM_TABLE_USERSCONNECTES
                " WHERE NomPosteConnecte = '" + QHostInfo::localHostName().left(60) + "'" +
                " AND (LastidModifSalDat < NewidModifSalDat OR (lastidmodifSalDat is null and Newidmodifsaldat > 0))";
        QSqlQuery VerifModifSalleDAttenteQuery (SalDatrequete, db);
        if (proc->TraiteErreurRequete(VerifModifSalleDAttenteQuery,SalDatrequete,""))
            return;
        else
        {
            if (VerifModifSalleDAttenteQuery.size() > 0)  // la salle d'attente a été modifiée => on la reconstruit
            {
                Slot_MetAJourLaConnexion();
                SalDatrequete = "UPDATE " NOM_TABLE_USERSCONNECTES " SET LastidModifSalDat = NewidModifSalDat"
                                " WHERE NomPosteConnecte = '" + QHostInfo::localHostName().left(60) + "'";
                QSqlQuery ModifSalleDAttenteQuery (SalDatrequete,db);
                if (proc->TraiteErreurRequete(ModifSalleDAttenteQuery,SalDatrequete,""))
                    return;
                Remplir_SalDat();
            }
        }
    }
    // on en profite aussi pour remettre à jour les treeview au besoin si un autre utilisteur a créé ou supprimé un dossier
    if (gflagPatients < proc->GetflagPatients())
    {
        Remplir_ListePatients_TableView(grequeteListe,"","");
        gflagPatients = proc->GetflagPatients();
        CalcNbDossiers();
    }
    // on en profite aussi pour remettre à jour la liste des correspondants au besoin si un autre utilisteur a créé ou supprimé un dossier
    if (gflagMG < proc->GetflagMG())
    {
        gflagMG = proc->GetflagMG();
        // on reconstruit la liste des MG et des correspondants
        ReconstruitListesCorresp();
        // on resynchronise l'affichage du combobox au besoin
        if (ui->tabWidget->indexOf(ui->tabDossier) > -1)
        {
            QString req = "select idcormedmg from " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS " where idpat = " + QString::number(gidPatient);
            QSqlQuery quer(req,db);
            if (quer.seek(0))
                ui->MGupComboBox->setCurrentIndex(ui->MGupComboBox->findData(quer.value(0).toInt()));
            else
                ui->MGupComboBox->setCurrentIndex(-1);
            req = "select idcormedspe1 from " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS " where idpat = " + QString::number(gidPatient);
            QSqlQuery quer1(req,db);
            if (quer1.seek(0))
                ui->AutresCorresp1upComboBox->setCurrentIndex(ui->AutresCorresp1upComboBox->findData(quer1.value(0).toInt()));
            else
                ui->AutresCorresp1upComboBox->setCurrentIndex(-1);
            req = "select idcormedspe2 from " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS " where idpat = " + QString::number(gidPatient);
            QSqlQuery quer2(req,db);
            if (quer2.seek(0))
                ui->AutresCorresp2upComboBox->setCurrentIndex(ui->AutresCorresp2upComboBox->findData(quer2.value(0).toInt()));
            else
                ui->AutresCorresp2upComboBox->setCurrentIndex(-1);
            Slot_OKModifierTerrain();
        }
    }
}

void Rufus::Slot_VerifVerrouDossier()
{
    // On en profite au passage pour sauvegarder la position de la fenêtre principale
     //bug Qt? -> cette ligne de code ne peut pas être mise juste avant exit(0) sinon elle n'est pas éxécutée...
     proc->gsettingsIni->setValue("PositionsFiches/Rufus", saveGeometry());

    // on verifie l'importateur des docs externes
     VerifImportateur();

    /* Cette fonction sert à déconnecter et lever les verrous d'un utilisateur qui se serait déconnecté accidentellement
     *
     on fait la liste des utilisateurs qui n'ont pas remis à jour leur connexion depuis plus de 60 secondes,
     on retire les verrous qu'ils auraient pu poser et on les déconnecte*/
    QString VerifOldUserreq = "select idUser, NomPosteConnecte from  " NOM_TABLE_USERSCONNECTES " where time_to_sec(timediff(now(),heurederniereconnexion)) > 60";
    QSqlQuery verifoldquery (VerifOldUserreq,db);
    //qDebug() << VerifOldUserreq;
    proc->TraiteErreurRequete(verifoldquery,VerifOldUserreq,"");

    if (verifoldquery.size() > 0)
    {
        verifoldquery.first();
        for (int i=0; i<verifoldquery.size();i++)
        {
            //on déverrouille les dossiers verrouillés par cet utilisateur et on les remet en salle d'attente
            QString blabla              = ENCOURSEXAMEN;
            int length                  = blabla.size();
            int a                       = verifoldquery.value(0).toInt();
            QString Poste               = verifoldquery.value(1).toString();
            QString LibereVerrouRequete;
            LibereVerrouRequete = "UPDATE " NOM_TABLE_SALLEDATTENTE " SET Statut = '" ARRIVE "', idUserEnCoursExam = null, PosteExamen = null"
                                  " WhERE idUserEnCoursExam = " + QString::number(a) +
                                  " AND PosteExamen = '" + Poste +
                                  "' AND Left(Statut," + QString::number(length) + ") = '" ENCOURSEXAMEN "'";
            QSqlQuery LibereVerrouRequeteQuery (LibereVerrouRequete,db);
            proc->TraiteErreurRequete(LibereVerrouRequeteQuery,LibereVerrouRequete,"");
            //qDebug() << LibereVerrouRequete;
            //on déverrouille les actes verrouillés en comptabilité par cet utilisateur
            LibereVerrouRequete = "delete from " NOM_TABLE_VERROUCOMPTAACTES " where PosePar = " + QString::number(a);
            QSqlQuery LibereVerrouComptaQuery (LibereVerrouRequete,db);
            proc->TraiteErreurRequete(LibereVerrouComptaQuery,LibereVerrouRequete,"");
            // on retire cet utilisateur de la table des utilisateurs connectés
            QString req = "delete from " NOM_TABLE_USERSCONNECTES " where NomPosteConnecte = '" + Poste + "'";
            QSqlQuery(req,db);
            proc->UpdVerrouSalDat();
            proc->Message(tr("Le poste ") + Poste + tr(" a été retiré de la liste des postes connectés actuellement au serveur"),1000);
            verifoldquery.next();
        }
    }

    // on donne le statut "arrivé" aux patients en salle d'attente dont le iduserencourssexam n'est plus present sur ce poste examen dans la liste des users connectes
    QString req = "select iduserencoursexam, posteexamen, idpat from " NOM_TABLE_SALLEDATTENTE " where statut like '" ENCOURSEXAMEN "%'";
    //qDebug() << req;
    QSqlQuery querr(req,db);
    for (int i=0; i<querr.size(); i++)
    {
        querr.seek(i);
        req = "select iduser, nomposteconnecte from " NOM_TABLE_USERSCONNECTES " where iduser = " + querr.value(0).toString()  + " and nomposteconnecte = '" + querr.value(1).toString() + "'";
        //qDebug() << req;
        QSqlQuery squer(req,db);
        if (squer.size()==0)
        {
            req = "update " NOM_TABLE_SALLEDATTENTE " set Statut = '" ARRIVE "', posteexamen = null, iduserencoursexam = null where idpat = " + querr.value(2).toString();
            //qDebug() << req;
            QSqlQuery(req, db);
        }
    }
}

void Rufus::VerifImportateur()
{
    /* s'il n'existe pas de poste défini comme importateur des documents, on prend la place si on est accrédite et qu'on n'est pas en accès distant
     * s'il existe un poste défini comme importateur des docs et qu'il est différent de ce poste,
     * on vérifie
        * qu'il est toujours actif
        * qu'il n'est pas prioritaire
     * si ce n'est pas le cas, on prend sa place
        * si on est prioritaire et pas lui
        * s'il n'est pas administrateur
        */
    QString ImportateurDocs       = proc->PosteImportDocs(); //le nom du poste importateur des docs externes
    if (ImportateurDocs == "Null")
    {
        if ((proc->gsettingsIni->value("BDD_LOCAL/PrioritaireGestionDocs").toString() == "YES" || proc->gsettingsIni->value("BDD_LOCAL/PrioritaireGestionDocs").toString() == "NORM")
                && proc->getModeConnexion() != Procedures::Distant)
        {
            proc->setPosteImportDocs();
            return;
        }
    }
    else
    {
        QString IpAdr = "";
        QString B = proc->gsettingsIni->value("BDD_LOCAL/PrioritaireGestionDocs").toString();
        if (B=="YES")
            IpAdr = QHostInfo::localHostName() + " - prioritaire";
        else if (B=="NORM")
            IpAdr = QHostInfo::localHostName();

        if (ImportateurDocs != IpAdr) //si le poste défini comme importateur des docs est différrent de ce poste, on vérifie qu'il est toujours actif et qu'il n'es pas prioritaire
        {
            QString req = "select nomposteconnecte from " NOM_TABLE_USERSCONNECTES " where nomposteconnecte = '" + ImportateurDocs.remove(" - prioritaire") + "'";
            QSqlQuery quer(req,db);
            if (quer.size()==0)
            {
                /*Si le poste défini comme importateur des docs externes n'est pas connecté,
                 on prend la place si
                    on n'est pas en accès distant
                    et si on est importateur
                sinon, on retire le poste*/
                proc->setPosteImportDocs((B == "YES" || B == "NORM") && proc->getModeConnexion() != Procedures::Distant);
            }
            else if (!ImportateurDocs.contains(" - " NOM_ADMINISTRATEURDOCS))
                // le poste défini comme importateur est valide mais pas administrateur, on prend sa place si
                //  on est prioritaire et pas lui
                //  à condition de ne pas être en accès distant
            {
                if (B == "YES" && !ImportateurDocs.contains(" - prioritaire") && proc->getModeConnexion() != Procedures::Distant)
                    proc->setPosteImportDocs();
                else if (ImportateurDocs.remove(" - prioritaire") == QHostInfo::localHostName()) // cas rare du poste qui a modifié son propre statut
                    proc->setPosteImportDocs((B == "YES" || B == "NORM") && proc->getModeConnexion() != Procedures::Distant);
            }
        }
    }
}

void Rufus::Slot_ActualiseDocsExternes()
{
//    if (gidUser==1)
//        Slot_eDocuments();
    /* Cette fonction sert à actualiser l'affichage des documents externes en cas de changement*/
    if (ui->tabWidget->currentWidget() == ui->tabDossier)
        OuvrirDocsExternes(gidPatient);   // depuis le timer gTimerVerifGestDocs
}

//-------------------------------------------------------------------------------------
// Interception des évènements clavier
//-------------------------------------------------------------------------------------
void Rufus::keyPressEvent (QKeyEvent * event )
{
        switch (event->key()) {
        case Qt::Key_F3:
            Refraction();
            break;
        case Qt::Key_F4:
            Tonometrie();
            break;
        case Qt::Key_F5:
            if (AutorDepartConsult(true))
            {
                if (ui->tabWidget->indexOf(ui->tabDossier) > 0)
                    ui->tabWidget->removeTab(ui->tabWidget->indexOf(ui->tabDossier));
                FermeDlgAnnexes();
                OuvrirListe(false);
            }
            break;
        case Qt::Key_F6:
            if (AutorDepartConsult(true))
            {
                if (ui->tabWidget->indexOf(ui->tabDossier) > 0)
                    ui->tabWidget->removeTab(ui->tabWidget->indexOf(ui->tabDossier));
                FermeDlgAnnexes();
                OuvrirNouveauDossier();
            }
            break;
        default:
            break;
    }
    if (ui->tabWidget->currentWidget() == ui->tabList)
    {
        if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
        {
            QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
            if ((keyEvent->modifiers() == Qt::MetaModifier))// || focusWidget() == ui->PatientsListeTableView)
            {
                if (gMode == NouveauDossier)
                {
                    gSexePat ="";
                    CreerDossier();
                }
                else
                    if (ui->PatientsListeTableView->selectionModel()->selectedIndexes().size() > 0)
                        ChoixDossier(gListePatientsModel->itemFromIndex(ui->PatientsListeTableView->selectionModel()->selectedIndexes().at(0))->text().toInt());
            }

        }
    }
}
// ------------------------------------------------------------------------------------------
// Interception des évènements internes
// ------------------------------------------------------------------------------------------

bool Rufus::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::FocusIn )
    {
        if (obj->objectName() == "ActeMontantlineEdit")         gActeMontant    = QLocale().toString(QLocale().toDouble(ui->ActeMontantlineEdit->text()),'f',2);
        if (obj->objectName() == "ActeDatedateEdit")            gActeDate       = ui->ActeDatedateEdit->text();
    }

    if (event->type() == QEvent::FocusOut )
    {
        if (obj->inherits("UpTextEdit"))
        {
            QString requetemodif;
            UpTextEdit* objUpText = static_cast<UpTextEdit*>(obj);
            objUpText->textCursor().clearSelection();
            if (objUpText->getValeurAvant() != objUpText->toHtml())
            {
                if (objUpText->getTableCorrespondant() == NOM_TABLE_ACTES)
                {
                    QString Corps = objUpText->toHtml();
                    Corps.replace("<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">","<p style=\" margin-top:0px; margin-bottom:0px;\">");
                    Corps.remove("border=\"0\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px;\" ");
                    requetemodif =   "UPDATE " + objUpText->getTableCorrespondant() + " SET " + objUpText->getChampCorrespondant() + " = '"
                            + proc->CorrigeApostrophe(Corps) + "' WHERE idActe = " + QString::number(gidActe);
                    QSqlQuery UpdateUpTextEditQuery (requetemodif,db);
                    proc->TraiteErreurRequete(UpdateUpTextEditQuery,requetemodif,tr("Impossible de mettre à jour le champ ") + objUpText->getChampCorrespondant() + "!");
                }
                else if (objUpText->getTableCorrespondant() == NOM_TABLE_MESSAGES)
                {
                    QString Corps = objUpText->toHtml();
                    Corps.replace("<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">","<p style=\" margin-top:0px; margin-bottom:0px;\">");
                    Corps.remove("border=\"0\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px;\" ");
                    requetemodif =   "UPDATE " + objUpText->getTableCorrespondant() + " SET " + objUpText->getChampCorrespondant() + " = '"
                            + proc->CorrigeApostrophe(Corps) + "' WHERE idMessage = " + QString::number(objUpText->getId());
                    QSqlQuery UpdateUpTextEditQuery (requetemodif,db);
                    proc->TraiteErreurRequete(UpdateUpTextEditQuery,requetemodif,tr("Impossible de mettre à jour le champ ") + objUpText->getChampCorrespondant() + "!");
                }
                else
                {
                    // on vérifie d'abord s'il existe un enregistrement pour ce patient dans la table correspondante, sinon, on le crée
                    QString verifrequete = "select idPat from " + objUpText->getTableCorrespondant() + " where idpat = " + QString::number(gidPatient);
                    QSqlQuery verifquery(verifrequete,db);
                    if (verifquery.size() == 0)
                        requetemodif =   "INSERT INTO " + objUpText->getTableCorrespondant() + " (" + objUpText->getChampCorrespondant() + ",idPat)"
                                + " VALUES ('" + proc->CorrigeApostrophe(objUpText->toPlainText()) + "', " + QString::number(gidPatient) + ")";
                    else
                        requetemodif =   "UPDATE " + objUpText->getTableCorrespondant() + " SET " + objUpText->getChampCorrespondant() + " = '"
                                + proc->CorrigeApostrophe(objUpText->toPlainText()) + "' WHERE idPat = " + QString::number(gidPatient);
                    QSqlQuery UpdateUpTextEditQuery (requetemodif,db);
                    proc->TraiteErreurRequete(UpdateUpTextEditQuery,requetemodif,tr("Impossible de mettre à jour le champ ") + objUpText->getChampCorrespondant() + "!");
                }
                if (objUpText == ui->ActeConclusiontextEdit || objUpText == ui->ActeMotiftextEdit || objUpText == ui->ActeTextetextEdit)
                {
                    QList<dlg_actesprecedents *> ListDialog = this->findChildren<dlg_actesprecedents *>();
                    for (int n = 0; n <  ListDialog.size(); n++)
                        if (ListDialog.at(n)->getActeAffiche() == gidActe)
                        {
                            ListDialog.at(n)->ActesPrecsAfficheActe(gidActe);
                            break;
                        }
                }
            }
        }
        else if (obj->inherits("UpLineEdit") && obj != MGlineEdit && obj != AutresCorresp1LineEdit && obj != AutresCorresp2LineEdit)
        {  
            if (obj == ui->ActeMontantlineEdit)
                // le contrôle de sortie pour ActeMontantlineEdit est traité la méthode ValideActeMontant();
                return QWidget::eventFilter(obj, event);
            QString requetemodif;
            UpLineEdit* objUpText = static_cast<UpLineEdit*>(obj);
            if (objUpText->getValeurAvant() != objUpText->text())
            {
                objUpText->setText(proc->MajusculePremiereLettre(objUpText->text(),true));
                if (objUpText->getTableCorrespondant() == NOM_TABLE_ACTES)
                {
                    requetemodif =   "UPDATE " + objUpText->getTableCorrespondant() + " SET " + objUpText->getChampCorrespondant() + " = '"
                            + proc->CorrigeApostrophe(objUpText->text()) + "' WHERE idActe = " + QString::number(gidPatient);
                    QSqlQuery UpdateUpTextEditQuery (requetemodif,db);
                    proc->TraiteErreurRequete(UpdateUpTextEditQuery,requetemodif,tr("Impossible de mettre à jour le champ ") + objUpText->getChampCorrespondant() + "!");
                }
                else
                {
                    objUpText->setText(proc->MajusculePremiereLettre(objUpText->text(),true));
                    // on vérifie d'abord s'il existe un enregistrement pour ce patient dans la table correspondante, sinon, on le crée
                    QString verifrequete = "select idPat from " + objUpText->getTableCorrespondant() + " where idpat = " + QString::number(gidPatient);
                    QSqlQuery verifquery(verifrequete,db);
                    if (verifquery.size() == 0)
                    {
                        requetemodif =   "INSERT INTO " + objUpText->getTableCorrespondant() + " (" + objUpText->getChampCorrespondant() + ",idPat)"
                                + " VALUES ('" + proc->CorrigeApostrophe(objUpText->text()) + "', " + QString::number(gidPatient) + ")";
                        QSqlQuery UpdateUpTextEditQuery (requetemodif,db);
                        proc->TraiteErreurRequete(UpdateUpTextEditQuery,requetemodif,tr("Impossible de mettre à jour le champ ") + objUpText->getChampCorrespondant() + "!");
                    }
                    else
                    {
                        requetemodif =   "UPDATE " + objUpText->getTableCorrespondant() + " SET " + objUpText->getChampCorrespondant() + " = '"
                                + proc->CorrigeApostrophe(objUpText->text()) + "' WHERE idPat = " + QString::number(gidPatient);
                        QSqlQuery UpdateUpTextEditQuery (requetemodif,db);
                        proc->TraiteErreurRequete(UpdateUpTextEditQuery,requetemodif,tr("Impossible de mettre à jour le champ ") + objUpText->getChampCorrespondant() + "!");
                    }
                    Slot_OKModifierTerrain();
                }
            }
        }
        else if (obj->objectName() == "ActeDatedateEdit")
        {
            if (ui->ActeDatedateEdit->text() != gActeDate)
            {
                QString requete =   "UPDATE " NOM_TABLE_ACTES " SET ActeDate = '" + ui->ActeDatedateEdit->date().toString("yyyy-MM-dd") + "' WHERE idActe = " + QString::number(gidActe);
                QSqlQuery UpdateUpTextEditQuery (requete,db);
                if (proc->TraiteErreurRequete(UpdateUpTextEditQuery,requete,tr("Impossible de mettre à jour la date de l'acte!")))
                    ui->ActeDatedateEdit->setDate(QDate::fromString(gActeDate,"dd/MM/yyyy"));
                else
                {
                    gActeDate       = ui->ActeDatedateEdit->text();
                    MAJActesPrecs();
                }
            }
            ui->ActeDatedateEdit->setEnabled(false);
        }
        else if (obj == ui->MGupComboBox || obj != ui->AutresCorresp1upComboBox || obj != ui->AutresCorresp2upComboBox)
            MAJMG(obj);
    }

    if(event->type() == QEvent::MouseButtonPress)
        if (obj == ui->FermepushButton  || obj == ui->LFermepushButton)
        {
            QPushButton* Button = static_cast<QPushButton*>(obj);
            Button->setIcon(proc->giconFermeAppuye);
        }

    if(event->type() == QEvent::MouseMove)
        if (obj == ui->FermepushButton  || obj == ui->LFermepushButton)
        {
            QPushButton* Button = static_cast<QPushButton*>(obj);
            QRect rect = QRect(Button->pos(),Button->size());
            QPoint pos = mapFromParent(cursor().pos());
            if (rect.contains(pos))
                Button->setIcon(proc->giconFermeAppuye);
            else
                Button->setIcon(proc->giconFermeRelache);
        }

    if(event->type() == QEvent::MouseButtonRelease)
    {
        if (obj == ui->FermepushButton  || obj == ui->LFermepushButton)
        {
            QPushButton* Button = static_cast<QPushButton*>(obj);
            Button->setIcon(proc->giconFermeRelache);
        }
        if (obj == ui->EnregistrePaiementpushButton && ui->EnregistrePaiementpushButton->isEnabled())
        {
            emit EnregistrePaiement("Bouton");
        }
    }

    // GESTION DU CLAVIER _____________________________________________________________________________________________________________________________________________________
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
    // RETURN _________________________________________________________________________________________________________________________________________________________________
        // Return ou Enter - On va au Tab Suivant -----------------------------------------------------------------------------------------------------------------------------
        if ((keyEvent->key()==Qt::Key_Return  && keyEvent->modifiers() == Qt::NoModifier) || keyEvent->key() == Qt::Key_Enter)
        {
            if (obj == ui->TabaclineEdit
                 || obj == MGlineEdit                   || obj == ui->MGupComboBox
                 || obj == AutresCorresp1LineEdit       || obj == AutresCorresp2LineEdit
                 || obj == ui->AutresCorresp1upComboBox || obj == ui->AutresCorresp2upComboBox
                 || obj == ui->ActeDatedateEdit         || obj == ui->CourrierAFairecheckBox
                 || obj == ui->ActeCotationcomboBox     || obj == ui->ActeMontantlineEdit
                 || obj == ui->CreerNomlineEdit         || obj == ui->CreerPrenomlineEdit
                    || obj == ui->PatientsListeTableView
               )
                return QWidget::focusNextChild();

            if (obj == ui->CreerDDNdateEdit)
            {
                QKeyEvent *newevent = new QKeyEvent ( QEvent::KeyPress, Qt::Key_Tab , Qt::NoModifier);
                QCoreApplication::postEvent (obj, newevent);
                return QWidget::eventFilter(obj, newevent);
            }
            if (obj->inherits("UpTextEdit"))
            {
                UpTextEdit* objUpText = static_cast<UpTextEdit*>(obj);
                if (objUpText->getTableCorrespondant() == NOM_TABLE_MESSAGES)
                {
                    int nlines = objUpText->document()->lineCount()+1;
                    int a = QFontMetrics(qApp->font()).height()*1.4;
                    objUpText->setFixedHeight(a*nlines);
                    objUpText->layout()->update();
                    //gMsgDialog->layout()->update();
                }
            }
        }
        // Shift-Return ou Shift-Enter - On va au Tab précédent --------------------------------------------------------------------------------------------------------------------
        if (keyEvent->key()==Qt::Key_Return && keyEvent->modifiers() == Qt::ShiftModifier )
        {
           if (obj == ui->TabaclineEdit
                || obj == MGlineEdit                    || obj == ui->MGupComboBox
                || obj == ui->AutresCorresp1upComboBox  || obj == ui->AutresCorresp2upComboBox
                || obj == ui->ActeDatedateEdit          || obj == ui->CourrierAFairecheckBox
                || obj == ui->ActeCotationcomboBox      || obj == ui->ActeMontantlineEdit
              )
                return QWidget::focusPreviousChild();
        }

        // Cas particulier de CreerNomlineEdit et CreerPrenomlineEdit ---------------------------------------------------------------------------------------------------------------
        if (obj == ui->CreerNomlineEdit || obj == ui->CreerPrenomlineEdit)
        {
            QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
            QLineEdit* Qobj = static_cast<QLineEdit*>(obj);
            switch (keyEvent->key())
            {
            case Qt::Key_Delete:
                Qobj->clear();;
                ChercheNomFiltre();
                return true;
                break;
            case Qt::Key_Up:
                MonteUneLigne();
                return true;
                break;
            case Qt::Key_Down:
                DescendUneLigne();
                return true;
                break;
            case Qt::Key_PageDown :
                Descend20Lignes();
                return true;
                break;
            case Qt::Key_PageUp :
                Monte20Lignes();
                return true;
                break;
            default:
                break;
            }
        }
        // LES FLECHES ______________________________________________________________________________________________________________________________________________________________
        // Flèche bas - variable suivant les cas ------------------------------------------------------------------------------------------------------------------------------------
        if (keyEvent->key()==Qt::Key_Down)
        {
            if (obj == ui->TabaclineEdit)           MGlineEdit->setFocus();
            if (obj == MGlineEdit)                  AutresCorresp1LineEdit->setFocus();
            if (obj == AutresCorresp1LineEdit)      AutresCorresp2LineEdit->setFocus();
        }

        // Flèche haut - variable suivant les cas ------------------------------------------------------------------------------------------------------------------------------------
        if (keyEvent->key()==Qt::Key_Up)
        {
            if (obj == ui->TabaclineEdit)               ui->AutresToxiquestextEdit->setFocus();
            if (obj == MGlineEdit)                      ui->TabaclineEdit->setFocus();
            if (obj == AutresCorresp1LineEdit)          MGlineEdit->setFocus();
            if (obj == AutresCorresp2LineEdit)          AutresCorresp1LineEdit->setFocus();
        }

        // Flèche droit - variable suivant les cas ------------------------------------------------------------------------------------------------------------------------------------
        if (keyEvent->key() == Qt::Key_Right)
        {
            if (obj == ui->TabaclineEdit)           ui->AtcdtsOphstextEdit->setFocus();
            if (obj == ui->AutresToxiquestextEdit)  ui->TabaclineEdit->setFocus();
            if (obj == MGlineEdit)                  AutresCorresp1LineEdit->setFocus();
            if (obj == AutresCorresp1LineEdit)      AutresCorresp2LineEdit->setFocus();
            if (obj == ui->MGupComboBox)            ui->AutresCorresp1upComboBox->setFocus();
            if (obj == ui->AutresCorresp1upComboBox)ui->AutresCorresp2upComboBox->setFocus();
        }
        if (keyEvent->key() == Qt::Key_Right && keyEvent->modifiers() == Qt::MetaModifier)
        {
            if (obj == ui->AtcdtsPersostextEdit)                                                                    return QWidget::focusNextChild();
        }

        // Flèche gauche - variable suivant les cas -----------------------------------------------------------------------------------------------------------------------------------
        if (keyEvent->key() == Qt::Key_Left)
        {
            if (obj == ui->TabaclineEdit)               ui->AutresToxiquestextEdit->setFocus();
            if (obj == MGlineEdit)                      ui->TabaclineEdit->setFocus();
            if (obj == AutresCorresp1LineEdit)          MGlineEdit->setFocus();
            if (obj == AutresCorresp2LineEdit)          AutresCorresp1LineEdit->setFocus();
            if (obj == ui->ActeMontantlineEdit  || obj == ui->ActeCotationcomboBox)
                                                        ui->ActeTextetextEdit->setFocus();
        }
     }
    return QWidget::eventFilter(obj, event);
}

/*------------------------------------------------------------------------------------------------------------------------------------
-- Afficher la consultation d'un patient à partir de son idActe ----------------------------------------------------------------------
------------------------------------------------------------------------------------------------------------------------------------*/
void Rufus::AfficheActe(int idActe)
{
    QString req = "SELECT idActe, idPat, idUser, ActeDate, ActeMotif, ActeTexte, ActeConclusion, ActeCourrierAFaire, ActeCotation, ActeMontant, ActeMonnaie, CreePar from " NOM_TABLE_ACTES
            " WHERE idActe = '" + QString::number(idActe) + "'";
    QSqlQuery AfficheActeQuery (req,db);
    if (proc->TraiteErreurRequete(AfficheActeQuery,req,tr("Impossible de retrouver la dernière consultation"))
        || AfficheActeQuery.size() == 0)                            // Aucune consultation trouvee pour ce  patient
    {
        nbActes = 0;
        noActe = 0;
        ui->Acteframe->setVisible(false);
        ui->CreerActepushButton_2->setVisible(true);
        ui->CreerBOpushButton_2->setVisible(true);
        ui->idActelineEdit->clear();
        return;
    }
    else
    {
        gidActe = idActe;
        //1.  Retrouver l'acte défini par son idActe et afficher les champs
        AfficheActeQuery.last();
        ui->Acteframe->setVisible(true);
        ui->CreerActepushButton_2->setVisible(false);
        ui->CreerBOpushButton_2->setVisible(false);

        ui->ActeDatedateEdit->setDate(AfficheActeQuery.value(3).toDate());
        ui->ActeDatedateEdit->setEnabled(false);

        ui->ActeMotiftextEdit->setText(AfficheActeQuery.value(4).toString());
        ui->ActeTextetextEdit->setText(AfficheActeQuery.value(5).toString());
        ui->ActeConclusiontextEdit->setText(AfficheActeQuery.value(6).toString());
        ui->idActelineEdit->setText(AfficheActeQuery.value(0).toString());
        ui->CourrierAFairecheckBox->setChecked(AfficheActeQuery.value(7).toString() == "T");
        ui->ActeCotationcomboBox->setCurrentText(AfficheActeQuery.value(8).toString());
        // on affiche tous les montants en euros, même ce qui a été payé en francs.
        gMonnaie  = AfficheActeQuery.value(10).toString();
        double H = 1;
        if (gMonnaie == "F")
        {
            ui->ActeMontantLabel->setText("Montant (€)\n(payé en F)");
            H = 6.55957;
        }
        else
            ui->ActeMontantLabel->setText("Montant (€)");
        ui->ActeMontantlineEdit->setText(QLocale().toString(AfficheActeQuery.value(9).toDouble()/H,'f',2));
        int idx = ui->ActeCotationcomboBox->findText(AfficheActeQuery.value(8).toString());
        if (idx>0)
        {
            QStringList listMontantActe = ui->ActeCotationcomboBox->itemData(idx).toStringList();
            double MontantConv, MontantPrat, MontantActe;
            MontantActe = AfficheActeQuery.value(9).toDouble()/H;
            MontantConv = listMontantActe.at(0).toDouble();
            MontantPrat = listMontantActe.at(1).toDouble();
            ui->BasculerMontantpushButton->setVisible((MontantActe!=MontantConv) || (MontantActe!=MontantPrat) || (MontantConv!=MontantPrat));
            if (ui->BasculerMontantpushButton->isVisible())
            {
                if (MontantActe!=MontantPrat)
                    ui->BasculerMontantpushButton->setImmediateToolTip("Revenir au tarif habituellement pratiqué");
                else
                    ui->BasculerMontantpushButton->setImmediateToolTip("Revenir au tarif conventionnel");
            }
        }

        ui->CreerActepushButton->setToolTip(tr("Créer un nouvel acte pour ") + gPrenomPatient + " " + gNomPatient);
        ui->CreerBOpushButton->setToolTip(tr("Créer un bilan orthoptique pour ") + gPrenomPatient + " " + gNomPatient);

        QMap<QString,QVariant>  Age = proc->CalculAge(gDDNPatient, ui->ActeDatedateEdit->date());
        ui->AgelineEdit->setText(Age["Total"].toString());
        ui->AgelineEdit->setAlignment(Qt::AlignCenter);
        gAgePatient = Age["Annee"].toInt();

        //2. retrouver le créateur de l'acte et le médecin superviseur de l'acte
        ui->CreeParlineEdit->setText(tr("Créé par ") + proc->getLogin(AfficheActeQuery.value(11).toInt())
                                     + tr(" pour ") + proc->getLogin(AfficheActeQuery.value(2).toInt()));

        //3. Mettre à jour le numéro d'acte
        req = "SELECT idActe FROM " NOM_TABLE_ACTES
                " WHERE idPat = '" + QString::number(gidPatient) + "' ORDER BY ActeDate";
        QSqlQuery ChercheNumActeQuery (req,db);
        if (!proc->TraiteErreurRequete(ChercheNumActeQuery,req,tr("Impossible de retrouver les consultations de ce patient")))
        {
            if (ChercheNumActeQuery.size() > 0)           // Il y a des consultations
            {
                nbActes = ChercheNumActeQuery.size();
                if (nbActes == 1)
                {
                    ui->ActePrecedentpushButton->setEnabled(false);
                    ui->ActeSuivantpushButton->setEnabled(false);
                    ui->PremierActepushButton->setEnabled(false);
                    ui->DernierActepushButton->setEnabled(false);
                    noActe = 1;
                    ui->OuvreActesPrecspushButton->setEnabled(false);
                    ui->OuvreActesPrecspushButton->setToolTip("");
                }
                if (nbActes > 1)
                {
                    ChercheNumActeQuery.first();
                    for (int i = 0; i < nbActes; i++)
                    {
                        if (ChercheNumActeQuery.value(0).toInt() == idActe)
                        {
                            noActe = ChercheNumActeQuery.at() + 1;
                            break;
                        }
                        else
                            ChercheNumActeQuery.next();
                    }
                    if (noActe == 1)
                    {
                        ui->ActePrecedentpushButton->setEnabled(false);
                        ui->PremierActepushButton->setEnabled(false);
                        ui->ActeSuivantpushButton->setEnabled(true);
                        ui->DernierActepushButton->setEnabled(true);
                    }
                    else
                    {
                        if (noActe == nbActes)
                        {
                            ui->ActePrecedentpushButton->setEnabled(true);
                            ui->ActeSuivantpushButton->setEnabled(false);
                            ui->PremierActepushButton->setEnabled(true);
                            ui->DernierActepushButton->setEnabled(false);
                        }
                        else
                        {
                            ui->ActePrecedentpushButton->setEnabled(true);
                            ui->ActeSuivantpushButton->setEnabled(true);
                            ui->PremierActepushButton->setEnabled(true);
                            ui->DernierActepushButton->setEnabled(true);
                        }
                    }
                    ui->OuvreActesPrecspushButton->setEnabled(true);
                    ui->OuvreActesPrecspushButton->setToolTip(tr("Voir les consultations précédentes de ") + gPrenomPatient + " " + gNomPatient);
                }
                ui->NoActelabel->setText(QString::number(noActe) + " / " + QString::number(nbActes));
            }
        }
    }
    gAutorModifConsult = false;
    AfficheActeCompta();
}

/*------------------------------------------------------------------------------------------------------------------------------------
-- Afficher les renseignements comptables de l'acte en cours -------------------------------------------------------------------------
------------------------------------------------------------------------------------------------------------------------------------*/
void Rufus::AfficheActeCompta()
{
    //1. on recherche d'abord s'il y a eu un paiement enregistré pour cette consultation
    QString req = "SELECT TypePaiement, Tiers From " NOM_TABLE_TYPEPAIEMENTACTES " WHERE idActe = " + QString::number(gidActe);
    QSqlQuery AfficheTypePaiementQuery (req,db);
    proc->TraiteErreurRequete(AfficheTypePaiementQuery,req,"Impossible de retrouver les renseignements comptables");

    bool a = (AfficheTypePaiementQuery.size() == 0);

    ui->Comptaframe->setVisible(!a);
    ui->Cotationframe->setEnabled(a);
    ui->CCAMlinklabel->setVisible(a);
    ui->EnregistrePaiementpushButton->setVisible(a && (soignant && !assistant));
    ui->ModifierCotationActepushButton->setVisible(!a);
    if (a)
    {
        ui->EnregistrePaiementpushButton->setEnabled(ui->ActeCotationcomboBox->lineEdit()->text()!="");
        return;
    }

    //2. on recherche ensuite le type de paiement : espèces, chèque, tiers, cb, impayé, gratuit

    AfficheTypePaiementQuery.first();
    if (AfficheTypePaiementQuery.value(0).toString() == "T"  && AfficheTypePaiementQuery.value(1).toString() != "CB") ui->PaiementlineEdit->setText(AfficheTypePaiementQuery.value(1).toString());

    QString lbl = "Paiement : ";
    if (AfficheTypePaiementQuery.value(0).toString() == "C") ui->PaiementlineEdit->setText("chèque");
    if (AfficheTypePaiementQuery.value(0).toString() == "E") ui->PaiementlineEdit->setText("espèces");
    if (AfficheTypePaiementQuery.value(0).toString() == "B") ui->PaiementlineEdit->setText("carte de crédit");
    if (AfficheTypePaiementQuery.value(0).toString() == "G") ui->PaiementlineEdit->setText("Acte gratuit");
    if (AfficheTypePaiementQuery.value(0).toString() == "I") ui->PaiementlineEdit->setText("Impayé");
    if (AfficheTypePaiementQuery.value(0).toString() == "T"
            && AfficheTypePaiementQuery.value(1).toString() == "CB") ui->PaiementlineEdit->setText("carte de crédit");
    if (AfficheTypePaiementQuery.value(0).toString() == "T"
            && AfficheTypePaiementQuery.value(1).toString() != "CB") ui->PaiementlineEdit->setText(AfficheTypePaiementQuery.value(1).toString());

    // on calcule le montant payé pour l'acte
    if (AfficheTypePaiementQuery.value(0).toString() != "G" || AfficheTypePaiementQuery.value(0).toString() != "I")
    {
        double TotalPaye = 0;
        // on récupère les lignes de paiement
        QString requete = " SELECT idRecette, Paye FROM " NOM_TABLE_LIGNESPAIEMENTS " WHERE idActe = " + QString::number(gidActe);
        QSqlQuery ListePaiementsQuery (requete,db);
        proc->TraiteErreurRequete(ListePaiementsQuery, requete, "");
        ListePaiementsQuery.first();
        for (int l = 0; l < ListePaiementsQuery.size(); l++)
        {
            requete = "SELECT Monnaie FROM " NOM_TABLE_RECETTES " WHERE idRecette = " + ListePaiementsQuery.value(0).toString();
            QSqlQuery MonnaieQuery (requete,db);
            proc->TraiteErreurRequete(MonnaieQuery,requete,"");
            MonnaieQuery.first();
            if (MonnaieQuery.value(0).toString() == "F")
                TotalPaye = TotalPaye + (ListePaiementsQuery.value(1).toDouble() / 6.55957);
            else
                TotalPaye = TotalPaye + ListePaiementsQuery.value(1).toDouble();
            ListePaiementsQuery.next();
        }
        ui->PayelineEdit->setText(QLocale().toString(TotalPaye,'f',2));
    }

    if (AfficheTypePaiementQuery.value(0).toString() == "B"
        ||(AfficheTypePaiementQuery.value(0).toString() == "T"
            && AfficheTypePaiementQuery.value(1).toString() == "CB")
        || AfficheTypePaiementQuery.value(0).toString() == "C"
        || AfficheTypePaiementQuery.value(0).toString() == "E")
    {
        ui->PaiementLabel->setVisible(true);
        ui->PaiementLabel->setText("Paiement:");
        ui->PayeLabel->setVisible(true);
        ui->PayelineEdit->setVisible(true);
        ui->Comptaframe->setGeometry(580,510,180,50);
        ui->PaiementlineEdit->setGeometry(79,4,91,18);
        return;
    }
    if (AfficheTypePaiementQuery.value(0).toString() == "G"
        || AfficheTypePaiementQuery.value(0).toString() == "I")
    {
        ui->PaiementLabel->setVisible(false);
        ui->PayeLabel->setVisible(false);
        ui->PayelineEdit->setVisible(false);
        ui->Comptaframe->setGeometry(580,510,180,26);
        ui->PaiementlineEdit->setGeometry(8,4,164,18);
        return;
    }
    if (AfficheTypePaiementQuery.value(0).toString() == "T"
            && AfficheTypePaiementQuery.value(1).toString() != "CB")
    {
        ui->PaiementLabel->setText("Tiers");
        ui->PaiementLabel->setVisible(true);
        ui->PaiementlineEdit->setVisible(true);
        ui->PayeLabel->setVisible(true);
        ui->PayelineEdit->setVisible(true);
        ui->Comptaframe->setGeometry(580,510,180,50);
        ui->PaiementlineEdit->setGeometry(79,4,91,18);
        return;
    }
}

/*------------------------------------------------------------------------------------------------------------------------------------
-- Afficher le dossier d'un patient à partir de son idPat ----------------------------------------------------------------------------
------------------------------------------------------------------------------------------------------------------------------------*/
void Rufus::AfficheDossier(int idPat)
{
//  Afficher les éléments de la tables Patients

    QString     Msg;

    QString req = "SELECT idPat, PatNom, PatPrenom, PatDDN, Sexe, PatCreele, PatCreePar FROM " NOM_TABLE_PATIENTS
              " WHERE idPat = '" + QString::number(idPat) + "'";
    QSqlQuery AfficheDossierQuery (req,db);
    if (proc->TraiteErreurRequete(AfficheDossierQuery,req,tr("Impossible de retrouver le dossier de ce patient")))
        return;
    if (AfficheDossierQuery.size() == 0)           // Aucune mesure trouvee pour ces criteres
        return;
    AfficheDossierQuery.first();
    gidPatient      = idPat;
    gdossierAOuvrir = idPat;
    gNomPatient     = AfficheDossierQuery.value(1).toString();
    gPrenomPatient  = AfficheDossierQuery.value(2).toString();
    gDDNPatient     = AfficheDossierQuery.value(3).toDate();
    gSexePat = AfficheDossierQuery.value(4).toString();
    ui->DateCreationDossierlineEdit->setText(AfficheDossierQuery.value(5).toDate().toString(tr("d-M-yyyy")));
    ui->idPatientlineEdit->setText(QString::number(idPat));
    QString label= gNomPatient + " " + gPrenomPatient;

    ui->IdCreateurDossierlineEdit->setText(proc->getLogin(AfficheDossierQuery.value(6).toInt()));

    //2 - récupération des données sociales

    req = "SELECT idPat, PatAdresse1, PatAdresse2, PatAdresse3, PatCodePostal, PatVille, PatTelephone, PatPortable, PatMail, PatNNI, PatALD, PatProfession, PatCMU FROM " NOM_TABLE_DONNEESSOCIALESPATIENTS
              " WHERE idPat = '" + QString::number(idPat) + "'";
    QSqlQuery DonneesSocialesQuery (req,db);
    if (!proc->TraiteErreurRequete(DonneesSocialesQuery,req,tr("Impossible de retrouver les données sociales!")))
    {
        if (DonneesSocialesQuery.size() > 0)
            DonneesSocialesQuery.first();
        else
        {
            // ui->NNIlineEdit->clear();            // CZ001
            //ui->NNIlineEdit->setText(gNNIPat);      // CZ001
        }
    }
    QString html, img, Age;
    QMap<QString,QVariant>  AgeTotal = proc->CalculAge(gDDNPatient, QDate::currentDate(),gSexePat);
    gCMUPatient = (DonneesSocialesQuery.value(12).toInt() == 1);
    img = AgeTotal["Icone"].toString();
    Age = AgeTotal["Total"].toString();
    QIcon icon = proc->CalcIconAge(img);

    html =
    "<html>"
    "<head>"
      "<style type=\"text/css\">"
      "  p.p1 {font:16px; margin: 0px 0px 10px 100px;}"
      "  p.p10 {font:16px; margin: 10px 0px 0px 100px;}"
      "  p.p2 {margin: 0px 0px 0px 100px;}"
      "  p.p3 {margin: 10px 0px 0px 100px;}"
      "  p.p4 {margin: 10px 0px 10px 100px;}"
      "  .image {position:absolute; top:100px; left:10px; float: left;}"
      "</style>"
    "</head>"
    "<body LANG=\"fr-FR\" DIR=\"LTR\">";
    if (img != "")
        html += "<img class=\"image\" src=\"://" + img + ".png\" WIDTH=\"100\" HEIGHT=\"100\" BORDER=\"10\" />";                            //Icone
    html += "<p class=\"p10\"><b>" + gNomPatient + " " + gPrenomPatient + "</b></p>";                                                       //Nom Prenom
    html += "<p class=\"p1\"><b>" + Age + "</b> (" +gDDNPatient.toString(tr("d MMM yyyy")) + ")</p>";                                           //DDN
    if (DonneesSocialesQuery.value(1).toString() != "")
        html += "<p class=\"p2\">" + DonneesSocialesQuery.value(1).toString() + "</p>";                                                     //Adresse1
    if (DonneesSocialesQuery.value(2).toString() != "")
        html += "<p class=\"p2\">" + DonneesSocialesQuery.value(2).toString() + "</p>";                                                     //Adresse2
    if (DonneesSocialesQuery.value(3).toString() != "")
        html += "<p class=\"p2\">" + DonneesSocialesQuery.value(3).toString() + "</p>";                                                     //Adresse3
    if (DonneesSocialesQuery.value(4).toString() != "")
    {
        html += "<p class=\"p2\">" + DonneesSocialesQuery.value(4).toString() + " " + DonneesSocialesQuery.value(5).toString() + "</p>";    //CP + ville
    }
    else
        if (DonneesSocialesQuery.value(5).toString() != "")
            html += "<p class=\"p2\">" + DonneesSocialesQuery.value(5).toString() + "</p>";                                                 //Ville
    if (DonneesSocialesQuery.value(6).toString() != "")
        html += "<p class=\"p3\">"+ tr("Tél.") + "\t" + DonneesSocialesQuery.value(6).toString() + "</p>";                                               //Tél
    if (DonneesSocialesQuery.value(7).toString() != "")
        html += "<p class=\"p2\">" + tr("Portable") + "\t" + DonneesSocialesQuery.value(7).toString() + "</p>";                                           //Portable
    if (DonneesSocialesQuery.value(8).toString() != "")
        html += "<p class=\"p3\">" + tr("Mail") + "\t" + DonneesSocialesQuery.value(8).toString() + "</p>";                                               //Mail
    if (DonneesSocialesQuery.value(9).toInt() > 0)
        html += "<p class=\"p2\">" + tr("NNI") + "\t" + DonneesSocialesQuery.value(9).toString() + "</p>";                                                //NNI
    if (DonneesSocialesQuery.value(11).toString() != "")
        html += "<p class=\"p3\">" + DonneesSocialesQuery.value(11).toString() + "</p>";                                                    //Profession
    if (DonneesSocialesQuery.value(10).toInt() == 1 || DonneesSocialesQuery.value(12).toInt() == 1)
    {
        html += "<p class=\"p3\"><td width=\"60\">";
        if (DonneesSocialesQuery.value(10).toInt() == 1)
            html += "<font size = \"5\"><font color = \"red\"><b>ALD</b></font>";                                                           //ALD
        if (DonneesSocialesQuery.value(12).toInt() == 1)
            html += "</td><td width=\"60\"><font size = \"5\"><font color = \"blue\"><b>CMU</b></font>";                                    //CMU
        html += "</td></p>";
    }

    html += "</body></html>";
    //proc->EcritDansUnFichier(QDir::homePath()+ NOMFIC_TEST, html);

    ui->IdentPatienttextEdit->setHtml(html);

    //3 - récupération des données médicales

    req = "SELECT idPat, idCorMedMG, idCorMedSpe1, idCorMedSpe2, idCorMedSpe3, idCorNonMed, RMPAtcdtsPersos, RMPTtGeneral, RMPAtcdtsFamiliaux"
              ", RMPAtcdtsOphs, Tabac, Autrestoxiques, Gencorresp, Important, Resume, RMPTtOphs FROM " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS
              " WHERE idPat = " + QString::number(idPat);
    //qDebug() << requete;
    QSqlQuery DonneesMedicalesQuery (req,db);
    if (!proc->TraiteErreurRequete(DonneesMedicalesQuery,req,"Impossible de retrouver les données médicales"))
    {
        if (DonneesMedicalesQuery.size() > 0)           // Il y a des renseignements medicaux
        {
            DonneesMedicalesQuery.first();
            ui->AtcdtsPersostextEdit->setText(DonneesMedicalesQuery.value(6).toString());
            ui->TtGeneraltextEdit->setText(DonneesMedicalesQuery.value(7).toString());
            ui->AtcdtsFamiliauxtextEdit->setText(DonneesMedicalesQuery.value(8).toString());
            ui->AtcdtsOphstextEdit->setText(DonneesMedicalesQuery.value(9).toString());
            ui->TabaclineEdit->setText(DonneesMedicalesQuery.value(10).toString());
            ui->AutresToxiquestextEdit->setText(DonneesMedicalesQuery.value(11).toString());
            QString tooltp = "";
            if (DonneesMedicalesQuery.value(1).toInt()>0)
            {
                int id = DonneesMedicalesQuery.value(1).toInt();
                ui->MGupComboBox->setCurrentIndex
                        (ui->MGupComboBox->findData(id));
                tooltp = CalcToolTipCorrespondant(id);
            }
            else
                ui->MGupComboBox->setCurrentIndex(-1);
            ui->MGupComboBox->setImmediateToolTip(tooltp);
            tooltp = "";
            if (DonneesMedicalesQuery.value(2).toInt()>0)
            {
                int id = DonneesMedicalesQuery.value(2).toInt();
                ui->AutresCorresp1upComboBox->setCurrentIndex
                        (ui->AutresCorresp1upComboBox->findData(id));
                tooltp = CalcToolTipCorrespondant(id);
            }
            else
                ui->AutresCorresp1upComboBox->setCurrentIndex(-1);
            ui->AutresCorresp1upComboBox->setImmediateToolTip(tooltp);
            tooltp = "";
            if (DonneesMedicalesQuery.value(3).toInt()>0)
            {
                int id = DonneesMedicalesQuery.value(3).toInt();
                ui->AutresCorresp2upComboBox->setCurrentIndex
                        (ui->AutresCorresp2upComboBox->findData(id));
                tooltp = CalcToolTipCorrespondant(id);
            }
            else
                ui->AutresCorresp2upComboBox->setCurrentIndex(-1);
            ui->AutresCorresp2upComboBox->setImmediateToolTip(tooltp);
            ui->ImportanttextEdit->setText(DonneesMedicalesQuery.value(13).toString());
            ui->ResumetextEdit->setText(DonneesMedicalesQuery.value(14).toString());
            ui->TtOphtextEdit->setText(DonneesMedicalesQuery.value(15).toString());
        }
        else
        {
            ui->AtcdtsPersostextEdit->clear();
            ui->TtGeneraltextEdit->clear();
            ui->AtcdtsFamiliauxtextEdit->clear();
            ui->AtcdtsOphstextEdit->clear();
            ui->TabaclineEdit->clear();
            ui->AutresToxiquestextEdit->clear();
            MGlineEdit->clear();
            AutresCorresp1LineEdit->clear();
            AutresCorresp2LineEdit->clear();
            ui->ImportanttextEdit->clear();
            ui->ResumetextEdit->clear();
            ui->TtOphtextEdit->clear();
        }
        Slot_OKModifierTerrain();
    }
    FermeDlgAnnexes();

    //3 - récupération des actes

    QString recuprequete = "SELECT idActe FROM " NOM_TABLE_ACTES
              " WHERE idPat = '" + QString::number(idPat) + "' ORDER BY ActeDate";
    QSqlQuery ActesQuery (recuprequete,db);
    if (proc->TraiteErreurRequete(ActesQuery,recuprequete,tr("Impossible de retrouver les consultations de ce patient")))
    {
        ui->Acteframe->setVisible(false);
        ui->CreerActepushButton_2->setVisible(true);
        ui->CreerBOpushButton_2->setVisible(true);
        ui->idActelineEdit->clear();
        gidActe = 0;
        return;
    }
        else
    {
        OuvrirDocsExternes(gidPatient);  // depuis AfficheDossier
        if (ActesQuery.size() > 0)           // Il y a des consultations
        {
            ActesQuery.last();
            int a = ActesQuery.value(0).toInt();
            AfficheActe(a);
            if (ActesQuery.size() > 1)
                OuvrirActesPrecedents(a);
            ui->ActeMotiftextEdit->setFocus();
        }
        else
        {
            gidActe = 0;
            ui->Acteframe->setVisible(false);
            ui->CreerActepushButton_2->setVisible(true);
            ui->CreerBOpushButton_2->setVisible(true);
            ui->idActelineEdit->clear();
        }
    }
    //4 - réglage du refracteur
    if (proc->PortRefracteur()!=NULL)
    {
        gMesureFronto.clear();
        gMesureAutoref.clear();
        RegleRefracteur("P");
        RegleRefracteur("R");
        proc->SetDataAEnvoyerAuRefracteur(gMesureFronto, gMesureAutoref);
    }

    //5 - mise à jour du dossier en salle d'attente
    req =   "SELECT idPat FROM " NOM_TABLE_SALLEDATTENTE
                " WHERE idPat = " + QString::number(gidPatient);
    QSqlQuery SalDatQuery (req,db);
    proc->TraiteErreurRequete(SalDatQuery,req,tr("Impossible de trouver la salle d'attente!"));

    if (SalDatQuery.size() == 0)
    {
        req =   "INSERT INTO " NOM_TABLE_SALLEDATTENTE
                    " (idPat, idUser, Statut, HeureStatut, idUserEnCoursExam, PosteExamen, HeureArrivee)"
                    " VALUES ('" + QString::number(gidPatient) + "','" + QString::number(gidUserSuperviseur) + "','" ENCOURSEXAMEN + gUserLogin + "','" + QTime::currentTime().toString("hh:mm")
                    + "'," + QString::number(gidUser) + ", '" + QHostInfo::localHostName().left(60) + "','" + QTime::currentTime().toString("hh:mm") +"')";
        Msg = tr("Impossible de mettre ce dossier en salle d'attente");
    }
    else
    {
        req =   "UPDATE " NOM_TABLE_SALLEDATTENTE
                    " SET Statut = '" ENCOURSEXAMEN + gUserLogin +
                    "', HeureStatut = '" + QTime::currentTime().toString("hh:mm") +
                    "', idUserEnCoursExam = " + QString::number(gidUser) +
                    ", PosteExamen = '" + QHostInfo::localHostName().left(60) +
                    "' WHERE idPat = '" + QString::number(gidPatient) + "'";
        Msg = tr("Impossible de modifier le statut du dossier en salle d'attente!");
    }
    //UpMessageBox::Watch(this,req);
    QSqlQuery ModifSalDatQuery (req,db);
    proc->TraiteErreurRequete(ModifSalDatQuery,req,Msg);

    if (!ui->tabDossier->isVisible())
    {
        ui->tabWidget->insertTab(1,ui->tabDossier,label);
        ui->tabWidget->setCurrentIndex(ui->tabWidget->indexOf(ui->tabDossier));
    }
    else
        ui->tabWidget->setTabText(ui->tabWidget->indexOf(ui->tabDossier) ,label);

    ui->tabWidget->setTabIcon(ui->tabWidget->indexOf(ui->tabDossier),icon);

    ui->AtcdtsPersostextEdit->setFocus();
    RecaleTableView(gidPatient);
    CalcMotsCles(idPat);
    proc->UpdVerrouSalDat();


    if (gidUser > 1) return;
    QString Sexe = "";
    req ="select idpat from " NOM_TABLE_PATIENTS " where patPrenom = '" + gPrenomPatient + "' and sexe = '' and patPrenom <> 'Dominique' and patPrenom <> 'Claude'";
    QSqlQuery quer(req,db);
    if (quer.size()>0)
    {
        if (UpMessageBox::Question(this, tr("Il existe ") + QString::number(quer.size()) + " " + gPrenomPatient + tr(" dont le sexe n'est pas précisé."), tr("Les convertir?")) == UpSmallButton::STARTBUTTON)
        {
            UpMessageBox *msgbox = new UpMessageBox(this);
            msgbox->setText(tr("Convertir ") + QString::number(quer.size()) + " " + gPrenomPatient + "...");
            msgbox->setIcon(UpMessageBox::Warning);
            UpSmallButton *MBouton = new UpSmallButton();
            MBouton->setText(tr("Masculin"));
            UpSmallButton *FBouton = new UpSmallButton();
            FBouton->setText(tr("Féminin"));
            UpSmallButton *AnnulBouton = new UpSmallButton();
            AnnulBouton->setText(tr("Annuler"));
            msgbox->addButton(AnnulBouton, UpSmallButton::CANCELBUTTON);
            msgbox->addButton(MBouton, UpSmallButton::COPYBUTTON);
            msgbox->addButton(FBouton, UpSmallButton::STARTBUTTON);
            MBouton->setIcon(proc->giconMan);
            FBouton->setIcon(proc->giconWomen);
            msgbox->exec();
            if (msgbox->clickedButton() == MBouton)
                Sexe = "M";
            else if (msgbox->clickedButton() == FBouton)
                Sexe = "F";
            if (Sexe != ""){
                QSqlQuery ("update " NOM_TABLE_PATIENTS " set sexe = '" + Sexe + "' where PatPrenom = '" + gPrenomPatient + "' and sexe = ''", db);
                req ="select idpat from " NOM_TABLE_PATIENTS " where sexe = ''";
                QSqlQuery quer1(req,db);
                UpMessageBox::Information(this, tr("Il reste ") + QString::number(quer1.size()) + tr(" dossiers pour lesquels le sexe n'est pas précisé"),"");
                AfficheDossier(gidPatient);
            }
        }
    }
}

/*-----------------------------------------------------------------------------------------------------------------
-- Vérifie la cohérence des renseignements comptables avant de quitter un acte ------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
bool Rufus::AutorDepartConsult(bool ChgtDossier)
{
    QString         Titre = "";
    UpMessageBox    msgbox;
    bool            AutorDepart = true;
     // 1. On se repositionne sur le tab dossier
    if (ui->tabWidget->indexOf(ui->tabDossier) < 0)
        return true;
    ui->tabWidget->setCurrentWidget(ui->tabDossier);
    if (!ui->Acteframe->isVisible()) return FermeDossier();


    //1. On vérifie si on peut quitter la consultation sans quitter le dossier (il n'est pas obligatoire d'avoir la ligne correspondante dans typepaiementactes
    if (ui->ActeCotationcomboBox->currentText() == "")
        Titre = tr("Il manque la cotation!");
    else if (ui->ActeMontantlineEdit->text() == "")
        Titre = tr("Il manque le montant!");
    if (Titre != "")
        AutorDepart = false;

    //2. On ne cherche pas à quitter le dossier mais seulement à se déplacer dans les consultations du dossier
    if (!ChgtDossier)
    {
        if (AutorDepart)    return true;
        else
        {
            UpMessageBox::Watch(this, tr("Consultation incomplète"), Titre);
            if (Titre == tr("Il manque la cotation!"))
            {
                ui->ActeCotationcomboBox->setFocus();
                ui->ActeCotationcomboBox->showPopup();
            }
            else if (Titre == tr("Il manque le montant!"))
                ui->ActeMontantlineEdit->setFocus();
            return false;
        }
    }
    else
    {
        // 3 On veut quitter le dossier;
        // On vérifie si on peut quitter le dossier et la cohérence avec TypePaimentsActes
        if (AutorDepart)
        {
            // on recherche si le dernier acte du dossier est enregistré dans typepaiements - si lemontant de l'acte est 0, on propose de l'enregistrer comme gratuit

            QString requete =   "SELECT max(act.idActe), ActeDate, ActeCotation, ActeMontant FROM " NOM_TABLE_ACTES
                    " act WHERE idPat = " + QString::number(gidPatient) +
                    " AND act.idActe NOT IN (SELECT typ.idActe FROM " NOM_TABLE_TYPEPAIEMENTACTES " typ)";

            QSqlQuery EnregDernierActeQuery (requete,db);
            proc->TraiteErreurRequete(EnregDernierActeQuery,requete, "Impossible de trouver le dernier acte du patient pour le contrôler");

            // cette requête renvoie toujours une table non vide en QT même si elle est vide en mysql... d'où la suite
            EnregDernierActeQuery.first();
            if (EnregDernierActeQuery.value(0).toInt() > 0) // =il n'y a pas de paiement enregistré pour le dernier acte
            {
                if (EnregDernierActeQuery.value(0).toInt() != gidActe)
                    AfficheActe(EnregDernierActeQuery.value(0).toInt());
                if (QLocale().toDouble(ui->ActeMontantlineEdit->text()) == 0 && ui->ActeCotationcomboBox->currentText() != "")   // il s'agit d'un acte gratuit - on propose de le classer
                {
                    msgbox.setText(tr("Vous avez entré un montant nul !"));
                    msgbox.setInformativeText(tr("Enregistrer cette consultation comme gratuite?"));
                    msgbox.setIcon(UpMessageBox::Warning);
                    UpSmallButton *OKBouton = new UpSmallButton();
                    OKBouton->setText(tr("Consultation gratuite"));
                    UpSmallButton *NoBouton = new UpSmallButton();
                    NoBouton->setText(tr("Non"));
                    msgbox.addButton(NoBouton, UpSmallButton::CANCELBUTTON);
                    msgbox.addButton(OKBouton, UpSmallButton::STARTBUTTON);
                    msgbox.exec();
                    if (msgbox.clickedButton() != OKBouton)
                        return false;
                    else
                    {
                        requete = "INSERT INTO " NOM_TABLE_TYPEPAIEMENTACTES " (idActe, TypePaiement) VALUES (" + QString::number(gidActe) + ",'G')";
                        QSqlQuery InsertGratuitQuery (requete,db);
                        if (!proc->TraiteErreurRequete(InsertGratuitQuery,requete,""))
                            AutorDepart = true;
                    }
                }
                else
                {
                    Titre = tr("il manque les informations de paiement");
                    AutorDepart = false;
                }
            }
        }

        if (AutorDepart)
            return FermeDossier();
        else
        {
            if (Titre == tr("Il manque la cotation!"))
            {
                if (!SalleDattente(Titre))
                {
                    ui->ActeCotationcomboBox->setFocus();
                    return false;
                }
                else return true;
            }
            if (Titre == tr("Il manque le montant!"))
            {
                if (!SalleDattente(Titre))
                {
                    ui->ActeMontantlineEdit->setFocus();
                    return false;
                }
                else return true;
            }
            if (Titre.contains(tr("il manque les informations de paiement")))
                return SalleDattente(Titre);
        }
    }
    return true;
}


/*-----------------------------------------------------------------------------------------------------------------
-- Autorisation de sortie de l'application - appelé par closeevent()  ---------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
bool Rufus::AutorSortieAppli()
{
    QList<dlg_paiement *> PaimtList = findChildren<dlg_paiement*>();
    if (PaimtList.size()>0)
        for (int i=0; i<PaimtList.size();i++)
            if (PaimtList.at(i)->isVisible())
            {
                QSound::play(NOM_ALARME);
                PaimtList.at(i)->raise();
                return false;
            }

    // si le tab dossier est ouvert, on vérifie le droit de fermer le dossier en cours
    if (ui->tabWidget->indexOf(ui->tabDossier) != -1)
    {
        ui->tabWidget->setCurrentWidget(ui->tabDossier);
        if (AutorDepartConsult(true))
        {
            FermeDlgAnnexes();
            ui->tabWidget->removeTab(ui->tabWidget->indexOf(ui->tabDossier));
            proc->UpdVerrouSalDat();
        }
        else
            return false;
    }

    // le tab dossier est fermé, on vérifie s'il y a du monde en salle d'attente
    QString req = "SELECT Statut, IdPat, PosteExamen FROM " NOM_TABLE_SALLEDATTENTE " WHERE IdUser = '" + QString::number(gidUser) + "'";
    QSqlQuery SaldatQuery (req,db);
    proc->TraiteErreurRequete(SaldatQuery,req,"");
    SaldatQuery.first();
    if (SaldatQuery.size() > 0)
    {
     /* 2 possibilités
     * 1. C'est le seul poste connecté pour cet utilisateur
     * 2. cet utilisateur est connecté sur d'autres postes, on peut partir
    */
        req = "select distinct nomposteconnecte from " NOM_TABLE_USERSCONNECTES " where idUser = " + QString::number(gidUser);
        QSqlQuery nbpostesquery(req,db);
        if (nbpostesquery.size()<2)
            for (int i = 0; i < SaldatQuery.size() ; i++)  // il reste des patients pour cet utilisateur dans le centre
            {
                QString Statut = SaldatQuery.value(0).toString();
                QString blabla = ENATTENTENOUVELEXAMEN;
                if (Statut == ENCOURS
                        || Statut == ARRIVE
                        || Statut.contains(blabla)
                        || Statut == ENCOURSEXAMEN + gUserLogin
                        || Statut == RETOURACCUEIL)
                {
                    // il y a du monde en salle d'attente, on refuse la fermeture
                    QSound::play(NOM_ALARME);
                    UpMessageBox msgbox;
                    UpSmallButton *OKBouton = new UpSmallButton();
                    OKBouton->setText("OK");
                    UpSmallButton *NoBouton = new UpSmallButton();
                    NoBouton->setText(tr("Fermer quand même"));
                    msgbox.setText("Euuhh... " + gUserLogin + ", " + tr("vous ne pouvez pas fermer l'application."));
                    msgbox.setInformativeText(tr("Vous avez encore des patients en salle d'attente dont la consultation n'est pas terminée."));
                    msgbox.setIcon(UpMessageBox::Warning);
                    msgbox.addButton(NoBouton, UpSmallButton::CLOSEBUTTON);
                    msgbox.addButton(OKBouton, UpSmallButton::OUPSBUTTON);
                    msgbox.exec();
                    if (msgbox.clickedButton() != NoBouton)
                    {
                        ui->tabWidget->setCurrentIndex(ui->tabWidget->indexOf(ui->tabList));
                        return false;
                    }
                    else i = SaldatQuery.size();
                }
                SaldatQuery.next();
            }
        else
        {
            // l'utilisateur est connecté sur plusieurs postes, on peut fermer
        }
    }
    //on déverrouille les actes verrouillés en comptabilité par cet utilisateur
    QString LibereVerrouRequete = "DELETE FROM " NOM_TABLE_VERROUCOMPTAACTES " WHERE PosePar = " + QString::number(gidUser);
    QSqlQuery LibereVerrouComptaQuery (LibereVerrouRequete,db);
    proc->TraiteErreurRequete(LibereVerrouComptaQuery,LibereVerrouRequete,"");
    // on retire cet utilisateur de la table des utilisateurs connectés
    req = "delete from " NOM_TABLE_USERSCONNECTES " where NomPosteConnecte = '" + QHostInfo::localHostName().left(60) + "'";
    QSqlQuery qer(req,db);
    proc->TraiteErreurRequete(qer,req,"");
    proc->UpdVerrouSalDat();
    if (gIPadr != "")
    {
        QString IPV4 = QString::number(QHostAddress(gIPadr).toIPv4Address());
        QSqlQuery("delete from " NOM_TABLE_GESTIONFICHIERS " where IPV4Gestionnaire = " + IPV4,db);
    }
    if (proc->PosteImportDocs().remove(" - prioritaire")==QHostInfo::localHostName())
        proc->setPosteImportDocs(false);

    req = "update " NOM_TABLE_UTILISATEURS " set datederniereconnexion = '" + QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss")
            + "' where idUser = " + QString::number(gidUser);
    proc->TraiteErreurRequete(QSqlQuery(req,db), req, "");

    return true;
}

/*-----------------------------------------------------------------------------------------------------------------
-- Chargement des données de l'utilisateur --------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
QString Rufus::CalcToolTipCorrespondant(int idcor)
{
    QString tooltp = "";
    QString req = " select nomSpecialite,CorAdresse1,CorAdresse2,CorAdresse3,CorVille,CorTelephone, corspecialite, corautreprofession"
                  " FROM " NOM_TABLE_CORRESPONDANTS " as cor"
                  " left outer join " NOM_TABLE_SPECIALITES " on idspecialite = corspecialite"
                  " where idcor = " + QString::number(idcor);
    //qDebug() << req;
    QSqlQuery tooltpquer(req,db);
    if (tooltpquer.seek(0))
    {
        if (tooltpquer.value(6).toInt() != 0)
            tooltp = tooltpquer.value(0).toString();
        else if (tooltpquer.value(7).toString() != "")
            tooltp = tooltpquer.value(7).toString();
        if (tooltpquer.value(1).toString() != "")
        {
            if (tooltp != "") tooltp += "\n";
            tooltp += tooltpquer.value(1).toString();
        }
        if (tooltpquer.value(2).toString() != "")
        {
            if (tooltp != "") tooltp += "\n";
            tooltp += tooltpquer.value(2).toString();
        }
        if (tooltpquer.value(3).toString() != "")
        {
            if (tooltp != "") tooltp += "\n";
            tooltp += tooltpquer.value(3).toString();
        }
        if (tooltpquer.value(4).toString() != "")
        {
            if (tooltp != "") tooltp += "\n";
            tooltp += tooltpquer.value(4).toString();
        }
        if (tooltpquer.value(5).toString() != "")
        {
            if (tooltp != "") tooltp += "\n";
            tooltp += tooltpquer.value(5).toString();
        }
    }
    return tooltp;
}

/*-----------------------------------------------------------------------------------------------------------------
-- Chargement des données de l'utilisateur --------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
bool    Rufus::ChargeDataUser()
{
    gDataUser                           = proc->getDataUser();
    gidUserSuperviseur                  = gDataUser["UserSuperviseur"].toInt();
    gidUserParent                       = gDataUser["idParent"].toInt();
    gidUserComptable                    = gDataUser["idUserComptable"].toInt();

    gUserDroits                         = gDataUser["Droits"].toString();
    gUserAGA                            = gDataUser["AGA"].toBool();
    gUserLogin                          = gDataUser["UserLogin"].toString();
    gUserFonction                       = gDataUser["Fonction"].toString();
    gUserTitre                          = gDataUser["Titre"].toString();
    gUserNom                            = gDataUser["Nom"].toString();
    gUserPrenom                         = gDataUser["Prenom"].toString();
    gUserAdresse1                       = gDataUser["Adresse1"].toString();
    gUserAdresse2                       = gDataUser["Adresse2"].toString();
    gUserAdresse3                       = gDataUser["Adresse3"].toString();
    gUserCodePostal                     = gDataUser["CodePostal"].toString();
    gUserVille                          = gDataUser["Ville"].toString();
    gUserTelephone                      = gDataUser["Telephone"].toString();
    gUserMail                           = gDataUser["Mail"].toString();
    gUserNumPS                          = gDataUser["NumPS"].toString();
    gUserNumCO                          = gDataUser["NumCO"].toString();
    gUserSpecialite                     = gDataUser["Specialite"].toString();
    gUserNoSpecialite                   = gDataUser["NoSpecialite"].toInt();
    gUserLiberal                        = (gDataUser["EnregHonoraires"].toInt()==1);
    gUserMDP                            = gDataUser["MDP"].toString();
    gUserPortable                       = gDataUser["Portable"].toString();
    gUserPoste                          = gDataUser["Poste"].toString();
    gUserFax                            = gDataUser["Fax"].toString();
    gUserWeb                            = gDataUser["Web"].toString();
    gUserMemo                           = gDataUser["Memo"].toString();
    gUserDesactive                      = gDataUser["Desactive"].toBool();
    gUserSecteur                        = gDataUser["Secteur"].toInt();
    gUserPoliceEcran                    = gDataUser["PoliceEcran"].toString();
    gUserPoliceAttribut                 = gDataUser["PoliceAttribut"].toString();
    gUserdateDerniereConnexion          = gDataUser["DateDerniereConnexion"].toDateTime();
    gMedecin                            = (gDataUser["Medecin"].toInt()==1);
    gUserSecteur                        = gDataUser["Secteur"].toInt();
    gUserOPTAM                          = gDataUser["OPTAM"].toBool();

    gidCompteParDefaut                  = gDataUser["idCompteParDefaut"].toInt();
    gidCompteEncaissHonoraires          = gDataUser["idCompteEncaissHonoraires"].toInt();
    gNomCompteEncaissHonoraires         = gDataUser["NomCompteEncaissHonoraires"].toString();

    ophtalmo       = gDataUser["Soignant"].toInt() == 1;
    orthoptist     = gDataUser["Soignant"].toInt() == 2;
    autresoignant  = gDataUser["Soignant"].toInt() == 3;
    secretaire     = gDataUser.value("Droits").toString() == SECRETAIRE;
    soignant       = ophtalmo || orthoptist || autresoignant;
    assistant      = soignant && (proc->UserSuperviseur() != gidUser);
    liberal        = soignant && (gDataUser["EnregHonoraires"].toInt() == 1);
    salarie        = soignant && (gDataUser["EnregHonoraires"].toInt() == 2);
    remplacant     = soignant && (gDataUser["EnregHonoraires"].toInt() == 3);
    pasremplacant  = soignant && (gDataUser["EnregHonoraires"].toInt() != 3);
    return true;
}

/*-----------------------------------------------------------------------------------------------------------------
-- Calcule la liste des mots clés du patient  -------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void    Rufus::CalcMotsCles(int idpt)
{
    QString req = "select motcle from " NOM_TABLE_MOTSCLES " where idmotcle in (select idmotcle from " NOM_TABLE_MOTSCLESJOINTURES " where idpat = " + QString::number(idpt) + ")";
    QSqlQuery quer(req,db);
    QString result ("<font color=\"green\">Mots clés: </font>");
    if (quer.size()>0)
    {
        quer.first();
        for (int i=0; i<quer.size(); i++)
        {
            result += quer.value(0).toString();
            if (i<quer.size()-1)
                result += ", ";
            quer.next();
        }
   }
    ui->MotsClesLabel->setText(result);
    ui->MotsClesLabel->setFont(qApp->font());
}


/*-----------------------------------------------------------------------------------------------------------------
-- Affiche le nombre de dossiers du treeView en cours -------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void    Rufus::CalcNbDossiers()
{
    int a = gListePatientsModel->rowCount();
    switch (a) {
    case 0:
        ui->label_15->setText(tr("aucun dossier pour ces critères"));
        break;
    case 1:
        ui->label_15->setText("1 dossier");
        break;
    default:
        if (proc->getModeConnexion() == Procedures::Distant && a==1000)
            ui->label_15->setText("> 1000 " + tr("dossiers"));
        else
            ui->label_15->setText(QString::number(a) + " " + tr("dossiers"));
        break;
    }
}


/*-----------------------------------------------------------------------------------------------------------------
-- Rechercher le nom dans les TreeWidget  en restreignant la liste ------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void    Rufus::ChercheNomFiltre(int id) // Dans ce mode de recherche, la liste est filtrée aux valeurs correspondant aux zones de saisie
{
    QString Filtrerequete;
    QString idPat = "0";

    if (id>0)
        idPat = QString::number(id);
    else if (ui->PatientsListeTableView->selectionModel()->selectedIndexes().size() > 0)
        idPat = gListePatientsModel->itemFromIndex(ui->PatientsListeTableView->selectionModel()->selectedIndexes().at(0))->text();

    Filtrerequete = "SELECT IdPat, PatNom, PatPrenom, PatDDN, Sexe "
                    " FROM "  NOM_TABLE_PATIENTS;
    if (ui->CreerNomlineEdit->text() != "" || ui->CreerPrenomlineEdit->text() != "")
        Filtrerequete += " WHERE PatNom LIKE '" + proc->CorrigeApostrophe(ui->CreerNomlineEdit->text()) + "%'" +
                " AND PatPrenom LIKE '" + proc->CorrigeApostrophe(ui->CreerPrenomlineEdit->text()) + "%'";

    Remplir_ListePatients_TableView(Filtrerequete,"","");
    CalcNbDossiers();

    if ((ui->CreerNomlineEdit->text() != "" || ui->CreerPrenomlineEdit->text() != "") && id==0)
    {
        if (gListePatientsModel->rowCount()>0)
        {
            ui->PatientsListeTableView->selectRow(0);
            ui->PatientsListeTableView->scrollTo(gListePatientsModel->item(0)->index(),QAbstractItemView::PositionAtTop);
        }
        ListeRestreinte = true;
    }
    else
    {
        if (idPat != "0")
        {
            QList<QStandardItem*> listitems = gListePatientsModel->findItems(idPat);
            if (!listitems.isEmpty())
            {
                ui->PatientsListeTableView->selectRow(listitems.at(0)->row());
                ui->PatientsListeTableView->scrollTo(listitems.at(0)->index(),QAbstractItemView::PositionAtTop);
            }
        }
        ListeRestreinte = false;
    }
    Slot_EnableCreerDossierButton();
}

/*-----------------------------------------------------------------------------------------------------------------
-- Rechercher le nom dans la listePatients-------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void    Rufus::ChercheNom(int id)  // ce mode de recherche se fait sans filtrage de la liste qui est vue dans son intégralité
{
    QString requete;
    int i;
    if (id>0)
        i = id;
    else
    {
        requete = "SELECT IdPat "
                  " FROM "  NOM_TABLE_PATIENTS
                " WHERE PatNom LIKE '" + proc->CorrigeApostrophe(ui->CreerNomlineEdit->text()) + "%'" +
                " AND PatPrenom LIKE '" + proc->CorrigeApostrophe(ui->CreerPrenomlineEdit->text()) + "%' ORDER BY PatNom, PatPrenom, PatDDN ";
        QSqlQuery ChercheNomQuery (requete,db);
        if (proc->TraiteErreurRequete(ChercheNomQuery,requete,tr("Impossible de rechercher dans le fichier")))
            return;
        if (ChercheNomQuery.size() == 0)
            return;
        ChercheNomQuery.first();
        i = ChercheNomQuery.value(0).toInt();
    }
    QList<QStandardItem*> listitems = gListePatientsModel->findItems(QString::number(i));
    if (!listitems.isEmpty())
    {
        ui->PatientsListeTableView->selectRow(listitems.at(0)->row());
        ui->PatientsListeTableView->scrollTo(listitems.at(0)->index(),QAbstractItemView::PositionAtTop);
    }
    Slot_EnableCreerDossierButton();
}

/*-----------------------------------------------------------------------------------------------------------------
-- Vérifie les verrous d'un dossier avant de l'afficher -----------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void    Rufus::ChoixDossier(int idpat)  // appelée depuis la tablist ou la salle d'attente - vérifie qu'un dossier n'est pas verrouillé avant de l'afficher
{
    if (gUserDroits == SECRETAIRE)    // si l'utilisateur est une secrétaire, on propose de mettre le patient en salle d'attente
        Slot_InscritEnSalDat(idpat);
    else
    {
        if (ui->tabWidget->indexOf(ui->tabDossier) > 0)
        {
            if (gidPatient == idpat)
            {
                ui->tabWidget->setCurrentWidget(ui->tabDossier);
                return;
            }
            else
                if (!AutorDepartConsult(true)) return;
        }
        else
        {
            gdossierAOuvrir = 0;
            // On vérifie si le dossier n'est pas verrouillé par un autre utilisateur
            QString blabla = ENCOURSEXAMEN;
            int length = blabla.size();
            QString Verrourequete =
                    "SELECT idPat, Statut, PosteExamen FROM " NOM_TABLE_SALLEDATTENTE
                    " WHERE idPat = " + QString::number(idpat) + " AND Left(Statut," + QString::number(length) + ") = '" ENCOURSEXAMEN "'"
                    " AND (idUserEnCoursExam != " + QString::number(gidUser) + " OR (idUserEnCoursExam = " + QString::number(gidUser) + " AND PosteExamen != '" + QHostInfo::localHostName().left(60) +"'))";
            //proc->Edit(Verrourequete);
            QSqlQuery ChercheVerrouQuery (Verrourequete,db);
            proc->TraiteErreurRequete(ChercheVerrouQuery,Verrourequete,"");
            if (ChercheVerrouQuery.size() > 0)
            {
                ChercheVerrouQuery.first();
                UpMessageBox::Watch(this,tr("Impossible d'ouvrir ce dossier!"),
                                    tr("Ce patient est") + ChercheVerrouQuery.value(1).toString() + "\n" + tr("sur ") + ChercheVerrouQuery.value(2).toString());
                return;
            }
        }
        AfficheDossier(idpat);
    }
}

/*-----------------------------------------------------------------------------------------------------------------
-- Créer une consultation ------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void    Rufus::CreerActe(int idPat)
{
    if (ui->Acteframe->isVisible())
        if(!AutorDepartConsult(false)) return;
    QString rempla = (gDataUser["EnregHonoraires"].toInt()==3? "1" : "null");
    QString creerrequete =
            "INSERT INTO " NOM_TABLE_ACTES
            " (idPat, idUser, ActeDate, ActeHeure, CreePar, UserComptable, UserParent,SuperViseurRemplacant, NumCentre, idLieu)"
            " VALUES (" +
            QString::number(idPat) + ", " +
            QString::number(gidUserSuperviseur) + ", "
            "NOW(), "
            "NOW(), " +
            QString::number(gidUser) + ", " +
            QString::number(gidUserComptable) + ", " +
            QString::number(gidUserParent) + ", " +
            rempla + ", " +
            QString::number(proc->idCentre()) + ", " +
            gDataUser["idLieu"].toString() +")";
    //qDebug() << creerrequete;
    QSqlQuery CreerActeQuery (creerrequete,db);
    if (proc->TraiteErreurRequete(CreerActeQuery,creerrequete,tr("Impossible de créer cette consultation dans ") + NOM_TABLE_ACTES))
            return ;
    // Récupération de l'idActe créé et affichage du dossier ------------------------------------
    QString maxrequete = "SELECT MAX(idActe) FROM " NOM_TABLE_ACTES
                " WHERE idUser = " + QString::number(gidUserSuperviseur) + " AND idPat = "+ QString::number(idPat);
    QSqlQuery SelectActeQuery (maxrequete,db);
    if (proc->TraiteErreurRequete(SelectActeQuery,maxrequete,tr("Impossible de retrouver l'acte qui vient d'être créé")))
            return ;
    SelectActeQuery.first();
    AfficheActe(SelectActeQuery.value(0).toInt());
    QString req = "SELECT idActe FROM " NOM_TABLE_ACTES " WHERE idPat = " + QString::number(idPat);
    QSqlQuery quer (req,db);
    if (proc->TraiteErreurRequete(quer,req,tr("Impossible de compter le nombre d'actes")))
            return ;
    else if (quer.size()>1)
    {
        QList<dlg_actesprecedents *> listactesprecs = findChildren<dlg_actesprecedents *>();
        for (int i = 0; i<listactesprecs.size();i++)
        {
            if (listactesprecs.at(i)->getidPatient() != idPat)
            {
                listactesprecs.at(i)->close();
                listactesprecs.removeAt(i);
                i--;
            }
        }
        if (listactesprecs.size() == 0)
        {
            quer.last();
            OuvrirActesPrecedents(quer.value(0).toInt());
        }
        else
            listactesprecs.at(0)->Actualise();
    }
    ui->ActeMotiftextEdit->setFocus();
}

/*-----------------------------------------------------------------------------------------------------------------
-- Basculer le mode de recherche du dossier (DDN ou Nom/Prenom) -------------
-----------------------------------------------------------------------------------------------------------------*/
void    Rufus::ChercherDepuisListe()
{
    ui->CreerDDNdateEdit->setDate(gdateParDefaut);
    ui->CreerNomlineEdit->setVisible(!ui->CreerNomlineEdit->isVisible());
    ui->CreerPrenomlineEdit->setVisible(!ui->CreerPrenomlineEdit->isVisible());
    ui->CreerDDNdateEdit->setVisible(!ui->CreerDDNdateEdit->isVisible());
    ui->Nomlabel->setVisible(!ui->Nomlabel->isVisible());
    ui->Prenomlabel->setVisible(!ui->Prenomlabel->isVisible());
    ui->DDNlabel->setVisible(!ui->DDNlabel->isVisible());
    if (gMode != RechercheDDN)
    {
        ui->ChercherDepuisListepushButton->setText(tr("Chercher avec\nnom et prénom"));
        ui->ChercherDepuisListepushButton->setIcon(proc->giconContact);
        gMode = RechercheDDN;
        ui->CreerDDNdateEdit->setFocus();
        ui->FiltrecheckBox->setVisible(false);
    }
    else
    {
        OuvrirListe(false);
    }
}

/*-----------------------------------------------------------------------------------------------------------------
-- Créer un dossier -----------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void    Rufus::CreerDossier()
{
    QString PatNom, PatPrenom, PatDDN, PatCreePar, PatCreeLe;
    int idPat; // on n'utilise pas gidPatient qui ne sera initialisé qu'après que le dossier ait été réellement affiché.

    PatNom      = proc->MajusculePremiereLettre(ui->CreerNomlineEdit->text(),true);
    PatPrenom   = proc->MajusculePremiereLettre(ui->CreerPrenomlineEdit->text(),true);
    PatDDN      = ui->CreerDDNdateEdit->date().toString("yyyy-MM-dd");
    PatCreeLe   = QDateTime::currentDateTime().date().toString("yyyy-MM-dd");
    PatCreePar  = QString::number(gidUser);
    if (PatNom == "")
    {
        UpMessageBox::Watch(this, tr("Vous devez spécifier un nom!"));
        ui->CreerNomlineEdit->setFocus();
        return;
    }
    if (PatPrenom == "")
    {
        UpMessageBox::Watch(this, tr("Vous devez spécifier un prénom!"));
        ui->CreerPrenomlineEdit->setFocus();
        return;
    }

    // On vérifie qu'une date de naissance a été enregistrée, différente de gdateParDefaut
    if (ui->CreerDDNdateEdit->date() == gdateParDefaut)
    {
        QSound::play(NOM_ALARME);
        UpMessageBox msgbox;
        UpSmallButton *OKBouton = new UpSmallButton();
        OKBouton->setText(tr("Je confirme"));
        UpSmallButton *NoBouton = new UpSmallButton();
        NoBouton->setText(tr("Annuler"));
        msgbox.setText("Euuhh... " + gUserLogin);
        msgbox.setInformativeText(tr("Confirmez vous la date de naissance?") + "\n" + ui->CreerDDNdateEdit->date().toString(tr("d-MMM-yyyy")));
        msgbox.setIcon(UpMessageBox::Warning);
        msgbox.addButton(NoBouton, UpSmallButton::CANCELBUTTON);
        msgbox.addButton(OKBouton, UpSmallButton::STARTBUTTON);
        msgbox.exec();
        if (msgbox.clickedButton() != OKBouton)
            return;
    }

    // 1. On recherche d'abord si le dossier existe
    idPat = LectureMesure("", PatNom, PatPrenom, PatDDN, "", "", "Impossible de rechercher le dossier");
    if (idPat == -1)                                                // il y a eu une erreur pendant la recherche
        return ;
    if (idPat > 0)                                                  // Le dossier existe, on l'affiche
    {
        UpMessageBox::Watch(this, tr("Ce patient est déjà venu!"));
        if (gUserDroits != SECRETAIRE)
            AfficheDossier(gidPatient);
        else
        {
            UpMessageBox msgbox;
            msgbox.setText(tr("Ouverture du dossier de ")  + PatNom + " " + PatPrenom);
            msgbox.setInformativeText(tr("Inscrire le dossier en salle d'attente?"));
            msgbox.setIcon(UpMessageBox::Warning);
            UpSmallButton *OKBouton = new UpSmallButton();
            OKBouton->setText(tr("Inscrire en\nsalle d'attente"));
            UpSmallButton *NoBouton = new UpSmallButton();
            NoBouton->setText(tr("Ne pas inscrire"));
            msgbox.addButton(NoBouton, UpSmallButton::CANCELBUTTON);
            msgbox.addButton(OKBouton, UpSmallButton::STARTBUTTON);
            msgbox.exec();
            if (msgbox.clickedButton() == OKBouton)
                Slot_InscritEnSalDat(gidPatient);
        }
        return;
    }

    /*----------------------------------------------------------------------------------------------------------------
       // il faudrait faire une recherche de dossier avec orthographe similaire
    -----------------------------------------------------------------------------------------------------------------*/

    // 2. On recherche ensuite un dossier similaire Nom + Prenom
    if (ui->PatientsListeTableView->isVisible()  && gListePatientsModel->rowCount()>0)
    {
        UpMessageBox msgbox;
        msgbox.setText(tr("Un ou plusieurs dossiers similaires!"));
        msgbox.setInformativeText(tr("Il existe des dossiers similaires\nà celui que vous essayez d'enregistrer!\nLe créer quand meme?"));
        msgbox.setIcon(UpMessageBox::Warning);
        UpSmallButton *OKBouton = new UpSmallButton();
        OKBouton->setText(tr("Créer le dossier"));
        UpSmallButton *NoBouton = new UpSmallButton();
        NoBouton->setText(tr("Annuler"));
        msgbox.addButton(NoBouton, UpSmallButton::CANCELBUTTON);
        msgbox.addButton(OKBouton, UpSmallButton::STARTBUTTON);
        msgbox.exec();
        if (msgbox.clickedButton() != OKBouton)
            return;
    }

    // 3. le dossier n'existe pas, on le crée
    if (idPat == 0)
    {
        // Récupération de nom, prénom et DDN puis création du dossier---------------------------------
        QString insrequete = "INSERT INTO " NOM_TABLE_PATIENTS
                " (PatNom, PatPrenom, PatDDN, PatCreele, PatCreePar, Sexe) "                                                                      // CZ001
                " VALUES "
                " ('" + proc->CorrigeApostrophe(PatNom) + "', '" + proc->CorrigeApostrophe(PatPrenom) + "', '" + PatDDN + "', NOW(), '" + PatCreePar +"' , '" + gSexePat +"');";   // CZ001
        QSqlQuery InsertPatQuery (insrequete,db);
        if (proc->TraiteErreurRequete(InsertPatQuery,insrequete,tr("Impossible de créer le dossier")))
            return ;

        // Récupération de l'idPatient créé et affichage du dossier ------------------------------------
        QString recuprequete = "SELECT  idPat, PatNom, PatPrenom FROM " NOM_TABLE_PATIENTS
                " WHERE PatNom = '" + proc->CorrigeApostrophe(PatNom) + "' AND PatPrenom = '" + proc->CorrigeApostrophe(PatPrenom) + "' AND PatDDN = '" + PatDDN + "'";
        QSqlQuery ChercheIdPatientQuery (recuprequete,db);
        if (proc->TraiteErreurRequete(ChercheIdPatientQuery,recuprequete,tr("Impossible de sélectionner les enregistrements")))
            return ;

        ChercheIdPatientQuery.first();
        gidPatient      = ChercheIdPatientQuery.value(0).toInt();
        gNomPatient     = ChercheIdPatientQuery.value(1).toString();
        gPrenomPatient  = ChercheIdPatientQuery.value(2).toString();
        QString requete =   "INSERT INTO " NOM_TABLE_DONNEESSOCIALESPATIENTS " (idPat) VALUES ('" + QString::number(gidPatient) + "')";
        QSqlQuery CreeDonneeSocialePatientQuery (requete,db);
        proc->TraiteErreurRequete(CreeDonneeSocialePatientQuery,requete,tr("Impossible de créerles données sociales"));

        if (!IdentificationPatient("Creation",gidPatient)) return;
        FlagMetAjourTreeView();

        // Si le User est un soignant, on crée d'emblée une consultation et on l'affiche
        if (gUserDroits != SECRETAIRE)
        {
            UpMessageBox msgbox;
            msgbox.setText(tr("Dossier ") + gNomPatient + " " + gPrenomPatient + tr(" créé"));
            msgbox.setInformativeText(tr("Ouvrir le dossier ou inscrire le dossier en salle d'attente?"));
            msgbox.setIcon(UpMessageBox::Quest);
            UpSmallButton *OKBouton = new UpSmallButton();
            OKBouton->setText(tr("Inscrire le dossier\nen salle d'attente"));
            UpSmallButton *AnnulBouton = new UpSmallButton();
            AnnulBouton->setText("Ouvrir le dossier");
            UpSmallButton *NoBouton = new UpSmallButton();
            NoBouton->setText(tr("Ne rien faire"));
            msgbox.addButton(NoBouton,UpSmallButton::CLOSEBUTTON);
            msgbox.addButton(OKBouton, UpSmallButton::NOBUTTON);
            msgbox.addButton(AnnulBouton, UpSmallButton::NOBUTTON);
            OKBouton->setIcon(proc->giconAttente);
            AnnulBouton->setIcon(proc->giconSortirDossier);
            msgbox.setDefaultButton(NoBouton);
            //NoBouton->setFocus();
            msgbox.exec();
            if (msgbox.clickedButton() == OKBouton)
            {
                if (!Slot_InscritEnSalDat(gidPatient))
                    RecaleTableView(gidPatient);
            }
            else if (msgbox.clickedButton() == AnnulBouton)
            {
                CreerActe(gidPatient);
                AfficheDossier(gidPatient);
            }
            else
                RecaleTableView(gidPatient);
            delete NoBouton;
            delete AnnulBouton;
            delete OKBouton;
        }
        else
            if (!Slot_InscritEnSalDat(gidPatient))
                RecaleTableView(gidPatient);
    }
}


// ------------------------------------------------------------------------------------------
// Creer le menu général de l'appliaction
// ------------------------------------------------------------------------------------------

void Rufus::CreerMenu()
{
    QAction *Apropos = new QAction(tr("A propos"));
    menuBar()->addAction(Apropos);
    menuDossier         = menuBar()->addMenu(tr("Dossier"));
    menuEdition         = menuBar()->addMenu(tr("Edition"));
    menuActe            = menuBar()->addMenu(tr("Acte"));
    menuDocuments       = menuBar()->addMenu(tr("Documents"));
    menuEmettre         = new QMenu(tr("Emettre"));

    actionCreerDossier              = new QAction(tr("Créer"));
    actionCreerDossier              ->setStatusTip(tr("Créer un nouveau dossier"));
    actionOuvrirDossier             = new QAction(tr("Ouvrir"));
    actionOuvrirDossier             ->setStatusTip(tr("Ouvrir un dossier existant"));
    actionRecopierDossier           = new QAction(tr("Recopier"));
    actionRecopierDossier           ->setStatusTip(tr("Créer un dossier de la même famille"));
    actionSupprimerDossier          = new QAction(tr("Supprimer"));
    actionSupprimerDossier          ->setStatusTip(tr("Supprimer un dossier"));
    actionRechercheParMotCle        = new QAction(tr("Recherche par mot clé"));
    actionRechercheParID            = new QAction(tr("Recherche par ID patient"));

    actionParametres                = new QAction(tr("Paramètres"));

    actionCreerActe                 = new QAction(tr("Créer"));
    actionSupprimerActe             = new QAction(tr("Supprimer"));

    actionEmettreDocument           = new QAction(tr("Document simple"));
    actionDossierPatient            = new QAction(tr("Dossier patient"));
    actionEnregistrerDocScanner     = new QAction(tr("Enregistrer un document scanné"));
    actionEnregistrerVideo          = new QAction(tr("Enregistrer une video"));
    actionCorrespondants            = new QAction(tr("Liste des correspondants"));

    actionQuit                      = new QAction(tr("Quitter"));
    actionQuit                      ->setMenuRole(QAction::PreferencesRole);
    menuBar()->addAction(actionQuit);


    menuDossier->addAction(actionCreerDossier);
    menuDossier->addAction(actionOuvrirDossier);
    menuDossier->addSeparator();
    menuDossier->addAction(actionRecopierDossier);
    menuDossier->addSeparator();
    menuDossier->addAction(actionSupprimerDossier);
    menuDossier->addSeparator();
    menuDossier->addAction(actionRechercheParMotCle);
    menuDossier->addAction(actionRechercheParID);

    menuEdition->addAction(tr("Copier"));
    menuEdition->addAction(tr("Couper"));
    menuEdition->addAction(tr("Coller"));
    menuEdition->addSeparator();
    menuEdition->addAction(actionParametres);

    menuActe->addAction(actionCreerActe);
    menuActe->addAction(actionSupprimerActe);

    menuEmettre->addAction(actionEmettreDocument);
    menuEmettre->addAction(actionDossierPatient);

    menuDocuments->addMenu(menuEmettre);
    menuDocuments->addAction(actionEnregistrerDocScanner);
    menuDocuments->addAction(actionEnregistrerVideo);
    menuDocuments->addSeparator();
    menuDocuments->addAction(actionCorrespondants);

    // Les menus --------------------------------------------------------------------------------------------------
    connect (actionCreerDossier,                &QAction::triggered,                                [=] {Slot_OuvrirNouveauDossierpushButtonClicked();});
    connect (actionOuvrirDossier,               &QAction::triggered,                                [=] {Slot_OuvrirListepushButtonClicked();});
    connect (actionSupprimerDossier,            &QAction::triggered,                                [=] {Slot_SupprimerDossier();});
    connect (actionRechercheParMotCle,          &QAction::triggered,                                [=] {Slot_RechercheParMotCle();});
    connect (actionRechercheParID,              &QAction::triggered,                                [=] {Slot_RechercheParID();});
    connect (actionCreerActe,                   &QAction::triggered,                                [=] {Slot_CreerActe();});

    connect (actionParametres,                  &QAction::triggered,                                [=] {Slot_OuvrirParametres();});
    connect (actionSupprimerActe,               &QAction::triggered,                                [=] {Slot_SupprimerActepushButtonClicked();});
    // Documents
    connect (actionEmettreDocument,             &QAction::triggered,                                [=] {Slot_OuvrirDocuments();});
    connect (actionDossierPatient,              &QAction::triggered,                                [=] {Slot_ImprimeDossier();});
    connect (actionCorrespondants,              &QAction::triggered,                                [=] {Slot_ListeCorrespondants();});
    connect (actionEnregistrerDocScanner,       &QAction::triggered,                                [=] {EnregistreDocScanner();});
    connect (actionEnregistrerVideo,            &QAction::triggered,                                [=] {EnregistreVideo();});
    // Comptabilité

    connect (menuActe,                          SIGNAL(aboutToShow()),                              this,       SLOT (Slot_AfficheMenu()));
    connect (menuEdition,                       SIGNAL(aboutToShow()),                              this,       SLOT (Slot_AfficheMenu()));
    connect (menuDocuments,                     SIGNAL(aboutToShow()),                              this,       SLOT (Slot_AfficheMenu()));
    connect (menuDossier,                       SIGNAL(aboutToShow()),                              this,       SLOT (Slot_AfficheMenu()));

    menuComptabilite                = menuBar()->addMenu(tr("Comptabilité"));

    actionPaiementDirect            = new QAction(tr("Gestion des paiements directs"));
    actionPaiementTiers             = new QAction(tr("Gestion des tiers payants"));
    actionBilanRecettes             = new QAction(tr("Bilan des recettes"));
    actionRecettesSpeciales         = new QAction(tr("Enregistrement des recettes spéciales"));
    actionJournalDepenses           = new QAction(tr("Journal des dépenses"));
    actionGestionComptesBancaires   = new QAction(tr("Gestion des comptes bancaires"));
    actionRemiseCheques             = new QAction(tr("Effectuer une remise de chèques"));
    actionImpayes                   = new QAction(tr("Impayés"));

    menuComptabilite->addAction(actionPaiementDirect);
    menuComptabilite->addAction(actionPaiementTiers);
    menuComptabilite->addAction(actionBilanRecettes);
    menuComptabilite->addAction(actionRecettesSpeciales);
    menuComptabilite->addSeparator();
    menuComptabilite->addAction(actionJournalDepenses);
    menuComptabilite->addSeparator();
    menuComptabilite->addAction(actionGestionComptesBancaires);
    menuComptabilite->addAction(actionRemiseCheques);
    menuComptabilite->addAction(actionImpayes);

    connect (actionGestionComptesBancaires,     &QAction::triggered,                                [=] {Slot_GestionComptes();});
    connect (actionPaiementDirect,              &QAction::triggered,                                [=] {Slot_AppelPaiementDirect();});
    connect (actionPaiementTiers,               &QAction::triggered,                                [=] {Slot_AppelPaiementTiers();});
    connect (actionRecettesSpeciales,           &QAction::triggered,                                [=] {Slot_RecettesSpeciales();});
    connect (actionBilanRecettes,               &QAction::triggered,                                [=] {Slot_BilanRecettes();});
    connect (actionJournalDepenses,             &QAction::triggered,                                [=] {Slot_OuvrirJournalDepenses();});
    connect (actionRemiseCheques,               &QAction::triggered,                                [=] {Slot_RemiseCheques();});

    connect (menuComptabilite,                  SIGNAL(aboutToShow()),                              this,       SLOT (Slot_AfficheMenu()));
}

// ------------------------------------------------------------------------------------------
// Descend une ligne dans la table
// ------------------------------------------------------------------------------------------

void Rufus::DescendUneLigne()
{
    QModelIndexList listindx = ui->PatientsListeTableView->selectionModel()->selectedIndexes();
    if (listindx.size()==0) return;
    int itrow = listindx.at(0).row();
    if (itrow < gListePatientsModel->rowCount()-1)
    {
        ui->PatientsListeTableView->selectRow(gListePatientsModel->item(itrow+1)->row());
        ui->PatientsListeTableView->scrollTo(gListePatientsModel->item(itrow+1)->index(),QAbstractItemView::PositionAtCenter);
    }
}

// ------------------------------------------------------------------------------------------
// Descend 20 lignes dans la table
// ------------------------------------------------------------------------------------------

void Rufus::Descend20Lignes()
{
    QModelIndexList listindx = ui->PatientsListeTableView->selectionModel()->selectedIndexes();
    if (listindx.size()==0) return;
    int itrow = listindx.at(0).row();
    if (itrow < gListePatientsModel->rowCount()-20)
    {
        ui->PatientsListeTableView->selectRow(gListePatientsModel->item(itrow+20)->row());
        ui->PatientsListeTableView->scrollTo(gListePatientsModel->item(itrow+20)->index(),QAbstractItemView::PositionAtCenter);
    }
    else
    {
        ui->PatientsListeTableView->selectRow(gListePatientsModel->rowCount()-1);
        ui->PatientsListeTableView->scrollTo(gListePatientsModel->item(gListePatientsModel->rowCount()-1)->index(),QAbstractItemView::PositionAtCenter);
    }
}

// ------------------------------------------------------------------------------------------
// Enregistre un nouveau correpondant
// ------------------------------------------------------------------------------------------
int Rufus::EnregistreNouveauCorresp(QString Cor, QString Nom)
{
    int idcor = -1;
    bool onlydoctors = (Cor == "MG");
    Dlg_IdentCorresp        = new dlg_identificationcorresp("Creation", onlydoctors, 0, proc, this);
    Dlg_IdentCorresp->ui->NomlineEdit->setText(Nom);
    Dlg_IdentCorresp->ui->PrenomlineEdit->setFocus();
    if (Cor == "MG")
        Dlg_IdentCorresp->ui->MGradioButton->setChecked(true);
    if (Dlg_IdentCorresp->exec()>0)
        idcor = Dlg_IdentCorresp->gidCor;
    delete Dlg_IdentCorresp;
    return idcor;
}

/*-----------------------------------------------------------------------------------------------------------------
-- Fermeture de la fiche dlg_actesprecedents ----------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void Rufus::FermeDlgAnnexes()
{
    QList<dlg_actesprecedents *> ListDialog = this->findChildren<dlg_actesprecedents *>();
    for (int n = 0; n <  ListDialog.size(); n++)
        ListDialog.at(n)->close();
    QList<dlg_docsexternes *> ListDialogDocs = this->findChildren<dlg_docsexternes *>();
    for (int n = 0; n <  ListDialogDocs.size(); n++)
        ListDialogDocs.at(n)->close();
    QString req = "select idimpression from " NOM_TABLE_IMPRESSIONS " where idpat = " + QString::number(gidPatient);
    QSqlQuery quer(req,db);
    ui->OuvreDocsExternespushButton->setEnabled((quer.size()>0));
}

/*-----------------------------------------------------------------------------------------------------------------
-- Fermeture du dossier en cours ----------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
bool Rufus::FermeDossier()
{
    bool a = true;
    UpMessageBox msgbox;
    msgbox.setInformativeText(tr("Garder le dossier en salle d'attente?"));
    msgbox.setText(tr("Fermeture du dossier de ") + gNomPatient + " " + gPrenomPatient);
    msgbox.setIcon(UpMessageBox::Quest);

    UpSmallButton *OKBouton     = new UpSmallButton(tr("Garder le dossier\nen salle d'attente"));
    UpSmallButton *AnnulBouton  = new UpSmallButton(tr("Annuler"));
    UpSmallButton *NoBouton     = new UpSmallButton(tr("Fermer\nle dossier"));
    UpSmallButton *ReBouton     = new UpSmallButton(tr("Retour\nà l'accueil"));

    msgbox.addButton(AnnulBouton,   UpSmallButton::CANCELBUTTON);
    msgbox.addButton(OKBouton,      UpSmallButton::STARTBUTTON);
    msgbox.addButton(NoBouton,      UpSmallButton::CLOSEBUTTON);
    msgbox.exec();
    if (msgbox.clickedButton() == NoBouton)                                                         // Fermer le dossier
    {
        QString requete =   "DELETE FROM " NOM_TABLE_SALLEDATTENTE
                " WHERE idPat = '" + QString::number(gidPatient) + "'";
        QSqlQuery supprimePatSalDatQuery (requete,db);
        proc->TraiteErreurRequete(supprimePatSalDatQuery,requete, tr("Impossible de supprimer ce patient de la salle d'attente!"));
    }
    else if (msgbox.clickedButton() == OKBouton)                                                    // Garder le dossier en salle d'attente
    {
        QString Message(""), Motif(""), idUser ("");
        QString req = "select Motif, Message from " NOM_TABLE_SALLEDATTENTE " where idPat = " + QString::number(gidPatient);
        QSqlQuery quer(req,db);
        QStringList llist;
        if (quer.size()>0)
        {
            quer.first();
            Motif = quer.value(0).toString();
            Message = quer.value(1).toString();
            if (Motif=="")
            {
                llist = MotifMessage(Motif, Message);
                if (llist.isEmpty())
                    return false;
                Motif   = llist.at(0);
                Message = llist.at(1);
                idUser  = llist.at(3);
            }
            QString saldatrequete =   "SELECT idPat FROM " NOM_TABLE_SALLEDATTENTE " WHERE idPat = " + QString::number(gidPatient);
            QSqlQuery SalDatQuery(saldatrequete,db);
            proc->TraiteErreurRequete(SalDatQuery,saldatrequete,tr("Impossible de trouver le dossier dans la salle d'attente!"));
            if (SalDatQuery.size() == 0)
            {
                saldatrequete =   "INSERT INTO " NOM_TABLE_SALLEDATTENTE
                        " (idPat, Statut, HeureArrivee, Motif, Message)"
                        " VALUES (" + QString::number(gidPatient) + ",'"
                                    ARRIVE "','" +
                                    QTime::currentTime().toString("hh:mm") + "','" +
                                    Motif + "','" +
                                    proc->CorrigeApostrophe(Message) + "', " +
                                    idUser +
                                    ")";
            }
            else
            {
                saldatrequete = "UPDATE " NOM_TABLE_SALLEDATTENTE
                                " SET STATUT = '" ARRIVE "',"
                                " idUserEnCoursExam = null,"
                                " PosteExamen = null,"
                                " Motif = '" + Motif + "',"
                                " Message = '" + proc->CorrigeApostrophe(Message) + "'"
                                " WHERE idPat = " + QString::number(gidPatient);
            }
            //proc->Edit(saldatrequete);
            QSqlQuery ModifSalDatQuery(saldatrequete,db);
            proc->TraiteErreurRequete(ModifSalDatQuery,saldatrequete,"");
        }
        else
            a = Slot_InscritEnSalDat(gidPatient);
    }
    else if (msgbox.clickedButton() == ReBouton)                                                         // Fermer le dossier
    {
        QString saldatrequete   = "UPDATE " NOM_TABLE_SALLEDATTENTE " SET Statut = '" RETOURACCUEIL
                                  "', HeureStatut = '" + QTime::currentTime().toString("hh:mm") +
                                  "', idUserEnCoursExam = null"
                                  ", PosteExamen = null"
                                  ", idActeAPayer = " + QString::number(gidActe);
        saldatrequete           += " WHERE idPat = '" + QString::number(gidPatient) + "'";
        QString MsgErreur       = tr("Impossible de modifier le statut du dossier en salle d'attente!");
        //qDebug() << saldatrequete;
        QSqlQuery ModifSalDatQuery(saldatrequete,db);
        proc->TraiteErreurRequete(ModifSalDatQuery,saldatrequete,MsgErreur);
        proc->UpdVerrouSalDat();
    }
    else a = false;                                                                                 // Annuler et revenir au dossier
    if (a) gidPatient = 0;
    proc->UpdVerrouSalDat();

    delete NoBouton;
    delete AnnulBouton;
    delete OKBouton;
    delete ReBouton;
    return a;
}

/*-----------------------------------------------------------------------------------------------------------------
-- Flag pour signifier aux utilisateurs de mettre à jour leur médecin traitant ------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void Rufus::FlagMetAjourMG()
{
    gflagMG = proc->MAJflagMG();

    ReconstruitListesCorresp();

    // on resynchronise l'affichage du combobox au besoin
    if (ui->tabWidget->indexOf(ui->tabDossier) > -1)
    {
        QString req = "select idcormedmg from " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS " where idpat = " + QString::number(gidPatient);
        QSqlQuery quer(req,db);
        if (quer.seek(0))
        {
            ui->MGupComboBox->setCurrentIndex(ui->MGupComboBox->findData(quer.value(0).toInt()));
            ui->MGupComboBox->setImmediateToolTip(CalcToolTipCorrespondant(quer.value(0).toInt()));
        }
        else
        {
            ui->MGupComboBox->setCurrentIndex(-1);
            ui->MGupComboBox->setImmediateToolTip("");
        }
        req = "select idcormedspe1 from " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS " where idpat = " + QString::number(gidPatient);
        QSqlQuery quer1(req,db);
        if (quer1.seek(0))
        {
            ui->AutresCorresp1upComboBox->setCurrentIndex(ui->AutresCorresp1upComboBox->findData(quer1.value(0).toInt()));
            ui->AutresCorresp1upComboBox->setImmediateToolTip(CalcToolTipCorrespondant(quer1.value(0).toInt()));
        }
        else
        {
            ui->AutresCorresp1upComboBox->setCurrentIndex(-1);
            ui->AutresCorresp1upComboBox->setImmediateToolTip("");
        }
        req = "select idcormedspe2 from " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS " where idpat = " + QString::number(gidPatient);
        QSqlQuery quer2(req,db);
        if (quer2.seek(0))
        {
            ui->AutresCorresp2upComboBox->setCurrentIndex(ui->AutresCorresp2upComboBox->findData(quer2.value(0).toInt()));
            ui->AutresCorresp2upComboBox->setImmediateToolTip(CalcToolTipCorrespondant(quer2.value(0).toInt()));
        }
        else
        {
            ui->AutresCorresp2upComboBox->setCurrentIndex(-1);
            ui->AutresCorresp2upComboBox->setImmediateToolTip("");
        }
        Slot_OKModifierTerrain();
    }
}

/*-----------------------------------------------------------------------------------------------------------------
-- Flag pour signifier aux utilisateurs de mettre à jour leurs TreeView ----------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void Rufus::FlagMetAjourTreeView()
{
    Remplir_ListePatients_TableView(grequeteListe,"","");
    gflagPatients = proc->MAJflagPatients();
    RecaleTableView(gidPatient);
    CalcNbDossiers();
}


// ------------------------------------------------------------------------------------------
// Modifier nom et prénom directement dans la fiche
// ------------------------------------------------------------------------------------------

bool Rufus::IdentificationPatient(QString mode, int idPat)
{
    int idPatAPasser = idPat;
    QString NomPat, PrenomPat, DDNPat;

    Dlg_IdentPatient           = new dlg_identificationpatient(&mode, &idPatAPasser, proc, this);
    if (mode == "Copie")
    {
        QString req = "SELECT pat.idPat, PatNom, PatAdresse1, PatAdresse2, PatAdresse3, PatCodePostal, PatVille, PatTelephone FROM "
                NOM_TABLE_PATIENTS " pat, " NOM_TABLE_DONNEESSOCIALESPATIENTS " don"
                " WHERE pat.idPat = don.idPat and Pat.idPat = " + QString::number(gidARecopier);
        QSqlQuery quer (req,db);
        if (quer.size() > 0)
        {
            quer.first();
            Dlg_IdentPatient->ui->NomlineEdit->setText(quer.value(1).toString());
            Dlg_IdentPatient->ui->Adresse1lineEdit->setText(quer.value(2).toString());
            Dlg_IdentPatient->ui->Adresse2lineEdit->setText(quer.value(3).toString());
            Dlg_IdentPatient->ui->Adresse3lineEdit->setText(quer.value(4).toString());
            Dlg_IdentPatient->CPlineEdit->setText(quer.value(5).toString());
            Dlg_IdentPatient->VillelineEdit->setText(quer.value(6).toString());
            Dlg_IdentPatient->ui->TellineEdit->setText(quer.value(7).toString());
            Dlg_IdentPatient->ui->idPatientlabel->setVisible(false);
            Dlg_IdentPatient->ui->Createurlabel->setVisible(false);
            req = "select cornom, corprenom from " NOM_TABLE_CORRESPONDANTS ", " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS
                  " where idcor = idcormedmg"
                  " and idpat = " + QString::number(gidARecopier);
            QSqlQuery ListMGQuery(req,db);
            if (ListMGQuery.size() > 0)
            {
                ListMGQuery.first();
                int e = Dlg_IdentPatient->ui->MGupComboBox->findText(ListMGQuery.value(0).toString() + " " + ListMGQuery.value(1).toString());
                if (e >-1 && e<Dlg_IdentPatient->ui->MGupComboBox->count())
                    Dlg_IdentPatient->ui->MGupComboBox->setCurrentIndex(e);
            }
        }
    }

    if (mode == "Creation")
    {
        Dlg_IdentPatient->ui->NomlineEdit->setEnabled(false);
        Dlg_IdentPatient->ui->PrenomlineEdit->setEnabled(false);
        Dlg_IdentPatient->ui->DDNdateEdit->setEnabled(false);
    }
    if (mode == "Modification")
    {
        Dlg_IdentPatient->ui->DDNdateEdit->setEnabled(false);
        Dlg_IdentPatient->ui->ModifierDDNupPushButton->setVisible(true);
    }


    //AA FICHE ACCEPTEE
    //*************************************************************************
    int a = Dlg_IdentPatient->exec();
    if (Dlg_IdentPatient->ReconstruireListMG)            FlagMetAjourMG();
    if (a > 0)
    {
        NomPat      = Dlg_IdentPatient->ui->NomlineEdit->text();
        PrenomPat   = Dlg_IdentPatient->ui->PrenomlineEdit->text();
        DDNPat      = Dlg_IdentPatient->ui->DDNdateEdit->date().toString("yyyy-MM-dd");
        gSexePat = "";
        if (Dlg_IdentPatient->ui->MradioButton->isChecked()) gSexePat = "M";
        if (Dlg_IdentPatient->ui->FradioButton->isChecked()) gSexePat = "F";

        //A MODE MODIFICATION
        //*************************************************************************
        if (mode == "Modification")
        {
            //            Mise à jour patients
            QString req = "Select PatNom, PatPrenom, PatDDN from " NOM_TABLE_PATIENTS " where idPat = " + QString::number(idPat);
            QSqlQuery querc (req,db);

            QString patreq =    "UPDATE " NOM_TABLE_PATIENTS
                                " SET PatNom = '" + proc->CorrigeApostrophe(NomPat) +
                                "', PatPrenom = '" + proc->CorrigeApostrophe(PrenomPat) +
                                "', PatDDN = '" + DDNPat;
            if (gSexePat != "")
                patreq +=       "', Sexe = '" + gSexePat;
            patreq +=           "' WHERE idPat = " + QString::number(idPat);
            QSqlQuery PatQuery (patreq,db);
            proc->TraiteErreurRequete(PatQuery,patreq,tr("Impossible d'écrire dans la table PATIENTS"));

            //ON VÉRIFIE QU'IL Y A BIEN UN ENREGISTREMENT POUR CE PATIENT DANS LA TABLE DONNEESSOCIALES SINON ON LE CRÈE
            req = "select idpat from " NOM_TABLE_DONNEESSOCIALESPATIENTS " where idpat = " + QString::number(idPat);
            QSqlQuery qer(req,db);
            if (qer.size() == 0)
            {
                req =   "INSERT INTO " NOM_TABLE_DONNEESSOCIALESPATIENTS " (idPat) VALUES ('" + QString::number(idPat) + "')";
                QSqlQuery (req,db);
            }

            //            Mise à jour de donneessocialespatients
            QString requete =   "UPDATE " NOM_TABLE_DONNEESSOCIALESPATIENTS
                    " SET PatAdresse1 = '" + proc->CorrigeApostrophe(Dlg_IdentPatient->ui->Adresse1lineEdit->text()) +
                    "', PatAdresse2 = '" + proc->CorrigeApostrophe(Dlg_IdentPatient->ui->Adresse2lineEdit->text()) +
                    "', PatAdresse3 = '" + proc->CorrigeApostrophe(Dlg_IdentPatient->ui->Adresse3lineEdit->text()) +
                    "', PatCodePostal = '" + Dlg_IdentPatient->CPlineEdit->text() + "'";
            if (!Dlg_IdentPatient->ui->NNIlineEdit->text().isEmpty())
                requete += ", PatNNI = " + Dlg_IdentPatient->ui->NNIlineEdit->text();
            requete +=
                    ", PatVille = '" + proc->CorrigeApostrophe(Dlg_IdentPatient->VillelineEdit->text()).left(70) +
                    "', PatTelephone = '" + Dlg_IdentPatient->ui->TellineEdit->text() +
                    "', PatPortable = '" + Dlg_IdentPatient->ui->PortablelineEdit->text() +
                    "', PatMail = '" + Dlg_IdentPatient->ui->MaillineEdit->text() +
                    "', PatProfession = '" + proc->CorrigeApostrophe(Dlg_IdentPatient->ui->ProfessionlineEdit->text()) + "'";
            if (Dlg_IdentPatient->ui->ALDcheckBox->isChecked())
                requete += ", PatALD = 1";
            else
                requete += ", PatALD = null";
            if (Dlg_IdentPatient->ui->CMUcheckBox->isChecked())
                requete += ", PatCMU = 1";
            else
                requete += ", PatCMU = null";
            requete += " WHERE idPat = " + QString::number(idPat);

            QSqlQuery MAJSocialQuery (requete,db);
            proc->TraiteErreurRequete(MAJSocialQuery,requete,tr("Impossible d'écrire dans la table des données sociales"));

            //ON VÉRIFIE QU'IL Y A BIEN UN ENREGISTREMENT POUR CE PATIENT DANS LA TABLE RENSEIGNEMENTSMEDICAUXPATIENTS SINON ON LE CRÈE
            int e = Dlg_IdentPatient->ui->MGupComboBox->currentData().toInt();
            req = "select idpat from " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS " where idpat = " + QString::number(idPat);
            QSqlQuery quer(req,db);
            if (e>-1)
            {
                if (quer.size() == 0)
                    req =   "INSERT INTO " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS
                            " (idPat, IDCORMEDMG) VALUES (" + QString::number(idPat) + "," + QString::number(e) + ")";
                else
                    req = "update " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS " set idcormedmg = " + QString::number(e)
                            + " where idpat = " + QString::number(idPat);
                QSqlQuery (req,db);
            }
            else if (quer.size() >0)
                QSqlQuery ("update " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS " set idcormedmg = null where idpat = " + QString::number(idPat),db);

            //          Mise à jour de l'affichage si le dossier modifié est le dossier en cours
            if (idPat == gidPatient)
            {
                gNomPatient     = NomPat;
                gPrenomPatient  = PrenomPat;
                gDDNPatient     = Dlg_IdentPatient->ui->DDNdateEdit->date();
                gCMUPatient     = Dlg_IdentPatient->ui->CMUcheckBox->isChecked();
                QString html, img, Age;
                QMap<QString,QVariant>  AgeTotal;
                AgeTotal        = proc->CalculAge(gDDNPatient, QDate::currentDate(), gSexePat);
                img             = AgeTotal["Icone"].toString();
                Age             = AgeTotal["Total"].toString();
                QIcon icon      = proc->CalcIconAge(img);

                html =
                        "<html>"
                        "<head>"
                        "<style type=\"text/css\">"
                        "  p.p1 {font:16px; margin: 0px 0px 10px 100px;}"
                        "  p.p10 {font:16px; margin: 10px 0px 0px 100px;}"
                        "  p.p2 {margin: 0px 0px 0px 100px;}"
                        "  p.p3 {margin: 10px 0px 0px 100px;}"
                        "  p.p4 {margin: 10px 0px 10px 100px;}"
                        "  .image {position:absolute; top:100px; left:10px; float: left;}"
                        "</style>"
                        "</head>"
                        "<body LANG=\"fr-FR\" DIR=\"LTR\">";
                if (img != "")
                    html += "<img class=\"image\" src=\"://" + img + ".png\" WIDTH=\"100\" HEIGHT=\"100\" BORDER=\"10\" />";                            //Icone
                html += "<p class=\"p10\"><b>" + NomPat + " " + PrenomPat + "</b></p>";                                                                 //Nom Prenom
                html += "<p class=\"p1\"><b>" + Age + "</b> (" + gDDNPatient.toString(tr("d MMM yyyy")) + ")</p>";                                          //DDN
                if (Dlg_IdentPatient->ui->Adresse1lineEdit->text() != "")
                    html += "<p class=\"p2\">" + Dlg_IdentPatient->ui->Adresse1lineEdit->text() + "</p>";                                                 //Adresse1
                if (Dlg_IdentPatient->ui->Adresse2lineEdit->text() != "")
                    html += "<p class=\"p2\">" + Dlg_IdentPatient->ui->Adresse2lineEdit->text() + "</p>";                                                 //Adresse2
                if (Dlg_IdentPatient->ui->Adresse3lineEdit->text() != "")
                    html += "<p class=\"p2\">" + Dlg_IdentPatient->ui->Adresse3lineEdit->text() + "</p>";                                                 //Adresse3
                if (Dlg_IdentPatient->CPlineEdit->text() != "")
                {
                    html += "<p class=\"p2\">" + Dlg_IdentPatient->CPlineEdit->text() +
                            " " + Dlg_IdentPatient->VillelineEdit->text() + "</p>";                                                                   //CP + ville
                }
                else
                    if (Dlg_IdentPatient->VillelineEdit->text() != "")
                        html += "<p class=\"p2\">" + Dlg_IdentPatient->VillelineEdit->text() + "</p>";                                                //Ville
                if (Dlg_IdentPatient->ui->TellineEdit->text() != "")
                    html += "<p class=\"p3\">" + tr("Tél.") + "\t" + Dlg_IdentPatient->ui->TellineEdit->text() + "</p>";                                                //Tél
                if (Dlg_IdentPatient->ui->PortablelineEdit->text() != "")
                    html += "<p class=\"p2\">" + tr("Portable") + "\t" + Dlg_IdentPatient->ui->PortablelineEdit->text() + "</p>";                                       //Portable
                if (Dlg_IdentPatient->ui->MaillineEdit->text() != "")
                    html += "<p class=\"p3\">" + tr("Mail") + "\t" + Dlg_IdentPatient->ui->MaillineEdit->text() + "</p>";                                               //Mail
                if (Dlg_IdentPatient->ui->NNIlineEdit->text() > 0)
                    html += "<p class=\"p2\">" + tr("NNI") + "\t" + Dlg_IdentPatient->ui->NNIlineEdit->text() + "</p>";                                                 //NNI
                if (Dlg_IdentPatient->ui->ProfessionlineEdit->text() != "")
                    html += "<p class=\"p3\">" + Dlg_IdentPatient->ui->ProfessionlineEdit->text() + "</p>";                                               //Profession
                if (Dlg_IdentPatient->ui->ALDcheckBox->isChecked() || Dlg_IdentPatient->ui->CMUcheckBox->isChecked())
                {
                    html += "<p class=\"p3\"><td width=\"60\">";
                    if (Dlg_IdentPatient->ui->ALDcheckBox->isChecked())
                        html += "<font size = \"5\"><font color = \"red\"><b>ALD</b></font>";                                                              //ALD
                    if (Dlg_IdentPatient->ui->CMUcheckBox->isChecked())
                        html += "</td><td width=\"60\"><font size = \"5\"><font color = \"blue\"><b>CMU</b><\font>";                                       //CMU
                    html += "</td></p>";
                }

                html += "</body></html>";
                //proc->EcritDansUnFichier(QDir::homePath()+ "/Documents/test.txt", html);
                ui->IdentPatienttextEdit->setHtml(html);

                ui->tabWidget->setTabIcon(ui->tabWidget->indexOf(ui->tabDossier),icon);
                ui->tabWidget->setTabText(ui->tabWidget->indexOf(ui->tabDossier) ,NomPat + " " + PrenomPat);
                int e = Dlg_IdentPatient->ui->MGupComboBox->currentData().toInt();
                if (ui->MGupComboBox->currentData().toInt()!=e)
                {
                    ui->MGupComboBox->setCurrentIndex(ui->MGupComboBox->findData(e));
                    Slot_OKModifierTerrain();
                }
                QMap<QString,QVariant>  NewAge = proc->CalculAge(gDDNPatient, ui->ActeDatedateEdit->date());
                ui->AgelineEdit->setText(NewAge["Total"].toString());
            }
            proc->UpdVerrouSalDat();
            if (querc.size() > 0)
            {
                QString NomPatDepart, PrenomPatDepart, DDNPatDepart;
                querc.first();
                NomPatDepart    = querc.value(0).toString();
                PrenomPatDepart = querc.value(1).toString();
                DDNPatDepart    = querc.value(2).toDate().toString("yyyy-MM-dd");
                if (NomPatDepart != NomPat || PrenomPatDepart != PrenomPat || DDNPatDepart != DDNPat)
                    FlagMetAjourTreeView();
            }
            delete Dlg_IdentPatient;
            return true;
        }
        //A FIN MODE MODIFICATION
        //*************************************************************************

        //B MODE CREATION
        //*************************************************************************
        if (mode == "Creation")
        {
            //1 - Mise à jour patients
            if (gSexePat != "")
            {
                QString requete =   "UPDATE " NOM_TABLE_PATIENTS " SET Sexe = '" + gSexePat + "' WHERE idPat = " + QString::number(idPat);
                QSqlQuery MAJPatientQuery (requete,db);
                proc->TraiteErreurRequete(MAJPatientQuery,requete,"Impossible d'écrire dans la table des patients");
            }
            //2 - Mise à jour de donneessocialespatients
            QString requete =   "UPDATE " NOM_TABLE_DONNEESSOCIALESPATIENTS
                    " SET PatAdresse1 = '" + proc->CorrigeApostrophe(Dlg_IdentPatient->ui->Adresse1lineEdit->text()) +
                    "', PatAdresse2 = '" + proc->CorrigeApostrophe(Dlg_IdentPatient->ui->Adresse2lineEdit->text()) +
                    "', PatAdresse3 = '" + proc->CorrigeApostrophe(Dlg_IdentPatient->ui->Adresse3lineEdit->text()) +
                    "', PatCodePostal = '" + Dlg_IdentPatient->CPlineEdit->text() + "'";
            if (!Dlg_IdentPatient->ui->NNIlineEdit->text().isEmpty())
                requete += ", PatNNI = " + Dlg_IdentPatient->ui->NNIlineEdit->text();
            requete +=
                    ", PatVille = '" + proc->CorrigeApostrophe(Dlg_IdentPatient->VillelineEdit->text()).left(70) +
                    "', PatTelephone = '" + Dlg_IdentPatient->ui->TellineEdit->text() +
                    "', PatPortable = '" + Dlg_IdentPatient->ui->PortablelineEdit->text() +
                    "', PatMail = '" + Dlg_IdentPatient->ui->MaillineEdit->text() +
                    "', PatProfession = '" + proc->CorrigeApostrophe(Dlg_IdentPatient->ui->ProfessionlineEdit->text()) + "'";
            if (Dlg_IdentPatient->ui->ALDcheckBox->isChecked())
                requete += ", PatALD = 1";
            else
                requete += ", PatALD = null";
            if (Dlg_IdentPatient->ui->CMUcheckBox->isChecked())
                requete += ", PatCMU = 1";
            else
                requete += ", PatCMU = null";
            requete += " WHERE idPat = " + QString::number(idPat);
            QSqlQuery MAJSocialQuery (requete,db);
            proc->TraiteErreurRequete(MAJSocialQuery,requete, tr("Impossible d'écrire dans la table des données sociales"));
            //2 - Mise à jour de medecin traitant
            int e = Dlg_IdentPatient->ui->MGupComboBox->currentData().toInt();
            ui->MGupComboBox->setCurrentIndex(ui->MGupComboBox->findData(e));
            requete =   "INSERT INTO " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS
                    " (idPat, idCorMedMG) VALUES(" + QString::number(idPat) + "," + QString::number(e) + ")";
            QSqlQuery MAJMGQuery (requete,db);
            proc->TraiteErreurRequete(MAJMGQuery,requete, tr("Impossible d'enregistrer le médecin traitant"));

            delete  Dlg_IdentPatient;
            return true;
        }
        //B FIN MODE CREATION
        //*************************************************************************

        //A MODE COPIE
        //*************************************************************************
        if (mode == "Copie")
        {
            idPat = Dlg_IdentPatient->gidPatient;
            //            Mise à jour patients
            QString patreq =    "UPDATE " NOM_TABLE_PATIENTS
                    " SET PatNom = '" + proc->CorrigeApostrophe(NomPat) +
                    "', PatPrenom = '" + proc->CorrigeApostrophe(PrenomPat) +
                    "', PatDDN = '" + DDNPat;
            if (gSexePat != "")
                patreq +=   "', Sexe = '" + gSexePat;
            patreq +=           "' WHERE idPat = " + QString::number(idPat);
            QSqlQuery PatQuery (patreq,db);
            proc->TraiteErreurRequete(PatQuery,patreq, tr("Impossible d'écrire dans la table des données sociales"));

            // Mise à jour de donneessocialespatients
            QString requete =   "UPDATE " NOM_TABLE_DONNEESSOCIALESPATIENTS
                    " SET PatAdresse1 = '" + proc->CorrigeApostrophe(Dlg_IdentPatient->ui->Adresse1lineEdit->text()) +
                    "', PatAdresse2 = '" + proc->CorrigeApostrophe(Dlg_IdentPatient->ui->Adresse2lineEdit->text()) +
                    "', PatAdresse3 = '" + proc->CorrigeApostrophe(Dlg_IdentPatient->ui->Adresse3lineEdit->text()) +
                    "', PatCodePostal = '" + Dlg_IdentPatient->CPlineEdit->text() + "'";
            if (!Dlg_IdentPatient->ui->NNIlineEdit->text().isEmpty())
                requete += ", PatNNI = " + Dlg_IdentPatient->ui->NNIlineEdit->text();
            requete +=
                    ", PatVille = '" + proc->CorrigeApostrophe(Dlg_IdentPatient->VillelineEdit->text()).left(70) +
                    "', PatTelephone = '" + Dlg_IdentPatient->ui->TellineEdit->text() +
                    "', PatPortable = '" + Dlg_IdentPatient->ui->PortablelineEdit->text() +
                    "', PatMail = '" + Dlg_IdentPatient->ui->MaillineEdit->text() +
                    "', PatProfession = '" + proc->CorrigeApostrophe(Dlg_IdentPatient->ui->ProfessionlineEdit->text()) + "'";
            if (Dlg_IdentPatient->ui->ALDcheckBox->isChecked())
                requete += ", PatALD = 1";
            else
                requete += ", PatALD = null";
            if (Dlg_IdentPatient->ui->CMUcheckBox->isChecked())
                requete += ", PatCMU = 1";
            else
                requete += ", PatCMU = null";
            requete += " WHERE idPat = " + QString::number(idPat);

            QSqlQuery MAJSocialQuery (requete,db);
            proc->TraiteErreurRequete(MAJSocialQuery,requete, tr("Impossible d'écrire dans la table des données sociales"));

            // on met à jour les atcdts familiaux
            QString req = "select RMPAtcdtsFamiliaux from " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS " where idPat = " + QString::number(gidARecopier);
            QSqlQuery quer(req,db);
            if (quer.size() > 0)
            {
                quer.first();
                QString atcdts = quer.value(0).toString();
                QString insreq = "insert into " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS " (idPat, RMPAtcdtsFamiliaux) VALUES ('" +
                        QString::number(idPat) + "', '" + atcdts + "')";
                QSqlQuery (insreq,db);
            }
            // Mise à jour du medecin traitant
            int e = Dlg_IdentPatient->ui->MGupComboBox->currentData().toInt();
             req = "select idpat from " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS " where idpat = " + QString::number(idPat);
            QSqlQuery quer2(req,db);
            if (quer2.size() == 0)
                req = "INSERT INTO " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS " (idPat, idCorMedMG)"
                      " VALUES (" + QString::number(idPat) + "," + QString::number(e) + ")";
            else
                req = "update " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS " set idcormedmg = " + QString::number(e) + " where idpat = " + QString::number(idPat);
            QSqlQuery MAJMGQuery (req,db);
            proc->TraiteErreurRequete(MAJMGQuery,req, tr("Impossible d'enregistrer le médecin traitant"));

            gidPatient = idPat;
            FlagMetAjourTreeView();
            // Si le User est un soignant, on crée d'emblée une consultation et on l'affiche
            if (gUserDroits != SECRETAIRE)
            {
                UpMessageBox msgbox;
                msgbox.setText(tr("Dossier ") + NomPat + " " + PrenomPat + " créé");
                msgbox.setInformativeText(tr("Ouvrir le dossier ou inscrire le dossier en salle d'attente?"));
                msgbox.setIcon(UpMessageBox::Quest);

                UpSmallButton *OKBouton = new UpSmallButton();
                OKBouton->setText(tr("Inscrire le dossier\nen salle d'attente"));
                UpSmallButton *AnnulBouton = new UpSmallButton();
                AnnulBouton->setText(tr("Ouvrir\n le dossier"));
                UpSmallButton *NoBouton = new UpSmallButton();
                NoBouton->setText(tr("Ne rien faire"));
                msgbox.addButton(NoBouton,UpSmallButton::CANCELBUTTON);
                msgbox.addButton(OKBouton, UpSmallButton::STARTBUTTON);
                msgbox.addButton(AnnulBouton, UpSmallButton::CLOSEBUTTON);
                AnnulBouton->setIcon(proc->giconAttente);
                OKBouton->setIcon(proc->giconSortirDossier);
                msgbox.setDefaultButton(NoBouton);
                msgbox.exec();
                if (msgbox.clickedButton() == OKBouton)
                {
                    if (!Slot_InscritEnSalDat(gidPatient))
                        RecaleTableView(gidPatient);
                }
                else if (msgbox.clickedButton() == AnnulBouton)
                {
                    CreerActe(gidPatient);
                    AfficheDossier(gidPatient);
                }
                else
                    RecaleTableView(gidPatient);
                delete NoBouton;
                delete AnnulBouton;
                delete OKBouton;
             }
            else
                Slot_InscritEnSalDat(gidPatient);
            delete Dlg_IdentPatient;
            return true;
        }
        //B FIN MODE COPIE
        //*************************************************************************
    }
    //AA FIN FICHE ACCEPTEE
    //*************************************************************************

    //BB FICHE REFUSEE
    //*************************************************************************
        else  // si la fiche est rejetée
    {
        NomPat      = proc->CorrigeApostrophe(Dlg_IdentPatient->ui->NomlineEdit->text());
        PrenomPat   = proc->CorrigeApostrophe(Dlg_IdentPatient->ui->PrenomlineEdit->text());
        DDNPat      = Dlg_IdentPatient->ui->DDNdateEdit->date().toString("yyyy-MM-dd");
        gSexePat = "";
        if (Dlg_IdentPatient->ui->MradioButton->isChecked()) gSexePat = "M";
        if (Dlg_IdentPatient->ui->FradioButton->isChecked()) gSexePat = "F";
        if (mode == "Copie" && Dlg_IdentPatient->gidPatient > 0)  // il n'y a pas eu de copie parce que le dossssier existait déjà
        {
            ui->CreerNomlineEdit->setText(NomPat);
            ui->CreerPrenomlineEdit->setText(PrenomPat);
            ui->CreerDDNdateEdit->setDate(Dlg_IdentPatient->ui->DDNdateEdit->date());
            ui->tabWidget->setCurrentWidget(ui->tabList);
            ChercheNomFiltre();
        }
        delete Dlg_IdentPatient;
        return false;
    }
    //BB FIN FICHE REFUSEE
    //*************************************************************************
    return true;
}

/*-----------------------------------------------------------------------------------------------------------------
-- Ouvrir la fiche documents ------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
bool   Rufus::Imprimer_Document(QString idUser, QString titre, QString Entete, QString text, QDate date, QString nom, QString prenom,
                                 bool Prescription, bool ALD, bool AvecPrevisu, bool AvecDupli, bool AvecChoixImprimante)
{
    QString     Corps, Pied;;
    QTextEdit   *Etat_textEdit = new QTextEdit;
    bool        AvecNumPage = false;
    bool        aa;

    Entete.replace("{{PRENOM PATIENT}}", (Prescription? prenom        : ""));
    Entete.replace("{{NOM PATIENT}}"   , (Prescription? nom.toUpper() : ""));

    //création du pied
    Pied = proc->ImpressionPied(false, ALD);
    if (Pied == "") return false;

    // creation du corps
    Corps = proc->ImpressionCorps(text, ALD);
    if (Corps == "") return false;
    Etat_textEdit->setHtml(Corps);
    if (Etat_textEdit->toPlainText() == "") return false;

    int TailleEnTete = proc->TailleEnTete();
    if (ALD) TailleEnTete = proc->TailleEnTeteALD();
    aa = proc->Imprime_Etat(Etat_textEdit, Entete, Pied,
                       proc->TaillePieddePage(), TailleEnTete, proc->TailleTopMarge(),
                       AvecDupli, AvecPrevisu, AvecNumPage, AvecChoixImprimante);

    // stockage du document dans la base de donnees - table impressions
    if (aa)
    {
        bool a = true;
        while (a) {
                 // il faut retirer la dernière ligne du html qui contient le retour à la ligne
            int debut = Corps.lastIndexOf("<p");
            int fin   = Corps.lastIndexOf("</p>");
            int longARetirer = fin - debut + 4;
            if (Corps.mid(debut,longARetirer).contains("-qt-paragraph-type:empty;"))
                Corps.remove(debut,longARetirer);
            else a = false;
        }
        Corps.replace("<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">","<p style=\" margin-top:0px; margin-bottom:0px;\">");
        Corps.replace("<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">","<p style=\" margin-top:0px; margin-bottom:0px;\">");

        int idpat = 0;
        if (ui->tabWidget->currentWidget() == ui->tabDossier)
            idpat = gidPatient;
        else
            idpat = gdossierAOuvrir;

        QSqlQuery query = QSqlQuery(db);

        // on doit passer par les bindvalue pour incorporer le bytearray dans la requête
        query.prepare("insert into " NOM_TABLE_IMPRESSIONS " (idUser, idpat, TypeDoc, SousTypeDoc, Titre, TextEntete, TextCorps, TextOrigine,"
                                                           " TextPied, Dateimpression, UserEmetteur, ALD, EmisRecu, FormatDoc, idLieu)"
                                                           " values"
                                                           "(:iduser, :idpat, :typeDoc, :soustypedoc, :titre, :textEntete, :textCorps,"
                                                           " :textOrigine, :textPied, :dateimpression, :useremetteur, :ald, :emisrecu, :formatdoc, :idlieu)");
        query.bindValue(":iduser", idUser);
        query.bindValue(":idpat", QString::number(idpat));
        query.bindValue(":typeDoc", (Prescription? "Prescription" : "Courrier"));
        query.bindValue(":soustypedoc", titre);
        query.bindValue(":titre",titre);
        query.bindValue(":textEntete",Entete);
        query.bindValue(":textCorps", Corps);
        query.bindValue(":textOrigine", text);
        query.bindValue(":textPied", Pied);
        query.bindValue(":dateimpression", date.toString("yyyy-MM-dd") + " " + QTime::currentTime().toString("HH:mm:ss"));
        query.bindValue(":useremetteur", QString::number(gidUser));
        QVariant ALD100 = QVariant(QVariant::String);
        if (ALD) ALD100 = "1";
        query.bindValue(":ald", ALD100);
        query.bindValue(":emisrecu", "0");
        query.bindValue(":formatdoc", (Prescription? PRESCRIPTION : COURRIER));
        query.bindValue(":idlieu", gDataUser["idLieu"].toString());
        if(!query.exec())
            UpMessageBox::Watch(this, tr("Impossible d'enregistrer ce document dans la base!"));
        ui->OuvreDocsExternespushButton->setEnabled(true);
    }
    delete Etat_textEdit;
    return aa;
}

/*-----------------------------------------------------------------------------------------------------------------
-- Initialisation divers (Validator, geometry...etc...) -----------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void Rufus::InitDivers()
{
    MGlineEdit = new UpLineEdit();
    MGlineEdit->setStyleSheet(
    "UpLineEdit {background-color:white; border-style: none;}"
    "UpLineEdit:focus {border-style:none;}");
    MGlineEdit->setMaxLength(90);
    ui->MGupComboBox->setLineEdit(MGlineEdit);

    int a = -3;
    proc->ModifTailleFont(ui->Cotationframe,a);
    proc->ModifTailleFont(ui->Comptaframe,a);
    proc->ModifTailleFont(ui->CreeParlineEdit,a);
    proc->ModifTailleFont(ui->NoActelabel,a);
    proc->ModifTailleFont(ui->InfoPatientframe,a);
    proc->ModifTailleFont(ui->EnregistrePaiementpushButton,a);
    a = -1;
    proc->ModifTailleFont(ui->RefractionpushButton,a);
    proc->ModifTailleFont(ui->TonometriepushButton,a);
    proc->ModifTailleFont(ui->OuvrirDocumentpushButton,a);
    proc->ModifTailleFont(ui->OuvreActesPrecspushButton,a);
    proc->ModifTailleFont(ui->OuvreDocsExternespushButton,a);
    proc->ModifTailleFont(ui->CreerActepushButton,a);
    proc->ModifTailleFont(ui->CreerBOpushButton,a);
    proc->ModifTailleFont(ui->MotsClesLabel,a);
    proc->ModifTailleFont(ui->ModifierCotationActepushButton,a);

    QFont font = qApp->font();
    font.setBold(true);
    ui->ActeDatedateEdit->setFont(font);

    AutresCorresp1LineEdit = new UpLineEdit();
    AutresCorresp1LineEdit->setStyleSheet(
    "UpLineEdit {background-color:white; border-style: none;}"
    "UpLineEdit:focus {border-style:none;}");
    AutresCorresp1LineEdit->setMaxLength(90);
    ui->AutresCorresp1upComboBox->setLineEdit(AutresCorresp1LineEdit);

    AutresCorresp2LineEdit = new UpLineEdit();
    AutresCorresp2LineEdit->setStyleSheet(
    "UpLineEdit {background-color:white; border-style: none;}"
    "UpLineEdit:focus {border-style:none;}");
    AutresCorresp2LineEdit->setMaxLength(90);
    ui->AutresCorresp2upComboBox->setLineEdit(AutresCorresp2LineEdit);

    ui->ActeCotationcomboBox->lineEdit()->setStyleSheet(
    "QLineEdit {background-color:white; border-style: none;}"
    "QLineEdit:focus {border-style:none;}");
    ui->ActeCotationcomboBox->lineEdit()->setMaxLength(20);
    //ui->ActeCotationcomboBox->lineEdit()->setValidator(new QRegExpValidator(proc->getrxCot(),this));
    ui->ActeCotationcomboBox->lineEdit()->setFont(ui->ActeMontantlineEdit->font());
    ui->ActeCotationcomboBox->setFont(ui->ActeMontantlineEdit->font());
    QString req = "select typeacte as code from " NOM_TABLE_COTATIONS
                  " union "
                  " select codeccam as code from " NOM_TABLE_CCAM
                  " order by code asc";
    QSqlQuery listactquer(req,db);
    QStringList listcodes;
    for (int i=0; i<listactquer.size(); i++)
    {
        listactquer.seek(i);
        listcodes << listactquer.value(0).toString();
    }
    QCompleter *comp = new QCompleter(listcodes);
    //comp->setCompletionMode(QCompleter::InlineCompletion);
    comp->setCaseSensitivity(Qt::CaseInsensitive);
    comp->popup()->setFont(ui->ActeMontantlineEdit->font());
    comp->setMaxVisibleItems(5);
    connect(comp,   SIGNAL(activated(QString)), this, SLOT(Slot_RetrouveMontantActe()));


    ui->ActeCotationcomboBox->lineEdit()->setCompleter(comp);

    QDoubleValidator *val = new QDoubleValidator(this);
    val->setDecimals(2);
    ui->ActeMontantlineEdit->setValidator(val);
    ui->PayelineEdit->setValidator(val);
    ui->TabaclineEdit->setValidator(new QRegExpValidator(proc->getrxTabac(),this));
    MGlineEdit->setValidator(new QRegExpValidator(proc->getrx(),this));
    AutresCorresp1LineEdit->setValidator(new QRegExpValidator(proc->getrx(),this));
    AutresCorresp2LineEdit->setValidator(new QRegExpValidator(proc->getrx(),this));
    ui->tabWidget->setTabText(0,tr("Liste des patients"));

    gdateParDefaut = QDate::fromString("2000-01-01", "yyyy-MM-dd");
    ui->ActeMontantlineEdit->setAlignment(Qt::AlignRight);
    ui->PayelineEdit->setAlignment(Qt::AlignRight);
    ui->CreerDDNdateEdit->setDate(gdateParDefaut);
    ui->ActeDatedateEdit->setMaximumDate(QDate::currentDate());
    ui->CreerDossierframe->setGeometry(10,170,356,170);
    ui->tabWidget->setIconSize(QSize(25,25));
    ui->VitaleupPushButton->setIconSize(QSize(120,100));

    ui->SalDatlabel     ->setPixmap(QPixmap("://waiting_room.png")          .scaled(QSize(60,60), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->Bureauxlabel    ->setPixmap(QPixmap("://AVTest1.png")               .scaled(QSize(100,100), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->Accueillabel    ->setPixmap(QPixmap("://reception_icon.png")        .scaled(QSize(70,70), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->PatientVuslabel ->setPixmap(QPixmap("://list_all_participants.png") .scaled(QSize(60,60), Qt::KeepAspectRatio, Qt::SmoothTransformation));

    ui->FiltrecheckBox  ->setChecked(true);
    ui->FiltrecheckBox  ->setEnabled(proc->getModeConnexion() != Procedures::Distant);

    ui->CreerDDNdateEdit->setDateRange(QDate::currentDate().addYears(-105),QDate::currentDate());

    if (gUserFonction == tr("Médecin") || gUserFonction == tr("Orthoptiste"))
        ReconstruitListesActes();
    ReconstruitListesCorresp();
    gflagPatients   = proc->GetflagPatients();
    gflagMG         = proc->GetflagMG();
    gDirSauv        = QDir::homePath() + "/dumps";

    QMenu *trayIconMenu;
    trayIconMenu = new QMenu();

    QAction *pAction_VoirMessages = trayIconMenu->addAction(tr("Voir les messages"));
    connect (pAction_VoirMessages, &QAction::triggered,    [=] {Slot_AfficheMessages();});

    gMessageIcon = new QSystemTrayIcon();
    gMessageIcon->setContextMenu(trayIconMenu);
    gMessageIcon->setIcon(proc->giconPostit);
    connect(gMessageIcon,   &QSystemTrayIcon::messageClicked,   [=] {Slot_AfficheMessages();});

    gMsgRepons = new QDialog();
    gMsgDialog = new QDialog();


    ui->CCAMlinklabel->setText("<a href=\"" LIEN_CCAM "\">CCAM...</a>");
}

/*-----------------------------------------------------------------------------------------------------------------
-- Initialisation des eventfilters --------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void Rufus::InitEventFilters()
{
    // tabListe -----------------------------------------------------
    ui->CreerNomlineEdit                ->installEventFilter(this);
    ui->CreerPrenomlineEdit             ->installEventFilter(this);
    ui->CreerDDNdateEdit                ->installEventFilter(this);
    ui->ChercherDepuisListepushButton   ->installEventFilter(this);
    ui->CreerDossierpushButton          ->installEventFilter(this);
    ui->LFermepushButton                ->installEventFilter(this);

    // tabDossier --------------------------------------------------
    ui->FermepushButton                 ->installEventFilter(this);

    ui->TabaclineEdit                   ->installEventFilter(this);
    MGlineEdit                          ->installEventFilter(this);
    AutresCorresp1LineEdit              ->installEventFilter(this);
    AutresCorresp2LineEdit              ->installEventFilter(this);
    ui->MGupComboBox                    ->installEventFilter(this);
    ui->AutresCorresp1upComboBox        ->installEventFilter(this);
    ui->AutresCorresp2upComboBox        ->installEventFilter(this);
    ui->AtcdtsPersostextEdit            ->installEventFilter(this);
    ui->AtcdtsFamiliauxtextEdit         ->installEventFilter(this);
    ui->TtGeneraltextEdit               ->installEventFilter(this);
    ui->AutresToxiquestextEdit          ->installEventFilter(this);

    ui->AtcdtsOphstextEdit              ->installEventFilter(this);
    ui->TtOphtextEdit                   ->installEventFilter(this);
    ui->ImportanttextEdit               ->installEventFilter(this);
    ui->ResumetextEdit                  ->installEventFilter(this);
    ui->ActeMotiftextEdit               ->installEventFilter(this);
    ui->ActeTextetextEdit               ->installEventFilter(this);
    ui->ActeConclusiontextEdit          ->installEventFilter(this);
    ui->ActeCotationcomboBox            ->installEventFilter(this);
    ui->ActeDatedateEdit                ->installEventFilter(this);
    ui->CourrierAFairecheckBox          ->installEventFilter(this);
    ui->ActeMontantlineEdit             ->installEventFilter(this);

    ui->tabDossier                      ->installEventFilter(this);

    ui->EnregistrePaiementpushButton    ->installEventFilter(this);
}

/*-----------------------------------------------------------------------------------------------------------------
-- Initialisation des Menus -----------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void Rufus::InitMenus()
{
    bool a = (liberal || secretaire);
    actionPaiementTiers             ->setVisible(a || (salarie && !assistant));
    actionPaiementDirect            ->setVisible(a || (salarie && !assistant) || remplacant);
    actionBilanRecettes             ->setVisible(a);
    actionRecettesSpeciales         ->setVisible(a);
    actionJournalDepenses           ->setVisible(a);
    actionGestionComptesBancaires   ->setVisible(liberal);
    actionRemiseCheques             ->setVisible(a);
    actionImpayes                   ->setVisible(a);
    menuComptabilite                ->setVisible(a || (salarie && !assistant) || remplacant);
    actionEnregistrerVideo          ->setVisible(proc->getModeConnexion() != Procedures::Distant);
}

/*-----------------------------------------------------------------------------------------------------------------
    -- Initialisation des TableViews -----------------------------------------------------------------------------------
    -----------------------------------------------------------------------------------------------------------------*/
void Rufus::InitTables()
{
    grequeteListe   = "SELECT IdPat, PatNom, PatPrenom, PatDDN, Sexe FROM " NOM_TABLE_PATIENTS;

    idPatientAuHasard = 0;
    if (Remplir_ListePatients_TableView(grequeteListe,"","") && gNombreDossiers>0)
    {
        srand(time(NULL));
        idPatientAuHasard = rand() % (gNombreDossiers);
    }
    CalcNbDossiers();

    QHBoxLayout *hlay = new QHBoxLayout();
    gSalDatTab  = new QTabBar;
    gSalDatTab  ->setExpanding(false);
    hlay        ->insertWidget(0,gSalDatTab);
    hlay        ->addSpacerItem(new QSpacerItem(5,5,QSizePolicy::Expanding,QSizePolicy::Fixed));
    hlay        ->setContentsMargins(0,0,0,0);
    hlay        ->setSpacing(0);
    connect (gSalDatTab,    SIGNAL(currentChanged(int)),    this,   SLOT (Slot_FiltreSalleDAttente(int)));
    ui->SalDatLayout->insertLayout(1,hlay);
    ui->SalDatWidget->setVisible(false);

    QHBoxLayout *halay = new QHBoxLayout();
    gAccueilTab  = new QTabBar;
    gAccueilTab  ->setExpanding(false);
    halay        ->insertWidget(0,gAccueilTab);
    halay        ->addSpacerItem(new QSpacerItem(5,5,QSizePolicy::Expanding,QSizePolicy::Fixed));
    halay        ->setContentsMargins(0,0,0,0);
    halay        ->setSpacing(0);
    connect (gAccueilTab,    SIGNAL(currentChanged(int)),    this,   SLOT (Slot_FiltreAccueil(int)));
    ui->AccueilLayout->insertLayout(1,halay);
    ui->AccueilWidget->setVisible(false);

    ui->SalleDAttenteupTableWidget->verticalHeader()->setVisible(false);
    ui->SalleDAttenteupTableWidget->setGridStyle(Qt::NoPen);
    ui->SalleDAttenteupTableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->SalleDAttenteupTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->SalleDAttenteupTableWidget->setColumnCount(7);                 // on remplit la liste des patients en cours d'examen
    ui->SalleDAttenteupTableWidget ->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->SalleDAttenteupTableWidget ->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->SalleDAttenteupTableWidget ->verticalHeader()->setVisible(false);
    ui->SalleDAttenteupTableWidget ->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->SalleDAttenteupTableWidget ->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
    ui->SalleDAttenteupTableWidget ->horizontalHeader()->setVisible(true);
    ui->SalleDAttenteupTableWidget->setColumnWidth(0,199);             //Nom Prénom
    ui->SalleDAttenteupTableWidget->setColumnWidth(1,265);             //Statut
    ui->SalleDAttenteupTableWidget->setColumnWidth(2,15);              //Msg
    ui->SalleDAttenteupTableWidget->setColumnWidth(3,49);              //Heure RDV
    ui->SalleDAttenteupTableWidget->setColumnWidth(4,48);              //Motif
    ui->SalleDAttenteupTableWidget->setColumnWidth(5,49);              //Heure arrivée
    ui->SalleDAttenteupTableWidget->setColumnWidth(6,120);              //Heure arrivée
    ui->SalleDAttenteupTableWidget ->setHorizontalHeaderItem(0, new QTableWidgetItem(""));
    ui->SalleDAttenteupTableWidget ->setHorizontalHeaderItem(1, new QTableWidgetItem(""));
    ui->SalleDAttenteupTableWidget ->setHorizontalHeaderItem(2, new QTableWidgetItem(""));
    ui->SalleDAttenteupTableWidget ->setHorizontalHeaderItem(3, new QTableWidgetItem("RDV"));
    ui->SalleDAttenteupTableWidget ->setHorizontalHeaderItem(4, new QTableWidgetItem(""));
    ui->SalleDAttenteupTableWidget ->setHorizontalHeaderItem(5, new QTableWidgetItem("Arr."));
    ui->SalleDAttenteupTableWidget ->setHorizontalHeaderItem(6, new QTableWidgetItem("RDV avec"));
    ui->SalleDAttenteupTableWidget ->FixLargeurTotale();

    ui->scrollArea                 ->setFixedWidth(ui->SalleDAttenteupTableWidget->width());

    QStringList PatsList;
    PatsList << tr("Heure");
    PatsList << tr("Nom Prénom");
    PatsList << tr("Soignant");
    PatsList << tr("Cotation");
    PatsList << tr("Montant");
    PatsList << "idActe";

    ui->AccueilupTableWidget->verticalHeader()->setVisible(false);
    ui->AccueilupTableWidget->setGridStyle(Qt::NoPen);
    ui->AccueilupTableWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->AccueilupTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->AccueilupTableWidget->setColumnCount(7);
    ui->AccueilupTableWidget->setColumnWidth(0,70);     //Heure acte
    ui->AccueilupTableWidget->setColumnWidth(1,249);    //Nom Prenom
    ui->AccueilupTableWidget->setColumnWidth(2,120);    //Soignant
    ui->AccueilupTableWidget->setColumnWidth(3,190);    //Cotation
    ui->AccueilupTableWidget->setColumnWidth(4,115);    //Montant
    ui->AccueilupTableWidget->setColumnWidth(5,0);      //idActe
    ui->AccueilupTableWidget->setColumnWidth(6,0);      //UserParent
    ui->AccueilupTableWidget->verticalHeader()->setVisible(false);
    ui->AccueilupTableWidget->setGridStyle(Qt::NoPen);
    ui->AccueilupTableWidget->setSelectionMode(QAbstractItemView::MultiSelection);
    ui->AccueilupTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->AccueilupTableWidget->setColumnHidden(5,true);
    ui->AccueilupTableWidget->setColumnHidden(6,true);
    ui->AccueilupTableWidget->setHorizontalHeaderLabels(PatsList);

    ui->PatientsVusupTableWidget->verticalHeader()->setVisible(false);
    ui->PatientsVusupTableWidget->setGridStyle(Qt::NoPen);
    ui->PatientsVusupTableWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->PatientsVusupTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->PatientsVusupTableWidget->setColumnCount(5);                     // liste des patients vus ce jour
    ui->PatientsVusupTableWidget->setColumnWidth(0,70);                  //Heure Acte
    ui->PatientsVusupTableWidget->setColumnWidth(1,249);                 //Nom Prénom
    ui->PatientsVusupTableWidget->setColumnWidth(2,120);                 //Paiement
    ui->PatientsVusupTableWidget->setColumnWidth(3,190);                 //Cotation
    ui->PatientsVusupTableWidget->setColumnWidth(4,115);                  //Montant
    ui->PatientsVusupTableWidget->verticalHeader()->setVisible(false);
    ui->PatientsVusupTableWidget->setGridStyle(Qt::NoPen);
    ui->PatientsVusupTableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->PatientsVusupTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->PatientsVusupTableWidget->horizontalHeader()->setVisible(true);
    PatsList.replace(2,tr("Paiement"));
    PatsList.removeLast();
    ui->PatientsVusupTableWidget->setHorizontalHeaderLabels(PatsList);

    ui->SalleDAttenteupTableWidget  ->FixLargeurTotale();
    ui->AccueilupTableWidget        ->FixLargeurTotale();
    ui->PatientsVusupTableWidget    ->FixLargeurTotale();

    Remplir_SalDat();
}

/*-----------------------------------------------------------------------------------------------------------------
-- Initialisation des variables -----------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void Rufus::InitVariables()
{
    gAutorModifConsult  = false;

    gidPatient          = 0;
    gCMUPatient         = false;
    gidActe             = 0;
    nbActes             = 0;
    noActe              = 0;
    gidARecopier        = 0;
    gMode               = Liste;
    gNomPatient         = "";
    gPrenomPatient      = "";
    grequeteListe       = "";
    gNombreDossiers     = 1;
    gdossierAOuvrir     = 0;
    gAffichTotalMessages = true;

    proc->CouleurTitres = "blue";

    ui->AtcdtsOphstextEdit      ->setChampCorrespondant("RMPAtcdtsOphs");
    ui->AtcdtsOphstextEdit      ->setTableCorrespondant(NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS);
    ui->TtOphtextEdit           ->setChampCorrespondant("RMPTtOphs");
    ui->TtOphtextEdit           ->setTableCorrespondant(NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS);
    ui->ImportanttextEdit       ->setChampCorrespondant("Important");
    ui->ImportanttextEdit       ->setTableCorrespondant(NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS);
    ui->ResumetextEdit          ->setChampCorrespondant("Resume");
    ui->ResumetextEdit          ->setTableCorrespondant(NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS);

    ui->AtcdtsPersostextEdit    ->setChampCorrespondant("RMPAtcdtsPersos");
    ui->AtcdtsPersostextEdit    ->setTableCorrespondant(NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS);
    ui->AtcdtsFamiliauxtextEdit ->setChampCorrespondant("RMPAtcdtsFamiliaux");
    ui->AtcdtsFamiliauxtextEdit ->setTableCorrespondant(NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS);
    ui->TtGeneraltextEdit       ->setChampCorrespondant("RMPTtGeneral");
    ui->TtGeneraltextEdit       ->setTableCorrespondant(NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS);
    ui->AutresToxiquestextEdit  ->setChampCorrespondant("Autrestoxiques");
    ui->AutresToxiquestextEdit  ->setTableCorrespondant(NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS);

    MGlineEdit                  ->setTableCorrespondant(NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS);
    MGlineEdit                  ->setChampCorrespondant("idCorMedMG");
    AutresCorresp1LineEdit      ->setTableCorrespondant(NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS);
    AutresCorresp1LineEdit      ->setChampCorrespondant("idCorMedSpe1");
    AutresCorresp2LineEdit      ->setTableCorrespondant(NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS);
    AutresCorresp2LineEdit      ->setChampCorrespondant("idCorMedSpe2");
    ui->TabaclineEdit           ->setTableCorrespondant(NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS);
    ui->TabaclineEdit           ->setChampCorrespondant("Tabac");

    ui->ActeMotiftextEdit       ->setChampCorrespondant("ActeMotif");
    ui->ActeMotiftextEdit       ->setTableCorrespondant(NOM_TABLE_ACTES);
    ui->ActeTextetextEdit       ->setChampCorrespondant("ActeTexte");
    ui->ActeTextetextEdit       ->setTableCorrespondant(NOM_TABLE_ACTES);
    ui->ActeConclusiontextEdit  ->setChampCorrespondant("ActeConclusion");
    ui->ActeConclusiontextEdit  ->setTableCorrespondant(NOM_TABLE_ACTES);

    ui->LListepushButton        ->setEnabled(false);
    ui->CreerDossierpushButton  ->setText(tr("Ouvrir\nle dossier"));
    ui->CreerDossierpushButton  ->setEnabled(false);
    ui->CreerDDNdateEdit        ->setVisible(false);
    ui->DDNlabel                ->setVisible(false);

}

/*-----------------------------------------------------------------------------------------------------------------
-- Recherche d'un dossier -----------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
int Rufus::LectureMesure(QString lidPatient, QString lPatNom, QString lPatPrenom, QString lPatDDN, QString lPatCreeLe, QString lPatCreePar, QString MessageErreur)

{
    QString     Addrequete;

    QString requete = "SELECT  idPat, PatNom, PatPrenom, PatDDN, PatCreeLe, PatCreePar"
            " FROM " NOM_TABLE_PATIENTS ;

    // On relit la mesure après selection dans la liste mesure (reprendre)
    Addrequete = "";
    if (lidPatient.length() > 0 || proc->CorrigeApostrophe(lPatNom).length() > 0 || proc->CorrigeApostrophe(lPatPrenom).length() > 0 || lPatDDN.length() > 0 )
        Addrequete += " WHERE ";
    if (lidPatient.length() > 0 )
        Addrequete += " idPat = '" + lidPatient + "'";
    if (proc->CorrigeApostrophe(lPatNom).length() > 0)
    {
        if (Addrequete != " WHERE ")
            Addrequete += " AND PatNom = '" + proc->CorrigeApostrophe(lPatNom) + "'";
        else
            Addrequete += "PatNom = '" + proc->CorrigeApostrophe(lPatNom) + "'";
    }
    if (proc->CorrigeApostrophe(lPatPrenom).length() > 0)
    {
        if (Addrequete != " WHERE ")
            Addrequete += " AND PatPrenom = '" + proc->CorrigeApostrophe(lPatPrenom) + "'";
        else
            Addrequete += "PatPrenom = '" + proc->CorrigeApostrophe(lPatPrenom) + "'";
    }
    if (lPatDDN.length() > 0)
    {
        if (Addrequete != " WHERE ")
            Addrequete += " AND PatDDN = '" + lPatDDN +"'";
        else
            Addrequete += "PatDDN = '" + lPatDDN + "'";
    }
    if (lPatCreeLe.length() > 0)
    {
        if (Addrequete != " WHERE ")
            Addrequete += " AND PatCreeLe = '" + lPatCreeLe +"'";
        else
            Addrequete += "PatCreeLe = '" + lPatCreeLe + "'";
    }
    if (lPatCreePar.length() > 0)
    {
        if (Addrequete != " WHERE ")
            Addrequete += " AND PatCreePar = '" + lPatCreePar +"'";
        else
            Addrequete += "PatCreePar = '" + lPatCreePar + "'";
    }
    requete += Addrequete;

    QSqlQuery LectureMesureQuery (requete,db);
    if (proc->TraiteErreurRequete(LectureMesureQuery,requete, MessageErreur))
        return -1;
    LectureMesureQuery.first();
    if (LectureMesureQuery.size() == 0)
    { // Aucune mesure trouvee pour ces criteres
        return 0;
    }

    return LectureMesureQuery.value(0).toInt();              // retourne idPatient
}

// ------------------------------------------------------------------------------------------
// Enregistre ou met à jour les coordonnées du médecin traitant
// ------------------------------------------------------------------------------------------
void Rufus::MAJMG(QObject *obj)
{
    UpComboBox* cbox = dynamic_cast<UpComboBox*>(obj);
    if (cbox == NULL) return;
    UpLineEdit *Upline = dynamic_cast<UpLineEdit*>(cbox->lineEdit());
    if (Upline == NULL) return;
    QString anc = cbox->getValeurAvant();
    QString nou = proc->MajusculePremiereLettre(cbox->currentText(),true);
    QString req;
    cbox->setCurrentText(nou);
    int i = cbox->findText(nou, Qt::MatchFixedString);
    if (-1 < i && i < cbox->count())
        return;
    if (nou != "")
    {
        if (nou != anc)
        {
            UpMessageBox msgbox;
            msgbox.setText("Euuhh... " + gUserLogin);
            msgbox.setInformativeText(tr("Correspondant inconnu! Souhaitez-vous l'enregistrer?"));
            msgbox.setIcon(UpMessageBox::Warning);
            UpSmallButton *OKBouton = new UpSmallButton();
            OKBouton->setText(tr("Enregistrer"));
            UpSmallButton *NoBouton = new UpSmallButton();
            NoBouton->setText(tr("Annuler"));
            msgbox.addButton(NoBouton, UpSmallButton::CANCELBUTTON);
            msgbox.addButton(OKBouton, UpSmallButton::STARTBUTTON);
            msgbox.exec();
            if (msgbox.clickedButton() == OKBouton)
            {
                QString typecor;
                if (cbox == ui->MGupComboBox)
                    typecor = "MG";
                else
                    typecor = "TousCor";
                int idcor = EnregistreNouveauCorresp(typecor,cbox->lineEdit()->text());
                if (idcor<0)
                    cbox->setCurrentText(anc);
                else
                {
                    req = "select idpat from " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS " where idpat = " + QString::number(gidPatient);
                    QSqlQuery quer(req,db);
                    if (quer.size() == 0)
                        req =   "INSERT INTO " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS
                                    " (idPat, " + cbox->getChampCorrespondant() + ") VALUES (" + QString::number(gidPatient) + "," + QString::number(idcor) + ")";
                    else
                        req = "update " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS " set " + Upline->getChampCorrespondant() + " = " + QString::number(idcor)
                            + " where idpat = " + QString::number(gidPatient);
                    QSqlQuery (req,db);
                    FlagMetAjourMG();
                }
            }
            else
            {
                int e = cbox->findText(anc);
                if (-1<e && e<cbox->count())
                    cbox->setCurrentText(anc);
                else
                    cbox->setCurrentIndex(-1);
            }
            delete OKBouton;
            delete NoBouton;
            msgbox.close();
        }
    }
    else
    {
        req = "update " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS " set " + Upline->getChampCorrespondant() + " = null where idpat = " + QString::number(gidPatient);
        QSqlQuery (req,db);
        cbox->setToolTip("");
        Slot_OKModifierTerrain();
    }
    //qDebug() << req;
}

// ------------------------------------------------------------------------------------------
// Monte une ligne dans la table
// ------------------------------------------------------------------------------------------

void Rufus::MonteUneLigne()
{
    QModelIndexList listindx = ui->PatientsListeTableView->selectionModel()->selectedIndexes();
    if (listindx.size()==0) return;
    int itrow = listindx.at(0).row();
    if (itrow > 0)
    {
        ui->PatientsListeTableView->selectRow(gListePatientsModel->item(itrow-1)->row());
        ui->PatientsListeTableView->scrollTo(gListePatientsModel->item(itrow-1)->index(),QAbstractItemView::PositionAtCenter);
    }
}


// ------------------------------------------------------------------------------------------
// Monte 20 lignes dans la table
// ------------------------------------------------------------------------------------------

void Rufus::Monte20Lignes()
{
    QModelIndexList listindx = ui->PatientsListeTableView->selectionModel()->selectedIndexes();
    if (listindx.size()==0) return;
    int itrow = listindx.at(0).row();
    if (itrow > 19)
    {
        ui->PatientsListeTableView->selectRow(gListePatientsModel->item(itrow-20)->row());
        ui->PatientsListeTableView->scrollTo(gListePatientsModel->item(itrow-20)->index(),QAbstractItemView::PositionAtCenter);
    }
    else
    {
        ui->PatientsListeTableView->selectRow(0);
        ui->PatientsListeTableView->scrollTo(gListePatientsModel->item(0)->index(),QAbstractItemView::PositionAtTop);
    }
}

/*------------------------------------------------------------------------------------------------------------------------------------
-- Afficher la consultation précédente ou suivante d'un patient ----------------------------------------------------------------------
------------------------------------------------------------------------------------------------------------------------------------*/
bool Rufus::NavigationConsult(int i)
{
    if(!AutorDepartConsult(false)) return false;
    //  Afficher les éléments de la tables Actes
   int idActe = -1;
    QString requete = "SELECT idActe, ActeDate FROM " NOM_TABLE_ACTES " WHERE idPat = '" + QString::number(gidPatient) + "' ORDER BY ActeDate, ActeHeure";
    QSqlQuery NavigationConsultQuery (requete,db);
    if (proc->TraiteErreurRequete(NavigationConsultQuery,requete, tr("Impossible de retrouver les consultations de ce patient!")))
        return false;
    if (NavigationConsultQuery.size() < 2)
        return false;
    NavigationConsultQuery.first();
    for (int a = 0; a < NavigationConsultQuery.size(); a++)
    {
        if (NavigationConsultQuery.value(0).toInt() == gidActe) break;
        NavigationConsultQuery.next();
    }

    if (i == 1)
        if (NavigationConsultQuery.next())          idActe = NavigationConsultQuery.value(0).toInt();
    if (i == -1)
        if (NavigationConsultQuery.previous())      idActe = NavigationConsultQuery.value(0).toInt();
    if (i == 0)
        if (NavigationConsultQuery.first())         idActe = NavigationConsultQuery.value(0).toInt();
    if (i == 100)
        if (NavigationConsultQuery.last())          idActe = NavigationConsultQuery.value(0).toInt();

    if (idActe > -1)
    {
        AfficheActe(idActe);
        return true;
    }
    else
        return false;
    }

/*------------------------------------------------------------------------------------------------------------------------------------
-- Afficher le dossier précédente ou suivant du dossier en cours ---------------------------------------------------------------------
------------------------------------------------------------------------------------------------------------------------------------*/
void Rufus::NavigationDossier(int i)
{
    int idPatient = -1;
    int idPatientDeDepart = gidPatient;
    QString requete;
    bool loop = true;
    bool findetable = false;
    while (loop)
    {
        requete = "SELECT idPat FROM " NOM_TABLE_PATIENTS " ORDER BY PatNom, PatPrenom, PatDDN";
        QSqlQuery   NavigationDossierQuery (requete,db);
        proc->TraiteErreurRequete(NavigationDossierQuery,requete, tr("Impossible de rechercher les autres enregistrements!"));
        if ( NavigationDossierQuery.size() == 0)
            return;
        // On se positionne sur l'enregistrement en cours avec le gIdPatient
        NavigationDossierQuery.first();
        if (NavigationDossierQuery.value(0).toInt() != idPatientDeDepart)
            for (int a = 1; a < NavigationDossierQuery.size(); a++)
            {
                NavigationDossierQuery.next();
                if (NavigationDossierQuery.value(0).toInt() == idPatientDeDepart) break;
            }

        // On défile en avant
        if (i > 0)
        {
            if (NavigationDossierQuery.at() < NavigationDossierQuery.size()-1)
            {
                NavigationDossierQuery.next();
                if (NavigationDossierQuery.at() == NavigationDossierQuery.size()-1) findetable = true;
                idPatient = NavigationDossierQuery.value(0).toInt();
            }
            else return;
        }


        //On défile en arrière
        if (i < 0)
        {
            if (NavigationDossierQuery.at() > 0)
            {
                NavigationDossierQuery.previous();
                if (NavigationDossierQuery.at() == 0) findetable = true;
                idPatient = NavigationDossierQuery.value(0).toInt();
            }
            else return;
        }
        loop = false;
        if (idPatient > -1)  // on a un enregistrement valide à afficher
            // On vérifie qu'il n'est pas verrouillé par un autre utilisateur
        {
            QString blabla = ENCOURSEXAMEN;
            int length = blabla.size();

            requete =   "SELECT idPat, Statut FROM " NOM_TABLE_SALLEDATTENTE
                    " WHERE idUserEnCoursExam != " + QString::number(gidUser) + " AND idPat = " + QString::number(idPatient) + " AND Left(Statut," + QString::number(length) + ") = '" ENCOURSEXAMEN "'";
            QSqlQuery ChercheVerrouQuery (requete,db);
            proc->TraiteErreurRequete(ChercheVerrouQuery,requete,"");
            if (ChercheVerrouQuery.size() > 0)
            {
                idPatientDeDepart = idPatient;
                if (findetable)
                    return;
                else
                    loop = true;
            }
        }
    }
    if (idPatient > -1)
    {
        if(!AutorDepartConsult(true)) return;
        AfficheDossier(idPatient);
    }
}



/*-----------------------------------------------------------------------------------------------------------------
-- Visualiser la fiche dlg_actesprecedents ------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void    Rufus::OuvrirActesPrecedents(int idActeEnCours)
{
    Dlg_ActesPrecs      = new dlg_actesprecedents(gidPatient, idActeEnCours, proc, true, this);
    Dlg_ActesPrecs->setWindowTitle(tr("Consultations précédentes de ") + gNomPatient + " " + gPrenomPatient);
    Dlg_ActesPrecs->show();
    Dlg_ActesPrecs->setWindowIcon(proc->giconLoupe);
}

/*-----------------------------------------------------------------------------------------------------------------
-- Ouvrir la fiche documents ------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void    Rufus::OuvrirDocuments(bool AffichDocsExternes)
{
    QString nom, prenom;
    if (ui->tabWidget->currentWidget() == ui->tabDossier)
    {
        nom         = gNomPatient;
        prenom      = gPrenomPatient;
        Dlg_Docs    = new dlg_documents(gidPatient, nom, prenom, proc, this);
    }
    else
    {
        QString autrerequete = "select PatNom, PatPrenom from " NOM_TABLE_PATIENTS " where idPat = " + QString::number(gdossierAOuvrir);
        QSqlQuery autrequery (autrerequete, db);
        if (proc->TraiteErreurRequete(autrequery,autrerequete,""))
            return;
        if (autrequery.size() == 0)
        {
            UpMessageBox::Watch(this, tr("Pas de consultation enregistrée pour ce patient"));
            return;
        }
        else
        {
            autrequery  .first();
            nom         = autrequery.value(0).toString();
            prenom      = autrequery.value(1).toString();
            Dlg_Docs    = new dlg_documents(gdossierAOuvrir, nom, prenom, proc, this);
        }
    }
    Dlg_Docs->setWindowTitle(tr("Préparer un document pour ") + nom + " " + prenom);
    Dlg_Docs->setWindowIcon(proc->giconLoupe);
    bool aa = true;
    if (Dlg_Docs->exec() > 0)
    {
        int UserEntete = Dlg_Docs->gidUserEntete;
         if (!proc->setDataOtherUser(UserEntete).value("Success").toBool())
            return;
        QString     Entete;
        QDate DateDoc           = Dlg_Docs->ui->dateEdit->date();
        //création de l'entête
        QMap<QString,QString> EnteteMap = proc->ImpressionEntete(DateDoc);

        bool ALD;
        QString imprimante = "";
        for (int k = 0; k < Dlg_Docs->TextDocumentsAImprimerList.size(); k++)
        {
            /* On dispose de 5 QStringList qui décrivent ce q'on doit imprimer pour chaque itération
             * TitreDocumentAImprimerList       -> le titre qui sera inséré dans la fiche docsexternes et dans la conclusion
             * prescriptionAImprimerList        -> précise si le document est une prescription - le formatage n'est pas le même
             * DupliAImprimerList               -> la nécessité ou non d'imprimer un dupli
             * TextDocumentsAImprimerList       -> le corps du document à imprimer
            */
            ALD                     = (Dlg_Docs->ui->ALDcheckBox->checkState() == Qt::Checked) && (Dlg_Docs->PrescriptionAImprimerList.at(k) == "1");
            bool Prescription       = (Dlg_Docs->PrescriptionAImprimerList.at(k) == "1");
            bool AvecDupli          = (Dlg_Docs->DupliAImprimerList.at(k) == "1");
            bool AvecChoixImprimante= (k == 0);                             // s'il y a plusieurs documents à imprimer on détermine l'imprimante pour le premier et on garde ce choix pour les autres
            bool AvecPrevisu        = proc->ApercuAvantImpression();
            QString Titre           = Dlg_Docs->TitreDocumentAImprimerList.at(k);
            QString TxtDocument     = Dlg_Docs->TextDocumentsAImprimerList.at(k);
            Entete = (ALD? EnteteMap.value("ALD") : EnteteMap.value("Norm"));
            if (Entete == "") return;
            Entete.replace("{{TITRE1}}"        , "");
            Entete.replace("{{TITRE}}"         , "");
            Entete.replace("{{DDN}}"           , "");
            proc                    ->setNomImprimante(imprimante);
            aa                      = Imprimer_Document(QString::number(UserEntete), Titre, Entete, TxtDocument, DateDoc, nom, prenom, Prescription, ALD, AvecPrevisu, AvecDupli, AvecChoixImprimante);
            if (!aa)
                break;
            imprimante = proc->getNomImprimante();
        }
    }
    delete Dlg_Docs;
    if (aa && AffichDocsExternes)
        MAJDocsExternes();
}

/*-----------------------------------------------------------------------------------------------------------------
-- Ouvrir la liste des patients -----------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void    Rufus::OuvrirListe(bool AvecRecalcul)
{
    if (ui->tabWidget->currentIndex() == ui->tabWidget->indexOf(ui->tabDossier))
        ui->AtcdtsPersostextEdit->setFocus();
    ui->CreerNomlineEdit->clear();
    ui->CreerPrenomlineEdit->clear();
    ui->CreerNomlineEdit->setFocus();
    if (gMode == Liste && ui->tabWidget->currentIndex() == ui->tabWidget->indexOf(ui->tabList))
        return;
    ui->tabWidget->setCurrentIndex(ui->tabWidget->indexOf(ui->tabList));
    if(ListeRestreinte) Remplir_ListePatients_TableView(grequeteListe,"","");
    ui->CreerNomlineEdit->clear();
    ui->CreerPrenomlineEdit->clear();
    ui->ChercherDepuisListepushButton->setText(tr("Chercher avec la\ndate de naissance"));
    ui->ChercherDepuisListepushButton->setIcon(proc->giconDate);
    ui->ChercherDepuisListepushButton->setVisible(true);
    ui->CreerNomlineEdit->setVisible(true);
    ui->CreerPrenomlineEdit->setVisible(true);
    ui->CreerDDNdateEdit->setVisible(false);
    ui->Nomlabel->setVisible(true);
    ui->Prenomlabel->setVisible(true);
    ui->DDNlabel->setVisible(false);
    ui->FiltrecheckBox->setVisible(true);

    ui->CreerDossierpushButton->setIcon(proc->giconSortirDossier);
    ui->CreerDossierpushButton->setText(tr("Ouvrir\nle dossier"));
    ui->LListepushButton->setEnabled(false);
    ui->LNouvDossierpushButton->setEnabled(true);
    ui->LRecopierpushButton->setEnabled(ui->PatientsListeTableView->model()->rowCount() > 0);
    ui->CreerNomlineEdit->setValidator(new QRegExpValidator(proc->getrxRecherche(),this));
    ui->CreerPrenomlineEdit->setValidator(new QRegExpValidator(proc->getrxRecherche(),this));

    gSexePat = "";      // CZ001
    gNNIPat  = "";      // CZ001

    if (gidPatient > 0)
    {
        QList<QStandardItem*> listitems = gListePatientsModel->findItems(QString::number(gidPatient));
        if (!listitems.isEmpty())
        {
            ui->PatientsListeTableView->selectRow(listitems.at(0)->row());
            ui->PatientsListeTableView->scrollTo(listitems.at(0)->index(),QAbstractItemView::PositionAtCenter);
        }
    }
    else if (AvecRecalcul && gListePatientsModel->rowCount() > 0)
    {
        ui->PatientsListeTableView->selectRow(0);
        ui->PatientsListeTableView->scrollTo(gListePatientsModel->item(0)->index(),QAbstractItemView::PositionAtTop);
    }
    CalcNbDossiers();
    ui->CreerNomlineEdit->setValidator(new QRegExpValidator(proc->getrxRecherche(),this));
    ui->CreerPrenomlineEdit->setValidator(new QRegExpValidator(proc->getrxRecherche(),this));
    gMode = Liste;
}


/*-----------------------------------------------------------------------------------------------------------------
-- Enregistrer les données pour créer un nouveau dossier - Mise en place de la fiche ------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void Rufus::OuvrirNouveauDossier()
{
    if(!AutorDepartConsult(true)) return;
    if (ui->tabWidget->indexOf(ui->tabDossier) != -1)
    {
        ui->tabWidget->setCurrentWidget(ui->tabList);
        ui->tabWidget->removeTab(ui->tabWidget->indexOf(ui->tabDossier));
        FermeDlgAnnexes();
    }

    if (gMode == NouveauDossier && ui->tabWidget->currentIndex() == ui->tabWidget->indexOf(ui->tabList)) return;
    ui->tabWidget->setCurrentIndex(ui->tabWidget->indexOf(ui->tabList));
    ui->CreerNomlineEdit->setVisible(true);
    ui->CreerPrenomlineEdit->setVisible(true);
    ui->Nomlabel->setVisible(true);
    ui->Prenomlabel->setVisible(true);
    ui->ChercherDepuisListepushButton->setVisible(false);
    ui->CreerNomlineEdit->setValidator(new QRegExpValidator(proc->getrx(),this));
    ui->CreerPrenomlineEdit->setValidator(new QRegExpValidator(proc->getrx(),this));

    ui->FiltrecheckBox->setVisible(false);
    ui->CreerNomlineEdit->setFocus();
    ui->CreerDossierpushButton->setIcon(proc->giconOK);
    ui->CreerDossierpushButton->setIconSize(QSize(30,30));
    ui->CreerDossierpushButton->setText(tr("Créer\nle dossier"));
    ui->LListepushButton->setEnabled(true);
    ui->LNouvDossierpushButton->setEnabled(false);
    ui->LRecopierpushButton->setEnabled(gListePatientsModel->rowCount() > 0);
    ui->CreerDDNdateEdit->setVisible(true);
    ui->CreerDDNdateEdit->setDate(gdateParDefaut);
    ui->DDNlabel->setVisible(true);
    gMode = NouveauDossier;
}

/*-----------------------------------------------------------------------------------------------------------------
-- Créer un dossier de la meme famille - Mise en place de la fiche ------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void    Rufus::RecopierDossier(int idARecopier)
{
 //   if (gMode == RecopierDossier && ui->tabWidget->currentWidget() == ui->tabList && idARecopier == 0) return;

    if (ui->tabWidget->indexOf(ui->tabDossier) > -1)
    {if(!AutorDepartConsult(true)) return;}

    if (ui->tabWidget->currentWidget() == ui->tabDossier)
    {
        ui->tabWidget->setCurrentWidget(ui->tabList);
        idARecopier = gidPatient;
    }
    ui->tabWidget->removeTab(ui->tabWidget->indexOf(ui->tabDossier));

    if (idARecopier == 0)          // la fonction a été appelée depuis un bouton
    {
        QModelIndexList listitems = ui->PatientsListeTableView->selectionModel()->selectedIndexes();
        if (listitems.size() == 0)
        {
            UpMessageBox::Watch(this, tr("Aucun dossier sélectionné!"), tr("Sélectionnez d'abord un dossier à recopier."));
            return;
        }
        idARecopier = gListePatientsModel->itemFromIndex(listitems.at(0))->text().toInt();
    }
    FermeDlgAnnexes();

    QString req = "select patnom from " NOM_TABLE_PATIENTS " where idpat = " + QString::number(idARecopier);
    QSqlQuery quer(req,db);
    if (proc->TraiteErreurRequete(quer,req, tr("Impossible de retrouver le dossier d'origine"))) return;
    if (quer.size() == 0)
    {
        UpMessageBox::Watch(this,tr("Dossier à recopier non trouvé"));
        return;
    }
    gidARecopier = idARecopier;
    IdentificationPatient("Copie",0);
}

/*-----------------------------------------------------------------------------------------------------------------
-- Recaleles TreeView sur le dossier en cours ---------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void    Rufus::RecaleTableView(int idPat)
{
    QList<QStandardItem*> listitems = gListePatientsModel->findItems(QString::number(idPat));
    if (listitems.size() > 0)
    {
        QStandardItem *item = listitems.at(0);
        int itrow = item->row();
        ui->PatientsListeTableView->selectRow(itrow);
        ui->PatientsListeTableView->scrollTo(item->index(), QAbstractItemView::PositionAtCenter);
    }
}

/*-----------------------------------------------------------------------------------------------------------------
-- Reconstruit la liste des Cotations dans le combobox ActeCotations ----------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void    Rufus::ReconstruitListesActes()
{
    QString req;
    // il faut d'abord reconstruire la table des cotations
    ui->ActeCotationcomboBox->clear();
    if (gUserOPTAM)
        req = "SELECT TypeActe, montantoptam, montantpratique FROM " NOM_TABLE_COTATIONS " where idUser = " + QString::number(gidUserParent);
    else
        req = "SELECT TypeActe, montantnonoptam, montantpratique FROM " NOM_TABLE_COTATIONS " where idUser = " + QString::number(gidUserParent);
    QSqlQuery ListCotationsQuery (req,db);
    proc->TraiteErreurRequete(ListCotationsQuery,req,"");
    for (int i = 0; i < ListCotationsQuery.size(); i++)
    {
        ListCotationsQuery.seek(i);
        QStringList list;
        list << ListCotationsQuery.value(1).toString() << ListCotationsQuery.value(2).toString();
        ui->ActeCotationcomboBox->addItem(ListCotationsQuery.value(0).toString(),list);
    }
}

void Rufus::ReconstruitListesCorresp()
{
    ui->MGupComboBox->clear();
    QStringList ListMG;
    QList<int> ListidMG;
    QString req = "SELECT idCor, CorNom, CorPrenom FROM " NOM_TABLE_CORRESPONDANTS " where cormedecin = 1 order by cornom, corprenom";
    QSqlQuery ListMGQuery (req,db);
    proc->TraiteErreurRequete(ListMGQuery,req,"");
    for (int i = 0; i < ListMGQuery.size(); i++)
    {
        ListMGQuery.seek(i);
        ListMG << ListMGQuery.value(1).toString() + " " + ListMGQuery.value(2).toString();
        ListidMG << ListMGQuery.value(0).toInt();
        ui->MGupComboBox->insertItem(i, ListMG.at(i), ListidMG.at(i));
    }

    ui->AutresCorresp1upComboBox->clear();
    ui->AutresCorresp2upComboBox->clear();
    QStringList ListCor;
    QList<int> ListidCor;
    req = "SELECT idCor, CorNom, CorPrenom FROM " NOM_TABLE_CORRESPONDANTS " order by cornom, corprenom";
    QSqlQuery ListCorQuery (req,db);
    proc->TraiteErreurRequete(ListCorQuery,req,"");
    for (int i = 0; i < ListCorQuery.size(); i++)
    {
        ListCorQuery.seek(i);
        ListCor << ListCorQuery.value(1).toString() + " " + ListCorQuery.value(2).toString();
        ListidCor << ListCorQuery.value(0).toInt();
        ui->AutresCorresp1upComboBox->insertItem(i, ListCor.at(i), ListidCor.at(i));
        ui->AutresCorresp2upComboBox->insertItem(i, ListCor.at(i), ListidCor.at(i));
    }
}

/*-----------------------------------------------------------------------------------------------------------------
-- Mesurer la Refraction ------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void    Rufus::Refraction()
{
    if (ui->tabWidget->currentIndex() != 1) return;
    int idPatAPasser        = gidPatient;
    QString NomPatient      = gNomPatient;
    QString PrenomPatient   = gPrenomPatient;
    int idActeAPasser       = gidActe;
    int AgeAPasser          = gAgePatient;

    Dlg_Refraction     = new dlg_refraction(&idPatAPasser, &NomPatient, &PrenomPatient, &idActeAPasser, &AgeAPasser, proc, this);
    Dlg_Refraction->setWindowTitle("Refraction - " + gNomPatient + " " + gPrenomPatient);
    Dlg_Refraction->setWindowIcon(proc->giconLunettes);
    proc->setFicheRefractionOuverte(true);
    int result = Dlg_Refraction->exec();
    proc->setFicheRefractionOuverte(false);
    if (result > 0)
    {
        if (Dlg_Refraction->gResultatObservation != "")  // Ce n'est pas une prescription de verres correcteurs
        {
            for (int i= 0; i<Dlg_Refraction->gResultatObservation.size();i++)
                if (Dlg_Refraction->gResultatObservation.at(i).unicode() == 10) Dlg_Refraction->gResultatObservation.replace(Dlg_Refraction->gResultatObservation.at(i),"<br>");
            // si le dernier caractère n'est pas un retour à la ligne, on en rajoute un
            QString texte = ui->ActeTextetextEdit->toHtml();
            bool a = true;
            while (a) {
                     // il faut retirer la dernière ligne du html qui contient le retour à la ligne
                int debut = texte.lastIndexOf("<p");
                int fin   = texte.lastIndexOf("</p>");
                int longARetirer = fin - debut + 4;
                if (texte.mid(debut,longARetirer).contains("-qt-paragraph-type:empty;"))
                    texte.remove(debut,longARetirer);
                else a = false;
            }
            QString ARajouterEnText =  "<p style = \"margin-top:0px; margin-bottom:0px;\" >" + Dlg_Refraction->gResultatObservation  + "</p>"
                    + "<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px;\"></p>";
            if (ui->ActeTextetextEdit->toPlainText().size() == 0)
                texte = ARajouterEnText;
            else
                texte += ARajouterEnText;
            UpTextEdit *mod = new UpTextEdit;
            mod->setText(texte);
            QString Corps = mod->toHtml();
            Corps.replace("<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">","<p style=\" margin-top:0px; margin-bottom:0px;\">");
            Corps.remove("border=\"0\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px;\" ");

            QString updaterequete =  "UPDATE " NOM_TABLE_ACTES " SET ActeTexte = '" + proc->CorrigeApostrophe(Corps) +
                                     "' where idActe = " + ui->idActelineEdit->text();
            QSqlQuery UpdateUpTextEditQuery (updaterequete,db);
            proc->TraiteErreurRequete(UpdateUpTextEditQuery,updaterequete,"Impossible de mettre à jour le champ Texte !");
            ui->ActeTextetextEdit->setText(Corps);
            mod = 0;
            delete mod;

            ui->ActeTextetextEdit->setFocus();
            ui->ActeTextetextEdit->moveCursor(QTextCursor::End);
        }
        if (Dlg_Refraction->gResultatPR != "")  // C'est une prescription de verres correcteurs
        {
            // mettre à jour docsexterns
            MAJDocsExternes(); //Refraction()

            // si le dernier caractère n'est pas un retour à la ligne, on en rajoute un
            QString Date = "";
            for (int i= 0; i<Dlg_Refraction->gResultatPR.size();i++)
                if (Dlg_Refraction->gResultatPR.at(i).unicode() == 10) Dlg_Refraction->gResultatPR.replace(Dlg_Refraction->gResultatPR.at(i),"<br>");
            QString conclusion = ui->ActeConclusiontextEdit->toHtml();
            bool a = true;
            while (a) {
                int debut = conclusion.lastIndexOf("<p");
                int fin   = conclusion.lastIndexOf("</p>");
                int longARetirer = fin - debut + 4;
                if (conclusion.mid(debut,longARetirer).contains("-qt-paragraph-type:empty;"))
                    conclusion.remove(debut,longARetirer);
                else a = false;
            }
            QString larg = "550";
            if (ui->ActeDatedateEdit->date() != QDate::currentDate())
            {
                Date = "<td width=\"80\">le " + QDate::currentDate().toString("d.M.yyyy") + "</td>";
                larg = "470";
            }
            QString ARajouterEnConcl =  "<p style = \"margin-top:0px; margin-bottom:0px;\" >" + Date + Dlg_Refraction->gResultatPR  + "</p>"
                                         + "<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px;\">";
            if (ui->ActeConclusiontextEdit->toPlainText().size() == 0)
                conclusion = ARajouterEnConcl;
            else
                conclusion += ARajouterEnConcl;
            UpTextEdit *mod = new UpTextEdit;
            mod->setText(conclusion);
            QString Corps = mod->toHtml();
            Corps.remove("<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><br /></p>");
            Corps.replace("<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">","<p style=\" margin-top:0px; margin-bottom:0px;\">");
            Corps.remove("border=\"0\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px;\" ");

            QString updaterequete =  "UPDATE " NOM_TABLE_ACTES " SET ActeConclusion = '" + proc->CorrigeApostrophe(Corps) +
                                     "' where idActe = " + ui->idActelineEdit->text();
            QSqlQuery UpdateUpTextEditQuery (updaterequete,db);
            proc->TraiteErreurRequete(UpdateUpTextEditQuery,updaterequete, tr("Impossible de mettre à jour le champ conclusion !"));
            ui->ActeConclusiontextEdit->setFocus();
            ui->ActeConclusiontextEdit->moveCursor(QTextCursor::End);
            ui->ActeConclusiontextEdit->setText(Corps);

            mod = 0;
            delete mod;
       }
    }
    delete Dlg_Refraction;
}

void Rufus::RegleRefracteur(QString TypeMesure)
{
    QMap<QString,QVariant>      Mesure;
    Mesure["AxeOD"]     = "180";
    Mesure["AxeOG"]     = "180";
    Mesure["SphereOD"]  = "+00.00";
    Mesure["SphereOG"]  = "+00.00";
    Mesure["CylOD"]     = "+00.00";
    Mesure["CylOG"]     = "+00.00";
    Mesure["AddOD"]     = "+00.00";
    Mesure["AddOG"]     = "+00.00";
    QString req = "SELECT SphereOD, CylindreOD, AxeCylindreOD, AddVPOD, SphereOG, CylindreOG, AxeCylindreOG, AddVPOG, FormuleOD, FormuleOG FROM " NOM_TABLE_REFRACTION
            " WHERE IdPat = " + QString::number(gidPatient);
    if (TypeMesure=="P")
        req += " and (QuelleMesure = '" + TypeMesure + "' or quellemesure = 'O')";
    else if (TypeMesure=="R")
        req += " and QuelleMesure = '" + TypeMesure + "'";
    req += " And quelledistance <> 'P' order by idrefraction desc";
    //qDebug() << req;
    QSqlQuery quer (req,db);
    proc->TraiteErreurRequete(quer,req);
    if (quer.size()>0)
    {
        quer.first();
        QString prefix = "";
        // Les axes
        if (quer.value(2).toDouble()!=0)
        {
            if (quer.value(2).toInt()<10)
                prefix = "  ";
            else if (quer.value(2).toInt()<100)
                prefix = " ";
            Mesure["AxeOD"] = prefix + quer.value(2).toString();
        }
        prefix = "";
        if (quer.value(6).toDouble()!=0)
        {
            if (quer.value(6).toInt()<10)
                prefix = "  ";
            else if (quer.value(6).toInt()<100)
                prefix = " ";
            Mesure["AxeOG"] = prefix + quer.value(6).toString();
        }

        // Les spheres
        prefix = "";
        if (quer.value(0).toDouble()>0)
        {
            if (quer.value(0).toDouble()<10)
                prefix = "+0";
            else
                prefix = "+";
            Mesure["SphereOD"] = prefix + QString::number(quer.value(0).toDouble(),'f',2);
        }
        else if (quer.value(0).toDouble()<0)
        {
            prefix = QString::number(quer.value(0).toDouble(),'f',2);
            if (quer.value(0).toDouble()>-10)
                prefix.replace("-", "-0");
            Mesure["SphereOD"] = prefix;
        }
        prefix = "";
        if (quer.value(4).toDouble()>0)
        {
            if (quer.value(4).toDouble()<10)
                prefix = "+0";
            else
                prefix = "+";
            Mesure["SphereOG"] = prefix + QString::number(quer.value(4).toDouble(),'f',2);
        }
        else if (quer.value(4).toDouble()<0)
        {
            prefix = QString::number(quer.value(4).toDouble(),'f',2);
            if (quer.value(4).toDouble()>-10)
                prefix.replace("-", "-0");
            Mesure["SphereOG"] = prefix;
        }

        // Les cylindres
        prefix = "";
        if (quer.value(1).toDouble()>0)
        {
            if (quer.value(1).toDouble()<10)
                prefix = "+0";
            else
                prefix = "+";
            Mesure["CylOD"] = prefix + QString::number(quer.value(1).toDouble(),'f',2);
        }
        else if (quer.value(1).toDouble()<0)
        {
            prefix = QString::number(quer.value(1).toDouble(),'f',2);
            if (quer.value(1).toDouble()>-10)
                prefix.replace("-", "-0");
            Mesure["CylOD"] = prefix;
        }
        prefix = "";
        if (quer.value(5).toDouble()>0)
        {
            if (quer.value(5).toDouble()<10)
                prefix = "+0";
            else
                prefix = "+0";
            Mesure["CylOG"] = prefix + QString::number(quer.value(5).toDouble(),'f',2);
        }
        else if (quer.value(5).toDouble()<0)
        {
            prefix = QString::number(quer.value(5).toDouble(),'f',2);
            if (quer.value(5).toDouble()>-10)
                prefix.replace("-", "-0");
            Mesure["CylOG"] = prefix;
        }

        // Les additions
        if (quer.value(3).toDouble()!=0)
            Mesure["AddOD"] = "+0" + QString::number(quer.value(3).toDouble(),'f',2);
        if (quer.value(7).toDouble()!=0)
            Mesure["AddOG"] = "+0" + QString::number(quer.value(7).toDouble(),'f',2);

        // Les formules
            Mesure["FormuleOD"] = quer.value(8).toString();
            Mesure["FormuleOG"] = quer.value(9).toString();
        /*QString AB;
        if (TypeMesure == "R")
            AB = "Autoref";
        if (TypeMesure == "P")
            AB = "Fronto";
        qDebug() << AB << Mesure["SphereOD"].toString() << Mesure["CylOD"].toString() << Mesure["AxeOD"].toString() << Mesure["AddOD"].toString()
                << Mesure["SphereOG"].toString() << Mesure["CylOG"].toString() << Mesure["AxeOG"].toString() << Mesure["AddOG"].toString();
        */
    }
    if (TypeMesure == "R")
        gMesureAutoref = Mesure;
    if (TypeMesure == "P")
        gMesureFronto = Mesure;
 }

/*-----------------------------------------------------------------------------------------------------------------
-- Remise de chèques ----------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void Rufus::RemiseCheques()
{
    Dlg_RemCheq          = new dlg_remisecheques(proc, this);
    if (Dlg_RemCheq->getInitOK())
    {
        Dlg_RemCheq->setWindowTitle(tr("Remise de chèques"));
        Dlg_RemCheq->exec();
        delete Dlg_RemCheq;
    }
}

/*-----------------------------------------------------------------------------------------------------------------
-- Remplir la liste avec les noms, prénoms et DDN des patients ----------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
bool Rufus::Remplir_ListePatients_TableView(QString requete, QString PatNom, QString PatPrenom)
{
    QString Addrequete;
    QStandardItem *pitem, *pitem0, *pitem1;

    ListeRestreinte = false;
    if (proc->CorrigeApostrophe(PatNom).length() > 0 || proc->CorrigeApostrophe(PatPrenom).length() > 0)
        Addrequete += " WHERE ";
    if (proc->CorrigeApostrophe(PatNom).length() > 0)
        Addrequete += "PatNom = '" + proc->CorrigeApostrophe(PatNom) + "'";
    if (proc->CorrigeApostrophe(PatPrenom).length() > 0)
    {
        if (Addrequete != " WHERE ")
            Addrequete += " AND PatPrenom = '" + proc->CorrigeApostrophe(PatPrenom) + "'";
        else
            Addrequete += "PatPrenom = '" + proc->CorrigeApostrophe(PatPrenom) + "'";
    }
    requete += Addrequete;
    requete += " ORDER BY PatNom, PatPrenom, PatDDN ";
    if (proc->getModeConnexion() == Procedures::Distant)
        requete += " LIMIT 1000";
    QSqlQuery   RemplirTableViewQuery (requete,db);
    if (proc->TraiteErreurRequete(RemplirTableViewQuery,requete,"")) return false;
    gNombreDossiers = RemplirTableViewQuery.size();


    gListePatientsModel = dynamic_cast<QStandardItemModel*>(ui->PatientsListeTableView->model());
    if (gListePatientsModel)
        gListePatientsModel->clear();
    else
        gListePatientsModel = new QStandardItemModel;

    if (gNombreDossiers > 0)
    {
        RemplirTableViewQuery.first();
        for (int i=0;i<gNombreDossiers;i++)
        {
            pitem   = new QStandardItem(RemplirTableViewQuery.value(0).toString());                                                             // IdPatient
            pitem0  = new QStandardItem(RemplirTableViewQuery.value(1).toString().toUpper() + " " + RemplirTableViewQuery.value(2).toString());  // Nom + Prénom
            pitem1  = new QStandardItem(RemplirTableViewQuery.value(3).toDate().toString(tr("dd-MM-yyyy")));                                        // date de naissance
            QList<QStandardItem *> pitemlist;
            pitemlist << pitem << pitem0 << pitem1;
            gListePatientsModel->appendRow(pitemlist);
            RemplirTableViewQuery.next();
        }
    }
    QStandardItem *itnom = new QStandardItem();
    itnom->setText("Nom");
    itnom->setTextAlignment(Qt::AlignLeft);
    gListePatientsModel->setHorizontalHeaderItem(1,itnom);
    QStandardItem *itDDN = new QStandardItem();
    itDDN->setText("Date de naissance");
    itDDN->setTextAlignment(Qt::AlignLeft);
    gListePatientsModel->setHorizontalHeaderItem(2,itDDN);
    ui->PatientsListeTableView->setModel(gListePatientsModel);
    ui->PatientsListeTableView->setColumnWidth(0,0);          //IdPat
    ui->PatientsListeTableView->setColumnWidth(1,230 );       //Nom + Prénom
    ui->PatientsListeTableView->setColumnWidth(2,122 );       //DDN

    QFontMetrics fm(qApp->font());
    for (int j=0; j<gNombreDossiers; j++)
         ui->PatientsListeTableView->setRowHeight(j,fm.height()*1.3);

    ui->PatientsListeTableView->setStyleSheet("QTableView {selection-color: rgb(0,0,0); selection-background-color: rgb(164, 205, 255);}");
    ui->PatientsListeTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->PatientsListeTableView->horizontalHeader()->setFixedHeight(fm.height()*1.3);

    return true;
}

/*-----------------------------------------------------------------------------------------------------------------
-- Remplir le treeView de salle d'attente ----------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void Rufus::Slot_MAJ_SalleDAttente()
{
    Remplir_SalDat();
}

void Rufus::Remplir_SalDat()
{
    QTableWidget        *TableAMettreAJour;
    int                 i;
    QString             NomPrenom, zw, A;
    QFontMetrics        fm(qApp->font());

    // SALLE D'ATTENTE ---------------------------------------------------------------------------------------------------
    QString SalDatrequete =   "SELECT saldat.IdPat, PatNom, PatPrenom, HeureArrivee, Statut, Motif, HeureRDV, Message, saldat.idUser, UserLogin FROM " NOM_TABLE_SALLEDATTENTE " AS saldat"
                       " INNER JOIN " NOM_TABLE_PATIENTS " ON " NOM_TABLE_PATIENTS ".idPat = saldat.idPat "
                       " INNER JOIN " NOM_TABLE_UTILISATEURS " ON " NOM_TABLE_UTILISATEURS ".idUser = saldat.idUser "
                       " WHERE saldat.Statut = '" ARRIVE "'"
                       " OR saldat.Statut = '" ENCOURS "'"
                       " OR (LOCATE('" ENATTENTENOUVELEXAMEN "', saldat.Statut,1) > 0) "
                       " ORDER BY HeureRDV";
    //proc->Edit(SalDatrequete);
    /*
      SELECT saldat.IdPat, PatNom, PatPrenom, HeureArrivee, Statut, Motif, HeureRDV, Message, saldat.idUser, UserLogin
      FROM rufus.salledattente AS saldat
      INNER JOIN rufus.Patients ON rufus.Patients.idPat = saldat.idPat
      INNER JOIN rufus.utilisateurs ON rufus.utilisateurs.idUser = saldat.idUser
      WHERE saldat.Statut = 'Arrivé'
      OR saldat.Statut = 'En cours'
      OR (LOCATE('En attente de nouvel examen par ', saldat.Statut,1) > 0)
      ORDER BY HeureRDV
    */

    TableAMettreAJour = ui->SalleDAttenteupTableWidget;

    for (int i =0; i< ui->SalleDAttenteupTableWidget->rowCount();i++)
        ui->SalleDAttenteupTableWidget->removeRow(i);
    QSqlQuery       RemplirTableViewUserQuery (SalDatrequete,db);
    proc->TraiteErreurRequete(RemplirTableViewUserQuery,SalDatrequete, tr("Impossible de remplir la salle d'attente!"));
    RemplirTableViewUserQuery.first();

    TableAMettreAJour   ->setRowCount(RemplirTableViewUserQuery.size());
    gListeSuperviseursModel  = new QStandardItemModel;
    QStandardItem       *pitem0, *pitem1;
    QList<int>          listidusers;
    for (i = 0; i < RemplirTableViewUserQuery.size(); i++)
    {
        UpLabel *label0, *label1, *label2, *label3, *label4, *label5, *label6;
        label0 = new UpLabel(TableAMettreAJour);
        label1 = new UpLabel(TableAMettreAJour);
        label2 = new UpLabel(TableAMettreAJour);
        label3 = new UpLabel(TableAMettreAJour);
        label4 = new UpLabel(TableAMettreAJour);
        label5 = new UpLabel(TableAMettreAJour);
        label6 = new UpLabel(TableAMettreAJour);

        label0->setId(RemplirTableViewUserQuery.value(0).toInt());                      // idPat
        label1->setId(RemplirTableViewUserQuery.value(0).toInt());                      // idPat
        label2->setId(RemplirTableViewUserQuery.value(0).toInt());                      // idPat
        label3->setId(RemplirTableViewUserQuery.value(0).toInt());                      // idPat
        label4->setId(RemplirTableViewUserQuery.value(0).toInt());                      // idPat
        label5->setId(RemplirTableViewUserQuery.value(0).toInt());                      // idPat
        label6->setId(RemplirTableViewUserQuery.value(0).toInt());                      // idPat

        label0->setRow(i);
        label1->setRow(i);
        label2->setRow(i);
        label3->setRow(i);
        label4->setRow(i);
        label5->setRow(i);
        label6->setRow(i);

        label0->setContextMenuPolicy(Qt::CustomContextMenu);
        label1->setContextMenuPolicy(Qt::CustomContextMenu);
        label2->setContextMenuPolicy(Qt::CustomContextMenu);
        label3->setContextMenuPolicy(Qt::CustomContextMenu);
        label4->setContextMenuPolicy(Qt::CustomContextMenu);
        label5->setContextMenuPolicy(Qt::CustomContextMenu);
        label6->setContextMenuPolicy(Qt::CustomContextMenu);

        label3->setAlignment(Qt::AlignCenter);
        label4->setAlignment(Qt::AlignCenter);
        label5->setAlignment(Qt::AlignCenter);
        label6->setAlignment(Qt::AlignCenter);

        QString Msg = RemplirTableViewUserQuery.value(5).toString();
        NomPrenom = " " + RemplirTableViewUserQuery.value(1).toString().toUpper()
                    + " " + RemplirTableViewUserQuery.value(2).toString();
        label0->setText(NomPrenom);                                                     // Nom + Prénom
        label1->setText(RemplirTableViewUserQuery.value(4).toString());                 // Statut
        label4->setText(RemplirTableViewUserQuery.value(5).toString());                 // Motif
        if (RemplirTableViewUserQuery.value(7).toString()!="")
            label2->setPixmap(QPixmap("://button_blue_play.png").scaled(10,10));

        QString color;
        if (RemplirTableViewUserQuery.value(3).toTime().toString("HH:mm") != "")
        {
            QTime heureArriv = RemplirTableViewUserQuery.value(3).toTime();
            label5->setText(heureArriv.toString("HH:mm"));                              // Heure arrivée
            if (heureArriv.secsTo(QTime::currentTime())/60 < 15)
                color = "color: green";
            else if (heureArriv.secsTo(QTime::currentTime())/60 < 30)
               color = "color: orange";
            else
               color = "color: red";
        }
        label6->setText(RemplirTableViewUserQuery.value(9).toString());    // Superviseur
        if (!listidusers.contains(RemplirTableViewUserQuery.value(8).toInt()))
        {
            listidusers << RemplirTableViewUserQuery.value(8).toInt();
            pitem0 = new QStandardItem(RemplirTableViewUserQuery.value(8).toString());
            pitem1 = new QStandardItem(RemplirTableViewUserQuery.value(9).toString());
            QList<QStandardItem*> listitems;
            listitems << pitem0 << pitem1;
            gListeSuperviseursModel    ->appendRow(listitems);
        }
        QString colorRDV;
        if (RemplirTableViewUserQuery.value(6).toTime().toString("HH:mm") != "")
        {
            QTime heureRDV = RemplirTableViewUserQuery.value(6).toTime();
            label3->setText(heureRDV.toString("HH:mm"));                                // Heure arrivée
            if (heureRDV.secsTo(QTime::currentTime())/60 < 15)
                colorRDV = "color: green";
            else if (heureRDV.secsTo(QTime::currentTime())/60 < 30)
               colorRDV = "color: orange";
            else
               colorRDV = "color: red";
        }
        QString background;
        if (Msg=="URG")
        {
            QString styleurg = "background:#EEFFFF ; color: red";
            label0->setStyleSheet(styleurg);
            label1->setStyleSheet(styleurg);
            label2->setStyleSheet(styleurg);
            label3->setStyleSheet(styleurg);
            label4->setStyleSheet(styleurg);
            label5->setStyleSheet(styleurg);
            label6->setStyleSheet(styleurg);
        }
        else
        {
            QString req2 = "select Couleur from " NOM_TABLE_MOTIFSRDV " where raccourci = '" + Msg + "'";
            QSqlQuery colorquer(req2,db);
            if (colorquer.size()>0)
            {
                colorquer.first();
                background = "background:#" + colorquer.value(0).toString();
            }
            else
                background = "background:#FFFFEE";
            label0->setStyleSheet(background);
            label1->setStyleSheet(background);
            label2->setStyleSheet(background);
            label3->setStyleSheet(background + ";" + colorRDV);
            label4->setStyleSheet(background);
            label5->setStyleSheet(background + ";" + color);
            label6->setStyleSheet(background);
        }

        label0->setAlignment(Qt::AlignLeft);
        label1->setAlignment(Qt::AlignLeft);
        label2->setAlignment(Qt::AlignCenter);
        connect (label0,        SIGNAL(customContextMenuRequested(QPoint)),         this,       SLOT (Slot_MenuContextuelSalDat()));
        connect (label1,        SIGNAL(customContextMenuRequested(QPoint)),         this,       SLOT (Slot_MenuContextuelSalDat()));
        connect (label2,        SIGNAL(customContextMenuRequested(QPoint)),         this,       SLOT (Slot_MenuContextuelSalDat()));
        connect (label3,        SIGNAL(customContextMenuRequested(QPoint)),         this,       SLOT (Slot_MenuContextuelSalDat()));
        connect (label4,        SIGNAL(customContextMenuRequested(QPoint)),         this,       SLOT (Slot_MenuContextuelSalDat()));
        connect (label5,        SIGNAL(customContextMenuRequested(QPoint)),         this,       SLOT (Slot_MenuContextuelSalDat()));
        connect (label6,        SIGNAL(customContextMenuRequested(QPoint)),         this,       SLOT (Slot_MenuContextuelSalDat()));
        connect (label0,        SIGNAL(enter(int)),                                 this,       SLOT (Slot_AfficheMotif(int)));
        connect (label1,        SIGNAL(enter(int)),                                 this,       SLOT (Slot_AfficheMotif(int)));
        connect (label2,        SIGNAL(enter(int)),                                 this,       SLOT (Slot_AfficheMotif(int)));
        connect (label3,        SIGNAL(enter(int)),                                 this,       SLOT (Slot_AfficheMotif(int)));
        connect (label4,        SIGNAL(enter(int)),                                 this,       SLOT (Slot_AfficheMotif(int)));
        connect (label5,        SIGNAL(enter(int)),                                 this,       SLOT (Slot_AfficheMotif(int)));
        connect (label6,        SIGNAL(enter(int)),                                 this,       SLOT (Slot_AfficheMotif(int)));
        connect (label0,        SIGNAL(clicked(int)),                               this,       SLOT (Slot_SurbrillanceSalDat()));
        connect (label1,        SIGNAL(clicked(int)),                               this,       SLOT (Slot_SurbrillanceSalDat()));
        connect (label2,        SIGNAL(clicked(int)),                               this,       SLOT (Slot_SurbrillanceSalDat()));
        connect (label3,        SIGNAL(clicked(int)),                               this,       SLOT (Slot_SurbrillanceSalDat()));
        connect (label4,        SIGNAL(clicked(int)),                               this,       SLOT (Slot_SurbrillanceSalDat()));
        connect (label5,        SIGNAL(clicked(int)),                               this,       SLOT (Slot_SurbrillanceSalDat()));
        connect (label6,        SIGNAL(clicked(int)),                               this,       SLOT (Slot_SurbrillanceSalDat()));
        connect (label0,        SIGNAL(dblclick(int)),                              this,       SLOT (Slot_ChoixDossier(int)));
        connect (label1,        SIGNAL(dblclick(int)),                              this,       SLOT (Slot_ChoixDossier(int)));
        connect (label2,        SIGNAL(dblclick(int)),                              this,       SLOT (Slot_ChoixDossier(int)));
        connect (label3,        SIGNAL(dblclick(int)),                              this,       SLOT (Slot_ChoixDossier(int)));
        connect (label4,        SIGNAL(dblclick(int)),                              this,       SLOT (Slot_ChoixDossier(int)));
        connect (label5,        SIGNAL(dblclick(int)),                              this,       SLOT (Slot_ChoixDossier(int)));
        connect (label6,        SIGNAL(dblclick(int)),                              this,       SLOT (Slot_ChoixDossier(int)));
        TableAMettreAJour->setCellWidget(i,0,label0);
        TableAMettreAJour->setCellWidget(i,1,label1);
        TableAMettreAJour->setCellWidget(i,2,label2);
        TableAMettreAJour->setCellWidget(i,3,label3);
        TableAMettreAJour->setCellWidget(i,4,label4);
        TableAMettreAJour->setCellWidget(i,5,label5);
        TableAMettreAJour->setCellWidget(i,6,label6);
        TableAMettreAJour->setRowHeight(i,QFontMetrics(qApp->font()).height()*1.1);

        RemplirTableViewUserQuery.next();
    }
    while (gSalDatTab->count()>0)
        gSalDatTab->removeTab(0);
    int k =0;
    if (gListeSuperviseursModel->rowCount()==0)
        gSalDatTab->setVisible(false);
    else
    {
        gSalDatTab->setVisible(true);
        if (gListeSuperviseursModel->rowCount()>1)
        {
            gSalDatTab  ->insertTab(0, proc->giconFamily, tr("Tout le monde"));
            gSalDatTab  ->setTabData(k, -1);
            k++;
        }
        for (int i=0; i<gListeSuperviseursModel->rowCount(); i++)
        {
            gSalDatTab  ->insertTab(k,gListeSuperviseursModel->item(i,1)->text());
            gSalDatTab  ->setTabData(k, gListeSuperviseursModel->item(i,0)->text());
            k++;
        }
        bool a = false;
        for (int i=0; i<gSalDatTab->count(); i++)
        {
            if (gSalDatTab->tabData(i).toInt() == gidUserSuperviseur)
            {
                gSalDatTab->setCurrentIndex(i);
                a = true;
            }
        }
        if (!a)
            gSalDatTab->setCurrentIndex(0);
        Slot_FiltreSalleDAttente(gSalDatTab->currentIndex());
    }


    // BUREAUX ---------------------------------------------------------------------------------------------------

    QList<UpTextEdit *> listuptext = ui->scrollArea->findChildren<UpTextEdit*>();
    if (listuptext.size() > 0)
        for (int j=0; j<listuptext.size();j++)
            delete listuptext.at(j);
    ui->scrollArea->takeWidget();
    proc->commit(db);
    QString bureauxreq = "SELECT usc.idUser, UserLogin, NomPosteConnecte, PatNom, PatPreNom, HeureArrivee, saldat.idPat FROM " NOM_TABLE_USERSCONNECTES " AS usc\n"
                         " INNER JOIN " NOM_TABLE_UTILISATEURS " ON " NOM_TABLE_UTILISATEURS ".idUser = usc.idUser\n"
                         " LEFT OUTER JOIN " NOM_TABLE_SALLEDATTENTE " as saldat on (idUserEnCoursExam = usc.idUser and saldat.posteexamen = usc.NomPosteConnecte)\n"
                         " LEFT OUTER JOIN " NOM_TABLE_PATIENTS " ON " NOM_TABLE_PATIENTS ".idPat = saldat.idPat\n"
                         " where time_to_sec(timediff(NOW(), HeureDerniereConnexion)) < 60\n"
                         " and (soignant = 1 or soignant = 2 or soignant = 3)\n"
                         " order by UserLogin";

    //UpMessageBox::Watch(this,bureauxreq);
    QSqlQuery   BureauxQuery (bureauxreq,db);
    proc->TraiteErreurRequete(BureauxQuery,bureauxreq,"bureauxreq");

    //UpMessageBox::Watch(this,userconnectreq + "\n- " + QString::number(userconnectQuery.size()) + " -\n- " + userconnectQuery.value(0).toString() + " -");
    proc->TraiteErreurRequete(BureauxQuery, bureauxreq,"");
    if (BureauxQuery.size() >0)
    {
        ui->scrollArea->setWidgetResizable(true);
        QHBoxLayout *lay = new QHBoxLayout();
        QWidget *widg = new QWidget();
        widg->setLayout(lay);
        ui->scrollArea->setWidget(widg);
        int a = 0;
        lay->setContentsMargins(a,a,a,a);
        lay->setSpacing(2);
        BureauxQuery.first();
        for (int i=0;i<BureauxQuery.size();i++)
        {
            QString UserLogin = BureauxQuery.value(1).toString();
            QString PosteLog  = BureauxQuery.value(2).toString().remove(".local");
            UpTextEdit *UserBureau;
            UserBureau = new UpTextEdit;
            UserBureau->setObjectName(UserLogin + "BureauupTextEdit");
            UserBureau->setIdUser(BureauxQuery.value(0).toInt());
            ui->scrollArea->setStyleSheet("border: 1px none gray;  border-radius: 10px;");
            UserBureau->setStyleSheet("background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #f6f7fa, stop: 1 rgba(200, 255, 200, 50));"
                                      "border: 1px solid gray;  border-radius: 10px;");
            UserBureau->setFocusPolicy(Qt::NoFocus);
            QString html =
            "<html>"
            "<head>"
              "<style type=\"text/css\">"
              "  p.p10 {font:16px; margin: 10px 0px 0px 5px;}"
              "  p.p2 {margin: 0px 0px 0px 5px;}"
              "</style>"
            "</head>"
            "<body LANG=\"fr-FR\" DIR=\"LTR\">";
            html += "<p class=\"p10\"><b>" + PosteLog + "</b></p><p class=\"p2\"><b><span style=\"color:green;\">" + UserLogin + "</b></p>";
            if (BureauxQuery.value(3).toString() != "")
            {
                UserBureau->setId(BureauxQuery.value(6).toInt());
                UserBureau->setContextMenuPolicy(Qt::CustomContextMenu);
                if (UserBureau->getIdUser() == gidUser)
                    connect(UserBureau, SIGNAL(dblclick(int)),this,SLOT(Slot_ChoixDossier(int)));
                else
                {
                    connect(UserBureau, SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(Slot_MenuContextuelBureaux()));
                    connect(UserBureau, SIGNAL(dblclick(int)),this,SLOT(Slot_AutreDossier(int)));
                }
                html += "<p class=\"p2\">" +  BureauxQuery.value(3).toString() + " " + BureauxQuery.value(4).toString() + "</p>";      //Nom Prenom
                QString color = "black";
                if (BureauxQuery.value(5).toTime().toString("HH:mm") != "")
                {
                    QTime heureArriv = BureauxQuery.value(5).toTime();
                    if (heureArriv.secsTo(QTime::currentTime())/60 < 15)        color = "green";
                    else if (heureArriv.secsTo(QTime::currentTime())/60 < 30)   color = "orange";
                    else                                                        color ="red";
                }
                //html += "<p class=\"p2\">" +  RemplirBureauxQuery.value(4).toTime().toString("HH:mm") + "</p>";                                      //heure arrivée
                html += "<p class=\"p2\"><span style=\"color:" + color + ";\">" +  BureauxQuery.value(5).toTime().toString("HH:mm") + "</span></p>";                                      //heure arrivée
            }
            else
                html += "<p class=\"p2\">ZZzzz...</p>";
            html += "</body></html>";
            UserBureau->setHtml(html);
            lay->addWidget(UserBureau);
            BureauxQuery.next();            
        }
    }


    // ACCUEIL ----------------------------------------------------------------------------------------------------------
    QString PaiementsEnAttenterequete  =    "SELECT saldat.IdPat, PatNom, PatPrenom, UserLogin, ActeCotation,"                      // 0,1,2,3,4
                                            " ActeMontant, ActeHeure, saldat.idActeAPayer, Message, saldat.idUser,"                 // 5,6,7,8,9
                                            " UserParent"                                                                           // 10
                                            " FROM " NOM_TABLE_SALLEDATTENTE " AS saldat"
                                            " INNER JOIN " NOM_TABLE_PATIENTS " ON " NOM_TABLE_PATIENTS ".idPat = saldat.idPat "
                                            " INNER JOIN " NOM_TABLE_UTILISATEURS " ON " NOM_TABLE_UTILISATEURS ".idUser = saldat.idUser"
                                            " INNER JOIN " NOM_TABLE_ACTES " ON " NOM_TABLE_ACTES ".idActe = saldat.idActeAPayer"
                                            " WHERE saldat.Statut = '" RETOURACCUEIL "'"
                                            " ORDER BY UserLogin, HeureArrivee";
    //qDebug() << PaiementsEnAttenterequete;

    TableAMettreAJour = ui->AccueilupTableWidget;
    QSqlQuery RemplirTableViewPaiementQuery (PaiementsEnAttenterequete,db);
    proc->TraiteErreurRequete(RemplirTableViewPaiementQuery,PaiementsEnAttenterequete, tr("Impossible de remplir la salle d'attente!"));
    RemplirTableViewPaiementQuery.first();
    TableAMettreAJour->clearContents();
    TableAMettreAJour->setRowCount(RemplirTableViewPaiementQuery.size());
    gListeParentsModel  = new QStandardItemModel;
    QStandardItem       *oitem0, *oitem1;
    QList<int>          listidparents;

    for (i = 0; i < RemplirTableViewPaiementQuery.size(); i++)
    {
        UpLabel *label0, *label1, *label2, *label3, *label4, *label5;
        label0 = new UpLabel;
        label1 = new UpLabel;
        label2 = new UpLabel;
        label3 = new UpLabel;
        label4 = new UpLabel;
        label5 = new UpLabel;

        label0->setRow(i);
        label1->setRow(i);
        label2->setRow(i);
        label3->setRow(i);
        label4->setRow(i);
        label5->setRow(i);

        QTableWidgetItem *pItem = new QTableWidgetItem;

        label0->setId(RemplirTableViewPaiementQuery.value(0).toInt());                      // idPat
        label1->setId(RemplirTableViewPaiementQuery.value(0).toInt());                      // idPat
        label2->setId(RemplirTableViewPaiementQuery.value(0).toInt());                      // idPat
        label3->setId(RemplirTableViewPaiementQuery.value(0).toInt());                      // idPat
        label4->setId(RemplirTableViewPaiementQuery.value(0).toInt());                      // idPat
        label5->setId(RemplirTableViewPaiementQuery.value(0).toInt());                      // idPat

        label0->setContextMenuPolicy(Qt::CustomContextMenu);
        label1->setContextMenuPolicy(Qt::CustomContextMenu);
        label2->setContextMenuPolicy(Qt::CustomContextMenu);
        label3->setContextMenuPolicy(Qt::CustomContextMenu);
        label4->setContextMenuPolicy(Qt::CustomContextMenu);
        label5->setContextMenuPolicy(Qt::CustomContextMenu);

        NomPrenom = RemplirTableViewPaiementQuery.value(1).toString().toUpper()
                + " " + RemplirTableViewPaiementQuery.value(2).toString();
        zw = RemplirTableViewPaiementQuery.value(6).toTime().toString("HH:mm");
        label0->setText(" " + zw);                                                                                // Heure acte
        label1->setText(" " + NomPrenom);                                                                         // Nom + Prénom
        QString Soignant  = RemplirTableViewPaiementQuery.value(3).toString();
        if (RemplirTableViewPaiementQuery.value(9).toString() != RemplirTableViewPaiementQuery.value(10).toString())
            Soignant +=  " / " + proc->getLogin(RemplirTableViewPaiementQuery.value(10).toInt());
        label2->setText(" " + RemplirTableViewPaiementQuery.value(3).toString());                                 // Soignant
        label3->setText(" " + RemplirTableViewPaiementQuery.value(4).toString());                                 // Cotation
        label4->setText(QLocale().toString(RemplirTableViewPaiementQuery.value(5).toDouble(),'f',2) + " ");       // Montant
        label5->setText(RemplirTableViewPaiementQuery.value(10).toString());                                      // idParent
        QString typpaiement = "";
        if (RemplirTableViewPaiementQuery.value(5).toDouble() == 0)
            typpaiement = "Gratuit";
        label4->setAlignment(Qt::AlignRight);
        if (RemplirTableViewPaiementQuery.value(8).toString()!="")
        {
            QString color = "color:green";
            label0->setStyleSheet(color);
            label1->setStyleSheet(color);
            label2->setStyleSheet(color);
            label3->setStyleSheet(color);
            label4->setStyleSheet(color);
            label5->setStyleSheet(color);
        }
        if (!listidparents.contains(RemplirTableViewPaiementQuery.value(10).toInt()))
        {
            listidparents           << RemplirTableViewPaiementQuery.value(10).toInt();
            oitem0                  = new QStandardItem(RemplirTableViewPaiementQuery.value(10).toString());
            oitem1                  = new QStandardItem(proc->getLogin(RemplirTableViewPaiementQuery.value(10).toInt()));
            QList<QStandardItem*>   listitems;
            listitems               << oitem0 << oitem1;
            gListeParentsModel      ->appendRow(listitems);
        }

        connect (label0,        SIGNAL(customContextMenuRequested(QPoint)),         this,       SLOT (Slot_MenuContextuelSalDatPaiemt()));
        connect (label1,        SIGNAL(customContextMenuRequested(QPoint)),         this,       SLOT (Slot_MenuContextuelSalDatPaiemt()));
        connect (label2,        SIGNAL(customContextMenuRequested(QPoint)),         this,       SLOT (Slot_MenuContextuelSalDatPaiemt()));
        connect (label3,        SIGNAL(customContextMenuRequested(QPoint)),         this,       SLOT (Slot_MenuContextuelSalDatPaiemt()));
        connect (label4,        SIGNAL(customContextMenuRequested(QPoint)),         this,       SLOT (Slot_MenuContextuelSalDatPaiemt()));
        connect (label5,        SIGNAL(customContextMenuRequested(QPoint)),         this,       SLOT (Slot_MenuContextuelSalDatPaiemt()));
        connect (label0,        SIGNAL(enter(int)),                                 this,       SLOT (Slot_AfficheMotif(int)));
        connect (label1,        SIGNAL(enter(int)),                                 this,       SLOT (Slot_AfficheMotif(int)));
        connect (label2,        SIGNAL(enter(int)),                                 this,       SLOT (Slot_AfficheMotif(int)));
        connect (label3,        SIGNAL(enter(int)),                                 this,       SLOT (Slot_AfficheMotif(int)));
        connect (label4,        SIGNAL(enter(int)),                                 this,       SLOT (Slot_AfficheMotif(int)));
        connect (label5,        SIGNAL(enter(int)),                                 this,       SLOT (Slot_AfficheMotif(int)));
        TableAMettreAJour   ->setCellWidget(i,0,label0);
        TableAMettreAJour   ->setCellWidget(i,1,label1);
        TableAMettreAJour   ->setCellWidget(i,2,label2);
        TableAMettreAJour   ->setCellWidget(i,3,label3);
        TableAMettreAJour   ->setCellWidget(i,4,label4);
        pItem               ->setText(RemplirTableViewPaiementQuery.value(7).toString());
        TableAMettreAJour   ->setItem(i,5,pItem);
        TableAMettreAJour   ->setCellWidget(i,6,label5);
        TableAMettreAJour   ->setRowHeight(i,fm.height()*1.1);

        RemplirTableViewPaiementQuery.next();
    }
    while (gAccueilTab->count()>0)
        gAccueilTab->removeTab(0);
    if (gListeParentsModel->rowCount()==0)
        gAccueilTab->setVisible(false);
    else
    {
        gAccueilTab->setVisible(true);
        for (int i=0; i<gListeParentsModel->rowCount(); i++)
        {
            gAccueilTab  ->insertTab(i,gListeParentsModel->item(i,1)->text());
            gAccueilTab  ->setTabData(i, gListeParentsModel->item(i,0)->text());
        }
        if (ui->AccueilupTableWidget->selectedRanges().size()>0)
        {
            bool a = false;
            for (int i=0; i<gAccueilTab->count(); i++)
                if (gAccueilTab->tabData(i).toInt() == gidUserParent)
                {
                    gAccueilTab->setCurrentIndex(i);
                    a = true;
                }
            if (!a)
                gAccueilTab->setCurrentIndex(0);
        }
        Slot_FiltreAccueil(gAccueilTab->currentIndex());
    }

    // PATIENTS VUS AUJOURD'HUI ---------------------------------------------------------------------------------------------------
    QString SalDatVusrequete;
    QSqlQuery RemplirTableViewListJourQuery (db);

    SalDatVusrequete =   "SELECT pat.IdPat, act.idacte, PatNom, PatPrenom, UserLogin, ActeDate, ActeCotation, ActeMontant, ActeHeure, TypePaiement, Tiers FROM "
                           NOM_TABLE_PATIENTS " as pat, " NOM_TABLE_ACTES " as act, " NOM_TABLE_UTILISATEURS " as usr, " NOM_TABLE_TYPEPAIEMENTACTES " as typ"
                           " WHERE usr.idUser = act.idUser and act.idPat = pat.idPat and actedate = curdate()"
                           " and act.idPat not in (select idpat from " NOM_TABLE_SALLEDATTENTE ")"
                           " and act.idActe = typ.idActe"
                           " ORDER BY ActeHeure DESC";

    TableAMettreAJour = ui->PatientsVusupTableWidget;

    RemplirTableViewListJourQuery.exec(SalDatVusrequete);
    proc->TraiteErreurRequete(RemplirTableViewListJourQuery,SalDatVusrequete, tr("Impossible de remplir la salle d'attente!"));
    RemplirTableViewListJourQuery.first();
    TableAMettreAJour->clearContents();
    TableAMettreAJour->setRowCount(RemplirTableViewListJourQuery.size());

    for (i = 0; i < RemplirTableViewListJourQuery.size(); i++)
    {
        UpLabel *label0, *label1, *label2, *label3, *label4;
        label0 = new UpLabel;
        label1 = new UpLabel;
        label2 = new UpLabel;
        label3 = new UpLabel;
        label4 = new UpLabel;

        label0->setId(RemplirTableViewListJourQuery.value(0).toInt());                      // idPat
        label1->setId(RemplirTableViewListJourQuery.value(0).toInt());                      // idPat
        label2->setId(RemplirTableViewListJourQuery.value(0).toInt());                      // idPat
        label3->setId(RemplirTableViewListJourQuery.value(0).toInt());                      // idPat
        label4->setId(RemplirTableViewListJourQuery.value(0).toInt());                      // idPat

        label0->setContextMenuPolicy(Qt::CustomContextMenu);
        label1->setContextMenuPolicy(Qt::CustomContextMenu);
        label2->setContextMenuPolicy(Qt::CustomContextMenu);
        label3->setContextMenuPolicy(Qt::CustomContextMenu);
        label4->setContextMenuPolicy(Qt::CustomContextMenu);

        NomPrenom = RemplirTableViewListJourQuery.value(2).toString().toUpper()
                + " " + RemplirTableViewListJourQuery.value(3).toString();
        zw = RemplirTableViewListJourQuery.value(8).toTime().toString("HH:mm");
        if (zw == "") zw = "hors RDV";
        label0->setText(" " + zw);                                                                                // Heure acte
        label1->setText(" " + NomPrenom);                                                                         // Nom + Prénom
        QString P=RemplirTableViewListJourQuery.value(9).toString();
        if (P == "E")           P = "Espèces";
        else if (P == "C")      P = "Chèque";
        else if (P == "I")      P = "Impayé";
        else if (P == "G")      P = "Gratuit";
        else if (P == "T")
        {
            P = RemplirTableViewListJourQuery.value(10).toString();
            if (P == "CB")      P = "Carte";
        }
        label2->setText(" " + P);                                                                                 // Mode de paiement
        label3->setText(" " + RemplirTableViewListJourQuery.value(6).toString());                                 // Cotation
        label4->setText(QLocale().toString(RemplirTableViewListJourQuery.value(7).toDouble(),'f',2) + " ");       // Montant

        label0->setAlignment(Qt::AlignLeft);
        label1->setAlignment(Qt::AlignLeft);
        label3->setAlignment(Qt::AlignLeft);
        label4->setAlignment(Qt::AlignRight);

        connect (label0,        SIGNAL(customContextMenuRequested(QPoint)),         this,       SLOT (Slot_MenuContextuelSalDat()));
        connect (label1,        SIGNAL(customContextMenuRequested(QPoint)),         this,       SLOT (Slot_MenuContextuelSalDat()));
        connect (label2,        SIGNAL(customContextMenuRequested(QPoint)),         this,       SLOT (Slot_MenuContextuelSalDat()));
        connect (label3,        SIGNAL(customContextMenuRequested(QPoint)),         this,       SLOT (Slot_MenuContextuelSalDat()));
        connect (label4,        SIGNAL(customContextMenuRequested(QPoint)),         this,       SLOT (Slot_MenuContextuelSalDat()));
        connect (label0,        SIGNAL(enter(int)),                                 this,       SLOT (Slot_AfficheToolTip(int)));
        connect (label1,        SIGNAL(enter(int)),                                 this,       SLOT (Slot_AfficheToolTip(int)));
        connect (label2,        SIGNAL(enter(int)),                                 this,       SLOT (Slot_AfficheToolTip(int)));
        connect (label3,        SIGNAL(enter(int)),                                 this,       SLOT (Slot_AfficheToolTip(int)));
        connect (label4,        SIGNAL(enter(int)),                                 this,       SLOT (Slot_AfficheToolTip(int)));
        connect (label0,        SIGNAL(dblclick(int)),                              this,       SLOT (Slot_ChoixDossier(int)));
        connect (label1,        SIGNAL(dblclick(int)),                              this,       SLOT (Slot_ChoixDossier(int)));
        connect (label2,        SIGNAL(dblclick(int)),                              this,       SLOT (Slot_ChoixDossier(int)));
        connect (label3,        SIGNAL(dblclick(int)),                              this,       SLOT (Slot_ChoixDossier(int)));
        connect (label4,        SIGNAL(dblclick(int)),                              this,       SLOT (Slot_ChoixDossier(int)));

        TableAMettreAJour->setCellWidget(i,0,label0);
        TableAMettreAJour->setCellWidget(i,1,label1);
        TableAMettreAJour->setCellWidget(i,2,label2);
        TableAMettreAJour->setCellWidget(i,3,label3);
        TableAMettreAJour->setCellWidget(i,4,label4);
        TableAMettreAJour->setRowHeight(i,fm.height()*1.1);

        RemplirTableViewListJourQuery.next();
    }
}


/*-----------------------------------------------------------------------------------------------------------------
-- Salle d'attente ------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
bool Rufus::SalleDattente(QString Titre)
{
    int idPatAPasser    = gidPatient;
    int idActeAPasser   = gidActe;
    QString TitreAPasser= gNomPatient + " " + gPrenomPatient + "\n" + Titre;
    bool retour         = false;

    Dlg_SalDat           = new dlg_salledattente(&idPatAPasser, &idActeAPasser, &TitreAPasser, proc, this);
    Dlg_SalDat->setWindowTitle(tr("Consultation incomplète!"));

    if (Titre == tr("Il manque le montant!") || Titre == tr("Il manque la cotation!"))
        Dlg_SalDat->ui->RetourAccueilradioButton->setEnabled(false);
    if (Titre.contains(tr("il manque les informations de paiement")))
        Dlg_SalDat->ui->RetourAccueilradioButton->setEnabled(true);

    if (Dlg_SalDat->exec() > 0)
        retour = true;
    Dlg_SalDat->close(); // nécessaire pour enregistrer la géométrie
    delete Dlg_SalDat;
    return retour;
}

/*-----------------------------------------------------------------------------------------------------------------
-- Supprimer un acte ----------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void Rufus::SupprimerActe()
{
    UpMessageBox     msgbox;
    int             idASupprimer, idAAficher;


    // On vérifie que cet acte n'a pas été payé par une opération bancaire déjà enregistrée sur le compte
    // on récupère les lignes de paiement
    QString Messg ="";
    QString requete = "SELECT idRecette, Paye FROM " NOM_TABLE_LIGNESPAIEMENTS " WHERE idActe = " + QString::number(gidActe);
    QSqlQuery ListeRecettesQuery (requete,db);
    proc->TraiteErreurRequete(ListeRecettesQuery,requete,"");
    ListeRecettesQuery.first();
    for (int l = 0; l < ListeRecettesQuery.size(); l++)
    {
        // on vérifie pour chaque ligne s'il s'agit d'un virement, d'une carte bleue ou d'un chèque enregistré
        requete = "SELECT ModePaiement, NomTiers, idRemise FROM " NOM_TABLE_RECETTES " WHERE idRecette = " + ListeRecettesQuery.value(0).toString();
        QSqlQuery ModePaiementQuery (requete,db);
        proc->TraiteErreurRequete(ModePaiementQuery,requete,"");
        ModePaiementQuery.first();
        if (ModePaiementQuery.value(0).toString() == "V")                                                  Messg = tr("Je crains de ne pas pouvoir supprimer cet acte\nIl y a des versements enregistrés.");
        if (ModePaiementQuery.value(0).toString() == "V" && ModePaiementQuery.value(1).toString() == "CB") Messg = tr("Je crains de ne pas pouvoir supprimer cet acte\nIl y a des paiements par carte de crédit enregistrés.");
        if (ModePaiementQuery.value(0).toString() == "C" && ModePaiementQuery.value(2).toInt() > 0)        Messg = tr("Je crains de ne pas pouvoir supprimer cet acte\nIl y a des paiements par chèque enregistrés.");
        if (ModePaiementQuery.value(0).toString() == "C" && ModePaiementQuery.value(2).toInt() == 0)       Messg = tr("Je crains de ne pas pouvoir supprimer cet acte\nIl y a des paiements par chèque enregistrés."
                                                                                                                   "\nCe ou ces chèques ne sont pas encaissés."
                                                                                                                   "\nVous devez modifier l'écriture correspondante pour pouvoir supprimer l'acte.");
        if (Messg != "")
            l = ListeRecettesQuery.size();
        ListeRecettesQuery.next();
    }
    if (Messg != "")
    {
        UpMessageBox::Watch(this, "Euuhh... " + gUserLogin, Messg);
        return;
    }

    // On demande confirmation de la suppression de l'acte
    msgbox.setText("Euuhh... " + gUserLogin);
    msgbox.setInformativeText(tr("Etes vous sûr de vouloir supprimer cet acte?"));
    msgbox.setIcon(UpMessageBox::Warning);
    UpSmallButton *OKBouton = new UpSmallButton();
    OKBouton->setText(tr("Supprimer"));
    UpSmallButton *NoBouton = new UpSmallButton();
    NoBouton->setText(tr("Annuler"));
    msgbox.addButton(NoBouton,UpSmallButton::CANCELBUTTON);
    msgbox.addButton(OKBouton, UpSmallButton::SUPPRBUTTON);
    msgbox.exec();
    if (msgbox.clickedButton() != OKBouton)
        return;

    // On récupère la date de l'acte
     //on va rechercher l'idActe suivant
    idASupprimer = gidActe;
    idAAficher = 0;
    QDate dateacte;

    requete = "SELECT idActe, ActeDate FROM " NOM_TABLE_ACTES " WHERE idPat = '" + QString::number(gidPatient) + "'";
    QSqlQuery       SupprimerActeQuery (requete,db);
    if (proc->TraiteErreurRequete(SupprimerActeQuery,requete, tr("Impossible de retrouver les consultations de ce patient")))
        return;
    if (SupprimerActeQuery.size() == 0)
    {
        UpMessageBox::Watch(this, tr("Impossible de retrouver les données de l'acte"));
        return;
    }
    SupprimerActeQuery.last();
    dateacte = SupprimerActeQuery.value(1).toDate();
    if (SupprimerActeQuery.size() < 2)           // Aucune autre consultation trouvee pour ces criteres
    {
        gidActe = 0;
        ui->Acteframe->setVisible(false);
        ui->CreerActepushButton_2->setVisible(true);
        ui->CreerBOpushButton_2->setVisible(true);
        ui->idActelineEdit->clear();
    }
    else
    {
        SupprimerActeQuery.first();
        for (int a = 0; a < SupprimerActeQuery.size(); a++)
        {
            if (SupprimerActeQuery.value(0).toInt() == gidActe) break;
            SupprimerActeQuery.next();
        }
        if (SupprimerActeQuery.at() < SupprimerActeQuery.size() - 1)
        {   //on est au milieu des actes -> on va rechercher l'idActe suivant
            SupprimerActeQuery.next();
            idAAficher = SupprimerActeQuery.value(0).toInt();
        }
        else
        {
            //on est sur le dernier acte -> on va rechercher l'idActe précédant
            SupprimerActeQuery.previous();
            idAAficher = SupprimerActeQuery.value(0).toInt();
        }
        if (idAAficher == 0)  return;
    }

    // on supprime les éventuelles réfractions liées à cette consultation -----------------------------------------------------------
    requete = "DELETE FROM " NOM_TABLE_REFRACTION " WHERE idActe  = " + QString::number(idASupprimer);
    QSqlQuery DelRefractionQuery(requete,db);
    proc->TraiteErreurRequete(DelRefractionQuery,requete,"");

    // on supprime les éventuels bilans orthoptiques liés à cette consultation -----------------------------------------------------------
    QString delborequete = "DELETE FROM " NOM_TABLE_BILANORTHO " WHERE idBilanOrtho  = " + QString::number(idASupprimer);
    QSqlQuery DelBOQuery(delborequete,db);
    proc->TraiteErreurRequete(DelBOQuery,delborequete,"");

    // On supprime les documents émis
    requete =  "DELETE FROM " NOM_TABLE_IMPRESSIONS " WHERE idPat = " + QString::number(gidPatient) + " and dateimpression = '" + dateacte.toString("yyyy-MM-dd") + "'";
    QSqlQuery DelPatientImpressionsQuery(requete,db);
    if (proc->TraiteErreurRequete(DelPatientImpressionsQuery,requete, tr("Impossible de supprimer les impressoins de ce dossier!")))
        return;
    QList<dlg_docsexternes *> ListDialogDocs = this->findChildren<dlg_docsexternes *>();
    if (ListDialogDocs.size()>0)
        for (int n = 0; n <  ListDialogDocs.size(); n++)
            ListDialogDocs.at(n)->RemplirTreeView();

    /* on corrige la compta
    */
    if (ListeRecettesQuery.size()>0)     // inutile de le faire pour les gratuits et les impayés ou les tiers non encore encaissés
        /* on corrige les lignes de recette correspondant à ce dossier -------------------------------------------------------------------------
        // ça ne peut concerner que des paiements en espèces -----------------------------------------------------------------------------------
        parce qu'on ne peut pas supprimer les dossiers pour lesquels des recettes ont été enregistrées avec d'autres formes de paiement
        (chèque ou virement)
        */
    {
        QList<int>      listrecettesacorriger;
        QList<double>   listmontantsacorriger;
        ListeRecettesQuery.first();
        for (int l = 0; l < ListeRecettesQuery.size(); l++)
        {
            listrecettesacorriger << ListeRecettesQuery.value(0).toInt();
            listmontantsacorriger << ListeRecettesQuery.value(1).toDouble();
            ListeRecettesQuery.next();
        }
        for (int j=0; j<listrecettesacorriger.size(); j++)
        {
            int recetteACorriger = listrecettesacorriger.at(j);
            requete = "SELECT Montant FROM " NOM_TABLE_RECETTES " WHERE idRecette = " + QString::number(recetteACorriger);
            QSqlQuery RecetteQuery (requete,db);
            proc->TraiteErreurRequete(RecetteQuery,requete,"");
            if (RecetteQuery.size()>0)
            {
                RecetteQuery.first();
                for (int k=0; k<RecetteQuery.size(); k++)
                {
                    QString req = "delete from " NOM_TABLE_RECETTES " where idrecette = " + QString::number(recetteACorriger);
                    if (RecetteQuery.value(0).toDouble() > listmontantsacorriger.at(j))
                        req = "update " NOM_TABLE_RECETTES " set Montant = " + QString::number(RecetteQuery.value(0).toDouble() - listmontantsacorriger.at(j)) +
                                " where idRecette = " + QString::number(recetteACorriger);
                    QSqlQuery quer(req,db);
                    proc->TraiteErreurRequete(quer,req,"");
                    RecetteQuery.next();
                }
            }
        }

        // On actualise la table des lignes de paiement et la table des Type de paieement
        requete = "DELETE FROM " NOM_TABLE_LIGNESPAIEMENTS " WHERE idActe = " + QString::number(idASupprimer);
        QSqlQuery DelLignesPaiementsQuery(requete,db);
        proc->TraiteErreurRequete(DelLignesPaiementsQuery,requete,"");
        requete = "DELETE FROM " NOM_TABLE_TYPEPAIEMENTACTES " WHERE idActe = " + QString::number(idASupprimer);
        QSqlQuery DelTypesPaiementsQuery(requete,db);
        proc->TraiteErreurRequete(DelTypesPaiementsQuery,requete,"");
    }

    // on supprime la consultation -------------------------------------------------------------------------------------------------
    requete = "DELETE FROM " NOM_TABLE_ACTES " WHERE idActe = " + QString::number(idASupprimer);
    QSqlQuery DelActesQuery(requete,db);
    if (proc->TraiteErreurRequete(DelActesQuery,requete, tr("Impossible de supprimer cet acte!")))
        return;

    // On affiche la nouvelle consultation
    if (gidActe > 0)
        AfficheActe(idAAficher);

    // On met à jour l'affichage éventuel de dlg_actesprecedents
    QList<dlg_actesprecedents *> listactesprecs = findChildren<dlg_actesprecedents *>();
    for (int i = 0; i<listactesprecs.size();i++)
    {
        if (listactesprecs.at(i)->getidPatient() != gidPatient)
        {
            listactesprecs.at(i)->close();
            listactesprecs.removeAt(i);
            i--;
        }
    }
    if (listactesprecs.size()>0)
    {
        if (SupprimerActeQuery.size() < 3)
        {
            listactesprecs.at(0)->close();
            listactesprecs.removeAt(0);
        }
        else
            listactesprecs.at(0)->Actualise();
    }
}

/*-----------------------------------------------------------------------------------------------------------------
-- Supprimer un dossier--------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void Rufus::SupprimerDossier(int idpat)
{
    // On ne peut supprimer un dossier que s'il n'y a pas eu d'actes avec des tiers payants, des chèques ou des cartes de crédit enregistrés pour ce dossier.
    UpMessageBox     msgbox, msgbox2;
    gidPatient = idpat;

    //1. On recherche le nom et le prenom du patient s'ils ne sont pas déterminés - c'est le cas quand il n'y a pas de dossier affiché
    if (ui->tabWidget->indexOf(ui->tabDossier) < 0)
    {
        QSqlQuery quer0("select patnom, PatPrenom from " NOM_TABLE_PATIENTS " where idpat = " + QString::number(gidPatient), db);
        if (quer0.size() == 0) return;
        quer0.first();
        gNomPatient = quer0.value(0).toString();
        gPrenomPatient = quer0.value(1).toString();
    }

    //2. On recherche les actes de ce dossier qui seraient en tiers payant et qui auraient déjà reçu des versements auquel cas, on ne peut pas supprimer les actes ni le dossier
    QString Messg = "";
    QString requete = " SELECT idRecette, Paye FROM " NOM_TABLE_LIGNESPAIEMENTS " WHERE idActe in (SELECT act.idActe FROM " NOM_TABLE_ACTES " act WHERE act.idPat = '" + QString::number(gidPatient) + "')";
    QSqlQuery ListeRecettesQuery (requete,db);
    proc->TraiteErreurRequete(ListeRecettesQuery,requete,"");
    ListeRecettesQuery.first();
    for (int l = 0; l < ListeRecettesQuery.size(); l++)
    {
        // on vérifie pour chaque ligne s'il s'agit d'un virement ou d'une carte bleue ou d'un chèque enregistré
        requete = "SELECT ModePaiement, NomTiers, idRemise FROM " NOM_TABLE_RECETTES " WHERE idRecette = " + ListeRecettesQuery.value(0).toString();
        QSqlQuery ModePaiementQuery (requete,db);
        proc->TraiteErreurRequete(ModePaiementQuery,requete,"");
        ModePaiementQuery.first();
        if (ModePaiementQuery.value(0).toString() == "V")
            Messg = tr("Je crains de ne pas pouvoir supprimer ce dossier\nIl y a des versements enregistrés.");
        if (ModePaiementQuery.value(0).toString() == "V" && ModePaiementQuery.value(1).toString() == "CB")
            Messg = tr("Je crains de ne pas pouvoir supprimer ce dossier\nIl y a des paiements par carte de crédit enregistrés.");
        if (ModePaiementQuery.value(0).toString() == "C" && ModePaiementQuery.value(2).toInt() > 0)
            Messg = tr("Je crains de ne pas pouvoir supprimer cet acte\nIl y a des paiements par chèque enregistrés.");
        if (ModePaiementQuery.value(0).toString() == "C" && ModePaiementQuery.value(2).toInt() == 0)
            Messg = tr("Je crains de ne pas pouvoir supprimer ce dossier\nIl y a des paiements par chèque enregistrés."
                    "\nCe ou ces chèques ne sont pas encaissés."
                    "\nVous devez modifier l'écriture correspondante pour pouvoir supprimer l'acte.");
        if (Messg != "") l = ListeRecettesQuery.size();
        ListeRecettesQuery.next();
    }
    if (Messg != "")
    {
        UpMessageBox::Watch(this, "Euuhh... " + gUserLogin, Messg);
        return;
    }

    //3. On commence par demander la confirmation de la suppression du dossier
    msgbox.setText("Euuhh... " + gUserLogin);
    msgbox.setInformativeText(tr("Etes vous sûr de vouloir supprimer le dossier de ") + gNomPatient + " " + gPrenomPatient + "?");
    msgbox.setIcon(UpMessageBox::Warning);
    UpSmallButton *OKBouton = new UpSmallButton();
    OKBouton->setText(tr("Supprimer"));
    UpSmallButton *NoBouton = new UpSmallButton();
    NoBouton->setText(tr("Annuler"));
    msgbox.addButton(NoBouton, UpSmallButton::CANCELBUTTON);
    msgbox.addButton(OKBouton, UpSmallButton::SUPPRBUTTON);
    msgbox.exec();
    if (msgbox.clickedButton() != OKBouton)
        return;

    /* on corrige la compta
    */
    if (ListeRecettesQuery.size()>0)     // inutile de le faire pour les gratuits et les impayés ou les tiers non encore encaissés
        /* on corrige les lignes de recette correspondant à ce dossier -------------------------------------------------------------------------
        // ça ne peut concerner que des paiements en espèces -----------------------------------------------------------------------------------
        parce qu'on ne peut pas supprimer les dossiers pour lesquels des recettes ont été enregistrées avec d'autres formes de paiement
        (chèque ou virement)
        */
    {
        QList<int>      listrecettesacorriger;
        QList<double>   listmontantsacorriger;
        ListeRecettesQuery.first();
        for (int l = 0; l < ListeRecettesQuery.size(); l++)
        {
            listrecettesacorriger << ListeRecettesQuery.value(0).toInt();
            listmontantsacorriger << ListeRecettesQuery.value(1).toDouble();
            ListeRecettesQuery.next();
        }
        for (int j=0; j<listrecettesacorriger.size(); j++)
        {
            int recetteACorriger = listrecettesacorriger.at(j);
            requete = "SELECT Montant FROM " NOM_TABLE_RECETTES " WHERE idRecette = " + QString::number(recetteACorriger);
            QSqlQuery RecetteQuery (requete,db);
            proc->TraiteErreurRequete(RecetteQuery,requete,"");
            if (RecetteQuery.size()>0)
            {
                RecetteQuery.first();
                for (int k=0; k<RecetteQuery.size(); k++)
                {
                    QString req = "delete from " NOM_TABLE_RECETTES " where idrecette = " + QString::number(recetteACorriger);
                    if (RecetteQuery.value(0).toDouble() > listmontantsacorriger.at(j))
                        req = "update " NOM_TABLE_RECETTES " set Montant = " + QString::number(RecetteQuery.value(0).toDouble() - listmontantsacorriger.at(j)) +
                                " where idRecette = " + QString::number(recetteACorriger);
                    QSqlQuery quer(req,db);
                    proc->TraiteErreurRequete(quer,req,"");
                    RecetteQuery.next();
                }
            }
        }
        //4, On actualise la table des lignes de paiement el la table des Type de paieement
        requete = "DELETE FROM " NOM_TABLE_LIGNESPAIEMENTS " WHERE idActe in (SELECT act.idActe FROM " NOM_TABLE_ACTES " act WHERE act.idPat = '" + QString::number(gidPatient) + "')";
        QSqlQuery DelLignesPaiementsQuery(requete,db);
        proc->TraiteErreurRequete(DelLignesPaiementsQuery,requete,"");
        requete = "DELETE FROM " NOM_TABLE_TYPEPAIEMENTACTES " WHERE idActe in (SELECT act.idActe FROM " NOM_TABLE_ACTES " act WHERE act.idPat = '" + QString::number(gidPatient) + "')";
        QSqlQuery DelTypesPaiementsQuery(requete,db);
        proc->TraiteErreurRequete(DelTypesPaiementsQuery,requete,"");
    }

    //5. On ferme l'onglet dossier
    ui->tabWidget->removeTab(ui->tabWidget->indexOf(ui->tabDossier));
    FermeDlgAnnexes();

    //6. On supprime les bilans orthoptiques correspondants à ce dossier
    requete =  "DELETE FROM " NOM_TABLE_BILANORTHO " WHERE idbilanortho in (SELECT idActe from " NOM_TABLE_ACTES " where idPat = " + QString::number(gidPatient) + ")";
    QSqlQuery       SupprimerblorthoQuery (requete,db);
    if (proc->TraiteErreurRequete(SupprimerblorthoQuery,requete, tr("Impossible de retrouver les bilans orthoptiques de ce patient")))
        return;

    //7. On supprime tous les actes correspondants à ce dossier
    requete =  "DELETE FROM " NOM_TABLE_ACTES " WHERE idPat = " + QString::number(gidPatient);
    QSqlQuery       SupprimerActeQuery (requete,db);
    if (proc->TraiteErreurRequete(SupprimerActeQuery,requete, tr("Impossible de retrouver les consultations de ce patient")))
        return;

    //8. On supprime le dossier dans la base Rufus
    requete =  "DELETE FROM " NOM_TABLE_PATIENTS " WHERE idPat = " + QString::number(gidPatient);
    QSqlQuery DelPatientQuery(requete,db);
    if (proc->TraiteErreurRequete(DelPatientQuery,requete, tr("Impossible de supprimer ce dossier!")))
        return;

    //9. On supprime les documents émis
    requete =  "DELETE FROM " NOM_TABLE_IMPRESSIONS " WHERE idPat = " + QString::number(gidPatient);
    QSqlQuery DelPatientImpressionsQuery(requete,db);
    if (proc->TraiteErreurRequete(DelPatientImpressionsQuery,requete, tr("Impossible de supprimer les impressoins de ce dossier!")))
        return;

    requete =  "DELETE FROM " NOM_TABLE_DONNEESSOCIALESPATIENTS " WHERE idPat = " + QString::number(gidPatient);
    QSqlQuery MAJSocialQuery (requete,db);
    if (proc->TraiteErreurRequete(MAJSocialQuery,requete, tr("Impossible de supprimer ce dossier de la table des données sociales!")))
        return;

    requete =  "DELETE FROM " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS " WHERE idPat = " + QString::number(gidPatient);
    QSqlQuery MAJMedicalQuery (requete,db);
    if (proc->TraiteErreurRequete(MAJMedicalQuery,requete, tr("Impossible de supprimer ce dossier de la table des données medicales!")))
        return;

    requete =  "DELETE FROM " NOM_TABLE_MOTSCLESJOINTURES " WHERE idPat = " + QString::number(gidPatient);
    QSqlQuery MAJMotsQuery (requete,db);
    if (proc->TraiteErreurRequete(MAJMotsQuery,requete, tr("Impossible de supprimer ce dossier de la table des mots clés!")))
        return;

    requete =  "DELETE FROM " NOM_TABLE_SALLEDATTENTE " WHERE idPat = " + QString::number(gidPatient);
    QSqlQuery MAJSalDatQuery (requete,db);
    if (proc->TraiteErreurRequete(MAJSalDatQuery,requete, tr("Impossible de supprimer ce dossier de la salle d'attente!")))
        return;
    proc->UpdVerrouSalDat();

    //10. On supprime le dossier dans la base OPhtalmologie
    requete =  "DELETE FROM " NOM_TABLE_REFRACTION
            " WHERE idPat = " + QString::number(gidPatient);
    QSqlQuery DelRefractionQuery (requete,db);
    proc->TraiteErreurRequete(DelRefractionQuery,requete, tr("Impossible de supprimer ce dossier de la table des réfractions!"));

    requete =  "DELETE FROM " NOM_TABLE_DONNEES_OPHTA_PATIENTS
            " WHERE idPat = " + QString::number(gidPatient);
    QSqlQuery DelDonneesOphtaQuery (requete,db);
    proc->TraiteErreurRequete(DelDonneesOphtaQuery,requete, tr("Impossible de supprimer ce dossier de la table des données ophtalmologiques!"));

    requete =  "DELETE FROM " NOM_TABLE_BIOMETRIES
            " WHERE idPat = " + QString::number(gidPatient);
    QSqlQuery DelBiometrieQuery (requete,db);
    proc->TraiteErreurRequete(DelBiometrieQuery,requete, tr("Impossible de supprimer ce dossier de la table des biométries!"));

    requete =  "DELETE FROM " NOM_TABLE_TONOMETRIE
            " WHERE idPat = " + QString::number(gidPatient);
    QSqlQuery DelTonoQuery (requete,db);
    proc->TraiteErreurRequete(DelTonoQuery,requete, tr("Impossible de supprimer ce dossier de la table des tonométries!"));


    //10. On reconstruit le treeView Liste
    FlagMetAjourTreeView();
    gidPatient = idPatientAuHasard;
    OuvrirListe();

    //11. On ferme la fiche dlg_actesprecedents
    FermeDlgAnnexes();
}

/*-----------------------------------------------------------------------------------------------------------------
-- Tonometrie -----------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void Rufus::Tonometrie()
{
    if (ui->tabWidget->currentIndex() != ui->tabWidget->indexOf(ui->tabDossier)) return;
    int idPatAPasser = gidPatient;

    Dlg_AutresMes           = new dlg_autresmesures(&idPatAPasser, proc, dlg_autresmesures::TONO, this);
    QString TOD, TOG, Methode, TODcolor, TOGcolor;
    Dlg_AutresMes->setWindowTitle(tr("Tonométrie - ") + gNomPatient + " " + gPrenomPatient);

    if (Dlg_AutresMes->exec()> 0)
    {
        TOD = Dlg_AutresMes->widgto->ui->TOODSpinBox->text();
        TOG = Dlg_AutresMes->widgto->ui->TOOGSpinBox->text();
        if (TOD.toInt() > 21)
            TODcolor = "<font color = \"red\"><b>" + TOD + "</b></font>";
        else
            TODcolor = "<font color = \"blue\"><b>" + TOD + "</b></font>";
        if (TOG.toInt() > 21)
            TOGcolor = "<font color = \"red\"><b>" + TOG + "</b></font>";
        else
            TOGcolor = "<font color = \"blue\"><b>" + TOG + "</b></font>";
        if (TOD.toInt() > 0 || TOG.toInt() > 0)
        {
            if (Dlg_AutresMes->widgto->ui->AirRadioButton->isChecked())        Methode = tr("Air");
            if (Dlg_AutresMes->widgto->ui->AutreRadioButton->isChecked())      Methode = tr("Autre");
            if (Dlg_AutresMes->widgto->ui->AplanationRadioButton->isChecked()) Methode = tr("Aplanation");

            QString Tono;
            if (TOD.toInt() == 0 && TOG.toInt() > 0)
                Tono = "<td width=\"60\"><font color = \"" + proc->CouleurTitres + "\"><b>" + tr("TOG:") + "</b></font></td><td width=\"80\">" + TOGcolor + " à " + QTime::currentTime().toString("H") + "H</td><td width=\"80\">(" + Methode + ")</td><td>" + gUserLogin + "</td>";
            if (TOG.toInt() == 0 && TOD.toInt() > 0)
                Tono = "<td width=\"60\"><font color = \"" + proc->CouleurTitres + "\"><b>" + tr("TOD:") + "</b></font></td><td width=\"80\">" + TODcolor + " à " + QTime::currentTime().toString("H") + "H</td><td width=\"80\">(" + Methode + ")</td><td>" + gUserLogin + "</td>";
            if (TOD.toInt() > 0 && TOG.toInt() > 0)
            {
                if (TOD.toInt() == TOG.toInt())
                    Tono = "<td width=\"60\"><font color = \"" + proc->CouleurTitres + "\"><b>" + tr("TODG:") + "</b></font></td><td width=\"80\">" + TODcolor + " à " + QTime::currentTime().toString("H") + "H</td><td width=\"80\">(" + Methode + ")</td><td>" + gUserLogin + "</td>";
                else
                    Tono = "<td width=\"60\"><font color = \"" + proc->CouleurTitres + "\"><b>" + tr("TO:") +"</b></font></td><td width=\"80\">" + TODcolor + "/" + TOGcolor+ " à " + QTime::currentTime().toString("H") + "H</td><td width=\"80\">(" + Methode + ")</td><td>" + gUserLogin + "</td>";
            }

            // si le dernier caractère n'est pas un retour à la ligne, on en rajoute un
            QString texte = ui->ActeTextetextEdit->toHtml();
            bool a = true;
            while (a) {
                     // il faut retirer la dernière ligne du html qui contient le retour à la ligne
                int debut = texte.lastIndexOf("<p");
                int fin   = texte.lastIndexOf("</p>");
                int longARetirer = fin - debut + 4;
                if (texte.mid(debut,longARetirer).contains("-qt-paragraph-type:empty;"))
                    texte.remove(debut,longARetirer);
                else a = false;
            }
            QString ARajouterEnText =  "<p style = \"margin-top:0px; margin-bottom:0px;\" >" + Tono  + "</p>"
                    + "<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px;\"></p>";
            if (ui->ActeTextetextEdit->toPlainText().size() == 0)
                texte = ARajouterEnText;
            else
                texte += ARajouterEnText;
            ui->ActeTextetextEdit->setText(texte);

            QString updaterequete =  "UPDATE " NOM_TABLE_ACTES " SET ActeTexte = '" + proc->CorrigeApostrophe(ui->ActeTextetextEdit->toHtml()) +
                    "' where idActe = " + ui->idActelineEdit->text();
            QSqlQuery UpdateUpTextEditQuery (updaterequete,db);
            proc->TraiteErreurRequete(UpdateUpTextEditQuery,updaterequete, tr("Impossible de mettre à jour le champ Texte !"));
        }
        ui->ActeTextetextEdit->setFocus();
        ui->ActeTextetextEdit->moveCursor(QTextCursor::End);
    }
    Dlg_AutresMes->close(); // nécessaire pour enregistrer la géométrie
    delete Dlg_AutresMes;
}

/*------------------------------------------------------------------------------------------------------------------------------------
-- Créer le ToolBar en vue liste -----------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------------------------------------------*/
void Rufus::ToolBarDossier()
{
    ToolBarListe();
    ui->mainToolBar->addSeparator();
    ui->mainToolBar->addAction(proc->giconAvant, tr("Dossier précédent du fichier patients"),this,SLOT (Slot_NavigationDossierPrecedentListe()));
    ui->mainToolBar->addAction(proc->giconApres,tr("Dossier suivant du fichier patients"),this,SLOT (Slot_NavigationDossierSuivantListe()));
}

/*------------------------------------------------------------------------------------------------------------------------------------
-- Créer le ToolBar en vue liste -----------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------------------------------------------*/
void Rufus::ToolBarListe()
{

    ui->mainToolBar->clear();
    ui->mainToolBar->setIconSize(QSize(25,25));
    ui->mainToolBar->setFixedHeight(40);
    ui->mainToolBar->addAction(proc->giconCreer, tr("créer un nouveau dossier"),this,SLOT (Slot_OuvrirNouveauDossierpushButtonClicked()));
    if (gListePatientsModel->rowCount() > 0)
        ui->mainToolBar->addAction(proc->giconRecopier, tr("créer un dossier à partir d'un parent\nl'adresse et les antécédents familiaux\nseront recopiés automatiquement"),this,SLOT (Slot_RecopierDossierpushButtonClicked()));
    ui->mainToolBar->addSeparator();
    ui->mainToolBar->addAction(proc->giconListe, tr("ouvrir un dossier"),this,SLOT (Slot_OuvrirListepushButtonClicked()));
    ui->mainToolBar->addSeparator();
    ui->mainToolBar->addAction(proc->giconCPS,"Lire la CPS",this,SLOT (Slot_LireLaCPSpushButtonClicked()));                                                 // CZ001
    ui->mainToolBar->addAction(proc->giconVitale,"ouvrir ou créer un dossier à partir de la carte vitale",this,SLOT (Slot_LireLaCVpushButtonClicked()));    // CZ001
    //ui->mainToolBar->addAction(proc->giconVitale,"ouvrir ou créer un dossier à partir de la carte vitale",this,SLOT (Slot_OuvrirRecopierDossierpushButtonClicked()));
}

/*-----------------------------------------------------------------------------------------------------------------
-- Trouver un dossier d'après la DDN ------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void Rufus::TrouverDDN()
{
    QString requete = "SELECT idPat, PatNom, PatPrenom, PatDDN FROM " NOM_TABLE_PATIENTS
            " WHERE PatDDN = '" + ui->CreerDDNdateEdit->date().toString("yyyy-MM-dd") + "'";
    QSqlQuery   TrouverDDNQuery (requete,db);
    if (!proc->TraiteErreurRequete(TrouverDDNQuery,requete,tr("Impossible de retrouver un patient pour cette date de naissance")))
    {
        gListePatientsModel = dynamic_cast<QStandardItemModel*>(ui->PatientsListeTableView->model());
    if (gListePatientsModel)
        gListePatientsModel->clear();
    else
        gListePatientsModel = new QStandardItemModel;
    }
    gNombreDossiers = TrouverDDNQuery.size();
    if (gNombreDossiers > 0)
    {
        TrouverDDNQuery.first();
        QStandardItem *pitem, *pitem0, *pitem1;
        for (int i=0;i<gNombreDossiers;i++)
        {
            pitem   = new QStandardItem(TrouverDDNQuery.value(0).toString());                                                             // IdPatient
            pitem0  = new QStandardItem(TrouverDDNQuery.value(1).toString().toUpper() + " " + TrouverDDNQuery.value(2).toString());  // Nom + Prénom
            pitem1  = new QStandardItem(TrouverDDNQuery.value(3).toDate().toString(tr("dd-MM-yyyy")));                                        // date de naissance
            QList<QStandardItem *> pitemlist;
            pitemlist << pitem << pitem0 << pitem1;
            gListePatientsModel->appendRow(pitemlist);
            TrouverDDNQuery.next();
        }
        QStandardItem *itnom = new QStandardItem();
        itnom->setText(tr("Nom"));
        itnom->setTextAlignment(Qt::AlignLeft);
        gListePatientsModel->setHorizontalHeaderItem(1,itnom);
        QStandardItem *itDDN = new QStandardItem();
        itDDN->setText(tr("Date de naissance"));
        itDDN->setTextAlignment(Qt::AlignLeft);
        gListePatientsModel->setHorizontalHeaderItem(2,itDDN);
        ui->PatientsListeTableView->setModel(gListePatientsModel);
        ui->PatientsListeTableView->setColumnWidth(0,0);          //IdPat
        ui->PatientsListeTableView->setColumnWidth(1,230 );       //Nom + Prénom
        ui->PatientsListeTableView->setColumnWidth(2,122 );       //DDN

        QFontMetrics fm(qApp->font());
        for (int j=0; j<gNombreDossiers; j++)
             ui->PatientsListeTableView->setRowHeight(j,fm.height()*1.3);

        ui->PatientsListeTableView->setStyleSheet("QTableView {selection-color: rgb(0,0,0); selection-background-color: rgb(164, 205, 255);}");
        ui->PatientsListeTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
        ui->PatientsListeTableView->horizontalHeader()->setFixedHeight(fm.height()*1.3);

        ui->PatientsListeTableView->selectRow(0);
        ui->PatientsListeTableView->scrollTo(gListePatientsModel->item(0)->index(),QAbstractItemView::PositionAtTop);

        ListeRestreinte = true;
    }
    Slot_EnableCreerDossierButton();
}


/*-----------------------------------------------------------------------------------------------------------------
-- Valider et enregistrer le montant de la consultation en cours en cas de modification ---------------------------
-----------------------------------------------------------------------------------------------------------------*/
bool Rufus::ValideActeMontantLineEdit(QString NouveauMontant, QString AncienMontant)
{
    // On vérifie que le montant entré n'est pas inférieur au montant déjà payé et on invalide dans ce cas
    if (ui->PayelineEdit->isVisible() && QLocale().toDouble(NouveauMontant) < QLocale().toDouble(ui->PayelineEdit->text()))
    {
        QSound::play(NOM_ALARME);
        UpMessageBox::Watch(this, tr("Saisie de montant refusée !"),
                             tr("Le montant que vous souhaitez saisir\n\t") + NouveauMontant
                             + tr(" €\n\nest inférieur à la somme des paiements\ndéjà enregistrés pour cet acte\n\t") + ui->PayelineEdit->text() + " €");
        ui->ActeMontantlineEdit->setText(AncienMontant);
        return false;
    }
    else
         ui->ActeMontantlineEdit->setText(NouveauMontant);

    if (QLocale().toDouble(NouveauMontant) > 0)
    {
        // si le montant entré est supérieur à O, on vérifie qu'il n'y a pas d'enregistrement gratuit pour cet acte et on le supprime au cas où, après confirmation
        QString requete = "SELECT idActe, TypePaiement FROM " NOM_TABLE_TYPEPAIEMENTACTES " WHERE TypePaiement = 'G' AND idActe = " + QString::number(gidActe);
        QSqlQuery RechercheGratuitQuery (requete,db);
        proc->TraiteErreurRequete(RechercheGratuitQuery,requete,"Slot_EditingFinshed()");
        if (RechercheGratuitQuery.size() > 0)
        {
            QSound::play(NOM_ALARME);
            UpMessageBox msgbox;
            msgbox.setText(tr("Cet acte a déjà été enregistré comme acte gratuit !"));
            msgbox.setInformativeText(tr("Annuler et considérer comme acte payant?"));
            msgbox.setIcon(UpMessageBox::Warning);
            UpSmallButton *OKBouton = new UpSmallButton();
            OKBouton->setText(tr("Considérer comme payant"));
            UpSmallButton *AnnulBouton = new UpSmallButton();
            AnnulBouton->setText(tr("Annuler"));
            msgbox.addButton(AnnulBouton, UpSmallButton::CANCELBUTTON);
            msgbox.addButton(OKBouton, UpSmallButton::STARTBUTTON);
            msgbox.exec();
            if (msgbox.clickedButton() != OKBouton)
            {
                ui->ActeMontantlineEdit->setText(AncienMontant);
                return false;
            }
            else
            {
                requete = "DELETE FROM " NOM_TABLE_TYPEPAIEMENTACTES " WHERE idActe = " + QString::number(gidActe);
                QSqlQuery DelGratuitQuery (requete,db);
                proc->TraiteErreurRequete(DelGratuitQuery,requete,"");
                AfficheActeCompta();
            }
        }
        else
            ui->ActeMontantlineEdit->setText(NouveauMontant);
    }
    //on modifie la table Actes avec le nouveau montant
    QString requete = "UPDATE " NOM_TABLE_ACTES " SET ActeMontant = " + QString::number(QLocale().toDouble(NouveauMontant)) + " WHERE idActe = " + QString::number(gidActe);
    QSqlQuery UPDMontantActeQuery (requete,db);
    proc->TraiteErreurRequete(UPDMontantActeQuery,requete,"");
    MAJActesPrecs();

    int idx = ui->ActeCotationcomboBox->currentIndex();
    if (idx>0)
    {
        QStringList listMontantActe = ui->ActeCotationcomboBox->itemData(idx).toStringList();
        double MontantConv, MontantPrat, MontantActe;
        MontantActe = QLocale().toDouble(NouveauMontant);
        MontantConv = listMontantActe.at(0).toDouble();
        MontantPrat = listMontantActe.at(1).toDouble();
        ui->BasculerMontantpushButton->setVisible((MontantActe!=MontantConv) || (MontantActe!=MontantPrat) || (MontantConv!=MontantPrat));
        if (ui->BasculerMontantpushButton->isVisible())
        {
            if (MontantActe!=MontantPrat)
                ui->BasculerMontantpushButton->setImmediateToolTip(tr("Revenir au tarif habituellement pratiqué"));
            else
                ui->BasculerMontantpushButton->setImmediateToolTip(tr("Revenir au tarif conventionnel"));
        }
    }
    return true;
}

void Rufus::Slot_NouvMesureRefraction() //utilisé pour ouvrir la fiche refraction quand un appareil a transmis une mesure
{
    if (!ui->Acteframe->isVisible())
        return;
    for (int i= 0; i<findChildren<QDialog*>().size(); i++)
        if (findChildren<QDialog*>().at(i)->inherits("dlg_refraction"))
            return;
    QString TypeMesure = proc->TypeMesureRefraction();
    if (TypeMesure == "Refracteur")
    {
        QString ARajouterEnText= proc->HtmlRefracteur();
        QString texte = ui->ActeTextetextEdit->toHtml();
        bool a = true;
        while (a) {
            // il faut retirer la dernière ligne du html qui contient le retour à la ligne
            int debut = texte.lastIndexOf("<p");
            int fin   = texte.lastIndexOf("</p>");
            int longARetirer = fin - debut + 4;
            if (texte.mid(debut,longARetirer).contains("-qt-paragraph-type:empty;"))
                texte.remove(debut,longARetirer);
            else a = false;
        }
        if (ui->ActeTextetextEdit->toPlainText().size() == 0)
            texte = ARajouterEnText;
        else
            texte += ARajouterEnText;
        ui->ActeTextetextEdit->setText(texte);

        QString updaterequete =  "UPDATE " NOM_TABLE_ACTES " SET ActeTexte = '" + proc->CorrigeApostrophe(ui->ActeTextetextEdit->toHtml()) +
                "' where idActe = " + ui->idActelineEdit->text();
        QSqlQuery UpdateUpTextEditQuery (updaterequete,db);
        proc->TraiteErreurRequete(UpdateUpTextEditQuery,updaterequete, tr("Impossible de mettre à jour le champ Texte !"));
        ui->ActeTextetextEdit->setFocus();
        ui->ActeTextetextEdit->moveCursor(QTextCursor::End);
        if (!proc->DonneesRefracteurFin().isEmpty() && !proc->DonneesRefracteurSubj().isEmpty())
            proc->InsertRefraction(gidPatient, gidActe, "Subjectif");
        Refraction();
    }
    else if (TypeMesure == "Autoref")
    {
        proc->InsertRefraction(gidPatient, gidActe, "Autoref");
    }
    else if (TypeMesure == "Fronto")
    {
        proc->InsertRefraction(gidPatient, gidActe, "Fronto");
    }
    else if (TypeMesure == "Kerato")
    {
        QString ARajouterEnText= proc->HtmlKerato();
        QString texte = ui->ActeTextetextEdit->toHtml();
        bool a = true;
        while (a) {
            // il faut retirer la dernière ligne du html qui contient le retour à la ligne
            int debut = texte.lastIndexOf("<p");
            int fin   = texte.lastIndexOf("</p>");
            int longARetirer = fin - debut + 4;
            if (texte.mid(debut,longARetirer).contains("-qt-paragraph-type:empty;"))
                texte.remove(debut,longARetirer);
            else a = false;
        }
        if (ui->ActeTextetextEdit->toPlainText().size() == 0)
            texte = ARajouterEnText;
        else
            texte += ARajouterEnText;
        ui->ActeTextetextEdit->setText(texte);

        QString updaterequete =  "UPDATE " NOM_TABLE_ACTES " SET ActeTexte = '" + proc->CorrigeApostrophe(ui->ActeTextetextEdit->toHtml()) +
                "' where idActe = " + ui->idActelineEdit->text();
        QSqlQuery UpdateUpTextEditQuery (updaterequete,db);
        proc->TraiteErreurRequete(UpdateUpTextEditQuery,updaterequete, tr("Impossible de mettre à jour le champ Texte !"));
        ui->ActeTextetextEdit->setFocus();
        ui->ActeTextetextEdit->moveCursor(QTextCursor::End);
        proc->InsertRefraction(gidPatient, gidActe, "Kerato");
    }
    else if (TypeMesure == "Tono")
    {
        QString ARajouterEnText= proc->HtmlTono();
        QString texte = ui->ActeTextetextEdit->toHtml();
        bool a = true;
        while (a) {
            // il faut retirer la dernière ligne du html qui contient le retour à la ligne
            int debut = texte.lastIndexOf("<p");
            int fin   = texte.lastIndexOf("</p>");
            int longARetirer = fin - debut + 4;
            if (texte.mid(debut,longARetirer).contains("-qt-paragraph-type:empty;"))
                texte.remove(debut,longARetirer);
            else a = false;
        }
        if (ui->ActeTextetextEdit->toPlainText().size() == 0)
            texte = ARajouterEnText;
        else
            texte += ARajouterEnText;
        ui->ActeTextetextEdit->setText(texte);

        QString updaterequete =  "UPDATE " NOM_TABLE_ACTES " SET ActeTexte = '" + proc->CorrigeApostrophe(ui->ActeTextetextEdit->toHtml()) +
                "' where idActe = " + ui->idActelineEdit->text();
        QSqlQuery UpdateUpTextEditQuery (updaterequete,db);
        proc->TraiteErreurRequete(UpdateUpTextEditQuery,updaterequete, tr("Impossible de mettre à jour le champ Texte !"));
        ui->ActeTextetextEdit->setFocus();
        ui->ActeTextetextEdit->moveCursor(QTextCursor::End);
    }
    else if (TypeMesure == "Pachy")
    {
        QString ARajouterEnText= proc->HtmlPachy();
        QString texte = ui->ActeTextetextEdit->toHtml();
        bool a = true;
        while (a) {
            // il faut retirer la dernière ligne du html qui contient le retour à la ligne
            int debut = texte.lastIndexOf("<p");
            int fin   = texte.lastIndexOf("</p>");
            int longARetirer = fin - debut + 4;
            if (texte.mid(debut,longARetirer).contains("-qt-paragraph-type:empty;"))
                texte.remove(debut,longARetirer);
            else a = false;
        }
        if (ui->ActeTextetextEdit->toPlainText().size() == 0)
            texte = ARajouterEnText;
        else
            texte += ARajouterEnText;
        ui->ActeTextetextEdit->setText(texte);

        QString updaterequete =  "UPDATE " NOM_TABLE_ACTES " SET ActeTexte = '" + proc->CorrigeApostrophe(ui->ActeTextetextEdit->toHtml()) +
                "' where idActe = " + ui->idActelineEdit->text();
        QSqlQuery UpdateUpTextEditQuery (updaterequete,db);
        proc->TraiteErreurRequete(UpdateUpTextEditQuery,updaterequete, tr("Impossible de mettre à jour le champ Texte !"));
        ui->ActeTextetextEdit->setFocus();
        ui->ActeTextetextEdit->moveCursor(QTextCursor::End);
    }
    else
    {
        proc->setTypeMesureRefraction("");
        Refraction();
    }
}



// CZ001 début interface avec Pyxvital
/*-----------------------------------------------------------------------------------------------------------------
    Lire la CPS avec Pyxvital : Retour = fichier Particien.par
-----------------------------------------------------------------------------------------------------------------*/
void Rufus::Slot_LireLaCPSpushButtonClicked()
{
    QString requete, numPS;
    pyxi = new pyxinterf(proc, this);
    QString nomFicPraticienPar = pyxi->Lecture_CPS();
    delete pyxi;

    // Récup des infos du médecin dans le fichier ../Pyxvital/Interf/Praticien.par
    QSettings settingPraticienPar (nomFicPraticienPar, QSettings::IniFormat);
    settingPraticienPar.setIniCodec ("ISO 8859-1");

    numPS    = settingPraticienPar.value("PS/Numéro").toString() ; // 8 chiffres sans la clé
    if (numPS.length() == 0)
        { //  CPS non lue...
        return;
        }
    // recherche utilisateur avec ce n°ADELI
    requete =   "SELECT idUser FROM " NOM_TABLE_UTILISATEURS " WHERE UserNumPS = '" + numPS + "'" ;
    QSqlQuery ChercheUserQuery (requete,db);
    if (proc->TraiteErreurRequete(ChercheUserQuery,requete, "Impossible d'ouvrir la table Utilisateurs"))
        return;
    else
        {
        if (ChercheUserQuery.size() == 0)  // Aucune mesure trouvee pour ces criteres
            {
            UpMessageBox::Watch(this,"Lecture de la CPS", "Lecture de la carte :\n" +
                                 settingPraticienPar.value("PS/Nom").toString() + " " +
                                 settingPraticienPar.value("PS/Prénom").toString() + "\n" +
                                 settingPraticienPar.value("PS/Spécialité").toString() + "\n" +
                                 "Aucun utilisateur avec le n° de PS : " + numPS);
            return;
            }
        // A REVOIR : faire tout ce qu'il faut pour nouveau user ... mais quoi ???
        ChercheUserQuery.last();
        gidUser = ChercheUserQuery.value(0).toInt();
        gDataUser["idUser"] = gidUser;
        setWindowTitle("Rufus - " + gUserLogin + " - " + gUserFonction);
        }
}
/*-----------------------------------------------------------------------------------------------------------------
    Lire la CV avec Pyxvital : Retour = fichier Patient.par
-----------------------------------------------------------------------------------------------------------------*/
void Rufus::Slot_LireLaCVpushButtonClicked()
{
    QString nomPat, prenomPat, dateNaissPat;
    QString zdat;

    pyxi = new pyxinterf(proc, this);
    QString nomFicPatientPar = pyxi->Lecture_CV();
    delete pyxi;
    if (nomFicPatientPar.length() ==0)
        { // pas de CV lue...
        return;
        }
    // Récup des infos du bénéficiaire dans le fichier ../Pyxvital/Interf/Patient.par
    QSettings settingPatientPar (nomFicPatientPar, QSettings::IniFormat);
    settingPatientPar.setIniCodec ("ISO 8859-1");

    nomPat      = settingPatientPar.value("Bénéficiaire/Nom").toString();
    if (nomPat.length() == 0)
        { // pas de CV lue...
        return;
        }
    prenomPat   = settingPatientPar.value("Bénéficiaire/Prénom").toString();
    dateNaissPat= settingPatientPar.value("Bénéficiaire/Date_de_naissance").toString();                     // JJ/MM/AAAA
    zdat        = dateNaissPat.right(4) + "-" + dateNaissPat.mid(3,2) + "-" + dateNaissPat.left(2) ; // + " 00:00:00";  // AAAA-MM-JJ 00:00:00


    // Recherche d'un patient correspondant dans la base

    // première recherche sur le nom + prenom + date naissance
    QString requete = "SELECT IdPat, PatNom, PatPrenom, PatDDN, Sexe "
                      " FROM "  NOM_TABLE_PATIENTS
            " WHERE UPPER(PatNom) LIKE '" + nomPat.toUpper() + "%'" +
            " AND   UPPER(PatPrenom) LIKE '" + prenomPat.toUpper() + "%'" +
            " AND   PatDDN = '" + zdat + "'";
    Remplir_ListePatients_TableView(requete,"","") ;
    if (gNombreDossiers == 0)       // aucun patient trouvé
        {
        // si rien trouvé, deuxième recherche sur date de naissance seule
        requete = "SELECT IdPat, PatNom, PatPrenom, PatDDN, Sexe  "
                  " FROM "  NOM_TABLE_PATIENTS
                  " WHERE PatDDN = '" + zdat + "'";
        Remplir_ListePatients_TableView(requete,"","") ;
        CalcNbDossiers();
        OuvrirNouveauDossier();
        ui->CreerNomlineEdit->setText(nomPat);
        ui->CreerPrenomlineEdit->setText(prenomPat);
        ui->CreerDDNdateEdit->setDate(QDate::fromString(zdat, "yyyy-MM-dd"));
        gNNIPat     = settingPatientPar.value("Bénéficiaire/Numéro").toString();
        if (gNNIPat.left(1) == "1")  gSexePat = "M";
        else                         gSexePat = "F";

        if (gNombreDossiers == 0)       // aucun patient trouvé
            {// si rien non plus on propose la création du dossier.
            return;
            }       // A REVOIR
        else
            { // on a trouvé des patients avec la même date de naissance et on les a affichés.
            return;
            }       // A REVOIR
        }
    if (gListePatientsModel->rowCount()>0)
    {
        ui->PatientsListeTableView->selectRow(0);
        ui->PatientsListeTableView->scrollTo(gListePatientsModel->item(0)->index(),QAbstractItemView::PositionAtTop);
    }
    ListeRestreinte = true;
}
/*-----------------------------------------------------------------------------------------------------------------
    Saisie d'une facture avec Pyxvital : Retour = fichier Facture.par
-----------------------------------------------------------------------------------------------------------------*/
void Rufus::Slot_SaisieFSE()
{
    pyxi = new pyxinterf(proc, this);
    QString nomFicFacturePar = pyxi->Saisie_FSE();
    delete pyxi;
    if (nomFicFacturePar.length() ==0)
        { // pas de facture saisie ...
        return;
        }
    // Récup des infos de la facture dans le fichier ../Pyxvital/Interf/Facture.par pour alimenter la comptabilité
    QSettings settingFacturePar (nomFicFacturePar, QSettings::IniFormat);
    settingFacturePar.setIniCodec ("ISO 8859-1");

    /*-------- Exemple du contenu du fichier facture.par généré par Pyxvital ------------
      [Prestation]
      Date=27/01/15+27/01/15+27/01/15
      Quantite=1+1+1
      Code=C+CCAM+CCAM
      Coefficient=1+1+1
      Code_CCAM=NÈant+DEQP003+AAQM002
      Code_compl_CCAM=10+10+10
      ...
      [Tarification]
      Taux=100+100
      Exoneration=C+C
      Montant_honoraires=105.59+52.25
      Base_remboursement_theorique=105.59+52.25
      Base_remboursement=105.59+52.25
      Total_honoraires=157.84
      Montant_remboursable_AMO=105.59+52.25
      Total_AMO=157.84
      Total_assure=157.84
     -------------------------------------------------------------*/

    // Codes des Actes facturés
    QStringList lCode       = settingFacturePar.value("Prestation/Code").toString().split("+");
    QStringList lCode_CCAM  = settingFacturePar.value("Prestation/Code_CCAM").toString().split("+");
    QString zActesFactures  = "";
    QString zCode;
    int j = 0;
    for (int i=0; i<lCode.size(); i++)
        {zCode = lCode.at(i);
        if (zCode == "CCAM")
            zCode = lCode_CCAM.at(j++);
        if (i > 0) zActesFactures += "+";
        zActesFactures += zCode;
        }

    // Màj du ComboBox ActeCotation
    ui->ActeCotationcomboBox->lineEdit()->setText(zActesFactures);

    // total facture
    QString TotalFacture    = settingFacturePar.value("Tarification/Montant_honoraires").toString();
    QString PartAMC         = settingFacturePar.value(tr("Tarification/Total_AMC")).toString();
    QString PartAMO         = settingFacturePar.value(tr("Tarification/Total_AMO")).toString();
    QString BaseRembousement= settingFacturePar.value(tr("Tarification/Base_remboursement")).toString();
    QString TotalAssure     = settingFacturePar.value(tr("Tarification/Total_assure")).toString();
    QString Code            = settingFacturePar.value(tr("Prestation/Code")).toString();
    QString CodeCCAM        = settingFacturePar.value(tr("Prestation/Code_CCAM")).toString();

    ui->ActeMontantlineEdit->setText(QLocale().toString(TotalAssure.toDouble(),'f',2));
}
// CZ001 - fin interface Pyxvital

void Rufus::Slot_CalcIP(const QHostInfo &host)
{
    if (host.error() != QHostInfo::NoError) {
        //qDebug() << "Lookup failed:" << host.errorString();
        return;
    }
    for (int i=0; i<host.addresses().size(); i++)
    {
        quint32 adr = QHostAddress(host.addresses().at(i).toString()).toIPv4Address();
        //qDebug() << "Found address:" << "hostName() = " + host.hostName() << "localHostName() = " + QHostInfo::localHostName() << " - " << QString::number(adr) << " - " << host.addresses().at(0).toString();
        if ((host.hostName() == QHostInfo::localHostName() && QString::number(adr)>0))
        {
            gIPadr = host.addresses().at(i).toString();
            break;
        }
    }
}

