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

#include "dlg_param.h"
#include "icons.h"
#include "ui_dlg_param.h"
#include "utils.h"

dlg_param::dlg_param(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::dlg_param)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    proc            = Procedures::I();
    db              = DataBase::I();
    m_parametres    = db->parametres();

    m_modifposte     = false;
    m_currentuser   = Datas::I()->users->userconnected();

    m_nouveauMDP        = "nouv";
    m_ancienMDP         = "anc";
    m_confirmeMDP     = "confirm";

    m_MDPadminverifie= false;
    m_MDPuserverifie = false;

    QStringList ports;
    ports << "3306" << "3307";
    ui->SQLPortDistantcomboBox  ->addItems(ports);
    ui->SQLPortLocalcomboBox    ->addItems(ports);
    ui->SQLPortPostecomboBox    ->addItems(ports);
    m_donneesusermodifiees    = false;
    m_cotationsmodifiees     = false;

    wdg_appareilswdgbuttonframe = new WidgetButtonFrame(ui->AppareilsConnectesupTableWidget);
    wdg_appareilswdgbuttonframe->AddButtons(WidgetButtonFrame::PlusButton | WidgetButtonFrame::MoinsButton);
    connect(wdg_appareilswdgbuttonframe, SIGNAL(choix(int)), this, SLOT(Slot_ChoixButtonFrame(int)));

    wdg_HNcotationswdgbuttonframe = new WidgetButtonFrame(ui->HorsNomenclatureupTableWidget);
    wdg_HNcotationswdgbuttonframe->AddButtons(WidgetButtonFrame::PlusButton | WidgetButtonFrame::ModifButton | WidgetButtonFrame::MoinsButton);
    connect(wdg_HNcotationswdgbuttonframe,         SIGNAL(choix(int)), this, SLOT(Slot_ChoixButtonFrame(int)));

    wdg_assocCCAMcotationswdgbuttonframe = new WidgetButtonFrame(ui->AssocCCAMupTableWidget);
    wdg_assocCCAMcotationswdgbuttonframe->AddButtons(WidgetButtonFrame::PlusButton | WidgetButtonFrame::ModifButton | WidgetButtonFrame::MoinsButton);
    connect(wdg_assocCCAMcotationswdgbuttonframe,  SIGNAL(choix(int)), this, SLOT(Slot_ChoixButtonFrame(int)));

    wdg_assocCCAMcotationswdgbuttonframe->layButtons()->insertWidget(0, ui->ChercheCCAMlabel);
    wdg_assocCCAMcotationswdgbuttonframe->layButtons()->insertWidget(0, ui->ChercheCCAMupLineEdit);

    QHBoxLayout *EnteteCCAMlay  = new QHBoxLayout();
    QHBoxLayout *Margelay       = new QHBoxLayout();
    QHBoxLayout *Marge2lay      = new QHBoxLayout();
    QHBoxLayout *Marge0lay      = new QHBoxLayout();
    QVBoxLayout *CCAMlay        = new QVBoxLayout();
    QVBoxLayout *AssocCCAMlay   = new QVBoxLayout();
    QVBoxLayout *HorsCCAMlay    = new QVBoxLayout();
    QVBoxLayout *Cotationslay   = new QVBoxLayout();
    int marge   = 10;
    Cotationslay    ->setSpacing(marge);
    marge = 0;
    Cotationslay    ->setContentsMargins(marge,marge,marge,marge);
    Marge0lay       ->setContentsMargins(marge,marge,marge,marge);
    Marge0lay       ->setSpacing(marge);
    Margelay        ->setContentsMargins(marge,marge,marge,marge);
    Margelay        ->setSpacing(marge);
    Marge2lay       ->setContentsMargins(marge,marge,marge,marge);
    Marge2lay       ->setSpacing(marge);
    CCAMlay         ->setContentsMargins(marge,marge,marge,marge);
    CCAMlay         ->setSpacing(marge);
    EnteteCCAMlay   ->setContentsMargins(marge,marge,marge,marge);
    EnteteCCAMlay   ->setSpacing(marge);

    EnteteCCAMlay   ->addWidget(ui->ActesCCAMlabel);
    EnteteCCAMlay   ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding, QSizePolicy::Expanding));
    EnteteCCAMlay   ->addWidget(ui->OphtaSeulcheckBox);

    CCAMlay         ->addLayout(EnteteCCAMlay);
    CCAMlay         ->addWidget(ui->ActesCCAMupTableWidget);
    CCAMlay         ->setStretch(0,0);
    CCAMlay         ->setStretch(1,15);

    Marge0lay       ->addWidget(ui->ShowCCAMlabel);
    Marge0lay       ->addLayout(CCAMlay);
    Marge0lay       ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding, QSizePolicy::Expanding));

    Cotationslay    ->addLayout(Marge0lay);

    AssocCCAMlay     ->addWidget(ui->AssocCCAMlabel);
    AssocCCAMlay     ->addWidget(wdg_assocCCAMcotationswdgbuttonframe->widgButtonParent());
    AssocCCAMlay     ->setStretch(0,0);
    AssocCCAMlay     ->setStretch(1,15);

    Marge2lay       ->addWidget(ui->Marge2Widget);
    Marge2lay       ->addLayout(AssocCCAMlay);
    Marge2lay       ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding, QSizePolicy::Expanding));
    Marge2lay       ->setStretch(0,1);
    Marge2lay       ->setStretch(1,14);
    Marge2lay       ->setStretch(2,1);

    Cotationslay    ->addLayout(Marge2lay);

    HorsCCAMlay     ->addWidget(ui->HorsNomenclaturelabel);
    HorsCCAMlay     ->addWidget(wdg_HNcotationswdgbuttonframe->widgButtonParent());
    HorsCCAMlay     ->setStretch(0,15);

    Margelay        ->addWidget(ui->MargeWidget);
    Margelay        ->addLayout(HorsCCAMlay);
    Margelay        ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding, QSizePolicy::Expanding));

    Cotationslay    ->addLayout(Margelay);

    Cotationslay    ->setStretch(0,8);      // Marge0lay - les actes en CCAM
    Cotationslay    ->setStretch(1,7);      // Marge2lay - les associations
    Cotationslay    ->setStretch(2,5);      // Margelay - les actes hors nomenclature

    ui->Cotationswidget  ->setLayout(Cotationslay);

    ui->UserParamtab    ->setLayout(ui->UserLayout);
    ui->GeneralParamtab ->setLayout(ui->GeneralLayout);
    ui->PosteParamtab   ->setLayout(ui->PosteLayout);

    ui->ParamtabWidget              ->setTabIcon(ui->ParamtabWidget->indexOf(ui->UserParamtab),Icons::icContact());
    ui->ParamtabWidget              ->setTabIcon(ui->ParamtabWidget->indexOf(ui->PosteParamtab),Icons::icComputer());
    ui->ParamtabWidget              ->setTabIcon(ui->ParamtabWidget->indexOf(ui->GeneralParamtab),Icons::icMarteau());
    ui->ParamtabWidget              ->setIconSize(QSize(30,30));
    ui->EffacePrgSauvupPushButton   ->setIcon(Icons::icErase());
    ui->EffacePrgSauvupPushButton   ->setIconSize(QSize(35,35));
    ui->RestaurBaseupPushButton     ->setIcon(Icons::icCopy());
    ui->ImmediatBackupupPushButton  ->setIcon(Icons::icBackup());
    ui->ReinitBaseupPushButton      ->setIcon(Icons::icReinit());
    ui->ChoixFontupPushButton       ->setIconSize(QSize(35,35));
    ui->ChercheCCAMlabel            ->setPixmap(Icons::pxLoupe().scaled(20,20)); //WARNING : icon scaled : pxLoupe 20,20
    ui->ShowCCAMlabel               ->setPixmap(QPixmap());
    ui->StatutComptaupTextEdit      ->setAttribute( Qt::WA_NoSystemBackground, true );
    ui->StatutComptaupTextEdit      ->setReadOnly(true);
    ui->StatutComptaupTextEdit      ->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->StatutComptaupTextEdit      ->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QFont font = qApp->font();
    font.setBold(true);
    ui->PrenomuplineEdit->setFont(font);
    ui->NomuplineEdit->setFont(font);
    ui->TitreuplineEdit->setFont(font);


    ui->LockParamUserupLabel                ->setPixmap(Icons::pxVerrouiller());
    ui->LockParamPosteupLabel               ->setPixmap(Icons::pxVerrouiller());
    ui->LockParamGeneralupLabel             ->setPixmap(Icons::pxVerrouiller());
    ui->Frontolabel                         ->setPixmap(Icons::pxLensMeter().scaled(70,70)); //WARNING : icon scaled : pxLensMeter 70,70
    ui->Autoreflabel                        ->setPixmap(Icons::pxAutoref().scaled(80,80)); //WARNING : icon scaled : pxAutoref 80,80
    ui->Refracteurlabel                     ->setPixmap(Icons::pxRefracteur().scaled(70,70)); //WARNING : icon scaled : pxRefracteur 70,70
    ui->Tonometrelabel                      ->setPixmap(Icons::pxTonometre().scaled(80,80)); //WARNING : icon scaled : pxTonometre 80,80
    ui->LockParamPosteupLabel->installEventFilter(this);

    ui->ParamtabWidget->setStyleSheet("QTabWidget::pane {border: 2px solid #C2C7CB; border-radius: 4px;}");

    QList<QWidget *> widglist = this->findChildren<QWidget *>();
    for (int i=0; i<widglist.size(); i++)
        widglist.at(i)->installEventFilter(this);
    ui->FermepushButton->installEventFilter(this);
    QList<UpRadioButton*> listbutt = ui->Principalframe->findChildren<UpRadioButton*>();
    for (int i=0; i<listbutt.size(); i++)
        listbutt.at(i)->setToggleable(false);
    QList<UpCheckBox*> listcheck = ui->Principalframe->findChildren<UpCheckBox*>();
    for (int i=0; i<listcheck.size(); i++)
        listcheck.at(i)->setToggleable(false);
     QStringList ListPortsCOM;
    ListPortsCOM << "-" << "COM1" << "COM2" << "COM3" << "COM4";
    ui->PortFrontoupComboBox->insertItems(0,ListPortsCOM);
    ui->PortFrontoupComboBox->addItem("Box");
    ui->PortAutorefupComboBox->insertItems(0,ListPortsCOM);
    ui->PortAutorefupComboBox->addItem("Box");
    ui->PortRefracteurupComboBox->insertItems(0,ListPortsCOM);
    ui->PortTonometreupComboBox->insertItems(0,ListPortsCOM);

    QString tip = tr("Indiquez ici l'emplacement du dossier de stockage des documents d'imagerie <br /><font color=\"green\"><b>SUR CE POSTE SERVEUR</b></font>");
    ui->PosteStockageupLabel        ->setImmediateToolTip(tip);
    ui->PosteStockageupLineEdit     ->setImmediateToolTip(tip);
    tip = tr("Indiquez ici <br /><font color=\"green\"><b>LE LIEN</b></font><br /> vers l'emplacement du dossier de stockage des documents d'imagerie <br /><font color=\"green\"><b>SUR LE SERVEUR</b></font>");
    ui->LocalStockageupLabel        ->setImmediateToolTip(tip);
    ui->LocalStockageupLineEdit     ->setImmediateToolTip(tip);
    tip = tr("Indiquez ici l'emplacement du dossier de sauvegarde des documents d'imagerie <br /><font color=\"green\"><b>SUR CE POSTE</b></font>");
    ui->DistantStockageupLabel      ->setImmediateToolTip(tip);
    ui->DistantStockageupLineEdit   ->setImmediateToolTip(tip);

    QStringList Listapp;
    Listapp << "-";
    QString req;
    bool ok;
    req = "select Marque, Modele from " TBL_APPAREILSREFRACTION " where Type = 'Autoref' ORDER BY Marque, Modele";
    QList<QVariantList> listautoref = db->StandardSelectSQL(req, ok);
    if (ok && listautoref.size()>0)
        for (int i=0; i<listautoref.size(); i++)
            Listapp << listautoref.at(i).at(0).toString() + " " + listautoref.at(i).at(1).toString();
    ui->AutorefupComboBox->insertItems(0,Listapp);
    Listapp.clear();
    Listapp << "-";
    req = "select Marque, Modele from " TBL_APPAREILSREFRACTION " where Type = 'Fronto' ORDER BY Marque, Modele";
    QList<QVariantList> listfronto = db->StandardSelectSQL(req, ok);
    if (ok && listfronto.size()>0)
        for (int i=0; i<listfronto.size(); i++)
            Listapp << listfronto.at(i).at(0).toString() + " " + listfronto.at(i).at(1).toString();
    ui->FrontoupComboBox->insertItems(0,Listapp);
    Listapp.clear();
    Listapp << "-";
    req = "select Marque, Modele from " TBL_APPAREILSREFRACTION " where Type = 'Refracteur' ORDER BY Marque, Modele";
    QList<QVariantList> listrefracteur = db->StandardSelectSQL(req, ok);
    if (ok && listrefracteur.size()>0)
        for (int i=0; i<listrefracteur.size(); i++)
            Listapp << listrefracteur.at(i).at(0).toString() + " " + listrefracteur.at(i).at(1).toString();
    ui->RefracteurupComboBox->insertItems(0,Listapp);
    Listapp.clear();
    Listapp << "-";
    req = "select Marque, Modele from " TBL_APPAREILSREFRACTION " where Type = 'Tonometre' ORDER BY Marque, Modele";
    QList<QVariantList> listtono = db->StandardSelectSQL(req, ok);
    if (ok && listtono.size()>0)
        for (int i=0; i<listtono.size(); i++)
            Listapp << listtono.at(i).at(0).toString() + " " + listtono.at(i).at(1).toString();
    ui->TonometreupComboBox->insertItems(0,Listapp);

    ui->AutorefupComboBox           ->setCurrentText(proc->settings()->value("Param_Poste/Autoref").toString());
    ui->PortAutorefupComboBox       ->setCurrentText(proc->settings()->value("Param_Poste/PortAutoref").toString());
    ui->FrontoupComboBox            ->setCurrentText(proc->settings()->value("Param_Poste/Fronto").toString());
    ui->TonometreupComboBox         ->setCurrentText(proc->settings()->value("Param_Poste/Tonometre").toString());
    ui->PortFrontoupComboBox        ->setCurrentText(proc->settings()->value("Param_Poste/PortFronto").toString());
    ui->RefracteurupComboBox        ->setCurrentText(proc->settings()->value("Param_Poste/Refracteur").toString());
    ui->PortRefracteurupComboBox    ->setCurrentText(proc->settings()->value("Param_Poste/PortRefracteur").toString());
    ui->PortTonometreupComboBox     ->setCurrentText(proc->settings()->value("Param_Poste/PortTonometre").toString());

    /*-------------------- GESTION DES VILLES ET DES CODES POSTAUX-------------------------------------------------------*/
       wdg_villeCP   = new VilleCPWidget(Datas::I()->villes, ui->VilleDefautframe);
       wdg_CPDefautlineEdit    = wdg_villeCP->ui->CPlineEdit;
       wdg_VilleDefautlineEdit = wdg_villeCP->ui->VillelineEdit;
       wdg_villeCP   ->move(15,10);
       wdg_villeCP->ui->CPlabel      ->setText(tr("Code postal par défaut"));
       wdg_villeCP->ui->Villelabel   ->setText(tr("Ville par défaut"));
       wdg_VilleDefautlineEdit                 ->setText(proc->settings()->value("Param_Poste/VilleParDefaut").toString());
       wdg_CPDefautlineEdit                    ->completer()->setCurrentRow(proc->settings()->value("Param_Poste/CodePostalParDefaut").toInt());
       // ce micmac est nécessaire à cause d'un bug de QCompleter en mode InLineCompletion
       // il faut synchroniser à la main le QCompleter et le QlineEdit au premier affichage
       wdg_CPDefautlineEdit                    ->setText(proc->settings()->value("Param_Poste/CodePostalParDefaut").toString());
   /*-------------------- GESTION DES VILLES ET DES CODES POSTAUX-------------------------------------------------------*/

   /*-------------------- GESTION DES TabOrder-------------------------------------------------------*/
       QList <QWidget*> ListTab;
       ListTab << ui->ActesCCAMupTableWidget << ui->AssocCCAMupTableWidget << ui->ChercheCCAMupLineEdit << ui->HorsNomenclatureupTableWidget
               << ui->ChoixFontupPushButton;
       for (int i = 0; i<ListTab.size()-1 ; i++ )
           ui->UserParamtab->setTabOrder(ListTab.at(i), ListTab.at(i+1));
       ListTab.clear();
   /*-------------------- GESTION DES TabOrder-------------------------------------------------------*/

    ui->OphtaSeulcheckBox           ->setChecked(true);

    AfficheParamUser();

    ui->LoginuplineEdit             ->setValidator(new QRegExpValidator(Utils::rgx_AlphaNumeric,this));
    ui->MDPuplineEdit               ->setValidator(new QRegExpValidator(Utils::rgx_AlphaNumeric_5_15,this));
    ui->NomuplineEdit               ->setValidator(new QRegExpValidator(Utils::rgx_rx,this));
    ui->PrenomuplineEdit            ->setValidator(new QRegExpValidator(Utils::rgx_rx,this));
    ui->MailuplineEdit              ->setValidator(new QRegExpValidator(Utils::rgx_mail,this));
    ui->PortableuplineEdit          ->setValidator(new QRegExpValidator(Utils::rgx_telephone,this));
    ui->EmplacementLocaluplineEdit  ->setValidator(new QRegExpValidator(Utils::rgx_IPV4_mask,this));

    ui->AssocCCAMupTableWidget          ->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->HorsNomenclatureupTableWidget   ->setSelectionBehavior(QAbstractItemView::SelectRows);
    wdg_assocCCAMcotationswdgbuttonframe                       ->setEnabled(false);
    wdg_HNcotationswdgbuttonframe                              ->setEnabled(false);
    ui->ActesCCAMlabel                  ->setEnabled(false);
    ui->OphtaSeulcheckBox               ->setEnabled(false);

    ui->LoginuplineEdit                 ->setEnabled(false);
    ui->MDPuplineEdit                   ->setEnabled(false);
    ui->idUseruplineEdit                ->setEnabled(false);
    ui->ChoixFontupPushButton           ->setEnabled(false);
    ui->ModifDataUserpushButton         ->setEnabled(false);
    ui->ImportDocsgroupBox              ->setEnabled(false);
    EnableWidgContent(ui->Sauvegardeframe,false);
    EnableWidgContent(ui->Appareilsconnectesframe,false);
    ui->ParamConnexiontabWidget         ->setEnabled(false);
    EnableWidgContent(ui->Instrmtsframe,false);
    EnableWidgContent(ui->Imprimanteframe,false);
    EnableWidgContent(ui->VilleDefautframe,false);
    ui->GestUserpushButton              ->setEnabled(false);
    ui->GestLieuxpushButton             ->setEnabled(false);
    ui->ParamMotifspushButton           ->setEnabled(false);
    ui->GestionBanquespushButton        ->setEnabled(false);
    ui->InitMDPAdminpushButton          ->setEnabled(false);
    ui->EmplacementServeurupComboBox    ->setEnabled(false);

    bool a,b,c;

    QString Base;
    Base = "BDD_POSTE";
    a = (proc->settings()->value(Base + "/Active").toString() == "YES");
    ui->PosteServcheckBox           ->setChecked(a);
    ui->Posteframe                  ->setVisible(a);
    ui->MonoConnexionupLabel        ->setVisible(a);
    ui->MonoDocsExtupLabel          ->setVisible(a);
    ui->MonoDocupTableWidget        ->setVisible(a);
    ui->PosteStockageupLabel        ->setVisible(a);
    ui->PosteStockageupLineEdit     ->setVisible(a);
    ui->PosteStockageupPushButton   ->setVisible(a);
    if (a)
    {
        ui->SQLPortPostecomboBox    ->setCurrentText(proc->settings()->value(Base + "/Port").toString());
        ui->PosteStockageupLineEdit->setText(m_parametres->dirimagerie());
    }
    Base = "BDD_LOCAL";
    b = (proc->settings()->value(Base + "/Active").toString() == "YES");
    ui->LocalServcheckBox           ->setChecked(b);
    ui->Localframe                  ->setVisible(b);
    ui->LocalConnexionupLabel       ->setVisible(b);
    ui->LocalDocsExtupLabel         ->setVisible(b);
    ui->LocalDocupTableWidget       ->setVisible(b);
    ui->LocalStockageupLabel        ->setVisible(b);
    ui->LocalStockageupLineEdit     ->setVisible(b);
    ui->LocalStockageupPushButton   ->setVisible(b);
    if (b)
    {
        ui->EmplacementLocaluplineEdit  ->setText(proc->settings()->value(Base + "/Serveur").toString());
        ui->SQLPortLocalcomboBox        ->setCurrentText(proc->settings()->value(Base + "/Port").toString());
        ui->LocalStockageupLineEdit     ->setText(proc->settings()->value("BDD_LOCAL/DossierImagerie").toString());
    }
    Base = "BDD_DISTANT";
    c = (proc->settings()->value(Base + "/Active").toString() == "YES");
    ui->DistantServcheckBox         ->setChecked(c);
    ui->Distantframe                ->setVisible(c);
    ui->DistantConnexionupLabel     ->setVisible(c);
    ui->DistantDocsExtupLabel       ->setVisible(c);
    ui->DistantDocupTableWidget     ->setVisible(c);
    ui->DistantStockageupLabel      ->setVisible(c);
    ui->DistantStockageupLineEdit   ->setVisible(c);
    ui->DistantStockageupPushButton ->setVisible(c);
    if (c)
    {
        ui->EmplacementDistantuplineEdit->setText(proc->settings()->value(Base + "/Serveur").toString());
        ui->SQLPortDistantcomboBox      ->setCurrentText(proc->settings()->value(Base + "/Port").toString());
        ui->DistantStockageupLineEdit   ->setText(proc->settings()->value("BDD_DISTANT/DossierImagerie").toString());
    }

    if (db->getMode() == DataBase::Poste)
        ui->ParamConnexiontabWidget->setCurrentIndex(0);
    else if (db->getMode() == DataBase::ReseauLocal)
        ui->ParamConnexiontabWidget->setCurrentIndex(1);
    if (db->getMode() == DataBase::Distant)
        ui->ParamConnexiontabWidget->setCurrentIndex(2);

    ui->ParamtabWidget->setCurrentIndex(0);

    ui->EntetespinBox->setValue(proc->settings()->value("Param_Imprimante/TailleEnTete").toInt());
    ui->EnteteALDspinBox->setValue(proc->settings()->value("Param_Imprimante/TailleEnTeteALD").toInt());
    ui->PiedDePagespinBox->setValue(proc->settings()->value("Param_Imprimante/TaillePieddePage").toInt());
    ui->PiedDePageOrdoLunettesspinBox->setValue(proc->settings()->value("Param_Imprimante/TaillePieddePageOrdoLunettes").toInt());
    ui->TopMargespinBox->setValue(proc->settings()->value("Param_Imprimante/TailleTopMarge").toInt());
    ui->ApercuImpressioncheckBox->setChecked(proc->settings()->value("Param_Imprimante/ApercuAvantImpression").toString() ==  "YES");
    ui->OrdoAvecDuplicheckBox->setChecked(proc->settings()->value("Param_Imprimante/OrdoAvecDupli").toString() ==  "YES");
    QString A = proc->settings()->value("BDD_LOCAL/PrioritaireGestionDocs").toString();
    if (A=="YES")
        ui->PrioritaireImportDocscheckBox->setChecked(true);
    else if (A=="NO")
        ui->NonImportDocscheckBox->setChecked(true);
    else
    {
        ui->NonPrioritaireImportDocscheckBox->setChecked(true);
        proc->settings()->setValue("BDD_LOCAL/PrioritaireGestionDocs","NORM");
    }
    t_timerverifimportdocs.start(500);
    connect (&t_timerverifimportdocs,   &QTimer::timeout,           this,   &dlg_param::VerifPosteImportDocs);
    connect (proc,                        &Procedures::ConnectTimers, this,   [=](bool a) {ConnectTimers(a);});

     if (m_parametres->versionbase() == 0)
        ui->VersionBaselabel->setText(tr("Version de la base") + "\t<font color=\"red\"><b>" + tr("inconnue") + "</b></font>");
    else
        ui->VersionBaselabel->setText(tr("Version de la base ") + "<font color=\"green\"><b>" + QString::number(m_parametres->versionbase()) + "</b></font>");
    ui->VersionRufuslabel->setText(tr("Version de Rufus ") + "<font color=\"green\"><b>" + qApp->applicationVersion() + "</b></font>");

    // Mise en forme de la table DossiersDocuments
    ui->MonoDocupTableWidget->setPalette(QPalette(Qt::white));
    ui->MonoDocupTableWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->MonoDocupTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->MonoDocupTableWidget->verticalHeader()->setVisible(false);
    ui->MonoDocupTableWidget->setSelectionMode(QAbstractItemView::NoSelection);
    ui->MonoDocupTableWidget->setColumnCount(4);
    ui->MonoDocupTableWidget->setColumnWidth(0,0);       // idDossiersDocs
    ui->MonoDocupTableWidget->setColumnWidth(1,220);     // NomDocuments + NomAppareil
    ui->MonoDocupTableWidget->setColumnWidth(2,300);     // DossierDocuments
    ui->MonoDocupTableWidget->setColumnWidth(3,30);      // bouton dossier
    ui->MonoDocupTableWidget->setColumnHidden(0,true);
    ui->MonoDocupTableWidget->setHorizontalHeaderItem(0, new QTableWidgetItem("id"));
    ui->MonoDocupTableWidget->setHorizontalHeaderItem(1, new QTableWidgetItem(tr("Type d'examen")));
    ui->MonoDocupTableWidget->setHorizontalHeaderItem(2, new QTableWidgetItem(tr("Dossier de stockage")));
    ui->MonoDocupTableWidget->setHorizontalHeaderItem(3, new QTableWidgetItem(""));
    ui->MonoDocupTableWidget->horizontalHeader()->setVisible(true);
    for (int i=0; i<ui->MonoDocupTableWidget->columnCount(); i++)
        ui->MonoDocupTableWidget->horizontalHeaderItem(i)->setTextAlignment(Qt::AlignLeft);
    ui->LocalDocupTableWidget->setPalette(QPalette(Qt::white));
    ui->LocalDocupTableWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->LocalDocupTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->LocalDocupTableWidget->verticalHeader()->setVisible(false);
    ui->LocalDocupTableWidget->setSelectionMode(QAbstractItemView::NoSelection);
    ui->LocalDocupTableWidget->setColumnCount(4);
    ui->LocalDocupTableWidget->setColumnWidth(0,0);       // idDossiersDocs
    ui->LocalDocupTableWidget->setColumnWidth(1,220);     // NomDocuments + NomAppareil
    ui->LocalDocupTableWidget->setColumnWidth(2,300);     // DossierDocuments
    ui->LocalDocupTableWidget->setColumnWidth(3,30);      // bouton dossier
    ui->LocalDocupTableWidget->setColumnHidden(0,true);
    ui->LocalDocupTableWidget->setHorizontalHeaderItem(0, new QTableWidgetItem("id"));
    ui->LocalDocupTableWidget->setHorizontalHeaderItem(1, new QTableWidgetItem(tr("Type d'examen")));
    ui->LocalDocupTableWidget->setHorizontalHeaderItem(2, new QTableWidgetItem(tr("Dossier de stockage")));
    ui->LocalDocupTableWidget->setHorizontalHeaderItem(3, new QTableWidgetItem(""));
    ui->LocalDocupTableWidget->horizontalHeader()->setVisible(true);
    for (int i=0; i<ui->LocalDocupTableWidget->columnCount(); i++)
        ui->LocalDocupTableWidget->horizontalHeaderItem(i)->setTextAlignment(Qt::AlignLeft);
    ui->DistantDocupTableWidget->setPalette(QPalette(Qt::white));
    ui->DistantDocupTableWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->DistantDocupTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->DistantDocupTableWidget->verticalHeader()->setVisible(false);
    ui->DistantDocupTableWidget->setSelectionMode(QAbstractItemView::NoSelection);
    ui->DistantDocupTableWidget->setColumnCount(4);
    ui->DistantDocupTableWidget->setColumnWidth(0,0);       // idDossiersDocs
    ui->DistantDocupTableWidget->setColumnWidth(1,220);     // NomDocuments + NomAppareil
    ui->DistantDocupTableWidget->setColumnWidth(2,300);     // DossierDocuments
    ui->DistantDocupTableWidget->setColumnWidth(3,30);      // bouton dossier
    ui->DistantDocupTableWidget->setColumnHidden(0,true);
    ui->DistantDocupTableWidget->setHorizontalHeaderItem(0, new QTableWidgetItem("id"));
    ui->DistantDocupTableWidget->setHorizontalHeaderItem(1, new QTableWidgetItem(tr("Type d'examen")));
    ui->DistantDocupTableWidget->setHorizontalHeaderItem(2, new QTableWidgetItem(tr("Dossier de stockage")));
    ui->DistantDocupTableWidget->setHorizontalHeaderItem(3, new QTableWidgetItem(""));
    ui->DistantDocupTableWidget->horizontalHeader()->setVisible(true);
    for (int i=0; i<ui->DistantDocupTableWidget->columnCount(); i++)
        ui->DistantDocupTableWidget->horizontalHeaderItem(i)->setTextAlignment(Qt::AlignLeft);
    // Mise en forme de la table AppareilsConnectes
    ui->AppareilsConnectesupTableWidget->setPalette(QPalette(Qt::white));
    ui->AppareilsConnectesupTableWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->AppareilsConnectesupTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->AppareilsConnectesupTableWidget->verticalHeader()->setVisible(false);
    ui->AppareilsConnectesupTableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->AppareilsConnectesupTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->AppareilsConnectesupTableWidget->setColumnCount(4);
    ui->AppareilsConnectesupTableWidget->setColumnWidth(0,0);       // idppareil
    ui->AppareilsConnectesupTableWidget->setColumnWidth(1,258);     // TitreExamen
    ui->AppareilsConnectesupTableWidget->setColumnWidth(2,281);     // NomAppareil
    ui->AppareilsConnectesupTableWidget->setColumnWidth(3,0);       // Format
    ui->AppareilsConnectesupTableWidget->setHorizontalHeaderItem(0, new QTableWidgetItem("id"));
    ui->AppareilsConnectesupTableWidget->setHorizontalHeaderItem(1, new QTableWidgetItem(tr("Type d'examen")));
    ui->AppareilsConnectesupTableWidget->setHorizontalHeaderItem(2, new QTableWidgetItem(tr("Nom de l'appareil")));
    ui->AppareilsConnectesupTableWidget->setHorizontalHeaderItem(3, new QTableWidgetItem(tr("Format nom fichier")));
    ui->AppareilsConnectesupTableWidget->horizontalHeader()->setVisible(true);
    for (int i=0; i<ui->AppareilsConnectesupTableWidget->columnCount(); i++)
        ui->AppareilsConnectesupTableWidget->horizontalHeaderItem(i)->setTextAlignment(Qt::AlignLeft);
    ui->AppareilsConnectesupTableWidget->FixLargeurTotale();
    ui->AppareilsconnectesupLabel->setText(tr("Appareils connectés au réseau") + " <font color=\"green\"><b>" + Datas::I()->sites->getById(m_currentuser->idsitedetravail())->nom() + "</b></font> ");
    QVBoxLayout *applay = new QVBoxLayout();
    applay      ->addWidget(ui->AppareilsconnectesupLabel);
    applay      ->addWidget(wdg_appareilswdgbuttonframe->widgButtonParent());
    applay      ->addSpacerItem(new QSpacerItem(10,10,QSizePolicy::Expanding, QSizePolicy::Expanding));
    marge   = 10;
    applay      ->setContentsMargins(marge,marge,marge,marge);
    applay      ->setSpacing(marge);
    ui->Appareilsconnectesframe->setFixedWidth(wdg_appareilswdgbuttonframe->widgButtonParent()->width() + marge + marge);
    ui->Appareilsconnectesframe->setLayout(applay);

    ui->Sauvegardeframe         ->setEnabled(db->getMode() == DataBase::Poste);
    ui->DirBackupuplineEdit->setText(m_parametres->dirbkup());
    if (m_parametres->heurebkup().isValid())
        ui->HeureBackuptimeEdit->setTime(m_parametres->heurebkup());
    ui->LundiradioButton    ->setChecked(m_parametres->daysbkup().testFlag(Utils::Lundi));
    ui->MardiradioButton    ->setChecked(m_parametres->daysbkup().testFlag(Utils::Mardi));
    ui->MercrediradioButton ->setChecked(m_parametres->daysbkup().testFlag(Utils::Mercredi));
    ui->JeudiradioButton    ->setChecked(m_parametres->daysbkup().testFlag(Utils::Jeudi));
    ui->VendrediradioButton ->setChecked(m_parametres->daysbkup().testFlag(Utils::Vendredi));
    ui->SamediradioButton   ->setChecked(m_parametres->daysbkup().testFlag(Utils::Samedi));
    ui->DimancheradioButton ->setChecked(m_parametres->daysbkup().testFlag(Utils::Dimanche));
    if (Datas::I()->postesconnectes->admin() != Q_NULLPTR)
        ui->TCPlabel->setText("<font color=\"black\">" + tr("Serveur") + " </font>"
                            + "<font color=\"green\"><b>" + Datas::I()->postesconnectes->admin()->ipadress() + "</b></font>"
                            + "<font color=\"black\"> " + " - " + tr("Port") + " " + "</font>"
                            + "<font color=\"green\"><b> " NOM_PORT_TCPSERVEUR "</b></font>");
    else
        ui->TCPlabel->setText("");
    Remplir_Tables();
    ConnectSlots();
}

dlg_param::~dlg_param()
{
    delete ui;
}

void dlg_param::Slot_AfficheToolTip(QTableWidgetItem *id)
{
    QPoint pos = cursor().pos();
    QRect rect = QRect(pos,QSize(10,10));
    if (sender() == ui->ActesCCAMupTableWidget)
        QToolTip::showText(cursor().pos(),ui->ActesCCAMupTableWidget->item(id->row(),4)->text(), ui->ActesCCAMupTableWidget, rect, 2000);
    else if (sender() == ui->AssocCCAMupTableWidget)
    {
        QString tip = id->text();
        UpLineEdit * line = dynamic_cast<UpLineEdit*>(ui->AssocCCAMupTableWidget->cellWidget(id->row(),2));
        if (line != Q_NULLPTR)
            if (line->datas().toString() != "")
                tip += "\n" + line->datas().toString();
        QToolTip::showText(cursor().pos(),tip, ui->AssocCCAMupTableWidget, rect, 2000);
    }
    else if (sender() == ui->HorsNomenclatureupTableWidget)
    {
        QString tip = id->text();
        UpLineEdit * line = dynamic_cast<UpLineEdit*>(ui->HorsNomenclatureupTableWidget->cellWidget(id->row(),2));
        if (line != Q_NULLPTR)
            if (line->datas().toString() != "")
                tip += "\n" + line->datas().toString();
        QToolTip::showText(cursor().pos(),tip, ui->HorsNomenclatureupTableWidget, rect, 2000);
    }
}

void dlg_param::Slot_FermepushButtonClicked()
{
    if (m_modifposte)
    {
        UpMessageBox msgbox;
        msgbox.setText(tr("Modifications non enregistrées!"));
        msgbox.setInformativeText(tr("Vous avez modifié certains éléments. Voulez vous enregistrer ces modifications?"));
        msgbox.setIcon(UpMessageBox::Warning);
        UpSmallButton AnnulBouton(tr("Annuler"));
        UpSmallButton OKBouton(tr("Enregistrer"));
        msgbox.addButton(&AnnulBouton, UpSmallButton::CANCELBUTTON);
        msgbox.addButton(&OKBouton, UpSmallButton::STARTBUTTON);
        msgbox.exec();
        if (msgbox.clickedButton() == &OKBouton)
            if (!Valide_Modifications())
                return;
    }
    if (VerifDirStockageImagerie())
        reject();
}

void dlg_param::Slot_EnableAppBoutons()
{
   wdg_appareilswdgbuttonframe->wdg_moinsBouton->setEnabled(true);
}

void dlg_param::Slot_ChercheCCAM(QString txt)
{
    QList<QTableWidgetItem*> listitems = ui->ActesCCAMupTableWidget->findItems(txt, Qt::MatchStartsWith);
    if (listitems.size()<ui->ActesCCAMupTableWidget->rowCount())
        ui->ShowCCAMlabel               ->setPixmap(Icons::pxApres().scaled(10,10)); //WARNING : icon scaled : pApres 10,10
    else
        ui->ShowCCAMlabel               ->setPixmap(QPixmap());
    if (listitems.size()>0)
    {
        QTableWidgetItem *pitem = listitems.at(0);
        QModelIndex index = ui->ActesCCAMupTableWidget->model()->index(pitem->row(),1);
        ui->ActesCCAMupTableWidget->scrollTo(index, QAbstractItemView::PositionAtCenter);
    }
    listitems = ui->AssocCCAMupTableWidget->findItems(txt, Qt::MatchStartsWith);
    if (listitems.size()>0)
    {
        QTableWidgetItem *pitem = listitems.at(0);
        QModelIndex index = ui->AssocCCAMupTableWidget->model()->index(pitem->row(),1);
        ui->AssocCCAMupTableWidget->scrollTo(index, QAbstractItemView::PositionAtTop);
    }
}

void dlg_param::Slot_ChoixDossierStockageApp()
{
    UpPushButton *bout = static_cast<UpPushButton*>(sender());
    QString req = "select TitreExamen, NomAppareil from " TBL_LISTEAPPAREILS " where idAppareil = " + QString::number(bout->iD());
    bool ok;
    QVariantList examdata = db->getFirstRecordFromStandardSelectSQL(req, ok);
    QString exam = "";
    if (ok && examdata.size()>0)
        exam = examdata.at(1).toString();
    int mode = DataBase::ReseauLocal;
    if (ui->MonoDocupTableWidget->isAncestorOf(bout))
        mode = DataBase::Poste;
    else if (ui->LocalDocupTableWidget->isAncestorOf(bout))
        mode = DataBase::ReseauLocal;
    else if (ui->DistantDocupTableWidget->isAncestorOf(bout))
        mode = DataBase::Distant;
    QString dir = proc->pathDossierDocuments(exam, mode);
    if (dir == "")
        dir = QDir::homePath() + DIR_RUFUS;
    QFileDialog dialog(this, "", dir);
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setViewMode(QFileDialog::List);
    if (dialog.exec()>0)
    {
        QDir dockdir = dialog.directory();
        QString Base;
        int row;
        UpLineEdit *line = Q_NULLPTR;
        switch (mode) {
        case DataBase::Poste:
            row = ui->MonoDocupTableWidget->findItems(QString::number(bout->iD()), Qt::MatchExactly).at(0)->row();
            line    = dynamic_cast<UpLineEdit*>(ui->MonoDocupTableWidget->cellWidget(row,2));
            if (line!=Q_NULLPTR)
                line->setText(dockdir.path());
            Base = "BDD_POSTE";
            break;
        case DataBase::ReseauLocal:
            row = ui->LocalDocupTableWidget->findItems(QString::number(bout->iD()), Qt::MatchExactly).at(0)->row();
            line    = dynamic_cast<UpLineEdit*>(ui->LocalDocupTableWidget->cellWidget(row,2));
            if (line!=Q_NULLPTR)
                line->setText(dockdir.path());
            Base = "BDD_LOCAL";
            break;
        case DataBase::Distant:
            row = ui->DistantDocupTableWidget->findItems(QString::number(bout->iD()), Qt::MatchExactly).at(0)->row();
            line    = dynamic_cast<UpLineEdit*>(ui->DistantDocupTableWidget->cellWidget(row,2));
            if (line!=Q_NULLPTR)
                line->setText(dockdir.path());
            Base = "BDD_DISTANT";
            break;
        default:
            break;
        }
        proc->settings()->setValue(Base + "/DossiersDocuments/" + exam, dockdir.path());
    }
}

void dlg_param::Slot_EnregDossierStockageApp(QString dir)
{
    UpLineEdit *line    = dynamic_cast<UpLineEdit*>(sender());
    if (line==Q_NULLPTR) return;
    if (!QDir(dir).exists() && dir != "")
    {
        QString textline = line->valeuravant();
        UpMessageBox::Watch(this,tr("Repertoire invalide!"));
        line->setText(textline);
        return;
    }
    QString id, Base;
    int mode = DataBase::ReseauLocal;
    if (ui->MonoDocupTableWidget->isAncestorOf(line))
    {
        mode = DataBase::Poste;
        id = ui->MonoDocupTableWidget->item(line->Row(),0)->text();
    }
    else if (ui->LocalDocupTableWidget->isAncestorOf(line))
    {
        mode = DataBase::ReseauLocal;
        id = ui->LocalDocupTableWidget->item(line->Row(),0)->text();
    }
    else if (ui->DistantDocupTableWidget->isAncestorOf(line))
    {
        mode = DataBase::Distant;
        id = ui->DistantDocupTableWidget->item(line->Row(),0)->text();
    }
    switch (mode) {
    case DataBase::Poste:
        Base = "BDD_POSTE";
        break;
    case DataBase::ReseauLocal:
        Base = "BDD_LOCAL";
        break;
    case DataBase::Distant:
        Base = "BDD_DISTANT";
        break;
    default:
        break;
    }
    QString req = "select NomAppareil from " TBL_LISTEAPPAREILS " where idAppareil = " + id;
    bool ok;
    QVariantList appdata = db->getFirstRecordFromStandardSelectSQL(req, ok);
    QString app = "";
    if (ok && appdata.size()>0)
        app = appdata.at(0).toString();
    if (app != "")
        proc->settings()->setValue(Base + "/DossiersDocuments/" + app, dir);
    else
        UpMessageBox::Watch(this,tr("Impossible de retrouver le nom de l'appareil"));

}

void dlg_param::Slot_ChoixButtonFrame(int i)
{
    WidgetButtonFrame *widgbutt = dynamic_cast<WidgetButtonFrame*>(sender());
    if (widgbutt== wdg_HNcotationswdgbuttonframe)
    {
        switch (i) {
        case 1:
            NouvHorsNomenclature();
            break;
        case 0:
            ModifHorsNomenclature();
            break;
        case -1:
            SupprHorsNomenclature();
            break;
        default:
            break;
        }
    }
    else if (widgbutt== wdg_assocCCAMcotationswdgbuttonframe)
    {
        switch (i) {
        case 1:
            NouvAssocCCAM();
            break;
        case 0:
            ModifAssocCCAM();
            break;
        case -1:
            SupprAssocCCAM();
            break;
        default:
            break;
        }
    }
    else if (widgbutt== wdg_appareilswdgbuttonframe)
    {
        switch (i) {
        case 1:
            NouvAppareil();
            break;
        case -1:
            SupprAppareil();
            break;
        default:
            break;
        }
    }
}

void dlg_param::Slot_ChoixFontpushButtonClicked()
{
    Dlg_Fonts = new dlg_fontdialog(QDir::homePath() + FILE_INI, "PositionsFiches/PositionFontDialog");
    Dlg_Fonts->setFont(qApp->font());
    Dlg_Fonts->setWindowTitle(tr("Choisissez la police d'écran"));
    if (Dlg_Fonts->exec() > 0)
    {
        QString fontrequete = "update " TBL_UTILISATEURS " set UserPoliceEcran = '" + Dlg_Fonts->font().toString()
                                + "', UserPoliceAttribut = '" + Dlg_Fonts->fontAttribut()
                                + "' where idUser = " + QString::number(m_currentuser->id());
        db->StandardSQL(fontrequete,"dlg_param::Slot__ChoixFontpushButtonClicked()");
    }
    delete Dlg_Fonts;
}

void dlg_param::Slot_ClearCom(int a)
{
    UpComboBox* box = dynamic_cast<UpComboBox*>(sender());
    if (box)
    {
        if (a==0)
        {
            if (box==ui->AutorefupComboBox)
                ui->PortAutorefupComboBox->setCurrentIndex(0);
            if (box==ui->FrontoupComboBox)
                ui->PortFrontoupComboBox->setCurrentIndex(0);
            if (box==ui->TonometreupComboBox)
                ui->PortTonometreupComboBox->setCurrentIndex(0);
            if (box==ui->RefracteurupComboBox)
                ui->PortRefracteurupComboBox->setCurrentIndex(0);
        }
    }
    ui->PortAutorefupComboBox->setEnabled(ui->AutorefupComboBox->currentIndex()>0);
    ui->PortFrontoupComboBox->setEnabled(ui->FrontoupComboBox->currentIndex()>0);
    ui->PortRefracteurupComboBox->setEnabled(ui->RefracteurupComboBox->currentIndex()>0);
    ui->PortTonometreupComboBox->setEnabled(ui->TonometreupComboBox->currentIndex()>0);
}

void dlg_param::ConnectTimers(bool a)
{
    if (a)
    {
        t_timerverifimportdocs  .start(5000);
        connect (&t_timerverifimportdocs,   &QTimer::timeout,           this,   &dlg_param::VerifPosteImportDocs);
    }
    else
    {
        t_timerverifimportdocs  .disconnect();
        t_timerverifimportdocs  .stop();
    }
}

void dlg_param::Slot_EnableModif(QWidget *obj)
{
    if (obj == ui->LockParamPosteupLabel)
    {
        if (ui->LockParamPosteupLabel->pixmap()->toImage() == Icons::pxVerrouiller().toImage())
        {
            m_MDPadminverifie = Utils::VerifMDP(proc->getMDPAdmin(),"Saisissez le mot de passe Administrateur", m_MDPadminverifie);
            if (m_MDPadminverifie)
            {
                ui->Posteframe->setEnabled(ui->PosteServcheckBox->isChecked());
                ui->Localframe->setEnabled(ui->LocalServcheckBox->isChecked());
                ui->Distantframe->setEnabled(ui->DistantServcheckBox->isChecked());
                ui->LockParamPosteupLabel->setPixmap(Icons::pxDeverouiller());
            }
        }
        else
        {
            if (m_modifposte)
                if (!Valide_Modifications()) return;
            ui->LockParamPosteupLabel->setPixmap(Icons::pxVerrouiller());
        }
        bool a = (ui->LockParamPosteupLabel->pixmap()->toImage() == Icons::pxDeverouiller().toImage());
        EnableWidgContent(ui->Instrmtsframe,a);
        if (a && ui->FrontoupComboBox       ->currentIndex()==0)    ui->PortFrontoupComboBox->setEnabled(false);
        if (a && ui->RefracteurupComboBox   ->currentIndex()==0)    ui->PortRefracteurupComboBox->setEnabled(false);
        if (a && ui->AutorefupComboBox      ->currentIndex()==0)    ui->PortAutorefupComboBox->setEnabled(false);
        if (a && ui->TonometreupComboBox    ->currentIndex()==0)    ui->PortTonometreupComboBox->setEnabled(false);
        ui->ParamConnexiontabWidget->setEnabled(a);
        EnableWidgContent(ui->Imprimanteframe,a);
        EnableWidgContent(ui->VilleDefautframe,a);
        ui->ImportDocsgroupBox->setEnabled(a);
    }
    else if (obj == ui->LockParamUserupLabel)
    {
        if (ui->LockParamUserupLabel->pixmap()->toImage() == Icons::pxVerrouiller().toImage())
        {
            m_MDPuserverifie = Utils::VerifMDP(m_currentuser->password(),tr("Saisissez votre mot de passe"), m_MDPuserverifie);
            if (m_MDPuserverifie)
                ui->LockParamUserupLabel->setPixmap(Icons::pxDeverouiller());
        }
        else
        {
            ui->LockParamUserupLabel->setPixmap(Icons::pxVerrouiller());
        }
        bool a = (ui->LockParamUserupLabel->pixmap()->toImage() == Icons::pxDeverouiller().toImage());

        ui->ChoixFontupPushButton   ->setEnabled(a);
        ui->ModifDataUserpushButton ->setEnabled(a);

        EnableActesCCAM(a);
        EnableAssocCCAM(a);
        EnableHorsNomenclature(a);
    }

    else if (obj == ui->LockParamGeneralupLabel)
    {
        QString A = proc->PosteImportDocs();
        if(A.contains(" - " NOM_ADMINISTRATEURDOCS))
        {
            UpMessageBox::Watch(this,tr("Vous ne pouvez pas modifier les paramètres généraux"),
                                     tr("Une session de RufusAdmin est actuellement active sur le poste") + " "
                                     + A.remove(" - " NOM_ADMINISTRATEURDOCS));
            return;
        }
        if (ui->LockParamGeneralupLabel->pixmap()->toImage() == Icons::pxVerrouiller().toImage())
        {
            m_MDPadminverifie = Utils::VerifMDP(proc->getMDPAdmin(),tr("Saisissez le mot de passe Administrateur"), m_MDPadminverifie);
            if (m_MDPadminverifie)
            {
                ui->LockParamGeneralupLabel ->setPixmap(Icons::pxDeverouiller());
                wdg_appareilswdgbuttonframe->wdg_moinsBouton      ->setEnabled(ui->AppareilsConnectesupTableWidget->selectedItems().size()>0);
            }
        }
        else
            ui->LockParamGeneralupLabel->setPixmap(Icons::pxVerrouiller());
        bool a = (ui->LockParamGeneralupLabel->pixmap()->toImage() == Icons::pxDeverouiller().toImage());
        if (db->getMode() == DataBase::Distant)
            EnableWidgContent(ui->Appareilsconnectesframe,false);
        else
            EnableWidgContent(ui->Appareilsconnectesframe,a);
        wdg_appareilswdgbuttonframe->wdg_moinsBouton          ->setEnabled(ui->AppareilsConnectesupTableWidget->selectedItems().size()>0);
        ui->GestUserpushButton              ->setEnabled(a);
        ui->GestLieuxpushButton             ->setEnabled(a);
        ui->ParamMotifspushButton           ->setEnabled(a);
        ui->InitMDPAdminpushButton          ->setEnabled(a);
        ui->GestionBanquespushButton        ->setEnabled(a);
        ui->EmplacementServeurupComboBox    ->setEnabled(a);
        EnableWidgContent(ui->Sauvegardeframe, db->getMode() == DataBase::Poste && a);
    }
}

void dlg_param::Slot_EnableFrameServeur(bool a)
{
    if (sender() == ui->PosteServcheckBox)
    {
        ui->Posteframe                  ->setVisible(a);
        ui->MonoConnexionupLabel        ->setVisible(a);
        ui->MonoDocsExtupLabel          ->setVisible(a);
        ui->MonoDocupTableWidget        ->setVisible(a);
        ui->PosteStockageupLabel        ->setVisible(a);
        ui->PosteStockageupLineEdit     ->setVisible(a);
        ui->PosteStockageupPushButton   ->setVisible(a);
        ui->Posteframe                  ->setEnabled(a);
        ui->MonoConnexionupLabel        ->setEnabled(a);
        ui->MonoDocsExtupLabel          ->setEnabled(a);
        ui->MonoDocupTableWidget        ->setEnabled(a);
        ui->PosteStockageupLabel        ->setEnabled(a);
        ui->PosteStockageupLineEdit     ->setEnabled(a);
        ui->PosteStockageupPushButton   ->setEnabled(a);
    }
    if (sender() == ui->LocalServcheckBox)
    {
        ui->Localframe                  ->setVisible(a);
        ui->LocalConnexionupLabel       ->setVisible(a);
        ui->LocalDocsExtupLabel         ->setVisible(a);
        ui->LocalDocupTableWidget       ->setVisible(a);
        ui->LocalStockageupLabel        ->setVisible(a);
        ui->LocalStockageupLineEdit     ->setVisible(a);
        ui->LocalStockageupPushButton   ->setVisible(a);
        ui->Localframe                  ->setEnabled(a);
        ui->LocalConnexionupLabel       ->setEnabled(a);
        ui->LocalDocsExtupLabel         ->setEnabled(a);
        ui->LocalDocupTableWidget       ->setEnabled(a);
        ui->LocalStockageupLabel        ->setEnabled(a);
        ui->LocalStockageupLineEdit     ->setEnabled(a);
        ui->LocalStockageupPushButton   ->setEnabled(a);
    }
    if (sender() == ui->DistantServcheckBox)
    {
        if (a)
            UpMessageBox::Watch(this,tr("L'accès distant fonctionne obligatoirement "
                                         "avec un cryptage de données SSL."
                                         "Si vous voulez utiliser l'accès distant "
                                         "sur ce poste, il faut vous assurer aue:\n"
                                         "1. le serveur MySQL est configuré pour le cryptage\n"
                                         "2. que les clés clientes SSL sont installées sur ce poste.\n\n"
                                         "Sinon, ça ne marchera pas.\n\n"
                                         "Reportez vous à la page internet\n"
                                         "http://www.rufusvision.org/configuration-pour-un-accegraves-agrave-distance---le-cryptage-de-donneacutees-ssl.html\n"
                                         "pour savoir comment faire"));
        ui->Distantframe                ->setVisible(a);
        ui->Distantframe                ->setEnabled(a);
        ui->DistantConnexionupLabel     ->setVisible(a);
        ui->DistantStockageupLabel      ->setVisible(a);
        ui->DistantStockageupLineEdit   ->setVisible(a);
        ui->DistantStockageupPushButton ->setVisible(a);
        ui->DistantConnexionupLabel     ->setEnabled(a);
        ui->DistantStockageupLabel      ->setEnabled(a);
        ui->DistantStockageupLineEdit   ->setEnabled(a);
        ui->DistantStockageupPushButton ->setEnabled(a);
     }
}

void dlg_param::Slot_EnableOKModifPosteButton()
{
    m_modifposte = true;
}

void dlg_param::Slot_FiltreActesOphtaSeulmt(bool b)
{
    Remplir_TableActesCCAM(b);
    bool a = (ui->LockParamUserupLabel->pixmap()->toImage() == Icons::pxDeverouiller().toImage());
    for (int i=0; i<ui->ActesCCAMupTableWidget->rowCount(); i++)
    {
        UpCheckBox *check = dynamic_cast<UpCheckBox*>(ui->ActesCCAMupTableWidget->cellWidget(i,0));
        if (check) check->setEnabled(a);
        if (ui->ActesCCAMupTableWidget->columnCount()==6)
        {
            UpLineEdit *lbl = dynamic_cast<UpLineEdit*>(ui->ActesCCAMupTableWidget->cellWidget(i,5));
            if (lbl)
                lbl->setEnabled(a);
        }
    }
}

void dlg_param::Slot_GestionBanques()
{
    Dlg_Banq = new dlg_gestionbanques();
    Dlg_Banq->exec();
}

void dlg_param::Slot_GestionDatasCurrentUser()
{
    Dlg_GestUsr = new dlg_gestionusers(proc->idLieuExercice(), dlg_gestionusers::MODIFUSER, m_MDPuserverifie);
    Dlg_GestUsr->setWindowTitle(tr("Enregistrement de l'utilisateur ") +  m_currentuser->login());
    m_donneesusermodifiees = (Dlg_GestUsr->exec()>0);
    if(m_donneesusermodifiees)
    {
        proc->SetUserAllData(m_currentuser, Item::ForceUpdate);
        AfficheParamUser();
    }
    if (!m_MDPuserverifie)
        m_MDPuserverifie = Dlg_GestUsr->isMDPverified();
    delete Dlg_GestUsr;
}

void dlg_param::Slot_GestionUsers()
{
    Dlg_GestUsr = new dlg_gestionusers(proc->idLieuExercice(), dlg_gestionusers::ADMIN, m_MDPadminverifie);
    Dlg_GestUsr->setWindowTitle(tr("Gestion des utilisateurs"));
    m_donneesusermodifiees = (Dlg_GestUsr->exec()>0);
    if(m_donneesusermodifiees)
    {
        Datas::I()->users->initListe();
        proc->SetUserAllData(m_currentuser, Item::ForceUpdate);
        AfficheParamUser();
    }
    delete Dlg_GestUsr;
    UpMessageBox::Watch(this, tr("Donnes utilisateurs modifiées?"),
                              tr("Si vous avez modifié des données d'utilisateurs actuellement connectés,\n"
                                 "chacun de ces utilisateurs doit relancer le programme\n"
                                 "pour pouvoir prendre en compte les modifications apportées!"));
}

void dlg_param::Slot_GestLieux()
{
    dlg_GestionLieux *gestLieux = new dlg_GestionLieux();
    gestLieux->exec();
    ReconstruitListeLieuxExerciceAllusers();
    delete gestLieux;
}

void dlg_param::ReconstruitListeLieuxExerciceUser(User *user)
{
    /*-------------------- GESTION DES LIEUX D'EXERCICE-------------------------------------------------------*/
    ui->AdressupTableWidget->clear();
    ui->EmplacementServeurupComboBox->clear();
    int             ColCount = 3;

    ui->AdressupTableWidget->setColumnCount(ColCount);
    ui->AdressupTableWidget->verticalHeader()->setVisible(false);
    ui->AdressupTableWidget->setGridStyle(Qt::NoPen);
    int li = 0;                                                                                   // Réglage de la largeur et du nombre des colonnes
    ui->AdressupTableWidget->setColumnWidth(li,200);                                              // nom du lieu
    li++;
    ui->AdressupTableWidget->setColumnWidth(li,180);                                              // ville
    li++;
    ui->AdressupTableWidget->setColumnWidth(li,110);                                              // téléphone
    ui->AdressupTableWidget->FixLargeurTotale();

    UpHeaderView *upheader = new UpHeaderView(ui->AdressupTableWidget->horizontalHeader());
    upheader->setVisible(true);
    QStringList list;
    list << tr("Liste des lieux d'exercice");
    QStandardItemModel *mod = new QStandardItemModel();
    mod->setHorizontalHeaderLabels(list);
    upheader->setModel(mod);
    upheader->reDim(0,0,2);

    QString req ="select j.idLieu, NomLieu, LieuAdresse1, LieuAdresse2, LieuAdresse3, LieuCodePostal, LieuVille, LieuTelephone from " TBL_LIEUXEXERCICE
                       " j inner join " TBL_JOINTURESLIEUX " p on j.idLieu = p.idLieu where iduser = " + QString::number(user->id());
    bool ok;
    QList<QVariantList> adrlist = db->StandardSelectSQL(req, ok);
    ui->AdressupTableWidget->setRowCount(adrlist.size());
    for (int i=0; i< adrlist.size(); i++)
    {
        QString data ("");
        if (adrlist.at(i).at(1).toString()!="")
            data += adrlist.at(i).at(1).toString();
        if (adrlist.at(i).at(2).toString()!="")
            data += (data != ""? "\n" : "") + adrlist.at(i).at(2).toString();
        if (adrlist.at(i).at(3).toString()!="")
            data += (data != ""? "\n" : "") + adrlist.at(i).at(3).toString();
        if (adrlist.at(i).at(4).toString()!="")
            data += (data != ""? "\n" : "") + adrlist.at(i).at(4).toString();
        if (adrlist.at(i).at(5).toString()!="")
            data += (data != ""? "\n" : "") + adrlist.at(i).at(5).toString();
        if (adrlist.at(i).at(6).toString()!="")
            data += (data != ""? " " : "") + adrlist.at(i).at(6).toString();
        if (adrlist.at(i).at(7).toString()!="")
            data += (data != ""? "\nTel: " : "Tel: ") + adrlist.at(i).at(7).toString();

        QTableWidgetItem *pitem1, *pitem2, *pitem3;
        pitem1 = new QTableWidgetItem();
        pitem2 = new QTableWidgetItem();
        pitem3 = new QTableWidgetItem();
        pitem1->setText(adrlist.at(i).at(1).toString());
        pitem2->setText(adrlist.at(i).at(6).toString());
        pitem3->setText(adrlist.at(i).at(7).toString());
        ui->AdressupTableWidget->setItem(i,0,pitem1);
        ui->AdressupTableWidget->setItem(i,1,pitem2);
        ui->AdressupTableWidget->setItem(i,2,pitem3);
        pitem1->setToolTip(data);
        pitem2->setToolTip(data);
        pitem3->setToolTip(data);
        ui->AdressupTableWidget->setRowHeight(i,int(QFontMetrics(qApp->font()).height()*1.3));
    }
    ReconstruitListeLieuxExerciceAllusers();
    /*-------------------- GESTION DES LIEUX D'EXRCICE-------------------------------------------------------*/
}

void dlg_param::ReconstruitListeLieuxExerciceAllusers()
{
    disconnect(ui->EmplacementServeurupComboBox,       SIGNAL(currentIndexChanged(int)),       this,   SLOT(Slot_EnregistreEmplacementServeur(int)));
    QString req ="select idLieu, NomLieu, LieuVille from " TBL_LIEUXEXERCICE;
    bool ok;
    QList<QVariantList> servlist = db->StandardSelectSQL(req, ok);
    if (ok && servlist.size()>0)
    {
        for (int i=0; i<servlist.size(); ++i)
            ui->EmplacementServeurupComboBox->addItem(servlist.at(i).at(1).toString() + " " + servlist.at(i).at(2).toString(), servlist.at(i).at(0));
        if (m_parametres->idlieupardefaut()>0)
            ui->EmplacementServeurupComboBox->setCurrentIndex(ui->EmplacementServeurupComboBox->findData(m_parametres->idlieupardefaut()));
        else
        {
            ui->EmplacementServeurupComboBox->setCurrentIndex(0);
            Slot_EnregistreEmplacementServeur(0);
        }
    }
    connect(ui->EmplacementServeurupComboBox,       SIGNAL(currentIndexChanged(int)),       this,   SLOT(Slot_EnregistreEmplacementServeur(int)));
}

void dlg_param::NouvAppareil()
{
    dlg_askappareil = new UpDialog();
    dlg_askappareil->setModal(true);
    dlg_askappareil->move(QPoint(x()+width()/2,y()+height()/2));
    dlg_askappareil->setFixedWidth(400);
    dlg_askappareil->setWindowTitle(tr("Choisissez un appareil"));
    QHBoxLayout *lay = new QHBoxLayout;
    UpLabel *label = new UpLabel();
    label->setText("Nom de l'appareil");
    label->setFixedSize(120,25);
    lay->addWidget(label);
    lay->addSpacerItem(new QSpacerItem(10,10,QSizePolicy::Expanding));
    UpComboBox *upCombo = new UpComboBox();
    upCombo->insertItems(0,m_listeappareils);
    upCombo->setFixedSize(260,32);
    upCombo->setchamp("NomAppareil");
    upCombo->showPopup();
    lay->addWidget(upCombo);
    dlg_askappareil->dlglayout()->insertLayout(0,lay);
    dlg_askappareil->dlglayout()->setSizeConstraint(QLayout::SetFixedSize);
    dlg_askappareil->AjouteLayButtons(UpDialog::ButtonOK);
    connect(dlg_askappareil->OKButton,    SIGNAL(clicked(bool)), this, SLOT(Slot_EnregistreAppareil()));
    dlg_askappareil->exec();
    delete dlg_askappareil;
}

void dlg_param::startImmediateBackup()
{
    if (proc->AutresPostesConnectes())
        return;
    QString dirsauvorigin   = ui->DirBackupuplineEdit->text();
    QString dirSauv         = QFileDialog::getExistingDirectory(this,
                                                                tr("Choisissez le dossier dans lequel vous voulez sauvegarder la base\n"
                                                                   "Le nom de dossier ne doit pas contenir d'espace"),
                                                                (QDir(dirsauvorigin).exists()? dirsauvorigin : QDir::homePath()),
                                                                QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (dirSauv.contains(" "))
    {
        UpMessageBox::Watch(this, tr("Nom de dossier non conforme"),tr("Vous ne pouvez pas choisir un dossier dont le nom contient des espaces"));
        return;
    }
    if (dirSauv != "")
        proc->ImmediateBackup(dirSauv, false);
}

void dlg_param::Slot_MAJActesCCAM(QString txt)
{
    QString req;
    UpCheckBox* check = dynamic_cast<UpCheckBox*>(sender());
    if (check)
    {
        int row = check->rowTable();
        QString codeccam = ui->ActesCCAMupTableWidget->item(row,1)->text();
        QString montantpratique="";
        if (check->checkState() == Qt::Unchecked)
        {
            req = "delete from " TBL_COTATIONS " where typeacte = '" + codeccam + "' and idUser = " + QString::number(m_currentuser->id());
        }
        else
        {
            int secteur = m_currentuser->secteurconventionnel();
            if (secteur>1)
            {
                UpLineEdit *line = dynamic_cast<UpLineEdit*>(ui->ActesCCAMupTableWidget->cellWidget(row,5));
                if (line)
                    montantpratique = QString::number(QLocale().toDouble(line->text()));
                else
                {
                    UpLineEdit *lbl = new UpLineEdit();
                    if (m_currentuser->isOPTAM())
                        lbl->setText(ui->ActesCCAMupTableWidget->item(row,2)->text());
                    else
                        lbl->setText(ui->ActesCCAMupTableWidget->item(row,3)->text());
                    lbl->setAlignment(Qt::AlignRight);
                    lbl->setStyleSheet("QLineEdit {border: 0px solid rgb(150,150,150);}"
                                       "QLineEdit:disabled {background-color:lightGray;}");
                    lbl->setRow(row);
                    QDoubleValidator *val = new QDoubleValidator(this);
                    val->setDecimals(2);
                    lbl->setValidator(val);
                    connect(lbl,    SIGNAL(TextModified(QString)),  this,   SLOT(Slot_MAJActesCCAM(QString)));
                    ui->ActesCCAMupTableWidget->setCellWidget(row,5,lbl);
                    montantpratique = QString::number(QLocale().toDouble(lbl->text()));
                }
             }
            else if (m_currentuser->isOPTAM())
                montantpratique = QString::number(QLocale().toDouble(ui->ActesCCAMupTableWidget->item(row,2)->text()));
            else
                montantpratique = QString::number(QLocale().toDouble(ui->ActesCCAMupTableWidget->item(row,3)->text()));

            req = "insert into " TBL_COTATIONS " (typeacte, MontantOPTAM, MontantNonOPTAM, montantpratique, CCAM, iduser) values ('" +
                    codeccam + "', " +
                    QString::number(QLocale().toDouble(ui->ActesCCAMupTableWidget->item(row,2)->text())) + ", " +
                    QString::number(QLocale().toDouble(ui->ActesCCAMupTableWidget->item(row,3)->text())) + ", " +
                    montantpratique + ", 1," +
                    QString::number(m_currentuser->id()) +")";
        }
        if (db->StandardSQL(req))
            m_cotationsmodifiees = true;
    }
    else
    {
        UpLineEdit *line = dynamic_cast<UpLineEdit*>(sender());
        if (line)
        {
            QString montant = QString::number(QLocale().toDouble(txt));
            line->setText(QLocale().toString(montant.toDouble(),'f',2));
            int row = line->Row();
            UpCheckBox* check1 = dynamic_cast<UpCheckBox*>(ui->ActesCCAMupTableWidget->cellWidget(row,0));
            if (check1)
                if (check1->isChecked())
                {
                    line->setText(QLocale().toString(montant.toDouble(),'f',2));
                    QString req = "update " TBL_COTATIONS " set montantpratique = " + montant +
                                  " where typeacte = '" + ui->ActesCCAMupTableWidget->item(row,1)->text() + "' and idUser = " + QString::number(m_currentuser->id());
                    if (db->StandardSQL(req))
                        m_cotationsmodifiees = true;
                }
        }
    }
}

void dlg_param::Slot_MAJAssocCCAM(QString txt)
{
    bool ok;
    QString req;
    UpCheckBox* check = dynamic_cast<UpCheckBox*>(sender());
    if (check)
    {
        int row                 = check->rowTable();
        QString codeccam        = ui->AssocCCAMupTableWidget->item(row,1)->text();
        QString montantpratique = "";
        if (check->checkState() == Qt::Unchecked)
        {
            QList<QVariantList> calclist = db->StandardSelectSQL("select typeacte from " TBL_COTATIONS " where typeacte = '" + codeccam + "'", ok);
            req = "delete from " TBL_COTATIONS " where typeacte = '" + codeccam + "' and idUser = " + QString::number(m_currentuser->id());
            if (calclist.size()==1)
            {
                if (UpMessageBox::Question(this,tr("Suppression de cotation"),
                                            tr("Vous étiez le seul à utiliser cette cotation") + "\n" +
                                            tr("Voulez-vous la supprimer définitvement?"),
                                            UpDialog::ButtonCancel | UpDialog::ButtonSuppr,
                                            QStringList() << tr("Annuler") << tr("Supprimer la cotation") + " " + codeccam)
                    != UpSmallButton::SUPPRBUTTON)
                    req = "update " TBL_COTATIONS " set idUser = NULL where typeacte = '" + codeccam + "' and idUser = " + QString::number(m_currentuser->id());
            }
        }
        else
        {
            int secteur = m_currentuser->secteurconventionnel();
            QString montantOPTAM(""), montantNonOPTAM("");
            UpLineEdit *lineOPTAM = dynamic_cast<UpLineEdit*>(ui->AssocCCAMupTableWidget->cellWidget(row,2));
            if (lineOPTAM != Q_NULLPTR)
                montantOPTAM    = QString::number(QLocale().toDouble(lineOPTAM->text()));
            UpLineEdit *lineNonOPTAM = dynamic_cast<UpLineEdit*>(ui->AssocCCAMupTableWidget->cellWidget(row,3));
            if (lineNonOPTAM != Q_NULLPTR)
                montantNonOPTAM    = QString::number(QLocale().toDouble(lineNonOPTAM->text()));
            if (secteur>1)
            {
                UpLineEdit *lineprat = dynamic_cast<UpLineEdit*>(ui->AssocCCAMupTableWidget->cellWidget(row,4));
                if (lineprat)
                    montantpratique = QString::number(QLocale().toDouble(lineprat->text()));
                else
                {
                    UpLineEdit *lbl = new UpLineEdit();
                    if (m_currentuser->isOPTAM())
                        lbl->setText(ui->AssocCCAMupTableWidget->item(row,2)->text());
                    else
                        lbl->setText(ui->AssocCCAMupTableWidget->item(row,3)->text());
                    lbl->setAlignment(Qt::AlignRight);
                    lbl->setStyleSheet("QLineEdit {border: 0px solid rgb(150,150,150);}"
                                       "QLineEdit:disabled {background-color:lightGray;}");
                    lbl->setRow(row);
                    QDoubleValidator *val = new QDoubleValidator(this);
                    val->setDecimals(2);
                    lbl->setValidator(val);
                    connect(lbl,    SIGNAL(TextModified(QString)),  this,   SLOT(Slot_MAJAssocCCAM(QString)));
                    ui->AssocCCAMupTableWidget->setCellWidget(row,4,lbl);
                    montantpratique = QString::number(QLocale().toDouble(lbl->text()));
                }
            }
            else if (m_currentuser->isOPTAM())
                montantpratique = montantOPTAM;
            else
                montantpratique = montantNonOPTAM;
            req = "insert into " TBL_COTATIONS " (typeacte, MontantOPTAM, MontantNonOPTAM, montantpratique, CCAM, iduser) values ('" +
                    codeccam + "', " +
                    montantOPTAM + ", " +
                    montantNonOPTAM + ", " +
                    montantpratique + ", 2," +
                    QString::number(m_currentuser->id()) +")";
        }
        if (db->StandardSQL(req))
            m_cotationsmodifiees = true;
    }
    else
    {
        UpLineEdit *line = dynamic_cast<UpLineEdit*>(sender());
        if (line)
        {
            int row = line->Row();
            UpCheckBox* check1 = dynamic_cast<UpCheckBox*>(ui->AssocCCAMupTableWidget->cellWidget(row,0));
            if (check1)
                if (check1->isChecked())
                {
                    QString req;
                    QString montant = QString::number(QLocale().toDouble(txt));
                    line->setText(QLocale().toString(montant.toDouble(),'f',2));
                    if (line->Column()==2)
                        req = "update " TBL_COTATIONS " set montantoptam = " + montant +
                            " where typeacte = '" + ui->AssocCCAMupTableWidget->item(row,1)->text() + "' and idUser = " + QString::number(m_currentuser->id());
                    else if (line->Column()==3)
                       req = "update " TBL_COTATIONS " set montantnonoptam = " + montant +
                           " where typeacte = '" + ui->AssocCCAMupTableWidget->item(row,1)->text() + "' and idUser = " + QString::number(m_currentuser->id());
                    else if (line->Column()==4)
                       req = "update " TBL_COTATIONS " set montantpratique = " + montant +
                           " where typeacte = '" + ui->AssocCCAMupTableWidget->item(row,1)->text() + "' and idUser = " + QString::number(m_currentuser->id());
                    if (db->StandardSQL(req))
                        m_cotationsmodifiees = true;
                }
        }
    }
}

void dlg_param::Slot_MAJHorsNomenclature(QString txt)
{
    QString req;
    UpCheckBox* check = dynamic_cast<UpCheckBox*>(sender());
    if (check)
    {
        int row                 = check->rowTable();
        QString codeccam        = ui->HorsNomenclatureupTableWidget->item(row,1)->text();
        QString montantpratique = "";
        if (check->checkState() == Qt::Unchecked)
            req = "delete from " TBL_COTATIONS " where typeacte = '" + codeccam + "' and idUser = " + QString::number(m_currentuser->id());
        else
        {
            UpLineEdit *lineprat = dynamic_cast<UpLineEdit*>(ui->HorsNomenclatureupTableWidget->cellWidget(row,2));
            if (lineprat)
                montantpratique = QString::number(QLocale().toDouble(lineprat->text()));
            req = "insert into " TBL_COTATIONS " (typeacte, MontantOPTAM, MontantNonOPTAM, montantpratique, CCAM, iduser) values ('" +
                    codeccam + "', " +
                    montantpratique + ", " +
                    montantpratique + ", " +
                    montantpratique + ", " +
                    " 2, " + QString::number(m_currentuser->id()) +")";
        }
        if (db->StandardSQL(req))
            m_cotationsmodifiees = true;
    }
    else
    {
        UpLineEdit *line = dynamic_cast<UpLineEdit*>(sender());
        if (line)
        {
            int row = line->Row();
            UpCheckBox* check1 = dynamic_cast<UpCheckBox*>(ui->HorsNomenclatureupTableWidget->cellWidget(row,0));
            if (check1)
                if (check1->isChecked())
                {
                    QString req;
                    QString montant = QString::number(QLocale().toDouble(txt));
                    line->setText(QLocale().toString(montant.toDouble(),'f',2));
                    req = "update " TBL_COTATIONS " set montantOPTAM = " + montant + ", montantNonOPTAM = " + montant + ", montantpratique = " + montant +
                          " where typeacte = '" + ui->HorsNomenclatureupTableWidget->item(row,1)->text() + "' and idUser = " + QString::number(m_currentuser->id());
                    if (db->StandardSQL(req))
                        m_cotationsmodifiees = true;
                }
        }
    }
}

void dlg_param::SupprAppareil()
{
    if (ui->AppareilsConnectesupTableWidget->selectedItems().size()==0)
        return;
    bool ok;
    QString req = " select list.TitreExamen, list.NomAppareil from " TBL_LISTEAPPAREILS " list, " TBL_APPAREILSCONNECTESCENTRE " appcon"
                  " where list.idAppareil = appcon.idappareil"
                  " and list.idappareil = " + ui->AppareilsConnectesupTableWidget->selectedItems().at(0)->text();
    QVariantList appdata = db->getFirstRecordFromStandardSelectSQL(req, ok);
    if (!ok || appdata.size()==0)
        return;
    UpMessageBox msgbox;
    msgbox.setText(tr("Suppression d'un appareil!"));
    msgbox.setInformativeText(tr("Voulez vous vraiment supprimer l'appareil") + "\n"
                              + appdata.at(1).toString() + " ?");
    msgbox.setIcon(UpMessageBox::Warning);
    UpSmallButton OKBouton(tr("Supprimer"));
    UpSmallButton NoBouton(tr("Annuler"));
    msgbox.addButton(&NoBouton, UpSmallButton::CANCELBUTTON);
    msgbox.addButton(&OKBouton, UpSmallButton::SUPPRBUTTON);
    msgbox.exec();
    if (msgbox.clickedButton() == &OKBouton)
    {
        req = "delete from " TBL_APPAREILSCONNECTESCENTRE " where idAppareil = "
              + ui->AppareilsConnectesupTableWidget->selectedItems().at(0)->text()
              + " and idLieu = " + QString::number(m_currentuser->idsitedetravail());
        db->StandardSQL(req);
        QString Base;
        if (db->getMode() == DataBase::Poste)
            Base = "BDD_POSTE";
        else if (db->getMode() == DataBase::ReseauLocal)
            Base = "BDD_LOCAL";
        else if (db->getMode() == DataBase::Distant)
            Base = "BDD_DISTANT";
        proc->settings()->remove(Base + "/DossiersDocuments/" + appdata.at(1).toString());
        Remplir_Tables();
    }
}

void dlg_param::Slot_RegleAssocBoutons()
{
    bool modifboutonsActes  = false;
    bool modifboutonsAssoc  = false;
    bool modifboutonsHN     = false;


    UpCheckBox* check0      = dynamic_cast<UpCheckBox*>(sender());
    if (check0 != Q_NULLPTR)
    {
        if (ui->ActesCCAMupTableWidget->isAncestorOf(check0))
            modifboutonsActes = true;
        else if (ui->AssocCCAMupTableWidget->isAncestorOf(check0))
        {
            if (ui->AssocCCAMupTableWidget->selectedRanges().size()>0)
            {
                if (check0->rowTable()==ui->AssocCCAMupTableWidget->selectedRanges().at(0).topRow())
                    modifboutonsAssoc = true;
            }
            if (!modifboutonsAssoc)
            {
                ui->AssocCCAMupTableWidget          ->clearSelection();
                ui->HorsNomenclatureupTableWidget   ->clearSelection();
                wdg_assocCCAMcotationswdgbuttonframe->wdg_modifBouton          ->setEnabled(false);
                wdg_assocCCAMcotationswdgbuttonframe->wdg_moinsBouton          ->setEnabled(false);
                wdg_HNcotationswdgbuttonframe->wdg_modifBouton                 ->setEnabled(false);
                wdg_HNcotationswdgbuttonframe->wdg_moinsBouton                 ->setEnabled(false);
            }
        }
        else if (ui->HorsNomenclatureupTableWidget->isAncestorOf(check0))
        {
            if (ui->HorsNomenclatureupTableWidget->selectedRanges().size()>0)
            {
                if (check0->rowTable()==ui->HorsNomenclatureupTableWidget->selectedRanges().at(0).topRow())
                    modifboutonsHN = true;
            }
            if (!modifboutonsHN)
            {
                ui->AssocCCAMupTableWidget          ->clearSelection();
                ui->HorsNomenclatureupTableWidget   ->clearSelection();
                wdg_assocCCAMcotationswdgbuttonframe->wdg_modifBouton          ->setEnabled(false);
                wdg_assocCCAMcotationswdgbuttonframe->wdg_moinsBouton          ->setEnabled(false);
                wdg_HNcotationswdgbuttonframe->wdg_modifBouton                 ->setEnabled(false);
                wdg_HNcotationswdgbuttonframe->wdg_moinsBouton                 ->setEnabled(false);
            }
        }
    }

    if (sender() == ui->AssocCCAMupTableWidget || modifboutonsAssoc)
    {
        ui->ActesCCAMupTableWidget          ->clearSelection();
        ui->HorsNomenclatureupTableWidget   ->clearSelection();
        bool checked = true;
        if (ui->AssocCCAMupTableWidget->selectedRanges().size()>0)
        {
            UpCheckBox* check                   = static_cast<UpCheckBox*>(ui->AssocCCAMupTableWidget->cellWidget(ui->AssocCCAMupTableWidget->selectedRanges().at(0).topRow(),0));
            checked = check->isChecked();
        }
        wdg_assocCCAMcotationswdgbuttonframe->wdg_modifBouton          ->setEnabled((ui->AssocCCAMupTableWidget->selectedRanges().size()>0
                                                 || ui->ActesCCAMupTableWidget->selectedRanges().size()>0)
                                                 && checked);
        wdg_assocCCAMcotationswdgbuttonframe->wdg_moinsBouton          ->setEnabled((ui->AssocCCAMupTableWidget->selectedRanges().size()>0
                                                 || ui->ActesCCAMupTableWidget->selectedRanges().size()>0)
                                                 && checked);
        wdg_HNcotationswdgbuttonframe->wdg_modifBouton                 ->setEnabled(false);
        wdg_HNcotationswdgbuttonframe->wdg_moinsBouton                 ->setEnabled(false);
    }
    else if (sender() == ui->ActesCCAMupTableWidget || modifboutonsActes)
    {
        ui->AssocCCAMupTableWidget          ->clearSelection();
        ui->HorsNomenclatureupTableWidget   ->clearSelection();
        wdg_assocCCAMcotationswdgbuttonframe->wdg_modifBouton          ->setEnabled(false);
        wdg_assocCCAMcotationswdgbuttonframe->wdg_moinsBouton          ->setEnabled(false);
        wdg_HNcotationswdgbuttonframe->wdg_modifBouton                 ->setEnabled(false);
        wdg_HNcotationswdgbuttonframe->wdg_moinsBouton                 ->setEnabled(false);
    }
    else if (sender() == ui->HorsNomenclatureupTableWidget || modifboutonsHN)
    {
        ui->ActesCCAMupTableWidget          ->clearSelection();
        ui->AssocCCAMupTableWidget          ->clearSelection();
        bool checked = true;
        if (ui->HorsNomenclatureupTableWidget->isAncestorOf(check0))
        {
            UpCheckBox* check                   = static_cast<UpCheckBox*>(ui->HorsNomenclatureupTableWidget->cellWidget(ui->HorsNomenclatureupTableWidget->selectedRanges().at(0).topRow(),0));
            checked = check->isChecked();
        }
        wdg_assocCCAMcotationswdgbuttonframe->wdg_modifBouton          ->setEnabled(false);
        wdg_assocCCAMcotationswdgbuttonframe->wdg_moinsBouton          ->setEnabled(false);
        wdg_HNcotationswdgbuttonframe->wdg_modifBouton                 ->setEnabled(ui->HorsNomenclatureupTableWidget->selectedRanges().size()>0
                                                         && checked);
        wdg_HNcotationswdgbuttonframe->wdg_moinsBouton                 ->setEnabled(ui->HorsNomenclatureupTableWidget->selectedRanges().size()>0
                                                         && checked);
    }
}

void dlg_param::Slot_ResetImprimante()
{
    UpMessageBox msgbox;
    msgbox.setText(tr("Reset des paramètres imprimante!"));
    msgbox.setInformativeText(tr("Restaurer les valeurs par défaut?"));
    msgbox.setIcon(UpMessageBox::Warning);
    UpSmallButton OKBouton(tr("Restaurer"));
    UpSmallButton NoBouton(tr("Annuler"));
    msgbox.addButton(&NoBouton, UpSmallButton::CANCELBUTTON);
    msgbox.addButton(&OKBouton, UpSmallButton::STARTBUTTON);
    msgbox.exec();
    if (msgbox.clickedButton() != &OKBouton) return;
    ui->EntetespinBox->setValue(45);
    ui->EnteteALDspinBox->setValue(63);
    ui->PiedDePagespinBox->setValue(20);
    ui->PiedDePageOrdoLunettesspinBox->setValue(40);
    ui->TopMargespinBox->setValue(3);
    Slot_EnableOKModifPosteButton();
}

void dlg_param::Slot_EnregistreAppareil()
{
    if (!dlg_askappareil) return;
    QString req = "insert into " TBL_APPAREILSCONNECTESCENTRE " (idAppareil, idLieu) Values("
                  " (select idappareil from " TBL_LISTEAPPAREILS " where NomAppareil = '" + dlg_askappareil->findChildren<UpComboBox*>().at(0)->currentText() + "'), "
                  + QString::number(m_currentuser->idsitedetravail()) + ")";
    db->StandardSQL(req);
    dlg_askappareil->done(0);
    Remplir_Tables();
}

void dlg_param::Slot_EnregistreEmplacementServeur(int idx)
{
    if (ui->EmplacementServeurupComboBox->itemData(idx).toString() != "")
        db->setidlieupardefaut(ui->EmplacementServeurupComboBox->itemData(idx).toInt());
}

void dlg_param::NouvAssocCCAM()
{
    Dlg_CrrCot = new dlg_gestioncotations(dlg_gestioncotations::Association, dlg_gestioncotations::Creation);
    if (Dlg_CrrCot->exec()>0)
    {
        Remplir_TableAssocCCAM();
        EnableAssocCCAM();
        m_cotationsmodifiees = true;
    }
    delete Dlg_CrrCot;
}

void dlg_param::ModifAssocCCAM()
{
    QString CodeActe = "";
    if (ui->AssocCCAMupTableWidget->selectedRanges().size()==0)
        return;
    int row = ui->AssocCCAMupTableWidget->selectedRanges().at(0).topRow();
    CodeActe = ui->AssocCCAMupTableWidget->item(row,1)->text();
    Dlg_CrrCot = new dlg_gestioncotations(dlg_gestioncotations::Association, dlg_gestioncotations::Modification, CodeActe);
    if (Dlg_CrrCot->exec()>0)
    {
        Remplir_TableAssocCCAM();
        EnableAssocCCAM();
        m_cotationsmodifiees = true;
    }
    delete Dlg_CrrCot;
}

void dlg_param::SupprAssocCCAM()
{
    int row = ui->AssocCCAMupTableWidget->selectedRanges().at(0).topRow();
    QString CodeActe = ui->AssocCCAMupTableWidget->item(row,1)->text();
    bool ok;
    QString req = "select typeacte from " TBL_COTATIONS
                  " where typeacte = '" + CodeActe + "'"
                  " and iduser <> NULL"
                  " and iduser <> " + QString::number(m_currentuser->id());
    QList<QVariantList> typactlist = db->StandardSelectSQL(req,ok);
    if (!ok)
        return;
    if (typactlist.size()>0)
    {
        UpMessageBox::Watch(this, tr("Suppression refusée"),
                            tr("La cotation") + " " + CodeActe + " " +
                            tr("est utilisée par d'autres utilisateurs"));
        return;
    }

    if (UpMessageBox::Question(this, tr("Suppression de cotation"), tr("Confirmez la suppression de la cotation ") + CodeActe)==UpSmallButton::STARTBUTTON)
    {
        db->StandardSQL("delete from " TBL_COTATIONS " where typeacte = '" + CodeActe + "'");
        Remplir_TableAssocCCAM();
        EnableAssocCCAM();
        m_cotationsmodifiees = true;
    }
}

void dlg_param::NouvHorsNomenclature()
{
    Dlg_CrrCot = new dlg_gestioncotations(dlg_gestioncotations::HorsNomenclature, dlg_gestioncotations::Creation);
    if (Dlg_CrrCot->exec()>0)
    {
        Remplir_TableHorsNomenclature();
        EnableHorsNomenclature();
        m_cotationsmodifiees = true;
    }
    delete Dlg_CrrCot;
}

void dlg_param::ModifHorsNomenclature()
{
    int row = ui->HorsNomenclatureupTableWidget->selectedRanges().at(0).topRow();
    QString CodeActe = ui->HorsNomenclatureupTableWidget->item(row,1)->text();
    Dlg_CrrCot = new dlg_gestioncotations(dlg_gestioncotations::HorsNomenclature, dlg_gestioncotations::Modification, CodeActe);
    if (Dlg_CrrCot->exec()>0)
    {
        Remplir_TableHorsNomenclature();
        EnableHorsNomenclature();
        m_cotationsmodifiees = true;
    }
    delete Dlg_CrrCot;
}

void dlg_param::SupprHorsNomenclature()
{
    int row = ui->HorsNomenclatureupTableWidget->selectedRanges().at(0).topRow();
    QString CodeActe = ui->HorsNomenclatureupTableWidget->item(row,1)->text();
    if (UpMessageBox::Question(this, tr("Suppression de cotation"), tr("Confirmez la suppression de la cotation ") + CodeActe)==UpSmallButton::STARTBUTTON)
    {
        db->StandardSQL("delete from " TBL_COTATIONS " where typeacte = '" + CodeActe + "'");
        Remplir_TableHorsNomenclature();
        EnableHorsNomenclature();
        m_cotationsmodifiees = true;
    }
}

void dlg_param::Slot_ModifMDPAdmin()
{
    dlg_askMDP    = new UpDialog(this);
    dlg_askMDP    ->setModal(true);
    dlg_askMDP    ->move(QPoint(x()+width()/2,y()+height()/2));

    UpLineEdit *ConfirmMDP = new UpLineEdit(dlg_askMDP);
    ConfirmMDP->setEchoMode(QLineEdit::Password);
    ConfirmMDP->setObjectName(m_confirmeMDP);
    ConfirmMDP->setValidator(new QRegExpValidator(Utils::rgx_AlphaNumeric_5_12,this));
    ConfirmMDP->setAlignment(Qt::AlignCenter);
    ConfirmMDP->setMaxLength(12);
    dlg_askMDP->dlglayout()->insertWidget(0,ConfirmMDP);
    UpLabel *labelConfirmMDP = new UpLabel();
    labelConfirmMDP->setText(tr("Confirmez le nouveau mot de passe"));
    dlg_askMDP->dlglayout()->insertWidget(0,labelConfirmMDP);
    UpLineEdit *NouvMDP = new UpLineEdit(dlg_askMDP);
    NouvMDP->setEchoMode(QLineEdit::Password);
    NouvMDP->setObjectName(m_nouveauMDP);
    NouvMDP->setValidator(new QRegExpValidator(Utils::rgx_AlphaNumeric_5_12,this));
    NouvMDP->setAlignment(Qt::AlignCenter);
    NouvMDP->setMaxLength(12);
    dlg_askMDP->dlglayout()->insertWidget(0,NouvMDP);
    UpLabel *labelNewMDP = new UpLabel();
    labelNewMDP->setText(tr("Entrez le nouveau mot de passe"));
    dlg_askMDP->dlglayout()->insertWidget(0,labelNewMDP);
    UpLineEdit *AncMDP = new UpLineEdit(dlg_askMDP);
    AncMDP->setEchoMode(QLineEdit::Password);
    AncMDP->setAlignment(Qt::AlignCenter);
    AncMDP->setValidator(new QRegExpValidator(Utils::rgx_AlphaNumeric_3_12,this));
    AncMDP->setObjectName(m_ancienMDP);
    AncMDP->setMaxLength(12);
    dlg_askMDP->dlglayout()->insertWidget(0,AncMDP);
    UpLabel *labelOldMDP = new UpLabel();
    labelOldMDP->setText(tr("Entrez votre mot de passe"));
    dlg_askMDP->dlglayout()->insertWidget(0,labelOldMDP);
    AncMDP->setFocus();

    dlg_askMDP->AjouteLayButtons(UpDialog::ButtonOK);
    QList <QWidget*> ListTab;
    ListTab << AncMDP << NouvMDP << ConfirmMDP << dlg_askMDP->OKButton;
    for (int i = 0; i<ListTab.size()-1 ; i++ )
        dlg_askMDP->setTabOrder(ListTab.at(i), ListTab.at(i+1));
    dlg_askMDP    ->setWindowTitle(tr("Mot de passe administrateur"));
    connect(dlg_askMDP->OKButton,    SIGNAL(clicked(bool)), this, SLOT(Slot_EnregistreNouvMDPAdmin()));
    dlg_askMDP->dlglayout()->setSizeConstraint(QLayout::SetFixedSize);

    dlg_askMDP->exec();
}

void dlg_param::Slot_ParamMotifs()
{
    Dlg_motifs = new dlg_motifs();
    Dlg_motifs->setWindowTitle(tr("Motifs de consultations"));
    Dlg_motifs->exec();
    delete Dlg_motifs;
}

void dlg_param::ModifDirBackup()
{
    if (db->getMode() != DataBase::Poste)
        return;
    QString dirsauvorigin   = ui->DirBackupuplineEdit->text();
    QString dirSauv         = QFileDialog::getExistingDirectory(this,tr("Choisissez le dossier dans lequel vous voulez sauvegarder la base\n"
                                                                "Le nom de dossier ne doit pas contenir d'espace"), QDir::homePath());
    if (dirSauv.contains(" "))
        UpMessageBox::Watch(this, tr("Nom de dossier non conforme"),tr("Vous ne pouvez pas choisir un dossier dont le nom contient des espaces"));
    if (dirsauvorigin == dirSauv || dirSauv.contains(" "))
        return;

    ui->DirBackupuplineEdit ->setText(dirSauv);
    db->setdirbkup(dirSauv);
    proc->ParamAutoBackup();
    ui->EffacePrgSauvupPushButton->setEnabled(m_parametres->daysbkup()
                                           && QDir(m_parametres->dirbkup()).exists()
                                           && m_parametres->dirbkup() != ""
                                           && m_parametres->heurebkup() != QTime());
}

void dlg_param::ModifDateHeureBackup()    //Modification de la date ou de l'heure et date du backup
{
    db->setheurebkup(ui->HeureBackuptimeEdit->time());
    db->setlundibkup(ui->LundiradioButton->isChecked());
    db->setmardibkup(ui->MardiradioButton->isChecked());
    db->setmercredibkup(ui->MercrediradioButton->isChecked());
    db->setjeudibkup(ui->JeudiradioButton->isChecked());
    db->setvendredibkup(ui->VendrediradioButton->isChecked());
    db->setsamedibkup(ui->SamediradioButton->isChecked());
    db->setdimanchebkup(ui->DimancheradioButton->isChecked());
    proc->ParamAutoBackup();
    ui->EffacePrgSauvupPushButton->setEnabled(m_parametres->daysbkup()
                                           && QDir(m_parametres->dirbkup()).exists()
                                           && m_parametres->dirbkup() != ""
                                           && m_parametres->heurebkup() != QTime());
}

void dlg_param::Slot_DirLocalStockage()
{
    QString dir = proc->settings()->value("BDD_LOCAL/DossierImagerie").toString();
    if (dir == "")
        dir = QDir::homePath() + DIR_RUFUS;
    QFileDialog dialog(this, "", dir);
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setViewMode(QFileDialog::List);
    if (dialog.exec()>0)
    {
        QDir dockdir = dialog.directory();
        ui->LocalStockageupLineEdit->setText(dockdir.path());
        proc->settings()->setValue("BDD_LOCAL/DossierImagerie", dockdir.path());
    }
}

void dlg_param::Slot_DirDistantStockage()
{
    QString dir = proc->settings()->value("BDD_DISTANT/DossierImagerie").toString();
    if (dir == "")
        dir = QDir::homePath() + DIR_RUFUS;
    QFileDialog dialog(this, "", dir);
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setViewMode(QFileDialog::List);
    if (dialog.exec()>0)
    {
        QDir dockdir = dialog.directory();
        ui->DistantStockageupLineEdit->setText(dockdir.path());
        proc->settings()->setValue("BDD_DISTANT/DossierImagerie", dockdir.path());
    }
}

void dlg_param::Slot_DirPosteStockage()
{
    if (db->getMode() != DataBase::Poste)
    {
        UpMessageBox::Watch(this, tr("Impossible de modifier ce paramètre"), tr("Vous devez être connecté sur le serveur de ce poste pour\npouvoir modifier le répertoire de stockage des documents"));
        return;
    }
    QString dir = ui->PosteStockageupLineEdit->text();
    if (dir == "")
        dir = QDir::homePath() + DIR_RUFUS;
    QFileDialog dialog(this, "", dir);
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setViewMode(QFileDialog::List);
    if (dialog.exec()>0)
    {
        QDir dockdir = dialog.directory();
        if (!dockdir.match(QDir::homePath() + DIR_RUFUS "/*", dockdir.path()))
        {
            UpMessageBox::Watch(this, tr("Vous devez choisir un sous-dossier du dossier Rufus"), QDir::homePath() + DIR_RUFUS);
            return;
        }
        ui->PosteStockageupLineEdit->setText(dockdir.path());
        db->setdirimagerie(dockdir.path());
        proc->setDirImagerie();
    }
}

void dlg_param::Slot_EffacePrgSauvegarde()
{
    QList<QRadioButton*> listbutton2 = ui->JourSauvegardeframe->findChildren<QRadioButton*>();
    for (int i=0; i<listbutton2.size(); i++)
       listbutton2.at(i)->setChecked(false);
    ui->DirBackupuplineEdit->setText("");
    ui->HeureBackuptimeEdit->setTime(QTime(0,0));
    proc->EffaceBDDDataBackup();
    ui->EffacePrgSauvupPushButton->setEnabled(false);
}

bool dlg_param::VerifDirStockageImagerie()
{
    if (ui->NonImportDocscheckBox->isChecked())
        return true;
    if (ui->PosteServcheckBox->isChecked() && db->getMode() == DataBase::Poste)
    {
        bool DirStockageAVerifier = false;
        if (ui->MonoDocupTableWidget->rowCount()>0)
        {
            for (int i=0; i<ui->MonoDocupTableWidget->rowCount(); i++)
            {
                UpLineEdit *line = dynamic_cast<UpLineEdit*>(ui->MonoDocupTableWidget->cellWidget(i,2));
                if (line !=  Q_NULLPTR)
                    if (line->text() != "")
                    {
                        DirStockageAVerifier = true;
                        break;
                    }
            }
        }
        if (DirStockageAVerifier)
        {
            if (!QDir(ui->PosteStockageupLineEdit->text()).exists() || ui->PosteStockageupLineEdit->text() == "")
            {
                UpMessageBox::Watch(this,tr("Vous n'avez pas spécifié de dossier de stockage valide pour les documents d'imagerie !"));
                ui->ParamtabWidget->setCurrentWidget(ui->PosteParamtab);
                ui->ParamConnexiontabWidget->setCurrentWidget(ui->tabMono);
                ui->PosteStockageupLineEdit->setFocus();
                return false;
            }
        }
    }
    if (ui->LocalServcheckBox->isChecked() && db->getMode() == DataBase::ReseauLocal)
    {
        bool DirStockageAVerifier = false;
        if (ui->LocalDocupTableWidget->rowCount()>0)
        {
            for (int i=0; i<ui->LocalDocupTableWidget->rowCount(); i++)
            {
                UpLineEdit *line = dynamic_cast<UpLineEdit*>(ui->LocalDocupTableWidget->cellWidget(i,2));
                if (line !=  Q_NULLPTR)
                    if (line->text() != "")
                    {
                        DirStockageAVerifier = true;
                        break;
                    }
            }
        }
        if (DirStockageAVerifier)
        {
            if (!QDir(ui->LocalStockageupLineEdit->text()).exists() || ui->LocalStockageupLineEdit->text() == "")
            {
                UpMessageBox::Watch(this,tr("Vous n'avez pas spécifié de dossier de stockage valide pour les documents d'imagerie !"));
                ui->ParamtabWidget->setCurrentWidget(ui->PosteParamtab);
                ui->ParamConnexiontabWidget->setCurrentWidget(ui->tabLocal);
                ui->LocalStockageupLineEdit->setFocus();
                return false;
            }
        }
    }
    if (ui->DistantServcheckBox->isChecked() && db->getMode() == DataBase::Distant)
    {
        bool DirStockageAVerifier = false;
        if (ui->DistantDocupTableWidget->rowCount()>0)
        {
            for (int i=0; i<ui->DistantDocupTableWidget->rowCount(); i++)
            {
                UpLineEdit *line = dynamic_cast<UpLineEdit*>(ui->DistantDocupTableWidget->cellWidget(i,2));
                if (line !=  Q_NULLPTR)
                    if (line->text() != "")
                    {
                        DirStockageAVerifier = true;
                        break;
                    }
            }
        }
        if (DirStockageAVerifier)
        {
            if (!QDir(ui->DistantStockageupLineEdit->text()).exists() || ui->DistantStockageupLineEdit->text() == "")
            {
                UpMessageBox::Watch(this,tr("Vous n'avez pas spécifié de dossier de stockage valide pour les documents d'imagerie !"));
                ui->ParamtabWidget->setCurrentWidget(ui->PosteParamtab);
                ui->ParamConnexiontabWidget->setCurrentWidget(ui->tabDistant);
                ui->DistantStockageupLineEdit->setFocus();
                return false;
            }
        }
    }
    return true;
}

void dlg_param::RestaureBase()
{
    if (proc->RestaureBase())
    {
        UpMessageBox::Watch(this,tr("Le programme va se fermer pour que certaines données puissent être prises en compte"));
        Datas::I()->postesconnectes->SupprimeAllPostesConnectes();
        exit(0);
    }
}

void dlg_param::VerifPosteImportDocs()
{
    QString A = proc->PosteImportDocs();
    ui->LockParamGeneralupLabel->setEnabled(!A.contains(" - " NOM_ADMINISTRATEURDOCS));
    if (A == "")
    {
        ui->PosteImportDocslabel->setText(tr("Pas de poste paramétré"));
        ui->PosteImportDocsPrioritairelabel->setText("");
    }
    else
    {
        A = "<font color=\"green\"><b>" + A.remove(".local") + "</b></font>";
        QString B;
        if (A.contains(" - " NOM_ADMINISTRATEURDOCS))
            B = tr("Administrateur");
        else
            B = (A.contains(" - prioritaire")? tr("prioritaire") : tr("non prioritaire"));
        B = "<b>" + B + "</b>";
        A.remove(" - prioritaire");
        A.remove(" - " NOM_ADMINISTRATEURDOCS);

        ui->PosteImportDocslabel->setText(A);
        if (B == tr("non prioritaire"))
            B = "<font color=\"red\">" + B + "</font>";
        else
            B = "<font color=\"green\">" + B + "</font>";
        ui->PosteImportDocsPrioritairelabel->setText(B);
    }
}

bool dlg_param::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonRelease)
    {
        UpLabel *lbl = dynamic_cast<UpLabel *>(obj);
        if (lbl != Q_NULLPTR)
        {
            UpLabel* upLabelObj = static_cast<UpLabel*>(obj);
            emit click(upLabelObj);
        }
        if (obj == ui->FermepushButton)
        {
            QPushButton* Button = static_cast<QPushButton*>(obj);
            Button->setIcon(Icons::icFerme());
        }
    }
    if(event->type() == QEvent::MouseButtonPress)
    {
        if (obj == ui->FermepushButton)
        {
            QPushButton* Button = static_cast<QPushButton*>(obj);
            Button->setIcon(Icons::icFermeAppuye());
        }
    }
    if(event->type() == QEvent::MouseMove)
        if (obj == ui->FermepushButton)
        {
            QPushButton* Button = static_cast<QPushButton*>(obj);
            QRect rect = QRect(Button->mapToGlobal(QPoint(0,0)),Button->size());
            QPoint pos = mapFromParent(cursor().pos());
            if (rect.contains(pos))
                Button->setIcon(Icons::icFermeAppuye());
            else
                Button->setIcon(Icons::icFerme());
        }

   return QWidget::eventFilter(obj, event);
}

// ----------------------------------------------------------------------------------
// Remplissage de la fiche des paramètres utilisateur
// ----------------------------------------------------------------------------------
void dlg_param::AfficheParamUser()
{
    bool ok;
    ui->idUseruplineEdit                ->setText(QString::number(m_currentuser->id()));
    ui->LoginuplineEdit                 ->setText(m_currentuser->login());
    ui->MDPuplineEdit                   ->setText(m_currentuser->password());
    ui->NomuplineEdit                   ->setText(m_currentuser->nom());
    ui->PrenomuplineEdit                ->setText(m_currentuser->prenom());
    QList<QVariantList> listlieux = db->StandardSelectSQL("select idlieu from " TBL_JOINTURESLIEUX " where iduser = " + QString::number(m_currentuser->id()), ok);
    QList<int> idlieuxlist;
    for (int k=0; k< listlieux.size(); k++)
        idlieuxlist << listlieux.at(k).at(0).toInt();

    ui->PortableuplineEdit              ->setText(m_currentuser->portable());
    ui->MailuplineEdit                  ->setText(m_currentuser->mail());
    ui->Titrelabel                      ->setVisible(m_currentuser->isMedecin());
    ui->Cotationswidget                 ->setVisible(!m_currentuser->isSecretaire() && !m_currentuser->isAutreFonction());

//    ui->ModeExercicegroupBox->setVisible(false);
    bool soccomptable   = m_currentuser->isSocComptable();
    bool medecin        = m_currentuser->isMedecin();

    ui->StatutComptaupTextEdit->setText(proc->getSessionStatus());

    ui->TitreuplineEdit             ->setVisible(medecin);
    ui->Titrelabel                  ->setVisible(medecin);
    ui->Prenomlabel                 ->setVisible(!soccomptable);
    ui->PrenomuplineEdit            ->setVisible(!soccomptable);

    ui->idUseruplineEdit            ->setText(QString::number(m_currentuser->id()));
    ui->LoginuplineEdit             ->setText(m_currentuser->login());
    ui->MDPuplineEdit               ->setText(m_currentuser->password());
    if (medecin)
        ui->TitreuplineEdit         ->setText(m_currentuser->titre());

    if (!m_currentuser->isSecretaire() && !m_currentuser->isAutreFonction())
    {
        ui->Cotationswidget->setVisible(true);
        Remplir_TableActesCCAM();
        Remplir_TableAssocCCAM();
        Remplir_TableHorsNomenclature();

        wdg_assocCCAMcotationswdgbuttonframe->wdg_modifBouton->setEnabled(false);
        wdg_assocCCAMcotationswdgbuttonframe->wdg_moinsBouton->setEnabled(false);
        wdg_HNcotationswdgbuttonframe->wdg_modifBouton->setEnabled(false);
        wdg_HNcotationswdgbuttonframe->wdg_moinsBouton->setEnabled(false);
    }
    else
        ui->Cotationswidget->setVisible(false);
    ReconstruitListeLieuxExerciceUser(m_currentuser);
}

void dlg_param::ConnectSlots()
{
    connect(ui->FermepushButton,                    SIGNAL(clicked(bool)),                  this,   SLOT(Slot_FermepushButtonClicked()));
    connect(ui->InitMDPAdminpushButton,             SIGNAL(clicked(bool)),                  this,   SLOT(Slot_ModifMDPAdmin()));;
    connect(ui->ChoixFontupPushButton,              SIGNAL(clicked(bool)),                  this,   SLOT(Slot_ChoixFontpushButtonClicked()));
    connect(ui->PosteServcheckBox,                  SIGNAL(clicked(bool)),                  this,   SLOT(Slot_EnableFrameServeur(bool)));
    connect(ui->LocalServcheckBox,                  SIGNAL(clicked(bool)),                  this,   SLOT(Slot_EnableFrameServeur(bool)));
    connect(ui->DistantServcheckBox,                SIGNAL(clicked(bool)),                  this,   SLOT(Slot_EnableFrameServeur(bool)));
    connect(ui->GestUserpushButton,                 SIGNAL(clicked(bool)),                  this,   SLOT(Slot_GestionUsers()));
    connect(ui->GestLieuxpushButton,                SIGNAL(clicked(bool)),                  this,   SLOT(Slot_GestLieux()));
    connect(ui->ModifDataUserpushButton,            SIGNAL(clicked(bool)),                  this,   SLOT(Slot_GestionDatasCurrentUser()));
    connect(ui->GestionBanquespushButton,           SIGNAL(clicked(bool)),                  this,   SLOT(Slot_GestionBanques()));
    connect(ui->OupspushButton,                     SIGNAL(clicked(bool)),                  this,   SLOT(Slot_ResetImprimante()));
    connect(ui->LocalStockageupPushButton,          SIGNAL(clicked(bool)),                  this,   SLOT(Slot_DirLocalStockage()));
    connect(ui->DistantStockageupPushButton,        SIGNAL(clicked(bool)),                  this,   SLOT(Slot_DirDistantStockage()));
    connect(ui->PosteStockageupPushButton,          SIGNAL(clicked(bool)),                  this,   SLOT(Slot_DirPosteStockage()));
    connect(ui->EffacePrgSauvupPushButton,          SIGNAL(clicked(bool)),                  this,   SLOT(Slot_EffacePrgSauvegarde()));
    connect(ui->AppareilsConnectesupTableWidget,    SIGNAL(itemSelectionChanged()),         this,   SLOT(Slot_EnableAppBoutons()));
    connect(ui->AutorefupComboBox,                  SIGNAL(currentIndexChanged(int)),       this,   SLOT(Slot_ClearCom(int)));
    connect(ui->TonometreupComboBox,                SIGNAL(currentIndexChanged(int)),       this,   SLOT(Slot_ClearCom(int)));
    connect(ui->FrontoupComboBox,                   SIGNAL(currentIndexChanged(int)),       this,   SLOT(Slot_ClearCom(int)));
    connect(ui->RefracteurupComboBox,               SIGNAL(currentIndexChanged(int)),       this,   SLOT(Slot_ClearCom(int)));
    connect(ui->ActesCCAMupTableWidget,             SIGNAL(itemEntered(QTableWidgetItem*)), this,   SLOT(Slot_AfficheToolTip(QTableWidgetItem*)));
    connect(ui->AssocCCAMupTableWidget,             SIGNAL(itemEntered(QTableWidgetItem*)), this,   SLOT(Slot_AfficheToolTip(QTableWidgetItem*)));
    connect(ui->HorsNomenclatureupTableWidget,      SIGNAL(itemEntered(QTableWidgetItem*)), this,   SLOT(Slot_AfficheToolTip(QTableWidgetItem*)));
    connect(ui->ChercheCCAMupLineEdit,              SIGNAL(textEdited(QString)),            this,   SLOT(Slot_ChercheCCAM(QString)));
    connect(ui->ParamMotifspushButton,              SIGNAL(clicked(bool)),                  this,   SLOT(Slot_ParamMotifs()));
    connect(this,                                   SIGNAL(click(QWidget*)),                this,   SLOT(Slot_EnableModif(QWidget*)));
    connect(ui->OphtaSeulcheckBox,                  SIGNAL(clicked(bool)),                  this,   SLOT(Slot_FiltreActesOphtaSeulmt(bool)));

    QList<QLineEdit*> listline1 = ui->PosteParamtab->findChildren<QLineEdit*>();
    for (int i=0; i<listline1.size(); i++)
        connect(listline1.at(i),                    SIGNAL(textEdited(QString)),        this,   SLOT(Slot_EnableOKModifPosteButton()));
    QList<QComboBox*> listcombo1 = ui->PosteParamtab->findChildren<QComboBox*>();
    for (int i=0; i<listcombo1.size(); i++)
        connect(listcombo1.at(i),                   SIGNAL(currentIndexChanged(int)),   this,   SLOT(Slot_EnableOKModifPosteButton()));
    QList<QRadioButton*> listbutton1 = ui->PosteParamtab->findChildren<QRadioButton*>();
    for (int i=0; i<listbutton1.size(); i++)
        connect(listbutton1.at(i),                  SIGNAL(clicked(bool)),              this,   SLOT(Slot_EnableOKModifPosteButton()));
    QList<QCheckBox*> listcheck = ui->PosteParamtab->findChildren<QCheckBox*>();
    for (int i=0; i<listcheck.size(); i++)
        connect(listcheck.at(i),                    SIGNAL(clicked(bool)),              this,   SLOT(Slot_EnableOKModifPosteButton()));
     QList<QSpinBox*> listspin = ui->PosteParamtab->findChildren<QSpinBox*>();
    for (int i=0; i<listspin.size(); i++)
        connect(listspin.at(i),                     SIGNAL(valueChanged(int)),          this,   SLOT(Slot_EnableOKModifPosteButton()));
    foreach(QRadioButton *butt, ui->JourSauvegardeframe->findChildren<QRadioButton*>())
        connect(butt,                               &QPushButton::clicked,              this,   &dlg_param::ModifDateHeureBackup);
    connect(ui->HeureBackuptimeEdit,                &QTimeEdit::timeChanged,            this,   &dlg_param::ModifDateHeureBackup);
    connect(ui->DirBackuppushButton,                &QPushButton::clicked,              this,   &dlg_param::ModifDirBackup);
    connect(ui->ImmediatBackupupPushButton,         &QPushButton::clicked,              this,   &dlg_param::startImmediateBackup);
    connect(ui->RestaurBaseupPushButton,            &QPushButton::clicked,              this,   &dlg_param::RestaureBase);
    connect(ui->ReinitBaseupPushButton,             &QPushButton::clicked,              proc,   &Procedures::ReinitBase);
}

bool dlg_param::CotationsModifiees() const
{
    return m_cotationsmodifiees;
}

bool dlg_param::DataUserModifiees() const
{
    return m_donneesusermodifiees;
}

void dlg_param::EnableActesCCAM(bool enable)
{
    ui->OphtaSeulcheckBox   ->setEnabled(enable);
    bool autormodif         = enable && (m_currentuser->idparent() == m_currentuser->id());            // les remplaçants ne peuvent pas modifier les actes
    for (int i=0; i<ui->ActesCCAMupTableWidget->rowCount(); i++)
    {
        UpCheckBox *check = dynamic_cast<UpCheckBox*>(ui->ActesCCAMupTableWidget->cellWidget(i,0));
        if (check) check->setEnabled(autormodif);
        if (ui->ActesCCAMupTableWidget->columnCount()==6)
        {
            UpLineEdit *lbl = dynamic_cast<UpLineEdit*>(ui->ActesCCAMupTableWidget->cellWidget(i,5));
            if (lbl)
                lbl->setEnabled(autormodif);
        }
    }
}

void dlg_param::EnableAssocCCAM(bool enable)
{
    bool autormodif = enable && m_currentuser->idparent() == m_currentuser->id();  // les remplaçants ne peuvent pas modifier les actes
    for (int i=0; i<ui->AssocCCAMupTableWidget->rowCount(); i++)
    {
        UpCheckBox *check = dynamic_cast<UpCheckBox*>(ui->AssocCCAMupTableWidget->cellWidget(i,0));
        if (check) check->setEnabled(autormodif);
        UpLineEdit *lbl1 = dynamic_cast<UpLineEdit*>(ui->AssocCCAMupTableWidget->cellWidget(i,2));
        if (lbl1)
            lbl1->setEnabled(autormodif);
        if (ui->AssocCCAMupTableWidget->columnCount()==5)
        {
            UpLineEdit *lbl = dynamic_cast<UpLineEdit*>(ui->AssocCCAMupTableWidget->cellWidget(i,4));
            if (lbl)
                lbl->setEnabled(autormodif);
        }
    }
    ui->AssocCCAMupTableWidget->setSelectionMode(autormodif? QAbstractItemView::SingleSelection : QAbstractItemView::NoSelection);
    if (!autormodif)
        ui->AssocCCAMupTableWidget->clearSelection();
    ui->AssocCCAMupTableWidget  ->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->AssocCCAMupTableWidget  ->setSelectionBehavior(QAbstractItemView::SelectRows);
    wdg_assocCCAMcotationswdgbuttonframe               ->setEnabled(autormodif);
    wdg_assocCCAMcotationswdgbuttonframe->wdg_modifBouton  ->setEnabled(autormodif && ui->AssocCCAMupTableWidget->selectedRanges().size()>0);
    wdg_assocCCAMcotationswdgbuttonframe->wdg_moinsBouton  ->setEnabled(autormodif && ui->AssocCCAMupTableWidget->selectedRanges().size()>0);
}

void dlg_param::EnableHorsNomenclature(bool enable)
{
    bool autormodif = enable && m_currentuser->idparent() == m_currentuser->id();  // les remplaçants ne peuvent pas modifier les actes
    for (int i=0; i<ui->HorsNomenclatureupTableWidget->rowCount(); i++)
    {
        UpCheckBox *check = dynamic_cast<UpCheckBox*>(ui->HorsNomenclatureupTableWidget->cellWidget(i,0));
        if (check) check->setEnabled(autormodif);
        UpLineEdit *lbl = dynamic_cast<UpLineEdit*>(ui->HorsNomenclatureupTableWidget->cellWidget(i,2));
        if (lbl)
            lbl->setEnabled(autormodif);
    }
    ui->HorsNomenclatureupTableWidget->setSelectionMode(autormodif? QAbstractItemView::SingleSelection : QAbstractItemView::NoSelection);
    if (!autormodif)
        ui->HorsNomenclatureupTableWidget->clearSelection();
    wdg_HNcotationswdgbuttonframe              ->setEnabled(autormodif);
    wdg_HNcotationswdgbuttonframe->wdg_modifBouton ->setEnabled(autormodif && ui->HorsNomenclatureupTableWidget->selectedRanges().size()>0);
    wdg_HNcotationswdgbuttonframe->wdg_moinsBouton ->setEnabled(autormodif && ui->HorsNomenclatureupTableWidget->selectedRanges().size()>0);
}

void dlg_param::EnableWidgContent(QWidget *widg, bool a)
{
    QList<QWidget*> listwidg = widg->findChildren<QWidget*>();
    for (int i=0; i<listwidg.size(); i++)
        listwidg.at(i)->setEnabled(a);
    if (widg == ui->Sauvegardeframe)
    {
        ui->ModifBaselabel->setVisible(db->getMode() != DataBase::Poste);
        if (db->getMode() == DataBase::Poste)
            ui->EffacePrgSauvupPushButton->setEnabled(m_parametres->daysbkup()
                                                   && QDir(m_parametres->dirbkup()).exists()
                                                   && m_parametres->dirbkup() != ""
                                                   && m_parametres->heurebkup() != QTime());
        else
            ui->ModifBaselabel->setEnabled(true);
    }
}

void dlg_param::Slot_EnregistreNouvMDPAdmin()
{
    if (dlg_askMDP != Q_NULLPTR)
    {
        // Vérifier la cohérence
        QString anc, nouv, confirm;
        UpMessageBox msgbox;
        msgbox.setText(tr("Erreur"));
        msgbox.setIcon(UpMessageBox::Warning);
        UpSmallButton OKBouton("OK");
        msgbox.addButton(&OKBouton, UpSmallButton::STARTBUTTON);
        anc         = dlg_askMDP->findChild<UpLineEdit*>(m_ancienMDP)->text();
        nouv        = dlg_askMDP->findChild<UpLineEdit*>(m_nouveauMDP)->text();
        confirm     = dlg_askMDP->findChild<UpLineEdit*>(m_confirmeMDP)->text();

        if (anc == "")
        {
            QSound::play(NOM_ALARME);
            msgbox.setInformativeText(tr("Ancien mot de passe requis"));
            dlg_askMDP->findChild<UpLineEdit*>(m_ancienMDP)->setFocus();
            msgbox.exec();
            return;
        }
        if (anc != proc->getMDPAdmin())
        {
            QSound::play(NOM_ALARME);
            msgbox.setInformativeText(tr("Le mot de passe que vous voulez modifier n'est pas le bon\n"));
            dlg_askMDP->findChild<UpLineEdit*>(m_ancienMDP)->setFocus();
            msgbox.exec();
            return;
        }
        if (!Utils::rgx_AlphaNumeric_5_12.exactMatch(nouv) || nouv == "")
        {
            QSound::play(NOM_ALARME);
            msgbox.setInformativeText(tr("Le nouveau mot de passe n'est pas conforme\n(au moins 5 caractères - chiffres ou lettres non accentuées -\n"));
            dlg_askMDP->findChild<UpLineEdit*>(m_nouveauMDP)->setFocus();
            msgbox.exec();
            return;
        }
        if (nouv != confirm)
        {
            QSound::play(NOM_ALARME);
            msgbox.setInformativeText("Les mots de passe ne correspondent pas\n");
            dlg_askMDP->findChild<UpLineEdit*>(m_nouveauMDP)->setFocus();
            msgbox.exec();
            return;
        }
        msgbox.setText(tr("Modifications enregistrées"));
        msgbox.setInformativeText(tr("Le nouveau mot de passe a été enregistré avec succès"));
        //recherche de l'iUser du compte AdminDocs
        int idAdminDocs = 0;
        bool ok;
        QVariantList mdpdata = db->getFirstRecordFromStandardSelectSQL("select iduser from " TBL_UTILISATEURS " where UserNom = '" NOM_ADMINISTRATEURDOCS "'", ok);
        if (!ok || mdpdata.size()==0)
        {
            db->StandardSQL("insert into " TBL_UTILISATEURS " (UserNom, UserLogin) values ('" NOM_ADMINISTRATEURDOCS "', '" NOM_ADMINISTRATEURDOCS "')");
            mdpdata = db->getFirstRecordFromStandardSelectSQL("select iduser from " TBL_UTILISATEURS " where UserNom = '" NOM_ADMINISTRATEURDOCS "'", ok);
        }
        idAdminDocs = mdpdata.at(0).toInt();
        db->setmdpadmin(nouv);
        // Enregitrer le nouveau MDP de la base
        QString req = "update " TBL_UTILISATEURS " set userMDP = '" + nouv + "' where idUser = " + QString::number(idAdminDocs);
        db->StandardSQL(req);
        // Enregitrer le nouveau MDP de connexion à MySQL
        req = "set password for '" NOM_ADMINISTRATEURDOCS "'@'localhost' = '" + nouv + "'";
        db->StandardSQL(req);
        QString AdressIP;
        foreach (const QHostAddress &address, QNetworkInterface::allAddresses()) {
            if (address.protocol() == QAbstractSocket::IPv4Protocol && address != QHostAddress(QHostAddress::LocalHost))
                 AdressIP = address.toString();
        }
        QString Domaine;
        QStringList listIP = AdressIP.split(".");
        for (int i=0;i<listIP.size()-1;i++)
            Domaine += listIP.at(i) + ".";
        req = "set password for '" NOM_ADMINISTRATEURDOCS "'@'" + Domaine + "%' = '" + nouv + "'";
        db->StandardSQL(req);
        req = "set password for '" NOM_ADMINISTRATEURDOCS "SSL'@'%' = '" + nouv + "'";
        db->StandardSQL(req);
        dlg_askMDP->done(0);
        msgbox.exec();
    }
}

// ----------------------------------------------------------------------------------
// Remplissage de la table des actes en CCAM.
// ----------------------------------------------------------------------------------
void dlg_param::Remplir_TableActesCCAM(bool ophtaseul)
{
    bool ok;
    // Mise en forme de la table ActesCCAM
    ui->ActesCCAMupTableWidget->setPalette(QPalette(Qt::white));
    ui->ActesCCAMupTableWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->ActesCCAMupTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->ActesCCAMupTableWidget->verticalHeader()->setVisible(false);
    ui->ActesCCAMupTableWidget->setSelectionMode(QAbstractItemView::NoSelection);
    ui->ActesCCAMupTableWidget->setMouseTracking(true);
    int ncol = (m_currentuser->secteurconventionnel()>1? 6 : 5);
    ui->ActesCCAMupTableWidget->setColumnCount(ncol);
    ui->ActesCCAMupTableWidget->setColumnWidth(0,20);           //checkbox
    ui->ActesCCAMupTableWidget->setColumnWidth(1,90);           //code CCAM
    ui->ActesCCAMupTableWidget->setColumnWidth(2,80);           //OPTAM
    ui->ActesCCAMupTableWidget->setColumnWidth(3,90);           //NonOPTAM
    ui->ActesCCAMupTableWidget->setColumnWidth(4,0);            //descriptif
    ui->ActesCCAMupTableWidget->setHorizontalHeaderItem(0, new QTableWidgetItem(""));
    ui->ActesCCAMupTableWidget->setHorizontalHeaderItem(1, new QTableWidgetItem("Cotation"));
    ui->ActesCCAMupTableWidget->setHorizontalHeaderItem(2, new QTableWidgetItem("OPTAM"));
    ui->ActesCCAMupTableWidget->setHorizontalHeaderItem(3, new QTableWidgetItem("non OPTAM"));
    ui->ActesCCAMupTableWidget->setHorizontalHeaderItem(4, new QTableWidgetItem(""));
    ui->ActesCCAMupTableWidget->horizontalHeader()->setVisible(true);
    ui->ActesCCAMupTableWidget->horizontalHeaderItem(1)->setTextAlignment(Qt::AlignCenter);
    ui->ActesCCAMupTableWidget->horizontalHeaderItem(2)->setTextAlignment(Qt::AlignCenter);
    ui->ActesCCAMupTableWidget->horizontalHeaderItem(3)->setTextAlignment(Qt::AlignCenter);
    if (ncol==6)
    {
        ui->ActesCCAMupTableWidget->setColumnWidth(5,80);      //Tarif pratiqué
        ui->ActesCCAMupTableWidget->setHorizontalHeaderItem(5, new QTableWidgetItem("Tarif\npratiqué"));
        ui->ActesCCAMupTableWidget->horizontalHeaderItem(5)->setTextAlignment(Qt::AlignCenter);
    }
    ui->ActesCCAMupTableWidget->FixLargeurTotale();
    ui->ActesCCAMupTableWidget->horizontalHeader()->setFixedHeight(int(QFontMetrics(qApp->font()).height()*2.3));
    connect(ui->ActesCCAMupTableWidget,     SIGNAL(currentCellChanged(int,int,int,int)),    this, SLOT(Slot_RegleAssocBoutons()));
    connect(ui->ActesCCAMupTableWidget,     SIGNAL(cellClicked(int,int)),                   this, SLOT(Slot_RegleAssocBoutons()));

    //Remplissage Table Actes
    QTableWidgetItem    *pItem0;
    QTableWidgetItem    *pItem1;
    QTableWidgetItem    *pItem2;
    QTableWidgetItem    *pItem3;
    UpCheckBox          *check;
    ui->ActesCCAMupTableWidget->clearContents();
    QString Remplirtablerequete = "SELECT nom, codeccam, OPTAM, NonOPTAM from "  TBL_CCAM;
    if (ophtaseul)
        Remplirtablerequete += " where codeccam like 'B%'";
    Remplirtablerequete +=  " order by codeccam";
    QList<QVariantList> Acteslist = db->StandardSelectSQL(Remplirtablerequete, ok);
    if (!ok)
        return;
    ui->ActesCCAMupTableWidget->setRowCount(Acteslist.size());
    for (int i=0; i<Acteslist.size(); i++)
    {
        pItem0      = new QTableWidgetItem();
        pItem1      = new QTableWidgetItem();
        pItem2      = new QTableWidgetItem();
        pItem3      = new QTableWidgetItem();
        check       = new UpCheckBox(ui->ActesCCAMupTableWidget);
        check->setRowTable(i);
        check->setEnabled(false);
        connect(check,  SIGNAL(clicked(bool)),  this,   SLOT(Slot_MAJActesCCAM()));
        connect(check,  SIGNAL(clicked(bool)),  this,   SLOT(Slot_RegleAssocBoutons()));
        ui->ActesCCAMupTableWidget->setCellWidget(i,0,check);
        pItem0->setText(Acteslist.at(i).at(1).toString());                             // codeCCAM
        ui->ActesCCAMupTableWidget->setItem(i,1,pItem0);
        pItem1->setTextAlignment(Qt::AlignRight);
        pItem1->setText(QLocale().toString(Acteslist.at(i).at(2).toDouble(),'f',2));   // Montant OPTAM
        ui->ActesCCAMupTableWidget->setItem(i,2,pItem1);
        pItem2->setTextAlignment(Qt::AlignRight);
        pItem2->setText(QLocale().toString(Acteslist.at(i).at(3).toDouble(),'f',2));   // Montant no OPTAM
        ui->ActesCCAMupTableWidget->setItem(i,3,pItem2);
        pItem3->setText(Acteslist.at(i).at(0).toString());                             // descriptif
        ui->ActesCCAMupTableWidget->setItem(i,4,pItem3);
        ui->ActesCCAMupTableWidget->setRowHeight(i, int(QFontMetrics(qApp->font()).height()*1.1));
    }
    QString reqactes = "select typeacte, montantpratique from " TBL_COTATIONS " where idUser = " + QString::number(m_currentuser->id());
    QList<QVariantList> Actesusrlist = db->StandardSelectSQL(reqactes, ok);
    if (Actesusrlist.size()>0)
    {
        for (int i=0; i<Actesusrlist.size(); i++)
        {
            QString codeCCAM = Actesusrlist.at(i).at(0).toString();
            QList<QTableWidgetItem*> listitems = ui->ActesCCAMupTableWidget->findItems(codeCCAM, Qt::MatchExactly);
            if (listitems.size()>0)
                static_cast<UpCheckBox*>(ui->ActesCCAMupTableWidget->cellWidget(listitems.at(0)->row(),0))->setCheckState(Qt::Checked);
            if (ncol==6)
            {
                QDoubleValidator *val = new QDoubleValidator(this);
                val->setDecimals(2);
                if (listitems.size()>0)
                {
                    UpLineEdit *lbl = new UpLineEdit();
                    lbl->setText(QLocale().toString(Actesusrlist.at(i).at(1).toDouble(),'f',2));      // Tarif pratiqué
                    lbl->setAlignment(Qt::AlignRight);
                    lbl->setStyleSheet("border: 0px solid rgb(150,150,150)");
                    lbl->setRow(listitems.at(0)->row());
                    lbl->setValidator(val);
                    lbl->setEnabled(false);
                    connect(lbl,    SIGNAL(TextModified(QString)),  this,   SLOT(Slot_MAJActesCCAM(QString)));
                    ui->ActesCCAMupTableWidget->setCellWidget(listitems.at(0)->row(),5,lbl);
                }
            }
        }
    }
}

// ----------------------------------------------------------------------------------
// Remplissage de la table des associations CCAM.
// ----------------------------------------------------------------------------------
void dlg_param::Remplir_TableAssocCCAM()
{
    bool ok;
    // Mise en forme de la table AssocCCAM
    ui->AssocCCAMupTableWidget->setPalette(QPalette(Qt::white));
    ui->AssocCCAMupTableWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->AssocCCAMupTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->AssocCCAMupTableWidget->verticalHeader()->setVisible(false);
    ui->AssocCCAMupTableWidget->setSelectionMode(QAbstractItemView::NoSelection);
    ui->AssocCCAMupTableWidget->setMouseTracking(true);
    int ncol = (m_currentuser->secteurconventionnel() > 1 ? 5 : 4);
    ui->AssocCCAMupTableWidget->setColumnCount(ncol);
    ui->AssocCCAMupTableWidget->setColumnWidth(0,20);           //checkbox
    ui->AssocCCAMupTableWidget->setColumnWidth(1,135);          //code CCAM
    ui->AssocCCAMupTableWidget->setColumnWidth(2,65);           //OPTAM
    ui->AssocCCAMupTableWidget->setColumnWidth(3,65);           //NonOPTAM
    ui->AssocCCAMupTableWidget->setHorizontalHeaderItem(0, new QTableWidgetItem(""));
    ui->AssocCCAMupTableWidget->setHorizontalHeaderItem(1, new QTableWidgetItem("Cotation"));
    ui->AssocCCAMupTableWidget->setHorizontalHeaderItem(2, new QTableWidgetItem("OPTAM"));
    ui->AssocCCAMupTableWidget->setHorizontalHeaderItem(3, new QTableWidgetItem("non\nOPTAM"));
    ui->AssocCCAMupTableWidget->horizontalHeader()->setVisible(true);
    ui->AssocCCAMupTableWidget->horizontalHeaderItem(1)->setTextAlignment(Qt::AlignCenter);
    ui->AssocCCAMupTableWidget->horizontalHeaderItem(2)->setTextAlignment(Qt::AlignCenter);
    ui->AssocCCAMupTableWidget->horizontalHeaderItem(3)->setTextAlignment(Qt::AlignCenter);
    if (ncol==5)
    {
        ui->AssocCCAMupTableWidget->setColumnWidth(4,75);      //Tarif pratiqué
        ui->AssocCCAMupTableWidget->setHorizontalHeaderItem(4, new QTableWidgetItem("Tarif\npratiqué"));
        ui->AssocCCAMupTableWidget->horizontalHeaderItem(4)->setTextAlignment(Qt::AlignCenter);
    }
    ui->AssocCCAMupTableWidget->FixLargeurTotale();
    wdg_assocCCAMcotationswdgbuttonframe->widgButtonParent()->setFixedWidth(ui->AssocCCAMupTableWidget->width());
    ui->AssocCCAMupTableWidget->horizontalHeader()->setFixedHeight(int(QFontMetrics(qApp->font()).height()*2.3));
    connect(ui->AssocCCAMupTableWidget,     SIGNAL(currentCellChanged(int,int,int,int)),    this, SLOT(Slot_RegleAssocBoutons()));
    connect(ui->AssocCCAMupTableWidget,     SIGNAL(cellClicked(int,int)),                   this, SLOT(Slot_RegleAssocBoutons()));

    //Remplissage Table AssocCCCAM
    QTableWidgetItem    *pItem0;
    UpCheckBox          *check;
    QDoubleValidator *val = new QDoubleValidator(this);
    val->setDecimals(2);
    ui->AssocCCAMupTableWidget->clearContents();
    QString Assocrequete = "SELECT TYPEACTE, montantOPTAM, montantNonOptam, montantpratique, tip from "  TBL_COTATIONS " WHERE CCAM = 2 AND iduser = " + QString::number(m_currentuser->id()) + " order by typeacte";
    //qDebug() << Assocrequete;
    QList<QVariantList> Assoclist = db->StandardSelectSQL(Assocrequete, ok);
    if (!ok)
        return;
    ui->AssocCCAMupTableWidget->setRowCount(Assoclist.size());
    for (int i=0; i<Assoclist.size(); i++)
    {
        pItem0      = new QTableWidgetItem();
        check       = new UpCheckBox(ui->AssocCCAMupTableWidget);
        check->setRowTable(i);
        check->setEnabled(false);
        check->setChecked(true);
        connect(check,  SIGNAL(clicked(bool)),  this,   SLOT(Slot_MAJAssocCCAM()));
        connect(check,  SIGNAL(clicked(bool)),  this,   SLOT(Slot_RegleAssocBoutons()));
        ui->AssocCCAMupTableWidget->setCellWidget(i,0,check);
        pItem0->setText(Assoclist.at(i).at(0).toString());                             // codeCCAM
        ui->AssocCCAMupTableWidget->setItem(i,1,pItem0);

        UpLineEdit *lbl1 = new UpLineEdit();
        lbl1->setText(QLocale().toString(Assoclist.at(i).at(1).toDouble(),'f',2));      // montant OPTAM
        lbl1->setdatas(Assoclist.at(i).at(4));                                           // Tip
        lbl1->setAlignment(Qt::AlignRight);
        lbl1->setStyleSheet("border: 0px solid rgb(150,150,150)");
        lbl1->setRow(i);
        lbl1->setColumn(2);
        lbl1->setValidator(val);
        lbl1->setEnabled(false);
        connect(lbl1,    SIGNAL(TextModified(QString)),  this,   SLOT(Slot_MAJAssocCCAM(QString)));
        ui->AssocCCAMupTableWidget->setCellWidget(i,2,lbl1);

        UpLineEdit *lbl2 = new UpLineEdit();
        lbl2->setText(QLocale().toString(Assoclist.at(i).at(2).toDouble(),'f',2));      // montant nonOPTAM
        lbl2->setAlignment(Qt::AlignRight);
        lbl2->setStyleSheet("border: 0px solid rgb(150,150,150)");
        lbl2->setRow(i);
        lbl2->setColumn(3);
        lbl2->setValidator(val);
        lbl2->setEnabled(false);
        connect(lbl2,    SIGNAL(TextModified(QString)),  this,   SLOT(Slot_MAJAssocCCAM(QString)));
        ui->AssocCCAMupTableWidget->setCellWidget(i,3,lbl2);

        if (ncol == 5)
        {
            UpLineEdit *lbl3 = new UpLineEdit();
            lbl3->setText(QLocale().toString(Assoclist.at(i).at(3).toDouble(),'f',2));      // Tarif pratiqué
            lbl3->setAlignment(Qt::AlignRight);
            lbl3->setStyleSheet("border: 0px solid rgb(150,150,150)");
            lbl3->setRow(i);
            lbl3->setColumn(4);
            lbl3->setValidator(val);
            lbl3->setEnabled(false);
            connect(lbl3,    SIGNAL(TextModified(QString)),  this,   SLOT(Slot_MAJAssocCCAM(QString)));
            ui->AssocCCAMupTableWidget->setCellWidget(i,4,lbl3);
        }
        ui->AssocCCAMupTableWidget->setRowHeight(i, int(QFontMetrics(qApp->font()).height()*1.1));
    }
    Assocrequete = "SELECT DISTINCT TYPEACTE, montantoptam, montantnonoptam, montantpratique, Tip from "  TBL_COTATIONS " WHERE CCAM = 2"
                   " and typeacte not in (SELECT TYPEACTE from "  TBL_COTATIONS " WHERE CCAM = 2 AND iduser = " + QString::number(m_currentuser->id()) + ")";
    QList<QVariantList> Assoc2list = db->StandardSelectSQL(Assocrequete, ok);
    if (!ok)
        return;
    for (int i=0; i<Assoc2list.size(); i++)
    {
        int row = ui->AssocCCAMupTableWidget->rowCount();
        ui->AssocCCAMupTableWidget->insertRow(row);
        pItem0      = new QTableWidgetItem();
        check       = new UpCheckBox(ui->HorsNomenclatureupTableWidget);
        check->setRowTable(row);
        check->setEnabled(false);
        check->setChecked(false);
        connect(check,  SIGNAL(clicked(bool)),  this,   SLOT(Slot_MAJAssocCCAM()));
        connect(check,  SIGNAL(clicked(bool)),  this,   SLOT(Slot_RegleAssocBoutons()));
        ui->AssocCCAMupTableWidget->setCellWidget(row,0,check);
        pItem0->setText(Assoc2list.at(i).at(0).toString());                             // codeCCAM
        ui->AssocCCAMupTableWidget->setItem(row,1,pItem0);

        UpLineEdit *lbl1 = new UpLineEdit();
        lbl1->setText(QLocale().toString(Assoc2list.at(i).at(1).toDouble(),'f',2));      // montant OPTAM
        lbl1->setdatas(Assoc2list.at(i).at(4));                                           // Tip
        lbl1->setAlignment(Qt::AlignRight);
        lbl1->setStyleSheet("border: 0px solid rgb(150,150,150)");
        lbl1->setRow(i);
        lbl1->setColumn(2);
        lbl1->setValidator(val);
        lbl1->setEnabled(false);
        connect(lbl1,    SIGNAL(TextModified(QString)),  this,   SLOT(Slot_MAJAssocCCAM(QString)));
        ui->AssocCCAMupTableWidget->setCellWidget(row,2,lbl1);

        UpLineEdit *lbl2 = new UpLineEdit();
        lbl2->setText(QLocale().toString(Assoc2list.at(i).at(2).toDouble(),'f',2));      // montant nonOPTAM
        lbl2->setAlignment(Qt::AlignRight);
        lbl2->setStyleSheet("border: 0px solid rgb(150,150,150)");
        lbl2->setRow(i);
        lbl2->setColumn(3);
        lbl2->setValidator(val);
        lbl2->setEnabled(false);
        connect(lbl2,    SIGNAL(TextModified(QString)),  this,   SLOT(Slot_MAJAssocCCAM(QString)));
        ui->AssocCCAMupTableWidget->setCellWidget(row,3,lbl2);

        if (ncol == 5)
        {
            UpLineEdit *lbl3 = new UpLineEdit();
            lbl3->setText(QLocale().toString(Assoc2list.at(i).at(3).toDouble(),'f',2));      // Tarif pratiqué
            lbl3->setAlignment(Qt::AlignRight);
            lbl3->setStyleSheet("border: 0px solid rgb(150,150,150)");
            lbl3->setRow(i);
            lbl3->setColumn(4);
            lbl3->setValidator(val);
            lbl3->setEnabled(false);
            connect(lbl3,    SIGNAL(TextModified(QString)),  this,   SLOT(Slot_MAJAssocCCAM(QString)));
            ui->AssocCCAMupTableWidget->setCellWidget(row,4,lbl3);
        }
        ui->AssocCCAMupTableWidget->setRowHeight(row, int(QFontMetrics(qApp->font()).height()*1.1));
        row ++;
    }
}

// ----------------------------------------------------------------------------------
// Remplissage de la table des actes hors nomenclature.
// ----------------------------------------------------------------------------------
void dlg_param::Remplir_TableHorsNomenclature()
{
    bool ok;
    // Mise en forme de la table HorsNomenclature
    ui->HorsNomenclatureupTableWidget->setPalette(QPalette(Qt::white));
    ui->HorsNomenclatureupTableWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->HorsNomenclatureupTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->HorsNomenclatureupTableWidget->verticalHeader()->setVisible(false);
    ui->HorsNomenclatureupTableWidget->setSelectionMode(QAbstractItemView::NoSelection);
    ui->HorsNomenclatureupTableWidget->setMouseTracking(true);

    ui->HorsNomenclatureupTableWidget->setColumnCount(3);
    ui->HorsNomenclatureupTableWidget->setColumnWidth(0,20);           //checkbox
    ui->HorsNomenclatureupTableWidget->setColumnWidth(1,160);          //code CCAM
    ui->HorsNomenclatureupTableWidget->setColumnWidth(2,90);           //Secteur I
    ui->HorsNomenclatureupTableWidget->FixLargeurTotale();
    ui->HorsNomenclatureupTableWidget->setHorizontalHeaderItem(0, new QTableWidgetItem(""));
    ui->HorsNomenclatureupTableWidget->setHorizontalHeaderItem(1, new QTableWidgetItem("Cotation"));
    ui->HorsNomenclatureupTableWidget->setHorizontalHeaderItem(2, new QTableWidgetItem("Tarif\npratiqué"));
    ui->HorsNomenclatureupTableWidget->horizontalHeader()->setVisible(true);
    ui->HorsNomenclatureupTableWidget->horizontalHeaderItem(1)->setTextAlignment(Qt::AlignCenter);
    ui->HorsNomenclatureupTableWidget->horizontalHeaderItem(2)->setTextAlignment(Qt::AlignCenter);
    ui->HorsNomenclatureupTableWidget->horizontalHeader()->setFixedHeight(int(QFontMetrics(qApp->font()).height()*2.3));
    connect(ui->HorsNomenclatureupTableWidget,     SIGNAL(currentCellChanged(int,int,int,int)),    this, SLOT(Slot_RegleAssocBoutons()));
    connect(ui->HorsNomenclatureupTableWidget,     SIGNAL(cellClicked(int,int)),                   this, SLOT(Slot_RegleAssocBoutons()));

    //Remplissage Table Horsnomenclature
    QTableWidgetItem    *pItem0;
    UpCheckBox          *check;
    QDoubleValidator *val = new QDoubleValidator(this);
    val->setDecimals(2);
    ui->HorsNomenclatureupTableWidget->clearContents();
    QString Horsrequete = "SELECT TYPEACTE, montantpratique, Tip from "  TBL_COTATIONS " WHERE CCAM = 3 AND iduser = " + QString::number(m_currentuser->id());
    QList<QVariantList> Horslist = db->StandardSelectSQL(Horsrequete, ok);
    if (!ok)
        return;
    ui->HorsNomenclatureupTableWidget->setRowCount(Horslist.size());
    for (int i=0; i<Horslist.size(); i++)
    {
        pItem0      = new QTableWidgetItem();
        check       = new UpCheckBox();
        check->setRowTable(i);
        check->setEnabled(false);
        check->setChecked(true);
        connect(check,  SIGNAL(clicked(bool)),  this,   SLOT(Slot_MAJHorsNomenclature()));
        connect(check,  SIGNAL(clicked(bool)),  this,   SLOT(Slot_RegleAssocBoutons()));
        ui->HorsNomenclatureupTableWidget->setCellWidget(i,0,check);
        pItem0->setText(Horslist.at(i).at(0).toString());                             // codeCCAM
        ui->HorsNomenclatureupTableWidget->setItem(i,1,pItem0);
        UpLineEdit *lbl1 = new UpLineEdit();
        lbl1->setText(QLocale().toString(Horslist.at(i).at(1).toDouble(),'f',2));      // montant
        lbl1->setdatas(Horslist.at(i).at(2));                                           // Tip
        lbl1->setAlignment(Qt::AlignRight);
        lbl1->setStyleSheet("border: 0px solid rgb(150,150,150)");
        lbl1->setRow(i);
        lbl1->setValidator(val);
        lbl1->setEnabled(false);
        connect(lbl1,    SIGNAL(TextModified(QString)),  this,   SLOT(Slot_MAJHorsNomenclature(QString)));
        ui->HorsNomenclatureupTableWidget->setCellWidget(i,2,lbl1);
        ui->HorsNomenclatureupTableWidget->setRowHeight(i, int(QFontMetrics(qApp->font()).height()*1.1));
    }
    Horsrequete = "SELECT TYPEACTE from "  TBL_COTATIONS " WHERE CCAM = 3 AND iduser <> " + QString::number(m_currentuser->id())+
            " and typeacte not in (SELECT TYPEACTE from "  TBL_COTATIONS " WHERE CCAM = 3 AND iduser = " + QString::number(m_currentuser->id()) + ")";
    QList<QVariantList> Hors2list = db->StandardSelectSQL(Horsrequete, ok);
    if (!ok)
        return;
    for (int i=0; i<Hors2list.size(); i++)
    {
        int row = ui->HorsNomenclatureupTableWidget->rowCount();
        ui->HorsNomenclatureupTableWidget->insertRow(row);
        pItem0      = new QTableWidgetItem();
        check       = new UpCheckBox();
        check->setRowTable(row);
        check->setEnabled(false);
        check->setChecked(false);
        connect(check,  SIGNAL(clicked(bool)),  this,   SLOT(Slot_MAJHorsNomenclature()));
        ui->HorsNomenclatureupTableWidget->setCellWidget(row,0,check);
        pItem0->setText(Hors2list.at(i).at(0).toString());                             // codeCCAM
        ui->HorsNomenclatureupTableWidget->setItem(row,1,pItem0);
        UpLineEdit *lbl1 = new UpLineEdit();
        lbl1->setText("");      // montant pratiqué = 0
        lbl1->setAlignment(Qt::AlignRight);
        lbl1->setStyleSheet("border: 0px solid rgb(150,150,150)");
        lbl1->setRow(row);
        lbl1->setValidator(val);
        lbl1->setEnabled(false);
        connect(lbl1,    SIGNAL(TextModified(QString)),  this,   SLOT(Slot_MAJHorsNomenclature(QString)));
        ui->HorsNomenclatureupTableWidget->setCellWidget(row,2,lbl1);
        ui->HorsNomenclatureupTableWidget->setRowHeight(row, int(QFontMetrics(qApp->font()).height()*1.1));
    }
}

// ----------------------------------------------------------------------------------
// Remplissage des tables.
// ----------------------------------------------------------------------------------
void dlg_param::Remplir_Tables()
{
    bool ok;
    QTableWidgetItem    *pItem0;
    QTableWidgetItem    *pItem0a, *pItem0b, *pItem0c;
    QTableWidgetItem    *pItem1;
    QTableWidgetItem    *pItem3;
    QTableWidgetItem    *pItem4a, *pItem4b, *pItem4c;
    UpLineEdit          *line5a, *line5b, *line5c;
    QTableWidgetItem    *pItem6;
    UpPushButton        *dossbouton;
    UpPushButton        *dossbouton1;
    UpPushButton        *dossbouton2;
    int i;
    QFontMetrics fm(qApp->font());
    QFont disabledFont = qApp->font();
    disabledFont.setItalic(true);
    QPalette palette;
    palette.setColor(QPalette::Text,Qt::gray);

    ui->AppareilsConnectesupTableWidget->clearContents();
    ui->MonoDocupTableWidget->clearContents();
    ui->LocalDocupTableWidget->clearContents();
    ui->DistantDocupTableWidget->clearContents();

    //Remplissage Table Documents

    QString  req = "SELECT list.idAppareil, list.TitreExamen, list.NomAppareil, Format"
              " FROM "  TBL_APPAREILSCONNECTESCENTRE " appcon , " TBL_LISTEAPPAREILS " list"
              " where list.idappareil = appcon.idappareil and idLieu = " + QString::number(m_currentuser->idsitedetravail()) +
              " ORDER BY TitreExamen";

    QList<QVariantList> Applist = db->StandardSelectSQL(req, ok);
    if (!ok)
        return;
    ui->AppareilsConnectesupTableWidget->setRowCount(Applist.size());
    ui->MonoDocupTableWidget->setRowCount(Applist.size());
    ui->DistantDocupTableWidget->setRowCount(Applist.size());
    ui->LocalDocupTableWidget->setRowCount(Applist.size());
    for (i = 0; i < Applist.size(); i++)
    {
        pItem0      = new QTableWidgetItem() ;
        pItem0a     = new QTableWidgetItem() ;
        pItem0b     = new QTableWidgetItem() ;
        pItem0c     = new QTableWidgetItem() ;
        pItem1      = new QTableWidgetItem() ;
        pItem3      = new QTableWidgetItem() ;
        pItem4a     = new QTableWidgetItem() ;
        pItem4b     = new QTableWidgetItem() ;
        pItem4c     = new QTableWidgetItem() ;
        line5a      = new UpLineEdit(ui->MonoDocupTableWidget);
        line5b      = new UpLineEdit(ui->LocalDocupTableWidget);
        line5c      = new UpLineEdit(ui->DistantDocupTableWidget);
        pItem6      = new QTableWidgetItem() ;
        dossbouton  = new UpPushButton(ui->MonoDocupTableWidget);
        dossbouton1 = new UpPushButton(ui->LocalDocupTableWidget);
        dossbouton2 = new UpPushButton(ui->DistantDocupTableWidget);

        int col = 0;
        pItem0->setText(Applist.at(i).at(0).toString());                             // idAppareil
        pItem0a->setText(Applist.at(i).at(0).toString());
        pItem0b->setText(Applist.at(i).at(0).toString());
        pItem0c->setText(Applist.at(i).at(0).toString());
        ui->AppareilsConnectesupTableWidget->setItem(i,col,pItem0);
        ui->MonoDocupTableWidget->setItem(i,col,pItem0a);
        ui->LocalDocupTableWidget->setItem(i,col,pItem0b);
        ui->DistantDocupTableWidget->setItem(i,col,pItem0c);

        col++; //1
        pItem1->setText(Applist.at(i).at(1).toString());                             // TypeExamen
        ui->AppareilsConnectesupTableWidget->setItem(i,col,pItem1);
        pItem4a->setText(Applist.at(i).at(1).toString() + " - " + Applist.at(i).at(2).toString());
        pItem4b->setText(Applist.at(i).at(1).toString() + " - " + Applist.at(i).at(2).toString());
        pItem4c->setText(Applist.at(i).at(1).toString() + " - " + Applist.at(i).at(2).toString());
        ui->MonoDocupTableWidget->setItem(i,col,pItem4a);
        ui->LocalDocupTableWidget->setItem(i,col,pItem4b);
        ui->DistantDocupTableWidget->setItem(i,col,pItem4c);

        col++; //2
        pItem3->setText(Applist.at(i).at(2).toString());                             // NomAppareil
        ui->AppareilsConnectesupTableWidget->setItem(i,col,pItem3);
        line5a->setText(proc->pathDossierDocuments(Applist.at(i).at(2).toString(),DataBase::Poste));
        line5b->setText(proc->pathDossierDocuments(Applist.at(i).at(2).toString(),DataBase::ReseauLocal));
        line5c->setText(proc->pathDossierDocuments(Applist.at(i).at(2).toString(),DataBase::Distant));
        line5a->setRow(i);
        line5b->setRow(i);
        line5c->setRow(i);
        ui->MonoDocupTableWidget->setCellWidget(i,col,line5a);
        ui->LocalDocupTableWidget->setCellWidget(i,col,line5b);
        ui->DistantDocupTableWidget->setCellWidget(i,col,line5c);
        connect(line5a,                    SIGNAL(textEdited(QString)),         this,   SLOT(Slot_EnableOKModifPosteButton()));
        connect(line5b,                    SIGNAL(textEdited(QString)),         this,   SLOT(Slot_EnableOKModifPosteButton()));
        connect(line5c,                    SIGNAL(textEdited(QString)),         this,   SLOT(Slot_EnableOKModifPosteButton()));
        connect(line5a,                    SIGNAL(TextModified(QString)),       this,   SLOT(Slot_EnregDossierStockageApp(QString)));
        connect(line5b,                    SIGNAL(TextModified(QString)),       this,   SLOT(Slot_EnregDossierStockageApp(QString)));
        connect(line5c,                    SIGNAL(TextModified(QString)),       this,   SLOT(Slot_EnregDossierStockageApp(QString)));
        line5a->setStyleSheet("UpLineEdit {background-color:white; border: 0px solid rgb(150,150,150);border-radius: 0px;}"
                              "UpLineEdit:focus {border: 0px solid rgb(164, 205, 255);border-radius: 0px;}");
        line5b->setStyleSheet("UpLineEdit {background-color:white; border: 0px solid rgb(150,150,150);border-radius: 0px;}"
                              "UpLineEdit:focus {border: 0px solid rgb(164, 205, 255);border-radius: 0px;}");
        line5c->setStyleSheet("UpLineEdit {background-color:white; border: 0px solid rgb(150,150,150);border-radius: 0px;}"
                              "UpLineEdit:focus {border: 0px solid rgb(164, 205, 255);border-radius: 0px;}");
        col++; //3
        dossbouton->setIcon(Icons::icSortirDossier());
        dossbouton->setIconSize(QSize(15,15));
        dossbouton->setiD(Applist.at(i).at(0).toInt());
        dossbouton->setFixedSize(15,15);
        dossbouton->setFlat(true);
        dossbouton->setFocusPolicy(Qt::NoFocus);
        QWidget *widg = new QWidget;
        QHBoxLayout *l = new QHBoxLayout();
        l->setAlignment( Qt::AlignCenter );
        l->setContentsMargins(0,0,0,0);
        l->addWidget(dossbouton);
        widg->setLayout(l);
        dossbouton1->setIcon(Icons::icSortirDossier());
        dossbouton1->setIconSize(QSize(15,15));
        dossbouton1->setiD(Applist.at(i).at(0).toInt());
        dossbouton1->setFixedSize(15,15);
        dossbouton1->setFlat(true);
        dossbouton1->setFocusPolicy(Qt::NoFocus);
        QWidget *widg1 = new QWidget;
        QHBoxLayout *l1 = new QHBoxLayout();
        l1->setAlignment( Qt::AlignCenter );
        l1->setContentsMargins(0,0,0,0);
        l1->addWidget(dossbouton1);
        widg1->setLayout(l1);
        dossbouton2->setIcon(Icons::icSortirDossier());
        dossbouton2->setIconSize(QSize(15,15));
        dossbouton2->setiD(Applist.at(i).at(0).toInt());
        dossbouton2->setFixedSize(15,15);
        dossbouton2->setFlat(true);
        dossbouton2->setFocusPolicy(Qt::NoFocus);
        QWidget *widg2 = new QWidget;
        QHBoxLayout *l2 = new QHBoxLayout();
        l2->setAlignment( Qt::AlignCenter );
        l2->setContentsMargins(0,0,0,0);
        l2->addWidget(dossbouton2);
        widg2->setLayout(l2);
        ui->MonoDocupTableWidget->setCellWidget(i,col,widg);
        ui->LocalDocupTableWidget->setCellWidget(i,col,widg1);
        ui->DistantDocupTableWidget->setCellWidget(i,col,widg2);
        connect(dossbouton,       SIGNAL(clicked(bool)), this   ,SLOT(Slot_ChoixDossierStockageApp()));
        connect(dossbouton1,      SIGNAL(clicked(bool)), this   ,SLOT(Slot_ChoixDossierStockageApp()));
        connect(dossbouton2,      SIGNAL(clicked(bool)), this   ,SLOT(Slot_ChoixDossierStockageApp()));

        pItem6->setText(Applist.at(i).at(3).toString());                             // Format
        ui->AppareilsConnectesupTableWidget->setItem(i,col,pItem6);

        ui->AppareilsConnectesupTableWidget->setRowHeight(i,int(fm.height()*1.3));
        ui->MonoDocupTableWidget->setRowHeight(i,int(fm.height()*1.3));
        ui->LocalDocupTableWidget->setRowHeight(i,int(fm.height()*1.3));
        ui->DistantDocupTableWidget->setRowHeight(i,int(fm.height()*1.3));
    }

    m_listeappareils.clear();
    req = "select NomAppareil from " TBL_LISTEAPPAREILS
          " where idAppareil not in (select idAppareil from " TBL_APPAREILSCONNECTESCENTRE " where idlieu = " + QString::number(m_currentuser->idsitedetravail()) + ")";
    QList<QVariantList> Appareilslist = db->StandardSelectSQL(req, ok);
    if (!ok)
        return;
    if (Appareilslist.size() == 0)
        wdg_appareilswdgbuttonframe->wdg_plusBouton->setEnabled(false);
    else
        for (int i=0; i<Appareilslist.size(); i++)
            m_listeappareils << Appareilslist.at(i).at(0).toString();
    wdg_appareilswdgbuttonframe->wdg_moinsBouton->setEnabled(Applist.size()>0);
}

bool dlg_param::Valide_Modifications()
{
    if (m_modifposte)
    {
        QStringList listcomm;
        listcomm << ui->PortAutorefupComboBox->currentText() << ui->PortFrontoupComboBox->currentText() << ui->PortRefracteurupComboBox->currentText() << ui->PortTonometreupComboBox->currentText();
        QStringList result = listcomm.filter("COM");
        int a = result.size();
        result.removeDuplicates();
        if (result.size()<a)
        {
            UpMessageBox::Watch(this,tr("Un même port COM est affecté à plusieurs appareils différents!"));
            ui->ParamtabWidget->setCurrentWidget(ui->PosteParamtab);
            return false;
        }
        if (ui->AutorefupComboBox->currentIndex()>0 && ui->PortAutorefupComboBox->currentIndex() == 0)
        {
            UpMessageBox::Watch(this,tr("Vous n'avez pas spécifié de port pour l'autorefractomètre ") + ui->AutorefupComboBox->currentText() + " !");
            ui->ParamtabWidget->setCurrentWidget(ui->PosteParamtab);
            ui->PortAutorefupComboBox->setFocus();
            return false;
        }
        if (ui->FrontoupComboBox->currentIndex()>0 && ui->PortFrontoupComboBox->currentIndex() == 0)
        {
            UpMessageBox::Watch(this,tr("Vous n'avez pas spécifié de port pour le frontofocomètre ") + ui->FrontoupComboBox->currentText() + " !");
            ui->ParamtabWidget->setCurrentWidget(ui->PosteParamtab);
            ui->PortFrontoupComboBox->setFocus();
            return false;
        }
        if (ui->RefracteurupComboBox->currentIndex()>0 && ui->PortRefracteurupComboBox->currentIndex() == 0)
        {
            UpMessageBox::Watch(this,tr("Vous n'avez pas spécifié de port COM pour le réfracteur ") + ui->RefracteurupComboBox->currentText() + " !");
            ui->ParamtabWidget->setCurrentWidget(ui->PosteParamtab);
            ui->PortRefracteurupComboBox->setFocus();
            return false;
        }
        if (ui->TonometreupComboBox->currentIndex()>0 && ui->PortTonometreupComboBox->currentIndex() == 0)
        {
            UpMessageBox::Watch(this,tr("Vous n'avez pas spécifié de port COM pour le tonomètre ") + ui->TonometreupComboBox->currentText() + " !");
            ui->ParamtabWidget->setCurrentWidget(ui->PosteParamtab);
            ui->PortTonometreupComboBox->setFocus();
            return false;
        }
        if (ui->AutorefupComboBox->currentIndex()==0 && ui->PortAutorefupComboBox->currentIndex() > 0)
        {
            UpMessageBox::Watch(this,tr("Vous avez spécifié un port COM pour l'autorefractomètre sans sélectionner de machine !"));
            ui->ParamtabWidget->setCurrentWidget(ui->PosteParamtab);
            ui->AutorefupComboBox->setFocus();
            return false;
        }
        if (ui->FrontoupComboBox->currentIndex()==0 && ui->PortFrontoupComboBox->currentIndex() > 0)
        {
            UpMessageBox::Watch(this,tr("Vous avez spécifié un port COM pour le frontofocotomètre sans sélectionner de machine !"));
            ui->ParamtabWidget->setCurrentWidget(ui->PosteParamtab);
            ui->FrontoupComboBox->setFocus();
            return false;
        }
        if (ui->RefracteurupComboBox->currentIndex()==0 && ui->PortRefracteurupComboBox->currentIndex() < 0)
        {
            UpMessageBox::Watch(this,tr("Vous avez spécifié un port COM pour le réfracteur sans sélectionner de machine !"));
            ui->ParamtabWidget->setCurrentWidget(ui->PosteParamtab);
            ui->RefracteurupComboBox->setFocus();
            return false;
        }
        if (ui->TonometreupComboBox->currentIndex()==0 && ui->PortTonometreupComboBox->currentIndex() > 0)
        {
            UpMessageBox::Watch(this,tr("Vous avez spécifié un port COM pour le tonomètre sans sélectionner de machine !"));
            ui->ParamtabWidget->setCurrentWidget(ui->PosteParamtab);
            ui->TonometreupComboBox->setFocus();
            return false;
        }
        QString Base = "BDD_POSTE";
        if (ui->PosteServcheckBox->isChecked())
            proc->settings()->setValue(Base + "/Active","YES");
        else
            proc->settings()->setValue(Base + "/Active","NO");
        proc->settings()->setValue(Base + "/Port",ui->SQLPortPostecomboBox->currentText());

        Base = "BDD_LOCAL";
        if (ui->LocalServcheckBox->isChecked())
            proc->settings()->setValue(Base + "/Active","YES");
        else
            proc->settings()->setValue(Base + "/Active","NO");
        proc->settings()->setValue(Base + "/Serveur",ui->EmplacementLocaluplineEdit->text());
        db->setadresseserveurlocal(ui->EmplacementLocaluplineEdit->text());
        proc->settings()->setValue(Base + "/Port",ui->SQLPortLocalcomboBox->currentText());

        Base = "BDD_DISTANT";
        if (ui->DistantServcheckBox->isChecked())
            proc->settings()->setValue(Base + "/Active","YES");
        else
            proc->settings()->setValue(Base + "/Active","NO");
        proc->settings()->setValue(Base + "/Serveur",ui->EmplacementDistantuplineEdit->text());
        db->setadresseserveurdistant(ui->EmplacementDistantuplineEdit->text());
        proc->settings()->setValue(Base + "/Port",ui->SQLPortDistantcomboBox->currentText());
        proc->settings()->setValue("Param_Imprimante/TailleEnTete",ui->EntetespinBox->value());
        proc->settings()->setValue("Param_Imprimante/TailleEnTeteALD",ui->EnteteALDspinBox->value());
        proc->settings()->setValue("Param_Imprimante/TaillePieddePage",ui->PiedDePagespinBox->value());
        proc->settings()->setValue("Param_Imprimante/TaillePieddePageOrdoLunettes",ui->PiedDePageOrdoLunettesspinBox->value());
        QString OK = (ui->ApercuImpressioncheckBox->isChecked()? "YES" : "NO");
        proc->settings()->setValue("Param_Imprimante/ApercuAvantImpression", OK);
        OK = (ui->OrdoAvecDuplicheckBox->isChecked()? "YES" : "NO");
        proc->settings()->setValue("Param_Imprimante/OrdoAvecDupli",OK);
        if (ui->PrioritaireImportDocscheckBox->isChecked())
            proc->settings()->setValue("BDD_LOCAL/PrioritaireGestionDocs","YES");
        else if (ui->NonImportDocscheckBox->isChecked())
            proc->settings()->setValue("BDD_LOCAL/PrioritaireGestionDocs","NO");
        else
            proc->settings()->setValue("BDD_LOCAL/PrioritaireGestionDocs","NORM");
        proc->settings()->setValue("Param_Imprimante/TailleEnTete",ui->EntetespinBox->text());
        proc->settings()->setValue("Param_Imprimante/TailleEnTeteALD",ui->EnteteALDspinBox->text());
        proc->settings()->setValue("Param_Imprimante/TaillePieddePage",ui->PiedDePagespinBox->text());
        proc->settings()->setValue("Param_Imprimante/TaillePieddePageOrdoLunettes",ui->PiedDePageOrdoLunettesspinBox->text());
        proc->settings()->setValue("Param_Imprimante/TailleTopMarge",ui->TopMargespinBox->text());

        proc->settings()->setValue("Param_Poste/Fronto",ui->FrontoupComboBox->currentText());
        proc->settings()->setValue("Param_Poste/PortFronto",ui->PortFrontoupComboBox->currentText());
        proc->settings()->setValue("Param_Poste/Autoref",ui->AutorefupComboBox->currentText());
        proc->settings()->setValue("Param_Poste/PortAutoref",ui->PortAutorefupComboBox->currentText());
        proc->settings()->setValue("Param_Poste/Refracteur",ui->RefracteurupComboBox->currentText());
        proc->settings()->setValue("Param_Poste/PortRefracteur",ui->PortRefracteurupComboBox->currentText());
        proc->settings()->setValue("Param_Poste/Tonometre",ui->TonometreupComboBox->currentText());
        proc->settings()->setValue("Param_Poste/PortTonometre",ui->PortTonometreupComboBox->currentText());

        proc->settings()->setValue("Param_Poste/VilleParDefaut",wdg_VilleDefautlineEdit->text());
        proc->settings()->setValue("Param_Poste/CodePostalParDefaut",wdg_CPDefautlineEdit->text());

        m_modifposte = false;
    }
    return true;
}
