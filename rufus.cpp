﻿/* (C) 2018 LAINE SERGE
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

#include "rufus.h"
#include "ui_rufus.h"

Rufus::Rufus(QWidget *parent) : QMainWindow(parent)
{
    Datas::I();

    // la version du programme correspond à la date de publication, suivie de "/" puis d'un sous-n° - p.e. "23-6-2017/3"
    qApp->setApplicationVersion("02-08-2019/1");       // doit impérativement être composé de date version / n°version;

    ui = new Ui::Rufus;
    ui->setupUi(this);
    setWindowFlags(Qt::Window | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint | Qt::WindowMinMaxButtonsHint);

    srand(static_cast<uint>(time(Q_NULLPTR)));

    qApp->setStyleSheet(Styles::StyleAppli());

    proc = Procedures::I();
    if (!proc->Init())                                                  //! vérifie que le fichier rufus.ini est cohérent
        exit(0);                                                        //! choisit le mode de connexion au serveur

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
    db = DataBase::I();
    m_parametres = db->parametres();
    proc->setDirImagerie();                                //! lit l'emplacement du dossier d'imagerie sur le serveur

    //! 1 - Restauration de la position de la fenetre et de la police d'écran
    restoreGeometry(proc->gsettingsIni->value("PositionsFiches/Rufus").toByteArray());
    setWindowIcon(Icons::icSunglasses());

    //! 2 - charge les data du user connecté
    m_currentuser = Datas::I()->users->userconnected();
    if (m_currentuser == Q_NULLPTR)
    {
        UpMessageBox::Watch(this, tr("Pas d'utilisateur identifié!\nSortie du programme"));
        exit(0);
    }
    qApp->setStyleSheet(Styles::StyleAppli());
    dlg_message(m_currentuser->status() + "\n" + tr("Site") + "\t\t= " + Datas::I()->sites->getById(m_currentuser->idsitedetravail())->nom(), 3000);

    //! 3 Initialisation de tout
    InitVariables();
    InitWidgets();
    InitEventFilters();
    CreerMenu();
    InitMenus();

    //! 4 reconstruction des combobox des correspondants et de la liste des documents
    ReconstruitCombosCorresp();                 //! initialisation de la liste

    FiltreTable();                         //! InitTables()
    if (Datas::I()->postesconnectes->admin() == Q_NULLPTR)
        VerifVerrouDossier();
    MAJPosteConnecte();

    //! 5 - lancement du TCP
    UtiliseTCP = false;
    QString log;
    if (Datas::I()->postesconnectes->admin() != Q_NULLPTR)
    {
        log = tr("RufusAdmin présent");
        Logs::MSGSOCKET(log);
        if (Datas::I()->postesconnectes->admin()->ipadress() == "")
        {
            log = tr("Aucun serveur TCP enregistré dans la base");
            Logs::MSGSOCKET(log);
            dlg_message(QStringList() << log, false);
        }
        else
        {
            Utils::Pause(100);
            TcPConnect = TcpSocket::I();
            UtiliseTCP = TcPConnect->TcpConnectToServer(Datas::I()->postesconnectes->admin()->ipadress());
            if (UtiliseTCP)
            {
                QString msg;
                dlg_message(QStringList() << tr("Connexion TCP OK"), 3000, false);
                connect(TcPConnect, &TcpSocket::tcpmessage, this, [=](QString msg) {TraiteTCPMessage(msg);});  // traitement des messages reçus
                // envoi iduser
                msg = QString::number(m_currentuser->id()) + TCPMSG_idUser;
                envoieMessage(msg);
                // envoi adresse IP, adresse MAC, nom d'hôte
                msg = Utils::getIpAdress() + TCPMSG_Separator + Utils::getMACAdress() + TCPMSG_Separator + QHostInfo::localHostName() + TCPMSG_DataSocket;
                envoieMessage(msg);
            }
            else {
                log = tr("RufusAdmin présent mais échec connexion");
                Logs::MSGSOCKET(log);
            }
        }
    }
    else
    {
        log = tr("RufusAdmin absent");
        Logs::MSGSOCKET(log);
    }

    //! 6 - mettre en place le TcpSocket et/ou les timer
    gTimerPatientsVus           = new QTimer(this);     // effacement automatique de la liste des patients vus - réglé à 20"
    gTimerSalDat                = new QTimer(this);     // scrutation des modifs de la salle d'attente                                                          utilisé en cas de non utilisation des tcpsocket (pas de rufusadmin ou poste distant)
    gTimerCorrespondants        = new QTimer(this);     // scrutation des modifs de la liste des correspondants                                                 utilisé en cas de non utilisation des tcpsocket (pas de rufusadmin ou poste distant)
    gTimerVerifMessages         = new QTimer(this);     // scrutation des nouveaux message                                                                      utilisé en cas de non utilisation des tcpsocket (pas de rufusadmin ou poste distant)
    gTimerPosteConnecte         = new QTimer(this);     // mise à jour de la connexion à la base de données
    gTimerVerifImportateurDocs  = new QTimer(this);     // vérifie que le poste importateur des documents externes est toujours là
    gTimerExportDocs            = new QTimer(this);     // utilisé par le poste importateur pour vérifier s'il y a des documents à sortir de la base
    gTimerActualiseDocsExternes = new QTimer(this);     // actualise l'affichage des documents externes si un dossier est ouvert
    gTimerImportDocsExternes    = new QTimer(this);     // utilisé par le poste importateur pour vérifier s'il y a des documents à importer dans la base
    gTimerVerifVerrou           = new QTimer(this);     // utilisé en  l'absence de TCPServer pour vérifier l'absence d'utilisateurs déconnectés dans la base
    gTimerSupprDocs             = new QTimer(this);     // utilisé par le poste importateur pour vérifier s'il y a des documents à supprimer

    gTimerPatientsVus   ->setSingleShot(true);          // il est singleshot et n'est démarré que quand on affiche la liste des patients vus
    gTimerPatientsVus   ->setInterval(20000);
    // Lancement des timers de gestion des documents
    gTimerVerifImportateurDocs      ->start(60000);
    // Lancement du timer de vérification des verrous - +++ à lancer après le timer gTimerVerifImportateurDocs puisqu'il l'utilise
    gTimerVerifVerrou               ->start(60000);// "toutes les 60 secondes"


    if (db->getMode() == DataBase::Distant)
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
                gTimerSalDat        ->start(1000);
                gTimerCorrespondants->start(30000);
        }
    }

    gTimerPosteConnecte->start(10000);// "toutes les 10 secondes - remet à jour le statut connecté du poste dans la base - tables utilisateursconnectes"

    if (!UtiliseTCP)
    {
        m_flagcorrespondants    = Flags::I()->flagCorrespondants();
        m_flagsalledattente     = Flags::I()->flagSalleDAttente();
        m_flagmessages          = Flags::I()->flagMessages();
        connect (gTimerSalDat,              &QTimer::timeout,   this,   &Rufus::VerifSalleDAttente);
        connect (gTimerCorrespondants,      &QTimer::timeout,   this,   &Rufus::VerifCorrespondants);
        connect (gTimerVerifImportateurDocs,&QTimer::timeout,   this,   &Rufus::VerifImportateur);
        connect (gTimerVerifVerrou,         &QTimer::timeout,   this,   &Rufus::VerifVerrouDossier);
        connect (gTimerVerifMessages,       &QTimer::timeout,   this,   &Rufus::VerifMessages);
        connect (gTimerImportDocsExternes,  &QTimer::timeout,   this,   &Rufus::ImportDocsExternes);
        if (db->getMode() != DataBase::Distant)
            connect(gTimerSupprDocs,        &QTimer::timeout,   this,   &Rufus::SupprimerDocsEtFactures);
        VerifImportateur();
    }
    connect (gTimerPosteConnecte,           &QTimer::timeout,   this,   &Rufus::MAJPosteConnecte);
    connect (gTimerActualiseDocsExternes,   &QTimer::timeout,   this,   &Rufus::ActualiseDocsExternes);
    connect (gTimerPatientsVus,             &QTimer::timeout,   this,   &Rufus::MasquePatientsVusWidget);

    //! 7 - Nettoyage des erreurs éventuelles de la salle d'attente
    m_listepatientsencours->initListeAll();
    // on donne le statut "arrivé" aux patients en salle d'attente dont le iduserencourssexam est l'utilisateur actuel et qui n'auraient pas été supprimés (plantage)
    QString blabla              = ENCOURSEXAMEN;
    int length                  = blabla.size();
    foreach (PatientEnCours *pat, m_listepatientsencours->patientsencours()->values())
    {
        if (pat != Q_NULLPTR)
            if (pat->iduser() == m_currentuser->id() && pat->statut().left(length) == ENCOURSEXAMEN && pat->posteexamen() == QHostInfo::localHostName().left(60))
            {
                ItemsList::update(pat, CP_STATUT_SALDAT, ARRIVE);
                ItemsList::update(pat, CP_POSTEEXAMEN_SALDAT);
                ItemsList::update(pat, CP_IDUSERENCOURSEXAM_SALDAT);
            }
    }

    //! 8 les slots
    Connect_Slots();

    //! 9 - libération des verrous de la compta
    QString req = " delete from " TBL_VERROUCOMPTAACTES " where PosePar = " + QString::number(m_currentuser->id());
    db->StandardSQL(req);

    closeFlag = false;

    setTitre();
    if (m_currentuser->isSoignant())
        ReconstruitListesCotations();

    //! 10 - Mise à jour des salles d'attente
    Remplir_SalDat();
    if(UtiliseTCP)
        envoieMessage(TCPMSG_MAJSalAttente);

    //! 11 - Vérification de la messagerie
    ReconstruitListeMessages();

    //! 12 - Affichage des boutons bilan orthoptique
    ui->CreerBOpushButton   ->setVisible(m_currentuser->isOrthoptist());
    ui->CreerBOpushButton_2 ->setVisible(m_currentuser->isOrthoptist());

    //! 13 - mise à jour du programmateur de sauvegardes
    proc->InitBackupAuto();

    //! 14 - choix mode (création dossier ou sélection de patient)
    if (m_listepatientsmodel->rowCount() == 0)
    {
        ModeCreationDossier();
        ui->LListepushButton->setEnabled(false);
        UpMessageBox::Watch(this,tr("Vous n'avez aucun dossier de patient enregistré!"), tr("Vous devez d'abord en créer un."));
    }
    else
        ModeSelectDepuisListe();
 }

Rufus::~Rufus()
{
    delete ui;
}

/*-----------------------------------------------------------------------------------------------------------------
-- Connexion des actions associees a chaque objet du formulaire et aux menus --------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void Rufus::Connect_Slots()
{
    // Les objets -------------------------------------------------------------------------------------------------
    connect (ui->AccueilupTableWidget,                              &QTableWidget::currentCellChanged,                  this,   [=] {ActiveActeAccueil(ui->AccueilupTableWidget->currentRow());});
    connect (ui->ActePrecedentpushButton,                           &QPushButton::clicked,                              this,   [=] {NavigationConsult(ItemsList::Prec);});
    connect (ui->ActeSuivantpushButton,                             &QPushButton::clicked,                              this,   [=] {NavigationConsult(ItemsList::Suiv);});
    connect (ui->CourrierAFairecheckBox,                            &QPushButton::clicked,                              this,   [=] {CourrierAFaireChecked();});
    connect (ui->CreerActepushButton,                               &QPushButton::clicked,                              this,   [=] {CreerActe(Datas::I()->patients->currentpatient());});
    connect (ui->CreerActepushButton_2,                             &QPushButton::clicked,                              this,   [=] {CreerActe(Datas::I()->patients->currentpatient());});
    connect (ui->CreerBOpushButton,                                 &QPushButton::clicked,                              this,   [=] {CreerBilanOrtho(Datas::I()->patients->currentpatient());});
    connect (ui->CreerBOpushButton_2,                               &QPushButton::clicked,                              this,   [=] {CreerBilanOrtho(Datas::I()->patients->currentpatient());});
    connect (ui->CreerDDNdateEdit,                                  &QDateEdit::dateChanged,                            this,   [=] {if (gMode == RechercheDDN) FiltreTableparDDN();});
    connect (ui->ChercherDepuisListepushButton,                     &QPushButton::clicked,                              this,   [=] {ChercherDepuisListe();});
    connect (ui->CreerNomlineEdit,                                  &QLineEdit::textEdited,                             this,   [=] {MajusculeCreerNom();});
    connect (ui->CreerPrenomlineEdit,                               &QLineEdit::textEdited,                             this,   [=] {MajusculeCreerPrenom();});
    connect (ui->CreerDossierpushButton,                            &QPushButton::clicked,                              this,   [=] {CreerDossierpushButtonClicked();});
    connect (ui->DernierActepushButton,                             &QPushButton::clicked,                              this,   [=] {NavigationConsult(ItemsList::Fin);});
    connect (ui->EnregistrePaiementpushButton,                      &QPushButton::clicked,                              this,   [=] {AppelPaiementDirect(BoutonPaiement);});
    connect (ui->FermepushButton,                                   &QPushButton::clicked,                              this,   &Rufus::SortieAppli);
    connect (ui->FSEpushButton,                                     &QPushButton::clicked,                              this,   [=] {SaisieFSE();});       // CZ001
    connect (ui->IdentPatienttextEdit,                              &QWidget::customContextMenuRequested,               this,   [=] {MenuContextuelIdentPatient();});
    connect (ui->LFermepushButton,                                  &QPushButton::clicked,                              this,   &Rufus::SortieAppli);
    connect (ui->ListepushButton,                                   &QPushButton::clicked,                              this,   [=] {ModeSelectDepuisListe();});
    connect (ui->LListepushButton,                                  &QPushButton::clicked,                              this,   [=] {ModeSelectDepuisListe();});
    connect (ui->LNouvDossierpushButton,                            &QPushButton::clicked,                              this,   [=] {ModeCreationDossier();;});
    connect (ui->LRecopierpushButton,                               &QPushButton::clicked,                              this,   [=] {RecopierDossier();});
    connect (ui->SendMessagepushButton,                             &QPushButton::clicked,                              this,   [=] {QMap<QString, QVariant> map;  map["null"] = true; SendMessage(map, Datas::I()->patients->currentpatient()->id());});
    connect (ui->LSendMessagepushButton,                            &QPushButton::clicked,                              this,   [=] {QMap<QString, QVariant> map;  map["null"] = true; SendMessage(map);});
    connect (ui->MGupComboBox,                                      QOverload<int>::of(&QComboBox::activated),          this,   [=] {ChoixMG();});
    connect (ui->AutresCorresp1upComboBox,                          QOverload<int>::of(&QComboBox::activated),          this,   [=] {ChoixCor(ui->AutresCorresp1upComboBox);});
    connect (ui->AutresCorresp2upComboBox,                          QOverload<int>::of(&QComboBox::activated),          this,   [=] {ChoixCor(ui->AutresCorresp2upComboBox);});
    connect (ui->MGupComboBox,                                      &QWidget::customContextMenuRequested,               this,   [=] {MenuContextuelMedecin();});
    connect (ui->AutresCorresp1upComboBox,                          &QWidget::customContextMenuRequested,               this,   [=] {MenuContextuelCorrespondant(ui->AutresCorresp1upComboBox);});
    connect (ui->AutresCorresp2upComboBox,                          &QWidget::customContextMenuRequested,               this,   [=] {MenuContextuelCorrespondant(ui->AutresCorresp2upComboBox);});
    connect (ui->ModifDatepushButton,                               &QPushButton::clicked,                              this,   [=] {ui->ActeDatedateEdit->setEnabled(true); ui->ActeDatedateEdit->setFocus();});
    connect (ModifIdentificationupSmallButton,                      &QPushButton::clicked,                              this,   [=] {ChoixMenuContextuelIdentPatient();});
    connect (ui->MotsClesLabel,                                     &QWidget::customContextMenuRequested,               this,   [=] {MenuContextuelMotsCles();});
    connect (ui->MotsClesupSmallButton,                             &QPushButton::clicked,                              this,   [=] {ChoixMenuContextuelMotsCles();});
    connect (ui->NouvDossierpushButton,                             &QPushButton::clicked,                              this,   [=] {ModeCreationDossier();});
    connect (ui->OuvreActesPrecspushButton,                         &QPushButton::clicked,                              this,   [=] {OuvrirActesPrecspushButtonClicked();});
    connect (ui->OuvreDocsExternespushButton,                       &QPushButton::clicked,                              this,   &Rufus::ActualiseDocsExternes);
    connect (ui->OuvrirDocumentpushButton,                          &QPushButton::clicked,                              this,   [=] {OuvrirDocuments();});
    connect (ui->PatientsListeTableView,                            &QWidget::customContextMenuRequested,               this,   [=] {MenuContextuelListePatients();});
    connect (ui->PatientsListeTableView,                            &QTableView::doubleClicked,                         this,   [=] {ChoixDossier(getPatientFromCursorPositionInTable());});
    connect (ui->PatientsListeTableView,                            &QTableView::entered,                               this,   [=] {AfficheToolTip(getPatientFromCursorPositionInTable());});
    connect (ui->PatientsListeTableView->selectionModel(),          &QItemSelectionModel::selectionChanged,             this,   [=] {EnableButtons();});
    connect (ui->PatientsVusFlecheupLabel,                          &UpLabel::clicked,                                  this,   [=] {AffichePatientsVusWidget();});
    connect (ui->PatientsVusupTableWidget,                          &QTableView::activated,                             this,   [=] {gTimerPatientsVus->start();});
    connect (ui->PremierActepushButton,                             &QPushButton::clicked,                              this,   [=] {NavigationConsult(ItemsList::Debut);});
    connect (ui->RefractionpushButton,                              &QPushButton::clicked,                              this,   &Rufus::RefractionMesure);
    connect (ui->SalleDAttentepushButton,                           &QPushButton::clicked,                              this,   &Rufus::SalleDAttente);
    connect (ui->SalleDAttenteupTableWidget,                        &UpTableWidget::dropsignal,                         this,   [=] {DropPatient(ui->SalleDAttenteupTableWidget->dropData());});
    connect (ui->SupprimeActepushButton,                            &QPushButton::clicked,                              this,   [=] {SupprimerActe(m_currentact);});
    connect (ui->TonometriepushButton,                              &QPushButton::clicked,                              this,   [=] {Tonometrie();});
    connect (ui->VitaleupPushButton,                                &QPushButton::clicked,                              this,   [=] {LireLaCV();}); // CZ001

    connect (ui->ActeMontantlineEdit,                               &UpLineEdit::TextModified,                          this,   [=] {ActeMontantModifie();});
    connect (ui->BasculerMontantpushButton,                         &QPushButton::clicked,                              this,   [=] {BasculerMontantActe();});
    connect (ui->CCAMlinklabel,                                     &QLabel::linkActivated,                             this,   [=] {QDesktopServices::openUrl(QUrl(LIEN_CCAM));});
    connect (ui->ModifierCotationActepushButton,                    &QPushButton::clicked,                              this,   [=] {ModfiCotationActe();});
    // Les tabs --------------------------------------------------------------------------------------------------
    connect (ui->tabWidget,                                         &QTabWidget::currentChanged,                        this,   [=] {ChangeTabBureau();});

    connect (proc,                                                  &Procedures::ConnectTimers,                         this,   [=] {ConnectTimers(proc->Connexion());});

    // MAJ Salle d'attente ----------------------------------------------------------------------------------
    connect(Flags::I(),                                             &Flags::UpdSalleDAttente,                           this,   [=](int a)  {   if (UtiliseTCP)
                                                                                                                                                    envoieMessage(TCPMSG_MAJSalAttente);
                                                                                                                                                else
                                                                                                                                                    m_flagsalledattente = a;
                                                                                                                                                Remplir_SalDat();
                                                                                                                                            } );
    // MAJ Correspondants ----------------------------------------------------------------------------------
    connect(Flags::I(),                                             &Flags::UpdCorrespondants,                          this,   [=](int a)  {   if (UtiliseTCP)
                                                                                                                                                    envoieMessage(TCPMSG_MAJCorrespondants);
                                                                                                                                                else
                                                                                                                                                    m_flagcorrespondants = a;
                                                                                                                                                ReconstruitCombosCorresp(false);
                                                                                                                                            } );
    // MAJ messages ----------------------------------------------------------------------------------
    connect(Flags::I(),                                             &Flags::UpdMessages,                                this,   [=](int a)  {   if (!UtiliseTCP)
                                                                                                                                                    m_flagmessages = a;
                                                                                                                                                ReconstruitListeMessages();
                                                                                                                                            } );

    // Nouvelle mesure d'appareil de refraction ----------------------------------------------------------------------------------
    if (proc->PortFronto()!=Q_NULLPTR || proc->PortAutoref()!=Q_NULLPTR || proc->PortRefracteur()!=Q_NULLPTR)
        connect(proc,                                               &Procedures::NouvMesureRefraction,                  this,   [=] {NouvelleMesureRefraction();});

    connect (ui->MoulinettepushButton,                              &QPushButton::clicked,                              this,   &Rufus::Moulinette);
    ui->MoulinettepushButton->setVisible(false);
}

void Rufus::OuvrirDocsExternes(Patient *pat)
{
    //! si la fiche est déjà ouverte, on quitte
    if (pat == Datas::I()->patients->currentpatient())  // -> depuis gTimerVerifGestDocs, AfficheDossier() ou ui->OuvreDocsExternespushButton
    {
        QList<dlg_docsexternes *> ListDialogDocs = this->findChildren<dlg_docsexternes *>();
        if (ListDialogDocs.size()>0)
            for (int i=0; i< ListDialogDocs.size();++i)
                if (ListDialogDocs.at(i)->getPatient() == Datas::I()->patients->currentpatient())
                    return;
    }
    DocsExternes *docs;
    if (pat == Datas::I()->patients->currentpatient())
        docs = Datas::I()->docsexternes;
    else docs = new DocsExternes;
    docs->initListeByPatient(pat);
    if (docs->docsexternes()->size()>0)
    {
        Dlg_DocsExt = new dlg_docsexternes(docs, (pat == Datas::I()->patients->currentpatient()), UtiliseTCP, this);
        ui->OuvreDocsExternespushButton->setEnabled(true);
        Dlg_DocsExt->show();
    }
    else
        if (pat == Datas::I()->patients->currentpatient())
            ui->OuvreDocsExternespushButton->setEnabled(false);
}

/*-----------------------------------------------------------------------------------------------------------------
-- // mettre à jour dlg_actesprecedents quand le contenu de l'acte affiché a été modifié  --------------------------
-----------------------------------------------------------------------------------------------------------------*/
void Rufus::MAJActesPrecs()
{
    QList<dlg_actesprecedents *> ListDialog = this->findChildren<dlg_actesprecedents *>();
    if (m_currentact != Q_NULLPTR)
        for (int n = 0; n < ListDialog.size(); n++)
            if (ListDialog.at(n)->getActeAffiche() == m_currentact)
                ListDialog.at(n)->ActesPrecsAfficheActe(m_currentact);
}

/*-----------------------------------------------------------------------------------------------------------------
-- // mettre à jour dlg_docsexternes   ----------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void Rufus::MAJDocsExternes()
{
    QList<dlg_docsexternes *> ListDialogDocs = findChildren<dlg_docsexternes *>();
    if (ListDialogDocs.size()>0)
    {
        for (int i=0; i< ListDialogDocs.size();++i)
            if (ListDialogDocs.at(i)->getPatient() == Datas::I()->patients->currentpatient())
            {
                proc->emit UpdDocsExternes();
                break;
            }
    }
    else if (m_currentuser->isSoignant())
    {
        if (Datas::I()->docsexternes->docsexternes()->size()>0)
        {
            Dlg_DocsExt = new dlg_docsexternes(Datas::I()->docsexternes, true, UtiliseTCP, this);
            Dlg_DocsExt->show();
            ui->OuvreDocsExternespushButton->setEnabled(true);
        }
        ui->OuvreDocsExternespushButton->setEnabled(false);
    }
}


/*-----------------------------------------------------------------------------------------------------------------
-- La moulinette ne sert qu'à tester des fonctions et n'est pas utilisée par le programe        ------------------
-----------------------------------------------------------------------------------------------------------------*/
void Rufus::Moulinette()
{

    // MODIFICATION DES TABLES CCAM ============================================================================================================================================================
    bool ok;
    QString req = "select codeccam from rufus.ccam";
    QList<QVariantList> listcodes = db->StandardSelectSQL(req, ok);
    for (int i=0; i< listcodes.size(); i++)
    {
        //qDebug() << i+1 << " - " << listcodes.at(i).at(0);
        QString code = listcodes.at(i).at(0).toString();
        req = "select max(date) from rufus.r_pu_base where codeccam = '" + code + "'";
        //qDebug() << req;
        QDate date = db->StandardSelectSQL(req , ok).at(0).at(0).toDate();
        req = "select max(montant), min(montant) from rufus.r_pu_base"
              " where codeccam = '" + code + "' and date = '" + date.toString("yyyy-MM-dd") + "'";
        //qDebug() << req;
        QVariantList opt = db->StandardSelectSQL(req, ok).at(0);
        req = "update rufus.ccam set"
              " OPTAM = " + QString::number(opt.at(0).toDouble(), 'f', 2) + ","
              " nonOPTAM = " + QString::number(opt.at(1).toDouble(), 'f', 2) +
              " where codeccam = '" + code + "'";
        //qDebug() << req;
        db->StandardSQL(req);
     }
    /*    QString req= "select codeCCAM, modificateur, montant from ccam.ccamd";
    QSqlQuery quer(req, db->getDataBase() );
    for (int i=0; i< quer.size(); i++)
    {
        quer.seek(i);
        if (quer.value(1).toInt()==1)
            req = "update ccam.ccamd set OPTAM = " + quer.value(2).toString().replace(",",".") + " where codeCCAM = '" + quer.value(0).toString() + "'";
        else if (quer.value(1).toInt()==2)
            req = "update ccam.ccamd set NonOPTAM = " + quer.value(2).toString().replace(",",".") + " where codeCCAM = '" + quer.value(0).toString() + "'";
        //proc->Edit(req);
        QSqlQuery(req, db->getDataBase() );
    }
    */

    /*QString req, str;
    QTextEdit txt;
    req = "select idimpression, Titre from " TBL_IMPRESSIONS " where soustypedoc = null and typedoc = 'DocRecu'";
    //qDebug() << req;
    QSqlQuery idquer (req, db->getDataBase() );
    idquer.first();
    for (int i=0; i<idquer.size(); i++)
    {
        txt.setHtml(idquer.value(1).toString());
        str = txt.toPlainText();
        req = "update " TBL_IMPRESSIONS " set soustypedoc = '" + str + "', Formatdoc = 'Prescription', emisrecu = 1 where idimpression = " + idquer.value(0).toString();
        //qDebug() << req;
        QSqlQuery (req, db->getDataBase() );
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
    QString req = "select idActe, ActeTexte from " TBL_ACTES " order by idacte desc";
    QSqlQuery imp(req, db->getDataBase() );
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
        req = "update " TBL_ACTES " set actetexte = '" + Utils::correctquoteSQL(texte2) + "' where idacte = " + imp.value(0).toString();
        QSqlQuery modif(req, db->getDataBase() );
        db->erreurRequete(modif, req,"");
        imp.next();
        if (b==10)
            b=0;
        if (b==0)
        {
            QTime dieTime= QTime::currentTime().addMSecs(1);
            while (QTime::currentTime() < dieTime)
                QCoreApplication::processEvents(QEventLoop::AllEvents, 1);
            dlg_message("correction des actes - acte n° " + imp.value(0).toString());
        }
        b+=1;
    }
    req = "select idimpression, textCorps from " TBL_IMPRESSIONS " order by idimpression desc";
    QSqlQuery imp2(req, db->getDataBase() );
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
        req = "update " TBL_IMPRESSIONS " set textcorps = '" + Utils::correctquoteSQL(texte2) + "' where idimpression = " + imp2.value(0).toString();
        QSqlQuery modif(req, db->getDataBase() );
        db->erreurRequete(modif, req,"");
        imp2.next();
        if (b==10)
            b=0;
        if (b==0)
        {
            QTime dieTime= QTime::currentTime().addMSecs(1);
            while (QTime::currentTime() < dieTime)
                QCoreApplication::processEvents(QEventLoop::AllEvents, 1);
            dlg_message("correction des impressions - impression n° " + imp2.value(0).toString());
        }
        b+=1;
    }
    */


    /*
    // CORRECTION DE LA BASE VILLES - ELIMINATION DES TIRETS ============================================================================================================================================================
    QString req = "select patville from " TBL_DONNEESSOCIALESPATIENTS;
    QSqlQuery quer(req, db->getDataBase() );
    quer.first();
    for (int i=0;i<quer.size();i++)
    {
        QString ville = quer.value(0).toString();
        if (ville.contains("-"))
        {
        req = "update " TBL_DONNEESSOCIALESPATIENTS " set patVille = '" + Utils::correctquoteSQL(ville.replace("-"," ")) + "' where patville = '" + Utils::correctquoteSQL(quer.value(0).toString()) + "'";
        QSqlQuery(req, db->getDataBase() );
        }
        quer.next();
    }
    req = "select corville from " TBL_CORRESPONDANTS;
    QSqlQuery quer1(req, db->getDataBase() );
    quer1.first();
    for (int i=0;i<quer1.size();i++)
    {
        QString ville = quer1.value(0).toString();
        if (ville.contains("-"))
        {
        req = "update " TBL_CORRESPONDANTS " set corVille = '" + Utils::correctquoteSQL(ville.replace("-"," ")) + "' where corville = '" + Utils::correctquoteSQL(quer1.value(0).toString()) + "'";
        QSqlQuery(req, db->getDataBase() );
        }
        quer1.next();
    }
    proc->Edit("OK pour villes");*/


/*    // CREATION D'UNE BASE FACTICE ============================================================================================================================================================
    //Mélange les noms, et 1ère ligne d'adresse dans la base
    if (UpMessageBox::Question(this,tr("ATTENTION"),tr("Cette fonction sert à générer une base factice pour la démonstration du logiciel") + "<br />"
                               + tr("Si vous cliquez sur OK, tous les enregistrements de la base seront mélangés et les données seront donc irrémédiablement perdues")) != UpSmallButton::STARTBUTTON)
        return;
    int idauhasard;
    QString copierequete = "drop table if exists rufus.patients2;\n";
    copierequete += "create table rufus.patients2 like rufus.patients;\n";
    copierequete += "insert into rufus.patients2 (select * from " TBL_PATIENTS ");";
    db->StandardSQL(copierequete);

    QStringList listNoms;
    copierequete = "select idPat, patNom from rufus.patients2;";
    QList<QVariantList> copielist = db->StandardSelectSQL(copierequete,ok);
    int s = copielist.size();
    for (int i = 0; i < copielist.size(); i++)
        listNoms << copielist.at(i).at(1).toString();
    for (int k = 0; k < s ; k++)
    {
        QString idpat = copielist.at(k).at(0).toString();
        idauhasard = rand() % (listNoms.size());
        QString AncNom (""), NouvNom;
        NouvNom = listNoms.at(idauhasard);
        QVariantList patdata = db->getFirstRecordFromStandardSelectSQL("select patnom, patprenom from " TBL_PATIENTS " where idPat = " + idpat, ok);
        if (patdata.size()>0)
            AncNom = patdata.at(0).toString();
            //dlg_message(quernom.value(1).toString() + " " + AncNom + " - " + QString::number(k) + "/" + QString::number(s), 1);
            //qDebug() << quernom.value(1).toString() + " " + AncNom + " - " + QString::number(k) + "/" + QString::number(s);
        copierequete = "update rufus.patients2 set patnom = '" + Utils::correctquoteSQL(listNoms.at(idauhasard)) + "' where idPat = " + idpat;
        db->StandardSQL(copierequete);
        listNoms.removeAt(idauhasard);

        QList<QVariantList> modifacteslist = db->StandardSelectSQL("select idacte, actemotif, acteconclusion, actetexte from " TBL_ACTES " where idpat = " + idpat, ok);
        for (int m=0; m<modifacteslist.size(); m++)
        {
            QString nouvmotif   = modifacteslist.at(m).at(1).toString();
            QString nouvconcl   = modifacteslist.at(m).at(2).toString();
            QString nouvtxt     = modifacteslist.at(m).at(3).toString();
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
                QString req1 = "update " TBL_ACTES " set"
                       " actemotif = '"         + Utils::correctquoteSQL(nouvmotif) + "',"
                       " actetexte = '"         + Utils::correctquoteSQL(nouvtxt)   + "',"
                       " acteconclusion = '"    + Utils::correctquoteSQL(nouvconcl) + "'"
                       " where idacte = " + modifacteslist.at(m).at(0).toString();
                //qDebug() << req1;
                db->StandardSQL(req1);
            }
        }
        QList<QVariantList> modifimprlist = db->StandardSelectSQL("select idimpression, textentete, textcorps, textorigine from " TBL_IMPRESSIONS " where idpat = " + idpat, ok);
        for (int m=0; m<modifimprlist.size(); m++)
        {
            QString nouventete  = modifimprlist.at(m).at(1).toString();
            QString nouvcorps   = modifimprlist.at(m).at(2).toString();
            QString nouvorigine = modifimprlist.at(m).at(3).toString();
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
                QString req1 = "update " TBL_IMPRESSIONS " set"
                       " textentete = '"         + Utils::correctquoteSQL(nouventete) + "',"
                       " textcorps = '"         + Utils::correctquoteSQL(nouvcorps)   + "',"
                       " textorigine = '"    + Utils::correctquoteSQL(nouvorigine) + "'"
                       " where idimpression = " + modifimprlist.at(m).at(0).toString();
                //qDebug() << req1;
                db->StandardSQL(req1);
            }
        }
    }
    copierequete = "delete from rufus.patients;\n";
    copierequete += "insert into rufus.patients (select * from rufus.patients2);\n";
    copierequete += "drop table if exists rufus.patients2;\n";
    db->StandardSQL(copierequete);

    UpMessageBox::Watch(this,"OK pour nom");

    copierequete = "drop table if exists rufus.donneessocialespatients2;\n";
    copierequete += "create table rufus.donneessocialespatients2 like rufus.donneessocialespatients;\n";
    copierequete += "insert into rufus.donneessocialespatients2 (select * from rufus.donneessocialespatients);";
    db->StandardSQL(copierequete);

    QStringList listAdresses;
    copierequete = "select idPat, patAdresse1 from rufus.donneessocialespatients2 order by patAdresse1;";
    QList<QVariantList> copieAlist = db->StandardSelectSQL(copierequete,ok);
    s = copieAlist.size();
    for (int i = 0; i < copieAlist.size(); i++)
        listAdresses << copieAlist.at(i).at(1).toString();
    for (int j = 0; j < s ; j++)
    {
        QString idpat = copieAlist.at(j).at(0).toString();
        idauhasard = rand() % (listAdresses.size());
        copierequete = "update rufus.donneessocialespatients2 set patAdresse1 = '" + Utils::correctquoteSQL(listAdresses.at(idauhasard))
                + "' where idPat = " + idpat;
        db->StandardSQL(copierequete);
        listAdresses.removeAt(idauhasard);
    }
    listAdresses.clear();
    copierequete = "select idPat, patAdresse2 from rufus.donneessocialespatients2 order by patAdresse2;";
    QList<QVariantList> copieA2list = db->StandardSelectSQL(copierequete,ok);
    s = copieA2list.size();
    for (int i = 0; i < copieA2list.size(); i++)
        listAdresses << copieA2list.at(i).at(1).toString();
    for (int j = 0; j < s ; j++)
    {
        QString idpat = copieA2list.at(j).at(0).toString();
        idauhasard = rand() % (listAdresses.size());
        copierequete = "update rufus.donneessocialespatients2 set patAdresse2 = '" + Utils::correctquoteSQL(listAdresses.at(idauhasard))
                + "' where idPat = " + idpat;
        db->StandardSQL(copierequete);
        listAdresses.removeAt(idauhasard);
    }
    copierequete = "delete from rufus.donneessocialespatients;\n";
    copierequete += "insert into rufus.donneessocialespatients (select * from rufus.donneessocialespatients2);\n";
    copierequete += "drop table if exists rufus.donneessocialespatients2;\n";
    db->StandardSQL(copierequete);
    UpMessageBox::Watch(this,"OK pour adresse1 et 2");
    Remplir_ListePatients_TableView(grequeteListe,"","");       // Moulinette()

    //Melange des noms des correspondants
    db->StandardSQL("update " TBL_CORRESPONDANTS " set CorNom = 'Porteix' where CorNom = 'Porte'");
    db->StandardSQL("update " TBL_CORRESPONDANTS " set CorNom = 'Longeix' where CorNom = 'Long'");
    QString Corcopierequete = "select idCor, CorNom from " TBL_CORRESPONDANTS;
    QList<QVariantList> corlist = db->StandardSelectSQL(Corcopierequete, ok);
    QStringList listnomcor;
    QList<QVariantList> patlist = db->StandardSelectSQL("select patnom from " TBL_PATIENTS " where patnom not in (select Cornom from " TBL_CORRESPONDANTS ") order by rand() limit " + QString::number(corlist.size()), ok);
    for (int e=0; e<corlist.size();e++)
        listnomcor <<  patlist.at(e).at(0).toString();
    QString Corimpr = "select idimpression, textcorps, textorigine from " TBL_IMPRESSIONS " where textcorps is not null";
    QList<QVariantList> corimprlist = db->StandardSelectSQL(Corimpr, ok);
    QString CorAct = "select idacte, actemotif, actetexte from " TBL_ACTES;
    QList<QVariantList> coractlist = db->StandardSelectSQL(CorAct,ok);
    for (int k = 0; k< corlist.size(); k++)
    {
        QString AncNom, NouvNom, NouvAdresse, idCor;
        idCor       = corlist.at(k).at(0).toString();
        AncNom      = corlist.at(k).at(1).toString();
        NouvNom     = listnomcor.at(k);
        Corcopierequete = "update " TBL_CORRESPONDANTS " set Cornom = '" + Utils::correctquoteSQL(NouvNom) + "' where idCor = " + idCor;
        db->StandardSQL(Corcopierequete);

        for (int p=0; p<corimprlist.size(); p++)
        {
            if (corimprlist.at(p).at(1).toString().contains(AncNom, Qt::CaseInsensitive))
            {
                //qDebug() << AncNom + " - " + QString::number(k) + "/" + QString::number(Corcopie.size()) + " // " + QString::number(p) + "/" + QString::number(Corimprquery.size());
                db->StandardSQL("update " TBL_IMPRESSIONS " set textcorps = '" + Utils::correctquoteSQL(corimprlist.at(p).at(1).toString().replace(AncNom,NouvNom))
                           + "' where idimpression = " + corimprlist.at(p).at(0).toString());
            }
            if (corimprlist.at(p).at(2).toString().contains(AncNom, Qt::CaseInsensitive))
            {
                db->StandardSQL("update " TBL_IMPRESSIONS " set textorigine = '" + Utils::correctquoteSQL(corimprlist.at(p).at(2).toString().replace(AncNom,NouvNom))
                           + "' where idimpression = " + corimprlist.at(p).at(0).toString());
            }
        }

        for (int q=0; q<coractlist.size(); q++)
        {
            if (coractlist.at(q).at(1).toString().contains(AncNom, Qt::CaseInsensitive))
            {
                //qDebug() << AncNom + " - " + QString::number(k) + "/" + QString::number(Corcopie.size()) + " // " + QString::number(q) + "/" + QString::number(CorActquery.size());
                db->StandardSQL("update " TBL_ACTES " set actemotif = 'Courrier efffacé' where idacte = " + coractlist.at(q).at(0).toString());
            }
            if (coractlist.at(q).at(2).toString().contains(AncNom, Qt::CaseInsensitive))
            {
                db->StandardSQL("update " TBL_ACTES " set actetexte = 'Courrier effacé' where idacte = " + coractlist.at(q).at(0).toString());
            }
        }

        QVariantList adrdata = db->getFirstRecordFromStandardSelectSQL("select patadresse1 from " TBL_DONNEESSOCIALESPATIENTS " order by rand() limit 1",ok);
        NouvAdresse = adrdata.at(0).toString();
        Corcopierequete = "update rufus.correspondants set Coradresse1 = '" + Utils::correctquoteSQL(NouvAdresse) + "' where idCor = " + idCor;
        db->StandardSQL(Corcopierequete);
    }
    UpMessageBox::Watch(this,"OK pour Correspondants");
    */
}

void Rufus::ActeMontantModifie()
{
    QString b = QLocale().toString(QLocale().toDouble(ui->ActeMontantlineEdit->text()),'f',2);
    if (b != gActeMontant)
        ValideActeMontantLineEdit(b, gActeMontant);  // ActeMontantModifie()
}

/*------------------------------------------------------------------------------------------------------------------------------------
-- Afficher le motif de l'acte ----------------------------------------------------------------
------------------------------------------------------------------------------------------------------------------------------------*/
void Rufus::AfficheMotif(UpLabel *lbl)
{
    QMap<QString, QVariant> rsgnmt = lbl->getData();
    QString Msg("");
    Msg += Utils::CalculAge(rsgnmt["ddnpat"].toDate())["toString"].toString();
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
    int                 i;
    QString             NomPrenom, zw, A;
    QFontMetrics        fm(qApp->font());
    // PATIENTS VUS AUJOURD'HUI ---------------------------------------------------------------------------------------------------
    QString req =   "SELECT pat.IdPat, act.idacte, PatNom, PatPrenom, UserLogin, ActeDate, ActeCotation, ActeMontant, ActeHeure, TypePaiement, Tiers, usr.idUser FROM "
                           TBL_PATIENTS " as pat, " TBL_ACTES " as act, " TBL_UTILISATEURS " as usr, " TBL_TYPEPAIEMENTACTES " as typ"
                           " WHERE usr.idUser = act.idUser and act.idPat = pat.idPat and actedate = curdate()"
                           " and act.idPat not in (select idpat from " TBL_SALLEDATTENTE ")"
                           " and act.idActe = typ.idActe"
                           " ORDER BY ActeHeure DESC";
    QList<QVariantList> patlist = db->StandardSelectSQL(req, ok, tr("Impossible de remplir la salle d'attente!"));
    if (!ok)
        return;
    ui->PatientsVusupTableWidget->clearContents();
    ui->PatientsVusupTableWidget->setRowCount(patlist.size());

    for (i = 0; i < patlist.size(); i++)
    {
        QMap<QString, QVariant> rsgnmt;
        rsgnmt["idpat"] = patlist.at(i).at(0).toInt();
        rsgnmt["idsuperviseur"] = patlist.at(i).at(11).toInt();
        rsgnmt["loginsuperviseur"] = patlist.at(i).at(4).toString();

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

        NomPrenom = patlist.at(i).at(2).toString().toUpper()
                + " " + patlist.at(i).at(3).toString();
        zw = patlist.at(i).at(8).toTime().toString("HH:mm");
        if (zw == "") zw = "hors RDV";
        label0->setText(" " + zw);                                                          // Heure acte
        label1->setText(" " + NomPrenom);                                                   // Nom + Prénom
        QString P=patlist.at(i).at(9).toString();
        if (P == "E")           P = "Espèces";
        else if (P == "C")      P = "Chèque";
        else if (P == "I")      P = "Impayé";
        else if (P == "G")      P = "Gratuit";
        else if (P == "T")
        {
            P = patlist.at(i).at(10).toString();
            if (P == "CB")      P = "Carte";
        }
        label2->setText(" " + P);                                                           // Mode de paiement
        label3->setText(" " + patlist.at(i).at(6).toString());                              // Cotation
        label4->setText(QLocale().toString(patlist.at(i).at(7).toDouble(),'f',2) + " ");    // Montant

        label0->setAlignment(Qt::AlignLeft);
        label1->setAlignment(Qt::AlignLeft);
        label3->setAlignment(Qt::AlignLeft);
        label4->setAlignment(Qt::AlignRight);

        connect (label0,        &QWidget::customContextMenuRequested,       this,   [=] {gTimerPatientsVus->start(); MenuContextuelSalDat(label0);});
        connect (label1,        &QWidget::customContextMenuRequested,       this,   [=] {gTimerPatientsVus->start(); MenuContextuelSalDat(label1);});
        connect (label2,        &QWidget::customContextMenuRequested,       this,   [=] {gTimerPatientsVus->start(); MenuContextuelSalDat(label2);});
        connect (label3,        &QWidget::customContextMenuRequested,       this,   [=] {gTimerPatientsVus->start(); MenuContextuelSalDat(label3);});
        connect (label4,        &QWidget::customContextMenuRequested,       this,   [=] {gTimerPatientsVus->start(); MenuContextuelSalDat(label4);});
        connect (label0,        &UpLabel::enter,            this,                   [=] {gTimerPatientsVus->start(); AfficheToolTip(m_patients->getById(rsgnmt["idpat"].toInt()));});
        connect (label1,        &UpLabel::enter,            this,                   [=] {gTimerPatientsVus->start(); AfficheToolTip(m_patients->getById(rsgnmt["idpat"].toInt()));});
        connect (label2,        &UpLabel::enter,            this,                   [=] {gTimerPatientsVus->start(); AfficheToolTip(m_patients->getById(rsgnmt["idpat"].toInt()));});
        connect (label3,        &UpLabel::enter,            this,                   [=] {gTimerPatientsVus->start(); AfficheToolTip(m_patients->getById(rsgnmt["idpat"].toInt()));});
        connect (label4,        &UpLabel::enter,            this,                   [=] {gTimerPatientsVus->start(); AfficheToolTip(m_patients->getById(rsgnmt["idpat"].toInt()));});
        connect (label0,        &UpLabel::dblclick,         this,                   [=] {if (m_currentuser->isSoignant()) ChoixDossier(m_patients->getById(rsgnmt["idpat"].toInt()));});
        connect (label1,        &UpLabel::dblclick,         this,                   [=] {if (m_currentuser->isSoignant()) ChoixDossier(m_patients->getById(rsgnmt["idpat"].toInt()));});
        connect (label2,        &UpLabel::dblclick,         this,                   [=] {if (m_currentuser->isSoignant()) ChoixDossier(m_patients->getById(rsgnmt["idpat"].toInt()));});
        connect (label3,        &UpLabel::dblclick,         this,                   [=] {if (m_currentuser->isSoignant()) ChoixDossier(m_patients->getById(rsgnmt["idpat"].toInt()));});
        connect (label4,        &UpLabel::dblclick,         this,                   [=] {if (m_currentuser->isSoignant()) ChoixDossier(m_patients->getById(rsgnmt["idpat"].toInt()));});

        ui->PatientsVusupTableWidget->setCellWidget(i,0,label0);
        ui->PatientsVusupTableWidget->setCellWidget(i,1,label1);
        ui->PatientsVusupTableWidget->setCellWidget(i,2,label2);
        ui->PatientsVusupTableWidget->setCellWidget(i,3,label3);
        ui->PatientsVusupTableWidget->setCellWidget(i,4,label4);
        ui->PatientsVusupTableWidget->setRowHeight(i,int(fm.height()*1.1));
    }
}

/*------------------------------------------------------------------------------------------------------------------------------------
    -- Afficher l'adresse du patient en toolTip ----------------------------------------------------------------
    ------------------------------------------------------------------------------------------------------------------------------------*/
void Rufus::AfficheToolTip(Patient *pat)
{
    if (pat == Q_NULLPTR)
        return;
    m_patients->loadAll(pat, Item::ForceUpdate);
    QString Msg = "";
    if (pat->datedenaissance().isValid())
        Msg += Utils::CalculAge(pat->datedenaissance())["toString"].toString();
    if (pat->ville() != "")
    {
        if (Msg!="") Msg = "\n" + Msg;
        Msg = pat->ville() + Msg;
    }
    if (pat->adresse3() != "")
    {
        if (Msg!="") Msg = "\n" + Msg;
        Msg = pat->adresse3() + Msg;
    }
    if (pat->adresse2() != "")
    {
        if (Msg!="") Msg = "\n" + Msg;
        Msg = pat->adresse2() + Msg;
    }
    if (pat->adresse1() != "")
    {
        if (Msg!="") Msg = "\n" + Msg;
        Msg = pat->adresse1() + Msg;
    }
    if (Msg!="") Msg += "\n";
    Msg += QString::number(pat->id());

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
    actionParametres->setEnabled(a);
    actionRechercheCourrier->setVisible(m_currentuser->isSoignant() && a);
    if (m_currentuser->isSoignant())
    {
        bool c;
        QString req = "select idActe from " TBL_ACTES " where ActeCourrierafaire = 'T' and idUser = " + QString::number(m_currentuser->id());
        c = (db->StandardSelectSQL(req, ok).size()>0);
        actionRechercheCourrier     ->setEnabled(a && c);
    }

    bool b = (ui->tabWidget->currentWidget() == ui->tabDossier);
    actionSupprimerActe->setVisible(b);
    actionCreerActe->setVisible(b);

    if (menu == menuDossier)
    {
        actionSupprimerDossier->setEnabled(true);
        actionSupprimerDossier->setVisible(ui->tabWidget->currentWidget() == ui->tabDossier && m_currentuser->isSoignant());
    }
    if (menu == menuDocuments)
    {
        menuDocuments->clear();
        if (ui->tabWidget->currentWidget() == ui->tabDossier)
        {
            menuDocuments       ->addMenu(menuEmettre);
            menuDocuments       ->addAction(actionEnregistrerDocScanner);
#ifdef Q_OS_MACX
            if (db->getMode() != DataBase::Distant)
                menuDocuments   ->addAction(actionEnregistrerVideo);
#endif
            menuDocuments       ->addSeparator();
            actionExportActe    ->setVisible(ui->Acteframe->isVisible());
        }
        menuDocuments->addAction(actionRechercheCourrier);
        menuDocuments->addAction(actionCorrespondants);
    }
}

void Rufus::AppelPaiementDirect(Origin origin)
{
    QList<int> ListidActeAPasser;
    if (origin == Accueil)    // l'appel est fait par un clic dans le menu contextuel de la salle d'attente des paiements en attente
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
                QString req = "SELECT idActe FROM "  TBL_VERROUCOMPTAACTES
                              " WHERE idActe = "  + ui->AccueilupTableWidget->item(debut+k,5)->text();
                QVariantList actdata = db->getFirstRecordFromStandardSelectSQL(req, ok);
                if (!ok)
                    return;
                if (actdata.size()==0)
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
    else if (origin == BoutonPaiement)        // l'appel est fait par un clic sur le bouton enregistrepaiement
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
        QString req = "SELECT UserLogin FROM " TBL_VERROUCOMPTAACTES ", " TBL_UTILISATEURS
                      " WHERE idActe = "  + QString::number(m_currentact->id()) +
                      " AND PosePar = idUser";
        QVariantList verroudata = db->getFirstRecordFromStandardSelectSQL(req, ok);
        if (!ok)
            return;
        if (verroudata.size()>0)
        {
            UpMessageBox::Watch(this,tr("Vous ne pouvez paz enregistrer le paiement de cet acte !"),
                                tr("Il est déjà en cours d'enregistrement par ") + verroudata.at(0).toString());
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
                QString enreggratuit = "INSERT INTO " TBL_TYPEPAIEMENTACTES " (idActe, TypePaiement) VALUES (" + QString::number(m_currentact->id()) + ",'G')";
                db->StandardSQL(enreggratuit, tr("Impossible d'enregister cet acte comme gratuit"));
                AfficheActeCompta(m_currentact);
            }
            return;
        }

        QString ActeSal = QString::number(m_currentact->id());
        QString Msg;
        PatientEnCours *pat = m_listepatientsencours->getById(Datas::I()->patients->currentpatient()->id());
        if (pat == Q_NULLPTR)
            m_listepatientsencours->CreationPatient(Datas::I()->patients->currentpatient()->id(),                             //! idPat
                                                     m_currentuser->idsuperviseur(),         //! idUser
                                                     RETOURACCUEIL,                                     //! Statut
                                                     db->ServerDateTime().time(),                       //! heureStatut
                                                     QTime(),                                           //! heureRDV
                                                     QTime(),                                           //! heureArrivee
                                                     "",                                                //! Motif
                                                     "",                                                //! Message
                                                     ActeSal.toInt(),                                   //! idActeAPayer
                                                     "",                                                //! PosteExamen
                                                     0,                                                 //! idUserEnCoursExamen
                                                     0);                                                //! idSalDat
        else
        {
            ItemsList::update(pat, CP_STATUT_SALDAT, RETOURACCUEIL);
            ItemsList::update(pat, CP_IDACTEAPAYER_SALDAT, ActeSal.toInt());
            ItemsList::update(pat, CP_MESSAGE_SALDAT, Msg);
            ItemsList::update(pat, CP_HEURESTATUT_SALDAT, db->ServerDateTime().time());
            ItemsList::update(pat, CP_IDUSERENCOURSEXAM_SALDAT);
            ItemsList::update(pat, CP_POSTEEXAMEN_SALDAT);
        }
        Flags::I()->MAJFlagSalleDAttente();
        ListidActeAPasser << m_currentact->id();
    }

    Dlg_PmtDirect = new dlg_paiementdirect(ListidActeAPasser, this);//NOTE : New Paiement
    if(Dlg_PmtDirect->getInitOK())
        Dlg_PmtDirect->exec();
    if (origin == BoutonPaiement)  // on redonne le statut en cours d'examen au dossier
    {
        QString Msg;
        PatientEnCours *pat = m_listepatientsencours->getById(Datas::I()->patients->currentpatient()->id());
        if (pat == Q_NULLPTR)
            m_listepatientsencours->CreationPatient(Datas::I()->patients->currentpatient()->id(),                             //! idPat
                                                     m_currentuser->idsuperviseur(),         //! idUser
                                                     ENCOURSEXAMEN + m_currentuser->login(),         //! Statut
                                                     db->ServerDateTime().time(),                       //! heureStatut
                                                     QTime(),                                           //! heureRDV
                                                     QTime(),                                           //! heureArrivee
                                                     "",                                                //! Motif
                                                     "",                                                //! Message
                                                     0,                                                 //! idActeAPayer
                                                     QHostInfo::localHostName().left(60),               //! PosteExamen
                                                     m_currentuser->id(),                               //! idUserEnCoursExamen
                                                     0);                                                //! idSalDat
        else
        {
            ItemsList::update(pat, CP_STATUT_SALDAT, ENCOURSEXAMEN + m_currentuser->login());
            ItemsList::update(pat, CP_HEURESTATUT_SALDAT, db->ServerDateTime().time());
            ItemsList::update(pat, CP_IDUSERENCOURSEXAM_SALDAT, m_currentuser->id());
            ItemsList::update(pat, CP_POSTEEXAMEN_SALDAT, QHostInfo::localHostName().left(60));
        }
        Flags::I()->MAJFlagSalleDAttente();
    }
    if (Datas::I()->patients->currentpatient()->id() > 0)
    {
        m_lignespaiements->initListeByPatient(Datas::I()->patients->currentpatient());
        if (m_currentact->id()>0)
        {
            m_listeactes->initListeByPatient(Datas::I()->patients->currentpatient(), Item::ForceUpdate);
            if (ui->tabDossier->isVisible())
                AfficheActeCompta(m_currentact);
        }
    }
}

void Rufus::AppelPaiementTiers()
{
    QList<dlg_paiementtiers *> PaimtList = findChildren<dlg_paiementtiers*>();
    if (PaimtList.size()>0)
        for (int i=0; i<PaimtList.size();i++)
            if (PaimtList.at(i)->isVisible())
            {
                QSound::play(NOM_ALARME);
                PaimtList.at(i)->raise();
                return;
            }
    Dlg_PmtTiers = new dlg_paiementtiers(this); //NOTE : New Paiement
    if(Dlg_PmtTiers->getInitOK())
    {
        Dlg_PmtTiers->setWindowTitle(tr("Gestion des tiers payants"));
        Dlg_PmtTiers->show();
        connect(Dlg_PmtTiers, &QDialog::finished, this, [=]{
            if (Datas::I()->patients->currentpatient()->id() > 0)
            {
                m_lignespaiements->initListeByPatient(Datas::I()->patients->currentpatient());
                if (m_currentact->id()>0 && ui->tabDossier->isVisible())
                    AfficheActeCompta(m_currentact);
            }
        });
    }
}

void Rufus::AutreDossier(Patient *pat)
{
    if(m_currentuser->isSoignant())
    {
        Datas::I()->patients->setdossierpatientaouvrir(pat->id());
        ChoixMenuContextuelListePatients(tr("Autre Dossier"));
    }
}

void Rufus::BasculerMontantActe()
{
    gActeMontant = QLocale().toString(QLocale().toDouble(ui->ActeMontantlineEdit->text()),'f',2);
    int idx = ui->ActeCotationcomboBox->findText(ui->ActeCotationcomboBox->currentText());
    if (idx>-1)
    {
        QStringList listMontantActe = ui->ActeCotationcomboBox->itemData(idx).toStringList();
        double MontantConv, MontantPrat, MontantActe;
        MontantActe = QLocale().toDouble(ui->ActeMontantlineEdit->text());
        MontantConv = listMontantActe.at(0).toDouble();
        MontantPrat = listMontantActe.at(1).toDouble();

        if (fabs(MontantActe)!=fabs(MontantPrat))
        {
            ui->ActeMontantlineEdit->setText(QLocale().toString(MontantPrat,'f',2));
            ui->BasculerMontantpushButton->setImmediateToolTip(tr("Revenir au tarif conventionnel"));
        }
        else
        {

            ui->ActeMontantlineEdit->setText(QLocale().toString(MontantConv,'f',2));
            ui->BasculerMontantpushButton->setImmediateToolTip(tr("Revenir au tarif habituellement pratiqué"));
        }
        ValideActeMontantLineEdit(ui->ActeMontantlineEdit->text(), gActeMontant);       //BasculerMontantActe()
    }
}

void Rufus::BilanRecettes()
{   
    Dlg_BilanRec            = new dlg_bilanrecettes();
    if (!Dlg_BilanRec->getInitOK())
    {
        delete Dlg_BilanRec;
        return;
    }
    Dlg_BilanRec->exec();
    Dlg_BilanRec->close(); // nécessaire pour enregistrer la géométrie
    delete Dlg_BilanRec;
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
    //ItemsList::update(Datas::I()->patients->currentpatient(), CP_IDMG_RMP,ui->MGupComboBox->currentData().toInt());
    Datas::I()->patients->updateCorrespondant(Datas::I()->patients->currentpatient(), Correspondant::MG, Datas::I()->correspondants->getById(ui->MGupComboBox->currentData().toInt()));
    OKModifierTerrain(Datas::I()->patients->currentpatient(), false);
    ui->MGupComboBox->setImmediateToolTip(CalcToolTipCorrespondant(ui->MGupComboBox->currentData().toInt()));
}

/*-----------------------------------------------------------------------------------------------------------------
-- Rechercher le nom dans la listePatients par l'id ---------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void Rufus::CherchePatientParID(int id)
{
    Patient *pat = m_patients->getById(id);
    if (pat != Q_NULLPTR)
    {
        ui->CreerNomlineEdit->setText(pat->nom());
        ui->CreerPrenomlineEdit->setText(pat->prenom());
        lblnom->setText(pat->nom().toUpper() + " " + pat->prenom());
        RecaleTableView(pat);
    }
    else
    {
        ui->CreerNomlineEdit->clear();
        ui->CreerPrenomlineEdit->clear();
        lblnom->clear();
    }
}

void Rufus::ChoixCor(UpComboBox *box)
{
    QString idcor;
    if (box==ui->AutresCorresp1upComboBox)
        Datas::I()->patients->updateCorrespondant(Datas::I()->patients->currentpatient(), Correspondant::Spe1, Datas::I()->correspondants->getById(box->currentData().toInt()));
    else if (box==ui->AutresCorresp2upComboBox)
        Datas::I()->patients->updateCorrespondant(Datas::I()->patients->currentpatient(), Correspondant::Spe2, Datas::I()->correspondants->getById(box->currentData().toInt()));
     box->setImmediateToolTip(CalcToolTipCorrespondant(box->currentData().toInt()));
}
/* -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    tous les timers sont déconnectés pendant les procédures de sauvegarde de la base ---------------------------------------------------------------------------------------------------------------------------
 -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void Rufus::ConnectTimers(bool a)
{
    if (a)
    {
        if (db->getMode() == DataBase::Distant)
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
        gTimerPosteConnecte  ->start(10000);
        gTimerVerifVerrou   ->start(60000);

        connect (gTimerPosteConnecte,               &QTimer::timeout,   this,   &Rufus::MAJPosteConnecte);
        connect (gTimerActualiseDocsExternes,       &QTimer::timeout,   this,   &Rufus::ActualiseDocsExternes);
        if (!UtiliseTCP)
        {
            connect (gTimerSalDat,                  &QTimer::timeout,   this,   &Rufus::VerifSalleDAttente);
            connect (gTimerCorrespondants,          &QTimer::timeout,   this,   &Rufus::VerifCorrespondants);
            connect (gTimerVerifVerrou,             &QTimer::timeout,   this,   &Rufus::VerifVerrouDossier);
            connect (gTimerVerifMessages,           &QTimer::timeout,   this,   &Rufus::VerifMessages);
            connect (gTimerVerifImportateurDocs,    &QTimer::timeout,   this,   &Rufus::VerifImportateur);
            connect (gTimerImportDocsExternes,      &QTimer::timeout,   this,   &Rufus::ImportDocsExternes);
            if (db->getMode() != DataBase::Distant)
                connect(gTimerSupprDocs,                &QTimer::timeout,   this,   &Rufus::SupprimerDocsEtFactures);
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
        gTimerPosteConnecte          ->disconnect();
        gTimerVerifVerrou           ->disconnect();
        gTimerSupprDocs             ->disconnect();
        gTimerVerifImportateurDocs  ->stop();
        gTimerSalDat                ->stop();
        gTimerCorrespondants        ->stop();
        gTimerExportDocs            ->stop();
        gTimerActualiseDocsExternes ->stop();
        gTimerImportDocsExternes    ->stop();
        gTimerVerifMessages         ->stop();
        gTimerPosteConnecte         ->stop();
        gTimerVerifVerrou           ->stop();
        gTimerSupprDocs             ->stop();
    }
}

void Rufus::CourrierAFaireChecked()
{
    QString cr = (ui->CourrierAFairecheckBox->isChecked()? "T" :"");
    ItemsList::update(m_currentact, CP_COURRIERAFAIRE_ACTES, cr);
}

/*------------------------------------------------------------------------------------------------------------------
-- création - gestion des bilans orthoptiques ----------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void Rufus::CreerBilanOrtho(Patient *pat)
{
    bool    nouveauBO       = true;
    bool    creeracte       = true;
    QDate DateBl;
    if (pat == Q_NULLPTR)
        return;
    if (ui->Acteframe->isVisible() && m_currentact != Q_NULLPTR)
    {
        QString requete = "select idbilanortho from " TBL_BILANORTHO
                " where idbilanortho = " + QString::number(m_currentact->id());
        QVariantList bodata = db->getFirstRecordFromStandardSelectSQL(requete, ok);
        if (!ok)
            return;
        nouveauBO = (bodata.size()<1);
        if (!nouveauBO)
        {
            DateBl = m_currentact->date();
            if (DateBl != QDate::currentDate())
            {
                UpMessageBox msgbox;
                UpSmallButton ReprendreBOBouton(tr("Reprendre"));
                UpSmallButton NouveauBOBouton(tr("Créer un nouveau\nbilan orthoptique"));
                UpSmallButton AnnulBouton(tr("Annuler"));
                msgbox.setText("Euuhh... " + m_currentuser->login());
                msgbox.setInformativeText(tr("Voulez-vous reprendre le bilan affiché\nou créer un nouveau bilan à la date d'aujourd'hui?"));
                msgbox.setIcon(UpMessageBox::Quest);
                msgbox.addButton(&AnnulBouton, UpSmallButton::CANCELBUTTON);
                msgbox.addButton(&NouveauBOBouton, UpSmallButton::COPYBUTTON);
                msgbox.addButton(&ReprendreBOBouton, UpSmallButton::STARTBUTTON);
                msgbox.exec();
                if (msgbox.clickedButton() != &ReprendreBOBouton && msgbox.clickedButton() != &NouveauBOBouton)
                    return;
                nouveauBO = (msgbox.clickedButton() == &NouveauBOBouton);
                creeracte = (msgbox.clickedButton() == &NouveauBOBouton);
                msgbox.close();
            }
            else if (!Datas::I()->users->getById(m_currentact->idCreatedBy())->isOrthoptist())
            {
                nouveauBO = true;
                creeracte = false;
            }
        }
        else
            creeracte = !Datas::I()->users->getById(m_currentact->idCreatedBy())->isOrthoptist();
    }

    if (!nouveauBO)
    {
        Dlg_BlOrtho             = new dlg_bilanortho(m_currentact, nouveauBO);
        QString Titre           = tr("Bilan orthoptique - ") + pat->prenom() + " " + pat->nom();
        Dlg_BlOrtho->ui->OcclAlternlabel    ->setVisible(pat->datedenaissance().daysTo(DateBl) < 730);
        Dlg_BlOrtho->ui->OcclAlterncomboBox ->setVisible(pat->datedenaissance().daysTo(DateBl) < 730);
        Dlg_BlOrtho->ui->MotiftextEdit      ->setHtml(ui->ActeMotiftextEdit->toHtml());
        Dlg_BlOrtho->ui->ConclusiontextEdit ->setHtml(ui->ActeConclusiontextEdit->toHtml());
        Dlg_BlOrtho             ->setWindowTitle(Titre);
        Dlg_BlOrtho             ->setDateBO(QDate::fromString(ui->ActeDatedateEdit->text(),"dd/MM/yyyy"));
    }
    else
    {
        if (creeracte)
        {
            if (ui->Acteframe->isVisible())
                if (!AutorDepartConsult(false)) return;
            CreerActe(pat);
        }
        Dlg_BlOrtho             = new dlg_bilanortho(m_currentact, nouveauBO);
        QString Titre           = tr("Bilan orthoptique - ") + pat->prenom() + " " + pat->nom();
        Dlg_BlOrtho             ->setWindowTitle(Titre);
        QString RefractionOD    = "";
        QString RefractionOG    = "";
        Dlg_BlOrtho             ->setDateBO(QDate::currentDate());

        QMapIterator<int, Refraction*> itref (*Datas::I()->refractions->refractions());
        itref.toBack();
        while (itref.hasPrevious()) {
            itref.previous();
            Refraction* ref = const_cast<Refraction*>(itref.value());
            if (ref->mesure() == Refraction::Acuite)
            {
                if (ref->formuleOD() != "" && RefractionOD == "")
                    RefractionOD = ref->formuleOD();
                if (ref->formuleOG() != "" && RefractionOG == "")
                    RefractionOG = ref->formuleOG();
                if (RefractionOG != "" && RefractionOD != "")
                    itref.toFront();
            }
        }
        if (RefractionOD != "")     Dlg_BlOrtho->ui->AVODlineEdit->setText(RefractionOD);
        if (RefractionOG != "")     Dlg_BlOrtho->ui->AVOGlineEdit->setText(RefractionOG);
        Dlg_BlOrtho->ui->OcclAlternlabel->setVisible(pat->datedenaissance().daysTo(QDate::currentDate()) < 730);
        Dlg_BlOrtho->ui->OcclAlterncomboBox->setVisible(pat->datedenaissance().daysTo(QDate::currentDate()) < 730);
    }
    if (Dlg_BlOrtho->exec()> 0)
    {
        QString updaterequete;
        QString const paragraph         = "<p style = \"margin-top:0px; margin-bottom:0px;\">";
        QString const debutdelimiter    = "<a name=\"BODEBUT\"></a>";
        QString const findelimiter      = "<a name=\"BOFIN\"></a>";

        // Compléter le champ Motif et mettre à jour l'affichage de ActeMotiftextEdit
        QString Motif                   = Dlg_BlOrtho->ui->MotiftextEdit->toPlainText();
        Motif                           .insert(Motif.length()-2, findelimiter);
        Motif                           = paragraph + debutdelimiter + Motif + "</a></p>";
        ui->ActeMotiftextEdit           ->setText(Motif);
        ItemsList::update(m_currentact, CP_MOTIF_ACTES, Motif);

        // Compléter le Champ Texte et mettre à jour l'affichage de ActeTextetextEdit
        QString Reponse                 = Dlg_BlOrtho->calcReponsehTml(ui->ActeTextetextEdit->toHtml());
        ui->ActeTextetextEdit           ->setHtml(Reponse);
        ItemsList::update(m_currentact, CP_TEXTE_ACTES, Reponse);

        // Compléter le Champ Conclusion et mettre à jour l'affichage de ActeConclusiontextEdit
        QString Concl                   = Dlg_BlOrtho->ui->ConclusiontextEdit->toPlainText();
        Concl                           .insert(Concl.length()-2, findelimiter);
        Concl                           = paragraph + debutdelimiter + Concl + "</p>";
        ui->ActeConclusiontextEdit      ->setText(Concl);
        ItemsList::update(m_currentact, CP_CONCLUSION_ACTES, Concl);

        //! Mettre à jour la table bilanortho
        QString deleteblorthorequete = "delete from " TBL_BILANORTHO " where idBilanOrtho = " + ui->idActelineEdit->text();
        QString bilanorthorequete = "insert into " TBL_BILANORTHO
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
        bilanorthorequete += QString::number(m_currentact->id());                                                                          //0 idActe
        bilanorthorequete += ", '" + Utils::correctquoteSQL(Motif) + "'\n";                           //1 Motif
        bilanorthorequete += ", '" + Utils::correctquoteSQL(Dlg_BlOrtho->ui->AVODlineEdit->text()) + "'\n";                     //2
        bilanorthorequete += ", '" + Utils::correctquoteSQL(Dlg_BlOrtho->ui->AVOGlineEdit->text()) + "'\n";                     //3
        bilanorthorequete += ", '" + Utils::correctquoteSQL(Dlg_BlOrtho->ui->OcclAlterncomboBox->currentText()) + "'\n";        //4
        bilanorthorequete += ", '" + Utils::correctquoteSQL(Dlg_BlOrtho->ui->WirtcomboBox->currentText()) + "'\n";              //5
        bilanorthorequete += ", '" + Utils::correctquoteSQL(Dlg_BlOrtho->ui->AnimauxWirtcomboBox->currentText()) + "'\n";       //6
        bilanorthorequete += ", '" + Utils::correctquoteSQL(Dlg_BlOrtho->ui->LangcomboBox->currentText()) + "'\n";              //7 Lang
        if (Dlg_BlOrtho->ui->ODdirecteurradioButton->isChecked())                                                               //8 ODirecteur
            bilanorthorequete += ", 'D'";
        else if (Dlg_BlOrtho->ui->OGdirecteurradioButton->isChecked())
            bilanorthorequete += ", 'G'";
        else
            bilanorthorequete += ", ''";
        if (Dlg_BlOrtho->ui->ODOrientationradioButton->isChecked())                                                               //9 Orientation
            bilanorthorequete += ", 'D'";
        else if (Dlg_BlOrtho->ui->OGOrientationradioButton->isChecked())
            bilanorthorequete += ", 'G'";
        else
            bilanorthorequete += ", ''";
        bilanorthorequete += ", '" + Utils::correctquoteSQL(Dlg_BlOrtho->ui->EcranVLSCcomboBox->currentText()) + "'\n";          //10 EcranVLSC
        if (Dlg_BlOrtho->ui->EcranVLSCDcomboBox->currentText() != "-")                                                            //11 EcranVLSCD
            bilanorthorequete += ", " + Utils::correctquoteSQL(Dlg_BlOrtho->ui->EcranVLSCDcomboBox->currentText()) + "\n";
        else
            bilanorthorequete += ", null";
        if (Dlg_BlOrtho->ui->fixSCVLcomboBox->currentText() != "-")                                                               //12 EcranfixresVLSC
            bilanorthorequete += ", '" + Utils::correctquoteSQL(Dlg_BlOrtho->ui->fixSCVLcomboBox->currentText()) + "'\n";
        else
            bilanorthorequete += ", null";
        bilanorthorequete += ", '" + Utils::correctquoteSQL(Dlg_BlOrtho->ui->EcranVPSCcomboBox->currentText()) + "'\n";          //13 EcranVPSC
        if (Dlg_BlOrtho->ui->EcranVPSCDcomboBox->currentText() != "-")                                                            //14 ECranVPSCD
            bilanorthorequete += ", " + Utils::correctquoteSQL(Dlg_BlOrtho->ui->EcranVPSCDcomboBox->currentText()) + "\n";
        else
            bilanorthorequete += ", null";
        if (Dlg_BlOrtho->ui->fixSCVPcomboBox->currentText() != "-")                                                               //15 EcranfixresVPSC
            bilanorthorequete += ", '" + Utils::correctquoteSQL(Dlg_BlOrtho->ui->fixSCVPcomboBox->currentText()) + "'\n";
        else
            bilanorthorequete += ", null";
        bilanorthorequete += ", '" + Utils::correctquoteSQL(Dlg_BlOrtho->ui->EcranVLASCcomboBox->currentText()) + "'\n";         //16 EcranVLASC
        if (Dlg_BlOrtho->ui->EcranVLASCDcomboBox->currentText() != "-")                                                           //17 EcranVLASCD
            bilanorthorequete += ", " + Utils::correctquoteSQL(Dlg_BlOrtho->ui->EcranVLASCDcomboBox->currentText()) + "\n";
        else
            bilanorthorequete += ", null";
        if (Dlg_BlOrtho->ui->fixASCVLcomboBox->currentText() != "-")                                                              //18 EcranfixresVLASC
            bilanorthorequete += ", '" + Utils::correctquoteSQL(Dlg_BlOrtho->ui->fixASCVLcomboBox->currentText()) + "'\n";
        else
            bilanorthorequete += ", null";
        bilanorthorequete += ", '" + Utils::correctquoteSQL(Dlg_BlOrtho->ui->EcranVPASCcomboBox->currentText()) + "'\n";         //19 EcranVPASC
        if (Dlg_BlOrtho->ui->EcranVPASCDcomboBox->currentText() != "-")                                                           //20 EcranVPASCD
            bilanorthorequete += ", " + Utils::correctquoteSQL(Dlg_BlOrtho->ui->EcranVPASCDcomboBox->currentText()) + "\n";
        else
            bilanorthorequete += ", null";
        if (Dlg_BlOrtho->ui->fixASCVPcomboBox->currentText() != "-")                                                              //21 EcranfixresVPASC
            bilanorthorequete += ", '" + Utils::correctquoteSQL(Dlg_BlOrtho->ui->fixASCVPcomboBox->currentText()) + "'\n";
        else
            bilanorthorequete += ", null";
        bilanorthorequete += ", '" + Utils::correctquoteSQL(Dlg_BlOrtho->ui->MaddoxVLSCcomboBox->currentText()) + "'\n";
        if (Dlg_BlOrtho->ui->MaddoxVLSCDcomboBox->currentText() != "-")
            bilanorthorequete += ", '" + Utils::correctquoteSQL(Dlg_BlOrtho->ui->MaddoxVLSCDcomboBox->currentText()) + "'\n";
        else
            bilanorthorequete += ", null";
        bilanorthorequete += ", '" + Utils::correctquoteSQL(Dlg_BlOrtho->ui->MaddoxVPSCcomboBox->currentText()) + "'\n";
        if (Dlg_BlOrtho->ui->MaddoxVPSCDcomboBox->currentText() != "-")
            bilanorthorequete += ", '" + Utils::correctquoteSQL(Dlg_BlOrtho->ui->MaddoxVPSCDcomboBox->currentText()) + "'\n";
        else
            bilanorthorequete += ", null";
        bilanorthorequete += ", '" + Utils::correctquoteSQL(Dlg_BlOrtho->ui->MaddoxVLASCcomboBox->currentText()) + "'\n";
        if (Dlg_BlOrtho->ui->MaddoxVLASCDcomboBox->currentText() != "-")
            bilanorthorequete += ", '" + Utils::correctquoteSQL(Dlg_BlOrtho->ui->MaddoxVLASCDcomboBox->currentText()) + "'\n";
        else
            bilanorthorequete += ", null";
        bilanorthorequete += ", '" + Utils::correctquoteSQL(Dlg_BlOrtho->ui->MaddoxVPASCcomboBox->currentText()) + "'\n";
        if (Dlg_BlOrtho->ui->MaddoxVPASCDcomboBox->currentText() != "-")
            bilanorthorequete += ", '" + Utils::correctquoteSQL(Dlg_BlOrtho->ui->MaddoxVPASCDcomboBox->currentText()) + "'\n";
        else
            bilanorthorequete += ", null";
        bilanorthorequete += ", '" + Utils::correctquoteSQL(Dlg_BlOrtho->ui->MotilitetextEdit->toHtml()) + "'\n";
        bilanorthorequete += ", '" + Utils::correctquoteSQL(Dlg_BlOrtho->ui->PPCcomboBox->currentText()) + "'\n";
        bilanorthorequete += ", '" + Utils::correctquoteSQL(Dlg_BlOrtho->ui->PPClineEdit->text()) + "'\n";
        bilanorthorequete += ", '" + Utils::correctquoteSQL(Dlg_BlOrtho->ui->SaccadeslineEdit->text()) + "'\n";
        bilanorthorequete += ", '" + Utils::correctquoteSQL(Dlg_BlOrtho->ui->PoursuitelineEdit->text()) + "'\n";
        bilanorthorequete += ", '" + Utils::correctquoteSQL(Dlg_BlOrtho->ui->Worth1lineEdit->text()) + "'\n";
        bilanorthorequete += ", '" + Utils::correctquoteSQL(Dlg_BlOrtho->ui->Worth2lineEdit->text()) + "'\n";
        bilanorthorequete += ", '" + Utils::correctquoteSQL(Dlg_BlOrtho->ui->Bagolini1lineEdit->text()) + "'\n";
        bilanorthorequete += ", '" + Utils::correctquoteSQL(Dlg_BlOrtho->ui->Bagolini2lineEdit->text()) + "'\n";
        bilanorthorequete += ", '" + Utils::correctquoteSQL(Dlg_BlOrtho->ui->VergenceDLcomboBox->currentText()) + "'\n";
        bilanorthorequete += ", '" + Utils::correctquoteSQL(Dlg_BlOrtho->ui->VergenceCLcomboBox->currentText()) + "'\n";
        bilanorthorequete += ", '" + Utils::correctquoteSQL(Dlg_BlOrtho->ui->VergenceDPcomboBox->currentText()) + "'\n";
        bilanorthorequete += ", '" + Utils::correctquoteSQL(Dlg_BlOrtho->ui->VergenceCPcomboBox->currentText()) + "'\n";
        bilanorthorequete += ", '" + Utils::correctquoteSQL(Dlg_BlOrtho->ui->Degre1lineEdit->text()) + "'\n";
        bilanorthorequete += ", '" + Utils::correctquoteSQL(Dlg_BlOrtho->ui->Degre2lineEdit->text()) + "'\n";
        bilanorthorequete += ", '" + Utils::correctquoteSQL(Dlg_BlOrtho->ui->Degre3lineEdit->text()) + "'\n";
        bilanorthorequete += ", '" + Utils::correctquoteSQL(Concl) + "'\n";
        bilanorthorequete += ", '" + Utils::correctquoteSQL(Dlg_BlOrtho->ui->TNOcomboBox->currentText()) + "'\n";
        bilanorthorequete += ", '" + Utils::correctquoteSQL(Dlg_BlOrtho->ui->VergenceRestDLcomboBox->currentText()) + "'\n";
        bilanorthorequete += ", '" + Utils::correctquoteSQL(Dlg_BlOrtho->ui->VergenceRestDPcomboBox->currentText()) + "'\n";
        bilanorthorequete += ", '" + Utils::correctquoteSQL(Dlg_BlOrtho->ui->HEcranVLSCcomboBox->currentText()) + "'\n";         //50 HEcranVLSC
        if (Dlg_BlOrtho->ui->HEcranVLSCDcomboBox->currentText() != "-")                                                           //51 HEcranVLSCD
            bilanorthorequete += ", " + Utils::correctquoteSQL(Dlg_BlOrtho->ui->HEcranVLSCDcomboBox->currentText()) + "\n";
        else
            bilanorthorequete += ", null";
        if (Dlg_BlOrtho->ui->HfixSCVLcomboBox->currentText() != "-")                                                              //52 HEcranfixresVLSC
            bilanorthorequete += ", '" + Utils::correctquoteSQL(Dlg_BlOrtho->ui->HfixSCVLcomboBox->currentText()) + "'\n";
        else
            bilanorthorequete += ", null";
        bilanorthorequete += ", '" + Utils::correctquoteSQL(Dlg_BlOrtho->ui->HEcranVPSCcomboBox->currentText()) + "'\n";         //53 HEcranVPSC
        if (Dlg_BlOrtho->ui->HEcranVPSCDcomboBox->currentText() != "-")                                                           //54 HECranVPSCD
            bilanorthorequete += ", " + Utils::correctquoteSQL(Dlg_BlOrtho->ui->HEcranVPSCDcomboBox->currentText()) + "\n";
        else
            bilanorthorequete += ", null";
        if (Dlg_BlOrtho->ui->HfixSCVPcomboBox->currentText() != "-")                                                              //55 HEcranfixresVPSC
            bilanorthorequete += ", '" + Utils::correctquoteSQL(Dlg_BlOrtho->ui->HfixSCVPcomboBox->currentText()) + "'\n";
        else
            bilanorthorequete += ", null";
        bilanorthorequete += ", '" + Utils::correctquoteSQL(Dlg_BlOrtho->ui->HEcranVLASCcomboBox->currentText()) + "'\n";        //56 HEcranVLASC
        if (Dlg_BlOrtho->ui->HEcranVLASCDcomboBox->currentText() != "-")                                                          //57 HEcranVLASCD
            bilanorthorequete += ", " + Utils::correctquoteSQL(Dlg_BlOrtho->ui->HEcranVLASCDcomboBox->currentText()) + "\n";
        else
            bilanorthorequete += ", null";
        if (Dlg_BlOrtho->ui->HfixASCVLcomboBox->currentText() != "-")                                                             //58 HEcranfixresVLASC
            bilanorthorequete += ", '" + Utils::correctquoteSQL(Dlg_BlOrtho->ui->HfixASCVLcomboBox->currentText()) + "'\n";
        else
            bilanorthorequete += ", null";
        bilanorthorequete += ", '" + Utils::correctquoteSQL(Dlg_BlOrtho->ui->HEcranVPASCcomboBox->currentText()) + "'\n";        //59 HEcranVPASC
        if (Dlg_BlOrtho->ui->HEcranVPASCDcomboBox->currentText() != "-")                                                          //60 HEcranVPASCD
            bilanorthorequete += ", " + Utils::correctquoteSQL(Dlg_BlOrtho->ui->HEcranVPASCDcomboBox->currentText()) + "\n";
        else
            bilanorthorequete += ", null";
        if (Dlg_BlOrtho->ui->HfixASCVPcomboBox->currentText() != "-")                                                             //61 HEcranfixresVPASC
            bilanorthorequete += ", '" + Utils::correctquoteSQL(Dlg_BlOrtho->ui->HfixASCVPcomboBox->currentText()) + "'\n";
        else
            bilanorthorequete += ", null";
        bilanorthorequete += ", '" + Utils::correctquoteSQL(Dlg_BlOrtho->ui->HMaddoxVLSCcomboBox->currentText()) + "'\n";        //62 HMaddoxVLSC
        if (Dlg_BlOrtho->ui->HMaddoxVLSCDcomboBox->currentText() != "-")                                                          //63 HMaddoxVLSCD
            bilanorthorequete += ", '" + Utils::correctquoteSQL(Dlg_BlOrtho->ui->HMaddoxVLSCDcomboBox->currentText()) + "'\n";
        else
            bilanorthorequete += ", null";
        bilanorthorequete += ", '" + Utils::correctquoteSQL(Dlg_BlOrtho->ui->HMaddoxVPSCcomboBox->currentText()) + "'\n";        //64 HMaddoxVPSC
        if (Dlg_BlOrtho->ui->HMaddoxVPSCDcomboBox->currentText() != "-")                                                          //65 HMaddoxVPSCD
            bilanorthorequete += ", '" + Utils::correctquoteSQL(Dlg_BlOrtho->ui->HMaddoxVPSCDcomboBox->currentText()) + "'\n";
        else
            bilanorthorequete += ", null";
        bilanorthorequete += ", '" + Utils::correctquoteSQL(Dlg_BlOrtho->ui->HMaddoxVLASCcomboBox->currentText()) + "'\n";       //66 HMaddoxVLASC
        if (Dlg_BlOrtho->ui->HMaddoxVLASCDcomboBox->currentText() != "-")                                                         //67 HMaddoxVLASCD
            bilanorthorequete += ", '" + Utils::correctquoteSQL(Dlg_BlOrtho->ui->HMaddoxVLASCDcomboBox->currentText()) + "'\n";
        else
            bilanorthorequete += ", null";
        bilanorthorequete += ", '" + Utils::correctquoteSQL(Dlg_BlOrtho->ui->HMaddoxVPASCcomboBox->currentText()) + "'\n";       //68 HMaddoxVPASC
        if (Dlg_BlOrtho->ui->HMaddoxVPASCDcomboBox->currentText() != "-")                                                         //69 HMaddoxVPASCD
            bilanorthorequete += ", '" + Utils::correctquoteSQL(Dlg_BlOrtho->ui->HMaddoxVPASCDcomboBox->currentText()) + "'\n";
        else
            bilanorthorequete += ", null";
        bilanorthorequete += ");";

        if (!db->createtransaction(QStringList() << TBL_BILANORTHO))
           return;
        if (!db->StandardSQL(deleteblorthorequete)){
            db->rollback(); return;}
        if (!db->StandardSQL(bilanorthorequete)){
            db->rollback(); return;}
        db->commit();

        ui->ActeTextetextEdit->setFocus();
        ui->ActeTextetextEdit->moveCursor(QTextCursor::End);
    }
    else
        if (nouveauBO) SupprimerActe(m_currentact);

    Dlg_BlOrtho->close();
    delete Dlg_BlOrtho;
    MAJDocsExternes();  //CreerBilanOrtho()
}

void Rufus::CreerDossierpushButtonClicked()
{
    if (gMode == NouveauDossier)
        CreerDossier();
    else if (ui->PatientsListeTableView->selectionModel()->selectedIndexes().size() > 0)
        ChoixDossier(getPatientFromSelectionInTable());
}

void Rufus::EnableButtons()
{
    if (gMode == Liste || gMode == RechercheDDN)
    {
        ui->LRecopierpushButton->setEnabled(ui->PatientsListeTableView->selectionModel()->selectedIndexes().size()>0);
    }
    else if (gMode == NouveauDossier)
    {
        ui->LRecopierpushButton->setEnabled(ui->PatientsListeTableView->selectionModel()->selectedIndexes().size()>0);
        ui->CreerDossierpushButton->setEnabled(ui->CreerNomlineEdit->text() != "" && ui->CreerPrenomlineEdit->text() != "");
    }
}

void Rufus::EnregistreDocScanner(Patient *pat)
{
    if (pat == Q_NULLPTR)
        return;
    Dlg_DocsScan = new dlg_docsscanner(pat, dlg_docsscanner::Document, "", this);
    if (!Dlg_DocsScan->getinitOK())
        return;
    Dlg_DocsScan->setWindowTitle(tr("Enregistrer un document issu du scanner pour ") + pat->nom().toUpper() + " " + pat->prenom());
    Dlg_DocsScan->exec();
    if (pat == Datas::I()->patients->currentpatient())
        MAJDocsExternes();
}

void Rufus::EnregistreVideo(Patient *pat)
{
    if (pat == Q_NULLPTR)
        return;
    Dlg_DocsVideo = new dlg_docsvideo(pat, this);
    Dlg_DocsVideo->setWindowTitle(tr("Enregistrer une video dans le dossier de ") + pat->nom().toUpper() + " " + pat->prenom());
    Dlg_DocsVideo->exec();
    if (pat == Datas::I()->patients->currentpatient())
        MAJDocsExternes();
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
    Dlg_Cmpt = new dlg_comptes(this);
    if(Dlg_Cmpt->getInitOK())
        Dlg_Cmpt->exec();
}

void Rufus::ExporteDocs()
{
    if (!isPosteImport())
        return;
    if (PasDExportPourLeMoment)
        return;
    QString NomDirStockageImagerie = proc->DirImagerie();

    if (!QDir(NomDirStockageImagerie).exists() || NomDirStockageImagerie == "")
    {
        QString msg = tr("Le dossier de sauvegarde d'imagerie") + " <font color=\"red\"><b>" + NomDirStockageImagerie + "</b></font>" + tr(" n'existe pas");
        msg += "<br />" + tr("Renseignez un dossier valide dans") + " <font color=\"green\"><b>" + tr("Emplacement de stockage des documents archivés") + "</b></font>";
        QStringList listmsg;
        listmsg << msg;
        dlg_message(listmsg, 6000, false);
        return;
    }
    QString CheminEchecTransfrDir   = NomDirStockageImagerie + DIR_ECHECSTRANSFERTS;
    if (!Utils::mkpath(CheminEchecTransfrDir))
    {
        QString msg = tr("Dossier de sauvegarde ") + "<font color=\"red\"><b>" + CheminEchecTransfrDir + "</b></font>" + tr(" invalide");
        QStringList listmsg;
        listmsg << msg;
        dlg_message(listmsg, 6000, false);        return;
    }

    int total = db->StandardSelectSQL("SELECT idimpression FROM " TBL_IMPRESSIONS " where jpg is not null or pdf is not null",ok).size();
    total +=    db->StandardSelectSQL("SELECT idFacture FROM " TBL_FACTURES " where jpg is not null or pdf is not null", ok).size();
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

/* LES DOCUMENTS  ============================================*/
    int faits = 0;
    QTime debut = QTime::currentTime();
    QStringList listmsg;
    QString duree;
    QString CheminOKTransfrDir      = NomDirStockageImagerie + DIR_IMAGES;
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

    //-----------------------------------------------------------------------------------------------------------------------------------------
    //              LES JPG
    //-----------------------------------------------------------------------------------------------------------------------------------------
        QString req = "SELECT idimpression, idpat, SousTypeDoc, Dateimpression, jpg, lienversfichier, typedoc FROM " TBL_IMPRESSIONS " where jpg is not null";
        //qDebug() << req;
        QList<QVariantList> listexportjpg = db->StandardSelectSQL(req, ok );
        if (ok)
            for (int i=0; i<listexportjpg.size(); i++)
            {
                /* si le lien vers le fichier est valide, on efface le champ jpg et on passe à la réponse suivante*/
                if (listexportjpg.at(i).at(5).toString() != "")
                {
                    QString CheminFichier = NomDirStockageImagerie + DIR_IMAGES + listexportjpg.at(i).at(5).toString();
                    if (QFile(CheminFichier).exists())
                    {
                        db->StandardSQL("update " TBL_IMPRESSIONS " set jpg = null where idimpression = " + listexportjpg.at(i).at(0).toString());
                        continue;
                    }
                }
                QDate datetransfer    = listexportjpg.at(i).at(3).toDate();
                CheminOKTransfrDir    = CheminOKTransfrDir + "/" + datetransfer.toString("yyyy-MM-dd");
                if (!QDir(CheminOKTransfrDir).exists())
                    if (!DirTrsferOK.mkdir(CheminOKTransfrDir))
                    {
                        QString msg = tr("Dossier de sauvegarde ") + "<font color=\"red\"><b>" + CheminOKTransfrDir + "</b></font>" + tr(" invalide");
                        QStringList listmsg;
                        listmsg << msg;
                        dlg_message(listmsg, 3000, false);
                        return;
                    }
                QString NomFileDoc = listexportjpg.at(i).at(1).toString() + "_" + listexportjpg.at(i).at(6).toString() + "-"
                        + listexportjpg.at(i).at(2).toString().replace("/",".") + "_"
                        + listexportjpg.at(i).at(3).toDate().toString("yyyyMMdd") + "-" + QTime::currentTime().toString("HHmmss")
                        + "-" + listexportjpg.at(i).at(0).toString()  + ".jpg";
                QString CheminOKTransfrDoc  = CheminOKTransfrDir + "/" + NomFileDoc + "." JPG;
                QString CheminOKTransfrProv = CheminOKTransfrDir + "/" + NomFileDoc + "prov." JPG;
                QByteArray ba = listexportjpg.at(i).at(6).toByteArray();
                QPixmap pix;
                pix.loadFromData(ba);
                /*
             * On utilise le passage par les QPixmap parce que le mèthode suivante consistant
             * à réintégrer le QByteArray directement dans le fichier aboutit à un fichier corrompu...
             * QFile prov (CheminOKTransfrProv);
                if (prov.open(QIODevice::Append))
                {
                    QTextStream out(&prov);
                    out << ba;
                }
            */
                if (!pix.save(CheminOKTransfrProv, "jpeg"))
                {
                    qDebug() << "erreur";
                    return;
                }
                if (!Utils::CompressFileJPG(CheminOKTransfrProv, proc->DirImagerie()))
                {
                    db->SupprRecordFromTable(listexportjpg.at(i).at(0).toInt(), "idFacture", TBL_FACTURES);
                    continue;
                }
                QFile prov(CheminOKTransfrProv);
                if (prov.open(QIODevice::ReadWrite))
                {
                    prov.copy(CheminOKTransfrDoc);
                    prov.remove();
                }
                else
                    return;
                db->StandardSQL("update " TBL_IMPRESSIONS " set jpg = null,"
                                " lienversfichier = '/" + datetransfer.toString("yyyy-MM-dd") + "/" + Utils::correctquoteSQL(NomFileDoc) +
                                "' where idimpression = " + listexportjpg.at(i).at(0).toString() );
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
    QString reqpdf = "SELECT idimpression, idpat, SousTypeDoc, Dateimpression, pdf, lienversfichier, compression, typedoc FROM " TBL_IMPRESSIONS " where pdf is not null";
    QList<QVariantList> listexportpdf = db->StandardSelectSQL(reqpdf, ok );
    if (ok)
        for (int i=0; i<listexportpdf.size(); i++)
        {
            if (listexportpdf.at(i).at(5).toString() != "")
            {
                QString CheminFichier = NomDirStockageImagerie + DIR_IMAGES + listexportpdf.at(i).at(5).toString();
                if (QFile(CheminFichier).exists())
                {
                    db->StandardSQL("update " TBL_IMPRESSIONS " set pdf = null where idimpression = " + listexportpdf.at(i).at(0).toString());
                    continue;
                }
            }
            QDate datetransfer    = listexportpdf.at(i).at(3).toDate();
            CheminOKTransfrDir      = CheminOKTransfrDir + "/" + datetransfer.toString("yyyy-MM-dd");
            if (!QDir(CheminOKTransfrDir).exists())
                if (!DirTrsferOK.mkdir(CheminOKTransfrDir))
                {
                    QString msg = tr("Dossier de sauvegarde ") + "<font color=\"red\"><b>" + CheminOKTransfrDir + "</b></font>" + tr(" invalide");
                    QStringList listmsg;
                    listmsg << msg;
                    dlg_message(listmsg, 3000, false);
                    return;
                }
            QString NomFileDoc = listexportpdf.at(i).at(1).toString() + "_" + listexportpdf.at(i).at(7).toString() + "-"
                    + listexportpdf.at(i).at(2).toString().replace("/",".") + "_"
                    + listexportpdf.at(i).at(3).toDate().toString("yyyyMMdd") + "-" + QTime::currentTime().toString("HHmmss")
                    + "-" + listexportpdf.at(i).at(0).toString()  + ".pdf";
            QString CheminOKTransfrDoc = CheminOKTransfrDir + "/" + NomFileDoc;

            QByteArray bapdf;
            bapdf.append(listexportpdf.at(i).at(4).toByteArray());

            Poppler::Document* document = Poppler::Document::loadFromData(bapdf);
            if (!document || document->isLocked() || document == Q_NULLPTR)
            {
                QStringList listmsg;
                listmsg << tr("Impossible de charger le document ") + NomFileDoc;
                dlg_message(listmsg, 3000, false);
                QString echectrsfername         = CheminEchecTransfrDir + "/0EchecTransferts - " + datetransfer.toString("yyyy-MM-dd") + ".txt";
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
                        out << listexportpdf.at(i).at(4).toByteArray() ;
                    }
                }
                QString delreq = "delete from  " TBL_IMPRESSIONS " where idimpression = " + listexportpdf.at(i).at(0).toString();
                //qDebug() << delreq;
                db->StandardSQL(delreq);
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
            db->StandardSQL("update " TBL_IMPRESSIONS " set pdf = null, compression = null,"
                            " lienversfichier = '/" + datetransfer.toString("yyyy-MM-dd") + "/" + Utils::correctquoteSQL(NomFileDoc)  + "'"
                            " where idimpression = " + listexportpdf.at(i).at(0).toString());
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
    int totdoc = listexportjpg.size() + listexportpdf.size();
    if (totdoc > 0)
    {
        listmsg <<  tr("export terminé") << QString::number(totdoc) + (totdoc>1? tr(" documents exportés en ") : tr(" document exporté en "))  + duree;
        dlg_message(listmsg, 3000);
    }




/* LES FACTURES  ============================================*/

    faits = 0;
    debut = QTime::currentTime();
    listmsg.clear();
    duree = "";
    CheminOKTransfrDir  = NomDirStockageImagerie + DIR_FACTURES;
    if (!QDir(CheminOKTransfrDir).exists())
        if (!DirTrsferOK.mkdir(CheminOKTransfrDir))
        {
            QString msg = tr("Dossier de sauvegarde ") + "<font color=\"red\"><b>" + CheminOKTransfrDir + "</b></font>" + tr(" invalide");
            QStringList listmsg;
            listmsg << msg;
            dlg_message(listmsg, 3000, false);
            return;
        }

    //-----------------------------------------------------------------------------------------------------------------------------------------
    //              LES JPG
    //-----------------------------------------------------------------------------------------------------------------------------------------
    req = "SELECT idFacture, DateFacture, LienFichier, Intitule, Echeancier, idDepense, jpg FROM " TBL_FACTURES
                  " where jpg is not null";
    //qDebug() << req;
    QList<QVariantList> listexportjpgfact = db->StandardSelectSQL(req, ok);
    if (ok)
        for (int i=0; i<listexportjpgfact.size(); i++)
        {
            /* si le lien vers le fichier est valide, on efface le champ jpg et on passe à la réponse suivante*/
            if (listexportjpgfact.at(i).at(2).toString() != "")
            {
                QString CheminFichier = NomDirStockageImagerie + DIR_FACTURES + listexportjpgfact.at(i).at(2).toString();
                if (QFile(CheminFichier).exists())
                {
                    db->StandardSQL("update " TBL_FACTURES " set jpg = null where idfacture = " + listexportjpgfact.at(i).at(0).toString());
                    continue;
                }
            }
            /* nommage d'un fichier facture
         * idFacture + "_" + "ECHEANCIER ou FACTURE" + "_" + Intitule + "_" + DateFacture + ( + "_" + iddepense si facture et pas échéancier)
         */
            QDate datetransfer  = listexportjpgfact.at(i).at(1).toDate();
            QString user;

            QString NomFileDoc = listexportjpgfact.at(i).at(0).toString() + "_"
                    + (listexportjpgfact.at(i).at(4).toInt()==1? ECHEANCIER : FACTURE) + "-"
                    + listexportjpgfact.at(i).at(3).toString().replace("/",".") + "_"
                    + datetransfer.toString("yyyyMMdd");
            // on recherche le user à l'origine de cette facture
            QList<QVariantList> Listeusr;
            if (listexportjpgfact.at(i).at(4).toInt()==1)          // c'est un échéancier
                req = "select dep.idUser, UserLogin from " TBL_DEPENSES " dep, " TBL_UTILISATEURS " usr"
                                                                                                              " where dep.idUser  = usr.idUser"
                                                                                                              " and idFacture = " + listexportjpgfact.at(i).at(0).toString();
            else                                                // c'est une facture, l'iduser est dans la table
                req = "select dep.idUser, UserLogin from " TBL_DEPENSES " dep, " TBL_UTILISATEURS " usr"
                                                                                                              " where dep.idUser  = usr.idUser"
                                                                                                              " and idDep = " + listexportjpgfact.at(i).at(5).toString();
            Listeusr = db->StandardSelectSQL(req, ok);
            if (Listeusr.size()==0) // il n'y a aucune depense enregistrée pour cette facture, on la détruit
            {
                db->SupprRecordFromTable(listexportjpgfact.at(i).at(0).toInt(), "idFacture", TBL_FACTURES);
                continue;
            }
            user = Listeusr.at(0).at(1).toString();
            if (listexportjpgfact.at(i).at(4).toInt()!=1)
                NomFileDoc += "-"+listexportjpgfact.at(i).at(5).toString();

            CheminOKTransfrDir  = CheminOKTransfrDir + "/" + user;
            if (!QDir(CheminOKTransfrDir).exists())
                if (!DirTrsferOK.mkdir(CheminOKTransfrDir))
                {
                    QString msg = tr("Dossier de sauvegarde ") + "<font color=\"red\"><b>" + CheminOKTransfrDir + "</b></font>" + tr(" invalide");
                    QStringList listmsg;
                    listmsg << msg;
                    dlg_message(listmsg, 3000, false);
                    return;
                }

            QString CheminOKTransfrDoc  = CheminOKTransfrDir + "/" + NomFileDoc + "." JPG;
            QString CheminOKTransfrProv = CheminOKTransfrDir + "/" + NomFileDoc + "prov." JPG;
            QByteArray ba = listexportjpgfact.at(i).at(6).toByteArray();
            QPixmap pix;
            pix.loadFromData(ba);
            /*
         * On utilise le passage par les QPixmap parce que le mèthode suivante consistant
         * à réintégrer le QByteArray directement dans le fichier aboutit à un fichier corrompu et je ne sais pas pourquoi
         * QFile prov (CheminOKTransfrProv);
            if (prov.open(QIODevice::Append))
            {
                QTextStream out(&prov);
                out << ba;
            }
        */
            if (!pix.save(CheminOKTransfrProv, "jpeg"))
            {
                qDebug() << "erreur";
                return;
            }
            if (!Utils::CompressFileJPG(CheminOKTransfrProv, proc->DirImagerie()))
            {
                db->SupprRecordFromTable(listexportjpgfact.at(i).at(0).toInt(), "idFacture", TBL_FACTURES);
                continue;
            }
            QFile prov(CheminOKTransfrProv);
            if (prov.open(QIODevice::ReadWrite))
            {
                prov.copy(CheminOKTransfrDoc);
                prov.remove();
            }
            else
                return;
            db->StandardSQL("update " TBL_FACTURES " set jpg = null, LienFichier = '/" + user + "/" + Utils::correctquoteSQL(NomFileDoc) + "." JPG "'"
                            " where idFacture = " + listexportjpgfact.at(i).at(0).toString());
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
    reqpdf = "SELECT idFacture, DateFacture, LienFichier, Intitule, Echeancier, idDepense, pdf FROM " TBL_FACTURES
                  " where pdf is not null";
    QList<QVariantList> listexportpdffact = db->StandardSelectSQL(reqpdf, ok );
    if (ok)
        for (int i=0; i<listexportpdffact.size(); i++)
        {
            if (listexportpdffact.at(i).at(2).toString() != "")
            {
                QString CheminFichier = NomDirStockageImagerie + DIR_FACTURES + listexportpdffact.at(i).at(2).toString();
                if (QFile(CheminFichier).exists())
                {
                    db->StandardSQL("update " TBL_FACTURES " set pdf = null where idFacture = " + listexportpdffact.at(i).at(0).toString());
                    continue;
                }
            }
            QDate datetransfer  = listexportpdffact.at(i).at(1).toDate();
            QString user;

            QString NomFileDoc = listexportpdffact.at(i).at(0).toString() + "_"
                    + (listexportpdffact.at(i).at(4).toInt()==1? ECHEANCIER : FACTURE) + "-"
                    + listexportpdffact.at(i).at(3).toString().replace("/",".") + "_"
                    + datetransfer.toString("yyyyMMdd");
            // on recherche le user à l'origine de cette facture
            QList<QVariantList> Listeusr;
            if (listexportpdffact.at(i).at(4).toInt()==1)          // c'est un échéancier
                req = "select dep.idUser, UserLogin from " TBL_DEPENSES " dep, " TBL_UTILISATEURS " usr"
                                                                                                              " where dep.idUser  = usr.idUser"
                                                                                                              " and idFacture = " + listexportpdffact.at(i).at(0).toString();
            else                                                // c'est une facture, l'iduser est dans la table
                req = "select dep.idUser, UserLogin from " TBL_DEPENSES " dep, " TBL_UTILISATEURS " usr"
                                                                                                              " where dep.idUser  = usr.idUser"
                                                                                                              " and idDep = " + listexportpdffact.at(i).at(5).toString();
            Listeusr = db->StandardSelectSQL(req, ok);
            if (Listeusr.size()==0) // il n'y a aucune depense enregistrée pour cette facture, on la détruit
            {
                db->SupprRecordFromTable(listexportpdffact.at(i).at(0).toInt(), "idFacture", TBL_FACTURES);
                continue;
            }
            user = Listeusr.at(0).at(1).toString();
            if (listexportpdffact.at(i).at(4).toInt()!=1)
                NomFileDoc += "-"+listexportpdffact.at(i).at(5).toString();

            CheminOKTransfrDir  = CheminOKTransfrDir + "/" + user;
            if (!QDir(CheminOKTransfrDir).exists())
                if (!DirTrsferOK.mkdir(CheminOKTransfrDir))
                {
                    QString msg = tr("Dossier de sauvegarde ") + "<font color=\"red\"><b>" + CheminOKTransfrDir + "</b></font>" + tr(" invalide");
                    QStringList listmsg;
                    listmsg << msg;
                    dlg_message(listmsg, 3000, false);
                    return;
                }
            QString CheminOKTransfrDoc      = CheminOKTransfrDir + "/" + NomFileDoc + "." PDF;

            QByteArray bapdf;
            bapdf.append(listexportpdffact.at(i).at(6).toByteArray());

            Poppler::Document* document = Poppler::Document::loadFromData(bapdf);
            if (!document || document->isLocked() || document == Q_NULLPTR)
            {
                QStringList listmsg;
                listmsg << tr("Impossible de charger le document ") + NomFileDoc;
                dlg_message(listmsg, 3000, false);
                QString echectrsfername         = CheminEchecTransfrDir + "/0EchecTransferts - " + datetransfer.toString("yyyy-MM-dd") + ".txt";
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
                        out << listexportpdffact.at(i).at(6).toByteArray() ;
                    }
                }
                QString delreq = "delete from  " TBL_FACTURES " where idFacture = " + listexportpdffact.at(i).at(0).toString();
                //qDebug() << delreq;
                db->StandardSQL(delreq);
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
            db->StandardSQL("update " TBL_FACTURES " set pdf = null, LienFichier = '/" + user + "/" + Utils::correctquoteSQL(NomFileDoc)  + "." PDF "'"
                            " where idFacture = " + listexportpdffact.at(i).at(0).toString());
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
    int totfac = listexportjpgfact.size() + listexportpdffact.size();
    if (totfac > 0)
    {
        listmsg <<  tr("export terminé") << QString::number(totfac) + (totfac>1? tr(" documents comptables exportés en ") :tr(" document comptable exporté en ")) + duree;
        dlg_message(listmsg, 3000);
    }
}

void Rufus::ImportDocsExternes()
{
    if (ImportDocsExtThread != Q_NULLPTR)
        if (isPosteImport())
        {
            QString req = "select distinct list.TitreExamen, list.NomAPPareil from " TBL_APPAREILSCONNECTESCENTRE " appcon, " TBL_LISTEAPPAREILS " list"
                  " where list.idappareil = appcon.idappareil and idLieu = " + QString::number(m_currentuser->idsitedetravail());
            //qDebug()<< req;
            QList<QVariantList> listdocs = db->StandardSelectSQL(req, ok);
            if (ok && listdocs.size()>0)
                ImportDocsExtThread->RapatrieDocumentsThread(listdocs);
        }
}

void Rufus::ImprimeDossier(Patient *pat)
{
    if(pat==Q_NULLPTR)
        return;
    QMap<int, Acte*> *listeactes = m_listeactes->actes();
    if (listeactes->size() == 0)
    {
        UpMessageBox::Watch(this,tr("Pas d'actes enregistré pour ce patient!"));
        return;
    }

    else
    {
        auto fixdateacte        = [] (UpComboBox *debutbox, UpComboBox *finbox, Acte *acte)
        {
            debutbox    ->setCurrentIndex(debutbox  ->findData(acte->id()));
            finbox      ->setCurrentIndex(finbox    ->findData(acte->id()));
        };
        auto recalclistitems    = [] (UpComboBox *box, QMap<int, Acte*> *listeactes, QDate date, bool verslehaut)
        {
            QDate dateencours = QDate::fromString(box->currentText(),"dd-MMM-yyyy");
            box->clear();
            foreach (Acte* act, listeactes->values())
            {
                QString dateacte = act->date().toString("dd-MMM-yyyy");
                if (verslehaut)
                {
                    if (act->date() >= date)
                        box->addItem(dateacte, act->id());
                }
                else
                {
                    if (act->date() <= date)
                        box->addItem(dateacte, act->id());
                }
            }
            if (verslehaut)
            {
                if (dateencours >= date && box->findText(dateencours.toString("dd-MMM-yyyy"))>-1)
                    box->setCurrentIndex(box->findText(dateencours.toString("dd-MMM-yyyy")));
                else
                    box->setCurrentIndex(box->count()-1);
            }
            else
            {
                if (dateencours <= date && box->findText(dateencours.toString("dd-MMM-yyyy"))>-1)
                    box->setCurrentIndex(box->findText(dateencours.toString("dd-MMM-yyyy")));
                else
                    box->setCurrentIndex(0);
            }
        };
        auto recalcallitems     = [] (UpComboBox *debutbox, UpComboBox *finbox, QMap<int, Acte*> *listeactes)
        {
            // remplissage des combobox de date des actes
            foreach (Acte* act, listeactes->values())
            {
                QString dateacte = act->date().toString("dd-MMM-yyyy");
                debutbox    ->addItem(dateacte, act->id());
                finbox      ->addItem(dateacte, act->id());
            }
            debutbox    ->setCurrentIndex(0);
            finbox      ->setCurrentIndex(finbox->count()-1);
        };

        gAsk            = new UpDialog(this);
        int w = 120;
        QHBoxLayout     *debutlayout    = new QHBoxLayout();
        UpLabel         *lbldebut       = new UpLabel;
        UpComboBox      *combodebut     = new UpComboBox;
        lbldebut        ->setText(tr("depuis le"));
        combodebut      ->setFixedWidth(w);
        debutlayout     ->addWidget(lbldebut);
        debutlayout     ->addSpacerItem(new QSpacerItem(30,0,QSizePolicy::Expanding));
        debutlayout     ->addWidget(combodebut);
        QHBoxLayout     *finlayout      = new QHBoxLayout();
        UpLabel         *lblfin         = new UpLabel;
        UpComboBox      *combofin       = new UpComboBox;
        lblfin          ->setText(tr("jusqu'au"));
        combofin        ->setFixedWidth(w);
        finlayout       ->addWidget(lblfin);
        finlayout       ->addSpacerItem(new QSpacerItem(30,0,QSizePolicy::Expanding));
        finlayout       ->addWidget(combofin);
        UpPushButton    *Dossierbutton  = new UpPushButton(tr("tout le dossier"));
        UpPushButton    *Actebutton     = new UpPushButton(tr("acte en cours"));
        gAsk->dlglayout()->insertLayout(0,debutlayout);
        gAsk->dlglayout()->insertLayout(1,finlayout);
        gAsk->dlglayout()->insertWidget(2,Dossierbutton);
        gAsk->dlglayout()->insertWidget(3,Actebutton);
        gAsk->AjouteLayButtons(UpDialog::ButtonOK);
        //gAsk->setStageCount(0.7);

        gAsk->setWindowTitle(tr("Impression dossier"));
        gAsk->dlglayout()->setSizeConstraint(QLayout::SetFixedSize);

        combodebut      ->setEditable(false);
        combofin        ->setEditable(false);
        // remplissage des combobox de date des actes
        recalcallitems (combodebut, combofin, listeactes);

        connect(gAsk->OKButton, &QPushButton::clicked,              gAsk, &UpDialog::accept);
        connect(Actebutton,     &QPushButton::clicked,              gAsk, [=] {fixdateacte(combodebut, combofin, m_currentact);});
        connect(Dossierbutton,  &QPushButton::clicked,              gAsk, [=] {recalcallitems (combodebut, combofin, listeactes);});
        connect(combodebut,     QOverload<int>::of(&QComboBox::activated),    gAsk, [=] {recalclistitems(combofin,
                                                                                               listeactes,
                                                                                               QDate::fromString(combodebut->currentText(),"dd-MMM-yyyy"),
                                                                                               true);});
        connect(combofin,       QOverload<int>::of(&QComboBox::activated),    gAsk, [=] {recalclistitems(combodebut,
                                                                                               listeactes,
                                                                                               QDate::fromString(combofin->currentText(),"dd-MMM-yyyy"),
                                                                                               false);});
        int result = gAsk->exec();
        if (result==0)
        {
            delete gAsk;
            return;
        }
        QList<Acte*> listeactesaimprimer;
        foreach (Acte* act, listeactes->values())
        {
            int idacte = act->id();
            if (idacte >= combodebut->currentData().toInt() && idacte <= combofin->currentData().toInt())
                listeactesaimprimer << act;
        }
        delete gAsk;
        if (listeactesaimprimer.size() > 0)
        {
            bool toutledossier = (listeactes->size() == listeactesaimprimer.size());
            ImprimeListActes(listeactesaimprimer, toutledossier);
        }
    }
    MAJDocsExternes();
}

void Rufus::ImprimeListActes(QList<Acte*> listeactes, bool toutledossier, bool queLePdf, QString nomdossier)
{
    Patient *pat = m_patients->getById(listeactes.at(0)->idPatient());
    if (pat == Q_NULLPTR)
        return;
    if (focusWidget() != Q_NULLPTR)
        focusWidget()->clearFocus();      //!> Valide les changements dans les champs du dossier en cours d'affichage
    int taillefont  = 8;
    QString Reponse =        "<html><head><meta name=\"qrichtext\" content=\"1\" />"
                             "<style type=\"text/css\">"
                             "p {margin-top:0px; margin-bottom:0px;margin-left: 0px; font-size:" + QString::number(taillefont) + "pt}, li { white-space: pre-wrap; }"
                             "</style>"
                             "</head><body>";

    UpTextEdit textprov;

    QString Age;
    QMap<QString,QVariant>  AgeTotal = Utils::CalculAge(pat->datedenaissance(), pat->sexe());
    Age = AgeTotal["toString"].toString();
    Reponse += "<p><font color = \"" COULEUR_TITRES "\"><b>" + pat->nom() + " " + pat->prenom() + "</font> - " + Age + "</b> (" + pat->datedenaissance().toString(tr("d MMM yyyy")) + ")</p>";                   //DDN
    if (pat->adresse1() != "")
        Reponse += "<p>" + pat->adresse1() + "</p>";                                              //Adresse1
    if (pat->adresse2() != "")
        Reponse += "<p>" + pat->adresse2() + "</p>";                                              //Adresse2
    if (pat->adresse3() != "")
        Reponse += "<p>" + pat->adresse3() + "</p>";                                              //Adresse3
    if (pat->codepostal() != "")
    {
        Reponse += "<p>" + pat->codepostal() + " " + pat->ville() + "</p>";           //CP + ville
    }
    else
        if (pat->ville() != "")
            Reponse += "<p>" + pat->ville() + "</p>";                                             //Ville
    if (pat->telephone() != "")
        Reponse += "<p>" + tr("Tél.") + "\t" +pat->telephone() + "</p>";                          //Tél
    if (pat->portable() != "")
        Reponse += "<p>" + tr("Portable") + "\t" + pat->portable() + "</p>";                      //Portable
    if (pat->mail() != "")
        Reponse += "<p>" + tr("Mail") + "\t" + pat->mail() + "</p>";                              //Mail
    if (pat->NNI() > 0)
        Reponse += "<p>" + tr("NNI") + "\t" + QString::number(pat->NNI()) + "</p>";               //NNI
    if (pat->profession() != "")
        Reponse += "<p>" + pat->profession() + "</p>";                                            //Profession

    // collecte des antécédents
    QString AtcdtsOphs = pat->atcdtsophtalmos();
    QString testatcdtsophs = AtcdtsOphs;
    Utils::convertPlainText(testatcdtsophs);
    if (testatcdtsophs != "")
    {
        Utils::convertHTML(AtcdtsOphs);
        Reponse += "<p><td width=\"480\"><font color = \"" COULEUR_TITRES "\">" + tr("Antécédents ophtalmologiques: ") + "</font>" + AtcdtsOphs + "</td></p>";
    }
    QString TtOph = pat->traitementoph();
    QString testttoph = TtOph;
    Utils::convertPlainText(testttoph);
    if (testttoph != "")
    {
        Utils::convertHTML(TtOph);
        Reponse += "<p><td width=\"480\"><font color = \"" COULEUR_TITRES "\">" + tr("Traitements ophtalmologiques: ") + "</font>" + TtOph + "</td></p>";
    }
    QString AtcdtsGenx = pat->atcdtspersos();
    QString testatcdtsgen = AtcdtsGenx;
    Utils::convertPlainText(testatcdtsgen);
    if (testatcdtsgen != "")
    {
        Utils::convertHTML(AtcdtsGenx);
        Reponse += "<p><td width=\"480\"><font color = \"" COULEUR_TITRES "\">" + tr("Antécédents généraux: ") + "</font>" + AtcdtsGenx + "</td></p>";
    }
    QString TtGen = pat->traitementgen();
    QString testttgen = TtGen;
    Utils::convertPlainText(testttgen);
    if (testttgen != "")
    {
        Utils::convertHTML(TtGen);
        Reponse += "<p><td width=\"480\"><font color = \"" COULEUR_TITRES "\">" + tr("Traitements généraux: ") + "</font>" + TtGen + "</td></p>";
    }
    Correspondant *cor = Datas::I()->correspondants->getById(pat->idmg());
    if (cor != Q_NULLPTR)
    {
        QString correspondant = "Dr " + cor->prenom() + " " + cor->nom();
        if (cor->ville() != "")
            correspondant += " - " + cor->ville();
        Reponse += "<p><td width=\"640\"><font color = \"" COULEUR_TITRES "\">" + tr("Médecin traitant: ") + "</font>" + correspondant + "</td></p>";
    }
    if (testatcdtsgen != "" || testttgen != "" || testttoph != "" || testatcdtsophs != "" || cor != Q_NULLPTR)
        Reponse += "<br>";

    bool reponsevide = true;
    QString datedebut, datefin;
    foreach (Acte* act, listeactes)
    {
        if (act == Q_NULLPTR)
                continue;
        if (act == listeactes.first())
            datedebut = act->date().toString(tr("d MMM yyyy"));
        if (act == listeactes.last())
            datefin = act->date().toString(tr("d MMM yyyy"));
        if (act->motif() != ""
            || act->texte() != ""
            || act->conclusion() != "")
        {
            reponsevide = false;
            Reponse += "<p><td width=\"140\"><font color = \"" COULEUR_TITRES "\" ><u><b>" + act->date().toString(tr("d MMMM yyyy")) +"</b></u></font></td>"
                    "<td width=\"400\">"
                    + Datas::I()->users->getById(act->idUser())->titre() + " " + Datas::I()->users->getById(act->idUser())->prenom() + " " + Datas::I()->users->getById(act->idUser())->nom() + "</td></p>";
            if (act->motif() != "")
            {
                QString texte = act->motif();
                Utils::convertHTML(texte);
                Reponse += "<p><td width=\"10\"></td><td width=\"450\"><font color = \"" COULEUR_TITRES "\"" + tr("Motif:") + "</font>" + texte + "</td></p>";
            }
            if (act->texte() != "")
            {
                QString texte = act->texte();
                Utils::convertHTML(texte);
                Reponse += "<p><td width=\"10\"></td><td width=\"450\"><font color = \"" COULEUR_TITRES "\">" + tr("Examen:") + "</font>" + texte + "</td></p>";
            }
            if (act->conclusion() != "")
            {
                QString texte = act->conclusion();
                Utils::convertHTML(texte);
                if (act->motif() != "" || act->texte() != "")
                    Reponse += "<p><td width=\"10\"></td><td width=\"450\"><font color = \"" COULEUR_TITRES "\">" + tr("Conclusion:") + "</font>" + texte + "</td></p>";
                else
                    Reponse += "<p><td width=\"10\"></td><td width=\"450\">" + texte + "</td></p>";
            }
        }
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
   User *userEntete = Datas::I()->users->getById(m_currentuser->idparent(), Item::LoadDetails);
   if (!userEntete)
   {
       UpMessageBox::Watch(this, tr("Impossible de retrouver les données de l'en-tête"), tr("Annulation de l'impression"));
       return;
   }

   Entete = proc->ImpressionEntete(QDate::currentDate(), userEntete).value("Norm");
   if (Entete == "") return;
   Entete.replace("{{TITRE1}}"             , "");
   QString comment;
   if (toutledossier)
       comment = tr("COMPTE RENDU DE DOSSIER");
   else if (listeactes.size() > 1)
       comment = tr("Actes du") + " " + datedebut + tr("au") + " " + datefin;
   else
       comment = tr("Acte du") + " " + datedebut;
   Entete.replace("{{TITRE}}"              , "<font color = \"" COULEUR_TITRES "\">" + comment + "</font>");
   Entete.replace("{{PRENOM PATIENT}}"     , pat->prenom());
   Entete.replace("{{NOM PATIENT}}"        , pat->nom().toUpper());
   Entete.replace("{{DDN}}"                , "(" + pat->datedenaissance().toString(tr("d MMM yyyy")) + ")");


   // création du pied
   Pied = proc->ImpressionPied(userEntete);
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

   Reponse.replace(QRegExp("font-size( *: *[\\d]{1,2} *)pt"),"font-size:" + QString::number(taillefont) + "pt");
   QString largeurformule = LARGEUR_FORMULE;
   Reponse.replace("<td width=\"" LARGEUR_FORMULE "\">","<td width=\"" + QString::number(largeurformule.toInt() - 40) + "\">");
   Corps.replace("{{TEXTE ORDO}}",Reponse);

   QTextEdit *Etat_textEdit = new QTextEdit;
   Etat_textEdit->setHtml(Corps);
   bool aa = false;
   if (queLePdf)
   {
       aa = proc->Imprime_pdf(Etat_textEdit, Entete, Pied,
                             (listeactes.size() > 1?
                                  tr("Actes") + " - " + pat->nom() + " " + pat->prenom() + " - " + tr("du ") + datedebut + tr(" au ") + datefin + ".pdf":
                                  tr("Acte") + " - " + pat->nom() + " " + pat->prenom() + " - " + listeactes.at(0)->date().toString("d MMM yyyy")) + ".pdf",
                             nomdossier);
   }
   else
       aa = proc->Imprime_Etat(Etat_textEdit, Entete, Pied,
                              proc->TaillePieddePage(), proc->TailleEnTete(), proc->TailleTopMarge(),
                              AvecDupli, AvecPrevisu, AvecNumPage);
   if (aa)
   {
       QHash<QString, QVariant> listbinds;
       listbinds["iduser"] =            m_currentuser->id();
       listbinds["idpat"] =             pat->id();
       listbinds["typeDoc"] =           COURRIER;
       listbinds["soustypedoc"] =       (queLePdf? tr("Export") : tr("Impression")) + " " + (toutledossier? tr("dossier"): tr("actes"));
       listbinds["titre"] =             (queLePdf? tr("Export") : tr("Impression")) + " " + (toutledossier? tr("dossier"): tr("actes"));
       listbinds["textEntete"] =        Entete;
       listbinds["textCorps"] =         Corps;
       listbinds["textPied"] =          Pied;
       listbinds["dateimpression"] =    QDate::currentDate().toString("yyyy-MM-dd") + " " + QTime::currentTime().toString("HH:mm:ss");
       listbinds["useremetteur"] =      m_currentuser->id();
       listbinds["emisrecu"] =          "0";
       listbinds["formatdoc"] =         COURRIER;
       listbinds["idlieu"] =            m_currentuser->idsitedetravail();
       if(!db->InsertSQLByBinds(TBL_IMPRESSIONS, listbinds))
           UpMessageBox::Watch(this,tr("Impossible d'enregistrer ce document dans la base!"));
       ui->OuvreDocsExternespushButton->setEnabled(true);
   }
   delete Etat_textEdit;
}

void Rufus::DropPatient(QByteArray data)
{
    /*! Dans le mimedata du qabstractitemmodeldatalist se succédent row(), column() et datas de chaque item
     * stream << rowitem1 << colitem1 << datasitem1 << rowitem2 << colitem2 << datasitem2 << rowitem3 << colitem3 << datasitem3...etc...
     * les datas de l'item sont sous forme de QMap<int, QVariant> à un seul élémént, key() = 0 et value() = le QVariant du contenu de l'item

    \code
    QDataStream         datastream(&data, QIODevice::ReadOnly);
    while (!datastream.atEnd())
    {
        //! on découpe le datatsream 3 éléménts par 3 éléménts: row, col et dataMap
        int                 row, col;
        QMap<int,QVariant>  dataMap;
        datastream >> row >> col >> dataMap;
        QString ab = "col = " + QString::number(col) + " row = " + QString::number(row);
        foreach (int val, dataMap.keys())
        {
            QString bc = ab + " datamap = " + QString::number(val) + " / " + dataMap.find(val).value().toString();
            qDebug() << bc;
        {
    }
         * ça donne ça
                col = 0 row = 235 datamap = 0 / 11510                       Le contenu de la colonne 0 et de la ligne 235 = l'id du patient
                col = 1 row = 235 datamap = 0 / DUPONT Marcel               Le contenu de la colonne 1 et de la ligne 235 = NOM prénom
                col = 2 row = 235 datamap = 0 / 10-06-1957                  Le contenu de la colonne 2 et de la ligne 235 = DDN du patient
                col = 3 row = 235 datamap = 0 / 19570610                    Le contenu de la colonne 3 et de la ligne 235 = DDN du patient au format yyyyMMdd pour le tri
                col = 4 row = 235 datamap = 0 / Dupont                      Le contenu de la colonne 4 et de la ligne 235 = le nom du patient pour le tri
                col = 5 row = 235 datamap = 0 / Marcel                      Le contenu de la colonne 5 et de la ligne 235 = le prénom du patient pour le tri
    */

    /*! en l'occurence, on ne relève que le troisième élément du datastream
     * qui correspond à la colonne 0 du qabstractitemmodel et donc ici à l'idpatient
     * mais on est obligé de faire sortir row et col (qui ne servent à rien pour ce qu'on veut faire)
    */
    QDataStream         datastream(&data, QIODevice::ReadOnly);
    int                 row, col;
    QMap<int,QVariant>  dataMap;
    datastream >> row >> col >> dataMap;
    int idpat = dataMap.value(0).toInt();
    InscritEnSalDat(m_patients->getById(idpat));
}

bool Rufus::InscritEnSalDat(Patient *pat)
{
    if (pat == Q_NULLPTR)
        return false;
    if (m_listepatientsencours->patientsencours()->find(pat->id()) != m_listepatientsencours->patientsencours()->constEnd())
    {
        UpMessageBox::Watch(this, tr("Patient déjà inscrit en salle d'attente"));
        return false;
    }
    else
    {
        //créer une fiche avec la liste des checkbox
        QStringList llist = MotifRDV();
        if (llist.isEmpty())
            return false;
        m_listepatientsencours->CreationPatient(pat->id(),                             //! idPat
                                                 llist.at(3).toInt(),                               //! idUser
                                                 ARRIVE,                                            //! Statut
                                                 QTime(),                                           //! heureStatut
                                                 QTime().fromString(llist.at(2),"HH:mm"),           //! heureRDV
                                                 db->ServerDateTime().time(),                       //! heureArrivee
                                                 llist.at(0),                                       //! Motif
                                                 llist.at(1),                                       //! Message
                                                 0,                                                 //! idActeAPayer
                                                 "",                                                //! PosteExamen
                                                 0,                                                 //! idUserEnCoursExamen
                                                 0);                                                //! idSalDat
        Flags::I()->MAJFlagSalleDAttente();
        RecaleTableView(pat);
    }
    return true;
}


void Rufus::ListeCorrespondants()
{
    if (Datas::I()->correspondants->correspondants()->size()==0)
    {
        UpMessageBox::Watch(this, tr("pas de correspondant enregistré") );
        bool onlydoctors    = false;
        Dlg_IdentCorresp    = new dlg_identificationcorresp(dlg_identificationcorresp::Creation, onlydoctors);
        Dlg_IdentCorresp->exec();
        delete Dlg_IdentCorresp;
        return;
    }
    Dlg_ListCor = new dlg_listecorrespondants(this);
    Dlg_ListCor->exec();
    delete Dlg_ListCor;
}

void Rufus::MajusculeCreerNom()
{
    ui->CreerNomlineEdit->setText(Utils::trimcapitilize(ui->CreerNomlineEdit->text(), false));
    FiltreTable(ui->CreerNomlineEdit->text(), ui->CreerPrenomlineEdit->text());
}

void Rufus::MajusculeCreerPrenom()
{
    ui->CreerPrenomlineEdit->setText(Utils::trimcapitilize(ui->CreerPrenomlineEdit->text(), false));
    FiltreTable(ui->CreerNomlineEdit->text(), ui->CreerPrenomlineEdit->text());
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
    IdentificationPatient(dlg_identificationpatient::Modification,Datas::I()->patients->currentpatient());  // aussi appelé depuis le bouton ModifIdentificationupSmallButton
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
    if (Datas::I()->patients->currentpatient()->id() == 0)
        return;
    dlg_listemotscles *ListMCDialog = new dlg_listemotscles(Datas::I()->patients->currentpatient());
    if (ListMCDialog->exec()==0)
    {
        QStringList listMC = ListMCDialog->listMCDepart();
        if (listMC.size()>0)
        {
            db->StandardSQL("delete from " TBL_MOTSCLESJOINTURES " where idpat = " + QString::number(Datas::I()->patients->currentpatient()->id()));
            QString req = "insert into " TBL_MOTSCLESJOINTURES " (idpat, idmotcle) values ";
            req += "(" + QString::number(Datas::I()->patients->currentpatient()->id()) + ", " + listMC.at(0) + ")";
            for (int j=1; j<listMC.size(); j++)
                req += ", (" + QString::number(Datas::I()->patients->currentpatient()->id()) + ", " + listMC.at(j) + ")";
            db->StandardSQL(req);
            db->StandardSQL("delete from " TBL_MOTSCLESJOINTURES " where idMotCle not in (select idmotcle from " TBL_MOTSCLES ")");
        }
    }
    CalcMotsCles(Datas::I()->patients->currentpatient());
    ListMCDialog->close(); // nécessaire pour enregistrer la géométrie
    delete ListMCDialog;
}

void Rufus::RechercheParID()
{
    if (!m_patients->isfull())
        FiltreTable();
    gAskRechParIDDialog                 = new UpDialog();
    gAskRechParIDDialog                 ->setAttribute(Qt::WA_DeleteOnClose);
    UpLabel         *idlabel            = new UpLabel(gAskRechParIDDialog, tr("id du patient"));
    UpLineEdit      *idLine             = new UpLineEdit(gAskRechParIDDialog);
    lblnom                              = new UpLabel(gAskRechParIDDialog);
    idLine                              ->setMaxLength(8);
    idLine                              ->setValidator((new QIntValidator(1,99999999)));
    gAskRechParIDDialog->dlglayout()    ->insertWidget(0,idlabel);
    gAskRechParIDDialog->dlglayout()    ->insertWidget(1,idLine);
    gAskRechParIDDialog->dlglayout()    ->insertWidget(2,lblnom);
    gAskRechParIDDialog                 ->AjouteLayButtons();
    connect(idLine,                         &QLineEdit::textEdited,         [=] {CherchePatientParID(idLine->text().toInt());});
    connect(gAskRechParIDDialog->OKButton,  &QPushButton::clicked,          [=] {gAskRechParIDDialog->close();});
    gAskRechParIDDialog->exec();
}

void Rufus::RechercheParMotCle()
{
    QString req = "select idmotcle, motcle from " TBL_MOTSCLES " order by motcle";
    QList<QVariantList> listmotscle = db->StandardSelectSQL(req, ok);
    if (!ok || listmotscle.size()==0)
    {
        UpMessageBox::Watch(this, tr("Aucun mot clé défini dans la base"), tr("Recherche impossible"));
        return;
    }

    gAskRechParMotCleDialog                 = new UpDialog();
    QTableView      *tabMC                  = new QTableView(gAskRechParMotCleDialog);
    gAskRechParMotCleDialog->dlglayout()    ->insertWidget(0,tabMC);
    gAskRechParMotCleDialog                 ->AjouteLayButtons();
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
    for (int i=0; i<listmotscle.size(); i++)
    {
        pitem   = new QStandardItem(listmotscle.at(i).at(1).toString());
        pitem   ->setAccessibleDescription(listmotscle.at(i).at(0).toString());
        pitem   ->setCheckable(true);
        modele  ->appendRow(pitem);
    }
    tabMC       ->setModel(modele);
    for (int i=0; i<modele->rowCount(); i++)
        tabMC   ->setRowHeight(i,hauteurligne);
    tabMC       ->setColumnWidth(0,300);
    tabMC       ->setFixedWidth(tabMC->columnWidth(0)+2);
    gAskRechParMotCleDialog->setFixedWidth(tabMC->width()
                        + gAskRechParMotCleDialog->dlglayout()->contentsMargins().left()*2);
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
                  " from " TBL_MOTSCLESJOINTURES " as mcjoin"
                  " left outer join " TBL_PATIENTS " pat"
                  " on mcjoin.idPat = pat.IDPAT"
                  " left outer join " TBL_MOTSCLES " msc"
                  " on mcjoin.idmotcle = msc.idmotcle"
                  " where mcjoin.idmotcle in (";
    req += listidMc.at(0);
    for (int i=1; i<listidMc.size(); i++)
        req += ", " +listidMc.at(i);
    req += ") order by patnom, patprenom";
    QList<QVariantList> listpats = db->StandardSelectSQL(req, ok);
    if (!ok || listpats.size()==0)
    {
        UpMessageBox::Watch(this, tr("Aucun patient retrouvé pour ces critères"));
        return;
    }
    gAskListPatients = new UpDialog();
    QTableView      *tabMC              = new QTableView(gAskListPatients);
    gAskListPatients->dlglayout()       ->insertWidget(0,tabMC);
    gAskListPatients->AjouteLayButtons(UpDialog::ButtonPrint | UpDialog::ButtonOK);
    gAskListPatients->PrintButton   ->setData(listidMc);
    connect(gAskListPatients->OKButton,     &QPushButton::clicked,   [=] {gAskListPatients->accept();});
    connect(gAskListPatients->PrintButton,  &QPushButton::clicked,   [=] {ImprimeListPatients(gAskListPatients->PrintButton->Data());});

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
    for (int i=0; i<listpats.size(); i++)
    {
        pitem   = new QStandardItem(listpats.at(i).at(1).toString() + " " + listpats.at(i).at(2).toString());
        pitem1  = new QStandardItem(listpats.at(i).at(3).toDate().toString(tr("dd-MMM-yyyy")));
        pitem2  = new QStandardItem(listpats.at(i).at(4).toString());
        pitem   ->setAccessibleDescription(listpats.at(i).at(0).toString());
        pitem1  ->setAccessibleDescription(listpats.at(i).at(0).toString());
        pitem2  ->setAccessibleDescription(listpats.at(i).at(0).toString());
        QList <QStandardItem*> listitems;
        listitems << pitem << pitem1 << pitem2;
        modele  ->appendRow(listitems);
    }
    tabMC->setModel(modele);
    for (int i=0; i<modele->rowCount(); i++)
        tabMC->setRowHeight(i,hauteurligne);
    tabMC->setColumnWidth(0,200);
    tabMC->setColumnWidth(1,100);
    tabMC->setColumnWidth(2,250);
    tabMC->setFixedWidth(tabMC->columnWidth(0)+tabMC->columnWidth(1)+tabMC->columnWidth(2)+2);
    gAskListPatients->setFixedWidth(tabMC->width()
                        + gAskListPatients->dlglayout()->contentsMargins().left()*2);
    gAskListPatients->exec();
    delete gAskListPatients;
}

void Rufus::AfficheCourriersAFaire()
{
    QString req = "select idActe, act.idPat, PatNom, PatPrenom, ActeDate"
                  " from " TBL_ACTES " as act"
                  " left outer join " TBL_PATIENTS " pat"
                  " on act.idPat = pat.IDPAT"
                  " where ActeCourrierafaire = 'T' and act.iduser = " + QString::number(m_currentuser->id()) + " order by patnom, patprenom";
    QList<QVariantList> listcourriers = db->StandardSelectSQL(req, ok);
    if (!ok || listcourriers.size()==0)
    {
        UpMessageBox::Watch(this, tr("Pas de courrier en attente"));
        return;
    }
    gAskListPatients = new UpDialog();
    QTableView      *tabCourriers              = new QTableView();
    gAskListPatients->dlglayout()       ->insertWidget(0,tabCourriers);
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
    for (int i=0; i<listcourriers.size(); i++)
    {
        pitem   = new QStandardItem(listcourriers.at(i).at(2).toString().toUpper() + " " + listcourriers.at(i).at(3).toString());       // Nom Prenom
        pitem1  = new QStandardItem(listcourriers.at(i).at(4).toDate().toString(tr("dd-MMM-yyyy")));                        // Date de l'acte
        pitem2  = new QStandardItem(listcourriers.at(i).at(1).toString());                                                  // idPat
        pitem   ->setAccessibleDescription(listcourriers.at(i).at(0).toString());                                           // idActe
        pitem1  ->setAccessibleDescription(listcourriers.at(i).at(0).toString());                                           // idActe
        QList <QStandardItem*> listitems;
        listitems << pitem << pitem1 << pitem2;
        modele  ->appendRow(listitems);
    }
    tabCourriers->setModel(modele);
    for (int i=0; i<modele->rowCount(); i++)
        tabCourriers->setRowHeight(i,hauteurligne);
    tabCourriers->setColumnWidth(0,200);
    tabCourriers->setColumnWidth(1,100);
    tabCourriers->setColumnWidth(2,0);
    tabCourriers->setFixedWidth(tabCourriers->columnWidth(0)+tabCourriers->columnWidth(1)+tabCourriers->columnWidth(2)+2);
    gAskListPatients->setFixedWidth(tabCourriers->width()
                        + gAskListPatients->dlglayout()->contentsMargins().left()*2);
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
            ChoixDossier(m_patients->getById(idPat), idacte);
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
            ChoixDossier(m_patients->getById(idPat), idacte);
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
    QString req1 = "select motcle from " TBL_MOTSCLES " where idmotcle in(";
    req1 += listidMc.at(0);
    for (int i=1; i<listidMc.size(); i++)
        req1 += ", " +listidMc.at(i);
    req1 += ")";
    QList<QVariantList> titrlist = db->StandardSelectSQL(req1,ok);
    QString titre = tr("recherche de patients sur ");
    titre += titrlist.at(0).at(0).toString();
    for (int i=1; i<titrlist.size(); i++)
        titre += ", " + titrlist.at(i).at(0).toString();
    QString req = "select mcjoin.idPat, PatNom, PatPrenom, PatDDN, motcle"
                  " from " TBL_MOTSCLESJOINTURES " as mcjoin"
                  " left outer join " TBL_PATIENTS " pat"
                  " on mcjoin.idPat = pat.IDPAT"
                  " left outer join " TBL_MOTSCLES " msc"
                  " on mcjoin.idmotcle = msc.idmotcle"
                  " where mcjoin.idmotcle in (";
    req += listidMc.at(0);
    for (int i=1; i<listidMc.size(); i++)
        req += ", " +listidMc.at(i);
    req += ") order by patnom, patprenom";
    QList<QVariantList> patlist = db->StandardSelectSQL(req,ok);
    int     gtotalNbreDossiers    = patlist.size();
    QDate   date = QDate::currentDate();;

    //création de l'entête
    QString EnTete;
    User *userEntete = Datas::I()->users->getById(m_currentuser->idparent(), Item::LoadDetails);
    if (userEntete == Q_NULLPTR)
        return;
    EnTete = proc->ImpressionEntete(date, userEntete).value("Norm");
    if (EnTete == "") return;
    EnTete.replace("{{TITRE1}}"            , "");
    EnTete.replace("{{PRENOM PATIENT}}"    , "");
    EnTete.replace("{{NOM PATIENT}}"       , "");
    EnTete.replace("{{TITRE}}"             , titre);
    EnTete.replace("{{DDN}}"               , "<font color = \"" COULEUR_TITRES "\">" + QString::number(gtotalNbreDossiers)
                   + " " + (gtotalNbreDossiers>1? tr("dossiers") : tr("dosssier")) + "</font>");
    // création du pied
    QString Pied = proc->ImpressionPied(userEntete);
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
    for (int k = 0; k < patlist.size(); k++)
    {
        // Remplacement des variables par les valeurs lues.
        lignepat = ligne;
        lignepat.replace("{{NOM PATIENT}}", patlist.at(k).at(1).toString() + " " + patlist.at(k).at(2).toString());
        lignepat.replace("{{DDN}}", patlist.at(k).at(3).toDate().toString(tr("dd-MMM-yyyy")));
        lignepat.replace("{{MOTIF}}", patlist.at(k).at(4).toString());
        texte += lignepat;
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
    Datas::I()->patients->setdossierpatientaouvrir(UpText->getId());
    if( m_currentuser->isSoignant() )
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
    else if( m_currentuser->isSecretaire() )
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
    Patient *pat = getPatientFromCursorPositionInTable();
    if (pat == Q_NULLPTR)
        return;
    //if (!pat->isalloaded())
    Datas::I()->patients->setdossierpatientaouvrir(pat->id());

    gmenuContextuel = new QMenu(this);

    QAction *pAction_MettreEnSalDat = gmenuContextuel->addAction(tr("Inscrire ce patient en salle d'attente"));
    connect (pAction_MettreEnSalDat,        &QAction::triggered,    [=] {ChoixMenuContextuelListePatients("SalDat");});

    QAction *pAction_ModifierDossier = gmenuContextuel->addAction(tr("Modifier les données de ce patient"));
    connect (pAction_ModifierDossier,       &QAction::triggered,    [=] {ChoixMenuContextuelListePatients("Modifier");});

    QAction *pAction_Copier = gmenuContextuel->addAction(tr("Créer un dossier de la même famille"));
    connect (pAction_Copier,                &QAction::triggered,    [=] {ChoixMenuContextuelListePatients("Copie");});

    if( m_currentuser->isSoignant() )
    {
        QAction *pAction_ReprendreDossier = gmenuContextuel->addAction(tr("Visualiser le dossier"));
        connect (pAction_ReprendreDossier,  &QAction::triggered,    [=] {ChoixMenuContextuelListePatients("Autre Dossier");});
    }
    QAction *pAction_EmettreDoc = gmenuContextuel->addAction(tr("Emettre un document"));
    connect (pAction_EmettreDoc,            &QAction::triggered,    [=] {ChoixMenuContextuelListePatients("Document");});

    QString req = "Select idImpression from " TBL_IMPRESSIONS " where idpat = " + QString::number(Datas::I()->patients->dossierpatientaouvrir()->id());
    if (db->StandardSelectSQL(req,ok).size() > 0){
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
    if (Datas::I()->patients->dossierpatientaouvrir() == Q_NULLPTR)
        return;
    if (choix == "Autre Dossier")
    {
        Actes *acts = new Actes;
        acts->initListeByPatient(Datas::I()->patients->dossierpatientaouvrir());
        if (acts->actes()->size()  == 0)
        {
            UpMessageBox::Watch(this, tr("Pas de consultation enregistrée pour ") + Datas::I()->patients->dossierpatientaouvrir()->prenom() + " " + Datas::I()->patients->dossierpatientaouvrir()->nom());
            return;
        }
        else
        {
            Dlg_ActesPrecs  = new dlg_actesprecedents(acts, false, this);
            Dlg_ActesPrecs  ->setWindowTitle(tr("Consultations précédentes de ") + Datas::I()->patients->dossierpatientaouvrir()->nom() + " " + Datas::I()->patients->dossierpatientaouvrir()->prenom());
            Dlg_ActesPrecs  ->setWindowIcon(Icons::icLoupe());
            Dlg_ActesPrecs  ->exec();
        }
        acts->clearAll(acts->actes());
        delete acts;
    }
    else if (choix == "SalDat")
        InscritEnSalDat(Datas::I()->patients->dossierpatientaouvrir());
    else if (choix == "Copie")
        RecopierDossier(Datas::I()->patients->dossierpatientaouvrir());
    else if (choix == "Modifier")
        IdentificationPatient(dlg_identificationpatient::Modification,Datas::I()->patients->dossierpatientaouvrir());     //depuis menu contextuel de la table liste
    else if (choix == "Document")
        OuvrirDocuments(false);
    else if (choix == "ImprimeAncienDoc")
        OuvrirDocsExternes(Datas::I()->patients->dossierpatientaouvrir());                                                //depuis menu contextuel ListePatients
    else if (choix == "EnregDocScan")
        EnregistreDocScanner(Datas::I()->patients->dossierpatientaouvrir());                                              //depuis menu contextuel ListePatients
    else if (choix == "EnregVideo")
        EnregistreVideo(Datas::I()->patients->dossierpatientaouvrir());                                                   //depuis menu contextuel ListePatients
    else if (choix == "SendMess")
    {
        QMap<QString, QVariant> map;
        map["null"] = true;
        SendMessage(map, Datas::I()->patients->dossierpatientaouvrir()->id());                           //depuis menu contextuel ListePatients
    }
}

void Rufus::MenuContextuelMedecin()
{
    if (ui->MGupComboBox->findText(ui->MGupComboBox->currentText()) != -1)
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
    bool onlydoctors = true;
    Dlg_IdentCorresp          = new dlg_identificationcorresp(dlg_identificationcorresp::Modification, onlydoctors, Datas::I()->correspondants->getById(id, Item::LoadDetails));
    if (Dlg_IdentCorresp->exec()>0)
        if (Dlg_IdentCorresp->identcorrespondantmodifiee())
            ui->MGupComboBox->setCurrentIndex(idxMG);
    delete Dlg_IdentCorresp;
}

void Rufus::MenuContextuelCorrespondant(UpComboBox *box)
{
    if (box->findText(box->currentText())  != -1)
    {
        QString choix = "";
        if (box == ui->AutresCorresp1upComboBox) choix = "Modifier1";
        else if (box == ui->AutresCorresp2upComboBox) choix = "Modifier2";
        else return;
        gmenuContextuel = new QMenu(this);
        QAction *pAction_IdentPatient = gmenuContextuel->addAction(tr("Modifier les coordonnées de ce correspondant"));
        connect (pAction_IdentPatient,      &QAction::triggered,    [=] {ChoixMenuContextuelCorrespondant(choix);});

        // ouvrir le menu
        gmenuContextuel->exec(cursor().pos());
        delete gmenuContextuel;
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
    bool onlydoctors = false;
    Dlg_IdentCorresp = new dlg_identificationcorresp(dlg_identificationcorresp::Modification, onlydoctors, Datas::I()->correspondants->getById(id, Item::LoadDetails));
    if (Dlg_IdentCorresp->exec()>0)
    {
        int idCor = Dlg_IdentCorresp->correspondantrenvoye()->id();
        if (choix == "Modifier1")
            ui->AutresCorresp1upComboBox->setCurrentIndex(ui->AutresCorresp1upComboBox->findData(idCor));
        else if (choix == "Modifier2")
            ui->AutresCorresp2upComboBox->setCurrentIndex(ui->AutresCorresp2upComboBox->findData(idCor));
        OKModifierTerrain(Datas::I()->patients->currentpatient());
    }
    delete Dlg_IdentCorresp;
}

void Rufus::MenuContextuelSalDat(UpLabel *labelClicked)
{
    if (labelClicked == Q_NULLPTR) return;
    QMap<QString, QVariant> rsgnmt = labelClicked->getData();
    int id = rsgnmt["idpat"].toInt();
    Datas::I()->patients->setdossierpatientaouvrir(id);
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
    if( m_currentuser->isSoignant() )
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

    Datas::I()->patients->setdossierpatientaouvrir(labelClicked->getId());

    gmenuContextuel = new QMenu(this);

    if (listRange.size() == 1 && listRange.at(0).rowCount()== 1)
    {
        if( m_currentuser->isSoignant() )
        {
            QAction *pAction_ReprendreDossier = gmenuContextuel->addAction(tr("Reprendre le dossier"));
            connect (pAction_ReprendreDossier,  &QAction::triggered,    this,   [=] {ChoixMenuContextuelSalDat("Reprendre");});
        }
        QAction *pAction_EmettreDoc = gmenuContextuel->addAction(tr("Emettre un document"));
        connect (pAction_EmettreDoc,            &QAction::triggered,    this,   [=] {ChoixMenuContextuelSalDat("Document");});
    }
    if (m_currentuser->isSecretaire() || labelClicked->getData().value("idComptable").toInt()==m_currentuser->idcomptable())
    {
        QAction *pAction_EnregistrePaiement = gmenuContextuel->addAction(tr("Enregistrer le paiement"));
        connect (pAction_EnregistrePaiement,    &QAction::triggered,    this, [=] {ChoixMenuContextuelSalDat("Payer");});
    }

    // ouvrir le menu
    gmenuContextuel->exec(cursor().pos());
    delete gmenuContextuel;
}

void Rufus::ChoixMenuContextuelSalDat(QString choix)
{
    if (Datas::I()->patients->dossierpatientaouvrir()->id() == 0)
        return;
    if (choix == "Reprendre")
        ChoixDossier(Datas::I()->patients->dossierpatientaouvrir());
    else if (choix == "Payer")
        AppelPaiementDirect(Accueil);
    else if (choix == "Modifier")
        IdentificationPatient(dlg_identificationpatient::Modification, Datas::I()->patients->dossierpatientaouvrir());  //appelé depuis le menu contextuel de la table salle d'attente

    else if (choix == "Ouvrir")
        ChoixDossier(Datas::I()->patients->dossierpatientaouvrir());
    else if (choix == "Retirer" || choix == "Fermer")
    {
        m_listepatientsencours->SupprimePatientEnCours(m_listepatientsencours->getById(Datas::I()->patients->dossierpatientaouvrir()->id()));
        Flags::I()->MAJFlagSalleDAttente();
    }
    else if (choix == "Copie")
        RecopierDossier(Datas::I()->patients->dossierpatientaouvrir());
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
             if (rsgnmt["idpat"].toInt()== Datas::I()->patients->dossierpatientaouvrir()->id())
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

        QStringList llist = MotifRDV(Motif, Message, heurerdv);
        if (llist.isEmpty())
            return;
        PatientEnCours *pat = m_listepatientsencours->getById(Datas::I()->patients->dossierpatientaouvrir()->id());
        if (pat == Q_NULLPTR)
            m_listepatientsencours->CreationPatient(Datas::I()->patients->dossierpatientaouvrir()->id(),//! idPat
                                                     llist.at(3).toInt(),                               //! idUser
                                                     ARRIVE,                                            //! Statut
                                                     QTime(),                                           //! heureStatut
                                                     QTime().fromString(llist.at(2), "HH:mm"),          //! heureRDV
                                                     db->ServerDateTime().time(),                       //! heureArrivee
                                                     llist.at(0),                                       //! Motif
                                                     llist.at(1),                                       //! Message
                                                     0,                                                 //! idActeAPayer
                                                     "",                                                //! PosteExamen
                                                     0,                                                 //! idUserEnCoursExamen
                                                     0);                                                //! idSalDat
        else
        {
            ItemsList::update(pat, CP_MOTIF_SALDAT, llist.at(0));
            ItemsList::update(pat, CP_MESSAGE_SALDAT, llist.at(1));
            ItemsList::update(pat, CP_HEURERDV_SALDAT, QTime().fromString(llist.at(2), "HH:mm"));
            ItemsList::update(pat, CP_IDUSER_SALDAT, llist.at(3).toInt());
        }
        Flags::I()->MAJFlagSalleDAttente();
    }
}


QStringList Rufus::MotifRDV(QString motif, QString Message, QTime heurerdv)
{
    //créer une fiche avec tous les checkbox correspondant aux motifs de RDV : Cs, OCT, CV, BO, Biométrie, Urgence, Angio,...etc...
    gAsk            = new UpDialog(this);
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

    foreach (User *usr, Datas::I()->users->superviseurs()->values() )
        ComboSuperviseurs->addItem(usr->login(), QString::number(usr->id()) );
    ComboSuperviseurs->setFixedWidth(100);

    QHBoxLayout *soignantlayout     = new QHBoxLayout();
    lblsoignt->setText(tr("RDV avec"));
    soignantlayout->addWidget(lblsoignt);
    soignantlayout->addWidget(ComboSuperviseurs);
    soignantlayout->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding));


    if (Datas::I()->motifs->motifs()->size()==0)
    {
        llist << "" << "";
        return llist;
    }
    int defaut = -1;
    int k = -1;
    foreach (Motif *mtf, Datas::I()->motifs->motifs()->values())
    {
        ++k;
        QRadioButton *radiobut = new QRadioButton(grpBox);
        radiobut->setAutoExclusive(true);
        radiobut->setText(mtf->motif());
        radiobut->setChecked(motif==mtf->raccourci());
        if (mtf->pardefaut())
            defaut = k;
    }
    QRadioButton    *UrgButton      = new QRadioButton(grpBox);
    UrgButton   ->setAutoExclusive(true);
    UrgButton   ->setText(tr("Urgence"));
    UrgButton   ->setChecked(motif=="URG");
    bool b = false;
    foreach (QRadioButton *butt, grpBox->findChildren<QRadioButton*>())
    {
        if (butt->isChecked())
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
    gAsk->setStageCount(1);

    motiflayout->addLayout(soignantlayout);
    motiflayout->addWidget(grpBox);
    motiflayout->addWidget(lbltitre);
    motiflayout->addWidget(MsgText);
    gAsk->dlglayout()->insertLayout(0,motiflayout);

    connect(gAsk->OKButton,   &QPushButton::clicked,  [=] {gAsk->accept();});

    gAsk->setWindowTitle(tr("Enregistrer le motif de l'acte"));

    gAsk->setModal(true);
    gAsk->setFixedWidth(320);
    gAsk->dlglayout()->setSizeConstraint(QLayout::SetFixedSize);
    MsgText->setText(Message);
    if (gAsk->exec()>0)
    {
        Message = MsgText->toPlainText();
        for (int m=0; m<grpBox->findChildren<QRadioButton*>().size(); m++)
        {
            if (grpBox->findChildren<QRadioButton*>().at(m)->isChecked())
            {
                motif = grpBox->findChildren<QRadioButton*>().at(m)->text();
                break;
            }
        }
        if (motif==tr("Urgence"))
            motif= "URG";
        else
        {
            foreach (Motif *mtf, Datas::I()->motifs->motifs()->values())
            {
                if (mtf->motif()==motif)
                {
                    motif = mtf->raccourci();
                    break;
                }
            }
        }
        llist << motif << Message << HeureRDV->time().toString("HH:mm") << ComboSuperviseurs->currentData().toString();
    }
    delete gAsk;
    return llist;
}


void Rufus::MAJPosteConnecte()
{
    // On en profite au passage pour sauvegarder la position de la fenêtre principale
    //bug Qt? -> cette ligne de code ne peut pas être mise juste avant exit(0) sinon elle n'est pas éxécutée...
    proc->gsettingsIni->setValue("PositionsFiches/Rufus", saveGeometry());
    if (Datas::I()->postesconnectes->currentpost() != Q_NULLPTR)
        ItemsList::update(Datas::I()->postesconnectes->currentpost(), CP_HEUREDERNIERECONNECTION_USRCONNECT, db->ServerDateTime());
    else
    {
        Datas::I()->postesconnectes->CreationPosteConnecte();
        Flags::I()->MAJFlagSalleDAttente();
    }
}

void Rufus::ModfiCotationActe()
{
    gAutorModifConsult = true;
    ui->Cotationframe->setEnabled(true);
    ui->CCAMlinklabel->setVisible(true);
}

void Rufus::ModifierTerrain()
{
    ui->TerraintreeWidget->setVisible(false);
    ui->ModifTerrainwidget->setVisible(true);
    ui->TabacLabel->setVisible(true);
    ui->TabaclineEdit->setVisible(true);
    ui->OKModifTerrainupSmallButton->setUpButtonStyle(UpSmallButton::STARTBUTTON);
    ui->OKModifTerrainupSmallButton->disconnect();
    connect (ui->OKModifTerrainupSmallButton,   &QPushButton::clicked,  this,   [=] {OKModifierTerrain(Datas::I()->patients->currentpatient());});
}

void Rufus::OKModifierTerrain(Patient *pat, bool recalclesdonnees) // recalcule le ui->TerraintreeWidget et l'affiche
{
    if (pat == Q_NULLPTR)
        return;
    if (recalclesdonnees)
        m_patients->loadAll(pat, Item::ForceUpdate);
    ui->TerraintreeWidget->clear();
    bool a = false;
    ui->TerraintreeWidget->setColumnCount(2);
    ui->TerraintreeWidget->setColumnWidth(0,70);        //IdPat
    ui->TerraintreeWidget->setColumnWidth(1,180 );     //
    ui->TerraintreeWidget->setStyleSheet("QTreeWidget {selection-color: rgb(0,0,0);"
                                         " selection-background-color: rgb(164, 205, 255);"
                                         " background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #f6f7fa, stop: 1 rgba(200, 230, 200, 50));"
                                         " border: 1px solid rgb(150,150,150); border-radius: 10px;}");
    ui->TerraintreeWidget->setIconSize(QSize(25,25));
    ui->TerraintreeWidget->header()->setVisible(false);
    QTreeWidgetItem *pItem0, *pItem1, *pItem2, *pItem3, *pItem4, *pItem5;
    pItem0 = new QTreeWidgetItem() ;
    pItem0->setText(1,QString::number(pat->id()));                                                             // IdPatient
    pItem0->setText(0,tr("ANTÉCÉDENTS GÉNÉRAUX"));
    pItem0->setIcon(0,Icons::icStetho());
    pItem0->setTextAlignment(1,Qt::AlignLeft);
    pItem1 = new QTreeWidgetItem() ;
    pItem1->setText(1,QString::number(pat->id()));                                                             // IdPatient
    pItem1->setText(0,tr("TRAITEMENTS EN COURS"));
    pItem1->setIcon(0,Icons::icMedoc());
    pItem1->setTextAlignment(1,Qt::AlignLeft);
    pItem2 = new QTreeWidgetItem();
    pItem2->setText(1,QString::number(pat->id()));                                                             // IdPatient
    pItem2->setText(0,tr("ATCDTS OPHTALMOLOGIQUES FAMILIAUX"));
    pItem2->setIcon(0,Icons::icFamily());
    pItem2->setTextAlignment(1,Qt::AlignLeft);
    pItem3 = new QTreeWidgetItem();
    pItem3->setText(1,QString::number(pat->id()));                                                             // IdPatient
    pItem3->setIcon(0,Icons::icSmoking());
    pItem3->setTextAlignment(1,Qt::AlignLeft);
    pItem4 = new QTreeWidgetItem();
    pItem4->setText(1,QString::number(pat->id()));                                                             // IdPatient
    pItem4->setText(0,tr("AUTRES"));
    pItem4->setIcon(0,Icons::icAlcool());
    pItem4->setTextAlignment(1,Qt::AlignLeft);
    pItem5 = new QTreeWidgetItem() ;
    pItem5->setText(0,tr("MÉDECIN GÉNÉRALISTE"));
    pItem5->setText(1,QString::number(pat->id()));                                                             // IdPatient
    pItem5->setIcon(0,Icons::icDoctor());
    pItem5->setTextAlignment(1,Qt::AlignLeft);

    pItem0->setForeground(0,QBrush(QColor(Qt::red)));
    pItem1->setForeground(0,QBrush(QColor(Qt::blue)));
    pItem2->setForeground(0,QBrush(QColor(Qt::darkGreen)));
    pItem3->setForeground(0,QBrush(QColor(Qt::darkMagenta)));
    pItem4->setForeground(0,QBrush(QColor(Qt::darkYellow)));
    pItem5->setForeground(0,QBrush(QColor(Qt::darkBlue)));

    QString hash;
    QStringList listhash;
    QFontMetrics fm(qApp->font());
    hash = Utils::trim(pat->atcdtspersos(), true, true);
    if (hash != "")
    {
        a = true;
        ui->TerraintreeWidget->addTopLevelItem(pItem0);
        pItem0->setFirstColumnSpanned(true);
        listhash = hash.split("\n");
        pItem0->setExpanded(listhash.size() > 0);
        for (int i=0;i<listhash.size();i++)
        {
            QTreeWidgetItem *pit = new QTreeWidgetItem(pItem0);
            pit->setText(0,"");
            pit->setText(1,listhash.at(i));
            if (fm.width(listhash.at(i)) > (ui->TerraintreeWidget->width() - ui->TerraintreeWidget->columnWidth(0)))
                pit->setToolTip(1, listhash.at(i));
        }
    }
    listhash.clear();
    hash = Utils::trim(pat->traitementgen(), true, true);
    if (hash != "")
    {
        a = true;
        ui->TerraintreeWidget->addTopLevelItem(pItem1);
        pItem1->setFirstColumnSpanned(true);
        listhash = hash.split("\n");
        pItem1->setExpanded(listhash.size() > 0);
        foreach(const QString &txt, listhash)
        {
            QTreeWidgetItem *pit = new QTreeWidgetItem(pItem1);
            pit->setText(0,"");
            pit->setText(1,txt);
            if (fm.width(txt) > (ui->TerraintreeWidget->width() - ui->TerraintreeWidget->columnWidth(0)))
                pit->setToolTip(1, txt);
        }
    }
    listhash.clear();
    hash = Utils::trim(pat->atcdtsfamiliaux(), true, true);
    if (hash != "")
    {
        a = true;
        ui->TerraintreeWidget->addTopLevelItem(pItem2);
        pItem2->setFirstColumnSpanned(true);
        listhash = hash.split("\n");
        pItem2->setExpanded(listhash.size() > 0);
        foreach(const QString &txt, listhash)
        {
            QTreeWidgetItem *pit = new QTreeWidgetItem(pItem2);
            pit->setText(0,"");
            pit->setText(1,txt);
            if (fm.width(txt) > (ui->TerraintreeWidget->width() - ui->TerraintreeWidget->columnWidth(0)))
                pit->setToolTip(1, txt);
        }
    }
    listhash.clear();
    hash = Utils::trim(pat->tabac(), true, true);
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
        ui->TerraintreeWidget->addTopLevelItem(pItem3);
        pItem3->setFirstColumnSpanned(true);
    }
    listhash.clear();
    hash = Utils::trim(pat->toxiques(), true, true);
    if (hash != "")
    {
        a = true;
        ui->TerraintreeWidget->addTopLevelItem(pItem4);
        pItem4->setFirstColumnSpanned(true);
        listhash = hash.split("\n");
        pItem4->setExpanded(listhash.size() > 0);
        foreach(const QString &txt, listhash)
        {
            QTreeWidgetItem *pit = new QTreeWidgetItem(pItem4);
            pit->setText(0,"");
            pit->setText(1,txt);
            if (fm.width(txt) > (ui->TerraintreeWidget->width() - ui->TerraintreeWidget->columnWidth(0)))
                pit->setToolTip(1, txt);
        }
    }
    if (pat->idmg()>0)
    {
        QString tooltp ="";
        Correspondant * cor = Datas::I()->correspondants->getById(pat->idmg());
        if (cor != Q_NULLPTR)
        {
            if (cor->adresse1() != "")
                tooltp += cor->adresse1();
            if (cor->adresse2() != "")
                tooltp += "\n" + cor->adresse2();
            if (cor->adresse3() != "")
                tooltp += "\n" + cor->adresse3();
            if (cor->ville() != "")
                tooltp += "\n" + cor->ville();
            if (cor->telephone() != "")
                tooltp += "\n" + cor->telephone();
            hash = "Dr " + cor->prenom() + " " + cor->nom();
            a = true;
            ui->TerraintreeWidget->addTopLevelItem(pItem5);
            pItem5->setFirstColumnSpanned(true);
            pItem5->setExpanded(true);
            QTreeWidgetItem *pit = new QTreeWidgetItem(pItem5);
            pit->setText(0,"");
            pit->setText(1,hash);
            if (fm.width(hash) > (ui->TerraintreeWidget->width() - ui->TerraintreeWidget->columnWidth(0)))
                pit->setToolTip(1, hash);
        }
    }
    ui->TerraintreeWidget->setVisible(a);
    ui->ModifTerrainwidget->setVisible(!a);
    ui->TabacLabel->setVisible(!a);
    ui->TabaclineEdit->setVisible(!a);
    ui->OKModifTerrainupSmallButton->setUpButtonStyle(a? UpSmallButton::EDITBUTTON : UpSmallButton::STARTBUTTON);

    ui->OKModifTerrainupSmallButton->disconnect();
    connect (ui->OKModifTerrainupSmallButton,   &QPushButton::clicked,  this,   [=] {a? ModifierTerrain() : OKModifierTerrain(Datas::I()->patients->currentpatient());});
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
        OuvrirActesPrecedents();            //! depuis OuvrirActesPrecspushButtonClicked()
}

void Rufus::OuvrirJournalDepenses()
{
    Dlg_Deps = new dlg_depenses();
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
    Dlg_Param = new dlg_param(this);
    Dlg_Param->setWindowTitle(tr("Paramètres"));
    Dlg_Param->exec();
    if (Dlg_Param->DataUserModifiees())
        setWindowTitle("Rufus - " + m_currentuser->login() + " - " + m_currentuser->fonction());
    if (Dlg_Param->CotationsModifiees())
    {
        QString req = "insert into " TBL_COTATIONS " (typeacte, MontantOPTAM, MontantNonOPTAM, montantpratique, CCAM, iduser, tip) values \n";
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
                    QString mtconv  = (m_currentuser->isOPTAM() ? montantOPTAM : montantNonOPTAM);
                    if (Dlg_Param->ui->ActesCCAMupTableWidget->columnCount()==6)
                    {
                        UpLineEdit *line = dynamic_cast<UpLineEdit*>(Dlg_Param->ui->ActesCCAMupTableWidget->cellWidget(i,5));
                        if (line != Q_NULLPTR)
                            montantprat = (line->text()!=""? line->text() : mtconv);
                        else
                            montantprat = mtconv;
                    }
                    QString mtprat = (m_currentuser->secteurconventionnel() >1 ? montantprat : mtconv);
                    QString montantpratique = QString::number(QLocale().toDouble(mtprat));
                    req += "('" + codeCCAM +  "', " + montantOPTAM + "," + montantNonOPTAM + "," + montantpratique + ", 1, " + QString::number(m_currentuser->id()) + ", null),\n";
                }
        }
        for (int i=0; i<Dlg_Param->ui->AssocCCAMupTableWidget->rowCount(); i++)
        {
            UpCheckBox *check = dynamic_cast<UpCheckBox*>(Dlg_Param->ui->AssocCCAMupTableWidget->cellWidget(i,0));
            if (check != Q_NULLPTR)
                if (check->isChecked())
                {
                    QString codeCCAM, montantOPTAM(""), montantNonOPTAM(""), montantprat(""), tip ("");
                    codeCCAM        = Dlg_Param->ui->AssocCCAMupTableWidget->item(i,1)->text();
                    UpLineEdit *lineOPTAM = dynamic_cast<UpLineEdit*>(Dlg_Param->ui->AssocCCAMupTableWidget->cellWidget(i,2));
                    if (lineOPTAM != Q_NULLPTR)
                    {
                        montantOPTAM    = QString::number(QLocale().toDouble(lineOPTAM->text()));
                        tip             = lineOPTAM->getData().toString();
                    }
                    UpLineEdit *lineNonOPTAM = dynamic_cast<UpLineEdit*>(Dlg_Param->ui->AssocCCAMupTableWidget->cellWidget(i,3));
                    if (lineNonOPTAM != Q_NULLPTR)
                        montantNonOPTAM    = QString::number(QLocale().toDouble(lineNonOPTAM->text()));
                    QString mtconv  = (m_currentuser->isOPTAM() ? montantOPTAM : montantNonOPTAM);
                    if (Dlg_Param->ui->AssocCCAMupTableWidget->columnCount()==5)
                    {
                        UpLineEdit *line = dynamic_cast<UpLineEdit*>(Dlg_Param->ui->AssocCCAMupTableWidget->cellWidget(i,4));
                        if (line != Q_NULLPTR)
                            montantprat = (line->text()!=""? line->text() : mtconv);
                        else
                            montantprat = mtconv;
                    }
                    QString mtprat = (m_currentuser->secteurconventionnel() >1? montantprat : mtconv);
                    QString montantpratique = QString::number(QLocale().toDouble(mtprat));
                    req += "('" + codeCCAM +  "', " + montantOPTAM + "," + montantNonOPTAM + "," + montantpratique + ", 2, " + QString::number(m_currentuser->id()) + ", '" + tip + "'),\n";
                }
        }
        for (int i=0; i<Dlg_Param->ui->HorsNomenclatureupTableWidget->rowCount(); i++)
        {
            UpCheckBox *check = dynamic_cast<UpCheckBox*>(Dlg_Param->ui->HorsNomenclatureupTableWidget->cellWidget(i,0));
            if (check != Q_NULLPTR)
                if (check->isChecked())
                {
                    QString codeCCAM, mtconv(""), montantconv, tip("");
                    codeCCAM = Dlg_Param->ui->HorsNomenclatureupTableWidget->item(i,1)->text();
                    UpLineEdit *lineconv = dynamic_cast<UpLineEdit*>(Dlg_Param->ui->HorsNomenclatureupTableWidget->cellWidget(i,2));
                    if (lineconv != Q_NULLPTR)
                    {
                        mtconv  = lineconv->text();
                        tip     = lineconv->getData().toString();
                    }
                    montantconv = QString::number(QLocale().toDouble(mtconv));
                    req += "('" + codeCCAM +  "', " + montantconv + "," + montantconv + "," + montantconv + ", 3, " + QString::number(m_currentuser->id()) + ", '" + tip + "'), \n";
                }
        }
        req = req.left(req.lastIndexOf(")")+1);
        //proc->Edit(req);
        QString reqDel = "delete from " TBL_COTATIONS " where idUser = " + QString::number(m_currentuser->id());
        db->StandardSQL(reqDel);
        db->StandardSQL(req);
        ReconstruitListesCotations();
    }
    delete Dlg_Param;
}

void Rufus::RecettesSpeciales()
{
    Dlg_RecSpec           = new dlg_recettesspeciales();
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
    if (Datas::I()->patients->currentpatient()->id() == 0)
        return;
    //TODO : SQL
    QString Cotation = ui->ActeCotationcomboBox->currentText();
    ui->EnregistrePaiementpushButton->setEnabled(Cotation!="");
    // On recherche s'il y a un montant enregistré pour cette cotation
    int idx = ui->ActeCotationcomboBox->findText(Cotation, Qt::MatchExactly);
    if (idx>-1)
    {
        QStringList listMontantActe = ui->ActeCotationcomboBox->itemData(idx).toStringList();
        QString MontantActe;
        if( m_currentuser->secteurconventionnel()>1 && !Datas::I()->patients->currentpatient()->iscmu())
            MontantActe = QLocale().toString(listMontantActe.at(1).toDouble(),'f',2);
        else
            MontantActe = QLocale().toString(listMontantActe.at(0).toDouble(),'f',2);
        ui->ActeMontantlineEdit->setText(MontantActe);
        if (listMontantActe.at(0) != listMontantActe.at(1))
        {
            ui->BasculerMontantpushButton->setVisible(true);
            ui->BasculerMontantpushButton->setImmediateToolTip(tr("Revenir au tarif conventionnel"));
        }
    }
    else
    {
        QString tarifconventionne = (m_currentuser->isOPTAM() ? "OPTAM" : "NonOPTAM");
        QString req =
                "SELECT " + tarifconventionne + ", montantpratique FROM " TBL_COTATIONS " cot, " TBL_CCAM " cc"
                " where Typeacte = codeccam"
                " and iduser = " + QString::number(m_currentuser->idparent()) +
                " and codeccam like '" + Utils::correctquoteSQL(Cotation) + "%'";
        //qDebug() << req;
        QVariantList cotdata = db->getFirstRecordFromStandardSelectSQL(req, ok);
        if (ok && cotdata.size()>0)
        {
            QString MontantActe;
            if (m_currentuser->secteurconventionnel()>1 && !Datas::I()->patients->currentpatient()->iscmu())
                MontantActe = QLocale().toString(cotdata.at(1).toDouble(),'f',2);
            else
                MontantActe = QLocale().toString(cotdata.at(0).toDouble(),'f',2);
            ui->ActeMontantlineEdit->setText(MontantActe);
            if (cotdata.at(0) != cotdata.at(1))
            {
                ui->BasculerMontantpushButton->setVisible(true);
                ui->BasculerMontantpushButton->setImmediateToolTip(tr("Revenir au tarif conventionnel"));
            }
        }
        else
        {
            QString req = "SELECT OPTAM, NonOPTAM FROM " TBL_CCAM " where codeccam like '" + Utils::correctquoteSQL(Cotation) + "%'";
            QVariantList cot2data = db->getFirstRecordFromStandardSelectSQL(req, ok);
            if (ok && cot2data.size()>0)
            {
                QString MontantActe;
                if (m_currentuser->secteurconventionnel()>1 && !Datas::I()->patients->currentpatient()->iscmu() && !m_currentuser->isOPTAM())
                    MontantActe = QLocale().toString(cot2data.at(1).toDouble(),'f',2);
                else
                    MontantActe = QLocale().toString(cot2data.at(0).toDouble(),'f',2);
                ui->ActeMontantlineEdit->setText(MontantActe);
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
            QString req = "select codeccam, OPTAM, NonOPTAM from " TBL_CCAM " where codeccam = " + ui->ActeCotationcomboBox->currentText().toUpper();
            QSqlQuery quer(req, db->getDataBase() );
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
                    req = "insert into " TBL_COTATIONS " (TypeActe, MontantConv, MontantPratique, CCAM, idUser) VALUES ('" +
                            quer.value(0).toString() + "', " +
                            quer.value(1).toString() + ", "  +
                            quer.value(1).toString() + ", "  +
                            "1, " +
                            QString::number(gidUser) + ")";
                    QSqlQuery (req, db->getDataBase() );
                }
                else
                {
                    req = "insert into " TBL_COTATIONS " (TypeActe, MontantConv, MontantPratique, CCAM, idUser) VALUES ('" +
                            quer.value(0).toString() + "', " +
                            quer.value(2).toString() + ", "  +
                            quer.value(2).toString() + ", "  +
                            "1, " +
                            QString::number(gidUser) + ")";
                    QSqlQuery (req, db->getDataBase() );
                }
            }
        }
    }*/
}

void Rufus::SalleDAttente()
{
    if (AutorDepartConsult(true))
    {
        ui->tabWidget->removeTab(ui->tabWidget->indexOf(ui->tabDossier));
        ui->tabWidget->setCurrentWidget(ui->tabList);
        FermeDlgActesPrecedentsEtDocsExternes();
        ModeSelectDepuisListe();
    }
}

void Rufus::AllusrChkBoxSendMsg(bool a)
{
    for (int i=0; i< gAsk->findChildren<UpCheckBox*>().size(); i++)
        if (gAsk->findChildren<UpCheckBox*>().at(i)->rowTable() == 1)
            gAsk->findChildren<UpCheckBox*>().at(i)->setChecked(a);
}

void Rufus::OneusrChkBoxSendMsg(bool a)
{
    if (a)
    {
        bool allchk = true;
        for (int i=0; i< gAsk->findChildren<UpCheckBox*>().size(); i++)
            if (gAsk->findChildren<UpCheckBox*>().at(i)->rowTable() == 1)
            {
                allchk = gAsk->findChildren<UpCheckBox*>().at(i)->isChecked();
                if (!allchk)
                    break;
            }
        for (int j=0; j< gAsk->findChildren<UpCheckBox*>().size(); j++)
            if (gAsk->findChildren<UpCheckBox*>().at(j)->rowTable() == 2)
                gAsk->findChildren<UpCheckBox*>().at(j)->setChecked(allchk);
    }
    else
        for (int j=0; j< gAsk->findChildren<UpCheckBox*>().size(); j++)
            if (gAsk->findChildren<UpCheckBox*>().at(j)->rowTable() == 2)
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
        QString req = "select patnom, patprenom from " TBL_PATIENTS " where idpat = " + QString::number(id);
        QVariantList patdata = db->getFirstRecordFromStandardSelectSQL(req, ok);
        if (ok && patdata.size()>0)
        {
            checkpat       = new UpCheckBox(gAsk);
            checkpat        ->setObjectName("AboutPatupCheckBox");
            checkpat        ->setText(tr("A propos de ") + patdata.at(0).toString().toUpper() + " " + patdata.at(1).toString());
            checkpat        ->setChecked(true);
            checkpat        ->setiD(id);
            msglayout       ->addWidget(checkpat);
        }
    }

    QString req1 = "select idUser, UserLogin from " TBL_UTILISATEURS " where UserDesactive is NULL and userlogin is not null";
    QList<QVariantList> usrlist = db->StandardSelectSQL(req1, ok);
    if (ok && usrlist.size()>0)
    {
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
        for (int i=0; i<usrlist.size(); i++)
        {
            UpCheckBox *chk0 = new UpCheckBox();
            chk0->setText(usrlist.at(i).at(1).toString());
            chk0->setiD(usrlist.at(i).at(0).toInt());
            if (idMsg>-1)
                chk0->setChecked(map["listdestinataires"].toStringList().contains(usrlist.at(i).at(0).toString()));
            chk0->setRowTable(1);
            connect(chk0,  &QCheckBox::clicked,  [=] {OneusrChkBoxSendMsg(chk0->isChecked());});
            vbox->addWidget(chk0);
            chk0->setAutoExclusive(false);
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
        MsgText->setText(map[CP_TEXTMSG_MESSAGERIE].toString());
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
    gAsk->dlglayout()->insertLayout(0,totallayout);

    gAsk            ->setWindowTitle(tr("Envoyer un message"));
    gAsk            ->setFixedWidth(510);
    gAsk->dlglayout()->setSizeConstraint(QLayout::SetFixedSize);

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
        dlg_message(tr("Vous avez oublié de rédiger le texte de votre message!"),2000,false);
        return;
    }
    bool checkusr = false;
    for (int j=0; j< gAsk->findChildren<UpCheckBox*>().size(); j++)
        if (gAsk->findChildren<UpCheckBox*>().at(j)->rowTable() == 1)
        {
            checkusr = gAsk->findChildren<UpCheckBox*>().at(j)->isChecked();
            if (checkusr)
                break;
        }
    if (!checkusr)
    {
        dlg_message(tr("Vous avez oublié de choisir un destinataire!"),2000,false);
        return;
    }
    QStringList locklist;
    locklist <<  TBL_MESSAGES << TBL_MESSAGESJOINTURES << TBL_FLAGS;
    db->createtransaction(locklist);
    if (idMsg<0)  // Enregistrement d'un nouveau message
    {
        QString req = "insert into " TBL_MESSAGES " (idEmetteur, " CP_TEXTMSG_MESSAGERIE ", idPatient, Tache, DateLimite, Urge, CreeLe)\n values(";
        req += QString::number(m_currentuser->id()) + ", ";
        req += "'" + Utils::correctquoteSQL(gAsk->findChildren<UpTextEdit*>().at(0)->toHtml()) + "', ";
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
        //qDebug() << req;
        if (!db->StandardSQL(req,tr("Impossible d'enregistrer ce message")))
            db->rollback();

        req = "SELECT Max(idMessage) FROM " TBL_MESSAGES;
        QVariantList msgdata = db->getFirstRecordFromStandardSelectSQL(req, ok);
        if (!ok || msgdata.size()==0)
        {
            db->rollback();
            return;
        }
        int idmsg = msgdata.at(0).toInt();
        QList<int> listidusr;
        for (int j=0; j< gAsk->findChildren<UpCheckBox*>().size(); j++)
            if (gAsk->findChildren<UpCheckBox*>().at(j)->rowTable() == 1)       // c'est le checkbox d'un user
                if (gAsk->findChildren<UpCheckBox*>().at(j)->isChecked())
                    listidusr << gAsk->findChildren<UpCheckBox*>().at(j)->iD();
        if (listidusr.size()==0)
        {
            db->rollback();
            return;
        }
        req = "insert into " TBL_MESSAGESJOINTURES " (idMessage, idDestinataire) Values ";
        for (int i=0; i<listidusr.size(); i++)
        {
            req += "(" + QString::number(idmsg) + "," + QString::number(listidusr.at(i)) + ")";
            if (i<listidusr.size()-1)
                req += ",";
        }
        if (!db->StandardSQL(req, tr("Impossible d'enregistrer le message")))
        {
            db->rollback();
            return;
        }
        db->commit();
        envoieMessageA(listidusr);
    }
    else  //    modification d'un message existant
    {
        QString req = "update " TBL_MESSAGES " set ";
        req += CP_TEXTMSG_MESSAGERIE " = '" + Utils::correctquoteSQL(gAsk->findChildren<UpTextEdit*>().at(0)->toHtml()) + "', ";
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
        //qDebug() << req;
        if (!db->StandardSQL(req,tr("Impossible d'enregistrer ce message")))
            db->rollback();
        db->StandardSQL("delete from " TBL_MESSAGESJOINTURES " where idmessage = " + QString::number(idMsg));
        QList<int> listidusr;
        for (int j=0; j< gAsk->findChildren<UpCheckBox*>().size(); j++)
            if (gAsk->findChildren<UpCheckBox*>().at(j)->rowTable() == 1)       // c'est le checkbox d'un user
                if (gAsk->findChildren<UpCheckBox*>().at(j)->isChecked())
                    listidusr << gAsk->findChildren<UpCheckBox*>().at(j)->iD();
        if (listidusr.size()==0)
        {
            db->rollback();
            return;
        }
        req = "insert into " TBL_MESSAGESJOINTURES " (idMessage, idDestinataire) Values ";
        for (int i=0; i<listidusr.size(); i++)
        {
            req += "(" + QString::number(idMsg) + "," + QString::number(listidusr.at(i)) + ")";
            if (i<listidusr.size()-1)
                req += ",";
        }
        if (!db->StandardSQL(req, tr("Impossible d'enregistrer le message")))
        {
            db->rollback();
            return;
        }
        db->commit();
    }
    dlg_message(tr("Message enregistré"),1000,false);
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
    if (db->getMode() == DataBase::Poste)
        modeconnexion = tr("monoposte");
    else if (db->getMode() == DataBase::ReseauLocal)
        modeconnexion = tr("réseau local");
    if (db->getMode() == DataBase::Distant)
    {
        modeconnexion = tr("accès distant - connexion ");
        if (proc->gsettingsIni->value("BDD_DISTANT/SSL").toString() != "NO")
            modeconnexion += tr("cryptée (SSL)");
        else
            modeconnexion += tr("non cryptée");
    }
    QString windowtitle = "Rufus - " + m_currentuser->login() + " - " + m_currentuser->fonction() + " - " + modeconnexion + " - " + qApp->applicationVersion();
    if (db->getMode() != DataBase::Distant)
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
    PatientEnCours *pat = Q_NULLPTR;
    auto itpat = m_listepatientsencours->patientsencours()->find(idpat);
    if (itpat != m_listepatientsencours->patientsencours()->cend())
    {
        pat = const_cast<PatientEnCours*>(itpat.value());
        Msg = pat->message();
        if (pat->heurerarrivee().isValid())
        {
            QTime heureArriv = pat->heurerarrivee();
            if (heureArriv.secsTo(QTime::currentTime())/60 < 15)
                color = "color: green";
            else if (heureArriv.secsTo(QTime::currentTime())/60 < 30)
               color = "color: orange";
            else
               color = "color: red";
        }
        if (pat->heurerdv().isValid())
        {
            QTime heureRDV = pat->heurerdv();
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
            if (pat != Q_NULLPTR)
            {
                Motif *mtf = Datas::I()->motifs->getMotifFromRaccourci(pat->motif());
                if (mtf != Q_NULLPTR)
                background = "background:#" + mtf->couleur();
            }
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
                QString color2, colorRDV2;
                pat = Q_NULLPTR;
                auto itpat = m_listepatientsencours->patientsencours()->find(labi0->getData()["idpat"].toInt());
                if (itpat != m_listepatientsencours->patientsencours()->cend())
                {
                    pat = const_cast<PatientEnCours*>(itpat.value());
                    Msgi = pat->message();
                    if (pat->heurerarrivee().isValid())
                    {
                        QTime heureArriv = pat->heurerarrivee();
                        if (heureArriv.secsTo(QTime::currentTime())/60 < 15)
                            color2 = "color: green";
                        else if (heureArriv.secsTo(QTime::currentTime())/60 < 30)
                           color2 = "color: orange";
                        else
                           color2 = "color: red";
                    }
                    if (pat->heurerdv().isValid())
                    {
                        QTime heureRDV = pat->heurerdv();
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
                    if (pat != Q_NULLPTR)
                    {
                        Motif *mtf = Datas::I()->motifs->getMotifFromRaccourci(pat->motif());
                        if (mtf != Q_NULLPTR)
                        background = "background:#" + mtf->couleur();
                    }
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

// utilisé par le poste importateur des documents pour supprimer les documents et les factures
// ayant été inscrits dans les tables DocsASupprimer et FacturesASupprimer
// par les autres postes

void Rufus::SupprimerDocsEtFactures()
{
    if (!isPosteImport())
        return;
    QString NomDirStockageImagerie = proc->DirImagerie();

    /* Supprimer les documents en attente de suppression*/
    QString req = "Select filepath from " TBL_DOCSASUPPRIMER;
    QList<QVariantList> ListeDocs = db->StandardSelectSQL(req, ok);
    for (int i=0; i<ListeDocs.size(); i++)
    {
        QString CheminFichier = NomDirStockageImagerie + ListeDocs.at(i).at(0).toString();
        if (!QFile(CheminFichier).remove())
            UpMessageBox::Watch(this, tr("Fichier introuvable!"), CheminFichier);
        db->StandardSQL("delete from " TBL_DOCSASUPPRIMER " where filepath = '" + Utils::correctquoteSQL(ListeDocs.at(i).at(0).toString()) + "'");
    }

    /* Supprimer les factures en attente de suppression - même démarche mais on fait une copie de la facture dans le dossier FACTURESSANSLIEN avant de la supprimer*/
    QString CheminOKTransfrDir = NomDirStockageImagerie + DIR_FACTURESSANSLIEN;
    if (!Utils::mkpath(CheminOKTransfrDir))
    {
        QString msg = tr("Dossier de sauvegarde ") + "<font color=\"red\"><b>" + CheminOKTransfrDir + "</b></font>" + tr(" invalide");
        QStringList listmsg;
        listmsg << msg;
        dlg_message(listmsg, 3000, false);
        return;
    }
    req = "select LienFichier from " TBL_FACTURESASUPPRIMER;
    QList<QVariantList> ListeFactures = db->StandardSelectSQL(req, ok);
    for (int i=0; i<ListeFactures.size(); i++)
    {
        QString lienfichier = ListeFactures.at(i).at(0).toString();
        /*  on copie le fichier dans le dossier facturessanslien*/
        QString user = lienfichier.split("/").at(1);
        CheminOKTransfrDir = CheminOKTransfrDir + "/" + user;
        if (!Utils::mkpath(CheminOKTransfrDir))
        {
            QString msg = tr("Dossier de sauvegarde ") + "<font color=\"red\"><b>" + CheminOKTransfrDir + "</b></font>" + tr(" invalide");
            QStringList listmsg;
            listmsg << msg;
            dlg_message(listmsg, 3000, false);
            continue;
        }
        QFile(NomDirStockageImagerie + DIR_FACTURES + lienfichier).copy(NomDirStockageImagerie + DIR_FACTURESSANSLIEN + lienfichier);
        /*  on l'efface du dossier de factures*/
        QFile(NomDirStockageImagerie + DIR_FACTURES + lienfichier).remove();
        /* on détruit l'enregistrement dans la table FacturesASupprimer*/
        db->StandardSQL("delete from " TBL_FACTURESASUPPRIMER " where LienFichier = '" + Utils::correctquoteSQL(lienfichier) + "'");
    }
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
    int x = QGuiApplication::screens().first()->geometry().width();
    int y = QGuiApplication::screens().first()->geometry().height();
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
        "select Distinct mess.idMessage, idEmetteur, " CP_TEXTMSG_MESSAGERIE ", idPatient, Tache, DateLimite, CreeLe, Urge, lu, Fait, idJointure from "
        TBL_MESSAGES " mess left outer join " TBL_MESSAGESJOINTURES " joint on mess.idmessage = joint.idmessage \n"
        " where \n"
        " iddestinataire = " + QString::number(m_currentuser->id()) + "\n"
        " order by urge desc, CreeLe desc";
    //proc->Edit(req);
    QList<QVariantList> destlist = db->StandardSelectSQL(req, ok);
    if (ok && destlist.size()>0)
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

        for (int i=0; i<destlist.size(); i++)
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
            if (destlist.at(i).at(7).toInt()==1)
                Titredoc->setStyleSheet("color: red");
            QString txt = destlist.at(i).at(6).toDate().toString(tr("d-MMM-yy")) + " " + destlist.at(i).at(6).toTime().toString("h:mm");
            if (destlist.at(i).at(1).toInt()>0)
                txt += tr(" de ") + Datas::I()->users->getById(destlist.at(i).at(1).toInt())->login();
            Titredoc->setText(txt);
            titrelay->addWidget(Titredoc);
            UpCheckBox *Rdchk = new UpCheckBox();
            Rdchk->setChecked(destlist.at(i).at(8).toInt()==1);
            Rdchk->setiD(destlist.at(i).at(10).toInt());
            Rdchk->setText(tr("lu"));
            Rdchk->setFixedWidth(45);
            connect(Rdchk,  &QCheckBox::clicked,    [=] {MsgRead(Rdchk);});
            titrelay->addSpacerItem(new QSpacerItem(10,10,QSizePolicy::Expanding));
            titrelay->addWidget(Rdchk);
            reclay->addLayout(titrelay);

            QHBoxLayout *Msglay = new QHBoxLayout();

            QVBoxLayout *Droplay = new QVBoxLayout();
            Droplay->addSpacerItem(new QSpacerItem(10,10, QSizePolicy::Expanding, QSizePolicy::Expanding));
            if (destlist.at(i).at(1).toInt() != m_currentuser->id())
            {
                UpLabel *Respondlbl = new UpLabel();
                Respondlbl->setId(destlist.at(i).at(0).toInt());
                Respondlbl->setPixmap(Icons::pxConversation().scaled(20,20)); //WARNING : icon scaled : pxConversation 20,20
                Respondlbl->setImmediateToolTip(tr("Répondre"));
                connect(Respondlbl,     QOverload<int>::of(&UpLabel::clicked), [=] {MsgResp(Respondlbl->getId());});
                Respondlbl->setFixedWidth(25);
                Droplay->addWidget(Respondlbl);
            }

            UpLabel *Dellbl = new UpLabel();
            Dellbl->setId(destlist.at(i).at(10).toInt());
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
            if (destlist.at(i).at(4).toInt()==1)                                                                           // le message est une tâche
            {
                QHBoxLayout *Tasklay = new QHBoxLayout();
                UpLabel *Todolbl = new UpLabel();
                Todolbl->setText(tr("A effectuer avant le ") + destlist.at(i).at(5).toDate().toString(tr("d-MMM-yy")));
                if (destlist.at(i).at(9).toInt()!=1)
                {
                    if (QDate::currentDate() >= destlist.at(i).at(5).toDate())
                        Todolbl->setStyleSheet("color: red");
                    else if (QDate::currentDate().addDays(3) > destlist.at(i).at(5).toDate())
                        Todolbl->setStyleSheet("color: orange");
                }
                Tasklay->addWidget(Todolbl);
                Tasklay->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding));
                UpCheckBox *Dnchk = new UpCheckBox();
                Dnchk->setChecked(destlist.at(i).at(9).toInt()==1);
                Dnchk->setiD(destlist.at(i).at(10).toInt());
                Dnchk->setText(tr("fait"));
                Dnchk->setFixedWidth(45);
                connect(Dnchk,  &QCheckBox::clicked,    [=] {MsgDone(Dnchk);});
                Tasklay->addWidget(Dnchk);
                SMlay->addLayout(Tasklay);
            }
            if (destlist.at(i).at(3).toInt()>0)                                                                            // le message concerne un patient
            {
                QHBoxLayout *aboutlay = new QHBoxLayout();
                UpLabel *aboutdoc = new UpLabel();
                QString nomprenom ("");
                Patient *pat = m_patients->getById(destlist.at(i).at(3).toInt());
                if (pat != Q_NULLPTR)
                    nomprenom = pat->prenom() + " " + pat->nom();
                if (nomprenom != "")
                {
                    aboutdoc->setText(nomprenom);
                    aboutdoc->setId(destlist.at(i).at(3).toInt());
                    aboutlay->addWidget(aboutdoc);
                    aboutlay->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding));
                    SMlay->addLayout(aboutlay);
                }
            }

            UpTextEdit *Msgtxt = new UpTextEdit();
            Msgtxt->setStyleSheet("border: 1px solid rgb(164, 205, 255);border-radius: 5px; background-color:#FFFFFF;");
            if  (destlist.at(i).at(7).toInt() == 1)
                Msgtxt->setStyleSheet("border: 2px solid rgb(251, 51, 61);border-radius: 5px; background-color:#FFFFFF;");
            Msgtxt->setText(destlist.at(i).at(2).toString());
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
        }
        lay->addSpacerItem(new QSpacerItem(10,10,QSizePolicy::Minimum,QSizePolicy::Expanding));
    }

    // I - Les messages emis
    req =
        "select Distinct mess.idMessage, iddestinataire, " CP_TEXTMSG_MESSAGERIE ", idPatient, Tache, DateLimite, CreeLe, Urge, lu, Fait, idJointure from \n"
        TBL_MESSAGES " mess left outer join " TBL_MESSAGESJOINTURES " joint \non mess.idmessage = joint.idmessage \n"
        " where \n"
        " idemetteur = " + QString::number(m_currentuser->id()) + "\n"
        " and asupprimer is null\n"
        " order by urge desc, CreeLe desc";
    /*
    select Distinct mess.idMessage, iddestinataire, " CP_TEXTMSG_MESSAGERIE ", idPatient, Tache, DateLimite, CreeLe, Urge, lu, Fait, idJointure from
    Rufus.Messagerie mess left outer join Rufus.MessagerieJointures joint
    on mess.idmessage = joint.idmessage
    where
    idemetteur = 1
    and asupprimer is null
    order by urge desc, CreeLe desc
    */
    QList<QVariantList> emetlist = db->StandardSelectSQL(req, ok);
    if (ok && emetlist.size()>0)
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

        for (int i=0; i<emetlist.size(); i++)
        {
            QFrame *framerec = new QFrame();
            //framerec->setFrameShape(QFrame::Panel);
            QVBoxLayout *reclay = new QVBoxLayout(framerec);
            QHBoxLayout *titrelay = new QHBoxLayout();
            UpLabel *Titredoc = new UpLabel();
            Titredoc->setStyleSheet("color: green");
            if (emetlist.at(i).at(7).toInt()==1)
                Titredoc->setStyleSheet("color: red");
            QString txt = emetlist.at(i).at(6).toDate().toString(tr("d-MMM-yy")) + " " + emetlist.at(i).at(6).toTime().toString("h:mm");
            if (emetlist.at(i).at(1).toInt()>0)
                txt += tr(" pour ") + Datas::I()->users->getById(emetlist.at(i).at(1).toInt())->login();
            Titredoc->setText(txt);
            titrelay->addWidget(Titredoc);
            UpCheckBox *Rdchk = new UpCheckBox();
            Rdchk->setChecked(emetlist.at(i).at(8).toInt()==1);
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
            Modiflbl->setId(emetlist.at(i).at(0).toInt());
            Modiflbl->setPixmap(Icons::pxEditer().scaled(20,20)); //WARNING : icon scaled : pxEditer 20,20
            Modiflbl->setImmediateToolTip(tr("Modifier"));
            Modiflbl->setFixedWidth(25);
            connect(Modiflbl, QOverload<int>::of(&UpLabel::clicked),  [=] {MsgModif(Modiflbl->getId());});
            Droplay->addWidget(Modiflbl);

            UpLabel *Dellbl = new UpLabel();
            Dellbl->setId(emetlist.at(i).at(0).toInt());
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
            if (emetlist.at(i).at(4).toInt()==1)                                                                           // le message est une tâche
            {
                QHBoxLayout *Tasklay = new QHBoxLayout();
                UpLabel *Todolbl = new UpLabel();
                Tasklay->setSpacing(0);
                Todolbl->setText(tr("A effectuer avant le ") + emetlist.at(i).at(5).toDate().toString(tr("d-MMM-yy")));
                if (emetlist.at(i).at(9).toInt()!=1)
                {
                    if (QDate::currentDate() >= emetlist.at(i).at(5).toDate())
                        Todolbl->setStyleSheet("color: red");
                    else if (QDate::currentDate().addDays(3) > emetlist.at(i).at(5).toDate())
                        Todolbl->setStyleSheet("color: orange");
                }
                Tasklay->addWidget(Todolbl);
                Tasklay->addSpacerItem(new QSpacerItem(10,10,QSizePolicy::Expanding,QSizePolicy::Minimum));
                UpCheckBox *Dnchk = new UpCheckBox();
                Dnchk->setChecked(emetlist.at(i).at(9).toInt()==1);
                Dnchk->setEnabled(false);
                Dnchk->setText(tr("fait"));
                Dnchk->setFixedWidth(45);
                Tasklay->addWidget(Dnchk);
                SMlay->addLayout(Tasklay);
            }
            if (emetlist.at(i).at(3).toInt()>0)                                                                            // le message concerne un patient
            {
                QHBoxLayout *aboutlay = new QHBoxLayout();
                UpLabel *aboutdoc = new UpLabel();
                QString nomprenom ("");
                Patient *pat = m_patients->getById(emetlist.at(i).at(3).toInt());
                if (pat != Q_NULLPTR)
                    nomprenom = pat->prenom() + " " + pat->nom();
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
            Msgtxt->setText(emetlist.at(i).at(2).toString());
            QString txt1 = Msgtxt->toHtml();
            Msgtxt->setText(txt1);
            Msgtxt->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            Msgtxt->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

            Msgtxt->document()->setTextWidth(370);
            Msgtxt->setFixedSize(380,int(Msgtxt->document()->size().height())+2);
            Msgtxt->setTable(TBL_MESSAGES);
            Msgtxt->setChamp(CP_TEXTMSG_MESSAGERIE);
            Msgtxt->setId(emetlist.at(i).at(0).toInt());
            Msgtxt->installEventFilter(this);
            Msgtxt->setReadOnly(true);

            SMlay->addWidget(Msgtxt);

            Msglay->addLayout(SMlay);

            reclay->addLayout(Msglay);
            lay->addWidget(framerec);
            lay->setSpacing(3);
            reclay->setSpacing(3);
        }
        lay->addSpacerItem(new QSpacerItem(10,10,QSizePolicy::Minimum,QSizePolicy::Expanding));
    }

    if (destlist.size()==0 && emetlist.size()==0)
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

    QString req = "select userlogin from " TBL_UTILISATEURS " where iduser in (select idemetteur from " TBL_MESSAGES " where idmessage = " + QString::number(idmsg) +  ")";
    QVariantList usrdata = db->getFirstRecordFromStandardSelectSQL(req,ok);
    if (!ok || usrdata.size()==0)
    {
        UpMessageBox::Watch(this,tr("Impossible de retrouver l'expéditeur du message"));
        return;
    }
    QLabel *lbl = new QLabel(gMsgRepons);
    lbl->setText(tr("Réponse au message de ") + "<font color=\"green\"><b>" + usrdata.at(0).toString() + "</b></font>");
    globallay->addWidget(lbl);
    req = "select " CP_TEXTMSG_MESSAGERIE ", idpatient from " TBL_MESSAGES " where idmessage = " + QString::number(idmsg);
    QVariantList txtdata = db->getFirstRecordFromStandardSelectSQL(req,ok);
    if (ok && txtdata.size()>0)
    {
        QHBoxLayout *lbllayout  = new QHBoxLayout();
        UpLabel     *msglbl     = new UpLabel(gMsgRepons);
        QString nomprenom = "";
        if (txtdata.at(1).toInt()>0)
        {
            Patient *pat = m_patients->getById(txtdata.at(1).toInt());
            if (pat != Q_NULLPTR)
                nomprenom = tr("à propos de ") + "<b>" + pat->prenom() + " " + pat->nom() + "</b>";
        }
        if (nomprenom != "")
            msglbl->setText(nomprenom + "\n");
        msglbl      ->setText(msglbl->text() + txtdata.at(0).toString());
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
    int y = QGuiApplication::screens().first()->geometry().height();
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
        dlg_message(tr("Vous avez oublié de rédiger le texte de votre message!"),2000,false);
        return;
    }
    QString req = "select idemetteur, tache, datelimite, urge from " TBL_MESSAGES " where idmessage = " + QString::number(idmsg);
    QVariantList emtdata = db->getFirstRecordFromStandardSelectSQL(req,ok);
    if (!ok || emtdata.size() == 0)
        return;
    int iddest          = emtdata.at(0).toInt();
    QString tache       = ((emtdata.at(1).toInt()==1)? "1" : "null");
    QString DateLimit   = ((emtdata.at(2).toDate().isValid())? "'" + emtdata.at(2).toDate().toString("yyyy-MM-dd") + "'" : "null");
    QString Urg         = ((emtdata.at(3).toInt()==1)? "1" : "null");

    QStringList locklist;
    locklist << TBL_MESSAGES << TBL_MESSAGESJOINTURES;
    db->createtransaction(locklist);

    req  = "insert into " TBL_MESSAGES " (idEmetteur, " CP_TEXTMSG_MESSAGERIE ", CreeLe, ReponseA, Tache, Datelimite, Urge)\n values(";
    req += QString::number(m_currentuser->id()) + ", ";
    QString Reponse = "<font color = " COULEUR_TITRES ">" + gMsgRepons->findChildren<UpLabel*>().at(0)->text() + "</font>"
            + "------<br><b>" + m_currentuser->login() + ":</b> " + gMsgRepons->findChildren<UpTextEdit*>().at(0)->toPlainText().replace("\n","<br>");
    UpTextEdit txt;
    txt.setText(Reponse);
    req += "'" + Utils::correctquoteSQL(txt.toHtml()) + "', ";
    req += "'" + QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") + "', ";
    req += QString::number(idmsg) + ", ";
    req += tache + ", ";
    req += DateLimit + ", ";
    req += Urg + ")";
    //qDebug() << req;
    if (!db->StandardSQL(req, tr("Impossible d'enregistrer ce message")))
        db->rollback();

    int idrep = db->selectMaxFromTable("idMessage", TBL_MESSAGES, ok);
    if (!ok)
    {
        db->rollback();
        return;
    }
    req = "insert into " TBL_MESSAGESJOINTURES " (idMessage, idDestinataire) Values ";
    req += "(" + QString::number(idrep) + "," + QString::number(iddest) + ")";
    if (!db->StandardSQL(req, tr("Impossible d'enregistrer le message")))
    {
        db->rollback();
        return;
    }
    else
    {
        dlg_message(tr("Message enregistré"),1000,false);
        db->commit();
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
                QString req = "select " CP_TEXTMSG_MESSAGERIE ", idPatient, Tache, DateLimite, CreeLe, Urge from " TBL_MESSAGES
                              " where idMessage = " + QString::number(idmsg);
                QVariantList msgdata = db->getFirstRecordFromStandardSelectSQL(req,ok);
                QMap<QString, QVariant> map;
                map[CP_TEXTMSG_MESSAGERIE]   = msgdata.at(0).toString();
                map["idPatient"]            = msgdata.at(1).toInt();
                map["Tache"]                = msgdata.at(2).toInt();
                map["DateLimite"]           = msgdata.at(3).toDate();
                map["CreeLe"]               = msgdata.at(4).toDateTime();
                map["Urge"]                 = msgdata.at(5).toInt();
                map["null"]                 = false;

                QStringList listdestinataires;
                req = "select iddestinataire from " TBL_MESSAGESJOINTURES " where idmessage = " + QString::number(idmsg);
                QList<QVariantList> destlist = db->StandardSelectSQL(req,ok);
                for (int i=0; i<destlist.size();i++)
                    listdestinataires << destlist.at(i).at(0).toString();
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
    db->StandardSQL("update " TBL_MESSAGESJOINTURES " set Fait = " + res + " where idjointure = " + QString::number(idjoin));
}

void Rufus::MsgRead(UpCheckBox *chk)
{
    int idjoin = chk->iD();
    QString res = (chk->isChecked()? "1" : "NULL");
    db->StandardSQL("update " TBL_MESSAGESJOINTURES " set Lu = " + res + " where idjointure = " + QString::number(idjoin));
}

void Rufus::SupprimerMessageEmis(int idMsg)
{
    QString req = "update " TBL_MESSAGES " set ASupprimer = 1 where idmessage = " + QString::number(idMsg);
    db->StandardSQL(req);
    req = "delete from " TBL_MESSAGESJOINTURES " where "
          "idmessage = " + QString::number(idMsg) +
          " and iddestinataire = " + QString::number(m_currentuser->id());
    db->StandardSQL(req);
    if (gMsgDialog->findChildren<QScrollArea*>().size()>0)
        AfficheMessages(1);
}

void Rufus::SupprimerMessageRecu(int idJoint)
{
    QString req = "select idmessage from " TBL_MESSAGESJOINTURES  " where idjointure = " + QString::number(idJoint);
    QVariantList msgdata = db->getFirstRecordFromStandardSelectSQL(req,ok);
    int idmsg = msgdata.at(0).toInt();
    req = "select idemetteur from " TBL_MESSAGES  " where idmessage = " + QString::number(idmsg);
    QVariantList usrdata = db->getFirstRecordFromStandardSelectSQL(req,ok);
    int idusr = usrdata.at(0).toInt();
    if (idusr == m_currentuser->id())
        db->StandardSQL("update " TBL_MESSAGES " set ASupprimer = 1 where idmessage = " + QString::number(idmsg));
    db->StandardSQL("delete from " TBL_MESSAGESJOINTURES " where idjointure = " + QString::number(idJoint));
    req = "delete from " TBL_MESSAGES " where "
          "idmessage not in (select idmessage from " TBL_MESSAGESJOINTURES ") "
          " and ASupprimer = 1";
    db->StandardSQL(req);
    if (gMsgDialog->findChildren<QScrollArea*>().size()>0)
        AfficheMessages();
}

void Rufus::ReconstruitListeMessages()
{
    QDateTime DateMsg;
    gTotalNvxMessages = 0;
    QString req =
            "select Distinct mess.idMessage, Creele, ReponseA from "
            TBL_MESSAGES " mess left outer join " TBL_MESSAGESJOINTURES " joint on mess.idmessage = joint.idmessage \n"
                                                                                    " where \n"
                                                                                    " iddestinataire = " + QString::number(m_currentuser->id()) + "\n"
                                                                                    " or (idemetteur = " + QString::number(m_currentuser->id()) + " and asupprimer is null)"
                                                                                    " order by CreeLe";
    /*
select Distinct mess.idMessage, Creele, ReponseA from Rufus.Messagerie mess left outer join Rufus.MessagerieJointures joint on mess.idmessage = joint.idmessage
where iddestinataire = 1
or (idemetteur = 1 and asupprimer is null)
order by CreeLe
*/
    QList<QVariantList> msglist = db->StandardSelectSQL(req,ok);
    gTotalMessages = msglist.size();
    gMessageIcon->setVisible(gTotalMessages>0);
    if (gTotalMessages>0)
    {
        for (int i=0; i<gTotalMessages; i++)
        {
            DateMsg = QDateTime(msglist.at(i).at(1).toDate(), msglist.at(i).at(1).toTime());
            if (DateMsg > QDateTime(gUserDateDernierMessage))
                gTotalNvxMessages += 1;
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

void Rufus::VerifMessages()
{
    if (!QSystemTrayIcon::isSystemTrayAvailable())
        return;
    int flagmsg = Flags::I()->flagMessages();
    if (m_flagmessages < flagmsg)
    {
        ReconstruitListeMessages();
        m_flagmessages = flagmsg;
    }
}

void Rufus::VerifSalleDAttente()
{
    int flagsaldat = Flags::I()->flagSalleDAttente();

    if (m_flagsalledattente < flagsaldat)
    {
        m_flagsalledattente = flagsaldat;
        Remplir_SalDat();                       // par le timer VerifSalleDAttente
    }
}

void Rufus::VerifCorrespondants()
{
    if (Datas::I()->patients->currentpatient()->id() == 0)
        return;
    int flagcor = Flags::I()->flagCorrespondants();
    if (m_flagcorrespondants < flagcor)
    {
        m_flagcorrespondants = flagcor;
        // on reconstruit la liste des MG et des correspondants
        ReconstruitCombosCorresp();                     // par le timer gTimerCorrespondants
        // on resynchronise l'affichage du combobox au besoin
        if (ui->tabWidget->indexOf(ui->tabDossier) > -1)
        {
            int idx (-1), idxsp1 (-1), idxsp2(-1);
            QString req = "select idcormedmg, idcormedspe1, idcormedspe2 from " TBL_RENSEIGNEMENTSMEDICAUXPATIENTS " where idpat = " + QString::number(Datas::I()->patients->currentpatient()->id());
            QVariantList cordata = db->getFirstRecordFromStandardSelectSQL(req,ok);
            if (!ok)
                return;
            if (cordata.size()>0)
            {
                if (cordata.at(0) != QVariant())
                    idx = ui->MGupComboBox->findData(cordata.at(0).toInt());
                if (cordata.at(1) != QVariant())
                    idxsp1 = ui->AutresCorresp1upComboBox->findData(cordata.at(1).toInt());
                if (cordata.at(2) != QVariant())
                    idxsp2 = ui->AutresCorresp1upComboBox->findData(cordata.at(2).toInt());
            }
            ui->MGupComboBox->setCurrentIndex(idx);
            ui->AutresCorresp1upComboBox->setCurrentIndex(idxsp1);
            ui->AutresCorresp2upComboBox->setCurrentIndex(idxsp2);
            OKModifierTerrain(Datas::I()->patients->currentpatient());
        }
    }
}

void Rufus::VerifVerrouDossier()
{
    /* Cette fonction sert à déconnecter et lever les verrous d'un utilisateur qui se serait déconnecté accidentellement
     * elle n'est utilisée qu'en cas de non utilisation du tcp
     on fait la liste des utilisateurs qui n'ont pas remis à jour leur connexion depuis plus de 60 secondes,
     on retire les verrous qu'ils auraient pu poser et on les déconnecte*/
    Datas::I()->postesconnectes->initListe();
    Datas::I()->patientsencours->initListeAll();
    QDateTime timenow = db->ServerDateTime();
    QList<PosteConnecte*> listpostsAEliminer = QList<PosteConnecte*>();
    foreach(PosteConnecte* post, *Datas::I()->postesconnectes->postesconnectes())
    {
        /*
        qDebug() << "post->id()" << post->id();
        qDebug() << "m_currentuser->id()" << m_currentuser->id();
        qDebug() << "post->macadress()" << post->macadress();
        qDebug() << "Utils::getMACAdress()" << Utils::getMACAdress();
        */
        qint64 tempsecouledepuisactualisation = post->heurederniereconnexion().secsTo(timenow);
        if (tempsecouledepuisactualisation > 120)
        {
            qDebug() << "VerifVerrouDossier()" << timenow;
            qDebug() << "VerifVerrouDossier()" << post->heurederniereconnexion();
            qDebug() << "VerifVerrouDossier()" << tempsecouledepuisactualisation;
            qDebug() << "VerifVerrouDossier()" << post->stringid();
            qDebug() << "post->id()" << post->id();
            qDebug() << "m_currentuser->id()" << m_currentuser->id();
            qDebug() << "post->macadress()" << post->macadress();
            qDebug() << "Utils::getMACAdress()" << Utils::getMACAdress();
            //l'utilisateur n'a pas remis sa connexion aà jour depuis plus de 120 secondes
            //on déverrouille les dossiers verrouillés par cet utilisateur et on les remet en salle d'attente
            QString blabla              = ENCOURSEXAMEN;
            int length                  = blabla.size();
            foreach (PatientEnCours* pat, m_listepatientsencours->patientsencours()->values())
            {
                if (pat != Q_NULLPTR)
                    if (pat->iduserencoursexam() == post->id() && pat->statut().left(length) == ENCOURSEXAMEN && pat->posteexamen() == post->nomposte())
                    {
                        ItemsList::update(pat, CP_STATUT_SALDAT, ARRIVE);
                        ItemsList::update(pat, CP_POSTEEXAMEN_SALDAT);
                        ItemsList::update(pat, CP_IDUSERENCOURSEXAM_SALDAT);
                    }
            }
            if (!listpostsAEliminer.contains(post))
                listpostsAEliminer << post;
        }
        if (post->id() != m_currentuser->id() && post->macadress() == Utils::getMACAdress())
            if (!listpostsAEliminer.contains(post))
                listpostsAEliminer << post;
    }
    if (listpostsAEliminer.size() > 0)
    {
       foreach (PosteConnecte* post, listpostsAEliminer)
       {
           QString nomposte = post->nomposte();
           Datas::I()->postesconnectes->SupprimePosteConnecte(post);
           dlg_message(tr("Le poste ") + nomposte + tr(" a été retiré de la liste des postes connectés actuellement au serveur"),1000);
       }
       Flags::I()->MAJFlagSalleDAttente();
    }

    // on donne le statut "arrivé" aux patients en salle d'attente dont le iduserencourssexam n'est plus present sur ce poste examen dans la liste des users connectes
    QString blabla              = ENCOURSEXAMEN;
    int length                  = blabla.size();
    QList<PatientEnCours*> listpatasupprimer = QList<PatientEnCours*>();
    foreach (PatientEnCours *pat, m_listepatientsencours->patientsencours()->values())
    {
        if (pat != Q_NULLPTR)
        {
            if (pat->statut().left(length) == ENCOURSEXAMEN)
            {
                bool posttrouve = false;
                foreach(PosteConnecte* post, *Datas::I()->postesconnectes->postesconnectes())
                {
                    if (post->id() == pat->iduser() && post->nomposte() == pat->posteexamen())
                    {
                        posttrouve = true;
                        break;;
                    }
                }
                if (!posttrouve)
                {
                    ItemsList::update(pat, CP_STATUT_SALDAT, ARRIVE);
                    ItemsList::update(pat, CP_POSTEEXAMEN_SALDAT);
                    ItemsList::update(pat, CP_IDUSERENCOURSEXAM_SALDAT);
                }
            }
        }
        // on retire de la salle d'attente les patients qui n'existent pas
        if (m_patients->getById(pat->id(), Item::NoLoadDetails) == Q_NULLPTR)
            listpatasupprimer << pat;
    }
    if (listpatasupprimer.size() > 0)
        for (int i=0; i<listpatasupprimer.size(); i++)
            m_listepatientsencours->SupprimePatientEnCours(listpatasupprimer.at(i));

}

bool Rufus::isPosteImport()
{
    return PosteImport;
}

void Rufus::VerifImportateur()  //!< uniquement utilisé quand le TCP n'est pas utilisé et ne mode réseau local ou monoposte
{
     /*! s'il n'existe pas de poste défini comme importateur des documents, on prend la place si on est accrédite et qu'on n'est pas en accès distant
     * s'il existe un poste défini comme importateur des docs et qu'il est différent de ce poste,
     * on vérifie
        * s'il est toujours actif
        * et si oui, s'il est prioritaire
     * si ce n'est pas le cas, on prend sa place
        * si on est prioritaire et pas lui
        * s'il n'est pas administrateur
        */
    if (db->getMode() == DataBase::Distant)
    {
        if (ImportDocsExtThread == Q_NULLPTR)
        {
            ImportDocsExtThread = new ImportDocsExternesThread(proc);
            connect(ImportDocsExtThread, SIGNAL(emitmsg(QStringList, int, bool)), this, SLOT(AfficheMessageImport(QStringList, int, bool)));
        }
        PosteImport = true;
        return;
    }

    bool statut = isPosteImport();
    QString ImportateurDocs = proc->PosteImportDocs(); //le nom du poste importateur des docs externes
    if (ImportateurDocs.toUpper() == "NULL")
    {
        if ((proc->gsettingsIni->value("BDD_LOCAL/PrioritaireGestionDocs").toString() == "YES" || proc->gsettingsIni->value("BDD_LOCAL/PrioritaireGestionDocs").toString() == "NORM")
                && db->getMode() != DataBase::Distant)
             proc->setPosteImportDocs();
    }
    else
    {
        QString Adr = "";
        QString B = proc->gsettingsIni->value("BDD_LOCAL/PrioritaireGestionDocs").toString();
        if (B=="YES")
            Adr = QHostInfo::localHostName() + " - prioritaire";
        else if (B=="NORM")
            Adr = QHostInfo::localHostName();

        if (ImportateurDocs != Adr) //si le poste défini comme importateur des docs est différent de ce poste, on vérifie qu'il est toujours actif et qu'il n'est pas prioritaire
        {
            // on vérifie que l'importateur est toujours connecté
            int idx = -1;
            foreach (PosteConnecte* post, Datas::I()->postesconnectes->postesconnectes()->values())
            {
                if (post->nomposte() == ImportateurDocs.remove(" - prioritaire"))
                {
                    idx = Datas::I()->postesconnectes->postesconnectes()->values().indexOf(post);
                    break;
                }
            }
            if (idx<0)
            {
                /*Le poste défini comme importateur des docs externes n'est pas connecté,
                 on prend la place si
                    on n'est pas en accès distant
                    et si on est importateur
                sinon, on retire le poste*/
                proc->setPosteImportDocs((B == "YES" || B == "NORM") && db->getMode() != DataBase::Distant);
            }
            else if (!ImportateurDocs.contains(" - " NOM_ADMINISTRATEURDOCS))
                // le poste défini comme importateur est valide mais pas administrateur, on prend sa place si
                //  on est prioritaire et pas lui
                //  à condition de ne pas être en accès distant
            {
                if (B == "YES" && !ImportateurDocs.contains(" - prioritaire") && db->getMode() != DataBase::Distant)
                    proc->setPosteImportDocs();
                else if (ImportateurDocs.remove(" - prioritaire") == QHostInfo::localHostName()) // cas rare du poste qui a modifié son propre statut
                    proc->setPosteImportDocs((B == "YES" || B == "NORM") && db->getMode() != DataBase::Distant);
            }
        }
    }
    PosteImport = (proc->PosteImportDocs().remove(" - prioritaire") == QHostInfo::localHostName());
    bool chgtstatut = (statut != isPosteImport());
    if (chgtstatut)
    {
        if (isPosteImport())
        {
            connect(gTimerExportDocs,           &QTimer::timeout,   this,   &Rufus::ExporteDocs);
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
    if (ui->tabWidget->indexOf(ui->tabDossier)<1)
        return;
    OuvrirDocsExternes(Datas::I()->patients->currentpatient());   // depuis le timer gTimerVerifGestDocs ou le bouton ui->OuvreDocsExternspushButton
}

//-------------------------------------------------------------------------------------
// Interception des évènements clavier
//-------------------------------------------------------------------------------------
void Rufus::keyPressEvent (QKeyEvent * event )
{
        switch (event->key()) {
        case Qt::Key_F3:
            RefractionMesure();
            break;
        case Qt::Key_F4:
            Tonometrie();
            break;
        case Qt::Key_F5:
            if (AutorDepartConsult(true))
            {
                if (ui->tabWidget->indexOf(ui->tabDossier) > 0)
                    ui->tabWidget->removeTab(ui->tabWidget->indexOf(ui->tabDossier));
                FermeDlgActesPrecedentsEtDocsExternes();
                ModeSelectDepuisListe();
            }
            break;
        case Qt::Key_F6:
            if (AutorDepartConsult(true))
            {
                if (ui->tabWidget->indexOf(ui->tabDossier) > 0)
                    ui->tabWidget->removeTab(ui->tabWidget->indexOf(ui->tabDossier));
                FermeDlgActesPrecedentsEtDocsExternes();
                ModeCreationDossier();
            }
            break;
        case Qt::Key_F9:
            if (ui->tabWidget->indexOf(ui->tabDossier) < 0 && m_currentuser->isSoignant())
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
                    CreerDossier();
                else if (ui->PatientsListeTableView->selectionModel()->selectedIndexes().size() > 0)
                    ChoixDossier(getPatientFromSelectionInTable());
            }

        }
    }
}

Patient* Rufus::getPatientFromRow(int row)
{
    QModelIndex psortindx = m_listepatientsproxymodel->index(row, 0);
    return getPatientFromIndex(psortindx);
}

Patient* Rufus::getPatientFromSelectionInTable()
{
    if (ui->PatientsListeTableView->selectionModel()->selectedIndexes().size() == 0)
        return Q_NULLPTR;
    QModelIndex psortindx   = ui->PatientsListeTableView->selectionModel()->selectedIndexes().at(0);    //  -> listepatientsproxymodel
    return getPatientFromIndex(psortindx);
}

Patient* Rufus::getPatientFromCursorPositionInTable()
{
    QModelIndex psortindx   = ui->PatientsListeTableView->indexAt(ui->PatientsListeTableView->viewport()->mapFromGlobal(cursor().pos()));
    Patient *pat = getPatientFromIndex(psortindx);
    //qDebug() << "getPatientFromCursorPositionInTable() " << pat->nom()  << pat->prenom() << pat->id();
    return pat;
}

Patient* Rufus::getPatientFromIndex(QModelIndex idx)
{
    QModelIndex pprenomindx = m_listepatientsproxymodel->mapToSource(idx);                              //  -> prenomsortmodel
    QModelIndex pDDNindx    = m_prenomfiltersortmodel->mapToSource(pprenomindx);                        //  -> DDNsortmodel
    QModelIndex pindx       = m_DDNsortmodel->mapToSource(pDDNindx);                                    //  -> m_listepatientsmodel

    UpStandardItem *upitem = dynamic_cast<UpStandardItem *>(m_listepatientsmodel->itemFromIndex(pindx));
    if (upitem == Q_NULLPTR)
        return Q_NULLPTR;
    if (upitem->item() == Q_NULLPTR)
    {
        qDebug() << "erreur sur l'item - row = " << upitem->row() << " - col = " << upitem->column() << upitem->text();
        return Q_NULLPTR;
    }
    Patient *pat = dynamic_cast<Patient *>(upitem->item());
    return pat;
}

int Rufus::getRowFromPatient(Patient *pat)
{
    int row = -1;
    if (pat == Q_NULLPTR)
        return row;
    for (int i=0; i< m_listepatientsproxymodel->rowCount(); ++i)
        if (getPatientFromRow(i)->id() == pat->id())
        {
            row = i;
            break;
        }
    return row;
}

// ------------------------------------------------------------------------------------------
// Interception des évènements internes
// ------------------------------------------------------------------------------------------

bool Rufus::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::FocusIn )
    {
        if (obj == ui->ActeMontantlineEdit)         gActeMontant    = QLocale().toString(QLocale().toDouble(ui->ActeMontantlineEdit->text()),'f',2);
        if (obj == ui->ActeCotationcomboBox)        gActeMontant    = QLocale().toString(QLocale().toDouble(ui->ActeMontantlineEdit->text()),'f',2);
        if (obj == ui->ActeDatedateEdit)            gActeDate       = ui->ActeDatedateEdit->text();
    }

    if (event->type() == QEvent::FocusOut )
    {
        UpTextEdit* objUpText = dynamic_cast<UpTextEdit*>(obj);
        if (objUpText != Q_NULLPTR)
        {
            QString requetemodif;
            objUpText->textCursor().clearSelection();
            if (objUpText->getValeurAvant() != objUpText->toHtml())
            {
                QString Corps = objUpText->toHtml();
                if (objUpText->getTableCorrespondant() == TBL_ACTES || objUpText->getTableCorrespondant() == TBL_MESSAGES)
                {
                    Corps.replace("<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">","<p style=\" margin-top:0px; margin-bottom:0px;\">");
                    Corps.remove("border=\"0\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px;\" ");
                    Corps.remove(HTMLCOMMENT_LINUX);
                    Corps.remove(HTMLCOMMENT_MAC);
        #ifdef Q_OS_LINUX
                    Corps.append(HTMLCOMMENT_LINUX);
        #endif
        #ifdef Q_OS_MAC
                    Corps.append(HTMLCOMMENT_MAC);
        #endif
                    if (objUpText->getTableCorrespondant() == TBL_ACTES)
                        ItemsList::update(m_currentact, objUpText->getChampCorrespondant(), Corps);
                    else if (objUpText->getTableCorrespondant() == TBL_MESSAGES)
                    {
                        requetemodif =   "UPDATE " + objUpText->getTableCorrespondant() + " SET " + objUpText->getChampCorrespondant() + " = '"
                                + Utils::correctquoteSQL(Corps) + "' WHERE idMessage = " + QString::number(m_currentact->id());
                        db->StandardSQL(requetemodif, tr("Impossible de mettre à jour le champ ") + objUpText->getChampCorrespondant() + "!");
                    }
                }
                else if (objUpText->getTableCorrespondant() == TBL_RENSEIGNEMENTSMEDICAUXPATIENTS)
                {
                    Datas::I()->patients->updatePatientData(Datas::I()->patients->currentpatient(), objUpText->getChampCorrespondant(), objUpText->toPlainText());
                }
            }
        }
        else
        {
            UpLineEdit* objUpLine = dynamic_cast<UpLineEdit*>(obj);
            if (objUpLine != Q_NULLPTR && obj != MGlineEdit && obj != AutresCorresp1LineEdit && obj != AutresCorresp2LineEdit)
            {
                if (obj == ui->ActeMontantlineEdit)
                    // le contrôle de sortie pour ActeMontantlineEdit est traité la méthode ValideActeMontant();
                    return QWidget::eventFilter(obj, event);
                QString requetemodif;
                if (objUpLine->getValeurAvant() != objUpLine->text())
                {
                    objUpLine->setText(Utils::trimcapitilize(objUpLine->text(),true));
                    if (objUpLine->getTableCorrespondant() == TBL_ACTES)
                    {
                        ItemsList::update( m_currentact, objUpLine->getChampCorrespondant(),objUpLine->text());
                    }
                    else if (objUpLine->getTableCorrespondant() == TBL_RENSEIGNEMENTSMEDICAUXPATIENTS)
                    {
                        Datas::I()->patients->updatePatientData(Datas::I()->patients->currentpatient(), objUpLine->getChampCorrespondant(), objUpLine->text());
                        OKModifierTerrain(Datas::I()->patients->currentpatient());
                    }
                }
            }
            else if (obj == ui->ActeDatedateEdit)
            {
                if (ui->ActeDatedateEdit->text() != gActeDate)
                {
                    ItemsList::update(m_currentact, CP_DATE_ACTES, ui->ActeDatedateEdit->date());
                    gActeDate       = ui->ActeDatedateEdit->text();
                }
                ui->ActeDatedateEdit->setEnabled(false);
            }
            else if (obj == ui->MGupComboBox || obj != ui->AutresCorresp1upComboBox || obj != ui->AutresCorresp2upComboBox)
                MAJCorrespondant(obj);
        }
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
                if (objUpText->getTableCorrespondant() == TBL_MESSAGES)
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
                FiltreTable(ui->CreerNomlineEdit->text(), ui->CreerPrenomlineEdit->text());
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
void Rufus::AfficheActe(Acte* acte)
{
    int nbActes (0);
    int noActe (0);
    if (acte == Q_NULLPTR)          // Aucune consultation trouvee pour ce  patient
    {
        ui->Acteframe->setVisible(false);
        ui->CreerActepushButton_2->setVisible(true);
        ui->CreerBOpushButton_2->setVisible(true);
        ui->idActelineEdit->clear();
        return;
    }
    else
    {
        m_currentact    = acte;
        //1.  Retrouver l'acte défini par son idActe et afficher les champs
        ui->Acteframe               ->setVisible(true);
        ui->CreerActepushButton_2   ->setVisible(false);
        ui->CreerBOpushButton_2     ->setVisible(false);

        ui->ActeDatedateEdit        ->setDate(acte->date());
        ui->ActeDatedateEdit        ->setEnabled(false);
        ui->ActeMotiftextEdit       ->setText(acte->motif());
        ui->ActeTextetextEdit       ->setText(acte->texte());
        ui->ActeConclusiontextEdit  ->setText(acte->conclusion());
        ui->idActelineEdit          ->setText(QString::number(acte->id()));
        ui->CourrierAFairecheckBox  ->setChecked(acte->courrierAFaire());

        ui->ActeCotationcomboBox    ->disconnect();                       //! il faut faire ça pour éviter un foutoir de messages quand on navigue d'un acte à l'autre dans le dossier du patient
        ui->ActeCotationcomboBox    ->setCurrentText(acte->cotation());
        ConnectCotationComboBox();
        // on affiche tous les montants en euros, même ce qui a été payé en francs.
        double H = 1;
        if (acte->isFactureEnFranc())
        {
            ui->ActeMontantLabel    ->setText(tr("Montant (€)\n(payé en F)"));
            H = 6.55957;
        }
        else
            ui->ActeMontantLabel    ->setText(tr("Montant (€)"));
        double MontantActe = acte->montant()/H;
        ui->ActeMontantlineEdit     ->setText(QLocale().toString(MontantActe,'f',2));
        int idx = ui->ActeCotationcomboBox->findText(acte->cotation());
        if (idx>0)
        {
            QStringList listMontantActe = ui->ActeCotationcomboBox->itemData(idx).toStringList();
            double MontantConv, MontantPrat;
            MontantConv = listMontantActe.at(0).toDouble();
            MontantPrat = listMontantActe.at(1).toDouble();
            ui->BasculerMontantpushButton->setVisible((fabs(MontantActe)!=fabs(MontantConv))
                                                      || (fabs(MontantActe)!=fabs(MontantPrat))
                                                      || (fabs(MontantConv)!=fabs(MontantPrat)));
            if (ui->BasculerMontantpushButton->isVisible())
            {
                if (fabs(MontantActe)!=fabs(MontantPrat))
                    ui->BasculerMontantpushButton->setImmediateToolTip("Revenir au tarif habituellement pratiqué");
                else
                    ui->BasculerMontantpushButton->setImmediateToolTip("Revenir au tarif conventionnel");
            }
        }

        ui->CreerActepushButton     ->setToolTip(tr("Créer un nouvel acte pour ") + Datas::I()->patients->currentpatient()->prenom() + " " + Datas::I()->patients->currentpatient()->nom());
        ui->CreerBOpushButton       ->setToolTip(tr("Créer un bilan orthoptique pour ") + Datas::I()->patients->currentpatient()->prenom() + " " + Datas::I()->patients->currentpatient()->nom());

        QMap<QString,QVariant>  Age = Utils::CalculAge(Datas::I()->patients->currentpatient()->datedenaissance(), ui->ActeDatedateEdit->date());
        ui->AgelineEdit             ->setText(Age["toString"].toString());
        ui->AgelineEdit             ->setAlignment(Qt::AlignCenter);

        //2. retrouver le créateur de l'acte et le médecin superviseur de l'acte
        ui->CreeParlineEdit         ->setText(tr("Créé par ") + Datas::I()->users->getById(acte->idCreatedBy())->login()
                                     + tr(" pour ") + Datas::I()->users->getById(acte->idUser())->login());

        //3. Mettre à jour le numéro d'acte
        if (m_listeactes->actes()->size() > 0)           // Il y a des consultations
        {
            nbActes = m_listeactes->actes()->size();
            if (nbActes == 1)
            {
                ui->ActePrecedentpushButton ->setEnabled(false);
                ui->ActeSuivantpushButton   ->setEnabled(false);
                ui->PremierActepushButton   ->setEnabled(false);
                ui->DernierActepushButton   ->setEnabled(false);
                noActe = 1;
                ui->OuvreActesPrecspushButton->setEnabled(false);
                ui->OuvreActesPrecspushButton->setToolTip("");
            }
            else if (nbActes > 1)
            {
                noActe = m_listeactes->actes()->keys().indexOf(acte->id()) + 1;
                if (noActe == 1)
                {
                    ui->ActePrecedentpushButton->setEnabled(false);
                    ui->PremierActepushButton->setEnabled(false);
                    ui->ActeSuivantpushButton->setEnabled(true);
                    ui->DernierActepushButton->setEnabled(true);
                }
                else if (noActe == nbActes)
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
                ui->OuvreActesPrecspushButton->setEnabled(true);
                ui->OuvreActesPrecspushButton->setToolTip(tr("Voir les consultations précédentes de ") + Datas::I()->patients->currentpatient()->prenom() + " " + Datas::I()->patients->currentpatient()->nom());
            }
            ui->NoActelabel->setText(QString::number(noActe) + " / " + QString::number(nbActes));
        }

        //4. régler la scrollbar
        ui->ScrollBar->setVisible(m_listeactes->actes()->size() > 1);
        ui->ScrollBar->disconnect();
        ui->ScrollBar->setMinimum(0);
        ui->ScrollBar->setMaximum(m_listeactes->actes()->size() - 1);
        ui->ScrollBar->setSingleStep(1);
        ui->ScrollBar->setValue(noActe-1);
        if( ui->ScrollBar->maximum() > 0 )
            connect(ui->ScrollBar, &QScrollBar::valueChanged, this, [=](int newValue) {
                Acte *act = m_listeactes->actes()->values().at(newValue);
                if (AutorDepartConsult(false))
                    AfficheActe(act);
            });

    }
    gAutorModifConsult = false;
    AfficheActeCompta(m_currentact);
}

/*------------------------------------------------------------------------------------------------------------------------------------
-- Afficher les renseignements comptables de l'acte en cours -------------------------------------------------------------------------
------------------------------------------------------------------------------------------------------------------------------------*/
void Rufus::AfficheActeCompta(Acte *acte)
{
    if (acte == Q_NULLPTR)
        return;

    bool a = (acte->paiementType() == "");

    ui->Comptaframe->setVisible(!a);
    ui->Cotationframe->setEnabled(a);
    ui->CCAMlinklabel->setVisible(a);
    ui->EnregistrePaiementpushButton->setVisible(a && (m_currentuser->isSoignant() && !m_currentuser->isAssistant()));
    ui->ModifierCotationActepushButton->setVisible(!a);
    if (a) // seul le superviseur de l'acte ou son parent peuvent modifier sa cotation
    {
        int iduser = m_currentact->idUser();
        int idparent = m_currentact->idParent();
        ui->EnregistrePaiementpushButton->setEnabled(ui->ActeCotationcomboBox->lineEdit()->text()!=""
                                                    && ( iduser == m_currentuser->id() || idparent == m_currentuser->id()));
        return;
    }

    //2. on recherche ensuite le type de paiement : espèces, chèque, tiers, cb, impayé, gratuit

    if (acte->paiementType() == "T"  && acte->paiementTiers() != "CB") ui->PaiementlineEdit->setText(acte->paiementTiers());

    QString txtpaiement = Utils::ConvertitModePaiement(acte->paiementType());
    if (acte->paiementType() == "T"
            && acte->paiementTiers() == "CB") txtpaiement = tr("carte de crédit");
     else if (acte->paiementType() == "T") txtpaiement = acte->paiementTiers();
    ui->PaiementlineEdit->setText(txtpaiement);

    // on calcule le montant payé pour l'acte
    if (acte->paiementType() != "G" || acte->paiementType() != "I")
    {
        double TotalPaye = 0;
        foreach (LignePaiement *lign, m_lignespaiements->lignespaiements()->values())
        {
            if (lign->idacte() == acte->id())
            {
                if (lign->monnaie() == "F")
                    TotalPaye += lign->paye() / 6.55957;
                else
                    TotalPaye += lign->paye();
            }
        }
        ui->PayelineEdit->setText(QLocale().toString(TotalPaye,'f',2));
    }

    if (acte->paiementType() == "B"
        ||(acte->paiementType() == "T"
            && acte->paiementTiers() == "CB")
        || acte->paiementType() == "C"
        || acte->paiementType() == "E")
    {
        ui->PaiementLabel->setVisible(true);
        ui->PaiementLabel->setText("Paiement:");
        ui->PayeLabel->setVisible(true);
        ui->PayelineEdit->setVisible(true);
        ui->Comptaframe->setGeometry(580,510,180,50);
        ui->PaiementlineEdit->setGeometry(79,4,91,18);
        return;
    }
    if (acte->paiementType() == "G"
        || acte->paiementType() == "I")
    {
        ui->PaiementLabel->setVisible(false);
        ui->PayeLabel->setVisible(false);
        ui->PayelineEdit->setVisible(false);
        ui->Comptaframe->setGeometry(580,510,180,26);
        ui->PaiementlineEdit->setGeometry(8,4,164,18);
        return;
    }
    if (acte->paiementType() == "T"
            && acte->paiementTiers() != "CB")
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
void Rufus::AfficheDossier(Patient *pat, int idacte)
{
//  Afficher les éléments de la tables Patients
    if (pat == Q_NULLPTR)
        return;
    Datas::I()->patients->setcurrentpatient(pat->id());
    QString     Msg;

    //qDebug() << "AfficheDossier() " +  Datas::I()->patients->currentpatient()->nom() + " " + Datas::I()->patients->currentpatient()->prenom() + " - id = " + QString::number(Datas::I()->patients->currentpatient()->id());
    ui->DateCreationDossierlineEdit->setText(Datas::I()->patients->currentpatient()->datecreationdossier().toString(tr("d-M-yyyy")));
    ui->idPatientlineEdit->setText(QString::number(Datas::I()->patients->currentpatient()->id()));
    if (!ui->tabDossier->isVisible())
    {
        ui->tabWidget->insertTab(1,ui->tabDossier,"");
        ui->tabWidget->setCurrentIndex(ui->tabWidget->indexOf(ui->tabDossier));
    }

    ui->IdCreateurDossierlineEdit->setText(Datas::I()->users->getById(Datas::I()->patients->currentpatient()->idcreateur())->login());

    ui->IdentPatienttextEdit->setHtml(CalcHtmlIdentificationPatient(Datas::I()->patients->currentpatient()));
    ui->tabWidget->setTabIcon(ui->tabWidget->indexOf(ui->tabDossier),CalcIconPatient(Datas::I()->patients->currentpatient()));
    ui->tabWidget->setTabText(ui->tabWidget->indexOf(ui->tabDossier) ,Datas::I()->patients->currentpatient()->nom() + " " + Datas::I()->patients->currentpatient()->prenom());

    //3 - récupération des données médicales

    ui->AtcdtsPersostextEdit->setText(Datas::I()->patients->currentpatient()->atcdtspersos());
    ui->TtGeneraltextEdit->setText(Datas::I()->patients->currentpatient()->traitementgen());
    ui->AtcdtsFamiliauxtextEdit->setText(Datas::I()->patients->currentpatient()->atcdtsfamiliaux());
    ui->AtcdtsOphstextEdit->setText(Datas::I()->patients->currentpatient()->atcdtsophtalmos());
    ui->TabaclineEdit->setText(Datas::I()->patients->currentpatient()->tabac());
    ui->AutresToxiquestextEdit->setText(Datas::I()->patients->currentpatient()->toxiques());

    MGlineEdit->clear();
    AutresCorresp1LineEdit->clear();
    AutresCorresp2LineEdit->clear();
    QString tooltp = "";
    if (Datas::I()->patients->currentpatient()->idmg()>0)
    {
        int id = Datas::I()->patients->currentpatient()->idmg();
        ui->MGupComboBox->setCurrentIndex
                (ui->MGupComboBox->findData(id));
        tooltp = CalcToolTipCorrespondant(id);
    }
    else
        ui->MGupComboBox->setCurrentIndex(-1);
    ui->MGupComboBox->setImmediateToolTip(tooltp);
    tooltp = "";
    if (Datas::I()->patients->currentpatient()->idspe1()>0)
    {
        int id = Datas::I()->patients->currentpatient()->idspe1();
        ui->AutresCorresp1upComboBox->setCurrentIndex
                (ui->AutresCorresp1upComboBox->findData(id));
        tooltp = CalcToolTipCorrespondant(id);
    }
    else
        ui->AutresCorresp1upComboBox->setCurrentIndex(-1);
    ui->AutresCorresp1upComboBox->setImmediateToolTip(tooltp);
    tooltp = "";
    if (Datas::I()->patients->currentpatient()->idspe2()>0)
    {
        int id = Datas::I()->patients->currentpatient()->idspe2();
        ui->AutresCorresp2upComboBox->setCurrentIndex
                (ui->AutresCorresp2upComboBox->findData(id));
        tooltp = CalcToolTipCorrespondant(id);
    }
    else
        ui->AutresCorresp2upComboBox->setCurrentIndex(-1);
    ui->AutresCorresp2upComboBox->setImmediateToolTip(tooltp);
    ui->ImportanttextEdit->setText(Datas::I()->patients->currentpatient()->important());
    ui->ResumetextEdit->setText(Datas::I()->patients->currentpatient()->resume());
    ui->TtOphtextEdit->setText(Datas::I()->patients->currentpatient()->traitementoph());
    OKModifierTerrain(Datas::I()->patients->currentpatient(), false);

    FermeDlgActesPrecedentsEtDocsExternes();

    //3 - récupération des actes

    m_listeactes->initListeByPatient(Datas::I()->patients->currentpatient());
    m_lignespaiements->initListeByPatient(Datas::I()->patients->currentpatient());

    if (m_listeactes->actes()->size() == 0)
    {
        ui->Acteframe->setVisible(false);
        ui->CreerActepushButton_2->setVisible(true);
        ui->CreerBOpushButton_2->setVisible(true);
        ui->idActelineEdit->clear();
        m_currentact = Q_NULLPTR;
    }
    else
    {
        OuvrirDocsExternes(Datas::I()->patients->currentpatient());        // depuis AfficheDossier
        if (idacte > 0)
            AfficheActe(m_listeactes->getById(idacte));
        else
            AfficheActe(m_listeactes->actes()->last());
        if (m_listeactes->actes()->size() > 1)
            OuvrirActesPrecedents();            //! depuis AfficheDossier()
        ui->ActeMotiftextEdit->setFocus();
    }
    //4 - réglage du refracteur
    Datas::I()->refractions->initListebyPatId(Datas::I()->patients->currentpatient()->id());
    if (proc->PortRefracteur()!=Q_NULLPTR)
    {
        gMesureFronto.clear();
        gMesureAutoref.clear();
        RegleRefracteur(Refraction::Fronto);
        RegleRefracteur(Refraction::Autoref);
        proc->SetDataAEnvoyerAuRefracteur(gMesureFronto, gMesureAutoref);
    }

    //5 - mise à jour du dossier en salle d'attente
    PatientEnCours *patcours = Q_NULLPTR;
    patcours = m_listepatientsencours->getById(Datas::I()->patients->currentpatient()->id());
    QTime currenttime = db->ServerDateTime().time();
    if (patcours == Q_NULLPTR)
        m_listepatientsencours->CreationPatient(pat->id(),                                          //! idPat
                                                 m_currentuser->idsuperviseur(),         //! idUser
                                                 ENCOURSEXAMEN + m_currentuser->login(),         //! Statut
                                                 currenttime,                                       //! heureStatut
                                                 QTime(),                                           //! heureRDV
                                                 currenttime,                                       //! heureArrivee
                                                 "",                                                //! Motif
                                                 "",                                                //! Message
                                                 0,                                                 //! idActeAPayer
                                                 QHostInfo::localHostName().left(60),               //! PosteExamen
                                                 m_currentuser->id(),                               //! idUserEnCoursExamen
                                                 0);                                                //! idSalDat
    else
    {
        ItemsList::update(patcours, CP_STATUT_SALDAT, ENCOURSEXAMEN + m_currentuser->login());
        ItemsList::update(patcours, CP_HEURESTATUT_SALDAT, currenttime);
        ItemsList::update(patcours, CP_IDUSERENCOURSEXAM_SALDAT, m_currentuser->id());
        ItemsList::update(patcours, CP_POSTEEXAMEN_SALDAT, QHostInfo::localHostName().left(60));
    }

    ui->AtcdtsPersostextEdit->setFocus();
    RecaleTableView(Datas::I()->patients->currentpatient());
    CalcMotsCles(Datas::I()->patients->currentpatient());
    Flags::I()->MAJFlagSalleDAttente();

    if (m_currentuser->id() > 1) return;
    QString prenom = Datas::I()->patients->currentpatient()->prenom();
    QString Sexe = "";
    QString req =   "select idpat from " TBL_PATIENTS
                    " where patPrenom = '" + prenom + "'"
                    " and (sexe is null or sexe = '')"
                    " and patPrenom <> 'Dominique' and patPrenom <> 'Claude'";
    //qDebug() << req;
    QList<QVariantList> patlist = db->StandardSelectSQL(req, ok);
    if (patlist.size()>0)
    {
        if (UpMessageBox::Question(this, tr("Il existe ") + QString::number(patlist.size()) + " " + prenom + tr(" dont le sexe n'est pas précisé."), tr("Les convertir?")) == UpSmallButton::STARTBUTTON)
        {
            UpMessageBox msgbox;
            msgbox.setText(tr("Convertir ") + QString::number(patlist.size()) + " " + prenom + "...");
            msgbox.setIcon(UpMessageBox::Warning);
            UpSmallButton MBouton;
            MBouton.setText(tr("Masculin"));
            UpSmallButton FBouton;
            FBouton.setText(tr("Féminin"));
            UpSmallButton AnnulBouton;
            AnnulBouton.setText(tr("Annuler"));
            msgbox.addButton(&AnnulBouton, UpSmallButton::CANCELBUTTON);
            msgbox.addButton(&MBouton, UpSmallButton::COPYBUTTON);
            msgbox.addButton(&FBouton, UpSmallButton::STARTBUTTON);
            MBouton.setIcon(Icons::icMan());
            FBouton.setIcon(Icons::icWoman());
            msgbox.exec();
            if (msgbox.clickedButton() == &MBouton)
                Sexe = "M";
            else if (msgbox.clickedButton() == &FBouton)
                Sexe = "F";
            if (Sexe != ""){
                db->StandardSQL("update " TBL_PATIENTS " set sexe = '" + Sexe + "' where PatPrenom = '" + prenom + "' and sexe = ''");
                req ="select idpat from " TBL_PATIENTS " where sexe = ''";
                QList<QVariantList> patlist = db->StandardSelectSQL(req,ok);
                if (ok && patlist.size()>0)
                UpMessageBox::Information(this, tr("Il reste ") + QString::number(patlist.size()) + tr(" dossiers pour lesquels le sexe n'est pas précisé"),"");
                Datas::I()->patients->currentpatient()->setSexe(Sexe);
                ui->IdentPatienttextEdit->setHtml(CalcHtmlIdentificationPatient(Datas::I()->patients->currentpatient()));
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
    //! qDebug() << "AutorDepartConsult() " << Datas::I()->patients->currentpatient()->nom()  << Datas::I()->patients->currentpatient()->prenom() << Datas::I()->patients->currentpatient()->id();
    if (ui->tabWidget->indexOf(ui->tabDossier) < 0)
        return true;
    ui->tabWidget->setCurrentWidget(ui->tabDossier);
    if (!ui->Acteframe->isVisible())
        return FermeDossier(Datas::I()->patients->currentpatient());
    if (focusWidget() != Q_NULLPTR)
        focusWidget()->clearFocus();      //!> Valide les changements dans les champs du dossier en cours d'affichage

    /*! 1. On vérifie si on peut quitter la consultation sans quitter le dossier (il n'est pas obligatoire d'avoir la ligne correspondante dans typepaiementactes */
    if (ui->ActeCotationcomboBox->currentText() == "")
        Titre = tr("Il manque la cotation!");
    else if (ui->ActeMontantlineEdit->text() == "")
        Titre = tr("Il manque le montant!");
    if (Titre != "")
        AutorDepart = false;

    /*! 2. On ne cherche pas à quitter le dossier mais seulement à se déplacer dans les consultations du dossier */
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
        /*! 3 On veut quitter le dossier;
        * On vérifie si on peut quitter le dossier et la cohérence avec TypePaimentsActes */
        if (AutorDepart)
        {
            //! on recherche si le dernier acte du dossier est enregistré dans typepaiements - si le montant de l'acte est 0, on propose de l'enregistrer comme gratuit

            QString requete =   "SELECT max(act.idActe), ActeDate, ActeCotation, ActeMontant FROM " TBL_ACTES
                    " act WHERE idPat = " + QString::number(Datas::I()->patients->currentpatient()->id()) +
                    " AND act.idActe NOT IN (SELECT typ.idActe FROM " TBL_TYPEPAIEMENTACTES " typ)";

            QVariantList actdata = db->getFirstRecordFromStandardSelectSQL(requete,ok,tr("Impossible de retrouver  le dernier acte du patient pour le contrôler!"));
            // cette requête renvoie toujours une table non vide en QT même si elle est vide en mysql... d'où la suite
            if (actdata.size()>0 && actdata.at(0).toInt() > 0) // =il n'y a pas de paiement enregistré pour le dernier acte
            {
                if (actdata.at(0).toInt() != m_currentact->id())
                    AfficheActe(m_listeactes->getById(actdata.at(0).toInt()));
                if (QLocale().toDouble(ui->ActeMontantlineEdit->text()) == 0.0 && ui->ActeCotationcomboBox->currentText() != "")   // il s'agit d'un acte gratuit - on propose de le classer
                {
                    msgbox.setText(tr("Vous avez entré un montant nul !"));
                    msgbox.setInformativeText(tr("Enregistrer cet acte comme gratuit?"));
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
                        requete = "INSERT INTO " TBL_TYPEPAIEMENTACTES " (idActe, TypePaiement) VALUES (" + QString::number(m_currentact->id()) + ",'G')";
                        if (db->StandardSQL(requete))
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
            return FermeDossier(Datas::I()->patients->currentpatient());
        else
        {
            bool a = (Titre == ""? true : RetourSalleDattente(Titre));
            if (!a)
            {
                if (Titre == tr("Il manque le montant!"))
                    ui->ActeMontantlineEdit->setFocus();
                else
                    ui->ActeCotationcomboBox->setFocus();
            }
            return a;
        }
    }
}


/*-----------------------------------------------------------------------------------------------------------------
-- Sortie de l'application  ---------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void Rufus::SortieAppli()
{
    if (!proc->gdbOK)
        exit(0);
    QList<dlg_paiementtiers *> PaimtList = findChildren<dlg_paiementtiers*>();
    if (PaimtList.size()>0)
        for (int i=0; i<PaimtList.size();i++)
            if (PaimtList.at(i)->isVisible())
            {
                QSound::play(NOM_ALARME);
                PaimtList.at(i)->raise();
                return;
            }

    // si le tab dossier est ouvert, on vérifie le droit de fermer le dossier en cours
    if (ui->tabWidget->indexOf(ui->tabDossier) != -1)
    {
        ui->tabWidget->setCurrentWidget(ui->tabDossier);
        if (AutorDepartConsult(true))
        {
            FermeDlgActesPrecedentsEtDocsExternes();
            ui->tabWidget->removeTab(ui->tabWidget->indexOf(ui->tabDossier));
            Flags::I()->MAJFlagSalleDAttente();
        }
        else
            return;;
    }

    // le tab dossier est fermé, on vérifie s'il y a du monde en salle d'attente
    QString req = "SELECT Statut, IdPat, PosteExamen FROM " TBL_SALLEDATTENTE " WHERE IdUser = '" + QString::number(m_currentuser->id()) + "'";
    QList<QVariantList> saldatlist = db->StandardSelectSQL(req,ok);
    if (ok && saldatlist.size()>0)
    {
        /* 2 possibilités
     * 1. C'est le seul poste connecté pour cet utilisateur
     * 2. cet utilisateur est connecté sur d'autres postes, on peut partir
    */
        bool IlResteDesPostesConnectesAvecCeUser = false;
        foreach (PosteConnecte *post, Datas::I()->postesconnectes->postesconnectes()->values())
        {
            if (post->nomposte() != Datas::I()->postesconnectes->currentpost()->nomposte() && post->id() == Datas::I()->postesconnectes->currentpost()->id())
            {
                IlResteDesPostesConnectesAvecCeUser = true;
                break;
            }
        }
        if (IlResteDesPostesConnectesAvecCeUser)
            for (int i = 0; i < saldatlist.size() ; i++)  // il reste des patients pour cet utilisateur dans le centre
            {
                QString Statut = saldatlist.at(i).at(0).toString();
                QString blabla = ENATTENTENOUVELEXAMEN;
                if (Statut == ENCOURS
                        || Statut == ARRIVE
                        || Statut.contains(blabla)
                        || Statut == ENCOURSEXAMEN + m_currentuser->login()
                        || Statut == RETOURACCUEIL)
                {
                    // il y a du monde en salle d'attente, on refuse la fermeture
                    QSound::play(NOM_ALARME);
                    UpMessageBox msgbox;
                    UpSmallButton OKBouton("OK");
                    UpSmallButton NoBouton(tr("Fermer quand même"));
                    msgbox.setText("Euuhh... " + m_currentuser->login() + ", " + tr("vous ne pouvez pas fermer l'application."));
                    msgbox.setInformativeText(tr("Vous avez encore des patients en salle d'attente dont la consultation n'est pas terminée."));
                    msgbox.setIcon(UpMessageBox::Warning);
                    msgbox.addButton(&NoBouton, UpSmallButton::CLOSEBUTTON);
                    msgbox.addButton(&OKBouton, UpSmallButton::OUPSBUTTON);
                    msgbox.exec();
                    if (msgbox.clickedButton() != &NoBouton)
                    {
                        ui->tabWidget->setCurrentIndex(ui->tabWidget->indexOf(ui->tabList));
                        return;
                    }
                    else i = saldatlist.size();
                }
            }
    }

    // on retire cet utilisateur de la table des utilisateurs connectés
    PosteConnecte *post = Datas::I()->postesconnectes->currentpost();
    if (post != Q_NULLPTR)
        Datas::I()->postesconnectes->SupprimePosteConnecte(post);
    Flags::I()->MAJFlagSalleDAttente();
    if ( proc->PosteImportDocs().remove(" - prioritaire")== Utils::getIpAdress())
        proc->setPosteImportDocs(false);

    req = "update " TBL_UTILISATEURS " set datederniereconnexion = '" + QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss")
            + "' where idUser = " + QString::number(m_currentuser->id());
    db->StandardSQL(req);
    if (UtiliseTCP && TcPConnect->state() == QAbstractSocket::ConnectedState)
    {
        TcPConnect->close();
        delete TcPConnect;
    }
    exit(0);
}

/*-----------------------------------------------------------------------------------------------------------------
-- Chargement des données de l'utilisateur --------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
QString Rufus::CalcToolTipCorrespondant(int idcor)
{
    QString tooltp = "";
    Correspondant *cor = Datas::I()->correspondants->getById(idcor, Item::LoadDetails);
    if (cor->idspecialite() != 0)
        tooltp = cor->metier();
    else if (cor->metier() != "")
        tooltp = cor->metier();
    if (cor->adresse1() != "")
    {
        if (tooltp != "") tooltp += "\n";
        tooltp += cor->adresse1();
    }
    if (cor->adresse2() != "")
    {
        if (tooltp != "") tooltp += "\n";
        tooltp += cor->adresse2();
    }
    if (cor->adresse3() != "")
    {
        if (tooltp != "") tooltp += "\n";
        tooltp += cor->adresse3();
    }
    if (cor->ville() != "")
    {
        if (tooltp != "") tooltp += "\n";
        tooltp += cor->ville();
    }
    if (cor->telephone() != "")
    {
        if (tooltp != "") tooltp += "\n";
        tooltp += cor->telephone();
    }
    return tooltp;
}

/*-----------------------------------------------------------------------------------------------------------------
-- Redessine le cadre d'identification du patient en html  -------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
QString Rufus::CalcHtmlIdentificationPatient(Patient *pat)
{
    if (pat == Q_NULLPTR)
        return QString();
    QString html, img, Age;
    QMap<QString,QVariant>  AgeTotal;
    AgeTotal        = Utils::CalculAge(pat->datedenaissance(), pat->sexe());
    img             = AgeTotal["icone"].toString(); //TODO : User icone
    Age             = AgeTotal["toString"].toString();

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
        html += "<img class=\"image\" src=\"://" + img + ".png\" WIDTH=\"100\" HEIGHT=\"100\" BORDER=\"10\" />";            //Icone
    html += "<p class=\"p10\"><b>" + pat->nom() + " " + pat->prenom() + "</b></p>";                                       //Nom Prenom
    html += "<p class=\"p1\"><b>" + Age + "</b> (" + pat->datedenaissance().toString(tr("d MMM yyyy")) + ")</p>";                      //DDN
    if (pat->adresse1() != "")
        html += "<p class=\"p2\">" + pat->adresse1() + "</p>";                                                  //Adresse1
    if (pat->adresse2() != "")
        html += "<p class=\"p2\">" + pat->adresse2() + "</p>";                                                  //Adresse2
    if (pat->adresse3() != "")
        html += "<p class=\"p2\">" + pat->adresse3() + "</p>";                                                  //Adresse3
    if (pat->codepostal() != "")
    {
        html += "<p class=\"p2\">" + pat->codepostal() +
                " " + pat->ville() + "</p>";                                                                    //CP + ville
    }
    else
        if (pat->ville() != "")
            html += "<p class=\"p2\">" + pat->ville() + "</p>";                                                 //Ville
    if (pat->telephone() != "")
        html += "<p class=\"p3\">" + tr("Tél.") + "\t" + pat->telephone() + "</p>";                             //Tél
    if (pat->portable() != "")
        html += "<p class=\"p2\">" + tr("Portable") + "\t" + pat->portable() + "</p>";                          //Portable
    if (pat->mail() != "")
        html += "<p class=\"p3\">" + tr("Mail") + "\t" + pat->mail() + "</p>";                                  //Mail
    if (pat->NNI() > 0)
        html += "<p class=\"p2\">" + tr("NNI") + "\t" + QString::number(pat->NNI()) + "</p>";                   //NNI
    if (pat->profession() != "")
        html += "<p class=\"p3\">" + pat->profession() + "</p>";                                                //Profession
    if (pat->isald() || pat->iscmu())
    {
        html += "<p class=\"p3\"><td width=\"60\">";
        if (pat->isald())
            html += "<font size = \"5\"><font color = \"red\"><b>ALD</b></font>";                                           //ALD
        if (pat->iscmu())
            html += "</td><td width=\"60\"><font size = \"5\"><font color = \"blue\"><b>CMU</b><\font>";                    //CMU
        html += "</td></p>";
    }

    html += "</body></html>";
    return html;
}

QIcon Rufus::CalcIconPatient(Patient *pat)
{
    if (pat == Q_NULLPTR)
        return QIcon();
    QString img;
    QMap<QString,QVariant>  AgeTotal;
    AgeTotal        = Utils::CalculAge(pat->datedenaissance(), pat->sexe());
    img             = AgeTotal["icone"].toString(); //TODO : User icone
    return Icons::getIconAge(img);
}

/*-----------------------------------------------------------------------------------------------------------------
-- Calcule la liste des mots clés du patient  -------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void    Rufus::CalcMotsCles(Patient *pat)
{
    if (pat == Q_NULLPTR)
        return;
    QString req = "select motcle from " TBL_MOTSCLES " where idmotcle in (select idmotcle from " TBL_MOTSCLESJOINTURES " where idpat = " + QString::number(pat->id()) + ")";
    QList<QVariantList> mtcllist = db->StandardSelectSQL(req,ok);
    QString result ("<font color=\"green\">Mots clés: </font>");
    if (ok && mtcllist.size()>0)
    {
        for (int i=0; i<mtcllist.size(); i++)
        {
            result += mtcllist.at(i).at(0).toString();
            if (i<mtcllist.size()-1)
                result += ", ";
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
    qint64 a = m_listepatientsmodel->rowCount();
    switch (a) {
    case 0:
        ui->label_15->setText(tr("aucun dossier pour ces critères"));
        break;
    case 1:
        ui->label_15->setText("1 dossier");
        break;
    default:
        if (a == 1000)
             a = db->countPatientsAll(ui->CreerNomlineEdit->text(), ui->CreerPrenomlineEdit->text());
        ui->label_15->setText(QString::number(a) + " " + tr("dossiers"));
        break;
    }
}


/*-----------------------------------------------------------------------------------------------------------------
-- Rechercher le nom dans les TreeWidget  en restreignant la liste ------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void Rufus::FiltreTable(QString nom, QString prenom)
{
    m_patients->initListeTable(nom, prenom, true);
                //! mettre en place un filtre directement sur la liste est moins rapide que de réinterroger la BDD directement en SQL
    Remplir_ListePatients_TableView() ;   //FiltreTable()
    if (m_listepatientsmodel->rowCount()>0)
        RecaleTableView(getPatientFromRow(0), QAbstractItemView::PositionAtCenter);
    EnableButtons();
}

/*-----------------------------------------------------------------------------------------------------------------
-- Vérifie les verrous d'un dossier avant de l'afficher -----------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void    Rufus::ChoixDossier(Patient *pat, int idacte)  // appelée depuis la tablist ou la salle d'attente - vérifie qu'un dossier n'est pas verrouillé avant de l'afficher
{
    if (pat == Q_NULLPTR)
        return;
    if (m_currentuser->isSecretaire())    // si l'utilisateur est une secrétaire, on propose de mettre le patient en salle d'attente
        InscritEnSalDat(pat);
    else if (m_currentuser->isSoignant())
    {
        if (ui->tabWidget->indexOf(ui->tabDossier) > 0)
        {
            if (Datas::I()->patients->currentpatient() == pat)
            {
                ui->tabWidget->setCurrentWidget(ui->tabDossier);
                return;
            }
            else
            {
                //qDebug() << "ChoixDossier() " << Datas::I()->patients->currentpatient()->nom()  << Datas::I()->patients->currentpatient()->prenom() << Datas::I()->patients->currentpatient()->id();
                if (!AutorDepartConsult(true))
                    return;
            }
        }
        else
        {
            // On vérifie si le dossier n'est pas verrouillé par un autre utilisateur
            QString blabla = ENCOURSEXAMEN;
            int length = blabla.size();
            m_listepatientsencours->initListeAll(); //TODO si on utilise le TCP, on peut se passer de ça parce qu'on peut mettre en place un message tcp pour chaque modif de la salle d'attente
            foreach (PatientEnCours *patcrs, m_listepatientsencours->patientsencours()->values())
            {
                if (patcrs->id() == pat->id() && patcrs->statut().left(length) == ENCOURSEXAMEN
                        && (patcrs->iduserencoursexam() != m_currentuser->id() || (patcrs->iduserencoursexam() == m_currentuser->id() && patcrs->posteexamen() != QHostInfo::localHostName().left(60))))
                {
                    UpMessageBox::Watch(this,tr("Impossible d'ouvrir ce dossier!"),
                                        tr("Ce patient est") + patcrs->statut() + "\n" + tr("sur ") + patcrs->posteexamen());
                    return;
                }
            }
        }
        Datas::I()->patients->setcurrentpatient(pat->id());
        AfficheDossier(Datas::I()->patients->currentpatient(), idacte);
    }
}

/*-----------------------------------------------------------------------------------------------------------------
-- Créer un acte ------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void Rufus::CreerActe(Patient *pat)
{
    if (ui->Acteframe->isVisible())
        if(!AutorDepartConsult(false)) return;
    Acte * acte = m_listeactes->CreationActe(pat, proc->idCentre());
    m_currentact = acte;

    AfficheActe(m_currentact);
    if (m_listeactes->actes()->size() > 1)
    {
        QList<dlg_actesprecedents *> listactesprecs = findChildren<dlg_actesprecedents *>();
        for (int i = 0; i<listactesprecs.size();i++)
            if (listactesprecs.at(i)->getidPatient() != pat->id())
                listactesprecs.at(i)->close();
        if (findChildren<dlg_actesprecedents *>().size() == 0)
            OuvrirActesPrecedents();            //! depuis CreerActe()
        else
            findChildren<dlg_actesprecedents *>().at(0)->Actualise();
    }
    else
    {
        QList<dlg_actesprecedents *> listactesprecs = findChildren<dlg_actesprecedents *>();
        for (int i = 0; i<listactesprecs.size();i++)
            listactesprecs.at(i)->close();
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
        ModeSelectDepuisListe();
}

/*-----------------------------------------------------------------------------------------------------------------
-- Créer un dossier -----------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void Rufus::CreerDossier()
{
    if (ui->tabWidget->indexOf(ui->tabDossier) > 0)
        if (!AutorDepartConsult(true))
            return;
    QString PatNom, PatPrenom, PatDDN, PatCreePar, PatCreeLe;
    int idPat; // on n'utilise pas Datas::I()->patients->currentpatient()->id() qui ne sera initialisé qu'après que le dossier ait été réellement affiché.

    PatNom      = Utils::trimcapitilize(ui->CreerNomlineEdit->text(),true);
    PatPrenom   = Utils::trimcapitilize(ui->CreerPrenomlineEdit->text(),true);
    PatDDN      = ui->CreerDDNdateEdit->date().toString("yyyy-MM-dd");
    PatCreeLe   = QDateTime::currentDateTime().date().toString("yyyy-MM-dd");
    PatCreePar  = QString::number(m_currentuser->id());
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
        msgbox.setText("Euuhh... " + m_currentuser->login());
        msgbox.setInformativeText(tr("Confirmez vous la date de naissance?") + "\n" + ui->CreerDDNdateEdit->date().toString(tr("d-MMM-yyyy")));
        msgbox.setIcon(UpMessageBox::Warning);
        msgbox.addButton(&NoBouton, UpSmallButton::CANCELBUTTON);
        msgbox.addButton(&OKBouton, UpSmallButton::STARTBUTTON);
        msgbox.exec();
        if (msgbox.clickedButton() != &OKBouton)
            return;
    }

    // 1. On recherche d'abord si le dossier existe
    idPat = RecherchePatient(PatNom, PatPrenom, PatDDN, "Impossible de rechercher le dossier");
    if (idPat == -1)                                                // il y a eu une erreur pendant la recherche
        return ;
    if (idPat > 0)                                                  // Le dossier existe, on l'affiche
    {
        UpMessageBox::Watch(this, tr("Ce patient est déjà venu!"));
        if( m_currentuser->isSoignant() )
        {
            Datas::I()->patients->setcurrentpatient(idPat);
            AfficheDossier(Datas::I()->patients->currentpatient());
        }
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
                InscritEnSalDat(m_patients->getById(idPat));
        }
        return;
    }

    /*----------------------------------------------------------------------------------------------------------------
       // il faudrait faire une recherche de dossier avec orthographe similaire
    -----------------------------------------------------------------------------------------------------------------*/

    // 2. On recherche ensuite un dossier similaire Nom + Prenom
    if (ui->PatientsListeTableView->isVisible()  && m_listepatientsmodel->rowCount()>0)
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
        QHash<QString, QVariant> listbinds;
        listbinds[CP_NOM_PATIENTS]          = PatNom;
        listbinds[CP_PRENOM_PATIENTS]       = PatPrenom;
        listbinds[CP_DDN_PATIENTS]          = ui->CreerDDNdateEdit->date().toString("yyyy-MM-dd");
        Patient *pat  = Patients::CreationPatient(listbinds);
        if (pat == Q_NULLPTR)
            return;

        if (!IdentificationPatient(dlg_identificationpatient::Creation, pat))
        {
            m_listepatientsencours->SupprimePatientEnCours(m_listepatientsencours->getById(pat->id()));
            m_patients->SupprimePatient(pat);
            m_patients->initListeTable(ui->CreerNomlineEdit->text(), ui->CreerPrenomlineEdit->text());
            return;
        }
        FiltreTable(pat->nom(), pat->prenom());

        // Si le User est un soignant, on crée d'emblée une consultation et on l'affiche
        if( m_currentuser->isSoignant() )
        {
            UpMessageBox msgbox;
            msgbox.setText(tr("Dossier ") + pat->prenom() + " " + pat->nom() + tr(" créé"));
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
                if (!InscritEnSalDat(pat))
                    RecaleTableView(pat);
            }
            else if (msgbox.clickedButton() == &AnnulBouton)
            {
                Datas::I()->patients->setcurrentpatient(pat->id());
                CreerActe(Datas::I()->patients->currentpatient());
                AfficheDossier(Datas::I()->patients->currentpatient());
            }
            else
                RecaleTableView(pat);
        }
        else
            if (!InscritEnSalDat(pat))
                RecaleTableView(pat);
        pat = Q_NULLPTR;
        delete pat;
    }
}


// ------------------------------------------------------------------------------------------
// Creer le menu général de l'appliaction
// ------------------------------------------------------------------------------------------

void Rufus::CreerMenu()
{
//1. DEFINITION DES ACTIONS DE MENU -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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
    actionExportActe                = new QAction(tr("Exporter l'acte en cours"));
    actionEnregistrerDocScanner     = new QAction(tr("Enregistrer un document scanné"));
    actionEnregistrerVideo          = new QAction(tr("Enregistrer une video"));
    actionRechercheCourrier         = new QAction(tr("Afficher les courriers à faire"));
    actionCorrespondants            = new QAction(tr("Liste des correspondants"));

    actionPaiementDirect            = new QAction(tr("Gestion des paiements directs"));
    actionPaiementTiers             = new QAction(tr("Gestion des tiers payants"));
    actionBilanRecettes             = new QAction(tr("Bilan des recettes"));
    actionRecettesSpeciales         = new QAction(tr("Enregistrement des recettes spéciales"));
    actionJournalDepenses           = new QAction(tr("Journal des dépenses"));
    actionGestionComptesBancaires   = new QAction(tr("Gestion des comptes bancaires"));
    actionRemiseCheques             = new QAction(tr("Effectuer une remise de chèques"));

    QAction *Apropos                = new QAction(tr("A propos"));
    actionQuit                      = new QAction(tr("Quitter"));
    actionQuit                      ->setMenuRole(QAction::PreferencesRole);
    // Les connect des actions --------------------------------------------------------------------------------------------------
    connect (actionQuit,                        &QAction::triggered,        this,                   &Rufus::close);
    connect (actionCreerDossier,                &QAction::triggered,        this,                   &Rufus::ModeCreationDossier);
    connect (actionOuvrirDossier,               &QAction::triggered,        this,                   &Rufus::ModeSelectDepuisListe);
    connect (actionSupprimerDossier,            &QAction::triggered,        this,                   [=] {
                                                                                                            Patient *pat = Q_NULLPTR;;
                                                                                                            if (ui->tabWidget->currentWidget() == ui->tabList)
                                                                                                            {
                                                                                                                if (ui->PatientsListeTableView->selectionModel()->selectedIndexes().size() > 0)
                                                                                                                    pat = getPatientFromSelectionInTable();
                                                                                                            }
                                                                                                            else pat = Datas::I()->patients->currentpatient();
                                                                                                            if (pat != Q_NULLPTR)
                                                                                                                SupprimerDossier(pat);
                                                                                                        });
    connect (actionRechercheParMotCle,          &QAction::triggered,        this,                   &Rufus::RechercheParMotCle);
    connect (actionRechercheParID,              &QAction::triggered,        this,                   &Rufus::RechercheParID);
    connect (actionCreerActe,                   &QAction::triggered,        this,                   [=] {CreerActe(Datas::I()->patients->currentpatient());});

    connect (actionParametres,                  &QAction::triggered,        this,                   &Rufus::OuvrirParametres);
    connect (actionResumeStatut,                &QAction::triggered,        this,                   [=] {
                                                                                                            if (gResumeStatut =="")
                                                                                                                ResumeStatut();
                                                                                                            proc->Edit(gResumeStatut, tr("Information statut"), false, true );
                                                                                                        });
    connect (actionSupprimerActe,               &QAction::triggered,        this,                   [=] {SupprimerActe(m_currentact);});
    // Documents
    connect (actionEmettreDocument,             &QAction::triggered,        this,                   &Rufus::OuvrirDocuments);
    connect (actionDossierPatient,              &QAction::triggered,        this,                   [=] {ImprimeDossier(Datas::I()->patients->currentpatient());});
    connect (actionCorrespondants,              &QAction::triggered,        this,                   &Rufus::ListeCorrespondants);
    connect (actionEnregistrerDocScanner,       &QAction::triggered,        this,                   [=] {EnregistreDocScanner(Datas::I()->patients->currentpatient());});
    connect (actionEnregistrerVideo,            &QAction::triggered,        this,                   [=] {EnregistreVideo(Datas::I()->patients->currentpatient());});
    connect (actionExportActe,                  &QAction::triggered,        this,                   [=] {ExporteActe(m_currentact);});
    connect (actionRechercheCourrier,           &QAction::triggered,        this,                   &Rufus::AfficheCourriersAFaire);
    // Comptabilité
    connect (actionGestionComptesBancaires,     &QAction::triggered,        this,                   &Rufus::GestionComptes);
    connect (actionPaiementDirect,              &QAction::triggered,        this,                   [=] {AppelPaiementDirect(Menu);});
    connect (actionPaiementTiers,               &QAction::triggered,        this,                   &Rufus::AppelPaiementTiers);
    connect (actionRecettesSpeciales,           &QAction::triggered,        this,                   &Rufus::RecettesSpeciales);
    connect (actionBilanRecettes,               &QAction::triggered,        this,                   &Rufus::BilanRecettes);
    connect (actionJournalDepenses,             &QAction::triggered,        this,                   &Rufus::OuvrirJournalDepenses);
    connect (actionRemiseCheques,               &QAction::triggered,        this,                   &Rufus::RemiseCheques);


// 2. DEFINITION DES MENUS ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    menuDossier         = menuBar()->addMenu(tr("Dossier"));
    menuEdition         = menuBar()->addMenu(tr("Edition"));
    menuActe            = new QMenu(this);
    if (m_currentuser->isSoignant())
        menuActe        = menuBar()->addMenu(tr("Acte"));
    menuDocuments       = menuBar()->addMenu(tr("Documents"));
    menuEmettre         = menuDocuments->addMenu(tr("Emettre"));
    menuComptabilite    = menuBar()->addMenu(tr("Comptabilité"));

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
    menuBar()   ->addAction(Apropos);
    menuBar()   ->addAction(actionQuit);
#endif
#ifdef Q_OS_LINUX
    menuAide    = menuBar()->addMenu(tr("Aide"));
    menuAide    ->addAction(Apropos);
    menuDossier ->addAction(actionQuit);
#endif

    menuEdition->addAction(tr("Copier"));
    menuEdition->addAction(tr("Couper"));
    menuEdition->addAction(tr("Coller"));
    menuEdition->addSeparator();
    menuEdition->addAction(actionParametres);
    menuEdition->addAction(actionResumeStatut);

    menuActe    ->addAction(actionCreerActe);
    menuActe    ->addAction(actionSupprimerActe);

    menuEmettre ->addAction(actionEmettreDocument);
    menuEmettre ->addAction(actionExportActe);
    menuEmettre ->addAction(actionDossierPatient);

    menuComptabilite->addAction(actionPaiementDirect);
    menuComptabilite->addAction(actionPaiementTiers);
    menuComptabilite->addAction(actionBilanRecettes);
    menuComptabilite->addAction(actionRecettesSpeciales);
    menuComptabilite->addSeparator();
    menuComptabilite->addAction(actionJournalDepenses);
    menuComptabilite->addSeparator();
    menuComptabilite->addAction(actionGestionComptesBancaires);
    menuComptabilite->addAction(actionRemiseCheques);

    // Les connect des menus --------------------------------------------------------------------------------------------------
    connect (menuActe,                          &QMenu::aboutToShow,        this,                   [=] {AfficheMenu(menuActe);});
    connect (menuEdition,                       &QMenu::aboutToShow,        this,                   [=] {AfficheMenu(menuEdition);});
    connect (menuDocuments,                     &QMenu::aboutToShow,        this,                   [=] {AfficheMenu(menuDocuments);});
    connect (menuDossier,                       &QMenu::aboutToShow,        this,                   [=] {AfficheMenu(menuDossier);});
    connect (menuComptabilite,                  &QMenu::aboutToShow,        this,                   [=] {AfficheMenu(menuComptabilite);});
}

// ------------------------------------------------------------------------------------------
// Descend une ligne dans la table
// ------------------------------------------------------------------------------------------

void Rufus::DescendUneLigne()
{
    QModelIndexList listindx = ui->PatientsListeTableView->selectionModel()->selectedIndexes();
    if (listindx.size()==0) return;
    int row = listindx.at(0).row();
    if (row < m_listepatientsmodel->rowCount() - 1)
        RecaleTableView(getPatientFromRow(row + 1));
}

// ------------------------------------------------------------------------------------------
// Descend 20 lignes dans la table
// ------------------------------------------------------------------------------------------

void Rufus::Descend20Lignes()
{
    QModelIndexList listindx = ui->PatientsListeTableView->selectionModel()->selectedIndexes();
    if (listindx.size()==0) return;
    int row = listindx.at(0).row();
    if (row < m_listepatientsmodel->rowCount() - 20)
        RecaleTableView(getPatientFromRow(row + 20));
    else
        RecaleTableView(getPatientFromRow(m_listepatientsmodel->rowCount() - 1));
}

// ------------------------------------------------------------------------------------------
// Enregistre un nouveau correpondant
// ------------------------------------------------------------------------------------------
int Rufus::EnregistreNouveauCorresp(QString Cor, QString Nom)
{
    int idcor = -1;
    bool onlydoctors = (Cor == "MG");
    Dlg_IdentCorresp        = new dlg_identificationcorresp(dlg_identificationcorresp::Creation, onlydoctors);
    Dlg_IdentCorresp->ui->NomlineEdit->setText(Nom);
    Dlg_IdentCorresp->ui->PrenomlineEdit->setFocus();
    if (Cor == "MG")
        Dlg_IdentCorresp->ui->MGradioButton->setChecked(true);
    if (Dlg_IdentCorresp->exec()>0)
        idcor = Dlg_IdentCorresp->correspondantrenvoye()->id();
    delete Dlg_IdentCorresp;
    return idcor;
}

/*-----------------------------------------------------------------------------------------------------------------
-- Exporte l'acte en cours en réunissant dans un même dossier le contenu de l'acte en pdf et les documents d'imageir liés à cet acte
-----------------------------------------------------------------------------------------------------------------*/
void Rufus::ExporteActe(Acte *act)
{
    if (act == Q_NULLPTR)
        return;
    Patient *pat = m_patients->getById(act->idPatient());
    if (pat == Q_NULLPTR)
        return;
    QString nomdossier = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).at((0)) + "/" + pat->nom() + " " + pat->prenom() + " - " + act->date().toString("d MMM yyyy");
    ImprimeListActes(QList<Acte*>() << act, false, true, nomdossier);
    QString req = "select idimpression from " TBL_IMPRESSIONS
                  " where idpat = " +QString::number(pat->id()) +
                  " and DATE(dateimpression) = '" + m_currentact->date().toString("yyyy-MM-dd") + "' "
                  " and formatdoc = '" IMAGERIE "'";
    QList<QVariantList> listimages = db->StandardSelectSQL(req, ok);
    if (ok && listimages.size()>0)
    {
        for (int i=0; i<listimages.size(); i++)
        {
            DocExterne *docmt = Datas::I()->docsexternes->getById(listimages.at(i).at(0).toInt());
            QString filedest = pat->nom() + " " + pat->prenom() + " - " + docmt->typedoc() + " " + docmt->soustypedoc() + " " + QString::number(i);
            {
                if (db->getMode() != DataBase::Distant)
                {
                    QString fileorigin = proc->DirImagerie() + DIR_IMAGES + docmt->lienversfichier();
                    QFile origin(fileorigin);
                    origin.copy(nomdossier + "/" + filedest + "." + QFileInfo(origin).suffix());
                }
                else
                {
                    QByteArray ba;
                    QString filesufx;
                    if (docmt->lienversfichier().contains("."))
                    {
                        QStringList lst = docmt->lienversfichier().split(".");
                        filesufx        = lst.at(lst.size()-1);
                    }
                    QString sfx = (filesufx == PDF? PDF : JPG);
                    db->StandardSQL("delete from " TBL_ECHANGEIMAGES
                                    " where idimpression = " + QString::number(docmt->id()) +
                                    " and facture is null");
                    QString req = "INSERT INTO " TBL_ECHANGEIMAGES " (idimpression, " + sfx + ", compression)"
                                  " VALUES (" +
                                    QString::number(docmt->id()) + ", " +
                                    " LOAD_FILE('" + Utils::correctquoteSQL(proc->DirImagerieServeur() + DIR_IMAGES + Utils::correctquoteSQL(docmt->lienversfichier())) + "'), " +
                                    QString::number(docmt->compression()) + ")";
                    db->StandardSQL(req);

                    // On charge ensuite le contenu des champs longblob des tables concernées en mémoire pour les afficher
                    req = "select " + sfx + " from " TBL_ECHANGEIMAGES " where idimpression = " + QString::number(docmt->id()) + " and facture is null";
                    QVariantList impr = db->getFirstRecordFromStandardSelectSQL(req, ok, tr("Impossible d'accéder à la table ") + TBL_ECHANGEIMAGES);
                    if (!ok || impr.size() == 0)
                        return;
                    ba.append(impr.at(0).toByteArray());
                    filedest = nomdossier + "/" + filedest + "." + sfx;
                    if (sfx == JPG)
                    {
                        QImage image;
                        if (!image.loadFromData(ba))
                            return;
                        QPixmap pixmap;
                        pixmap = pixmap.fromImage(image);
                        pixmap.save(filedest, "jpeg");
                    }
                    else if (sfx == PDF)
                    {
                        Poppler::Document* document = Poppler::Document::loadFromData(ba);
                        Poppler::PDFConverter *pdfConv = document->pdfConverter();
                        pdfConv->setOutputFileName(filedest);
                        pdfConv->setPDFOptions(pdfConv->pdfOptions()|Poppler::PDFConverter::WithChanges);
                        pdfConv->convert();
                        delete pdfConv;
                        delete document;
                    }
                }
            }
         }
        int nb = listimages.size();
        QString msg = "";
        if (nb == 1)
            msg = "\n" + tr("Ce dossier contient le contenu de l'acte en cours et un document d'imagerie");
        else if (nb > 1)
            msg = "\n" + tr("Ce dossier contient le contenu de l'acte en cours et ") + QString::number(nb) + tr(" documents d'imagerie");
        UpMessageBox::Watch(this,
                        tr("Export d'acte effectué"),
                        tr("Le dossier ") + pat->nom() + " " + pat->prenom() + " - " + act->date().toString("d MMM yyyy") +
                        tr(" a été créé sur le bureau") + msg );
    }
    MAJDocsExternes();
}

/*-----------------------------------------------------------------------------------------------------------------
-- Fermeture de la fiche dlg_actesprecedents ----------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void Rufus::FermeDlgActesPrecedentsEtDocsExternes()
{
    QList<dlg_actesprecedents *> ListDialog = this->findChildren<dlg_actesprecedents *>();
    for (int n = 0; n <  ListDialog.size(); n++)
        ListDialog.at(n)->close();
    QList<dlg_docsexternes *> ListDialogDocs = this->findChildren<dlg_docsexternes *>();
    for (int n = 0; n <  ListDialogDocs.size(); n++)
        ListDialogDocs.at(n)->close();
    if (Datas::I()->patients->currentpatient()->id() > 0)
        ui->OuvreDocsExternespushButton->setEnabled(Datas::I()->docsexternes->docsexternes()->size()>0);
}

/*-----------------------------------------------------------------------------------------------------------------
-- Fermeture du dossier en cours ----------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
bool Rufus::FermeDossier(Patient *patient)
{
    if (patient == Q_NULLPTR)
        return false;
                                                    //! qDebug() << "FermeDossier() " << pat->nom()  << pat->prenom() << pat->id();
    bool a = true;
    UpMessageBox msgbox;
    msgbox.setInformativeText(tr("Garder le dossier en salle d'attente?"));
    msgbox.setText(tr("Fermeture du dossier de ") + patient->nom() + " " + patient->prenom());
    msgbox.setIcon(UpMessageBox::Quest);

    UpSmallButton SalDatBouton     (tr("Garder le dossier\nen salle d'attente"));
    UpSmallButton AnnulBouton  (tr("Annuler"));
    UpSmallButton CloseBouton     (tr("Fermer\nle dossier"));

    msgbox.addButton(&AnnulBouton,  UpSmallButton::CANCELBUTTON);
    msgbox.addButton(&SalDatBouton, UpSmallButton::STARTBUTTON);
    msgbox.addButton(&CloseBouton,  UpSmallButton::CLOSEBUTTON);
    msgbox.exec();
    if (msgbox.clickedButton() == &CloseBouton)                                                        // Fermer le dossier
        m_listepatientsencours->SupprimePatientEnCours(m_listepatientsencours->getById(patient->id()));
    else if (msgbox.clickedButton() == &SalDatBouton)                                                   // Garder le dossier en salle d'attente
    {
        QString Message(""), Motif(""), idUser ("");
        QStringList llist;
        PatientEnCours *pat = m_listepatientsencours->getById(patient->id());
        if (pat != Q_NULLPTR)
        {
            Motif = pat->motif();
            Message = pat->message();
            if (Motif=="")
            {
                llist = MotifRDV(Motif, Message);
                if (llist.isEmpty())
                    return false;
                Motif   = llist.at(0);
                Message = llist.at(1);
                idUser  = llist.at(3);
            }
            ItemsList::update(pat, CP_STATUT_SALDAT, ARRIVE);
            ItemsList::update(pat, CP_IDUSERENCOURSEXAM_SALDAT);
            ItemsList::update(pat, CP_POSTEEXAMEN_SALDAT);
            ItemsList::update(pat, CP_MOTIF_SALDAT, Motif);
            ItemsList::update(pat, CP_MESSAGE_SALDAT, Message);
        }
        else
            a = InscritEnSalDat(patient);
    }
    else a = false;                                                                                 // Annuler et revenir au dossier
    if (a) {
        Datas::I()->patients->currentpatient()->resetdatas();
        m_currentact = Q_NULLPTR;
    }
    Flags::I()->MAJFlagSalleDAttente();
    return a;
}

// ------------------------------------------------------------------------------------------
// Modifier nom et prénom directement dans la fiche
// ------------------------------------------------------------------------------------------

bool Rufus::IdentificationPatient(dlg_identificationpatient::Mode mode, Patient *pat)
{
    // TODO mettre en place un signal de mise à jour de la liste des patients
    bool unpatientaetecreeoumodifie = false;
    Dlg_IdentPatient           = new dlg_identificationpatient(mode, pat, this);

    if (Dlg_IdentPatient->exec() > 0)
    {
        if (mode == dlg_identificationpatient::Modification)
        {
            //  Mise à jour de Datas::I()->patients->currentpatient() et de l'affichage si le dossier modifié est le dossier en cours
            Patients::updatePatient(pat);
            if (m_listepatientsencours->getById(pat->id()) != Q_NULLPTR)
                Remplir_SalDat();
            if (m_listepatientsmodel->findItems(QString::number(pat->id())).size() > 0)
            {
                int row = m_listepatientsmodel->findItems(QString::number(pat->id())).at(0)->row();
                UpStandardItem *item = dynamic_cast<UpStandardItem*>(m_listepatientsmodel->item(row,1));
                item->setText(pat->nom().toUpper() + " " + pat->prenom());
                item = dynamic_cast<UpStandardItem*>(m_listepatientsmodel->item(row,2));
                item->setText(pat->datedenaissance().toString(tr("dd-MM-yyyy")));
                item = dynamic_cast<UpStandardItem*>(m_listepatientsmodel->item(row,3));
                item->setText(pat->datedenaissance().toString(tr("yyyyMMdd")));
                item = dynamic_cast<UpStandardItem*>(m_listepatientsmodel->item(row,4));
                item->setText(pat->nom());
                item = dynamic_cast<UpStandardItem*>(m_listepatientsmodel->item(row,5));
                item->setText(pat->prenom());
            }
            if (pat == Datas::I()->patients->currentpatient())
            {
                ui->IdentPatienttextEdit->setHtml(CalcHtmlIdentificationPatient(Datas::I()->patients->currentpatient()));
                ui->tabWidget->setTabIcon(ui->tabWidget->indexOf(ui->tabDossier),CalcIconPatient(Datas::I()->patients->currentpatient()));
                ui->tabWidget->setTabText(ui->tabWidget->indexOf(ui->tabDossier) ,pat->nom() + " " + pat->prenom());
                if (ui->MGupComboBox->currentData().toInt() != Datas::I()->patients->currentpatient()->idmg())
                {
                    int e = ui->MGupComboBox->findData(Datas::I()->patients->currentpatient()->idmg());
                    ui->MGupComboBox->setCurrentIndex(e);
                    OKModifierTerrain(Datas::I()->patients->currentpatient());
                }
                QMap<QString,QVariant>  NewAge = Utils::CalculAge(Datas::I()->patients->currentpatient()->datedenaissance(), ui->ActeDatedateEdit->date());
                ui->AgelineEdit->setText(NewAge["toString"].toString());
            }
            Flags::I()->MAJFlagSalleDAttente();
            unpatientaetecreeoumodifie = true;
        }

        else if (mode == dlg_identificationpatient::Creation)
        {
            unpatientaetecreeoumodifie = true;
        }

        else if (mode == dlg_identificationpatient::Copie)
        {
            pat = Dlg_IdentPatient->getPatient();
            FiltreTable(pat->nom(), pat->prenom());
            // Si le User est un soignant, on propose d'afficher le dossier et si oui, n crée une consutation d'emblée
            if( m_currentuser->isSoignant() )
            {
                UpMessageBox msgbox;
                msgbox.setText(tr("Dossier ") + pat->nom() + " " + pat->prenom() + " créé");
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
                    if (!InscritEnSalDat(pat))
                        RecaleTableView(pat);
                }
                else if (msgbox.clickedButton() == &AnnulBouton)
                {
                    Datas::I()->patients->setcurrentpatient(pat->id());
                    CreerActe(Datas::I()->patients->currentpatient());
                    AfficheDossier(Datas::I()->patients->currentpatient());
                }
                else
                    RecaleTableView(pat);
             }
            else
                InscritEnSalDat(pat);
            unpatientaetecreeoumodifie = true;
        }
    }
    else  // si la fiche est rejetée
    {
        if (mode == dlg_identificationpatient::Copie && Dlg_IdentPatient->getPatient() == Q_NULLPTR)  // il n'y a pas eu de copie parce que le dossssier existait déjà
        {
            QString Sexe ("");
            if (Dlg_IdentPatient->ui->MradioButton->isChecked()) Sexe = "M";
            if (Dlg_IdentPatient->ui->FradioButton->isChecked()) Sexe = "F";
            ui->CreerNomlineEdit->setText(Dlg_IdentPatient->ui->NomlineEdit->text());
            ui->CreerPrenomlineEdit->setText(Dlg_IdentPatient->ui->PrenomlineEdit->text());
            ui->CreerDDNdateEdit->setDate(Dlg_IdentPatient->ui->DDNdateEdit->date());
            ui->tabWidget->setCurrentWidget(ui->tabList);
            FiltreTable(ui->CreerNomlineEdit->text(), ui->CreerPrenomlineEdit->text());
        }
    }
    if (unpatientaetecreeoumodifie)
        envoieMessage(QString::number(Dlg_IdentPatient->getPatient()->id()) + TCPMSG_MAJPatient);
    delete Dlg_IdentPatient;
    return unpatientaetecreeoumodifie;
}

/*-----------------------------------------------------------------------------------------------------------------
-- Ouvrir la fiche documents ------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
bool   Rufus::Imprimer_Document(User * user, QString titre, QString Entete, QString text, QDate date, QString nom, QString prenom,
                                 bool Prescription, bool ALD, bool AvecPrevisu, bool AvecDupli, bool AvecChoixImprimante, bool Administratif)
{
    QString     Corps, Pied;;
    QTextEdit   *Etat_textEdit = new QTextEdit;
    bool        AvecNumPage = false;
    bool        aa;

    Entete.replace("{{PRENOM PATIENT}}", (Prescription? prenom        : ""));
    Entete.replace("{{NOM PATIENT}}"   , (Prescription? nom.toUpper() : ""));

    //création du pied
    Pied = proc->ImpressionPied(user, false, ALD);
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
        Utils::nettoieHTML(Corps);

        int idpat = 0;
        if (ui->tabWidget->currentWidget() == ui->tabDossier)
            idpat = Datas::I()->patients->currentpatient()->id();
        else
            idpat = Datas::I()->patients->dossierpatientaouvrir()->id();

        QHash<QString, QVariant> listbinds;
        // on doit passer par les bindvalue pour incorporer le bytearray dans la requête
        listbinds[CP_IDUSER_IMPRESSIONS]        = user->id();
        listbinds[CP_IDPAT_IMPRESSIONS]         = idpat;
        listbinds[CP_TYPEDOC_IMPRESSIONS]       = (Prescription? "Prescription" : "Courrier");
        listbinds[CP_SOUSTYPEDOC_IMPRESSIONS]   = titre;
        listbinds[CP_TITRE_IMPRESSIONS]         = titre;
        listbinds[CP_TEXTENTETE_IMPRESSIONS]    = Entete;
        listbinds[CP_TEXTCORPS_IMPRESSIONS]     = Corps;
        listbinds[CP_TEXTORIGINE_IMPRESSIONS]   = text;
        listbinds[CP_TEXTPIED_IMPRESSIONS]      = Pied;
        listbinds[CP_DATE_IMPRESSIONS]          = date.toString("yyyy-MM-dd") + " " + QTime::currentTime().toString("HH:mm:ss");
        listbinds[CP_IDEMETTEUR_IMPRESSIONS]    = m_currentuser->id();
        listbinds[CP_ALD_IMPRESSIONS]           = (ALD? "1": QVariant(QVariant::String));
        listbinds[CP_EMISORRECU_IMPRESSIONS]    = "0";
        listbinds[CP_FORMATDOC_IMPRESSIONS]     = (Prescription? PRESCRIPTION : (Administratif? COURRIERADMINISTRATIF : COURRIER));
        listbinds[CP_IDLIEU_IMPRESSIONS]        = m_currentuser->idsitedetravail();
        listbinds[CP_IMPORTANCE_IMPRESSIONS]    = (Administratif? "0" : "1");
        DocExterne * doc = DocsExternes::CreationDocumentExterne(listbinds);
        ui->OuvreDocsExternespushButton->setEnabled(doc != Q_NULLPTR);
        if(doc != Q_NULLPTR)
            delete doc;
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
    ui->ActeCotationcomboBox->lineEdit()->setValidator(new QRegExpValidator(Utils::rgx_cotation,this));
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
    ui->CreerNomlineEdit->setValidator(new QRegExpValidator(Utils::rgx_rx,this));
    ui->CreerPrenomlineEdit->setValidator(new QRegExpValidator(Utils::rgx_rx,this));
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

    ModifIdentificationupSmallButton = new UpSmallButton(ui->IdentPatienttextEdit);
    ModifIdentificationupSmallButton->setUpButtonStyle(UpSmallButton::EDITBUTTON);
    ModifIdentificationupSmallButton->move(315,190);
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
    bool a = (m_currentuser->isLiberal() || m_currentuser->isSecretaire());
    actionPaiementTiers             ->setVisible(a);
    actionPaiementDirect            ->setVisible(a || (m_currentuser->isSalarie() && !m_currentuser->isAssistant()) || m_currentuser->isRemplacant());
    actionBilanRecettes             ->setVisible(a);
    actionRecettesSpeciales         ->setVisible(m_currentuser->isComptable());
    actionJournalDepenses           ->setVisible(a);
    actionGestionComptesBancaires   ->setVisible(m_currentuser->isComptable());
    actionRemiseCheques             ->setVisible(a);
    menuComptabilite                ->setVisible(a || (m_currentuser->isSalarie() && !m_currentuser->isAssistant()) || m_currentuser->isRemplacant());
    actionEnregistrerVideo          ->setVisible(db->getMode() != DataBase::Distant);
}

/*-----------------------------------------------------------------------------------------------------------------
-- Initialisation des variables -----------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void Rufus::InitVariables()
{
    gAutorModifConsult          = false;
    m_patients                  = Datas::I()->patients;
    m_listeactes                = Datas::I()->actes;
    m_lignespaiements            = Datas::I()->lignespaiements;
    gdateParDefaut              = QDate::fromString("2000-01-01", "yyyy-MM-dd");
    gAffichTotalMessages        = true;

    MGlineEdit                  = new UpLineEdit();
    AutresCorresp1LineEdit      = new UpLineEdit();
    AutresCorresp2LineEdit      = new UpLineEdit();
    ImportDocsExtThread         = Q_NULLPTR;
    gUserDateDernierMessage     = QDateTime();

    ui->AtcdtsOphstextEdit      ->setChamp(CP_ATCDTSOPH_RMP);
    ui->AtcdtsOphstextEdit      ->setTable(TBL_RENSEIGNEMENTSMEDICAUXPATIENTS);
    ui->TtOphtextEdit           ->setChamp(CP_TRAITMTOPH_RMP);
    ui->TtOphtextEdit           ->setTable(TBL_RENSEIGNEMENTSMEDICAUXPATIENTS);
    ui->ImportanttextEdit       ->setChamp(CP_IMPORTANT_RMP);
    ui->ImportanttextEdit       ->setTable(TBL_RENSEIGNEMENTSMEDICAUXPATIENTS);
    ui->ResumetextEdit          ->setChamp(CP_RESUME_RMP);
    ui->ResumetextEdit          ->setTable(TBL_RENSEIGNEMENTSMEDICAUXPATIENTS);

    ui->AtcdtsPersostextEdit    ->setChamp(CP_ATCDTSPERSOS_RMP);
    ui->AtcdtsPersostextEdit    ->setTable(TBL_RENSEIGNEMENTSMEDICAUXPATIENTS);
    ui->AtcdtsFamiliauxtextEdit ->setChamp(CP_ATCDTSFAMLXS_RMP);
    ui->AtcdtsFamiliauxtextEdit ->setTable(TBL_RENSEIGNEMENTSMEDICAUXPATIENTS);
    ui->TtGeneraltextEdit       ->setChamp(CP_TRAITMTGEN_RMP);
    ui->TtGeneraltextEdit       ->setTable(TBL_RENSEIGNEMENTSMEDICAUXPATIENTS);
    ui->AutresToxiquestextEdit  ->setChamp(CP_AUTRESTOXIQUES_RMP);
    ui->AutresToxiquestextEdit  ->setTable(TBL_RENSEIGNEMENTSMEDICAUXPATIENTS);

    MGlineEdit                  ->setTable(TBL_RENSEIGNEMENTSMEDICAUXPATIENTS);
    MGlineEdit                  ->setChamp(CP_IDMG_RMP);
    AutresCorresp1LineEdit      ->setTable(TBL_RENSEIGNEMENTSMEDICAUXPATIENTS);
    AutresCorresp1LineEdit      ->setChamp(CP_IDSPE1_RMP);
    AutresCorresp2LineEdit      ->setTable(TBL_RENSEIGNEMENTSMEDICAUXPATIENTS);
    AutresCorresp2LineEdit      ->setChamp(CP_IDSPE2_RMP);
    ui->TabaclineEdit           ->setTable(TBL_RENSEIGNEMENTSMEDICAUXPATIENTS);
    ui->TabaclineEdit           ->setChamp(CP_TABAC_RMP);

    ui->ActeMotiftextEdit       ->setChamp(CP_MOTIF_ACTES);
    ui->ActeMotiftextEdit       ->setTable(TBL_ACTES);
    ui->ActeTextetextEdit       ->setChamp(CP_TEXTE_ACTES);
    ui->ActeTextetextEdit       ->setTable(TBL_ACTES);
    ui->ActeConclusiontextEdit  ->setChamp(CP_CONCLUSION_ACTES);
    ui->ActeConclusiontextEdit  ->setTable(TBL_ACTES);

    ui->LListepushButton        ->setEnabled(false);
    ui->CreerDossierpushButton  ->setText(tr("Ouvrir\nle dossier"));
    ui->CreerDossierpushButton  ->setEnabled(false);
    ui->CreerDDNdateEdit        ->setVisible(false);
    ui->DDNlabel                ->setVisible(false);

}

/*-----------------------------------------------------------------------------------------------------------------
-- Recherche d'un dossier -----------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
int Rufus::RecherchePatient(QString PatNom, QString PatPrenom, QString PatDDN, QString MessageErreur)

{
    QString requete = "SELECT  idPat, PatNom, PatPrenom, PatDDN, PatCreeLe, PatCreePar"
            " FROM " TBL_PATIENTS
            " WHERE PatNom = '" + Utils::correctquoteSQL(PatNom) + "'"
            " AND PatPrenom = '" + Utils::correctquoteSQL(PatPrenom) + "'"
            " AND PatDDN = '" + PatDDN +"'";

    QVariantList patdata = db->getFirstRecordFromStandardSelectSQL(requete, ok, MessageErreur);
    if (!ok)
        return -1;
    if (patdata.size() == 0)        // Aucune mesure trouvee pour ces criteres
        return 0;
    return patdata.at(0).toInt();   // renvoie idPatient
}

// ------------------------------------------------------------------------------------------
// Enregistre ou met à jour les coordonnées du médecin traitant
// ------------------------------------------------------------------------------------------
void Rufus::MAJCorrespondant(QObject *obj)
{
    if (Datas::I()->patients->currentpatient()->id() == 0)
        return;
    UpComboBox* cbox = dynamic_cast<UpComboBox*>(obj);
    if (cbox == Q_NULLPTR) return;
    UpLineEdit *Upline = dynamic_cast<UpLineEdit*>(cbox->lineEdit());
    if (Upline == Q_NULLPTR) return;
    QString anc = cbox->getValeurAvant();
    QString nou = Utils::trimcapitilize(cbox->currentText(),true);
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
            msgbox.setText("Euuhh... " + m_currentuser->login());
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
                    if (cbox == ui->MGupComboBox)
                        Datas::I()->patients->updateCorrespondant(Datas::I()->patients->currentpatient(), Correspondant::MG, Datas::I()->correspondants->getById(idcor));
                    else if (cbox == ui->AutresCorresp1upComboBox)
                        Datas::I()->patients->updateCorrespondant(Datas::I()->patients->currentpatient(), Correspondant::Spe1, Datas::I()->correspondants->getById(idcor));
                    else if (cbox == ui->AutresCorresp2upComboBox)
                        Datas::I()->patients->updateCorrespondant(Datas::I()->patients->currentpatient(), Correspondant::Spe2, Datas::I()->correspondants->getById(idcor));
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
        if (cbox == ui->MGupComboBox)
            Datas::I()->patients->updateCorrespondant(Datas::I()->patients->currentpatient(), Correspondant::MG);
        else if (cbox == ui->AutresCorresp1upComboBox)
            Datas::I()->patients->updateCorrespondant(Datas::I()->patients->currentpatient(), Correspondant::Spe1);
        else if (cbox == ui->AutresCorresp2upComboBox)
            Datas::I()->patients->updateCorrespondant(Datas::I()->patients->currentpatient(), Correspondant::Spe2);
        cbox->setCurrentIndex(-1);
        OKModifierTerrain(Datas::I()->patients->currentpatient());
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
    int row = listindx.at(0).row();
    if (row > 0)
        RecaleTableView(getPatientFromRow(row - 1));
}


// ------------------------------------------------------------------------------------------
// Monte 20 lignes dans la table
// ------------------------------------------------------------------------------------------

void Rufus::Monte20Lignes()
{
    QModelIndexList listindx = ui->PatientsListeTableView->selectionModel()->selectedIndexes();
    if (listindx.size()==0) return;
    int row = listindx.at(0).row();
    if (row > 19)
        RecaleTableView(getPatientFromRow(row - 20));
    else
        RecaleTableView(getPatientFromRow(0), QAbstractItemView::PositionAtTop);
}

/*------------------------------------------------------------------------------------------------------------------------------------
-- Afficher la consultation précédente ou suivante d'un patient ----------------------------------------------------------------------
------------------------------------------------------------------------------------------------------------------------------------*/
bool Rufus::NavigationConsult(ItemsList::POSITION i)
{
    if (Datas::I()->patients->currentpatient()->id() == 0)
        return false;
    if(!AutorDepartConsult(false)) return false;
    //  Afficher les éléments de la tables Actes
    int idActe = -1;
    if (m_listeactes->actes()->size() < 2)
        return false;
    int a (0);
    a = m_listeactes->actes()->keys().indexOf(m_currentact->id());

    if (i == ItemsList::Suiv)
        if (a< m_listeactes->actes()->size()-1)
            idActe = m_listeactes->actes()->keys().at(a+1);
    if (i == ItemsList::Prec)
        if (a > 0)
            idActe = m_listeactes->actes()->keys().at(a-1);
    if (i == ItemsList::Debut)
        idActe = m_listeactes->actes()->firstKey();
    if (i == ItemsList::Fin)
        idActe = m_listeactes->actes()->lastKey();

    if (idActe > -1)
    {
        AfficheActe(m_listeactes->getById(idActe));
        return true;
    }
    else
        return false;
}

/*-----------------------------------------------------------------------------------------------------------------
-- Visualiser la fiche dlg_actesprecedents ------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void    Rufus::OuvrirActesPrecedents()
{
    Dlg_ActesPrecs = new dlg_actesprecedents(Datas::I()->actes, true, this);
    Dlg_ActesPrecs->show();
}

/*-----------------------------------------------------------------------------------------------------------------
-- Ouvrir la fiche documents ------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void    Rufus::OuvrirDocuments(bool AffichDocsExternes)
{
    QString nom, prenom;
    if (ui->tabWidget->currentWidget() == ui->tabDossier)
    {
        nom         = Datas::I()->patients->currentpatient()->nom();
        prenom      = Datas::I()->patients->currentpatient()->prenom();
        Dlg_Docs    = new dlg_documents(Datas::I()->patients->currentpatient());
    }
    else
    {
        nom         = Datas::I()->patients->dossierpatientaouvrir()->nom();
        prenom      = Datas::I()->patients->dossierpatientaouvrir()->prenom();
        Dlg_Docs    = new dlg_documents(Datas::I()->patients->dossierpatientaouvrir(), this);
    }
    Dlg_Docs->setWindowTitle(tr("Préparer un document pour ") + nom + " " + prenom);
    Dlg_Docs->setWindowIcon(Icons::icLoupe());
    bool aa = true;
    if (Dlg_Docs->exec() > 0)
    {
        User *userEntete = Dlg_Docs->getUserEntete();
        if (userEntete == Q_NULLPTR)
            return;

        QString     Entete;
        QDate DateDoc = Dlg_Docs->ui->dateEdit->date();
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
            aa                      = Imprimer_Document(userEntete, Titre, Entete, TxtDocument, DateDoc, nom, prenom, Prescription, ALD, AvecPrevisu, AvecDupli, AvecChoixImprimante, Administratif);
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
void    Rufus::ModeSelectDepuisListe()
{
    if (ui->tabWidget->currentIndex() == ui->tabWidget->indexOf(ui->tabDossier))
        ui->AtcdtsPersostextEdit->setFocus();
    ui->CreerNomlineEdit->clear();
    ui->CreerPrenomlineEdit->clear();
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
    ui->LRecopierpushButton->setEnabled(false);
    if (Datas::I()->patients->currentpatient()->id() > 0)
        RecaleTableView(Datas::I()->patients->currentpatient());
    else if (m_listepatientsmodel->rowCount() > 0)
        RecaleTableView(getPatientFromRow(0), QAbstractItemView::PositionAtTop);
    gMode = Liste;
    EnableButtons();
    ui->CreerNomlineEdit->setFocus();
}


/*-----------------------------------------------------------------------------------------------------------------
-- Enregistrer les données pour créer un nouveau dossier - Mise en place de la fiche ------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void Rufus::ModeCreationDossier()
{
    if(!AutorDepartConsult(true)) return;
    if (ui->tabWidget->indexOf(ui->tabDossier) != -1)
    {
        ui->tabWidget->setCurrentWidget(ui->tabList);
        ui->tabWidget->removeTab(ui->tabWidget->indexOf(ui->tabDossier));
        FermeDlgActesPrecedentsEtDocsExternes();
    }

    if (gMode == NouveauDossier && ui->tabWidget->currentIndex() == ui->tabWidget->indexOf(ui->tabList)) return;
    ui->tabWidget->setCurrentIndex(ui->tabWidget->indexOf(ui->tabList));
    ui->CreerNomlineEdit->setVisible(true);
    ui->CreerPrenomlineEdit->setVisible(true);
    ui->Nomlabel->setVisible(true);
    ui->Prenomlabel->setVisible(true);
    ui->ChercherDepuisListepushButton->setVisible(false);

    ui->CreerNomlineEdit->setFocus();
    ui->CreerDossierpushButton->setIcon(Icons::icOK());
    ui->CreerDossierpushButton->setIconSize(QSize(30,30));
    ui->CreerDossierpushButton->setText(tr("Créer\nle dossier"));
    ui->LListepushButton->setEnabled(true);
    ui->LNouvDossierpushButton->setEnabled(false);
    ui->CreerDDNdateEdit->setVisible(true);
    ui->CreerDDNdateEdit->setDate(gdateParDefaut);
    ui->DDNlabel->setVisible(true);
    ui->CreerDossierpushButton->setEnabled(ui->CreerNomlineEdit->text() != "" && ui->CreerPrenomlineEdit->text() != "");
    gMode = NouveauDossier;
}

/*-----------------------------------------------------------------------------------------------------------------
-- Créer un dossier de la meme famille - Mise en place de la fiche ------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void    Rufus::RecopierDossier(Patient *patient)
{
    if (patient != Q_NULLPTR)
    {
        //if (!patient->isalloaded())
        patient = m_patients->getById(patient->id(), Item::LoadDetails);
        FermeDlgActesPrecedentsEtDocsExternes();
        IdentificationPatient(dlg_identificationpatient::Copie, patient);
        return;
    }
    else
    {
        if (ui->tabWidget->indexOf(ui->tabDossier) > -1)
            if(!AutorDepartConsult(true))
                return;
        ui->tabWidget->removeTab(ui->tabWidget->indexOf(ui->tabDossier));
        Patient* pat = getPatientFromSelectionInTable();
        if (pat == Q_NULLPTR)
        {
            UpMessageBox::Watch(this, tr("Aucun dossier sélectionné!"), tr("Sélectionnez d'abord un dossier à recopier."));
            return;
        }
        //if (!pat->isalloaded())
        pat = m_patients->getById(pat->id(), Item::LoadDetails);
        FermeDlgActesPrecedentsEtDocsExternes();
        IdentificationPatient(dlg_identificationpatient::Copie, pat);
    }
}

/*-----------------------------------------------------------------------------------------------------------------
-- Recaleles TreeView sur le dossier en cours ---------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void Rufus::RecaleTableView(Patient* pat, QAbstractItemView::ScrollHint scrollhint)
{
    if (pat == Q_NULLPTR)
        return;
    QList<QStandardItem*> listitems = m_listepatientsmodel->findItems(QString::number(pat->id()));
    if (listitems.size() > 0)
    {
        QStandardItem *item = listitems.at(0);
        QModelIndex pindx       = m_DDNsortmodel->mapFromSource(item->index());
        QModelIndex pprenomindx = m_prenomfiltersortmodel->mapFromSource(pindx);
        QModelIndex finalindx   = m_listepatientsproxymodel->mapFromSource(pprenomindx);
        int row = getRowFromPatient(pat);
        if (row > -1)
            ui->PatientsListeTableView->selectRow(row);
        ui->PatientsListeTableView->scrollTo(finalindx, scrollhint);
    }
}

/*-----------------------------------------------------------------------------------------------------------------
-- Reconstruit la liste des Cotations dans le combobox ActeCotations ----------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void    Rufus::ReconstruitListesCotations()
{
    ui->ActeCotationcomboBox->disconnect();
    if (ui->ActeCotationcomboBox->lineEdit()->completer() != Q_NULLPTR)
        delete ui->ActeCotationcomboBox->lineEdit()->completer();
    /* reconstruit les items du combobox ui->ActeCotationComboBox
       chaque item contient
            . le texte de l'item -> la cotation de l'acte
            . en data, une QStringList contenant dans l'ordre le montant (optam ou non), le montant pratiqué, le descriptif de l'acte CCAM
    */
    Datas::I()->cotations->initListeByUser(m_currentuser->idparent());
    QString req;
    QString champ = (m_currentuser->isOPTAM()? "montantoptam" : "montantnonoptam");
    // il faut d'abord reconstruire la table des cotations
    ui->ActeCotationcomboBox->clear();

    QStandardItemModel *cotationmodel = new QStandardItemModel(this);
    UpStandardItem *pitem0;
    pitem0 = new UpStandardItem(tr("Acte gratuit"));
    pitem0->setData(QStringList() << "0.00" << "0.00" << tr("Acte gratuit"));
    cotationmodel->appendRow(QList<QStandardItem*>() << pitem0);
    foreach (Cotation* cot, Datas::I()->cotations->cotations()->values())
    {
        QStringList list;
        QString champ = (m_currentuser->isOPTAM()? QString::number(cot->montantoptam(),'f',2) : QString::number(cot->montantnonoptam(), 'f', 2));
        list << champ << QString::number(cot->montantpratique(), 'f', 2) << cot->descriptif();
        pitem0 = new UpStandardItem(cot->typeacte());
        pitem0->setData(list);
        cotationmodel->appendRow(QList<QStandardItem*>() << pitem0);
    }

    ui->ActeCotationcomboBox->addItem(tr("Acte gratuit"),QStringList() << "0.00" << "0.00" << tr("Acte gratuit"));
    foreach (Cotation* cot, Datas::I()->cotations->cotations()->values())
    {
        QStringList list;
        QString champ = (m_currentuser->isOPTAM()? QString::number(cot->montantoptam(),'f',2) : QString::number(cot->montantnonoptam(), 'f', 2));
        list << champ << QString::number(cot->montantpratique(), 'f', 2) << cot->descriptif();
        ui->ActeCotationcomboBox->addItem(cot->typeacte(),list);
    }

    QCompleter *comp = new QCompleter(QStringList() << tr("Acte gratuit") << db->loadTypesCotations());
    comp->setCaseSensitivity(Qt::CaseInsensitive);
    comp->popup()->setFont(ui->ActeMontantlineEdit->font());
    comp->setMaxVisibleItems(5);
    ui->ActeCotationcomboBox->lineEdit()->setCompleter(comp);
    connect(comp,                       QOverload<const QString &>::of(&QCompleter::activated), this,   [=] {RetrouveMontantActe();});
    ConnectCotationComboBox();
}

void Rufus::ConnectCotationComboBox()
{
    connect (ui->ActeCotationcomboBox,  &QComboBox::currentTextChanged, this,
    [=] {
        RetrouveMontantActe();
        ValideActeMontantLineEdit(ui->ActeMontantlineEdit->text(), gActeMontant);
    });
    connect (ui->ActeCotationcomboBox,  QOverload<int>::of(&QComboBox::highlighted),    this,
    [=] (int a) {
        QString tip = ui->ActeCotationcomboBox->itemData(a).toStringList().at(2);
        QToolTip::showText(cursor().pos(),tip);
    });
}

void Rufus::ReconstruitCombosCorresp(bool reconstruireliste)
{
    if (reconstruireliste)
        Datas::I()->correspondants->initListe();
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
    proc->ReconstruitComboCorrespondants(ui->MGupComboBox, Correspondants::QueLesGeneralistes);
    ui->AutresCorresp1upComboBox->clear();
    ui->AutresCorresp2upComboBox->clear();
    proc->ReconstruitComboCorrespondants(ui->AutresCorresp1upComboBox, Correspondants::TousLesCorrespondants);
    proc->ReconstruitComboCorrespondants(ui->AutresCorresp2upComboBox, Correspondants::TousLesCorrespondants);
    if (ui->tabWidget->currentIndex() == ui->tabWidget->indexOf(ui->tabDossier))
    {
        ui->MGupComboBox            ->setCurrentIndex( idcor>-1?     ui->MGupComboBox               ->findData(idcor)   : -1 );
        ui->AutresCorresp1upComboBox->setCurrentIndex( idcorA1>-1?   ui->AutresCorresp1upComboBox   ->findData(idcorA1) : -1 );
        ui->AutresCorresp2upComboBox->setCurrentIndex( idcorA2>-1?   ui->AutresCorresp2upComboBox   ->findData(idcorA2) : -1 );
    }
    //Actualisation des combobox des correspondants
    if (ui->tabWidget->indexOf(ui->tabDossier) == -1)
        return;

    QString tooltp = "";
    if (Datas::I()->patients->currentpatient()->idmg()>0)
    {
        ui->MGupComboBox->setCurrentIndex(ui->MGupComboBox->findData(Datas::I()->patients->currentpatient()->idmg()));
        tooltp = CalcToolTipCorrespondant(Datas::I()->patients->currentpatient()->idmg());
    }
    else
    {
        MGlineEdit->clear();
        ui->MGupComboBox->setCurrentIndex(-1);
    }
    ui->MGupComboBox->setImmediateToolTip(tooltp);
    tooltp = "";
    if (Datas::I()->patients->currentpatient()->idspe1()>0)
    {
        ui->AutresCorresp1upComboBox->setCurrentIndex
                (ui->AutresCorresp1upComboBox->findData(Datas::I()->patients->currentpatient()->idspe1()));
        tooltp = CalcToolTipCorrespondant(Datas::I()->patients->currentpatient()->idspe1());
    }
    else
    {
        AutresCorresp1LineEdit->clear();
        ui->AutresCorresp1upComboBox->setCurrentIndex(-1);
    }
    ui->AutresCorresp1upComboBox->setImmediateToolTip(tooltp);
    tooltp = "";
    if (Datas::I()->patients->currentpatient()->idspe2()>0)
    {
        ui->AutresCorresp2upComboBox->setCurrentIndex
                (ui->AutresCorresp2upComboBox->findData(Datas::I()->patients->currentpatient()->idspe2()));
        tooltp = CalcToolTipCorrespondant(Datas::I()->patients->currentpatient()->idspe2());
    }
    else
    {
        AutresCorresp2LineEdit->clear();
        ui->AutresCorresp2upComboBox->setCurrentIndex(-1);
    }
    ui->AutresCorresp2upComboBox->setImmediateToolTip(tooltp);
    OKModifierTerrain(Datas::I()->patients->currentpatient());
}

/*-----------------------------------------------------------------------------------------------------------------
-- Mesurer la Refraction ------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void    Rufus::RefractionMesure()
{
    if (findChildren<dlg_refraction*>().size()>0)
        return;
    if (Datas::I()->patients->currentpatient()->id() == 0 || m_currentact == Q_NULLPTR)
        return;
    if (ui->tabWidget->currentIndex() != 1 || !ui->Acteframe->isVisible())
        return;

    Dlg_Refraction     = new dlg_refraction(m_currentact, this);
    proc->setFicheRefractionOuverte(true);
    int result = Dlg_Refraction->exec();
    proc->setFicheRefractionOuverte(false);
    if (result > 0)
    {
        if (Dlg_Refraction->ResultatObservation() != "")  // Ce n'est pas une prescription de verres correcteurs
        {
            for (int i= 0; i<Dlg_Refraction->ResultatObservation().size();i++)
                if (Dlg_Refraction->ResultatObservation().at(i).unicode() == 10) Dlg_Refraction->ResultatObservation().replace(Dlg_Refraction->ResultatObservation().at(i),"<br>");
            //qDebug() << Dlg_Refraction->ResultatObservation();
            QString ARajouterEnText =
                    "<p style = \"margin-top:0px; margin-bottom:0px;\">"
                    + Dlg_Refraction->ResultatObservation()
                    + "<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px;\"></p>";
            ItemsList::update(m_currentact, CP_TEXTE_ACTES, ui->ActeTextetextEdit->appendHtml(ARajouterEnText));
            ui->ActeTextetextEdit->setFocus();
            ui->ActeTextetextEdit->moveCursor(QTextCursor::End);
        }
        else if (Dlg_Refraction->ResultatPrescription() != "")  // C'est une prescription de verres correcteurs
        {
            // mettre à jour docsexterns
            MAJDocsExternes(); //Refraction()

            // si le dernier caractère n'est pas un retour à la ligne, on en rajoute un
            QString Date = "";
            for (int i= 0; i<Dlg_Refraction->ResultatPrescription().size();i++)
                if (Dlg_Refraction->ResultatPrescription().at(i).unicode() == 10) Dlg_Refraction->ResultatPrescription().replace(Dlg_Refraction->ResultatPrescription().at(i),"<br>");
            QString larg = "550";
            if (ui->ActeDatedateEdit->date() != QDate::currentDate())
            {
                Date = "<td width=\"80\">le " + QDate::currentDate().toString("d.M.yyyy") + "</td>";
                larg = "470";
            }
            QString ARajouterEnConcl =  "<p style = \"margin-top:0px; margin-bottom:0px;\" >" + Date + Dlg_Refraction->ResultatPrescription()  + "</p>"
                                         + "<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px;\">";
            ItemsList::update(m_currentact, CP_CONCLUSION_ACTES, ui->ActeConclusiontextEdit->appendHtml(ARajouterEnConcl,"","",true));
            ui->ActeConclusiontextEdit->setFocus();
            ui->ActeConclusiontextEdit->moveCursor(QTextCursor::End);
        }
        Datas::I()->refractions->initListebyPatId(Datas::I()->patients->currentpatient()->id());
    }
    delete Dlg_Refraction;
}

void Rufus::RegleRefracteur(Refraction::Mesure mesure)
{
    if (Datas::I()->patients->currentpatient()->id() == 0)
        return;
    QMap<QString,QVariant>      Mesure;
    Mesure["AxeOD"]     = "180";
    Mesure["AxeOG"]     = "180";
    Mesure["SphereOD"]  = "+00.00";
    Mesure["SphereOG"]  = "+00.00";
    Mesure["CylOD"]     = "+00.00";
    Mesure["CylOG"]     = "+00.00";
    Mesure["AddOD"]     = "+00.00";
    Mesure["AddOG"]     = "+00.00";
    Refraction *ref = Q_NULLPTR;
    QMapIterator<int, Refraction*> itref(*Datas::I()->refractions->refractions());
    itref.toBack();
    while (itref.hasPrevious()) {
        itref.previous();
        if (itref.value()->distance() != Refraction::Pres)
        {
                if (mesure == Refraction::Autoref)
                {
                    if (itref.value()->mesure() == Refraction::Acuite)
                        ref= const_cast<Refraction*>(itref.value());
                }
                else if (mesure == Refraction::Fronto)
                {
                    if (itref.value()->mesure() == Refraction::Prescription || itref.value()->mesure() == Refraction::Fronto)
                        ref= const_cast<Refraction*>(itref.value());
                }
                if (ref != Q_NULLPTR)
                    itref.toFront();
        }
    }
    if (ref == Q_NULLPTR)
        return;
    QString prefix = "";
    // Les axes
    if (ref->cylindreOD() != 0.0)
    {
        if (ref->axecylindreOD() < 10)
            prefix = "  ";
        else if (ref->axecylindreOD() < 100)
            prefix = " ";
        Mesure["AxeOD"] = prefix + QString::number(ref->axecylindreOD());
    }
    prefix = "";
    if (ref->cylindreOG() != 0.0)
    {
        if (ref->axecylindreOG() < 10)
            prefix = "  ";
        else if (ref->axecylindreOG() < 100)
            prefix = " ";
        Mesure["AxeOG"] = prefix + QString::number(ref->axecylindreOG());
    }

    // Les spheres
    prefix = "";
    if (ref->sphereOD() > 0)
    {
        if (ref->sphereOD() < 10)
            prefix = "+0";
        else
            prefix = "+";
        Mesure["SphereOD"] = prefix + QString::number(ref->sphereOD(),'f',2);
    }
    else if (ref->sphereOD() < 0)
    {
        prefix = QString::number(ref->sphereOD(),'f',2);
        if (ref->sphereOD() > -10)
            prefix.replace("-", "-0");
        Mesure["SphereOD"] = prefix;
    }
    prefix = "";
    if (ref->sphereOG() > 0)
    {
        if (ref->sphereOG() < 10)
            prefix = "+0";
        else
            prefix = "+";
        Mesure["SphereOG"] = prefix + QString::number(ref->sphereOG(),'f',2);
    }
    else if (ref->sphereOG() < 0)
    {
        prefix = QString::number(ref->sphereOG(),'f',2);
        if (ref->sphereOG() > -10)
            prefix.replace("-", "-0");
        Mesure["SphereOG"] = prefix;
    }

    // Les cylindres
    prefix = "";
    if (ref->cylindreOD() > 0)
    {
        if (ref->cylindreOD() < 10)
            prefix = "+0";
        else
            prefix = "+";
        Mesure["CylOD"] = prefix + QString::number(ref->cylindreOD(),'f',2);
    }
    else if (ref->cylindreOD() < 0)
    {
        prefix = QString::number(ref->cylindreOD(),'f',2);
        if (ref->cylindreOD() > -10)
            prefix.replace("-", "-0");
        Mesure["CylOD"] = prefix;
    }
    prefix = "";
    if (ref->cylindreOG() > 0)
    {
        if (ref->cylindreOG() < 10)
            prefix = "+0";
        else
            prefix = "+0";
        Mesure["CylOG"] = prefix + QString::number(ref->cylindreOG(),'f',2);
    }
    else if (ref->cylindreOG() < 0)
    {
        prefix = QString::number(ref->cylindreOG(),'f',2);
        if (ref->cylindreOG() > -10)
            prefix.replace("-", "-0");
        Mesure["CylOG"] = prefix;
    }

    // Les additions
    if (ref->addVPOD() != 0.0)
        Mesure["AddOD"] = "+0" + QString::number(ref->addVPOD(),'f',2);
    if (ref->addVPOG()!=0.0)
        Mesure["AddOG"] = "+0" + QString::number(ref->addVPOG(),'f',2);

    // Les formules
    Mesure["FormuleOD"] = ref->formuleOD();
    Mesure["FormuleOG"] = ref->formuleOG();

    if (mesure == Refraction::Autoref)
        gMesureAutoref = Mesure;
    else if (mesure == Refraction::Fronto)
        gMesureFronto = Mesure;
}

/*-----------------------------------------------------------------------------------------------------------------
-- Remise de chèques ----------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void Rufus::RemiseCheques()
{
    Dlg_RemCheq          = new dlg_remisecheques();
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
bool Rufus::Remplir_ListePatients_TableView()
{
    UpStandardItem *pitem0, *pitem1, *pitem2, *pitem3, *pitem4, *pitem5;
    m_listepatientsmodel = dynamic_cast<QStandardItemModel*>(ui->PatientsListeTableView->model());
    if (m_listepatientsmodel != Q_NULLPTR)
        m_listepatientsmodel->clear();
    else
        m_listepatientsmodel = new QStandardItemModel;
    foreach (Patient *pat, m_patients->patientstable()->values())
    {
        pitem0  = new UpStandardItem(QString::number(pat->id()));                                   // id                           -> utilisé pour le drop event
        pitem1  = new UpStandardItem(pat->nom().toUpper() + " " + pat->prenom());                   // Nom + Prénom
        pitem2  = new UpStandardItem(pat->datedenaissance().toString(tr("dd-MM-yyyy")));            // date de naissance
        pitem3  = new UpStandardItem(pat->datedenaissance().toString(tr("yyyyMMdd")));              // date de naissance inversée   -> utilisé pour le tri
        pitem4  = new UpStandardItem(pat->nom());                                                   // Nom                          -> utilisé pour le tri
        pitem5  = new UpStandardItem(pat->prenom());                                                // Prénom                       -> utilisé pour le tri
        pitem0  ->setItem(pat);
        pitem1  ->setItem(pat);
        pitem2  ->setItem(pat);
        pitem3  ->setItem(pat);
        pitem4  ->setItem(pat);
        pitem4  ->setItem(pat);
        m_listepatientsmodel->appendRow(QList<QStandardItem *>() << pitem0 << pitem1 << pitem2 << pitem3 << pitem4 << pitem5);
    }
    QStandardItem *itnom = new QStandardItem();
    itnom->setText("Nom");
    itnom->setTextAlignment(Qt::AlignLeft);
    m_listepatientsmodel->setHorizontalHeaderItem(1,itnom);
    QStandardItem *itDDN = new QStandardItem();
    itDDN->setText("Date de naissance");
    itDDN->setTextAlignment(Qt::AlignLeft);
    m_listepatientsmodel->setHorizontalHeaderItem(2,itDDN);

    if (m_DDNsortmodel == Q_NULLPTR)
        m_DDNsortmodel = new QSortFilterProxyModel();
    m_DDNsortmodel->setSourceModel(m_listepatientsmodel);
    m_DDNsortmodel->sort(3);

    if (m_prenomfiltersortmodel == Q_NULLPTR)
        m_prenomfiltersortmodel = new QSortFilterProxyModel();
    m_prenomfiltersortmodel->setSourceModel(m_DDNsortmodel);
    m_prenomfiltersortmodel->sort(5);
    m_prenomfiltersortmodel->setFilterKeyColumn(5);

    if (m_listepatientsproxymodel == Q_NULLPTR)
        m_listepatientsproxymodel = new QSortFilterProxyModel();
    m_listepatientsproxymodel->setSourceModel(m_prenomfiltersortmodel);
    m_listepatientsproxymodel->sort(4);
    m_listepatientsproxymodel->setFilterKeyColumn(4);

    ui->PatientsListeTableView->setModel(m_listepatientsproxymodel);
    ui->PatientsListeTableView->setColumnWidth(0,0 );         //id
    ui->PatientsListeTableView->setColumnWidth(1,230 );       //Nom + Prénom
    ui->PatientsListeTableView->setColumnWidth(2,122 );       //DDN
    ui->PatientsListeTableView->setColumnWidth(3,0 );         //DDN inversé pour le tri
    ui->PatientsListeTableView->setColumnWidth(4,0 );         //nom utilisé pour le tri
    ui->PatientsListeTableView->setColumnWidth(5,0 );         //prénom utilisé pour le tri

    QFontMetrics fm(qApp->font());
    for (int j=0; j<Datas::I()->patients->patientstable()->size(); j++)
         ui->PatientsListeTableView->setRowHeight(j,int(fm.height()*1.3));

    ui->PatientsListeTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->PatientsListeTableView->horizontalHeader()->setFixedHeight(int(fm.height()*1.3));
    CalcNbDossiers();

    return true;
}

void Rufus::Remplir_SalDat()
{
    QTableWidget        *TableAMettreAJour;
    int                 i=0;
    QString             NomPrenom, zw, A;
    QFontMetrics        fm(qApp->font());
    m_listepatientsencours->initListeAll();
    QList<int> listidpat;
    // toute la manip qui suit sert à remetre les patients en cours par ordre chronologique - si vous trouvez plus simple, ne vous génez pas

    if (m_listepatientsencoursmodel == Q_NULLPTR)
        m_listepatientsencoursmodel = new QStandardItemModel();
    else
        m_listepatientsencoursmodel->clear();
    foreach (PatientEnCours *pat, m_listepatientsencours->patientsencours()->values())
    {
        QList<QStandardItem *> items;
        listidpat << pat->id();
        UpStandardItem *itempat = new UpStandardItem(QString::number(pat->id()));
        itempat->setItem(pat);
        items << new UpStandardItem(pat->heurerdv().toString("HHmm"))
              << itempat;
        m_listepatientsencoursmodel->appendRow(items);
    }
    Datas::I()->patients->initListeSalDat(listidpat);
    QList<PatientEnCours*> listpat;
    m_listepatientsencoursmodel->sort(0);
    for (int i=0; i<m_listepatientsencoursmodel->rowCount(); ++i)
    {
        UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_listepatientsencoursmodel->item(i,1));
        if (itm != Q_NULLPTR)
        {
            PatientEnCours *pat = dynamic_cast<PatientEnCours*>(itm->item());
            if (pat != Q_NULLPTR)
            {
                //qDebug() << m_patients->getById(pat->id())->nom() + " " + m_patients->getById(pat->id())->prenom() + " " + pat->statut();
                listpat << pat;
            }
        }
    }

    // SALLE D'ATTENTE ---------------------------------------------------------------------------------------------------
    TableAMettreAJour = ui->SalleDAttenteupTableWidget;
    ui->SalleDAttenteupTableWidget->clearAllRowsExceptHeader();
    QList<PatientEnCours*> listpatsaldat;
    foreach (PatientEnCours* pat, listpat)
    {
        if (pat->statut() == ARRIVE || pat->statut() == ENCOURS || pat->statut().contains(ENATTENTENOUVELEXAMEN))
            listpatsaldat << pat;
    }
    TableAMettreAJour   ->setRowCount(listpatsaldat.size());
    if (m_listesuperviseursmodel == Q_NULLPTR)
        m_listesuperviseursmodel = new QStandardItemModel();
    else
        m_listesuperviseursmodel->clear();
    QStandardItem       *pitem0, *pitem1;
    QList<int>          listidusers;
    foreach (PatientEnCours* patencours, listpatsaldat)
    {
        Patient *pat                = m_patients->getById(patencours->id());
        QMap<QString, QVariant> rsgnmt;
        rsgnmt["idpat"]             = patencours->id();
        rsgnmt["motif"]             = patencours->motif();
        rsgnmt["ddnpat"]            = pat->datedenaissance();
        rsgnmt["idsuperviseur"]     = patencours->iduser();
        rsgnmt["loginsuperviseur"]  = Datas::I()->users->getById(patencours->iduser())->login();
        rsgnmt["urgence"]           = (patencours->motif() == "URG");
        rsgnmt["message"]           = patencours->message();

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

        QString Msg = patencours->message();
        NomPrenom = " " + pat->nom().toUpper() + " " + pat->prenom();
        label0->setText(NomPrenom);                                                     // Nom + Prénom
        label1->setText(patencours->statut());                                          // Statut
        label4->setText(patencours->motif());                                           // Motif
        if (Msg != "")
            label2->setPixmap(Icons::pxApres().scaled(10,10));                          //WARNING : icon scaled : pxApres 10,10

        QString color;
        if (patencours->heurerarrivee().isValid())
        {
            QTime heureArriv = patencours->heurerarrivee();
            label5->setText(heureArriv.toString("HH:mm"));                              // Heure RDV
            if (heureArriv.secsTo(QTime::currentTime())/60 < 15)
                color = "color: green";
            else if (heureArriv.secsTo(QTime::currentTime())/60 < 30)
               color = "color: orange";
            else
               color = "color: red";
        }
        label6->setText(Datas::I()->users->getById(patencours->iduser())->login());  // Superviseur
        if (!listidusers.contains(patencours->iduser()))
        {
            listidusers << patencours->iduser();
            pitem0 = new QStandardItem(QString::number(patencours->iduser()));
            pitem1 = new QStandardItem(Datas::I()->users->getById(patencours->iduser())->login());
            QList<QStandardItem*> listitems;
            listitems << pitem0 << pitem1;
            m_listesuperviseursmodel    ->appendRow(listitems);
        }
        QString colorRDV;
        if (patencours->heurerdv().isValid())
        {
            QTime heureRDV = patencours->heurerdv();
            label3->setText(heureRDV.toString("HH:mm"));                                // Heure RDV
            if (heureRDV.secsTo(QTime::currentTime())/60 < 15)
                colorRDV = "color: green";
            else if (heureRDV.secsTo(QTime::currentTime())/60 < 30)
               colorRDV = "color: orange";
            else
               colorRDV = "color: red";
        }
        QString background = "background:#FFFFEE";
        if (patencours->motif() == "URG")
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
            Motif *mtf = Datas::I()->motifs->getMotifFromRaccourci(patencours->motif());
            if (mtf != Q_NULLPTR)
                background = "background:#" + mtf->couleur();
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
        connect (label0,        &UpLabel::dblclick,         this,                   [=] {if (m_currentuser->isSoignant()) ChoixDossier(m_patients->getById(rsgnmt["idpat"].toInt()));});
        connect (label1,        &UpLabel::dblclick,         this,                   [=] {if (m_currentuser->isSoignant()) ChoixDossier(m_patients->getById(rsgnmt["idpat"].toInt()));});
        connect (label2,        &UpLabel::dblclick,         this,                   [=] {if (m_currentuser->isSoignant()) ChoixDossier(m_patients->getById(rsgnmt["idpat"].toInt()));});
        connect (label3,        &UpLabel::dblclick,         this,                   [=] {if (m_currentuser->isSoignant()) ChoixDossier(m_patients->getById(rsgnmt["idpat"].toInt()));});
        connect (label4,        &UpLabel::dblclick,         this,                   [=] {if (m_currentuser->isSoignant()) ChoixDossier(m_patients->getById(rsgnmt["idpat"].toInt()));});
        connect (label5,        &UpLabel::dblclick,         this,                   [=] {if (m_currentuser->isSoignant()) ChoixDossier(m_patients->getById(rsgnmt["idpat"].toInt()));});
        connect (label6,        &UpLabel::dblclick,         this,                   [=] {if (m_currentuser->isSoignant()) ChoixDossier(m_patients->getById(rsgnmt["idpat"].toInt()));});
        TableAMettreAJour->setCellWidget(i,0,label0);
        TableAMettreAJour->setCellWidget(i,1,label1);
        TableAMettreAJour->setCellWidget(i,2,label2);
        TableAMettreAJour->setCellWidget(i,3,label3);
        TableAMettreAJour->setCellWidget(i,4,label4);
        TableAMettreAJour->setCellWidget(i,5,label5);
        TableAMettreAJour->setCellWidget(i,6,label6);
        TableAMettreAJour->setRowHeight(i,int(QFontMetrics(qApp->font()).height()*1.1));
        ++i;
    }
    while (gSalDatTab->count()>0)
        gSalDatTab->removeTab(0);
    int k =0;
    if (m_listesuperviseursmodel->rowCount()==0)
        gSalDatTab->setVisible(false);
    else
    {
        gSalDatTab->setVisible(true);
        if (m_listesuperviseursmodel->rowCount()>1)
        {
            gSalDatTab  ->insertTab(0, Icons::icFamily(), tr("Tout le monde"));
            gSalDatTab  ->setTabData(k, -1);
            k++;
        }
        for (int i=0; i<m_listesuperviseursmodel->rowCount(); i++)
        {
            gSalDatTab  ->insertTab(k,m_listesuperviseursmodel->item(i,1)->text());
            gSalDatTab  ->setTabData(k, m_listesuperviseursmodel->item(i,0)->text());
            k++;
        }
        bool a = false;
        for (int i=0; i<gSalDatTab->count(); i++)
        {
            if (gSalDatTab->tabData(i).toInt() == m_currentuser->idsuperviseur())
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
    QList<PosteConnecte*> listpostsoignant;
    /*! sans TCP, on réinitialise la liste des postes connectés à chaque modif de la salle d'attente -
     *  avec TCP, c'est inutile puisque la réinitialisation se fait chaque fois qu'un poste se connecte ou se déconnecte */
    if (!UtiliseTCP)
        Datas::I()->postesconnectes->initListe();
    foreach (PosteConnecte* post, Datas::I()->postesconnectes->postesconnectes()->values())
    {
        if (post != Q_NULLPTR)
        if (Datas::I()->users->getById(post->id())->isSoignant())
            listpostsoignant << post;
    }
    if (listpostsoignant.size() >0)
    {
        ui->scrollArea->setWidgetResizable(true);
        QHBoxLayout *lay = new QHBoxLayout();
        QWidget *widg = new QWidget();
        widg->setLayout(lay);
        ui->scrollArea->setWidget(widg);
        int a = 0;
        lay->setContentsMargins(a,a,a,a);
        lay->setSpacing(2);
        foreach (PosteConnecte *post, listpostsoignant)
        {
            User *usr = Datas::I()->users->getById(post->id());
            QString PosteLog  = post->nomposte().remove(".local");
            PatientEnCours *patencours = Q_NULLPTR;
            foreach (PatientEnCours *pat, *m_listepatientsencours->patientsencours())
                if (pat->iduserencoursexam() == post->id() && pat->posteexamen() == post->nomposte())
                    patencours = pat;
            UpTextEdit *UserBureau;
            UserBureau = new UpTextEdit;
            UserBureau->disconnect();; // pour déconnecter la fonction MenuContextuel intrinsèque de la classe UpTextEdit
            UserBureau->setObjectName(usr->login() + "BureauupTextEdit");
            UserBureau->setIdUser(post->id());
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
            html += "<p class=\"p10\"><b>" + PosteLog + "</b></p><p class=\"p2\"><b><span style=\"color:green;\">" + usr->login() + "</b></p>";
            if (patencours == Q_NULLPTR)
                html += "<p class=\"p2\">ZZzzz...</p>";
            else if (m_patients->getById(patencours->id()) != Q_NULLPTR)
            {
                UserBureau->setId(post->idpatencours());
                if( UserBureau->getIdUser() == m_currentuser->id() )
                    connect(UserBureau, &UpTextEdit::dblclick, this,  [=] {if (m_currentuser->isSecretaire()) ChoixDossier(m_patients->getById(UserBureau->getId()));});
                else
                {
                    connect(UserBureau,         &QWidget::customContextMenuRequested,   [=] {MenuContextuelBureaux(UserBureau);});
                    connect(UserBureau,         &UpTextEdit::dblclick,                  [=] {AutreDossier(m_patients->getById(UserBureau->getId()));});
                }
                html += "<p class=\"p2\">" +  m_patients->getById(patencours->id())->nom() + " " + m_patients->getById(patencours->id())->prenom() + "</p>";      //Nom Prenom
                QString color = "black";
                if (patencours->heurerarrivee().isValid())
                {
                    QTime heureArriv = patencours->heurerarrivee();
                    if (heureArriv.secsTo(QTime::currentTime())/60 < 15)        color = "green";
                    else if (heureArriv.secsTo(QTime::currentTime())/60 < 30)   color = "orange";
                    else                                                        color ="red";
                }
                html += "<p class=\"p2\"><span style=\"color:" + color + ";\">" +  patencours->heurerarrivee().toString("HH:mm") + "</span></p>";                                      //heure arrivée
            }
            else
                html += "<p class=\"p2\">ZZzzz...</p>";
            html += "</body></html>";
            UserBureau->setHtml(html);
            lay->addWidget(UserBureau);
        }
    }


    // ACCUEIL ----------------------------------------------------------------------------------------------------------
    TableAMettreAJour = ui->AccueilupTableWidget;
    TableAMettreAJour->clearContents();
    QList<PatientEnCours*> listpatvus;
    foreach (PatientEnCours* pat, listpat)
    {
        //qDebug() << m_patients->getById(pat->id())->nom() + " " + m_patients->getById(pat->id())->prenom() + " " + pat->statut();
        if (pat->statut() == RETOURACCUEIL)
            listpatvus << pat;
    }
    TableAMettreAJour   ->setRowCount(listpatvus.size());
    if (m_listeparentsmodel == Q_NULLPTR)
        m_listeparentsmodel = new QStandardItemModel();
    else
        m_listeparentsmodel->clear();
    QStandardItem       *oitem0, *oitem1;
    QList<int>          listidparents;
    foreach (PatientEnCours *patencours, listpatvus)
    {
        Patient *pat                = m_patients->getById(patencours->id());
        Acte* actapayer             = Datas::I()->actes->getById(patencours->idacteapayer());
        QMap<QString, QVariant> rsgnmt;
        rsgnmt["idpat"]             = patencours->id();
        rsgnmt["motif"]             = patencours->motif();
        rsgnmt["ddnpat"]            = pat->datedenaissance();
        rsgnmt["idsuperviseur"]     = patencours->iduser();
        rsgnmt["loginsuperviseur"]  = Datas::I()->users->getById(patencours->iduser())->login();
        rsgnmt["urgence"]           = (patencours->motif() == "URG");
        rsgnmt["message"]           = patencours->message();
        rsgnmt["idComptable"]       = actapayer->idComptable();

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

        label0->setId(patencours->id());                      // idPat
        label1->setId(patencours->id());                      // idPat
        label2->setId(patencours->id());                      // idPat
        label3->setId(patencours->id());                      // idPat
        label4->setId(patencours->id());                      // idPat
        label5->setId(patencours->id());                      // idPat

        label0->setContextMenuPolicy(Qt::CustomContextMenu);
        label1->setContextMenuPolicy(Qt::CustomContextMenu);
        label2->setContextMenuPolicy(Qt::CustomContextMenu);
        label3->setContextMenuPolicy(Qt::CustomContextMenu);
        label4->setContextMenuPolicy(Qt::CustomContextMenu);
        label5->setContextMenuPolicy(Qt::CustomContextMenu);

        NomPrenom = pat->nom().toUpper() + " " + pat->prenom();
        zw = actapayer->heure().toString("HH:mm");
        int idparent = actapayer->idParent();
        label0->setText(" " + zw);                                                              // Heure acte
        label1->setText(" " + NomPrenom);                                                       // Nom + Prénom
        QString Soignant  = Datas::I()->users->getById(patencours->iduser())->login();
        if (patencours->iduser() != idparent)
            Soignant +=  " / " + Datas::I()->users->getById(idparent)->login();
        label2->setText(" " + Datas::I()->users->getById(patencours->iduser())->login());    // Soignant
        label3->setText(" " + actapayer->cotation());                                           // Cotation
        label4->setText(QLocale().toString(actapayer->montant(),'f',2) + " ");                  // Montant
        label5->setText(QString::number(idparent));                      // Parent
        QString typpaiement = "";
        if (actapayer->montant() == 0.0)
            typpaiement = "Gratuit";
        label4->setAlignment(Qt::AlignRight);
        if (patencours->message()!="")
        {
            QString color = "color:green";
            label0->setStyleSheet(color);
            label1->setStyleSheet(color);
            label2->setStyleSheet(color);
            label3->setStyleSheet(color);
            label4->setStyleSheet(color);
            label5->setStyleSheet(color);
        }
        if (!listidparents.contains(idparent))
        {
            listidparents           << idparent;
            oitem0                  = new QStandardItem(QString::number(idparent));
            oitem1                  = new QStandardItem(Datas::I()->users->getById(idparent)->login());
            QList<QStandardItem*>   listitems;
            listitems               << oitem0 << oitem1;
            m_listeparentsmodel     ->appendRow(listitems);
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
        pItem               ->setText(QString::number(patencours->idacteapayer()));
        TableAMettreAJour   ->setItem(i,5,pItem);
        TableAMettreAJour   ->setCellWidget(i,6,label5);
        TableAMettreAJour   ->setRowHeight(i,int(fm.height()*1.1));
    }
    while (gAccueilTab->count()>0)
        gAccueilTab->removeTab(0);
    if (m_listeparentsmodel->rowCount()==0)
        gAccueilTab->setVisible(false);
    else
    {
        gAccueilTab->setVisible(true);
        for (int i=0; i<m_listeparentsmodel->rowCount(); i++)
        {
            gAccueilTab  ->insertTab(i,m_listeparentsmodel->item(i,1)->text());
            gAccueilTab  ->setTabData(i, m_listeparentsmodel->item(i,0)->text());
        }
        if (ui->AccueilupTableWidget->selectedRanges().size()>0)
        {
            bool a = false;
            for (int i=0; i<gAccueilTab->count(); i++)
                if (gAccueilTab->tabData(i).toInt() == m_currentuser->idparent())
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
    gResumeStatut = proc->getSessionStatus() + "\n\n";

    // les socket
    if (UtiliseTCP)
    {
        for (auto it = gListSockets.begin(); it != gListSockets.end(); ++it)
        {
            QString statcp = *it;
            QString sep = "{!!}";
            statcp.replace(TCPMSG_Separator, sep); // juste pour que ce soit plus compréhensible en cas d'utilisation de qDebug()
            //qDebug() << statcp;
            if (it == gListSockets.begin())
            {
                // le 1er item de gListSockets est le serveur
                gResumeStatut += tr("ServeurTCP") + "\n\t";
                if (statcp.split(sep).size()>3)
                {
                    gResumeStatut += statcp.split(sep).at(2) + " - "
                            + statcp.split(sep).at(0) + " - "
                            + statcp.split(sep).at(1) + " --- "
                            + Datas::I()->users->getLoginById(statcp.split(sep).at(3).toInt());
                }
                else
                    gResumeStatut += tr("inconnu");
                gResumeStatut += "\n" + tr("Postes connectés") + "\n";
            }
            else
            {
                if (statcp.split(sep).size()>3)
                {
                    gResumeStatut += "\t" + statcp.split(sep).at(2) + " - "
                            + statcp.split(sep).at(0) + " - "
                            + statcp.split(sep).at(1) + " --- "
                            + Datas::I()->users->getLoginById(statcp.split(sep).at(3).toInt()) + "\n";
                }
                else
                    gResumeStatut += "\t" + tr("inconnu");
            }
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
    if (QString::number(m_parametres->versionbase()) == "")
        gResumeStatut += tr("inconnue");
    else
        gResumeStatut +=  QString::number(m_parametres->versionbase());
    proc->emit ModifEdit(gResumeStatut);
}

/*-----------------------------------------------------------------------------------------------------------------
-- Salle d'attente ------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
bool Rufus::RetourSalleDattente(QString Titre)
{
    bool retour         = false;

    Dlg_SalDat           = new dlg_salledattente(m_currentact, Titre);
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
void Rufus::SupprimerActe(Acte *act)
{
    UpMessageBox    msgbox;
    int             idAAficher;

    // On vérifie que cet acte n'a pas été payé par une opération bancaire déjà enregistrée sur le compte
    // on récupère les lignes de paiement
    QString Messg ="";

    QList<LignePaiement*> listlignespaiement;
    foreach (LignePaiement* lign, m_lignespaiements->lignespaiements()->values())
    {
        if (lign->idacte() == act->id())
            listlignespaiement << lign;
    }
    if (listlignespaiement.size() > 0)
    {
        // on vérifie pour chaque ligne s'il s'agit d'un virement, d'une carte bleue ou d'un chèque enregistré
        QString critere;
        for (int i=0; i<listlignespaiement.size(); ++i)
        {
            critere += QString::number(listlignespaiement.at(i)->idrecette());
            if (i<listlignespaiement.size()-1)
                critere += ",";
        }
        QString req = "SELECT ModePaiement, NomTiers, idRemise FROM " TBL_RECETTES " WHERE idRecette in (" + critere + ")";
        QList<QVariantList> pmtlist = db->StandardSelectSQL(req,ok);
        if(ok && pmtlist.size()>0)
            for (int j=0; j<pmtlist.size(); j++)
            {
                if (pmtlist.at(j).at(0).toString() == "V")                                     Messg = tr("Je crains de ne pas pouvoir supprimer cet acte\nIl y a des versements enregistrés.");
                if (pmtlist.at(j).at(0).toString() == "V" && pmtlist.at(j).at(1).toString() == "CB") Messg = tr("Je crains de ne pas pouvoir supprimer cet acte\nIl y a des paiements par carte de crédit enregistrés.");
                if (pmtlist.at(j).at(0).toString() == "C" && pmtlist.at(j).at(2).toInt() > 0)        Messg = tr("Je crains de ne pas pouvoir supprimer cet acte\nIl y a des paiements par chèque enregistrés.");
                if (pmtlist.at(j).at(0).toString() == "C" && pmtlist.at(j).at(2).toInt() == 0)       Messg = tr("Je crains de ne pas pouvoir supprimer cet acte\nIl y a des paiements par chèque enregistrés."
                                                                                                                "\nCe ou ces chèques ne sont pas encaissés."
                                                                                                                "\nVous devez modifier l'écriture correspondante pour pouvoir supprimer l'acte.");
                if (Messg != "")
                    j = pmtlist.size();
            }
    }

    if (Messg != "")
    {
        UpMessageBox::Watch(this, "Euuhh... " + m_currentuser->login(), Messg);
        return;
    }

    // On demande confirmation de la suppression de l'acte
    msgbox.setText("Euuhh... " + m_currentuser->login());
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
    idAAficher = 0;
    Patient * pat = m_patients->getById(act->idPatient());

    if (m_listeactes->actes()->size() == 0)
    {
        UpMessageBox::Watch(this, tr("Impossible de retrouver les données de l'acte"));
        return;
    }
    Acte *nouvact = Q_NULLPTR;
    if (m_listeactes->actes()->size() > 1 )
    {
        int a;
        a = m_listeactes->actes()->keys().indexOf(act->id());
        if (a < m_listeactes->actes()->size() - 1)
            ++a;    //on est au milieu des actes -> on va rechercher l'idActe suivant
        else
            --a;    //on est sur le dernier acte -> on va rechercher l'idActe précédent
        nouvact = m_listeactes->actes()->values().at(a);
    }

    // on supprime les éventuelles réfractions liées à cette consultation -----------------------------------------------------------
    foreach (Refraction* ref, Datas::I()->refractions->refractions()->values())
        if (ref->idacte() == act->id())
            Datas::I()->refractions->SupprimeRefraction(Datas::I()->refractions->getById(ref->id()));

    // on supprime les éventuels bilans orthoptiques liés à cette consultation -----------------------------------------------------------
    QString req = "DELETE FROM " TBL_BILANORTHO " WHERE idBilanOrtho  = " + QString::number(act->id());
    db->StandardSQL(req);

    /* on corrige la compta
    */
    if (listlignespaiement.size()>0)     // inutile de le faire pour les gratuits et les impayés ou les tiers non encore encaissés
        /* on corrige les lignes de recette correspondant à ce dossier -------------------------------------------------------------------------
        // ça ne peut concerner que des paiements en espèces -----------------------------------------------------------------------------------
        parce qu'on ne peut pas supprimer les dossiers pour lesquels des recettes ont été enregistrées avec d'autres formes de paiement
        (chèque ou virement)
        */
    {
        for (int j=0; j<listlignespaiement.size(); j++)
        {
            req = "SELECT Montant FROM " TBL_RECETTES " WHERE idRecette = " + QString::number(listlignespaiement.at(j)->idrecette());
            QList<QVariantList> mntlist = db->StandardSelectSQL(req,ok);
            for (int i=0; i<mntlist.size(); ++i)
            {
                QString req = "delete from " TBL_RECETTES " where idrecette = " + QString::number(listlignespaiement.at(j)->idrecette());
                if (mntlist.at(i).at(0).toDouble() > listlignespaiement.at(j)->paye())
                    req = "update " TBL_RECETTES " set Montant = " + QString::number(mntlist.at(i).at(0).toDouble() - listlignespaiement.at(j)->paye()) +
                          " where idRecette = " + QString::number(listlignespaiement.at(j)->idrecette());
                db->StandardSQL(req);
            }
        }

        // On actualise la table des lignes de paiement et la table des Types de paiement
        m_lignespaiements->SupprimeActeLignesPaiements(act);
        req = "DELETE FROM " TBL_TYPEPAIEMENTACTES " WHERE idActe = " + QString::number(act->id());
        db->StandardSQL(req);
    }

    // on supprime l'acte -------------------------------------------------------------------------------------------------
    m_listeactes->SupprimeActe(act);

    // On affiche la nouvelle consultation
    AfficheActe(nouvact);

    // On met à jour l'affichage éventuel de dlg_actesprecedents
    QList<dlg_actesprecedents *> listactesprecs = findChildren<dlg_actesprecedents *>();
    for (int i = 0; i<listactesprecs.size();i++)
        if (listactesprecs.at(i)->getidPatient() != pat->id())
            listactesprecs.at(i)->close();
    if (findChildren<dlg_actesprecedents *>().size()>0)
    {
        if (m_listeactes->actes()->size() < 2)
            findChildren<dlg_actesprecedents *>().at(0)->close();
        else
            findChildren<dlg_actesprecedents *>().at(0)->Actualise();
    }
}

/*-----------------------------------------------------------------------------------------------------------------
-- Supprimer un dossier--------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void Rufus::SupprimerDossier(Patient *pat)
{
    // On ne peut supprimer un dossier que s'il n'y a pas eu d'actes avec des tiers payants, des chèques ou des cartes de crédit enregistrés pour ce dossier.
    UpMessageBox     msgbox, msgbox2;

    //1. On recherche les actes de ce dossier qui seraient en tiers payant et qui auraient déjà reçu des versements auquel cas, on ne peut pas supprimer les actes ni le dossier
    QString Messg = "";
    foreach (LignePaiement* lign, m_lignespaiements->lignespaiements()->values())
    {
        // on vérifie pour chaque ligne s'il s'agit d'un virement ou d'une carte bleue ou d'un chèque enregistré
        QString requete = "SELECT ModePaiement, NomTiers, idRemise FROM " TBL_RECETTES " WHERE idRecette = " + QString::number(lign->idrecette());
        QVariantList pmydata = db->getFirstRecordFromStandardSelectSQL(requete,ok);
        if (!ok)
            return;
        if (pmydata.at(0).toString() == "V")
            Messg = tr("Je crains de ne pas pouvoir supprimer ce dossier\nIl y a des versements enregistrés.");
        if (pmydata.at(0).toString() == "V" && pmydata.at(1).toString() == "CB")
            Messg = tr("Je crains de ne pas pouvoir supprimer ce dossier\nIl y a des paiements par carte de crédit enregistrés.");
        if (pmydata.at(0).toString() == "C" && pmydata.at(2).toInt() > 0)
            Messg = tr("Je crains de ne pas pouvoir supprimer cet acte\nIl y a des paiements par chèque enregistrés.");
        if (pmydata.at(0).toString() == "C" && pmydata.at(2).toInt() == 0)
            Messg = tr("Je crains de ne pas pouvoir supprimer ce dossier\nIl y a des paiements par chèque enregistrés."
                    "\nCe ou ces chèques ne sont pas encaissés."
                    "\nVous devez modifier l'écriture correspondante pour pouvoir supprimer l'acte.");
        if (Messg != "") break;
    }
    if (Messg != "")
    {
        UpMessageBox::Watch(this, "Euuhh... " + m_currentuser->login(), Messg);
        return;
    }

    //2. On commence par demander la confirmation de la suppression du dossier
    msgbox.setText("Euuhh... " + m_currentuser->login());
    msgbox.setInformativeText(tr("Etes vous sûr de vouloir supprimer le dossier de ") + pat->nom() + " " + pat->prenom() + "?");
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
    if (m_lignespaiements->lignespaiements()->size()>0)     // inutile de le faire pour les gratuits et les impayés ou les tiers non encore encaissés
        /* on corrige les lignes de recette correspondant à ce dossier -------------------------------------------------------------------------
        // ça ne peut concerner que des paiements en espèces -----------------------------------------------------------------------------------
        parce qu'on ne peut pas supprimer les dossiers pour lesquels des recettes ont été enregistrées avec d'autres formes de paiement
        (chèque ou virement)
        */
    {
        for (int j=0; j < m_lignespaiements->lignespaiements()->size(); j++)
        {
            int idrecetteACorriger = m_lignespaiements->lignespaiements()->values().at(j)->idrecette();
            QString requete = "SELECT Montant FROM " TBL_RECETTES " WHERE idRecette = " + QString::number(idrecetteACorriger);
            QList<QVariantList> reclist = db->StandardSelectSQL(requete,ok);
            if (!ok)
                return;
            else if (ok && reclist.size()>0)
            {
                for (int k=0; k<reclist.size(); k++)
                {
                    QString req = "delete from " TBL_RECETTES " where idrecette = " + QString::number(idrecetteACorriger);
                    if (reclist.at(k).at(0).toDouble() > m_lignespaiements->lignespaiements()->values().at(j)->paye())
                        req = "update " TBL_RECETTES " set Montant = " + QString::number(reclist.at(k).at(0).toDouble() - m_lignespaiements->lignespaiements()->values().at(j)->paye()) +
                                " where idRecette = " + QString::number(idrecetteACorriger);
                    db->StandardSQL(req);
                }
            }
        }
        //4, On actualise la table des lignes de paiement el la table des Type de paieement
        QList<Acte*> listactes;
        foreach (Acte* act, m_listeactes->actes()->values())
        {
            m_lignespaiements->SupprimeActeLignesPaiements(act);
            listactes << act;
        }
        QString critere;
        for (int i=0; i<listactes.size(); ++i)
        {
            critere += QString::number(listactes.at(i)->id());
            if (i<listactes.size()-1)
                critere += ",";
        }
        QString requete = "DELETE FROM " TBL_TYPEPAIEMENTACTES " WHERE idActe in (" + critere + ")";
        db->StandardSQL(requete);
    }

    //!. Fermeture de l'onglet dossier
    ui->tabWidget->removeTab(ui->tabWidget->indexOf(ui->tabDossier));
    FermeDlgActesPrecedentsEtDocsExternes();

    //!. Suppression du dossier, reconstruction de la liste et du treeView
    m_listepatientsencours->SupprimePatientEnCours(m_listepatientsencours->getById(pat->id()));
    m_patients->SupprimePatient(pat);
    m_listeactes->clearAll(m_listeactes->actes());
    m_lignespaiements->clearAll(m_lignespaiements->lignespaiements());
    FiltreTable(ui->CreerNomlineEdit->text(), ui->CreerPrenomlineEdit->text());
    Flags::I()->MAJFlagSalleDAttente();
    ModeSelectDepuisListe();

    //!. Fermeture de la fiche dlg_actesprecedents
    FermeDlgActesPrecedentsEtDocsExternes();
}

/*-----------------------------------------------------------------------------------------------------------------
-- Tonometrie -----------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void Rufus::Tonometrie()
{
    if (ui->tabWidget->currentIndex() != ui->tabWidget->indexOf(ui->tabDossier)) return;

    Dlg_AutresMes           = new dlg_autresmesures(dlg_autresmesures::TONO);
    QString TOD, TOG, Methode, TODcolor, TOGcolor;
    Dlg_AutresMes->setWindowTitle(tr("Tonométrie - ") + Datas::I()->patients->currentpatient()->nom() + " " + Datas::I()->patients->currentpatient()->prenom());

    if (Dlg_AutresMes->exec()> 0)
    {
        WidgTono *widgtono = dynamic_cast<WidgTono*>(Dlg_AutresMes->Widget());
        if (widgtono == Q_NULLPTR)
            return;
        TOD = widgtono->ui->TOODSpinBox->text();
        TOG = widgtono->ui->TOOGSpinBox->text();
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
            if (widgtono->ui->AirRadioButton->isChecked())        Methode = tr("Air");
            if (widgtono->ui->AutreRadioButton->isChecked())      Methode = tr("Autre");
            if (widgtono->ui->AplanationRadioButton->isChecked()) Methode = tr("Aplanation");

            QString Tono;
            if (TOD.toInt() == 0 && TOG.toInt() > 0)
                Tono = "<td width=\"60\"><font color = \"" COULEUR_TITRES "\"><b>" + tr("TOG:") + "</b></font></td><td width=\"80\">" + TOGcolor + " à " + QTime::currentTime().toString("H") + "H</td><td width=\"80\">(" + Methode + ")</td><td>" + m_currentuser->login() + "</td>";
            if (TOG.toInt() == 0 && TOD.toInt() > 0)
                Tono = "<td width=\"60\"><font color = \"" COULEUR_TITRES "\"><b>" + tr("TOD:") + "</b></font></td><td width=\"80\">" + TODcolor + " à " + QTime::currentTime().toString("H") + "H</td><td width=\"80\">(" + Methode + ")</td><td>" + m_currentuser->login() + "</td>";
            if (TOD.toInt() > 0 && TOG.toInt() > 0)
            {
                if (TOD.toInt() == TOG.toInt())
                    Tono = "<td width=\"60\"><font color = \"" COULEUR_TITRES "\"><b>" + tr("TODG:") + "</b></font></td><td width=\"80\">" + TODcolor + " à " + QTime::currentTime().toString("H") + "H</td><td width=\"80\">(" + Methode + ")</td><td>" + m_currentuser->login() + "</td>";
                else
                    Tono = "<td width=\"60\"><font color = \"" COULEUR_TITRES "\"><b>" + tr("TO:") +"</b></font></td><td width=\"80\">" + TODcolor + "/" + TOGcolor+ " à " + QTime::currentTime().toString("H") + "H</td><td width=\"80\">(" + Methode + ")</td><td>" + m_currentuser->login() + "</td>";
            }

            QString ARajouterEnText =  "<p style = \"margin-top:0px; margin-bottom:0px;\" >" + Tono  + "</p>"
                    + "<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px;\"></p>";
            ItemsList::update(m_currentact, CP_TEXTE_ACTES, ui->ActeTextetextEdit->appendHtml(ARajouterEnText));
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
void Rufus::FiltreTableparDDN()
{
    m_patients->initListeByDDN(ui->CreerDDNdateEdit->date());
    Remplir_ListePatients_TableView();
}

/*-----------------------------------------------------------------------------------------------------------------
-- Vérifie que le montant facturé pour l'acte en cours n'est pas inférieur à la somme des paiements déjà enregistrés
-----------------------------------------------------------------------------------------------------------------*/
bool Rufus::VerifCoherenceMontantPaiement()
{
    QString NouveauMontant = QLocale().toString(QLocale().toDouble(ui->ActeMontantlineEdit->text()),'f',2);
    if (ui->PayelineEdit->isVisible() && QLocale().toDouble(ui->ActeMontantlineEdit->text()) < QLocale().toDouble(ui->PayelineEdit->text()))
    {
        QSound::play(NOM_ALARME);
        UpMessageBox::Watch(this, tr("Saisie de montant refusée !"),
                             tr("Le montant que vous souhaitez enregistrer") + "\n\t" + NouveauMontant + " " + tr("€") +"\n"
                             + tr("est inférieur à la somme des paiements") + "\n" + tr("déjà enregistrés pour cet acte") + "\n\t" + ui->PayelineEdit->text() + tr("€"));
        return false;
    }
    else
        return true;
}

void Rufus::updateActeData(Acte *act, QString nomchamp, QVariant value)
{
    ItemsList::update(act, nomchamp, value);
    MAJActesPrecs();
}

/*-----------------------------------------------------------------------------------------------------------------
-- Valider et enregistrer le montant de la consultation en cours en cas de modification ---------------------------
-----------------------------------------------------------------------------------------------------------------*/
bool Rufus::ValideActeMontantLineEdit(QString NouveauMontant, QString AncienMontant)
{
    // On vérifie que le montant entré n'est pas inférieur au montant déjà payé et on invalide dans ce cas
    QString req;
    if (!VerifCoherenceMontantPaiement())
    {
        ui->ActeMontantlineEdit->setText(AncienMontant);
        return false;
    }
    else
         ui->ActeMontantlineEdit->setText(NouveauMontant);

    if (QLocale().toDouble(NouveauMontant) > 0)
    {
        // si le montant entré est supérieur à O, on vérifie qu'il n'y a pas d'enregistrement gratuit pour cet acte et on le supprime au cas où, après confirmation
        req = "SELECT idActe, TypePaiement FROM " TBL_TYPEPAIEMENTACTES " WHERE TypePaiement = 'G' AND idActe = " + QString::number(m_currentact->id());
        QVariantList idactdata = db->getFirstRecordFromStandardSelectSQL(req,ok, "ValideActeMontantLineEdit");
        if (!ok)
            return false;
        if (idactdata.size() > 0)
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
                db->StandardSQL("DELETE FROM " TBL_TYPEPAIEMENTACTES " WHERE idActe = " + QString::number(m_currentact->id()));
                AfficheActeCompta(m_currentact);
            }
        }
        else
            ui->ActeMontantlineEdit->setText(NouveauMontant);
    }
    //on modifie la table Actes avec le nouveau montant
    m_listeactes->setMontantCotation(m_currentact, ui->ActeCotationcomboBox->currentText(), QLocale().toDouble(NouveauMontant));
    MAJActesPrecs();

    int idx = ui->ActeCotationcomboBox->currentIndex();
    if (idx>0)
    {
        QStringList listMontantActe = ui->ActeCotationcomboBox->itemData(idx).toStringList();
        double MontantConv, MontantPrat, MontantActe;
        MontantActe = QLocale().toDouble(NouveauMontant);
        MontantConv = listMontantActe.at(0).toDouble();
        MontantPrat = listMontantActe.at(1).toDouble();
        ui->BasculerMontantpushButton->setVisible((fabs(MontantActe)!=fabs(MontantConv))
                                                  || (fabs(MontantActe)!=fabs(MontantPrat))
                                                  || (fabs(MontantConv)!=fabs(MontantPrat)));
        if (ui->BasculerMontantpushButton->isVisible())
        {
            if (fabs(MontantActe)!=fabs(MontantPrat))
                ui->BasculerMontantpushButton->setImmediateToolTip(tr("Revenir au tarif habituellement pratiqué"));
            else
                ui->BasculerMontantpushButton->setImmediateToolTip(tr("Revenir au tarif conventionnel"));
        }
    }
    return true;
}

void Rufus::NouvelleMesureRefraction() //utilisé pour ouvrir la fiche refraction quand un appareil a transmis une mesure
{
    if (findChildren<dlg_refraction*>().size()>0)
        return;
    if (Datas::I()->patients->currentpatient()->id() == 0 || m_currentact == Q_NULLPTR)
        return;
    if (ui->tabWidget->currentIndex() != 1 || !ui->Acteframe->isVisible())
        return;

    Procedures::TypeMesure TypeMesure = proc->TypeMesureRefraction();
    if (TypeMesure == Procedures::Final || TypeMesure == Procedures::Subjectif)
    {
        QString ARajouterEnText= proc->HtmlRefracteur();
        ItemsList::update(m_currentact, CP_TEXTE_ACTES, ui->ActeTextetextEdit->appendHtml(ARajouterEnText));
        ui->ActeTextetextEdit->setFocus();
        ui->ActeTextetextEdit->moveCursor(QTextCursor::End);
        if (!proc->DonneesRefracteurFin().isEmpty() && !proc->DonneesRefracteurSubj().isEmpty())
            proc->InsertRefraction(Datas::I()->patients->currentpatient()->id(), m_currentact->id(), Procedures::Subjectif);
        RefractionMesure();
    }
    else if (TypeMesure == Procedures::Autoref)
        proc->InsertRefraction(Datas::I()->patients->currentpatient()->id(), m_currentact->id(), Procedures::Autoref);
    else if (TypeMesure == Procedures::Fronto)
        proc->InsertRefraction(Datas::I()->patients->currentpatient()->id(), m_currentact->id(), Procedures::Fronto);
    else if (TypeMesure == Procedures::Kerato)
    {
        QString ARajouterEnText= proc->HtmlKerato();
        ItemsList::update(m_currentact, CP_TEXTE_ACTES, ui->ActeTextetextEdit->appendHtml(ARajouterEnText));
        ui->ActeTextetextEdit->setFocus();
        ui->ActeTextetextEdit->moveCursor(QTextCursor::End);
        proc->InsertRefraction(Datas::I()->patients->currentpatient()->id(), m_currentact->id(), Procedures::Kerato);
    }
    else if (TypeMesure == Procedures::Tono)
    {
        QString ARajouterEnText= proc->HtmlTono();
        ItemsList::update(m_currentact, CP_TEXTE_ACTES, ui->ActeTextetextEdit->appendHtml(ARajouterEnText));
        ui->ActeTextetextEdit->setFocus();
        ui->ActeTextetextEdit->moveCursor(QTextCursor::End);
    }
    else if (TypeMesure == Procedures::Pachy)
    {
        QString ARajouterEnText= proc->HtmlPachy();
        ItemsList::update(m_currentact, CP_TEXTE_ACTES, ui->ActeTextetextEdit->appendHtml(ARajouterEnText));
        ui->ActeTextetextEdit->setFocus();
        ui->ActeTextetextEdit->moveCursor(QTextCursor::End);
    }
    else
    {
        proc->setTypeMesureRefraction();
        RefractionMesure();
    }
}



// CZ001 début interface avec Pyxvital
/*-----------------------------------------------------------------------------------------------------------------
    Lire la CPS avec Pyxvital : Retour = fichier Particien.par
-----------------------------------------------------------------------------------------------------------------*/
void Rufus::LireLaCPS()
{
    QString req, numPS;
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
    req =   "SELECT idUser FROM " TBL_UTILISATEURS " WHERE UserNumPS = '" + numPS + "'" ;
    QVariantList idusrdata = db->getFirstRecordFromStandardSelectSQL(req,ok, tr("Impossible d'ouvrir la table Utilisateurs"));
    if (!ok)
        return;
    else
        {
        if (idusrdata.size() == 0)  // Aucune mesure trouvee pour ces criteres
            {
            UpMessageBox::Watch(this,"Lecture de la CPS", "Lecture de la carte :\n" +
                                 settingPraticienPar.value("PS/Nom").toString() + " " +
                                 settingPraticienPar.value("PS/Prénom").toString() + "\n" +
                                 settingPraticienPar.value("PS/Spécialité").toString() + "\n" +
                                 "Aucun utilisateur avec le n° de PS : " + numPS);
            return;
            }
        // A REVOIR : faire tout ce qu'il faut pour nouveau user ... mais quoi ???
        m_currentuser->setid(idusrdata.at(0).toInt());
        setWindowTitle("Rufus - " + m_currentuser->login() + " - " + m_currentuser->fonction());
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
                      " FROM "  TBL_PATIENTS
            " WHERE UPPER(PatNom) LIKE '" + nomPat.toUpper() + "%'" +
            " AND   UPPER(PatPrenom) LIKE '" + prenomPat.toUpper() + "%'" +
            " AND   PatDDN = '" + zdat + "'";
    FiltreTable(nomPat.toUpper(), prenomPat.toUpper());
    if (m_patients->patients()->size() == 0)       // aucun patient trouvé
        {
        // si rien trouvé, deuxième recherche sur date de naissance seule
        requete = "SELECT IdPat, PatNom, PatPrenom, PatDDN, Sexe  "
                  " FROM "  TBL_PATIENTS
                  " WHERE PatDDN = '" + zdat + "'";
        FiltreTable();
        ModeCreationDossier();
        ui->CreerNomlineEdit->setText(nomPat);
        ui->CreerPrenomlineEdit->setText(prenomPat);
        ui->CreerDDNdateEdit->setDate(QDate::fromString(zdat, "yyyy-MM-dd"));
        QString NNIPat     = settingPatientPar.value("Bénéficiaire/Numéro").toString();
        QString Sexe = (NNIPat.left(1) == "1"? "M" : "F");

        // aucun patient trouvé
        // si rien non plus on propose la création du dossier.
        // A REVOIR
        // on a trouvé des patients avec la même date de naissance et on les a affichés.
        // A REVOIR
        }
    if (m_listepatientsmodel->rowCount()>0)
        RecaleTableView(getPatientFromRow(0) ,QAbstractItemView::PositionAtTop);
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

void Rufus::TesteConnexion()
{
    envoieMessage(TCPMSG_TestConnexion);
}


void Rufus::TraiteTCPMessage(QString msg)
{
    //qDebug() << msg + " - sur rufus::traitetcpmessage()";
    if (msg == TCPMSG_MAJSalAttente)
        Remplir_SalDat();                       // par le TCPSocket
    else if (msg == TCPMSG_MAJCorrespondants)
    {
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
        if (m_currentuser->isSoignant())
            if (Datas::I()->patients->currentpatient()->id() == msg.toInt())
                MAJDocsExternes();                  // depuis le tcpsocket
    }
    else if (msg.contains(TCPMSG_MAJPatient))
    {
        /* le message a le format suivant id du patient à mettre à jour + TCPMSG_MAJPatient) */
        msg.remove(TCPMSG_MAJPatient);
        if (Datas::I()->patients->patients()->find(msg.toInt()) != Datas::I()->patients->patients()->end())
        {
            Patient* pat = Datas::I()->patients->getById(msg.toInt(), Item::LoadDetails);
            if (Datas::I()->patients->patientstable()->find(msg.toInt()) != Datas::I()->patients->patientstable()->end())
            {
                int row = m_listepatientsmodel->findItems(msg).at(0)->row();
                m_listepatientsmodel->item(row,0)->setText(QString::number(pat->id()));                                   // id                           -> utilisé pour le drop event
                m_listepatientsmodel->item(row,1)->setText(pat->nom().toUpper() + " " + pat->prenom());                   // Nom + Prénom
                m_listepatientsmodel->item(row,2)->setText(pat->datedenaissance().toString(tr("dd-MM-yyyy")));            // date de naissance
                m_listepatientsmodel->item(row,3)->setText(pat->datedenaissance().toString(tr("yyyyMMdd")));              // date de naissance inversée   -> utilisé pour le tri
                m_listepatientsmodel->item(row,4)->setText(pat->nom());                                                   // Nom                          -> utilisé pour le tri
                m_listepatientsmodel->item(row,5)->setText(pat->prenom());                                                // Prénom                       -> utilisé pour le tri
            }
            if (Datas::I()->patients->patientssaldat()->find(msg.toInt()) != Datas::I()->patients->patientssaldat()->end())
            {
                Remplir_SalDat();
            }
        }
        if (Datas::I()->patients->currentpatient()->id() == msg.toInt())
        {
            Datas::I()->patients->setcurrentpatient(msg.toInt());
            ui->IdentPatienttextEdit->setHtml(CalcHtmlIdentificationPatient(Datas::I()->patients->currentpatient()));
        }
        if (Datas::I()->patients->dossierpatientaouvrir()->id() == msg.toInt())
        {
            Datas::I()->patients->setdossierpatientaouvrir(msg.toInt());
        }
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
        Datas::I()->postesconnectes->initListe();
        Remplir_SalDat();
    }
}

void Rufus::envoieMessage(QString msg)
{
    if (!UtiliseTCP)
        return;
    //qDebug() << msg + " - void Rufus::envoieMessage(QString msg)";
    TcPConnect->envoieMessage(msg);
}

void Rufus::envoieMessageA(QList<int> listidusr)
{
    Flags::I()->MAJflagMessages();
    if (!UtiliseTCP)
        return;
    QString listid;
    for (int i=0; i<listidusr.size(); i++)
    {
        listid += QString::number(listidusr.at(i));
        if (listidusr.at(i) < (listidusr.size()-1))
            listid += ",";
    }
    QString msg = listid + TCPMSG_Separator + "1" + TCPMSG_MsgBAL;
    TcPConnect->envoieMessage(msg);
}

