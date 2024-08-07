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

#include "rufus.h"
#include "ui_rufus.h"

Rufus::Rufus(QWidget *parent) : QMainWindow(parent)
{
    //! la version du programme correspond à la date de publication, suivie de "/" puis d'un sous-n° - p.e. "23-6-2017/3"
    //! la date doit impérativement être composée au format "00-00-0000" / n°version
    qApp->setApplicationVersion("07-08-2024/1");
    ui = new Ui::Rufus;
    ui->setupUi(this);
    setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint);

    srand(static_cast<uint>(time(Q_NULLPTR)));
    qApp->setStyleSheet(Styles::StyleAppli());
    QToolTip::setPalette(QPalette(Qt::yellow));


    //! 0. Choix du mode de connexion au serveur, connexion à la base et récupération des données utilisateur
    /*! récupération des différents modes d'accès paramétrés dans le fichier ini */
    switch (proc->ListeModesAcces().size()) {
    case 0:        /*! si aucun mode d'accès n'est paramétré, sortie du programme */
        UpMessageBox::Watch(this, tr("Erreur fichier Rufus.ini"), tr("Aucun paramétrage valide de connexion retrouvé") + "\n" + tr("Le fichier rufus.ini est endommagé et doit être réparé"));
        exit(0);
    case 1:        /*! si un seul mode d'accès est paramétré, on passe directement à la fiche de connexion */
        db->setModeacces(proc->ListeModesAcces().at(0));
        if (!proc->Connexion_A_La_Base())
            exit(0);
        break;
    default:        /*! si plusieurs modes d'accès sont paramétrés, on propose le choix du mode de connexion */
        bool a = false;
        while (!a)
        {
            if (!proc->FicheChoixConnexion())
                exit(0);
            a = proc->Connexion_A_La_Base();
        }
    }
    m_parametres = db->parametres();
    RecalcCurrentDateTime();

    //! 1 - Restauration de la position de la fenetre et de la police d'écran
    if (proc->settings()->value("PositionsFiches/Rufus") != QVariant())
    {
        restoreGeometry(proc->settings()->value("PositionsFiches/Rufus").toByteArray());
        proc->settings()->remove("PositionsFiches/Rufus");
    }
    else
        restoreGeometry(proc->settings()->value(Position_Fiche Nom_fiche_Rufus).toByteArray());
    setWindowIcon(Icons::icSunglasses());

    //! 2 - charge les data du user connecté
    if (currentuser() == Q_NULLPTR)
    {
        UpMessageBox::Watch(this, tr("Pas d'utilisateur identifié!\nSortie du programme"));
        exit(0);
    }
    ShowMessage::I()->SplashMessage(proc->currentuserstatus() + "\n" + tr("Site") + "\t\t= " + Datas::I()->sites->currentsite()->nom(), 3000);

    //! 3 Initialisation de tout
    InitVariables();
    InitWidgets();
    InitEventFilters();
    CreerMenu();
    InitMenus();

    //! 4 reconstruction des combobox des correspondants et de la liste des documents
    ReconstruitCombosCorresp();                 //! initialisation de la liste

    FiltreTable();                              //! InitTables()
    PosteConnecte* postadmin = Datas::I()->postesconnectes->admin();
    MAJPosteConnecte();
    if (postadmin == Q_NULLPTR)
        VerifVerrouDossier();
    if (Datas::I()->postesconnectes->postesconnectes()->size()==1)
        Flags::I()->videFlags();

    //! 5 - lancement du TCP
    m_utiliseTCP = false;
    Flags::I()->cleanFlags();
    QString log;
    qintptr z = 0;
    if (postadmin != Q_NULLPTR)
    {
        log = tr("RufusAdmin présent");
        Logs::LogSktMessage(log);
        if (postadmin->ipadress() == "")
        {
            log = tr("Aucun serveur TCP enregistré dans la base");
            Logs::LogSktMessage(log);
            UpSystemTrayIcon::I()->showMessage(tr("Messages"), log, Icons::icSunglasses(), 3000);
        }
        else
        {
            Utils::Pause(100);
            TcPConnect = TcpSocket::I();
            m_utiliseTCP = TcPConnect->TcpConnectToServer(postadmin->ipadress());
            if (m_utiliseTCP)
            {
                ShowMessage::I()->PriorityMessage(tr("Connexion TCP OK"), z, 3000);
                connect(TcPConnect, &TcpSocket::receiveTCPmsg, this, &Rufus::TraiteTCPMessage);  // traitement des messages reçus
                // envoie le stringid du poste qui vient de se connecter
                QString msg = currentpost()->stringid() + TCPMSG_StringidPoste;
                // qDebug() << msg;
                envoieTCPMessage(msg);
            }
            else {
                log = tr("RufusAdmin présent mais échec connexion");
                Logs::LogSktMessage(log);
            }
        }
    }
    else if (DataBase::I()->ModeAccesDataBase() != Utils::Distant)
    {
        log = tr("RufusAdmin absent");
        Logs::LogSktMessage(log);
    }
    else
    {
        log = tr("Connexion distante - pas d'utilisation de TCP");
        Flags::I()->MAJflagUserDistant();
        Logs::LogSktMessage(log);
    }

    //! 6 - mettre en place le TcpSocket et/ou les timer
    gTimerPatientsVus            = new QTimer(this);     // effacement automatique de la liste des patients vus - réglé à 20"
    t_timerSalDat                = new QTimer(this);     /* scrutation des modifs de la salle d'attente */                  //! utilisé en cas de non utilisation des tcpsocket (pas de rufusadmin ou poste distant)
    t_timerCorrespondants        = new QTimer(this);     /* scrutation des modifs de la liste des correspondants */         //! utilisé en cas de non utilisation des tcpsocket (pas de rufusadmin ou poste distant)
    t_timerVerifMessages         = new QTimer(this);     /* scrutation des nouveaux messages */                             //! utilisé en cas de non utilisation des tcpsocket (pas de rufusadmin ou poste distant)
    t_timerPosteConnecte         = new QTimer(this);     // mise à jour de la connexion à la base de données
    t_timerVerifImportateurDocs  = new QTimer(this);     /* vérifie que le poste importateur des documents externes est toujours là */
    t_timerExportDocs            = new QTimer(this);     /* utilisé par le poste importateur pour vérifier s'il y a des documents à sortir de la base */            //! pas utilisé si rufusadmin est  utilisé
    t_timerActualiseDocsExternes = new QTimer(this);     // actualise l'affichage des documents externes si un dossier est ouvert
    t_timerVerifVerrou           = new QTimer(this);     /* utilisé en  l'absence de TCPServer pour vérifier l'absence d'utilisateurs déconnectés dans la base*/    //! pas utilisé si rufusadmin est  utilisé
    t_timerSupprDocs             = new QTimer(this);     /* utilisé par le poste importateur pour vérifier s'il y a des documents à supprimer */

    gTimerPatientsVus   ->setSingleShot(true);           // il est singleshot et n'est démarré que quand on affiche la liste des patients vus
    gTimerPatientsVus   ->setInterval(20000);
    // Lancement des timers de gestion des documents
    t_timerVerifImportateurDocs         ->start(60000);
    // Lancement du timer de vérification des verrous - +++ à lancer après le timer gTimerVerifImportateurDocs puisqu'il l'utilise
    t_timerVerifVerrou                  ->start(60000);// "toutes les 60 secondes"


    if (db->ModeAccesDataBase() == Utils::Distant)
    {
        t_timerSalDat                   ->start(10000);
        t_timerCorrespondants           ->start(60000);
        t_timerActualiseDocsExternes    ->start(60000); // "toutes les 60 secondes"
        t_timerVerifMessages            ->start(60000); // "toutes les 60 secondes"
    }
    else
    {
        t_timerExportDocs               ->start(10000); // "toutes les 10 secondes"
        t_timerSupprDocs                ->start(60000); // "toutes les 60 secondes"
        t_timerVerifMessages            ->start(10000); // "toutes les 10 secondes"
        t_timerActualiseDocsExternes    ->start(5000);  // "toutes les 5 secondes"
        if (!m_utiliseTCP)
        {
            t_timerSalDat               ->start(1000);
            t_timerCorrespondants       ->start(30000);
        }
    }

    t_timerPosteConnecte->start(10000);// "toutes les 10 secondes - remet à jour le statut connecté du poste dans la base - tables utilisateursconnectes"

    if (!m_utiliseTCP)  //! pas de rufusadmin ou accès distant
    {
        m_flagcorrespondants    = Flags::I()->flagCorrespondants();
        m_flagsalledattente     = Flags::I()->flagSalleDAttente();
        m_flagmessages          = Flags::I()->flagMessages();
        connect (t_timerSalDat,                 &QTimer::timeout,  this,   &Rufus::VerifSalleDAttente);
        connect (t_timerCorrespondants,         &QTimer::timeout,  this,   &Rufus::VerifCorrespondants);
        connect (t_timerVerifImportateurDocs,   &QTimer::timeout,  this,   &Rufus::VerifImportateur);
        connect (t_timerVerifVerrou,            &QTimer::timeout,  this,   &Rufus::VerifVerrouDossier);
        connect (t_timerVerifMessages,          &QTimer::timeout,  this,   &Rufus::VerifMessages);
        //connect (t_timerImportDocsExternes,  &QTimer::timeout,  this,   &Rufus::ImportDocsExternes);
        if (db->ModeAccesDataBase() != Utils::Distant)
            connect(t_timerSupprDocs,           &QTimer::timeout,   this,   &Rufus::SupprimerDocsEtFactures);
        VerifImportateur();
    }
    connect (t_timerPosteConnecte,              &QTimer::timeout,   this,   &Rufus::MAJPosteConnecte);
    connect (gTimerPatientsVus,                 &QTimer::timeout,   this,   &Rufus::MasquePatientsVusWidget);
    connect (t_timerActualiseDocsExternes,      &QTimer::timeout,   this,   &Rufus::ActualiseDocsExternes);

    //! 7 - Nettoyage des erreurs éventuelles de la salle d'attente
    bool majsaldat = false;
    Datas::I()->patientsencours->initListeAll();
    // on donne le statut "arrivé" aux patients en salle d'attente dont le iduserencourssexam est l'utilisateur actuel et qui n'auraient pas été supprimés (plantage)
    QString blabla              = ENCOURSEXAMEN;
    int length                  = blabla.size();
    for (auto it = Datas::I()->patientsencours->patientsencours()->constBegin(); it != Datas::I()->patientsencours->patientsencours()->constEnd(); ++it)
    {
        PatientEnCours *pat = const_cast<PatientEnCours*>(it.value());
        if (pat != Q_NULLPTR)
            if (pat->idusersuperviseur() == currentuser()->id() && pat->statut().left(length) == ENCOURSEXAMEN && pat->posteexamen() == Utils::hostName().left(60))
            {
                ItemsList::update(pat, CP_STATUT_SALDAT, ARRIVE);
                ItemsList::update(pat, CP_POSTEEXAMEN_SALDAT);
                ItemsList::update(pat, CP_IDUSERENCOURSEXAM_SALDAT);
                majsaldat = true;
            }
    }
    if (majsaldat)
        Flags::I()->MAJFlagSalleDAttente();

    //! 8 les signaux
    ConnectSignals();

    //! 9 - libération des verrous de la compta
    QString req = " delete from " TBL_VERROUCOMPTAACTES " where " CP_POSEPAR_VERROUCOMPTA " = " + QString::number(currentuser()->id());
    db->StandardSQL(req);
    m_closeflag = false;
    setTitre();

    //!10 - reconstruction du QCompleter de la liste de cotations
    if (currentuser()->isSoignant())
    {
        if (ui->ActeCotationcomboBox->lineEdit()->completer() != Q_NULLPTR)
        {
            ui->ActeCotationcomboBox->lineEdit()->completer()->disconnect();
            delete ui->ActeCotationcomboBox->lineEdit()->completer();
        }
        QCompleter *comp = new QCompleter(QStringList() << tr(GRATUIT) << db->loadTypesCotations());
        comp->setCaseSensitivity(Qt::CaseInsensitive);
        comp->popup()->setFont(ui->ActeMontantlineEdit->font());
        comp->setMaxVisibleItems(5);
        ui->ActeCotationcomboBox->lineEdit()->setCompleter(comp);
        connect(comp, QOverload<const QString &>::of(&QCompleter::activated), this, &Rufus::RetrouveMontantActe);
    }

    //! 11 - Mise à jour des salles d'attente
    Remplir_SalDat();
    if(m_utiliseTCP)
        envoieTCPMessage(TCPMSG_MAJSalAttente);

    //! 12 - Vérification de la messagerie
    ReconstruitListeMessages();

    //! 13 - Affichage des boutons bilan orthoptique et création intervention
    ui->CreerBOpushButton   ->setVisible(currentuser()->isOrthoptist());
    ui->CreerBOpushButton_2 ->setVisible(currentuser()->isOrthoptist());
    ui->CreerInterventionpushButton->setVisible(currentuser()->isMedecin());

    //! 14 - mise à jour du programmateur de sauvegarde
    if (db->ModeAccesDataBase() == Utils::Poste)
        proc->ParamAutoBackup();
    /*! la suite sert à décharger le launchagent du programme de backup sous MacOs, plus utilisé depuis Catalina */
#ifdef Q_OS_MACOS
    if (QFile::exists(PATH_FILE_SCRIPT_MACOS_PLIST))
    {
        QFile::remove(PATH_FILE_SCRIPT_MACOS_PLIST);
        // décharge du launchd
        QString unload  = "bash -c \"/bin/launchctl unload \"" PATH_FILE_SCRIPT_MACOS_PLIST "\"\"";
        QProcess dumpProcess(this);
        dumpProcess.start(unload);
        dumpProcess.waitForFinished();
    }
#endif

    //! 15 - mise à jour du programmateur de l'effacement des fichiers images provisoires - abandonné parce qu'il continue à fonctionner même en cas de plantage du programme
     //if (db->ModeAccesDataBase() == Utils::Poste)
         //proc->ProgrammeSQLVideImagesTemp(m_parametres->heurebkup());
     db->StandardSQL("Use " DB_IMAGES);
     db->StandardSQL("DROP EVENT IF EXISTS VideImagesEchange");
     db->StandardSQL("Use " DB_COMPTA);
     db->StandardSQL("DROP EVENT IF EXISTS VideFactures");

    //! 16 - choix mode (création dossier ou sélection de patient)
    if (m_listepatientsmodel->rowCount() == 0)
    {
        ModeCreationDossier();
        ui->LListepushButton->setEnabled(false);
        UpMessageBox::Watch(this,tr("Vous n'avez aucun dossier de patient enregistré!"), tr("Vous devez d'abord en créer un."));
    }
    else
        ModeSelectDepuisListe();

    //! 17 - suppression des anciens fichiers de log
    Logs::EpureLogs();

    //! 18 - Vérifie si une nouvelle version est disponible
    VerifLastVersion();
    QString nameARK   = proc->settings()->value(Param_Poste_Autoref).toString();
    //nameARK = "HUVITZ HTR-1A";
    if (nameARK == "HUVITZ HTR-1A")
    {
        ShowMessage::I()->PriorityMessage("<font color=\"red\"><b>" + QObject::tr("Problème Autoref Huvitz") + "</b></font><br/>" +
                "<br/>" + tr("Des problémes techniques de collaboration avec la société Essilor") +
                "<br/>" + tr("ne nous permettent plus de maintenir de façon fiable l'implémentation de l'autoref HUVITZ HTR-1A pour le moment") +
                "<br/>" + tr("nous espérons que ce problème indépendant de l'équipe de développement pourra se résoudre rapidement") +
                "<br/>" + tr("nous vous invitons à nous contacter pour avoir plus d'informations"),
                z,8000);
    }
}

Rufus::~Rufus()
{
    delete ui;
}

/*-----------------------------------------------------------------------------------------------------------------
-- Connexion des actions associees a chaque objet du formulaire et aux menus --------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void Rufus::ConnectSignals()
{
    // Les objets -------------------------------------------------------------------------------------------------
    connect (ui->AccueilupTableWidget,                              &QTableWidget::currentCellChanged,                  this,   [=] {ActiveActeAccueil(ui->AccueilupTableWidget->currentRow());});
    connect (ui->ActePrecedentpushButton,                           &QPushButton::clicked,                              this,   [=] {NavigationConsult(ItemsList::Prec);});
    connect (ui->ActeSuivantpushButton,                             &QPushButton::clicked,                              this,   [=] {NavigationConsult(ItemsList::Suiv);});
    connect (ui->CourrierAFairecheckBox,                            &QPushButton::clicked,                              this,   &Rufus::CourrierAFaireChecked);
    connect (ui->CreerActepushButton,                               &QPushButton::clicked,                              this,   [=] {CreerActe(currentpatient());});
    connect (ui->CreerActepushButton_2,                             &QPushButton::clicked,                              this,   [=] {CreerActe(currentpatient());});
    connect (ui->CreerBOpushButton,                                 &QPushButton::clicked,                              this,   &Rufus::CreerBilanOrtho);
    connect (ui->CreerBOpushButton_2,                               &QPushButton::clicked,                              this,   &Rufus::CreerBilanOrtho);
    connect (ui->CreerInterventionpushButton,                       &QPushButton::clicked,                              this,   [=] { ProgrammationIntervention(currentpatient(), currentacte()); });
    connect (ui->CreerDDNdateEdit,                                  &QDateEdit::dateChanged,                            this,   [=] {if (m_mode == RechercheDDN) FiltreTableparDDN();});
    connect (ui->ChercherDepuisListepushButton,                     &QPushButton::clicked,                              this,   &Rufus::ChercherDepuisListe);
    connect (ui->CreerNomlineEdit,                                  &QLineEdit::textEdited,                             this,   &Rufus::MajusculeCreerNom);
    connect (ui->CreerPrenomlineEdit,                               &QLineEdit::textEdited,                             this,   &Rufus::MajusculeCreerPrenom);
    if (!currentuser()->isNeutre())
        connect (ui->CreerDossierpushButton,                        &QPushButton::clicked,                              this,   &Rufus::CreerDossierpushButtonClicked);
    connect (ui->DernierActepushButton,                             &QPushButton::clicked,                              this,   [=] {NavigationConsult(ItemsList::Fin);});
    connect (ui->EnregistrePaiementpushButton,                      &QPushButton::clicked,                              this,   [=] {AppelPaiementDirect(BoutonPaiement);});
    connect (ui->FermepushButton,                                   &QPushButton::clicked,                              this,   &Rufus::SortieAppli);
    connect (ui->GratuitpushButton,                                 &QPushButton::clicked,                              this,   &Rufus::ActeGratuit);
    connect (ui->FSEpushButton,                                     &QPushButton::clicked,                              this,   &Rufus::SaisieFSE);
    connect (ui->IdentPatienttextEdit,                              &QWidget::customContextMenuRequested,               this,   &Rufus::MenuContextuelIdentPatient);
    connect (ui->LFermepushButton,                                  &QPushButton::clicked,                              this,   &Rufus::SortieAppli);
    connect (ui->ListepushButton,                                   &QPushButton::clicked,                              this,   &Rufus::ModeSelectDepuisListe);
    connect (ui->LListepushButton,                                  &QPushButton::clicked,                              this,   &Rufus::ModeSelectDepuisListe);
    if (!currentuser()->isNeutre())
    {
        connect (ui->LNouvDossierpushButton,                        &QPushButton::clicked,                              this,   &Rufus::ModeCreationDossier);
        connect (ui->LRecopierpushButton,                           &QPushButton::clicked,                              this,   [=] {RecopierDossier();});
    }
    connect (ui->SendMessagepushButton,                             &QPushButton::clicked,                              this,   [=] {QMap<QString, QVariant> map;  map["null"] = true; SendMessage(map, currentpatient()->id());});
    if (!currentuser()->isNeutre())
        connect (ui->LSendMessagepushButton,                        &QPushButton::clicked,                              this,   [=] {QMap<QString, QVariant> map;  map["null"] = true; SendMessage(map);});
    connect (ui->MGupComboBox,                                      QOverload<int>::of(&QComboBox::activated),          this,   &Rufus::ChoixMG);
    connect (ui->AutresCorresp1upComboBox,                          QOverload<int>::of(&QComboBox::activated),          this,   [=] {ChoixCor(ui->AutresCorresp1upComboBox);});
    connect (ui->AutresCorresp2upComboBox,                          QOverload<int>::of(&QComboBox::activated),          this,   [=] {ChoixCor(ui->AutresCorresp2upComboBox);});
    connect (ui->MGupComboBox,                                      &QWidget::customContextMenuRequested,               this,   &Rufus::MenuContextuelMedecin);
    connect (ui->AutresCorresp1upComboBox,                          &QWidget::customContextMenuRequested,               this,   [=] {MenuContextuelCorrespondant(ui->AutresCorresp1upComboBox);});
    connect (ui->AutresCorresp2upComboBox,                          &QWidget::customContextMenuRequested,               this,   [=] {MenuContextuelCorrespondant(ui->AutresCorresp2upComboBox);});
    connect (ui->ModifDatepushButton,                               &QPushButton::clicked,                              this,   [=] {ui->ActeDatedateEdit->setEnabled(true); ui->ActeDatedateEdit->setFocus();});
    connect (ui->ActeDatedateEdit,                                  &QDateTimeEdit::dateChanged,
            this,
            [=] (QDate date) {
                QMap<QString,QVariant>  Age = Utils::CalculAge(currentpatient()->datedenaissance(), date);
                ui->AgelineEdit             ->setText(Age["toString"].toString());}
            );
    connect (wdg_modifIdentificationupSmallButton,                  &QPushButton::clicked,                              this,   &Rufus::ChoixMenuContextuelIdentPatient);
    connect (ui->MotsClesLabel,                                     &QWidget::customContextMenuRequested,               this,   &Rufus::MenuContextuelMotsCles);
    connect (ui->MotsClesupSmallButton,                             &QPushButton::clicked,                              this,   &Rufus::ChoixMenuContextuelMotsCles);
    connect (ui->NouvDossierpushButton,                             &QPushButton::clicked,                              this,   &Rufus::ModeCreationDossier);
    connect (ui->OuvreActesPrecspushButton,                         &QPushButton::clicked,                              this,   &Rufus::OuvrirActesPrecspushButtonClicked);
    connect (ui->OuvreDocsExternespushButton,                       &QPushButton::clicked,                              this,   &Rufus::ActualiseDocsExternes);
    connect (ui->OuvrirDocumentpushButton,                          &QPushButton::clicked,                              this,   [=] {ImprimeDocument(currentpatient());});
    if (!currentuser()->isNeutre())
    {
        connect (ui->PatientsListeTableView,                        &QWidget::customContextMenuRequested,               this,   &Rufus::MenuContextuelListePatients);
        connect (ui->PatientsListeTableView,                        &QTableView::doubleClicked,                         this,   [=] {OuvrirDossier(getPatientFromCursorPositionInTable());});
    }
    connect (ui->PatientsListeTableView,                            &QTableView::entered,                               this,   [=] {AfficheToolTip(getPatientFromCursorPositionInTable());});
    connect (ui->PatientsListeTableView->selectionModel(),          &QItemSelectionModel::selectionChanged,             this,   &Rufus::EnableButtons);
    connect (ui->PatientsVusFlecheupLabel,                          &UpLabel::clicked,                                  this,   &Rufus::AffichePatientsVusWidget);
    connect (ui->PatientsVusupTableWidget,                          &QTableView::activated,                             this,   [=] {gTimerPatientsVus->start();});
    connect (ui->PremierActepushButton,                             &QPushButton::clicked,                              this,   [=] {NavigationConsult(ItemsList::Debut);});
    connect (ui->RefractionpushButton,                              &QPushButton::clicked,                              this,   [=] {RefractionMesure(dlg_refraction::Manuel);});
    connect (ui->SalleDAttentepushButton,                           &QPushButton::clicked,                              this,   &Rufus::SalleDAttente);
    connect (ui->SalleDAttenteupTableWidget,                        &UpTableWidget::dropsignal,                         this,   [=] {DropPatient(ui->SalleDAttenteupTableWidget->dropData());});
    connect (ui->SupprimeActepushButton,                            &QPushButton::clicked,                              this,   [=] {SupprimerActe(currentacte());});
    connect (ui->TonometriepushButton,                              &QPushButton::clicked,                              this,   &Rufus::Tonometrie);
    connect (ui->PachymetriepushButton,                             &QPushButton::clicked,                              this,   &Rufus::Pachymetrie);
    connect (ui->VitaleupPushButton,                                &QPushButton::clicked,                              this,   &Rufus::LireLaCV);

    connect (ui->ActeMontantlineEdit,                               &UpLineEdit::TextModified,                          this,   &Rufus::ActeMontantModifie);
    connect (ui->BasculerMontantpushButton,                         &QPushButton::clicked,                              this,   &Rufus::BasculerMontantActe);
    connect (ui->CCAMlinklabel,                                     &QLabel::linkActivated,                             this,   [=] {QDesktopServices::openUrl(QUrl(LIEN_CCAM));});
    connect (ui->ModifierCotationActepushButton,                    &QPushButton::clicked,                              this,   &Rufus::ModifCotationActe);
    // Les tabs --------------------------------------------------------------------------------------------------
    connect (ui->tabWidget,                                         &QTabWidget::currentChanged,                        this,   &Rufus::ChangeTabBureau);

    connect (proc,                                                  &Procedures::ConnectTimers,                         this,   [=](bool a) {ConnectTimers(a);});

    // MAJ Salle d'attente ----------------------------------------------------------------------------------
    connect(Flags::I(),                                             &Flags::UpdSalleDAttente,                           this,   [=](int a)  {   if (m_utiliseTCP)
                                                                                                                                                    envoieTCPMessage(TCPMSG_MAJSalAttente);
                                                                                                                                                else
                                                                                                                                                    m_flagsalledattente = a;
                                                                                                                                                Remplir_SalDat();
                                                                                                                                            } );
    // MAJ Correspondants ----------------------------------------------------------------------------------
    connect(Flags::I(),                                             &Flags::UpdCorrespondants,                          this,   [=](int a)  {   if (m_utiliseTCP)
                                                                                                                                                    envoieTCPMessage(TCPMSG_MAJCorrespondants);
                                                                                                                                                else
                                                                                                                                                    m_flagcorrespondants = a;
                                                                                                                                                ReconstruitCombosCorresp(false);
                                                                                                                                            } );
    // MAJ messages ----------------------------------------------------------------------------------
    connect(Flags::I(),                                             &Flags::UpdMessages,                                this,   [=](int a)  {   if (!m_utiliseTCP)
                                                                                                                                                    m_flagmessages = a;
                                                                                                                                                ReconstruitListeMessages();
                                                                                                                                            } );

    // Nouvelle mesure d'appareil de refraction ----------------------------------------------------------------------------------
    if (proc->HasAppareilRefractionConnecte())
        connect(proc,                                               &Procedures::NouvMesure,                            this,   &Rufus::NouvelleMesure);

    connect (ui->MoulinettepushButton,                              &QPushButton::clicked,                              this,   &Rufus::Moulinette);
    ui->MoulinettepushButton->setVisible(false);
}


void Rufus::OuvrirDocsExternes(DocsExternes *docs)
{
    if (docs == Q_NULLPTR)
        return;
    if (docs->docsexternes()->size() == 0)
        return;
     //! si la fiche est déjà ouverte, on quitte
    QList<dlg_docsexternes *> ListDialogDocs = this->findChildren<dlg_docsexternes *>();
    bool founddlg = false;
    if (ListDialogDocs.size()>0)
        for (int i=0; i< ListDialogDocs.size();++i)
        {
            if (currentpatient() != Q_NULLPTR)
            {
                if (docs->patient()->id() == currentpatient()->id())
                {
                    {
                        if (ListDialogDocs.at(i)->currentpatient()->id() == currentpatient()->id())
                        {
                            if (docs->docsexternes()->size()==0)
                                ListDialogDocs.at(i)->close();
                            else
                            {
                                ListDialogDocs.at(i)->setVisible(true);
                                founddlg = true;
                            }
                        }
                        else
                            ListDialogDocs.at(i)->close();
                    }
                    if (founddlg)
                        return;
                }
            }
            else if (!ListDialogDocs.at(i)->isModal())
                    ListDialogDocs.at(i)->close();
        }
    if (docs->docsexternes()->size()>0)
    {
        dlg_docsexternes *Dlg_DocsExt = new dlg_docsexternes(docs, m_utiliseTCP, this);
        ui->OuvreDocsExternespushButton->setEnabled(true);
        if (docs == Datas::I()->docsexternes)
            Dlg_DocsExt->show();
        else
            Dlg_DocsExt->exec();
    }
    else
        if (docs->patient() == currentpatient())
            ui->OuvreDocsExternespushButton->setEnabled(false);
}


/*-----------------------------------------------------------------------------------------------------------------
-- // mettre à jour dlg_actesprecedents quand le contenu de l'acte affiché a été modifié  --------------------------
-----------------------------------------------------------------------------------------------------------------*/
void Rufus::MAJActesPrecs()
{
    QList<dlg_actesprecedents *> ListDialog = findChildren<dlg_actesprecedents *>();
    if (currentacte() != Q_NULLPTR)
        for (int n = 0; n < ListDialog.size(); n++)
            if (ListDialog.at(n)->currentacte() == currentacte())
                ListDialog.at(n)->ActesPrecsAfficheActe(currentacte());
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
            if (ListDialogDocs.at(i)->currentpatient()->id() == currentpatient()->id())
            {
                emit proc->UpdDocsExternes();
                break;
            }
    }
    else if (currentuser()->isSoignant())
    {
        Datas::I()->docsexternes->actualise();
        if (Datas::I()->docsexternes->NouveauDocumentExterne())
            Datas::I()->docsexternes->setNouveauDocumentExterneFalse();
        if (Datas::I()->docsexternes->docsexternes()->size()>0)
        {
            dlg_docsexternes *Dlg_DocsExt = new dlg_docsexternes(Datas::I()->docsexternes, m_utiliseTCP, this);
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

    //!CONVERSION DES BASES OPLUS ============================================================================================================================================================
    QString req;
    conversionbase *convbase = new conversionbase();
    convbase->conversionbaseoplus();
    delete convbase;
    Remplir_ListePatients_TableView();

    //! CONVERSION DES BASES OPHTALOGIC ============================================================================================================================================================
    /*
    int max = 1000;
    QString req;
    conversionbase *convbase = new conversionbase();
    convbase->conversionbaseophtalogic();
    delete convbase;
    Remplir_ListePatients_TableView();
    */

    //! MODIFICATION DES TABLES CCAM ============================================================================================================================================================
    /*
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
     */

    //! FIN MODIFICATION DES TABLES CCAM ============================================================================================================================================================
    /*
    QString req= "select codeCCAM, modificateur, montant from ccam.ccamd";
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
    req = "select idimpression, Titre from " TBL_IMPRESSIONS " where SousTypeDoc = null and typedoc = 'DocRecu'";
    //qDebug() << req;
    QSqlQuery idquer (req, db->getDataBase() );
    idquer.first();
    for (int i=0; i<idquer.size(); i++)
    {
        txt.setHtml(idquer.value(1).toString());
        str = txt.toPlainText();
        req = "update " TBL_IMPRESSIONS " set SousTypeDoc = '" + str + "', Formatdoc = 'Prescription', emisrecu = 1 where idimpression = " + idquer.value(0).toString();
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
            Message::I()->TrayMessage("correction des actes - acte n° " + imp.value(0).toString());
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
            Message::I()->TrayMessage("correction des impressions - impression n° " + imp2.value(0).toString());
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


/*!    // CREATION D'UNE BASE FACTICE ============================================================================================================================================================
    //Mélange les noms, et 1ère ligne d'adresse dans la base */
    /*
    if (UpMessageBox::Question(this,tr("ATTENTION"),tr("Cette fonction sert à générer une base factice pour la démonstration du logiciel") + "<br />"
                               + tr("Si vous cliquez sur OK, tous les enregistrements de la base seront mélangés et les données seront donc irrémédiablement perdues")) != UpSmallButton::STARTBUTTON)
        return;
    bool ok;
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
            //Message::I()->TrayMessage(quernom.value(1).toString() + " " + AncNom + " - " + QString::number(k) + "/" + QString::number(s), 1);
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

    copierequete = "drop table if exists  rufus.donneessocialespatients2;\n";
    copierequete += "create table rufus.donneessocialespatients2 like " TBL_DONNEESSOCIALESPATIENTS ";";
    copierequete += "insert into rufus.donneessocialespatients2 (select * from " TBL_DONNEESSOCIALESPATIENTS ");";
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
    FiltreTable();

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

void Rufus::ActeGratuit()
{
    ui->ActeCotationcomboBox->setCurrentIndex(0);
    if (Datas::I()->users->getById(currentacte()->idComptable()) == Q_NULLPTR)
        ItemsList::update(currentacte(),CP_IDUSERCOMPTABLE_ACTES, currentuser()->idcomptableactes());
}

void Rufus::ActeMontantModifie()
{
    QString b = QLocale().toString(QLocale().toDouble(ui->ActeMontantlineEdit->text()),'f',2);
    if (b != m_montantActe)
        ValideActeMontantLineEdit(b, m_montantActe);  // ActeMontantModifie()
    if (Datas::I()->users->getById(currentacte()->idComptable()) == Q_NULLPTR)
        ItemsList::update(currentacte(),CP_IDUSERCOMPTABLE_ACTES, currentuser()->idcomptableactes());
}

/*------------------------------------------------------------------------------------------------------------------------------------
-- Afficher le motif de l'acte ----------------------------------------------------------------
------------------------------------------------------------------------------------------------------------------------------------*/
void Rufus::AfficheMotif(UpLabel *lbl)
{
    QString Msg("");
    int id = lbl->iD();
    PatientEnCours *patcrs = Q_NULLPTR;
    for (auto it = Datas::I()->patientsencours->patientsencours()->constBegin(); it != Datas::I()->patientsencours->patientsencours()->constEnd(); ++it)
    {
        if (id == it.key())
            if (const_cast<PatientEnCours*>(it.value()) != Q_NULLPTR)
                    patcrs = const_cast<PatientEnCours*>(it.value());
    }
    if (patcrs == Q_NULLPTR)
        return;
    Patient *pat = Datas::I()->patients->getById(id);
    QDate ddn = pat->datedenaissance();
    QMap<QString,QVariant> mapage;
    if (ddn.isValid())
    {
        mapage = Utils::CalculAge(ddn, m_currentdate);
        Msg += mapage["toString"].toString();
    }
    if (patcrs->isurgence())
    {
        if (Msg != "") Msg += "\n";
        Msg += tr("Urgence");
    }
    else if (patcrs->motif() != "")
    {
        if (Msg != "") Msg += "\n";
        Msg += patcrs->motif();
    }
    if (lbl->parent()->parent() == ui->SalleDAttenteupTableWidget)
    {
        if (patcrs->message() != "")
        {
            if (Msg != "") Msg += "\n";
            Msg += patcrs->message();
        }
    }
    else if (lbl->parent()->parent() == ui->AccueilupTableWidget)
    {
        if (patcrs->messageretour() != "")
        {
            if (Msg != "") Msg += "\n";
            Msg += patcrs->messageretour();
        }
        else if (patcrs->message() != "")
        {
            if (Msg != "") Msg += "\n";
            Msg += patcrs->message();
        }
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
    QString             NomPrenom, zw;
    QFontMetrics        fm(qApp->font());
    // PATIENTS VUS AUJOURD'HUI ---------------------------------------------------------------------------------------------------
    QString req =   "SELECT pat." CP_IDPAT_PATIENTS ", act." CP_ID_ACTES " , " CP_NOM_PATIENTS ", " CP_PRENOM_PATIENTS ", " CP_LOGIN_USR ", " CP_DATE_ACTES ", " CP_COTATION_ACTES ", " CP_MONTANT_ACTES ", " CP_HEURE_ACTES ", " CP_TYPEPAIEMENT_TYPEPAIEMENTACTES ", " CP_TIERS_TYPEPAIEMENTACTES ", usr." CP_ID_USR " FROM "
                           TBL_PATIENTS " as pat, " TBL_ACTES " as act, " TBL_UTILISATEURS " as usr, " TBL_TYPEPAIEMENTACTES " as typ"
                           " WHERE usr." CP_ID_USR " = act." CP_IDUSER_ACTES " and act." CP_IDPAT_ACTES " = pat." CP_IDPAT_PATIENTS " and " CP_DATE_ACTES " = curdate()"
                           " and act." CP_IDPAT_ACTES " not in (select " CP_IDPAT_SALDAT " from " TBL_SALLEDATTENTE ")"
                           " and act." CP_ID_ACTES " = typ." CP_IDACTE_TYPEPAIEMENTACTES ""
                           " ORDER BY " CP_HEURE_ACTES " DESC";
    QList<QVariantList> patlist = db->StandardSelectSQL(req, m_ok, tr("Impossible de remplir la salle d'attente!"));
    if (!m_ok)
        return;
    /*! pas besoin de faire un delete de chaque PatientEncours de chaque UpLabel
     *  chaque Uplabel est parent de son PatientEncours qui va donc être delete audn le UpLable sera delete;
     *  Chaque UpLabel est delete par clearContents() */

    ui->PatientsVusupTableWidget->clearContents();
    ui->PatientsVusupTableWidget->setRowCount(patlist.size());

    for (i = 0; i < patlist.size(); i++)
    {
        int id = patlist.at(i).at(0).toInt();
        int idsuperviseur = patlist.at(i).at(11).toInt();
        User *usr = Datas::I()->users->getById(idsuperviseur);
        QString superviseurlogin = "";
        if (usr != Q_NULLPTR)
            superviseurlogin = usr->login();

        UpLabel *label0, *label1, *label2, *label3, *label4;
        label0 = new UpLabel(id);
        label1 = new UpLabel(id);
        label2 = new UpLabel(id);
        label3 = new UpLabel(id);
        label4 = new UpLabel(id);

        label0->setContextMenuPolicy(Qt::CustomContextMenu);
        label1->setContextMenuPolicy(Qt::CustomContextMenu);
        label2->setContextMenuPolicy(Qt::CustomContextMenu);
        label3->setContextMenuPolicy(Qt::CustomContextMenu);
        label4->setContextMenuPolicy(Qt::CustomContextMenu);

        NomPrenom = patlist.at(i).at(2).toString().toUpper()
                + " " + patlist.at(i).at(3).toString();
        zw = patlist.at(i).at(8).toTime().toString("HH:mm");
        if (zw == "") zw = "hors RDV";
        label0->setText(" " + zw);                                                          // Heure acte
        label1->setText(" " + NomPrenom);                                                   // Nom + Prénom
        QString P=patlist.at(i).at(9).toString();
        if (P == ESP)           P = ESPECES;
        else if (P == CHQ)      P = CHEQUE;
        else if (P == IMP)      P = IMPAYE;
        else if (P == GRAT)     P = "Gratuit";
        else if (P == TRS)
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
        Patient *pat = Datas::I()->patients->getById(id);
        if (pat != Q_NULLPTR)
        {
            connect (label0,        &UpLabel::enter,            this,           [=] {gTimerPatientsVus->start(); AfficheToolTip(pat);});
            connect (label1,        &UpLabel::enter,            this,           [=] {gTimerPatientsVus->start(); AfficheToolTip(pat);});
            connect (label2,        &UpLabel::enter,            this,           [=] {gTimerPatientsVus->start(); AfficheToolTip(pat);});
            connect (label3,        &UpLabel::enter,            this,           [=] {gTimerPatientsVus->start(); AfficheToolTip(pat);});
            connect (label4,        &UpLabel::enter,            this,           [=] {gTimerPatientsVus->start(); AfficheToolTip(pat);});
            connect (label0,        &UpLabel::dblclick,         this,           [=] {if (currentuser()->isSoignant()) OuvrirDossier(pat);});
            connect (label1,        &UpLabel::dblclick,         this,           [=] {if (currentuser()->isSoignant()) OuvrirDossier(pat);});
            connect (label2,        &UpLabel::dblclick,         this,           [=] {if (currentuser()->isSoignant()) OuvrirDossier(pat);});
            connect (label3,        &UpLabel::dblclick,         this,           [=] {if (currentuser()->isSoignant()) OuvrirDossier(pat);});
            connect (label4,        &UpLabel::dblclick,         this,           [=] {if (currentuser()->isSoignant()) OuvrirDossier(pat);});
        }

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
    Datas::I()->patients->loadAll(pat, Item::Update);
    QString Msg = "";
    if (pat->datedenaissance().isValid())
    {
        QMap<QString,QVariant> mapage = Utils::CalculAge(pat->datedenaissance(), m_currentdate);
        Msg += mapage["toString"].toString();
    }
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
    actionRechercheParID->setVisible(m_mode != NouveauDossier);
    bool a = ui->tabWidget->indexOf(ui->tabDossier) < 0;
    actionBilanRecettes->setEnabled(a);
    actionRemiseCheques->setEnabled(a);
    actionRecettesSpeciales->setEnabled(a);
    actionParametres->setEnabled(a);
    actionRechercheCourrier->setVisible(currentuser()->isSoignant() && a);
    if (currentuser()->isSoignant())
    {
        bool c = false;
        QList<int> listcourriers = Datas::I()->actes->listCourriersByUser(currentuser()->id());
        c = (listcourriers.size()>0);
        actionRechercheCourrier     ->setEnabled(a && c);
    }

    if (menu == menuDossier)
    {
        actionOuvrirDossier         ->setVisible(!currentuser()->isNeutre());
        actionCreerDossier          ->setVisible(!currentuser()->isNeutre());
        actionRecopierDossier       ->setVisible(!currentuser()->isNeutre());
        actionSupprimerDossier->setEnabled(true);
        actionSupprimerDossier->setVisible(ui->tabWidget->currentWidget() == ui->tabDossier && currentuser()->isSoignant());
    }
    else if (menu == menuEdition)
    {
        actionParametres         ->setVisible(!currentuser()->isNeutre());
    }
    else if (menu == menuActe && menuActe !=Q_NULLPTR)
    {
        bool b = (ui->tabWidget->currentWidget() == ui->tabDossier);
        actionSupprimerActe->setVisible(b);
        actionCreerActe->setVisible(b);
    }
    else if (menu == menuDocuments)
    {
        menuDocuments->clear();
        if (ui->tabWidget->currentWidget() == ui->tabDossier)
        {
            menuDocuments       ->addMenu(menuEmettre);
            menuDocuments       ->addAction(actionEnregistrerDocScanner);
#ifdef Q_OS_MACOS
            if (db->ModeAccesDataBase() != Utils::Distant)
                menuDocuments   ->addAction(actionEnregistrerVideo);
#endif
            menuDocuments       ->addSeparator();
            actionExportActe    ->setVisible(ui->Acteframe->isVisible());
        }
        menuDocuments->addAction(actionRechercheCourrier);
        menuDocuments->addAction(actionCorrespondants);
        menuDocuments->addAction(actionFabricants);
        if (currentuser()->isMedecin())
            menuDocuments->addAction(actionIOLs);
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
                QString req = "SELECT " CP_IDACTE_VERROUCOMPTA " FROM "  TBL_VERROUCOMPTAACTES
                              " WHERE " CP_IDACTE_VERROUCOMPTA " = "  + ui->AccueilupTableWidget->item(debut+k,5)->text();
                QVariantList actdata = db->getFirstRecordFromStandardSelectSQL(req, m_ok);
                if (!m_ok)
                    return;
                if (actdata.size()==0)
                    ListidActeAPasser << ui->AccueilupTableWidget->item(debut+k,5)->text().toInt();
            }
            if (ListidActeAPasser.size() == 0)
            {
                Utils::playAlarm();
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
        QString req = "SELECT " CP_LOGIN_USR " FROM " TBL_VERROUCOMPTAACTES ", " TBL_UTILISATEURS
                      " WHERE " CP_IDACTE_VERROUCOMPTA " = "  + QString::number(currentacte()->id()) +
                      " AND " CP_POSEPAR_VERROUCOMPTA " = " CP_ID_USR ;
        QVariantList verroudata = db->getFirstRecordFromStandardSelectSQL(req, m_ok);
        if (!m_ok)
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
            UpMessageBox msgbox(this);
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
                QString enreggratuit = "INSERT INTO " TBL_TYPEPAIEMENTACTES " (" CP_IDACTE_TYPEPAIEMENTACTES ", " CP_TYPEPAIEMENT_TYPEPAIEMENTACTES ") VALUES (" + QString::number(currentacte()->id()) + ",'G')";
                db->StandardSQL(enreggratuit, tr("Impossible d'enregister cet acte comme gratuit"));
                AfficheActeCompta(currentacte());
            }
            return;
        }

        QString ActeSal = QString::number(currentacte()->id());
        QString Msg;
        PatientEnCours *patcours = Q_NULLPTR;
        for (auto it = Datas::I()->patientsencours->patientsencours()->constBegin(); it != Datas::I()->patientsencours->patientsencours()->constEnd(); ++it)
        {
            PatientEnCours *patcrs = const_cast<PatientEnCours*>(it.value());
            if (patcrs->id() == currentpatient()->id())
            {
                patcours = patcrs;
                break;
            }
        }
        if (patcours == Q_NULLPTR)
            Datas::I()->patientsencours->CreationPatient(currentpatient()->id(),                                    //! idPat
                                                    Datas::I()->users->getById(currentuser()->idsuperviseur()),     //! User
                                                    RETOURACCUEIL,                                                  //! Statut
                                                    db->ServerDateTime().time(),                                    //! heureStatut
                                                    QTime(),                                                        //! heureRDV
                                                    QTime(),                                                        //! heureArrivee
                                                    "",                                                             //! Motif
                                                    "",                                                             //! Message
                                                    ActeSal.toInt(),                                                //! idActeAPayer
                                                    "",                                                             //! PosteExamen
                                                    0,                                                              //! idUserEnCoursExamen
                                                    0);                                                             //! idSalDat
        else
        {
            ItemsList::update(patcours, CP_STATUT_SALDAT, RETOURACCUEIL);
            ItemsList::update(patcours, CP_IDACTEAPAYER_SALDAT, ActeSal.toInt());
            ItemsList::update(patcours, CP_MESSAGE_SALDAT, Msg);
            ItemsList::update(patcours, CP_HEURESTATUT_SALDAT, db->ServerDateTime().time());
            ItemsList::update(patcours, CP_IDUSERENCOURSEXAM_SALDAT);
            ItemsList::update(patcours, CP_POSTEEXAMEN_SALDAT);
        }
        Flags::I()->MAJFlagSalleDAttente();
        ListidActeAPasser << currentacte()->id();
    }

    dlg_paiementdirect *Dlg_PmtDirect = new dlg_paiementdirect(ListidActeAPasser, this);//NOTE : New Paiement
    if(Dlg_PmtDirect->initOK())
        Dlg_PmtDirect->exec();
    delete Dlg_PmtDirect;
    if (origin == BoutonPaiement)  // on redonne le statut en cours d'examen au dossier
    {
        PatientEnCours *patcours = Q_NULLPTR;
        for (auto it = Datas::I()->patientsencours->patientsencours()->constBegin(); it != Datas::I()->patientsencours->patientsencours()->constEnd(); ++it)
        {
            PatientEnCours *patcrs = const_cast<PatientEnCours*>(it.value());
            if (patcrs->id() == currentpatient()->id())
            {
                patcours = patcrs;
                break;
            }
        }
        if (patcours == Q_NULLPTR)
            Datas::I()->patientsencours->CreationPatient(currentpatient()->id(),                                //! idPat
                                                     Datas::I()->users->getById(currentuser()->idsuperviseur()),//! User
                                                     ENCOURSEXAMEN + currentuser()->login(),                    //! Statut
                                                     db->ServerDateTime().time(),                               //! heureStatut
                                                     QTime(),                                                   //! heureRDV
                                                     QTime(),                                                   //! heureArrivee
                                                     "",                                                        //! Motif
                                                     "",                                                        //! Message
                                                     0,                                                         //! idActeAPayer
                                                     Utils::hostName().left(60),                       //! PosteExamen
                                                     currentuser()->id(),                                       //! idUserEnCoursExamen
                                                     0);                                                        //! idSalDat
        else
        {
            ItemsList::update(patcours, CP_STATUT_SALDAT, ENCOURSEXAMEN + currentuser()->login());
            ItemsList::update(patcours, CP_HEURESTATUT_SALDAT, db->ServerDateTime().time());
            ItemsList::update(patcours, CP_IDUSERENCOURSEXAM_SALDAT, currentuser()->id());
            ItemsList::update(patcours, CP_POSTEEXAMEN_SALDAT, Utils::hostName().left(60));
        }
        Flags::I()->MAJFlagSalleDAttente();
    }
    if (currentpatient() != Q_NULLPTR)
    {
        m_lignespaiements->initListeByPatient(currentpatient());
        if (currentacte() != Q_NULLPTR)
        {
            m_listeactes->initListeByPatient(currentpatient(), Item::Update);
            if (ui->tabDossier->isVisible())
                AfficheActeCompta(currentacte());
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
                Utils::playAlarm();
                PaimtList.at(i)->raise();
                return;
            }
    dlg_paiementtiers *Dlg_PmtTiers = new dlg_paiementtiers(this); //NOTE : New Paiement
    if(Dlg_PmtTiers->initOK())
    {
        Dlg_PmtTiers->setWindowTitle(tr("Gestion des tiers payants"));
        Dlg_PmtTiers->show();
        connect(Dlg_PmtTiers, &QDialog::finished, this, [=]{
            if (currentpatient() != Q_NULLPTR)
            {
                m_lignespaiements->initListeByPatient(currentpatient());
                if (currentacte() != Q_NULLPTR && ui->tabDossier->isVisible())
                    AfficheActeCompta(currentacte());
            }
        });
    }
}

void Rufus::AutreDossier(Patient *pat)
{
    if(currentuser()->isSoignant())
    {
        Datas::I()->patients->setdossierpatientaouvrir(pat);
        if (pat)
            ChoixMenuContextuelListePatients(pat->id(), tr("Autre Dossier"));
    }
}

void Rufus::BasculerMontantActe()
{
    m_montantActe = QLocale().toString(QLocale().toDouble(ui->ActeMontantlineEdit->text()),'f',2);
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
        ValideActeMontantLineEdit(ui->ActeMontantlineEdit->text(), m_montantActe);       //BasculerMontantActe()
    }
}

void Rufus::BilanRecettes()
{   
    dlg_bilanrecettes *Dlg_BilanRec = new dlg_bilanrecettes();
    if (Dlg_BilanRec->initOK())
        Dlg_BilanRec->exec();
    delete Dlg_BilanRec;
}

void Rufus::ChangeTabBureau()
{
    if(ui->tabWidget->currentWidget() == ui->tabList)
    {
        //setFixedWidth(LARGEURLISTE);
        ui->CreerNomlineEdit->setFocus();
        CalcNbDossiers();
    }
    //else
        //setFixedWidth(LARGEURNORMALE);
}

void Rufus::ChoixMG()
{
    //ItemsList::update(currentpatient(), CP_IDMG_RMP,ui->MGupComboBox->currentData().toInt());
    Datas::I()->patients->updateCorrespondant(currentpatient(), Correspondant::MG, Datas::I()->correspondants->getById(ui->MGupComboBox->currentData().toInt()));
    OKModifierTerrain(currentpatient(), false);
    ui->MGupComboBox->setImmediateToolTip(CalcToolTipCorrespondant(ui->MGupComboBox->currentData().toInt()));
}

/*-----------------------------------------------------------------------------------------------------------------
-- Rechercher le nom dans la listePatients par l'id ---------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void Rufus::CherchePatientParID(int id)
{
    Patient *pat = Datas::I()->patients->getById(id);
    if (pat != Q_NULLPTR)
    {
        ui->CreerNomlineEdit->setText(pat->nom());
        ui->CreerPrenomlineEdit->setText(pat->prenom());
        wdg_nomlbl->setText(pat->nom().toUpper() + " " + pat->prenom());
        FiltreTable(pat->nom(), pat->prenom());
        RecaleTableView(pat);
    }
    else
    {
        ui->CreerNomlineEdit->clear();
        ui->CreerPrenomlineEdit->clear();
        wdg_nomlbl->clear();
    }
}

void Rufus::ChoixCor(UpComboBox *box)
{
    if (box==ui->AutresCorresp1upComboBox)
        Datas::I()->patients->updateCorrespondant(currentpatient(), Correspondant::Spe1, Datas::I()->correspondants->getById(box->currentData().toInt()));
    else if (box==ui->AutresCorresp2upComboBox)
        Datas::I()->patients->updateCorrespondant(currentpatient(), Correspondant::Spe2, Datas::I()->correspondants->getById(box->currentData().toInt()));
     box->setImmediateToolTip(CalcToolTipCorrespondant(box->currentData().toInt()));
}
/* -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    tous les timers sont déconnectés pendant les procédures de sauvegarde de la base ---------------------------------------------------------------------------------------------------------------------------
 -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void Rufus::ConnectTimers(bool a)
{
    if (a)
    {
        if (db->ModeAccesDataBase() == Utils::Distant)
        {
            t_timerSalDat               ->start(10000);
            t_timerCorrespondants       ->start(60000);
            t_timerActualiseDocsExternes->start(60000);
            t_timerVerifMessages        ->start(60000);
        }
        else
        {
            if (!m_utiliseTCP)
            {
                t_timerSalDat           ->start(1000);
                t_timerCorrespondants   ->start(30000);
            }
            t_timerVerifImportateurDocs ->start(60000);
            t_timerExportDocs           ->start(10000);
            t_timerActualiseDocsExternes->start(5000);
            t_timerVerifMessages        ->start(10000);
            t_timerSupprDocs            ->start(60000);
        }
        t_timerPosteConnecte            ->start(10000);
        t_timerVerifVerrou              ->start(60000);

        connect (t_timerPosteConnecte,              &QTimer::timeout,   this,   &Rufus::MAJPosteConnecte);
        connect (t_timerActualiseDocsExternes,      &QTimer::timeout,   this,   &Rufus::ActualiseDocsExternes);
        if (!m_utiliseTCP)
        {
            connect (t_timerSalDat,                 &QTimer::timeout,   this,   &Rufus::VerifSalleDAttente);
            connect (t_timerCorrespondants,         &QTimer::timeout,   this,   &Rufus::VerifCorrespondants);
            connect (t_timerVerifVerrou,            &QTimer::timeout,   this,   &Rufus::VerifVerrouDossier);
            connect (t_timerVerifMessages,          &QTimer::timeout,   this,   &Rufus::VerifMessages);
            connect (t_timerVerifImportateurDocs,   &QTimer::timeout,   this,   &Rufus::VerifImportateur);
            if (db->ModeAccesDataBase() != Utils::Distant)
                connect(t_timerSupprDocs,           &QTimer::timeout,   this,   &Rufus::SupprimerDocsEtFactures);
        }

    }
    else
    {
        t_timerVerifImportateurDocs  ->disconnect();
        t_timerSalDat                ->disconnect();
        t_timerCorrespondants        ->disconnect();
        t_timerExportDocs            ->disconnect();
        t_timerActualiseDocsExternes ->disconnect();
        t_timerVerifMessages         ->disconnect();
        t_timerPosteConnecte         ->disconnect();
        t_timerVerifVerrou           ->disconnect();
        t_timerSupprDocs             ->disconnect();
        t_timerVerifImportateurDocs  ->stop();
        t_timerSalDat                ->stop();
        t_timerCorrespondants        ->stop();
        t_timerExportDocs            ->stop();
        t_timerActualiseDocsExternes ->stop();
        t_timerVerifMessages         ->stop();
        t_timerPosteConnecte         ->stop();
        t_timerVerifVerrou           ->stop();
        t_timerSupprDocs             ->stop();
    }
}

void Rufus::CourrierAFaireChecked()
{
    QString cr = (ui->CourrierAFairecheckBox->isChecked()? "T" :"");
    ItemsList::update(currentacte(), CP_COURRIERAFAIRE_ACTES, cr);
}

/*------------------------------------------------------------------------------------------------------------------
-- création - gestion des bilans orthoptiques ----------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void Rufus::CreerBilanOrtho()
{
    bool    nouveauBO       = true;
    bool    creeracte       = true;
    QDate DateBl;
    RecalcCurrentDateTime();
    if (ui->Acteframe->isVisible() && currentacte() != Q_NULLPTR)
    {
        QString requete = "select idbilanortho from " TBL_BILANORTHO
                " where idbilanortho = " + QString::number(currentacte()->id());
        QVariantList bodata = db->getFirstRecordFromStandardSelectSQL(requete, m_ok);
        if (!m_ok)
            return;
        nouveauBO = (bodata.size()<1);
        User *creatusr = Datas::I()->users->getById(currentacte()->idCreatedBy());
        if (!nouveauBO)
        {
            DateBl = currentacte()->date();
            if (DateBl != m_currentdate)
            {
                UpMessageBox msgbox(this);
                UpSmallButton ReprendreBOBouton(tr("Reprendre"));
                UpSmallButton NouveauBOBouton(tr("Créer un nouveau\nbilan orthoptique"));
                UpSmallButton AnnulBouton(tr("Annuler"));
                msgbox.setText("Euuhh... " + currentuser()->login());
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
            else if (creatusr)
                if (!creatusr->isOrthoptist())
                {
                    nouveauBO = true;
                    creeracte = false;
                }
        }
        else if (creatusr)
            creeracte = !creatusr->isOrthoptist();
    }

    dlg_bilanortho *Dlg_BlOrtho = new dlg_bilanortho(currentacte(), nouveauBO);
    if (!nouveauBO)
    {
        Dlg_BlOrtho->ui->OcclAlternlabel    ->setVisible(currentpatient()->datedenaissance().daysTo(DateBl) < 730);
        Dlg_BlOrtho->ui->OcclAlterncomboBox ->setVisible(currentpatient()->datedenaissance().daysTo(DateBl) < 730);
        Dlg_BlOrtho->ui->MotiftextEdit      ->setHtml(ui->ActeMotiftextEdit->toHtml());
        Dlg_BlOrtho->ui->ConclusiontextEdit ->setHtml(ui->ActeConclusiontextEdit->toHtml());
        Dlg_BlOrtho             ->setDateBO(QDate::fromString(ui->ActeDatedateEdit->text(),"dd/MM/yyyy"));
    }
    else
    {
        if (creeracte)
        {
            if (ui->Acteframe->isVisible())
                if (!AutorDepartConsult(false))
                {
                    delete Dlg_BlOrtho;
                    return;
                }
            CreerActe(currentpatient());
        }
        QString RefractionOD    = "";
        QString RefractionOG    = "";
        Dlg_BlOrtho             ->setDateBO(m_currentdate);

        QMapIterator<int, Refraction*> itref (*Datas::I()->refractions->refractions());
        itref.toBack();
        while (itref.hasPrevious()) {
            itref.previous();
            Refraction* ref = const_cast<Refraction*>(itref.value());
            if (ref->typemesure() == Refraction::Acuite)
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
        Dlg_BlOrtho->ui->OcclAlternlabel->setVisible(currentpatient()->datedenaissance().daysTo(m_currentdate) < 730);
        Dlg_BlOrtho->ui->OcclAlterncomboBox->setVisible(currentpatient()->datedenaissance().daysTo(m_currentdate) < 730);
    }
    if (Dlg_BlOrtho->exec() == QDialog::Accepted)
    {
        QString const debutdelimiter    = "<a name=\"BODEBUT\"></a>";
        QString const findelimiter      = "<a name=\"BOFIN\"></a>";

        // Compléter le champ Motif et mettre à jour l'affichage de ActeMotiftextEdit
        QString Motif                   = Dlg_BlOrtho->ui->MotiftextEdit->toPlainText();
        Motif                           .insert(Motif.length()-2, findelimiter);
        Motif                           = HTML_RETOURLIGNE + debutdelimiter + Motif + "</a></p>";
        ui->ActeMotiftextEdit           ->setText(Motif);
        ItemsList::update(currentacte(), CP_MOTIF_ACTES, Motif);

        // Compléter le Champ Texte et mettre à jour l'affichage de ActeTextetextEdit
        QString Reponse                 = Dlg_BlOrtho->calcReponsehTml(ui->ActeTextetextEdit->toHtml());
        ui->ActeTextetextEdit           ->setHtml(Reponse);
        ItemsList::update(currentacte(), CP_TEXTE_ACTES, Reponse);

        // Compléter le Champ Conclusion et mettre à jour l'affichage de ActeConclusiontextEdit
        QString Concl                   = Dlg_BlOrtho->ui->ConclusiontextEdit->toPlainText();
        Concl                           .insert(Concl.length()-2, findelimiter);
        Concl                           = HTML_RETOURLIGNE + debutdelimiter + Concl + "</p>";
        ui->ActeConclusiontextEdit      ->setText(Concl);
        ItemsList::update(currentacte(), CP_CONCLUSION_ACTES, Concl);

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
        bilanorthorequete += QString::number(currentacte()->id());                                                               //0 idActe
        bilanorthorequete += ", '" + Utils::correctquoteSQL(Motif) + "'\n";                                                     //1 Motif
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
        if (Dlg_BlOrtho->ui->ODOrientationradioButton->isChecked())                                                              //9 Orientation
            bilanorthorequete += ", 'D'";
        else if (Dlg_BlOrtho->ui->OGOrientationradioButton->isChecked())
            bilanorthorequete += ", 'G'";
        else
            bilanorthorequete += ", ''";
        bilanorthorequete += ", '" + Utils::correctquoteSQL(Dlg_BlOrtho->ui->EcranVLSCcomboBox->currentText()) + "'\n";          //10 EcranVLSC
        if (Dlg_BlOrtho->ui->EcranVLSCDcomboBox->currentText() != "-")                                                           //11 EcranVLSCD
            bilanorthorequete += ", " + Utils::correctquoteSQL(Dlg_BlOrtho->ui->EcranVLSCDcomboBox->currentText()) + "\n";
        else
            bilanorthorequete += ", null";
        if (Dlg_BlOrtho->ui->fixSCVLcomboBox->currentText() != "-")                                                              //12 EcranfixresVLSC
            bilanorthorequete += ", '" + Utils::correctquoteSQL(Dlg_BlOrtho->ui->fixSCVLcomboBox->currentText()) + "'\n";
        else
            bilanorthorequete += ", null";
        bilanorthorequete += ", '" + Utils::correctquoteSQL(Dlg_BlOrtho->ui->EcranVPSCcomboBox->currentText()) + "'\n";          //13 EcranVPSC
        if (Dlg_BlOrtho->ui->EcranVPSCDcomboBox->currentText() != "-")                                                           //14 ECranVPSCD
            bilanorthorequete += ", " + Utils::correctquoteSQL(Dlg_BlOrtho->ui->EcranVPSCDcomboBox->currentText()) + "\n";
        else
            bilanorthorequete += ", null";
        if (Dlg_BlOrtho->ui->fixSCVPcomboBox->currentText() != "-")                                                              //15 EcranfixresVPSC
            bilanorthorequete += ", '" + Utils::correctquoteSQL(Dlg_BlOrtho->ui->fixSCVPcomboBox->currentText()) + "'\n";
        else
            bilanorthorequete += ", null";
        bilanorthorequete += ", '" + Utils::correctquoteSQL(Dlg_BlOrtho->ui->EcranVLASCcomboBox->currentText()) + "'\n";         //16 EcranVLASC
        if (Dlg_BlOrtho->ui->EcranVLASCDcomboBox->currentText() != "-")                                                          //17 EcranVLASCD
            bilanorthorequete += ", " + Utils::correctquoteSQL(Dlg_BlOrtho->ui->EcranVLASCDcomboBox->currentText()) + "\n";
        else
            bilanorthorequete += ", null";
        if (Dlg_BlOrtho->ui->fixASCVLcomboBox->currentText() != "-")                                                             //18 EcranfixresVLASC
            bilanorthorequete += ", '" + Utils::correctquoteSQL(Dlg_BlOrtho->ui->fixASCVLcomboBox->currentText()) + "'\n";
        else
            bilanorthorequete += ", null";
        bilanorthorequete += ", '" + Utils::correctquoteSQL(Dlg_BlOrtho->ui->EcranVPASCcomboBox->currentText()) + "'\n";         //19 EcranVPASC
        if (Dlg_BlOrtho->ui->EcranVPASCDcomboBox->currentText() != "-")                                                          //20 EcranVPASCD
            bilanorthorequete += ", " + Utils::correctquoteSQL(Dlg_BlOrtho->ui->EcranVPASCDcomboBox->currentText()) + "\n";
        else
            bilanorthorequete += ", null";
        if (Dlg_BlOrtho->ui->fixASCVPcomboBox->currentText() != "-")                                                             //21 EcranfixresVPASC
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
        if (nouveauBO) SupprimerActe(currentacte());

    Dlg_BlOrtho->close();
    delete Dlg_BlOrtho;
    MAJDocsExternes();                  //CreerBilanOrtho()
}

void Rufus::CreerDossierpushButtonClicked()
{
    if (m_mode == NouveauDossier)
        CreerDossier();
    else if (ui->PatientsListeTableView->selectionModel()->selectedIndexes().size() > 0 && !currentuser()->isNeutre())
        OuvrirDossier(getPatientFromSelectionInTable());
}

void Rufus::EnableButtons()
{
    if (m_mode == Liste || m_mode == RechercheDDN)
    {
        ui->LRecopierpushButton->setEnabled(ui->PatientsListeTableView->selectionModel()->selectedIndexes().size()>0);
    }
    else if (m_mode == NouveauDossier)
    {
        ui->LRecopierpushButton->setEnabled(ui->PatientsListeTableView->selectionModel()->selectedIndexes().size()>0);
        ui->CreerDossierpushButton->setEnabled(ui->CreerNomlineEdit->text() != "" && ui->CreerPrenomlineEdit->text() != "");
    }
}

void Rufus::EnregistreDocScanner(Patient *pat)
{
    if (pat == Q_NULLPTR)
        return;
    dlg_docsscanner *Dlg_DocsScan = new dlg_docsscanner(pat, dlg_docsscanner::Document, "", this);
    if (Dlg_DocsScan->initOK())
    {
        Dlg_DocsScan->setWindowTitle(tr("Enregistrer un document issu du scanner pour ") + pat->nom().toUpper() + " " + pat->prenom());
        Dlg_DocsScan->exec();
    }
    delete  Dlg_DocsScan;
    if (currentpatient() != Q_NULLPTR)
        if (pat == currentpatient())
            MAJDocsExternes();          //EnregistreDocScanner()
}

void Rufus::EnregistreVideo(Patient *pat)
{
    if (pat == Q_NULLPTR)
        return;
    dlg_docsvideo *Dlg_DocsVideo = new dlg_docsvideo(pat, this);
    Dlg_DocsVideo->exec();
    delete Dlg_DocsVideo;
    if (currentpatient() != Q_NULLPTR)
        if (pat == currentpatient())
            MAJDocsExternes();          //EnregistreVideo()
}

void Rufus::FiltreSalleDAttente()
{
    int index       = wdg_salledattenteTab->currentIndex();
    int iduser      = wdg_salledattenteTab->tabData(index).toInt();
    QString usrlog  = wdg_salledattenteTab->tabText(index);
    if (iduser==-1)
        for(int i=0; i<ui->SalleDAttenteupTableWidget->rowCount(); i++)
            ui->SalleDAttenteupTableWidget->setRowHidden(i,false);
    else
        for(int i=0; i<ui->SalleDAttenteupTableWidget->rowCount(); i++)
        {
            UpLabel *lbl = qobject_cast<UpLabel*>(ui->SalleDAttenteupTableWidget->cellWidget(i,6));
            ui->SalleDAttenteupTableWidget->setRowHidden(i,lbl->text() != usrlog);
        }
}

void Rufus::ActiveActeAccueil(int row)
{
    UpLabel *lblr = qobject_cast<UpLabel*>(ui->AccueilupTableWidget->cellWidget(row,6));
    if (lblr == Q_NULLPTR)
        return;
    int idparent = lblr->text().toInt();
    for (int i=0; i<ui->AccueilupTableWidget->rowCount(); i++)
    {
        UpLabel *lbl = qobject_cast<UpLabel*>(ui->AccueilupTableWidget->cellWidget(i,6));
        if (lbl != Q_NULLPTR)
            if (lbl->text().toInt() != idparent)
                ui->AccueilupTableWidget->setRangeSelected(QTableWidgetSelectionRange(i,0,i,6),false);
    }
}

void Rufus::FiltreAccueil(int idx)
{
    int idsuperviseur        = wdg_accueilTab->tabData(idx).toInt();
    for(int i=0; i<ui->AccueilupTableWidget->rowCount(); i++)
    {
        UpLabel *lbl = qobject_cast<UpLabel*>(ui->AccueilupTableWidget->cellWidget(i,6));
        if (lbl != Q_NULLPTR)
            ui->AccueilupTableWidget->setRowHidden(i,lbl->text() != QString::number(idsuperviseur));
    }
}

void Rufus::GestionComptes()
{
    dlg_comptes *Dlg_Cmpt = new dlg_comptes(this);
    if(Dlg_Cmpt->initOK())
        Dlg_Cmpt->exec();
    delete Dlg_Cmpt;
}

void Rufus::ExporteDocs()
{
    if (!isPosteImport())
        return;
    if (m_pasDExportPourLeMoment)
        return;
    QString pathDirImagerie = proc->AbsolutePathDirImagerie();
    RecalcCurrentDateTime();

    if (!QDir(pathDirImagerie).exists() || pathDirImagerie == "")
    {
        QString msg = tr("Le dossier de sauvegarde d'imagerie") + " <font color=\"red\"><b>" + pathDirImagerie + "</b></font>" + tr(" n'existe pas");
        msg += "<br />" + tr("Renseignez un dossier valide dans") + " <font color=\"green\"><b>" + tr("Emplacement de stockage des documents archivés") + "</b></font>";
        ShowMessage::I()->SplashMessage(msg, 6000);
        return;
    }
    QString CheminEchecTransfrDir   = pathDirImagerie + NOM_DIR_ECHECSTRANSFERTS;
    if (!Utils::mkpath(CheminEchecTransfrDir))
    {
        QString msg = tr("Dossier de sauvegarde ") + "<font color=\"red\"><b>" + CheminEchecTransfrDir + "</b></font>" + tr(" invalide");
        ShowMessage::I()->SplashMessage(msg, 6000);        return;
    }

    int total = db->StandardSelectSQL("SELECT " CP_ID_DOCSEXTERNES " FROM " TBL_DOCSEXTERNES " where " CP_JPG_DOCSEXTERNES " is not null or " CP_PDF_DOCSEXTERNES " is not null",m_ok).size();
    total +=    db->StandardSelectSQL("SELECT " CP_ID_FACTURES " FROM " TBL_FACTURES " where " CP_JPG_FACTURES " is not null or " CP_PDF_FACTURES " is not null", m_ok).size();
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
            t_timerExportDocs->disconnect();
            m_pasDExportPourLeMoment = true;
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
    QString CheminOKTransfrDir      = pathDirImagerie + NOM_DIR_IMAGES;
    QDir DirTrsferOK;
    if (!QDir(CheminOKTransfrDir).exists())
        if (!DirTrsferOK.mkdir(CheminOKTransfrDir))
        {
            QString msg = tr("Dossier de sauvegarde ") + "<font color=\"red\"><b>" + CheminOKTransfrDir + "</b></font>" + tr(" invalide");
            ShowMessage::I()->SplashMessage(msg, 3000);
            return;
        }

    //-----------------------------------------------------------------------------------------------------------------------------------------
    //              LES JPG
    //-----------------------------------------------------------------------------------------------------------------------------------------
    QString req = "SELECT " CP_ID_DOCSEXTERNES ", " CP_IDPAT_DOCSEXTERNES ", " CP_SOUSTYPEDOC_DOCSEXTERNES ", " CP_DATE_DOCSEXTERNES ", " CP_JPG_DOCSEXTERNES ", " CP_LIENFICHIER_DOCSEXTERNES ", " CP_TYPEDOC_DOCSEXTERNES " FROM " TBL_DOCSEXTERNES " where " CP_JPG_DOCSEXTERNES " is not null";
    //qDebug() << req;
    QList<QVariantList> listexportjpg = db->StandardSelectSQL(req, m_ok );
    if (m_ok)
        for (int i=0; i<listexportjpg.size(); i++)
        {
            /* si le lien vers le fichier est valide, on efface le champ jpg et on passe à la réponse suivante*/
            if (listexportjpg.at(i).at(5).toString() != "")
            {
                QString CheminFichier = pathDirImagerie + NOM_DIR_IMAGES + listexportjpg.at(i).at(5).toString();
                if (QFile(CheminFichier).exists())
                {
                    db->StandardSQL("update " TBL_DOCSEXTERNES " set " CP_JPG_DOCSEXTERNES " = null where " CP_ID_DOCSEXTERNES " = " + listexportjpg.at(i).at(0).toString());
                    continue;
                }
            }
            QDate datetransfer    = listexportjpg.at(i).at(3).toDate();
            QString CheminOKTransfrDirImg    = CheminOKTransfrDir + "/" + datetransfer.toString("yyyy-MM-dd");
            if (!QDir(CheminOKTransfrDirImg).exists())
                if (!DirTrsferOK.mkdir(CheminOKTransfrDirImg))
                {
                    QString msg = tr("Dossier de sauvegarde ") + "<font color=\"red\"><b>" + CheminOKTransfrDirImg + "</b></font>" + tr(" invalide");
                    ShowMessage::I()->SplashMessage(msg, 3000);
                    return;
                }
            QString NomFileDoc = listexportjpg.at(i).at(1).toString() + "_" + listexportjpg.at(i).at(6).toString() + "-"
                    + listexportjpg.at(i).at(2).toString().replace("/",".") + "_"
                    + listexportjpg.at(i).at(3).toDate().toString("yyyyMMdd") + "-" + QTime::currentTime().toString("HHmmss")
                    + "-" + listexportjpg.at(i).at(0).toString()  + ".jpg";
            QString CheminOKTransfrDoc  = CheminOKTransfrDirImg + "/" + NomFileDoc + "." JPG;
            QString CheminOKTransfrProv = CheminOKTransfrDirImg + "/" + NomFileDoc + "prov." JPG;
            QByteArray ba = listexportjpg.at(i).at(4).toByteArray();
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
                //qDebug() << "erreur";
                return;
            }
            if (!Utils::CompressFileToJPG(CheminOKTransfrProv))
            {
                db->SupprRecordFromTable(listexportjpg.at(i).at(0).toInt(), CP_ID_FACTURES, TBL_FACTURES);
                continue;
            }
            QFile prov(CheminOKTransfrProv);
            if (prov.open(QIODevice::ReadWrite))
            {
                Utils::copyWithPermissions(prov, CheminOKTransfrDoc);
                Utils::removeWithoutPermissions(prov);
            }
            else
                return;
            db->StandardSQL("update " TBL_DOCSEXTERNES " set " CP_JPG_DOCSEXTERNES " = null,"
                            CP_LIENFICHIER_DOCSEXTERNES " = '/" + datetransfer.toString("yyyy-MM-dd") + "/" + Utils::correctquoteSQL(NomFileDoc) + "." JPG
                            "' where " CP_ID_DOCSEXTERNES " = " + listexportjpg.at(i).at(0).toString() );
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
            UpSystemTrayIcon::I()->showMessages(tr("Messages"), listmsg, Icons::icSunglasses(), 10);
        }

    //-----------------------------------------------------------------------------------------------------------------------------------------
    //              LES PDF
    //-----------------------------------------------------------------------------------------------------------------------------------------
    QString reqpdf = "SELECT " CP_ID_DOCSEXTERNES ", " CP_IDPAT_DOCSEXTERNES ", " CP_SOUSTYPEDOC_DOCSEXTERNES ", " CP_DATE_DOCSEXTERNES ", " CP_PDF_DOCSEXTERNES ", " CP_LIENFICHIER_DOCSEXTERNES ", " CP_COMPRESSION_DOCSEXTERNES ", " CP_TYPEDOC_DOCSEXTERNES " FROM " TBL_DOCSEXTERNES " where " CP_PDF_DOCSEXTERNES " is not null";
    QList<QVariantList> listexportpdf = db->StandardSelectSQL(reqpdf, m_ok );
    if (m_ok)
        for (int i=0; i<listexportpdf.size(); i++)
        {
            if (listexportpdf.at(i).at(5).toString() != "")
            {
                QString CheminFichier = pathDirImagerie + NOM_DIR_IMAGES + listexportpdf.at(i).at(5).toString();
                if (QFile(CheminFichier).exists())
                {
                    db->StandardSQL("update " TBL_DOCSEXTERNES " set " CP_PDF_DOCSEXTERNES " = null where " CP_ID_DOCSEXTERNES " = " + listexportpdf.at(i).at(0).toString());
                    continue;
                }
            }
            QDate datetransfer    = listexportpdf.at(i).at(3).toDate();
            QString CheminOKTransfrDirImg      = CheminOKTransfrDir + "/" + datetransfer.toString("yyyy-MM-dd");
            if (!QDir(CheminOKTransfrDirImg).exists())
                if (!DirTrsferOK.mkdir(CheminOKTransfrDirImg))
                {
                    QString msg = tr("Dossier de sauvegarde ") + "<font color=\"red\"><b>" + CheminOKTransfrDirImg + "</b></font>" + tr(" invalide");
                    ShowMessage::I()->SplashMessage(msg, 3000);
                    return;
                }
            QString NomFileDoc = listexportpdf.at(i).at(1).toString() + "_" + listexportpdf.at(i).at(7).toString() + "-"
                    + listexportpdf.at(i).at(2).toString().replace("/",".") + "_"
                    + listexportpdf.at(i).at(3).toDate().toString("yyyyMMdd") + "-" + QTime::currentTime().toString("HHmmss")
                    + "-" + listexportpdf.at(i).at(0).toString()  + ".pdf";
            QString CheminOKTransfrDoc = CheminOKTransfrDirImg + "/" + NomFileDoc;

            QByteArray bapdf;
            bapdf.append(listexportpdf.at(i).at(4).toByteArray());

            QBuffer buf(&bapdf);
            buf.open(QIODevice::ReadWrite);
            QPdfDocument document;
            document.load(&buf);
            if( document.pageCount() > 0)
            {
                QFile file(CheminOKTransfrDoc);
                if (file.open(QIODevice::NewOnly))
                {
                    QDataStream out(&file);
                    out << bapdf;
                }
            }
            else
            {
                UpSystemTrayIcon::I()->showMessages(tr("Messages"), listmsg, Icons::icSunglasses(), 3000);
                QString echectrsfername         = CheminEchecTransfrDir + "/0EchecTransferts - " + datetransfer.toString("yyyy-MM-dd") + ".txt";
                QFile   echectrsfer(echectrsfername);
                if (echectrsfer.open(QIODevice::Append))
                {
                    QTextStream out(&echectrsfer);
                    out << NomFileDoc << "\n" ;
                    echectrsfer.close();
                    QFile CD(CheminEchecTransfrDir + "/" + NomFileDoc);
                    if (CD.open(QIODevice::OpenModeFlag::NewOnly))
                    {
                        QDataStream out(&CD);
                        out << bapdf;
                    }
                }
                QString delreq = "delete from  " TBL_DOCSEXTERNES " where " CP_ID_DOCSEXTERNES " = " + listexportpdf.at(i).at(0).toString();
                //qDebug() << delreq;
                db->StandardSQL(delreq);
                continue;
            }

#if !defined(Q_OS_WIN)
            QFile CC(CheminOKTransfrDoc);
            CC.open(QIODevice::ReadWrite);
            CC.setPermissions( QFileDevice::ReadOther
                              | QFileDevice::ReadGroup
                              | QFileDevice::ReadOwner  | QFileDevice::WriteOwner
                              | QFileDevice::ReadUser   | QFileDevice::WriteUser);
            CC.close();
#endif
            db->StandardSQL("update " TBL_DOCSEXTERNES " set " CP_PDF_DOCSEXTERNES " = null, " CP_COMPRESSION_DOCSEXTERNES " = null,"
                            CP_LIENFICHIER_DOCSEXTERNES " = '/" + datetransfer.toString("yyyy-MM-dd") + "/" + Utils::correctquoteSQL(NomFileDoc)  + "'"
                            " where " CP_ID_DOCSEXTERNES " = " + listexportpdf.at(i).at(0).toString());
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
            UpSystemTrayIcon::I()->showMessages(tr("Messages"), listmsg, Icons::icSunglasses(), 10);
        }
    int totdoc = listexportjpg.size() + listexportpdf.size();
    if (totdoc > 0)
    {
        listmsg <<  tr("export terminé") << QString::number(totdoc) + (totdoc>1? tr(" documents exportés en ") : tr(" document exporté en "))  + duree;
        UpSystemTrayIcon::I()->showMessages(tr("Messages"), listmsg, Icons::icSunglasses(), 3000);
    }




/* LES FACTURES  ============================================*/

    faits = 0;
    debut = QTime::currentTime();
    listmsg.clear();
    duree = "";
    CheminOKTransfrDir  = pathDirImagerie + NOM_DIR_FACTURES;
    if (!QDir(CheminOKTransfrDir).exists())
        if (!DirTrsferOK.mkdir(CheminOKTransfrDir))
        {
            QString msg = tr("Dossier de sauvegarde ") + "<font color=\"red\"><b>" + CheminOKTransfrDir + "</b></font>" + tr(" invalide");
            ShowMessage::I()->SplashMessage(msg, 3000);
            return;
        }

    //-----------------------------------------------------------------------------------------------------------------------------------------
    //              LES JPG
    //-----------------------------------------------------------------------------------------------------------------------------------------
    req = "SELECT "
            CP_ID_FACTURES ", "
            CP_DATEFACTURE_FACTURES ", "
            CP_LIENFICHIER_FACTURES ", "
            CP_INTITULE_FACTURES ", "
            CP_ECHEANCIER_FACTURES ", "
            CP_IDDEPENSE_FACTURES ", "
            CP_JPG_FACTURES
            " FROM " TBL_FACTURES
            " where " CP_JPG_FACTURES " is not null";
    //qDebug() << req;
    QList<QVariantList> listexportjpgfact = db->StandardSelectSQL(req, m_ok);
    if (m_ok)
        for (int i=0; i<listexportjpgfact.size(); i++)
        {
            /* si le lien vers le fichier est valide, on efface le champ jpg et on passe à la réponse suivante*/
            if (listexportjpgfact.at(i).at(2).toString() != "")
            {
                QString CheminFichier = pathDirImagerie + NOM_DIR_FACTURES + listexportjpgfact.at(i).at(2).toString();
                if (QFile(CheminFichier).exists())
                {
                    db->StandardSQL("update " TBL_FACTURES " set " CP_JPG_FACTURES " = null where " CP_ID_FACTURES " = " + listexportjpgfact.at(i).at(0).toString());
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
                req = "select dep." CP_IDUSER_DEPENSES ", " CP_LOGIN_USR " from " TBL_DEPENSES " dep, " TBL_UTILISATEURS " usr"
                                                                                                              " where dep." CP_IDUSER_DEPENSES "  = usr." CP_ID_USR
                                                                                                              " and " CP_IDFACTURE_DEPENSES " = " + listexportjpgfact.at(i).at(0).toString();
            else                                                // c'est une facture, l'iduser est dans la table
                req = "select dep." CP_IDUSER_DEPENSES ", " CP_LOGIN_USR " from " TBL_DEPENSES " dep, " TBL_UTILISATEURS " usr"
                                                                                                              " where dep." CP_IDUSER_DEPENSES "  = usr." CP_ID_USR
                                                                                                              " and " CP_ID_DEPENSES " = " + listexportjpgfact.at(i).at(5).toString();
            Listeusr = db->StandardSelectSQL(req, m_ok);
            if (Listeusr.size()==0) // il n'y a aucune depense enregistrée pour cette facture, on la détruit
            {
                db->SupprRecordFromTable(listexportjpgfact.at(i).at(0).toInt(), CP_ID_FACTURES, TBL_FACTURES);
                continue;
            }
            user = Listeusr.at(0).at(1).toString();
            if (listexportjpgfact.at(i).at(4).toInt()!=1)
                NomFileDoc += "-"+listexportjpgfact.at(i).at(5).toString();

            QString CheminOKTransfrDirImg  = CheminOKTransfrDir + "/" + user;
            if (!QDir(CheminOKTransfrDirImg).exists())
                if (!DirTrsferOK.mkdir(CheminOKTransfrDirImg))
                {
                    QString msg = tr("Dossier de sauvegarde ") + "<font color=\"red\"><b>" + CheminOKTransfrDirImg + "</b></font>" + tr(" invalide");
                    ShowMessage::I()->SplashMessage(msg, 3000);
                    return;
                }

            QString CheminOKTransfrDoc  = CheminOKTransfrDirImg + "/" + NomFileDoc + "." JPG;
            QString CheminOKTransfrProv = CheminOKTransfrDirImg + "/" + NomFileDoc + "prov." JPG;
            QByteArray ba = listexportjpgfact.at(i).at(6).toByteArray();
            QPixmap pix;
            pix.loadFromData(ba);
            /*!
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
                //qDebug() << "erreur";
                return;
            }
            if (!Utils::CompressFileToJPG(CheminOKTransfrProv))
            {
                db->SupprRecordFromTable(listexportjpgfact.at(i).at(0).toInt(), CP_ID_FACTURES, TBL_FACTURES);
                continue;
            }
            QFile prov(CheminOKTransfrProv);
            if (prov.open(QIODevice::ReadWrite))
            {
                Utils::copyWithPermissions(prov, CheminOKTransfrDoc);
                Utils::removeWithoutPermissions(prov);
            }
            else
                return;
            db->StandardSQL("update " TBL_FACTURES " set " CP_JPG_FACTURES " = null, " CP_LIENFICHIER_FACTURES " = '/" + user + "/" + Utils::correctquoteSQL(NomFileDoc) + "." JPG "'"
                            " where " CP_ID_FACTURES " = " + listexportjpgfact.at(i).at(0).toString());
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
            UpSystemTrayIcon::I()->showMessages(tr("Messages"), listmsg, Icons::icSunglasses(), 10);
        }

    //-----------------------------------------------------------------------------------------------------------------------------------------
    //              LES PDF
    //-----------------------------------------------------------------------------------------------------------------------------------------
    reqpdf = "SELECT "
            CP_ID_FACTURES ", "
            CP_DATEFACTURE_FACTURES ", "
            CP_LIENFICHIER_FACTURES ", "
            CP_INTITULE_FACTURES ", "
            CP_ECHEANCIER_FACTURES ", "
            CP_IDDEPENSE_FACTURES ", "
            CP_PDF_FACTURES
            " FROM " TBL_FACTURES
            " where " CP_PDF_FACTURES " is not null";
    QList<QVariantList> listexportpdffact = db->StandardSelectSQL(reqpdf, m_ok );
    if (m_ok)
        for (int i=0; i<listexportpdffact.size(); i++)
        {
            if (listexportpdffact.at(i).at(2).toString() != "")
            {
                QString CheminFichier = pathDirImagerie + NOM_DIR_FACTURES + listexportpdffact.at(i).at(2).toString();
                if (QFile(CheminFichier).exists())
                {
                    db->StandardSQL("update " TBL_FACTURES " set " CP_PDF_FACTURES " = null where " CP_ID_FACTURES " = " + listexportpdffact.at(i).at(0).toString());
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
                req = "select dep." CP_IDUSER_DEPENSES ", " CP_LOGIN_USR " from " TBL_DEPENSES " dep, " TBL_UTILISATEURS " usr"
                                                                                                              " where dep." CP_IDUSER_DEPENSES "  = usr." CP_ID_USR
                                                                                                              " and " CP_IDFACTURE_DEPENSES " = " + listexportpdffact.at(i).at(0).toString();
            else                                                // c'est une facture, l'iduser est dans la table
                req = "select dep." CP_IDUSER_DEPENSES ", " CP_LOGIN_USR " from " TBL_DEPENSES " dep, " TBL_UTILISATEURS " usr"
                                                                                                              " where dep." CP_IDUSER_DEPENSES "  = usr." CP_ID_USR
                                                                                                              " and " CP_ID_DEPENSES " = " + listexportpdffact.at(i).at(5).toString();
            Listeusr = db->StandardSelectSQL(req, m_ok);
            if (Listeusr.size()==0) // il n'y a aucune depense enregistrée pour cette facture, on la détruit
            {
                db->SupprRecordFromTable(listexportpdffact.at(i).at(0).toInt(), CP_ID_FACTURES, TBL_FACTURES);
                continue;
            }
            user = Listeusr.at(0).at(1).toString();
            if (listexportpdffact.at(i).at(4).toInt()!=1)
                NomFileDoc += "-"+listexportpdffact.at(i).at(5).toString();

            QString CheminOKTransfrDirImg  = CheminOKTransfrDir + "/" + user;
            if (!QDir(CheminOKTransfrDirImg).exists())
                if (!DirTrsferOK.mkdir(CheminOKTransfrDirImg))
                {
                    QString msg = tr("Dossier de sauvegarde ") + "<font color=\"red\"><b>" + CheminOKTransfrDirImg + "</b></font>" + tr(" invalide");
                    ShowMessage::I()->SplashMessage(msg, 3000);
                    return;
                }
            QString CheminOKTransfrDoc      = CheminOKTransfrDirImg + "/" + NomFileDoc + "." PDF;

            QByteArray bapdf;
            bapdf.append(listexportpdffact.at(i).at(6).toByteArray());

            QBuffer buf(&bapdf);
            buf.open(QIODevice::ReadWrite);
            QPdfDocument document;
            document.load(&buf);
            if( document.pageCount() > 0)
            {
                QFile file(CheminOKTransfrDoc);
                if (file.open(QIODevice::NewOnly))
                {
                    QDataStream out(&file);
                    out << bapdf;
                }
            }
            else
            {
                QStringList listmsg;
                listmsg << tr("Impossible de charger le document ") + NomFileDoc;
                UpSystemTrayIcon::I()->showMessages(tr("Messages"), listmsg, Icons::icSunglasses(), 3000);
                QString echectrsfername         = CheminEchecTransfrDir + "/0EchecTransferts - " + datetransfer.toString("yyyy-MM-dd") + ".txt";
                QFile   echectrsfer(echectrsfername);
                if (echectrsfer.open(QIODevice::Append))
                {
                    QTextStream out(&echectrsfer);
                    out << NomFileDoc << "\n" ;
                    echectrsfer.close();
                    QFile CD(CheminEchecTransfrDir + "/" + NomFileDoc);
                    if (CD.open(QIODevice::OpenModeFlag::NewOnly))
                    {
                        QDataStream out(&CD);
                        out << bapdf;
                    }
                }
                QString delreq = "delete from  " TBL_DOCSEXTERNES " where " CP_ID_DOCSEXTERNES " = " + listexportpdf.at(i).at(0).toString();
                //qDebug() << delreq;
                db->StandardSQL(delreq);
                continue;
            }

            QFile CC(CheminOKTransfrDoc);
            CC.open(QIODevice::ReadWrite);
            CC.setPermissions(QFileDevice::ReadOther
                              | QFileDevice::ReadGroup
                              | QFileDevice::ReadOwner  | QFileDevice::WriteOwner
                              | QFileDevice::ReadUser   | QFileDevice::WriteUser);
            CC.close();
            db->StandardSQL("update " TBL_FACTURES " set " CP_PDF_FACTURES " = null, " CP_LIENFICHIER_FACTURES " = '/" + user + "/" + Utils::correctquoteSQL(NomFileDoc)  + "." PDF "'"
                            " where " CP_ID_FACTURES " = " + listexportpdffact.at(i).at(0).toString());
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
            UpSystemTrayIcon::I()->showMessages(tr("Messages"), listmsg, Icons::icSunglasses(), 10);
        }
    int totfac = listexportjpgfact.size() + listexportpdffact.size();
    if (totfac > 0)
    {
        listmsg <<  tr("export terminé") << QString::number(totfac) + (totfac>1? tr(" documents comptables exportés en ") :tr(" document comptable exporté en ")) + duree;
        UpSystemTrayIcon::I()->showMessages(tr("Messages"), listmsg, Icons::icSunglasses(), 3000);
    }
}

void Rufus::ImportNouveauDocExterne(AppareilImagerie *appareil)
{
    if (isPosteImport())
    {
        if (m_importdocsexternesthread == Q_NULLPTR)
        {
            m_importdocsexternesthread = new ImportDocsExternesThread();
            connect(m_importdocsexternesthread, &ImportDocsExternesThread::emitmsg, this, &Rufus::AfficheMessageImport);
        }
        QString nomdossier  = appareil->nomdossierechange();
        QStringList filters, listnomsfiles;
        filters << "*.pdf" << "*.jpg" << "*.png" << "*.jpeg";
        listnomsfiles = QDir(nomdossier).entryList(filters, QDir::Files | QDir::NoDotAndDotDot);
        for (int it=0; it<listnomsfiles.size(); it++)
        {
            QString nomfiledoc = listnomsfiles.at(it);
            if (!nomfiledoc.contains("smbtest"))
                m_importdocsexternesthread->RapatrieDocumentsThread(appareil, nomfiledoc);
        }
    }
}

void Rufus::ImprimeDossier(Patient *pat, bool quelepdf)
{
    if(pat == Q_NULLPTR)
        return;
    Actes *acts = Q_NULLPTR;
    QMap<int, Acte*> *listeactes;
    if (currentpatient() != Q_NULLPTR && pat == currentpatient())
        listeactes = m_listeactes->actes();
    else
    {
        acts = new Actes();
        acts->initListeByPatient(pat, Item::Update);
        listeactes = acts->actes();
    }
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
        auto recalclistitems    = [&] (UpComboBox *box, int actid, UpComboBox *finbox)
        {
            int idacteencours = box->currentData().toInt();
            box->clear();
            for (auto it = listeactes->constBegin(); it != listeactes->constEnd(); ++it)
            {
                Acte* act = const_cast<Acte*>(it.value());
                QString dateacte = QLocale::system().toString(act->date(),"dd-MMM-yyyy");
                if (box == finbox)
                {
                    if (act->id() >= actid)
                        box->addItem(dateacte, act->id());
                }
                else
                {
                    if (act->id() <= actid)
                        box->addItem(dateacte, act->id());
                }
            }
            if (box == finbox)
            {
                if (idacteencours >= actid && box->findData(idacteencours)>-1)
                    box->setCurrentIndex(box->findData(idacteencours));
                else
                    box->setCurrentIndex(box->count()-1);
            }
            else
            {
                if (idacteencours <= actid && box->findData(idacteencours)>-1)
                    box->setCurrentIndex(box->findData(idacteencours));
                else
                    box->setCurrentIndex(0);
            }
        };
        auto recalcallitems     = [] (UpComboBox *debutbox, UpComboBox *finbox, QMap<int, Acte*> *listeactes)
        {
            // remplissage des combobox de date des actes
            for (auto it = listeactes->constBegin(); it != listeactes->constEnd(); ++it)
            {
                Acte* act = const_cast<Acte*>(it.value());
                QString dateacte = QLocale::system().toString(act->date(),"dd-MMM-yyyy");
                debutbox    ->addItem(dateacte, act->id());
                finbox      ->addItem(dateacte, act->id());
            }
            debutbox    ->setCurrentIndex(0);
            finbox      ->setCurrentIndex(finbox->count()-1);
        };

        dlg_ask            = new UpDialog(this);
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
        dlg_ask->dlglayout()->insertLayout(0,debutlayout);
        dlg_ask->dlglayout()->insertLayout(1,finlayout);
        dlg_ask->dlglayout()->insertWidget(2,Dossierbutton);
        dlg_ask->dlglayout()->insertWidget(3,Actebutton);
        dlg_ask->AjouteLayButtons(UpDialog::ButtonCancel | UpDialog::ButtonOK);
        //gAsk->setStageCount(0.7);

        dlg_ask->setWindowTitle(tr("Impression dossier"));
        dlg_ask->dlglayout()->setSizeConstraint(QLayout::SetFixedSize);

        combodebut      ->setEditable(false);
        combofin        ->setEditable(false);
        // remplissage des combobox de date des actes
        recalcallitems (combodebut, combofin, listeactes);

        connect(dlg_ask->OKButton,  &QPushButton::clicked,  dlg_ask,    &UpDialog::accept);
        connect(Actebutton,         &QPushButton::clicked,  dlg_ask,    [=] {fixdateacte(combodebut, combofin, currentacte());});
        connect(Dossierbutton,      &QPushButton::clicked,  dlg_ask,    [=] {recalcallitems (combodebut, combofin, listeactes);});
        connect(combodebut,         QOverload<int>::of(&QComboBox::activated),
                                                            dlg_ask,    [=] {recalclistitems(combofin,
                                                                                             combodebut->currentData().toInt(),
                                                                                             combofin);});
        connect(combofin,       QOverload<int>::of(&QComboBox::activated),
                                                            dlg_ask,    [=] {recalclistitems(combodebut,
                                                                                             combofin->currentData().toInt(),
                                                                                             combofin);});
        QList<Acte*> listeactesaimprimer = QList<Acte*>();
        if (dlg_ask->exec() == QDialog::Accepted)
            for (auto it = listeactes->constBegin(); it != listeactes->constEnd(); ++it)
            {
                Acte* act = const_cast<Acte*>(it.value());
                int idacte = act->id();
                if (idacte >= combodebut->currentData().toInt() && idacte <= combofin->currentData().toInt())
                    listeactesaimprimer << act;
            }
        delete dlg_ask;
        dlg_ask = Q_NULLPTR;
        if (listeactesaimprimer.size() > 0)
        {
            bool toutledossier = (listeactes->size() == listeactesaimprimer.size());
            ImprimeListActes(listeactesaimprimer, toutledossier, quelepdf);
            MAJDocsExternes();              // ImprimeDossier()
        }
        if (acts != Q_NULLPTR)
        {
            ItemsList::clearAll(acts->actes());
            delete acts;
        }
    }
}

void Rufus::ImprimeListActes(QList<Acte*> listeactes, bool toutledossier, bool queLePdf, QString nomdossier)
{
    Patient *pat = Datas::I()->patients->getById(listeactes.at(0)->idPatient(), Item::LoadDetails);
    if (pat == Q_NULLPTR)
        return;
    RecalcCurrentDateTime();
    if (focusWidget() != Q_NULLPTR)
        focusWidget()->clearFocus();      //!> Valide les changements dans les champs du dossier en cours d'affichage
    int taillefont  = 8;
    QString Reponse =        "<html><head><meta name=\"qrichtext\" content=\"1\" />"
                             "<style type=\"text/css\">"
                             "p {margin-top:0px; margin-bottom:0px;margin-left: 0px; font-size:" + QString::number(taillefont) + "pt}, li { white-space: pre-wrap; }"
                             "</style>"
                             "</head><body>";

    QString Age;
    QMap<QString,QVariant>  AgeTotal = Utils::CalculAge(pat->datedenaissance(), m_currentdate, pat->sexe());
    Age = AgeTotal["toString"].toString();
    Reponse += "<p><font color = \"" COULEUR_TITRES "\"><b>" + pat->nom() + " " + pat->prenom() + "</font> - " + Age + "</b> (" + QLocale::system().toString(pat->datedenaissance(),tr("d MMM yyyy")) + ")</p>";                   //DDN
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
            datedebut = QLocale::system().toString(act->date(),tr("d MMM yyyy"));
        if (act == listeactes.last())
            datefin = QLocale::system().toString(act->date(),tr("d MMM yyyy"));
        if (act->motif() != ""
            || act->texte() != ""
            || act->conclusion() != "")
        {
            User *usr = Datas::I()->users->getById(act->idUserSuperviseur());
            QString titre (usr? usr->titre() + " " + usr->prenom() + " " + usr->nom() : "null");
            reponsevide = false;
            Reponse += "<p><td width=\"140\"><font color = \"" COULEUR_TITRES "\" ><u><b>" + QLocale::system().toString(act->date(),tr("d MMMM yyyy")) +"</b></u></font></td>"
                    "<td width=\"400\">"
                    + titre + "</td></p>";
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
   QString  textcorps, textentete, textpied;

   //!------------------------------------------------------------------------------------------ création de l'entête
   User *userEntete = Datas::I()->users->getById(currentuser()->idparent());
   if (!userEntete)
   {
       QList<User*> listusers;
       foreach (User* usr, *Datas::I()->users->actifs())
           if (usr->isSoignant())
               listusers << usr;
       if (listusers.size()>1)
       {
           UpDialog *dlg_askcorrespondant   = new UpDialog(this);
           dlg_askcorrespondant             ->setWindowModality(Qt::WindowModal);

           dlg_askcorrespondant             ->AjouteLayButtons(UpDialog::ButtonCancel | UpDialog::ButtonOK);
           UpLabel     *label               = new UpLabel(dlg_askcorrespondant);
           QFontMetrics fm                  = QFontMetrics(qApp->font());
           int largeurcolonne               = 0;
           const QString lbltxt             = tr("Quel soignant pour l'entête?");
           int largfinal                    = fm.horizontalAdvance(lbltxt);
           int hauteurligne                 = int(fm.height()*1.1);

           label       ->setText(lbltxt);
           label       ->setAlignment(Qt::AlignCenter);

           UpComboBox *Combo = new UpComboBox();
           Combo->setContentsMargins(0,0,0,0);
           Combo->setFixedHeight(34);
           Combo->setEditable(false);
           foreach (User* usr, listusers)
               Combo->addItem(usr->login(), usr->id());
           dlg_askcorrespondant->dlglayout()   ->insertWidget(0,Combo);
           dlg_askcorrespondant->dlglayout()   ->insertWidget(0,label);

           for (int i=0; i<Combo->count(); i++)
           {
               if (fm.horizontalAdvance(Combo->itemText(i)) > largeurcolonne)
                   largeurcolonne = fm.horizontalAdvance(Combo->itemText(i));
           }
           if ((largeurcolonne + 40 + 2) > largfinal)
               largfinal = largeurcolonne + 40 + 2;
           label       ->setFixedWidth(largfinal);
           label       ->setFixedHeight(hauteurligne + 2);
           dlg_askcorrespondant->dlglayout()   ->insertWidget(0,Combo);
           dlg_askcorrespondant->dlglayout()   ->insertWidget(0,label);

           dlg_askcorrespondant->dlglayout()   ->setSizeConstraint(QLayout::SetFixedSize);

           connect(dlg_askcorrespondant->OKButton,   &QPushButton::clicked, dlg_askcorrespondant, [&]
           {
               for (int i=0; i< Combo->count(); i++)
               {
                   int idusr = Combo->currentData().toInt();
                   userEntete = Datas::I()->users->getById(idusr);
               }
               if (userEntete)
                   dlg_askcorrespondant->accept();
           });

           if (dlg_askcorrespondant->exec() != QDialog::Accepted)
               return;
           delete dlg_askcorrespondant;
           dlg_askcorrespondant = Q_NULLPTR;
       }
       else if (listusers.size()==1)
           userEntete = listusers.at(0);
       else
       {
           UpMessageBox::Watch(this,tr("Pas de soignant retrouvé pour la liste d'actes"));
           return;
       }
   }
   textentete = proc->CalcEnteteImpression(m_currentdate, userEntete).value("Norm");
   if (textentete == "") return;
   textentete.replace("{{TITRE1}}"             , "");
   QString comment;
   if (toutledossier)
       comment = tr("COMPTE RENDU DE DOSSIER");
   else if (listeactes.size() > 1)
       comment = tr("Actes du") + " " + datedebut + tr("au") + " " + datefin;
   else
       comment = tr("Acte du") + " " + datedebut;
   textentete.replace("{{TITRE}}"              , "<font color = \"" COULEUR_TITRES "\">" + comment + "</font>");
   textentete.replace("{{PRENOM PATIENT}}"     , pat->prenom());
   textentete.replace("{{NOM PATIENT}}"        , pat->nom().toUpper());
   textentete.replace("{{DDN}}"                , "(" + QLocale::system().toString(pat->datedenaissance(),tr("d MMM yyyy")) + ")");


   //!-------------------------------------------------------------------------------------- création du pied
   textpied = proc->CalcPiedImpression(userEntete);
   textpied.replace("{{DUPLI}}","");
   if (textpied == "") return;

   //!--------------------------------------------------------------------------------------- creation du corps de l'impression
   textcorps = "<html>"
           "<body LANG=\"fr-FR\" DIR=\"LTR\">"
           "<p><div align=\"justify\">"
           "<span style=\"font-size:9pt\">"
           "{{TEXTE ORDO}}"
           "</span>"
           "</div></p>"
           "</body></html>";
   QRegularExpression reg;
   reg.setPattern("font-size( *: *[\\d]{1,2} *)pt");
   Reponse.replace(reg,"font-size:" + QString::number(taillefont) + "pt");
   QString largeurformule = LARGEUR_FORMULE;
   Reponse.replace("<td width=\"" LARGEUR_FORMULE "\">","<td width=\"" + QString::number(largeurformule.toInt() - 40) + "\">");
   textcorps.replace("{{TEXTE ORDO}}",Reponse);

   bool aa = false;
   if (queLePdf)
   {
       QString dirname     = ( nomdossier == ""? QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).at(0) : nomdossier);
       QString dossier     = ( dirname == QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).at(0)? tr("sur le bureau"): tr("dans le dossier ") + QDir::toNativeSeparators(nomdossier));
       QString filename    = (listeactes.size()>1? tr("Actes") : tr("Acte")) + " - " + pat->nom() + " " + pat->prenom() + " - " + tr("du ") + datedebut + tr(" au ") + datefin + ".pdf";
       QString msgOK       = tr("fichier") +" " + QDir::toNativeSeparators(filename) + "\n" +
                             tr ("sauvegardé ") + dossier;
       proc                ->Cree_pdf(textcorps, textentete, textpied,
                               filename, dirname);
       QFile file          = QFile(dirname + "/" + filename);
       aa                  = file.exists();
       UpMessageBox::Watch(this, aa? tr("Enregistrement pdf") : tr("Echec enregistrement pdf"),
                           aa? msgOK : tr ("Impossible d'enregistrer le fichier ") + QDir::toNativeSeparators(filename));
   }
   else
   {
       bool     AvecDupli   = false;
       bool     AvecNumPage = true;
       aa = proc->Imprime_Etat(this, textcorps, textentete, textpied,
                              proc->TaillePieddePage(), proc->TailleEnTete(), proc->TailleTopMarge(),
                              AvecDupli, AvecNumPage);
   }
   if (aa)
   {
       QHash<QString, QVariant> listbinds;
       int id = currentuser()->id();
       listbinds[CP_IDUSER_DOCSEXTERNES]        = id;
       listbinds[CP_IDPAT_DOCSEXTERNES]         = pat->id();
       listbinds[CP_TYPEDOC_DOCSEXTERNES]       = COURRIER;
       listbinds[CP_SOUSTYPEDOC_DOCSEXTERNES]   = (queLePdf? tr("Export") : tr("Impression")) + " " + (toutledossier? tr("dossier"): tr("actes"));
       listbinds[CP_TITRE_DOCSEXTERNES]         = (queLePdf? tr("Export") : tr("Impression")) + " " + (toutledossier? tr("dossier"): tr("actes"));
       listbinds[CP_TEXTENTETE_DOCSEXTERNES]    = textentete;
       listbinds[CP_TEXTCORPS_DOCSEXTERNES]     = textcorps;
       listbinds[CP_TEXTPIED_DOCSEXTERNES]      = textpied;
       listbinds[CP_DATE_DOCSEXTERNES]          = m_currentdate.toString("yyyy-MM-dd") + " " + m_currenttime.toString("HH:mm:ss");
       listbinds[CP_IDEMETTEUR_DOCSEXTERNES]    = currentuser()->id();
       listbinds[CP_EMISORRECU_DOCSEXTERNES]    = "0";
       listbinds[CP_FORMATDOC_DOCSEXTERNES]     = COURRIER;
       listbinds[CP_IDLIEU_DOCSEXTERNES]        = Datas::I()->sites->idcurrentsite();
       if(!db->InsertSQLByBinds(TBL_DOCSEXTERNES, listbinds))
           UpMessageBox::Watch(this,tr("Impossible d'enregistrer ce document dans la base!"));
       ui->OuvreDocsExternespushButton->setEnabled(true);
   }
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
    datastream >> col >> row >> dataMap;
    int idpat = dataMap.value(0).toInt();
    InscritEnSalDat(Datas::I()->patients->getById(idpat));
}

bool Rufus::InscritEnSalDat(Patient *pat)
{
    if (pat == Q_NULLPTR)
        return false;
    PatientEnCours *patcours = Q_NULLPTR;
    for (auto it = Datas::I()->patientsencours->patientsencours()->constBegin(); it != Datas::I()->patientsencours->patientsencours()->constEnd(); ++it)
    {
        PatientEnCours *patcrs = const_cast<PatientEnCours*>(it.value());
        if (patcrs->id() == pat->id())
        {
            patcours = patcrs;
            break;
        }
    }
    if (patcours != Q_NULLPTR)
    {
        if (patcours->statut() == ARRIVE)
        UpMessageBox::Information(this, tr("Patient déjà inscrit en salle d'attente"));
        return false;
    }
    else
    {
        //créer une fiche avec la liste des checkbox
        RendezVous* rdv = MotifRDV(pat->id());
        if (rdv == Q_NULLPTR)
            return false;
        Datas::I()->patientsencours->CreationPatient(pat->id(),                                                 //! idPat
                                                Datas::I()->users->getById(rdv->idsuperviseur()),               //! User
                                                ARRIVE,                                                         //! Statut
                                                QTime(),                                                        //! heureStatut
                                                rdv->heurerdv(),                                                //! heureRDV
                                                db->ServerDateTime().time(),                                    //! heureArrivee
                                                rdv->motif(),                                                   //! Motif
                                                rdv->message(),                                                 //! Message
                                                0,                                                              //! idActeAPayer
                                                "",                                                             //! PosteExamen
                                                0,                                                              //! idUserEnCoursExamen
                                                0);                                                             //! idSalDat
        Flags::I()->MAJFlagSalleDAttente();
        RecaleTableView(pat);
        delete rdv;
    }
    return true;
}


void Rufus::ListeCorrespondants()
{
    if (Datas::I()->correspondants->correspondants()->size()==0)
    {
        UpMessageBox::Watch(this, tr("pas de correspondant enregistré") );
        bool onlydoctors    = false;
        dlg_identificationcorresp *Dlg_IdentCorresp = new dlg_identificationcorresp(dlg_identificationcorresp::Creation, onlydoctors, Q_NULLPTR, this);
        Dlg_IdentCorresp->exec();
        delete Dlg_IdentCorresp;
        return;
    }
    dlg_listecorrespondants *Dlg_ListCor = new dlg_listecorrespondants(this);
    Dlg_ListCor->exec();
    delete Dlg_ListCor;
}

void Rufus::ListeIOLs()
{
    bool quelesactifs = false;
    dlg_listeiols *Dlg_ListIOLs = new dlg_listeiols(quelesactifs, this);
    Dlg_ListIOLs->exec();
    delete Dlg_ListIOLs;
}

void Rufus::ListeManufacturers()
{
    if (Datas::I()->manufacturers->manufacturers()->size()==0)
    {
        UpMessageBox::Watch(this, tr("pas de fournisseur enregistré") );
        dlg_identificationmanufacturer *Dlg_IdentManufacturer    = new dlg_identificationmanufacturer(dlg_identificationmanufacturer::Creation, Q_NULLPTR, this);
        Dlg_IdentManufacturer->exec();
        delete Dlg_IdentManufacturer;
        return;
    }
    dlg_listemanufacturers *Dlg_ListManufacturers = new dlg_listemanufacturers(this);
    Dlg_ListManufacturers->exec();
    delete Dlg_ListManufacturers;
}

void Rufus::ListeTiersPayants()
{
    if (Datas::I()->tierspayants->tierspayants()->size()==0)
    {
        UpMessageBox::Watch(this, tr("pas de tiers payant enregistré") );
        dlg_identificationtiers *Dlg_IdentTiers    = new dlg_identificationtiers(dlg_identificationtiers::Creation, Q_NULLPTR, this);
        Dlg_IdentTiers->exec();
        delete Dlg_IdentTiers;
        return;
    }
    dlg_listetiers *Dlg_ListTiers = new dlg_listetiers(this);
    Dlg_ListTiers->exec();
    delete Dlg_ListTiers;
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
    if (m_menuContextuel != Q_NULLPTR)
        delete m_menuContextuel;
    m_menuContextuel = new QMenu(this);
    QAction *pAction_IdentPatient = m_menuContextuel->addAction("Modifier les données patients") ;
    connect (pAction_IdentPatient, &QAction::triggered, this, &Rufus::ChoixMenuContextuelIdentPatient);

    // ouvrir le menu
    m_menuContextuel->exec(cursor().pos());
    delete m_menuContextuel;
    m_menuContextuel = Q_NULLPTR;
}

void Rufus::ChoixMenuContextuelIdentPatient()
{
    IdentificationPatient(dlg_identificationpatient::Modification,currentpatient());  // aussi appelé depuis le bouton ModifIdentificationupSmallButton
}

void Rufus::MenuContextuelMotsCles()
{
    if (m_menuContextuel != Q_NULLPTR)
        delete m_menuContextuel;
    m_menuContextuel = new QMenu(this);
    QAction *pAction_ModifMotCle = m_menuContextuel->addAction(tr("Modifier les mots clés"));
    connect (pAction_ModifMotCle, &QAction::triggered, this, &Rufus::ChoixMenuContextuelMotsCles);

    // ouvrir le menu
    m_menuContextuel->exec(cursor().pos());
    delete m_menuContextuel;
    m_menuContextuel = Q_NULLPTR;
}

void Rufus::ChoixMenuContextuelMotsCles()
{
    if (currentpatient() == Q_NULLPTR)
        return;
    dlg_listemotscles *ListMCDialog = new dlg_listemotscles(this);
    if (ListMCDialog->exec() == QDialog::Accepted)
    {
        QList<int> listMC = ListMCDialog->listMCDepart();
        if (listMC.size()>0)
        {
            db->StandardSQL("delete from " TBL_MOTSCLESJOINTURES " where idpat = " + QString::number(currentpatient()->id()));
            QString req = "insert into " TBL_MOTSCLESJOINTURES " (idpat, idmotcle) values ";
            req += "(" + QString::number(currentpatient()->id()) + ", " + QString::number(listMC.at(0)) + ")";
            for (int j=1; j<listMC.size(); j++)
                req += ", (" + QString::number(currentpatient()->id()) + ", " + QString::number(listMC.at(j)) + ")";
            db->StandardSQL(req);
            db->StandardSQL("delete from " TBL_MOTSCLESJOINTURES " where idMotCle not in (select idmotcle from " TBL_MOTSCLES ")");
        }
    }
    CalcMotsCles(currentpatient());
    delete ListMCDialog;
}

void Rufus::RechercheParID()
{
    if (!Datas::I()->patients->isfull())
        FiltreTable();
    UpDialog *dlg_rechParId       = new UpDialog();
    UpLabel         *idlabel      = new UpLabel(dlg_rechParId, tr("id du patient"));
    UpLineEdit      *idLine       = new UpLineEdit(dlg_rechParId);
    wdg_nomlbl                    = new UpLabel(dlg_rechParId);
    idLine                        ->setMaxLength(8);
    idLine                        ->setValidator((new QIntValidator(1,99999999)));
    dlg_rechParId->dlglayout()    ->insertWidget(0,idlabel);
    dlg_rechParId->dlglayout()    ->insertWidget(1,idLine);
    dlg_rechParId->dlglayout()    ->insertWidget(2,wdg_nomlbl);
    dlg_rechParId                 ->AjouteLayButtons(UpDialog::ButtonCancel | UpDialog::ButtonOK);
    connect(idLine,                   &QLineEdit::textEdited,   this,           [=] {CherchePatientParID(idLine->text().toInt());});
    connect(dlg_rechParId->OKButton,  &QPushButton::clicked,    dlg_rechParId,  [=] {ui->CreerPrenomlineEdit->clear(); dlg_rechParId->accept();});
    dlg_rechParId->exec();
    delete dlg_rechParId;
}

void Rufus::RechercheParMotCle()
{
    if (Datas::I()->motscles->motscles()->size() == 0)
    {
        UpMessageBox::Watch(this, tr("Aucun mot clé défini dans la base"), tr("Recherche impossible"));
        return;
    }

    dlg_rechParMotCle                 = new UpDialog(this);
    QTableView      *tabMC            = new QTableView(dlg_rechParMotCle);
    dlg_rechParMotCle->dlglayout()    ->insertWidget(0,tabMC);
    dlg_rechParMotCle                 ->AjouteLayButtons(UpDialog::ButtonCancel | UpDialog::ButtonOK);
    dlg_rechParMotCle                 ->addSearchLine();
    connect(dlg_rechParMotCle->OKButton,        &QPushButton::clicked,  this, &Rufus::AfficheDossiersRechercheParMotCle);
    connect(dlg_rechParMotCle->CancelButton,    &QPushButton::clicked,  dlg_rechParMotCle, &QDialog::reject);
    connect(dlg_rechParMotCle->searchline(),    &QLineEdit::textEdited, this,   [=] (QString txt) { txt = Utils::trimcapitilize(txt, false, true);
                                                                                                    dlg_rechParMotCle->searchline()->setText(txt);
                                                                                                    QStandardItemModel *model = qobject_cast<QStandardItemModel*>(tabMC->model());
                                                                                                    if (model)
                                                                                                    for (int i=0; i<model->rowCount();++i)
                                                                                                    {
                                                                                                        QStandardItem *itm = model->item(i);
                                                                                                        if (itm->text().startsWith(txt))
                                                                                                        {
                                                                                                            QModelIndex idx = itm->index();
                                                                                                            tabMC->scrollTo(idx, QAbstractItemView::PositionAtCenter);
                                                                                                            tabMC->setCurrentIndex(idx);
                                                                                                            tabMC->selectRow(i);
                                                                                                            break;
                                                                                                        }
                                                                                                    }
                                                                                                    });

    dlg_rechParMotCle ->setWindowTitle(tr("Recherche de patients par mots clés"));

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

    UpStandardItem      *pitem;
    QStandardItemModel *model =  new QStandardItemModel;
    for (auto it =  Datas::I()->motscles->motscles()->constBegin(); it !=  Datas::I()->motscles->motscles()->constEnd(); ++it)
    {
        MotCle *mc = const_cast<MotCle*>(it.value());
        pitem   = new UpStandardItem(mc->motcle(), mc);
        pitem   ->setEditable(false);
        pitem   ->setCheckable(true);
        model  ->appendRow(pitem);
    }
    model->sort(0);
    tabMC       ->setModel(model);
    for (int i=0; i<model->rowCount(); i++)
        tabMC   ->setRowHeight(i,hauteurligne);
    tabMC       ->setColumnWidth(0,300);
    tabMC       ->setFixedWidth(tabMC->columnWidth(0)+2);
    dlg_rechParMotCle->setFixedWidth(tabMC->width()
                        + dlg_rechParMotCle->dlglayout()->contentsMargins().left()*2);
    dlg_rechParMotCle->searchline()->setFocus();
    dlg_rechParMotCle->exec();
    delete dlg_rechParMotCle;
    dlg_rechParMotCle = Q_NULLPTR;
}

void Rufus::AfficheDossiersRechercheParMotCle()
{
    QList<QVariant> listidMc;
    QStandardItemModel *model = qobject_cast<QStandardItemModel*>(dlg_rechParMotCle->findChildren<QTableView *>().at(0)->model());
    if (model==Q_NULLPTR)
        return;
    for (int i=0; i< model->rowCount(); i++)
        if(model->item(i,0)->checkState() == Qt::Checked)
        {
            UpStandardItem *pitem = dynamic_cast<UpStandardItem*>(model->item(i,0));
            if (pitem)
            {
                MotCle *mc = qobject_cast<MotCle*>(pitem->item());
                if (mc)
                    listidMc << mc->id();
            }
        }
    if (listidMc.size()==0)
    {
        UpMessageBox::Watch(this, tr("Vous n'avez sélectionné aucun mot clé"));
        return;
    }
    QString req = "select mcjoin.idPat, PatNom, PatPrenom, PatDDN, motcle"
                  " from " TBL_MOTSCLESJOINTURES " as mcjoin"
                  " left outer join " TBL_PATIENTS " pat"
                  " on mcjoin.idPat = pat." CP_IDPAT_PATIENTS
                  " left outer join " TBL_MOTSCLES " msc"
                  " on mcjoin.idmotcle = msc.idmotcle"
                  " where mcjoin.idmotcle in (";
    req += listidMc.at(0).toString();
    for (int i=1; i<listidMc.size(); i++)
        req += ", " + listidMc.at(i).toString();
    req += ") order by " CP_NOM_PATIENTS ", " CP_PRENOM_PATIENTS;
    QList<QVariantList> listpats = db->StandardSelectSQL(req, m_ok);
    if (!m_ok || listpats.size()==0)
    {
        UpMessageBox::Watch(this, tr("Aucun patient retrouvé pour ces critères"));
        return;
    }
    dlg_listPatients = new UpDialog(this);
    QTableView      *tabMC              = new QTableView(dlg_listPatients);
    dlg_listPatients->dlglayout()       ->insertWidget(0,tabMC);
    dlg_listPatients->AjouteLayButtons(UpDialog::ButtonPrint | UpDialog::ButtonOK);
    dlg_listPatients->PrintButton   ->setdata(listidMc);
    connect(dlg_listPatients->OKButton,     &QPushButton::clicked,   dlg_listPatients,  &QDialog::accept);
    connect(dlg_listPatients->PrintButton,  &QPushButton::clicked,   this,              [=] {ImprimeListPatients(dlg_listPatients->PrintButton->data());});

    dlg_listPatients->setSizeGripEnabled(false);
    dlg_listPatients->setWindowTitle(tr("Recherche de patients par mots clés"));

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
        pitem1  = new QStandardItem(QLocale::system().toString(listpats.at(i).at(3).toDate(),tr("dd-MMM-yyyy")));
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
    dlg_listPatients->setFixedWidth(tabMC->width()
                        + dlg_listPatients->dlglayout()->contentsMargins().left()*2);
    dlg_listPatients->exec();
    delete dlg_listPatients;
    dlg_listPatients = Q_NULLPTR;
}

void Rufus::AfficheCourriersAFaire()
{
    QString req = "select " CP_ID_ACTES ", act." CP_IDPAT_ACTES ", " CP_NOM_PATIENTS ", " CP_PRENOM_PATIENTS ", " CP_DATE_ACTES
                  " from " TBL_ACTES " as act"
                  " left outer join " TBL_PATIENTS " pat"
                  " on act." CP_IDPAT_ACTES " = pat." CP_IDPAT_PATIENTS
                  " where " CP_COURRIERAFAIRE_ACTES " = 'T' or " CP_COURRIERAFAIRE_ACTES " = '1' and act." CP_IDUSER_ACTES " = " + QString::number(currentuser()->id()) + " order by patnom, patprenom";
    QList<QVariantList> listcourriers = db->StandardSelectSQL(req, m_ok);
    if (!m_ok || listcourriers.size()==0)
    {
        UpMessageBox::Watch(this, tr("Pas de courrier en attente"));
        return;
    }
    dlg_listPatients = new UpDialog(this);
    QTableView      *tabCourriers              = new QTableView();
    dlg_listPatients->dlglayout()       ->insertWidget(0,tabCourriers);
    dlg_listPatients->AjouteLayButtons(UpDialog::ButtonClose);
    connect(dlg_listPatients->CloseButton,     &QPushButton::clicked,   dlg_listPatients,   &QDialog::accept);

    UpLabel *lbl = new UpLabel();
    lbl->setText(tr("Double clic ou clic droit\nsur un dossier pour l'ouvrir"));
    lbl->setAlignment(Qt::AlignCenter);
    dlg_listPatients->AjouteWidgetLayButtons(lbl, false);

    dlg_listPatients->setSizeGripEnabled(false);
    dlg_listPatients->setWindowTitle(tr("Liste des courriers en attene"));

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
        pitem1  = new QStandardItem(QLocale::system().toString(listcourriers.at(i).at(4).toDate(),tr("dd-MMM-yyyy")));                  // Date de l'acte
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
    dlg_listPatients->setFixedWidth(tabCourriers->width()
                        + dlg_listPatients->dlglayout()->contentsMargins().left()*2);
    connect(tabCourriers, &QWidget::customContextMenuRequested, dlg_listPatients, [=]
    {
        QPoint tbpos    = tabCourriers->mapFromGlobal(cursor().pos());
        QModelIndex idx = tabCourriers->indexAt(tbpos);
        QString patient = modele->item(modele->itemFromIndex(idx)->row(),0)->text();
        if (m_menuContextuel != Q_NULLPTR)
            delete m_menuContextuel;
        m_menuContextuel = new QMenu(this);
        QAction *pAction_OuvrirDossier = m_menuContextuel->addAction("Ouvrir le dossier " + patient) ;
        connect (pAction_OuvrirDossier, &QAction::triggered, this, [=]
        {
            int idacte      = modele->itemFromIndex(idx)->accessibleDescription().toInt();
            int idPat       = modele->item(modele->itemFromIndex(idx)->row(),2)->text().toInt();
            OuvrirDossier(Datas::I()->patients->getById(idPat), idacte);
            dlg_listPatients->close();
        });
        m_menuContextuel->exec(cursor().pos());
        delete m_menuContextuel;
        m_menuContextuel = Q_NULLPTR;
    });

    connect(tabCourriers, &QAbstractItemView::doubleClicked, dlg_listPatients, [=]
    {
        QModelIndexList mdlist = tabCourriers->selectionModel()->selectedIndexes();
        if(mdlist.size()>0)
        {
            QModelIndex idx = mdlist.at(0);
            int idacte      = modele->itemFromIndex(idx)->accessibleDescription().toInt();
            int idPat       = modele->item(modele->itemFromIndex(idx)->row(),2)->text().toInt();
            OuvrirDossier(Datas::I()->patients->getById(idPat), idacte);
            dlg_listPatients->close();
        }
    });
    dlg_listPatients->exec();
    delete dlg_listPatients;
    dlg_listPatients = Q_NULLPTR;
}

void Rufus::ImprimeListPatients(QVariant var)
{
    QVariantList listidMc = var.toList();
    if (listidMc.size()==0)
        return;
    bool AvecDupli   = false;
    bool AvecNumPage = true;

    //--------------------------------------------------------------------
    // Préparation de l'état "liste patients" dans un QplainTextEdit
    //--------------------------------------------------------------------
    QString req1 = "select motcle from " TBL_MOTSCLES " where idmotcle in(";
    req1 += listidMc.at(0).toString();
    for (int i=1; i<listidMc.size(); i++)
        req1 += ", " +listidMc.at(i).toString();
    req1 += ")";
    QList<QVariantList> titrlist = db->StandardSelectSQL(req1,m_ok);
    QString titre = tr("recherche de patients sur ");
    titre += titrlist.at(0).at(0).toString();
    for (int i=1; i<titrlist.size(); i++)
        titre += ", " + titrlist.at(i).at(0).toString();
    QString req = "select mcjoin.idPat, PatNom, PatPrenom, PatDDN, motcle"
                  " from " TBL_MOTSCLESJOINTURES " as mcjoin"
                  " left outer join " TBL_PATIENTS " pat"
                  " on mcjoin.idPat = pat." CP_IDPAT_PATIENTS
                  " left outer join " TBL_MOTSCLES " msc"
                  " on mcjoin.idmotcle = msc.idmotcle"
                  " where mcjoin.idmotcle in (";
    req += listidMc.at(0).toString();
    for (int i=1; i<listidMc.size(); i++)
        req += ", " +listidMc.at(i).toString();
    req += ") order by " CP_NOM_PATIENTS ", " CP_PRENOM_PATIENTS;
    QList<QVariantList> patlist = db->StandardSelectSQL(req,m_ok);
    int     gtotalNbreDossiers    = patlist.size();
    QDate   date = QDate::currentDate();

    //création de l'entête
    QString textentete;
    User *userEntete = Datas::I()->users->getById(currentuser()->idparent());
    if (userEntete == Q_NULLPTR)
        return;
    textentete = proc->CalcEnteteImpression(date, userEntete).value("Norm");
    if (textentete == "") return;
    textentete.replace("{{TITRE1}}"            , "");
    textentete.replace("{{PRENOM PATIENT}}"    , "");
    textentete.replace("{{NOM PATIENT}}"       , "");
    textentete.replace("{{TITRE}}"             , titre);
    textentete.replace("{{DDN}}"               , "<font color = \"" COULEUR_TITRES "\">" + QString::number(gtotalNbreDossiers)
                   + " " + (gtotalNbreDossiers>1? tr("dossiers") : tr("dosssier")) + "</font>");
    // création du pied
    QString textpied = proc->CalcPiedImpression(userEntete);
    if (textpied == "") return;

    // creation du corps
    QString ligne = "<table width=\"490\" border=\"0\" cellspacing=\"0\" cellpadding=\"5\">"
                    "<tr>"
                    "<td width=\"150\" ><span style=\"font-size:9pt;\">{{NOM PATIENT}}</span></td>"
                    "<td width=\"100\" ><span style=\"font-size:9pt;\">{{DDN}}</span></td>"
                    "<td width=\"210\" ><span style=\"font-size:9pt;\">{{MOTIF}}</span></td>"
                    "</tr>"
                    "</table>";

    QString textecorps, lignepat;
    for (int k = 0; k < patlist.size(); k++)
    {
        // Remplacement des variables par les valeurs lues.
        lignepat = ligne;
        lignepat.replace("{{NOM PATIENT}}", patlist.at(k).at(1).toString() + " " + patlist.at(k).at(2).toString());
        lignepat.replace("{{DDN}}", QLocale::system().toString(patlist.at(k).at(3).toDate(),tr("dd-MMM-yyyy")));
        lignepat.replace("{{MOTIF}}", patlist.at(k).at(4).toString());
        textecorps += lignepat;
     }

    proc->Imprime_Etat(this, textecorps, textentete, textpied,
                       proc->TaillePieddePage(), proc->TailleEnTete(), proc->TailleTopMarge(),
                       AvecDupli, AvecNumPage);
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
    if (UpText->iD() == 0)
        return;
    int idpat = UpText->iD();
    if( currentuser()->isSoignant() )
    {
        if (m_menuContextuel != Q_NULLPTR)
            delete m_menuContextuel;
        m_menuContextuel = new QMenu(this);
        {
            QAction *pAction_ReprendreDossier = m_menuContextuel->addAction(tr("Visualiser le dossier"));
            connect (pAction_ReprendreDossier,  &QAction::triggered,    this,   [=] {ChoixMenuContextuelListePatients(idpat, "Autre Dossier");});
        }
        // ouvrir le menu
        m_menuContextuel->exec(cursor().pos());
        delete m_menuContextuel;
        m_menuContextuel = Q_NULLPTR;
    }
    else if( currentuser()->isSecretaire() )
    {
        if (m_menuContextuel != Q_NULLPTR)
            delete m_menuContextuel;
        m_menuContextuel = new QMenu(this);
        {
            QAction *pAction_ModifierDossier = m_menuContextuel->addAction(tr("Modifier les données de ce patient"));
            connect (pAction_ModifierDossier,   &QAction::triggered,    this,   [=] {ChoixMenuContextuelListePatients(idpat, "Modifier");});
        }
        // ouvrir le menu
        m_menuContextuel->exec(cursor().pos());
        delete m_menuContextuel;
        m_menuContextuel = Q_NULLPTR;
    }
}
void Rufus::MenuContextuelListePatients()
{
    Patient *pat = getPatientFromCursorPositionInTable();
    if (pat == Q_NULLPTR)
        return;
    int idpat = pat->id();
    bool iscurrentpatient =  false;
    if (currentpatient() != Q_NULLPTR)
        if (pat->id() == currentpatient()->id())
            iscurrentpatient = true;


    if (m_menuContextuel != Q_NULLPTR)
        delete m_menuContextuel;
    m_menuContextuel = new QMenu(this);

    if (!iscurrentpatient)
    {
        QAction *pAction_MettreEnSalDat = m_menuContextuel->addAction(tr("Inscrire ce patient en salle d'attente"));
        connect (pAction_MettreEnSalDat,        &QAction::triggered,    this,    [=] {ChoixMenuContextuelListePatients(idpat, "SalDat");});
        QAction *pAction_ModifierDossier = m_menuContextuel->addAction(tr("Modifier les données de ce patient"));
        connect (pAction_ModifierDossier,       &QAction::triggered,    this,    [=] {ChoixMenuContextuelListePatients(idpat, "Modifier");});

        QAction *pAction_Copier = m_menuContextuel->addAction(tr("Créer un dossier de la même famille"));
        connect (pAction_Copier,                &QAction::triggered,    this,    [=] {ChoixMenuContextuelListePatients(idpat, "Copie");});

    }

    if( currentuser()->isSoignant() && !iscurrentpatient)
    {
        QAction *pAction_ReprendreDossier = m_menuContextuel->addAction(tr("Visualiser le dossier"));
        connect (pAction_ReprendreDossier,  &QAction::triggered,    this,    [=] {ChoixMenuContextuelListePatients(idpat, "Autre Dossier");});
    }
    if( (currentuser()->isSoignant() || currentuser()->isSecretaire()) && Datas::I()->users->medecins()->size() > 0 )    {
        QAction *pAction_PrgIntervention = m_menuContextuel->addAction(tr("Programmer une intervention"));
        connect (pAction_PrgIntervention,  &QAction::triggered,    this,    [=] {ChoixMenuContextuelListePatients(idpat, "Intervention");});
    }
    if (!iscurrentpatient)
    {
        QAction *pAction_EmettreDoc = m_menuContextuel->addAction(tr("Emettre un document"));
        connect (pAction_EmettreDoc,            &QAction::triggered,    this,    [=] {ChoixMenuContextuelListePatients(idpat, "Document");});
        if (DataBase::I()->countRecords(TBL_DOCSEXTERNES, CP_IDPAT_DOCSEXTERNES " = " + QString::number(pat->id())) >0)
        {
            QAction *pAction_ImprimeDoc = m_menuContextuel->addAction(tr("Réimprimer un document"));
            connect (pAction_ImprimeDoc,        &QAction::triggered,    this,    [=] {ChoixMenuContextuelListePatients(idpat, "ImprimeAncienDoc");});
        }
    }
    if (currentuser()->isSecretaire() || currentuser()->isSoignant())
    {
        QAction *pAction_ImprimDoss = m_menuContextuel->addAction(tr("Exporter le dossier du patient"));
        connect (pAction_ImprimDoss,    &QAction::triggered,    this,    [=] {ChoixMenuContextuelListePatients(idpat, "ExportDoss");});
    }
    QAction *pAction_EnregDoc = m_menuContextuel->addAction(tr("Enregistrer un document scanné"));
    connect (pAction_EnregDoc,              &QAction::triggered,    this,    [=] {ChoixMenuContextuelListePatients(idpat, "EnregDocScan");});

    QAction *pAction_EnregVideo = m_menuContextuel->addAction(tr("Enregistrer une video"));
    connect (pAction_EnregVideo,            &QAction::triggered,    this,    [=] {ChoixMenuContextuelListePatients(idpat, "EnregVideo");});

    QAction *pAction_SendMess = m_menuContextuel->addAction(tr("Envoyer un message"));
    connect (pAction_SendMess,              &QAction::triggered,    this,    [=] {ChoixMenuContextuelListePatients(idpat, "SendMess");});

    // ouvrir le menu
    m_menuContextuel->exec(cursor().pos());
    delete m_menuContextuel;
    m_menuContextuel = Q_NULLPTR;
}

void Rufus::ChoixMenuContextuelListePatients(int idpat, QString choix)
{
    if (idpat == 0)
        return;
    Patient *pat = Datas::I()->patients->getById(idpat, Item::LoadDetails);
    Datas::I()->patients->setdossierpatientaouvrir(pat);
    if (pat == Q_NULLPTR)
        return;
    if (choix == "Autre Dossier")
    {
        dlg_actesprecedents *Dlg_ActesPrecs = new dlg_actesprecedents(pat, this);
        if (!Dlg_ActesPrecs->initOK())
        {
            UpMessageBox::Watch(this, tr("Pas de consultation enregistrée pour ") + dossierpatientaouvrir()->prenom() + " " + dossierpatientaouvrir()->nom());
            delete Dlg_ActesPrecs;
            return;
        }
        else
        {
            Dlg_ActesPrecs  ->exec();
            delete Dlg_ActesPrecs;
        }
    }
    else if (choix == "SalDat")
        InscritEnSalDat(dossierpatientaouvrir());                                                   //! depuis menu contextuel ListePatients
    else if (choix == "Copie")
        RecopierDossier(dossierpatientaouvrir());                                                   //! depuis menu contextuel ListePatients
    else if (choix == "Modifier")
        IdentificationPatient(dlg_identificationpatient::Modification,dossierpatientaouvrir());     //! depuis menu contextuel ListePatients
    else if (choix == "Document")
        ImprimeDocument(dossierpatientaouvrir());
    else if (choix == "ImprimeAncienDoc") {
        DocsExternes *docs = new DocsExternes;
        docs->initListeByPatient(dossierpatientaouvrir());
        OuvrirDocsExternes(docs);                                                                //! depuis menu contextuel ListePatients
        ItemsList::clearAll(docs->docsexternes());
        delete docs;
    }
    else if (choix == "EnregDocScan")
        EnregistreDocScanner(dossierpatientaouvrir());                                              //! depuis menu contextuel ListePatients
    else if (choix == "ExportDoss")
        ImprimeDossier(dossierpatientaouvrir(), true);                                              //! depuis menu contextuel ListePatients
    else if (choix == "EnregVideo")
        EnregistreVideo(dossierpatientaouvrir());                                                   //! depuis menu contextuel ListePatients
    else if (choix == "Intervention")
        ProgrammationIntervention(dossierpatientaouvrir());                                         //! depuis menu contextuel ListePatients
    else if (choix == "SendMess")
    {
        QMap<QString, QVariant> map;
        map["null"] = true;
        SendMessage(map, dossierpatientaouvrir()->id());                                            //! depuis menu contextuel ListePatients
    }
}

void Rufus::MenuContextuelMedecin()
{
    if (ui->MGupComboBox->findText(ui->MGupComboBox->currentText()) != -1)
    {
        if (m_menuContextuel != Q_NULLPTR)
            delete m_menuContextuel;
        m_menuContextuel = new QMenu(this);
        QAction *pAction_IdentPatient = m_menuContextuel->addAction(tr("Modifier les coordonnées de ce médecin"));
        connect (pAction_IdentPatient,      &QAction::triggered,    this,    &Rufus::ChoixMenuContextuelMedecin);

        // ouvrir le menu
        m_menuContextuel->exec(cursor().pos());
        delete m_menuContextuel;
        m_menuContextuel = Q_NULLPTR;
    }
}

void Rufus::ChoixMenuContextuelMedecin()
{
    int id = ui->MGupComboBox->currentData().toInt();
    bool onlydoctors = true;
    dlg_identificationcorresp *Dlg_IdentCorresp = new dlg_identificationcorresp(dlg_identificationcorresp::Modification, onlydoctors, Datas::I()->correspondants->getById(id, Item::LoadDetails), this);
    if (Dlg_IdentCorresp->exec() == QDialog::Accepted)
        if (Dlg_IdentCorresp->identcorrespondantmodifiee())
            ui->MGupComboBox->setCurrentIndex(ui->MGupComboBox->findData(id));
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
        if (m_menuContextuel != Q_NULLPTR)
            delete m_menuContextuel;
        m_menuContextuel = new QMenu(this);
        QAction *pAction_IdentPatient = m_menuContextuel->addAction(tr("Modifier les coordonnées de ce correspondant"));
        connect (pAction_IdentPatient,      &QAction::triggered,    this,   [=] {ChoixMenuContextuelCorrespondant(choix);});

        // ouvrir le menu
        m_menuContextuel->exec(cursor().pos());
        delete m_menuContextuel;
        m_menuContextuel = Q_NULLPTR;
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
    dlg_identificationcorresp *Dlg_IdentCorresp = new dlg_identificationcorresp(dlg_identificationcorresp::Modification, onlydoctors, Datas::I()->correspondants->getById(id, Item::LoadDetails));
    if (Dlg_IdentCorresp->exec() == QDialog::Accepted)
    {
        int idCor = Dlg_IdentCorresp->idcurrentcorrespondant();
        if (choix == "Modifier1")
            ui->AutresCorresp1upComboBox->setCurrentIndex(ui->AutresCorresp1upComboBox->findData(idCor));
        else if (choix == "Modifier2")
            ui->AutresCorresp2upComboBox->setCurrentIndex(ui->AutresCorresp2upComboBox->findData(idCor));
        OKModifierTerrain(currentpatient());
    }
    delete Dlg_IdentCorresp;
}

void Rufus::MenuContextuelSalDat(UpLabel *labelClicked)
{
    if (currentuser()->isNeutre())
        return;
    int idpat (0);
    if (labelClicked == Q_NULLPTR) return;
    idpat = labelClicked->iD();
    int row = labelClicked->Row();

    if (m_menuContextuel != Q_NULLPTR)
        delete m_menuContextuel;
    m_menuContextuel = new QMenu(this);

    if (ui->SalleDAttenteupTableWidget->isAncestorOf(labelClicked))
    {
        UpLabel *StatutClicked = qobject_cast<UpLabel *> (ui->SalleDAttenteupTableWidget->cellWidget(row,1));
        if (StatutClicked != Q_NULLPTR)
        {
            if (StatutClicked->text() == ARRIVE)
            {
                QAction *pAction_RetirerDossier = m_menuContextuel->addAction(tr("Retirer ce dossier de la salle d'attente"));
                connect (pAction_RetirerDossier, &QAction::triggered,   this,    [=] {ChoixMenuContextuelSalDat(idpat, "Retirer");});
            }
        }
    }
    QAction *pAction_ModifierDossier = m_menuContextuel->addAction(tr("Modifier les données de ce patient"));
    connect (pAction_ModifierDossier,           &QAction::triggered,    this,    [=] {ChoixMenuContextuelSalDat(idpat, "Modifier");});

    if (ui->SalleDAttenteupTableWidget->isAncestorOf(labelClicked))
    {
        QAction *pAction_ModifierMotif = m_menuContextuel->addAction(tr("Modifier le motif de l'acte"));
        connect (pAction_ModifierMotif,         &QAction::triggered,    this,    [=] {ChoixMenuContextuelSalDat(idpat, "Motif");});
    }

    QAction *pAction_Copier = m_menuContextuel->addAction(tr("Créer un dossier de la même famille"));
    connect (pAction_Copier,                    &QAction::triggered,    this,    [=] {ChoixMenuContextuelSalDat(idpat, "Copie");});
    if( currentuser()->isSoignant() )
    {
        QAction *pAction_OuvrirDossier = m_menuContextuel->addAction(tr("Ouvrir le dossier"));
        connect (pAction_OuvrirDossier,         &QAction::triggered,    this,    [=] {ChoixMenuContextuelSalDat(idpat, "Ouvrir");});
    }
    QAction *pAction_EmettreDoc = m_menuContextuel->addAction(tr("Emettre un document"));
    connect (pAction_EmettreDoc,                &QAction::triggered,    this,    [=] {ChoixMenuContextuelSalDat(idpat, "Document");});

    // ouvrir le menu
    m_menuContextuel->exec(cursor().pos());
    delete m_menuContextuel;
    m_menuContextuel = Q_NULLPTR;
}

void Rufus::MenuContextuelAccueil(UpLabel *labelClicked)
{
    if (currentuser()->isNeutre())
        return;
     QList<QTableWidgetSelectionRange> listRange = ui->AccueilupTableWidget->selectedRanges();
    if (labelClicked == Q_NULLPTR)
        return;
    int idpat = labelClicked->iD();
    PatientEnCours *pat = Q_NULLPTR;
    for (auto it = Datas::I()->patientsencours->patientsencours()->constBegin(); it != Datas::I()->patientsencours->patientsencours()->constEnd(); ++it)
    {
        if (idpat== it.key())
            if (const_cast<PatientEnCours*>(it.value()) != Q_NULLPTR)
                    pat = const_cast<PatientEnCours*>(it.value());
    }
    if (pat == Q_NULLPTR)
        return;
    bool a = false;
    // si le label qui émet la demande de menu n'est pas dans la plage sélectionnée, on n'affiche pas de menu
    for (int i = 0; i< listRange.size();i++)
        if (listRange.at(i).topRow() <= labelClicked->Row() && listRange.at(i).bottomRow() >= labelClicked->Row())
        {a = true;  break;}
    if (a == false) return;


    if (m_menuContextuel != Q_NULLPTR)
        delete m_menuContextuel;
    m_menuContextuel = new QMenu(this);

    if (listRange.size() == 1 && listRange.at(0).rowCount()== 1)
    {
        if( currentuser()->isSoignant() )
        {
            QAction *pAction_ReprendreDossier = m_menuContextuel->addAction(tr("Reprendre le dossier"));
            connect (pAction_ReprendreDossier,  &QAction::triggered,    this,   [=] {ChoixMenuContextuelSalDat(idpat, "Reprendre");});
        }
        QAction *pAction_EmettreDoc = m_menuContextuel->addAction(tr("Emettre un document"));
        connect (pAction_EmettreDoc,            &QAction::triggered,    this,   [=] {ChoixMenuContextuelSalDat(idpat, "Document");});
        if( (currentuser()->isSoignant() || currentuser()->isSecretaire()) && Datas::I()->users->medecins()->size() > 0 )    {
            QAction *pAction_PrgIntervention = m_menuContextuel->addAction(tr("Programmer une intervention"));
            connect (pAction_PrgIntervention,  &QAction::triggered,    this,    [=] {ChoixMenuContextuelSalDat(idpat, "Intervention");});
        }
    }
    if (currentuser()->isSecretaire() || labelClicked->datas().value("idComptable").toInt() == currentuser()->idcomptableactes())
    {
        QAction *pAction_EnregistrePaiement = m_menuContextuel->addAction(tr("Enregistrer le paiement"));
        connect (pAction_EnregistrePaiement,    &QAction::triggered,    this,   [=] {ChoixMenuContextuelSalDat(idpat, "Payer");});
    }

    // ouvrir le menu
    m_menuContextuel->exec(cursor().pos());
    delete m_menuContextuel;
    m_menuContextuel = Q_NULLPTR;
}

void Rufus::ChoixMenuContextuelSalDat(int idpat, QString choix)
{
    if (idpat == 0)
        return;
    Patient *pat = Datas::I()->patients->getById(idpat, Item::LoadDetails);
    Datas::I()->patients->setdossierpatientaouvrir(pat);
    if (pat == Q_NULLPTR)
        return;
    if (choix == "Reprendre")
        OuvrirDossier(dossierpatientaouvrir());
    else if (choix == "Payer")
        AppelPaiementDirect(Accueil);
    else if (choix == "Modifier")
        IdentificationPatient(dlg_identificationpatient::Modification, dossierpatientaouvrir());  //appelé depuis le menu contextuel de la table salle d'attente
    else if (choix == "Ouvrir")
        OuvrirDossier(dossierpatientaouvrir());
    else if (choix == "Retirer" || choix == "Fermer")
    {
        Datas::I()->patientsencours->SupprimePatientEnCours(Datas::I()->patientsencours->getById(dossierpatientaouvrir()->id()));
        Flags::I()->MAJFlagSalleDAttente();
    }
    else if (choix == "Copie")
        RecopierDossier(dossierpatientaouvrir());
    else if (choix == "Document")
        ImprimeDocument(dossierpatientaouvrir());
    else if (choix == "Intervention")
        ProgrammationIntervention(dossierpatientaouvrir());                                         //! depuis menu contextuel ListePatients
    else if (choix == "Motif")  //! il s'agit de modifier le motif de la consultation - la patient est dans la  salle d'attente, on a son id, il suffit de le retrouver sans passer par SQL
    {
        PatientEnCours *patencours = Q_NULLPTR;
        for (auto it = Datas::I()->patientsencours->patientsencours()->constBegin(); it != Datas::I()->patientsencours->patientsencours()->constEnd(); ++it)
        {
            if (idpat == it.key())
            {
                patencours = const_cast<PatientEnCours*>(it.value());
                break;
            }
        }
        if (patencours == Q_NULLPTR)
            return;
        QString Message = patencours->message();
        QString Motif   = patencours->motif();
        QTime heurerdv  = patencours->heurerdv();

        RendezVous *rdv = MotifRDV(patencours->id(), Motif, Message, heurerdv);
        if (rdv == Q_NULLPTR)
            return;
        ItemsList::update(patencours, CP_MOTIF_SALDAT, rdv->motif());
        ItemsList::update(patencours, CP_MESSAGE_SALDAT, rdv->message());
        ItemsList::update(patencours, CP_HEURERDV_SALDAT, rdv->heurerdv());
        ItemsList::update(patencours, CP_IDUSERSUPERVISEUR_SALDAT, rdv->idsuperviseur());
        Flags::I()->MAJFlagSalleDAttente();
        delete rdv;
    }
}


RendezVous* Rufus::MotifRDV(int idpatient, QString motif, QString Message, QTime heurerdv)
{
    //créer une fiche avec tous les checkbox correspondant aux motifs de RDV : Cs, OCT, CV, BO, Biométrie, Urgence, Angio,...etc...
    RendezVous *rdv = Q_NULLPTR;
    if (Datas::I()->motifs->motifs()->size()==0)
        return rdv;
    UpDialog        *dlg_ask            = new UpDialog(this);
    QVBoxLayout     *motiflayout        = new QVBoxLayout();
    UpLabel         *lbltitre           = new UpLabel(dlg_ask);
    UpLabel         *lblsoignt          = new UpLabel(dlg_ask);
    QTextEdit       *MsgText            = new QTextEdit(dlg_ask);
    QGroupBox       *grpBox             = new QGroupBox(dlg_ask);
    QTimeEdit       *HeureRDV           = new QTimeEdit(dlg_ask);
    UpComboBox      *ComboSuperviseurs  = new UpComboBox(dlg_ask);
    UpLabel         *HeureTitre         = new UpLabel(dlg_ask);
    grpBox          ->setTitle(tr("Motif de l'acte"));
    int width       = 380;

    for (auto it =  Datas::I()->users->superviseurs()->constBegin(); it !=  Datas::I()->users->superviseurs()->constEnd(); ++it)
    {
        User *usr = const_cast<User*>(it.value());
        ComboSuperviseurs->addItem(usr->login(), QString::number(usr->id()));
    }
    ComboSuperviseurs->setFixedWidth(100);

    QHBoxLayout *soignantlayout     = new QHBoxLayout();
    lblsoignt->setText(tr("RDV avec"));
    soignantlayout->addWidget(lblsoignt);
    soignantlayout->addWidget(ComboSuperviseurs);
    soignantlayout->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding));


    int defaut = -1;
    int k = -1;
    for (auto it = Datas::I()->motifs->motifs()->constBegin(); it != Datas::I()->motifs->motifs()->constEnd(); ++it)
    {
        Motif *mtf = const_cast<Motif*>(it.value());
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
        grpBox->findChildren<QRadioButton*>().at(i)->setGeometry(5,30+(25*i),(width/2)-20,20);
    for (int j=a;j<n;j++)
        grpBox->findChildren<QRadioButton*>().at(j)->setGeometry((width/2)-20,30+(25*(j-a)),(width/2)-20,20);

    grpBox      ->setFixedHeight((a*30)+20);

    lbltitre    ->setText(tr("Message"));
    MsgText     ->setFixedHeight(40);
    HeureTitre  ->setText(tr("Heure de RDV"));
    HeureRDV    ->setTime(heurerdv);
    HeureRDV    ->setMinimumSize(Utils::qtimeeditsize());
    HeureRDV    ->setCurrentSection(QDateTimeEdit::MinuteSection);
    dlg_ask     ->AjouteWidgetLayButtons(HeureRDV, false);
    dlg_ask     ->AjouteWidgetLayButtons(HeureTitre, false);
    dlg_ask     ->AjouteLayButtons(UpDialog::ButtonCancel | UpDialog::ButtonOK);
    dlg_ask     ->setStageCount(1);

    motiflayout ->addLayout(soignantlayout);
    motiflayout ->addWidget(grpBox);
    motiflayout ->addWidget(lbltitre);
    motiflayout ->addWidget(MsgText);
    dlg_ask->dlglayout()->insertLayout(0,motiflayout);

    connect(dlg_ask->OKButton,   &QPushButton::clicked,  dlg_ask,   &QDialog::accept);

    dlg_ask->setWindowTitle(tr("Enregistrer le motif de l'acte"));

    dlg_ask->setModal(true);
    dlg_ask->setFixedWidth(width);
    dlg_ask->dlglayout()->setSizeConstraint(QLayout::SetFixedSize);
    MsgText->setText(Message);
    if (dlg_ask->exec() == QDialog::Accepted)
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
            for (auto it = Datas::I()->motifs->motifs()->constBegin(); it != Datas::I()->motifs->motifs()->constEnd(); ++it)
            {
                Motif *mtf = const_cast<Motif*>(it.value());
                if (mtf->motif()==motif)
                {
                    motif = mtf->raccourci();
                    break;
                }
            }
        }
        rdv = new RendezVous;
        rdv->setmotif(motif);
        rdv->setmessage(Message);
        rdv->setheurerdv(HeureRDV->time());
        rdv->setidsuperviseur(ComboSuperviseurs->currentData().toInt());
        rdv->setidpatient(idpatient);
        rdv->setheurearrivee(QTime::currentTime());
    }
    delete dlg_ask;
    dlg_ask = Q_NULLPTR;
    return rdv;
}


void Rufus::MAJPosteConnecte()
{
    // On en profite au passage pour sauvegarder la position de la fenêtre principale
    //bug Qt? -> cette ligne de code ne peut pas être mise juste avant exit(0) sinon elle n'est pas éxécutée...
    proc->settings()->setValue(Position_Fiche Nom_fiche_Rufus, saveGeometry());
    if (currentpost() != Q_NULLPTR)
    {
        ItemsList::update(currentpost(), CP_HEUREDERNIERECONNECTION_USRCONNECT, db->ServerDateTime());
    }
    else
    {
        Datas::I()->postesconnectes->CreationPosteConnecte(currentuser(), Datas::I()->sites->idcurrentsite());
        Flags::I()->MAJFlagSalleDAttente();
    }
}

void Rufus::ModifCotationActe()
{
    m_autorModifConsult = true;
    ui->Cotationframe->setEnabled(true);
    ui->CCAMlinklabel->setVisible(true);
    ui->ActeMontantlineEdit->setFocus();
    ui->ActeMontantlineEdit->selectAll();
}

void Rufus::ModifierTerrain()
{
    ui->TerraintreeWidget->setVisible(false);
    ui->ModifTerrainwidget->setVisible(true);
    ui->TabacLabel->setVisible(true);
    ui->TabaclineEdit->setVisible(true);
    ui->OKModifTerrainupSmallButton->setUpButtonStyle(UpSmallButton::STARTBUTTON);
    ui->OKModifTerrainupSmallButton->disconnect();
    connect (ui->OKModifTerrainupSmallButton,   &QPushButton::clicked,  this,   [=] {OKModifierTerrain(currentpatient());});
}

void Rufus::OKModifierTerrain(Patient *pat, bool recalclesdonnees) // recalcule le ui->TerraintreeWidget et l'affiche
{
    if (pat == Q_NULLPTR)
        return;
    if (recalclesdonnees)
        Datas::I()->patients->loadAll(pat, Item::Update);
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
            if (fm.horizontalAdvance(listhash.at(i)) > (ui->TerraintreeWidget->width() - ui->TerraintreeWidget->columnWidth(0)))
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
            if (fm.horizontalAdvance(txt) > (ui->TerraintreeWidget->width() - ui->TerraintreeWidget->columnWidth(0)))
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
            if (fm.horizontalAdvance(txt) > (ui->TerraintreeWidget->width() - ui->TerraintreeWidget->columnWidth(0)))
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
            if (fm.horizontalAdvance(txt) > (ui->TerraintreeWidget->width() - ui->TerraintreeWidget->columnWidth(0)))
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
            if (fm.horizontalAdvance(hash) > (ui->TerraintreeWidget->width() - ui->TerraintreeWidget->columnWidth(0)))
                pit->setToolTip(1, hash);
        }
    }
    ui->TerraintreeWidget->setVisible(a);
    ui->ModifTerrainwidget->setVisible(!a);
    ui->TabacLabel->setVisible(!a);
    ui->TabaclineEdit->setVisible(!a);
    ui->OKModifTerrainupSmallButton->setUpButtonStyle(a? UpSmallButton::EDITBUTTON : UpSmallButton::STARTBUTTON);

    ui->OKModifTerrainupSmallButton->disconnect();
    connect (ui->OKModifTerrainupSmallButton,   &QPushButton::clicked,  this,   [=] {a? ModifierTerrain() : OKModifierTerrain(currentpatient());});
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
    dlg_depenses *Dlg_Deps = new dlg_depenses(this);
    if(Dlg_Deps->initOK())
    {
        Dlg_Deps->ui->GestionComptesupPushButton->setVisible(actionGestionComptesBancaires->isVisible());
        Dlg_Deps->exec();
    }
    if (Dlg_Deps->msgretour() != "")
        UpMessageBox::Watch(this, "", Dlg_Deps->msgretour());
    delete Dlg_Deps;
}

void Rufus::OuvrirParametres()
{
    //TODO : SQL
    dlg_param *Dlg_Param = new dlg_param(this);
    Dlg_Param->setWindowTitle(tr("Paramètres"));
    Dlg_Param->exec();
    if (Dlg_Param->DataUserModifiees())
        setWindowTitle("Rufus - " + currentuser()->login() + " - " + currentuser()->fonction());
    if (Dlg_Param->CotationsModifiees())
    {

        QString req = "insert into " TBL_COTATIONS " ( " CP_TYPEACTE_COTATIONS ", " CP_MONTANTOPTAM_COTATIONS ", " CP_MONTANTNONOPTAM_COTATIONS ", " CP_MONTANTPRATIQUE_COTATIONS ", "
                CP_CODECCAM_COTATIONS ", " CP_IDUSER_COTATIONS ", " CP_TIP_COTATIONS ") values \n";
        for (int i=0; i<Dlg_Param->ui->ActesCCAMupTableWidget->rowCount(); i++)
        {
            UpCheckBox *check = qobject_cast<UpCheckBox*>(Dlg_Param->ui->ActesCCAMupTableWidget->cellWidget(i,0));
            if (check != Q_NULLPTR)
                if (check->isChecked())
                {

                    QString typeacte, montantOPTAM, montantNonOPTAM, montantprat("");
                    typeacte        = Dlg_Param->ui->ActesCCAMupTableWidget->item(i,1)->text();
                    montantOPTAM    = QString::number(QLocale().toDouble(Dlg_Param->ui->ActesCCAMupTableWidget->item(i,2)->text()));
                    montantNonOPTAM = QString::number(QLocale().toDouble(Dlg_Param->ui->ActesCCAMupTableWidget->item(i,3)->text()));
                    QString mtconv  = (currentuser()->isOPTAM() ? montantOPTAM : montantNonOPTAM);
                    if (Dlg_Param->ui->ActesCCAMupTableWidget->columnCount()==6)
                    {
                        UpLineEdit *line = qobject_cast<UpLineEdit*>(Dlg_Param->ui->ActesCCAMupTableWidget->cellWidget(i,5));
                        if (line != Q_NULLPTR)
                            montantprat = (line->text()!=""? line->text() : mtconv);
                        else
                            montantprat = mtconv;
                    }
                    QString mtprat = (currentuser()->secteurconventionnel() >1 ? montantprat : mtconv);
                    QString montantpratique = QString::number(QLocale().toDouble(mtprat));
                    req += "('" + typeacte +  "', " + montantOPTAM + "," + montantNonOPTAM + "," + montantpratique + ", 1, " + QString::number(currentuser()->id()) + ", null),\n";
                }
        }
        for (int i=0; i<Dlg_Param->ui->AssocCCAMupTableWidget->rowCount(); i++)
        {
            UpCheckBox *check = qobject_cast<UpCheckBox*>(Dlg_Param->ui->AssocCCAMupTableWidget->cellWidget(i,0));
            if (check != Q_NULLPTR)
                if (check->isChecked())
                {
                    QString codeCCAM, montantOPTAM(""), montantNonOPTAM(""), montantprat(""), tip ("");
                    codeCCAM        = Dlg_Param->ui->AssocCCAMupTableWidget->item(i,1)->text();
                    UpLineEdit *lineOPTAM = qobject_cast<UpLineEdit*>(Dlg_Param->ui->AssocCCAMupTableWidget->cellWidget(i,2));
                    if (lineOPTAM != Q_NULLPTR)
                    {
                        montantOPTAM    = QString::number(QLocale().toDouble(lineOPTAM->text()));
                        tip             = lineOPTAM->datas().toString();
                    }
                    UpLineEdit *lineNonOPTAM = qobject_cast<UpLineEdit*>(Dlg_Param->ui->AssocCCAMupTableWidget->cellWidget(i,3));
                    if (lineNonOPTAM != Q_NULLPTR)
                        montantNonOPTAM    = QString::number(QLocale().toDouble(lineNonOPTAM->text()));
                    QString mtconv  = (currentuser()->isOPTAM() ? montantOPTAM : montantNonOPTAM);
                    if (Dlg_Param->ui->AssocCCAMupTableWidget->columnCount()==5)
                    {
                        UpLineEdit *line = qobject_cast<UpLineEdit*>(Dlg_Param->ui->AssocCCAMupTableWidget->cellWidget(i,4));
                        if (line != Q_NULLPTR)
                            montantprat = (line->text()!=""? line->text() : mtconv);
                        else
                            montantprat = mtconv;
                    }
                    QString mtprat = (currentuser()->secteurconventionnel() >1? montantprat : mtconv);
                    QString montantpratique = QString::number(QLocale().toDouble(mtprat));
                    req += "('" + codeCCAM +  "', " + montantOPTAM + "," + montantNonOPTAM + "," + montantpratique + ", 2, " + QString::number(currentuser()->id()) + ", '" + tip + "'),\n";
                }
        }
        for (int i=0; i<Dlg_Param->ui->HorsNomenclatureupTableWidget->rowCount(); i++)
        {
            UpCheckBox *check = qobject_cast<UpCheckBox*>(Dlg_Param->ui->HorsNomenclatureupTableWidget->cellWidget(i,0));
            if (check != Q_NULLPTR)
                if (check->isChecked())
                {
                    QString codeCCAM, mtconv(""), montantconv, tip("");
                    codeCCAM = Dlg_Param->ui->HorsNomenclatureupTableWidget->item(i,1)->text();
                    UpLineEdit *lineconv = qobject_cast<UpLineEdit*>(Dlg_Param->ui->HorsNomenclatureupTableWidget->cellWidget(i,2));
                    if (lineconv != Q_NULLPTR)
                    {
                        mtconv  = lineconv->text();
                        tip     = lineconv->datas().toString();
                    }
                    montantconv = QString::number(QLocale().toDouble(mtconv));
                    req += "('" + codeCCAM +  "', " + montantconv + "," + montantconv + "," + montantconv + ", 3, " + QString::number(currentuser()->id()) + ", '" + tip + "'), \n";
                }
        }
        req = req.left(req.lastIndexOf(")")+1);
        //proc->Edit(req);
        QString reqDel = "delete from " TBL_COTATIONS " where " CP_IDUSER_COTATIONS " = " + QString::number(currentuser()->id());
        db->StandardSQL(reqDel);
        db->StandardSQL(req);
        ReconstruitListesCotations();
    }
    delete Dlg_Param;
}

void Rufus::RecettesSpeciales()
{
    dlg_recettesspeciales *Dlg_RecSpec           = new dlg_recettesspeciales(this);
    if(Dlg_RecSpec->initOK())
    {
        Dlg_RecSpec->ui->GestionComptesupPushButton->setVisible(actionGestionComptesBancaires->isVisible());
        Dlg_RecSpec->exec();
    }
    delete Dlg_RecSpec;
}

void Rufus::RetrouveMontantActe()
{
    auto retrouvecotation = [=] (User *usr, QString cotation, QString &Montant)
    {
        Cotations * cots = getListeCotationdByUser(usr);
        if (cots)
        {
            User *parent = Datas::I()->users->getById(cots->iduser());
            if (parent != Q_NULLPTR)
            {
                for (auto it = cots->cotations()->constBegin(); it != cots->cotations()->constEnd(); ++it)
                {
                    Cotation* cot = const_cast<Cotation*>(it.value());
                    if (ui->ActeCotationcomboBox->currentText() == cot->typeacte())
                    {
                        if (parent->secteurconventionnel()>1)
                        {
                            if (!currentpatient()->iscmu())
                                Montant = QLocale().toString(cot->montantpratique(),'f',2);
                            else
                                Montant = QLocale().toString(cot->montantconventionnel(),'f',2);
                        }
                        else
                            Montant = QLocale().toString(cot->montantconventionnel(),'f',2);
                        ui->ActeMontantlineEdit->setText(Montant);
                        break;
                    }
                }
            }
            if (Montant == "0.00")
            {
                QVariantList cotdata = QVariantList();
                if (parent != Q_NULLPTR)
                {
                    QString tarifconventionne = (usr->isOPTAM() ? CP_MONTANTOPTAM_CCAM : CP_MONTANTNONOPTAM_CCAM );
                    // qDebug() << parent;
                    QString req =
                            "SELECT " + tarifconventionne + ", " CP_MONTANTPRATIQUE_COTATIONS " FROM " TBL_COTATIONS " cot, " TBL_CCAM " cc"
                                                                                                                                       " where " CP_TYPEACTE_COTATIONS " = " CP_CODECCAM_CCAM
                            " and " CP_IDUSER_COTATIONS " = " + QString::number(parent->id()) +
                            " and " CP_CODECCAM_CCAM " like '" + Utils::correctquoteSQL(cotation) + "%'";
                    // qDebug() << req;
                    cotdata = db->getFirstRecordFromStandardSelectSQL(req, m_ok);
                    if (m_ok && cotdata.size()>0)
                    {
                        if (parent->secteurconventionnel()>1 && !currentpatient()->iscmu())
                            Montant = QLocale().toString(cotdata.at(1).toDouble(),'f',2);
                        else
                            Montant = QLocale().toString(cotdata.at(0).toDouble(),'f',2);
                        ui->ActeMontantlineEdit->setText(Montant);
                    }
                }
                if (cotdata == QVariantList())
                {
                    QString req = "SELECT " CP_MONTANTOPTAM_CCAM ", " CP_MONTANTNONOPTAM_CCAM " FROM " TBL_CCAM " where " CP_CODECCAM_CCAM " like '" + Utils::correctquoteSQL(cotation) + "%'";
                    cotdata = db->getFirstRecordFromStandardSelectSQL(req, m_ok);
                    if (m_ok && cotdata.size()>0)
                    {
                        QString MontantActe;
                        if (parent->secteurconventionnel()>1 && !currentpatient()->iscmu() && !parent->isOPTAM())
                            MontantActe = QLocale().toString(cotdata.at(1).toDouble(),'f',2);
                        else
                            MontantActe = QLocale().toString(cotdata.at(0).toDouble(),'f',2);
                        ui->ActeMontantlineEdit->setText(MontantActe);
                    }                    
                    else
                    {
                        QString req =
                                "SELECT " CP_MONTANTOPTAM_COTATIONS ", " CP_MONTANTPRATIQUE_COTATIONS " FROM " TBL_COTATIONS
                                " where " CP_TYPEACTE_COTATIONS " like '" + Utils::correctquoteSQL(cotation) + "%'";
                        // qDebug() << req;
                        cotdata = db->getFirstRecordFromStandardSelectSQL(req, m_ok);
                        if (m_ok && cotdata.size()>0)
                        {
                            if (parent->secteurconventionnel()>1 && !currentpatient()->iscmu())
                                Montant = QLocale().toString(cotdata.at(1).toDouble(),'f',2);
                            else
                                Montant = QLocale().toString(cotdata.at(0).toDouble(),'f',2);
                            ui->ActeMontantlineEdit->setText(Montant);
                        }
                    }
                }
            }
            if (Montant != "0.00")
            {
                if (parent != Q_NULLPTR)
                {
                    if (parent->secteurconventionnel()>1)
                    {
                        ui->BasculerMontantpushButton->setVisible(true);
                        ui->BasculerMontantpushButton->setImmediateToolTip(tr("Revenir au tarif conventionnel"));
                    }
                }
                else
                    ui->BasculerMontantpushButton->setVisible(false);
            }
        }
    };

    if (currentpatient() == Q_NULLPTR)
        return;
    QString MontantActe = "0.00";
    QString cotation = ui->ActeCotationcomboBox->currentText();
    ui->EnregistrePaiementpushButton->setEnabled(cotation!="");
    if (cotation == GRATUIT)
        ui->ActeMontantlineEdit->setText(MontantActe);
    else
    {
        User *superviseur = Datas::I()->users->getById(currentacte()->idUserSuperviseur());
        if (superviseur)
            retrouvecotation(superviseur, cotation, MontantActe);
        else
            ui->ActeMontantlineEdit->setText(MontantActe);
    }
    if (Datas::I()->users->getById(currentacte()->idComptable()) == Q_NULLPTR)
        ItemsList::update(currentacte(),CP_IDUSERCOMPTABLE_ACTES, currentuser()->idcomptableactes());

    /*  else
   {
        UpMessageBox msgbox(this);
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
                UpMessageBox msgbox(this);
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


/*!
 * \brief Rufus::SendMessage - Affiche la fiche de rédaction des messages
 * \param map -> la map QMap<QString, QVariant> des caractéristiques du message à envoyer
 * \param id -> l'id du patient concerné
 * \param idMsg -> l'id du message - utilisé quand il s'agit de modifier un message
 */
void Rufus::SendMessage(QMap<QString, QVariant> map, int id, int idMsg){
    auto VerifMessage = [=] (UpDialog *dlg_sendMessage,int idMsg) /*! Vérifie la cohérence des messages avant de les envoyer */
    {
        if (dlg_sendMessage->findChildren<UpTextEdit*>().at(0)->toPlainText()=="")
        {
            UpSystemTrayIcon::I()->showMessage(tr("Messages"), tr("Vous avez oublié de rédiger le texte de votre message!"), Icons::icSunglasses(), 2000);
            return;
        }
        bool checkusr = false;
        for (int j=0; j< dlg_sendMessage->findChildren<UpCheckBox*>().size(); j++)
            if (dlg_sendMessage->findChildren<UpCheckBox*>().at(j)->rowTable() == 1)
            {
                checkusr = dlg_sendMessage->findChildren<UpCheckBox*>().at(j)->isChecked();
                if (checkusr)
                    break;
            }        
        if (!checkusr)
        {
            UpSystemTrayIcon::I()->showMessage(tr("Messages"), tr("Vous avez oublié de choisir un destinataire!"),Icons::icSunglasses(), 2000);
            return;
        }
        QHash<QString, QVariant> m_listbinds;
        m_listbinds[CP_IDEMETTEUR_MSG]  = currentuser()->id();
        m_listbinds[CP_TEXT_MSG]        = dlg_sendMessage->findChildren<UpTextEdit*>().at(0)->toHtml();
        int idpat = 0;
        int ncheck = dlg_sendMessage->findChildren<UpCheckBox*>().size();
        for (int i=0; i<ncheck; i++)
            if (dlg_sendMessage->findChildren<UpCheckBox*>().at(i)->objectName()=="AboutPatupCheckBox")
            {
                if (dlg_sendMessage->findChildren<UpCheckBox*>().at(i)->isChecked())
                {
                    idpat = dlg_sendMessage->findChildren<UpCheckBox*>().at(i)->iD();
                    break;
                }
            }
        m_listbinds[CP_IDPATIENT_MSG]       = (idpat > 0? idpat : QVariant());
        bool task = false;
        QDate datelimite = QDate();
        for (int i=0; i<ncheck; i++)
            if (dlg_sendMessage->findChildren<UpCheckBox*>().at(i)->objectName()=="TaskupCheckBox")
            {
                if (dlg_sendMessage->findChildren<UpCheckBox*>().at(i)->isChecked())
                {
                    task = true;
                    datelimite = dlg_sendMessage->findChildren<QDateTimeEdit*>().at(0)->date();
                    break;
                }
            }
        bool urge = false;
        for (int i=0; i<ncheck; i++)
            if (dlg_sendMessage->findChildren<UpCheckBox*>().at(i)->objectName()=="UrgeupCheckBox")
            {
                if (dlg_sendMessage->findChildren<UpCheckBox*>().at(i)->isChecked())
                {
                    urge = true;
                    break;
                }
            }
        m_listbinds[CP_TACHE_MSG]           = (task? 1 : QVariant());
        m_listbinds[CP_DATELIMITE_MSG]      = (datelimite.isValid()? datelimite.toString("yyyy-MM-dd") : QVariant());
        m_listbinds[CP_URGENT_MSG]          = (urge? 1 : QVariant());
        m_listbinds[CP_DATECREATION_MSG]    = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
        QList<int> listidusr;
        for (int j=0; j< dlg_sendMessage->findChildren<UpCheckBox*>().size(); j++)
            if (dlg_sendMessage->findChildren<UpCheckBox*>().at(j)->rowTable() == 1)       // c'est le checkbox d'un user
                if (dlg_sendMessage->findChildren<UpCheckBox*>().at(j)->isChecked())
                    listidusr << dlg_sendMessage->findChildren<UpCheckBox*>().at(j)->iD();
        if (listidusr.size() == 0)
            return;
        bool result = false;
        if (idMsg<0)  // Enregistrement d'un nouveau message
        {
            if (Datas::I()->messages->CreationMessage(m_listbinds, listidusr))
            {
                envoieTCPMessageA(listidusr);
                result = true;
            }
        }
        else  //    modification d'un message existant
            if (Datas::I()->messages->UpdateMessage(idMsg, m_listbinds, listidusr))
                result = true;
        if (result)
            UpSystemTrayIcon::I()->showMessage(tr("Messages"), tr("Message enregistré"),Icons::icSunglasses(), 1000);
        dlg_sendMessage->accept();
    };
    //TODO : SQL
    UpDialog *dlg_sendMessage       = new UpDialog(this);
    QHBoxLayout     *tasklayout;
    QHBoxLayout     *totallayout    = new QHBoxLayout();
    QVBoxLayout     *destlayout     = new QVBoxLayout();
    QVBoxLayout     *msglayout      = new QVBoxLayout();
    QVBoxLayout     *vbox           = new QVBoxLayout();

    QGroupBox       *UsrGroupBox;
    UpCheckBox      *checkpat, *checktask, *checkurg;
    UpTextEdit      *MsgText;
    QDateEdit       *limitdate;

    dlg_sendMessage->AjouteLayButtons(UpDialog::ButtonCancel | UpDialog::ButtonOK);

    /*! on prépare 2 layout verticaux et une ligne
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
        Patient *pat = Datas::I()->patients->getById(id);
        if (pat)
        {
            checkpat       = new UpCheckBox(dlg_sendMessage);
            checkpat        ->setObjectName("AboutPatupCheckBox");
            checkpat        ->setText(tr("A propos de ") + pat->nom().toUpper() + " " + pat->prenom());
            checkpat        ->setChecked(true);
            checkpat        ->setiD(id);
            msglayout       ->addWidget(checkpat);
        }
    }
    QList<User*> listactifs;
    for (auto it =  Datas::I()->users->actifs()->constBegin(); it !=  Datas::I()->users->actifs()->constEnd(); ++it)
    {
        User *usr = const_cast<User*>(it.value());
        if( usr->id() == currentuser()->id() )
            continue;
        if (usr->isadmin() || usr->isNeutre())
            continue;
        listactifs << usr;
    }
    if (m_ok && listactifs.size()>0)
    {
        UsrGroupBox = new QGroupBox(dlg_sendMessage);
        UsrGroupBox->setTitle(tr("Destinataire"));
        UpCheckBox *Allusrchk = new UpCheckBox();
        Allusrchk->setText(tr("Tout le monde"));
        Allusrchk->setAutoExclusive(false);
        Allusrchk->setRowTable(2);        
        connect(Allusrchk,  &QCheckBox::clicked,  this, [=] {   /*! tous les utilistauers sont cochés quand on coche la case tr("Tout le monde") */
                                                                for (int i=0; i< dlg_sendMessage->findChildren<UpCheckBox*>().size(); i++)
                                                                    if (dlg_sendMessage->findChildren<UpCheckBox*>().at(i)->rowTable() == 1)
                                                                        dlg_sendMessage->findChildren<UpCheckBox*>().at(i)->setChecked(Allusrchk->isChecked());
                                                            });
        vbox->addWidget(Allusrchk);
        QLabel *line = new QLabel(dlg_sendMessage);
        line->setFrameShape(QFrame::HLine);
        line->setFixedHeight(1);
        vbox->addWidget(line);
        for (int i=0; i<listactifs.size(); i++)
        {
            User* usr = listactifs.at(i);
            UpCheckBox *chk0 = new UpCheckBox();
            chk0->setText(usr->login());
            chk0->setiD(usr->id());
            if (idMsg>-1)
                chk0->setChecked(map["listdestinataires"].toStringList().contains(QString::number(usr->id())));
            chk0->setRowTable(1);            
            connect(chk0,  &QCheckBox::clicked,  this,  [=]{ /*! coche la case tr("Tout le monde") si tous les utilisateurs sont cochés, la décoche dans le cas inverse */
                                                                if (chk0->isChecked())
                                                                {
                                                                    bool allchk = true;
                                                                    for (int i=0; i< dlg_sendMessage->findChildren<UpCheckBox*>().size(); i++)
                                                                        if (dlg_sendMessage->findChildren<UpCheckBox*>().at(i)->rowTable() == 1)
                                                                        {
                                                                            allchk = dlg_sendMessage->findChildren<UpCheckBox*>().at(i)->isChecked();
                                                                            if (!allchk)
                                                                                break;
                                                                        }
                                                                    for (int j=0; j< dlg_sendMessage->findChildren<UpCheckBox*>().size(); j++)
                                                                        if (dlg_sendMessage->findChildren<UpCheckBox*>().at(j)->rowTable() == 2)
                                                                            dlg_sendMessage->findChildren<UpCheckBox*>().at(j)->setChecked(allchk);
                                                                }
                                                                else
                                                                    for (int j=0; j< dlg_sendMessage->findChildren<UpCheckBox*>().size(); j++)
                                                                        if (dlg_sendMessage->findChildren<UpCheckBox*>().at(j)->rowTable() == 2)
                                                                            dlg_sendMessage->findChildren<UpCheckBox*>().at(j)->setChecked(false);
                                                            });
            vbox->addWidget(chk0);
            chk0->setAutoExclusive(false);
        }
        UsrGroupBox->setLayout(vbox);
        destlayout ->addWidget(UsrGroupBox);
        destlayout ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Minimum,QSizePolicy::Expanding));
    }
    else
    {
        delete vbox;
        delete totallayout;
        delete destlayout;
        delete msglayout;
        delete dlg_sendMessage;
        return;
    }

    MsgText        = new UpTextEdit(dlg_sendMessage);
    MsgText         ->setFixedHeight(140);
    if (idMsg>-1)
        MsgText->setText(map[CP_TEXT_MSG].toString());
    msglayout       ->setContentsMargins(5,0,5,0);

    tasklayout     = new QHBoxLayout();
    limitdate      = new QDateEdit(dlg_sendMessage);
    checktask      = new UpCheckBox(dlg_sendMessage);
    checkurg       = new UpCheckBox(dlg_sendMessage);
    checktask       ->setObjectName("TaskupCheckBox");
    checkurg        ->setObjectName("UrgeupCheckBox");
    limitdate       ->setFixedHeight(26);
    limitdate       ->setFixedWidth(120);
    limitdate       ->setCalendarPopup(true);
    limitdate       ->setEnabled(false);
    limitdate       ->setDate(QDate::currentDate());
    if (idMsg>-1)
    {
        if (map[CP_TACHE_MSG].toInt()==1)
        {
            checktask->setChecked(true);
            limitdate->setDate(map[CP_DATELIMITE_MSG].toDate());
            limitdate->setEnabled(true);
        }
        checkurg->setChecked(map[CP_URGENT_MSG].toInt()==1);
    }
    checktask       ->setText(tr("Tâche à accomplir avant le "));
    checkurg        ->setText(tr("Urgent"));    
    connect(checktask,  &QCheckBox::clicked,  this, [=] {dlg_sendMessage->findChildren<QDateEdit*>().at(0)->setEnabled(checktask->isChecked());});
    tasklayout      ->addWidget(checktask);
    tasklayout      ->addWidget(limitdate);
    tasklayout      ->addSpacerItem(new QSpacerItem(10,10,QSizePolicy::Expanding));
    tasklayout      ->setContentsMargins(5,0,0,0);

    msglayout       ->addWidget(checkurg);
    msglayout       ->addWidget(MsgText);
    msglayout       ->addLayout(tasklayout);

    dlg_sendMessage->OKButton   ->setiD(idMsg);
    connect(dlg_sendMessage->OKButton,   &UpSmallButton::clickedint,  this,    [=] {VerifMessage(dlg_sendMessage, idMsg);});

    totallayout->addLayout(destlayout);
    QLabel *Vline = new QLabel(dlg_sendMessage);
    Vline->setFrameShape(QFrame::VLine);
    Vline->setFixedWidth(1);
    totallayout->addWidget(Vline);
    totallayout->addLayout(msglayout);
    dlg_sendMessage->dlglayout()->insertLayout(0,totallayout);

    dlg_sendMessage            ->setWindowTitle(tr("Envoyer un message"));
    dlg_sendMessage            ->setFixedWidth(510);
    dlg_sendMessage->dlglayout()->setSizeConstraint(QLayout::SetFixedSize);
    dlg_sendMessage->exec();
    delete dlg_sendMessage;
}

void Rufus::AfficheMessageImport(QStringList listmsg, int pause)
{
    ShowMessage::I()->SplashMessage(listmsg, pause);
}

void Rufus::setTitre()
{
    QString modeconnexion = "";
    if (db->ModeAccesDataBase() == Utils::Poste)
        modeconnexion = tr("monoposte");
    else if (db->ModeAccesDataBase() == Utils::ReseauLocal)
        modeconnexion = tr("réseau local");
    if (db->ModeAccesDataBase() == Utils::Distant)
    {
        modeconnexion = tr("accès distant - connexion ");
        if (proc->settings()->value(Utils::getBaseFromMode(Utils::Distant) + Param_SSL).toString() != "NO")
            modeconnexion += tr("cryptée (SSL)");
        else
            modeconnexion += tr("non cryptée");
    }
    QString windowtitle = "Rufus - " + currentuser()->login() + " - " + currentuser()->fonction() + " - " + modeconnexion + " - " + qApp->applicationVersion();
    if (db->ModeAccesDataBase() != Utils::Distant)
        windowtitle +=  (m_utiliseTCP? " - TCP" : "");
    setWindowTitle(windowtitle);
}

void Rufus::SurbrillanceSalDat(UpLabel *lab)
{
    QString styleurg = "background:#EEFFFF ; color: red";
    QString Msg, background;
    QString backgroundsurbrill = "background:#B2D7FF";
    if (lab==Q_NULLPTR)
        return;
    int id = lab->iD();
    PatientEnCours *pat = Q_NULLPTR;
    for (auto it = Datas::I()->patientsencours->patientsencours()->constBegin(); it != Datas::I()->patientsencours->patientsencours()->constEnd(); ++it)
    {
        if (id== it.key())
            if (const_cast<PatientEnCours*>(it.value()) != Q_NULLPTR)
                    pat = const_cast<PatientEnCours*>(it.value());
    }
    if (pat == Q_NULLPTR)
        return;
    int row         = lab->Row();
    QString color   = "color: black";
    QString colorRDV= "color: black";
    UpLabel *lab0   = qobject_cast<UpLabel*>(ui->SalleDAttenteupTableWidget->cellWidget(row,0));
    UpLabel *lab1   = qobject_cast<UpLabel*>(ui->SalleDAttenteupTableWidget->cellWidget(row,1));
    UpLabel *lab2   = qobject_cast<UpLabel*>(ui->SalleDAttenteupTableWidget->cellWidget(row,2));
    UpLabel *lab3   = qobject_cast<UpLabel*>(ui->SalleDAttenteupTableWidget->cellWidget(row,3));
    UpLabel *lab4   = qobject_cast<UpLabel*>(ui->SalleDAttenteupTableWidget->cellWidget(row,4));
    UpLabel *lab5   = qobject_cast<UpLabel*>(ui->SalleDAttenteupTableWidget->cellWidget(row,5));
    UpLabel *lab6   = qobject_cast<UpLabel*>(ui->SalleDAttenteupTableWidget->cellWidget(row,6));
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
            UpLabel *labi0   = qobject_cast<UpLabel*>(ui->SalleDAttenteupTableWidget->cellWidget(i,0));
            if (labi0->styleSheet().contains(backgroundsurbrill))       // l'enregistrement est en surbrillance, on le remet à la normale
            {
                QString Msgi;
                UpLabel *labi0   = qobject_cast<UpLabel*>(ui->SalleDAttenteupTableWidget->cellWidget(i,0));
                UpLabel *labi1   = qobject_cast<UpLabel*>(ui->SalleDAttenteupTableWidget->cellWidget(i,1));
                UpLabel *labi2   = qobject_cast<UpLabel*>(ui->SalleDAttenteupTableWidget->cellWidget(i,2));
                UpLabel *labi3   = qobject_cast<UpLabel*>(ui->SalleDAttenteupTableWidget->cellWidget(i,3));
                UpLabel *labi4   = qobject_cast<UpLabel*>(ui->SalleDAttenteupTableWidget->cellWidget(i,4));
                UpLabel *labi5   = qobject_cast<UpLabel*>(ui->SalleDAttenteupTableWidget->cellWidget(i,5));
                UpLabel *labi6   = qobject_cast<UpLabel*>(ui->SalleDAttenteupTableWidget->cellWidget(i,6));
                QString color2, colorRDV2;
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

/**
 * Affiche la fenêtre d'information sur Rufus
 */
void Rufus::Apropos()
{
    QString text = tr("Rufus est un logiciel libre - open source - publié sous licence GPLv3 de gestion d'activité en ophtalmologie et en orthoptie.") + "\n" +
            tr("En savoir plus sur RufusVision : ") + "https://www.rufusvision.org" + "\n\n";

#ifdef Q_OS_WIN
    QString os = "Windows";
#elif defined(Q_OS_MAC)
    QString os = "MacOS";
#elif defined(Q_OS_LINUX)
    QString os = "Linux";
#endif
    text +=
        tr("RufusVision pour ") + os + "\n"
        + tr("Version ") + qApp->applicationVersion() + "\n"
        + "\n"
        + tr("Rufus utilise MySQL ou MariaDB pour intéragir avec la base de données. Ces logiciels sont publiés sous licence GNU GPL.") + "\n";

    UpMessageBox::Information(this, tr("A propos de RufusVision"), text);
}

// utilisé par le poste importateur des documents pour supprimer les documents et les factures
// ayant été inscrits dans les tables DocsASupprimer et FacturesASupprimer
// par les autres postes

void Rufus::SupprimerDocsEtFactures()
{
    QString NomDirStockageImagerie = proc->AbsolutePathDirImagerie();

    /* Supprimer les documents en attente de suppression*/
    QString req = "delete from " TBL_DOCSASUPPRIMER " where " CP_FILEPATH_DOCSASUPPR " is null or " CP_FILEPATH_DOCSASUPPR " = \"\"" ;
    db->StandardSQL(req);
    req = "Select " CP_FILEPATH_DOCSASUPPR " from " TBL_DOCSASUPPRIMER;
    QList<QVariantList> ListeDocs = db->StandardSelectSQL(req, m_ok);
    for (int i=0; i<ListeDocs.size(); i++)
    {
        QString CheminFichier = NomDirStockageImagerie + ListeDocs.at(i).at(0).toString();
        QFile file(CheminFichier);
        if (!Utils::removeWithoutPermissions(file))
            UpMessageBox::Watch(this, tr("Fichier introuvable!"), CheminFichier);
        db->StandardSQL("delete from " TBL_DOCSASUPPRIMER " where " CP_FILEPATH_DOCSASUPPR " = '" + Utils::correctquoteSQL(ListeDocs.at(i).at(0).toString()) + "'");
    }

    /* Supprimer les factures en attente de suppression - même démarche mais on fait une copie de la facture dans le dossier FACTURESSANSLIEN avant de la supprimer*/
    QString CheminOKTransfrDir = NomDirStockageImagerie + NOM_DIR_FACTURESSANSLIEN;
    if (!Utils::mkpath(CheminOKTransfrDir))
    {
        QString msg = tr("Dossier de sauvegarde ") + "<font color=\"red\"><b>" + CheminOKTransfrDir + "</b></font>" + tr(" invalide");
        ShowMessage::I()->SplashMessage(msg, 3000);
        return;
    }
    req = "delete from " TBL_FACTURESASUPPRIMER " where " CP_LIENFICHIER_FACTASUPPR " is null or " CP_LIENFICHIER_FACTASUPPR " = \"\"" ;
    db->StandardSQL(req);
    req = "select " CP_LIENFICHIER_FACTASUPPR " from " TBL_FACTURESASUPPRIMER;
    QList<QVariantList> ListeFactures = db->StandardSelectSQL(req, m_ok);
    for (int i=0; i<ListeFactures.size(); i++)
    {
        QString lienfichier = ListeFactures.at(i).at(0).toString();
        /*  on copie le fichier dans le dossier facturessanslien*/
        QFile facturefile(NomDirStockageImagerie + NOM_DIR_FACTURES + lienfichier);
        if (facturefile.exists())
        {
            if (lienfichier.split("/").size() > 1)
            {
                QString user = lienfichier.split("/").at(1);
                QString CheminOKTransfrDirImg = CheminOKTransfrDir + "/" + user;
                if (!Utils::mkpath(CheminOKTransfrDir))
                {
                    QString msg = tr("Dossier de sauvegarde ") + "<font color=\"red\"><b>" + CheminOKTransfrDirImg + "</b></font>" + tr(" invalide");
                    ShowMessage::I()->SplashMessage(msg, 3000);
                    continue;
                }
                Utils::copyWithPermissions(facturefile, NomDirStockageImagerie + NOM_DIR_FACTURESSANSLIEN + lienfichier);
            }
            /*  on l'efface du dossier de factures*/
            Utils::removeWithoutPermissions(facturefile);
        }
        /* on détruit l'enregistrement dans la table FacturesASupprimer*/
        db->StandardSQL("delete from " TBL_FACTURESASUPPRIMER " where " CP_LIENFICHIER_FACTASUPPR " = '" + Utils::correctquoteSQL(lienfichier) + "'");
    }
}

void Rufus::AfficheBAL(int idx)
{
    if (dlg_msgBAL != Q_NULLPTR)
    {
        if (dlg_msgBAL->isVisible())
            dlg_msgBAL->close();
        delete dlg_msgBAL;
    }
    dlg_msgBAL = new QDialog(this);
    int x = 0;
    int y = 0;
    QList<QScreen*> listscreens = QGuiApplication::screens();
    if (listscreens.size())
    {
        x = listscreens.first()->geometry().width();
        y = listscreens.first()->geometry().height();
    }
    dlg_msgBAL->setStyleSheet("border-image: none; background-color:#FAFAFA;");;
    QVBoxLayout *globallay = new QVBoxLayout();
    QTabWidget* Tabw = Remplir_MsgTabWidget();
    if (Tabw->count()>idx)
        Tabw->setCurrentIndex(idx);
    Tabw->setParent(dlg_msgBAL);
    globallay->addWidget(Tabw);
    dlg_msgBAL->setLayout(globallay);
    dlg_msgBAL->setSizeGripEnabled(false);
    dlg_msgBAL->setWindowFlags(Qt::Tool | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::CustomizeWindowHint);
    dlg_msgBAL->setWindowTitle(tr("Messagerie"));
    dlg_msgBAL->setMaximumHeight(y-30);
    dlg_msgBAL->setWindowIcon(Icons::icSunglasses());
    dlg_msgBAL->move(x-470,30);
    dlg_msgBAL->setFixedWidth(500);
    dlg_msgBAL->show();
}

QTabWidget* Rufus::Remplir_MsgTabWidget()
{
    QTabWidget* tabw = new QTabWidget();
    tabw->setIconSize(QSize(25,25));
    RecalcCurrentDateTime();

    //! I - Les messages reçus ----------------------------------------------------------------------------------------------------------------------------------------------------------------------

    Datas::I()->messages->initListeMsgsrecusByIdUser(currentuser()->id());
    QList<Message*> listmessagesrecus = Datas::I()->messages->messagesrecus();
    if (listmessagesrecus.size() > 0)
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

        for (int i=0; i<listmessagesrecus.size(); i++)
        {
            Message *msg = listmessagesrecus.at(i);
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
            if (msg->isurgent())
                Titredoc->setStyleSheet("color: red");
            QString txt = msg->datecreation().toString(tr("d-MMM-yy")) + " " + msg->datecreation().toString("h:mm");
            if (msg->idemetteur() > 0)
                txt += tr(" de ") + (Datas::I()->users->getById(msg->idemetteur()) != Q_NULLPTR? Datas::I()->users->getById(msg->idemetteur())->login() : "null");
            Titredoc->setText(txt);
            titrelay->addWidget(Titredoc);
            UpCheckBox *Rdchk = new UpCheckBox();
            Rdchk->setChecked(msg->islu());
            Rdchk->setiD(msg->idjointure());
            Rdchk->setText(tr("lu"));
            Rdchk->setFixedWidth(45);            
            connect(Rdchk,  &QCheckBox::clicked,    this,   [=] {   /*! le message a été lu */
                                                                    int idjoin = Rdchk->iD();
                                                                    QString res = (Rdchk->isChecked()? "1" : "NULL");
                                                                    db->StandardSQL("update " TBL_MESSAGESJOINTURES " set " CP_LU_JOINTURESMSG " = " + res + " where " CP_ID_JOINTURESMSG " = " + QString::number(idjoin));
                                                                });
            titrelay->addSpacerItem(new QSpacerItem(10,10,QSizePolicy::Expanding));
            titrelay->addWidget(Rdchk);
            reclay->addLayout(titrelay);

            QHBoxLayout *Msglay = new QHBoxLayout();

            QVBoxLayout *Droplay = new QVBoxLayout();
            Droplay->addSpacerItem(new QSpacerItem(10,10, QSizePolicy::Expanding, QSizePolicy::Expanding));
            if (msg->idemetteur() != currentuser()->id())
            {
                UpLabel *Respondlbl = new UpLabel();
                Respondlbl->setiD(msg->id());
                Respondlbl->setPixmap(Icons::pxConversation().scaled(20,20)); //WARNING : icon scaled : pxConversation 20,20
                Respondlbl->setImmediateToolTip(tr("Répondre"));
                connect(Respondlbl,     QOverload<int>::of(&UpLabel::clicked), this, [=] {MsgResp(Respondlbl->iD());});
                Respondlbl->setFixedWidth(25);
                Droplay->addWidget(Respondlbl);
            }

            UpLabel *Dellbl = new UpLabel();
            Dellbl->setiD(msg->idjointure());
            Dellbl->setPixmap(Icons::pxPoubelle().scaled(20,20)); //WARNING : icon scaled : pxPoubelle 20,20
            Dellbl->setFixedWidth(25);
            Dellbl->setMinimumWidth(25);
            connect(Dellbl,             QOverload<int>::of(&UpLabel::clicked),  this, [=] {SupprimerMessageRecu(Dellbl->iD());});
            Droplay->addWidget(Dellbl);

            Msglay->addLayout(Droplay);

            QLabel *line = new QLabel();
            line->setFrameShape(QFrame::VLine);
            line->setFixedWidth(1);
            Msglay->addWidget(line);

            QVBoxLayout *SMlay = new QVBoxLayout();
            SMlay->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Minimum,QSizePolicy::Expanding));
            if (msg->istache())                                                                           // le message est une tâche
            {
                QHBoxLayout *Tasklay = new QHBoxLayout();
                UpLabel *Todolbl = new UpLabel();
                Todolbl->setText(tr("A effectuer avant le ") + QLocale::system().toString(msg->datelimite(),tr("d-MMM-yy")));
                if (!msg->isfait())
                {
                    if (m_currentdate >= msg->datelimite())
                        Todolbl->setStyleSheet("color: red");
                    else if (m_currentdate.addDays(3) > msg->datelimite())
                        Todolbl->setStyleSheet("color: orange");
                }
                Tasklay->addWidget(Todolbl);
                Tasklay->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding));
                UpCheckBox *Dnchk = new UpCheckBox();
                Dnchk->setChecked(msg->isfait());
                Dnchk->setiD(msg->idjointure());
                Dnchk->setText(tr("fait"));
                Dnchk->setFixedWidth(45);                
                connect(Dnchk,  &QCheckBox::clicked,    this,    [=] {  /*! la tache a été effectuée */
                                                                        int idjoin = Dnchk->iD();
                                                                        QString res = (Dnchk->isChecked()? "1" : "NULL");
                                                                        db->StandardSQL("update " TBL_MESSAGESJOINTURES " set " CP_FAIT_JOINTURESMSG " = " + res + " where " CP_ID_JOINTURESMSG " = " + QString::number(idjoin));
                                                                     });
                Tasklay->addWidget(Dnchk);
                SMlay->addLayout(Tasklay);
            }
            if (msg->idpatient()>0)                                                                            // le message concerne un patient
            {
                QString nomprenom ("");
                Patient *pat = Datas::I()->patients->getById(msg->idpatient());
                if (pat != Q_NULLPTR)
                    nomprenom = pat->prenom() + " " + pat->nom();
                if (nomprenom != "")
                {
                    QHBoxLayout *aboutlay = new QHBoxLayout();
                    UpLabel *aboutdoc = new UpLabel();
                    aboutdoc->setText(nomprenom);
                    aboutdoc->setiD(msg->idpatient());
                    aboutlay->addWidget(aboutdoc);
                    aboutlay->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding));
                    SMlay->addLayout(aboutlay);
                }
            }

            UpTextEdit *Msgtxt = new UpTextEdit();
            Msgtxt->setStyleSheet("border: 1px solid rgb(164, 205, 255);border-radius: 5px; background-color:#FFFFFF;");
            if  (msg->isurgent())
                Msgtxt->setStyleSheet("border: 2px solid rgb(251, 51, 61);border-radius: 5px; background-color:#FFFFFF;");
            Msgtxt->setText(msg->texte());
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

    //! II - Les messages emis ----------------------------------------------------------------------------------------------------------------------------------------------------------------------

    Datas::I()->messages->initListeMsgsenvoyesByIdUser(currentuser()->id());
    QList<Message*> listmessagesenvoyes = Datas::I()->messages->messagesenvoyes();
    if (listmessagesenvoyes.size() >0)
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

        for (int i=0; i<listmessagesenvoyes.size(); i++)
        {
            Message *msg = listmessagesenvoyes.at(i);
            QFrame *framerec = new QFrame();
            //framerec->setFrameShape(QFrame::Panel);
            QVBoxLayout *reclay = new QVBoxLayout(framerec);
            QHBoxLayout *titrelay = new QHBoxLayout();
            UpLabel *Titredoc = new UpLabel();
            Titredoc->setStyleSheet("color: green");
            if (msg->isurgent())
                Titredoc->setStyleSheet("color: red");
            QString txt = msg->datecreation().toString(tr("d-MMM-yy")) + " " + msg->datecreation().toString("h:mm");
            if (msg->iddestinataire()>0)
                txt += tr(" pour ") + (Datas::I()->users->getById(msg->iddestinataire()) != Q_NULLPTR? Datas::I()->users->getById(msg->iddestinataire())->login() : "null");
            Titredoc->setText(txt);
            titrelay->addWidget(Titredoc);
            UpCheckBox *Rdchk = new UpCheckBox();
            Rdchk->setChecked(msg->islu());
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
            Modiflbl->setiD(msg->id());
            Modiflbl->setPixmap(Icons::pxEditer().scaled(20,20)); //WARNING : icon scaled : pxEditer 20,20
            Modiflbl->setImmediateToolTip(tr("Modifier"));
            Modiflbl->setFixedWidth(25);
            connect(Modiflbl, QOverload<int>::of(&UpLabel::clicked),    this,  [=] {MsgModif(Modiflbl->iD());});
            Droplay->addWidget(Modiflbl);

            UpLabel *Dellbl = new UpLabel();
            Dellbl->setiD(msg->id());
            Dellbl->setPixmap(Icons::pxPoubelle().scaled(20,20)); //WARNING : icon scaled : pxPoubelle 20,20
            Dellbl->setFixedWidth(25);
            connect(Dellbl,     QOverload<int>::of(&UpLabel::clicked),  this,  [=] {SupprimerMessageEmis(Dellbl->iD());});
            Droplay->addWidget(Dellbl);
            Msglay->addLayout(Droplay);

            QLabel *line = new QLabel();
            line->setFrameShape(QFrame::VLine);
            line->setFixedWidth(1);
            Msglay->addWidget(line);

            QVBoxLayout *SMlay = new QVBoxLayout();
            SMlay->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Minimum,QSizePolicy::Expanding));
            if (msg->istache())                                                                           // le message est une tâche
            {
                QHBoxLayout *Tasklay = new QHBoxLayout();
                UpLabel *Todolbl = new UpLabel();
                Tasklay->setSpacing(0);
                Todolbl->setText(tr("A effectuer avant le ") + QLocale::system().toString(msg->datelimite(),tr("d-MMM-yy")));
                if (!msg->isfait())
                {
                    if (m_currentdate >= msg->datelimite())
                        Todolbl->setStyleSheet("color: red");
                    else if (m_currentdate.addDays(3) > msg->datelimite())
                        Todolbl->setStyleSheet("color: orange");
                }
                Tasklay->addWidget(Todolbl);
                Tasklay->addSpacerItem(new QSpacerItem(10,10,QSizePolicy::Expanding,QSizePolicy::Minimum));
                UpCheckBox *Dnchk = new UpCheckBox();
                Dnchk->setChecked(msg->isfait());
                Dnchk->setEnabled(false);
                Dnchk->setText(tr("fait"));
                Dnchk->setFixedWidth(45);
                Tasklay->addWidget(Dnchk);
                SMlay->addLayout(Tasklay);
            }
            if (msg->idpatient() > 0)                                                                            // le message concerne un patient
            {
                QString nomprenom ("");
                Patient *pat = Datas::I()->patients->getById(msg->idpatient());
                if (pat != Q_NULLPTR)
                    nomprenom = pat->prenom() + " " + pat->nom();
                if (nomprenom != "")
                {
                    QHBoxLayout *aboutlay = new QHBoxLayout();
                    UpLabel *aboutdoc = new UpLabel();
                    aboutdoc->setText(nomprenom);
                    aboutlay->addWidget(aboutdoc);
                    aboutlay->addSpacerItem(new QSpacerItem(40,10,QSizePolicy::Expanding, QSizePolicy::Minimum));
                    SMlay->addLayout(aboutlay);
                }
            }

            UpTextEdit *Msgtxt = new UpTextEdit();
            Msgtxt->setStyleSheet("border: 1px solid rgb(164, 205, 255);border-radius: 5px; background-color:#FFFFDD;");
            Msgtxt->setText(msg->texte());
            QString txt1 = Msgtxt->toHtml();
            Msgtxt->setText(txt1);
            Msgtxt->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            Msgtxt->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

            Msgtxt->document()->setTextWidth(370);
            Msgtxt->setFixedSize(380,int(Msgtxt->document()->size().height())+2);
            Msgtxt->settable(TBL_MESSAGES);
            Msgtxt->setchamp(CP_TEXT_MSG);
            Msgtxt->setiD(msg->id());
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

    if (listmessagesrecus.size()==0 && listmessagesenvoyes.size()==0)
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
    auto EnregResp = [=] (UpDialog *dlg_msgRepons)
    {
        if (dlg_msgRepons->findChildren<UpTextEdit*>().at(0)->toPlainText()=="")
       {
           UpSystemTrayIcon::I()->showMessage(tr("Messages"), tr("Vous avez oublié de rédiger le texte de votre message!"),Icons::icSunglasses(), 2000);
           return;
       }
        Message *msg = Datas::I()->messages->getById(idmsg);
        if (!msg)
            return;
       QHash<QString, QVariant> m_listbinds;
       m_listbinds[CP_IDEMETTEUR_MSG]  = currentuser()->id();
       QString Reponse = "<font color = " COULEUR_TITRES ">" + dlg_msgRepons->findChildren<UpLabel*>().at(0)->text() + "</font>"
               + "------<br><b>" + currentuser()->login() + ":</b> " + dlg_msgRepons->findChildren<UpTextEdit*>().at(0)->toPlainText().replace("\n","<br>");
       UpTextEdit txt;
       txt.setText(Reponse);
       m_listbinds[CP_TEXT_MSG]            = txt.toHtml();
       m_listbinds[CP_DATECREATION_MSG]    = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
       m_listbinds[CP_ENREPONSEA_MSG]      = idmsg;
       m_listbinds[CP_TACHE_MSG]           = (msg->istache()? 1 : QVariant());
       m_listbinds[CP_DATELIMITE_MSG]      = (msg->datelimite().isValid()? msg->datelimite().toString("yyyy-MM-dd") : QVariant());
       m_listbinds[CP_URGENT_MSG]          = (msg->isurgent()? 1 : QVariant());
       QList<int> listidusr = QList<int>() << msg->idemetteur();
       if (Datas::I()->messages->CreationMessage(m_listbinds, listidusr))
       {
           envoieTCPMessageA(listidusr);
           UpSystemTrayIcon::I()->showMessage(tr("Messages"), tr("Message enregistré"),Icons::icSunglasses(), 1000);
       }
       dlg_msgRepons->close();
    };

    Message *msg = Datas::I()->messages->getById(idmsg);
    if (!msg)
        return;
    User* usr = Datas::I()->users->getById(msg->idemetteur());
    if (!usr)
    {
        UpMessageBox::Watch(this,tr("Impossible de retrouver l'expéditeur du message"));
        return;
    }

    UpDialog *dlg_msgRepons = new UpDialog(this);

    UpTextEdit* rponstxt = new UpTextEdit();
    rponstxt->setStyleSheet("border: 1px solid rgb(164, 205, 255);border-radius: 5px; background-color:#FFFFFF;");
    dlg_msgRepons->dlglayout()->insertWidget(0,rponstxt);

    QHBoxLayout *lbllayout  = new QHBoxLayout();
    UpLabel     *msglbl     = new UpLabel(dlg_msgRepons);
    QString nomprenom = "";
    if (msg->idpatient() > 0)
    {
        Patient *pat = Datas::I()->patients->getById(msg->idpatient());
        if (pat != Q_NULLPTR)
            nomprenom = tr("à propos de ") + "<b>" + pat->prenom() + " " + pat->nom() + "</b>";
    }
    if (nomprenom != "")
        msglbl->setText(nomprenom + "\n");
    msglbl      ->setText(msglbl->text() + msg->texte());
    lbllayout   ->addSpacerItem(new QSpacerItem(30,1));
    lbllayout   ->addWidget(msglbl);

    dlg_msgRepons->dlglayout()   ->insertLayout(0,lbllayout);

    QLabel *lbl = new QLabel();
    lbl->setText(tr("Réponse au message de ") + "<font color=\"green\"><b>" + usr->login() + "</b></font>");
    dlg_msgRepons->dlglayout()->insertWidget(0,lbl);

    dlg_msgRepons->AjouteLayButtons(UpDialog::ButtonCancel | UpDialog::ButtonOK);
    dlg_msgRepons->OKButton        ->setiD(idmsg);
    connect(dlg_msgRepons->OKButton, &QPushButton::clicked, this, [=] {EnregResp(dlg_msgRepons);});

    dlg_msgRepons->setSizeGripEnabled(false);
    dlg_msgRepons->setWindowFlags(Qt::Tool | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::CustomizeWindowHint);
    dlg_msgRepons->setWindowTitle(tr("Messagerie"));
    int y = 0;
    QList<QScreen*> listscreens = QGuiApplication::screens();
    if (listscreens.size())
        y = listscreens.first()->geometry().height();
    dlg_msgRepons->setMaximumHeight(y-30);
    dlg_msgRepons->setWindowIcon(Icons::icSunglasses());
    dlg_msgRepons->setFixedWidth(450);

    dlg_msgRepons->exec();
    delete dlg_msgRepons;
}

void Rufus::MsgModif(int idmsg)
{
    QList<UpTextEdit*> listtxt = dlg_msgBAL->findChildren<UpTextEdit*>();
    if (listtxt.size()>0)
        for (int i=0; i<listtxt.size();i++)
        {
            if (listtxt.at(i)->iD()==idmsg)
            {
                QMap<QString, QVariant> map;
                Message *msg = Datas::I()->messages->getById(idmsg);
                if (msg)
                {
                    map[CP_TEXT_MSG]            = msg->texte();
                    map[CP_IDPATIENT_MSG]       = msg->idpatient();
                    map[CP_TACHE_MSG]           = (msg->istache()? 1 : 0);
                    map[CP_DATELIMITE_MSG]      = msg->datelimite();
                    map[CP_DATECREATION_MSG]    = msg->datecreation();
                    map[CP_URGENT_MSG]          = (msg->isurgent()? 1 : 0);
                    map["null"]                 = false;
                }
                QStringList listdestinataires;
                QString req = "select " CP_IDDESTINATAIRE_JOINTURESMSG " from " TBL_MESSAGESJOINTURES " where " CP_IDMSG_JOINTURESMSG " = " + QString::number(idmsg);
                QList<QVariantList> destlist = db->StandardSelectSQL(req,m_ok);
                for (int i=0; i<destlist.size();i++)
                    listdestinataires << destlist.at(i).at(0).toString();
                map["listdestinataires"] = listdestinataires;
                if (msg)
                    SendMessage(map, msg->idpatient(), idmsg);                           //depuis MsgModif
                i =listtxt.size();
                AfficheBAL(1);
            }
        }
}

void Rufus::SupprimerMessageEmis(int idMsg)
{
    QString req = "update " TBL_MESSAGES " set " CP_ASUPPRIMER_MSG " = 1 where " CP_ID_MSG " = " + QString::number(idMsg);
    db->StandardSQL(req);
    req = "delete from " TBL_MESSAGESJOINTURES " where "
          CP_IDMSG_JOINTURESMSG " = " + QString::number(idMsg) +
          " and " CP_IDDESTINATAIRE_JOINTURESMSG " = " + QString::number(currentuser()->id());
    db->StandardSQL(req);
    if (dlg_msgBAL->findChildren<QScrollArea*>().size()>0)
        AfficheBAL(1);
}

void Rufus::SupprimerMessageRecu(int idJoint)
{
    QString req = "select " CP_IDMSG_JOINTURESMSG " from " TBL_MESSAGESJOINTURES  " where " CP_ID_JOINTURESMSG " = " + QString::number(idJoint);
    QVariantList msgdata = db->getFirstRecordFromStandardSelectSQL(req,m_ok);
    int idmsg = msgdata.at(0).toInt();
    Message *msg = Datas::I()->messages->getById(idmsg);
    if (!msg)
        return;
    int idusr = msg->idemetteur();
    if (idusr == currentuser()->id())
        ItemsList::update(msg, CP_ASUPPRIMER_MSG, true);
    db->StandardSQL("delete from " TBL_MESSAGESJOINTURES " where " CP_ID_JOINTURESMSG " = " + QString::number(idJoint));
    bool ok;
    QList<QVariantList> listmsg = db->StandardSelectSQL("select " CP_IDMSG_JOINTURESMSG " from " TBL_MESSAGESJOINTURES " where " CP_IDMSG_JOINTURESMSG " = " + QString::number(msg->id()), ok);
    if (ok)
    {
        if (listmsg.size() == 0)
        {
            req = "delete from " TBL_MESSAGES " where "
                  CP_ID_MSG " not in (select " CP_IDMSG_JOINTURESMSG " from " TBL_MESSAGESJOINTURES ") "
                  " and " CP_ASUPPRIMER_MSG " = 1";
            db->StandardSQL(req);
            Datas::I()->messages->SupprimeMessage(msg);
        }
    }
    if (dlg_msgBAL->findChildren<QScrollArea*>().size()>0)
        AfficheBAL();
}

void Rufus::ReconstruitListeMessages()
{
    QDateTime DateMsg;
    m_totalNvxMessages = 0;
    Datas::I()->messages->initListeAllMsgsByIdUser(currentuser()->id());
    m_totalMessages = Datas::I()->messages->allmessages().size();
    ict_messageIcon->setVisible(m_totalMessages>0);
    if (m_totalMessages>0)
    {
        for (int i=0; i<Datas::I()->messages->allmessages().size(); i++)
        {
            Message *msg = Datas::I()->messages->allmessages().at(i);
            DateMsg = msg->datecreation();
            if (msg->datecreation() > QDateTime(m_datederniermessageuser))
                m_totalNvxMessages += 1;
        }
        m_datederniermessageuser = QDateTime(DateMsg);
    }
    else if (dlg_msgBAL != Q_NULLPTR)
    {
        if (dlg_msgBAL->isVisible())
            dlg_msgBAL->close();
        delete dlg_msgBAL;
        dlg_msgBAL = Q_NULLPTR;
    }

    QString msg = "";
    if (m_isTotalMessagesAffiche)
    {
        if (m_totalMessages==m_totalNvxMessages)
        {
            if (m_totalMessages>1)
                msg = tr("Vous avez ") + QString::number(m_totalMessages) + tr(" nouveaux messages");
            else if (m_totalMessages>0)
                msg = tr("Vous avez 1 nouveau message");
        }
        else if (m_totalMessages>m_totalNvxMessages)
        {
            if (m_totalMessages>1)
            {
                msg = tr("Vous avez ") + QString::number(m_totalMessages) + tr(" messages");
                if (m_totalNvxMessages>1)
                    msg += " dont " + QString::number(m_totalNvxMessages) + tr(" nouveaux");
                else if (m_totalNvxMessages>0)
                    msg += tr(" dont 1 nouveau");
            }
            else if (m_totalMessages>0)
                msg = tr("Vous avez 1 message");
        }
    }
    else if (m_totalNvxMessages>1)
        msg = tr("Vous avez ") + QString::number(m_totalNvxMessages) + tr(" nouveaux messages");
    else if (m_totalNvxMessages>0)
        msg = tr("Vous avez 1 nouveau message");
    if (msg!="")
    {
        Utils::playAlarm();
        ict_messageIcon->showMessage(tr("Messages"), msg, Icons::icPostit(), 10000);
        if (dlg_msgBAL != Q_NULLPTR)
            if (dlg_msgBAL->isVisible())
                AfficheBAL();
    }
    m_isTotalMessagesAffiche = false;
}

void Rufus::VerifMessages()
{
    if (!QSystemTrayIcon::isSystemTrayAvailable())
        return;
    int flagmsg = Flags::I()->flagMessages();
    if (m_flagmessages < flagmsg)
        ReconstruitListeMessages();
    m_flagmessages = flagmsg;
}

/*!
 * \brief Rufus::CleanSalleDAttente
 * supprime de la salle d'attente les postes dont le statut est aberrant
*/
void Rufus::CleanSalleDAttente()
{
    /*! il faut nettoyer la salle d'attente des patients en cours d'examen
        * soit inscits sur un poste qui n'est pas connecté
            * on vérifie dans les postes connectés qu'on trouve bien ce poste
        * soit inscrits sur un poste qui est connecté mais avec un autre utilisateur que celui qui est inscrit comme encours d'examen sur ce poste
            * on vérifie dans les postes connectés qu'on trouve bien ce poste et que l'utilisateur qui y est inscrit est le même que celui qui est déclaré comme iduserencoursexamen du pateint en salle d'attente
        * soit dont l'iduserencours d'examen n'est plus connecté
            * on vérifie dans les postes connectés qu'on trouve bien ce poste avec l'utilisateur déclaré comme iduserencoursexamen du patient en salle d'attente
    */
    QList<PosteConnecte*> listpostsoignant;
    QStringList listnomspostessoignants     = QStringList();
    QList<int>  listidpatientsencoursexamen = QList<int>();
    bool majsaldat = false;

    /*! Elaboration
     * de la liste des postes soignants
     * de la liste des noms des postes soignants
     * de la liste des idpatient en cours d'examen sur les postes soigants
    */
    for (auto it = Datas::I()->postesconnectes->postesconnectes()->constBegin(); it != Datas::I()->postesconnectes->postesconnectes()->constEnd(); ++it)
    {
        PosteConnecte* post = const_cast<PosteConnecte*>(it.value());
        if (post != Q_NULLPTR)
            if (Datas::I()->users->getById(post->iduser()) != Q_NULLPTR)
                if (Datas::I()->users->getById(post->iduser())->isSoignant())
                {
                    listpostsoignant            << post;
                    listnomspostessoignants     << post->nomposte();
                    listidpatientsencoursexamen << post->idpatencours();
                }
    }

    QString blabla              = ENCOURSEXAMEN;
    int length                  = blabla.size();
    QList<PatientEnCours*> listpatcrstoremove = QList<PatientEnCours*>();
    for (auto it = Datas::I()->patientsencours->patientsencours()->constBegin(); it != Datas::I()->patientsencours->patientsencours()->constEnd(); ++it)
    {
        PatientEnCours *patcrs = const_cast<PatientEnCours*>(it.value());
        if (patcrs != Q_NULLPTR)
        {
            /*! on retire de la salle d'attente les patients qui n'existent pas */
            if (Datas::I()->patients->getById(patcrs->id()) == Q_NULLPTR)
            {
                Datas::I()->patientsencours->SupprimePatientEnCours(patcrs);
                continue;
            }
            if (patcrs->statut().left(length) == ENCOURSEXAMEN)
            {
                //! on redonne le statut "arrivé" aux patients en salle d'attente dont le iduserencourssexam n'est plus connecté (plantage)
                if (!listidpatientsencoursexamen.contains(patcrs->id()))
                {
                    if (!listpatcrstoremove.contains(patcrs))
                    {
                        listpatcrstoremove << patcrs;
                        qDebug() << "1 - Retrait du patient" << Datas::I()->patients->getById(patcrs->id())->nomcomplet() << "de la salle d'attente";
                    }
                    continue;
                }
                //! on vérifie dans les postes connectés qu'on trouve bien ce poste
                if (!listnomspostessoignants.contains(patcrs->posteexamen()))
                {
                    if (!listpatcrstoremove.contains(patcrs))
                    {
                        listpatcrstoremove << patcrs;
                        qDebug() << "2 - Retrait du patient" << Datas::I()->patients->getById(patcrs->id())->nomcomplet() << "de la salle d'attente";
                    }
                     continue;
                }
                //! on vérifie dans les postes connectés utilisés par ce patientencours,
                //! que l'utilisateur qui est inscrit sur un poste est le même
                //! que celui qui est déclaré comme iduserencoursexamen du patient en salle d'attente pour ce poste
                //! que le patient qui est inscrit sur un poste est le même
                //! que celui qui est déclaré comme patient en cours pour ce poste
                else for (int itpost = 0; itpost < listpostsoignant.size(); ++ itpost)
                {
                    PosteConnecte *post = listpostsoignant.at(itpost);
                    if (post->nomposte() == patcrs->posteexamen())
                    {
                        if (post->iduser() != patcrs->iduserencoursexam())
                        {
                            if (!listpatcrstoremove.contains(patcrs))
                            {
                                listpatcrstoremove << patcrs;
                                qDebug() << "3 - Retrait du patient" << Datas::I()->patients->getById(patcrs->id())->nomcomplet() << "de la salle d'attente";
                            }
                            continue;
                        }
                        else if (post->idpatencours() != patcrs->id())
                        {
                            if (!listpatcrstoremove.contains(patcrs))
                            {
                                listpatcrstoremove << patcrs;
                                qDebug() << "4 - Retrait du patient" << Datas::I()->patients->getById(patcrs->id())->nomcomplet() << "de la salle d'attente";
                            }
                            continue;
                        }
                    }
                }
            }
        }
    }
    if (listpatcrstoremove.size() >0)
    {
        majsaldat = true;
        for (auto itpatcrs = listpatcrstoremove.cbegin(); itpatcrs != listpatcrstoremove.cend();++ itpatcrs)
        {
            PatientEnCours* patcrs = *itpatcrs;
            ItemsList::update(patcrs, CP_STATUT_SALDAT, ARRIVE);
            ItemsList::update(patcrs, CP_POSTEEXAMEN_SALDAT);
            ItemsList::update(patcrs, CP_IDUSERENCOURSEXAM_SALDAT);
            majsaldat = true;
            qDebug() << "Retrait du patient" << Datas::I()->patients->getById(patcrs->id())->nomcomplet() << "de la salle d'attente";
        }
    }
    if (majsaldat)
        Flags::I()->MAJFlagSalleDAttente();
}

void Rufus::VerifSalleDAttente()
{
    int flagsaldat = Flags::I()->flagSalleDAttente();
    if (m_flagsalledattente < flagsaldat)
        Remplir_SalDat();                       // par le timer VerifSalleDAttente
    m_flagsalledattente = flagsaldat;
}

void Rufus::VerifCorrespondants()
{
    if (currentpatient() == Q_NULLPTR)
        return;
    int flagcor = Flags::I()->flagCorrespondants();
    if (m_flagcorrespondants < flagcor)
    {
        // on reconstruit la liste des MG et des correspondants
        ReconstruitCombosCorresp();                     // par le timer gTimerCorrespondants
        // on resynchronise l'affichage du combobox au besoin
        if (ui->tabWidget->indexOf(ui->tabDossier) > -1)
        {
            int idx (-1), idxsp1 (-1), idxsp2(-1);
            QString req = "select " CP_IDMG_RMP ", " CP_IDSPE1_RMP ", " CP_IDSPE2_RMP " from " TBL_RENSEIGNEMENTSMEDICAUXPATIENTS " where " CP_IDPAT_RMP " = " + QString::number(currentpatient()->id());
            QVariantList cordata = db->getFirstRecordFromStandardSelectSQL(req,m_ok);
            if (!m_ok)
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
            OKModifierTerrain(currentpatient());
        }
    }
    m_flagcorrespondants = flagcor;
}

void Rufus::VerifDocsDossiersEchanges()
{
    if (isPosteImport())
    {
        if (m_importdocsexternesthread == Q_NULLPTR)
        {
            m_importdocsexternesthread = new ImportDocsExternesThread();
            connect(m_importdocsexternesthread, &ImportDocsExternesThread::emitmsg, this, &Rufus::AfficheMessageImport);
        }
        for (int itr=0; itr<proc->listeappareils().size(); itr++)
        {
            AppareilImagerie *appareil = proc->listeappareils().at(itr);
            QString nomdossier =  appareil->nomdossierechange();  /*! le dossier où sont exportés les documents d'un appareil donné */
            if (nomdossier != "")
                if (QDir(nomdossier).exists())
                {
                    QStringList filters, listnomsfiles;
                    filters << "*.pdf" << "*.jpg" << "*.png" << "*.jpeg";
                    listnomsfiles = QDir(nomdossier).entryList(filters, QDir::Files | QDir::NoDotAndDotDot);
                    if (listnomsfiles.size() > 0)
                        for (int it=0; it<listnomsfiles.size(); it++)
                        {
                            QString nomfiledoc = listnomsfiles.at(it);
                            if (!nomfiledoc.contains("smbtest"))
                                 m_importdocsexternesthread->RapatrieDocumentsThread(appareil, nomfiledoc);
                        }
                }
        }
    }
}


void Rufus::VerifVerrouDossier()
{
    if (DataBase::I()->ModeAccesDataBase() == Utils::Distant)
        return;
    /*! Cette fonction sert à déconnecter et lever les verrous d'un utilisateur qui se serait déconnecté accidentellement
     *  elle n'est utilisée qu'en cas de non utilisation du tcp
     on fait la liste des utilisateurs qui n'ont pas remis à jour leur connexion depuis plus de 60 secondes,
     on retire les verrous qu'ils auraient pu poser et on les déconnecte */
    Datas::I()->postesconnectes->initListe();
    QDateTime timenow = db->ServerDateTime();
    QList<PosteConnecte*> listpostsAEliminer = QList<PosteConnecte*>();
    foreach(PosteConnecte* post, *Datas::I()->postesconnectes->postesconnectes())
    {
        qint64 tempsecouledepuisactualisation = post->dateheurederniereconnexion().secsTo(timenow);
        if (tempsecouledepuisactualisation > 240)
        {
            qDebug() << "Suppression d'un poste débranché accidentellement" << "Rufus::VerifVerrouDossier()";
            qDebug() << "nom du poste" << post->nomposte();
            qDebug() << "stringid du poste" << post->stringid();
            qDebug() << "timenow = " << timenow;
            qDebug() << "heure dernière connexion = " << post->dateheurederniereconnexion();
            qDebug() << "temps ecoule depuis actualisation = " << tempsecouledepuisactualisation;
            qDebug() << "user = " << (Datas::I()->users->getById(post->iduser()) != Q_NULLPTR? Datas::I()->users->getById(post->iduser())->login() : tr("inconnu"));
            if (!listpostsAEliminer.contains(post))
                listpostsAEliminer << post;
        }
    }
    if (listpostsAEliminer.size() > 0)
    {
       Datas::I()->patientsencours->initListeAll();
       foreach (PosteConnecte* post, listpostsAEliminer)
       {
           QString nomposte = (post->isadmin()? tr("administrateur") + " " : "") + post->nomposte();
           //! l'utilisateur n'a pas remis sa connexion à jour depuis plus de 240 secondes
           //! on déverrouille les dossiers verrouillés par cet utilisateur et on les remet en salle d'attente
           for (auto it = Datas::I()->patientsencours->patientsencours()->constBegin(); it != Datas::I()->patientsencours->patientsencours()->constEnd(); ++it)
           {
               PatientEnCours *patcrs = const_cast<PatientEnCours*>(it.value());
               if (patcrs != Q_NULLPTR)
                   if (patcrs->iduserencoursexam() == post->iduser() && patcrs->statut().contains(ENCOURSEXAMEN) && patcrs->posteexamen() == post->nomposte())
                   {
                       //! on remet le patient en salle d'accueil avec le statut ARRIVE
                       ItemsList::update(patcrs, CP_STATUT_SALDAT, ARRIVE);
                       ItemsList::update(patcrs, CP_POSTEEXAMEN_SALDAT);
                       ItemsList::update(patcrs, CP_IDUSERENCOURSEXAM_SALDAT);
                   }
           }
           Datas::I()->postesconnectes->SupprimePosteConnecte(post);
           //ItemsList::update(Datas::I()->sessions->currentsession(), CP_ID_SESSIONS, QDateTime::currentDateTime());
           UpSystemTrayIcon::I()->showMessage(tr("Messages"), tr("Le poste ") + nomposte + tr(" a été retiré de la liste des postes connectés actuellement au serveur"),Icons::icSunglasses(), 1000);
       }
    }
    CleanSalleDAttente();
}

void Rufus::VerifLastVersion()
{
    auto comparelastversion = [&] {
        QString actversion = qApp->applicationVersion().split("/").at(0);
        QDate dateactversion = QDate::fromString(actversion,"dd-MM-yyyy");
        QDate datenewversion = QDate::fromString(m_MAJlastversion, "yyyy/MM/dd");
        if (dateactversion < datenewversion)
        {
            QString text = QObject::tr("La nouvelle version est datée du ") + QLocale::system().toString(datenewversion, "d MMM yyyy") + "<br/>"
                    + QObject::tr("Vous utilisez la version du ") + QLocale::system().toString(dateactversion, "d MMM yyyy");
            if (m_MAJBase)
            {
                text += "<br/>" + QObject::tr("Cette nouvelle version impose une mise à jour de la base de données");
                if (!m_MAJBaseCompatibiltyWithPrec)
                    text += "<br/>" + QObject::tr("Après cette mise à jour, tous les postes utilisant Rufus sur cette base devront aussi évoluer vers la nouvelle versionr");
                else
                    text += "<br/>" + QObject::tr("Cette mise à jour de la base de données reste compatible avec votre version actuelle de Rufus");
            }
            else
                text += "<br/>" + QObject::tr("Cette nouvelle version n'impose pas de mise à jour de la base de données et est compatible avec la précédente version de Rufus");
            text += "<br/>" + QObject::tr("Vous pouvez télécharger la nouvelle version sur la page Téléchargements du site") + " <a href=\'https://www.rufusvision.org\'>www.rufusvision.org</a>";
            UpMessageBox::Watch(this, QObject::tr("Une nouvelle version de Rufus est en ligne"), text, UpDialog::ButtonOK, "https://www.rufusvision.org");
        }
        //qDebug() << "OS = " << m_os;
    };

    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    QNetworkRequest request;
    request.setUrl(QUrl(LIEN_XML_RUFUSLASTVERSION));
    QNetworkReply *reply = manager->get(request);

    connect(manager, &QNetworkAccessManager::finished,
            this, [=]
    {
        QByteArray data;
        if(reply->error() == QNetworkReply::NoError)
        {
            m_os = QSysInfo::productType();
            if (m_os == "osx")
                m_os = "macos";
            else if (m_os == "" || m_os == "ubuntu")
                m_os = "linux";
            reply->deleteLater();
            data = reply->readAll();
            QDomDocument docxml;
            docxml.setContent(data);
            QDomElement xml = docxml.documentElement();
            for (int i=0; i<xml.childNodes().size(); i++)
            {
                QDomElement system = xml.childNodes().at(i).toElement();
                if (m_os != system.tagName())
                    continue;
                for (int j=0; j<system.childNodes().size(); j++)
                {
                    QDomElement child = system.childNodes().at(j).toElement();
                    if (child.tagName() == "Date")
                        m_MAJlastversion = child.text();
                    else if (child.tagName() == "Base")
                    {
                        for (int k=0; k<child.childNodes().size(); k++)
                        {
                            QDomElement basechild = child.childNodes().at(k).toElement();
                            if (basechild.tagName() == "UPDBase")
                                m_MAJBase = (basechild.text() == "Yes");
                            else if (basechild.tagName() == "CompatibleWithPrecedent")
                                m_MAJBaseCompatibiltyWithPrec = (basechild.text() == "Yes");
                        }
                    }
                    else if (child.tagName() == "Comment")
                        m_MAJcomment = child.text();
                }
                i = xml.childNodes().size();
            }
        }
        comparelastversion();
    });
}

bool Rufus::isPosteImport()
{
    return m_isposteImport;
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
    if (db->ModeAccesDataBase() == Utils::Distant)
    {
        m_isposteImport = true;
        VerifDossiersImagerie();
        return;
    }

    bool statut = isPosteImport();
    //qDebug()<< statut;
    QString ImportateurDocs = proc->PosteImportDocs(); //le nom et l'adresse Mac du poste importateur des docs externes
    QString importsetting = proc->settings()->value(Utils::getBaseFromMode(Utils::ReseauLocal) + PrioritaireGestionDocs).toString();
    QString IPAdress = "NULL";
    if (importsetting == "YES")
        IPAdress = Utils::hostNameMacAdress() + " - prioritaire";
    else if (importsetting == "NORM")
        IPAdress = Utils::hostNameMacAdress();

    if (ImportateurDocs.toUpper() == "NULL")
    {
        if ((importsetting == "YES" || importsetting == "NORM") && db->ModeAccesDataBase() != Utils::Distant)
             proc->setPosteImportDocs(IPAdress);
    }
    else
    {

        if (ImportateurDocs != IPAdress) //si le poste défini comme importateur des docs est différent de ce poste, on vérifie qu'il est toujours actif et qu'il n'est pas prioritaire
        {
            // on vérifie que l'importateur est toujours connecté
            int idx = -1;
            for (auto it = Datas::I()->postesconnectes->postesconnectes()->constBegin(); it != Datas::I()->postesconnectes->postesconnectes()->constEnd(); ++it)
            {
                PosteConnecte *post = const_cast<PosteConnecte*>(it.value());
                if (post->nomposte()+ " - " + post->macadress() == ImportateurDocs.remove(" - prioritaire"))
                {
                    idx = Datas::I()->postesconnectes->postesconnectes()->values().indexOf(post);
                    break;
                }
            }
            if (idx<0)                                                      //! Le poste défini comme importateur des docs externes n'est pas connecté,
            {
                /*! on prend la place si
                 *      on n'est pas en accès distant
                 *      et si on est importateur
                 *  sinon, on retire le poste */
                proc->setPosteImportDocs(IPAdress);
            }
            else if (!ImportateurDocs.contains(" - " NOM_ADMINISTRATEUR))   //! le poste défini comme importateur est valide mais pas administrateur
            {
                /*! on prend sa place si on est prioritaire et pas lui */
                if (importsetting == "YES" && !ImportateurDocs.contains(" - prioritaire"))
                    proc->setPosteImportDocs(IPAdress);
                else if (ImportateurDocs.remove(" - prioritaire") == Utils::hostNameMacAdress() && ImportateurDocs != IPAdress)
                                                                            //! cas rare du poste qui a modifié son propre statut
                    proc->setPosteImportDocs(IPAdress);
            }
        }
    }
    m_isposteImport = (proc->PosteImportDocs().remove(" - prioritaire") == Utils::hostNameMacAdress());
    bool chgtstatut = (statut != isPosteImport());
    if (chgtstatut)
    {
        if (isPosteImport())
            connect(t_timerExportDocs,           &QTimer::timeout,   this,   &Rufus::ExporteDocs);
        else
            t_timerExportDocs->disconnect();
        VerifDossiersImagerie();
    }
}

void Rufus::VerifDossiersImagerie()
{
    if (isPosteImport())
    {
        QList<AppareilImagerie*> listappareils = QList<AppareilImagerie*>();
        bool usetimer = true;  /*! Il semble que la classe QSystemFileWatcher pose quelques problèmes.
                             * au démarrage du système le signal directorychanged ne marche pas bien sur Mac quand le fichier d'échange est sur une machine Linux ou Windows
                             * il faut redémarrer Rufus pour que ça se décide à marcher
                             * On peut utiliser un timer à la place. C'est nettement moins élégant mais ça marche très bien.
                             * Il suffit de mettre ce bool à true pour utiliser le timer
                             * Le code pour le QFileSystemWatcher a été conservé au cas où le problème serait résolu */

        QString req =   "select distinct list." CP_TITREEXAMEN_APPAREIL ", list." CP_NOMAPPAREIL_APPAREIL " from " TBL_APPAREILSCONNECTESCENTRE " appcon, " TBL_LISTEAPPAREILS " list"
                        " where list." CP_ID_APPAREIL " = appcon." CP_IDAPPAREIL_APPAREILS " and " CP_IDLIEU_APPAREILS " = " + QString::number(Datas::I()->sites->idcurrentsite());
        //qDebug()<< req;
        QList<QVariantList> listdocs = db->StandardSelectSQL(req, m_ok);
        if (m_ok && listdocs.size()>0)
        {
            for (int itr=0; itr<listdocs.size(); itr++)
            {
                QString nomappareil = listdocs.at(itr).at(1).toString();
                QString nomdossier = proc->pathDossierDocuments(nomappareil, db->ModeAccesDataBase());  // le dossier où sont exportés les documents d'un appareil donné
                if (!usetimer)
                    m_filewatcher.addPath(nomdossier);
                if (nomdossier != "")
                    if (QDir(nomdossier).exists())
                    {
                        QString titreexamen = listdocs.at(itr).at(0).toString();
                        AppareilImagerie *appareil = new AppareilImagerie(titreexamen, nomappareil, nomdossier);
                        listappareils << appareil;
                        //qDebug() << "l'appareil " + nomappareil + " est surveillé sur le dossier " + nomdossier;
                    }
                //else
                //    qDebug() << "le dossier " + nomdossier + " n'existe pas";
            }
        }
        // Surveillance du dossier d'imagerie ----------------------------------------------------------------------------------
        proc->setlisteappareils(listappareils);
        if (listappareils.size() > 0)
        {
            for (int it=0; it<listappareils.size(); it++)
                ImportNouveauDocExterne(listappareils.at(it));
            if (!usetimer)
            {
                disconnect (&m_filewatcher, nullptr, nullptr, nullptr);
                connect(&m_filewatcher,     &QFileSystemWatcher::directoryChanged,  this,   [=](QString nomdossier)
                {
                    for (int i=0; i<proc->listeappareils().size(); i++)
                        if (proc->listeappareils().at(i)->nomdossierechange() == nomdossier)
                            ImportNouveauDocExterne(proc->listeappareils().at(i));
                } );
            }
            else
            {
                t_timerfilewatcher.stop();
                disconnect (&t_timerfilewatcher, nullptr, nullptr, nullptr);
                t_timerfilewatcher.start(2500);
                connect (&t_timerfilewatcher,   &QTimer::timeout,   this, &Rufus::VerifDocsDossiersEchanges);
            }
        }
    }
    else
    {
        disconnect (&m_filewatcher, nullptr, nullptr, nullptr);
        disconnect (&t_timerfilewatcher, nullptr, nullptr, nullptr);
    }
}

/*!
    * \brief Rufus::ActualiseDocsExternes
    * \abstract vérifier l'apparition de nouveaux documents si la fiche dlg_docsexternes est fermée et l'ouvrir si c'est le cas
*/
void Rufus::ActualiseDocsExternes()
{
    if (ui->tabWidget->indexOf(ui->tabDossier)<1)
        return;
    if (currentpatient() != Q_NULLPTR)
    {
        QList<dlg_docsexternes *> ListDialogDocs = this->findChildren<dlg_docsexternes *>();
        if (ListDialogDocs.size()>0)
            for (int i=0; i< ListDialogDocs.size();++i)
                if (ListDialogDocs.at(i)->currentpatient() == currentpatient())
                    return;
        Datas::I()->docsexternes->initListeByPatient(currentpatient());
        OuvrirDocsExternes();
    }
}

//-------------------------------------------------------------------------------------
// Interception des évènements clavier
//-------------------------------------------------------------------------------------
void Rufus::keyPressEvent (QKeyEvent * event )
{
        switch (event->key()) {
        case Qt::Key_F3:
            RefractionMesure(dlg_refraction::Manuel);
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
        case Qt::Key_F7:
            Pachymetrie();
            break;
        case Qt::Key_F9:
            if (ui->tabWidget->indexOf(ui->tabDossier) < 0 && currentuser()->isSoignant())
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
                if (m_mode == NouveauDossier)
                    CreerDossier();
                else if (ui->PatientsListeTableView->selectionModel()->selectedIndexes().size() > 0)
                    if (!currentuser()->isNeutre())
                        OuvrirDossier(getPatientFromSelectionInTable());
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


Cotations* Rufus::getListeCotationdByUser(User *usr)
{
    if (usr == Q_NULLPTR)
        return Q_NULLPTR;
    User *userparent = usr;
    int id = usr->id();

    /*! si le user est connecté , on sélectionne la liste de cotations de son parent
     * il ne peut pas y avoir d'appel d'un user non soignant puisque la fonction est appelée pour un user qui est
            * soit le currentuser() qui est forcément soignant puisqu'on recrée la liste de cotations seulement dans cette condition
            * soit un user qui est le superviseur d'un acte donc forcément soignant aussi */
    if (proc->isUserConnected(usr))
        for (auto it = Datas::I()->postesconnectes->postesconnectes()->constBegin(); it !=  Datas::I()->postesconnectes->postesconnectes()->constEnd(); ++it)
        {
            PosteConnecte *post = const_cast<PosteConnecte*>(it.value());
            if (post->iduser() == usr->id())
            {
                id = post->idparent();
                userparent = Datas::I()->users->getById(id);
                if (userparent == Q_NULLPTR)
                    return Q_NULLPTR;
                break;
            }
        }
    auto itcot = Datas::I()->listecotations->constFind(id);
    if (itcot != Datas::I()->listecotations->constEnd())
        return itcot.value();
    else
    {
        Cotations *cots = new Cotations();
        cots->initListeByUser(userparent);
        if (cots)
            Datas::I()->listecotations->insert(id, cots);
        return cots;
    }
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
        return Q_NULLPTR;
    Patient *pat = qobject_cast<Patient *>(upitem->item());
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

void Rufus::closeEvent(QCloseEvent *)
{
    int iduserposte = 0;
    if (currentpost() != Q_NULLPTR)
        iduserposte = currentpost()->id();
    if ( proc->PosteImportDocs().remove(" - prioritaire")== Utils::hostNameMacAdress())
        proc->setPosteImportDocs("NULL");

    QString req = "update " TBL_UTILISATEURS " set " CP_DATEDERNIERECONNEXION_USR " = '" + QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss")
            + "' where " CP_ID_USR " = " + QString::number(currentuser()->id());
    db->StandardSQL(req);
    if (DataBase::I()->ModeAccesDataBase() == Utils::Distant)
        Flags::I()->MAJflagUserDistant();
    //! on retire cet utilisateur de la table des utilisateurs connectés
    //! avec TCP, les actions qui suivent sont effectuées par RufusAdmin
    if (!m_utiliseTCP)
    {
        if (currentpost() != Q_NULLPTR)
            Datas::I()->postesconnectes->SupprimePosteConnecte(currentpost());
        Flags::I()->MAJFlagSalleDAttente();
        //!> on déverrouille les actes verrouillés en comptabilité par cet utilisateur s'il n'est plus connecté sur aucun poste
        bool usernotconnectedever = true;
        for (auto it = Datas::I()->postesconnectes->postesconnectes()->constBegin(); it != Datas::I()->postesconnectes->postesconnectes()->constEnd(); ++it)
        {
            PosteConnecte *post = const_cast<PosteConnecte*>(it.value());
            if(post->iduser() == iduserposte)
            {
                usernotconnectedever = false;
                break;
            }
        }
        if (usernotconnectedever)
            db->StandardSQL("delete from " TBL_VERROUCOMPTAACTES " where " CP_POSEPAR_VERROUCOMPTA " = " + QString::number(iduserposte));
    }
    ItemsList::update(Datas::I()->sessions->currentsession(), CP_DATEFIN_SESSIONS, QDateTime::currentDateTime());
}

// ------------------------------------------------------------------------------------------
// Interception des évènements internes
// ------------------------------------------------------------------------------------------

bool Rufus::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::FocusIn )
    {
        if (obj == ui->ActeMontantlineEdit)         { m_montantActe    = QLocale().toString(QLocale().toDouble(ui->ActeMontantlineEdit->text()),'f',2); ui->ActeMontantlineEdit->selectAll(); }
        if (obj == ui->ActeCotationcomboBox)        m_montantActe    = QLocale().toString(QLocale().toDouble(ui->ActeMontantlineEdit->text()),'f',2);
        if (obj == ui->ActeDatedateEdit)            m_dateActe       = ui->ActeDatedateEdit->text();
    }

    if (event->type() == QEvent::FocusOut )
    {
        UpTextEdit* objUpText = qobject_cast<UpTextEdit*>(obj);
        if (objUpText != Q_NULLPTR)
        {
            QString requetemodif;
            objUpText->textCursor().clearSelection();
            if (objUpText->valeuravant() != objUpText->toHtml() || objUpText->ismodified())
            {
                QString Corps = objUpText->toHtml();
                if (objUpText->table() == TBL_ACTES || objUpText->table() == TBL_MESSAGES)
                {
                    Corps.replace("<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">", HTML_RETOURLIGNE);
                    Corps.remove("border=\"0\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px;\" ");
                    Corps.remove(HTMLCOMMENT);

                    Corps.append(HTMLCOMMENT);

                    if (objUpText->table() == TBL_ACTES)
                    {
                        ItemsList::update(currentacte(), objUpText->champ(), Corps);
                        MAJActesPrecs();
                    }
                    else if (objUpText->table() == TBL_MESSAGES)
                    {
                        requetemodif =   "UPDATE " + objUpText->table() + " SET " + objUpText->champ() + " = '"
                                + Utils::correctquoteSQL(Corps) + "' WHERE idMessage = " + QString::number(currentacte()->id());
                        db->StandardSQL(requetemodif, tr("Impossible de mettre à jour le champ ") + objUpText->champ() + "!");
                    }
                }
                else if (objUpText->table() == TBL_RENSEIGNEMENTSMEDICAUXPATIENTS)
                    ItemsList::update(currentpatient(), objUpText->champ(), objUpText->toPlainText());
            }
        }
        else
        {
            UpLineEdit* objUpLine = qobject_cast<UpLineEdit*>(obj);
            if (objUpLine != Q_NULLPTR && obj != wdg_MGlineEdit && obj != wdg_autresCorresp1LineEdit && obj != wdg_autresCorresp2LineEdit)
            {
                if (obj == ui->ActeMontantlineEdit)
                    // le contrôle de sortie pour ActeMontantlineEdit est traité la méthode ValideActeMontant();
                    return QWidget::eventFilter(obj, event);
                if (objUpLine->valeuravant() != objUpLine->text())
                {
                    objUpLine->setText(Utils::trimcapitilize(objUpLine->text(),true));
                    if (objUpLine->table() == TBL_ACTES)
                    {
                        ItemsList::update( currentacte(), objUpLine->champ(),objUpLine->text());
                    }
                    else if (objUpLine->table() == TBL_RENSEIGNEMENTSMEDICAUXPATIENTS)
                    {
                        ItemsList::update(currentpatient(), objUpLine->champ(), objUpLine->text());
                        OKModifierTerrain(currentpatient());
                    }
                }
            }
            else if (obj == ui->ActeDatedateEdit)
            {
                QString date = ui->ActeDatedateEdit->text();
                if (date != m_dateActe)
                {
                    ItemsList::update(currentacte(), CP_DATE_ACTES, ui->ActeDatedateEdit->date());
                    m_dateActe       = ui->ActeDatedateEdit->text();
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
            QPushButton* Button = qobject_cast<QPushButton*>(obj);
            Button->setIcon(Icons::icFermeAppuye());
        }

    if(event->type() == QEvent::MouseMove)
        if (obj == ui->FermepushButton  || obj == ui->LFermepushButton)
        {
            QPushButton* Button = qobject_cast<QPushButton*>(obj);
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
            QPushButton* Button = qobject_cast<QPushButton*>(obj);
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
                 || obj == wdg_MGlineEdit                   || obj == ui->MGupComboBox
                 || obj == wdg_autresCorresp1LineEdit       || obj == wdg_autresCorresp2LineEdit
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
            if (qobject_cast<UpTextEdit *>(obj) != Q_NULLPTR)
            {
                UpTextEdit* objUpText = qobject_cast<UpTextEdit*>(obj);
                if (objUpText->table() == TBL_MESSAGES)
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
                || obj == wdg_MGlineEdit                    || obj == ui->MGupComboBox
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
            QLineEdit* Qobj = qobject_cast<QLineEdit*>(obj);
            switch (keyEvent->key())
            {
            case Qt::Key_Delete:
                Qobj->clear();
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
            if (obj == ui->TabaclineEdit)           wdg_MGlineEdit->setFocus();
            if (obj == wdg_MGlineEdit)                  wdg_autresCorresp1LineEdit->setFocus();
            if (obj == wdg_autresCorresp1LineEdit)      wdg_autresCorresp2LineEdit->setFocus();
        }

        // Flèche haut - variable suivant les cas ------------------------------------------------------------------------------------------------------------------------------------
        if (keyEvent->key()==Qt::Key_Up)
        {
            if (obj == ui->TabaclineEdit)               ui->AutresToxiquestextEdit->setFocus();
            if (obj == wdg_MGlineEdit)                      ui->TabaclineEdit->setFocus();
            if (obj == wdg_autresCorresp1LineEdit)          wdg_MGlineEdit->setFocus();
            if (obj == wdg_autresCorresp2LineEdit)          wdg_autresCorresp1LineEdit->setFocus();
        }

        // Flèche droit - variable suivant les cas ------------------------------------------------------------------------------------------------------------------------------------
        if (keyEvent->key() == Qt::Key_Right)
        {
            if (obj == ui->TabaclineEdit)           ui->AtcdtsOphstextEdit->setFocus();
            if (obj == ui->AutresToxiquestextEdit)  ui->TabaclineEdit->setFocus();
            if (obj == wdg_MGlineEdit)                  wdg_autresCorresp1LineEdit->setFocus();
            if (obj == wdg_autresCorresp1LineEdit)      wdg_autresCorresp2LineEdit->setFocus();
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
            if (obj == wdg_MGlineEdit)                      ui->TabaclineEdit->setFocus();
            if (obj == wdg_autresCorresp1LineEdit)          wdg_MGlineEdit->setFocus();
            if (obj == wdg_autresCorresp2LineEdit)          wdg_autresCorresp1LineEdit->setFocus();
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
    Datas::I()->actes->setcurrentacte(acte);
    if (acte == Q_NULLPTR)
        return;

    int nbActes (0);
    int noActe (0);
    //1. retrouver le créateur de l'acte et le soignant superviseur de l'acte
    QString nomsuperviseur(""), superviseurlogin ("");
    User * usr = Datas::I()->users->getById(acte->idUserSuperviseur());
    ui->ActeCotationcomboBox    ->disconnect();                       //! il faut faire ça pour éviter un foutoir de messages quand on navigue d'un acte à l'autre dans le dossier du patient
    if (usr != Q_NULLPTR)
    {
        nomsuperviseur =  usr->prenom() + " " + usr->nom();
        superviseurlogin = usr->login();
        ReconstruitListesCotations(usr);
    }
    QString createurconsult ("");
    if (Datas::I()->users->getById(acte->idCreatedBy()) != Q_NULLPTR)
        createurconsult = tr("Créé par ")
                + Datas::I()->users->getById(acte->idCreatedBy())->login()
                + tr(" pour ") + superviseurlogin; //Avant idPatient
    ui->CreeParlineEdit ->setText(createurconsult); //Avant idPatient
    ui->SitelineEdit->setText(Datas::I()->sites->getById(acte->idsite()) != Q_NULLPTR? Datas::I()->sites->getById(acte->idsite())->nom() : "");

    //2.  Retrouver l'acte défini par son idActe et afficher les champs
    ui->Acteframe               ->setVisible(true);
    ui->CreerActepushButton_2   ->setVisible(false);
    ui->CreerBOpushButton_2     ->setVisible(false);

    ui->ActeDatedateEdit        ->setDate(acte->date());
    ui->ActeDatedateEdit        ->setEnabled(false);
    ui->ActeMotiftextEdit       ->setText(acte->motif());
    QString path = proc->AbsolutePathDirImagerie() + NOM_DIR_IMAGES + "/" + acte->date().toString("yyyy-MM-dd");
    if (Utils::mkpath(path))
        ui->ActeTextetextEdit   ->document()->setBaseUrl("file://" + path);
    ui->ActeTextetextEdit       ->setText(acte->texte());
    ui->ActeConclusiontextEdit  ->setText(acte->conclusion());
    ui->idActelineEdit          ->setText(QString::number(acte->id()));
    ui->CourrierAFairecheckBox  ->setChecked(acte->courrierAFaire());

    ui->ActeCotationcomboBox    ->setCurrentText(acte->cotation());
    ConnectCotationComboBox();
    // on affiche tous les montants en euros, même ce qui a été payé en francs.
    double H = 1;
    if (acte->isFactureEnFranc())
    {
        ui->ActeMontantLabel    ->setText(tr("Montant") + "(" + QLocale().currencySymbol() + ")\n(payé en F)");
        H = 6.55957;
    }
    else
        ui->ActeMontantLabel    ->setText(tr("Montant") + "(" + QLocale().currencySymbol() + ")");
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

    ui->CreerActepushButton     ->setToolTip(tr("Créer un nouvel acte pour ") + currentpatient()->prenom() + " " + currentpatient()->nom());
    ui->CreerBOpushButton       ->setToolTip(tr("Créer un bilan orthoptique pour ") + currentpatient()->prenom() + " " + currentpatient()->nom());

    QMap<QString,QVariant>  Age = Utils::CalculAge(currentpatient()->datedenaissance(), ui->ActeDatedateEdit->date());
    ui->AgelineEdit             ->setText(Age["toString"].toString());
    ui->AgelineEdit             ->setAlignment(Qt::AlignCenter);

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
            ui->OuvreActesPrecspushButton->setToolTip(tr("Voir les consultations précédentes de ") + currentpatient()->prenom() + " " + currentpatient()->nom());
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
    m_autorModifConsult = false;
    AfficheActeCompta(currentacte());
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
    ui->Cotationframe->setEnabled(a && currentuser()->ishisownsupervisor());
    ui->CCAMlinklabel->setVisible(a && currentuser()->ishisownsupervisor());
    ui->EnregistrePaiementpushButton->setVisible(a && currentuser()->ishisownsupervisor());
    ui->ModifierCotationActepushButton->setVisible(!a && currentuser()->ishisownsupervisor());
    if (a) // seul le superviseur de l'acte ou son parent peuvent modifier sa cotation
    {
        int iduser = currentacte()->idUserSuperviseur();
        int idparent = currentacte()->idParent();
        ui->EnregistrePaiementpushButton->setEnabled(ui->ActeCotationcomboBox->lineEdit()->text()!=""
                                                    && ( iduser == currentuser()->id() || idparent == currentuser()->id()));
        return;
    }

    //2. on recherche ensuite le type de paiement : espèces, chèque, tiers, cb, impayé, gratuit

    if (acte->paiementType() == TRS  && acte->paiementTiers() != "CB") ui->PaiementlineEdit->setText(acte->paiementTiers());

    QString txtpaiement = Utils::ConvertitModePaiement(acte->paiementType());
    if (acte->paiementType() == TRS
            && acte->paiementTiers() == "CB") txtpaiement = tr(CARTECREDIT);
     else if (acte->paiementType() == TRS) txtpaiement = acte->paiementTiers();
    ui->PaiementlineEdit->setText(txtpaiement);

    // on calcule le montant payé pour l'acte
    if (acte->paiementType() != GRAT || acte->paiementType() != IMP)
    {
        double TotalPaye = 0;
        for (auto it = m_lignespaiements->lignespaiements()->constBegin(); it != m_lignespaiements->lignespaiements()->constEnd(); ++it)
        {
            LignePaiement *lign = const_cast<LignePaiement*>(it.value());
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

    if ((acte->paiementType() == TRS
            && acte->paiementTiers() == "CB")
        || acte->paiementType() == CHQ
        || acte->paiementType() == ESP)
    {
        ui->PaiementLabel->setVisible(true);
        ui->PaiementLabel->setText("Paiement:");
        ui->PayeLabel->setVisible(true);
        ui->PayelineEdit->setVisible(true);
        ui->Comptaframe->setGeometry(580,510,180,50);
        ui->PaiementlineEdit->setGeometry(79,4,91,18);
        return;
    }
    if (acte->paiementType() == GRAT
        || acte->paiementType() == IMP)
    {
        ui->PaiementLabel->setVisible(false);
        ui->PayeLabel->setVisible(false);
        ui->PayelineEdit->setVisible(false);
        ui->Comptaframe->setGeometry(580,510,180,26);
        ui->PaiementlineEdit->setGeometry(8,4,164,18);
        return;
    }
    if (acte->paiementType() == TRS
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
    Datas::I()->patients->setcurrentpatient(pat);
    if (pat == Q_NULLPTR)
        return;

    RecalcCurrentDateTime();
    ui->DateCreationDossierlineEdit->setText(currentpatient()->datecreationdossier().isValid()?
                                                 currentpatient()->datecreationdossier().toString("d-M-yyyy") : "???");
    ui->idPatientlineEdit->setText(QString::number(currentpatient()->id()));
    if (!ui->tabDossier->isVisible())
    {
        ui->tabWidget->insertTab(1,ui->tabDossier,"");
        ui->tabWidget->setCurrentIndex(ui->tabWidget->indexOf(ui->tabDossier));
    }

    ui->IdCreateurDossierlineEdit   ->setText(Datas::I()->users->getById(currentpatient()->idcreateur())? Datas::I()->users->getById(currentpatient()->idcreateur())->login() : "");

    ui->IdentPatienttextEdit        ->setHtml(CalcHtmlIdentificationPatient(currentpatient()));
    ui->tabWidget                   ->setTabIcon(ui->tabWidget->indexOf(ui->tabDossier),CalcIconPatient(currentpatient()));
    ui->tabWidget                   ->setTabText(ui->tabWidget->indexOf(ui->tabDossier) ,currentpatient()->nom() + " " + currentpatient()->prenom());

    //3 - récupération des données médicales

    ui->AtcdtsPersostextEdit        ->setText(currentpatient()->atcdtspersos());
    ui->TtGeneraltextEdit           ->setText(currentpatient()->traitementgen());
    ui->AtcdtsFamiliauxtextEdit     ->setText(currentpatient()->atcdtsfamiliaux());
    ui->AtcdtsOphstextEdit          ->setText(currentpatient()->atcdtsophtalmos());
    ui->TabaclineEdit               ->setText(currentpatient()->tabac());
    ui->AutresToxiquestextEdit      ->setText(currentpatient()->toxiques());
    ui->ActeTextetextEdit           ->setiD(currentpatient()->id());

    wdg_MGlineEdit->clear();
    wdg_autresCorresp1LineEdit->clear();
    wdg_autresCorresp2LineEdit->clear();
    QString tooltp = "";
    if (currentpatient()->idmg()>0)
    {
        int id = currentpatient()->idmg();
        ui->MGupComboBox->setCurrentIndex
                (ui->MGupComboBox->findData(id));
        tooltp = CalcToolTipCorrespondant(id);
    }
    else
        ui->MGupComboBox->setCurrentIndex(-1);
    ui->MGupComboBox->setImmediateToolTip(tooltp);
    tooltp = "";
    if (currentpatient()->idspe1()>0)
    {
        int id = currentpatient()->idspe1();
        ui->AutresCorresp1upComboBox->setCurrentIndex
                (ui->AutresCorresp1upComboBox->findData(id));
        tooltp = CalcToolTipCorrespondant(id);
    }
    else
        ui->AutresCorresp1upComboBox->setCurrentIndex(-1);
    ui->AutresCorresp1upComboBox->setImmediateToolTip(tooltp);
    tooltp = "";
    if (currentpatient()->idspe2()>0)
    {
        int id = currentpatient()->idspe2();
        ui->AutresCorresp2upComboBox->setCurrentIndex
                (ui->AutresCorresp2upComboBox->findData(id));
        tooltp = CalcToolTipCorrespondant(id);
    }
    else
        ui->AutresCorresp2upComboBox->setCurrentIndex(-1);
    ui->AutresCorresp2upComboBox->setImmediateToolTip(tooltp);
    ui->ImportanttextEdit->setText(currentpatient()->important());
    ui->ResumetextEdit->setText(currentpatient()->resume());
    ui->TtOphtextEdit->setText(currentpatient()->traitementoph());
    OKModifierTerrain(currentpatient(), false);

    Datas::I()->docsexternes->initListeByPatient(currentpatient());
    FermeDlgActesPrecedentsEtDocsExternes();

    //3 - récupération des actes

    m_listeactes->initListeByPatient(currentpatient());
    m_lignespaiements->initListeByPatient(currentpatient());

    if (m_listeactes->actes()->size() == 0)
    {
        ui->Acteframe->setVisible(false);
        ui->CreerActepushButton_2->setVisible(true);
        ui->CreerBOpushButton_2->setVisible(true);
        ui->idActelineEdit->clear();
        Datas::I()->actes->setcurrentacte(Q_NULLPTR);
    }
    else
    {
        OuvrirDocsExternes();                   //! depuis AfficheDossier
        if (idacte > 0)
            AfficheActe(m_listeactes->getById(idacte));
        else
            AfficheActe(m_listeactes->actes()->last());
        if (m_listeactes->actes()->size() > 1)
            OuvrirActesPrecedents();            //! depuis AfficheDossier()
        ui->ActeMotiftextEdit->setFocus();
    }
    //4 - rapel des réfractions et réglage du refracteur
    Datas::I()->patients->setDonneesOphtaCurrentPatient();
    Datas::I()->refractions->initListebyPatId(currentpatient()->id());
    SetDatasRefractionKerato();
    /*! Cette fonction va créer 3 mesures
     * une de fronto qui reprendra la dernière prescription de verres du patient
     * une autoref qui reprendra la dernière mesure d'autoref du patient
     * et une de subjectif qui reprendra la dernière mesure d'acuité du patient
     Le refracteur sera réglé avec
     * la mesure fronto créée en fronto et refraction finale
     * la mesure autoref créée en autoref
     * et la mesure acuité créée en refraction subjective
     */
    if (proc->PortRefracteur()!=Q_NULLPTR || proc->LANRefracteur())
        proc->EnvoiDataPatientAuRefracteur();

    //5 - mise à jour du dossier en salle d'attente
    PatientEnCours * patcours= Q_NULLPTR;

    for (auto it = Datas::I()->patientsencours->patientsencours()->constBegin(); it != Datas::I()->patientsencours->patientsencours()->constEnd(); ++it)
    {
        PatientEnCours *patcrs = const_cast<PatientEnCours*>(it.value());
        if (patcrs->id() == currentpatient()->id())
        {
            patcours = patcrs;
            break;
        }
    }
    QTime currenttime = db->ServerDateTime().time();
    if (patcours == Q_NULLPTR)
        Datas::I()->patientsencours->CreationPatient( pat->id(),                                                //! idPat
                                                 Datas::I()->users->getById(currentuser()->idsuperviseur()),    //! User
                                                 ENCOURSEXAMEN + currentuser()->login(),                        //! Statut
                                                 currenttime,                                                   //! heureStatut
                                                 QTime(),                                                       //! heureRDV
                                                 currenttime,                                                   //! heureArrivee
                                                 "",                                                            //! Motif
                                                 "",                                                            //! Message
                                                 0,                                                             //! idActeAPayer
                                                 Utils::hostName().left(60),                           //! PosteExamen
                                                 currentuser()->id(),                                           //! idUserEnCoursExamen
                                                 0);                                                            //! idSalDat
    else
    {
        ItemsList::update(patcours, CP_STATUT_SALDAT, ENCOURSEXAMEN + currentuser()->login());
        ItemsList::update(patcours, CP_HEURESTATUT_SALDAT, currenttime);
        ItemsList::update(patcours, CP_IDUSERENCOURSEXAM_SALDAT, currentuser()->id());
        ItemsList::update(patcours, CP_POSTEEXAMEN_SALDAT, Utils::hostName().left(60));
    }

    ItemsList::update(currentpost(), CP_IDPATENCOURS_USRCONNECT, pat->id());

    ui->AtcdtsPersostextEdit->setFocus();
    RecaleTableView(currentpatient());
    CalcMotsCles(currentpatient());
    Flags::I()->MAJFlagSalleDAttente();
    if (currentuser()->id() > 1) return;
    QString prenom = currentpatient()->prenom();
    QString Sexe = "";
    QString req =   "select " CP_IDPAT_PATIENTS " from " TBL_PATIENTS
                    " where " CP_PRENOM_PATIENTS " = '" + prenom + "'"
                    " and (" CP_SEXE_PATIENTS " is null or " CP_SEXE_PATIENTS " = '')"
                    " and " CP_PRENOM_PATIENTS " <> 'Dominique' and " CP_PRENOM_PATIENTS " <> 'Claude'";
    //qDebug() << req;
    QList<QVariantList> patlist = db->StandardSelectSQL(req, m_ok);
    if (patlist.size()>0)
    {
        if (UpMessageBox::Question(this, tr("Il existe ") + QString::number(patlist.size()) + " " + prenom + tr(" dont le sexe n'est pas précisé."), tr("Les convertir?")) == UpSmallButton::STARTBUTTON)
        {
            UpMessageBox msgbox(this);
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
                db->StandardSQL("update " TBL_PATIENTS " set " CP_SEXE_PATIENTS " = '" + Sexe + "' where " CP_PRENOM_PATIENTS " = '" + prenom + "' and (" CP_SEXE_PATIENTS " is null or " CP_SEXE_PATIENTS " = '')");
                req ="select " CP_IDPAT_PATIENTS " from " TBL_PATIENTS " where " CP_SEXE_PATIENTS " = ''";
                QList<QVariantList> patlist = db->StandardSelectSQL(req,m_ok);
                if (m_ok && patlist.size()>0)
                UpMessageBox::Information(this, tr("Il reste ") + QString::number(patlist.size()) + tr(" dossiers pour lesquels le sexe n'est pas précisé"),"");
                currentpatient()->setSexe(Sexe);
                ui->IdentPatienttextEdit->setHtml(CalcHtmlIdentificationPatient(currentpatient()));
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
    if (!ui->Acteframe->isVisible())
        return FermeDossier(currentpatient());
    if (focusWidget() != Q_NULLPTR)
        focusWidget()->clearFocus();      //!> Valide les changements dans les champs du dossier en cours d'affichage

    /*! 1. On vérifie si on peut quitter la consultation sans quitter le dossier (il n'est pas obligatoire d'avoir la ligne correspondante dans typepaiementactes */
    if (ui->ActeCotationcomboBox->currentText() == "")
        Titre = tr("Il manque la cotation!");
    else if (ui->ActeMontantlineEdit->text() == "")
        Titre = tr("Il manque le montant!");
    if (Titre != "")
        AutorDepart = false;

    /* 2. On ne cherche pas à quitter le dossier mais seulement à se déplacer dans les consultations du dossier */
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
        /* 3 On veut quitter le dossier;
        * On vérifie si on peut quitter le dossier et la cohérence avec TypePaimentsActes */
        if (AutorDepart)
        {
            //! on recherche si le dernier acte du dossier est enregistré dans typepaiements - si le montant de l'acte est 0, on propose de l'enregistrer comme gratuit

            QString requete =   "SELECT max(act." CP_ID_ACTES "), " CP_DATE_ACTES ", " CP_COTATION_ACTES ", " CP_MONTANT_ACTES " FROM " TBL_ACTES
                    " act WHERE " CP_IDPAT_ACTES " = " + QString::number(currentpatient()->id()) +
                    " AND act." CP_ID_ACTES " NOT IN (SELECT typ." CP_IDACTE_TYPEPAIEMENTACTES " FROM " TBL_TYPEPAIEMENTACTES " typ)";

            QVariantList actdata = db->getFirstRecordFromStandardSelectSQL(requete,m_ok,tr("Impossible de retrouver le dernier acte du patient pour le contrôler!"));
            // cette requête renvoie toujours une table non vide en QT même si elle est vide en mysql... d'où la suite
            if (actdata.size()>0 && actdata.at(0).toInt() > 0) // =il n'y a pas de paiement enregistré pour le dernier acte
            {
                if (actdata.at(0).toInt() != currentacte()->id())
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
                        requete = "INSERT INTO " TBL_TYPEPAIEMENTACTES " (" CP_IDACTE_TYPEPAIEMENTACTES ", " CP_TYPEPAIEMENT_TYPEPAIEMENTACTES ") VALUES (" + QString::number(currentacte()->id()) + ",'G')";
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
            return FermeDossier(currentpatient());
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
    if (!proc->ConnexionBaseOK())
        exit(0);
    QList<dlg_paiementtiers *> PaimtList = findChildren<dlg_paiementtiers*>();
    if (PaimtList.size()>0)
        for (int i=0; i<PaimtList.size();i++)
            if (PaimtList.at(i)->isVisible())
            {
               Utils::playAlarm();
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
            return;
    }

    // le tab dossier est fermé, on vérifie s'il y a du monde en salle d'attente
    QString req = "SELECT " CP_STATUT_SALDAT ", " CP_IDPAT_SALDAT ", " CP_POSTEEXAMEN_SALDAT " FROM " TBL_SALLEDATTENTE " WHERE " CP_IDUSERSUPERVISEUR_SALDAT " = '" + QString::number(currentuser()->id()) + "'";
    QList<QVariantList> saldatlist = db->StandardSelectSQL(req,m_ok);
    if (m_ok && saldatlist.size()>0)
    {
    /* 2 possibilités
     * 1. C'est le seul poste connecté pour cet utilisateur
     * 2. cet utilisateur est connecté sur d'autres postes, on peut partir
     */
        bool IlResteDesPostesConnectesAvecCeUser = false;
        for (auto it = Datas::I()->postesconnectes->postesconnectes()->constBegin(); it != Datas::I()->postesconnectes->postesconnectes()->constEnd(); ++it)
        {
            PosteConnecte *post = const_cast<PosteConnecte*>(it.value());
            if (post->nomposte() != currentpost()->nomposte() && post->iduser() == currentpost()->id())
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
                        || Statut == ENCOURSEXAMEN + currentuser()->login()
                        || Statut == RETOURACCUEIL)
                {
                    // il y a du monde en salle d'attente, on refuse la fermeture
                    Utils::playAlarm();
                    UpMessageBox msgbox(this);
                    UpSmallButton OKBouton("OK");
                    UpSmallButton NoBouton(tr("Fermer quand même"));
                    msgbox.setText("Euuhh... " + currentuser()->login() + ", " + tr("vous ne pouvez pas fermer l'application."));
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
    close();
    exit(0);
}

/*-----------------------------------------------------------------------------------------------------------------
-- Chargement des données de l'utilisateur --------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
QString Rufus::CalcToolTipCorrespondant(int idcor)
{
    QString tooltp = "";
    Correspondant *cor = Datas::I()->correspondants->getById(idcor, Item::LoadDetails);
    if (!cor)
        return tooltp;
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
    AgeTotal        = Utils::CalculAge(pat->datedenaissance(), m_currentdate, pat->sexe());
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
    html += "<p class=\"p1\"><b>" + Age + "</b> (" + QLocale::system().toString(pat->datedenaissance(),tr("d MMM yyyy")) + ")</p>";                      //DDN
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
    if (db->parametres()->cotationsfrance())
        if ((pat->isald() || pat->iscmu()))
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
    AgeTotal        = Utils::CalculAge(pat->datedenaissance(), m_currentdate, pat->sexe());
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
    QList<QVariantList> mtcllist = db->StandardSelectSQL(req,m_ok);
    QString result ("<font color=\"green\">Mots clés: </font>");
    if (m_ok && mtcllist.size()>0)
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
    Datas::I()->patients->initListeTable(nom, prenom, true);
                //! mettre en place un filtre directement sur la liste est moins rapide que de réinterroger la BDD directement en SQL
    Remplir_ListePatients_TableView() ;   //FiltreTable()
    if (m_listepatientsmodel->rowCount()>0)
        RecaleTableView(getPatientFromRow(0), QAbstractItemView::PositionAtCenter);
    EnableButtons();
}

/*-----------------------------------------------------------------------------------------------------------------
-- Vérifie les verrous d'un dossier avant de l'afficher -----------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void Rufus::OuvrirDossier(Patient *pat, int idacte)  // appelée depuis la tablist ou la salle d'attente - vérifie qu'un dossier n'est pas verrouillé avant de l'afficher
{
    //qDebug() << "Rufus::OuvrirDossier" << pat->nomcomplet() << pat->id();
    if (pat == Q_NULLPTR)
        return;
    RecalcCurrentDateTime();
    if (currentuser()->isSecretaire())    // si l'utilisateur est une secrétaire, on propose de mettre le patient en salle d'attente
        InscritEnSalDat(pat);
    else if (currentuser()->isSoignant())
    {
        if (ui->tabWidget->indexOf(ui->tabDossier) > 0 && currentpatient() != Q_NULLPTR)
        {
            if (currentpatient()->id() == pat->id())
            {
                ui->tabWidget->setCurrentWidget(ui->tabDossier);
                return;
            }
            else
            {
                if (!AutorDepartConsult(true))
                    return;
            }
        }
        else
        {
            // On vérifie si le dossier n'est pas verrouillé par un autre utilisateur
            QString blabla = ENCOURSEXAMEN;
            int length = blabla.size();
            Datas::I()->patientsencours->initListeAll(); //TODO si on utilise le TCP, on peut se passer de ça parce qu'on peut mettre en place un message tcp pour chaque modif de la salle d'attente
            for (auto it = Datas::I()->patientsencours->patientsencours()->constBegin(); it != Datas::I()->patientsencours->patientsencours()->constEnd(); ++it)
            {
                PatientEnCours *patcrs = const_cast<PatientEnCours*>(it.value());
                if (patcrs->id() == pat->id() && patcrs->statut().left(length) == ENCOURSEXAMEN
                        && (patcrs->iduserencoursexam() != currentuser()->id() || (patcrs->iduserencoursexam() == currentuser()->id() && patcrs->posteexamen() != Utils::hostName().left(60))))
                {
                    UpMessageBox::Watch(this,tr("Impossible d'ouvrir ce dossier!"),
                                        tr("Ce patient est") + "\n" + patcrs->statut().toLower() + "\n" + tr("sur ") + patcrs->posteexamen());
                    return;
                }
            }
        }
        AfficheDossier(pat, idacte);
    }
}

/*-----------------------------------------------------------------------------------------------------------------
-- Créer un acte ------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void Rufus::CreerActe(Patient *pat)
{
    //qDebug() << pat->nomcomplet() << pat->id();
    if (pat == Q_NULLPTR)
        return;
    if (ui->Acteframe->isVisible())
        if(!AutorDepartConsult(false)) return;
    RecalcCurrentDateTime();
    Acte * acte = m_listeactes->CreationActe(pat, currentuser(), proc->idCentre(), Datas::I()->sites->idcurrentsite());
    //! on recherche si cet acte pourrait correspondre à une intervention programmée
    Intervention* interv = Datas::I()->interventions->getInterventionByDateIdPatient(m_currentdate, pat->id());
    if (interv != Q_NULLPTR)
    {
        ItemsList::update(interv, CP_IDACTE_LIGNPRGOPERATOIRE, acte->id());
        acte->setidintervention(interv->id());
    }
    AfficheActe(acte);
    if (m_listeactes->actes()->size() > 1)
    {
        QList<dlg_actesprecedents *> listactesprecs = findChildren<dlg_actesprecedents *>();
        for (int i = 0; i<listactesprecs.size();i++)
            if (listactesprecs.at(i)->idcurrentpatient() != pat->id())
            {
                listactesprecs.at(i)->close();
                delete listactesprecs.at(i);
            }
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
    ui->CreerDDNdateEdit->setDate(m_datepardefaut);
    ui->CreerNomlineEdit->setVisible(!ui->CreerNomlineEdit->isVisible());
    ui->CreerPrenomlineEdit->setVisible(!ui->CreerPrenomlineEdit->isVisible());
    ui->CreerDDNdateEdit->setVisible(!ui->CreerDDNdateEdit->isVisible());
    ui->Nomlabel->setVisible(!ui->Nomlabel->isVisible());
    ui->Prenomlabel->setVisible(!ui->Prenomlabel->isVisible());
    ui->DDNlabel->setVisible(!ui->DDNlabel->isVisible());
    if (m_mode != RechercheDDN)
    {
        ui->ChercherDepuisListepushButton->setText(tr("Chercher avec\nnom et prénom"));
        ui->ChercherDepuisListepushButton->setIcon(Icons::icContact());
        m_mode = RechercheDDN;
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
    RecalcCurrentDateTime();
    QString PatNom, PatPrenom, PatDDN, PatCreePar, PatCreeLe;
    int idPat; // on n'utilise pas currentpatient()->id() qui ne sera initialisé qu'après que le dossier ait été réellement affiché.

    PatNom      = Utils::trimcapitilize(ui->CreerNomlineEdit->text(),true);
    PatPrenom   = Utils::trimcapitilize(ui->CreerPrenomlineEdit->text(),true);
    PatDDN      = ui->CreerDDNdateEdit->date().toString("yyyy-MM-dd");
    PatCreeLe   = m_currentdate.toString("yyyy-MM-dd");
    PatCreePar  = QString::number(currentuser()->id());
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
    if (ui->CreerDDNdateEdit->date() == m_datepardefaut)
    {
        Utils::playAlarm();
        UpMessageBox msgbox(this);
        UpSmallButton OKBouton(tr("Je confirme"));
        UpSmallButton NoBouton(tr("Annuler"));
        msgbox.setText("Euuhh... " + currentuser()->login());
        msgbox.setInformativeText(tr("Confirmez vous la date de naissance?") + "\n" + QLocale::system().toString(ui->CreerDDNdateEdit->date(),tr("d-MMM-yyyy")));
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
        if( currentuser()->isSoignant() )
            AfficheDossier(Datas::I()->patients->getById(idPat));
        else
        {
            UpMessageBox msgbox(this);
            msgbox.setText(tr("Ouverture du dossier de ")  + PatNom + " " + PatPrenom);
            msgbox.setInformativeText(tr("Inscrire le dossier en salle d'attente?"));
            msgbox.setIcon(UpMessageBox::Warning);
            UpSmallButton OKBouton(tr("Inscrire en\nsalle d'attente"));
            UpSmallButton NoBouton(tr("Ne pas inscrire"));
            msgbox.addButton(&NoBouton, UpSmallButton::CANCELBUTTON);
            msgbox.addButton(&OKBouton, UpSmallButton::STARTBUTTON);
            msgbox.exec();
            if (msgbox.clickedButton() == &OKBouton)
                InscritEnSalDat(Datas::I()->patients->getById(idPat));
        }
        return;
    }

    /*----------------------------------------------------------------------------------------------------------------
       // il faudrait faire une recherche de dossier avec orthographe similaire
    -----------------------------------------------------------------------------------------------------------------*/

    // 2. On recherche ensuite un dossier similaire Nom + Prenom
    if (ui->PatientsListeTableView->isVisible()  && m_listepatientsmodel->rowCount()>0)
    {
        UpMessageBox msgbox(this);
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
            Datas::I()->patientsencours->SupprimePatientEnCours(Datas::I()->patientsencours->getById(pat->id()));
            Datas::I()->patients->SupprimePatient(pat);
            Datas::I()->patients->initListeTable(ui->CreerNomlineEdit->text(), ui->CreerPrenomlineEdit->text());
            return;
        }
        FiltreTable(pat->nom(), pat->prenom());

        // Si le User est un soignant, on crée d'emblée une consultation et on l'affiche
        if( currentuser()->isSoignant() )
        {
            UpMessageBox msgbox(this);
            msgbox.setText(tr("Dossier ") + pat->prenom() + " " + pat->nom() + tr(" créé"));
            msgbox.setInformativeText(tr("Ouvrir le dossier ou inscrire le dossier en salle d'attente?"));
            msgbox.setIcon(UpMessageBox::Quest);
            UpSmallButton SalDatBouton(tr("Inscrire le dossier\nen salle d'attente"));
            UpSmallButton OuvrirBouton(tr("Ouvrir le dossier"));
            UpSmallButton NeRienFaireBouton(tr("Ne rien faire"));
            msgbox.addButton(&NeRienFaireBouton,UpSmallButton::CLOSEBUTTON);
            msgbox.addButton(&SalDatBouton, UpSmallButton::NOBUTTON);
            msgbox.addButton(&OuvrirBouton, UpSmallButton::NOBUTTON);
            SalDatBouton.setIcon(Icons::icAttente());
            OuvrirBouton.setIcon(Icons::icSortirDossier());
            msgbox.setDefaultButton(&NeRienFaireBouton);
            //NoBouton->setFocus();
            msgbox.exec();
            if (msgbox.clickedButton() == &SalDatBouton)
            {
                if (!InscritEnSalDat(pat))
                    RecaleTableView(pat);
            }
            else if (msgbox.clickedButton() == &OuvrirBouton)
            {
                Datas::I()->patients->setcurrentpatient(pat);
                CreerActe(pat);
                AfficheDossier(pat);
            }
            else
                RecaleTableView(pat);
        }
        else
            if (!InscritEnSalDat(pat))
                RecaleTableView(pat);
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
    actionFabricants                = new QAction(tr("Liste des fabricants"));
    actionIOLs                      = new QAction(tr("Liste des implants"));
    actionTiers                     = new QAction(tr("Liste des tiers payants"));

    actionPaiementDirect            = new QAction(tr("Gestion des paiements directs"));
    actionPaiementTiers             = new QAction(tr("Gestion des tiers payants"));
    actionBilanRecettes             = new QAction(tr("Bilan des recettes"));
    actionRecettesSpeciales         = new QAction(tr("Enregistrement des recettes spéciales"));
    actionJournalDepenses           = new QAction(tr("Journal des dépenses"));
    actionGestionComptesBancaires   = new QAction(tr("Gestion des comptes bancaires"));
    actionRemiseCheques             = new QAction(tr("Effectuer une remise de chèques"));

    actionApropos                   = new QAction(tr("A propos"));
    actionQuit                      = new QAction(tr("Quitter"));
    actionQuit                      ->setMenuRole(QAction::PreferencesRole);
    // Les connect des actions --------------------------------------------------------------------------------------------------
    connect (actionQuit,                        &QAction::triggered,        this,                   &Rufus::close);
    connect (actionApropos,                     &QAction::triggered,        this,                   &Rufus::Apropos);
    connect (actionCreerDossier,                &QAction::triggered,        this,                   &Rufus::ModeCreationDossier);
    connect (actionOuvrirDossier,               &QAction::triggered,        this,                   &Rufus::ModeSelectDepuisListe);
    connect (actionSupprimerDossier,            &QAction::triggered,        this,                   [=] { SupprimerDossier(currentpatient()); });
    connect (actionRechercheParMotCle,          &QAction::triggered,        this,                   &Rufus::RechercheParMotCle);
    connect (actionRechercheParID,              &QAction::triggered,        this,                   &Rufus::RechercheParID);
    connect (actionCreerActe,                   &QAction::triggered,        this,                   [=] {CreerActe(currentpatient());});

    connect (actionParametres,                  &QAction::triggered,        this,                   &Rufus::OuvrirParametres);
    connect (actionResumeStatut,                &QAction::triggered,        this,                   [=] {
                                                                                                            if (m_utiliseTCP)
                                                                                                                envoieTCPMessage(TCPMSG_AskListeStringId);
                                                                                                            else
                                                                                                            {
                                                                                                                ResumeStatut();
                                                                                                                proc->Edit(m_resumeStatut, tr("Information statut"), false, true );
                                                                                                            }
                                                                                                        });
    connect (actionSupprimerActe,               &QAction::triggered,        this,                   [=] {SupprimerActe(currentacte());});
    // Documents
    connect (actionEmettreDocument,             &QAction::triggered,        this,                   [=] {ImprimeDocument(currentpatient());});
    connect (actionDossierPatient,              &QAction::triggered,        this,                   [=] {ImprimeDossier(currentpatient());});
    connect (actionCorrespondants,              &QAction::triggered,        this,                   &Rufus::ListeCorrespondants);
    connect (actionFabricants,                  &QAction::triggered,        this,                   &Rufus::ListeManufacturers);
    connect (actionIOLs,                        &QAction::triggered,        this,                   &Rufus::ListeIOLs);
    connect (actionTiers,                       &QAction::triggered,        this,                   &Rufus::ListeTiersPayants);
    connect (actionEnregistrerDocScanner,       &QAction::triggered,        this,                   [=] {EnregistreDocScanner(currentpatient());});
    connect (actionEnregistrerVideo,            &QAction::triggered,        this,                   [=] {EnregistreVideo(currentpatient());});
    connect (actionExportActe,                  &QAction::triggered,        this,                   [=] {ExporteActe(currentacte());});
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
    if (currentuser()->isSoignant())
        menuActe        = menuBar()->addMenu(tr("Acte"));
    else
        menuActe        = Q_NULLPTR;
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

#ifdef Q_OS_MACOS
    menuBar()   ->addAction(actionApropos);
    menuBar()   ->addAction(actionQuit);
#endif
#if defined(Q_OS_LINUX) || defined(Q_OS_WIN)
    menuAide    = menuBar()->addMenu(tr("Aide"));
    menuAide    ->addAction(actionApropos);
    menuDossier ->addAction(actionQuit);
#endif

    menuEdition->addAction(tr("Copier"));
    menuEdition->addAction(tr("Couper"));
    menuEdition->addAction(tr("Coller"));
    menuEdition->addSeparator();
    menuEdition->addAction(actionParametres);
    menuEdition->addAction(actionResumeStatut);

    if (menuActe != Q_NULLPTR) {
        menuActe    ->addAction(actionCreerActe);
        menuActe    ->addAction(actionSupprimerActe);
    }

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
    menuComptabilite->addAction(actionTiers);

    // Les connect des menus --------------------------------------------------------------------------------------------------
    if(menuActe != Q_NULLPTR)
        connect (menuActe,                      &QMenu::aboutToShow,        this,                   [=] {AfficheMenu(menuActe);});
    if(menuAide != Q_NULLPTR)
        connect (menuAide,                      &QMenu::aboutToShow,        this,                   [=] {AfficheMenu(menuAide);});
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
    dlg_identificationcorresp *Dlg_IdentCorresp = new dlg_identificationcorresp(dlg_identificationcorresp::Creation, onlydoctors, Q_NULLPTR, this);
    Dlg_IdentCorresp->ui->NomlineEdit->setText(Nom);
    Dlg_IdentCorresp->ui->PrenomlineEdit->setFocus();
    if (Cor == "MG")
        Dlg_IdentCorresp->ui->MGradioButton->setChecked(true);
    if (Dlg_IdentCorresp->exec() == QDialog::Accepted)
        idcor = Dlg_IdentCorresp->idcurrentcorrespondant();
    delete Dlg_IdentCorresp;
    return idcor;
}

/*-----------------------------------------------------------------------------------------------------------------
-- Exporte l'acte en cours en réunissant dans un même dossier le contenu de l'acte en pdf et les documents d'imagerie liés à cet acte
-----------------------------------------------------------------------------------------------------------------*/
void Rufus::ExporteActe(Acte *act)
{
    if (act == Q_NULLPTR)
        return;
    Patient *pat = Datas::I()->patients->getById(act->idPatient());
    if (pat == Q_NULLPTR)
        return;
    QString nomdossier = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).at((0)) + "/" + pat->nom() + " " + pat->prenom() + " - " + QLocale::system().toString(act->date(),"d MMM yyyy");
    ImprimeListActes(QList<Acte*>() << act, false, true, nomdossier);
    QString req = "select " CP_ID_DOCSEXTERNES " from " TBL_DOCSEXTERNES
                  " where " CP_IDPAT_DOCSEXTERNES " = " + QString::number(pat->id()) +
                  " and DATE(" CP_DATE_DOCSEXTERNES ") = '" + currentacte()->date().toString("yyyy-MM-dd") + "' "
                  " and " CP_FORMATDOC_DOCSEXTERNES " = '" IMAGERIE "'";
    QList<QVariantList> listimages = db->StandardSelectSQL(req, m_ok);
    if (m_ok && listimages.size()>0)
    {
        for (int i=0; i<listimages.size(); i++)
        {
            DocExterne *docmt = Datas::I()->docsexternes->getById(listimages.at(i).at(0).toInt());
            if (!docmt)
                continue;
            QString filedest = pat->nom() + " " + pat->prenom() + " - " + docmt->typedoc() + " " + docmt->soustypedoc() + " " + QString::number(i);
            {
                if (db->ModeAccesDataBase() != Utils::Distant)
                {
                    QString fileorigin = proc->AbsolutePathDirImagerie() + NOM_DIR_IMAGES + docmt->lienversfichier();
                    QFile origin(fileorigin);
                    Utils::copyWithPermissions(origin, nomdossier + "/" + filedest + "." + QFileInfo(origin).suffix().toLower());
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
                                    " where " CP_ID_ECHGIMAGES " = " + QString::number(docmt->id()) +
                                    " and " CP_FACTURE_ECHGIMAGES " is null");
                    QString req = "INSERT INTO " TBL_ECHANGEIMAGES " (" CP_ID_ECHGIMAGES ", " + sfx + ", " CP_COMPRESSION_ECHGIMAGES ")"
                                  " VALUES (" +
                                    QString::number(docmt->id()) + ", " +
                                    " LOAD_FILE('" + Utils::correctquoteSQL(db->dirimagerie() + NOM_DIR_IMAGES + Utils::correctquoteSQL(docmt->lienversfichier())) + "'), " +
                                    QString::number(docmt->compression()) + ")";
                    db->StandardSQL(req);

                    // On charge ensuite le contenu des champs longblob des tables concernées en mémoire pour les afficher
                    req = "select " + sfx + " from " TBL_ECHANGEIMAGES " where " CP_ID_ECHGIMAGES " = " + QString::number(docmt->id()) + " and " CP_FACTURE_ECHGIMAGES " is null";
                    QVariantList impr = db->getFirstRecordFromStandardSelectSQL(req, m_ok, tr("Impossible d'accéder à la table ") + TBL_ECHANGEIMAGES);
                    if (!m_ok || impr.size() == 0)
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

                        QBuffer buf(&ba);
                        buf.open(QIODevice::ReadWrite);
                        QPdfDocument document;
                        document.load(&buf);
                        if( document.pageCount() > 0)
                        {
                            QFile file(filedest);
                            if (file.open(QIODevice::NewOnly))
                            {
                                QDataStream out(&file);
                                out << ba;
                            }
                        }
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
                        tr("Le dossier ") + pat->nom() + " " + pat->prenom() + " - " + QLocale::system().toString(act->date(),"d MMM yyyy") +
                        tr(" a été créé sur le bureau") + msg );
    }
    MAJDocsExternes();              //ExporteActe()
}

/*-----------------------------------------------------------------------------------------------------------------
-- Fermeture de la fiche dlg_actesprecedents ----------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void Rufus::FermeDlgActesPrecedentsEtDocsExternes()
{
    QList<dlg_actesprecedents *> ListDialog = this->findChildren<dlg_actesprecedents *>();
    for (int n = 0; n <  ListDialog.size(); n++)
    {
        ListDialog.at(n)->close();
        delete ListDialog.at(n);
    }
    QList<dlg_docsexternes *> ListDialogDocs = this->findChildren<dlg_docsexternes *>();
    for (int n = 0; n < ListDialogDocs.size(); n++)
    {
        if (n == ListDialogDocs.size()-1)
            if (ListDialogDocs.at(n)->mode() == dlg_docsexternes::Normal)
                proc->settings()->setValue(Position_Fiche Nom_fiche_DocsExternes, ListDialogDocs.at(n)->saveGeometry());
        ListDialogDocs.at(n)->close();
    }
    if (currentpatient() != Q_NULLPTR)
        ui->OuvreDocsExternespushButton->setEnabled(!Datas::I()->docsexternes->docsexternes()->isEmpty());
}

/*-----------------------------------------------------------------------------------------------------------------
-- Fermeture du dossier en cours ----------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
bool Rufus::FermeDossier(Patient *patient)
{
    if (patient == Q_NULLPTR)
        return false;
    //qDebug() << "FermeDossier() " << patient << patient->nom()  << patient->prenom() << patient->id();
    bool a = true;
    UpMessageBox msgbox(this);
    msgbox.setInformativeText(tr("Garder le dossier en salle d'attente?"));
    msgbox.setText(tr("Fermeture du dossier de ") + patient->nom() + " " + patient->prenom());

    msgbox.setIcon(UpMessageBox::Quest);

    UpSmallButton SalDatBouton      (tr("Garder le dossier\nen salle d'attente"));
    UpSmallButton AnnulBouton       (tr("Annuler"));
    UpSmallButton CloseBouton       (tr("Fermer\nle dossier"));

    msgbox.addButton(&AnnulBouton,  UpSmallButton::CANCELBUTTON);
    msgbox.addButton(&SalDatBouton, UpSmallButton::STARTBUTTON);
    msgbox.addButton(&CloseBouton,  UpSmallButton::CLOSEBUTTON);
    msgbox.exec();
    if (msgbox.clickedButton() == &CloseBouton)                         // Fermer le dossier et le rtire de la liste despatients en cours
    {
        Datas::I()->patientsencours->SupprimePatientEnCours(Datas::I()->patientsencours->getById(patient->id()));
    }
    else if (msgbox.clickedButton() == &SalDatBouton)                   // Garder le dossier en salle d'attente
    {
        QString Message(""), Motif(""), idUser ("");
        PatientEnCours *patcours = Q_NULLPTR;
        for (auto it = Datas::I()->patientsencours->patientsencours()->constBegin(); it != Datas::I()->patientsencours->patientsencours()->constEnd(); ++it)
        {
            PatientEnCours *patcrs = const_cast<PatientEnCours*>(it.value());
            if (patcrs->id() == currentpatient()->id())
            {
                patcours = patcrs;
                break;
            }
        }
        if (patcours != Q_NULLPTR)
        {
            Motif = patcours->motif();
            Message = patcours->message();
            QTime heurerdv = QTime(0,0);
            QTime heurearrivee = QTime(0,0);
            idUser = QString::number(patcours->idusersuperviseur());
            RendezVous *rdv = Q_NULLPTR;
            if (Motif=="")
            {
                rdv = MotifRDV(patcours->id(), Motif, Message);
                if (rdv == Q_NULLPTR)
                    return false;
                Motif   = rdv->motif();
                Message = rdv->message();
                if (rdv->heurearrivee() == QTime() || rdv->heurearrivee() == QTime(0,0))
                    rdv->setheurearrivee(heurearrivee);
                if (rdv->heurerdv() == QTime() || rdv->heurerdv() == QTime(0,0))
                    rdv->setheurerdv(heurerdv);
                idUser  = QString::number(rdv->idsuperviseur());
            }
            if (rdv != Q_NULLPTR)
            {
                heurerdv = rdv->heurerdv();
                heurearrivee = rdv->heurearrivee();
                delete rdv;
            }
            else
            {
                heurerdv = patcours->heurerdv();
                heurearrivee = patcours->heurerarrivee();
            }
            ItemsList::update(patcours, CP_STATUT_SALDAT, ARRIVE);
            ItemsList::update(patcours, CP_IDUSERSUPERVISEUR_SALDAT, idUser);
            ItemsList::update(patcours, CP_IDUSERENCOURSEXAM_SALDAT);
            ItemsList::update(patcours, CP_POSTEEXAMEN_SALDAT);
            ItemsList::update(patcours, CP_MOTIF_SALDAT, Motif);
            ItemsList::update(patcours, CP_MESSAGE_SALDAT, Message);
            ItemsList::update(patcours, CP_HEURERDV_SALDAT, (heurerdv == QTime()? QTime::currentTime() : heurerdv));
            ItemsList::update(patcours, CP_HEUREARRIVEE_SALDAT, (heurearrivee == QTime()? QTime::currentTime() : patcours->heurerarrivee()));
        }
        else
            a = InscritEnSalDat(patient);
    }
    else a = false;            // Annuler et revenir au dossier
    if (a) {
        Datas::I()->patients        ->setcurrentpatient(Q_NULLPTR);
        Datas::I()->actes           ->setcurrentacte(Q_NULLPTR);
        Datas::I()->docsexternes    ->reset();
        ItemsList::update(currentpost(), CP_IDPATENCOURS_USRCONNECT);
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
    dlg_identificationpatient *Dlg_IdentPatient = new dlg_identificationpatient(mode, pat, this);

    if (Dlg_IdentPatient->exec() == QDialog::Accepted)
    {
        if (mode == dlg_identificationpatient::Modification)
        {
            //  Mise à jour de currentpatient() et de l'affichage si le dossier modifié est le dossier en cours
            Patients::updatePatient(pat);
            if (Datas::I()->patientsencours->patientsencours()->constFind(pat->id()) != Datas::I()->patientsencours->patientsencours()->constEnd())
            {
                Remplir_SalDat();
                Flags::I()->MAJFlagSalleDAttente();
            }
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
            if (pat == currentpatient())
            {
                ui->IdentPatienttextEdit->setHtml(CalcHtmlIdentificationPatient(currentpatient()));
                ui->tabWidget->setTabIcon(ui->tabWidget->indexOf(ui->tabDossier),CalcIconPatient(currentpatient()));
                ui->tabWidget->setTabText(ui->tabWidget->indexOf(ui->tabDossier) ,pat->nom() + " " + pat->prenom());
                if (ui->MGupComboBox->currentData().toInt() != currentpatient()->idmg())
                {
                    int e = ui->MGupComboBox->findData(currentpatient()->idmg());
                    ui->MGupComboBox->setCurrentIndex(e);
                    OKModifierTerrain(currentpatient());
                }
                QMap<QString,QVariant>  NewAge = Utils::CalculAge(currentpatient()->datedenaissance(), ui->ActeDatedateEdit->date());
                ui->AgelineEdit->setText(NewAge["toString"].toString());
            }
            unpatientaetecreeoumodifie = true;
        }

        else if (mode == dlg_identificationpatient::Creation)
        {
            unpatientaetecreeoumodifie = true;
        }

        else if (mode == dlg_identificationpatient::Copie)
        {
            pat = Dlg_IdentPatient->currentpatient();
            if (!pat)
                return false;
            FiltreTable(pat->nom(), pat->prenom());
            // Si le User est un soignant, on propose d'afficher le dossier et si oui, n crée une consutation d'emblée
            if( currentuser()->isSoignant() )
            {
                UpMessageBox msgbox(this);
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
                    CreerActe(pat);
                    AfficheDossier(pat);
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
        if (mode == dlg_identificationpatient::Copie && Dlg_IdentPatient->currentpatient() == Q_NULLPTR)  // il n'y a pas eu de copie parce que le dossssier existait déjà
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
    if (unpatientaetecreeoumodifie && Dlg_IdentPatient->currentpatient() != Q_NULLPTR && m_utiliseTCP)
        envoieTCPMessage(QString::number(Dlg_IdentPatient->currentpatient()->id()) + TCPMSG_MAJPatient);
    delete Dlg_IdentPatient;
    return unpatientaetecreeoumodifie;
}

/*-----------------------------------------------------------------------------------------------------------------
-- Initialisation divers (Validator, geometry...etc...) -----------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void Rufus::InitWidgets()
{
    //setFixedWidth(LARGEURLISTE);
    ui->tabWidget->setGeometry(5,10,-10,920);

    wdg_MGlineEdit->setStyleSheet(
    "UpLineEdit {background-color:white; border-style: none;}"
    "UpLineEdit:focus {border-style:none;}");
    wdg_MGlineEdit->setMaxLength(90);
    ui->MGupComboBox->setLineEdit(wdg_MGlineEdit);

    int a = -3;
    proc->ModifTailleFont(ui->Cotationframe,a);
    proc->ModifTailleFont(ui->Comptaframe,a);
    proc->ModifTailleFont(ui->CreeParlineEdit,a);
    proc->ModifTailleFont(ui->SitelineEdit,a);
    proc->ModifTailleFont(ui->NoActelabel,a);
    proc->ModifTailleFont(ui->InfoPatientframe,a);
    proc->ModifTailleFont(ui->EnregistrePaiementpushButton,a);
    a = -1;
    proc->ModifTailleFont(ui->RefractionpushButton,a);
    proc->ModifTailleFont(ui->TonometriepushButton,a);
    proc->ModifTailleFont(ui->PachymetriepushButton,a);
    proc->ModifTailleFont(ui->OuvrirDocumentpushButton,a);
    proc->ModifTailleFont(ui->OuvreActesPrecspushButton,a);
    proc->ModifTailleFont(ui->OuvreDocsExternespushButton,a);
    proc->ModifTailleFont(ui->CreerActepushButton,a);
    proc->ModifTailleFont(ui->CreerBOpushButton,a);
    proc->ModifTailleFont(ui->CreerInterventionpushButton,a);
    proc->ModifTailleFont(ui->MotsClesLabel,a);
    proc->ModifTailleFont(ui->ModifierCotationActepushButton,a);

    QFont font = qApp->font();
    font.setBold(true);
    ui->ActeDatedateEdit->setFont(font);
    if (db->ModeAccesDataBase() != Utils::Distant)
        ui->ActeTextetextEdit       ->setAcceptImageMimeDatas(true);

    wdg_autresCorresp1LineEdit->setStyleSheet(
    "UpLineEdit {background-color:white; border-style: none;}"
    "UpLineEdit:focus {border-style:none;}");
    wdg_autresCorresp1LineEdit->setMaxLength(90);
    ui->AutresCorresp1upComboBox->setLineEdit(wdg_autresCorresp1LineEdit);

    wdg_autresCorresp2LineEdit->setStyleSheet(
    "UpLineEdit {background-color:white; border-style: none;}"
    "UpLineEdit:focus {border-style:none;}");
    wdg_autresCorresp2LineEdit->setMaxLength(90);
    ui->AutresCorresp2upComboBox->setLineEdit(wdg_autresCorresp2LineEdit);

    ui->ActeCotationcomboBox->lineEdit()->setStyleSheet(
    "QLineEdit {background-color:white; border-style: none;}"
    "QLineEdit:focus {border-style:none;}");
    ui->ActeCotationcomboBox->lineEdit()->setMaxLength(20);
    ui->ActeCotationcomboBox->lineEdit()->setValidator( new QRegularExpressionValidator(Utils::rgx_cotation,this));
    ui->ActeCotationcomboBox->lineEdit()->setFont(ui->ActeMontantlineEdit->font());
    ui->ActeCotationcomboBox->setFont(ui->ActeMontantlineEdit->font());

    m_val = new upDoubleValidator(0, 99999 , 2, this);
    ui->ActeMontantlineEdit->setValidator(m_val);
    ui->PayelineEdit->setValidator(m_val);
    ui->TabaclineEdit->setValidator(new QRegularExpressionValidator(Utils::rgx_tabac,this));
    wdg_MGlineEdit->setValidator(new QRegularExpressionValidator(Utils::rgx_rx,this));
    wdg_autresCorresp1LineEdit->setValidator(new QRegularExpressionValidator(Utils::rgx_rx,this));
    wdg_autresCorresp2LineEdit->setValidator(new QRegularExpressionValidator(Utils::rgx_rx,this));
    ui->CreerNomlineEdit->setValidator(new QRegularExpressionValidator(Utils::rgx_rx,this));
    ui->CreerPrenomlineEdit->setValidator(new QRegularExpressionValidator(Utils::rgx_rx,this));
    ui->tabWidget->setTabText(0,tr("Liste des patients"));

    ui->ActeMontantlineEdit->setAlignment(Qt::AlignRight);
    ui->PayelineEdit->setAlignment(Qt::AlignRight);
    ui->CreerDDNdateEdit->setDate(m_datepardefaut);
    ui->ActeDatedateEdit->setMaximumDate(m_currentdate);
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

    ui->CreerDDNdateEdit->setDateRange(m_currentdate.addYears(-105),m_currentdate);

    QAction *pAction_VoirMessages = m_trayIconMenu->addAction(tr("Voir les messages"));
    connect (pAction_VoirMessages, &QAction::triggered, this,    &Rufus::AfficheBAL);

    ict_messageIcon = new QSystemTrayIcon(this);
    ict_messageIcon->setContextMenu(m_trayIconMenu);
    ict_messageIcon->setIcon(Icons::icPostit());
    connect(ict_messageIcon,        &QSystemTrayIcon::messageClicked,   this,   [=] {AfficheBAL();});

    dlg_msgRepons = new QDialog();
    dlg_msgBAL = new QDialog();

    ui->CCAMlinklabel->setText("<a href=\"" LIEN_CCAM "\">CCAM...</a>");

    QHBoxLayout *hlay = new QHBoxLayout();
    wdg_salledattenteTab  = new QTabBar;
    wdg_salledattenteTab  ->setExpanding(false);
    hlay        ->insertWidget(0,wdg_salledattenteTab);
    hlay        ->addSpacerItem(new QSpacerItem(5,5,QSizePolicy::Expanding,QSizePolicy::Fixed));
    hlay        ->setContentsMargins(0,0,0,0);
    hlay        ->setSpacing(0);
    connect (wdg_salledattenteTab,  &QTabBar::currentChanged,           this,    &Rufus::FiltreSalleDAttente);
    ui->SalDatLayout->insertLayout(1,hlay);
    ui->SalDatWidget->setVisible(false);

    QHBoxLayout *halay = new QHBoxLayout();
    wdg_accueilTab  = new QTabBar;
    wdg_accueilTab  ->setExpanding(false);
    halay        ->insertWidget(0,wdg_accueilTab);
    halay        ->addSpacerItem(new QSpacerItem(5,5,QSizePolicy::Expanding,QSizePolicy::Fixed));
    halay        ->setContentsMargins(0,0,0,0);
    halay        ->setSpacing(0);
    connect (wdg_accueilTab,        &QTabBar::currentChanged,           this,   [=] {FiltreAccueil(wdg_accueilTab->currentIndex());});
    ui->AccueilLayout->insertLayout(1,halay);
    ui->AccueilWidget->setVisible(false);

    wdg_modifIdentificationupSmallButton = new UpSmallButton(ui->IdentPatienttextEdit);
    wdg_modifIdentificationupSmallButton->setUpButtonStyle(UpSmallButton::EDITBUTTON);
    wdg_modifIdentificationupSmallButton->move(315,190);
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
    ui->AccueilupTableWidget->setColumnWidth(3,180);    //Cotation
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
    ui->PatientsVusupTableWidget->setColumnWidth(3,180);                 //Cotation
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

    ui->GratuitpushButton->setImmediateToolTip(tr(GRATUIT));
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
    wdg_MGlineEdit                      ->installEventFilter(this);
    wdg_autresCorresp1LineEdit          ->installEventFilter(this);
    wdg_autresCorresp2LineEdit          ->installEventFilter(this);
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
    if (QLocale().territory() == QLocale::Madagascar)
    {
        actionPaiementTiers             ->setVisible(currentuser()->isComptableActes() || currentuser()->isSecretaire());
        actionPaiementDirect            ->setVisible(currentuser()->isComptableActes() || currentuser()->isSecretaire() || currentuser()->isRemplacant());
        actionBilanRecettes             ->setVisible(currentuser()->isSecretaire() || currentuser()->isAutreFonction() || currentuser()->isComptableActes());
        actionRecettesSpeciales         ->setVisible(currentuser()->isComptableActes());
        actionJournalDepenses           ->setVisible(currentuser()->isComptableActes() || currentuser()->isSecretaire());
        actionGestionComptesBancaires   ->setVisible(currentuser()->isComptableActes());
        actionRemiseCheques             ->setVisible(currentuser()->isComptableActes() || currentuser()->isSecretaire());
        menuComptabilite                ->setVisible(currentuser()->isComptableActes() || currentuser()->isSecretaire());
    }
    else
    {
        bool a = (currentuser()->isComptableActes() || currentuser()->isSecretaire() || currentuser()->isSoignantSalarie());
        actionPaiementTiers             ->setVisible(a);
        actionPaiementDirect            ->setVisible(a || (currentuser()->isSoignantSalarie() && !currentuser()->isAssistant()) || currentuser()->isRemplacant());
        actionBilanRecettes             ->setVisible(a);
        actionRecettesSpeciales         ->setVisible(currentuser()->modecomptable().testFlag(User::ComptaNoMedical));
        actionJournalDepenses           ->setVisible(a && (Datas::I()->users->comptablesActes()->size() + Datas::I()->users->liberaux()->size() > 0));
        actionGestionComptesBancaires   ->setVisible(currentuser()->modecomptable().testFlag(User::ComptaNoMedical));
        actionRemiseCheques             ->setVisible(a);
        menuComptabilite                ->setVisible(a || (currentuser()->isSoignantSalarie() && !currentuser()->isAssistant()) || currentuser()->isRemplacant());
    }
    actionEnregistrerVideo          ->setVisible(db->ModeAccesDataBase() != Utils::Distant);
}

/*-----------------------------------------------------------------------------------------------------------------
-- Initialisation des variables -----------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void Rufus::InitVariables()
{
    m_autorModifConsult         = false;
    m_listeactes                = Datas::I()->actes;
    m_lignespaiements           = Datas::I()->lignespaiements;
    m_datepardefaut             = QDate::fromString("2000-01-01", "yyyy-MM-dd");
    m_isTotalMessagesAffiche    = true;

    wdg_MGlineEdit              = new UpLineEdit();
    wdg_autresCorresp1LineEdit  = new UpLineEdit();
    wdg_autresCorresp2LineEdit  = new UpLineEdit();
    m_datederniermessageuser    = QDateTime();

    ui->AtcdtsOphstextEdit      ->setchamp(CP_ATCDTSOPH_RMP);
    ui->AtcdtsOphstextEdit      ->settable(TBL_RENSEIGNEMENTSMEDICAUXPATIENTS);
    ui->TtOphtextEdit           ->setchamp(CP_TRAITMTOPH_RMP);
    ui->TtOphtextEdit           ->settable(TBL_RENSEIGNEMENTSMEDICAUXPATIENTS);
    ui->ImportanttextEdit       ->setchamp(CP_IMPORTANT_RMP);
    ui->ImportanttextEdit       ->settable(TBL_RENSEIGNEMENTSMEDICAUXPATIENTS);
    ui->ResumetextEdit          ->setchamp(CP_RESUME_RMP);
    ui->ResumetextEdit          ->settable(TBL_RENSEIGNEMENTSMEDICAUXPATIENTS);

    ui->AtcdtsPersostextEdit    ->setchamp(CP_ATCDTSPERSOS_RMP);
    ui->AtcdtsPersostextEdit    ->settable(TBL_RENSEIGNEMENTSMEDICAUXPATIENTS);
    ui->AtcdtsFamiliauxtextEdit ->setchamp(CP_ATCDTSFAMLXS_RMP);
    ui->AtcdtsFamiliauxtextEdit ->settable(TBL_RENSEIGNEMENTSMEDICAUXPATIENTS);
    ui->TtGeneraltextEdit       ->setchamp(CP_TRAITMTGEN_RMP);
    ui->TtGeneraltextEdit       ->settable(TBL_RENSEIGNEMENTSMEDICAUXPATIENTS);
    ui->AutresToxiquestextEdit  ->setchamp(CP_AUTRESTOXIQUES_RMP);
    ui->AutresToxiquestextEdit  ->settable(TBL_RENSEIGNEMENTSMEDICAUXPATIENTS);

    wdg_MGlineEdit              ->settable(TBL_RENSEIGNEMENTSMEDICAUXPATIENTS);
    wdg_MGlineEdit              ->setchamp(CP_IDMG_RMP);
    wdg_autresCorresp1LineEdit  ->settable(TBL_RENSEIGNEMENTSMEDICAUXPATIENTS);
    wdg_autresCorresp1LineEdit  ->setchamp(CP_IDSPE1_RMP);
    wdg_autresCorresp2LineEdit  ->settable(TBL_RENSEIGNEMENTSMEDICAUXPATIENTS);
    wdg_autresCorresp2LineEdit  ->setchamp(CP_IDSPE2_RMP);
    ui->TabaclineEdit           ->settable(TBL_RENSEIGNEMENTSMEDICAUXPATIENTS);
    ui->TabaclineEdit           ->setchamp(CP_TABAC_RMP);

    ui->ActeMotiftextEdit       ->setchamp(CP_MOTIF_ACTES);
    ui->ActeMotiftextEdit       ->settable(TBL_ACTES);
    ui->ActeTextetextEdit       ->setchamp(CP_TEXTE_ACTES);
    ui->ActeTextetextEdit       ->settable(TBL_ACTES);
    ui->ActeConclusiontextEdit  ->setchamp(CP_CONCLUSION_ACTES);
    ui->ActeConclusiontextEdit  ->settable(TBL_ACTES);

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
    QString requete = "SELECT " CP_IDPAT_PATIENTS ", " CP_NOM_PATIENTS ", " CP_PRENOM_PATIENTS ", " CP_DDN_PATIENTS ", " CP_DATECREATION_PATIENTS ", " CP_IDCREATEUR_PATIENTS
            " FROM " TBL_PATIENTS
            " WHERE " CP_NOM_PATIENTS " = '" + Utils::correctquoteSQL(PatNom) + "'"
            " AND " CP_PRENOM_PATIENTS " = '" + Utils::correctquoteSQL(PatPrenom) + "'"
            " AND " CP_DDN_PATIENTS " = '" + PatDDN +"'";

    QVariantList patdata = db->getFirstRecordFromStandardSelectSQL(requete, m_ok, MessageErreur);
    if (!m_ok)
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
    if (currentpatient() == Q_NULLPTR)
        return;
    UpComboBox* cbox = qobject_cast<UpComboBox*>(obj);
    if (cbox == Q_NULLPTR) return;
    UpLineEdit *Upline = qobject_cast<UpLineEdit*>(cbox->lineEdit());
    if (Upline == Q_NULLPTR) return;
    QString anc = cbox->valeuravant();
    QString nou = Utils::trimcapitilize(cbox->currentText(),true);
    cbox->setCurrentText(nou);
    int i = cbox->findText(nou, Qt::MatchFixedString);
    if (-1 < i && i < cbox->count())
        return;
    if (nou != "")
    {
        if (nou != anc)
        {
            UpMessageBox msgbox(this);
            msgbox.setText("Euuhh... " + currentuser()->login());
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
                        Datas::I()->patients->updateCorrespondant(currentpatient(), Correspondant::MG, Datas::I()->correspondants->getById(idcor));
                    else if (cbox == ui->AutresCorresp1upComboBox)
                        Datas::I()->patients->updateCorrespondant(currentpatient(), Correspondant::Spe1, Datas::I()->correspondants->getById(idcor));
                    else if (cbox == ui->AutresCorresp2upComboBox)
                        Datas::I()->patients->updateCorrespondant(currentpatient(), Correspondant::Spe2, Datas::I()->correspondants->getById(idcor));
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
            Datas::I()->patients->updateCorrespondant(currentpatient(), Correspondant::MG);
        else if (cbox == ui->AutresCorresp1upComboBox)
            Datas::I()->patients->updateCorrespondant(currentpatient(), Correspondant::Spe1);
        else if (cbox == ui->AutresCorresp2upComboBox)
            Datas::I()->patients->updateCorrespondant(currentpatient(), Correspondant::Spe2);
        cbox->setCurrentIndex(-1);
        OKModifierTerrain(currentpatient());
    }
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
    if (currentpatient() == Q_NULLPTR)
        return false;
    if(!AutorDepartConsult(false)) return false;
    //  Afficher les éléments de la tables Actes
    int idActe = -1;
    if (m_listeactes->actes()->size() < 2)
        return false;
    int a (0);
    a = m_listeactes->actes()->keys().indexOf(currentacte()->id());

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
        AfficheActe(m_listeactes->getById(idActe));     //! depuis NavigationConsult(ItemsList::POSITION i)
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
    if (currentpatient() == Q_NULLPTR)
        return;
    dlg_actesprecedents *Dlg_ActesPrecs = new dlg_actesprecedents(currentpatient(), this);
    Dlg_ActesPrecs->show();
}

/*-----------------------------------------------------------------------------------------------------------------
-- Ouvrir la fiche dlg_impressions------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void    Rufus::ImprimeDocument(Patient *pat)
{
    if (pat == Q_NULLPTR)
        return;
    Intervention * interv = Q_NULLPTR;
    if (pat == currentpatient())
        if (currentacte())
            if (currentacte()->isintervention())
                interv = Datas::I()->interventions->getById(currentacte()->idIntervention());
    dlg_impressions *Dlg_Imprs   = new dlg_impressions(pat, interv, this);
    bool success = true;
    if (Dlg_Imprs->exec() == QDialog::Accepted)
    {
        User *userEntete = Dlg_Imprs->userentete();
        if (userEntete == Q_NULLPTR)
        {
            UpMessageBox::Watch(this, tr("Impossible d'imprimer"),tr("Aucun émetteur n'est précisé pour l'impression"));
            return;
        }
        //qDebug() << userEntete->login() << pat->nomcomplet();
        QDate DateDoc = Dlg_Imprs->ui->dateImpressiondateEdit->date();

        bool ALD;
        QString imprimante = "";
        QMap<int, QMap<dlg_impressions::DATASAIMPRIMER, QString>> listdocs = Dlg_Imprs->mapdocsaimprimer();
        QMap<dlg_impressions::DATASAIMPRIMER, QString> mapdoc;
        foreach (mapdoc, listdocs)
        {
            bool Prescription           = (mapdoc.find(dlg_impressions::d_Prescription).value() == "1");
            bool AvecDupli              = (mapdoc.find(dlg_impressions::d_Dupli).value() == "1");
            bool Administratif          = (mapdoc.find(dlg_impressions::d_Administratif).value() == "1");
            QString Titre               =  mapdoc.find(dlg_impressions::d_Titre).value();
            QString TxtDocument         =  mapdoc.find(dlg_impressions::d_Texte).value();
            QMap<dlg_impressions::DATASAIMPRIMER, QString> mapdocfirst = listdocs.first();
            bool AvecChoixImprimante    = (mapdoc == mapdocfirst);            // s'il y a plusieurs documents à imprimer on détermine l'imprimante pour le premier et on garde ce choix pour les autres
            ALD                         = Dlg_Imprs->ui->ALDcheckBox->checkState() == Qt::Checked && Prescription && db->parametres()->cotationsfrance();
            proc                        ->setNomImprimante(imprimante);
            success                     = proc->Imprimer_Document(this, pat, userEntete, Titre,
                                                                  TxtDocument, DateDoc, Prescription, ALD,
                                                                  AvecDupli, AvecChoixImprimante, Administratif);
            if (!success)
                break;
            imprimante = proc->nomImprimante();
        }
    }
    delete Dlg_Imprs;
    if (currentpatient() != Q_NULLPTR)
        if (success && currentpatient()->id() == pat->id())
            MAJDocsExternes();              // depuis ImprimeDocument()
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
    if (m_mode == Liste && ui->tabWidget->currentIndex() == ui->tabWidget->indexOf(ui->tabList))
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

    FiltreTable();       //! Réaffiche tous les dossiers
    if (currentpatient() != Q_NULLPTR)
        RecaleTableView(currentpatient());
    else if (m_listepatientsmodel->rowCount() > 0)
        RecaleTableView(getPatientFromRow(0), QAbstractItemView::PositionAtTop);
    m_mode = Liste;
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

    if (m_mode == NouveauDossier && ui->tabWidget->currentIndex() == ui->tabWidget->indexOf(ui->tabList)) return;
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
    ui->CreerDDNdateEdit->setDate(m_datepardefaut);
    ui->DDNlabel->setVisible(true);
    ui->CreerDossierpushButton->setEnabled(ui->CreerNomlineEdit->text() != "" && ui->CreerPrenomlineEdit->text() != "");
    m_mode = NouveauDossier;
}

void Rufus::ProgrammationIntervention(Patient *pat, Acte *act)
{
    if (pat == Q_NULLPTR)
        return;
    Acte *chiract = Q_NULLPTR;
    if (act)
        if (act->isintervention())
            chiract = act;
    dlg_programmationinterventions *dlg_progr = new dlg_programmationinterventions(pat, chiract, this);
    connect(dlg_progr, &dlg_programmationinterventions::updateHtml, this, [&](Patient *chirpat) {
        if (chirpat->id() == pat->id())
            ui->IdentPatienttextEdit->setHtml(CalcHtmlIdentificationPatient(pat));
    });
    dlg_progr->exec();
    if (dlg_progr->docimprime())
        MAJDocsExternes();                  //ProgrammationIntervention()
    dlg_progr->disconnect();
    delete dlg_progr;
}

/*-----------------------------------------------------------------------------------------------------------------
-- Créer un dossier de la meme famille - Mise en place de la fiche ------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void    Rufus::RecopierDossier(Patient *patient)
{
    Patient* pat = patient;
    if (pat != Q_NULLPTR)
    {
        Datas::I()->patients->loadAll(patient, Item::Update);
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
        Datas::I()->patients->loadAll(pat, Item::Update);
    }
    FermeDlgActesPrecedentsEtDocsExternes();
    IdentificationPatient(dlg_identificationpatient::Copie, patient);
}

/*-----------------------------------------------------------------------------------------------------------------
-- Recale les TreeView sur le dossier en cours ---------------------------------------------------------------------
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

void    Rufus::ReconstruitListesCotations(User *usr)
{
    if (usr == Q_NULLPTR)
        return;
    Cotations *cots = Q_NULLPTR;
    /* reconstruit les items du combobox ui->ActeCotationComboBox
       chaque item contient
            . le texte de l'item -> la cotation de l'acte
            . en data, une QStringList contenant dans l'ordre le montant (optam ou non), le montant pratiqué, le descriptif de l'acte CCAM
    */
    User *userparent = usr;

    /*! si le user est connecté , on sélectionne la liste de cotations de son parent
     * il ne peut pas y avoir d'appel d'un user non soignant puisque la fonction est appelée pour un user qui est
            * soit le currentuser() qui est forcément soignant puisqu'on recrée la liste de cotations seulement dans cette condition
            * soit un user qui est le superviseur d'un acte donc forcément soignant aussi */
    if (usr == currentuser())
        userparent = Datas::I()->users->getById(usr->idparent());
    else if (proc->isUserConnected(usr))
        for (auto it = Datas::I()->postesconnectes->postesconnectes()->constBegin(); it !=  Datas::I()->postesconnectes->postesconnectes()->constEnd(); ++it)
        {
            PosteConnecte *post = const_cast<PosteConnecte*>(it.value());
            if (post->iduser() == usr->id())
            {
                userparent = Datas::I()->users->getById(post->idparent());
                if (userparent == Q_NULLPTR)
                    return;
                break;
            }
        }
    if (userparent ==  Q_NULLPTR)
        return;
     if (userparent != Q_NULLPTR)
    {
        auto itcot = Datas::I()->listecotations->constFind(userparent->id());
        if (itcot != Datas::I()->listecotations->constEnd())
            cots = itcot.value();
        else
        {
            cots = new Cotations();
            cots->initListeByUser(userparent);
            if (cots)
                Datas::I()->listecotations->insert(userparent->id(), cots);
    }

    //! le fait de ne pas réinitialiser le combobox permet de garder en item par défaut le dernier item utilisé qui est celui qu'on réutilisera la plupart du temps
    if (currentlistecotations() != Q_NULLPTR)
        if (cots->iduser() == currentlistecotations()->iduser())
            return;
    setcurrentlistecotations(cots);        }


    // il faut d'abord reconstruire la table des cotations
    ui->ActeCotationcomboBox->clear();

    ui->ActeCotationcomboBox->addItem(tr(GRATUIT),QStringList() << "0.00" << "0.00" << tr(GRATUIT));
    if (cots != Q_NULLPTR)
        for (auto it = cots->cotations()->constBegin(); it != cots->cotations()->constEnd(); ++it)
        {
            Cotation* cot = const_cast<Cotation*>(it.value());
            QStringList list;
            QString champ = QString::number(cot->montantconventionnel(), 'f', 2);
            list << champ << QString::number(cot->montantpratique(), 'f', 2) << cot->descriptif();
            ui->ActeCotationcomboBox->addItem(cot->typeacte(),list);
        }
}
void Rufus::ConnectCotationComboBox()
{
    connect (ui->ActeCotationcomboBox,  &QComboBox::currentTextChanged, this,
    [=] {
        RetrouveMontantActe();
        ValideActeMontantLineEdit(ui->ActeMontantlineEdit->text(), m_montantActe);
        ui->GratuitpushButton->setVisible(ui->ActeCotationcomboBox->currentIndex() != 0);
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
    if (currentpatient() != Q_NULLPTR)
    {
        if (currentpatient()->idmg()>0)
        {
            ui->MGupComboBox->setCurrentIndex(ui->MGupComboBox->findData(currentpatient()->idmg()));
            tooltp = CalcToolTipCorrespondant(currentpatient()->idmg());
        }
    }
    else
    {
        wdg_MGlineEdit->clear();
        ui->MGupComboBox->setCurrentIndex(-1);
    }
    ui->MGupComboBox->setImmediateToolTip(tooltp);
    tooltp = "";
    if (currentpatient() != Q_NULLPTR)
    {
        if (currentpatient()->idspe1()>0)
        {
            ui->AutresCorresp1upComboBox->setCurrentIndex
                    (ui->AutresCorresp1upComboBox->findData(currentpatient()->idspe1()));
            tooltp = CalcToolTipCorrespondant(currentpatient()->idspe1());
        }
    }
    else
    {
        wdg_autresCorresp1LineEdit->clear();
        ui->AutresCorresp1upComboBox->setCurrentIndex(-1);
    }
    ui->AutresCorresp1upComboBox->setImmediateToolTip(tooltp);
    tooltp = "";
    if (currentpatient() != Q_NULLPTR)
    {
        if (currentpatient()->idspe2()>0)
        {
            ui->AutresCorresp2upComboBox->setCurrentIndex
                    (ui->AutresCorresp2upComboBox->findData(currentpatient()->idspe2()));
            tooltp = CalcToolTipCorrespondant(currentpatient()->idspe2());
        }
    }
    else
    {
        wdg_autresCorresp2LineEdit->clear();
        ui->AutresCorresp2upComboBox->setCurrentIndex(-1);
    }
    ui->AutresCorresp2upComboBox->setImmediateToolTip(tooltp);
    OKModifierTerrain(currentpatient());
}

/*-----------------------------------------------------------------------------------------------------------------
-- Mesurer la Refraction ------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void    Rufus::RefractionMesure(dlg_refraction::ModeOuverture mode)
{
//    if (proc->FicheRefractionOuverte())
    if (findChildren<dlg_refraction*>().size()>0)
        return;
    if (currentpatient() == Q_NULLPTR || currentacte() == Q_NULLPTR)
        return;
    if (ui->tabWidget->currentIndex() != 1 || !ui->Acteframe->isVisible())
        return;
    dlg_refraction *Dlg_Refraction     = new dlg_refraction(mode, this);
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
            QString ARajouterEnText = HTML_RETOURLIGNE + Dlg_Refraction->ResultatObservation()  + "</p>" + HTML_FINPARAGRAPH;
            ItemsList::update(currentacte(), CP_TEXTE_ACTES, ui->ActeTextetextEdit->appendHtml(ARajouterEnText));
            ui->ActeTextetextEdit->setFocus();
            ui->ActeTextetextEdit->moveCursor(QTextCursor::End);
        }
        else if (Dlg_Refraction->ResultatPrescription() != "")  // C'est une prescription de verres correcteurs
        {
            // mettre à jour docsexterns
            MAJDocsExternes();              //Refraction()

            // si le dernier caractère n'est pas un retour à la ligne, on en rajoute un
            QString Date = "";
            for (int i= 0; i<Dlg_Refraction->ResultatPrescription().size();i++)
                if (Dlg_Refraction->ResultatPrescription().at(i).unicode() == 10) Dlg_Refraction->ResultatPrescription().replace(Dlg_Refraction->ResultatPrescription().at(i),"<br>");
            QString larg = "550";
            if (ui->ActeDatedateEdit->date() != m_currentdate)
            {
                Date = "<td width=\"80\">le " + m_currentdate.toString("d.M.yyyy") + "</td>";
                larg = "470";
            }
            QString ARajouterEnConcl =  HTML_RETOURLIGNE + Date + Dlg_Refraction->ResultatPrescription()  + "</p>" + HTML_FINPARAGRAPH;
            ItemsList::update(currentacte(), CP_CONCLUSION_ACTES, ui->ActeConclusiontextEdit->appendHtml(ARajouterEnConcl));
            ui->ActeConclusiontextEdit->setFocus();
            ui->ActeConclusiontextEdit->moveCursor(QTextCursor::End);
        }
    }
    delete Dlg_Refraction;
}

void Rufus::SetDatasRefractionKerato()
{
    if (currentpatient() == Q_NULLPTR)
        return;
    QMapIterator<int, Refraction*> itref(*Datas::I()->refractions->refractions());

    /*! On va créer 3 mesures
     * une de fronto qui reprendra la dernière prescription de verres du patient
     * une autoref qui reprendra la dernière mesure d'autoref du patient
     * et une de subjectif qui reprendra la dernière mesure d'acuité du patient
     Le refracteur sera réglé avec
     * la mesure fronto en fronto et refraction finale
     * la mesure autoref en autoref
     * et la mesure acuité en refraction subjective
     */
    Datas::I()->mesureautoref   ->cleandatas();
    Datas::I()->mesurefronto    ->cleandatas();
    Datas::I()->mesureacuite    ->cleandatas();
    Datas::I()->mesurefinal     ->cleandatas();
    Datas::I()->mesurekerato    ->cleandatas();
    Datas::I()->mesuretono      ->cleandatas();
    Datas::I()->mesurepachy     ->cleandatas();

    /*! Autoref     on cherche à régler la position autoref du refracteur - on utilise la dernière mesure d'acuité pour ça
     *  parce qu'elle sera aussi mise en refraction subjective
     *  (chez Nidek, on ne peut pas régler directement la réfraction subjective autrement qu'en réglant l'autoref...
                    * si on en n'a pas, on cherche la dernière mesure de fronto
                    * Si on utilise un Glasspop on va lui rentrer la mesure du joour d'autoref si on en a une puisque le Galsspop ne veut pas savoir s'il y a eu une refraction antérieure
 */
    itref.toBack();
    if (proc->settings()->value("Param_Poste/Refracteur").toString()=="NIDEK Glasspop")
        while (itref.hasPrevious()) {  // si on a un autoref frais du jour on le prend
            itref.previous();
            if (Datas::I()->actes->getById(itref.value()->idacte()) != Q_NULLPTR)
                if (itref.value()->typemesure() == Refraction::Autoref && Datas::I()->actes->getById(itref.value()->idacte())->date() == QDate::currentDate())
                {
                    Datas::I()->mesureautoref->setdatas(const_cast<Refraction*>(itref.value()));
                    itref.toFront();
                }
        }
    if (Datas::I()->mesureautoref->isdataclean())
    {
        itref.toBack();
        while (itref.hasPrevious()) {
            itref.previous();
            if (itref.value()->typemesure() == Refraction::Acuite)
            {
                Datas::I()->mesureautoref->setdatas(const_cast<Refraction*>(itref.value()));
                itref.toFront();
            }
        }
    }
    if (Datas::I()->mesureautoref->isdataclean())
    {
        itref.toBack();
        while (itref.hasPrevious()) {
            itref.previous();
            if (itref.value()->typemesure() == Refraction::Autoref)
            {
                Datas::I()->mesureautoref->setdatas(const_cast<Refraction*>(itref.value()));
                itref.toFront();
            }
        }
    }

    /*! Fronto et Final    on cherche à régler les positions fronto et final du refracteur - on utilise la dernière mesure de prescrition ou de fronto */
    itref.toBack();
    while (itref.hasPrevious()) {
        itref.previous();
        if ((itref.value()->typemesure() == Refraction::Prescription || itref.value()->typemesure() == Refraction::Fronto) && itref.value()->distance() != Refraction::Pres)
        {
            Datas::I()->mesurefronto->setdatas(const_cast<Refraction*>(itref.value()));
            Datas::I()->mesurefinal->setdatas(const_cast<Refraction*>(itref.value()), false);
            itref.toFront();
        }
    }

    /*! Acuite      on cherche à régler la position subjectif du refracteur - on utilise la dernière mesure d'acuité pour ça
                    * si on en n'a pas, on utilise la dernière mesure d'autoref
                    * si on en n'a pas, on utilise la dernière mesure de fronto */
    itref.toBack();
    while (itref.hasPrevious()) {
        itref.previous();
        if (itref.value()->typemesure() == Refraction::Acuite && itref.value()->distance() != Refraction::Pres)
        {
            Datas::I()->mesureacuite->setdatas(const_cast<Refraction*>(itref.value()), false);
            itref.toFront();
        }
    }
    if (Datas::I()->mesureacuite->isdataclean())
    {
        if (!Datas::I()->mesureautoref->isdataclean())
            Datas::I()->mesureacuite->setdatas(Datas::I()->mesureautoref);
        else if (!Datas::I()->mesureacuite->isdataclean())
            Datas::I()->mesureacuite->setdatas(Datas::I()->mesurefronto);
    }

    if (DataBase::I()->donneesOphtaPatient()->ismesurekerato())
    {
        Datas::I()->mesurekerato->setK1OD(DataBase::I()->donneesOphtaPatient()->K1OD());
        Datas::I()->mesurekerato->setK2OD(DataBase::I()->donneesOphtaPatient()->K2OD());
        Datas::I()->mesurekerato->setaxeKOD(Utils::roundToNearestFive(DataBase::I()->donneesOphtaPatient()->axeKOD()));
        Datas::I()->mesurekerato->setdioptriesK1OD(DataBase::I()->donneesOphtaPatient()->dioptriesK1OD());
        Datas::I()->mesurekerato->setdioptriesK2OD(DataBase::I()->donneesOphtaPatient()->dioptriesK2OD());
        Datas::I()->mesurekerato->setK1OG(DataBase::I()->donneesOphtaPatient()->K1OG());
        Datas::I()->mesurekerato->setK2OG(DataBase::I()->donneesOphtaPatient()->K2OG());
        Datas::I()->mesurekerato->setaxeKOG(Utils::roundToNearestFive(DataBase::I()->donneesOphtaPatient()->axeKOG()));
        Datas::I()->mesurekerato->setdioptriesK1OG(DataBase::I()->donneesOphtaPatient()->dioptriesK1OG());
        Datas::I()->mesurekerato->setdioptriesK2OG(DataBase::I()->donneesOphtaPatient()->dioptriesK2OG());
    }
    Datas::I()->mesureautoref   ->ConversionCylindreNeg();
    Datas::I()->mesurefronto    ->ConversionCylindreNeg();
    Datas::I()->mesureacuite    ->ConversionCylindreNeg();
    Datas::I()->mesurefinal     ->ConversionCylindreNeg();

    //proc->debugMesure(Datas::I()->mesurekerato);
}

/*-----------------------------------------------------------------------------------------------------------------
-- Remise de chèques ----------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void Rufus::RemiseCheques()
{
    dlg_remisecheques *Dlg_RemCheq = new dlg_remisecheques(this);
    if (Dlg_RemCheq->initOK())
        Dlg_RemCheq->exec();
    delete Dlg_RemCheq;
}

/*-----------------------------------------------------------------------------------------------------------------
-- Remplir la liste avec les noms, prénoms et DDN des patients ----------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
bool Rufus::Remplir_ListePatients_TableView()
{
    UpStandardItem *pitem0, *pitem1, *pitem2, *pitem3, *pitem4, *pitem5;
    if (m_listepatientsmodel != Q_NULLPTR)
        delete m_listepatientsmodel;
    m_listepatientsmodel = new QStandardItemModel(this);
    for (auto it = Datas::I()->patients->patientstable()->constBegin(); it != Datas::I()->patients->patientstable()->constEnd(); ++it)
    {
        Patient *pat = const_cast<Patient*>(it.value());
        pitem0  = new UpStandardItem(QString::number(pat->id()), pat);                                   // id                           -> utilisé pour le drop event
        pitem1  = new UpStandardItem(pat->nom().toUpper() + " " + pat->prenom(), pat);                   // Nom + Prénom
        pitem2  = new UpStandardItem(pat->datedenaissance().toString(tr("dd-MM-yyyy")), pat);            // date de naissance
        pitem3  = new UpStandardItem(pat->datedenaissance().toString(tr("yyyyMMdd")), pat);              // date de naissance inversée   -> utilisé pour le tri
        pitem4  = new UpStandardItem(pat->nom(), pat);                                                   // Nom                          -> utilisé pour le tri
        pitem5  = new UpStandardItem(pat->prenom(), pat);                                                // Prénom                       -> utilisé pour le tri
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
    ui->PatientsListeTableView->FixLargeurTotale();
    CalcNbDossiers();

    return true;
}

void Rufus::Remplir_SalDat()
{
    QTableWidget        *TableAMettreAJour;
    int                 i=0;
    QString             NomPrenom, zw;
    QFontMetrics        fm(qApp->font());
    QList<int> listidpat;
    Datas::I()->postesconnectes->initListe();
    Datas::I()->patientsencours->initListeAll();

    // toute la manip qui suit sert à remetre les patients en cours par ordre chronologique - si vous trouvez plus simple, ne vous génez pas
    QStandardItemModel      *m_listepatientsencoursmodel    = new QStandardItemModel();
    /*! Sort listpatients on rdv time */
    for (auto it = Datas::I()->patientsencours->patientsencours()->constBegin(); it != Datas::I()->patientsencours->patientsencours()->constEnd(); ++it)
    {
        PatientEnCours *patencrs = const_cast<PatientEnCours*>(it.value());
        QList<QStandardItem *> items;
        listidpat << patencrs->id();
        UpStandardItem *itempat = new UpStandardItem(QString::number(patencrs->id()));
        itempat->setitem(patencrs);
        items << new UpStandardItem(patencrs->heurerdv().toString("HHmm"))
              << itempat;
        m_listepatientsencoursmodel->appendRow(items);
    }
    QList<PatientEnCours*> listpat;
    m_listepatientsencoursmodel->sort(0);

    /*! create list idpat sorted on rdv time */
    for (int i=0; i<m_listepatientsencoursmodel->rowCount(); ++i)
    {
        UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_listepatientsencoursmodel->item(i,1));
        if (itm != Q_NULLPTR)
        {
            PatientEnCours *pat = qobject_cast<PatientEnCours*>(itm->item());
            if (pat != Q_NULLPTR)
            {
                //qDebug() << Datas::I()->patients->getById(pat->id())->nom() + " " + Datas::I()->patients->getById(pat->id())->prenom() + " " + pat->statut();
                listpat << pat;
            }
        }
    }

/*! ---------------------------------------------------------------------------------------------------------------------------------------------------------------
    1 - SALLE D'ATTENTE ----------------------------------------------------------------------------------------------------------
    --------------------------------------------------------------------------------------------------------------------------------------------------------------- */
    TableAMettreAJour = ui->SalleDAttenteupTableWidget;
    ui->SalleDAttenteupTableWidget->clearAllRowsExceptHeader();
    QList<PatientEnCours*> listpatsaldat;
    foreach (PatientEnCours* patcrs, listpat)
    {
        if (patcrs->statut() == ARRIVE || patcrs->statut() == ENCOURS || patcrs->statut().contains(ENATTENTENOUVELEXAMEN))
            listpatsaldat << patcrs;
    }
    TableAMettreAJour   ->setRowCount(listpatsaldat.size());

    if (m_listesuperviseursmodel != Q_NULLPTR)
        delete m_listesuperviseursmodel;
    m_listesuperviseursmodel = new QStandardItemModel(this);

    QStandardItem       *pitem0, *pitem1;
    QList<int>          listidusers;
    foreach (PatientEnCours* patencours, listpatsaldat)
    {
        Patient *pat    = Datas::I()->patients->getById(patencours->id());
        if (pat==Q_NULLPTR)
            continue;

        UpLabel *label0, *label1, *label2, *label3, *label4, *label5, *label6;
        label0 = new UpLabel(patencours->id(), "", TableAMettreAJour);
        label1 = new UpLabel(patencours->id(), "", TableAMettreAJour);
        label2 = new UpLabel(patencours->id(), "", TableAMettreAJour);
        label3 = new UpLabel(patencours->id(), "", TableAMettreAJour);
        label4 = new UpLabel(patencours->id(), "", TableAMettreAJour);
        label5 = new UpLabel(patencours->id(), "", TableAMettreAJour);
        label6 = new UpLabel(patencours->id(), "", TableAMettreAJour);

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
        NomPrenom   = " " + pat->nom().toUpper() + " " + pat->prenom();
        label0      ->setText(NomPrenom);                                                   // Nom + Prénom
        label1      ->setText(patencours->statut());                                        // Statut
        label4      ->setText(patencours->motif());                                         // Motif
        if (Msg != "")
            label2  ->setPixmap(Icons::pxApres().scaled(10,10));                            //WARNING : icon scaled : pxApres 10,10

        QString color;
        if (patencours->heurerarrivee().isValid())
        {
            QTime heureArriv    = patencours->heurerarrivee();
            label5              ->setText(heureArriv.toString("HH:mm"));                     // Heure RDV
            if (heureArriv.secsTo(QTime::currentTime())/60 < 15)
                color = "color: green";
            else if (heureArriv.secsTo(QTime::currentTime())/60 < 30)
                color = "color: orange";
            else
                color = "color: red";
        }
        QString superviseurlogin ("");
        User *superviseur = Datas::I()->users->getById(patencours->idusersuperviseur());
        if (superviseur != Q_NULLPTR)
            superviseurlogin = superviseur->login();
        label6->setText(superviseurlogin);  // Superviseur

        if (!listidusers.contains(patencours->idusersuperviseur()))
        {
            listidusers << patencours->idusersuperviseur();
            pitem0 = new QStandardItem(QString::number(patencours->idusersuperviseur()));
            pitem1 = new QStandardItem(superviseurlogin);
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
        if (patencours->isurgence())
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
        connect (label0,        &UpLabel::enter,            this,               [=] {AfficheMotif(label0);});
        connect (label1,        &UpLabel::enter,            this,               [=] {AfficheMotif(label1);});
        connect (label2,        &UpLabel::enter,            this,               [=] {AfficheMotif(label2);});
        connect (label3,        &UpLabel::enter,            this,               [=] {AfficheMotif(label3);});
        connect (label4,        &UpLabel::enter,            this,               [=] {AfficheMotif(label4);});
        connect (label5,        &UpLabel::enter,            this,               [=] {AfficheMotif(label5);});
        connect (label6,        &UpLabel::enter,            this,               [=] {AfficheMotif(label6);});
        connect (label0,        &UpLabel::clicked,          this,               [=] {SurbrillanceSalDat(label0);});
        connect (label1,        &UpLabel::clicked,          this,               [=] {SurbrillanceSalDat(label1);});
        connect (label2,        &UpLabel::clicked,          this,               [=] {SurbrillanceSalDat(label2);});
        connect (label3,        &UpLabel::clicked,          this,               [=] {SurbrillanceSalDat(label3);});
        connect (label4,        &UpLabel::clicked,          this,               [=] {SurbrillanceSalDat(label4);});
        connect (label5,        &UpLabel::clicked,          this,               [=] {SurbrillanceSalDat(label5);});
        connect (label6,        &UpLabel::clicked,          this,               [=] {SurbrillanceSalDat(label6);});
        connect (label0,        &UpLabel::dblclick,         this,               [=] {if (currentuser()->isSoignant()) OuvrirDossier(pat);});
        connect (label1,        &UpLabel::dblclick,         this,               [=] {if (currentuser()->isSoignant()) OuvrirDossier(pat);});
        connect (label2,        &UpLabel::dblclick,         this,               [=] {if (currentuser()->isSoignant()) OuvrirDossier(pat);});
        connect (label3,        &UpLabel::dblclick,         this,               [=] {if (currentuser()->isSoignant()) OuvrirDossier(pat);});
        connect (label4,        &UpLabel::dblclick,         this,               [=] {if (currentuser()->isSoignant()) OuvrirDossier(pat);});
        connect (label5,        &UpLabel::dblclick,         this,               [=] {if (currentuser()->isSoignant()) OuvrirDossier(pat);});
        connect (label6,        &UpLabel::dblclick,         this,               [=] {if (currentuser()->isSoignant()) OuvrirDossier(pat);});
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
    while (wdg_salledattenteTab->count()>0)
        wdg_salledattenteTab->removeTab(0);
    int k =0;
    if (m_listesuperviseursmodel->rowCount()==0)
        wdg_salledattenteTab->setVisible(false);
    else
    {
        wdg_salledattenteTab->setVisible(true);
        if (m_listesuperviseursmodel->rowCount()>1)
        {
            wdg_salledattenteTab  ->insertTab(0, Icons::icFamily(), tr("Tout le monde"));
            wdg_salledattenteTab  ->setTabData(k, -1);
            k++;
        }
        for (int i=0; i<m_listesuperviseursmodel->rowCount(); i++)
        {
            wdg_salledattenteTab  ->insertTab(k,m_listesuperviseursmodel->item(i,1)->text());
            wdg_salledattenteTab  ->setTabData(k, m_listesuperviseursmodel->item(i,0)->text());
            k++;
        }
        bool a = false;
        for (int i=0; i<wdg_salledattenteTab->count(); i++)
        {
            if (wdg_salledattenteTab->tabData(i).toInt() == currentuser()->idsuperviseur())
            {
                wdg_salledattenteTab->setCurrentIndex(i);
                a = true;
            }
        }
        if (!a)
            wdg_salledattenteTab->setCurrentIndex(0);
        FiltreSalleDAttente();
    }

/*! ---------------------------------------------------------------------------------------------------------------------------------------------------------------
    2 - BUREAUX ----------------------------------------------------------------------------------------------------------
    --------------------------------------------------------------------------------------------------------------------------------------------------------------- */

    QList<UpTextEdit *> listuptext = ui->scrollArea->findChildren<UpTextEdit*>();
    if (listuptext.size() > 0)
        for (int j=0; j<listuptext.size();j++)
            delete listuptext.at(j);
    ui->scrollArea->takeWidget();
    QList<PosteConnecte*> listpostsoignant;
    /*! sans TCP, on réinitialise la liste des postes connectés à chaque modif de la salle d'attente -
     *  avec TCP, c'est inutile puisque la réinitialisation se fait chaque fois qu'un poste se connecte ou se déconnecte */
    if (!m_utiliseTCP)
        Datas::I()->postesconnectes->initListe();
    for (auto it = Datas::I()->postesconnectes->postesconnectes()->constBegin(); it != Datas::I()->postesconnectes->postesconnectes()->constEnd(); ++it)
    {
        PosteConnecte* post = const_cast<PosteConnecte*>(it.value());
        if (post != Q_NULLPTR)
            if (Datas::I()->users->getById(post->iduser()) != Q_NULLPTR)
                if (Datas::I()->users->getById(post->iduser())->isSoignant())
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
            User *usr = Datas::I()->users->getById(post->iduser());
            QString usrlogin = (usr? usr->login() : "");
            QString PosteLog  = post->nomposte();
            PatientEnCours *patencours = Q_NULLPTR;
            foreach (PatientEnCours *patcrs, *Datas::I()->patientsencours->patientsencours())
                if (patcrs->iduserencoursexam() == post->iduser() && patcrs->posteexamen() == post->nomposte() && patcrs->id() == post->idpatencours())
                {
                    patencours = patcrs;
                    break;
                }
            UpTextEdit *UserBureau;
            UserBureau = new UpTextEdit;
            UserBureau->disconnect(); // pour déconnecter la fonction MenuContextuel intrinsèque de la classe UpTextEdit
            UserBureau->setObjectName(usrlogin + "BureauupTextEdit");
            UserBureau->setIdUser(post->iduser());
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
            html += "<p class=\"p10\"><b>" + PosteLog + "</b></p><p class=\"p2\"><b><span style=\"color:green;\">" + usrlogin + "</b></p>";
            if (post->idpatencours() == 0)
                html += "<p class=\"p2\">ZZzzz...</p>";
            else if (patencours == Q_NULLPTR)
                html += "<p class=\"p2\">ZZzzz...</p>";
            else
            {
                Patient *pat    = Datas::I()->patients->getById(post->idpatencours());
                if (pat != Q_NULLPTR)
                {
                    connect(UserBureau,         &QWidget::customContextMenuRequested,   this,   [=] {MenuContextuelBureaux(UserBureau);});
                    connect(UserBureau,         &UpTextEdit::dblclick,                  this,   [=] {AutreDossier(pat);});
                    html += "<p class=\"p2\">" +  pat->nom() + " " + pat->prenom() + "</p>";      //Nom Prenom
                    QString color = "black";
                    if (patencours->heurerarrivee().isValid())
                    {
                        QTime heureArriv = patencours->heurerarrivee();
                        if (heureArriv.secsTo(QTime::currentTime())/60 < 15)        color = "green";
                        else if (heureArriv.secsTo(QTime::currentTime())/60 < 30)   color = "orange";
                        else                                                        color ="red";
                    }
                    html += "<p class=\"p2\"><span style=\"color:" + color + ";\">" +  patencours->heurerarrivee().toString("HH:mm") + "</span></p>";   //heure arrivée
                }
                else
                    html += "<p class=\"p2\">ZZzzz...</p>";
            }
            html += "</body></html>";
            UserBureau->setHtml(html);
            lay->addWidget(UserBureau);
        }
    }


/*! ---------------------------------------------------------------------------------------------------------------------------------------------------------------
    3 - ACCUEIL ----------------------------------------------------------------------------------------------------------
    --------------------------------------------------------------------------------------------------------------------------------------------------------------- */

    TableAMettreAJour = ui->AccueilupTableWidget;
    TableAMettreAJour->clearContents();
    QList<PatientEnCours*> listpatvus;
    foreach (PatientEnCours* pat, listpat)
    {
        //qDebug() << Datas::I()->patients->getById(pat->id())->nom() + " " + Datas::I()->patients->getById(pat->id())->prenom() + " " + pat->statut();
        if (pat->statut() == RETOURACCUEIL)
            listpatvus << pat;
    }
    TableAMettreAJour   ->setRowCount(listpatvus.size());
    if (m_listesuperviseursaccueilmodel != Q_NULLPTR)
        delete m_listesuperviseursaccueilmodel;
    m_listesuperviseursaccueilmodel = new QStandardItemModel(this);
    QStandardItem       *oitem0, *oitem1;
    QList<int>          listidsuperviseurs;
    i = 0;
    foreach (PatientEnCours *patencours, listpatvus)
    {
        Patient *pat    = Datas::I()->patients->getById(patencours->id());
        if (pat==Q_NULLPTR)
            continue;
        Acte* actapayer             = Datas::I()->actes->getById(patencours->idacteapayer());
        if (actapayer == Q_NULLPTR)
            continue;

        QString superviseurlogin    = (Datas::I()->users->getById(patencours->idusersuperviseur()) != Q_NULLPTR? Datas::I()->users->getById(patencours->idusersuperviseur())->login() : "");

        UpLabel *label0, *label1, *label2, *label3, *label4, *label5;
        label0 = new UpLabel(patencours->id(), "", TableAMettreAJour);
        label1 = new UpLabel(patencours->id(), "", TableAMettreAJour);
        label2 = new UpLabel(patencours->id(), "", TableAMettreAJour);
        label3 = new UpLabel(patencours->id(), "", TableAMettreAJour);
        label4 = new UpLabel(patencours->id(), "", TableAMettreAJour);
        label5 = new UpLabel(patencours->id(), "", TableAMettreAJour);

        label0->setRow(i);
        label1->setRow(i);
        label2->setRow(i);
        label3->setRow(i);
        label4->setRow(i);
        label5->setRow(i);

        QTableWidgetItem *pItem = new QTableWidgetItem;

        label0->setContextMenuPolicy(Qt::CustomContextMenu);
        label1->setContextMenuPolicy(Qt::CustomContextMenu);
        label2->setContextMenuPolicy(Qt::CustomContextMenu);
        label3->setContextMenuPolicy(Qt::CustomContextMenu);
        label4->setContextMenuPolicy(Qt::CustomContextMenu);
        label5->setContextMenuPolicy(Qt::CustomContextMenu);

        NomPrenom = pat->nom().toUpper() + " " + pat->prenom();
        zw = actapayer->heure().toString("HH:mm");
        int idparent = actapayer->idParent();
        int idsuperviseur = actapayer->idUserSuperviseur();
        label0->setText(" " + zw);                                                              // Heure acte
        label1->setText(" " + NomPrenom);                                                       // Nom + Prénom
        QString Soignant = superviseurlogin;
        if (patencours->idusersuperviseur() != idparent)
            Soignant +=  " / " +  (Datas::I()->users->getById(idparent) != Q_NULLPTR? Datas::I()->users->getById(idparent)->login() : "null");
        label2->setText(" " + superviseurlogin);       // Soignant
        label3->setText(" " + actapayer->cotation());                                           // Cotation
        label4->setText(QLocale().toString(actapayer->montant(),'f',2) + " ");                  // Montant
        label5->setText(QString::number(idsuperviseur));                                             // Parent
        QString typpaiement = "";
        if (actapayer->montant() == 0.0)
            typpaiement = "Gratuit";
        label4->setAlignment(Qt::AlignRight);
        if (patencours->messageretour()!="" || patencours->message()!="")
        {
            QString color = "color:green";
            label0->setStyleSheet(color);
            label1->setStyleSheet(color);
            label2->setStyleSheet(color);
            label3->setStyleSheet(color);
            label4->setStyleSheet(color);
            label5->setStyleSheet(color);
        }
        if (!listidsuperviseurs.contains(idsuperviseur))
        {
            listidsuperviseurs           << idsuperviseur;
            oitem0                  = new QStandardItem(QString::number(idsuperviseur));
            User *usr = Datas::I()->users->getById(idsuperviseur);
            oitem1                  = new QStandardItem(usr? usr->login() : "");
            QList<QStandardItem*>   listitems;
            listitems               << oitem0 << oitem1;
            m_listesuperviseursaccueilmodel     ->appendRow(listitems);
        }

        connect (label0,        &QWidget::customContextMenuRequested,       this,   [=] {MenuContextuelAccueil(label0);});
        connect (label1,        &QWidget::customContextMenuRequested,       this,   [=] {MenuContextuelAccueil(label1);});
        connect (label2,        &QWidget::customContextMenuRequested,       this,   [=] {MenuContextuelAccueil(label2);});
        connect (label3,        &QWidget::customContextMenuRequested,       this,   [=] {MenuContextuelAccueil(label3);});
        connect (label4,        &QWidget::customContextMenuRequested,       this,   [=] {MenuContextuelAccueil(label4);});
        connect (label5,        &QWidget::customContextMenuRequested,       this,   [=] {MenuContextuelAccueil(label5);});
        connect (label0,        &UpLabel::enter,            this,               [=] {AfficheMotif(label0);});
        connect (label1,        &UpLabel::enter,            this,               [=] {AfficheMotif(label1);});
        connect (label2,        &UpLabel::enter,            this,               [=] {AfficheMotif(label2);});
        connect (label3,        &UpLabel::enter,            this,               [=] {AfficheMotif(label3);});
        connect (label4,        &UpLabel::enter,            this,               [=] {AfficheMotif(label4);});
        connect (label5,        &UpLabel::enter,            this,               [=] {AfficheMotif(label5);});
        TableAMettreAJour   ->setCellWidget(i,0,label0);
        TableAMettreAJour   ->setCellWidget(i,1,label1);
        TableAMettreAJour   ->setCellWidget(i,2,label2);
        TableAMettreAJour   ->setCellWidget(i,3,label3);
        TableAMettreAJour   ->setCellWidget(i,4,label4);
        pItem               ->setText(QString::number(patencours->idacteapayer()));
        TableAMettreAJour   ->setItem(i,5,pItem);
        TableAMettreAJour   ->setCellWidget(i,6,label5);
        TableAMettreAJour   ->setRowHeight(i,int(fm.height()*1.1));

        ++ i;
    }
    while (wdg_accueilTab->count()>0)
        wdg_accueilTab->removeTab(0);
    if (m_listesuperviseursaccueilmodel->rowCount() == 0)
        wdg_accueilTab->setVisible(false);
    else
    {
        if (m_listesuperviseursaccueilmodel->rowCount() > 1)
            m_listesuperviseursaccueilmodel->sort(0);
        wdg_accueilTab->setVisible(true);
        for (int i=0; i<m_listesuperviseursaccueilmodel->rowCount(); i++)
        {
            wdg_accueilTab  ->insertTab(i,m_listesuperviseursaccueilmodel->item(i,1)->text());
            wdg_accueilTab  ->setTabData(i, m_listesuperviseursaccueilmodel->item(i,0)->text());
        }
        if (ui->AccueilupTableWidget->selectedRanges().size()>0)
        {
            bool a = false;
            for (int i=0; i<wdg_accueilTab->count(); i++)
                if (wdg_accueilTab->tabData(i).toInt() == currentuser()->idparent())
                {
                    wdg_accueilTab->setCurrentIndex(i);
                    a = true;
                }
            if (!a)
                wdg_accueilTab->setCurrentIndex(0);
        }
        FiltreAccueil(wdg_accueilTab->currentIndex());
    }
/*! ---------------------------------------------------------------------------------------------------------------------------------------------------------------
    4 -PATIENTS VUS CE JOUR ----------------------------------------------------------------------------------------------------------
    --------------------------------------------------------------------------------------------------------------------------------------------------------------- */
    if(ui->PatientsVusWidget->isVisible())
        MAJPatientsVus();
}


/*-----------------------------------------------------------------------------------------------------------------
-- Afficher un résumé du statut -----------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void Rufus::ResumeStatut()
{
    // le statut utilisateur
    m_resumeStatut = proc->SessionStatus() + "\n\n";

    // les socket
    if (m_utiliseTCP)
    {
        for (auto it = m_listesockets.begin(); it != m_listesockets.end(); ++it)
        {
            PosteConnecte *post = Datas::I()->postesconnectes->getByStringId(*it);
            if (it == m_listesockets.begin())
            {
                // le 1er item de gListSockets est le serveur
                m_resumeStatut += tr("ServeurTCP") + "\n\t";
                if (post != Q_NULLPTR) //!>* chaque item contient adresseIP, adresseMac, LoaclhostName(), idUser puis  TCPMSG_ListeSockets
                {
                    m_resumeStatut += post->ipadress() + " - "
                            + post->macadress() + " - "
                            + post->nomposte() + " --- "
                            + (Datas::I()->users->getById(post->iduser()) != Q_NULLPTR? Datas::I()->users->getById(post->iduser())->login() : "") ;
                }
                else
                    m_resumeStatut += tr("inconnu");
                m_resumeStatut += "\n" + tr("Postes connectés") + "\n";
            }
            else
            {
                if (post != Q_NULLPTR)
                {
                    m_resumeStatut += "\t" + post->ipadress() + " - "
                            + post->macadress() + " - "
                            + post->nomposte() + " --- "
                            + (Datas::I()->users->getById(post->iduser()) != Q_NULLPTR? Datas::I()->users->getById(post->iduser())->login() : "") + "\n";
                }
                else
                    m_resumeStatut += "\t" + tr("inconnu");
            }
        }
    }
    else
    {
        m_resumeStatut += "\n" + tr("Postes connectés") + "\n";
        foreach (PosteConnecte *post, *Datas::I()->postesconnectes->postesconnectes())
        {
            if(!post->isdistant())
                m_resumeStatut += "\t" + post->nomposte() + " - "
                        + post->ipadress() + " - "
                        + post->macadress() + " --- "
                        + (Datas::I()->users->getById(post->iduser()) != Q_NULLPTR? Datas::I()->users->getById(post->iduser())->login() : "") + "\n";
        }
    }
    foreach (PosteConnecte *post, *Datas::I()->postesconnectes->postesconnectes())
    {
        if(post->isdistant())
            m_resumeStatut += "\t" + (Datas::I()->sites->getById(post->idlieu())? Datas::I()->sites->getById(post->idlieu())->nom() : "") + " ---- "
                    + (Datas::I()->users->getById(post->iduser()) != Q_NULLPTR? Datas::I()->users->getById(post->iduser())->login() : "" )+ "\n";
    }

    // l'importateur des documents
    m_resumeStatut += "\n" + tr("Poste importateur des documents") + "\t";
    QString A = proc->PosteImportDocs();
    if (A == "")
        m_resumeStatut += tr("Pas de poste paramétré");
    else
    {
        QString B;
        if (A.contains(" - " NOM_ADMINISTRATEUR))
            B = tr("Administrateur");
        else
            B = (A.contains(" - prioritaire")? tr("prioritaire") : tr("non prioritaire"));
        A.remove(" - prioritaire");
        A.remove(" - " NOM_ADMINISTRATEUR);
        m_resumeStatut += A + " - " + B;
    }

    // version de Rufus et de la base
    m_resumeStatut += "\n\n" + tr("Version de Rufus ") + "\t\t" + qApp->applicationVersion();
    m_resumeStatut += "\n" + tr("Version de la base ") + "\t\t";
    if (QString::number(m_parametres->versionbase()) == "")
        m_resumeStatut += tr("inconnue");
    else
        m_resumeStatut +=  QString::number(m_parametres->versionbase());
    emit proc->ModifEdit(m_resumeStatut);
}

/*-----------------------------------------------------------------------------------------------------------------
-- Salle d'attente ------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
bool Rufus::RetourSalleDattente(QString Titre)
{
    bool retour         = false;

    dlg_salledattente *Dlg_SalDat = new dlg_salledattente(currentacte(), Titre);
    Dlg_SalDat->setWindowTitle(tr("Consultation incomplète!"));

    if (Titre == tr("Il manque le montant!") || Titre == tr("Il manque la cotation!"))
        Dlg_SalDat->ui->RetourAccueilradioButton->setEnabled(false);
    if (Titre.contains(tr("il manque les informations de paiement")))
        Dlg_SalDat->ui->RetourAccueilradioButton->setEnabled(true);

    if (Dlg_SalDat->exec() == QDialog::Accepted)
        retour = true;
    delete Dlg_SalDat;
    return retour;
}

/*-----------------------------------------------------------------------------------------------------------------
-- Supprimer un acte ----------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void Rufus::SupprimerActe(Acte *act)
{
    UpMessageBox    msgbox;

    // On vérifie que cet acte n'a pas été payé par une opération bancaire déjà enregistrée sur le compte
    // on récupère les lignes de paiement
    QString Messg ="";

    QList<LignePaiement*> listlignespaiement;
    for (auto it = m_lignespaiements->lignespaiements()->constBegin(); it != m_lignespaiements->lignespaiements()->constEnd(); ++it)
    {
        LignePaiement* lign = const_cast<LignePaiement*>(it.value());
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
        QString req = "SELECT " CP_MODEPAIEMENT_LIGNRECETTES ", " CP_NOMPAYEUR_LIGNRECETTES ", " CP_IDREMISECHQ_LIGNRECETTES " FROM " TBL_RECETTES " WHERE " CP_ID_LIGNRECETTES " in (" + critere + ")";
        QList<QVariantList> pmtlist = db->StandardSelectSQL(req,m_ok);
        if(m_ok && pmtlist.size()>0)
            for (int j=0; j<pmtlist.size(); j++)
            {
                if (pmtlist.at(j).at(0).toString() == VRMT)                                             Messg = tr("Je crains de ne pas pouvoir supprimer cet acte\nIl y a des versements enregistrés.");
                if (pmtlist.at(j).at(0).toString() == VRMT && pmtlist.at(j).at(1).toString() == "CB")   Messg = tr("Je crains de ne pas pouvoir supprimer cet acte\nIl y a des paiements par carte de crédit enregistrés.");
                if (pmtlist.at(j).at(0).toString() == CHQ && pmtlist.at(j).at(2).toInt() > 0)           Messg = tr("Je crains de ne pas pouvoir supprimer cet acte\nIl y a des paiements par chèque enregistrés.");
                if (pmtlist.at(j).at(0).toString() == CHQ && pmtlist.at(j).at(2).toInt() == 0)          Messg = tr("Je crains de ne pas pouvoir supprimer cet acte\nIl y a des paiements par chèque enregistrés."
                                                                                                                "\nCe ou ces chèques ne sont pas encaissés."
                                                                                                                "\nVous devez modifier l'écriture correspondante pour pouvoir supprimer l'acte.");
                if (Messg != "")
                    j = pmtlist.size();
            }
    }

    if (Messg != "")
    {
        UpMessageBox::Watch(this, "Euuhh... " + currentuser()->login(), Messg);
        return;
    }

    // On demande confirmation de la suppression de l'acte
    msgbox.setText("Euuhh... " + currentuser()->login());
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
    for (auto it = Datas::I()->refractions->refractions()->cbegin(); it != Datas::I()->refractions->refractions()->cend();)
    {
        Refraction* ref = const_cast<Refraction*>(it.value());
        if (ref)
        {
            if (ref->idacte() == act->id())
            {
                DataBase::I()->SupprRecordFromTable(ref->id(), CP_ID_REFRACTIONS, TBL_REFRACTIONS);
                delete ref;
                it = Datas::I()->refractions->refractions()->erase(it);
            }
            else
                ++ it;
        }
        else
            it = Datas::I()->refractions->refractions()->erase(it);
    }

    // on supprime les éventuels bilans orthoptiques liés à cette consultation -----------------------------------------------------------
    QString req = "DELETE FROM " TBL_BILANORTHO " WHERE idBilanOrtho  = " + QString::number(act->id());
    db->StandardSQL(req);

    // on supprime les éventuelles interventions du programme opératoire liées à cette consultation -----------------------------------------------------------
    req = "update " TBL_LIGNESPRGOPERATOIRES " set " CP_IDACTE_LIGNPRGOPERATOIRE " = null where " CP_IDACTE_LIGNPRGOPERATOIRE " = "+ QString::number(act->id());
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
            req = "SELECT " CP_MONTANT_LIGNRECETTES " FROM " TBL_RECETTES " WHERE " CP_ID_LIGNRECETTES " = " + QString::number(listlignespaiement.at(j)->idrecette());
            QList<QVariantList> mntlist = db->StandardSelectSQL(req,m_ok);
            for (int i=0; i<mntlist.size(); ++i)
            {
                QString req = "delete from " TBL_RECETTES " where " CP_ID_LIGNRECETTES " = " + QString::number(listlignespaiement.at(j)->idrecette());
                if (mntlist.at(i).at(0).toDouble() > listlignespaiement.at(j)->paye())
                    req = "update " TBL_RECETTES " set " CP_MONTANT_LIGNRECETTES " = " + QString::number(mntlist.at(i).at(0).toDouble() - listlignespaiement.at(j)->paye()) +
                          " where " CP_ID_LIGNRECETTES " = " + QString::number(listlignespaiement.at(j)->idrecette());
                db->StandardSQL(req);
            }
        }

        // On actualise la table des lignes de paiement et la table des Types de paiement
        m_lignespaiements->SupprimeActeLignesPaiements(act);
        db->SupprRecordFromTable(act->id(), CP_IDACTE_TYPEPAIEMENTACTES, TBL_TYPEPAIEMENTACTES);
    }

    // on supprime l'acte -------------------------------------------------------------------------------------------------
    m_listeactes->SupprimeActe(act);
    if (m_listeactes->actes()->size() == 0)
    {
        ui->Acteframe->setVisible(false);
        ui->CreerActepushButton_2->setVisible(true);
        ui->CreerBOpushButton_2->setVisible(true);
        ui->idActelineEdit->clear();
        Datas::I()->actes->setcurrentacte(Q_NULLPTR);
    }
    else
        AfficheActe(nouvact);                           //! depuis SupprimerActe(Acte *act)

    // On met à jour l'affichage éventuel de dlg_actesprecedents
    QList<dlg_actesprecedents *> listactesprecs = findChildren<dlg_actesprecedents *>();
    for (int i = 0; i<listactesprecs.size();i++)
        if (listactesprecs.at(i)->idcurrentpatient() != currentpatient()->id())
        {
            listactesprecs.at(i)->close();
            delete listactesprecs.at(i);
        }
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
    if (pat == Q_NULLPTR)
        return;
    // On ne peut supprimer un dossier que s'il n'y a pas eu d'actes avec des tiers payants, des chèques ou des cartes de crédit enregistrés pour ce dossier.
    UpMessageBox     msgbox, msgbox2;

    //1. On recherche les actes de ce dossier qui seraient en tiers payant et qui auraient déjà reçu des versements auquel cas, on ne peut pas supprimer les actes ni le dossier
    QString Messg = "";
    for (auto it = m_lignespaiements->lignespaiements()->constBegin(); it != m_lignespaiements->lignespaiements()->constEnd(); ++it)
    {
        LignePaiement* lign = const_cast<LignePaiement*>(it.value());
        // on vérifie pour chaque ligne s'il s'agit d'un virement ou d'une carte bleue ou d'un chèque enregistré
        QString requete = "SELECT " CP_MODEPAIEMENT_LIGNRECETTES ", " CP_NOMPAYEUR_LIGNRECETTES ", " CP_IDREMISECHQ_LIGNRECETTES " FROM " TBL_RECETTES " WHERE " CP_ID_LIGNRECETTES " = " + QString::number(lign->idrecette());
        QVariantList pmydata = db->getFirstRecordFromStandardSelectSQL(requete,m_ok);
        if (!m_ok)
            return;
        if (pmydata.at(0).toString() == VRMT)
            Messg = tr("Je crains de ne pas pouvoir supprimer ce dossier\nIl y a des versements enregistrés.");
        if (pmydata.at(0).toString() == VRMT && pmydata.at(1).toString() == "CB")
            Messg = tr("Je crains de ne pas pouvoir supprimer ce dossier\nIl y a des paiements par carte de crédit enregistrés.");
        if (pmydata.at(0).toString() == CHQ && pmydata.at(2).toInt() > 0)
            Messg = tr("Je crains de ne pas pouvoir supprimer cet acte\nIl y a des paiements par chèque enregistrés.");
        if (pmydata.at(0).toString() == CHQ && pmydata.at(2).toInt() == 0)
            Messg = tr("Je crains de ne pas pouvoir supprimer ce dossier\nIl y a des paiements par chèque enregistrés."
                    "\nCe ou ces chèques ne sont pas encaissés."
                    "\nVous devez modifier l'écriture correspondante pour pouvoir supprimer l'acte.");
        if (Messg != "") break;
    }
    if (Messg != "")
    {
        UpMessageBox::Watch(this, "Euuhh... " + currentuser()->login(), Messg);
        return;
    }

    //2. On commence par demander la confirmation de la suppression du dossier
    msgbox.setText("Euuhh... " + currentuser()->login());
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
            QString requete = "SELECT " CP_MONTANT_LIGNRECETTES " FROM " TBL_RECETTES " WHERE " CP_ID_LIGNRECETTES " = " + QString::number(idrecetteACorriger);
            QList<QVariantList> reclist = db->StandardSelectSQL(requete,m_ok);
            if (!m_ok)
                return;
            else if (m_ok && reclist.size()>0)
            {
                for (int k=0; k<reclist.size(); k++)
                {
                    QString req = "delete from " TBL_RECETTES " where " CP_ID_LIGNRECETTES " = " + QString::number(idrecetteACorriger);
                    if (reclist.at(k).at(0).toDouble() > m_lignespaiements->lignespaiements()->values().at(j)->paye())
                        req = "update " TBL_RECETTES " set " CP_MONTANT_LIGNRECETTES " = " + QString::number(reclist.at(k).at(0).toDouble() - m_lignespaiements->lignespaiements()->values().at(j)->paye()) +
                                " where " CP_ID_LIGNRECETTES " = " + QString::number(idrecetteACorriger);
                    db->StandardSQL(req);
                }
            }
        }
        //4, On actualise la table des lignes de paiement el la table des Type de paieement
        QList<Acte*> listactes;
        for (auto it = m_listeactes->actes()->constBegin(); it != m_listeactes->actes()->constEnd(); ++it)
        {
            Acte* act = const_cast<Acte*>(it.value());
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
        QString requete = "DELETE FROM " TBL_TYPEPAIEMENTACTES " WHERE " CP_IDACTE_TYPEPAIEMENTACTES " in (" + critere + ")";
        db->StandardSQL(requete);
    }

    // on supprime les éventuelles interventions du programme opératoire liées à ce dossier -----------------------------------------------------------
    QString req = "delete from " TBL_LIGNESPRGOPERATOIRES " where " CP_IDPATIENT_LIGNPRGOPERATOIRE " = " + QString::number(pat->id());
    db->StandardSQL(req);

    //!. Fermeture de l'onglet dossier
    ui->tabWidget->removeTab(ui->tabWidget->indexOf(ui->tabDossier));

    //!. Suppression du dossier, reconstruction de la liste et du treeView
    Datas::I()->patientsencours->SupprimePatientEnCours(Datas::I()->patientsencours->getById(pat->id()));
    Datas::I()->patients->SupprimePatient(pat);
    ItemsList::clearAll(m_listeactes->actes());

    Datas::I()->actes->setcurrentacte(Q_NULLPTR);
    Datas::I()->patients->setcurrentpatient(Q_NULLPTR);
    ItemsList::clearAll(m_lignespaiements->lignespaiements());
    FiltreTable(ui->CreerNomlineEdit->text(), ui->CreerPrenomlineEdit->text());
    Flags::I()->MAJFlagSalleDAttente();
    ModeSelectDepuisListe();

    //!. Fermeture de la fiche dlg_actesprecedents
    FermeDlgActesPrecedentsEtDocsExternes();
}

/*-----------------------------------------------------------------------------------------------------------------
-- Pachymetrie -----------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void Rufus::Pachymetrie()
{
    if (ui->tabWidget->currentIndex() != ui->tabWidget->indexOf(ui->tabDossier)) return;

    dlg_autresmesures *Dlg_AutresMes= new dlg_autresmesures(dlg_autresmesures::PACHY, this);
    Dlg_AutresMes->setWindowTitle(tr("Pachymétrie - ") + currentpatient()->nom() + " " + currentpatient()->prenom());

    if (Dlg_AutresMes->exec() == QDialog::Accepted)
    {
        proc->InsertMesure(GenericProtocol::MesurePachy);
    }
    delete Dlg_AutresMes;
}

void Rufus::AffichePachymetrie()
{
    QString pachy = proc->HtmlPachy();
    if (pachy != "")
    {
        QString ARajouterEnText = HTML_RETOURLIGNE + pachy;
        ItemsList::update(currentacte(), CP_TEXTE_ACTES, ui->ActeTextetextEdit->appendHtml(ARajouterEnText));
        QString Methode = Pachymetrie::ConvertToReadableMesure(Datas::I()->mesurepachy);
        QString resumetxt = ui->ResumetextEdit->toHtml();
        QString const dd    = "<a name=\"" HTMLANCHOR_PACHYDEBUT "[0-9]{0,11}\"></a>";
        QString const fd    = "<a name=\"" HTMLANCHOR_PACHYFIN "\"></a>";
        if (resumetxt.contains(QRegularExpression(dd + ".*" + fd)))
        {
            int n = pachy.indexOf(QRegularExpression(dd));
            pachy = pachy.mid(n);
            n = pachy.indexOf(fd) + fd.size();
            pachy = pachy.left(n);
            pachy.replace(Methode, Methode + " - " + QDate::currentDate().toString("dd.MM.yy"));
            resumetxt.replace(QRegularExpression(dd + ".*" + fd), pachy);
            ItemsList::update(currentpatient(), CP_RESUME_RMP, resumetxt);
            ui->ResumetextEdit->setText(resumetxt);
        }
        else
            ItemsList::update(currentpatient(), CP_RESUME_RMP, ui->ResumetextEdit->appendHtml(ARajouterEnText.replace(Methode, Methode + " - " + QDate::currentDate().toString("dd.MM.yy"))));
        ui->ActeTextetextEdit->setFocus();
        ui->ActeTextetextEdit->moveCursor(QTextCursor::End);
    }
}

/*-----------------------------------------------------------------------------------------------------------------
-- Tonometrie -----------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void Rufus::Tonometrie()
{
    if (ui->tabWidget->currentIndex() != ui->tabWidget->indexOf(ui->tabDossier)) return;

    dlg_autresmesures *Dlg_AutresMes = new dlg_autresmesures(dlg_autresmesures::TONO, this);
    Dlg_AutresMes->setWindowTitle(tr("Tonométrie - ") + currentpatient()->nom() + " " + currentpatient()->prenom());

    if (Dlg_AutresMes->exec() == QDialog::Accepted)
    {
        proc->InsertMesure(GenericProtocol::MesureTono);
        ui->ActeTextetextEdit->setFocus();
        ui->ActeTextetextEdit->moveCursor(QTextCursor::End);
    }
    delete Dlg_AutresMes;
}

/*-----------------------------------------------------------------------------------------------------------------
-- Trouver un dossier d'après la DDN ------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void Rufus::FiltreTableparDDN()
{
    Datas::I()->patients->initListeByDDN(ui->CreerDDNdateEdit->date());
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
        Utils::playAlarm();
        UpMessageBox::Watch(this, tr("Saisie de montant refusée !"),
                             tr("Le montant que vous souhaitez enregistrer") + "\n\t" + NouveauMontant + " " + tr("€") +"\n"
                             + tr("est inférieur à la somme des paiements") + "\n" + tr("déjà enregistrés pour cet acte") + "\n\t" + ui->PayelineEdit->text() + tr("€"));
        return false;
    }
    else
        return true;
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
        req = "SELECT " CP_IDACTE_TYPEPAIEMENTACTES ", " CP_TYPEPAIEMENT_TYPEPAIEMENTACTES " FROM " TBL_TYPEPAIEMENTACTES " WHERE " CP_TYPEPAIEMENT_TYPEPAIEMENTACTES " = 'G' AND " CP_IDACTE_TYPEPAIEMENTACTES " = " + QString::number(currentacte()->id());
        QVariantList idactdata = db->getFirstRecordFromStandardSelectSQL(req,m_ok, "ValideActeMontantLineEdit");
        if (!m_ok)
            return false;
        if (idactdata.size() > 0)
        {
            Utils::playAlarm();
            UpMessageBox msgbox(this);
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
                db->SupprRecordFromTable(currentacte()->id(), CP_IDACTE_TYPEPAIEMENTACTES, TBL_TYPEPAIEMENTACTES);
               AfficheActeCompta(currentacte());
            }
        }
        else
            ui->ActeMontantlineEdit->setText(NouveauMontant);
    }
    //on modifie la table Actes avec le nouveau montant
    ItemsList::update(currentacte(), CP_COTATION_ACTES, ui->ActeCotationcomboBox->currentText());
    ItemsList::update(currentacte(), CP_MONTANT_ACTES, QLocale().toDouble(NouveauMontant));

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

void Rufus::NouvelleMesure(GenericProtocol::TypeMesure TypeMesure) //utilisé pour ouvrir la fiche refraction quand un appareil a transmis une mesure
{
    if (findChildren<dlg_refraction*>().size()>0)
        if (TypeMesure == GenericProtocol::MesureRefracteur
                || TypeMesure == GenericProtocol::MesureFronto
                || TypeMesure == GenericProtocol::MesureAutoref)
            return;
    if (currentpatient() == Q_NULLPTR
            || currentacte() == Q_NULLPTR)
        return;

    switch (TypeMesure) {
    case  GenericProtocol::MesureRefracteur:
        if (!Datas::I()->mesureacuite->isdataclean() && !Datas::I()->mesurefinal->isdataclean())
            ItemsList::update(currentacte(), CP_TEXTE_ACTES, ui->ActeTextetextEdit->appendHtml(proc->HtmlRefracteur()));
        RefractionMesure(dlg_refraction::Auto);
        break;
    case GenericProtocol::MesureAutoref:
        ItemsList::update(currentacte(), CP_TEXTE_ACTES, ui->ActeTextetextEdit->appendHtml(proc->HtmlAutoref()));
        break;
    case GenericProtocol::MesureFronto:
        ItemsList::update(currentacte(), CP_TEXTE_ACTES, ui->ActeTextetextEdit->appendHtml(proc->HtmlFronto()));
        break;
    case GenericProtocol::MesureKerato:
        ItemsList::update(currentacte(), CP_TEXTE_ACTES, ui->ActeTextetextEdit->appendHtml(proc->HtmlKerato()));
        break;
    case GenericProtocol::MesurePachy:
        AffichePachymetrie();
        break;
    case GenericProtocol::MesureTono:
        ItemsList::update(currentacte(), CP_TEXTE_ACTES, ui->ActeTextetextEdit->appendHtml(proc->HtmlTono()));
        break;
    default:
        RefractionMesure(dlg_refraction::Manuel);
    }
    ui->ActeTextetextEdit->setFocus();
    ui->ActeTextetextEdit->moveCursor(QTextCursor::End);
}



// CZ001 début interface avec Pyxvital
/*-----------------------------------------------------------------------------------------------------------------
    Lire la CPS avec Pyxvital : Retour = fichier Particien.par
-----------------------------------------------------------------------------------------------------------------*/
void Rufus::LireLaCPS()
{
    QString req, numPS;
    m_pyxi = new pyxinterf(this);
    QString nomFicPraticienPar = m_pyxi->Lecture_CPS();
    delete m_pyxi;

    // Récup des infos du médecin dans le fichier ../Pyxvital/Interf/Praticien.par
    QSettings settingPraticienPar (nomFicPraticienPar, QSettings::IniFormat);
    //settingPraticienPar.setIniCodec ("ISO 8859-1");

    numPS    = settingPraticienPar.value("PS/Numéro").toString() ; // 8 chiffres sans la clé
    if (numPS.length() == 0)
        { //  CPS non lue...
        return;
        }
    // recherche utilisateur avec ce n°ADELI
    req =   "SELECT " CP_ID_USR " FROM " TBL_UTILISATEURS " WHERE " CP_NUMPS_USR " = '" + numPS + "'" ;
    QVariantList idusrdata = db->getFirstRecordFromStandardSelectSQL(req,m_ok, tr("Impossible d'ouvrir la table Utilisateurs"));
    if (!m_ok)
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
        currentuser()->setid(idusrdata.at(0).toInt());
        setWindowTitle("RufusQt6 - " + currentuser()->login() + " - " + currentuser()->fonction());
        }
}
/*-----------------------------------------------------------------------------------------------------------------
    Lire la CV avec Pyxvital : Retour = fichier Patient.par
-----------------------------------------------------------------------------------------------------------------*/
void Rufus::LireLaCV()
{
    QString nomPat, prenomPat, dateNaissPat;
    QString zdat;

    m_pyxi = new pyxinterf(this);
    QString nomFicPatientPar = m_pyxi->Lecture_CV();
    delete m_pyxi;
    if (nomFicPatientPar.length() ==0)
        { // pas de CV lue...
        return;
        }
    // Récup des infos du bénéficiaire dans le fichier ../Pyxvital/Interf/Patient.par
    QSettings settingPatientPar (nomFicPatientPar, QSettings::IniFormat);
    //settingPatientPar.setIniCodec ("ISO 8859-1");

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
    if (Datas::I()->patients->patientstable()->size() == 0)       // aucun patient trouvé
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
    m_pyxi = new pyxinterf(this);
    QString nomFicFacturePar = m_pyxi->Saisie_FSE();
    delete m_pyxi;
    if (nomFicFacturePar.length() ==0)
        { // pas de facture saisie ...
        return;
        }
    // Récup des infos de la facture dans le fichier ../Pyxvital/Interf/Facture.par pour alimenter la comptabilité
    QSettings settingFacturePar (nomFicFacturePar, QSettings::IniFormat);
    //settingFacturePar.setIniCodec ("ISO 8859-1");

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
    envoieTCPMessage(TCPMSG_TestConnexion);
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
        //Message::I()->TrayMessage(QStringList() << tr("Mise à jour de la liste des correspondants"), 3000);
    }
    else if (msg.contains(TCPMSG_MsgBAL))
    {
        /*! message -> nombredemessages + TCPMSG_MsgBAL) */
        msg.remove(TCPMSG_MsgBAL);
        m_totalNvxMessages = msg.toInt();
        msg = "";
        if (m_totalNvxMessages>1)
               msg = tr("Vous avez ") + QString::number(m_totalNvxMessages) + tr(" nouveaux messages");
           else if (m_totalNvxMessages>0)
               msg = tr("Vous avez 1 nouveau message");
           if (msg!="")
           {
               Utils::playAlarm();
               ict_messageIcon->showMessage(tr("Messages"), msg, Icons::icPostit(), 10000);
               if (dlg_msgBAL != Q_NULLPTR)
                   if (dlg_msgBAL->isVisible())
                       AfficheBAL();
           }
    }
    else if (msg.contains(TCPMSG_MAJDocsExternes))
    {
        /*! message -> idpatient + TCPMSG_MAJDocsExternes) */
        //qDebug() << msg;
        msg.remove(TCPMSG_MAJDocsExternes);
        if (currentuser()->isSoignant() && currentpatient() != Q_NULLPTR)
            if (currentpatient()->id() == msg.toInt())
                MAJDocsExternes();              // depuis le tcpsocket
    }
    else if (msg.contains(TCPMSG_MAJPatient))
    {
        /*! message -> idpatient + TCPMSG_MAJPatient) */
        msg.remove(TCPMSG_MAJPatient);
        auto it = Datas::I()->patients->patientstable()->find(msg.toInt());
        if (it != Datas::I()->patients->patientstable()->end())
        {
            Patient* pat = const_cast<Patient*>(it.value());
            Datas::I()->patients->loadAll(pat, Item::Update);
            int row = m_listepatientsmodel->findItems(msg).at(0)->row();
            m_listepatientsmodel->item(row,0)->setText(QString::number(pat->id()));                                   // id                           -> utilisé pour le drop event
            m_listepatientsmodel->item(row,1)->setText(pat->nom().toUpper() + " " + pat->prenom());                   // Nom + Prénom
            m_listepatientsmodel->item(row,2)->setText(pat->datedenaissance().toString(tr("dd-MM-yyyy")));            // date de naissance
            m_listepatientsmodel->item(row,3)->setText(pat->datedenaissance().toString(tr("yyyyMMdd")));              // date de naissance inversée   -> utilisé pour le tri
            m_listepatientsmodel->item(row,4)->setText(pat->nom());                                                   // Nom                          -> utilisé pour le tri
            m_listepatientsmodel->item(row,5)->setText(pat->prenom());                                                // Prénom                       -> utilisé pour le tri
        }
        if (Datas::I()->patientsencours->patientsencours()->find(msg.toInt()) != Datas::I()->patientsencours->patientsencours()->end())
            Remplir_SalDat();
        it = Datas::I()->patients->patients()->find(msg.toInt());
        if (it != Datas::I()->patients->patients()->end())
        {
            Patient* pat = const_cast<Patient*>(it.value());
            Datas::I()->patients->loadAll(pat, Item::Update);
        }
        if (currentpatient() != Q_NULLPTR)
            if (currentpatient()->id() == msg.toInt())
            {
                DataBase::I()->loadPatientById(msg.toInt(), currentpatient(), Item::LoadDetails);
                ui->IdentPatienttextEdit->setHtml(CalcHtmlIdentificationPatient(currentpatient()));
            }
    }
    else if (msg.contains(TCPMSG_ListeStringIdPostesConnectes))
    {
        bool afficheresume = msg.contains(TCPMSG_AskListeStringId);
        if (afficheresume)
            msg.remove(TCPMSG_AskListeStringId);
        msg.remove(TCPMSG_ListeStringIdPostesConnectes);
        m_listesockets.clear();
        m_listesockets = msg.split(TCPMSG_Separator);
        //qDebug() << "liste des clients connectés rufus.cpp - " + QTime::currentTime().toString("hh-mm-ss");
        ResumeStatut();
        if (afficheresume)
            proc->Edit(m_resumeStatut, tr("Information statut"), false, true );
        Remplir_SalDat();
    }
    else if (msg.contains(TCPMSG_DeconnexionPoste))
    {
        msg.remove(TCPMSG_DeconnexionPoste);
        m_listesockets.removeAll(msg);
        ItemsList::remove(Datas::I()->postesconnectes->postesconnectes(), Datas::I()->postesconnectes->getByStringId(msg));
        ResumeStatut();
        Remplir_SalDat();
    }
}

void Rufus::envoieTCPMessage(QString msg)
{
    if (!m_utiliseTCP)
        return;
    //qDebug() << msg + " - void Rufus::envoieMessage(QString msg)";
    TcPConnect->envoieMessage(msg);
}

void Rufus::envoieTCPMessageA(QList<int> listidusr)
{
    Flags::I()->MAJflagMessages();
    if (!m_utiliseTCP)
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

