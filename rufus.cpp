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

#include "gbl_datas.h"
#include "cls_item.h"
#include "database.h"
#include "icons.h"
#include "rufus.h"
#include "ui_rufus.h"
#include "styles.h"

Rufus::Rufus(QWidget *parent) : QMainWindow(parent)
/*--------------------------------------------------------------------------------------------------------------
-- Création de la fiche ----------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------------------------------*/
{
    Datas::I();

    // la version du programme correspond à la date de publication, suivie de "/" puis d'un sous-n° - p.e. "23-6-2017/3"
    qApp->setApplicationVersion("27-11-2018/1");       // doit impérativement être composé de date version / n°version;

    ui = new Ui::Rufus;
    ui->setupUi(this);
    setWindowFlags(Qt::Window | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint | Qt::WindowMinMaxButtonsHint);

    srand(static_cast<uint>(time(Q_NULLPTR)));

    qApp->setStyleSheet(Styles::StyleAppli());

    proc = new Procedures(this);

    if (!proc->Init())                                                  // vérifie que le fichier rufus.ini est cohérent
        exit(0);                                                        // choisit le mode de connexion au serveur

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

    proc->setDirImagerie();                                                 // lit l'emplacement du dossier d'imagerie sur le serveur

    //1. Restauration de la position de la fenetre et de la police d'écran
    restoreGeometry(proc->gsettingsIni->value("PositionsFiches/Rufus").toByteArray());
    setWindowIcon(Icons::icSunglasses());


    //2 charge les data du user connecté
    int gidUser = -1;
    if( DataBase::getInstance()->getUserConnected() != nullptr )
        gidUser = DataBase::getInstance()->getUserConnected()->id();
    if (gidUser == -1)
        gidUser = proc->gsettingsIni->value("idUtilisateur/idUser").toInt();
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
    qApp->setStyleSheet(Styles::StyleAppli());
    proc->Message(gDataUser->getStatus(), 6000);

    //3 Initialisation de tout
    InitVariables();
    InitWidgets();
    InitEventFilters();
    CreerMenu();
    InitMenus();


    //4 reconstruction des combobox
    if (gDataUser->isMedecin() || gDataUser->isOrthoptist())
        ReconstruitListesActes();
    proc->initListeCorrespondants();
    ReconstruitCombosCorresp();                 // initialisation de la liste

    grequeteListe   = "SELECT IdPat, PatNom, PatPrenom, PatDDN, Sexe FROM " NOM_TABLE_PATIENTS;
    Remplir_ListePatients_TableView(grequeteListe,"","");       //InitTables()
    CalcNbDossiers();
    MetAJourUserConnectes();
    gidPatient = 0;

    UtiliseTCP = (proc->UtiliseTCP() && DataBase::getInstance()->getMode() != DataBase::Distant);
    if (UtiliseTCP)
    {
        currentmsg = "";
        erreurmsg  = "";
        TcPConnect = TcpSocket::getInstance();
        UtiliseTCP = TcPConnect->TcpConnectToServer();
        if (UtiliseTCP)
            connect(TcPConnect, &TcpSocket::tcpmessage, this, [=](QString msg) {TraiteTCPMessage(msg);});
    }

    // 5 mettre en place le TcpSocket et/ou les timer
    gTimerPatientsVus           = new QTimer(this);     // effacement automatique de la liste des patients vus - réglé à 20"
    gTimerSalDat                = new QTimer(this);     // scrutation des modifs de la salle d'attente, utilisé en cas de non utilisation des tcpsocket (pas de rufusadmin ou poste distant)
    gTimerCorrespondants        = new QTimer(this);     // scrutation des modifs de la liste des correspondants, utilisé en cas de non utilisation des tcpsocket (pas de rufusadmin ou poste distant)
    gTimerUserConnecte          = new QTimer(this);     // mise à jour de la connexion à la base de données
    gTimerVerifImportateurDocs  = new QTimer(this);     // vérifie que le poste importateur des documents externes est toujours là
    gTimerExportDocs            = new QTimer(this);     // utilisé par le poste importateur pour vérifier s'il y a des documents à sortir de la base
    gTimerActualiseDocsExternes = new QTimer(this);     // actualise l'affichage des documents externes si un dossier est ouvert
    gTimerImportDocsExternes    = new QTimer(this);     // utilisé par le poste importateur pour vérifier s'il y a des documents à importer dans la base
    gTimerVerifMessages         = new QTimer(this);     // utilisé par les postes en accès distants ou les réseaux sans RufusAdmin pour vérifier l'arrivée de nouveaux messages
    gTimerVerifVerrou           = new QTimer(this);     // utilisé par le TcpServer pour vérifier l'absence d'utilisateurs déconnectés dans la base
    gTimerSupprDocs             = new QTimer(this);     // utilisé par le poste importateur pour vérifier s'il y a des documents à supprimer

    gTimerPatientsVus   ->setSingleShot(true);          // il est singleshot et n'est démarré que quand on affiche la liste des patients vus
    gTimerPatientsVus   ->setInterval(20000);
    // Lancement des timers de gestion des documents
    gTimerVerifImportateurDocs      ->start(60000);
    // Lancement du timer de vérification des verrous - +++ à lancer après le timer gTimerVerifGestDocs puisqu'il l'utilise
    gTimerVerifVerrou               ->start(60000);// "toutes les 60 secondes"


    if (DataBase::getInstance()->getMode() == DataBase::Distant)
    {
        gTimerSalDat                ->start(10000);
        gTimerCorrespondants        ->start(60000);
        gTimerActualiseDocsExternes ->start(60000);// "toutes les 60 secondes"
        gTimerImportDocsExternes    ->start(60000);// "toutes les 60 secondes"
        gTimerVerifMessages         ->start(60000);// "toutes les 60 secondes"
    }
    else
    {
        gTimerExportDocs            ->start(10000);// "toutes les 10 secondes"
        gTimerActualiseDocsExternes ->start(10000);// "toutes les 10 secondes"
        gTimerImportDocsExternes    ->start(10000);// "toutes les 10 secondes"
        gTimerSupprDocs             ->start(60000);// "toutes les 60 secondes"
        gTimerVerifMessages         ->start(10000);// "toutes les 10 secondes"
        if (!UtiliseTCP)
        {
                gTimerSalDat->start(1000);
                gTimerCorrespondants->start(30000);
        }
    }

    gTimerUserConnecte->start(10000);// "toutes les 10 secondes - remet à jour le statut connecté du poste dans la base - tables utilisateursconnectes"

    if (!UtiliseTCP)
    {
        gflagMG     = proc->GetflagMG();
        gflagSalDat = proc->GetflagSalDat();
        connect (gTimerSalDat,              &QTimer::timeout,   this,   [=] {VerifSalleDAttente();});
        connect (gTimerCorrespondants,      &QTimer::timeout,   this,   [=] {VerifCorrespondants();});
        connect (gTimerVerifImportateurDocs,&QTimer::timeout,   this,   [=] {VerifImportateur();});
        connect (gTimerVerifVerrou,         &QTimer::timeout,   this,   [=] {VerifVerrouDossier();});
        connect (gTimerVerifMessages,       &QTimer::timeout,   this,   [=] {VerifMessages();});
        connect (gTimerImportDocsExternes,  &QTimer::timeout,   this,   &Rufus::ImportDocsExternes);
        if (DataBase::getInstance()->getMode() != DataBase::Distant)
            connect(gTimerSupprDocs,        &QTimer::timeout,   this,   [=] {SupprimerDocs();});
        VerifImportateur();
    }

    connect (gTimerUserConnecte,            &QTimer::timeout,   this,   [=] {MetAJourUserConnectes();});
    connect (gTimerActualiseDocsExternes,   &QTimer::timeout,   this,   [=] {ActualiseDocsExternes();});
    connect (gTimerPatientsVus,             &QTimer::timeout,   this,   [=] {MasquePatientsVusWidget();});


    //Nettoyage des erreurs éventuelles de la salle d'attente
    QString blabla              = ENCOURSEXAMEN;
    int length                  = blabla.size();
    QString req = "UPDATE " NOM_TABLE_SALLEDATTENTE
                    " SET Statut = '" ARRIVE "', idUserEnCoursExam = null, PosteExamen = null"
                    " WhERE idUser = " + QString::number(gidUser) +
                    " AND Left(Statut," + QString::number(length) + ") = '" ENCOURSEXAMEN "'";
    QSqlQuery (req, DataBase::getInstance()->getDataBase());

    // les slots
    Connect_Slots();

    //libération des verrous de la compta
    req = " delete from " NOM_TABLE_VERROUCOMPTAACTES " where PosePar = " + QString::number(gidUser);
    QSqlQuery lib(req, DataBase::getInstance()->getDataBase() );
    DataBase::getInstance()->traiteErreurRequete(lib,req,tr("impossible de libérer les verrous de la comptabilité"));

    if (gListePatientsModel->rowCount() == 0)
    {
        OuvrirNouveauDossier();
        ui->LListepushButton->setEnabled(false);
        UpMessageBox::Watch(this,tr("Vous n'avez aucun dossier de patient enregistré!"), tr("Vous devez d'abord en créer un."));
    }

    closeFlag = false;

    setTitre();
    Remplir_SalDat();
    VerifMessages();
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
    connect (ui->AccueilupTableWidget,                              &QTableWidget::currentCellChanged,                  this,   [=] {ActiveActeAccueil(ui->AccueilupTableWidget->currentRow());});
    connect (ui->ActePrecedentpushButton,                           &QPushButton::clicked,                              this,   [=] {NavigationConsult(-1);});
    connect (ui->ActeSuivantpushButton,                             &QPushButton::clicked,                              this,   [=] {NavigationConsult(1);});
    connect (ui->CourrierAFairecheckBox,                            &QPushButton::clicked,                              this,   [=] {CourrierAFaireChecked();});
    connect (ui->CreerActepushButton,                               &QPushButton::clicked,                              this,   [=] {CreerActe(gidPatient);});
    connect (ui->CreerActepushButton_2,                             &QPushButton::clicked,                              this,   [=] {CreerActe(gidPatient);});
    connect (ui->CreerBOpushButton,                                 &QPushButton::clicked,                              this,   [=] {CreerBilanOrtho();});
    connect (ui->CreerBOpushButton_2,                               &QPushButton::clicked,                              this,   [=] {CreerBilanOrtho();});
    connect (ui->CreerDDNdateEdit,                                  &QDateEdit::dateChanged,                            this,   [=] {if (gMode == RechercheDDN) TrouverDDN();});
    connect (ui->ChercherDepuisListepushButton,                     &QPushButton::clicked,                              this,   [=] {ChercherDepuisListe();});
    connect (ui->CreerNomlineEdit,                                  &QLineEdit::textEdited,                             this,   [=] {MajusculeCreerNom();});
    connect (ui->CreerPrenomlineEdit,                               &QLineEdit::textEdited,                             this,   [=] {MajusculeCreerPrenom();});
    connect (ui->CreerDossierpushButton,                            &QPushButton::clicked,                              this,   [=] {CreerDossierpushButtonClicked();});
    connect (ui->DernierActepushButton,                             &QPushButton::clicked,                              this,   [=] {NavigationConsult(100);});
    connect (ui->FermepushButton,                                   &QPushButton::clicked,                              this,   [=] {close();});
    connect (ui->FSEpushButton,                                     &QPushButton::clicked,                              this,   [=] {SaisieFSE();});       // CZ001
    connect (ui->IdentPatienttextEdit,                              &QWidget::customContextMenuRequested,               this,   [=] {MenuContextuelIdentPatient();});
    connect (ui->LFermepushButton,                                  &QPushButton::clicked,                              this,   [=] {close();});
    connect (ui->ListepushButton,                                   &QPushButton::clicked,                              this,   [=] {OuvrirListe();});
    connect (ui->LListepushButton,                                  &QPushButton::clicked,                              this,   [=] {OuvrirListe();});
    connect (ui->LNouvDossierpushButton,                            &QPushButton::clicked,                              this,   [=] {OuvrirNouveauDossier();;});
    connect (ui->LRecopierpushButton,                               &QPushButton::clicked,                              this,   [=] {RecopierDossier();});
    connect (ui->SendMessagepushButton,                             &QPushButton::clicked,                              this,   [=] {QMap<QString, QVariant> map;  map["null"] = true; SendMessage(map, gidPatient);});
    connect (ui->LSendMessagepushButton,                            &QPushButton::clicked,                              this,   [=] {QMap<QString, QVariant> map;  map["null"] = true; SendMessage(map);});
    connect (ui->MGupComboBox,                                      QOverload<int>::of(&QComboBox::activated),          this,   [=] {ChoixMG();});
    connect (ui->AutresCorresp1upComboBox,                          QOverload<int>::of(&QComboBox::activated),          this,   [=] {ChoixCor(ui->AutresCorresp1upComboBox);});
    connect (ui->AutresCorresp2upComboBox,                          QOverload<int>::of(&QComboBox::activated),          this,   [=] {ChoixCor(ui->AutresCorresp2upComboBox);});
    connect (ui->MGupComboBox,                                      &QWidget::customContextMenuRequested,               this,   [=] {MenuContextuelMedecin();});
    connect (ui->AutresCorresp1upComboBox,                          &QWidget::customContextMenuRequested,               this,   [=] {MenuContextuelCorrespondant(ui->AutresCorresp1upComboBox);});
    connect (ui->AutresCorresp2upComboBox,                          &QWidget::customContextMenuRequested,               this,   [=] {MenuContextuelCorrespondant(ui->AutresCorresp2upComboBox);});
    connect (ui->ModifDatepushButton,                               &QPushButton::clicked,                              this,   [=] {ui->ActeDatedateEdit->setEnabled(true); ui->ActeDatedateEdit->setFocus();});
    connect (ui->ModifIdentificationSmallButton,                    &QPushButton::clicked,                              this,   [=] {ChoixMenuContextuelIdentPatient();});
    connect (ModifTerrainupSmallButton,                             &QPushButton::clicked,                              this,   [=] {ModifierTerrain();});
    connect (ui->MotsClesLabel,                                     &QWidget::customContextMenuRequested,               this,   [=] {MenuContextuelMotsCles();});
    connect (ui->MotsClesupSmallButton,                             &QPushButton::clicked,                              this,   [=] {ChoixMenuContextuelMotsCles();});
    connect (ui->OKModifTerrainupSmallButton,                       &QPushButton::clicked,                              this,   [=] {OKModifierTerrain();});
    connect (ui->NouvDossierpushButton,                             &QPushButton::clicked,                              this,   [=] {OuvrirNouveauDossier();});
    connect (ui->OuvreActesPrecspushButton,                         &QPushButton::clicked,                              this,   [=] {OuvrirActesPrecspushButtonClicked();});
    connect (ui->OuvreDocsExternespushButton,                       &QPushButton::clicked,                              this,   [=] {OuvrirDocsExternes(gidPatient);});
    connect (ui->OuvrirDocumentpushButton,                          &QPushButton::clicked,                              this,   [=] {OuvrirDocuments();});
    connect (ui->PatientsListeTableView,                            &QWidget::customContextMenuRequested,               this,   [=] {MenuContextuelListePatients();});
    connect (ui->PatientsListeTableView,                            &QTableView::doubleClicked,                         this,   [=] {ChoixDossier(gListePatientsModel->itemFromIndex(ui->PatientsListeTableView->selectionModel()->selectedIndexes().at(0))->text().toInt());});
    connect (ui->PatientsListeTableView,                            &QTableView::entered,                               this,   [=] {AfficheToolTip(-1);});
    connect (ui->PatientsListeTableView->selectionModel(),          &QItemSelectionModel::selectionChanged,             this,   [=] {EnableCreerDossierButton();});
    connect (ui->PatientsVusFlecheupLabel,                          &UpLabel::clicked,                                  this,   [=] {AffichePatientsVusWidget();});
    connect (ui->PatientsVusupTableWidget,                          &QTableView::activated,                             this,   [=] {gTimerPatientsVus->start();});
    connect (ui->PremierActepushButton,                             &QPushButton::clicked,                              this,   [=] {NavigationConsult(0);});
    connect (ui->RefractionpushButton,                              &QPushButton::clicked,                              this,   [=] {Refraction();});
    connect (ui->SalleDAttentepushButton,                           &QPushButton::clicked,                              this,   [=] {SalleDAttente();});
    connect (ui->SalleDAttenteupTableWidget,                        &UpTableWidget::dropsignal,                         this,   [=] {DropPatient(ui->SalleDAttenteupTableWidget->dropData());});
    connect (ui->SupprimeActepushButton,                            &QPushButton::clicked,                              this,   [=] {SupprimerActe();});
    connect (ui->TonometriepushButton,                              &QPushButton::clicked,                              this,   [=] {Tonometrie();});
    connect (ui->VitaleupPushButton,                                &QPushButton::clicked,                              this,   [=] {LireLaCV();}); // CZ001

    connect (ui->ActeCotationcomboBox->lineEdit(),                  &QLineEdit::textEdited,                             this,   [=] {RetrouveMontantActe();});
    connect (ui->ActeCotationcomboBox,                              QOverload<int>::of(&QComboBox::highlighted),        this,   [=](int a)  {QToolTip::showText(cursor().pos(),ui->ActeCotationcomboBox->itemData(a).toStringList().at(2));});
    connect (ui->ActeCotationcomboBox,                              QOverload<int>::of(&QComboBox::currentIndexChanged),this,   [=] {RetrouveMontantActe();});
    connect (ui->ActeMontantlineEdit,                               &UpLineEdit::TextModified,                          this,   [=] {ActeMontantModifie();});
    connect (ui->BasculerMontantpushButton,                         &QPushButton::clicked,                              this,   [=] {BasculerMontantActe();});
    connect (ui->CCAMlinklabel,                                     &QLabel::linkActivated,                             this,   [=] {QDesktopServices::openUrl(QUrl(LIEN_CCAM));});
    connect (ui->ModifierCotationActepushButton,                    &QPushButton::clicked,                              this,   [=] {ModfiCotationActe();});
    // Les tabs --------------------------------------------------------------------------------------------------
    connect (ui->tabWidget,                                         &QTabWidget::currentChanged,                        this,   [=] {ChangeTabBureau();});

    connect (proc,                                                  &Procedures::ConnectTimers,                         this,   [=] {ConnectTimers(proc->Connexion());});

    // les signaux personnalisés ----------------------------------------------------------------------------------
    connect(this,                                                   &Rufus::EnregistrePaiement,                         this,   [=] {AppelPaiementDirect("Bouton");});

    // MAJ Salle d'attente ----------------------------------------------------------------------------------
    connect(proc,                                                   &Procedures::UpdSalDat,                             this,   [=] {
                                                                                                                                        envoieMessage(TCPMSG_MAJSalAttente);
                                                                                                                                        Remplir_SalDat();
                                                                                                                                    } );
    connect(proc,                                                   &Procedures::UpdCorrespondants,                     this,   [=] {
                                                                                                                                        envoieMessage(TCPMSG_MAJCorrespondants);
                                                                                                                                        ReconstruitCombosCorresp();
                                                                                                                                     } );

    // Nouvelle mesure d'appareil de refraction ----------------------------------------------------------------------------------
    if (proc->PortFronto()!=Q_NULLPTR || proc->PortAutoref()!=Q_NULLPTR || proc->PortRefracteur()!=Q_NULLPTR)
        connect(proc,                                               &Procedures::NouvMesureRefraction,                  this,   [=] {NouvelleMesureRefraction();});

    connect (ui->MoulinettepushButton,                              &QPushButton::clicked,                              this,   [=] {Moulinette();});
    ui->MoulinettepushButton->setVisible(false);
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
    QSqlQuery quer(req, DataBase::getInstance()->getDataBase());
    if (quer.size()>0)
    {
        Dlg_DocsExt = new dlg_docsexternes(proc,idpat, UtiliseTCP, this);
        ui->OuvreDocsExternespushButton->setEnabled(true);
        if (Dlg_DocsExt->InitOK())
        {
            Dlg_DocsExt->show();
            if (depuismenu)
                Dlg_DocsExt->setModal(true); //quand la fiche est ouverte depuis le menu contectuel de la liste des patients
        }
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
    //TODO : a revoir
    /*
    QList<dlg_actesprecedents *> ListDialog = this->findChildren<dlg_actesprecedents *>();
    for (int n = 0; n <  ListDialog.size(); n++)
        if (ListDialog.at(n)->getActeAffiche() == gidActe)
        {
            ListDialog.at(n)->ActesPrecsAfficheActe(gidActe);
            break;
        }
    */
}

/*-----------------------------------------------------------------------------------------------------------------
-- // mettre à jour dlg_docsexternes   ----------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void Rufus::MAJDocsExternes()
{
    QList<dlg_docsexternes *> ListDialogDocs = this->findChildren<dlg_docsexternes *>();
    if (ListDialogDocs.size()>0)
        proc->emit UpdDocsExternes();
    else
    {
        QString req = "Select idImpression from " NOM_TABLE_IMPRESSIONS " where idpat = " + QString::number(gidPatient);
        QSqlQuery quer(req, DataBase::getInstance()->getDataBase());
        if (quer.size()>0)
        {
            Dlg_DocsExt = new dlg_docsexternes(proc, gidPatient, UtiliseTCP, this);
            if (Dlg_DocsExt->InitOK())
                Dlg_DocsExt->show();
        }
        ui->OuvreDocsExternespushButton->setEnabled(quer.size()>0);
    }
}


/*-----------------------------------------------------------------------------------------------------------------
-- La moulinette ne sert qu'à tester des fonctions et n'est pas utilisée par le programe        ------------------
-----------------------------------------------------------------------------------------------------------------*/
void Rufus::Moulinette()
{
    envoieMessage("testtesttest");
    return;
    /*QString req, str;
    QTextEdit txt;
    req = "select idimpression, Titre from " NOM_TABLE_IMPRESSIONS " where soustypedoc = null and typedoc = 'DocRecu'";
    //qDebug() << req;
    QSqlQuery idquer (req, DataBase::getInstance()->getDataBase() );
    idquer.first();
    for (int i=0; i<idquer.size(); i++)
    {
        txt.setHtml(idquer.value(1).toString());
        str = txt.toPlainText();
        req = "update " NOM_TABLE_IMPRESSIONS " set soustypedoc = '" + str + "', Formatdoc = 'Prescription', emisrecu = 1 where idimpression = " + idquer.value(0).toString();
        //qDebug() << req;
        QSqlQuery (req, DataBase::getInstance()->getDataBase() );
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
    QSqlQuery imp(req, DataBase::getInstance()->getDataBase() );
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
        QSqlQuery modif(req, DataBase::getInstance()->getDataBase() );
        DataBase::getInstance()->traiteErreurRequete(modif, req,"");
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
    QSqlQuery imp2(req, DataBase::getInstance()->getDataBase() );
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
        QSqlQuery modif(req, DataBase::getInstance()->getDataBase() );
        DataBase::getInstance()->traiteErreurRequete(modif, req,"");
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
    QSqlQuery quer(req, DataBase::getInstance()->getDataBase() );
    for (int i=0; i< quer.size(); i++)
    {
        quer.seek(i);
        if (quer.value(1).toInt()==1)
            req = "update ccam.ccamd set OPTAM = " + quer.value(2).toString().replace(",",".") + " where codeCCAM = '" + quer.value(0).toString() + "'";
        else if (quer.value(1).toInt()==2)
            req = "update ccam.ccamd set NoOPTAM = " + quer.value(2).toString().replace(",",".") + " where codeCCAM = '" + quer.value(0).toString() + "'";
        //proc->Edit(req);
        QSqlQuery(req, DataBase::getInstance()->getDataBase() );
    }
    */

    /*
    // CORRECTION DE LA BASE VILLES - ELIMINATION DES TIRETS ============================================================================================================================================================
    QString req = "select patville from " NOM_TABLE_DONNEESSOCIALESPATIENTS;
    QSqlQuery quer(req, DataBase::getInstance()->getDataBase() );
    quer.first();
    for (int i=0;i<quer.size();i++)
    {
        QString ville = quer.value(0).toString();
        if (ville.contains("-"))
        {
        req = "update " NOM_TABLE_DONNEESSOCIALESPATIENTS " set patVille = '" + proc->CorrigeApostrophe(ville.replace("-"," ")) + "' where patville = '" + proc->CorrigeApostrophe(quer.value(0).toString()) + "'";
        QSqlQuery(req, DataBase::getInstance()->getDataBase() );
        }
        quer.next();
    }
    req = "select corville from " NOM_TABLE_CORRESPONDANTS;
    QSqlQuery quer1(req, DataBase::getInstance()->getDataBase() );
    quer1.first();
    for (int i=0;i<quer1.size();i++)
    {
        QString ville = quer1.value(0).toString();
        if (ville.contains("-"))
        {
        req = "update " NOM_TABLE_CORRESPONDANTS " set corVille = '" + proc->CorrigeApostrophe(ville.replace("-"," ")) + "' where corville = '" + proc->CorrigeApostrophe(quer1.value(0).toString()) + "'";
        QSqlQuery(req, DataBase::getInstance()->getDataBase() );
        }
        quer1.next();
    }
    proc->Edit("OK pour villes");*/


    // CREATION D'UNE BASE FACTICE ============================================================================================================================================================
    //Mélange les noms, et 1ère ligne d'adresse dans la base
    if (UpMessageBox::Question(this,tr("ATTENTION"),tr("Cette fonction sert à générer une base factice pour la démonstration du logiciel") + "<br />"
                               + tr("Si vous cliquez sur OK, tous les enregistrements de la base seront mélangés et les données seront donc irrémédiablement perdues")) != UpSmallButton::STARTBUTTON)
        return;
    int idauhasard;
    QString copierequete = "drop table if exists rufus.patients2;\n";
    copierequete += "create table rufus.patients2 like rufus.patients;\n";
    copierequete += "insert into rufus.patients2 (select * from " NOM_TABLE_PATIENTS ");";
    QSqlQuery copiequery (copierequete, DataBase::getInstance()->getDataBase() );
    DataBase::getInstance()->traiteErreurRequete(copiequery,copierequete,"");

    QStringList listNoms;
    copierequete = "select idPat, patNom from rufus.patients2;";
    QSqlQuery copie(copierequete, DataBase::getInstance()->getDataBase() );
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
        QSqlQuery quernom("select patnom, patprenom from " NOM_TABLE_PATIENTS " where idPat = " + idpat,  DataBase::getInstance()->getDataBase() );
        if (quernom.size()>0)
        {
            quernom.first();
            AncNom = quernom.value(0).toString();
            //proc->Message(quernom.value(1).toString() + " " + AncNom + " - " + QString::number(k) + "/" + QString::number(s), 1);
            //qDebug() << quernom.value(1).toString() + " " + AncNom + " - " + QString::number(k) + "/" + QString::number(s);
        }
        copierequete = "update rufus.patients2 set patnom = '" + proc->CorrigeApostrophe(listNoms.at(idauhasard)) + "' where idPat = " + idpat;
        QSqlQuery modif (copierequete, DataBase::getInstance()->getDataBase() );
        DataBase::getInstance()->traiteErreurRequete(modif,copierequete,"");
        listNoms.removeAt(idauhasard);

        QSqlQuery modifactesquer("select idacte, actemotif, acteconclusion, actetexte from " NOM_TABLE_ACTES " where idpat = " + idpat,  DataBase::getInstance()->getDataBase() );
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
                QSqlQuery (req1,  DataBase::getInstance()->getDataBase() );
            }
        }
        QSqlQuery modifimprquer("select idimpression, textentete, textcorps, textorigine from " NOM_TABLE_IMPRESSIONS " where idpat = " + idpat,  DataBase::getInstance()->getDataBase() );
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
                QSqlQuery (req1,  DataBase::getInstance()->getDataBase() );
            }
        }
    }
    copierequete = "delete from rufus.patients;\n";
    copierequete += "insert into rufus.patients (select * from rufus.patients2);\n";
    copierequete += "drop table if exists rufus.patients2;\n";
    QSqlQuery (copierequete, DataBase::getInstance()->getDataBase() );

    UpMessageBox::Watch(this,"OK pour nom");

        copierequete = "drop table if exists rufus.donneessocialespatients2;\n";
    copierequete += "create table rufus.donneessocialespatients2 like rufus.donneessocialespatients;\n";
    copierequete += "insert into rufus.donneessocialespatients2 (select * from rufus.donneessocialespatients);";
    QSqlQuery copieAquery (copierequete, DataBase::getInstance()->getDataBase() );
    DataBase::getInstance()->traiteErreurRequete(copieAquery,copierequete,"");

    QStringList listAdresses;
    copierequete = "select idPat, patAdresse1 from rufus.donneessocialespatients2 order by patAdresse1;";
    QSqlQuery copieA (copierequete, DataBase::getInstance()->getDataBase() );
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
        QSqlQuery modif3 (copierequete, DataBase::getInstance()->getDataBase() );
        DataBase::getInstance()->traiteErreurRequete(modif3,copierequete,"");
        listAdresses.removeAt(idauhasard);
    }
    listAdresses.clear();
    copierequete = "select idPat, patAdresse2 from rufus.donneessocialespatients2 order by patAdresse2;";
    QSqlQuery copieA2 (copierequete, DataBase::getInstance()->getDataBase() );
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
        QSqlQuery modif4 (copierequete, DataBase::getInstance()->getDataBase() );
        DataBase::getInstance()->traiteErreurRequete(modif4,copierequete,"");
        listAdresses.removeAt(idauhasard);
    }
    copierequete = "delete from rufus.donneessocialespatients;\n";
    copierequete += "insert into rufus.donneessocialespatients (select * from rufus.donneessocialespatients2);\n";
    copierequete += "drop table if exists rufus.donneessocialespatients2;\n";
    QSqlQuery (copierequete, DataBase::getInstance()->getDataBase() );
    UpMessageBox::Watch(this,"OK pour adresse1 et 2");
    Remplir_ListePatients_TableView(grequeteListe,"","");       // Moulinette()

    //Melange des noms des correspondants
    QSqlQuery("update " NOM_TABLE_CORRESPONDANTS " set CorNom = 'Porteix' where CorNom = 'Porte'", DataBase::getInstance()->getDataBase() );
    QSqlQuery("update " NOM_TABLE_CORRESPONDANTS " set CorNom = 'Longeix' where CorNom = 'Long'", DataBase::getInstance()->getDataBase() );
    QString Corcopierequete = "select idCor, CorNom from " NOM_TABLE_CORRESPONDANTS;
    QSqlQuery Corcopie(Corcopierequete, DataBase::getInstance()->getDataBase() );
    QStringList listnomcor;
    QSqlQuery nompatquery("select patnom from " NOM_TABLE_PATIENTS " where patnom not in (select Cornom from " NOM_TABLE_CORRESPONDANTS ") order by rand() limit " + QString::number(Corcopie.size()),  DataBase::getInstance()->getDataBase() );
    for (int e=0; e<Corcopie.size();e++)
    {
        nompatquery.seek(e);
        listnomcor <<  nompatquery.value(0).toString();
    }
    QString Corimpr = "select idimpression, textcorps, textorigine from " NOM_TABLE_IMPRESSIONS " where textcorps is not null";
    QSqlQuery Corimprquery(Corimpr,  DataBase::getInstance()->getDataBase() );
    QString CorAct = "select idacte, actemotif, actetexte from " NOM_TABLE_ACTES;
    QSqlQuery CorActquery(CorAct,  DataBase::getInstance()->getDataBase() );
    DataBase::getInstance()->traiteErreurRequete(CorActquery, CorAct,"");
    for (int k = 0; k< Corcopie.size(); k++)
    {
        QString AncNom, NouvNom, NouvAdresse, idCor;
        Corcopie.seek(k);
        idCor       = Corcopie.value(0).toString();
        AncNom      = Corcopie.value(1).toString();
        NouvNom     = listnomcor.at(k);
        Corcopierequete = "update " NOM_TABLE_CORRESPONDANTS " set Cornom = '" + proc->CorrigeApostrophe(NouvNom) + "' where idCor = " + idCor;
        QSqlQuery modifnom (Corcopierequete, DataBase::getInstance()->getDataBase() );
        DataBase::getInstance()->traiteErreurRequete(modifnom,Corcopierequete,"");


        for (int p=0; p<Corimprquery.size(); p++)
        {
            Corimprquery.seek(p);
            if (Corimprquery.value(1).toString().contains(AncNom, Qt::CaseInsensitive))
            {
                //qDebug() << AncNom + " - " + QString::number(k) + "/" + QString::number(Corcopie.size()) + " // " + QString::number(p) + "/" + QString::number(Corimprquery.size());
                QSqlQuery ("update " NOM_TABLE_IMPRESSIONS " set textcorps = '" + proc->CorrigeApostrophe(Corimprquery.value(1).toString().replace(AncNom,NouvNom))
                           + "' where idimpression = " + Corimprquery.value(0).toString(),  DataBase::getInstance()->getDataBase() );
            }
            if (Corimprquery.value(2).toString().contains(AncNom, Qt::CaseInsensitive))
            {
                QSqlQuery ("update " NOM_TABLE_IMPRESSIONS " set textorigine = '" + proc->CorrigeApostrophe(Corimprquery.value(2).toString().replace(AncNom,NouvNom))
                           + "' where idimpression = " + Corimprquery.value(0).toString(),  DataBase::getInstance()->getDataBase() );
            }
        }

        for (int q=0; q<CorActquery.size(); q++)
        {
            CorActquery.seek(q);
            if (CorActquery.value(1).toString().contains(AncNom, Qt::CaseInsensitive))
            {
                //qDebug() << AncNom + " - " + QString::number(k) + "/" + QString::number(Corcopie.size()) + " // " + QString::number(q) + "/" + QString::number(CorActquery.size());
                QSqlQuery ("update " NOM_TABLE_ACTES " set actemotif = 'Courrier efffacé' where idacte = " + CorActquery.value(0).toString(),  DataBase::getInstance()->getDataBase() );
            }
            if (CorActquery.value(2).toString().contains(AncNom, Qt::CaseInsensitive))
            {
                QSqlQuery("update " NOM_TABLE_ACTES " set actetexte = 'Courrier effacé' where idacte = " + CorActquery.value(0).toString(),  DataBase::getInstance()->getDataBase() );
            }
        }

        QSqlQuery adresspatquery("select patadresse1 from " NOM_TABLE_DONNEESSOCIALESPATIENTS " order by rand() limit 1",  DataBase::getInstance()->getDataBase() );
        adresspatquery.first();
        NouvAdresse = adresspatquery.value(0).toString();
        Corcopierequete = "update rufus.correspondants set Coradresse1 = '" + proc->CorrigeApostrophe(NouvAdresse) + "' where idCor = " + idCor;
        QSqlQuery modif (Corcopierequete, DataBase::getInstance()->getDataBase() );
        DataBase::getInstance()->traiteErreurRequete(modif,Corcopierequete,"");
    }
    UpMessageBox::Watch(this,"OK pour Correspondants");
}

void Rufus::ActeMontantModifie()
{
    QString b = QLocale().toString(QLocale().toDouble(ui->ActeMontantlineEdit->text()),'f',2);
    if (b != gActeMontant)
        ValideActeMontantLineEdit(b, gActeMontant);
}

/*------------------------------------------------------------------------------------------------------------------------------------
-- Afficher le motif de l'acte ----------------------------------------------------------------
------------------------------------------------------------------------------------------------------------------------------------*/
void Rufus::AfficheMotif(UpLabel *lbl)
{
    QMap<QString, QVariant> rsgnmt = lbl->getData();
    QString Msg("");
    Msg += Item::CalculAge(rsgnmt["ddnpat"].toDate())["toString"].toString();
    if (rsgnmt["urgence"].toBool())
    {
        if (Msg != "") Msg += "\n";
        Msg += tr("Urgence");
    }
    else if (rsgnmt["motif"].toString()!= "")
    {
        if (Msg != "") Msg += "\n";
        Msg += rsgnmt["motif"].toString();
    }
    if (rsgnmt["message"].toString()!= "")
    {
        if (Msg != "") Msg += "\n";
        Msg += rsgnmt["message"].toString();
    }
    if (Msg!="")
        QToolTip::showText(cursor().pos(),Msg);
}

void Rufus::AffichePatientsVusWidget()
{
    MAJPatientsVus();
    ui->PatientsVusWidget->setVisible(true);
    ui->PatientsVusEntete2label->setVisible(false);
    ui->PatientsVusupLabel->setVisible(false);
    ui->PatientsVusFlecheupLabel->setVisible(false);
    gTimerPatientsVus->start();
}

void Rufus::MAJPatientsVus()
{
    QTableWidget        *TableAMettreAJour;
    int                 i;
    QString             NomPrenom, zw, A;
    QFontMetrics        fm(qApp->font());
    // PATIENTS VUS AUJOURD'HUI ---------------------------------------------------------------------------------------------------
    QString SalDatVusrequete;
    QSqlQuery RemplirTableViewListJourQuery ( DataBase::getInstance()->getDataBase() );

    SalDatVusrequete =   "SELECT pat.IdPat, act.idacte, PatNom, PatPrenom, UserLogin, ActeDate, ActeCotation, ActeMontant, ActeHeure, TypePaiement, Tiers, usr.idUser FROM "
                           NOM_TABLE_PATIENTS " as pat, " NOM_TABLE_ACTES " as act, " NOM_TABLE_UTILISATEURS " as usr, " NOM_TABLE_TYPEPAIEMENTACTES " as typ"
                           " WHERE usr.idUser = act.idUser and act.idPat = pat.idPat and actedate = curdate()"
                           " and act.idPat not in (select idpat from " NOM_TABLE_SALLEDATTENTE ")"
                           " and act.idActe = typ.idActe"
                           " ORDER BY ActeHeure DESC";

    TableAMettreAJour = ui->PatientsVusupTableWidget;

    RemplirTableViewListJourQuery.exec(SalDatVusrequete);
    DataBase::getInstance()->traiteErreurRequete(RemplirTableViewListJourQuery,SalDatVusrequete, tr("Impossible de remplir la salle d'attente!"));
    RemplirTableViewListJourQuery.first();
    TableAMettreAJour->clearContents();
    TableAMettreAJour->setRowCount(RemplirTableViewListJourQuery.size());

    for (i = 0; i < RemplirTableViewListJourQuery.size(); i++)
    {
        QMap<QString, QVariant> rsgnmt;
        rsgnmt["idpat"] = RemplirTableViewListJourQuery.value(0).toInt();
        rsgnmt["idsuperviseur"] = RemplirTableViewListJourQuery.value(11).toInt();
        rsgnmt["loginsuperviseur"] = RemplirTableViewListJourQuery.value(4).toString();

        UpLabel *label0, *label1, *label2, *label3, *label4;
        label0 = new UpLabel;
        label1 = new UpLabel;
        label2 = new UpLabel;
        label3 = new UpLabel;
        label4 = new UpLabel;

        label0->setContextMenuPolicy(Qt::CustomContextMenu);
        label1->setContextMenuPolicy(Qt::CustomContextMenu);
        label2->setContextMenuPolicy(Qt::CustomContextMenu);
        label3->setContextMenuPolicy(Qt::CustomContextMenu);
        label4->setContextMenuPolicy(Qt::CustomContextMenu);

        label0->setData(rsgnmt);
        label1->setData(rsgnmt);
        label2->setData(rsgnmt);
        label3->setData(rsgnmt);
        label4->setData(rsgnmt);

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

        connect (label0,        &QWidget::customContextMenuRequested,       this,   [=] {gTimerPatientsVus->start(); MenuContextuelSalDat(label0);});
        connect (label1,        &QWidget::customContextMenuRequested,       this,   [=] {gTimerPatientsVus->start(); MenuContextuelSalDat(label1);});
        connect (label2,        &QWidget::customContextMenuRequested,       this,   [=] {gTimerPatientsVus->start(); MenuContextuelSalDat(label2);});
        connect (label3,        &QWidget::customContextMenuRequested,       this,   [=] {gTimerPatientsVus->start(); MenuContextuelSalDat(label3);});
        connect (label4,        &QWidget::customContextMenuRequested,       this,   [=] {gTimerPatientsVus->start(); MenuContextuelSalDat(label4);});
        connect (label0,        &UpLabel::enter,            this,                   [=] {gTimerPatientsVus->start(); AfficheToolTip(rsgnmt["idpat"].toInt());});
        connect (label1,        &UpLabel::enter,            this,                   [=] {gTimerPatientsVus->start(); AfficheToolTip(rsgnmt["idpat"].toInt());});
        connect (label2,        &UpLabel::enter,            this,                   [=] {gTimerPatientsVus->start(); AfficheToolTip(rsgnmt["idpat"].toInt());});
        connect (label3,        &UpLabel::enter,            this,                   [=] {gTimerPatientsVus->start(); AfficheToolTip(rsgnmt["idpat"].toInt());});
        connect (label4,        &UpLabel::enter,            this,                   [=] {gTimerPatientsVus->start(); AfficheToolTip(rsgnmt["idpat"].toInt());});
        connect (label0,        &UpLabel::dblclick,         this,                   [=] {if (gDataUser->isSoignant()) ChoixDossier(rsgnmt["idpat"].toInt());});
        connect (label1,        &UpLabel::dblclick,         this,                   [=] {if (gDataUser->isSoignant()) ChoixDossier(rsgnmt["idpat"].toInt());});
        connect (label2,        &UpLabel::dblclick,         this,                   [=] {if (gDataUser->isSoignant()) ChoixDossier(rsgnmt["idpat"].toInt());});
        connect (label3,        &UpLabel::dblclick,         this,                   [=] {if (gDataUser->isSoignant()) ChoixDossier(rsgnmt["idpat"].toInt());});
        connect (label4,        &UpLabel::dblclick,         this,                   [=] {if (gDataUser->isSoignant()) ChoixDossier(rsgnmt["idpat"].toInt());});

        TableAMettreAJour->setCellWidget(i,0,label0);
        TableAMettreAJour->setCellWidget(i,1,label1);
        TableAMettreAJour->setCellWidget(i,2,label2);
        TableAMettreAJour->setCellWidget(i,3,label3);
        TableAMettreAJour->setCellWidget(i,4,label4);
        TableAMettreAJour->setRowHeight(i,int(fm.height()*1.1));

        RemplirTableViewListJourQuery.next();
    }

}

/*------------------------------------------------------------------------------------------------------------------------------------
    -- Afficher l'adresse du patient en toolTip ----------------------------------------------------------------
    ------------------------------------------------------------------------------------------------------------------------------------*/
void Rufus::AfficheToolTip(int id)
{
    if (id==-1)
    {
        QModelIndex pindx = ui->PatientsListeTableView->indexAt(ui->PatientsListeTableView->viewport()->mapFromGlobal(cursor().pos()));
        if (gListePatientsModel->itemFromIndex(pindx) == Q_NULLPTR)
            return;
        int row = gListePatientsModel->itemFromIndex(pindx)->row();
        id = gListePatientsModel->item(row)->text().toInt();
    }
    QString req = "SELECT PatAdresse1, PatAdresse2, PatAdresse3, PatVille, PatDDN FROM " NOM_TABLE_DONNEESSOCIALESPATIENTS " dos, " NOM_TABLE_PATIENTS " pat"
                  "  WHERE pat.idPat = " + QString::number(id) + " and pat.idpat = dos.idpat";
    QSqlQuery quer (req, DataBase::getInstance()->getDataBase() );
    DataBase::getInstance()->traiteErreurRequete(quer,req,"");
    QString Msg = "";
    if (quer.first())
    {
        if (quer.value(4).toString() != "")
            Msg += Item::CalculAge(quer.value(4).toDate())["toString"].toString();
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
        if (Msg!="") Msg += "\n";
        Msg += QString::number(id);
    }

    if (Msg != "")
        QToolTip::showText(cursor().pos(),Msg);
}


/*-----------------------------------------------------------------------------------------------------------------
    -- Gère l'affichage des menus -------------------------------------------------------------------------------------
    -----------------------------------------------------------------------------------------------------------------*/
void Rufus::AfficheMenu(QMenu *menu)
{
    actionRechercheParID->setVisible(gMode != NouveauDossier);
    bool a = ui->tabWidget->indexOf(ui->tabDossier) < 0;
        actionBilanRecettes->setEnabled(a);
        actionRemiseCheques->setEnabled(a);
        actionRecettesSpeciales->setEnabled(a);
        actionImpayes->setEnabled(a);
        actionParametres->setEnabled(a);
        actionRechercheCourrier->setVisible(gDataUser->isSoignant() && a);
        if (gDataUser->isSoignant())
        {
            bool c;
            QString req = "select idActe from " NOM_TABLE_ACTES " where ActeCourrierafaire = 'T' and idUser = " + QString::number(gDataUser->id());
            c = (QSqlQuery(req, DataBase::getInstance()->getDataBase()).size()>0);
            actionRechercheCourrier     ->setEnabled(a && c);
        }

    bool b = (ui->tabWidget->currentWidget() == ui->tabDossier);
        actionSupprimerActe->setVisible(b);
        actionCreerActe->setVisible(b);
        actionEmettreDocument->setVisible(b);
        actionDossierPatient->setVisible(b);
        menuEmettre->setEnabled(b);

    if (menu == menuDossier)
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
                   QSqlQuery quer("select idacte from " NOM_TABLE_ACTES " where idpat = " + QString::number(idpat), DataBase::getInstance()->getDataBase() );
                   actionSupprimerDossier->setVisible(quer.size() == 0);
               }
                else
                   actionSupprimerDossier->setVisible(false);
            }
            else
            {
                actionSupprimerDossier->setVisible(gDataUser->getFonction() == tr("Médecin") || gDataUser->getFonction() == tr("Orthoptiste"));
                actionSupprimerDossier->setEnabled(false);
            }
        }
        else if (ui->tabWidget->currentWidget() == ui->tabDossier) {
            QSqlQuery quer("select idacte from " NOM_TABLE_ACTES " where idpat = " + QString::number(gidPatient), DataBase::getInstance()->getDataBase() );
            actionSupprimerDossier->setVisible(quer.size() == 0 || gDataUser->getFonction() == tr("Médecin") || gDataUser->getFonction() == tr("Orthoptiste"));
        }
    }
    if (menu == menuDocuments)
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

void Rufus::AppelPaiementDirect(QString Origin)
{
    QList<int> ListidActeAPasser;
    int Mode = 1;

    if (Origin == "AttentePaiement") // l'appel est fait par un clic dans le menu contextuel de la salle d'attente des paiements en attente
    {
        QList<QTableWidgetSelectionRange> ListItems = ui->AccueilupTableWidget->selectedRanges();
        if (ListItems.size() == 0) return;
        for (int i = 0; i < ListItems.size(); i++) //FIXME : appel SQL dans 1 boucle de boucle
        {
            int debut = ListItems.at(i).topRow();
            int hauteur = ListItems.at(i).rowCount();
            for (int k=0;k<hauteur;k++)
            {
                // On vérifie que chaque acte sélectionné n'est pas déjà en cours d'enregistrement sur un autre poste
                QString req = "SELECT idActe FROM "  NOM_TABLE_VERROUCOMPTAACTES
                              " WHERE idActe = "  + ui->AccueilupTableWidget->item(debut+k,5)->text();
                QSqlQuery ChercheVerrouQuery (req, DataBase::getInstance()->getDataBase() );
                DataBase::getInstance()->traiteErreurRequete(ChercheVerrouQuery, req,"");
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
    else if (Origin == "Bouton") // l'appel est fait par un clic sur le bouton enregistrepaiement
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
        QSqlQuery ChercheVerrouQuery (req, DataBase::getInstance()->getDataBase() );
        DataBase::getInstance()->traiteErreurRequete(ChercheVerrouQuery, req,"");
        if (ChercheVerrouQuery.size() > 0)
        {
            ChercheVerrouQuery.first();
            UpMessageBox::Watch(this,tr("Vous ne pouvez paz enregistrer le paiement de cet acte !"),
                                tr("Il est déjà en cours d'enregistrement par ") + ChercheVerrouQuery.value(0).toString());
            return;
        }
        // il s'agit d'un acte gratuit - on propose de le classer
        if (QLocale().toDouble(ui->ActeMontantlineEdit->text()) == 0.0 && ui->ActeCotationcomboBox->currentText() != "")
        {
            UpMessageBox msgbox;
            UpSmallButton OKBouton(tr("Consultation gratuite"));
            msgbox.setText(tr("Vous avez entré un montant nul !"));
            msgbox.setInformativeText(tr("Enregistrer cette consultation comme gratuite?"));
            UpSmallButton NoBouton(tr("Annuler"));
            msgbox.addButton(&NoBouton, UpSmallButton::CANCELBUTTON);
            msgbox.addButton(&OKBouton, UpSmallButton::STARTBUTTON);
            msgbox.exec();
            if (msgbox.clickedButton() != &OKBouton)
                ui->ActeMontantlineEdit->setFocus();
            else
            {
                QString enreggratuit = "INSERT INTO " NOM_TABLE_TYPEPAIEMENTACTES " (idActe, TypePaiement) VALUES (" + QString::number(gidActe) + ",'G')";
                QSqlQuery InsertGratuitQuery (enreggratuit, DataBase::getInstance()->getDataBase() );
                DataBase::getInstance()->traiteErreurRequete(InsertGratuitQuery,enreggratuit,tr("Impossible d'enregister cet acte comme gratuit"));
                AfficheActeCompta();
            }
            return;
        }

        QString ActeSal = QString::number(gidActe);
        QString Msg;
        QString requete =   "SELECT idPat FROM " NOM_TABLE_SALLEDATTENTE
                    " WHERE idPat = " + QString::number(gidPatient);
        QSqlQuery SalDatQuery(requete, DataBase::getInstance()->getDataBase() );
        DataBase::getInstance()->traiteErreurRequete(SalDatQuery,requete,tr("Impossible de trouver le dossier dans la salle d'attente!"));

        if (SalDatQuery.size() == 0)
        {

            requete = "INSERT INTO " NOM_TABLE_SALLEDATTENTE
                    " (idPat, idUser, Statut, HeureStatut, idUserEnCoursExam, idActeAPayer, PosteExamen)"
                    " VALUES (" + QString::number(gidPatient) + "," + QString::number(gDataUser->getIdUserActeSuperviseur()) + ",'" RETOURACCUEIL "',"
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
        FlagMetAjourSalDat();
        QSqlQuery ModifSalDatQuery(requete, DataBase::getInstance()->getDataBase() );
        DataBase::getInstance()->traiteErreurRequete(ModifSalDatQuery,requete,Msg);
        ListidActeAPasser << gidActe;
    }
    else// l'appel est fait par le menu
        ListidActeAPasser << 0;

    Dlg_PaimtDirect = new dlg_paiement(ListidActeAPasser, Mode, proc, 0, 0);//NOTE : New Paiement
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
        QSqlQuery SalDatQuery(req, DataBase::getInstance()->getDataBase() );
        DataBase::getInstance()->traiteErreurRequete(SalDatQuery,req, tr("Impossible de trouver le dossier dans la salle d'attente!"));
        if (SalDatQuery.size() == 0)
        {
            req =   "INSERT INTO " NOM_TABLE_SALLEDATTENTE
                    " (idPat, idUser, Statut, HeureStatut, idUserEnCoursExam, PosteExamen)"
                    " VALUES ('" + QString::number(gidPatient) + "','" + QString::number(gDataUser->getIdUserActeSuperviseur()) + "','" ENCOURSEXAMEN + gDataUser->getLogin() + "','" + QTime::currentTime().toString("hh:mm")
                    + "'," + QString::number(gDataUser->id()) + ",'" + QHostInfo::localHostName().left(60) + "')";
            Msg = tr("Impossible de modifier le statut du dossier");
        }
        else
        {
            req =   "UPDATE " NOM_TABLE_SALLEDATTENTE
                    " SET Statut = '" ENCOURSEXAMEN + gDataUser->getLogin() +
                    "', HeureStatut = '" + QTime::currentTime().toString("hh:mm") +
                    "', idUserEnCoursExam = " + QString::number(gDataUser->id()) +
                    ", PosteExamen = '" + QHostInfo::localHostName().left(60) +
                    "' WHERE idPat = " + QString::number(gidPatient);
            Msg = tr("Impossible de modifier le statut du dossier");
        }
        QSqlQuery ModifSalDatQuery (req, DataBase::getInstance()->getDataBase() );
        DataBase::getInstance()->traiteErreurRequete(ModifSalDatQuery,req,Msg);
        FlagMetAjourSalDat();
    }
    AfficheActeCompta();
    delete Dlg_PaimtDirect;
}

void Rufus::AppelPaiementDirect2(QString Origin)
{
    QList<int> ListidActeAPasser;
    if (Origin == "AttentePaiement") // l'appel est fait par un clic dans le menu contextuel de la salle d'attente des paiements en attente
    {
        QList<QTableWidgetSelectionRange> ListItems = ui->AccueilupTableWidget->selectedRanges();
        if (ListItems.size() == 0) return;
        for (int i = 0; i < ListItems.size(); i++) //FIXME : appel SQL dans 1 boucle de boucle
        {
            int debut = ListItems.at(i).topRow();
            int hauteur = ListItems.at(i).rowCount();
            for (int k=0;k<hauteur;k++)
            {
                // On vérifie que chaque acte sélectionné n'est pas déjà en cours d'enregistrement sur un autre poste
                QString req = "SELECT idActe FROM "  NOM_TABLE_VERROUCOMPTAACTES
                              " WHERE idActe = "  + ui->AccueilupTableWidget->item(debut+k,5)->text();
                QSqlQuery ChercheVerrouQuery (req, DataBase::getInstance()->getDataBase() );
                DataBase::getInstance()->traiteErreurRequete(ChercheVerrouQuery, req,"");
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
    else if (Origin == "Bouton") // l'appel est fait par un clic sur le bouton enregistrepaiement
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
        QSqlQuery ChercheVerrouQuery (req, DataBase::getInstance()->getDataBase() );
        DataBase::getInstance()->traiteErreurRequete(ChercheVerrouQuery, req,"");
        if (ChercheVerrouQuery.size() > 0)
        {
            ChercheVerrouQuery.first();
            UpMessageBox::Watch(this,tr("Vous ne pouvez paz enregistrer le paiement de cet acte !"),
                                tr("Il est déjà en cours d'enregistrement par ") + ChercheVerrouQuery.value(0).toString());
            return;
        }
        // il s'agit d'un acte gratuit - on propose de le classer
        if (QLocale().toDouble(ui->ActeMontantlineEdit->text()) == 0.0 && ui->ActeCotationcomboBox->currentText() != "")
        {
            UpMessageBox msgbox;
            UpSmallButton OKBouton(tr("Consultation gratuite"));
            msgbox.setText(tr("Vous avez entré un montant nul !"));
            msgbox.setInformativeText(tr("Enregistrer cette consultation comme gratuite?"));
            UpSmallButton NoBouton(tr("Annuler"));
            msgbox.addButton(&NoBouton, UpSmallButton::CANCELBUTTON);
            msgbox.addButton(&OKBouton, UpSmallButton::STARTBUTTON);
            msgbox.exec();
            if (msgbox.clickedButton() != &OKBouton)
                ui->ActeMontantlineEdit->setFocus();
            else
            {
                QString enreggratuit = "INSERT INTO " NOM_TABLE_TYPEPAIEMENTACTES " (idActe, TypePaiement) VALUES (" + QString::number(gidActe) + ",'G')";
                QSqlQuery InsertGratuitQuery (enreggratuit, DataBase::getInstance()->getDataBase() );
                DataBase::getInstance()->traiteErreurRequete(InsertGratuitQuery,enreggratuit,tr("Impossible d'enregister cet acte comme gratuit"));
                AfficheActeCompta();
            }
            return;
        }

        QString ActeSal = QString::number(gidActe);
        QString Msg;
        QString requete =   "SELECT idPat FROM " NOM_TABLE_SALLEDATTENTE
                    " WHERE idPat = " + QString::number(gidPatient);
        QSqlQuery SalDatQuery(requete, DataBase::getInstance()->getDataBase() );
        DataBase::getInstance()->traiteErreurRequete(SalDatQuery,requete,tr("Impossible de trouver le dossier dans la salle d'attente!"));

        if (SalDatQuery.size() == 0)
        {

            requete = "INSERT INTO " NOM_TABLE_SALLEDATTENTE
                    " (idPat, idUser, Statut, HeureStatut, idUserEnCoursExam, idActeAPayer, PosteExamen)"
                    " VALUES (" + QString::number(gidPatient) + "," + QString::number(gDataUser->getIdUserActeSuperviseur()) + ",'" RETOURACCUEIL "',"
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
        FlagMetAjourSalDat();
        QSqlQuery ModifSalDatQuery(requete, DataBase::getInstance()->getDataBase() );
        DataBase::getInstance()->traiteErreurRequete(ModifSalDatQuery,requete,Msg);
        ListidActeAPasser << gidActe;
    }
    else
        ListidActeAPasser << 0;

    Dlg_PmtDirect = new dlg_paiementdirect(ListidActeAPasser, proc, 0, this);//NOTE : New Paiement
 //   if(Dlg_PmtDirect->getInitOK())
    {
        Dlg_PmtDirect->setWindowTitle(tr("Gestion des paiements directs"));
        Dlg_PmtDirect->exec();
    }
    if (Origin == "Bouton")  // on redonne le statut en cours d'examen au dossier
    {
        QString Msg;
        QString req =   "SELECT idPat FROM " NOM_TABLE_SALLEDATTENTE
                " WHERE idPat = " + QString::number(gidPatient);
        QSqlQuery SalDatQuery(req, DataBase::getInstance()->getDataBase() );
        DataBase::getInstance()->traiteErreurRequete(SalDatQuery,req, tr("Impossible de trouver le dossier dans la salle d'attente!"));
        if (SalDatQuery.size() == 0)
        {
            req =   "INSERT INTO " NOM_TABLE_SALLEDATTENTE
                    " (idPat, idUser, Statut, HeureStatut, idUserEnCoursExam, PosteExamen)"
                    " VALUES ('" + QString::number(gidPatient) + "','" + QString::number(gDataUser->getIdUserActeSuperviseur()) + "','" ENCOURSEXAMEN + gDataUser->getLogin() + "','" + QTime::currentTime().toString("hh:mm")
                    + "'," + QString::number(gDataUser->id()) + ",'" + QHostInfo::localHostName().left(60) + "')";
            Msg = tr("Impossible de modifier le statut du dossier");
        }
        else
        {
            req =   "UPDATE " NOM_TABLE_SALLEDATTENTE
                    " SET Statut = '" ENCOURSEXAMEN + gDataUser->getLogin() +
                    "', HeureStatut = '" + QTime::currentTime().toString("hh:mm") +
                    "', idUserEnCoursExam = " + QString::number(gDataUser->id()) +
                    ", PosteExamen = '" + QHostInfo::localHostName().left(60) +
                    "' WHERE idPat = " + QString::number(gidPatient);
            Msg = tr("Impossible de modifier le statut du dossier");
        }
        QSqlQuery ModifSalDatQuery (req, DataBase::getInstance()->getDataBase() );
        DataBase::getInstance()->traiteErreurRequete(ModifSalDatQuery,req,Msg);
        FlagMetAjourSalDat();
    }
}

void Rufus::AppelPaiementTiers()
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
    Dlg_PaimtTiers = new dlg_paiement(ListidActeAPasser, Mode, proc, 0, 0, this); //NOTE : New Paiement
    if(Dlg_PaimtTiers->getInitOK())
    {
        Dlg_PaimtTiers->setWindowTitle(tr("Gestion des tiers payants"));
        Dlg_PaimtTiers->show();
    }
}

void Rufus::AutreDossier(int idPat)
{
    if(gDataUser->isSoignant())
    {
        gdossierAOuvrir = idPat;
        ChoixMenuContextuelListePatients(tr("Autre Dossier"));
    }
}

void Rufus::BasculerMontantActe()
{
    int idx = ui->ActeCotationcomboBox->findText(ui->ActeCotationcomboBox->currentText());
    if (idx>-1)
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

void Rufus::BilanRecettes()
{   
    Dlg_Rec            = new dlg_recettes(proc);
    if (!Dlg_Rec->getInitOK())
    {
        delete Dlg_Rec;
        return;
    }
    Dlg_Rec->exec();
    Dlg_Rec->close(); // nécessaire pour enregistrer la géométrie
    delete Dlg_Rec;
}

void Rufus::ChangeTabBureau()
{
    if(ui->tabWidget->currentWidget() == ui->tabList)
    {
        setFixedWidth(LARGEURLISTE);
        ui->CreerNomlineEdit->setFocus();
        CalcNbDossiers();
    }
    else
        setFixedWidth(LARGEURNORMALE);
}

void Rufus::ChoixMG()
{
    QString req = "select idpat from " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS " where idpat = " + QString::number(gidPatient);
    QSqlQuery quer(req, DataBase::getInstance()->getDataBase() );
    if (quer.size() == 0)
        req =   "INSERT INTO " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS
                " (idPat, idCorMedMG) VALUES (" + QString::number(gidPatient) + "," + ui->MGupComboBox->currentData().toString() + ")";
    else
        req = "update " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS " set idcormedmg = " + ui->MGupComboBox->currentData().toString() + " where idpat = " + QString::number(gidPatient);
    //qDebug() << req;
    QSqlQuery (req, DataBase::getInstance()->getDataBase() );
    OKModifierTerrain();
    ui->MGupComboBox->setImmediateToolTip(CalcToolTipCorrespondant(ui->MGupComboBox->currentData().toInt()));
}

/*-----------------------------------------------------------------------------------------------------------------
-- Rechercher le nom dans la listePatients par l'id ---------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void    Rufus::ChercheNomparID(QString id)  // ce mode de recherche se fait sans filtrage de la liste qui est vue dans son intégralité
{
    QSqlQuery quer("SELECT PatNom, PatPrenom FROM "  NOM_TABLE_PATIENTS " WHERE idPat = " + id,  DataBase::getInstance()->getDataBase() );
    if (quer.size()>0)
    {
        quer.first();
        ui->CreerNomlineEdit->setText(quer.value(0).toString());
        ui->CreerPrenomlineEdit->clear();
        lblnom->setText(quer.value(0).toString().toUpper() + " " + quer.value(1).toString());
        ChercheNomFiltre(id.toInt());
    }
}

void Rufus::ChoixCor(UpComboBox *box)
{
    QString idcor;
    if (box==ui->AutresCorresp1upComboBox)
        idcor = "idcormedspe1";
    else if (box==ui->AutresCorresp2upComboBox)
        idcor = "idcormedspe2";
    QString req = "select idpat from " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS " where idpat = " + QString::number(gidPatient);
    QSqlQuery quer(req, DataBase::getInstance()->getDataBase() );
    if (quer.size() == 0)
        req =   "INSERT INTO " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS
                " (idPat, " + idcor + ") VALUES (" + QString::number(gidPatient) + "," + box->currentData().toString() + ")";
    else
        req = "update " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS " set " + idcor + " = " + box->currentData().toString() + " where idpat = " + QString::number(gidPatient);
    QSqlQuery (req, DataBase::getInstance()->getDataBase() );
    OKModifierTerrain();
    box->setImmediateToolTip(CalcToolTipCorrespondant(box->currentData().toInt()));
}
/* -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    tous les timers sont déconnectés pendant les procédures de sauvegarde de la base ---------------------------------------------------------------------------------------------------------------------------
 -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void Rufus::ConnectTimers(bool a)
{
    if (a)
    {
        if (DataBase::getInstance()->getMode() == DataBase::Distant)
        {
            gTimerSalDat                ->start(10000);
            gTimerCorrespondants        ->start(60000);
            gTimerActualiseDocsExternes ->start(60000);
            gTimerImportDocsExternes    ->start(60000);
            gTimerVerifMessages         ->start(60000);
        }
        else
        {
            if (!UtiliseTCP)
            {
                gTimerSalDat            ->start(1000);
                gTimerCorrespondants    ->start(30000);
            }
            gTimerVerifImportateurDocs  ->start(60000);
            gTimerExportDocs            ->start(10000);
            gTimerActualiseDocsExternes ->start(10000);
            gTimerImportDocsExternes    ->start(10000);
            gTimerVerifMessages         ->start(10000);
            gTimerSupprDocs             ->start(60000);
        }
        gTimerUserConnecte  ->start(10000);
        gTimerVerifVerrou   ->start(60000);

        connect (gTimerUserConnecte,                &QTimer::timeout,   this,   [=] {MetAJourUserConnectes();});
        connect (gTimerActualiseDocsExternes,       &QTimer::timeout,   this,   [=] {ActualiseDocsExternes();});
        if (!UtiliseTCP)
        {
            connect (gTimerSalDat,                  &QTimer::timeout,   this,   [=] {VerifSalleDAttente();});
            connect (gTimerCorrespondants,          &QTimer::timeout,   this,   [=] {VerifCorrespondants();});
            connect (gTimerVerifVerrou,             &QTimer::timeout,   this,   [=] {VerifVerrouDossier();});
            connect (gTimerVerifMessages,           &QTimer::timeout,   this,   [=] {VerifMessages();});
            connect (gTimerVerifImportateurDocs,    &QTimer::timeout,   this,   [=] {VerifImportateur();});
            connect (gTimerImportDocsExternes,      &QTimer::timeout,   this,   &Rufus::ImportDocsExternes);
            if (DataBase::getInstance()->getMode() != DataBase::Distant)
                connect(gTimerSupprDocs,                &QTimer::timeout,   this,   [=] {SupprimerDocs();});
        }
    }
    else
    {
        gTimerVerifImportateurDocs  ->disconnect();
        gTimerSalDat                ->disconnect();
        gTimerCorrespondants        ->disconnect();
        gTimerExportDocs            ->disconnect();
        gTimerActualiseDocsExternes ->disconnect();
        gTimerImportDocsExternes    ->disconnect();
        gTimerVerifMessages         ->disconnect();
        gTimerUserConnecte          ->disconnect();
        gTimerVerifVerrou           ->disconnect();
        gTimerSupprDocs             ->disconnect();
        gTimerVerifImportateurDocs  ->stop();
        gTimerSalDat                ->stop();
        gTimerCorrespondants        ->stop();
        gTimerExportDocs            ->stop();
        gTimerActualiseDocsExternes ->stop();
        gTimerImportDocsExternes    ->stop();
        gTimerVerifMessages         ->stop();
        gTimerUserConnecte          ->stop();
        gTimerVerifVerrou           ->stop();
        gTimerSupprDocs             ->stop();
    }
}

void Rufus::CourrierAFaireChecked()
{
    QString cr = (ui->CourrierAFairecheckBox->isChecked()? "'T'" :"NULL");
    QString requete =   "UPDATE " NOM_TABLE_ACTES " SET ActeCourrieraFaire = " + cr + " WHERE idActe = " + QString::number(gidActe);
    QSqlQuery UpdateCourrierAFaireQuery (requete, DataBase::getInstance()->getDataBase() );
    DataBase::getInstance()->traiteErreurRequete(UpdateCourrierAFaireQuery,requete,tr("Impossible d'écrire dans la table des Actes"));
    MAJActesPrecs();
}

/*------------------------------------------------------------------------------------------------------------------
-- création - gestion des bilans orthoptiques ----------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void Rufus::CreerBilanOrtho()
{
    bool    nouveauBO       = true;
    bool    ConversionEnBO  = false;
    if (ui->Acteframe->isVisible())
    {
        QString chborequete = "select idbilanortho, ActeDate from " NOM_TABLE_BILANORTHO ", " NOM_TABLE_ACTES
                " where idActe = idbilanortho and idbilanortho = " + ui->idActelineEdit->text();
        //UpMessageBox::Watch(this,chborequete);
        QSqlQuery chboquery(chborequete, DataBase::getInstance()->getDataBase());
        QDate DateBl;
        nouveauBO = (chboquery.size()<1);
        if (chboquery.size() > 0)
        {
            chboquery.first();
            DateBl = chboquery.value(1).toDate();
            if (DateBl != QDate::currentDate())
            {
                UpMessageBox msgbox;
                UpSmallButton OKBouton(tr("Reprendre"));
                UpSmallButton NoBouton(tr("Créer un nouveau\nbilan orthoptique"));
                UpSmallButton AnnulBouton(tr("Annuler"));
                msgbox.setText("Euuhh... " + gDataUser->getLogin());
                msgbox.setInformativeText(tr("Voulez-vous reprendre le bilan affiché\nou créer un nouveau bilan à la date d'aujourd'hui?"));
                msgbox.setIcon(UpMessageBox::Quest);
                msgbox.addButton(&AnnulBouton, UpSmallButton::CANCELBUTTON);
                msgbox.addButton(&NoBouton, UpSmallButton::COPYBUTTON);
                msgbox.addButton(&OKBouton, UpSmallButton::STARTBUTTON);
                msgbox.exec();
                if (msgbox.clickedButton() != &OKBouton && msgbox.clickedButton() != &NoBouton)
                    return;
                nouveauBO = (msgbox.clickedButton() == &NoBouton);
                msgbox.close();
            }
        }
        else
        {
            if (ui->ActeDatedateEdit->date() == QDate::currentDate())
            {
                UpMessageBox msgbox;
                UpSmallButton NoBouton(tr("Annuler"));
                UpSmallButton OKBouton(tr("Convertir en\nbilan orthoptique"));
                msgbox.setText("Euuhh... " + gDataUser->getLogin());
                msgbox.setInformativeText(tr("Voulez-vous transformer l'acte en cours en bilan orthoptique?\nToutes les données saisies seront perdues!"));
                msgbox.setIcon(UpMessageBox::Warning);
                msgbox.addButton(&NoBouton, UpSmallButton::CANCELBUTTON);
                msgbox.addButton(&OKBouton, UpSmallButton::STARTBUTTON);
                msgbox.exec();
                nouveauBO = (msgbox.clickedButton() == &OKBouton);
                if (nouveauBO)
                {
                    ui->ActeMotiftextEdit->clear();
                    ui->ActeTextetextEdit->clear();
                    ui->ActeConclusiontextEdit->clear();
                    ConversionEnBO = true;
                }
                msgbox.close();
                if (!nouveauBO) return;
            }

        }
        if (!nouveauBO)
        {
            Dlg_BlOrtho             = new dlg_bilanortho(proc, ui->idActelineEdit->text().toInt(), chboquery.value(0).toInt(), gidPatient);
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
        Dlg_BlOrtho             = new dlg_bilanortho(proc, ui->idActelineEdit->text().toInt(), 0, gidPatient);
        UiDLg_BlOrtho           = Dlg_BlOrtho->ui;
        QString Titre           = tr("Bilan orthoptique - ") + gPrenomPatient + " " + gNomPatient;
        Dlg_BlOrtho             ->setWindowTitle(Titre);
        QString RefractionOD    = "";
        QString RefractionOG    = "";
        Dlg_BlOrtho             ->setDateBO(QDate::currentDate());

        QString RefODrequete    = "select max(idrefraction), formuleOD from " NOM_TABLE_REFRACTION " where quelleMesure = 'R' and quelleDistance = '2' and idPat = " + QString::number(gidPatient);
        QSqlQuery RefODquery(RefODrequete, DataBase::getInstance()->getDataBase());
        RefODquery.last();
        if (RefODquery.value(1).toString() != "")  // On ne peut pas utiliser if (query.size() > 0) dans ce cas car l'opérateur max retourne toujours un size() = 1, même quand il n'y a pas de réponse...
            RefractionOD += RefODquery.value(1).toString();
        else
        {
            QString RefODVLrequete = "select max(idrefraction), formuleOD from " NOM_TABLE_REFRACTION " where quelleMesure = 'R' and quelleDistance = 'L' and idPat = " + QString::number(gidPatient);
            QSqlQuery RefODVLquery(RefODVLrequete, DataBase::getInstance()->getDataBase());
            RefODVLquery.last();
            if (RefODVLquery.value(1).toString() != "")
                RefractionOD += RefODVLquery.value(1).toString();
        }

        QString RefOGrequete    = "select max(idrefraction), formuleOG from " NOM_TABLE_REFRACTION " where quelleMesure = 'R' and quelleDistance = '2' and idPat = " + QString::number(gidPatient);
        QSqlQuery RefOGquery(RefOGrequete, DataBase::getInstance()->getDataBase());
        RefOGquery.last();
        if (RefOGquery.value(1).toString() != "")
            RefractionOG += RefOGquery.value(1).toString();
        else
        {
            QString RefOGVLrequete = "select max(idrefraction), formuleOG from " NOM_TABLE_REFRACTION " where quelleMesure = 'R' and quelleDistance = 'L' and idPat = " + QString::number(gidPatient);
            QSqlQuery RefOGVLquery(RefOGVLrequete, DataBase::getInstance()->getDataBase());
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
        QSqlQuery UpdateUpMotifTextEditQuery (updaterequete, DataBase::getInstance()->getDataBase() );
        DataBase::getInstance()->traiteErreurRequete(UpdateUpMotifTextEditQuery,updaterequete,tr("Impossible de mettre à jour le champ Motif !"));

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
        QSqlQuery UpdateUpTextEditQuery (updaterequete, DataBase::getInstance()->getDataBase() );
        DataBase::getInstance()->traiteErreurRequete(UpdateUpTextEditQuery,updaterequete, tr("Impossible de mettre à jour le champ Texe !"));

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
            QSqlQuery UpdateUpTextEditQuery (updaterequete, DataBase::getInstance()->getDataBase() );
            DataBase::getInstance()->traiteErreurRequete(UpdateUpTextEditQuery,updaterequete, tr("Impossible de mettre à jour le champ Texte !"));
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

        if (!DataBase::getInstance()->locktables(QStringList(NOM_TABLE_BILANORTHO)))
           return;
        QSqlQuery deleteblorthoquery(deleteblorthorequete, DataBase::getInstance()->getDataBase() );
        if (DataBase::getInstance()->traiteErreurRequete(deleteblorthoquery,deleteblorthorequete,"")){
            DataBase::getInstance()->rollback(); return;}
        QSqlQuery blorthoquery(bilanorthorequete, DataBase::getInstance()->getDataBase() );
        if (DataBase::getInstance()->traiteErreurRequete(blorthoquery,bilanorthorequete,"")){
            DataBase::getInstance()->rollback(); return;}
        DataBase::getInstance()->commit();

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

void Rufus::CreerDossierpushButtonClicked()
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

void Rufus::EnableCreerDossierButton()
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
    if (!Dlg_DocsScan->getinitOK())
        return;
    Dlg_DocsScan->setWindowTitle(tr("Enregistrer un document issu du scanner pour ") + nomprenompat);
    Dlg_DocsScan->show();
    Dlg_DocsScan->NavigueVers("Fin"); /*on fait comme ça parce que si on utilise NavigueVers
                                        dans le constructeur de dlg_docsscanner,
                                        la première image a une résolution très dégradée*/
    Dlg_DocsScan->disconnect();
    connect(Dlg_DocsScan, &dlg_docsscanner::accepted, this, &Rufus::MAJDocsExternes);
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
    Dlg_DocsVideo->NavigueVers("Fin");
    Dlg_DocsVideo->disconnect();
    connect(Dlg_DocsVideo, &dlg_docsvideo::accepted, this, &Rufus::MAJDocsExternes);
}

void Rufus::FiltreSalleDAttente()
{
    int index       = gSalDatTab->currentIndex();
    int iduser      = gSalDatTab->tabData(index).toInt();
    QString usrlog  = gSalDatTab->tabText(index);
    if (iduser==-1)
        for(int i=0; i<ui->SalleDAttenteupTableWidget->rowCount(); i++)
            ui->SalleDAttenteupTableWidget->setRowHidden(i,false);
    else
        for(int i=0; i<ui->SalleDAttenteupTableWidget->rowCount(); i++)
        {
            UpLabel *lbl = dynamic_cast<UpLabel*>(ui->SalleDAttenteupTableWidget->cellWidget(i,6));
            ui->SalleDAttenteupTableWidget->setRowHidden(i,lbl->text() != usrlog);
        }
}

void Rufus::ActiveActeAccueil(int row)
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

void Rufus::FiltreAccueil(int idx)
{
    int idparent        = gAccueilTab->tabData(idx).toInt();
    for(int i=0; i<ui->AccueilupTableWidget->rowCount(); i++)
    {
        UpLabel *lbl = dynamic_cast<UpLabel*>(ui->AccueilupTableWidget->cellWidget(i,6));
        ui->AccueilupTableWidget->setRowHidden(i,lbl->text() != QString::number(idparent));
    }
}

void Rufus::GestionComptes()
{
    Dlg_Cmpt = new dlg_comptes(proc, this);
    if(Dlg_Cmpt->getInitOK())
        Dlg_Cmpt->exec();
}

void Rufus::ExporteDocs()
{
    if (!isPosteImport())
        return;
    if (PasDExportPourLeMoment)
        return;
    QString NomDirStockageImagerie;
    if (DataBase::getInstance()->getMode() == DataBase::Poste)
    {
        QSqlQuery dirquer("select dirimagerie from " NOM_TABLE_PARAMSYSTEME,  DataBase::getInstance()->getDataBase() );
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
    QSqlQuery totquer(totreq, DataBase::getInstance()->getDataBase() );
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
            gTimerExportDocs->disconnect();
            PasDExportPourLeMoment = true;
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
    QSqlQuery exportjpgquer (req, DataBase::getInstance()->getDataBase() );
    for (int i=0; i<exportjpgquer.size(); i++)
    {
        exportjpgquer.seek(i);
        if (exportjpgquer.value(5).toString() != "")
        {
            QString CheminFichier = NomDirStockageImagerie + NOMDIR_IMAGES + exportjpgquer.value(5).toString();
            if (QFile(CheminFichier).exists())
            {
                QSqlQuery ("update " NOM_TABLE_IMPRESSIONS " set jpg = null where idimpression = " + exportjpgquer.value(0).toString(), DataBase::getInstance()->getDataBase() );
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
            QSqlQuery ("delete from  " NOM_TABLE_IMPRESSIONS " where idimpression = " + exportjpgquer.value(0).toString(), DataBase::getInstance()->getDataBase() );
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
            sz = static_cast<int>(CC.size());
            if (sz > TAILLEMAXIIMAGES)
                tauxcompress -= 10;
            CC.close();
        }
        if (!OK)
        {
            QSqlQuery ("delete from  " NOM_TABLE_IMPRESSIONS " where idimpression = " + exportjpgquer.value(0).toString(), DataBase::getInstance()->getDataBase() );
            continue;
        }
        CC.open(QIODevice::ReadWrite);
        CC.setPermissions(QFileDevice::ReadOther
                          | QFileDevice::ReadGroup
                          | QFileDevice::ReadOwner  | QFileDevice::WriteOwner
                          | QFileDevice::ReadUser   | QFileDevice::WriteUser);
        CC.close();
        QSqlQuery ("update " NOM_TABLE_IMPRESSIONS " set jpg = null, lienversfichier = '/" + datetransfer + "/" + NomFileDoc + "' where idimpression = " + exportjpgquer.value(0).toString(), DataBase::getInstance()->getDataBase() );
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
    QSqlQuery exportpdfquer (reqpdf, DataBase::getInstance()->getDataBase() );
    for (int i=0; i<exportpdfquer.size(); i++)
    {
        exportpdfquer.seek(i);
        if (exportpdfquer.value(5).toString() != "")
        {
            QString CheminFichier = NomDirStockageImagerie + NOMDIR_IMAGES + exportpdfquer.value(5).toString();
            if (QFile(CheminFichier).exists())
            {
                QSqlQuery ("update " NOM_TABLE_IMPRESSIONS " set pdf = null where idimpression = " + exportpdfquer.value(0).toString(), DataBase::getInstance()->getDataBase() );
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
        if (!document || document->isLocked() || document == Q_NULLPTR)
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
            QSqlQuery (delreq, DataBase::getInstance()->getDataBase() );
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
        QSqlQuery ("update " NOM_TABLE_IMPRESSIONS " set pdf = null, compression = null, lienversfichier = '/" + datetransfer + "/" + NomFileDoc  + "' where idimpression = " + exportpdfquer.value(0).toString(), DataBase::getInstance()->getDataBase() );
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
    listmsg <<  tr("export terminé") << QString::number(total) + tr(" documents exportés en ")  + duree;
    dlg_message(listmsg, 3000);
}

void Rufus::ImportDocsExternes()
{
    if (ImportDocsExtThread != Q_NULLPTR)
        if (PosteImport)
        {
            QString req = "select distinct list.TitreExamen, list.NomAPPareil from " NOM_TABLE_APPAREILSCONNECTESCENTRE " appcon, " NOM_TABLE_LISTEAPPAREILS " list"
                  " where list.idappareil = appcon.idappareil and idLieu = " + QString::number( gDataUser->getSite()->id() );
            //qDebug()<< req;
            QSqlQuery docsquer(req,  DataBase::getInstance()->getDataBase());
            if (docsquer.size()>0)
                ImportDocsExtThread->RapatrieDocumentsThread(docsquer);
        }
}

void Rufus::ImprimeDossier()
{
    QString listactreq = "select idActe, ActeDate, ActeMotif, ActeTexte, ActeConclusion, usernom, userprenom, usertitre from " NOM_TABLE_ACTES
                         " as act left outer join " NOM_TABLE_UTILISATEURS " as usr on usr.iduser = act.iduser"
                         " where idPat = " + QString::number(gidPatient);
    //UpMessageBox::Watch(this,listactreq);
    QSqlQuery listactquer (listactreq, DataBase::getInstance()->getDataBase() );
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
    QSqlQuery atcdtsquer (req,  DataBase::getInstance()->getDataBase() );
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
        QSqlQuery MGQuery (req, DataBase::getInstance()->getDataBase() );
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
   User *userEntete = proc->setDataOtherUser(gDataUser->getIdUserParent());
   if (!userEntete)
   {
       UpMessageBox::Watch(this, tr("Impossible de retrouver les données de l'en-tête"), tr("Annulation de l'impression"));
       return;
   }

   Entete = proc->ImpressionEntete(QDate::currentDate(), userEntete).value("Norm");
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
       QSqlQuery query = QSqlQuery( DataBase::getInstance()->getDataBase() );
       query.prepare("insert into " NOM_TABLE_IMPRESSIONS " (idUser, idpat, TypeDoc, SousTypeDoc, Titre, TextEntete,"
                                                          " TextCorps,"
                                                          " TextPied, Dateimpression, UserEmetteur, EmisRecu, FormatDoc, idLieu)"
                                                          " values(:iduser, :idpat, :typeDoc, :soustypedoc, :titre,"
                                                          " :textEntete,"
                                                          " :textCorps,"
                                                          " :textPied, :dateimpression, :useremetteur, :emisrecu, :formatdoc, :idlieu)");
       query.bindValue(":iduser", QString::number(gDataUser->id()));
       query.bindValue(":idpat", QString::number(gidPatient));
       query.bindValue(":typeDoc", COURRIER);
       query.bindValue(":soustypedoc", "Impression dossier");
       query.bindValue(":titre", "Impression dossier");
       query.bindValue(":textEntete",Entete);
       query.bindValue(":textCorps", Corps);
       query.bindValue(":textPied", Pied);
       query.bindValue(":dateimpression", QDate::currentDate().toString("yyyy-MM-dd") + " " + QTime::currentTime().toString("HH:mm:ss"));
       query.bindValue(":useremetteur", QString::number(gDataUser->id()));
       query.bindValue(":emisrecu", "0");
       query.bindValue(":formatdoc", COURRIER);
       query.bindValue(":idlieu", QString::number(gDataUser->getSite()->id()) );
       if(!query.exec())
           UpMessageBox::Watch(this,tr("Impossible d'enregistrer ce document dans la base!"));
       ui->OuvreDocsExternespushButton->setEnabled(true);
   }
   MAJDocsExternes();
   delete Etat_textEdit;
}

void Rufus::DropPatient(QByteArray data)
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
    InscritEnSalDat(id);
}

bool Rufus::InscritEnSalDat(int idpat)
{
    QString choixreq = " select idPat from " NOM_TABLE_SALLEDATTENTE " where idpat = " + QString::number(idpat);
    QSqlQuery choixquery(choixreq, DataBase::getInstance()->getDataBase() );
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
        DataBase::getInstance()->traiteErreurRequete(QSqlQuery (Arriverequete, DataBase::getInstance()->getDataBase()),Arriverequete,"");
        FlagMetAjourSalDat();
        RecaleTableView(idpat);
    }
    return true;
}


void Rufus::ListeCorrespondants()
{
    if (Datas::I()->correspondants->getCorrespondants()->size()==0)
    {
        UpMessageBox::Watch(this, tr("pas de correspondant enregistré") );
        bool onlydoctors    = false;
        Dlg_IdentCorresp    = new dlg_identificationcorresp("Creation", onlydoctors, 0, proc);
        if (Dlg_IdentCorresp->exec()>0)
        {
            ReconstruitCombosCorresp();         // par une modif introduite par la fiche identcorrespondant
            FlagMetAjourMG();
        }
        delete Dlg_IdentCorresp;
        return;
    }
    Dlg_ListCor = new dlg_listecorrespondants(proc, this);

    Dlg_ListCor->exec();
    if (Dlg_ListCor->getListeModifiee())
    {
        ReconstruitCombosCorresp();             // par une modif introduite par la fiche listecorrespondant
        FlagMetAjourMG();
    }
    delete Dlg_ListCor;
}

void Rufus::MajusculeCreerNom()
{
//    QString nom;
//    QLineEdit *UiNom;
//    UiNom = ui->CreerNomlineEdit;
//    nom = proc->MajusculePremiereLettre(UiNom->text(),false);
//    UiNom->setText(nom);
    ui->CreerNomlineEdit->setText(proc->MajusculePremiereLettre(ui->CreerNomlineEdit->text()));
    if (gMode == NouveauDossier) ChercheNomFiltre();
    ChercheNomFiltre();
}

void Rufus::MajusculeCreerPrenom()
{
    QString nom;
    QLineEdit *UiNom;
    UiNom = ui->CreerPrenomlineEdit;
    nom =  proc->MajusculePremiereLettre(UiNom->text(),false);
    UiNom->setText(nom);
    if (gMode == NouveauDossier) ChercheNomFiltre();
    ChercheNomFiltre();
}

void Rufus::MenuContextuelIdentPatient()
{
    gmenuContextuel = new QMenu(this);
    QAction *pAction_IdentPatient = gmenuContextuel->addAction("Modifier les données patients") ;
    connect (pAction_IdentPatient, &QAction::triggered, [=] {ChoixMenuContextuelIdentPatient();});

    // ouvrir le menu
    gmenuContextuel->exec(cursor().pos());
    delete gmenuContextuel;
}

void Rufus::ChoixMenuContextuelIdentPatient()
{
    IdentificationPatient("Modification",gidPatient);  // aussi appelé depuis le bouton ui->ModifIdentificationSmallButton
}

void Rufus::MenuContextuelMotsCles()
{
    gmenuContextuel = new QMenu(this);

    QAction *pAction_ModifMotCle = gmenuContextuel->addAction(tr("Modifier les mots clés"));
    connect (pAction_ModifMotCle, &QAction::triggered, [=] {ChoixMenuContextuelMotsCles();});

    // ouvrir le menu
    gmenuContextuel->exec(cursor().pos());
    delete gmenuContextuel;
}

void Rufus::ChoixMenuContextuelMotsCles()
{
    dlg_listemotscles *ListMCDialog = new dlg_listemotscles(proc, gidPatient);
    if (ListMCDialog->exec()==0)
    {
        QStringList listMC = ListMCDialog->listMCDepart();
        if (listMC.size()>0)
        {
            QSqlQuery ("delete from " NOM_TABLE_MOTSCLESJOINTURES " where idpat = " + QString::number(gidPatient),  DataBase::getInstance()->getDataBase() );
            QString req = "insert into " NOM_TABLE_MOTSCLESJOINTURES " (idpat, idmotcle) values ";
            req += "(" + QString::number(gidPatient) + ", " + listMC.at(0) + ")";
            for (int j=1; j<listMC.size(); j++)
                req += ", (" + QString::number(gidPatient) + ", " + listMC.at(j) + ")";
            QSqlQuery (req, DataBase::getInstance()->getDataBase() );
            QSqlQuery ("delete from " NOM_TABLE_MOTSCLESJOINTURES " where idMotCle not in (select idmotcle from " NOM_TABLE_MOTSCLES ")",  DataBase::getInstance()->getDataBase() );
        }
    }
    CalcMotsCles(gidPatient);
    ListMCDialog->close(); // nécessaire pour enregistrer la géométrie
    delete ListMCDialog;
}

void Rufus::RechercheParID()
{
    gAskRechParIDDialog     = new UpDialog();
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
    connect(idLine,                         &QLineEdit::textEdited,         [=] {ChercheNomparID(idLine->text());});
    connect(gAskRechParIDDialog->OKButton,  &QPushButton::clicked,          [=] {gAskRechParIDDialog->close();});
    gAskRechParIDDialog->exec();
}

void Rufus::RechercheParMotCle()
{
    QString req = "select idmotcle, motcle from " NOM_TABLE_MOTSCLES " order by motcle";
    QSqlQuery quer(req, DataBase::getInstance()->getDataBase() );
    if (quer.size()==0)
    {
        UpMessageBox::Watch(this, tr("Aucun mot clé défini dans la base"), tr("Recherche impossible"));
        return;
    }

    gAskRechParMotCleDialog     = new UpDialog();
    QVBoxLayout     *globallay  = dynamic_cast<QVBoxLayout*>(gAskRechParMotCleDialog->layout());
    QTableView      *tabMC      = new QTableView(gAskRechParMotCleDialog);
    globallay               ->insertWidget(0,tabMC);
    gAskRechParMotCleDialog ->AjouteLayButtons();
    connect(gAskRechParMotCleDialog->OKButton,  &QPushButton::clicked,  [=] {AfficheDossiersRechercheParMotCle();});

    gAskRechParMotCleDialog ->setModal(true);
    gAskRechParMotCleDialog ->setWindowTitle(tr("Recherche de patients par mots clés"));

    tabMC                   ->verticalHeader()->setVisible(false);
    tabMC                   ->horizontalHeader()->setVisible(false);
    tabMC                   ->setFocusPolicy(Qt::StrongFocus);
    tabMC                   ->setSelectionMode(QAbstractItemView::SingleSelection);
    tabMC                   ->setGridStyle(Qt::NoPen);
    tabMC                   ->setEditTriggers(QAbstractItemView::NoEditTriggers);
    QFontMetrics fm(qApp->font());
    int hauteurligne        = int(fm.height()*1.1);
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

void Rufus::AfficheDossiersRechercheParMotCle()
{
    QStringList listidMc;
    QStandardItemModel *model = dynamic_cast<QStandardItemModel*>(gAskRechParMotCleDialog->findChildren<QTableView *>().at(0)->model());
    if (model==Q_NULLPTR)
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
    QSqlQuery quer(req, DataBase::getInstance()->getDataBase() );
    if (quer.size()==0)
    {
        UpMessageBox::Watch(this, tr("Aucun patient retrouvé pour ces critères"));
        return;
    }
    gAskListPatients = new UpDialog();
    QVBoxLayout     *globallay          = dynamic_cast<QVBoxLayout*>(gAskListPatients->layout());
    QTableView      *tabMC              = new QTableView(gAskListPatients);
    globallay       ->insertWidget(0,tabMC);
    gAskListPatients->AjouteLayButtons(UpDialog::ButtonPrint | UpDialog::ButtonOK);
    gAskListPatients->PrintButton   ->setLuggage(listidMc);
    connect(gAskListPatients->OKButton,     &QPushButton::clicked,   [=] {gAskListPatients->accept();});
    connect(gAskListPatients->PrintButton,  &QPushButton::clicked,   [=] {ImprimeListPatients(gAskListPatients->PrintButton->Luggage());});

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
    int hauteurligne = int(fm.height()*1.1);
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

void Rufus::AfficheCourriersAFaire()
{
    QString req = "select idActe, act.idPat, PatNom, PatPrenom, ActeDate"
                  " from " NOM_TABLE_ACTES " as act"
                  " left outer join " NOM_TABLE_PATIENTS " pat"
                  " on act.idPat = pat.IDPAT"
                  " where ActeCourrierafaire = 'T' and act.iduser = " + QString::number(gDataUser->id()) + " order by patnom, patprenom";
    QSqlQuery quer(req, DataBase::getInstance()->getDataBase() );
    if (quer.size()==0)
    {
        UpMessageBox::Watch(this, tr("Pas de courrier en attente"));
        return;
    }
    gAskListPatients = new UpDialog();
    QVBoxLayout     *globallay          = dynamic_cast<QVBoxLayout*>(gAskListPatients->layout());
    QTableView      *tabCourriers              = new QTableView();
    globallay       ->insertWidget(0,tabCourriers);
    gAskListPatients->AjouteLayButtons(UpDialog::ButtonClose);
    connect(gAskListPatients->CloseButton,     &QPushButton::clicked,   [=] {gAskListPatients->accept();});

    UpLabel *lbl = new UpLabel();
    lbl->setText(tr("Double clic ou clic droit\nsur un dossier pour l'ouvrir"));
    lbl->setAlignment(Qt::AlignCenter);
    gAskListPatients->AjouteWidgetLayButtons(lbl, false);

    gAskListPatients->setModal(true);
    gAskListPatients->setSizeGripEnabled(false);
    gAskListPatients->setWindowTitle(tr("Liste des courriers en attene"));

    tabCourriers->verticalHeader()->setVisible(false);
    tabCourriers->horizontalHeader()->setVisible(false);
    tabCourriers->setFocusPolicy(Qt::StrongFocus);
    tabCourriers->setSelectionMode(QAbstractItemView::SingleSelection);
    tabCourriers->setSelectionBehavior(QAbstractItemView::SelectRows);
    tabCourriers->setGridStyle(Qt::NoPen);
    tabCourriers->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tabCourriers->setContextMenuPolicy(Qt::CustomContextMenu);
    QFontMetrics fm(qApp->font());
    int hauteurligne = int(fm.height()*1.1);
    tabCourriers->setMinimumHeight(hauteurligne*20);
    tabCourriers->setSizeIncrement(0,hauteurligne);
    tabCourriers->setMouseTracking(true);

    QStandardItem       *pitem, *pitem1, *pitem2;
    QStandardItemModel *modele =  new QStandardItemModel;
    quer.first();
    for (int i=0; i<quer.size(); i++)
    {
        pitem   = new QStandardItem(quer.value(2).toString().toUpper() + " " + quer.value(3).toString());       // Nom Prenom
        pitem1  = new QStandardItem(quer.value(4).toDate().toString(tr("dd-MMM-yyyy")));                        // Date de l'acte
        pitem2  = new QStandardItem(quer.value(1).toString());                                                  // idPat
        pitem   ->setAccessibleDescription(quer.value(0).toString());                                           // idActe
        pitem1  ->setAccessibleDescription(quer.value(0).toString());                                           // idActe
        QList <QStandardItem*> listitems;
        listitems << pitem << pitem1 << pitem2;
        modele  ->appendRow(listitems);
        quer    .next();
    }
    tabCourriers->setModel(modele);
    for (int i=0; i<modele->rowCount(); i++)
        tabCourriers->setRowHeight(i,hauteurligne);
    tabCourriers->setColumnWidth(0,200);
    tabCourriers->setColumnWidth(1,100);
    tabCourriers->setColumnWidth(2,0);
    tabCourriers->setFixedWidth(tabCourriers->columnWidth(0)+tabCourriers->columnWidth(1)+tabCourriers->columnWidth(2)+2);
    gAskListPatients->setFixedWidth(tabCourriers->width()
                        + globallay->contentsMargins().left()*2);
    connect(tabCourriers, &QWidget::customContextMenuRequested, [=]
    {
        QPoint tbpos    = tabCourriers->mapFromGlobal(cursor().pos());
        QModelIndex idx = tabCourriers->indexAt(tbpos);
        QString patient = modele->item(modele->itemFromIndex(idx)->row(),0)->text();
        gmenuContextuel = new QMenu(this);
        QAction *pAction_OuvrirDossier = gmenuContextuel->addAction("Ouvrir le dossier " + patient) ;
        connect (pAction_OuvrirDossier, &QAction::triggered, [=]
        {
            int idacte      = modele->itemFromIndex(idx)->accessibleDescription().toInt();
            int idPat       = modele->item(modele->itemFromIndex(idx)->row(),2)->text().toInt();
            ChoixDossier(idPat, idacte);
            gAskListPatients->close();
        });
        gmenuContextuel->exec(cursor().pos());
        delete gmenuContextuel;
    });

    connect(tabCourriers, &QAbstractItemView::doubleClicked, gAskListPatients, [=]
    {
        QModelIndexList mdlist = tabCourriers->selectionModel()->selectedIndexes();
        if(mdlist.size()>0)
        {
            QModelIndex idx = mdlist.at(0);
            int idacte      = modele->itemFromIndex(idx)->accessibleDescription().toInt();
            int idPat       = modele->item(modele->itemFromIndex(idx)->row(),2)->text().toInt();
            ChoixDossier(idPat, idacte);
            gAskListPatients->close();
        }
    });
    gAskListPatients->exec();
    delete gAskListPatients;
}

void Rufus::ImprimeListPatients(QVariant var)
{
    QStringList listidMc = var.toStringList();
    if (listidMc.size()==0)
        return;
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
    QSqlQuery titrquer(req1, DataBase::getInstance()->getDataBase() );
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
    QSqlQuery quer(req, DataBase::getInstance()->getDataBase() );
    int     gtotalNbreDossiers    = quer.size();
    QDate   date = QDate::currentDate();;

    //création de l'entête
    QString EnTete;
    User *userEntete = proc->setDataOtherUser(gDataUser->getIdUserParent());
    if (userEntete == nullptr)
        return;
    EnTete = proc->ImpressionEntete(date, userEntete).value("Norm");
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

void Rufus::MasquePatientsVusWidget()
{
    ui->PatientsVusWidget->setVisible(false);
    ui->PatientsVusEntete2label->setVisible(true);
    ui->PatientsVusupLabel->setVisible(true);
    ui->PatientsVusFlecheupLabel->setVisible(true);
}

void Rufus::MenuContextuelBureaux(UpTextEdit *UpText)
{
    if (UpText->getId() == 0)
        return;
    gdossierAOuvrir = UpText->getId();
    if( gDataUser->isSoignant() )
    {
        gmenuContextuel = new QMenu(this);
        {
            QAction *pAction_ReprendreDossier = gmenuContextuel->addAction(tr("Visualiser le dossier"));
            connect (pAction_ReprendreDossier, &QAction::triggered,    [=] {ChoixMenuContextuelListePatients("Autre Dossier");});
        }
        // ouvrir le menu
        gmenuContextuel->exec(cursor().pos());
        delete gmenuContextuel;
    }
    else if( gDataUser->isSecretaire() )
    {
        gmenuContextuel = new QMenu(this);
        {
            QAction *pAction_ModifierDossier = gmenuContextuel->addAction(tr("Modifier les données de ce patient"));
            connect (pAction_ModifierDossier,       &QAction::triggered,    [=] {ChoixMenuContextuelListePatients("Modifier");});
        }
        // ouvrir le menu
        gmenuContextuel->exec(cursor().pos());
        delete gmenuContextuel;
    }
}
void Rufus::MenuContextuelListePatients()
{
    QModelIndex pindx = ui->PatientsListeTableView->indexAt(ui->PatientsListeTableView->viewport()->mapFromGlobal(cursor().pos()));
    if (gListePatientsModel->itemFromIndex(pindx) == Q_NULLPTR)
        return;
    int row = gListePatientsModel->itemFromIndex(pindx)->row();
    gdossierAOuvrir = gListePatientsModel->item(row)->text().toInt();

    gmenuContextuel = new QMenu(this);

    QAction *pAction_MettreEnSalDat = gmenuContextuel->addAction(tr("Inscrire ce patient en salle d'attente"));
    connect (pAction_MettreEnSalDat,        &QAction::triggered,    [=] {ChoixMenuContextuelListePatients("SalDat");});

    QAction *pAction_ModifierDossier = gmenuContextuel->addAction(tr("Modifier les données de ce patient"));
    connect (pAction_ModifierDossier,       &QAction::triggered,    [=] {ChoixMenuContextuelListePatients("Modifier");});

    QAction *pAction_Copier = gmenuContextuel->addAction(tr("Créer un dossier de la même famille"));
    connect (pAction_Copier,                &QAction::triggered,    [=] {ChoixMenuContextuelListePatients("Copie");});

    if( gDataUser->isSoignant() )
    {
        QAction *pAction_ReprendreDossier = gmenuContextuel->addAction(tr("Visualiser le dossier"));
        connect (pAction_ReprendreDossier,  &QAction::triggered,    [=] {ChoixMenuContextuelListePatients("Autre Dossier");});
    }
    QAction *pAction_EmettreDoc = gmenuContextuel->addAction(tr("Emettre un document"));
    connect (pAction_EmettreDoc,            &QAction::triggered,    [=] {ChoixMenuContextuelListePatients("Document");});

    QString req = "Select idImpression from " NOM_TABLE_IMPRESSIONS " where idpat = " + QString::number(gdossierAOuvrir);
    QSqlQuery quer(req, DataBase::getInstance()->getDataBase() );
    if (quer.size() > 0){
        QAction *pAction_ImprimeDoc = gmenuContextuel->addAction(tr("Réimprimer un document"));
        connect (pAction_ImprimeDoc,        &QAction::triggered,    [=] {ChoixMenuContextuelListePatients("ImprimeAncienDoc");});
    }
    QAction *pAction_EnregDoc = gmenuContextuel->addAction(tr("Enregistrer un document scanné"));
    connect (pAction_EnregDoc,              &QAction::triggered,    [=] {ChoixMenuContextuelListePatients("EnregDocScan");});

    QAction *pAction_EnregVideo = gmenuContextuel->addAction(tr("Enregistrer une video"));
    connect (pAction_EnregVideo,            &QAction::triggered,    [=] {ChoixMenuContextuelListePatients("EnregVideo");});

    QAction *pAction_SendMess = gmenuContextuel->addAction(tr("Envoyer un message"));
    connect (pAction_SendMess,              &QAction::triggered,    [=] {ChoixMenuContextuelListePatients("SendMess");});

    // ouvrir le menu
    gmenuContextuel->exec(cursor().pos());
    delete gmenuContextuel;
}

void Rufus::ChoixMenuContextuelListePatients(QString choix)
{
    if (choix == "Autre Dossier")
    {
        QString autrerequete = "select PatNom, PatPrenom from " NOM_TABLE_PATIENTS " where idPat = " + QString::number(gdossierAOuvrir);
        QSqlQuery autrequery (autrerequete,  DataBase::getInstance()->getDataBase() );
        if (DataBase::getInstance()->traiteErreurRequete(autrequery,autrerequete,""))
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
            QSqlQuery autreactequery (autreactes,  DataBase::getInstance()->getDataBase() );
            if (DataBase::getInstance()->traiteErreurRequete(autreactequery,autreactes,""))
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
                Dlg_ActesPrecs  ->setWindowIcon(Icons::icLoupe());
                Dlg_ActesPrecs  ->exec();
            }
        }
    }
    else if (choix == "SalDat")
        InscritEnSalDat(gdossierAOuvrir);
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

void Rufus::MenuContextuelMedecin()
{
    if (ui->MGupComboBox->findText(ui->MGupComboBox->currentText()) || ui->MGupComboBox->currentText() != "" || ui->MGupComboBox->currentIndex() != -1)
    {
        gmenuContextuel = new QMenu(this);
        QAction *pAction_IdentPatient = gmenuContextuel->addAction(tr("Modifier les coordonnées de ce médecin"));
        connect (pAction_IdentPatient,      &QAction::triggered,    [=] {ChoixMenuContextuelMedecin();});

        // ouvrir le menu
        gmenuContextuel->exec(cursor().pos());
        delete gmenuContextuel;
    }
}

void Rufus::ChoixMenuContextuelMedecin()
{
    int id = ui->MGupComboBox->currentData().toInt();
    int idxMG = ui->MGupComboBox->currentIndex();
    Dlg_IdentCorresp          = new dlg_identificationcorresp("Modification",true,id, proc);
    if (Dlg_IdentCorresp->exec()>0)
    {
        FlagMetAjourMG();
        ReconstruitCombosCorresp();             // par une modif introduite par la fiche identcorrespondant
        if (Dlg_IdentCorresp->IdentModified())
            ui->MGupComboBox->setCurrentIndex(idxMG);
        OKModifierTerrain();
    }
    delete Dlg_IdentCorresp;
}

void Rufus::MenuContextuelCorrespondant(UpComboBox *box)
{
    if (!box->findText(box->currentText()) || box->currentText() != "" || box->currentIndex() == -1)
    {
        QString choix = "";
        if (box == ui->AutresCorresp1upComboBox) choix = "Modifier1";
        else if (box == ui->AutresCorresp2upComboBox) choix = "Modifier2";
        else return;
        if (box->findText(box->currentText()))
        {
            gmenuContextuel = new QMenu(this);
            QAction *pAction_IdentPatient = gmenuContextuel->addAction(tr("Modifier les coordonnées de ce correspondant"));
            connect (pAction_IdentPatient,      &QAction::triggered,    [=] {ChoixMenuContextuelCorrespondant(choix);});

            // ouvrir le menu
            gmenuContextuel->exec(cursor().pos());
            delete gmenuContextuel;
        }
    }
}

void Rufus::ChoixMenuContextuelCorrespondant(QString choix)
{
    int id = -1;
    if (choix == "Modifier1")
        id = ui->AutresCorresp1upComboBox->currentData().toInt();
    else if (choix == "Modifier2")
        id = ui->AutresCorresp2upComboBox->currentData().toInt();
    if (id==-1) return;
    Dlg_IdentCorresp          = new dlg_identificationcorresp("Modification", false, id, proc);
    if (Dlg_IdentCorresp->exec()>0)
    {
        int idCor = Dlg_IdentCorresp->gidCor;
        ReconstruitCombosCorresp();             // par une modif introduite par la fiche identcorrespondant
        FlagMetAjourMG();
        if (choix == "Modifier1")
            ui->AutresCorresp1upComboBox->setCurrentIndex(ui->AutresCorresp1upComboBox->findData(idCor));
        else if (choix == "Modifier2")
            ui->AutresCorresp2upComboBox->setCurrentIndex(ui->AutresCorresp2upComboBox->findData(idCor));
        OKModifierTerrain();
    }
    delete Dlg_IdentCorresp;
}

void Rufus::MenuContextuelSalDat(UpLabel *labelClicked)
{
    if (labelClicked == Q_NULLPTR) return;
    QMap<QString, QVariant> rsgnmt = labelClicked->getData();

    gdossierAOuvrir = rsgnmt["idpat"].toInt();
    int row = labelClicked->getRow();

    gmenuContextuel = new QMenu(this);

    if (ui->SalleDAttenteupTableWidget->isAncestorOf(labelClicked))
    {
        UpLabel *StatutClicked = dynamic_cast<UpLabel *> (ui->SalleDAttenteupTableWidget->cellWidget(row,1));
        if (StatutClicked != Q_NULLPTR)
        {
            if (StatutClicked->text() == ARRIVE)
            {
                QAction *pAction_RetirerDossier = gmenuContextuel->addAction(tr("Retirer ce dossier de la salle d'attente"));
                connect (pAction_RetirerDossier, &QAction::triggered,    [=] {ChoixMenuContextuelSalDat("Retirer");});
            }
        }
    }
    QAction *pAction_ModifierDossier = gmenuContextuel->addAction(tr("Modifier les données de ce patient"));
    connect (pAction_ModifierDossier,           &QAction::triggered,    [=] {ChoixMenuContextuelSalDat("Modifier");});

    if (ui->SalleDAttenteupTableWidget->isAncestorOf(labelClicked))
    {
        QAction *pAction_ModifierMotif = gmenuContextuel->addAction(tr("Modifier le motif de l'acte"));
        connect (pAction_ModifierMotif,             &QAction::triggered,    [=] {ChoixMenuContextuelSalDat("Motif");});
    }

    QAction *pAction_Copier = gmenuContextuel->addAction(tr("Créer un dossier de la même famille"));
    connect (pAction_Copier,                    &QAction::triggered,    [=] {ChoixMenuContextuelSalDat("Copie");});
    if( gDataUser->isSoignant() )
    {
        QAction *pAction_OuvrirDossier = gmenuContextuel->addAction(tr("Ouvrir le dossier"));
        connect (pAction_OuvrirDossier,         &QAction::triggered,    [=] {ChoixMenuContextuelSalDat("Ouvrir");});
    }
    QAction *pAction_EmettreDoc = gmenuContextuel->addAction(tr("Emettre un document"));
    connect (pAction_EmettreDoc,                &QAction::triggered,    [=] {ChoixMenuContextuelSalDat("Document");});

    // ouvrir le menu
    gmenuContextuel->exec(cursor().pos());
    delete gmenuContextuel;
}

void Rufus::MenuContextuelSalDatPaiemt(UpLabel *labelClicked)
{
    QList<QTableWidgetSelectionRange> listRange = ui->AccueilupTableWidget->selectedRanges();
    if (labelClicked == Q_NULLPTR) return;
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
        if( gDataUser->isSoignant() )
        {
            QAction *pAction_ReprendreDossier = gmenuContextuel->addAction(tr("Reprendre le dossier"));
            connect (pAction_ReprendreDossier,  &QAction::triggered,    [=] {ChoixMenuContextuelSalDat("Reprendre");});
        }
        QAction *pAction_EmettreDoc = gmenuContextuel->addAction(tr("Emettre un document"));
        connect (pAction_EmettreDoc,            &QAction::triggered,    [=] {ChoixMenuContextuelSalDat("Document");});
    }
    QAction *pAction_EnregistrePaiement = gmenuContextuel->addAction(tr("Enregistrer le paiement"));
    connect (pAction_EnregistrePaiement,        &QAction::triggered,    [=] {ChoixMenuContextuelSalDat("Payer");});

    // ouvrir le menu
    gmenuContextuel->exec(cursor().pos());
    delete gmenuContextuel;
}

void Rufus::ChoixMenuContextuelSalDat(QString choix)
{
    if (choix == "Reprendre")
        ChoixDossier(gdossierAOuvrir);
    else if (choix == "Payer")
        AppelPaiementDirect("AttentePaiement");
    else if (choix == "Modifier")
        IdentificationPatient("Modification",gdossierAOuvrir);  //appelé depuis le menu contextuel de la table salle d'attente

    else if (choix == "Ouvrir")
        ChoixDossier(gdossierAOuvrir);
    else if (choix == "Retirer" || choix == "Fermer")
    {
        QSqlQuery ("delete from " NOM_TABLE_SALLEDATTENTE " where idPat = " + QString::number(gdossierAOuvrir), DataBase::getInstance()->getDataBase() );
        FlagMetAjourSalDat();
    }
    else if (choix == "Copie")
        RecopierDossier(gdossierAOuvrir);
    else if (choix == "Document")
        OuvrirDocuments(false);
    else if (choix == "Motif")  // il s'agit de modifier le motif de la consultation - la patient est dans la  salle d'attente, on a son id, il suffit de le retrouver sans passer par SQL
    {
        QMap<QString, QVariant> rsgnmt;
        rsgnmt["idpat"] = -1;
        int row(-1);
        for (int i=0; i< ui->SalleDAttenteupTableWidget->rowCount(); i++)
        {
             rsgnmt = static_cast<UpLabel*>(ui->SalleDAttenteupTableWidget->cellWidget(i,0))->getData();
             if (rsgnmt["idpat"].toInt()== gdossierAOuvrir)
             {
                 row = i;
                 break;
             }
        }
        if (rsgnmt["idpat"] == -1)
            return;

        QString Message(""), Motif("");
        Message = rsgnmt["message"].toString();
        Motif = static_cast<UpLabel *>(ui->SalleDAttenteupTableWidget->cellWidget(row,4))->text();
        QTime heurerdv = QTime::fromString(static_cast<UpLabel *>(ui->SalleDAttenteupTableWidget->cellWidget(row,3))->text(), "HH:mm");

        QStringList llist = MotifMessage(Motif, Message, heurerdv);
        if (llist.isEmpty())
            return;
        QString saldatrequete =   "SELECT idPat FROM " NOM_TABLE_SALLEDATTENTE " WHERE idPat = " + QString::number(gdossierAOuvrir);
        QSqlQuery SalDatQuery(saldatrequete, DataBase::getInstance()->getDataBase() );
        DataBase::getInstance()->traiteErreurRequete(SalDatQuery,saldatrequete,"Impossible de trouver le dossier dans la salle d'attente!");
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
        QSqlQuery ModifSalDatQuery(saldatrequete, DataBase::getInstance()->getDataBase() );
        DataBase::getInstance()->traiteErreurRequete(ModifSalDatQuery,saldatrequete,"");
        FlagMetAjourSalDat();
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

    for( QMap<int, User*>::const_iterator itUser = Datas::I()->users->superviseurs()->constBegin();
         itUser != Datas::I()->users->superviseurs()->constEnd(); ++itUser )
        ComboSuperviseurs->addItem(itUser.value()->getLogin(), QString::number(itUser.value()->id()) );
    ComboSuperviseurs->setFixedWidth(100);

    QHBoxLayout *soignantlayout     = new QHBoxLayout();
    lblsoignt->setText(tr("RDV avec"));
    soignantlayout->addWidget(lblsoignt);
    soignantlayout->addWidget(ComboSuperviseurs);
    soignantlayout->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding));


    QString req = "select motif, raccourci, couleur, ParDefaut from " NOM_TABLE_MOTIFSRDV " where utiliser = 1 order by NoOrdre";
    QSqlQuery motifQuery(req, DataBase::getInstance()->getDataBase() );
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

    connect(gAsk->OKButton,   &QPushButton::clicked,  [=] {gAsk->accept();});

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


void Rufus::MetAJourUserConnectes()
{
    // On en profite au passage pour sauvegarder la position de la fenêtre principale
     //bug Qt? -> cette ligne de code ne peut pas être mise juste avant exit(0) sinon elle n'est pas éxécutée...
     proc->gsettingsIni->setValue("PositionsFiches/Rufus", saveGeometry());

     //TODO : SQL
    QString lockrequete = "LOCK TABLES " NOM_TABLE_USERSCONNECTES " WRITE;";
    QSqlQuery lockquery (lockrequete,  DataBase::getInstance()->getDataBase() );
    if (DataBase::getInstance()->traiteErreurRequete(lockquery,lockrequete,"Impossible de verrouiller " NOM_TABLE_USERSCONNECTES))
        return;
    bool MAJsaldat =false;
    QString MAJConnexionRequete;
    QSqlQuery usrquer("select iduser from " NOM_TABLE_USERSCONNECTES
                      " where NomPosteConnecte = '" + QHostInfo::localHostName().left(60) + "'"
                      " and idUser = " + QString::number(gDataUser->id()),  DataBase::getInstance()->getDataBase() );
    if (usrquer.size()>0)
        MAJConnexionRequete = "UPDATE " NOM_TABLE_USERSCONNECTES " SET HeureDerniereConnexion = NOW(), "
                              " idUser = " + QString::number(gDataUser->id()) +
                              " where NomPosteConnecte = '" + QHostInfo::localHostName().left(60) + "'"
                              " and idUser = " + QString::number(gDataUser->id()) ;
    else
    {
        MAJsaldat = true;
        MAJConnexionRequete = "insert into " NOM_TABLE_USERSCONNECTES "(HeureDerniereConnexion, idUser,UserSuperviseur,UserComptable,UserParent,NomPosteConnecte, MACAdressePosteConnecte)"
                               " VALUES(NOW()," +
                               QString::number(gDataUser->id()) + "," +
                               QString::number(gDataUser->getIdUserActeSuperviseur()) + "," +
                               QString::number(gDataUser->getIdUserComptable()) + "," +
                               QString::number(gDataUser->getIdUserParent()) +",'" +
                               QHostInfo::localHostName().left(60) + "', '" +
                               Utils::getMACAdress() + " - " + gDataUser->getLogin() + "')";
    }
    //qDebug() << MAJConnexionRequete;
    QSqlQuery MAJConnexionQuery (MAJConnexionRequete,  DataBase::getInstance()->getDataBase() );
    DataBase::getInstance()->traiteErreurRequete(MAJConnexionQuery, MAJConnexionRequete,"");
    QSqlQuery("unlock tables", DataBase::getInstance()->getDataBase() );
    if (MAJsaldat)
        FlagMetAjourSalDat();
}

void Rufus::ModfiCotationActe()
{
    gAutorModifConsult = true;
    ui->Cotationframe->setEnabled(true);
    ui->CCAMlinklabel->setVisible(true);
}

void Rufus::ModifierTerrain()
{
    TerraintreeWidget->setVisible(false);
    ModifTerrainupSmallButton->setVisible(false);
    ui->ModifTerrainframe->setVisible(true);
    ui->ModifTerrainframe->setEnabled(true);
    ui->OKModifTerrainupSmallButton->setVisible(true);
}

void Rufus::OKModifierTerrain() // recalcule le TerrainTreeWidget et l'affiche
{
    //TODO : SQL
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
    QSqlQuery DonneesMedicalesQuery (requete, DataBase::getInstance()->getDataBase() );
    if (!DataBase::getInstance()->traiteErreurRequete(DonneesMedicalesQuery,requete,tr("Impossible de retrouver les données médicales")))
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
            pItem0->setIcon(0,Icons::icStetho());
            pItem0->setTextAlignment(1,Qt::AlignLeft);
            pItem1 = new QTreeWidgetItem() ;
            pItem1->setText(1,QString::number(gidPatient));                                                             // IdPatient
            pItem1->setText(0,tr("TRAITEMENTS EN COURS"));
            pItem1->setIcon(0,Icons::icMedoc());
            pItem1->setTextAlignment(1,Qt::AlignLeft);
            pItem2 = new QTreeWidgetItem();
            pItem2->setText(1,QString::number(gidPatient));                                                             // IdPatient
            pItem2->setText(0,tr("ATCDTS OPHTALMOLOGIQUES FAMILIAUX"));
            pItem2->setIcon(0,Icons::icFamily());
            pItem2->setTextAlignment(1,Qt::AlignLeft);
            pItem3 = new QTreeWidgetItem();
            pItem3->setText(1,QString::number(gidPatient));                                                             // IdPatient
            pItem3->setIcon(0,Icons::icSmoking());
            pItem3->setTextAlignment(1,Qt::AlignLeft);
            pItem4 = new QTreeWidgetItem();
            pItem4->setText(1,QString::number(gidPatient));                                                             // IdPatient
            pItem4->setText(0,tr("AUTRES"));
            pItem4->setIcon(0,Icons::icAlcool());
            pItem4->setTextAlignment(1,Qt::AlignLeft);
            pItem5 = new QTreeWidgetItem() ;
            pItem5->setText(0,tr("MÉDECIN GÉNÉRALISTE"));
            pItem5->setText(1,QString::number(gidPatient));                                                             // IdPatient
            pItem5->setIcon(0,Icons::icDoctor());
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

void Rufus::OuvrirActesPrecspushButtonClicked()
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

void Rufus::OuvrirJournalDepenses()
{
    Dlg_Deps           = new dlg_depenses(proc);
    if(Dlg_Deps->getInitOK())
    {
        Dlg_Deps->setWindowTitle(tr("Journal des dépenses"));
        Dlg_Deps->ui->GestionComptesupPushButton->setVisible(actionGestionComptesBancaires->isVisible());
        Dlg_Deps->exec();
    }
    delete Dlg_Deps;
}

void Rufus::OuvrirParametres()
{
    //TODO : SQL
    Dlg_Param = new dlg_param(gDataUser->id(), proc);
    Dlg_Param->setWindowTitle(tr("Paramètres"));
    Dlg_Param->exec();
    if (Dlg_Param->DataUserModifiees())
    {
        ChargeDataUser();
        setWindowTitle("Rufus - " + gDataUser->getLogin() + " - " + gDataUser->getFonction());
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
                    QString mtconv  = (gDataUser->isOPTAM() ? montantOPTAM : montantNonOPTAM);
                    if (Dlg_Param->ui->ActesCCAMupTableWidget->columnCount()==6)
                    {
                        UpLineEdit *line = dynamic_cast<UpLineEdit*>(Dlg_Param->ui->ActesCCAMupTableWidget->cellWidget(i,5));
                        if (line != Q_NULLPTR)
                            montantprat = (line->text()!=""? line->text() : mtconv);
                        else
                            montantprat = mtconv;
                    }
                    QString mtprat = (gDataUser->getSecteur() >1 ? montantprat : mtconv);
                    QString montantpratique = QString::number(QLocale().toDouble(mtprat));
                    req += "('" + codeCCAM +  "', " + montantOPTAM + "," + montantNonOPTAM + "," + montantpratique + ", 1, " + QString::number(gDataUser->id()) + "),\n";
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
                    QString mtconv  = (gDataUser->isOPTAM() ? montantOPTAM : montantNonOPTAM);
                    if (Dlg_Param->ui->AssocCCAMupTableWidget->columnCount()==5)
                    {
                        UpLineEdit *line = dynamic_cast<UpLineEdit*>(Dlg_Param->ui->AssocCCAMupTableWidget->cellWidget(i,4));
                        if (line != Q_NULLPTR)
                            montantprat = (line->text()!=""? line->text() : mtconv);
                        else
                            montantprat = mtconv;
                    }
                    QString mtprat = (gDataUser->getSecteur() >1? montantprat : mtconv);
                    QString montantpratique = QString::number(QLocale().toDouble(mtprat));
                    req += "('" + codeCCAM +  "', " + montantOPTAM + "," + montantNonOPTAM + "," + montantpratique + ", 2, " + QString::number(gDataUser->id()) + "),\n";
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
                    req += "('" + codeCCAM +  "', " + montantconv + "," + montantconv + "," + montantconv + ", 3, " + QString::number(gDataUser->id()) + "), \n";
                }
        }
        req = req.left(req.lastIndexOf(")")+1);
        //proc->Edit(req);
        QString reqDel = "delete from " NOM_TABLE_COTATIONS " where idUser = " + QString::number(gDataUser->id());
        DataBase::getInstance()->traiteErreurRequete(
                    QSqlQuery(reqDel,DataBase::getInstance()->getDataBase()),
                    reqDel);
        DataBase::getInstance()->traiteErreurRequete(QSqlQuery(req, DataBase::getInstance()->getDataBase()), req);
        ReconstruitListesActes();
    }
    delete Dlg_Param;
}

void Rufus::RecettesSpeciales()
{
    Dlg_RecSpec           = new dlg_recettesspeciales(proc);
    if(Dlg_RecSpec->getInitOK())
    {
        Dlg_RecSpec->setWindowTitle(tr("Journal des recettes spéciales"));
        Dlg_RecSpec->ui->GestionComptesupPushButton->setVisible(actionGestionComptesBancaires->isVisible());
        Dlg_RecSpec->exec();
    }
    delete Dlg_RecSpec;
}

void Rufus::RetrouveMontantActe()
{
    //TODO : SQL
    QString Cotation = ui->ActeCotationcomboBox->currentText();
    // On recherche s'il y a un montant enregistré pour cette cotation
    int idx = ui->ActeCotationcomboBox->findText(Cotation);
    if (idx>-1)
    {
        QStringList listMontantActe = ui->ActeCotationcomboBox->itemData(idx).toStringList();
        QString MontantActe;
        if( gDataUser->getSecteur()>1 && !gCMUPatient)
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
        QString tarifconventionne = (gDataUser->isOPTAM() ? "OPTAM" : "NoOPTAM");
        QString req =
                "SELECT " + tarifconventionne + ", montantpratique FROM " NOM_TABLE_COTATIONS " cot, " NOM_TABLE_CCAM " cc"
                " where Typeacte = codeccam"
                " and iduser = " + QString::number(gDataUser->getIdUserParent()) +
                " and codeccam like '" + proc->CorrigeApostrophe(Cotation) + "%'";
        //qDebug() << req;
        QSqlQuery ListCotationsQuery(req, DataBase::getInstance()->getDataBase() );
        DataBase::getInstance()->traiteErreurRequete(ListCotationsQuery,req,"");
        if (ListCotationsQuery.size()>0)
        {
            ListCotationsQuery.first();
            QString MontantActe;
            if (gDataUser->getSecteur()>1 && !gCMUPatient)
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
            QSqlQuery ListCotationsQuery (req, DataBase::getInstance()->getDataBase() );
            DataBase::getInstance()->traiteErreurRequete(ListCotationsQuery,req,"");
            if (ListCotationsQuery.size()>0)
            {
                ListCotationsQuery.first();
                QString MontantActe;
                if (gDataUser->getSecteur()>1 && !gCMUPatient && !gDataUser->isOPTAM())
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
            QSqlQuery quer(req, DataBase::getInstance()->getDataBase() );
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
                    QSqlQuery (req, DataBase::getInstance()->getDataBase() );
                }
                else
                {
                    req = "insert into " NOM_TABLE_COTATIONS " (TypeActe, MontantConv, MontantPratique, CCAM, idUser) VALUES ('" +
                            quer.value(0).toString() + "', " +
                            quer.value(2).toString() + ", "  +
                            quer.value(2).toString() + ", "  +
                            "1, " +
                            QString::number(gidUser) + ")";
                    QSqlQuery (req, DataBase::getInstance()->getDataBase() );
                }
            }
        }
    }*/
    //on modifie la table Actes avec la nouvelle cotation
    QString requete = "UPDATE " NOM_TABLE_ACTES " SET ActeCotation = '" + Cotation + "' WHERE idActe = " + QString::number(gidActe);
    QSqlQuery UpdateCotationQuery (requete, DataBase::getInstance()->getDataBase() );
    DataBase::getInstance()->traiteErreurRequete(UpdateCotationQuery,requete,"");
    ui->EnregistrePaiementpushButton->setEnabled(ui->ActeCotationcomboBox->currentText()!="");
}


void Rufus::SalleDAttente()
{
    if (AutorDepartConsult(true))
    {
        ui->tabWidget->removeTab(ui->tabWidget->indexOf(ui->tabDossier));
        FermeDlgAnnexes();
        OuvrirListe();
    }
}

void Rufus::AllusrChkBoxSendMsg(bool a)
{
    for (int i=0; i< gAsk->findChildren<UpCheckBox*>().size(); i++)
        if (gAsk->findChildren<UpCheckBox*>().at(i)->getRowTable() == 1)
            gAsk->findChildren<UpCheckBox*>().at(i)->setChecked(a);
}

void Rufus::OneusrChkBoxSendMsg(bool a)
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
    //TODO : SQL
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
        QSqlQuery quer(req, DataBase::getInstance()->getDataBase() );
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
    QSqlQuery usrquer(req1, DataBase::getInstance()->getDataBase() );
    if (usrquer.size()>0)
    {
        usrquer.first();
        UsrGroupBox = new QGroupBox(gAsk);
        UsrGroupBox->setTitle(tr("Destinataire"));
        UpCheckBox *Allusrchk = new UpCheckBox();
        Allusrchk->setText(tr("Tout le monde"));
        Allusrchk->setAutoExclusive(false);
        Allusrchk->setRowTable(2);
        connect(Allusrchk,  &QCheckBox::clicked,  [=] {AllusrChkBoxSendMsg(Allusrchk->isChecked());});
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
            connect(chk0,  &QCheckBox::clicked,  [=] {OneusrChkBoxSendMsg(chk0->isChecked());});
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
    connect(checktask,  &QCheckBox::clicked,  [=] {AfficheMessageLimitDate(checktask->isChecked());});
    tasklayout      ->addWidget(checktask);
    tasklayout      ->addWidget(limitdate);
    tasklayout      ->addSpacerItem(new QSpacerItem(10,10,QSizePolicy::Expanding));
    tasklayout      ->setContentsMargins(5,0,0,0);

    msglayout       ->addWidget(checkurg);
    msglayout       ->addWidget(MsgText);
    msglayout       ->addLayout(tasklayout);

    gAsk->OKButton   ->setId(idMsg);
    connect(gAsk->OKButton,   QOverload<int>::of(&UpSmallButton::clicked),  [=] {VerifSendMessage(idMsg);});

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

void Rufus::VerifSendMessage(int idMsg)
{
    //TODO : SQL
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
    QStringList locklist;
    locklist <<  NOM_TABLE_MESSAGES << NOM_TABLE_MESSAGESJOINTURES;
    DataBase::getInstance()->locktables(locklist);
    if (idMsg<0)  // Enregistrement d'un nouveau message
    {
        QString req = "insert into " NOM_TABLE_MESSAGES " (idEmetteur, TexteMessage, idPatient, Tache, DateLimite, Urge, CreeLe)\n values(";
        req += QString::number(gDataUser->id()) + ", ";
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
        QSqlQuery quer(req, DataBase::getInstance()->getDataBase() );
        //qDebug() << req;
        if (DataBase::getInstance()->traiteErreurRequete(quer,req,tr("Impossible d'enregistrer ce message")))
            DataBase::getInstance()->rollback();

        QString ChercheMaxrequete = "SELECT Max(idMessage) FROM " NOM_TABLE_MESSAGES;
        QSqlQuery ChercheMaxidMsgQuery (ChercheMaxrequete, DataBase::getInstance()->getDataBase() );
        if (DataBase::getInstance()->traiteErreurRequete(ChercheMaxidMsgQuery, ChercheMaxrequete,""))
        {
            DataBase::getInstance()->rollback();
            return;
        }
        if (ChercheMaxidMsgQuery.size()==0)
        {
            DataBase::getInstance()->rollback();
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
            DataBase::getInstance()->rollback();
            return;
        }
        req = "insert into " NOM_TABLE_MESSAGESJOINTURES " (idMessage, idDestinataire) Values ";
        for (int i=0; i<listidusr.size(); i++)
        {
            req += "(" + QString::number(idmsg) + "," + QString::number(listidusr.at(i)) + ")";
            if (i<listidusr.size()-1)
                req += ",";
        }
        QSqlQuery isnquer(req, DataBase::getInstance()->getDataBase() );

        if (DataBase::getInstance()->traiteErreurRequete(isnquer, req, tr("Impossible d'enregistrer le message")))
        {
            DataBase::getInstance()->rollback();
            return;
        }
        envoieMessageA(listidusr);
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
        QSqlQuery quer(req, DataBase::getInstance()->getDataBase() );
        //qDebug() << req;
        if (DataBase::getInstance()->traiteErreurRequete(quer,req,tr("Impossible d'enregistrer ce message")))
            DataBase::getInstance()->rollback();
        QSqlQuery ("delete from " NOM_TABLE_MESSAGESJOINTURES " where idmessage = " + QString::number(idMsg), DataBase::getInstance()->getDataBase() );
        QList<int> listidusr;
        for (int j=0; j< gAsk->findChildren<UpCheckBox*>().size(); j++)
            if (gAsk->findChildren<UpCheckBox*>().at(j)->getRowTable() == 1)       // c'est le checkbox d'un user
                if (gAsk->findChildren<UpCheckBox*>().at(j)->isChecked())
                    listidusr << gAsk->findChildren<UpCheckBox*>().at(j)->iD();
        if (listidusr.size()==0)
        {
            DataBase::getInstance()->rollback();
            return;
        }
        req = "insert into " NOM_TABLE_MESSAGESJOINTURES " (idMessage, idDestinataire) Values ";
        for (int i=0; i<listidusr.size(); i++)
        {
            req += "(" + QString::number(idMsg) + "," + QString::number(listidusr.at(i)) + ")";
            if (i<listidusr.size()-1)
                req += ",";
        }
        QSqlQuery isnquer(req, DataBase::getInstance()->getDataBase() );
        if (DataBase::getInstance()->traiteErreurRequete(isnquer, req, tr("Impossible d'enregistrer le message")))
        {
            DataBase::getInstance()->rollback();
            return;
        }
    }
    proc->Message(tr("Message enregistré"),1000,false);
    DataBase::getInstance()->commit();
    gAsk->accept();
}

void Rufus::AfficheMessageImport(QStringList listmsg, int pause, bool bottom)
{
    dlg_message(listmsg, pause, bottom);
}

void Rufus::AfficheMessageLimitDate(bool a)
{
    gAsk->findChildren<QDateEdit*>().at(0)->setEnabled(a);
}

void Rufus::setTitre()
{
    QString modeconnexion = "";
    if (DataBase::getInstance()->getMode() == DataBase::Poste)
        modeconnexion = tr("monoposte");
    else if (DataBase::getInstance()->getMode() == DataBase::ReseauLocal)
        modeconnexion = tr("réseau local");
    if (DataBase::getInstance()->getMode() == DataBase::Distant)
    {
        modeconnexion = tr("accès distant - connexion ");
        if (proc->gsettingsIni->value("BDD_DISTANT/SSL").toString() != "NO")
            modeconnexion += tr("cryptée (SSL)");
        else
            modeconnexion += tr("non cryptée");
    }
    QString windowtitle = "Rufus - " + gDataUser->getLogin() + " - " + gDataUser->getFonction() + " - " + modeconnexion + " - " + qApp->applicationVersion();
    if (DataBase::getInstance()->getMode() != DataBase::Distant)
        windowtitle +=  (UtiliseTCP? " - TCP" : "");
    setWindowTitle(windowtitle);
}

void Rufus::SurbrillanceSalDat(UpLabel *lab)
{
    QString styleurg = "background:#EEFFFF ; color: red";
    QString Msg, background;
    QString backgroundsurbrill = "background:#B2D7FF";
    if (lab==Q_NULLPTR)
        return;
    int idpat       = lab->getData()["idpat"].toInt();
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
    QSqlQuery quer(req, DataBase::getInstance()->getDataBase() );
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
                              "where idpat = " + QString::number(labi0->getData()["idpat"].toInt());
                QSqlQuery queri(req, DataBase::getInstance()->getDataBase() );
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

void Rufus::SupprimerDocs()
{
    if (PosteImport)
    {
        QSqlQuery ("lock tables '" NOM_TABLE_DOCSASUPPRIMER "' write",  DataBase::getInstance()->getDataBase() );
        QString req = "Select filepath from " NOM_TABLE_DOCSASUPPRIMER;
        //qDebug() << req;
        QSqlQuery delreq (req,  DataBase::getInstance()->getDataBase() );
        for (int i=0; i<delreq.size(); i++)
        {
            delreq.seek(i);
            QString CheminFichier ("");
            if (DataBase::getInstance()->getMode() == DataBase::ReseauLocal)
                CheminFichier = proc->gsettingsIni->value("BDD_LOCAL/DossierImagerie").toString();
            if (DataBase::getInstance()->getMode() == DataBase::Poste)
                CheminFichier = proc->DirImagerie();
            CheminFichier += delreq.value(0).toString();
            if (!QFile(CheminFichier).remove())
                UpMessageBox::Watch(this, tr("Fichier introuvable!"), CheminFichier);
            QSqlQuery("delete from " NOM_TABLE_DOCSASUPPRIMER " where filepath = '" + delreq.value(0).toString() + "'",  DataBase::getInstance()->getDataBase() );
        }
        QSqlQuery("unlock tables",  DataBase::getInstance()->getDataBase() );
    }
}

void Rufus::SupprimerDossier()
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

void Rufus::AfficheMessages(int idx)
{
    QTabWidget* Tabw = Remplir_MsgTabWidget();
    if (Tabw->count()>idx)
        Tabw->setCurrentIndex(idx);
    QVBoxLayout *globallay = new QVBoxLayout();
    if (gMsgDialog != Q_NULLPTR)
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
    gMsgDialog->setWindowIcon(Icons::icSunglasses());
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
        " iddestinataire = " + QString::number(gDataUser->id()) + "\n"
        " order by urge desc, CreeLe desc";
    //proc->Edit(req);
    QSqlQuery querdest(req, DataBase::getInstance()->getDataBase() );
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
        tabw->addTab(Scroll, Icons::icPostit(), tr("Reçu"));

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
            connect(Rdchk,  &QCheckBox::clicked,    [=] {MsgRead(Rdchk);});
            titrelay->addSpacerItem(new QSpacerItem(10,10,QSizePolicy::Expanding));
            titrelay->addWidget(Rdchk);
            reclay->addLayout(titrelay);

            QHBoxLayout *Msglay = new QHBoxLayout();

            QVBoxLayout *Droplay = new QVBoxLayout();
            Droplay->addSpacerItem(new QSpacerItem(10,10, QSizePolicy::Expanding, QSizePolicy::Expanding));
            if (querdest.value(1).toInt() != gDataUser->id())
            {
                UpLabel *Respondlbl = new UpLabel();
                Respondlbl->setId(querdest.value(0).toInt());
                Respondlbl->setPixmap(Icons::pxConversation().scaled(20,20)); //WARNING : icon scaled : pxConversation 20,20
                Respondlbl->setImmediateToolTip(tr("Répondre"));
                connect(Respondlbl,     QOverload<int>::of(&UpLabel::clicked), [=] {MsgResp(Respondlbl->getId());});
                Respondlbl->setFixedWidth(25);
                Droplay->addWidget(Respondlbl);
            }

            UpLabel *Dellbl = new UpLabel();
            Dellbl->setId(querdest.value(10).toInt());
            Dellbl->setPixmap(Icons::pxPoubelle().scaled(20,20)); //WARNING : icon scaled : pxPoubelle 20,20
            Dellbl->setFixedWidth(25);
            Dellbl->setMinimumWidth(25);
            connect(Dellbl,             QOverload<int>::of(&UpLabel::clicked), [=] {SupprimerMessageRecu(Dellbl->getId());});
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
                connect(Dnchk,  &QCheckBox::clicked,    [=] {MsgDone(Dnchk);});
                Tasklay->addWidget(Dnchk);
                SMlay->addLayout(Tasklay);
            }
            if (querdest.value(3).toInt()>0)                                                                            // le message concerne un patient
            {
                QHBoxLayout *aboutlay = new QHBoxLayout();
                UpLabel *aboutdoc = new UpLabel();
                QString nomprenom = "";
                QString reqq = "select patprenom, patnom from " NOM_TABLE_PATIENTS " where idpat = " + QString::number(querdest.value(3).toInt());
                QSqlQuery querr(reqq, DataBase::getInstance()->getDataBase() );
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
            Msgtxt->setFixedSize(380,int(Msgtxt->document()->size().height())+2);

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
        " idemetteur = " + QString::number(gDataUser->id()) + "\n"
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
    QSqlQuery queremet(req, DataBase::getInstance()->getDataBase() );
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
        tabw->addTab(Scroll, Icons::icMessage(), tr("Envoyé"));

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
            Modiflbl->setPixmap(Icons::pxEditer().scaled(20,20)); //WARNING : icon scaled : pxEditer 20,20
            Modiflbl->setImmediateToolTip(tr("Modifier"));
            Modiflbl->setFixedWidth(25);
            connect(Modiflbl, QOverload<int>::of(&UpLabel::clicked),  [=] {MsgModif(Modiflbl->getId());});
            Droplay->addWidget(Modiflbl);

            UpLabel *Dellbl = new UpLabel();
            Dellbl->setId(queremet.value(0).toInt());
            Dellbl->setPixmap(Icons::pxPoubelle().scaled(20,20)); //WARNING : icon scaled : pxPoubelle 20,20
            Dellbl->setFixedWidth(25);
            connect(Dellbl,     QOverload<int>::of(&UpLabel::clicked),  [=] {SupprimerMessageEmis(Dellbl->getId());});
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
                QSqlQuery querr(reqq, DataBase::getInstance()->getDataBase() );
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
            Msgtxt->setFixedSize(380,int(Msgtxt->document()->size().height())+2);
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

void Rufus::MsgResp(int idmsg)
{
    QVBoxLayout *globallay = new QVBoxLayout();
    gMsgRepons = new QDialog();

    QString req = "select userlogin from " NOM_TABLE_UTILISATEURS " where iduser in (select idemetteur from " NOM_TABLE_MESSAGES " where idmessage = " + QString::number(idmsg) +  ")";
    QSqlQuery quer(req, DataBase::getInstance()->getDataBase() );
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
    QSqlQuery txtquer(req, DataBase::getInstance()->getDataBase() );
    if (txtquer.size()>0)
    {
        QHBoxLayout *lbllayout  = new QHBoxLayout();
        UpLabel     *msglbl     = new UpLabel(gMsgRepons);
        txtquer     .first();
        QString nomprenom = "";
        if (txtquer.value(1).toInt()>0)
        {
            QString reqq = "select patprenom, patnom from " NOM_TABLE_PATIENTS " where idpat = " + QString::number(txtquer.value(1).toInt());
            QSqlQuery querr(reqq, DataBase::getInstance()->getDataBase() );
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
    connect(OKbutton, &QPushButton::clicked, [=] {EnregMsgResp(idmsg);});
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
    gMsgRepons->setWindowIcon(Icons::icSunglasses());
    gMsgRepons->setFixedWidth(450);

    gMsgRepons->exec();
    delete gMsgRepons;
}

void Rufus::EnregMsgResp(int idmsg)
{
     if (gMsgRepons->findChildren<UpTextEdit*>().at(0)->toPlainText()=="")
    {
        proc->Message(tr("Vous avez oublié de rédiger le texte de votre message!"),2000,false);
        return;
    }
    QString req = "select idemetteur, tache, datelimite, urge from " NOM_TABLE_MESSAGES " where idmessage = " + QString::number(idmsg);
    QSqlQuery quer(req, DataBase::getInstance()->getDataBase() );
    if (quer.size() == 0)
        return;
    quer.first();
    int iddest          = quer.value(0).toInt();
    QString tache       = ((quer.value(1).toInt()==1)? "1" : "null");
    QString DateLimit   = ((quer.value(2).toDate().isValid())? "'" + quer.value(2).toDate().toString("yyyy-MM-dd") + "'" : "null");
    QString Urg         = ((quer.value(3).toInt()==1)? "1" : "null");

    QStringList locklist;
    locklist << NOM_TABLE_MESSAGES << NOM_TABLE_MESSAGESJOINTURES;
    DataBase::getInstance()->locktables(locklist);

    req  = "insert into " NOM_TABLE_MESSAGES " (idEmetteur, TexteMessage, CreeLe, ReponseA, Tache, Datelimite, Urge)\n values(";
    req += QString::number(gDataUser->id()) + ", ";
    QString Reponse = "<font color = " + proc->CouleurTitres + ">" + gMsgRepons->findChildren<UpLabel*>().at(0)->text() + "</font>"
            + "------<br><b>" + gDataUser->getLogin() + ":</b> " + gMsgRepons->findChildren<UpTextEdit*>().at(0)->toPlainText().replace("\n","<br>");
    UpTextEdit txt;
    txt.setText(Reponse);
    req += "'" + proc->CorrigeApostrophe(txt.toHtml()) + "', ";
    req += "'" + QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") + "', ";
    req += QString::number(idmsg) + ", ";
    req += tache + ", ";
    req += DateLimit + ", ";
    req += Urg + ")";
    //qDebug() << req;
    QSqlQuery insquer(req, DataBase::getInstance()->getDataBase() );

    if (DataBase::getInstance()->traiteErreurRequete(insquer,req,tr("Impossible d'enregistrer ce message")))
        DataBase::getInstance()->rollback();

    QString ChercheMaxrequete = "SELECT Max(idMessage) FROM " NOM_TABLE_MESSAGES;
    QSqlQuery ChercheMaxidMsgQuery (ChercheMaxrequete, DataBase::getInstance()->getDataBase() );
    if (DataBase::getInstance()->traiteErreurRequete(ChercheMaxidMsgQuery, ChercheMaxrequete,""))
    {
        DataBase::getInstance()->rollback();
        return;
    }
    if (ChercheMaxidMsgQuery.size()==0)
    {
        DataBase::getInstance()->rollback();
        return;
    }
    ChercheMaxidMsgQuery.first();
    int idrep = ChercheMaxidMsgQuery.value(0).toInt();
    req = "insert into " NOM_TABLE_MESSAGESJOINTURES " (idMessage, idDestinataire) Values ";
    req += "(" + QString::number(idrep) + "," + QString::number(iddest) + ")";
    QSqlQuery isnquer(req, DataBase::getInstance()->getDataBase() );

    if (DataBase::getInstance()->traiteErreurRequete(isnquer, req, tr("Impossible d'enregistrer le message")))
    {
        DataBase::getInstance()->rollback();
        return;
    }
    else
    {
        proc->Message(tr("Message enregistré"),1000,false);
        DataBase::getInstance()->commit();
        envoieMessageA(QList<int>() << iddest);
    }
    gMsgRepons->accept();
}

void Rufus::MsgModif(int idmsg)
{
    QList<UpTextEdit*> listtxt = gMsgDialog->findChildren<UpTextEdit*>();
    if (listtxt.size()>0)
        for (int i=0; i<listtxt.size();i++)
        {
            if (listtxt.at(i)->getId()==idmsg)
            {
                QString req = "select TexteMessage, idPatient, Tache, DateLimite, CreeLe, Urge from " NOM_TABLE_MESSAGES
                              " where idMessage = " + QString::number(idmsg);
                QSqlQuery quer(req, DataBase::getInstance()->getDataBase() );
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
                QSqlQuery jointquer(req, DataBase::getInstance()->getDataBase() );
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

void Rufus::MsgDone(UpCheckBox *chk)
{
    int idjoin = chk->iD();
    QString res = (chk->isChecked()? "1" : "NULL");
    QSqlQuery("update " NOM_TABLE_MESSAGESJOINTURES " set Fait = " + res + " where idjointure = " + QString::number(idjoin),  DataBase::getInstance()->getDataBase() );
}

void Rufus::MsgRead(UpCheckBox *chk)
{
    int idjoin = chk->iD();
    QString res = (chk->isChecked()? "1" : "NULL");
    QSqlQuery("update " NOM_TABLE_MESSAGESJOINTURES " set Lu = " + res + " where idjointure = " + QString::number(idjoin),  DataBase::getInstance()->getDataBase() );
}

void Rufus::SupprimerMessageEmis(int idMsg)
{
    QString req = "update " NOM_TABLE_MESSAGES " set ASupprimer = 1 where idmessage = " + QString::number(idMsg);
    QSqlQuery (req, DataBase::getInstance()->getDataBase() );
    req = "delete from " NOM_TABLE_MESSAGESJOINTURES " where "
          "idmessage = " + QString::number(idMsg) +
          " and iddestinataire = " + QString::number(gDataUser->id());
    QSqlQuery (req, DataBase::getInstance()->getDataBase() );
    if (gMsgDialog->findChildren<QScrollArea*>().size()>0)
        AfficheMessages(1);
}

void Rufus::SupprimerMessageRecu(int idJoint)
{
    QString req = "select idmessage from " NOM_TABLE_MESSAGESJOINTURES  " where idjointure = " + QString::number(idJoint);
    //qDebug()<<req;
    QSqlQuery idmsgquer(req, DataBase::getInstance()->getDataBase() );
    idmsgquer.first();
    int idmsg = idmsgquer.value(0).toInt();
    req = "select idemetteur from " NOM_TABLE_MESSAGES  " where idmessage = " + QString::number(idmsg);
    QSqlQuery idemetquer(req, DataBase::getInstance()->getDataBase() );
    idemetquer.first();
    int idusr = idemetquer.value(0).toInt();
    if (idusr == gDataUser->id())
        QSqlQuery ("update " NOM_TABLE_MESSAGES " set ASupprimer = 1 where idmessage = " + QString::number(idmsg), DataBase::getInstance()->getDataBase() );
    QSqlQuery ("delete from " NOM_TABLE_MESSAGESJOINTURES " where idjointure = " + QString::number(idJoint), DataBase::getInstance()->getDataBase() );
    req = "delete from " NOM_TABLE_MESSAGES " where "
          "idmessage not in (select idmessage from " NOM_TABLE_MESSAGESJOINTURES ") "
          " and ASupprimer = 1";
    QSqlQuery (req, DataBase::getInstance()->getDataBase() );
    if (gMsgDialog->findChildren<QScrollArea*>().size()>0)
        AfficheMessages();
}

void Rufus::VerifMessages()
{
    if (!QSystemTrayIcon::isSystemTrayAvailable())
        return;
    QDateTime DateMsg;
    gTotalNvxMessages = 0;
    QString req =
        "select Distinct mess.idMessage, Creele, ReponseA from "
        NOM_TABLE_MESSAGES " mess left outer join " NOM_TABLE_MESSAGESJOINTURES " joint on mess.idmessage = joint.idmessage \n"
        " where \n"
        " iddestinataire = " + QString::number(gDataUser->id()) + "\n"
        " or (idemetteur = " + QString::number(gDataUser->id()) + " and asupprimer is null)"
        " order by CreeLe";
    /*
    select Distinct mess.idMessage, Creele, ReponseA from Rufus.Messagerie mess left outer join Rufus.MessagerieJointures joint on mess.idmessage = joint.idmessage
    where iddestinataire = 1
    or (idemetteur = 1 and asupprimer is null)
    order by CreeLe
    */
    QSqlQuery quer(req, DataBase::getInstance()->getDataBase());
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
    else if (gMsgDialog !=Q_NULLPTR)
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
        gMessageIcon->showMessage(tr("Messages"), msg, Icons::icPostit(), 10000);
        if (gMsgDialog != Q_NULLPTR)
            if (gMsgDialog->isVisible())
                AfficheMessages();
    }
    gAffichTotalMessages = false;
}

void Rufus::VerifSalleDAttente()
{
    int flagsaldat = proc->GetflagSalDat();
    if (gflagSalDat < flagsaldat)
    {
        gflagSalDat = flagsaldat;
        Remplir_SalDat();                       // par le timer VerifSalleDAttente
    }
}

void Rufus::VerifCorrespondants()
{
    int flagcor = proc->GetflagMG();
    if (gflagMG < flagcor)
    {
        gflagMG = flagcor;
        // on reconstruit la liste des MG et des correspondants
        proc->initListeCorrespondants();
        ReconstruitCombosCorresp();                     // par le timer VerifSalleDAttente
        // on resynchronise l'affichage du combobox au besoin
        if (ui->tabWidget->indexOf(ui->tabDossier) > -1)
        {
            QString req = "select idcormedmg from " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS " where idpat = " + QString::number(gidPatient);
            QSqlQuery quer(req, DataBase::getInstance()->getDataBase() );
            if (quer.seek(0))
                ui->MGupComboBox->setCurrentIndex(ui->MGupComboBox->findData(quer.value(0).toInt()));
            else
                ui->MGupComboBox->setCurrentIndex(-1);
            req = "select idcormedspe1 from " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS " where idpat = " + QString::number(gidPatient);
            QSqlQuery quer1(req, DataBase::getInstance()->getDataBase() );
            if (quer1.seek(0))
                ui->AutresCorresp1upComboBox->setCurrentIndex(ui->AutresCorresp1upComboBox->findData(quer1.value(0).toInt()));
            else
                ui->AutresCorresp1upComboBox->setCurrentIndex(-1);
            req = "select idcormedspe2 from " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS " where idpat = " + QString::number(gidPatient);
            QSqlQuery quer2(req, DataBase::getInstance()->getDataBase() );
            if (quer2.seek(0))
                ui->AutresCorresp2upComboBox->setCurrentIndex(ui->AutresCorresp2upComboBox->findData(quer2.value(0).toInt()));
            else
                ui->AutresCorresp2upComboBox->setCurrentIndex(-1);
            OKModifierTerrain();
        }
    }
}

void Rufus::VerifVerrouDossier()
{
    /* Cette fonction sert à déconnecter et lever les verrous d'un utilisateur qui se serait déconnecté accidentellement
     *
     on fait la liste des utilisateurs qui n'ont pas remis à jour leur connexion depuis plus de 60 secondes,
     on retire les verrous qu'ils auraient pu poser et on les déconnecte*/
    QString VerifOldUserreq = "select idUser, NomPosteConnecte from  " NOM_TABLE_USERSCONNECTES " where time_to_sec(timediff(now(),heurederniereconnexion)) > 60";
    QSqlQuery verifoldquery (VerifOldUserreq, DataBase::getInstance()->getDataBase() );
    //qDebug() << VerifOldUserreq;
    DataBase::getInstance()->traiteErreurRequete(verifoldquery,VerifOldUserreq,"");

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
            QSqlQuery LibereVerrouRequeteQuery (LibereVerrouRequete, DataBase::getInstance()->getDataBase() );
            DataBase::getInstance()->traiteErreurRequete(LibereVerrouRequeteQuery,LibereVerrouRequete,"");
            //qDebug() << LibereVerrouRequete;
            //on déverrouille les actes verrouillés en comptabilité par cet utilisateur
            LibereVerrouRequete = "delete from " NOM_TABLE_VERROUCOMPTAACTES " where PosePar = " + QString::number(a);
            QSqlQuery LibereVerrouComptaQuery (LibereVerrouRequete, DataBase::getInstance()->getDataBase() );
            DataBase::getInstance()->traiteErreurRequete(LibereVerrouComptaQuery,LibereVerrouRequete,"");
            // on retire cet utilisateur de la table des utilisateurs connectés
            QString req = "delete from " NOM_TABLE_USERSCONNECTES " where NomPosteConnecte = '" + Poste + "' and idUser = " + QString::number(a);
            QSqlQuery(req, DataBase::getInstance()->getDataBase() );
            FlagMetAjourSalDat();
            proc->Message(tr("Le poste ") + Poste + tr(" a été retiré de la liste des postes connectés actuellement au serveur"),1000);
            verifoldquery.next();
        }
    }

    // on donne le statut "arrivé" aux patients en salle d'attente dont le iduserencourssexam n'est plus present sur ce poste examen dans la liste des users connectes
    QString req = "select iduserencoursexam, posteexamen, idpat from " NOM_TABLE_SALLEDATTENTE " where statut like '" ENCOURSEXAMEN "%'";
    //qDebug() << req;
    QSqlQuery querr(req, DataBase::getInstance()->getDataBase() );
    for (int i=0; i<querr.size(); i++)
    {
        querr.seek(i);
        req = "select iduser, nomposteconnecte from " NOM_TABLE_USERSCONNECTES " where iduser = " + querr.value(0).toString()  + " and nomposteconnecte = '" + querr.value(1).toString() + "'";
        //qDebug() << req;
        QSqlQuery squer(req, DataBase::getInstance()->getDataBase() );
        if (squer.size()==0)
        {
            req = "update " NOM_TABLE_SALLEDATTENTE " set Statut = '" ARRIVE "', posteexamen = null, iduserencoursexam = null where idpat = " + querr.value(2).toString();
            //qDebug() << req;
            QSqlQuery(req,  DataBase::getInstance()->getDataBase() );
        }
    }
}

bool Rufus::isPosteImport()
{
    return PosteImport;
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
    if (DataBase::getInstance()->getMode() == DataBase::Distant)
    {
        if (ImportDocsExtThread == Q_NULLPTR)
        {
            ImportDocsExtThread = new ImportDocsExternesThread(proc);
            connect(ImportDocsExtThread, SIGNAL(emitmsg(QStringList, int, bool)), this, SLOT(AfficheMessageImport(QStringList, int, bool)));
        }
        PosteImport = true;
        return;
    }

    bool statut = PosteImport;
    QString ImportateurDocs       = proc->PosteImportDocs(); //le nom du poste importateur des docs externes
    if (ImportateurDocs.toUpper() == "NULL")
    {
        if ((proc->gsettingsIni->value("BDD_LOCAL/PrioritaireGestionDocs").toString() == "YES" || proc->gsettingsIni->value("BDD_LOCAL/PrioritaireGestionDocs").toString() == "NORM")
                && DataBase::getInstance()->getMode() != DataBase::Distant)
             proc->setPosteImportDocs();
    }
    else
    {
        QString IpAdr = "";
        QString B = proc->gsettingsIni->value("BDD_LOCAL/PrioritaireGestionDocs").toString();
        if (B=="YES")
            IpAdr = QHostInfo::localHostName() + " - prioritaire";
        else if (B=="NORM")
            IpAdr = QHostInfo::localHostName();

        if (ImportateurDocs != IpAdr) //si le poste défini comme importateur des docs est différent de ce poste, on vérifie qu'il est toujours actif et qu'il n'es pas prioritaire
        {
            QString req = "select nomposteconnecte from " NOM_TABLE_USERSCONNECTES " where nomposteconnecte = '" + ImportateurDocs.remove(" - prioritaire") + "'";
            QSqlQuery quer(req, DataBase::getInstance()->getDataBase() );
            if (quer.size()==0)
            {
                /*Si le poste défini comme importateur des docs externes n'est pas connecté,
                 on prend la place si
                    on n'est pas en accès distant
                    et si on est importateur
                sinon, on retire le poste*/
                proc->setPosteImportDocs((B == "YES" || B == "NORM") && DataBase::getInstance()->getMode() != DataBase::Distant);
            }
            else if (!ImportateurDocs.contains(" - " NOM_ADMINISTRATEURDOCS))
                // le poste défini comme importateur est valide mais pas administrateur, on prend sa place si
                //  on est prioritaire et pas lui
                //  à condition de ne pas être en accès distant
            {
                if (B == "YES" && !ImportateurDocs.contains(" - prioritaire") && DataBase::getInstance()->getMode() != DataBase::Distant)
                    proc->setPosteImportDocs();
                else if (ImportateurDocs.remove(" - prioritaire") == QHostInfo::localHostName()) // cas rare du poste qui a modifié son propre statut
                    proc->setPosteImportDocs((B == "YES" || B == "NORM") && DataBase::getInstance()->getMode() != DataBase::Distant);
            }
        }
    }
    PosteImport = (proc->PosteImportDocs().remove(" - prioritaire") == QHostInfo::localHostName());
    bool chgtstatut = (statut != PosteImport);
    if (chgtstatut)
    {
        if (PosteImport)
        {
            connect(gTimerExportDocs,           &QTimer::timeout,   [=] {ExporteDocs();});
            if (ImportDocsExtThread == Q_NULLPTR)
                if (proc->gsettingsIni->value("BDD_LOCAL/PrioritaireGestionDocs").toString() == "YES" || proc->gsettingsIni->value("BDD_LOCAL/PrioritaireGestionDocs").toString() == "NORM")
                {
                    ImportDocsExtThread = new ImportDocsExternesThread(proc);
                    connect(ImportDocsExtThread, SIGNAL(emitmsg(QStringList, int, bool)), this, SLOT(AfficheMessageImport(QStringList, int, bool)));
                }
        }
        else
            gTimerExportDocs->disconnect();
    }
}

void Rufus::ActualiseDocsExternes()
{
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
                OuvrirListe();
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
        case Qt::Key_F9:
            if (ui->tabWidget->indexOf(ui->tabDossier) < 0 && gDataUser->isSoignant())
                AfficheCourriersAFaire();
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
                if (objUpText->getTableCorrespondant() == NOM_TABLE_ACTES || objUpText->getTableCorrespondant() == NOM_TABLE_MESSAGES)
                {
                    QString Corps = objUpText->toHtml();
                    Corps.replace("<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">","<p style=\" margin-top:0px; margin-bottom:0px;\">");
                    Corps.remove("border=\"0\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px;\" ");
#ifdef Q_OS_LINUX
                    if (Corps.contains("<!MAC>"))
                        Corps.replace("<!MAC>","<!LINUX>");
                    else if (!Corps.contains("<!LINUX>"))
                        Corps.append("<!LINUX>");
#endif
#ifdef Q_OS_MAC
                    if (Corps.contains("<!LINUX>"))
                        Corps.replace("<!LINUX>","<!MAC>");
                    else if (!Corps.contains("<!MAC>"))
                        Corps.append("<!MAC>");
#endif
                    requetemodif =   "UPDATE " + objUpText->getTableCorrespondant() + " SET " + objUpText->getChampCorrespondant() + " = '"
                            + proc->CorrigeApostrophe(Corps) + "' WHERE " + (objUpText->getTableCorrespondant() == NOM_TABLE_ACTES? "idActe" : "idMessage") + "= " + QString::number(gidActe);
                    QSqlQuery UpdateUpTextEditQuery (requetemodif, DataBase::getInstance()->getDataBase() );
                    DataBase::getInstance()->traiteErreurRequete(UpdateUpTextEditQuery,requetemodif,tr("Impossible de mettre à jour le champ ") + objUpText->getChampCorrespondant() + "!");
                }
                else
                {
                    // on vérifie d'abord s'il existe un enregistrement pour ce patient dans la table correspondante, sinon, on le crée
                    QString verifrequete = "select idPat from " + objUpText->getTableCorrespondant() + " where idpat = " + QString::number(gidPatient);
                    QSqlQuery verifquery(verifrequete, DataBase::getInstance()->getDataBase() );
                    if (verifquery.size() == 0)
                        requetemodif =   "INSERT INTO " + objUpText->getTableCorrespondant() + " (" + objUpText->getChampCorrespondant() + ",idPat)"
                                + " VALUES ('" + proc->CorrigeApostrophe(objUpText->toPlainText()) + "', " + QString::number(gidPatient) + ")";
                    else
                        requetemodif =   "UPDATE " + objUpText->getTableCorrespondant() + " SET " + objUpText->getChampCorrespondant() + " = '"
                                + proc->CorrigeApostrophe(objUpText->toPlainText()) + "' WHERE idPat = " + QString::number(gidPatient);
                    QSqlQuery UpdateUpTextEditQuery (requetemodif, DataBase::getInstance()->getDataBase() );
                    DataBase::getInstance()->traiteErreurRequete(UpdateUpTextEditQuery,requetemodif,tr("Impossible de mettre à jour le champ ") + objUpText->getChampCorrespondant() + "!");
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
                    QSqlQuery UpdateUpTextEditQuery (requetemodif, DataBase::getInstance()->getDataBase() );
                    DataBase::getInstance()->traiteErreurRequete(UpdateUpTextEditQuery,requetemodif,tr("Impossible de mettre à jour le champ ") + objUpText->getChampCorrespondant() + "!");
                }
                else
                {
                    objUpText->setText(proc->MajusculePremiereLettre(objUpText->text(),true));
                    // on vérifie d'abord s'il existe un enregistrement pour ce patient dans la table correspondante, sinon, on le crée
                    QString verifrequete = "select idPat from " + objUpText->getTableCorrespondant() + " where idpat = " + QString::number(gidPatient);
                    QSqlQuery verifquery(verifrequete, DataBase::getInstance()->getDataBase() );
                    if (verifquery.size() == 0)
                    {
                        requetemodif =   "INSERT INTO " + objUpText->getTableCorrespondant() + " (" + objUpText->getChampCorrespondant() + ",idPat)"
                                + " VALUES ('" + proc->CorrigeApostrophe(objUpText->text()) + "', " + QString::number(gidPatient) + ")";
                        QSqlQuery UpdateUpTextEditQuery (requetemodif, DataBase::getInstance()->getDataBase() );
                        DataBase::getInstance()->traiteErreurRequete(UpdateUpTextEditQuery,requetemodif,tr("Impossible de mettre à jour le champ ") + objUpText->getChampCorrespondant() + "!");
                    }
                    else
                    {
                        requetemodif =   "UPDATE " + objUpText->getTableCorrespondant() + " SET " + objUpText->getChampCorrespondant() + " = '"
                                + proc->CorrigeApostrophe(objUpText->text()) + "' WHERE idPat = " + QString::number(gidPatient);
                        QSqlQuery UpdateUpTextEditQuery (requetemodif, DataBase::getInstance()->getDataBase() );
                        DataBase::getInstance()->traiteErreurRequete(UpdateUpTextEditQuery,requetemodif,tr("Impossible de mettre à jour le champ ") + objUpText->getChampCorrespondant() + "!");
                    }
                    OKModifierTerrain();
                }
            }
        }
        else if (obj->objectName() == "ActeDatedateEdit")
        {
            if (ui->ActeDatedateEdit->text() != gActeDate)
            {
                QString requete =   "UPDATE " NOM_TABLE_ACTES " SET ActeDate = '" + ui->ActeDatedateEdit->date().toString("yyyy-MM-dd") + "' WHERE idActe = " + QString::number(gidActe);
                QSqlQuery UpdateUpTextEditQuery (requete, DataBase::getInstance()->getDataBase() );
                if (DataBase::getInstance()->traiteErreurRequete(UpdateUpTextEditQuery,requete,tr("Impossible de mettre à jour la date de l'acte!")))
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
            Button->setIcon(Icons::icFermeAppuye());
        }

    if(event->type() == QEvent::MouseMove)
        if (obj == ui->FermepushButton  || obj == ui->LFermepushButton)
        {
            QPushButton* Button = static_cast<QPushButton*>(obj);
            QRect rect = QRect(Button->pos(),Button->size());
            QPoint pos = mapFromParent(cursor().pos());
            if (rect.contains(pos))
                Button->setIcon(Icons::icFermeAppuye());
            else
                Button->setIcon(Icons::icFerme());
        }

    if(event->type() == QEvent::MouseButtonRelease)
    {
        if (obj == ui->FermepushButton  || obj == ui->LFermepushButton)
        {
            QPushButton* Button = static_cast<QPushButton*>(obj);
            Button->setIcon(Icons::icFerme());
        }
        if (obj == ui->EnregistrePaiementpushButton && ui->EnregistrePaiementpushButton->isEnabled())
        {
            emit EnregistrePaiement();
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
                    int a = int(QFontMetrics(qApp->font()).height()*1.4);
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
            case Qt::Key_Up:
                MonteUneLigne();
                return true;
            case Qt::Key_Down:
                DescendUneLigne();
                return true;
            case Qt::Key_PageDown :
                Descend20Lignes();
                return true;
            case Qt::Key_PageUp :
                Monte20Lignes();
                return true;
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
    QSqlQuery AfficheActeQuery (req, DataBase::getInstance()->getDataBase() );
    if (DataBase::getInstance()->traiteErreurRequete(AfficheActeQuery,req,tr("Impossible de retrouver la dernière consultation"))
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

        QMap<QString,QVariant>  Age = Item::CalculAge(gDDNPatient, ui->ActeDatedateEdit->date());
        ui->AgelineEdit->setText(Age["toString"].toString());
        ui->AgelineEdit->setAlignment(Qt::AlignCenter);
        gAgePatient = Age["annee"].toInt();

        //2. retrouver le créateur de l'acte et le médecin superviseur de l'acte
        ui->CreeParlineEdit->setText(tr("Créé par ") + proc->getLogin(AfficheActeQuery.value(11).toInt())
                                     + tr(" pour ") + proc->getLogin(AfficheActeQuery.value(2).toInt()));

        //3. Mettre à jour le numéro d'acte
        req = "SELECT idActe FROM " NOM_TABLE_ACTES
                " WHERE idPat = '" + QString::number(gidPatient) + "' ORDER BY ActeDate";
        QSqlQuery ChercheNumActeQuery (req, DataBase::getInstance()->getDataBase() );
        if (!DataBase::getInstance()->traiteErreurRequete(ChercheNumActeQuery,req,tr("Impossible de retrouver les consultations de ce patient")))
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
    QSqlQuery AfficheTypePaiementQuery (req, DataBase::getInstance()->getDataBase() );
    DataBase::getInstance()->traiteErreurRequete(AfficheTypePaiementQuery,req,"Impossible de retrouver les renseignements comptables");

    bool a = (AfficheTypePaiementQuery.size() == 0);

    ui->Comptaframe->setVisible(!a);
    ui->Cotationframe->setEnabled(a);
    ui->CCAMlinklabel->setVisible(a);
    ui->EnregistrePaiementpushButton->setVisible(a && (gDataUser->isSoignant() && !gDataUser->isAssistant()));
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
        QSqlQuery ListePaiementsQuery (requete, DataBase::getInstance()->getDataBase() );
        DataBase::getInstance()->traiteErreurRequete(ListePaiementsQuery, requete, "");
        ListePaiementsQuery.first();
        for (int l = 0; l < ListePaiementsQuery.size(); l++)
        {
            requete = "SELECT Monnaie FROM " NOM_TABLE_RECETTES " WHERE idRecette = " + ListePaiementsQuery.value(0).toString();
            QSqlQuery MonnaieQuery (requete, DataBase::getInstance()->getDataBase() );
            DataBase::getInstance()->traiteErreurRequete(MonnaieQuery,requete,"");
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
void Rufus::AfficheDossier(int idPat, int idacte)
{
//  Afficher les éléments de la tables Patients

    QString     Msg;

    QString req = "SELECT idPat, PatNom, PatPrenom, PatDDN, Sexe, PatCreele, PatCreePar FROM " NOM_TABLE_PATIENTS
              " WHERE idPat = '" + QString::number(idPat) + "'";
    QSqlQuery AfficheDossierQuery (req, DataBase::getInstance()->getDataBase() );
    if (DataBase::getInstance()->traiteErreurRequete(AfficheDossierQuery,req,tr("Impossible de retrouver le dossier de ce patient")))
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
    QSqlQuery DonneesSocialesQuery (req, DataBase::getInstance()->getDataBase() );
    if (!DataBase::getInstance()->traiteErreurRequete(DonneesSocialesQuery,req,tr("Impossible de retrouver les données sociales!")))
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
    QMap<QString,QVariant>  AgeTotal = Item::CalculAge(gDDNPatient, gSexePat);
    gCMUPatient = (DonneesSocialesQuery.value(12).toInt() == 1);
    img = AgeTotal["icone"].toString();
    Age = AgeTotal["toString"].toString();
    QIcon icon = Icons::getIconAge(img);

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
    QSqlQuery DonneesMedicalesQuery (req, DataBase::getInstance()->getDataBase() );
    if (!DataBase::getInstance()->traiteErreurRequete(DonneesMedicalesQuery,req,"Impossible de retrouver les données médicales"))
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
        OKModifierTerrain();
    }
    FermeDlgAnnexes();

    //3 - récupération des actes

    QString recuprequete = "SELECT idActe FROM " NOM_TABLE_ACTES
            " WHERE idPat = '" + QString::number(idPat) + "' ORDER BY ActeDate";
    QSqlQuery ActesQuery (recuprequete, DataBase::getInstance()->getDataBase() );
    if (DataBase::getInstance()->traiteErreurRequete(ActesQuery,recuprequete,tr("Impossible de retrouver les consultations de ce patient")))
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
            int a = (idacte == 0? ActesQuery.value(0).toInt() : idacte);
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
    if (proc->PortRefracteur()!=Q_NULLPTR)
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
    QSqlQuery SalDatQuery (req, DataBase::getInstance()->getDataBase() );
    DataBase::getInstance()->traiteErreurRequete(SalDatQuery,req,tr("Impossible de trouver la salle d'attente!"));

    if (SalDatQuery.size() == 0)
    {
        req =   "INSERT INTO " NOM_TABLE_SALLEDATTENTE
                    " (idPat, idUser, Statut, HeureStatut, idUserEnCoursExam, PosteExamen, HeureArrivee)"
                    " VALUES ('" + QString::number(gidPatient) + "','" + QString::number(gDataUser->getIdUserActeSuperviseur()) + "','" ENCOURSEXAMEN + gDataUser->getLogin() + "','" + QTime::currentTime().toString("hh:mm")
                    + "'," + QString::number(gDataUser->id()) + ", '" + QHostInfo::localHostName().left(60) + "','" + QTime::currentTime().toString("hh:mm") +"')";
        Msg = tr("Impossible de mettre ce dossier en salle d'attente");
    }
    else
    {
        req =   "UPDATE " NOM_TABLE_SALLEDATTENTE
                    " SET Statut = '" ENCOURSEXAMEN + gDataUser->getLogin() +
                    "', HeureStatut = '" + QTime::currentTime().toString("hh:mm") +
                    "', idUserEnCoursExam = " + QString::number(gDataUser->id()) +
                    ", PosteExamen = '" + QHostInfo::localHostName().left(60) +
                    "' WHERE idPat = '" + QString::number(gidPatient) + "'";
        Msg = tr("Impossible de modifier le statut du dossier en salle d'attente!");
    }
    //UpMessageBox::Watch(this,req);
    QSqlQuery ModifSalDatQuery (req, DataBase::getInstance()->getDataBase() );
    DataBase::getInstance()->traiteErreurRequete(ModifSalDatQuery,req,Msg);

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
    FlagMetAjourSalDat();


    if (gDataUser->id() > 1) return;
    QString Sexe = "";
    req ="select idpat from " NOM_TABLE_PATIENTS " where patPrenom = '" + gPrenomPatient + "' and sexe = '' and patPrenom <> 'Dominique' and patPrenom <> 'Claude'";
    QSqlQuery quer(req, DataBase::getInstance()->getDataBase() );
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
            MBouton->setIcon(Icons::icMan());
            FBouton->setIcon(Icons::icWoman());
            msgbox->exec();
            if (msgbox->clickedButton() == MBouton)
                Sexe = "M";
            else if (msgbox->clickedButton() == FBouton)
                Sexe = "F";
            if (Sexe != ""){
                QSqlQuery ("update " NOM_TABLE_PATIENTS " set sexe = '" + Sexe + "' where PatPrenom = '" + gPrenomPatient + "' and sexe = ''",  DataBase::getInstance()->getDataBase() );
                req ="select idpat from " NOM_TABLE_PATIENTS " where sexe = ''";
                QSqlQuery quer1(req, DataBase::getInstance()->getDataBase() );
                UpMessageBox::Information(this, tr("Il reste ") + QString::number(quer1.size()) + tr(" dossiers pour lesquels le sexe n'est pas précisé"),"");
                AfficheDossier(gidPatient);
            }
            delete msgbox;
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

            QSqlQuery EnregDernierActeQuery (requete, DataBase::getInstance()->getDataBase() );
            DataBase::getInstance()->traiteErreurRequete(EnregDernierActeQuery,requete, "Impossible de trouver le dernier acte du patient pour le contrôler");

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
                    UpSmallButton OKBouton(tr("Consultation gratuite"));
                    UpSmallButton NoBouton(tr("Non"));
                    msgbox.addButton(&NoBouton, UpSmallButton::CANCELBUTTON);
                    msgbox.addButton(&OKBouton, UpSmallButton::STARTBUTTON);
                    msgbox.exec();
                    if (msgbox.clickedButton() != &OKBouton)
                        return false;
                    else
                    {
                        requete = "INSERT INTO " NOM_TABLE_TYPEPAIEMENTACTES " (idActe, TypePaiement) VALUES (" + QString::number(gidActe) + ",'G')";
                        QSqlQuery InsertGratuitQuery (requete, DataBase::getInstance()->getDataBase() );
                        if (!DataBase::getInstance()->traiteErreurRequete(InsertGratuitQuery,requete,""))
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
                if (!RetourSalleDattente(Titre))
                {
                    ui->ActeCotationcomboBox->setFocus();
                    return false;
                }
                else return true;
            }
            if (Titre == tr("Il manque le montant!"))
            {
                if (!RetourSalleDattente(Titre))
                {
                    ui->ActeMontantlineEdit->setFocus();
                    return false;
                }
                else return true;
            }
            if (Titre.contains(tr("il manque les informations de paiement")))
                return RetourSalleDattente(Titre);
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
            FlagMetAjourSalDat();
        }
        else
            return false;
    }

    // le tab dossier est fermé, on vérifie s'il y a du monde en salle d'attente
    QString req = "SELECT Statut, IdPat, PosteExamen FROM " NOM_TABLE_SALLEDATTENTE " WHERE IdUser = '" + QString::number(gDataUser->id()) + "'";
    QSqlQuery SaldatQuery (req, DataBase::getInstance()->getDataBase() );
    DataBase::getInstance()->traiteErreurRequete(SaldatQuery,req,"");
    if (SaldatQuery.first())
    {
     /* 2 possibilités
     * 1. C'est le seul poste connecté pour cet utilisateur
     * 2. cet utilisateur est connecté sur d'autres postes, on peut partir
    */
        req = "select distinct nomposteconnecte from " NOM_TABLE_USERSCONNECTES " where idUser = " + QString::number(gDataUser->id());
        QSqlQuery nbpostesquery(req, DataBase::getInstance()->getDataBase() );
        if (nbpostesquery.size()<2)
            for (int i = 0; i < SaldatQuery.size() ; i++)  // il reste des patients pour cet utilisateur dans le centre
            {
                QString Statut = SaldatQuery.value(0).toString();
                QString blabla = ENATTENTENOUVELEXAMEN;
                if (Statut == ENCOURS
                        || Statut == ARRIVE
                        || Statut.contains(blabla)
                        || Statut == ENCOURSEXAMEN + gDataUser->getLogin()
                        || Statut == RETOURACCUEIL)
                {
                    // il y a du monde en salle d'attente, on refuse la fermeture
                    QSound::play(NOM_ALARME);
                    UpMessageBox msgbox;
                    UpSmallButton OKBouton("OK");
                    UpSmallButton NoBouton(tr("Fermer quand même"));
                    msgbox.setText("Euuhh... " + gDataUser->getLogin() + ", " + tr("vous ne pouvez pas fermer l'application."));
                    msgbox.setInformativeText(tr("Vous avez encore des patients en salle d'attente dont la consultation n'est pas terminée."));
                    msgbox.setIcon(UpMessageBox::Warning);
                    msgbox.addButton(&NoBouton, UpSmallButton::CLOSEBUTTON);
                    msgbox.addButton(&OKBouton, UpSmallButton::OUPSBUTTON);
                    msgbox.exec();
                    if (msgbox.clickedButton() != &NoBouton)
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
    QString LibereVerrouRequete = "DELETE FROM " NOM_TABLE_VERROUCOMPTAACTES " WHERE PosePar = " + QString::number(gDataUser->id());
    QSqlQuery LibereVerrouComptaQuery (LibereVerrouRequete, DataBase::getInstance()->getDataBase() );
    DataBase::getInstance()->traiteErreurRequete(LibereVerrouComptaQuery,LibereVerrouRequete,"");
    // on retire cet utilisateur de la table des utilisateurs connectés
    req = "delete from " NOM_TABLE_USERSCONNECTES " where NomPosteConnecte = '" + QHostInfo::localHostName().left(60) + "'";
    QSqlQuery qer(req, DataBase::getInstance()->getDataBase() );
    DataBase::getInstance()->traiteErreurRequete(qer,req,"");
    FlagMetAjourSalDat();
    if (proc->PosteImportDocs().remove(" - prioritaire")==QHostInfo::localHostName())
        proc->setPosteImportDocs(false);

    req = "update " NOM_TABLE_UTILISATEURS " set datederniereconnexion = '" + QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss")
            + "' where idUser = " + QString::number(gDataUser->id());
    DataBase::getInstance()->traiteErreurRequete(QSqlQuery(req, DataBase::getInstance()->getDataBase()), req, "");

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
    QSqlQuery tooltpquer(req, DataBase::getInstance()->getDataBase() );
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
    gDataUser = DataBase::getInstance()->getUserConnected();
    return true;
}

/*-----------------------------------------------------------------------------------------------------------------
-- Calcule la liste des mots clés du patient  -------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void    Rufus::CalcMotsCles(int idpt)
{
    QString req = "select motcle from " NOM_TABLE_MOTSCLES " where idmotcle in (select idmotcle from " NOM_TABLE_MOTSCLESJOINTURES " where idpat = " + QString::number(idpt) + ")";
    QSqlQuery quer(req, DataBase::getInstance()->getDataBase() );
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
        if (DataBase::getInstance()->getMode() == DataBase::Distant && a==1000)
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

    Remplir_ListePatients_TableView(Filtrerequete,"","");   //ChercheNomFiltre()
    CalcNbDossiers();

    if ((ui->CreerNomlineEdit->text() != "" || ui->CreerPrenomlineEdit->text() != "") && id==0)
    {
        if (gListePatientsModel->rowCount()>0)
        {
            ui->PatientsListeTableView->selectRow(0);
            ui->PatientsListeTableView->scrollTo(gListePatientsModel->item(0)->index(),QAbstractItemView::PositionAtTop);
        }
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
    }
    EnableCreerDossierButton();
}

/*-----------------------------------------------------------------------------------------------------------------
-- Vérifie les verrous d'un dossier avant de l'afficher -----------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void    Rufus::ChoixDossier(int idpat, int idacte)  // appelée depuis la tablist ou la salle d'attente - vérifie qu'un dossier n'est pas verrouillé avant de l'afficher
{
    if (gDataUser->isSecretaire())    // si l'utilisateur est une secrétaire, on propose de mettre le patient en salle d'attente
        InscritEnSalDat(idpat);
    else if (gDataUser->isSoignant())
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
                    " AND (idUserEnCoursExam != " + QString::number(gDataUser->id()) + " OR (idUserEnCoursExam = " + QString::number(gDataUser->id()) + " AND PosteExamen != '" + QHostInfo::localHostName().left(60) +"'))";
            //proc->Edit(Verrourequete);
            QSqlQuery ChercheVerrouQuery (Verrourequete, DataBase::getInstance()->getDataBase() );
            DataBase::getInstance()->traiteErreurRequete(ChercheVerrouQuery,Verrourequete,"");
            if (ChercheVerrouQuery.size() > 0)
            {
                ChercheVerrouQuery.first();
                UpMessageBox::Watch(this,tr("Impossible d'ouvrir ce dossier!"),
                                    tr("Ce patient est") + ChercheVerrouQuery.value(1).toString() + "\n" + tr("sur ") + ChercheVerrouQuery.value(2).toString());
                return;
            }
        }
        AfficheDossier(idpat, idacte);
    }
}

/*-----------------------------------------------------------------------------------------------------------------
-- Créer une consultation ------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void    Rufus::CreerActe(int idPat)
{
    if (ui->Acteframe->isVisible())
        if(!AutorDepartConsult(false)) return;
    QString rempla = (gDataUser->getEnregHonoraires()==3? "1" : "null");
    QString creerrequete =
            "INSERT INTO " NOM_TABLE_ACTES
            " (idPat, idUser, ActeDate, ActeHeure, CreePar, UserComptable, UserParent,SuperViseurRemplacant, NumCentre, idLieu)"
            " VALUES (" +
            QString::number(idPat) + ", " +
            QString::number(gDataUser->getIdUserActeSuperviseur()) + ", "
            "NOW(), "
            "NOW(), " +
            QString::number(gDataUser->id()) + ", " +
            QString::number(gDataUser->getIdUserComptable()) + ", " +
            QString::number(gDataUser->getIdUserParent()) + ", " +
            rempla + ", " +
            QString::number(proc->idCentre()) + ", " +
            QString::number(gDataUser->getSite()->id()) +")";
    //qDebug() << creerrequete;
    QSqlQuery CreerActeQuery (creerrequete, DataBase::getInstance()->getDataBase() );
    if (DataBase::getInstance()->traiteErreurRequete(CreerActeQuery,creerrequete,tr("Impossible de créer cette consultation dans ") + NOM_TABLE_ACTES))
            return ;
    // Récupération de l'idActe créé et affichage du dossier ------------------------------------
    QString maxrequete = "SELECT MAX(idActe) FROM " NOM_TABLE_ACTES
                " WHERE idUser = " + QString::number(gDataUser->getIdUserActeSuperviseur()) + " AND idPat = "+ QString::number(idPat);
    QSqlQuery SelectActeQuery (maxrequete, DataBase::getInstance()->getDataBase() );
    if (DataBase::getInstance()->traiteErreurRequete(SelectActeQuery,maxrequete,tr("Impossible de retrouver l'acte qui vient d'être créé")))
            return ;
    SelectActeQuery.first();
    AfficheActe(SelectActeQuery.value(0).toInt());
    QString req = "SELECT idActe FROM " NOM_TABLE_ACTES " WHERE idPat = " + QString::number(idPat);
    QSqlQuery quer (req, DataBase::getInstance()->getDataBase() );
    if (DataBase::getInstance()->traiteErreurRequete(quer,req,tr("Impossible de compter le nombre d'actes")))
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
        ui->ChercherDepuisListepushButton->setIcon(Icons::icContact());
        gMode = RechercheDDN;
        ui->CreerDDNdateEdit->setFocus();
    }
    else
        OuvrirListe();
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
    PatCreePar  = QString::number(gDataUser->id());
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
        UpSmallButton OKBouton(tr("Je confirme"));
        UpSmallButton NoBouton(tr("Annuler"));
        msgbox.setText("Euuhh... " + gDataUser->getLogin());
        msgbox.setInformativeText(tr("Confirmez vous la date de naissance?") + "\n" + ui->CreerDDNdateEdit->date().toString(tr("d-MMM-yyyy")));
        msgbox.setIcon(UpMessageBox::Warning);
        msgbox.addButton(&NoBouton, UpSmallButton::CANCELBUTTON);
        msgbox.addButton(&OKBouton, UpSmallButton::STARTBUTTON);
        msgbox.exec();
        if (msgbox.clickedButton() != &OKBouton)
            return;
    }

    // 1. On recherche d'abord si le dossier existe
    idPat = LectureMesure("", PatNom, PatPrenom, PatDDN, "", "", "Impossible de rechercher le dossier");
    if (idPat == -1)                                                // il y a eu une erreur pendant la recherche
        return ;
    if (idPat > 0)                                                  // Le dossier existe, on l'affiche
    {
        UpMessageBox::Watch(this, tr("Ce patient est déjà venu!"));
        if( gDataUser->isSoignant() )
            AfficheDossier(gidPatient);
        else
        {
            UpMessageBox msgbox;
            msgbox.setText(tr("Ouverture du dossier de ")  + PatNom + " " + PatPrenom);
            msgbox.setInformativeText(tr("Inscrire le dossier en salle d'attente?"));
            msgbox.setIcon(UpMessageBox::Warning);
            UpSmallButton OKBouton(tr("Inscrire en\nsalle d'attente"));
            UpSmallButton NoBouton(tr("Ne pas inscrire"));
            msgbox.addButton(&NoBouton, UpSmallButton::CANCELBUTTON);
            msgbox.addButton(&OKBouton, UpSmallButton::STARTBUTTON);
            msgbox.exec();
            if (msgbox.clickedButton() == &OKBouton)
                InscritEnSalDat(gidPatient);
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
        UpSmallButton OKBouton(tr("Créer le dossier"));
        UpSmallButton NoBouton(tr("Annuler"));
        msgbox.addButton(&NoBouton, UpSmallButton::CANCELBUTTON);
        msgbox.addButton(&OKBouton, UpSmallButton::STARTBUTTON);
        msgbox.exec();
        if (msgbox.clickedButton() != &OKBouton)
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
        QSqlQuery InsertPatQuery (insrequete, DataBase::getInstance()->getDataBase() );
        if (DataBase::getInstance()->traiteErreurRequete(InsertPatQuery,insrequete,tr("Impossible de créer le dossier")))
            return ;

        // Récupération de l'idPatient créé et affichage du dossier ------------------------------------
        QString recuprequete = "SELECT  idPat, PatNom, PatPrenom FROM " NOM_TABLE_PATIENTS
                " WHERE PatNom = '" + proc->CorrigeApostrophe(PatNom) + "' AND PatPrenom = '" + proc->CorrigeApostrophe(PatPrenom) + "' AND PatDDN = '" + PatDDN + "'";
        QSqlQuery ChercheIdPatientQuery (recuprequete, DataBase::getInstance()->getDataBase() );
        if (DataBase::getInstance()->traiteErreurRequete(ChercheIdPatientQuery,recuprequete,tr("Impossible de sélectionner les enregistrements")))
            return ;

        ChercheIdPatientQuery.first();
        gidPatient      = ChercheIdPatientQuery.value(0).toInt();
        gNomPatient     = ChercheIdPatientQuery.value(1).toString();
        gPrenomPatient  = ChercheIdPatientQuery.value(2).toString();
        QString requete =   "INSERT INTO " NOM_TABLE_DONNEESSOCIALESPATIENTS " (idPat) VALUES ('" + QString::number(gidPatient) + "')";
        QSqlQuery CreeDonneeSocialePatientQuery (requete, DataBase::getInstance()->getDataBase() );
        DataBase::getInstance()->traiteErreurRequete(CreeDonneeSocialePatientQuery,requete,tr("Impossible de créerles données sociales"));

        if (!IdentificationPatient("Creation",gidPatient)) return;
        Remplir_ListePatients_TableView(grequeteListe, gNomPatient, gPrenomPatient);

        // Si le User est un soignant, on crée d'emblée une consultation et on l'affiche
        if( gDataUser->isSoignant() )
        {
            UpMessageBox msgbox;
            msgbox.setText(tr("Dossier ") + gNomPatient + " " + gPrenomPatient + tr(" créé"));
            msgbox.setInformativeText(tr("Ouvrir le dossier ou inscrire le dossier en salle d'attente?"));
            msgbox.setIcon(UpMessageBox::Quest);
            UpSmallButton OKBouton(tr("Inscrire le dossier\nen salle d'attente"));
            UpSmallButton AnnulBouton(tr("Ouvrir le dossier"));
            UpSmallButton NoBouton(tr("Ne rien faire"));
            msgbox.addButton(&NoBouton,UpSmallButton::CLOSEBUTTON);
            msgbox.addButton(&OKBouton, UpSmallButton::NOBUTTON);
            msgbox.addButton(&AnnulBouton, UpSmallButton::NOBUTTON);
            OKBouton.setIcon(Icons::icAttente());
            AnnulBouton.setIcon(Icons::icSortirDossier());
            msgbox.setDefaultButton(&NoBouton);
            //NoBouton->setFocus();
            msgbox.exec();
            if (msgbox.clickedButton() == &OKBouton)
            {
                if (!InscritEnSalDat(gidPatient))
                    RecaleTableView(gidPatient);
            }
            else if (msgbox.clickedButton() == &AnnulBouton)
            {
                CreerActe(gidPatient);
                AfficheDossier(gidPatient);
            }
            else
                RecaleTableView(gidPatient);
        }
        else
            if (!InscritEnSalDat(gidPatient))
                RecaleTableView(gidPatient);
    }
}


// ------------------------------------------------------------------------------------------
// Creer le menu général de l'appliaction
// ------------------------------------------------------------------------------------------

void Rufus::CreerMenu()
{
    QAction *Apropos = new QAction(tr("A propos"));
    menuDossier         = menuBar()->addMenu(tr("Dossier"));
    menuEdition         = menuBar()->addMenu(tr("Edition"));
    menuActe            = menuBar()->addMenu(tr("Acte"));
    menuDocuments       = menuBar()->addMenu(tr("Documents"));
    menuEmettre         = menuDocuments->addMenu(tr("Emettre"));

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
    actionResumeStatut              = new QAction(tr("Voir le statut"));

    actionCreerActe                 = new QAction(tr("Créer"));
    actionSupprimerActe             = new QAction(tr("Supprimer"));

    actionEmettreDocument           = new QAction(tr("Document simple"));
    actionDossierPatient            = new QAction(tr("Dossier patient"));
    actionEnregistrerDocScanner     = new QAction(tr("Enregistrer un document scanné"));
    actionEnregistrerVideo          = new QAction(tr("Enregistrer une video"));
    actionRechercheCourrier         = new QAction(tr("Afficher les courriers à faire"));
    actionCorrespondants            = new QAction(tr("Liste des correspondants"));

    actionQuit                      = new QAction(tr("Quitter"));
    actionQuit                      ->setMenuRole(QAction::PreferencesRole);
    connect (actionQuit,            &QAction::triggered,  [=] {close();});

    menuDossier->addAction(actionCreerDossier);
    menuDossier->addAction(actionOuvrirDossier);
    menuDossier->addSeparator();
    menuDossier->addAction(actionRecopierDossier);
    menuDossier->addSeparator();
    menuDossier->addAction(actionSupprimerDossier);
    menuDossier->addSeparator();
    menuDossier->addAction(actionRechercheParMotCle);
    menuDossier->addAction(actionRechercheParID);

#ifdef Q_OS_MACX
    menuBar()->addAction(Apropos);
    menuBar()->addAction(actionQuit);
#endif

    menuEdition->addAction(tr("Copier"));
    menuEdition->addAction(tr("Couper"));
    menuEdition->addAction(tr("Coller"));
    menuEdition->addSeparator();
    menuEdition->addAction(actionParametres);
    menuEdition->addAction(actionResumeStatut);

    menuActe->addAction(actionCreerActe);
    menuActe->addAction(actionSupprimerActe);

    menuEmettre->addAction(actionEmettreDocument);
    menuEmettre->addAction(actionDossierPatient);

    menuDocuments->addMenu(menuEmettre);
    menuDocuments->addAction(actionEnregistrerDocScanner);
    menuDocuments->addAction(actionEnregistrerVideo);
    menuDocuments->addSeparator();
    menuDocuments->addAction(actionRechercheCourrier);
    menuDocuments->addAction(actionCorrespondants);

    // Les menus --------------------------------------------------------------------------------------------------
    connect (actionCreerDossier,                &QAction::triggered,        this,                   [=] {OuvrirNouveauDossier();});
    connect (actionOuvrirDossier,               &QAction::triggered,        this,                   [=] {OuvrirListe();});
    connect (actionSupprimerDossier,            &QAction::triggered,        this,                   [=] {SupprimerDossier();});
    connect (actionRechercheParMotCle,          &QAction::triggered,        this,                   [=] {RechercheParMotCle();});
    connect (actionRechercheParID,              &QAction::triggered,        this,                   [=] {RechercheParID();});
    connect (actionCreerActe,                   &QAction::triggered,        this,                   [=] {CreerActe(gidPatient);});

    connect (actionParametres,                  &QAction::triggered,        this,                   [=] {OuvrirParametres();});
    connect (actionResumeStatut,                &QAction::triggered,        this,                   [=] {
                                                                                                            if (gResumeStatut =="")
                                                                                                                ResumeStatut();
                                                                                                            proc->Edit(gResumeStatut, tr("Information statut"), false, true );
                                                                                                        });
    connect (actionSupprimerActe,               &QAction::triggered,        this,                   [=] {SupprimerActe();});
    // Documents
    connect (actionEmettreDocument,             &QAction::triggered,        this,                   [=] {OuvrirDocuments();});
    connect (actionDossierPatient,              &QAction::triggered,        this,                   [=] {ImprimeDossier();});
    connect (actionCorrespondants,              &QAction::triggered,        this,                   [=] {ListeCorrespondants();});
    connect (actionEnregistrerDocScanner,       &QAction::triggered,        this,                   [=] {EnregistreDocScanner();});
    connect (actionEnregistrerVideo,            &QAction::triggered,        this,                   [=] {EnregistreVideo();});
    connect (actionRechercheCourrier,           &QAction::triggered,        this,                   [=] {AfficheCourriersAFaire();});
    // Comptabilité

    connect (menuActe,                          &QMenu::aboutToShow,        this,                   [=] {AfficheMenu(menuActe);});
    connect (menuEdition,                       &QMenu::aboutToShow,        this,                   [=] {AfficheMenu(menuEdition);});
    connect (menuDocuments,                     &QMenu::aboutToShow,        this,                   [=] {AfficheMenu(menuDocuments);});
    connect (menuDossier,                       &QMenu::aboutToShow,        this,                   [=] {AfficheMenu(menuDossier);});

    menuComptabilite                = menuBar()->addMenu(tr("Comptabilité"));

    actionPaiementDirect            = new QAction(tr("Gestion des paiements directs"));
    actionPaiementDirect2           = new QAction(tr("Gestion des paiements directs 2"));
    actionPaiementTiers             = new QAction(tr("Gestion des tiers payants"));
    actionBilanRecettes             = new QAction(tr("Bilan des recettes"));
    actionRecettesSpeciales         = new QAction(tr("Enregistrement des recettes spéciales"));
    actionJournalDepenses           = new QAction(tr("Journal des dépenses"));
    actionGestionComptesBancaires   = new QAction(tr("Gestion des comptes bancaires"));
    actionRemiseCheques             = new QAction(tr("Effectuer une remise de chèques"));
    actionImpayes                   = new QAction(tr("Impayés"));

    menuComptabilite->addAction(actionPaiementDirect);
    //menuComptabilite->addAction(actionPaiementDirect2);
    menuComptabilite->addAction(actionPaiementTiers);
    menuComptabilite->addAction(actionBilanRecettes);
    menuComptabilite->addAction(actionRecettesSpeciales);
    menuComptabilite->addSeparator();
    menuComptabilite->addAction(actionJournalDepenses);
    menuComptabilite->addSeparator();
    menuComptabilite->addAction(actionGestionComptesBancaires);
    menuComptabilite->addAction(actionRemiseCheques);
    menuComptabilite->addAction(actionImpayes);

    connect (actionGestionComptesBancaires,     &QAction::triggered,        this,                   [=] {GestionComptes();});
    connect (actionPaiementDirect,              &QAction::triggered,        this,                   [=] {AppelPaiementDirect();});
    connect (actionPaiementDirect2,             &QAction::triggered,        this,                   [=] {AppelPaiementDirect2();});
    connect (actionPaiementTiers,               &QAction::triggered,        this,                   [=] {AppelPaiementTiers();});
    connect (actionRecettesSpeciales,           &QAction::triggered,        this,                   [=] {RecettesSpeciales();});
    connect (actionBilanRecettes,               &QAction::triggered,        this,                   [=] {BilanRecettes();});
    connect (actionJournalDepenses,             &QAction::triggered,        this,                   [=] {OuvrirJournalDepenses();});
    connect (actionRemiseCheques,               &QAction::triggered,        this,                   [=] {RemiseCheques();});

    connect (menuComptabilite,                  &QMenu::aboutToShow,        this,                   [=] {AfficheMenu(menuComptabilite);});

#ifdef Q_OS_LINUX
    menuAide            = menuBar()->addMenu(tr("Aide"));
    menuAide->addAction(Apropos);
    menuDossier->addAction(actionQuit);
#endif
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
    Dlg_IdentCorresp        = new dlg_identificationcorresp("Creation", onlydoctors, 0, proc);
    Dlg_IdentCorresp->ui->NomlineEdit->setText(Nom);
    Dlg_IdentCorresp->ui->PrenomlineEdit->setFocus();
    if (Cor == "MG")
        Dlg_IdentCorresp->ui->MGradioButton->setChecked(true);
    if (Dlg_IdentCorresp->exec()>0)
    {
        FlagMetAjourMG();
        ReconstruitCombosCorresp();             // par une modif introduite par la fiche identcorrespondant
        idcor = Dlg_IdentCorresp->gidCor;
    }
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
    QSqlQuery quer(req, DataBase::getInstance()->getDataBase() );
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

    UpSmallButton OKBouton     (tr("Garder le dossier\nen salle d'attente"));
    UpSmallButton AnnulBouton  (tr("Annuler"));
    UpSmallButton NoBouton     (tr("Fermer\nle dossier"));

    msgbox.addButton(&AnnulBouton,   UpSmallButton::CANCELBUTTON);
    msgbox.addButton(&OKBouton,      UpSmallButton::STARTBUTTON);
    msgbox.addButton(&NoBouton,      UpSmallButton::CLOSEBUTTON);
    msgbox.exec();
    if (msgbox.clickedButton() == &NoBouton)                                                        // Fermer le dossier
    {
        QString requete =   "DELETE FROM " NOM_TABLE_SALLEDATTENTE
                " WHERE idPat = '" + QString::number(gidPatient) + "'";
        QSqlQuery supprimePatSalDatQuery (requete, DataBase::getInstance()->getDataBase() );
        DataBase::getInstance()->traiteErreurRequete(supprimePatSalDatQuery,requete, tr("Impossible de supprimer ce patient de la salle d'attente!"));
    }
    else if (msgbox.clickedButton() == &OKBouton)                                                   // Garder le dossier en salle d'attente
    {
        QString Message(""), Motif(""), idUser ("");
        QString req = "select Motif, Message from " NOM_TABLE_SALLEDATTENTE " where idPat = " + QString::number(gidPatient);
        QSqlQuery quer(req, DataBase::getInstance()->getDataBase() );
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
            QSqlQuery SalDatQuery(saldatrequete, DataBase::getInstance()->getDataBase() );
            DataBase::getInstance()->traiteErreurRequete(SalDatQuery,saldatrequete,tr("Impossible de trouver le dossier dans la salle d'attente!"));
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
            QSqlQuery ModifSalDatQuery(saldatrequete, DataBase::getInstance()->getDataBase() );
            DataBase::getInstance()->traiteErreurRequete(ModifSalDatQuery,saldatrequete,"");
        }
        else
            a = InscritEnSalDat(gidPatient);
    }
    else a = false;                                                                                 // Annuler et revenir au dossier
    if (a) gidPatient = 0;
    FlagMetAjourSalDat();
    return a;
}

/*-----------------------------------------------------------------------------------------------------------------
-- Flag pour signifier aux utilisateurs de mettre à jour leur médecin traitant ------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void Rufus::FlagMetAjourMG()
{
    proc->MAJflagMG();
    // on resynchronise l'affichage du combobox au besoin
    if (ui->tabWidget->indexOf(ui->tabDossier) > -1)
        OKModifierTerrain();
}

/*-----------------------------------------------------------------------------------------------------------------
-- Flag pour signifier aux utilisateurs de mettre à jour la salle d'attente ------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void Rufus::FlagMetAjourSalDat()
{
    proc->MAJTcpMsgEtFlagSalDat();
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
        QSqlQuery quer (req, DataBase::getInstance()->getDataBase() );
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
            QSqlQuery ListMGQuery(req, DataBase::getInstance()->getDataBase() );
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
    if (Dlg_IdentPatient->ReconstruireListMG)
    {
        ReconstruitCombosCorresp();             // par une modif introduite par la fiche identpatient
        FlagMetAjourMG();
    }
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
            QString req;
            QString patreq =    "UPDATE " NOM_TABLE_PATIENTS
                                " SET PatNom = '" + proc->CorrigeApostrophe(NomPat) +
                                "', PatPrenom = '" + proc->CorrigeApostrophe(PrenomPat) +
                                "', PatDDN = '" + DDNPat;
            if (gSexePat != "")
                patreq +=       "', Sexe = '" + gSexePat;
            patreq +=           "' WHERE idPat = " + QString::number(idPat);
            QSqlQuery PatQuery (patreq, DataBase::getInstance()->getDataBase() );
            DataBase::getInstance()->traiteErreurRequete(PatQuery,patreq,tr("Impossible d'écrire dans la table PATIENTS"));

            //ON VÉRIFIE QU'IL Y A BIEN UN ENREGISTREMENT POUR CE PATIENT DANS LA TABLE DONNEESSOCIALES SINON ON LE CRÈE
            req = "select idpat from " NOM_TABLE_DONNEESSOCIALESPATIENTS " where idpat = " + QString::number(idPat);
            QSqlQuery qer(req, DataBase::getInstance()->getDataBase() );
            if (qer.size() == 0)
            {
                req =   "INSERT INTO " NOM_TABLE_DONNEESSOCIALESPATIENTS " (idPat) VALUES ('" + QString::number(idPat) + "')";
                QSqlQuery (req, DataBase::getInstance()->getDataBase() );
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

            QSqlQuery MAJSocialQuery (requete, DataBase::getInstance()->getDataBase() );
            DataBase::getInstance()->traiteErreurRequete(MAJSocialQuery,requete,tr("Impossible d'écrire dans la table des données sociales"));

            //ON VÉRIFIE QU'IL Y A BIEN UN ENREGISTREMENT POUR CE PATIENT DANS LA TABLE RENSEIGNEMENTSMEDICAUXPATIENTS SINON ON LE CRÈE
            int e = Dlg_IdentPatient->ui->MGupComboBox->currentData().toInt();
            req = "select idpat from " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS " where idpat = " + QString::number(idPat);
            QSqlQuery quer(req, DataBase::getInstance()->getDataBase() );
            if (e>-1)
            {
                if (quer.size() == 0)
                    req =   "INSERT INTO " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS
                            " (idPat, IDCORMEDMG) VALUES (" + QString::number(idPat) + "," + QString::number(e) + ")";
                else
                    req = "update " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS " set idcormedmg = " + QString::number(e)
                            + " where idpat = " + QString::number(idPat);
                QSqlQuery (req, DataBase::getInstance()->getDataBase() );
            }
            else if (quer.size() >0)
                QSqlQuery ("update " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS " set idcormedmg = null where idpat = " + QString::number(idPat), DataBase::getInstance()->getDataBase() );

            //          Mise à jour de l'affichage si le dossier modifié est le dossier en cours
            if (idPat == gidPatient)
            {
                gNomPatient     = NomPat;
                gPrenomPatient  = PrenomPat;
                gDDNPatient     = Dlg_IdentPatient->ui->DDNdateEdit->date();
                gCMUPatient     = Dlg_IdentPatient->ui->CMUcheckBox->isChecked();
                QString html, img, Age;
                QMap<QString,QVariant>  AgeTotal;
                AgeTotal        = Item::CalculAge(gDDNPatient, gSexePat);
                img             = AgeTotal["icone"].toString(); //TODO : User icone
                Age             = AgeTotal["toString"].toString();
                QIcon icon      = Icons::getIconAge(img);

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
                if (Dlg_IdentPatient->ui->NNIlineEdit->text().toInt() > 0)
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
                    OKModifierTerrain();
                }
                QMap<QString,QVariant>  NewAge = Item::CalculAge(gDDNPatient, ui->ActeDatedateEdit->date());
                ui->AgelineEdit->setText(NewAge["toString"].toString());
            }
            FlagMetAjourSalDat();
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
                QSqlQuery MAJPatientQuery (requete, DataBase::getInstance()->getDataBase() );
                DataBase::getInstance()->traiteErreurRequete(MAJPatientQuery,requete,"Impossible d'écrire dans la table des patients");
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
            QSqlQuery MAJSocialQuery (requete, DataBase::getInstance()->getDataBase() );
            DataBase::getInstance()->traiteErreurRequete(MAJSocialQuery,requete, tr("Impossible d'écrire dans la table des données sociales"));
            //2 - Mise à jour de medecin traitant
            int e = Dlg_IdentPatient->ui->MGupComboBox->currentData().toInt();
            ui->MGupComboBox->setCurrentIndex(ui->MGupComboBox->findData(e));
            requete =   "INSERT INTO " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS
                    " (idPat, idCorMedMG) VALUES(" + QString::number(idPat) + "," + QString::number(e) + ")";
            QSqlQuery MAJMGQuery (requete, DataBase::getInstance()->getDataBase() );
            DataBase::getInstance()->traiteErreurRequete(MAJMGQuery,requete, tr("Impossible d'enregistrer le médecin traitant"));

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
            QSqlQuery PatQuery (patreq, DataBase::getInstance()->getDataBase() );
            DataBase::getInstance()->traiteErreurRequete(PatQuery,patreq, tr("Impossible d'écrire dans la table des données sociales"));

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

            QSqlQuery MAJSocialQuery (requete, DataBase::getInstance()->getDataBase() );
            DataBase::getInstance()->traiteErreurRequete(MAJSocialQuery,requete, tr("Impossible d'écrire dans la table des données sociales"));

            // on met à jour les atcdts familiaux
            QString req = "select RMPAtcdtsFamiliaux from " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS " where idPat = " + QString::number(gidARecopier);
            QSqlQuery quer(req, DataBase::getInstance()->getDataBase() );
            if (quer.size() > 0)
            {
                quer.first();
                QString atcdts = quer.value(0).toString();
                QString insreq = "insert into " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS " (idPat, RMPAtcdtsFamiliaux) VALUES ('" +
                        QString::number(idPat) + "', '" + atcdts + "')";
                QSqlQuery (insreq, DataBase::getInstance()->getDataBase() );
            }
            // Mise à jour du medecin traitant
            int e = Dlg_IdentPatient->ui->MGupComboBox->currentData().toInt();
             req = "select idpat from " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS " where idpat = " + QString::number(idPat);
            QSqlQuery quer2(req, DataBase::getInstance()->getDataBase() );
            if (quer2.size() == 0)
                req = "INSERT INTO " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS " (idPat, idCorMedMG)"
                      " VALUES (" + QString::number(idPat) + "," + QString::number(e) + ")";
            else
                req = "update " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS " set idcormedmg = " + QString::number(e) + " where idpat = " + QString::number(idPat);
            QSqlQuery MAJMGQuery (req, DataBase::getInstance()->getDataBase() );
            DataBase::getInstance()->traiteErreurRequete(MAJMGQuery,req, tr("Impossible d'enregistrer le médecin traitant"));

            gidPatient = idPat;
            // Si le User est un soignant, on crée d'emblée une consultation et on l'affiche
            if( gDataUser->isSoignant() )
            {
                UpMessageBox msgbox;
                msgbox.setText(tr("Dossier ") + NomPat + " " + PrenomPat + " créé");
                msgbox.setInformativeText(tr("Ouvrir le dossier ou inscrire le dossier en salle d'attente?"));
                msgbox.setIcon(UpMessageBox::Quest);

                UpSmallButton OKBouton      (tr("Inscrire le dossier\nen salle d'attente"));
                UpSmallButton AnnulBouton   (tr("Ouvrir\n le dossier"));
                UpSmallButton NoBouton      (tr("Ne rien faire"));
                msgbox.addButton(&NoBouton,UpSmallButton::CANCELBUTTON);
                msgbox.addButton(&OKBouton, UpSmallButton::STARTBUTTON);
                msgbox.addButton(&AnnulBouton, UpSmallButton::CLOSEBUTTON);
                AnnulBouton.setIcon(Icons::icAttente());
                OKBouton.setIcon(Icons::icSortirDossier());
                msgbox.setDefaultButton(&NoBouton);
                msgbox.exec();
                if (msgbox.clickedButton() == &OKBouton)
                {
                    if (!InscritEnSalDat(gidPatient))
                        RecaleTableView(gidPatient);
                }
                else if (msgbox.clickedButton() == &AnnulBouton)
                {
                    CreerActe(gidPatient);
                    AfficheDossier(gidPatient);
                }
                else
                    RecaleTableView(gidPatient);
             }
            else
                InscritEnSalDat(gidPatient);
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
                                 bool Prescription, bool ALD, bool AvecPrevisu, bool AvecDupli, bool AvecChoixImprimante, bool Administratif)
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

        QSqlQuery query = QSqlQuery( DataBase::getInstance()->getDataBase() );

        // on doit passer par les bindvalue pour incorporer le bytearray dans la requête
        query.prepare("insert into " NOM_TABLE_IMPRESSIONS " (idUser, idpat, TypeDoc, SousTypeDoc, Titre, TextEntete, TextCorps, TextOrigine,"
                                                           " TextPied, Dateimpression, UserEmetteur, ALD, EmisRecu, FormatDoc, idLieu, Importance)"
                                                           " values"
                                                           "(:iduser, :idpat, :typeDoc, :soustypedoc, :titre, :textEntete, :textCorps,"
                                                           " :textOrigine, :textPied, :dateimpression, :useremetteur, :ald, :emisrecu, :formatdoc, :idlieu, :importance)");
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
        query.bindValue(":useremetteur", QString::number(gDataUser->id()));
        QVariant ALD100 = QVariant(QVariant::String);
        if (ALD) ALD100 = "1";
        query.bindValue(":ald", ALD100);
        query.bindValue(":emisrecu", "0");
        query.bindValue(":formatdoc", (Prescription? PRESCRIPTION : (Administratif? COURRIERADMINISTRATIF : COURRIER)));
        query.bindValue(":idlieu", QString::number(gDataUser->getSite()->id()));
        query.bindValue(":importance", (Administratif? "0" : "1"));
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
void Rufus::InitWidgets()
{
    setFixedWidth(LARGEURLISTE);
    ui->tabWidget->setGeometry(5,10,-10,920);

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

    AutresCorresp1LineEdit->setStyleSheet(
    "UpLineEdit {background-color:white; border-style: none;}"
    "UpLineEdit:focus {border-style:none;}");
    AutresCorresp1LineEdit->setMaxLength(90);
    ui->AutresCorresp1upComboBox->setLineEdit(AutresCorresp1LineEdit);

    AutresCorresp2LineEdit->setStyleSheet(
    "UpLineEdit {background-color:white; border-style: none;}"
    "UpLineEdit:focus {border-style:none;}");
    AutresCorresp2LineEdit->setMaxLength(90);
    ui->AutresCorresp2upComboBox->setLineEdit(AutresCorresp2LineEdit);

    ui->ActeCotationcomboBox->lineEdit()->setStyleSheet(
    "QLineEdit {background-color:white; border-style: none;}"
    "QLineEdit:focus {border-style:none;}");
    ui->ActeCotationcomboBox->lineEdit()->setMaxLength(20);
    //ui->ActeCotationcomboBox->lineEdit()->setValidator(new QRegExpValidator(Utils::rgx_cotation,this));
    ui->ActeCotationcomboBox->lineEdit()->setFont(ui->ActeMontantlineEdit->font());
    ui->ActeCotationcomboBox->setFont(ui->ActeMontantlineEdit->font());

    QDoubleValidator *val = new QDoubleValidator(this);
    val->setDecimals(2);
    ui->ActeMontantlineEdit->setValidator(val);
    ui->PayelineEdit->setValidator(val);
    ui->TabaclineEdit->setValidator(new QRegExpValidator(Utils::rgx_tabac,this));
    MGlineEdit->setValidator(new QRegExpValidator(Utils::rgx_rx,this));
    AutresCorresp1LineEdit->setValidator(new QRegExpValidator(Utils::rgx_rx,this));
    AutresCorresp2LineEdit->setValidator(new QRegExpValidator(Utils::rgx_rx,this));
    ui->tabWidget->setTabText(0,tr("Liste des patients"));

    ui->ActeMontantlineEdit->setAlignment(Qt::AlignRight);
    ui->PayelineEdit->setAlignment(Qt::AlignRight);
    ui->CreerDDNdateEdit->setDate(gdateParDefaut);
    ui->ActeDatedateEdit->setMaximumDate(QDate::currentDate());
    ui->CreerDossierframe->setGeometry(10,170,356,170);
    ui->tabWidget->setIconSize(QSize(25,25));
    for (int abc = 1; abc < ui->tabWidget->count(); abc++)
        ui->tabWidget->removeTab(abc);

    ui->VitaleupPushButton->setIconSize(QSize(120,100));

    ui->SalDatlabel     ->setPixmap(Icons::pxSalleAttente().scaled(QSize(60,60), Qt::KeepAspectRatio, Qt::SmoothTransformation)); //WARNING : icon scaled : pxSalleAttente 60,60
    ui->Bureauxlabel    ->setPixmap(Icons::pxAVTest().scaled(QSize(100,100), Qt::KeepAspectRatio, Qt::SmoothTransformation)); //WARNING : icon scaled : pxAVTest 100,100
    ui->Accueillabel    ->setPixmap(Icons::pxReception().scaled(QSize(70,70), Qt::KeepAspectRatio, Qt::SmoothTransformation)); //WARNING : icon scaled : pxReception 70,70
    ui->PatientsVuslabel->setPixmap(Icons::pxListe().scaled(QSize(60,60), Qt::KeepAspectRatio, Qt::SmoothTransformation)); //WARNING : icon scaled : pxListe 60,60
    ui->PatientsVusupLabel->setPixmap(Icons::pxListe().scaled(QSize(60,60), Qt::KeepAspectRatio, Qt::SmoothTransformation)); //WARNING : icon scaled : pxListe 60,60
    ui->PatientsVusFlecheupLabel ->setPixmap(Icons::pxDown().scaled(QSize(30,30), Qt::KeepAspectRatio, Qt::SmoothTransformation));

    ui->CreerDDNdateEdit->setDateRange(QDate::currentDate().addYears(-105),QDate::currentDate());

    gDirSauv        = QDir::homePath() + "/dumps";

    QMenu *trayIconMenu;
    trayIconMenu = new QMenu();

    QAction *pAction_VoirMessages = trayIconMenu->addAction(tr("Voir les messages"));
    connect (pAction_VoirMessages, &QAction::triggered,    [=] {AfficheMessages();});

    gMessageIcon = new QSystemTrayIcon(this);
    gMessageIcon->setContextMenu(trayIconMenu);
    gMessageIcon->setIcon(Icons::icPostit());
    connect(gMessageIcon,   &QSystemTrayIcon::messageClicked,   [=] {AfficheMessages();});

    gMsgRepons = new QDialog();
    gMsgDialog = new QDialog();


    ui->CCAMlinklabel->setText("<a href=\"" LIEN_CCAM "\">CCAM...</a>");

    QHBoxLayout *hlay = new QHBoxLayout();
    gSalDatTab  = new QTabBar;
    gSalDatTab  ->setExpanding(false);
    hlay        ->insertWidget(0,gSalDatTab);
    hlay        ->addSpacerItem(new QSpacerItem(5,5,QSizePolicy::Expanding,QSizePolicy::Fixed));
    hlay        ->setContentsMargins(0,0,0,0);
    hlay        ->setSpacing(0);
    connect (gSalDatTab,    &QTabBar::currentChanged,   [=] {FiltreSalleDAttente();});
    ui->SalDatLayout->insertLayout(1,hlay);
    ui->SalDatWidget->setVisible(false);

    QHBoxLayout *halay = new QHBoxLayout();
    gAccueilTab  = new QTabBar;
    gAccueilTab  ->setExpanding(false);
    halay        ->insertWidget(0,gAccueilTab);
    halay        ->addSpacerItem(new QSpacerItem(5,5,QSizePolicy::Expanding,QSizePolicy::Fixed));
    halay        ->setContentsMargins(0,0,0,0);
    halay        ->setSpacing(0);
    connect (gAccueilTab,    &QTabBar::currentChanged,   [=] {FiltreAccueil(gAccueilTab->currentIndex());});
    ui->AccueilLayout->insertLayout(1,halay);
    ui->AccueilWidget->setVisible(false);

    TerraintreeWidget         = new QTreeWidget(ui->AtcdstMedxframe);
    ModifTerrainupSmallButton = new UpSmallButton(ui->AtcdstMedxframe);
    ModifTerrainupSmallButton ->setUpButtonStyle(UpSmallButton::EDITBUTTON);
    ModifTerrainupSmallButton ->setGeometry(330,640,30,30);
    ModifTerrainupSmallButton ->setIconSize(QSize(30,30));
    MasquePatientsVusWidget();

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
    bool a = (gDataUser->isLiberal() || gDataUser->isSecretaire());
    actionPaiementTiers             ->setVisible(a || (gDataUser->isSalarie() && !gDataUser->isAssistant()));
    actionPaiementDirect            ->setVisible(a || (gDataUser->isSalarie() && !gDataUser->isAssistant()) || gDataUser->isRemplacant());
    actionPaiementDirect2           ->setVisible(a || (gDataUser->isSalarie() && !gDataUser->isAssistant()) || gDataUser->isRemplacant());
    actionBilanRecettes             ->setVisible(a);
    actionRecettesSpeciales         ->setVisible(gDataUser->isComptable());
    actionJournalDepenses           ->setVisible(a);
    actionGestionComptesBancaires   ->setVisible(gDataUser->isComptable());
    actionRemiseCheques             ->setVisible(a);
    actionImpayes                   ->setVisible(a);
    menuComptabilite                ->setVisible(a || (gDataUser->isSalarie() && !gDataUser->isAssistant()) || gDataUser->isRemplacant());
    actionEnregistrerVideo          ->setVisible(DataBase::getInstance()->getMode() != DataBase::Distant);
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
    gdateParDefaut              = QDate::fromString("2000-01-01", "yyyy-MM-dd");
    gAffichTotalMessages        = true;
    gListeSuperviseursModel     = new QStandardItemModel();

    MGlineEdit                  = new UpLineEdit();
    AutresCorresp1LineEdit      = new UpLineEdit();
    AutresCorresp2LineEdit      = new UpLineEdit();
    ImportDocsExtThread         = Q_NULLPTR;
    gUserDateDernierMessage     = QDateTime();

    proc->CouleurTitres         = "blue";

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

    QSqlQuery LectureMesureQuery (requete, DataBase::getInstance()->getDataBase() );
    if (DataBase::getInstance()->traiteErreurRequete(LectureMesureQuery,requete, MessageErreur))
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
    if (cbox == Q_NULLPTR) return;
    UpLineEdit *Upline = dynamic_cast<UpLineEdit*>(cbox->lineEdit());
    if (Upline == Q_NULLPTR) return;
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
            msgbox.setText("Euuhh... " + gDataUser->getLogin());
            msgbox.setInformativeText(tr("Correspondant inconnu! Souhaitez-vous l'enregistrer?"));
            msgbox.setIcon(UpMessageBox::Warning);
            UpSmallButton OKBouton(tr("Enregistrer"));
            UpSmallButton NoBouton(tr("Annuler"));
            msgbox.addButton(&NoBouton, UpSmallButton::CANCELBUTTON);
            msgbox.addButton(&OKBouton, UpSmallButton::STARTBUTTON);
            msgbox.exec();
            if (msgbox.clickedButton() == &OKBouton)
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
                    QSqlQuery quer(req, DataBase::getInstance()->getDataBase() );
                    if (quer.size() == 0)
                        req =   "INSERT INTO " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS
                                    " (idPat, " + cbox->getChampCorrespondant() + ") VALUES (" + QString::number(gidPatient) + "," + QString::number(idcor) + ")";
                    else
                        req = "update " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS " set " + Upline->getChampCorrespondant() + " = " + QString::number(idcor)
                            + " where idpat = " + QString::number(gidPatient);
                    QSqlQuery (req, DataBase::getInstance()->getDataBase() );
                    ReconstruitCombosCorresp();             // par une modif introduite par la fiche identcorrespondant
                    FlagMetAjourMG();
                    cbox->setCurrentIndex(cbox->findData(idcor));
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
            msgbox.close();
        }
    }
    else
    {
        req = "update " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS " set " + Upline->getChampCorrespondant() + " = null where idpat = " + QString::number(gidPatient);
        QSqlQuery (req, DataBase::getInstance()->getDataBase() );
        cbox->setCurrentIndex(-1);
        OKModifierTerrain();
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
    QSqlQuery NavigationConsultQuery (requete, DataBase::getInstance()->getDataBase() );
    if (DataBase::getInstance()->traiteErreurRequete(NavigationConsultQuery,requete, tr("Impossible de retrouver les consultations de ce patient!")))
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

/*-----------------------------------------------------------------------------------------------------------------
-- Visualiser la fiche dlg_actesprecedents ------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void    Rufus::OuvrirActesPrecedents(int idActeEnCours)
{
    Dlg_ActesPrecs      = new dlg_actesprecedents(gidPatient, idActeEnCours, proc, true, this);
    Dlg_ActesPrecs->setWindowTitle(tr("Consultations précédentes de ") + gNomPatient + " " + gPrenomPatient);
    Dlg_ActesPrecs->show();
    Dlg_ActesPrecs->setWindowIcon(Icons::icLoupe());
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
        Dlg_Docs    = new dlg_documents(gidPatient, nom, prenom, proc);
    }
    else
    {
        QString autrerequete = "select PatNom, PatPrenom from " NOM_TABLE_PATIENTS " where idPat = " + QString::number(gdossierAOuvrir);
        QSqlQuery autrequery (autrerequete,  DataBase::getInstance()->getDataBase() );
        if (DataBase::getInstance()->traiteErreurRequete(autrequery,autrerequete,""))
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
    Dlg_Docs->setWindowIcon(Icons::icLoupe());
    bool aa = true;
    if (Dlg_Docs->exec() > 0)
    {
        int idUserEntete = Dlg_Docs->gidUserEntete;
        User *userEntete = proc->setDataOtherUser(idUserEntete);
        if (userEntete == nullptr)
            return;

        QString     Entete;
        QDate DateDoc           = Dlg_Docs->ui->dateEdit->date();
        //création de l'entête
        QMap<QString,QString> EnteteMap = proc->ImpressionEntete(DateDoc, userEntete);

        bool ALD;
        QString imprimante = "";
        for (int k = 0; k < Dlg_Docs->TextDocumentsAImprimerList.size(); k++)
        {
            /* On dispose de 6 QStringList qui décrivent ce q'on doit imprimer pour chaque itération
             * TitreDocumentAImprimerList       -> le titre qui sera inséré dans la fiche docsexternes et dans la conclusion
             * TextDocumentAImprimerList        -> le texte du document
             * prescriptionAImprimerList        -> précise si le document est une prescription - le formatage n'est pas le même
             * DupliAImprimerList               -> la nécessité ou non d'imprimer un dupli
             * AdministratifAImprimerList       -> le document est un document purement administratif
            */
            ALD                     = (Dlg_Docs->ui->ALDcheckBox->checkState() == Qt::Checked) && (Dlg_Docs->PrescriptionAImprimerList.at(k) == "1");
            bool Prescription       = (Dlg_Docs->PrescriptionAImprimerList.at(k) == "1");
            bool AvecDupli          = (Dlg_Docs->DupliAImprimerList.at(k) == "1");
            bool AvecChoixImprimante= (k == 0);                             // s'il y a plusieurs documents à imprimer on détermine l'imprimante pour le premier et on garde ce choix pour les autres
            bool AvecPrevisu        = proc->ApercuAvantImpression();
            bool Administratif      = (Dlg_Docs->AdministratifAImprimerList.at(k) != "1");
            QString Titre           = Dlg_Docs->TitreDocumentAImprimerList.at(k);
            QString TxtDocument     = Dlg_Docs->TextDocumentsAImprimerList.at(k);
            Entete = (ALD? EnteteMap.value("ALD") : EnteteMap.value("Norm"));
            if (Entete == "") return;
            Entete.replace("{{TITRE1}}"        , "");
            Entete.replace("{{TITRE}}"         , "");
            Entete.replace("{{DDN}}"           , "");
            proc                    ->setNomImprimante(imprimante);
            aa                      = Imprimer_Document(QString::number(idUserEntete), Titre, Entete, TxtDocument, DateDoc, nom, prenom, Prescription, ALD, AvecPrevisu, AvecDupli, AvecChoixImprimante, Administratif);
            if (!aa)
                break;
            imprimante = proc->getNomImprimante();
        }
    }
    delete Dlg_Docs;
    if (aa && AffichDocsExternes)
        MAJDocsExternes();              // depuis dlg_documents
}

/*-----------------------------------------------------------------------------------------------------------------
-- Ouvrir la liste des patients -----------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void    Rufus::OuvrirListe()
{
    if (ui->tabWidget->currentIndex() == ui->tabWidget->indexOf(ui->tabDossier))
        ui->AtcdtsPersostextEdit->setFocus();
    ui->CreerNomlineEdit->clear();
    ui->CreerPrenomlineEdit->clear();
    ui->CreerNomlineEdit->setFocus();
    if (gMode == Liste && ui->tabWidget->currentIndex() == ui->tabWidget->indexOf(ui->tabList))
        return;
    ui->tabWidget->setCurrentIndex(ui->tabWidget->indexOf(ui->tabList));
    ui->CreerNomlineEdit->clear();
    ui->CreerPrenomlineEdit->clear();
    ui->ChercherDepuisListepushButton->setText(tr("Chercher avec la\ndate de naissance"));
    ui->ChercherDepuisListepushButton->setIcon(Icons::icDate());
    ui->ChercherDepuisListepushButton->setVisible(true);
    ui->CreerNomlineEdit->setVisible(true);
    ui->CreerPrenomlineEdit->setVisible(true);
    ui->CreerDDNdateEdit->setVisible(false);
    ui->Nomlabel->setVisible(true);
    ui->Prenomlabel->setVisible(true);
    ui->DDNlabel->setVisible(false);

    ui->CreerDossierpushButton->setIcon(Icons::icSortirDossier());
    ui->CreerDossierpushButton->setText(tr("Ouvrir\nle dossier"));
    ui->LListepushButton->setEnabled(false);
    ui->LNouvDossierpushButton->setEnabled(true);
    ui->LRecopierpushButton->setEnabled(ui->PatientsListeTableView->model()->rowCount() > 0);
    ui->CreerNomlineEdit->setValidator(new QRegExpValidator(Utils::rgx_recherche,this));
    ui->CreerPrenomlineEdit->setValidator(new QRegExpValidator(Utils::rgx_recherche,this));

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
    else if (gListePatientsModel->rowCount() > 0)
    {
        ui->PatientsListeTableView->selectRow(0);
        ui->PatientsListeTableView->scrollTo(gListePatientsModel->item(0)->index(),QAbstractItemView::PositionAtTop);
    }
    CalcNbDossiers();
    ui->CreerNomlineEdit->setValidator(new QRegExpValidator(Utils::rgx_recherche,this));
    ui->CreerPrenomlineEdit->setValidator(new QRegExpValidator(Utils::rgx_recherche,this));
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
    ui->CreerNomlineEdit->setValidator(new QRegExpValidator(Utils::rgx_rx,this));
    ui->CreerPrenomlineEdit->setValidator(new QRegExpValidator(Utils::rgx_rx,this));

    ui->CreerNomlineEdit->setFocus();
    ui->CreerDossierpushButton->setIcon(Icons::icOK());
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
    QSqlQuery quer(req, DataBase::getInstance()->getDataBase() );
    if (DataBase::getInstance()->traiteErreurRequete(quer,req, tr("Impossible de retrouver le dossier d'origine"))) return;
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
    QString champ = (gDataUser->isOPTAM()? "montantoptam" : "montantnonoptam");
    // il faut d'abord reconstruire la table des cotations
    ui->ActeCotationcomboBox->clear();
    req = "(SELECT TypeActe, " + champ + ", montantpratique, nom FROM rufus.cotations cot left join rufus.ccam cc on cot.typeacte= cc.codeccam\n"
    "where idUser = " + QString::number(gDataUser->getIdUserParent()) + " and typeacte in (select codeccam from rufus.ccam)\n"
    "order by typeacte)";
    QSqlQuery ListCotationsQuery (req, DataBase::getInstance()->getDataBase() );
    DataBase::getInstance()->traiteErreurRequete(ListCotationsQuery,req,"");
    for (int i = 0; i < ListCotationsQuery.size(); i++)
    {
        ListCotationsQuery.seek(i);
        QStringList list;
        list << ListCotationsQuery.value(1).toString() << ListCotationsQuery.value(2).toString() << ListCotationsQuery.value(3).toString();
        ui->ActeCotationcomboBox->addItem(ListCotationsQuery.value(0).toString(),list);
    }
    req = "(SELECT TypeActe, " + champ + ", montantpratique, null as nom FROM rufus.cotations\n"
    "where idUser = " + QString::number(gDataUser->getIdUserParent()) + " and typeacte not in (select codeccam from rufus.ccam)\n"
    "order by typeacte)";
    QSqlQuery ListCotationsQuery2 (req, DataBase::getInstance()->getDataBase() );
    DataBase::getInstance()->traiteErreurRequete(ListCotationsQuery2,req,"");
    for (int i = 0; i < ListCotationsQuery2.size(); i++)
    {
        ListCotationsQuery2.seek(i);
        QStringList list;
        list << ListCotationsQuery2.value(1).toString() << ListCotationsQuery2.value(2).toString() << ListCotationsQuery2.value(3).toString();
        ui->ActeCotationcomboBox->addItem(ListCotationsQuery2.value(0).toString(),list);
    }

    req = "select typeacte as code from " NOM_TABLE_COTATIONS
                  " union "
                  " select codeccam as code from " NOM_TABLE_CCAM
                  " order by code asc";
    QSqlQuery listactquer(req, DataBase::getInstance()->getDataBase() );
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
    connect(comp,  QOverload<const QString &>::of(&QCompleter::activated), [=] {RetrouveMontantActe();});
    ui->ActeCotationcomboBox->lineEdit()->setCompleter(comp);
}

void Rufus::ReconstruitCombosCorresp()
{
    int idxcombo, idcor (-1), idxcomboA1, idcorA1 (-1), idxcomboA2, idcorA2 (-1);
    if (ui->tabWidget->currentIndex() == ui->tabWidget->indexOf(ui->tabDossier))
    {
        idxcombo = ui->MGupComboBox->currentIndex();
        if (idxcombo >-1)
            idcor = ui->MGupComboBox->currentData().toInt();
        idxcomboA1 = ui->AutresCorresp1upComboBox->currentIndex();
        if (idxcomboA1 >-1)
            idcorA1 = ui->AutresCorresp1upComboBox->currentData().toInt();
        idxcomboA2 = ui->AutresCorresp2upComboBox->currentIndex();
        if (idxcomboA2 >-1)
            idcorA2 = ui->AutresCorresp2upComboBox->currentData().toInt();
    }
    ui->MGupComboBox->clear();
    proc->ReconstruitComboCorrespondants(ui->MGupComboBox,false);
    ui->AutresCorresp1upComboBox->clear();
    ui->AutresCorresp2upComboBox->clear();
    proc->ReconstruitComboCorrespondants(ui->AutresCorresp1upComboBox);
    proc->ReconstruitComboCorrespondants(ui->AutresCorresp2upComboBox);
    if (ui->tabWidget->currentIndex() == ui->tabWidget->indexOf(ui->tabDossier))
    {
        ui->MGupComboBox            ->setCurrentIndex( idcor>-1?     ui->MGupComboBox               ->findData(idcor)   : -1 );
        ui->AutresCorresp1upComboBox->setCurrentIndex( idcorA1>-1?   ui->AutresCorresp1upComboBox   ->findData(idcorA1) : -1 );
        ui->AutresCorresp2upComboBox->setCurrentIndex( idcorA2>-1?   ui->AutresCorresp2upComboBox   ->findData(idcorA2) : -1 );
    }
    //Actualisation des combobox des correspondants
    if (ui->tabWidget->indexOf(ui->tabDossier) == -1)
        return;

    QString req = "SELECT idPat, idCorMedMG, idCorMedSpe1, idCorMedSpe2, idCorMedSpe3, idCorNonMed FROM " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS
              " WHERE idPat = " + QString::number(gidPatient);
    //qDebug() << requete;
    QSqlQuery DonneesMedicalesQuery (req, DataBase::getInstance()->getDataBase() );
    if (!DataBase::getInstance()->traiteErreurRequete(DonneesMedicalesQuery,req,"Impossible de retrouver les correspondants"))
    {
        if (DonneesMedicalesQuery.size() > 0)
        {
            DonneesMedicalesQuery.first();
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
        }
        else
        {
            MGlineEdit->clear();
            AutresCorresp1LineEdit->clear();
            AutresCorresp2LineEdit->clear();
        }
        OKModifierTerrain();
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

    Dlg_Refraction     = new dlg_refraction(&idPatAPasser, &NomPatient, &PrenomPatient, &idActeAPasser, &AgeAPasser, proc);
    Dlg_Refraction->setWindowTitle("Refraction - " + gNomPatient + " " + gPrenomPatient);
    Dlg_Refraction->setWindowIcon(Icons::icLunettes());
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
#ifdef Q_OS_LINUX
            Corps.append("<!LINUX>");
#endif
#ifdef Q_OS_MAC
            Corps.append("<!MAC>");
#endif

            QString updaterequete =  "UPDATE " NOM_TABLE_ACTES " SET ActeTexte = '" + proc->CorrigeApostrophe(Corps) +
                                     "' where idActe = " + ui->idActelineEdit->text();
            QSqlQuery UpdateUpTextEditQuery (updaterequete, DataBase::getInstance()->getDataBase() );
            DataBase::getInstance()->traiteErreurRequete(UpdateUpTextEditQuery,updaterequete,"Impossible de mettre à jour le champ Texte !");
            ui->ActeTextetextEdit->setText(Corps);
            mod = Q_NULLPTR;
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
#ifdef Q_OS_LINUX
            Corps.append("<!LINUX>");
#endif
#ifdef Q_OS_MAC
            Corps.append("<!MAC>");
#endif

            QString updaterequete =  "UPDATE " NOM_TABLE_ACTES " SET ActeConclusion = '" + proc->CorrigeApostrophe(Corps) +
                                     "' where idActe = " + ui->idActelineEdit->text();
            QSqlQuery UpdateUpTextEditQuery (updaterequete, DataBase::getInstance()->getDataBase() );
            DataBase::getInstance()->traiteErreurRequete(UpdateUpTextEditQuery,updaterequete, tr("Impossible de mettre à jour le champ conclusion !"));
            ui->ActeConclusiontextEdit->setFocus();
            ui->ActeConclusiontextEdit->moveCursor(QTextCursor::End);
            ui->ActeConclusiontextEdit->setText(Corps);

            mod = Q_NULLPTR;
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
    QSqlQuery quer (req, DataBase::getInstance()->getDataBase() );
    DataBase::getInstance()->traiteErreurRequete(quer,req);
    if (quer.size()>0)
    {
        quer.first();
        QString prefix = "";
        // Les axes
        if (quer.value(2).toDouble()!=0.0)
        {
            if (quer.value(2).toInt()<10)
                prefix = "  ";
            else if (quer.value(2).toInt()<100)
                prefix = " ";
            Mesure["AxeOD"] = prefix + quer.value(2).toString();
        }
        prefix = "";
        if (quer.value(6).toDouble()!=0.0)
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
        if (quer.value(3).toDouble()!=0.0)
            Mesure["AddOD"] = "+0" + QString::number(quer.value(3).toDouble(),'f',2);
        if (quer.value(7).toDouble()!=0.0)
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
    Dlg_RemCheq          = new dlg_remisecheques(proc);
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
//BUG : -> CREER PATIENT
bool Rufus::Remplir_ListePatients_TableView(QString requete, QString PatNom, QString PatPrenom)
{
    QString Addrequete;
    QStandardItem *pitem, *pitem0, *pitem1;

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
    if (DataBase::getInstance()->getMode() == DataBase::Distant)
        requete += " LIMIT 1000";
    QSqlQuery   RemplirTableViewQuery (requete, DataBase::getInstance()->getDataBase() );
    if (DataBase::getInstance()->traiteErreurRequete(RemplirTableViewQuery,requete,"")) return false;
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
         ui->PatientsListeTableView->setRowHeight(j,int(fm.height()*1.3));

    ui->PatientsListeTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->PatientsListeTableView->horizontalHeader()->setFixedHeight(int(fm.height()*1.3));

    return true;
}

void Rufus::Remplir_SalDat()
{
    QTableWidget        *TableAMettreAJour;
    int                 i;
    QString             NomPrenom, zw, A;
    QFontMetrics        fm(qApp->font());

    // SALLE D'ATTENTE ---------------------------------------------------------------------------------------------------
    QString SalDatrequete = "SELECT saldat.IdPat, PatNom, PatPrenom, HeureArrivee, Statut, saldat.Motif, HeureRDV, Message, saldat.idUser, UserLogin, PatDDN, rdv.motif "
                            " FROM " NOM_TABLE_SALLEDATTENTE " AS saldat"
                            " INNER JOIN " NOM_TABLE_PATIENTS " ON " NOM_TABLE_PATIENTS ".idPat = saldat.idPat "
                            " INNER JOIN " NOM_TABLE_UTILISATEURS " ON " NOM_TABLE_UTILISATEURS ".idUser = saldat.idUser "
                            " left outer join " NOM_TABLE_MOTIFSRDV " rdv on saldat.motif = rdv.raccourci "
                            " WHERE saldat.Statut = '" ARRIVE "'"
                            " OR saldat.Statut = '" ENCOURS "'"
                            " OR (LOCATE('" ENATTENTENOUVELEXAMEN "', saldat.Statut,1) > 0) "
                            " ORDER BY HeureRDV";
    TableAMettreAJour = ui->SalleDAttenteupTableWidget;

    for (int i =0; i< ui->SalleDAttenteupTableWidget->rowCount();i++)
        ui->SalleDAttenteupTableWidget->removeRow(i);
    QSqlQuery       RemplirTableViewUserQuery (SalDatrequete, DataBase::getInstance()->getDataBase() );
    DataBase::getInstance()->traiteErreurRequete(RemplirTableViewUserQuery,SalDatrequete, tr("Impossible de remplir la salle d'attente!"));
    RemplirTableViewUserQuery.first();

    TableAMettreAJour   ->setRowCount(RemplirTableViewUserQuery.size());
    gListeSuperviseursModel->clear();
    QStandardItem       *pitem0, *pitem1;
    QList<int>          listidusers;
    for (i = 0; i < RemplirTableViewUserQuery.size(); i++)
    {
        QMap<QString, QVariant> rsgnmt;
        rsgnmt["idpat"]             = RemplirTableViewUserQuery.value(0).toInt();
        rsgnmt["motif"]             = RemplirTableViewUserQuery.value(11).toString();
        rsgnmt["ddnpat"]            = RemplirTableViewUserQuery.value(10).toDate();
        rsgnmt["idsuperviseur"]     = RemplirTableViewUserQuery.value(8).toInt();
        rsgnmt["loginsuperviseur"]  = RemplirTableViewUserQuery.value(9).toString();
        rsgnmt["urgence"]           = (RemplirTableViewUserQuery.value(5).toString()=="URG");
        rsgnmt["message"]           = RemplirTableViewUserQuery.value(7).toString();

        UpLabel *label0, *label1, *label2, *label3, *label4, *label5, *label6;
        label0 = new UpLabel(TableAMettreAJour);
        label1 = new UpLabel(TableAMettreAJour);
        label2 = new UpLabel(TableAMettreAJour);
        label3 = new UpLabel(TableAMettreAJour);
        label4 = new UpLabel(TableAMettreAJour);
        label5 = new UpLabel(TableAMettreAJour);
        label6 = new UpLabel(TableAMettreAJour);


        label0->setData(rsgnmt);
        label1->setData(rsgnmt);
        label2->setData(rsgnmt);
        label3->setData(rsgnmt);
        label4->setData(rsgnmt);
        label5->setData(rsgnmt);
        label6->setData(rsgnmt);

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
            label2->setPixmap(Icons::pxApres().scaled(10,10)); //WARNING : icon scaled : pxApres 10,10

        QString color;
        if (RemplirTableViewUserQuery.value(3).toTime().toString("HH:mm") != "")
        {
            QTime heureArriv = RemplirTableViewUserQuery.value(3).toTime();
            label5->setText(heureArriv.toString("HH:mm"));                              // Heure RDV
            if (heureArriv.secsTo(QTime::currentTime())/60 < 15)
                color = "color: green";
            else if (heureArriv.secsTo(QTime::currentTime())/60 < 30)
               color = "color: orange";
            else
               color = "color: red";
        }
        label6->setText(RemplirTableViewUserQuery.value(9).toString());                 // Superviseur
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
            QSqlQuery colorquer(req2, DataBase::getInstance()->getDataBase() );
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
        connect (label0,        &QWidget::customContextMenuRequested,       this,   [=] {MenuContextuelSalDat(label0);});
        connect (label1,        &QWidget::customContextMenuRequested,       this,   [=] {MenuContextuelSalDat(label1);});
        connect (label2,        &QWidget::customContextMenuRequested,       this,   [=] {MenuContextuelSalDat(label2);});
        connect (label3,        &QWidget::customContextMenuRequested,       this,   [=] {MenuContextuelSalDat(label3);});
        connect (label4,        &QWidget::customContextMenuRequested,       this,   [=] {MenuContextuelSalDat(label4);});
        connect (label5,        &QWidget::customContextMenuRequested,       this,   [=] {MenuContextuelSalDat(label5);});
        connect (label6,        &QWidget::customContextMenuRequested,       this,   [=] {MenuContextuelSalDat(label6);});
        connect (label0,        &UpLabel::enter,            this,                   [=] {AfficheMotif(label0);});
        connect (label1,        &UpLabel::enter,            this,                   [=] {AfficheMotif(label1);});
        connect (label2,        &UpLabel::enter,            this,                   [=] {AfficheMotif(label2);});
        connect (label3,        &UpLabel::enter,            this,                   [=] {AfficheMotif(label3);});
        connect (label4,        &UpLabel::enter,            this,                   [=] {AfficheMotif(label4);});
        connect (label5,        &UpLabel::enter,            this,                   [=] {AfficheMotif(label5);});
        connect (label6,        &UpLabel::enter,            this,                   [=] {AfficheMotif(label6);});
        connect (label0,        &UpLabel::clicked,          this,                   [=] {SurbrillanceSalDat(label0);});
        connect (label1,        &UpLabel::clicked,          this,                   [=] {SurbrillanceSalDat(label1);});
        connect (label2,        &UpLabel::clicked,          this,                   [=] {SurbrillanceSalDat(label2);});
        connect (label3,        &UpLabel::clicked,          this,                   [=] {SurbrillanceSalDat(label3);});
        connect (label4,        &UpLabel::clicked,          this,                   [=] {SurbrillanceSalDat(label4);});
        connect (label5,        &UpLabel::clicked,          this,                   [=] {SurbrillanceSalDat(label5);});
        connect (label6,        &UpLabel::clicked,          this,                   [=] {SurbrillanceSalDat(label6);});
        connect (label0,        &UpLabel::dblclick,         this,                   [=] {if (gDataUser->isSoignant()) ChoixDossier(rsgnmt["idpat"].toInt());});
        connect (label1,        &UpLabel::dblclick,         this,                   [=] {if (gDataUser->isSoignant()) ChoixDossier(rsgnmt["idpat"].toInt());});
        connect (label2,        &UpLabel::dblclick,         this,                   [=] {if (gDataUser->isSoignant()) ChoixDossier(rsgnmt["idpat"].toInt());});
        connect (label3,        &UpLabel::dblclick,         this,                   [=] {if (gDataUser->isSoignant()) ChoixDossier(rsgnmt["idpat"].toInt());});
        connect (label4,        &UpLabel::dblclick,         this,                   [=] {if (gDataUser->isSoignant()) ChoixDossier(rsgnmt["idpat"].toInt());});
        connect (label5,        &UpLabel::dblclick,         this,                   [=] {if (gDataUser->isSoignant()) ChoixDossier(rsgnmt["idpat"].toInt());});
        connect (label6,        &UpLabel::dblclick,         this,                   [=] {if (gDataUser->isSoignant()) ChoixDossier(rsgnmt["idpat"].toInt());});
        TableAMettreAJour->setCellWidget(i,0,label0);
        TableAMettreAJour->setCellWidget(i,1,label1);
        TableAMettreAJour->setCellWidget(i,2,label2);
        TableAMettreAJour->setCellWidget(i,3,label3);
        TableAMettreAJour->setCellWidget(i,4,label4);
        TableAMettreAJour->setCellWidget(i,5,label5);
        TableAMettreAJour->setCellWidget(i,6,label6);
        TableAMettreAJour->setRowHeight(i,int(QFontMetrics(qApp->font()).height()*1.1));

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
            gSalDatTab  ->insertTab(0, Icons::icFamily(), tr("Tout le monde"));
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
            if (gSalDatTab->tabData(i).toInt() == gDataUser->getIdUserActeSuperviseur())
            {
                gSalDatTab->setCurrentIndex(i);
                a = true;
            }
        }
        if (!a)
            gSalDatTab->setCurrentIndex(0);
        FiltreSalleDAttente();
    }


    // BUREAUX ---------------------------------------------------------------------------------------------------

    QList<UpTextEdit *> listuptext = ui->scrollArea->findChildren<UpTextEdit*>();
    if (listuptext.size() > 0)
        for (int j=0; j<listuptext.size();j++)
            delete listuptext.at(j);
    ui->scrollArea->takeWidget();
    DataBase::getInstance()->commit();
    QString bureauxreq = "SELECT usc.idUser, UserLogin, NomPosteConnecte, PatNom, PatPreNom, HeureArrivee, saldat.idPat FROM " NOM_TABLE_USERSCONNECTES " AS usc\n"
                         " INNER JOIN " NOM_TABLE_UTILISATEURS " ON " NOM_TABLE_UTILISATEURS ".idUser = usc.idUser\n"
                         " LEFT OUTER JOIN " NOM_TABLE_SALLEDATTENTE " as saldat on (idUserEnCoursExam = usc.idUser and saldat.posteexamen = usc.NomPosteConnecte)\n"
                         " LEFT OUTER JOIN " NOM_TABLE_PATIENTS " ON " NOM_TABLE_PATIENTS ".idPat = saldat.idPat\n"
                         " where time_to_sec(timediff(NOW(), HeureDerniereConnexion)) < 60\n"
                         " and (soignant = 1 or soignant = 2 or soignant = 3)\n"
                         " order by UserLogin";

    //UpMessageBox::Watch(this,bureauxreq);
    QSqlQuery   BureauxQuery (bureauxreq, DataBase::getInstance()->getDataBase() );
    DataBase::getInstance()->traiteErreurRequete(BureauxQuery,bureauxreq,"bureauxreq");

    //UpMessageBox::Watch(this,userconnectreq + "\n- " + QString::number(userconnectQuery.size()) + " -\n- " + userconnectQuery.value(0).toString() + " -");
    DataBase::getInstance()->traiteErreurRequete(BureauxQuery, bureauxreq,"");
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
            UserBureau->disconnect();
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
                if( UserBureau->getIdUser() == gDataUser->id() )
                    connect(UserBureau, &UpTextEdit::dblclick,  [=] {if (gDataUser->isSecretaire()) ChoixDossier(UserBureau->getId());});
                else
                {
                    connect(UserBureau,         &QWidget::customContextMenuRequested,   [=] {MenuContextuelBureaux(UserBureau);});
                    connect(UserBureau,         &UpTextEdit::dblclick,                  [=] {AutreDossier(UserBureau->getId());});
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
                                            " UserParent, saldat.motif, PatDDN, rdv.motif, saldat.iduser "                          // 10, 11, 12, 13, 14
                                            " FROM " NOM_TABLE_SALLEDATTENTE " AS saldat"
                                            " INNER JOIN " NOM_TABLE_PATIENTS " ON " NOM_TABLE_PATIENTS ".idPat = saldat.idPat "
                                            " INNER JOIN " NOM_TABLE_UTILISATEURS " ON " NOM_TABLE_UTILISATEURS ".idUser = saldat.idUser"
                                            " INNER JOIN " NOM_TABLE_ACTES " ON " NOM_TABLE_ACTES ".idActe = saldat.idActeAPayer"
                                            " left outer join " NOM_TABLE_MOTIFSRDV " rdv on saldat.motif = rdv.raccourci "
                                            " WHERE saldat.Statut = '" RETOURACCUEIL "'"
                                            " ORDER BY UserLogin, HeureArrivee";
    //qDebug() << PaiementsEnAttenterequete;

    TableAMettreAJour = ui->AccueilupTableWidget;
    QSqlQuery RemplirTableViewPaiementQuery (PaiementsEnAttenterequete, DataBase::getInstance()->getDataBase() );
    DataBase::getInstance()->traiteErreurRequete(RemplirTableViewPaiementQuery,PaiementsEnAttenterequete, tr("Impossible de remplir la salle d'attente!"));
    RemplirTableViewPaiementQuery.first();
    TableAMettreAJour->clearContents();
    TableAMettreAJour->setRowCount(RemplirTableViewPaiementQuery.size());
    gListeParentsModel  = new QStandardItemModel;
    QStandardItem       *oitem0, *oitem1;
    QList<int>          listidparents;

    for (i = 0; i < RemplirTableViewPaiementQuery.size(); i++)
    {
        QMap<QString, QVariant> rsgnmt;
        rsgnmt["idpat"] = RemplirTableViewPaiementQuery.value(0).toInt();
        rsgnmt["motif"] = RemplirTableViewPaiementQuery.value(13).toString();
        rsgnmt["ddnpat"] = RemplirTableViewPaiementQuery.value(12).toDate();
        rsgnmt["idsuperviseur"] = RemplirTableViewPaiementQuery.value(14).toInt();
        rsgnmt["loginsuperviseur"] = RemplirTableViewPaiementQuery.value(3).toString();
        rsgnmt["urgence"] = (RemplirTableViewPaiementQuery.value(11).toString()=="URG");
        rsgnmt["message"] = RemplirTableViewPaiementQuery.value(8).toString();

        UpLabel *label0, *label1, *label2, *label3, *label4, *label5;
        label0 = new UpLabel;
        label1 = new UpLabel;
        label2 = new UpLabel;
        label3 = new UpLabel;
        label4 = new UpLabel;
        label5 = new UpLabel;

        label0->setData(rsgnmt);
        label1->setData(rsgnmt);
        label2->setData(rsgnmt);
        label3->setData(rsgnmt);
        label4->setData(rsgnmt);
        label5->setData(rsgnmt);

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
        if (RemplirTableViewPaiementQuery.value(5).toDouble() == 0.0)
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

        connect (label0,        &QWidget::customContextMenuRequested,       this,   [=] {MenuContextuelSalDatPaiemt(label0);});
        connect (label1,        &QWidget::customContextMenuRequested,       this,   [=] {MenuContextuelSalDatPaiemt(label1);});
        connect (label2,        &QWidget::customContextMenuRequested,       this,   [=] {MenuContextuelSalDatPaiemt(label2);});
        connect (label3,        &QWidget::customContextMenuRequested,       this,   [=] {MenuContextuelSalDatPaiemt(label3);});
        connect (label4,        &QWidget::customContextMenuRequested,       this,   [=] {MenuContextuelSalDatPaiemt(label4);});
        connect (label5,        &QWidget::customContextMenuRequested,       this,   [=] {MenuContextuelSalDatPaiemt(label5);});
        connect (label0,        &UpLabel::enter,            this,                   [=] {AfficheMotif(label0);});
        connect (label1,        &UpLabel::enter,            this,                   [=] {AfficheMotif(label1);});
        connect (label2,        &UpLabel::enter,            this,                   [=] {AfficheMotif(label2);});
        connect (label3,        &UpLabel::enter,            this,                   [=] {AfficheMotif(label3);});
        connect (label4,        &UpLabel::enter,            this,                   [=] {AfficheMotif(label4);});
        connect (label5,        &UpLabel::enter,            this,                   [=] {AfficheMotif(label5);});
        TableAMettreAJour   ->setCellWidget(i,0,label0);
        TableAMettreAJour   ->setCellWidget(i,1,label1);
        TableAMettreAJour   ->setCellWidget(i,2,label2);
        TableAMettreAJour   ->setCellWidget(i,3,label3);
        TableAMettreAJour   ->setCellWidget(i,4,label4);
        pItem               ->setText(RemplirTableViewPaiementQuery.value(7).toString());
        TableAMettreAJour   ->setItem(i,5,pItem);
        TableAMettreAJour   ->setCellWidget(i,6,label5);
        TableAMettreAJour   ->setRowHeight(i,int(fm.height()*1.1));

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
                if (gAccueilTab->tabData(i).toInt() == gDataUser->getIdUserParent())
                {
                    gAccueilTab->setCurrentIndex(i);
                    a = true;
                }
            if (!a)
                gAccueilTab->setCurrentIndex(0);
        }
        FiltreAccueil(gAccueilTab->currentIndex());
    }
    // PATIENTS VUS CE JOUR ----------------------------------------------------------------------------------------------------------
    if(ui->PatientsVusWidget->isVisible())
        MAJPatientsVus();
}


/*-----------------------------------------------------------------------------------------------------------------
-- Afficher un résumé du statut -----------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void Rufus::ResumeStatut()
{
    // le statut utilisateur
    gResumeStatut = proc->getsSessionStatus() + "\n\n";

    // les socket
    if (UtiliseTCP)
    {
        QStringList::const_iterator itsocket;
        QString Serveur = gListSockets.at(0);
        // le 1er item de gListSockets est le serveur
        gResumeStatut += tr("ServeurTCP") + "\n\t"
                + Serveur.split(TCPMSG_Separator).at(2) + " - "
                + Serveur.split(TCPMSG_Separator).at(0) + " - "
                + Serveur.split(TCPMSG_Separator).at(1) + " --- "
                + Datas::I()->users->getLoginById(Serveur.split(TCPMSG_Separator).at(3).toInt());

        gListSockets.removeFirst();
        gResumeStatut += "\n" + tr("Postes connectés") + "\n";
        for( itsocket = gListSockets.constBegin(); itsocket != gListSockets.constEnd(); ++itsocket )
        {
            gResumeStatut += "\t" + itsocket->split(TCPMSG_Separator).at(2) + " - "
                    + itsocket->split(TCPMSG_Separator).at(0) + " - "
                    + itsocket->split(TCPMSG_Separator).at(1) + " --- "
                    + Datas::I()->users->getLoginById(itsocket->split(TCPMSG_Separator).at(3).toInt()) + "\n";
        }
    }

    // l'importateur des documents
    gResumeStatut += "\n" + tr("Poste importateur des documents") + "\t";
    QString A = proc->PosteImportDocs();
    if (A == "")
        gResumeStatut += tr("Pas de poste paramétré");
    else
    {
        A.remove(".local");
        QString B;
        if (A.contains(" - " NOM_ADMINISTRATEURDOCS))
            B = tr("Administrateur");
        else
            B = (A.contains(" - prioritaire")? tr("prioritaire") : tr("non prioritaire"));
        A.remove(" - prioritaire");
        A.remove(" - " NOM_ADMINISTRATEURDOCS);
        gResumeStatut += A + " - " + B;
    }

    // version de Rufus et de la base
    gResumeStatut += "\n\n" + tr("Version de Rufus ") + "\t\t" + qApp->applicationVersion();
    gResumeStatut += "\n" + tr("Version de la base ") + "\t\t";
    QString VerifBasereq = "select VersionBase from " NOM_TABLE_PARAMSYSTEME;
    QSqlQuery VersionBaseQuery(VerifBasereq,DataBase::getInstance()->getDataBase());
    if (VersionBaseQuery.lastError().type() != QSqlError::NoError || VersionBaseQuery.size()==0)
        gResumeStatut += tr("inconnue");
    else
    {
        VersionBaseQuery.first();
        gResumeStatut +=  VersionBaseQuery.value(0).toString();
    }
    proc->emit ModifEdit(gResumeStatut);
}

/*-----------------------------------------------------------------------------------------------------------------
-- Salle d'attente ------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
bool Rufus::RetourSalleDattente(QString Titre)
{
    int idPatAPasser    = gidPatient;
    int idActeAPasser   = gidActe;
    QString TitreAPasser= gNomPatient + " " + gPrenomPatient + "\n" + Titre;
    bool retour         = false;

    Dlg_SalDat           = new dlg_salledattente(&idPatAPasser, &idActeAPasser, &TitreAPasser, proc);
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
    QSqlQuery ListeRecettesQuery (requete, DataBase::getInstance()->getDataBase() );
    DataBase::getInstance()->traiteErreurRequete(ListeRecettesQuery,requete,"");
    ListeRecettesQuery.first();
    for (int l = 0; l < ListeRecettesQuery.size(); l++)
    {
        // on vérifie pour chaque ligne s'il s'agit d'un virement, d'une carte bleue ou d'un chèque enregistré
        requete = "SELECT ModePaiement, NomTiers, idRemise FROM " NOM_TABLE_RECETTES " WHERE idRecette = " + ListeRecettesQuery.value(0).toString();
        QSqlQuery ModePaiementQuery (requete, DataBase::getInstance()->getDataBase() );
        DataBase::getInstance()->traiteErreurRequete(ModePaiementQuery,requete,"");
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
        UpMessageBox::Watch(this, "Euuhh... " + gDataUser->getLogin(), Messg);
        return;
    }

    // On demande confirmation de la suppression de l'acte
    msgbox.setText("Euuhh... " + gDataUser->getLogin());
    msgbox.setInformativeText(tr("Etes vous sûr de vouloir supprimer cet acte?"));
    msgbox.setIcon(UpMessageBox::Warning);
    UpSmallButton OKBouton  (tr("Supprimer"));
    UpSmallButton NoBouton  (tr("Annuler"));
    msgbox.addButton(&NoBouton,UpSmallButton::CANCELBUTTON);
    msgbox.addButton(&OKBouton, UpSmallButton::SUPPRBUTTON);
    msgbox.exec();
    if (msgbox.clickedButton() != &OKBouton)
        return;

    // On récupère la date de l'acte
     //on va rechercher l'idActe suivant
    idASupprimer = gidActe;
    idAAficher = 0;
    QDate dateacte;

    requete = "SELECT idActe, ActeDate FROM " NOM_TABLE_ACTES " WHERE idPat = '" + QString::number(gidPatient) + "'";
    QSqlQuery       SupprimerActeQuery (requete, DataBase::getInstance()->getDataBase() );
    if (DataBase::getInstance()->traiteErreurRequete(SupprimerActeQuery,requete, tr("Impossible de retrouver les consultations de ce patient")))
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
    QSqlQuery DelRefractionQuery(requete, DataBase::getInstance()->getDataBase() );
    DataBase::getInstance()->traiteErreurRequete(DelRefractionQuery,requete,"");

    // on supprime les éventuels bilans orthoptiques liés à cette consultation -----------------------------------------------------------
    QString delborequete = "DELETE FROM " NOM_TABLE_BILANORTHO " WHERE idBilanOrtho  = " + QString::number(idASupprimer);
    QSqlQuery DelBOQuery(delborequete, DataBase::getInstance()->getDataBase() );
    DataBase::getInstance()->traiteErreurRequete(DelBOQuery,delborequete,"");

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
            QSqlQuery RecetteQuery (requete, DataBase::getInstance()->getDataBase() );
            DataBase::getInstance()->traiteErreurRequete(RecetteQuery,requete,"");
            if (RecetteQuery.size()>0)
            {
                RecetteQuery.first();
                for (int k=0; k<RecetteQuery.size(); k++)
                {
                    QString req = "delete from " NOM_TABLE_RECETTES " where idrecette = " + QString::number(recetteACorriger);
                    if (RecetteQuery.value(0).toDouble() > listmontantsacorriger.at(j))
                        req = "update " NOM_TABLE_RECETTES " set Montant = " + QString::number(RecetteQuery.value(0).toDouble() - listmontantsacorriger.at(j)) +
                                " where idRecette = " + QString::number(recetteACorriger);
                    QSqlQuery quer(req, DataBase::getInstance()->getDataBase() );
                    DataBase::getInstance()->traiteErreurRequete(quer,req,"");
                    RecetteQuery.next();
                }
            }
        }

        // On actualise la table des lignes de paiement et la table des Type de paieement
        requete = "DELETE FROM " NOM_TABLE_LIGNESPAIEMENTS " WHERE idActe = " + QString::number(idASupprimer);
        QSqlQuery DelLignesPaiementsQuery(requete, DataBase::getInstance()->getDataBase() );
        DataBase::getInstance()->traiteErreurRequete(DelLignesPaiementsQuery,requete,"");
        requete = "DELETE FROM " NOM_TABLE_TYPEPAIEMENTACTES " WHERE idActe = " + QString::number(idASupprimer);
        QSqlQuery DelTypesPaiementsQuery(requete, DataBase::getInstance()->getDataBase() );
        DataBase::getInstance()->traiteErreurRequete(DelTypesPaiementsQuery,requete,"");
    }

    // on supprime la consultation -------------------------------------------------------------------------------------------------
    requete = "DELETE FROM " NOM_TABLE_ACTES " WHERE idActe = " + QString::number(idASupprimer);
    QSqlQuery DelActesQuery(requete, DataBase::getInstance()->getDataBase() );
    if (DataBase::getInstance()->traiteErreurRequete(DelActesQuery,requete, tr("Impossible de supprimer cet acte!")))
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
        QSqlQuery quer0("select patnom, PatPrenom from " NOM_TABLE_PATIENTS " where idpat = " + QString::number(gidPatient),  DataBase::getInstance()->getDataBase() );
        if (quer0.size() == 0) return;
        quer0.first();
        gNomPatient = quer0.value(0).toString();
        gPrenomPatient = quer0.value(1).toString();
    }

    //2. On recherche les actes de ce dossier qui seraient en tiers payant et qui auraient déjà reçu des versements auquel cas, on ne peut pas supprimer les actes ni le dossier
    QString Messg = "";
    QString requete = " SELECT idRecette, Paye FROM " NOM_TABLE_LIGNESPAIEMENTS " WHERE idActe in (SELECT act.idActe FROM " NOM_TABLE_ACTES " act WHERE act.idPat = '" + QString::number(gidPatient) + "')";
    QSqlQuery ListeRecettesQuery (requete, DataBase::getInstance()->getDataBase() );
    DataBase::getInstance()->traiteErreurRequete(ListeRecettesQuery,requete,"");
    ListeRecettesQuery.first();
    for (int l = 0; l < ListeRecettesQuery.size(); l++)
    {
        // on vérifie pour chaque ligne s'il s'agit d'un virement ou d'une carte bleue ou d'un chèque enregistré
        requete = "SELECT ModePaiement, NomTiers, idRemise FROM " NOM_TABLE_RECETTES " WHERE idRecette = " + ListeRecettesQuery.value(0).toString();
        QSqlQuery ModePaiementQuery (requete, DataBase::getInstance()->getDataBase() );
        DataBase::getInstance()->traiteErreurRequete(ModePaiementQuery,requete,"");
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
        UpMessageBox::Watch(this, "Euuhh... " + gDataUser->getLogin(), Messg);
        return;
    }

    //3. On commence par demander la confirmation de la suppression du dossier
    msgbox.setText("Euuhh... " + gDataUser->getLogin());
    msgbox.setInformativeText(tr("Etes vous sûr de vouloir supprimer le dossier de ") + gNomPatient + " " + gPrenomPatient + "?");
    msgbox.setIcon(UpMessageBox::Warning);
    UpSmallButton OKBouton  (tr("Supprimer"));
    UpSmallButton NoBouton  (tr("Annuler"));
    msgbox.addButton(&NoBouton, UpSmallButton::CANCELBUTTON);
    msgbox.addButton(&OKBouton, UpSmallButton::SUPPRBUTTON);
    msgbox.exec();
    if (msgbox.clickedButton() != &OKBouton)
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
            QSqlQuery RecetteQuery (requete, DataBase::getInstance()->getDataBase() );
            DataBase::getInstance()->traiteErreurRequete(RecetteQuery,requete,"");
            if (RecetteQuery.size()>0)
            {
                RecetteQuery.first();
                for (int k=0; k<RecetteQuery.size(); k++)
                {
                    QString req = "delete from " NOM_TABLE_RECETTES " where idrecette = " + QString::number(recetteACorriger);
                    if (RecetteQuery.value(0).toDouble() > listmontantsacorriger.at(j))
                        req = "update " NOM_TABLE_RECETTES " set Montant = " + QString::number(RecetteQuery.value(0).toDouble() - listmontantsacorriger.at(j)) +
                                " where idRecette = " + QString::number(recetteACorriger);
                    QSqlQuery quer(req, DataBase::getInstance()->getDataBase() );
                    DataBase::getInstance()->traiteErreurRequete(quer,req,"");
                    RecetteQuery.next();
                }
            }
        }
        //4, On actualise la table des lignes de paiement el la table des Type de paieement
        requete = "DELETE FROM " NOM_TABLE_LIGNESPAIEMENTS " WHERE idActe in (SELECT act.idActe FROM " NOM_TABLE_ACTES " act WHERE act.idPat = '" + QString::number(gidPatient) + "')";
        QSqlQuery DelLignesPaiementsQuery(requete, DataBase::getInstance()->getDataBase() );
        DataBase::getInstance()->traiteErreurRequete(DelLignesPaiementsQuery,requete,"");
        requete = "DELETE FROM " NOM_TABLE_TYPEPAIEMENTACTES " WHERE idActe in (SELECT act.idActe FROM " NOM_TABLE_ACTES " act WHERE act.idPat = '" + QString::number(gidPatient) + "')";
        QSqlQuery DelTypesPaiementsQuery(requete, DataBase::getInstance()->getDataBase() );
        DataBase::getInstance()->traiteErreurRequete(DelTypesPaiementsQuery,requete,"");
    }

    //5. On ferme l'onglet dossier
    ui->tabWidget->removeTab(ui->tabWidget->indexOf(ui->tabDossier));
    FermeDlgAnnexes();

    //6. On supprime les bilans orthoptiques correspondants à ce dossier
    requete =  "DELETE FROM " NOM_TABLE_BILANORTHO " WHERE idbilanortho in (SELECT idActe from " NOM_TABLE_ACTES " where idPat = " + QString::number(gidPatient) + ")";
    QSqlQuery       SupprimerblorthoQuery (requete, DataBase::getInstance()->getDataBase() );
    if (DataBase::getInstance()->traiteErreurRequete(SupprimerblorthoQuery,requete, tr("Impossible de retrouver les bilans orthoptiques de ce patient")))
        return;

    //7. On supprime tous les actes correspondants à ce dossier
    requete =  "DELETE FROM " NOM_TABLE_ACTES " WHERE idPat = " + QString::number(gidPatient);
    QSqlQuery       SupprimerActeQuery (requete, DataBase::getInstance()->getDataBase() );
    if (DataBase::getInstance()->traiteErreurRequete(SupprimerActeQuery,requete, tr("Impossible de retrouver les consultations de ce patient")))
        return;

    //8. On supprime le dossier dans la base Rufus
    requete =  "DELETE FROM " NOM_TABLE_PATIENTS " WHERE idPat = " + QString::number(gidPatient);
    QSqlQuery DelPatientQuery(requete, DataBase::getInstance()->getDataBase() );
    if (DataBase::getInstance()->traiteErreurRequete(DelPatientQuery,requete, tr("Impossible de supprimer ce dossier!")))
        return;

    //9. On supprime les documents émis
    requete =  "DELETE FROM " NOM_TABLE_IMPRESSIONS " WHERE idPat = " + QString::number(gidPatient);
    QSqlQuery DelPatientImpressionsQuery(requete, DataBase::getInstance()->getDataBase() );
    if (DataBase::getInstance()->traiteErreurRequete(DelPatientImpressionsQuery,requete, tr("Impossible de supprimer les impressoins de ce dossier!")))
        return;

    requete =  "DELETE FROM " NOM_TABLE_DONNEESSOCIALESPATIENTS " WHERE idPat = " + QString::number(gidPatient);
    QSqlQuery MAJSocialQuery (requete, DataBase::getInstance()->getDataBase() );
    if (DataBase::getInstance()->traiteErreurRequete(MAJSocialQuery,requete, tr("Impossible de supprimer ce dossier de la table des données sociales!")))
        return;

    requete =  "DELETE FROM " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS " WHERE idPat = " + QString::number(gidPatient);
    QSqlQuery MAJMedicalQuery (requete, DataBase::getInstance()->getDataBase() );
    if (DataBase::getInstance()->traiteErreurRequete(MAJMedicalQuery,requete, tr("Impossible de supprimer ce dossier de la table des données medicales!")))
        return;

    requete =  "DELETE FROM " NOM_TABLE_MOTSCLESJOINTURES " WHERE idPat = " + QString::number(gidPatient);
    QSqlQuery MAJMotsQuery (requete, DataBase::getInstance()->getDataBase() );
    if (DataBase::getInstance()->traiteErreurRequete(MAJMotsQuery,requete, tr("Impossible de supprimer ce dossier de la table des mots clés!")))
        return;

    requete =  "DELETE FROM " NOM_TABLE_SALLEDATTENTE " WHERE idPat = " + QString::number(gidPatient);
    QSqlQuery MAJSalDatQuery (requete, DataBase::getInstance()->getDataBase() );
    if (DataBase::getInstance()->traiteErreurRequete(MAJSalDatQuery,requete, tr("Impossible de supprimer ce dossier de la salle d'attente!")))
        return;
    FlagMetAjourSalDat();

    //10. On supprime le dossier dans la base OPhtalmologie
    requete =  "DELETE FROM " NOM_TABLE_REFRACTION
            " WHERE idPat = " + QString::number(gidPatient);
    QSqlQuery DelRefractionQuery (requete, DataBase::getInstance()->getDataBase() );
    DataBase::getInstance()->traiteErreurRequete(DelRefractionQuery,requete, tr("Impossible de supprimer ce dossier de la table des réfractions!"));

    requete =  "DELETE FROM " NOM_TABLE_DONNEES_OPHTA_PATIENTS
            " WHERE idPat = " + QString::number(gidPatient);
    QSqlQuery DelDonneesOphtaQuery (requete, DataBase::getInstance()->getDataBase() );
    DataBase::getInstance()->traiteErreurRequete(DelDonneesOphtaQuery,requete, tr("Impossible de supprimer ce dossier de la table des données ophtalmologiques!"));

    requete =  "DELETE FROM " NOM_TABLE_BIOMETRIES
            " WHERE idPat = " + QString::number(gidPatient);
    QSqlQuery DelBiometrieQuery (requete, DataBase::getInstance()->getDataBase() );
    DataBase::getInstance()->traiteErreurRequete(DelBiometrieQuery,requete, tr("Impossible de supprimer ce dossier de la table des biométries!"));

    requete =  "DELETE FROM " NOM_TABLE_TONOMETRIE
            " WHERE idPat = " + QString::number(gidPatient);
    QSqlQuery DelTonoQuery (requete, DataBase::getInstance()->getDataBase() );
    DataBase::getInstance()->traiteErreurRequete(DelTonoQuery,requete, tr("Impossible de supprimer ce dossier de la table des tonométries!"));


    //10. On reconstruit le treeView Liste
    gidPatient = 0;
    Remplir_ListePatients_TableView(grequeteListe,"","");
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

    Dlg_AutresMes           = new dlg_autresmesures(&idPatAPasser, proc, dlg_autresmesures::TONO);
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
                Tono = "<td width=\"60\"><font color = \"" + proc->CouleurTitres + "\"><b>" + tr("TOG:") + "</b></font></td><td width=\"80\">" + TOGcolor + " à " + QTime::currentTime().toString("H") + "H</td><td width=\"80\">(" + Methode + ")</td><td>" + gDataUser->getLogin() + "</td>";
            if (TOG.toInt() == 0 && TOD.toInt() > 0)
                Tono = "<td width=\"60\"><font color = \"" + proc->CouleurTitres + "\"><b>" + tr("TOD:") + "</b></font></td><td width=\"80\">" + TODcolor + " à " + QTime::currentTime().toString("H") + "H</td><td width=\"80\">(" + Methode + ")</td><td>" + gDataUser->getLogin() + "</td>";
            if (TOD.toInt() > 0 && TOG.toInt() > 0)
            {
                if (TOD.toInt() == TOG.toInt())
                    Tono = "<td width=\"60\"><font color = \"" + proc->CouleurTitres + "\"><b>" + tr("TODG:") + "</b></font></td><td width=\"80\">" + TODcolor + " à " + QTime::currentTime().toString("H") + "H</td><td width=\"80\">(" + Methode + ")</td><td>" + gDataUser->getLogin() + "</td>";
                else
                    Tono = "<td width=\"60\"><font color = \"" + proc->CouleurTitres + "\"><b>" + tr("TO:") +"</b></font></td><td width=\"80\">" + TODcolor + "/" + TOGcolor+ " à " + QTime::currentTime().toString("H") + "H</td><td width=\"80\">(" + Methode + ")</td><td>" + gDataUser->getLogin() + "</td>";
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
            QSqlQuery UpdateUpTextEditQuery (updaterequete, DataBase::getInstance()->getDataBase() );
            DataBase::getInstance()->traiteErreurRequete(UpdateUpTextEditQuery,updaterequete, tr("Impossible de mettre à jour le champ Texte !"));
        }
        ui->ActeTextetextEdit->setFocus();
        ui->ActeTextetextEdit->moveCursor(QTextCursor::End);
    }
    Dlg_AutresMes->close(); // nécessaire pour enregistrer la géométrie
    delete Dlg_AutresMes;
}

/*-----------------------------------------------------------------------------------------------------------------
-- Trouver un dossier d'après la DDN ------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void Rufus::TrouverDDN()
{
    QString requete = "SELECT idPat, PatNom, PatPrenom, PatDDN FROM " NOM_TABLE_PATIENTS
            " WHERE PatDDN = '" + ui->CreerDDNdateEdit->date().toString("yyyy-MM-dd") + "'";
    QSqlQuery   TrouverDDNQuery (requete, DataBase::getInstance()->getDataBase() );
    if (!DataBase::getInstance()->traiteErreurRequete(TrouverDDNQuery,requete,tr("Impossible de retrouver un patient pour cette date de naissance")))
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
             ui->PatientsListeTableView->setRowHeight(j,int(fm.height()*1.3));

        ui->PatientsListeTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
        ui->PatientsListeTableView->horizontalHeader()->setFixedHeight(int(fm.height()*1.3));

        ui->PatientsListeTableView->selectRow(0);
        ui->PatientsListeTableView->scrollTo(gListePatientsModel->item(0)->index(),QAbstractItemView::PositionAtTop);
    }
    EnableCreerDossierButton();
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
        QSqlQuery RechercheGratuitQuery (requete, DataBase::getInstance()->getDataBase() );
        DataBase::getInstance()->traiteErreurRequete(RechercheGratuitQuery,requete,"Slot_EditingFinshed()");
        if (RechercheGratuitQuery.size() > 0)
        {
            QSound::play(NOM_ALARME);
            UpMessageBox msgbox;
            msgbox.setText(tr("Cet acte a déjà été enregistré comme acte gratuit !"));
            msgbox.setInformativeText(tr("Annuler et considérer comme acte payant?"));
            msgbox.setIcon(UpMessageBox::Warning);
            UpSmallButton OKBouton      (tr("Considérer comme payant"));
            UpSmallButton AnnulBouton   (tr("Annuler"));
            msgbox.addButton(&AnnulBouton, UpSmallButton::CANCELBUTTON);
            msgbox.addButton(&OKBouton, UpSmallButton::STARTBUTTON);
            msgbox.exec();
            if (msgbox.clickedButton() != &OKBouton)
            {
                ui->ActeMontantlineEdit->setText(AncienMontant);
                return false;
            }
            else
            {
                requete = "DELETE FROM " NOM_TABLE_TYPEPAIEMENTACTES " WHERE idActe = " + QString::number(gidActe);
                QSqlQuery DelGratuitQuery (requete, DataBase::getInstance()->getDataBase() );
                DataBase::getInstance()->traiteErreurRequete(DelGratuitQuery,requete,"");
                AfficheActeCompta();
            }
        }
        else
            ui->ActeMontantlineEdit->setText(NouveauMontant);
    }
    //on modifie la table Actes avec le nouveau montant
    QString requete = "UPDATE " NOM_TABLE_ACTES " SET ActeMontant = " + QString::number(QLocale().toDouble(NouveauMontant)) + " WHERE idActe = " + QString::number(gidActe);
    QSqlQuery UPDMontantActeQuery (requete, DataBase::getInstance()->getDataBase() );
    DataBase::getInstance()->traiteErreurRequete(UPDMontantActeQuery,requete,"");
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

void Rufus::NouvelleMesureRefraction() //utilisé pour ouvrir la fiche refraction quand un appareil a transmis une mesure
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
        QSqlQuery UpdateUpTextEditQuery (updaterequete, DataBase::getInstance()->getDataBase() );
        DataBase::getInstance()->traiteErreurRequete(UpdateUpTextEditQuery,updaterequete, tr("Impossible de mettre à jour le champ Texte !"));
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
        QSqlQuery UpdateUpTextEditQuery (updaterequete, DataBase::getInstance()->getDataBase() );
        DataBase::getInstance()->traiteErreurRequete(UpdateUpTextEditQuery,updaterequete, tr("Impossible de mettre à jour le champ Texte !"));
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
        QSqlQuery UpdateUpTextEditQuery (updaterequete, DataBase::getInstance()->getDataBase() );
        DataBase::getInstance()->traiteErreurRequete(UpdateUpTextEditQuery,updaterequete, tr("Impossible de mettre à jour le champ Texte !"));
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
        QSqlQuery UpdateUpTextEditQuery (updaterequete, DataBase::getInstance()->getDataBase() );
        DataBase::getInstance()->traiteErreurRequete(UpdateUpTextEditQuery,updaterequete, tr("Impossible de mettre à jour le champ Texte !"));
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
void Rufus::LireLaCPS()
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
    QSqlQuery ChercheUserQuery (requete, DataBase::getInstance()->getDataBase() );
    if (DataBase::getInstance()->traiteErreurRequete(ChercheUserQuery,requete, "Impossible d'ouvrir la table Utilisateurs"))
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
        gDataUser->setId(ChercheUserQuery.value(0).toInt());
        setWindowTitle("Rufus - " + gDataUser->getLogin() + " - " + gDataUser->getFonction());
        }
}
/*-----------------------------------------------------------------------------------------------------------------
    Lire la CV avec Pyxvital : Retour = fichier Patient.par
-----------------------------------------------------------------------------------------------------------------*/
void Rufus::LireLaCV()
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
    Remplir_ListePatients_TableView(requete,"","") ;   // LireLaCV()
    if (gNombreDossiers == 0)       // aucun patient trouvé
        {
        // si rien trouvé, deuxième recherche sur date de naissance seule
        requete = "SELECT IdPat, PatNom, PatPrenom, PatDDN, Sexe  "
                  " FROM "  NOM_TABLE_PATIENTS
                  " WHERE PatDDN = '" + zdat + "'";
        Remplir_ListePatients_TableView(requete,"","") ;   // LireLaCV()
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
}
/*-----------------------------------------------------------------------------------------------------------------
    Saisie d'une facture avec Pyxvital : Retour = fichier Facture.par
-----------------------------------------------------------------------------------------------------------------*/
void Rufus::SaisieFSE()
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


void Rufus::TraiteTCPMessage(QString msg)
{
    //qDebug() << msg + " - sur rufus::traitetcpmessage()";
    if (msg == TCPMSG_MAJSalAttente)
        Remplir_SalDat();                       // par le TCPSocket
    else if (msg == TCPMSG_MAJCorrespondants)
    {
        proc->initListeCorrespondants();
        ReconstruitCombosCorresp();             // maj par le TCPSocket
        // TODO signifier à dlg_identificationpatient la modification au cas où cette fiche serait ouverte
        //dlg_message(QStringList() << tr("Mise à jour de la liste des correspondants"), 3000);
    }
    else if (msg.contains(TCPMSG_MsgBAL))
    {
        /* le message a le format suivant nombredemessages + TCPMSG_MsgBAL) */
        msg.remove(TCPMSG_MsgBAL);
        gTotalNvxMessages = msg.toInt();
        msg = "";
        if (gTotalNvxMessages>1)
               msg = tr("Vous avez ") + QString::number(gTotalNvxMessages) + tr(" nouveaux messages");
           else if (gTotalNvxMessages>0)
               msg = tr("Vous avez 1 nouveau message");
           if (msg!="")
           {
               QSound::play(NOM_ALARME);
               gMessageIcon->showMessage(tr("Messages"), msg, Icons::icPostit(), 10000);
               if (gMsgDialog != Q_NULLPTR)
                   if (gMsgDialog->isVisible())
                       AfficheMessages();
           }
    }
    else if (msg.contains(TCPMSG_MAJDocsExternes))
    {
        /* le message a le format suivant idpatient + TCPMSG_MAJDocsExternes) */
        msg.remove(TCPMSG_MAJDocsExternes);
        if (gidPatient == msg.toInt())
            MAJDocsExternes();                  // depuis le tcpsocket
    }
    else if (msg.contains(TCPMSG_ListeSockets))
    {
        msg.remove("{}" TCPMSG_ListeSockets);
        gListSockets.clear();
        gListSockets = msg.split("{}");
        //qDebug() << "liste des clients connectés rufus.cpp - " + QTime::currentTime().toString("hh-mm-ss");
        for (int i=0; i<gListSockets.size(); i++)
        {
            QString data = gListSockets.at(i);
            data.replace(TCPMSG_Separator, " - ");
            //qDebug() << data;
        }
        ResumeStatut();
    }
}

void Rufus::envoieMessage(QString msg)
{
    if (!UtiliseTCP)
        return;
    //qDebug() << msg + " - void Rufus::envoieMessage(QString msg)";
    currentmsg = msg;
    QByteArray paquet   = currentmsg.toUtf8();
    QByteArray size     = Utils::IntToArray(paquet.size());
    if(TcPConnect->state() == QAbstractSocket::ConnectedState)
    {
        TcPConnect->write(size);                //envoie la taille du message
        TcPConnect->write(paquet);              //envoie le message
        TcPConnect->waitForBytesWritten(5000);
    }
}

void Rufus::envoieMessageA(QList<int> listidusr)
{
    if (!UtiliseTCP)
    {
        proc->MAJflagMessages();
        return;
    }
    QString listid;
    for (int i=0; i<listidusr.size(); i++)
    {
        listid += QString::number(listidusr.at(i));
        if (listidusr.at(i) < (listidusr.size()-1))
            listid += ",";
    }
    QString msg = listid + TCPMSG_Separator + "1" + TCPMSG_MsgBAL;
    currentmsg = tr("courrier");
    envoieMessage(msg);
}

