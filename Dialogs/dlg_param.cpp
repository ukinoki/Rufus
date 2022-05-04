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

    QStringList ports;
    ports << "3306" << "3307";
    ui->SQLPortDistantcomboBox  ->addItems(ports);
    ui->SQLPortLocalcomboBox    ->addItems(ports);
    ui->SQLPortPostecomboBox    ->addItems(ports);

    wdg_appareilswdgbuttonframe                     = new WidgetButtonFrame(ui->AppareilsConnectesupTableWidget);
    wdg_appareilswdgbuttonframe                     ->AddButtons(WidgetButtonFrame::Plus | WidgetButtonFrame::Moins);
    connect(wdg_appareilswdgbuttonframe,            &WidgetButtonFrame::choix,  this,   [=] {ChoixButtonFrame(wdg_appareilswdgbuttonframe);});

    wdg_HNcotationswdgbuttonframe                   = new WidgetButtonFrame(ui->HorsNomenclatureupTableWidget);
    wdg_HNcotationswdgbuttonframe                   ->AddButtons(WidgetButtonFrame::Plus | WidgetButtonFrame::Modifier | WidgetButtonFrame::Moins);
    connect(wdg_HNcotationswdgbuttonframe,          &WidgetButtonFrame::choix,  this,   [=] {ChoixButtonFrame(wdg_HNcotationswdgbuttonframe);});

    wdg_assocCCAMcotationswdgbuttonframe            = new WidgetButtonFrame(ui->AssocCCAMupTableWidget);
    wdg_assocCCAMcotationswdgbuttonframe            ->AddButtons(WidgetButtonFrame::Plus | WidgetButtonFrame::Modifier | WidgetButtonFrame::Moins);
    connect(wdg_assocCCAMcotationswdgbuttonframe,   &WidgetButtonFrame::choix,  this,   [=] {ChoixButtonFrame(wdg_assocCCAMcotationswdgbuttonframe);});

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

    ui->PathServeurLocalStockageupLabel ->setStyleSheet("QLabel { background-color : white;}");
    ui->PathServeurLocalStockageupLabel ->setImmediateToolTip(tr("Cet emplacement ne peut être modifié qu'en modifiant l'emplacement du serveur d'imagerie dans une session Rufus en monoposte sur le serveur"));
    ui->PathServeurStockageupLabel      ->setStyleSheet("QLabel { background-color : white;}");
    ui->PathServeurStockageupLabel      ->setImmediateToolTip(tr("Cet emplacement ne peut être modifié qu'en modifiant l'emplacement du serveur d'imagerie dans une session Rufus en monoposte sur le serveur"));

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
    ui->PortFrontoupComboBox    ->insertItems(0,ListPortsCOM);
    ui->PortFrontoupComboBox    ->addItem("Box");
    ui->PortFrontoupComboBox    ->addItem(RESEAU);
    ui->PortAutorefupComboBox   ->insertItems(0,ListPortsCOM);
    ui->PortAutorefupComboBox   ->addItem("Box");
    ui->PortAutorefupComboBox   ->addItem(RESEAU);
    ui->PortRefracteurupComboBox->insertItems(0,ListPortsCOM);
    ui->PortRefracteurupComboBox->addItem(RESEAU);
    ui->PortTonometreupComboBox ->insertItems(0,ListPortsCOM);
    ui->PortTonometreupComboBox ->addItem(RESEAU);


    QString tip = tr("Indiquez ici l'emplacement du dossier de stockage des documents d'imagerie <br /><font color=\"green\"><b>SUR CE POSTE SERVEUR</b></font>");
    ui->PosteStockageupLabel        ->setImmediateToolTip(tip);
    ui->PosteStockageupLineEdit     ->setImmediateToolTip(tip);
    tip = tr("Indiquez ici <br /><font color=\"green\"><b>LE LIEN</b></font><br /> vers l'emplacement du dossier de stockage des documents d'imagerie <br /><font color=\"green\"><b>SUR LE SERVEUR</b></font>");
    ui->LocalPathStockageupLabel    ->setImmediateToolTip(tip);
    ui->LocalPathStockageupLineEdit ->setImmediateToolTip(tip);

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

    ui->FrontoupComboBox                ->setCurrentText(proc->settings()->value("Param_Poste/Fronto").toString());
    ui->PortFrontoupComboBox            ->setCurrentText(proc->settings()->value("Param_Poste/PortFronto").toString());
    if (ui->PortFrontoupComboBox->currentText() == RESEAU)
        ui->NetworkPathFrontoupLineEdit->setText(proc->settings()->value("Param_Poste/PortFronto/Reseau").toString());
    EnableNetworkAppareilRefraction(ui->PortFrontoupComboBox,       ui->PortFrontoupComboBox->currentIndex());
    ui->NetworkPathFrontoupLineEdit     ->setImmediateToolTip(ui->NetworkPathFrontoupLineEdit->text());

    ui->AutorefupComboBox               ->setCurrentText(proc->settings()->value("Param_Poste/Autoref").toString());
    ui->PortAutorefupComboBox           ->setCurrentText(proc->settings()->value("Param_Poste/PortAutoref").toString());
    if (ui->PortAutorefupComboBox->currentText() == RESEAU)
        ui->NetworkPathAutorefupLineEdit->setText(proc->settings()->value("Param_Poste/PortAutoref/Reseau").toString());
    EnableNetworkAppareilRefraction(ui->PortAutorefupComboBox,      ui->PortAutorefupComboBox->currentIndex());
    ui->NetworkPathAutorefupLineEdit    ->setImmediateToolTip(ui->NetworkPathAutorefupLineEdit->text());

    ui->RefracteurupComboBox            ->setCurrentText(proc->settings()->value("Param_Poste/Refracteur").toString());
    ui->PortRefracteurupComboBox        ->setCurrentText(proc->settings()->value("Param_Poste/PortRefracteur").toString());
    if (ui->PortRefracteurupComboBox->currentText() == RESEAU)
    {
        ui->NetworkPathRefracteurupLineEdit->setText(proc->settings()->value("Param_Poste/PortRefracteur/Reseau").toString());
        ui->NetworkPathEchangeFrontoupLineEdit->setText(proc->settings()->value("Param_Poste/PortRefracteur/Reseau/AdressFronto").toString());
        ui->NetworkPathEchangeAutorefupLineEdit->setText(proc->settings()->value("Param_Poste/PortRefracteur/Reseau/AdressAutoref").toString());
    }
    EnableNetworkAppareilRefraction(ui->PortRefracteurupComboBox,   ui->PortRefracteurupComboBox->currentIndex());
    ui->NetworkPathRefracteurupPushButton       ->setImmediateToolTip(tr("Emplacement du fichier d'échange du refracteur"));
    ui->NetworkPathRefracteurupLineEdit         ->setImmediateToolTip(ui->NetworkPathRefracteurupLineEdit->text());
    ui->NetworkPathEchangeFrontoupPushButton    ->setImmediateToolTip(tr("Emplacement du fichier d'échange du frontofocomètre"));
    ui->NetworkPathEchangeFrontoupLineEdit      ->setImmediateToolTip(ui->NetworkPathEchangeFrontoupLineEdit->text());
    ui->NetworkPathEchangeAutorefupPushButton   ->setImmediateToolTip(tr("Emplacement du fichier d'échange de l'autorefractomètre"));
    ui->NetworkPathEchangeAutorefupLineEdit     ->setImmediateToolTip(ui->NetworkPathEchangeAutorefupLineEdit->text());

    ui->TonometreupComboBox             ->setCurrentText(proc->settings()->value("Param_Poste/Tonometre").toString());
    ui->PortTonometreupComboBox         ->setCurrentText(proc->settings()->value("Param_Poste/PortTonometre").toString());
    if (ui->PortTonometreupComboBox->currentText() == RESEAU)
        ui->NetworkPathTonoupLineEdit   ->setText(proc->settings()->value("Param_Poste/PortTonometre/Reseau").toString());
    EnableNetworkAppareilRefraction(ui->PortTonometreupComboBox,    ui->PortTonometreupComboBox->currentIndex());
    ui->NetworkPathTonoupLineEdit       ->setImmediateToolTip(ui->NetworkPathTonoupLineEdit->text());

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
    Base = Utils::getBaseFromMode(Utils::Poste);
    a = (proc->settings()->value(Base + "/Active").toString() == "YES");
    ui->PosteServcheckBox           ->setChecked(a);
    ui->Posteframe                  ->setVisible(a);
    ui->MonoConnexionupLabel        ->setVisible(a);
    ui->MonoDocsExtupLabel          ->setVisible(a);
    ui->MonoDocupTableWidget        ->setVisible(a);
    if (a)
    {
        bool poste = DataBase::I()->ModeAccesDataBase() == Utils::Poste;
        ui->PosteStockageupLabel        ->setVisible(poste);
        ui->PosteStockageupLineEdit     ->setVisible(poste);
        ui->PosteStockageupPushButton   ->setVisible(poste);
        ui->SQLPortPostecomboBox        ->setCurrentText(proc->settings()->value(Base + "/Port").toString());
        ui->PosteStockageupLineEdit     ->setText(m_parametres->dirimagerieserveur());
    }
    Base = Utils::getBaseFromMode(Utils::ReseauLocal);
    b = (proc->settings()->value(Base + "/Active").toString() == "YES");
    ui->LocalServcheckBox               ->setChecked(b);
    ui->Localframe                      ->setVisible(b);
    ui->LocalConnexionupLabel           ->setVisible(b);
    ui->LocalDocsExtupLabel             ->setVisible(b);
    ui->LocalDocupTableWidget           ->setVisible(b);
    ui->LocalPathStockageupLabel        ->setVisible(b);
    ui->LocalPathStockageupLineEdit     ->setVisible(b);
    ui->LocalPathStockageupPushButton   ->setVisible(b);
    if (b)
    {
        ui->EmplacementLocaluplineEdit  ->setText(proc->settings()->value(Base + "/Serveur").toString());
        ui->SQLPortLocalcomboBox        ->setCurrentText(proc->settings()->value(Base + "/Port").toString());
        ui->LocalPathStockageupLineEdit ->setText(proc->settings()->value(Utils::getBaseFromMode(Utils::ReseauLocal) + "/DossierImagerie").toString());
        bool local = DataBase::I()->ModeAccesDataBase() == Utils::ReseauLocal;
        ui->ServeurLocalStockageupLabel      ->setVisible(local);
        ui->PathServeurLocalStockageupLabel  ->setVisible(local);
        if (local)
            ui->PathServeurLocalStockageupLabel ->setText(m_parametres->dirimagerieserveur());
    }
    Base = Utils::getBaseFromMode(Utils::Distant);
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
        QString dir = proc->settings()->value(Utils::getBaseFromMode(Utils::Distant) + "/DossierClesSSL").toString();
        if (dir == "")
        {
            dir = "/etc/mysql";
            proc->settings()->setValue(Utils::getBaseFromMode(Utils::Distant) + "/DossierClesSSL", dir);
        }
        ui->DossierClesSSLupLineEdit    ->setText(proc->settings()->value(Base + "/DossierClesSSL").toString());
        ui->DistantStockageupLineEdit   ->setText(proc->settings()->value(Utils::getBaseFromMode(Utils::Distant) + "/DossierImagerie").toString());
        bool distant = DataBase::I()->ModeAccesDataBase() == Utils::Distant;
        ui->ServeurStockageupLabel      ->setVisible(distant);
        ui->PathServeurStockageupLabel  ->setVisible(distant);
        if (distant)
            ui->PathServeurStockageupLabel  ->setText(m_parametres->dirimagerieserveur());
    }

    if (db->ModeAccesDataBase() == (Utils::Poste))
        ui->ParamConnexiontabWidget->setCurrentIndex(0);
    else if (db->ModeAccesDataBase() == Utils::ReseauLocal)
        ui->ParamConnexiontabWidget->setCurrentIndex(1);
    if (db->ModeAccesDataBase() == Utils::Distant)
        ui->ParamConnexiontabWidget->setCurrentIndex(2);

    ui->ParamtabWidget->setCurrentIndex(0);

    ui->EntetespinBox->setValue(proc->settings()->value("Param_Imprimante/TailleEnTete").toInt());
    ui->EnteteALDspinBox->setValue(proc->settings()->value("Param_Imprimante/TailleEnTeteALD").toInt());
    ui->PiedDePagespinBox->setValue(proc->settings()->value("Param_Imprimante/TaillePieddePage").toInt());
    ui->PiedDePageOrdoLunettesspinBox->setValue(proc->settings()->value("Param_Imprimante/TaillePieddePageOrdoLunettes").toInt());
    ui->TopMargespinBox->setValue(proc->settings()->value("Param_Imprimante/TailleTopMarge").toInt());
    ui->ApercuImpressioncheckBox->setChecked(proc->settings()->value("Param_Imprimante/ApercuAvantImpression").toString() ==  "YES");
    ui->OrdoAvecDuplicheckBox->setChecked(proc->settings()->value("Param_Imprimante/OrdoAvecDupli").toString() ==  "YES");
    QString A = proc->settings()->value(Utils::getBaseFromMode(Utils::ReseauLocal) + "/PrioritaireGestionDocs").toString();
    if (A=="YES")
        ui->PrioritaireImportDocscheckBox->setChecked(true);
    else if (A=="NO")
        ui->NonImportDocscheckBox->setChecked(true);
    else
    {
        ui->NonPrioritaireImportDocscheckBox->setChecked(true);
        proc->settings()->setValue(Utils::getBaseFromMode(Utils::ReseauLocal) + "/PrioritaireGestionDocs","NORM");
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
    ui->AppareilsconnectesupLabel->setText(tr("Appareils connectés au réseau") + " <font color=\"green\"><b>"
                                           + (Datas::I()->sites->currentsite() != Q_NULLPTR? Datas::I()->sites->currentsite()->nom() : "") + "</b></font> ");
    QVBoxLayout *applay = new QVBoxLayout();
    applay      ->addWidget(ui->AppareilsconnectesupLabel);
    applay      ->addWidget(wdg_appareilswdgbuttonframe->widgButtonParent());
    applay      ->addSpacerItem(new QSpacerItem(10,10,QSizePolicy::Expanding, QSizePolicy::Expanding));
    marge   = 10;
    applay      ->setContentsMargins(marge,marge,marge,marge);
    applay      ->setSpacing(marge);
    ui->Appareilsconnectesframe->setFixedWidth(wdg_appareilswdgbuttonframe->widgButtonParent()->width() + marge + marge);
    ui->Appareilsconnectesframe->setLayout(applay);

    ui->Sauvegardeframe         ->setEnabled(db->ModeAccesDataBase() == (Utils::Poste));
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
    PosteConnecte* post = Datas::I()->postesconnectes->admin(Item::NoUpdate);
    if (post != Q_NULLPTR)
        ui->TCPlabel->setText("<font color=\"black\">" + tr("Serveur") + " </font>"
                            + "<font color=\"green\"><b>" + post->ipadress() + "</b></font>"
                            + "<font color=\"black\"> " + " - " + tr("Port") + " " + "</font>"
                            + "<font color=\"green\"><b> " NOM_PORT_TCPSERVEUR "</b></font>");
    else
        ui->TCPlabel->setText("");
    Remplir_Tables();
    ConnectSignals();
}

dlg_param::~dlg_param()
{
    delete ui;
}

void dlg_param::AfficheToolTip(QTableWidget *table, QTableWidgetItem *item)
{
    QPoint pos = cursor().pos();
    QRect rect = QRect(pos,QSize(10,10));
    if (table == ui->ActesCCAMupTableWidget)
        QToolTip::showText(cursor().pos(),ui->ActesCCAMupTableWidget->item(item->row(),4)->text(), ui->ActesCCAMupTableWidget, rect, 2000);
    else if (table == ui->AssocCCAMupTableWidget)
    {
        QString tip = item->text();
        UpLineEdit * line = dynamic_cast<UpLineEdit*>(ui->AssocCCAMupTableWidget->cellWidget(item->row(),2));
        if (line != Q_NULLPTR)
            if (line->datas().toString() != "")
                tip += "\n" + line->datas().toString();
        QToolTip::showText(cursor().pos(),tip, ui->AssocCCAMupTableWidget, rect, 2000);
    }
    else if (table == ui->HorsNomenclatureupTableWidget)
    {
        QString tip = item->text();
        UpLineEdit * line = dynamic_cast<UpLineEdit*>(ui->HorsNomenclatureupTableWidget->cellWidget(item->row(),2));
        if (line != Q_NULLPTR)
            if (line->datas().toString() != "")
                tip += "\n" + line->datas().toString();
        QToolTip::showText(cursor().pos(),tip, ui->HorsNomenclatureupTableWidget, rect, 2000);
    }
}

void dlg_param::FermepushButtonClicked()
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

void dlg_param::EnableSupprAppareilBouton()
{
   wdg_appareilswdgbuttonframe->wdg_moinsBouton->setEnabled(true);
}

void dlg_param::ChercheCodeCCAM(QString txt)
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

void dlg_param::ChoixDossierStockageApp(UpPushButton *butt)
{
    QString req = "select " CP_TITREEXAMEN_APPAREIL ", " CP_NOMAPPAREIL_APPAREIL " from " TBL_LISTEAPPAREILS " where " CP_ID_APPAREIL " = " + QString::number(butt->iD());
    bool ok;
    QVariantList examdata = db->getFirstRecordFromStandardSelectSQL(req, ok);
    QString exam = "";
    if (ok && examdata.size()>0)
        exam = examdata.at(1).toString();
    Utils::ModeAcces mode = Utils::ReseauLocal;
    if (ui->MonoDocupTableWidget->isAncestorOf(butt))
        mode = Utils::Poste;
    else if (ui->LocalDocupTableWidget->isAncestorOf(butt))
        mode = Utils::ReseauLocal;
    else if (ui->DistantDocupTableWidget->isAncestorOf(butt))
        mode = Utils::Distant;
    QString dir = proc->pathDossierDocuments(exam, mode);
    if (dir == "")
        dir = PATH_DIR_RUFUS;
    QFileDialog dialog(this, "", dir);
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setViewMode(QFileDialog::List);
    if (dialog.exec()>0)
    {
        QDir dockdir = dialog.directory();
        int row;
        UpLineEdit *line = Q_NULLPTR;
        switch (mode) {
        case Utils::Poste:
            row = ui->MonoDocupTableWidget->findItems(QString::number(butt->iD()), Qt::MatchExactly).at(0)->row();
            line    = dynamic_cast<UpLineEdit*>(ui->MonoDocupTableWidget->cellWidget(row,2));
            if (line!=Q_NULLPTR)
                line->setText(dockdir.path());
            break;
        case Utils::ReseauLocal:
            row = ui->LocalDocupTableWidget->findItems(QString::number(butt->iD()), Qt::MatchExactly).at(0)->row();
            line    = dynamic_cast<UpLineEdit*>(ui->LocalDocupTableWidget->cellWidget(row,2));
            if (line!=Q_NULLPTR)
                line->setText(dockdir.path());
            break;
        case Utils::Distant:
            row = ui->DistantDocupTableWidget->findItems(QString::number(butt->iD()), Qt::MatchExactly).at(0)->row();
            line    = dynamic_cast<UpLineEdit*>(ui->DistantDocupTableWidget->cellWidget(row,2));
            if (line!=Q_NULLPTR)
                line->setText(dockdir.path());
            break;
        }
        proc->settings()->setValue(Utils::getBaseFromMode(mode) + "/DossiersDocuments/" + exam, dockdir.path());
    }
}

void dlg_param::EnregDossierStockageApp(UpLineEdit *line, QString dir)
{
    if (line==Q_NULLPTR) return;
    if (!QDir(dir).exists() && dir != "")
    {
        QString textline = line->valeuravant();
        UpMessageBox::Watch(this,tr("Repertoire invalide!"));
        line->setText(textline);
        return;
    }
    QString id;
    Utils::ModeAcces mode = Utils::ReseauLocal;
    if (ui->MonoDocupTableWidget->isAncestorOf(line))
    {
        mode = Utils::Poste;
        id = ui->MonoDocupTableWidget->item(line->Row(),0)->text();
    }
    else if (ui->LocalDocupTableWidget->isAncestorOf(line))
    {
        mode = Utils::ReseauLocal;
        id = ui->LocalDocupTableWidget->item(line->Row(),0)->text();
    }
    else if (ui->DistantDocupTableWidget->isAncestorOf(line))
    {
        mode = Utils::Distant;
        id = ui->DistantDocupTableWidget->item(line->Row(),0)->text();
    }
    QString req = "select " CP_NOMAPPAREIL_APPAREIL " from " TBL_LISTEAPPAREILS " where " CP_ID_APPAREIL " = " + id;
    bool ok;
    QVariantList appdata = db->getFirstRecordFromStandardSelectSQL(req, ok);
    QString app = "";
    if (ok && appdata.size()>0)
        app = appdata.at(0).toString();
    if (app != "")
        proc->settings()->setValue(Utils::getBaseFromMode(mode) + "/DossiersDocuments/" + app, dir);
    else
        UpMessageBox::Watch(this,tr("Impossible de retrouver le nom de l'appareil"));

}

void dlg_param::ChoixButtonFrame(WidgetButtonFrame *widgbutt)
{
    if (widgbutt== wdg_HNcotationswdgbuttonframe)
    {
        switch (wdg_HNcotationswdgbuttonframe->Choix()) {
        case WidgetButtonFrame::Plus:
            NouvHorsNomenclature();
            break;
        case WidgetButtonFrame::Modifier:
            ModifHorsNomenclature();
            break;
        case WidgetButtonFrame::Moins:
            SupprHorsNomenclature();
            break;
        }
    }
    else if (widgbutt== wdg_assocCCAMcotationswdgbuttonframe)
    {
        switch (wdg_assocCCAMcotationswdgbuttonframe->Choix()) {
        case WidgetButtonFrame::Plus:
            NouvAssocCCAM();
            break;
        case WidgetButtonFrame::Modifier:
            ModifAssocCCAM();
            break;
        case WidgetButtonFrame::Moins:
            SupprAssocCCAM();
            break;
        }
    }
    else if (widgbutt== wdg_appareilswdgbuttonframe)
    {
        switch (wdg_appareilswdgbuttonframe->Choix()) {
        case WidgetButtonFrame::Plus:
            NouvAppareil();
            break;
        case WidgetButtonFrame::Moins:
            SupprAppareil();
            break;
        default:
            break;
        }
    }
}

void dlg_param::ChoixFontpushButtonClicked()
{
    dlg_fontdialog *Dlg_Fonts = new dlg_fontdialog(PATH_FILE_INI, "PositionsFiches/PositionFontDialog");
    Dlg_Fonts->setFont(qApp->font());
    Dlg_Fonts->setWindowTitle(tr("Choisissez la police d'écran"));
    if (Dlg_Fonts->exec() > 0)
    {
        QString fontrequete = "update " TBL_UTILISATEURS " set " CP_POLICEECRAN_USR " = '" + Dlg_Fonts->font().family()
                                + "', " CP_POLICEATTRIBUT_USR " = '" + Dlg_Fonts->fontAttribut()
                                + "' where " CP_ID_USR " = " + QString::number(currentuser()->id());
        db->StandardSQL(fontrequete,"dlg_param::ChoixFontpushButtonClicked()");
    }
    delete Dlg_Fonts;
}

void dlg_param::ClearCom(UpComboBox* box, int a)
{
    if (a==0)
    {
        if (box==ui->AutorefupComboBox)
            ui->PortAutorefupComboBox   ->setCurrentIndex(0);
        if (box==ui->FrontoupComboBox)
            ui->PortFrontoupComboBox    ->setCurrentIndex(0);
        if (box==ui->TonometreupComboBox)
            ui->PortTonometreupComboBox ->setCurrentIndex(0);
        if (box==ui->RefracteurupComboBox)
            ui->PortRefracteurupComboBox->setCurrentIndex(0);
    }
    ui->PortAutorefupComboBox   ->setEnabled(ui->AutorefupComboBox->currentIndex()>0);
    ui->PortFrontoupComboBox    ->setEnabled(ui->FrontoupComboBox->currentIndex()>0);
    ui->PortRefracteurupComboBox->setEnabled(ui->RefracteurupComboBox->currentIndex()>0);
    ui->PortTonometreupComboBox ->setEnabled(ui->TonometreupComboBox->currentIndex()>0);
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

void dlg_param::EnableModif(QWidget *obj)
{
    if (obj == ui->LockParamPosteupLabel)
    {
        if (ui->LockParamPosteupLabel->pixmap()->toImage() == Icons::pxVerrouiller().toImage())
        {
            m_MDPadminverifie = Utils::VerifMDP(proc->MDPAdmin(),"Saisissez le mot de passe Administrateur", m_MDPadminverifie);
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
            m_MDPuserverifie = Utils::VerifMDP(currentuser()->password(),tr("Saisissez votre mot de passe"), m_MDPuserverifie);
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
        PosteConnecte* post = Datas::I()->postesconnectes->admin(Item::NoUpdate);
        if (post != Q_NULLPTR)
        {
            UpMessageBox::Watch(this,tr("Vous ne pouvez pas modifier les paramètres généraux"),
                                     tr("Une session de RufusAdmin est actuellement active sur le poste") + " "
                                     + post->nomposte());
            return;
        }
        if (ui->LockParamGeneralupLabel->pixmap()->toImage() == Icons::pxVerrouiller().toImage())
        {
            m_MDPadminverifie = Utils::VerifMDP(proc->MDPAdmin(),tr("Saisissez le mot de passe Administrateur"), m_MDPadminverifie);
            if (m_MDPadminverifie)
            {
                ui->LockParamGeneralupLabel ->setPixmap(Icons::pxDeverouiller());
                wdg_appareilswdgbuttonframe->wdg_moinsBouton      ->setEnabled(ui->AppareilsConnectesupTableWidget->selectedItems().size()>0);
            }
        }
        else
            ui->LockParamGeneralupLabel->setPixmap(Icons::pxVerrouiller());
        bool a = (ui->LockParamGeneralupLabel->pixmap()->toImage() == Icons::pxDeverouiller().toImage());
        if (db->ModeAccesDataBase() == Utils::Distant)
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
        EnableWidgContent(ui->Sauvegardeframe, db->ModeAccesDataBase() == Utils::Poste && a);
    }
}

void dlg_param::EnableFrameServeur(QCheckBox *box, bool a)
{
    if (box == ui->PosteServcheckBox)
    {
        ui->Posteframe                  ->setVisible(a);
        ui->MonoConnexionupLabel        ->setVisible(a);
        ui->MonoDocsExtupLabel          ->setVisible(a);
        ui->MonoDocupTableWidget        ->setVisible(a);
        ui->Posteframe                  ->setEnabled(a);
        ui->MonoConnexionupLabel        ->setEnabled(a);
        ui->MonoDocsExtupLabel          ->setEnabled(a);
        ui->MonoDocupTableWidget        ->setEnabled(a);
        ui->PosteStockageupLabel        ->setVisible(a && DataBase::I()->ModeAccesDataBase() == Utils::Poste);
        ui->PosteStockageupLineEdit     ->setVisible(a && DataBase::I()->ModeAccesDataBase() == Utils::Poste);
        ui->PosteStockageupPushButton   ->setVisible(a && DataBase::I()->ModeAccesDataBase() == Utils::Poste);
    }
    else if (box == ui->LocalServcheckBox)
    {
        ui->Localframe                      ->setVisible(a);
        ui->LocalConnexionupLabel           ->setVisible(a);
        ui->LocalDocsExtupLabel             ->setVisible(a);
        ui->LocalDocupTableWidget           ->setVisible(a);
        ui->LocalPathStockageupLabel        ->setVisible(a);
        ui->LocalPathStockageupLineEdit     ->setVisible(a);
        ui->LocalPathStockageupPushButton   ->setVisible(a);
        ui->Localframe                      ->setEnabled(a);
        ui->LocalConnexionupLabel           ->setEnabled(a);
        ui->LocalDocsExtupLabel             ->setEnabled(a);
        ui->LocalDocupTableWidget           ->setEnabled(a);
        ui->LocalPathStockageupLabel        ->setEnabled(a);
        ui->LocalPathStockageupLineEdit     ->setEnabled(a);
        ui->LocalPathStockageupPushButton   ->setEnabled(a);
    }
    else if (box == ui->DistantServcheckBox)
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
     }
}

void dlg_param::EnableOKModifPosteButton()
{
    m_modifposte = true;
}

void dlg_param::FiltreActesOphtaSeulmt(bool b)
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

void dlg_param::GestionBanques()
{
    dlg_gestionbanques *Dlg_Banq = new dlg_gestionbanques();
    Dlg_Banq->exec();
}

void dlg_param::GestionDatasCurrentUser()
{
    dlg_gestionusers *Dlg_GestUsr = new dlg_gestionusers(Datas::I()->sites->idcurrentsite(), dlg_gestionusers::MODIFUSER, m_MDPuserverifie);
    Dlg_GestUsr->setWindowTitle(tr("Enregistrement de l'utilisateur ") +  currentuser()->login());
    m_donneesusermodifiees = (Dlg_GestUsr->exec()>0);
    if(m_donneesusermodifiees)
    {
        Datas::I()->users->reload(currentuser());
        proc->MAJComptesBancaires(currentuser());
        AfficheParamUser();
    }
    if (!m_MDPuserverifie)
        m_MDPuserverifie = Dlg_GestUsr->isMDPverified();
    delete Dlg_GestUsr;
}

void dlg_param::GestionUsers()
{
    dlg_gestionusers *Dlg_GestUsr = new dlg_gestionusers(Datas::I()->sites->idcurrentsite(), dlg_gestionusers::ADMIN, m_MDPadminverifie);
    Dlg_GestUsr->setWindowTitle(tr("Gestion des utilisateurs"));
    m_donneesusermodifiees = (Dlg_GestUsr->exec()>0);
    if(m_donneesusermodifiees)
    {
        Datas::I()->users   ->initListe();
        proc                ->MAJComptesBancaires(currentuser());
        AfficheParamUser();
    }
    delete Dlg_GestUsr;
    UpMessageBox::Watch(this, tr("Donnes utilisateurs modifiées?"),
                              tr("Si vous avez modifié des données d'utilisateurs actuellement connectés,\n"
                                 "chacun de ces utilisateurs doit relancer le programme\n"
                                 "pour pouvoir prendre en compte les modifications apportées!"));
}

void dlg_param::GestionLieux()
{
    dlg_listelieux *gestLieux = new dlg_listelieux();
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
    QStandardItemModel *mod = new QStandardItemModel(this);
    mod->setHorizontalHeaderLabels(list);
    upheader->setModel(mod);
    upheader->reDim(0,0,2);

    QList<Site*> listsites = Datas::I()->sites->initListeByUser(user->id());
    ui->AdressupTableWidget->setRowCount(listsites.size());
    int i = 0;
    foreach (Site *sit, listsites)
    {
        QTableWidgetItem *pitem1, *pitem2, *pitem3;
        pitem1 = new QTableWidgetItem();
        pitem2 = new QTableWidgetItem();
        pitem3 = new QTableWidgetItem();
        pitem1->setText(sit->nom());
        pitem2->setText(sit->ville());
        pitem3->setText(sit->telephone());
        ui->AdressupTableWidget->setItem(i,0,pitem1);
        ui->AdressupTableWidget->setItem(i,1,pitem2);
        ui->AdressupTableWidget->setItem(i,2,pitem3);
        pitem1->setToolTip(sit->coordonnees());
        pitem2->setToolTip(sit->coordonnees());
        pitem3->setToolTip(sit->coordonnees());
        if (sit->couleur() != "")
        {
            pitem1->setTextColor(QColor("#" + sit->couleur()));
            pitem2->setTextColor(QColor("#" + sit->couleur()));
            pitem3->setTextColor(QColor("#" + sit->couleur()));
        }
        ui->AdressupTableWidget->setRowHeight(i,int(QFontMetrics(qApp->font()).height()*1.3));
        ++i;
    }
    ReconstruitListeLieuxExerciceAllusers();
    /*-------------------- GESTION DES LIEUX D'EXRCICE-------------------------------------------------------*/
}

void dlg_param::ReconstruitListeLieuxExerciceAllusers()
{
    disconnect(ui->EmplacementServeurupComboBox,    QOverload<int>::of(&QComboBox::currentIndexChanged),    this,   &dlg_param::EnregistreEmplacementServeur);
    ui->EmplacementServeurupComboBox->clear();
    foreach (Site* sit, *Datas::I()->sites->sites())
    {
        ui->EmplacementServeurupComboBox->addItem(sit->nom() + " " + sit->ville(), sit->id());
        if (m_parametres->idlieupardefaut()>0)
            ui->EmplacementServeurupComboBox->setCurrentIndex(ui->EmplacementServeurupComboBox->findData(m_parametres->idlieupardefaut()));
        else
        {
            ui->EmplacementServeurupComboBox->setCurrentIndex(0);
            EnregistreEmplacementServeur(0);
        }
    }
    connect(ui->EmplacementServeurupComboBox,       QOverload<int>::of(&QComboBox::currentIndexChanged),    this,   &dlg_param::EnregistreEmplacementServeur);
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
    connect(dlg_askappareil->OKButton,    &QPushButton::clicked,    this,   &dlg_param::EnregistreAppareil);
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

void dlg_param::MAJActesCCAM(QWidget * widg, QString txt)
{
    QString req;
    UpCheckBox* check = dynamic_cast<UpCheckBox*>(widg);
    if (check)
    {
        int row = check->rowTable();
        QString codeccam = ui->ActesCCAMupTableWidget->item(row,1)->text();
        QString montantpratique="";
        if (check->checkState() == Qt::Unchecked)
        {
            req = "delete from " TBL_COTATIONS " where typeacte = '" + codeccam + "' and idUser = " + QString::number(currentuser()->id());
        }
        else
        {
            int secteur = currentuser()->secteurconventionnel();
            if (secteur>1)
            {
                UpLineEdit *line = dynamic_cast<UpLineEdit*>(ui->ActesCCAMupTableWidget->cellWidget(row,5));
                if (line)
                    montantpratique = QString::number(QLocale().toDouble(line->text()));
                else
                {
                    UpLineEdit *lbl = new UpLineEdit();
                    if (currentuser()->isOPTAM())
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
                    connect(lbl,    &UpLineEdit::TextModified,  this,   [=] (QString txt) {MAJActesCCAM(lbl, txt);});
                    ui->ActesCCAMupTableWidget->setCellWidget(row,5,lbl);
                    montantpratique = QString::number(QLocale().toDouble(lbl->text()));
                }
             }
            else if (currentuser()->isOPTAM())
                montantpratique = QString::number(QLocale().toDouble(ui->ActesCCAMupTableWidget->item(row,2)->text()));
            else
                montantpratique = QString::number(QLocale().toDouble(ui->ActesCCAMupTableWidget->item(row,3)->text()));

            req = "insert into " TBL_COTATIONS " (typeacte, MontantOPTAM, MontantNonOPTAM, montantpratique, CCAM, iduser) values ('" +
                    codeccam + "', " +
                    QString::number(QLocale().toDouble(ui->ActesCCAMupTableWidget->item(row,2)->text())) + ", " +
                    QString::number(QLocale().toDouble(ui->ActesCCAMupTableWidget->item(row,3)->text())) + ", " +
                    montantpratique + ", 1," +
                    QString::number(currentuser()->id()) +")";
        }
        if (db->StandardSQL(req))
            m_cotationsmodifiees = true;
    }
    else
    {
        UpLineEdit *line = dynamic_cast<UpLineEdit*>(widg);
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
                                  " where typeacte = '" + ui->ActesCCAMupTableWidget->item(row,1)->text() + "' and idUser = " + QString::number(currentuser()->id());
                    if (db->StandardSQL(req))
                        m_cotationsmodifiees = true;
                }
        }
    }
}

void dlg_param::MAJAssocCCAM(QWidget *widg, QString txt)
{
    bool ok;
    QString req;
    UpCheckBox* check = dynamic_cast<UpCheckBox*>(widg);
    if (check)
    {
        int row                 = check->rowTable();
        QString codeccam        = ui->AssocCCAMupTableWidget->item(row,1)->text();
        QString montantpratique = "";
        if (check->checkState() == Qt::Unchecked)
        {
            QList<QVariantList> calclist = db->StandardSelectSQL("select typeacte from " TBL_COTATIONS " where typeacte = '" + codeccam + "'", ok);
            req = "delete from " TBL_COTATIONS " where typeacte = '" + codeccam + "' and idUser = " + QString::number(currentuser()->id());
            if (calclist.size()==1)
            {
                if (UpMessageBox::Question(this,tr("Suppression de cotation"),
                                            tr("Vous étiez le seul à utiliser cette cotation") + "\n" +
                                            tr("Voulez-vous la supprimer définitvement?"),
                                            UpDialog::ButtonCancel | UpDialog::ButtonSuppr,
                                            QStringList() << tr("Annuler") << tr("Supprimer la cotation") + " " + codeccam)
                    != UpSmallButton::SUPPRBUTTON)
                    req = "update " TBL_COTATIONS " set idUser = NULL where typeacte = '" + codeccam + "' and idUser = " + QString::number(currentuser()->id());
            }
        }
        else
        {
            int secteur = currentuser()->secteurconventionnel();
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
                    if (currentuser()->isOPTAM())
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
                    connect(lbl,    &UpLineEdit::TextModified,  this,   [=] (QString txt) {MAJAssocCCAM(lbl, txt);});
                    ui->AssocCCAMupTableWidget->setCellWidget(row,4,lbl);
                    montantpratique = QString::number(QLocale().toDouble(lbl->text()));
                }
            }
            else if (currentuser()->isOPTAM())
                montantpratique = montantOPTAM;
            else
                montantpratique = montantNonOPTAM;
            req = "insert into " TBL_COTATIONS " (typeacte, MontantOPTAM, MontantNonOPTAM, montantpratique, CCAM, iduser) values ('" +
                    codeccam + "', " +
                    montantOPTAM + ", " +
                    montantNonOPTAM + ", " +
                    montantpratique + ", 2," +
                    QString::number(currentuser()->id()) +")";
        }
        if (db->StandardSQL(req))
            m_cotationsmodifiees = true;
    }
    else
    {
        UpLineEdit *line = dynamic_cast<UpLineEdit*>(widg);
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
                            " where typeacte = '" + ui->AssocCCAMupTableWidget->item(row,1)->text() + "' and idUser = " + QString::number(currentuser()->id());
                    else if (line->Column()==3)
                       req = "update " TBL_COTATIONS " set montantnonoptam = " + montant +
                           " where typeacte = '" + ui->AssocCCAMupTableWidget->item(row,1)->text() + "' and idUser = " + QString::number(currentuser()->id());
                    else if (line->Column()==4)
                       req = "update " TBL_COTATIONS " set montantpratique = " + montant +
                           " where typeacte = '" + ui->AssocCCAMupTableWidget->item(row,1)->text() + "' and idUser = " + QString::number(currentuser()->id());
                    if (db->StandardSQL(req))
                        m_cotationsmodifiees = true;
                }
        }
    }
}

void dlg_param::MAJHorsNomenclature(QWidget *widg, QString txt)
{
    QString req;
    UpCheckBox* check = dynamic_cast<UpCheckBox*>(widg);
    if (check)
    {
        int row                 = check->rowTable();
        QString codeccam        = ui->HorsNomenclatureupTableWidget->item(row,1)->text();
        QString montantpratique = "";
        if (check->checkState() == Qt::Unchecked)
            req = "delete from " TBL_COTATIONS " where typeacte = '" + codeccam + "' and idUser = " + QString::number(currentuser()->id());
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
                    " 2, " + QString::number(currentuser()->id()) +")";
        }
        if (db->StandardSQL(req))
            m_cotationsmodifiees = true;
    }
    else
    {
        UpLineEdit *line = dynamic_cast<UpLineEdit*>(widg);
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
                          " where typeacte = '" + ui->HorsNomenclatureupTableWidget->item(row,1)->text() + "' and idUser = " + QString::number(currentuser()->id());
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
    QString req = " select list." CP_TITREEXAMEN_APPAREIL ", list." CP_NOMAPPAREIL_APPAREIL " from " TBL_LISTEAPPAREILS " list, " TBL_APPAREILSCONNECTESCENTRE " appcon"
                  " where list." CP_ID_APPAREIL " = appcon." CP_IDAPPAREIL_APPAREILS
                  " and list." CP_ID_APPAREIL " = " + ui->AppareilsConnectesupTableWidget->selectedItems().at(0)->text();
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
        req = "delete from " TBL_APPAREILSCONNECTESCENTRE " where " CP_IDAPPAREIL_APPAREILS " = "
              + ui->AppareilsConnectesupTableWidget->selectedItems().at(0)->text()
              + " and " CP_IDLIEU_APPAREILS " = " + QString::number(Datas::I()->sites->idcurrentsite());
        db->StandardSQL(req);
        proc->settings()->remove(Utils::getBaseFromMode(db->ModeAccesDataBase()) + "/DossiersDocuments/" + appdata.at(1).toString());
        Remplir_Tables();
    }
}

void dlg_param::RegleAssocBoutons(QWidget *widg)
{
    bool modifboutonsActes  = false;
    bool modifboutonsAssoc  = false;
    bool modifboutonsHN     = false;


    UpCheckBox* check0      = dynamic_cast<UpCheckBox*>(widg);
    if (check0)
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

    if (widg == ui->AssocCCAMupTableWidget || modifboutonsAssoc)
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
    else if (widg == ui->ActesCCAMupTableWidget || modifboutonsActes)
    {
        ui->AssocCCAMupTableWidget          ->clearSelection();
        ui->HorsNomenclatureupTableWidget   ->clearSelection();
        wdg_assocCCAMcotationswdgbuttonframe->wdg_modifBouton          ->setEnabled(false);
        wdg_assocCCAMcotationswdgbuttonframe->wdg_moinsBouton          ->setEnabled(false);
        wdg_HNcotationswdgbuttonframe->wdg_modifBouton                 ->setEnabled(false);
        wdg_HNcotationswdgbuttonframe->wdg_moinsBouton                 ->setEnabled(false);
    }
    else if (widg == ui->HorsNomenclatureupTableWidget || modifboutonsHN)
    {
        ui->ActesCCAMupTableWidget          ->clearSelection();
        ui->AssocCCAMupTableWidget          ->clearSelection();
        bool checked = true;
        if (check0)
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

void dlg_param::ResetImprimante()
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
    EnableOKModifPosteButton();
}

void dlg_param::EnregistreAppareil()
{
    if (!dlg_askappareil) return;
    QString req = "insert into " TBL_APPAREILSCONNECTESCENTRE " (" CP_IDAPPAREIL_APPAREILS ", " CP_IDLIEU_APPAREILS ") Values("
                  " (select " CP_ID_APPAREIL " from " TBL_LISTEAPPAREILS " where " CP_NOMAPPAREIL_APPAREIL " = '" + dlg_askappareil->findChildren<UpComboBox*>().at(0)->currentText() + "'), "
                  + QString::number(Datas::I()->sites->idcurrentsite()) + ")";
    db->StandardSQL(req);
    dlg_askappareil->done(0);
    Remplir_Tables();
}

void dlg_param::EnregistreEmplacementServeur(int idx)
{
    int idlieu = ui->EmplacementServeurupComboBox->itemData(idx).toInt();
    if (ui->EmplacementServeurupComboBox->itemData(idx).toString() != "")
    {
        db->setidlieupardefaut(idlieu);
        Site *sit = Datas::I()->sites->getById(idlieu);
        if (sit != Q_NULLPTR)
            ui->AppareilsconnectesupLabel->setText(tr("Appareils connectés au réseau") + " <font color=\"green\"><b>" + sit->nom() + "</b></font> ");
    }
}

void dlg_param::NouvAssocCCAM()
{
    dlg_gestioncotations *Dlg_CrrCot = new dlg_gestioncotations(dlg_gestioncotations::Association, dlg_gestioncotations::Creation);
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
    dlg_gestioncotations *Dlg_CrrCot = new dlg_gestioncotations(dlg_gestioncotations::Association, dlg_gestioncotations::Modification, CodeActe);
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
                  " and iduser <> " + QString::number(currentuser()->id());
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
    dlg_gestioncotations *Dlg_CrrCot = new dlg_gestioncotations(dlg_gestioncotations::HorsNomenclature, dlg_gestioncotations::Creation);
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
    dlg_gestioncotations *Dlg_CrrCot = new dlg_gestioncotations(dlg_gestioncotations::HorsNomenclature, dlg_gestioncotations::Modification, CodeActe);
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

void dlg_param::ModifMDPAdmin()
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
    connect(dlg_askMDP->OKButton,    &QPushButton::clicked, this, &dlg_param::EnregistreNouvMDPAdmin);
    dlg_askMDP->dlglayout()->setSizeConstraint(QLayout::SetFixedSize);

    dlg_askMDP->exec();
}

void dlg_param::ParamMotifs()
{
    dlg_motifs *Dlg_motifs = new dlg_motifs();
    Dlg_motifs->setWindowTitle(tr("Motifs de consultations"));
    Dlg_motifs->exec();
    delete Dlg_motifs;
}

void dlg_param::ModifDirBackup()
{
    if (db->ModeAccesDataBase() != Utils::Poste)
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

void dlg_param::ModifPathDirEchangeMesure(Mesure mesure)
{
    QString dirmesureorigin ("");
    QString dirmesure ("");
    switch (mesure) {
    case Fronto:
        dirmesureorigin   = ui->DirBackupuplineEdit->text();
        dirmesure         = QFileDialog::getExistingDirectory(this,tr("Choisissez le dossier d'enregistrement provisoire des mesures du frontofocomètre\n"
                                                                    "Le nom de dossier ne doit pas contenir d'espace"), QDir::homePath());
        if (dirmesure.contains(" "))
            UpMessageBox::Watch(this, tr("Nom de dossier non conforme"),tr("Vous ne pouvez pas choisir un dossier dont le nom contient des espaces"));
        if (dirmesureorigin == dirmesure || dirmesure.contains(" "))
            return;

        ui->NetworkPathFrontoupLineEdit ->setText(dirmesure);
        ui->NetworkPathFrontoupLineEdit ->setImmediateToolTip(dirmesure);
        proc->settings()->setValue("Param_Poste/PortFronto/Reseau", ui->NetworkPathFrontoupLineEdit->text());
        break;
    case Autoref:
        dirmesureorigin   = ui->DirBackupuplineEdit->text();
        dirmesure         = QFileDialog::getExistingDirectory(this,tr("Choisissez le dossier d'enregistrement provisoire des mesures de l'autorefractomètre\n"
                                                                    "Le nom de dossier ne doit pas contenir d'espace"), QDir::homePath());
        if (dirmesure.contains(" "))
            UpMessageBox::Watch(this, tr("Nom de dossier non conforme"),tr("Vous ne pouvez pas choisir un dossier dont le nom contient des espaces"));
        if (dirmesureorigin == dirmesure || dirmesure.contains(" "))
            return;

        ui->NetworkPathAutorefupLineEdit ->setText(dirmesure);
        ui->NetworkPathAutorefupLineEdit ->setImmediateToolTip(dirmesure);
        proc->settings()->setValue("Param_Poste/PortAutoref/Reseau", ui->NetworkPathAutorefupLineEdit->text());
        break;
    case Refracteur:
        dirmesureorigin   = ui->DirBackupuplineEdit->text();
        dirmesure         = QFileDialog::getExistingDirectory(this,tr("Choisissez le dossier d'enregistrement provisoire des mesures du refracteur\n"
                                                                    "Le nom de dossier ne doit pas contenir d'espace"), QDir::homePath());
        if (dirmesure.contains(" "))
            UpMessageBox::Watch(this, tr("Nom de dossier non conforme"),tr("Vous ne pouvez pas choisir un dossier dont le nom contient des espaces"));
        if (dirmesureorigin == dirmesure || dirmesure.contains(" "))
            return;

        ui->NetworkPathRefracteurupLineEdit ->setText(dirmesure);
        ui->NetworkPathRefracteurupLineEdit ->setImmediateToolTip(dirmesure);
        proc->settings()->setValue("Param_Poste/PortRefracteur/Reseau", ui->NetworkPathRefracteurupLineEdit->text());
        break;
    case Tono:
        dirmesureorigin   = ui->DirBackupuplineEdit->text();
        dirmesure         = QFileDialog::getExistingDirectory(this,tr("Choisissez le dossier d'enregistrement provisoire des mesures du tonomètre\n"
                                                                    "Le nom de dossier ne doit pas contenir d'espace"), QDir::homePath());
        if (dirmesure.contains(" "))
            UpMessageBox::Watch(this, tr("Nom de dossier non conforme"),tr("Vous ne pouvez pas choisir un dossier dont le nom contient des espaces"));
        if (dirmesureorigin == dirmesure || dirmesure.contains(" "))
            return;

        ui->NetworkPathTonoupLineEdit ->setText(dirmesure);
        ui->NetworkPathTonoupLineEdit ->setImmediateToolTip(dirmesure);
        proc->settings()->setValue("Param_Poste/PortTonometre/Reseau", ui->NetworkPathTonoupLineEdit->text());
        break;
    }
}

void dlg_param::ModifPathEchangeAppareilMesure(Mesure mesure)
{
    QString dirmesureorigin ("");
    QString dirmesure ("");
    switch (mesure) {
    case Fronto:
        dirmesureorigin   = ui->DirBackupuplineEdit->text();
        dirmesure         = QFileDialog::getExistingDirectory(this,tr("Choisissez le dossier d'enregistrement provisoire des mesures du frontofocometre\n"
                                                                    "Le nom de dossier ne doit pas contenir d'espace"), QDir::homePath());
        if (dirmesure.contains(" "))
        {
            UpMessageBox::Watch(this, tr("Nom de dossier non conforme"),tr("Vous ne pouvez pas choisir un dossier dont le nom contient des espaces"));
            return;
        }
        if (dirmesureorigin == dirmesure)
        {
            UpMessageBox::Watch(this, tr("Nom de dossier non conforme"),tr("Le dossier doit être différent du dossier de sauvegarde de la base"));
            return;
        }
        ui->NetworkPathEchangeFrontoupLineEdit ->setText(dirmesure);
        ui->NetworkPathEchangeFrontoupLineEdit ->setImmediateToolTip(dirmesure);
        proc->settings()->setValue("Param_Poste/PortRefracteur/Reseau/AdressFronto", dirmesure);
        break;
    case Autoref:
        dirmesureorigin   = ui->DirBackupuplineEdit->text();
        dirmesure         = QFileDialog::getExistingDirectory(this,tr("Choisissez le dossier d'enregistrement provisoire des mesures de l'autorefractometre\n"
                                                                    "Le nom de dossier ne doit pas contenir d'espace"), QDir::homePath());
        if (dirmesure.contains(" "))
        {
            UpMessageBox::Watch(this, tr("Nom de dossier non conforme"),tr("Vous ne pouvez pas choisir un dossier dont le nom contient des espaces"));
            return;
        }
        if (dirmesureorigin == dirmesure)
        {
            UpMessageBox::Watch(this, tr("Nom de dossier non conforme"),tr("Le dossier doit être différent du dossier de sauvegarde de la base"));
            return;
        }
        ui->NetworkPathEchangeAutorefupLineEdit ->setText(dirmesure);
        ui->NetworkPathEchangeAutorefupLineEdit ->setImmediateToolTip(dirmesure);
        proc->settings()->setValue("Param_Poste/PortRefracteur/Reseau/AdressAutoref", dirmesure);
        break;
    case Refracteur:
    case Tono:
        break;
    }
}

void dlg_param::ModifDateBackup()    //Modification de la date du backup
{
    Utils::Days days;
    days.setFlag(Utils::Lundi,      ui->LundiradioButton->isChecked());
    days.setFlag(Utils::Mardi,      ui->MardiradioButton->isChecked());
    days.setFlag(Utils::Mercredi,   ui->MercrediradioButton->isChecked());
    days.setFlag(Utils::Jeudi,      ui->JeudiradioButton->isChecked());
    days.setFlag(Utils::Vendredi,   ui->VendrediradioButton->isChecked());
    days.setFlag(Utils::Samedi,     ui->SamediradioButton->isChecked());
    days.setFlag(Utils::Dimanche,   ui->DimancheradioButton->isChecked());
    db->setdaysbkup(days);
    proc->ParamAutoBackup();
    ui->EffacePrgSauvupPushButton->setEnabled(m_parametres->daysbkup()
                                           && QDir(m_parametres->dirbkup()).exists()
                                           && m_parametres->dirbkup() != ""
                                           && m_parametres->heurebkup() != QTime());
}

void dlg_param::ModifHeureBackup()    //Modification de la date du backup
{
    db->setheurebkup(ui->HeureBackuptimeEdit->time());
    proc->ParamAutoBackup();
    ui->EffacePrgSauvupPushButton->setEnabled(m_parametres->daysbkup()
                                           && QDir(m_parametres->dirbkup()).exists()
                                           && m_parametres->dirbkup() != ""
                                           && m_parametres->heurebkup() != QTime());
}

void dlg_param::DirLocalStockage()
{
    /*! il faut utiliser la fonction static QFileDialog::getExistingDirectoryUrl() parce que la QFileDialog implémentée dans Qt ne donne pas accès aux lecteurs réseaux sous linux
     * avec la fonction static, on utilise la boîte de dialog du système
     * bien sûr, il faut paramétrer le fstab sous linux pour que le dossier réseau soit ouvert automatiquement au moment du boot*/
    QString dir = proc->settings()->value(Utils::getBaseFromMode(Utils::ReseauLocal) + "/DossierImagerie").toString();
    if (dir == "")
        dir = PATH_DIR_RUFUS;
    QUrl url = QFileDialog::getExistingDirectoryUrl(this, "", dir, QFileDialog::ShowDirsOnly);
    ui->LocalPathStockageupLineEdit->setText(url.path());
    proc->settings()->setValue(Utils::getBaseFromMode(Utils::ReseauLocal) + "/DossierImagerie", url.path());
}

void dlg_param::DirDistantStockage()
{
    /*! il faut utiliser la fonction static QFileDialog::getExistingDirectoryUrl() parce que la QFileDialog implémentée dans Qt ne donne pas accès aux lecteurs réseaux sous linux
     * avec la fonction static, on utilise la boîte de dialog du système
     * bien sûr, il faut paramétrer le fstab sous linux pour que le dossier réseau soit ouvert automatiquement au moment du boot*/
    QString dir = proc->settings()->value(Utils::getBaseFromMode(Utils::Distant) + "/DossierImagerie").toString();
    if (dir == "")
        dir = PATH_DIR_RUFUS;
    QUrl url = QFileDialog::getExistingDirectoryUrl(this, "", dir, QFileDialog::ShowDirsOnly);
    ui->DistantStockageupLineEdit->setText(url.path());
    proc->settings()->setValue(Utils::getBaseFromMode(Utils::Distant) + "/DossierImagerie", url.path());
}

void dlg_param::DirPosteStockage()
{
    if (db->ModeAccesDataBase() != Utils::Poste)
    {
        UpMessageBox::Watch(this, tr("Impossible de modifier ce paramètre"), tr("Vous devez être connecté sur le serveur de ce poste pour\npouvoir modifier le répertoire de stockage des documents"));
        return;
    }
    /*! il faut utiliser la fonction static QFileDialog::getExistingDirectoryUrl() parce que la QFileDialog implémentée dans Qt ne donne pas accès aux lecteurs réseaux sous linux
     * avec la fonction static, on utilise la boîte de dialog du système
     * bien sûr, il faut paramétrer le fstab sous linux pour que le dossier réseau soit ouvert automatiquement au moment du boot*/
    QString dir = ui->PosteStockageupLineEdit->text();
    if (dir == "")
        dir = PATH_DIR_RUFUS;
    QUrl url = QFileDialog::getExistingDirectoryUrl(this, "", dir, QFileDialog::ShowDirsOnly);
    if (!QDir::match(PATH_DIR_RUFUS "/*", url.path()))
    {
        UpMessageBox::Watch(this, tr("Vous devez choisir un sous-dossier du dossier Rufus"), PATH_DIR_RUFUS);
        return;
    }
    ui->PosteStockageupLineEdit->setText(url.path());
    db->setdirimagerie(url.path());
    proc->settings()->setValue(Utils::getBaseFromMode(Utils::Poste) + "/DossierImagerie", url.path());
}

void dlg_param::DossierClesSSL()
{
    QString dir = proc->settings()->value(Utils::getBaseFromMode(Utils::Distant) + "/DossierClesSSL").toString();
    if (dir == "")
    {
        dir = "/etc/mysql";
        proc->settings()->setValue(Utils::getBaseFromMode(Utils::Distant) + "/DossierClesSSL", dir);
    }
    QFileDialog dialog(this, "", dir);
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setViewMode(QFileDialog::List);
    if (dialog.exec()>0)
    {
        QDir dockdir = dialog.directory();
        ui->DossierClesSSLupLineEdit->setText(dockdir.path());
        proc->settings()->setValue(Utils::getBaseFromMode(Utils::Distant) + "/DossierClesSSL", dockdir.path());
    }
}

void dlg_param::EffaceProgrammationDataBackup()
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
    if (ui->PosteServcheckBox->isChecked() && db->ModeAccesDataBase() == Utils::Poste)
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
    if (ui->LocalServcheckBox->isChecked() && db->ModeAccesDataBase() == Utils::ReseauLocal)
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
            if (!QDir(ui->LocalPathStockageupLineEdit->text()).exists() || ui->LocalPathStockageupLineEdit->text() == "")
            {
                UpMessageBox::Watch(this,tr("Vous n'avez pas spécifié de dossier de stockage valide pour les documents d'imagerie !"));
                ui->ParamtabWidget              ->setCurrentWidget(ui->PosteParamtab);
                ui->ParamConnexiontabWidget     ->setCurrentWidget(ui->tabLocal);
                ui->LocalPathStockageupLineEdit ->setFocus();
                return false;
            }
        }
    }
    if (ui->DistantServcheckBox->isChecked() && db->ModeAccesDataBase() == Utils::Distant)
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
    PosteConnecte* post = Datas::I()->postesconnectes->admin(Item::NoUpdate);
    ui->LockParamGeneralupLabel->setEnabled(post == Q_NULLPTR);
    QString A = proc->PosteImportDocs();
    if (A == "")
    {
        ui->PosteImportDocslabel->setText(tr("Pas de poste paramétré"));
        ui->PosteImportDocsPrioritairelabel->setText("");
    }
    else
    {
        A = "<font color=\"green\"><b>" + A.remove(".local") + "</b></font>";
        QString B;
        if (A.contains(" - " NOM_ADMINISTRATEUR))
            B = tr("Administrateur");
        else
            B = (A.contains(" - prioritaire")? tr("prioritaire") : tr("non prioritaire"));
        B = "<b>" + B + "</b>";
        A.remove(" - prioritaire");
        A.remove(" - " NOM_ADMINISTRATEUR);

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
            ui->FermepushButton->setIcon(Icons::icFerme());
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
    ui->idUseruplineEdit                ->setText(QString::number(currentuser()->id()));
    ui->LoginuplineEdit                 ->setText(currentuser()->login());
    ui->MDPuplineEdit                   ->setText(currentuser()->password());
    ui->NomuplineEdit                   ->setText(currentuser()->nom());
    ui->PrenomuplineEdit                ->setText(currentuser()->prenom());
    QList<QVariantList> listlieux = db->StandardSelectSQL("select idlieu from " TBL_JOINTURESLIEUX " where iduser = " + QString::number(currentuser()->id()), ok);
    QList<int> idlieuxlist;
    for (int k=0; k< listlieux.size(); k++)
        idlieuxlist << listlieux.at(k).at(0).toInt();

    ui->PortableuplineEdit              ->setText(currentuser()->portable());
    ui->MailuplineEdit                  ->setText(currentuser()->mail());
    ui->Titrelabel                      ->setVisible(currentuser()->isMedecin());
    ui->Cotationswidget                 ->setVisible(!currentuser()->isSecretaire() && !currentuser()->isAutreFonction());

//    ui->ModeExercicegroupBox->setVisible(false);
    bool soccomptable   = currentuser()->isSocComptable();
    bool medecin        = currentuser()->isMedecin();

    ui->StatutComptaupTextEdit      ->setText(proc->SessionStatus());

    ui->TitreuplineEdit             ->setVisible(medecin);
    ui->Titrelabel                  ->setVisible(medecin);
    ui->Prenomlabel                 ->setVisible(!soccomptable);
    ui->PrenomuplineEdit            ->setVisible(!soccomptable);

    ui->idUseruplineEdit            ->setText(QString::number(currentuser()->id()));
    ui->LoginuplineEdit             ->setText(currentuser()->login());
    ui->MDPuplineEdit               ->setText(currentuser()->password());
    if (medecin)
        ui->TitreuplineEdit         ->setText(currentuser()->titre());

    if (!currentuser()->isSecretaire() && !currentuser()->isAutreFonction())
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
    ReconstruitListeLieuxExerciceUser(currentuser());
}

void dlg_param::ConnectSignals()
{
    connect(ui->FermepushButton,                    &QPushButton::clicked,                  this,   &dlg_param::FermepushButtonClicked);
    connect(ui->InitMDPAdminpushButton,             &QPushButton::clicked,                  this,   &dlg_param::ModifMDPAdmin);
    connect(ui->ChoixFontupPushButton,              &QPushButton::clicked,                  this,   &dlg_param::ChoixFontpushButtonClicked);
    connect(ui->PosteServcheckBox,                  &QCheckBox::clicked,                    this,   [=] (bool a) {EnableFrameServeur(ui->PosteServcheckBox, a);});
    connect(ui->LocalServcheckBox,                  &QCheckBox::clicked,                    this,   [=] (bool a) {EnableFrameServeur(ui->LocalServcheckBox, a);});
    connect(ui->DistantServcheckBox,                &QCheckBox::clicked,                    this,   [=] (bool a) {EnableFrameServeur(ui->DistantServcheckBox, a);});
    connect(ui->GestUserpushButton,                 &QPushButton::clicked,                  this,   &dlg_param::GestionUsers);
    connect(ui->GestLieuxpushButton,                &QPushButton::clicked,                  this,   &dlg_param::GestionLieux);
    connect(ui->ModifDataUserpushButton,            &QPushButton::clicked,                  this,   &dlg_param::GestionDatasCurrentUser);
    connect(ui->GestionBanquespushButton,           &QPushButton::clicked,                  this,   &dlg_param::GestionBanques);
    connect(ui->OupspushButton,                     &QPushButton::clicked,                  this,   &dlg_param::ResetImprimante);
    connect(ui->LocalPathStockageupPushButton,      &QPushButton::clicked,                  this,   &dlg_param::DirLocalStockage);
    connect(ui->DistantStockageupPushButton,        &QPushButton::clicked,                  this,   &dlg_param::DirDistantStockage);
    connect(ui->DossierCLesSSLupPushButton,         &QPushButton::clicked,                  this,   &dlg_param::DossierClesSSL);
    connect(ui->PosteStockageupPushButton,          &QPushButton::clicked,                  this,   &dlg_param::DirPosteStockage);
    connect(ui->AppareilsConnectesupTableWidget,    &QTableWidget::itemSelectionChanged,    this,   &dlg_param::EnableSupprAppareilBouton);
    connect(ui->AutorefupComboBox,                  QOverload<int>::of(&QComboBox::currentIndexChanged),
                                                                                            this,   [=] (int a) {ClearCom(ui->AutorefupComboBox,a);});
    connect(ui->TonometreupComboBox,                QOverload<int>::of(&QComboBox::currentIndexChanged),
                                                                                            this,   [=] (int a) {ClearCom(ui->TonometreupComboBox,a);});
    connect(ui->FrontoupComboBox,                   QOverload<int>::of(&QComboBox::currentIndexChanged),
                                                                                            this,   [=] (int a) {ClearCom(ui->FrontoupComboBox,a);});
    connect(ui->RefracteurupComboBox,               QOverload<int>::of(&QComboBox::currentIndexChanged),
                                                                                            this,   [=] (int a) {ClearCom(ui->RefracteurupComboBox,a);});
    connect(ui->ActesCCAMupTableWidget,             &QTableWidget::itemEntered,             this,   [=] (QTableWidgetItem* item) {AfficheToolTip(ui->ActesCCAMupTableWidget, item);});
    connect(ui->AssocCCAMupTableWidget,             &QTableWidget::itemEntered,             this,   [=] (QTableWidgetItem* item) {AfficheToolTip(ui->AssocCCAMupTableWidget, item);});
    connect(ui->HorsNomenclatureupTableWidget,      &QTableWidget::itemEntered,             this,   [=] (QTableWidgetItem* item) {AfficheToolTip(ui->HorsNomenclatureupTableWidget, item);});
    connect(ui->ChercheCCAMupLineEdit,              &QLineEdit::textEdited,                 this,   &dlg_param::ChercheCodeCCAM);
    connect(ui->ParamMotifspushButton,              &QPushButton::clicked,                  this,   &dlg_param::ParamMotifs);
    connect(this,                                   &dlg_param::click,                      this,   &dlg_param::EnableModif);
    connect(ui->OphtaSeulcheckBox,                  &QCheckBox::clicked,                    this,   &dlg_param::FiltreActesOphtaSeulmt);

    foreach (QLineEdit *line, ui->PosteParamtab->findChildren<QLineEdit*>())
        connect(line,   &QLineEdit::textEdited,                                 this,   &dlg_param::EnableOKModifPosteButton);
    foreach (QComboBox *box, ui->PosteParamtab->findChildren<QComboBox*>())
        connect(box,    QOverload<int>::of(&QComboBox::currentIndexChanged),    this,   &dlg_param::EnableOKModifPosteButton);
    foreach (QRadioButton *butt, ui->PosteParamtab->findChildren<QRadioButton*>())
        connect(butt,   &QRadioButton::clicked,                                 this,   &dlg_param::EnableOKModifPosteButton);
    foreach (QCheckBox *box, ui->PosteParamtab->findChildren<QCheckBox*>())
        connect(box,    &QCheckBox::clicked,                                    this,   &dlg_param::EnableOKModifPosteButton);
    foreach (QSpinBox *spin, ui->PosteParamtab->findChildren<QSpinBox*>())
        connect(spin,   QOverload<int>::of(&QSpinBox::valueChanged),            this,   &dlg_param::EnableOKModifPosteButton);
    foreach(QRadioButton *butt, ui->JourSauvegardeframe->findChildren<QRadioButton*>())
        connect(butt,                               &QPushButton::clicked,              this,   &dlg_param::ModifDateBackup);
    connect(ui->HeureBackuptimeEdit,                &QTimeEdit::timeChanged,            this,   &dlg_param::ModifHeureBackup);
    connect(ui->DirBackuppushButton,                &QPushButton::clicked,              this,   &dlg_param::ModifDirBackup);
    connect(ui->ImmediatBackupupPushButton,         &QPushButton::clicked,              this,   &dlg_param::startImmediateBackup);
    connect(ui->RestaurBaseupPushButton,            &QPushButton::clicked,              this,   &dlg_param::RestaureBase);
    connect(ui->ReinitBaseupPushButton,             &QPushButton::clicked,              proc,   &Procedures::ReinitBase);
    connect(ui->EffacePrgSauvupPushButton,          &QPushButton::clicked,              this,   &dlg_param::EffaceProgrammationDataBackup);

    connect(ui->PortFrontoupComboBox,               QOverload<int>::of(&QComboBox::currentIndexChanged),    this,   [=] (int a) {EnableNetworkAppareilRefraction(ui->PortFrontoupComboBox, a);});
    connect(ui->PortAutorefupComboBox,              QOverload<int>::of(&QComboBox::currentIndexChanged),    this,   [=] (int a) {EnableNetworkAppareilRefraction(ui->PortAutorefupComboBox, a);});
    connect(ui->PortRefracteurupComboBox,           QOverload<int>::of(&QComboBox::currentIndexChanged),    this,   [=] (int a) {EnableNetworkAppareilRefraction(ui->PortRefracteurupComboBox, a);});
    connect(ui->PortTonometreupComboBox,            QOverload<int>::of(&QComboBox::currentIndexChanged),    this,   [=] (int a) {EnableNetworkAppareilRefraction(ui->PortTonometreupComboBox, a);});

    connect(ui->NetworkPathFrontoupPushButton,      &QPushButton::clicked,              this,   [=] {ModifPathDirEchangeMesure(Fronto);});
    connect(ui->NetworkPathAutorefupPushButton,     &QPushButton::clicked,              this,   [=] {ModifPathDirEchangeMesure(Autoref);});
    connect(ui->NetworkPathRefracteurupPushButton,  &QPushButton::clicked,              this,   [=] {ModifPathDirEchangeMesure(Refracteur);});
    connect(ui->NetworkPathTonoupPushButton,        &QPushButton::clicked,              this,   [=] {ModifPathDirEchangeMesure(Tono);});

    connect(ui->NetworkPathEchangeFrontoupPushButton,       &QPushButton::clicked,              this,   [=] {ModifPathEchangeAppareilMesure(Fronto);});
    connect(ui->NetworkPathEchangeAutorefupPushButton,      &QPushButton::clicked,              this,   [=] {ModifPathEchangeAppareilMesure(Autoref);});
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
    bool autormodif         = enable && (currentuser()->idparent() == currentuser()->id());            // les remplaçants ne peuvent pas modifier les actes
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
    bool autormodif = enable && currentuser()->idparent() == currentuser()->id();  // les remplaçants ne peuvent pas modifier les actes
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
    bool autormodif = enable && currentuser()->idparent() == currentuser()->id();  // les remplaçants ne peuvent pas modifier les actes
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

void dlg_param::EnableNetworkAppareilRefraction(UpComboBox *combo, int idx)
{
    QString currtext = combo->itemText(idx);
    bool a = (currtext == RESEAU);
    if (combo == ui->PortFrontoupComboBox)
    {
        ui->NetworkPathFrontoupLineEdit->setVisible(a);
        ui->NetworkPathFrontoupPushButton->setVisible(a);
    }
    else if (combo == ui->PortAutorefupComboBox)
    {
        ui->NetworkPathAutorefupLineEdit->setVisible(a);
        ui->NetworkPathAutorefupPushButton->setVisible(a);
    }
    else if (combo == ui->PortRefracteurupComboBox)
    {
        ui->NetworkPathRefracteurupLineEdit->setVisible(a);
        ui->NetworkPathRefracteurupPushButton->setVisible(a);
        ui->NetworkPathEchangeAutorefupLineEdit->setVisible(a);
        ui->NetworkPathEchangeAutorefupPushButton->setVisible(a);
        ui->NetworkPathEchangeFrontoupLineEdit->setVisible(a);
        ui->NetworkPathEchangeFrontoupPushButton->setVisible(a);
    }
    else if (combo == ui->PortTonometreupComboBox)
    {
        ui->NetworkPathTonoupLineEdit->setVisible(a);
        ui->NetworkPathTonoupPushButton->setVisible(a);
    }
}

void dlg_param::EnableWidgContent(QWidget *widg, bool a)
{
    QList<QWidget*> listwidg = widg->findChildren<QWidget*>();
    for (int i=0; i<listwidg.size(); i++)
        listwidg.at(i)->setEnabled(a);
    if (widg == ui->Sauvegardeframe)
    {
        ui->ModifBaselabel->setVisible(db->ModeAccesDataBase() != Utils::Poste);
        if (db->ModeAccesDataBase() == Utils::Poste)
            ui->EffacePrgSauvupPushButton->setEnabled(m_parametres->daysbkup()
                                                   && QDir(m_parametres->dirbkup()).exists()
                                                   && m_parametres->dirbkup() != ""
                                                   && m_parametres->heurebkup() != QTime());
        else
            ui->ModifBaselabel->setEnabled(true);
    }
}

void dlg_param::EnregistreNouvMDPAdmin()
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
        qDebug() << anc << nouv << confirm;

        if (anc == "")
        {
            QSound::play(NOM_ALARME);
            msgbox.setInformativeText(tr("Ancien mot de passe requis"));
            dlg_askMDP->findChild<UpLineEdit*>(m_ancienMDP)->setFocus();
            msgbox.exec();
            return;
        }
        if (Utils::calcSHA1(anc) != proc->MDPAdmin())
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
        db->setmdpadmin(Utils::calcSHA1(nouv));
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
    int ncol = (currentuser()->secteurconventionnel()>1? 6 : 5);
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
    connect(ui->ActesCCAMupTableWidget,     &QTableWidget::currentCellChanged,  this, [=] {RegleAssocBoutons(ui->ActesCCAMupTableWidget);});
    connect(ui->ActesCCAMupTableWidget,     &QTableWidget::cellClicked,         this, [=] {RegleAssocBoutons(ui->ActesCCAMupTableWidget);});

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
        connect(check,  &QCheckBox::clicked,  this,   [=] { MAJActesCCAM(check);
                                                            RegleAssocBoutons(check); });
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
    QString reqactes = "select typeacte, montantpratique from " TBL_COTATIONS " where idUser = " + QString::number(currentuser()->id());
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
                    connect(lbl,    &UpLineEdit::TextModified,  this,   [=] (QString txt) {MAJActesCCAM(lbl, txt);});
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
    int ncol = (currentuser()->secteurconventionnel() > 1 ? 5 : 4);
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
    connect(ui->AssocCCAMupTableWidget,     &QTableWidget::currentCellChanged,  this, [=] {RegleAssocBoutons(ui->AssocCCAMupTableWidget);});
    connect(ui->AssocCCAMupTableWidget,     &QTableWidget::cellClicked,         this, [=] {RegleAssocBoutons(ui->AssocCCAMupTableWidget);});

    //Remplissage Table AssocCCCAM
    QTableWidgetItem    *pItem0;
    UpCheckBox          *check;
    QDoubleValidator *val = new QDoubleValidator(this);
    val->setDecimals(2);
    ui->AssocCCAMupTableWidget->clearContents();
    QString Assocrequete = "SELECT TYPEACTE, montantOPTAM, montantNonOptam, montantpratique, tip from "  TBL_COTATIONS " WHERE CCAM = 2 AND iduser = " + QString::number(currentuser()->id()) + " order by typeacte";
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
        connect(check,  &QCheckBox::clicked,  this,   [=] { MAJAssocCCAM(check);
                                                            RegleAssocBoutons(check); });
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
        connect(lbl1,    &UpLineEdit::TextModified,  this,   [=] (QString txt) {MAJAssocCCAM(lbl1, txt);});
        ui->AssocCCAMupTableWidget->setCellWidget(i,2,lbl1);

        UpLineEdit *lbl2 = new UpLineEdit();
        lbl2->setText(QLocale().toString(Assoclist.at(i).at(2).toDouble(),'f',2));      // montant nonOPTAM
        lbl2->setAlignment(Qt::AlignRight);
        lbl2->setStyleSheet("border: 0px solid rgb(150,150,150)");
        lbl2->setRow(i);
        lbl2->setColumn(3);
        lbl2->setValidator(val);
        lbl2->setEnabled(false);
        connect(lbl2,    &UpLineEdit::TextModified,  this,   [=] (QString txt) {MAJAssocCCAM(lbl2, txt);});
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
            connect(lbl3,    &UpLineEdit::TextModified,  this,   [=] (QString txt) {MAJAssocCCAM(lbl3, txt);});
            ui->AssocCCAMupTableWidget->setCellWidget(i,4,lbl3);
        }
        ui->AssocCCAMupTableWidget->setRowHeight(i, int(QFontMetrics(qApp->font()).height()*1.1));
    }
    Assocrequete = "SELECT DISTINCT TYPEACTE, montantoptam, montantnonoptam, montantpratique, Tip from "  TBL_COTATIONS " WHERE CCAM = 2"
                   " and typeacte not in (SELECT TYPEACTE from "  TBL_COTATIONS " WHERE CCAM = 2 AND iduser = " + QString::number(currentuser()->id()) + ")";
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
        connect(check,  &QCheckBox::clicked,  this,   [=] { MAJAssocCCAM(check);
                                                            RegleAssocBoutons(check); });
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
        connect(lbl1,    &UpLineEdit::TextModified,  this,   [=] (QString txt) {MAJAssocCCAM(lbl1, txt);});
        ui->AssocCCAMupTableWidget->setCellWidget(row,2,lbl1);

        UpLineEdit *lbl2 = new UpLineEdit();
        lbl2->setText(QLocale().toString(Assoc2list.at(i).at(2).toDouble(),'f',2));      // montant nonOPTAM
        lbl2->setAlignment(Qt::AlignRight);
        lbl2->setStyleSheet("border: 0px solid rgb(150,150,150)");
        lbl2->setRow(i);
        lbl2->setColumn(3);
        lbl2->setValidator(val);
        lbl2->setEnabled(false);
        connect(lbl2,    &UpLineEdit::TextModified,  this,   [=] (QString txt) {MAJAssocCCAM(lbl2, txt);});
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
            connect(lbl3,    &UpLineEdit::TextModified,  this,   [=] (QString txt) {MAJAssocCCAM(lbl3, txt);});
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
    connect(ui->HorsNomenclatureupTableWidget,     &QTableWidget::currentCellChanged,   this, [=] {RegleAssocBoutons(ui->HorsNomenclatureupTableWidget);});
    connect(ui->HorsNomenclatureupTableWidget,     &QTableWidget::cellClicked,          this, [=] {RegleAssocBoutons(ui->HorsNomenclatureupTableWidget);});

    //Remplissage Table Horsnomenclature
    QTableWidgetItem    *pItem0;
    UpCheckBox          *check;
    QDoubleValidator *val = new QDoubleValidator(this);
    val->setDecimals(2);
    ui->HorsNomenclatureupTableWidget->clearContents();
    QString Horsrequete = "SELECT TYPEACTE, montantpratique, Tip from "  TBL_COTATIONS " WHERE CCAM = 3 AND iduser = " + QString::number(currentuser()->id());
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
        connect(check,  &QCheckBox::clicked,  this,   [=] { MAJHorsNomenclature(check);
                                                            RegleAssocBoutons(check); });
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
        connect(lbl1,    &UpLineEdit::TextModified,  this,   [=] (QString txt) {MAJHorsNomenclature(lbl1, txt);});
        ui->HorsNomenclatureupTableWidget->setCellWidget(i,2,lbl1);
        ui->HorsNomenclatureupTableWidget->setRowHeight(i, int(QFontMetrics(qApp->font()).height()*1.1));
    }
    Horsrequete = "SELECT TYPEACTE from "  TBL_COTATIONS " WHERE CCAM = 3 AND iduser <> " + QString::number(currentuser()->id())+
            " and typeacte not in (SELECT TYPEACTE from "  TBL_COTATIONS " WHERE CCAM = 3 AND iduser = " + QString::number(currentuser()->id()) + ")";
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
        connect(check,  &QCheckBox::clicked,  this,   [=] {MAJHorsNomenclature(check);});
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
        connect(lbl1,    &UpLineEdit::TextModified,  this,   [=] (QString txt) {MAJHorsNomenclature(lbl1, txt);});
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

    QString  req = "SELECT list." CP_ID_APPAREIL ", list." CP_TITREEXAMEN_APPAREIL ", list." CP_NOMAPPAREIL_APPAREIL ", " CP_FORMAT_APPAREIL
              " FROM "  TBL_APPAREILSCONNECTESCENTRE " appcon , " TBL_LISTEAPPAREILS " list"
              " where list." CP_ID_APPAREIL " = appcon." CP_IDAPPAREIL_APPAREILS " and " CP_IDLIEU_APPAREILS " = " + QString::number(Datas::I()->sites->idcurrentsite()) +
              " ORDER BY " CP_TITREEXAMEN_APPAREIL;

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
        line5a->setText(proc->pathDossierDocuments(Applist.at(i).at(2).toString(),Utils::Poste));
        line5b->setText(proc->pathDossierDocuments(Applist.at(i).at(2).toString(),Utils::ReseauLocal));
        line5c->setText(proc->pathDossierDocuments(Applist.at(i).at(2).toString(),Utils::Distant));
        line5a->setRow(i);
        line5b->setRow(i);
        line5c->setRow(i);
        ui->MonoDocupTableWidget->setCellWidget(i,col,line5a);
        ui->LocalDocupTableWidget->setCellWidget(i,col,line5b);
        ui->DistantDocupTableWidget->setCellWidget(i,col,line5c);
        connect(line5a,                    &UpLineEdit::textEdited,         this,   &dlg_param::EnableOKModifPosteButton);
        connect(line5b,                    &UpLineEdit::textEdited,         this,   &dlg_param::EnableOKModifPosteButton);
        connect(line5c,                    &UpLineEdit::textEdited,         this,   &dlg_param::EnableOKModifPosteButton);
        connect(line5a,                    &UpLineEdit::TextModified,       this,   [=] (QString txt) {EnregDossierStockageApp(line5a, txt);});
        connect(line5b,                    &UpLineEdit::TextModified,       this,   [=] (QString txt) {EnregDossierStockageApp(line5b, txt);});
        connect(line5c,                    &UpLineEdit::TextModified,       this,   [=] (QString txt) {EnregDossierStockageApp(line5c, txt);});
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
        connect(dossbouton,     &QPushButton::clicked,  this,   [=] {ChoixDossierStockageApp(dossbouton);});
        connect(dossbouton1,    &QPushButton::clicked,  this,   [=] {ChoixDossierStockageApp(dossbouton1);});
        connect(dossbouton2,    &QPushButton::clicked,  this,   [=] {ChoixDossierStockageApp(dossbouton2);});

        pItem6->setText(Applist.at(i).at(3).toString());                             // Format
        ui->AppareilsConnectesupTableWidget->setItem(i,col,pItem6);

        ui->AppareilsConnectesupTableWidget->setRowHeight(i,int(fm.height()*1.3));
        ui->MonoDocupTableWidget->setRowHeight(i,int(fm.height()*1.3));
        ui->LocalDocupTableWidget->setRowHeight(i,int(fm.height()*1.3));
        ui->DistantDocupTableWidget->setRowHeight(i,int(fm.height()*1.3));
    }

    m_listeappareils.clear();
    req = "select " CP_NOMAPPAREIL_APPAREIL " from " TBL_LISTEAPPAREILS
          " where " CP_ID_APPAREIL " not in (select " CP_IDAPPAREIL_APPAREILS " from " TBL_APPAREILSCONNECTESCENTRE " where " CP_IDLIEU_APPAREILS " = " + QString::number(Datas::I()->sites->idcurrentsite()) + ")";
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
        if (ui->AutorefupComboBox->currentIndex()==0 && ui->PortAutorefupComboBox->currentIndex() > 0)
        {
            UpMessageBox::Watch(this,tr("Vous avez spécifié un port COM pour l'autorefractomètre sans sélectionner de machine !"));
            ui->ParamtabWidget->setCurrentWidget(ui->PosteParamtab);
            ui->AutorefupComboBox->setFocus();
            return false;
        }
        if (ui->AutorefupComboBox->currentIndex()>0 && ui->PortAutorefupComboBox->currentText() == RESEAU && !QDir(ui->NetworkPathAutorefupLineEdit->text()).exists())
        {
            UpMessageBox::Watch(this,tr("Vous n'avez pas spécifié de dossier réseau valide pour l'autorefractomètre") + " !");
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
        if (ui->FrontoupComboBox->currentIndex()==0 && ui->PortFrontoupComboBox->currentIndex() > 0)
        {
            UpMessageBox::Watch(this,tr("Vous avez spécifié un port COM pour le frontofocotomètre sans sélectionner de machine !"));
            ui->ParamtabWidget->setCurrentWidget(ui->PosteParamtab);
            ui->FrontoupComboBox->setFocus();
            return false;
        }
        if (ui->FrontoupComboBox->currentIndex()>0 && ui->PortFrontoupComboBox->currentText() == RESEAU && !QDir(ui->NetworkPathFrontoupLineEdit->text()).exists())
        {
            UpMessageBox::Watch(this,tr("Vous n'avez pas spécifié de dossier réseau valide pour le frontofocomètre") + " !");
            ui->ParamtabWidget->setCurrentWidget(ui->PosteParamtab);
            ui->PortFrontoupComboBox->setFocus();
            return false;
        }

        if (ui->RefracteurupComboBox->currentIndex()>0 && ui->PortRefracteurupComboBox->currentIndex() == 0)
        {
            UpMessageBox::Watch(this,tr("Vous n'avez pas spécifié de mode de communication pour le réfracteur ") + ui->RefracteurupComboBox->currentText() + " !");
            ui->ParamtabWidget->setCurrentWidget(ui->PosteParamtab);
            ui->PortRefracteurupComboBox->setFocus();
            return false;
        }
        if (ui->RefracteurupComboBox->currentIndex()==0 && ui->PortRefracteurupComboBox->currentIndex() < 0)
        {
            UpMessageBox::Watch(this,tr("Vous avez spécifié un port COM pour le réfracteur sans sélectionner de machine !"));
            ui->ParamtabWidget->setCurrentWidget(ui->PosteParamtab);
            ui->RefracteurupComboBox->setFocus();
            return false;
        }
        if (ui->RefracteurupComboBox->currentIndex()>0 && ui->PortRefracteurupComboBox->currentText() == RESEAU && !QDir(ui->NetworkPathRefracteurupLineEdit->text()).exists())
        {
            UpMessageBox::Watch(this,tr("Vous n'avez pas spécifié de dossier réseau valide pour le refracteur") + " !");
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
        if (ui->TonometreupComboBox->currentIndex()==0 && ui->PortTonometreupComboBox->currentIndex() > 0)
        {
            UpMessageBox::Watch(this,tr("Vous avez spécifié un port COM pour le tonomètre sans sélectionner de machine !"));
            ui->ParamtabWidget->setCurrentWidget(ui->PosteParamtab);
            ui->TonometreupComboBox->setFocus();
            return false;
        }
        if (ui->TonometreupComboBox->currentIndex()>0 && ui->PortTonometreupComboBox->currentText() == RESEAU && !QDir(ui->NetworkPathTonoupLineEdit->text()).exists())
        {
            UpMessageBox::Watch(this,tr("Vous n'avez pas spécifié de dossier réseau valide pour le tonomètre") + " !");
            ui->ParamtabWidget->setCurrentWidget(ui->PosteParamtab);
            ui->PortTonometreupComboBox->setFocus();
            return false;
        }

        QString Base = Utils::getBaseFromMode(Utils::Poste);
        if (ui->PosteServcheckBox->isChecked())
            proc->settings()->setValue(Base + "/Active","YES");
        else
            proc->settings()->setValue(Base + "/Active","NO");
        proc->settings()->setValue(Base + "/Port",ui->SQLPortPostecomboBox->currentText());

        Base = Utils::getBaseFromMode(Utils::ReseauLocal);
        if (ui->LocalServcheckBox->isChecked())
            proc->settings()->setValue(Base + "/Active","YES");
        else
            proc->settings()->setValue(Base + "/Active","NO");
        proc->settings()->setValue(Base + "/Serveur",Utils::calcIP(ui->EmplacementLocaluplineEdit->text(), false));
        db->setadresseserveurlocal(ui->EmplacementLocaluplineEdit->text());
        proc->settings()->setValue(Base + "/Port",ui->SQLPortLocalcomboBox->currentText());

        Base = Utils::getBaseFromMode(Utils::Distant);
        if (ui->DistantServcheckBox->isChecked())
            proc->settings()->setValue(Base + "/Active","YES");
        else
            proc->settings()->setValue(Base + "/Active","NO");
        if (Utils::rgx_IPV4.exactMatch(ui->EmplacementDistantuplineEdit->text()))
            proc->settings()->setValue(Base + "/Serveur", Utils::calcIP(ui->EmplacementDistantuplineEdit->text(), false));
        else
            proc->settings()->setValue(Base + "/Serveur", ui->EmplacementDistantuplineEdit->text());
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
            proc->settings()->setValue(Utils::getBaseFromMode(Utils::ReseauLocal) + "/PrioritaireGestionDocs","YES");
        else if (ui->NonImportDocscheckBox->isChecked())
            proc->settings()->setValue(Utils::getBaseFromMode(Utils::ReseauLocal) + "/PrioritaireGestionDocs","NO");
        else
            proc->settings()->setValue(Utils::getBaseFromMode(Utils::ReseauLocal) + "/PrioritaireGestionDocs","NORM");
        proc->settings()->setValue("Param_Imprimante/TailleEnTete",ui->EntetespinBox->text());
        proc->settings()->setValue("Param_Imprimante/TailleEnTeteALD",ui->EnteteALDspinBox->text());
        proc->settings()->setValue("Param_Imprimante/TaillePieddePage",ui->PiedDePagespinBox->text());
        proc->settings()->setValue("Param_Imprimante/TaillePieddePageOrdoLunettes",ui->PiedDePageOrdoLunettesspinBox->text());
        proc->settings()->setValue("Param_Imprimante/TailleTopMarge",ui->TopMargespinBox->text());

        proc->settings()->setValue("Param_Poste/Fronto",ui->FrontoupComboBox->currentText());
        if (ui->FrontoupComboBox->currentText() == "-")
        {
            proc->settings()->setValue("Param_Poste/PortFronto", "");
            proc->settings()->setValue("Param_Poste/PortFronto/Reseau", "");
        }
        else
        {
            proc->settings()->setValue("Param_Poste/PortFronto",ui->PortFrontoupComboBox->currentText());
            if (ui->PortFrontoupComboBox->currentText() == RESEAU)
                proc->settings()->setValue("Param_Poste/PortFronto/Reseau", ui->NetworkPathFrontoupLineEdit->text());
            else
                proc->settings()->setValue("Param_Poste/PortFronto/Reseau", "");
        }

        proc->settings()->setValue("Param_Poste/Autoref",ui->AutorefupComboBox->currentText());
        if (ui->AutorefupComboBox->currentText() == "-")
        {
            proc->settings()->setValue("Param_Poste/PortAutoref", "");
            proc->settings()->setValue("Param_Poste/PortAutoref/Reseau", "");
        }
        else
        {
            proc->settings()->setValue("Param_Poste/PortAutoref",ui->PortAutorefupComboBox->currentText());
            if (ui->PortAutorefupComboBox->currentText() == RESEAU)
                proc->settings()->setValue("Param_Poste/PortAutoref/Reseau", ui->NetworkPathAutorefupLineEdit->text());
            else
                proc->settings()->setValue("Param_Poste/PortAutoref/Reseau", "");
        }

        proc->settings()->setValue("Param_Poste/Refracteur",ui->RefracteurupComboBox->currentText());
        proc->settings()->setValue("Param_Poste/PortRefracteur",ui->PortRefracteurupComboBox->currentText());
        if (ui->RefracteurupComboBox->currentText() == "-")
        {
            proc->settings()->setValue("Param_Poste/PortRefracteur", "");
            proc->settings()->setValue("Param_Poste/PortRefracteur/Reseau", "");
            proc->settings()->setValue("Param_Poste/PortRefracteur/Reseau/AdressFronto", "");
            proc->settings()->setValue("Param_Poste/PortRefracteur/Reseau/AdressAutoref", "");
        }
        else
        {
            if (ui->PortRefracteurupComboBox->currentText() == RESEAU)
            {
                proc->settings()->setValue("Param_Poste/PortRefracteur/Reseau", ui->NetworkPathRefracteurupLineEdit->text());
                proc->settings()->setValue("Param_Poste/PortRefracteur/Reseau/AdressFronto", ui->NetworkPathEchangeFrontoupLineEdit->text());
                proc->settings()->setValue("Param_Poste/PortRefracteur/Reseau/AdressAutoref", ui->NetworkPathEchangeAutorefupLineEdit->text());
            }
            else
            {
                proc->settings()->setValue("Param_Poste/PortRefracteur/Reseau", "");
                proc->settings()->setValue("Param_Poste/PortRefracteur/Reseau/AdressFronto", "");
                proc->settings()->setValue("Param_Poste/PortRefracteur/Reseau/AdressAutoref", "");
            }
        }

        proc->settings()->setValue("Param_Poste/Tonometre",ui->TonometreupComboBox->currentText());
        proc->settings()->setValue("Param_Poste/PortTonometre",ui->PortTonometreupComboBox->currentText());
        if (ui->TonometreupComboBox->currentText() == "-")
        {
            proc->settings()->setValue("Param_Poste/PortTonometre", "");
            proc->settings()->setValue("Param_Poste/PortTonometre/Reseau", "");
        }
        else
        {
            if (ui->PortTonometreupComboBox->currentText() == RESEAU)
                proc->settings()->setValue("Param_Poste/PortTonometre/Reseau", ui->NetworkPathTonoupLineEdit->text());
            else
                proc->settings()->setValue("Param_Poste/PortTonometre/Reseau", "");
        }

        proc->settings()->setValue("Param_Poste/VilleParDefaut",wdg_VilleDefautlineEdit->text());
        proc->settings()->setValue("Param_Poste/CodePostalParDefaut",wdg_CPDefautlineEdit->text());

        m_modifposte = false;
    }
    return true;
}
