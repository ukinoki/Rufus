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

#include <QFile>
#include <QMenu>
#include "dlg_param.h"
#include "icons.h"
#include "ui_dlg_param.h"
#include "utils.h"
#include "mysqlinstaller.h"

dlg_param::dlg_param(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::dlg_param)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    //! Hauteur du sous-onglet de connexion : laissée au .ui (min = max = 335 px), qui
    //! loge déjà les frames Mono/Local/Distant avec leurs champs mot de passe MySQL.
    //! NE PAS la forcer ici : un setFixedHeight() écrase le min ET le max du .ui ; s'il
    //! impose une valeur plus grande que celle du .ui, le bas de ParamConnexiontabWidget
    //! déborde sur Instrmtsframe à l'exécution (et seulement à l'exécution — Designer
    //! n'exécute pas ce code). C'était la cause de l'empiètement constaté sous Ubuntu.

    QStringList ports;
    ports << "3306" << "3307";
    ui->SQLPortDistantcomboBox  ->addItems(ports);
    ui->SQLPortLocalcomboBox    ->addItems(ports);
    ui->SQLPortPostecomboBox    ->addItems(ports);

    wdg_appareilswdgbuttonframe                     = new WidgetButtonFrame(ui->AppareilsConnectesupTableWidget);
    wdg_appareilswdgbuttonframe                     ->AddButtons(WidgetButtonFrame::Plus | WidgetButtonFrame::Moins);
    connect(wdg_appareilswdgbuttonframe,            &WidgetButtonFrame::choix,  this,   [=] {ChoixButtonFrame(wdg_appareilswdgbuttonframe);});

    wdg_cotationswdgbuttonframe            = new WidgetButtonFrame(ui->cotationsUpTableView);
    wdg_cotationswdgbuttonframe            ->AddButtons(WidgetButtonFrame::Plus | WidgetButtonFrame::Modifier | WidgetButtonFrame::Moins);
    connect(wdg_cotationswdgbuttonframe,   &WidgetButtonFrame::choix,  this,   [=] {ChoixButtonFrame(wdg_cotationswdgbuttonframe);});

    wdg_cotationswdgbuttonframe->layButtons()->insertWidget(0, ui->ChercheCotationlabel);
    wdg_cotationswdgbuttonframe->layButtons()->insertWidget(0, ui->ChercheCotationupLineEdit);

    QHBoxLayout *Margelay       = new QHBoxLayout();
    QVBoxLayout *Cotationslay   = new QVBoxLayout();
    QVBoxLayout *AllCotationslay= new QVBoxLayout();
    int marge   = 10;
    AllCotationslay     ->setSpacing(marge);
    marge = 0;
    AllCotationslay     ->setContentsMargins(marge,marge,marge,marge);
    Margelay            ->setContentsMargins(marge,marge,marge,marge);
    Margelay            ->setSpacing(marge);

    Cotationslay        ->addWidget(ui->Cotationslabel);
    Cotationslay        ->addWidget(wdg_cotationswdgbuttonframe->widgButtonParent());
    //Cotationslay        ->setStretch(1,15);

    Margelay            ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Maximum, QSizePolicy::Minimum));
    Margelay            ->addLayout(Cotationslay);
    Margelay            ->setStretch(1,14);

    AllCotationslay     ->addLayout(Margelay);

    AllCotationslay     ->setStretch(0,7);
    AllCotationslay     ->setStretch(1,5);

    if (m_parametres->cotationsfrance())    {

        AllCotationslay ->insertWidget(0,ui->CCAMwidget);
        AllCotationslay ->setStretch(0,8);
        AllCotationslay ->setStretch(1,7);
    }
    else
        ui->CCAMwidget->setVisible(false);

    ui->Cotationslabel  ->setText(tr("Cotations") + "\n" + tr("Clic droit sur un item pour modifier le montant pratiqué"));
    ui->Cotationswidget ->setLayout(AllCotationslay);

    ui->UserParamtab    ->setLayout(ui->UserLayout);
    ui->GeneralParamtab ->setLayout(ui->GeneralLayout);
    //! NE PAS « voler » PosteLayout vers PosteParamtab : ce layout a 3 items dont la somme des
    //! hauteurs mini (tab 335 + Instrmtsframe 250 + ligne Imprimante 185 ≈ 770 px) doit tenir
    //! dans la page d'onglet. Posé sur PosteParamtab (trop courte), le layout est sur-contraint
    //! et déborde. Laissé sur son wrapper ParamWidget (841 px, défini dans le .ui), il a la
    //! place nécessaire et tout se met en page sans chevauchement.

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
    ui->ChercheCotationlabel        ->setPixmap(Icons::pxLoupe().scaled(20,20)); //WARNING : icon scaled : pxLoupe 20,20
    ui->StatutComptaupTextEdit      ->setAttribute( Qt::WA_NoSystemBackground, true );
    ui->StatutComptaupTextEdit      ->setReadOnly(true);
    ui->StatutComptaupTextEdit      ->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->StatutComptaupTextEdit      ->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    ui->HeureBackuptimeEdit         ->setFixedSize(Utils::qtimeeditsize());

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

    ui->PosteStockageInfoUpLabel    ->setStyleSheet("background-color: white");
    QString tip = tr("Emplacement du dossier de stockage des documents d'imagerie <br /><font color=\"green\"><b>SUR CE POSTE SERVEUR</b></font>"
                     "<br/>Cet emplacement n'est pas modifiable et correspond à la variable secure_file_priv de MySQL");
    ui->PosteStockageInfoUpLabel    ->setImmediateToolTip(tip);
    tip = tr("Indiquez ici <br /><font color=\"green\"><b>LE LIEN</b></font><br /> vers l'emplacement du dossier de stockage des documents d'imagerie <br /><font color=\"green\"><b>SUR LE SERVEUR</b></font>");
    ui->LocalPathStockageupLabel    ->setImmediateToolTip(tip);
    ui->LocalPathStockageupLineEdit ->setImmediateToolTip(tip);

    ui->PosteVideoDirupLineEdit     ->useselftextastooltip();
    ui->LocalVideoDirupLineEdit     ->useselftextastooltip();
    ui->DistantVideoDirupLineEdit   ->useselftextastooltip();

    QStringList Listapp;
    Listapp << "-" << Datas::I()->refractiondevices->listRefractionDevices(RefractionDevice::AutorefDev);
    ui->AutorefupComboBox->insertItems(0, Listapp);
    Listapp.clear();
    Listapp << "-" << Datas::I()->refractiondevices->listRefractionDevices(RefractionDevice::FrontoDev);
    ui->FrontoupComboBox->insertItems(0, Listapp);
    Listapp.clear();
    Listapp << "-" << Datas::I()->refractiondevices->listRefractionDevices(RefractionDevice::RefracteurDev);
    ui->RefracteurupComboBox->insertItems(0, Listapp);
    Listapp.clear();
    Listapp << "-" << Datas::I()->refractiondevices->listRefractionDevices(RefractionDevice::TonometreDev);
    ui->TonometreupComboBox->insertItems(0, Listapp);

    RecalcAvailablesPorts(true);
    QString set (""), setport ("");
    int idx;

    set     = proc->settings()->value(Param_Poste_Fronto).toString();
    setport = proc->settings()->value(Param_Poste_PortFronto).toString();
    ui->FrontoupComboBox                ->setCurrentText(set);
    idx = ui->PortFrontoupComboBox->findText(setport);
    if (idx == -1)
        idx = 0;
    ui->PortFrontoupComboBox            ->setCurrentIndex(idx);
    if (ui->PortFrontoupComboBox->currentText() == DOSSIER_ECHANGE)
        ui->NetworkPathFrontoupLineEdit->setText(proc->settings()->value(Param_Poste_PortFronto_DossierEchange).toString());
    EnableComOrNetworkWidgetsAppareilRefraction(ui->PortFrontoupComboBox,       ui->PortFrontoupComboBox->currentText());
    ui->NetworkPathFrontoupLineEdit     ->setImmediateToolTip(ui->NetworkPathFrontoupLineEdit->text());
    ui->NetworkPathFrontoupPushButton   ->setImmediateToolTip(tr("Emplacement du fichier de mesures émises par le frontofocomètre"));

    set     = proc->settings()->value(Param_Poste_Autoref).toString();
    setport = proc->settings()->value(Param_Poste_PortAutoref).toString();
    ui->AutorefupComboBox               ->setCurrentText(set);
    idx = ui->PortAutorefupComboBox->findText(setport);
    if (idx == -1)
        idx = 0;
    ui->PortAutorefupComboBox           ->setCurrentIndex(idx);
    if (ui->PortAutorefupComboBox->currentText() == DOSSIER_ECHANGE)
        ui->NetworkPathAutorefupLineEdit->setText(proc->settings()->value(Param_Poste_PortAutoref_DossierEchange).toString());
    EnableComOrNetworkWidgetsAppareilRefraction(ui->PortAutorefupComboBox,      ui->PortAutorefupComboBox->currentText());
    ui->NetworkPathAutorefupLineEdit    ->setImmediateToolTip(ui->NetworkPathAutorefupLineEdit->text());
    ui->NetworkPathAutorefupPushButton  ->setImmediateToolTip(tr("Emplacement du fichier de mesures émises par l'autorefractomètre"));

    set     = proc->settings()->value(Param_Poste_Refracteur).toString();
    setport = proc->settings()->value(Param_Poste_PortRefracteur).toString();
    ui->RefracteurupComboBox            ->setCurrentText(set);
    idx = ui->PortRefracteurupComboBox->findText(setport);
    if (idx == -1)
        idx = 0;
    ui->PortRefracteurupComboBox            ->setCurrentIndex(idx);
    if (ui->PortRefracteurupComboBox->currentText() == DOSSIER_ECHANGE)
    {
        ui->NetworkPathRefracteurupLineEdit->setText(proc->settings()->value(Param_Poste_PortRefracteur_DossierEchange).toString());
        ui->NetworkPathEchangeFrontoupLineEdit->setText(proc->settings()->value(Param_Poste_PortRefracteur_DossierEchange_Fronto).toString());
        ui->NetworkPathEchangeAutorefupLineEdit->setText(proc->settings()->value(Param_Poste_PortRefracteur_DossierEchange_Autoref).toString());
    }
    EnableComOrNetworkWidgetsAppareilRefraction(ui->PortRefracteurupComboBox,   ui->PortRefracteurupComboBox->currentText());
    ui->NetworkPathRefracteurupPushButton       ->setImmediateToolTip(tr("Emplacement du fichier de mesures émises par le refracteur"));
    ui->NetworkPathRefracteurupLineEdit         ->setImmediateToolTip(ui->NetworkPathRefracteurupLineEdit->text());
    ui->NetworkPathEchangeFrontoupPushButton    ->setImmediateToolTip(tr("Emplacement du fichier de mesures de frontofocomètre élaborées par Rufus à envoyer au réfracteur"));
    ui->NetworkPathEchangeFrontoupLineEdit      ->setImmediateToolTip(ui->NetworkPathEchangeFrontoupLineEdit->text());
    ui->NetworkPathEchangeAutorefupPushButton   ->setImmediateToolTip(tr("Emplacement du fichier de mesures d'autorefractomètre élaborées par Rufus à envoyer au réfracteur"));
    ui->NetworkPathEchangeAutorefupLineEdit     ->setImmediateToolTip(ui->NetworkPathEchangeAutorefupLineEdit->text());

    set     = proc->settings()->value(Param_Poste_Tono).toString();
    setport = proc->settings()->value(Param_Poste_PortTono).toString();
    ui->TonometreupComboBox             ->setCurrentText(set);
    idx = ui->PortTonometreupComboBox->findText(setport);
    if (idx == -1)
        idx = 0;
    ui->PortTonometreupComboBox         ->setCurrentIndex(idx);
    if (ui->PortTonometreupComboBox->currentText() == DOSSIER_ECHANGE)
        ui->NetworkPathTonoupLineEdit   ->setText(proc->settings()->value(Param_Poste_PortTono_DossierEchange).toString());
    EnableComOrNetworkWidgetsAppareilRefraction(ui->PortTonometreupComboBox,    ui->PortTonometreupComboBox->currentText());
    ui->NetworkPathTonoupLineEdit       ->setImmediateToolTip(ui->NetworkPathTonoupLineEdit->text());
    ui->NetworkPathTonoupPushButton     ->setImmediateToolTip(tr("Emplacement du fichier de mesures émises par le tonomètre"));

    if (QSerialPortInfo::availablePorts().size()>0)
    {
        QList<UpComboBox*> listportsbox;
        listportsbox << ui->PortFrontoupComboBox << ui->PortAutorefupComboBox << ui->PortRefracteurupComboBox << ui->PortTonometreupComboBox;
        for (int l=0; l< listportsbox.size(); ++l) {
            connect(listportsbox.at(l),   &QComboBox::currentTextChanged,    this,   [=] (QString s) {
                 if (s!= listportsbox.at(l)->valeuravant())
                {
                    listportsbox.at(l)->setvaleuravant(listportsbox.at(l)->currentText());
                    EnableComOrNetworkWidgetsAppareilRefraction(listportsbox.at(l), s);
                    EnableOKModifPosteButton();
                    RecalcAvailablesPorts();
                    proc->Ouverture_Ports_Series(this);
                    proc->Ouverture_Fichiers_Echange(this);
                }
            });
        }
    }

    /*-------------------- GESTION DE LA COMPTABILITÉ-------------------------------------------------------*/
    ui->ComptagroupBox->setVisible(false);  // pour le moment
    /*-------------------- GESTION DES COTATIONS FRANCE-------------------------------------------------------*/

    /*-------------------- GESTION DES COTATIONS FRANCE-------------------------------------------------------*/
    ui->CotationsFrancecheckBox->setChecked(m_parametres->cotationsfrance());
    connect (ui->CotationsFrancecheckBox, &QCheckBox::checkStateChanged, this, [=](int state){db->setcotationsfrance(state == Qt::Checked);});
    /*-------------------- GESTION DES COTATIONS FRANCE-------------------------------------------------------*/


    /*-------------------- GESTION DES VILLES ET DES CODES POSTAUX-------------------------------------------------------*/
        ui->UtiliseBDDVillescheckBox     ->setChecked(m_parametres->villesfrance() == true);
        ui->UtiliseCustomVillescheckBox  ->setChecked(m_parametres->villesfrance() == false);
        ui->ModifListVillesupPushButton  ->setVisible(m_parametres->villesfrance() == false);
        connect(ui->UtiliseCustomVillescheckBox, &QCheckBox::checkStateChanged, this, [=](int state){
                ui->ModifListVillesupPushButton->setVisible(state == Qt::Checked);
                enum Villes::TownsFrom from;
                if (state == Qt::Checked)
                    from = Villes::CUSTOM;
                else
                    from = Villes::DATABASE;
                ModifBDDVilles(from);
            });
        connect(ui->ModifListVillesupPushButton,    &QPushButton::clicked, this, [=]{
                                                                                       dlg_listevilles *dlg_listvilles = new dlg_listevilles(this);
                                                                                       dlg_listvilles->exec();
                                                                                       delete dlg_listvilles;
                                                                                   });

       wdg_villeCP                  = new VilleCPWidget(Datas::I()->villes, ui->VilleDefautframe);
       wdg_CPDefautlineEdit         = wdg_villeCP->ui->CPlineEdit;
       wdg_VilleDefautlineEdit      = wdg_villeCP->ui->VillelineEdit;
       wdg_villeCP                  ->move(15,10);
       wdg_villeCP->ui->CPlabel     ->setText(tr("Code postal par défaut"));
       wdg_villeCP->ui->Villelabel  ->setText(tr("Ville par défaut"));
       wdg_villeCP->ui->CPlabel     ->setMinimumWidth(300);
       wdg_villeCP->ui->Villelabel  ->setMinimumWidth(300);
       wdg_VilleDefautlineEdit      ->setText(proc->settings()->value(Ville_Defaut).toString());
       wdg_CPDefautlineEdit         ->completer()->setCurrentRow(proc->settings()->value(CodePostal_Defaut).toInt());
       // ce micmac est nécessaire à cause d'un bug de QCompleter en mode InLineCompletion
       // il faut synchroniser à la main le QCompleter et le QlineEdit au premier affichage
       wdg_CPDefautlineEdit          ->setText(proc->settings()->value(CodePostal_Defaut).toString());
   /*-------------------- GESTION DES VILLES ET DES CODES POSTAUX-------------------------------------------------------*/

    AfficheParamUser();

    ui->LoginuplineEdit             ->setValidator(new QRegularExpressionValidator(Utils::rgx_AlphaNumeric,this));
    ui->MDPuplineEdit               ->setValidator(new QRegularExpressionValidator(Utils::rgx_AlphaNumeric_5_15,this));
    ui->NomuplineEdit               ->setValidator(new QRegularExpressionValidator(Utils::rgx_rx,this));
    ui->PrenomuplineEdit            ->setValidator(new QRegularExpressionValidator(Utils::rgx_rx,this));
    ui->MailuplineEdit              ->setValidator(new QRegularExpressionValidator(Utils::rgx_mail,this));
    ui->PortableuplineEdit          ->setValidator(new QRegularExpressionValidator(Utils::rgx_telephone,this));
    ui->EmplacementLocaluplineEdit  ->setValidator(new QRegularExpressionValidator(Utils::rgx_IPV4_mask,this));

    ui->cotationsUpTableView            ->setSelectionBehavior(QAbstractItemView::SelectRows);

    ui->LoginuplineEdit                 ->setEnabled(false);
    ui->MDPuplineEdit                   ->setEnabled(false);
    ui->idUseruplineEdit                ->setEnabled(false);
    ui->ChoixFontupPushButton           ->setEnabled(false);
    ui->ModifDataUserpushButton         ->setEnabled(false);
    enableCotations(false);
    ui->ImportDocsgroupBox              ->setEnabled(false);
    EnableWidgContent(ui->BackupRestoreframe,false);
    EnableWidgContent(ui->Appareilsconnectesframe,false);
    EnableWidgContent(ui->Languagewidget, false);
    ui->ParamConnexiontabWidget         ->setEnabled(false);
    EnableWidgContent(ui->Instrmtsframe,false);
    EnableWidgContent(ui->Imprimanteframe,false);
    EnableWidgContent(ui->VilleDefautframe,false);
    ui->CotationsFrancecheckBox         ->setEnabled(false);
    ui->VillesgroupBox                  ->setEnabled(false);
    ui->ModifListVillesupPushButton     ->setEnabled(false);
    ui->GestUserpushButton              ->setEnabled(false);
    ui->GestLieuxpushButton             ->setEnabled(false);
    ui->ParamMotifspushButton           ->setEnabled(false);
    ui->GestionBanquespushButton        ->setEnabled(false);
    ui->InitMDPAdminpushButton          ->setEnabled(false);
    ui->EmplacementServeurupComboBox    ->setEnabled(false);

    bool a,b,c;

    ui->DossierClesSSLupLineEdit    ->useselftextastooltip();
    QString Base;
    Base                            = Utils::getBaseFromMode(Utils::Poste);
    ui->PosteVideoDirupLineEdit     ->setText(proc->settings()->value(Base + Dossier_Videos).toString());
    a                               = (proc->settings()->value(Base + Param_Active).toString() == "YES");
    ui->PosteServcheckBox           ->setChecked(a);
    ui->Posteframe                  ->setVisible(a);
    ui->MonoDocsExtupLabel          ->setVisible(a);
    ui->MonoDocupTableWidget        ->setVisible(a);
    if (a)
    {
        bool poste = DataBase::I()->ModeAccesDataBase() == Utils::Poste;
        ui->PosteStockageupLabel        ->setVisible(poste);
        ui->PosteStockageInfoUpLabel    ->setVisible(poste);
        ui->PosteVideoDirupLabel        ->setVisible(poste);
        ui->PosteVideoDirupLineEdit     ->setVisible(poste);
        ui->PosteVideoDirupPushButton   ->setVisible(poste);

        ui->SQLPortPostecomboBox        ->setCurrentText(proc->settings()->value(Base + Param_Port).toString());
        ui->MDPMonouplineEdit           ->setText(MySQLInstaller::motDePasseStockePourMode(Utils::Poste));
        ui->PosteStockageInfoUpLabel    ->setText(QDir::toNativeSeparators(db->dirimagerie()));
    }
    Base                                = Utils::getBaseFromMode(Utils::ReseauLocal);
    ui->LocalVideoDirupLineEdit         ->setText(proc->settings()->value(Base + Dossier_Videos).toString());
    b                                   = (proc->settings()->value(Base + Param_Active).toString() == "YES");
    ui->LocalServcheckBox               ->setChecked(b);
    ui->Localframe                      ->setVisible(b);
    ui->LocalDocsExtupLabel             ->setVisible(b);
    ui->LocalDocupTableWidget           ->setVisible(b);
    ui->LocalPathStockageupLabel        ->setVisible(b);
    ui->LocalPathStockageupLineEdit     ->setVisible(b);
    ui->LocalPathStockageupPushButton   ->setVisible(b);
    ui->LocalVideoDirupLabel            ->setVisible(b);
    ui->LocalVideoDirupLineEdit         ->setVisible(b);
    ui->LocalVideoDirupPushButton       ->setVisible(b);
    if (b)
    {
        ui->EmplacementLocaluplineEdit  ->setText(proc->settings()->value(Base + Param_Serveur).toString());
        ui->SQLPortLocalcomboBox        ->setCurrentText(proc->settings()->value(Base + Param_Port).toString());
        ui->MDPLocaluplineEdit          ->setText(MySQLInstaller::motDePasseStockePourMode(Utils::ReseauLocal));
        ui->LocalPathStockageupLineEdit ->setText(proc->settings()->value(Utils::getBaseFromMode(Utils::ReseauLocal) + Dossier_Imagerie).toString());
    }
    Base                            = Utils::getBaseFromMode(Utils::Distant);
    ui->DistantVideoDirupLineEdit   ->setText(proc->settings()->value(Base + Dossier_Videos).toString());
    c                               = (proc->settings()->value(Base + Param_Active).toString() == "YES");
    ui->DistantServcheckBox         ->setChecked(c);
    ui->Distantframe                ->setVisible(c);
    ui->DistantDocsExtupLabel       ->setVisible(c);
    ui->DistantDocupTableWidget     ->setVisible(c);
    ui->DistantStockageupLabel      ->setVisible(c);
    ui->DistantStockageupLineEdit   ->setVisible(c);
    ui->DistantStockageupPushButton ->setVisible(c);
    ui->DistantVideoDirupLabel      ->setVisible(c);
    ui->DistantVideoDirupLineEdit   ->setVisible(c);
    ui->DistantVideoDirupPushButton ->setVisible(c);
    if (c)
    {
        ui->EmplacementDistantuplineEdit->setText(proc->settings()->value(Base + Param_Serveur).toString());
        ui->SQLPortDistantcomboBox      ->setCurrentText(proc->settings()->value(Base + Param_Port).toString());
        ui->MDPDistantuplineEdit        ->setText(MySQLInstaller::motDePasseStockePourMode(Utils::Distant));
        QString dir = proc->settings()->value(Utils::getBaseFromMode(Utils::Distant) + Dossier_ClesSSL).toString();
        if (dir == "")
        {
            dir = "/etc/mysql";
            proc->settings()->setValue(Utils::getBaseFromMode(Utils::Distant) + Dossier_ClesSSL, dir);
        }
        ui->DossierClesSSLupLineEdit    ->setText(proc->settings()->value(Base + Dossier_ClesSSL).toString());
        ui->DistantStockageupLineEdit   ->setText(proc->settings()->value(Utils::getBaseFromMode(Utils::Distant) + Dossier_Imagerie).toString());
    }

    if (db->ModeAccesDataBase() == (Utils::Poste))
        ui->ParamConnexiontabWidget->setCurrentIndex(0);
    else if (db->ModeAccesDataBase() == Utils::ReseauLocal)
        ui->ParamConnexiontabWidget->setCurrentIndex(1);
    if (db->ModeAccesDataBase() == Utils::Distant)
        ui->ParamConnexiontabWidget->setCurrentIndex(2);

    ui->ParamtabWidget->setCurrentIndex(0);

    ui->EntetespinBox                   ->setValue(proc->settings()->value(Imprimante_TailleEnTete).toInt());
    ui->EnteteALDspinBox                ->setValue(proc->settings()->value(Imprimante_TailleEnTeteALD).toInt());
    ui->PiedDePagespinBox               ->setValue(proc->settings()->value(Imprimante_TaillePieddePage).toInt());
    ui->TopMargespinBox                 ->setValue(proc->settings()->value(Imprimante_TailleTopMarge).toInt());
    ui->ApercuImpressioncheckBox        ->setChecked(proc->settings()->value(Imprimante_ApercuAvantImpression).toString() ==  "YES");
    ui->OrdoAvecDuplicheckBox           ->setChecked(proc->settings()->value(Imprimante_OrdoAvecDupli).toString() ==  "YES");
    ui->Imprimanteframe                 ->setToolTip(tr("Evitez de modifier les réglages de l'imprimante") + "\n"
                                                     + tr("Les résultats peuvent être assez surprenants!") + "\n"
                                                     + tr("Cliquez sur le bouton Oups! pour revenir aux réglages par défaut"));

    QString A = proc->settings()->value(Utils::getBaseFromMode(Utils::ReseauLocal) + PrioritaireGestionDocs).toString();
    if (A==YESimport)
        ui->PrioritaireImportDocscheckBox->setChecked(true);
    else if (A==NOimport)
        ui->NonImportDocscheckBox->setChecked(true);
    else
    {
        ui->NonPrioritaireImportDocscheckBox->setChecked(true);
        proc->settings()->setValue(Utils::getBaseFromMode(Utils::ReseauLocal) + PrioritaireGestionDocs,NORMimport);
    }
    t_timerverifimportdocs.start(500);
    connect (&t_timerverifimportdocs,   &QTimer::timeout,           this,   &dlg_param::VerifPosteImportDocs);
    connect (proc,                        &Procedures::ConnectTimers, this,   [=](bool a) {ConnectTimers(a);});

     if (m_parametres->versionbase() == 0)
        ui->VersionBaselabel->setText("<font color=\"red\"><b>" + tr("inconnue") + "</b></font>");
    else
        ui->VersionBaselabel->setText("<font color=\"green\"><b>" + QString::number(m_parametres->versionbase()) + "</b></font>");
    ui->VersionBaseIOLlabel->setText("<font color=\"green\"><b>" + QLocale(QLocale::English).toString(DataBase::I()->parametres()->versionbaseiol(),'f',1) + "</b></font>");
    ui->VersionRufuslabel->setText(" <font color=\"green\"><b>" + qApp->applicationVersion() + "</b></font>");

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

    ui->BackupRestoreframe         ->setEnabled(db->ModeAccesDataBase() != Utils::Distant);
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

    QListWidget *contents   = new QListWidget(ui->LanguagecomboBox);
    contents                ->hide();
    ui->LanguagecomboBox    ->setModel(contents->model());
    QListWidgetItem *frwidg = new QListWidgetItem (QPixmap("://France.ico"), "Français");
    frwidg                  ->setData(Qt::UserRole, "FR");
    contents                ->addItem(frwidg);
    QListWidgetItem *enwidg = new QListWidgetItem (QPixmap("://United-kingdom.ico"), "English");
    enwidg                  ->setData(Qt::UserRole, "EN");
    contents                ->addItem(enwidg);
    QListWidgetItem *eswidg = new QListWidgetItem (QPixmap("://Spain.ico"), "Español");
    eswidg                  ->setData(Qt::UserRole, "ES");
    contents                ->addItem(eswidg);
    QListWidgetItem *brwidg = new QListWidgetItem (QPixmap("://Brazil.ico"), "Brasileiro");
    brwidg                  ->setData(Qt::UserRole, "BR");
    contents                ->addItem(brwidg);
    QListWidgetItem *ptwidg = new QListWidgetItem (QPixmap("://Portugal.ico"), "Português");
    ptwidg                  ->setData(Qt::UserRole, "PT");
    contents                ->addItem(ptwidg);
    QListWidgetItem *itwidg = new QListWidgetItem (QPixmap("://Italy.ico"), "Italiano");
    itwidg                  ->setData(Qt::UserRole, "IT");
    contents                ->addItem(itwidg);
    QList<QListWidgetItem*> listwidg = QList<QListWidgetItem*>() << enwidg << frwidg << eswidg << brwidg << ptwidg << itwidg;
    foreach (QListWidgetItem *item, listwidg) {
        if (item->data(Qt::UserRole).toString() == m_parametres->version())
        {
            contents->setCurrentItem(item);
            break;
        }
    }
    ui->LanguagecomboBox->setCurrentIndex(contents->currentRow());
    connect (ui->LanguagecomboBox, &QComboBox::currentIndexChanged, this, [=](int idx) {db->setVersion(contents->item(idx)->data(Qt::UserRole).toString());
                                                                                        proc->settings()->setValue(Param_Poste_Version, contents->item(idx)->data(Qt::UserRole));
                                                                                        ShowMessage::I()->SplashMessage(tr("Le changement de version ne prendra effet qu'après redémarrage du logiciel"),6000);});

    Remplir_Tables();
    ConnectSignals();
}

dlg_param::~dlg_param()
{
    delete ui;
}


/*!
 * \brief dlg_param::AfficheToolTip
 * \param idx  index d'une cellule de cotationsUpTableView
 * infobulle de la table des cotations : le tip (descriptif) de la Cotation portée par la ligne
 */
void dlg_param::AfficheToolTip(QModelIndex idx)
{
    Cotation *cot = getCotationFromIndex(idx);
    if (cot == Q_NULLPTR)
        return;
    const QRect rect = QRect(cursor().pos(), QSize(10,10));
    QToolTip::showText(cursor().pos(), cot->descriptif(), ui->cotationsUpTableView, rect, 2000);
}

void dlg_param::FermepushButtonClicked()
{
    if (m_modifposte)
    {
        UpMessageBox msgbox(this);
        msgbox.setText(tr("Modifications non enregistrées!"));
        msgbox.setInformativeText(tr("Vous avez modifié certains éléments. Voulez vous enregistrer ces modifications?"));
        msgbox.setIcon(UpMessageBox::Warning);
        UpSmallButton AnnulBouton(tr("Annuler"));
        UpSmallButton OKBouton(tr("Enregistrer"));
        msgbox.addButton(&AnnulBouton, UpSmallButton::CANCELBUTTON);
        msgbox.addButton(&OKBouton, UpSmallButton::STARTBUTTON);
        msgbox.exec();
        if (msgbox.clickedButton() == &OKBouton)
        {
            if (!Valide_Modifications())
                return;
        }
        //! si on annule alors qu'on a entretemps modifié la base des villes, on revient à l'ancienne base de villes
        else if (m_custombasevilles != Datas::I()->villes->iscustomizedbase())
        {
            if (m_custombasevilles)
                Datas::I()->villes->initListe(Villes::CUSTOM);
            else
                Datas::I()->villes->initListe(Villes::DATABASE);
        }
    }
    if (VerifDirStockageImagerie())
        reject();
}

void dlg_param::EnableSupprAppareilBouton()
{
   wdg_appareilswdgbuttonframe->wdg_moinsBouton->setEnabled(true);
}

void dlg_param::scrollToCotation(QString txt)
{
    //! recherche la cotation dont le Typeacte (colonne 0) commence par txt et amène la table dessus
    if (m_modelCotations == Q_NULLPTR)
        return;
    const QList<QStandardItem*> items = m_modelCotations->findItems(txt, Qt::MatchStartsWith, 0);
    if (items.isEmpty())
        return;
    const QModelIndex index = m_modelCotations->indexFromItem(items.at(0));
    ui->cotationsUpTableView    ->scrollTo(index, QAbstractItemView::PositionAtTop);
    ui->cotationsUpTableView    ->setCurrentIndex(index);   //! sélectionne la ligne (met à jour les boutons +/-/modifier)
}

void dlg_param::ChoixDossierEchangeAppareilImagerie(UpPushButton *butt)
{
    /*! il faut utiliser la fonction static QFileDialog::getExistingDirectoryUrl() parce que la QFileDialog implémentée dans Qt ne donne pas accès aux lecteurs réseaux sous linux
     * avec la fonction static, on utilise la boîte de dialog du système
     * bien sûr, il faut paramétrer le fstab sous linux pour que le dossier réseau soit ouvert automatiquement au moment du boot*/

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
    if (dir == "" || !QDir(dir).exists())
        dir = QDir::homePath();
    QUrl url = Utils::getExistingDirectoryUrl(this, "", QUrl::fromLocalFile(dir), QStringList()<<m_parametres->dirbkup());
    if (url == QUrl())
        return;
    int row;
    UpLineEdit *line = Q_NULLPTR;
    switch (mode) {
    case Utils::Poste:
        row = ui->MonoDocupTableWidget->findItems(QString::number(butt->iD()), Qt::MatchExactly).at(0)->row();
        line    = qobject_cast<UpLineEdit*>(ui->MonoDocupTableWidget->cellWidget(row,2));
        if (line!=Q_NULLPTR)
            line->setText(url.path());
        break;
    case Utils::ReseauLocal:
        row = ui->LocalDocupTableWidget->findItems(QString::number(butt->iD()), Qt::MatchExactly).at(0)->row();
        line    = qobject_cast<UpLineEdit*>(ui->LocalDocupTableWidget->cellWidget(row,2));
        if (line!=Q_NULLPTR)
            line->setText(url.path());
        break;
    case Utils::Distant:
        row = ui->DistantDocupTableWidget->findItems(QString::number(butt->iD()), Qt::MatchExactly).at(0)->row();
        line    = qobject_cast<UpLineEdit*>(ui->DistantDocupTableWidget->cellWidget(row,2));
        if (line!=Q_NULLPTR)
            line->setText(url.path());
        break;
    default:
        break;
    }
    proc->settings()->setValue(Utils::getBaseFromMode(mode) + Dossier_Documents + exam, url.path());
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
    {
        if (dir != "")
            proc->settings()->setValue(Utils::getBaseFromMode(mode) + Dossier_Documents + app, dir);
        else
            proc->settings()->remove(Utils::getBaseFromMode(mode) + Dossier_Documents + app);
    }
    else
        UpMessageBox::Watch(this,tr("Impossible de retrouver le nom de l'appareil"));

}

void dlg_param::ChoixButtonFrame(WidgetButtonFrame *widgbutt)
{
    if (widgbutt== wdg_cotationswdgbuttonframe)
    {
        switch (wdg_cotationswdgbuttonframe->Choix()) {
        case WidgetButtonFrame::Plus:
            NouvCotation();
            break;
        case WidgetButtonFrame::Modifier:
            ModifCotation();
            break;
        case WidgetButtonFrame::Moins:
            supprimeCotation(getCotationFromIndex(ui->cotationsUpTableView->currentIndex()));
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
    dlg_fontdialog *Dlg_Fonts = new dlg_fontdialog(this);
    if (Dlg_Fonts->exec() == QDialog::Accepted)
    {
        QString fontrequete = "update " TBL_UTILISATEURS " set " CP_POLICEECRAN_USR " = '" + Dlg_Fonts->font().family()
                                + "', " CP_POLICEATTRIBUT_USR " = '" + Dlg_Fonts->fontAttribut()
                                + "' where " CP_ID_USR " = " + QString::number(currentuser()->id());
        db->StandardSQL(fontrequete,"dlg_param::ChoixFontpushButtonClicked()");
    }
    delete Dlg_Fonts;
}

void dlg_param::ClearPortsComboBox(UpComboBox* box, int a)
{
    if (box == Q_NULLPTR)
        return;
    if (box==ui->AutorefupComboBox)
    {
        if (a == 0)
        {
            proc->settings()->remove(Param_Poste_Autoref);
            ui->PortAutorefupComboBox   ->setCurrentIndex(0);
            EnableComOrNetworkWidgetsAppareilRefraction(ui->PortAutorefupComboBox, N_NULL);
        }
        else
            proc->settings()->setValue(Param_Poste_Autoref, box->currentText());
        ui->PortAutorefupComboBox->setEnabled(a>0);
    }
    else if (box==ui->FrontoupComboBox)
    {
        if (a == 0)
        {
            proc->settings()->remove(Param_Poste_Fronto);
            ui->PortFrontoupComboBox   ->setCurrentIndex(0);
            EnableComOrNetworkWidgetsAppareilRefraction(ui->PortFrontoupComboBox, N_NULL);
        }
        else
            proc->settings()->setValue(Param_Poste_Fronto, box->currentText());
        ui->PortFrontoupComboBox->setEnabled(a>0);
    }
    else if (box==ui->RefracteurupComboBox)
    {
        if (a == 0)
        {
            proc->settings()->remove(Param_Poste_Refracteur);
            ui->PortRefracteurupComboBox   ->setCurrentIndex(0);
            EnableComOrNetworkWidgetsAppareilRefraction(ui->PortRefracteurupComboBox,  N_NULL);
        }
        else
            proc->settings()->setValue(Param_Poste_Refracteur, box->currentText());
        ui->PortRefracteurupComboBox->setEnabled(a>0);
    }
    else if (box==ui->TonometreupComboBox)
    {
        if (a == 0)
        {
            proc->settings()->remove(Param_Poste_Tono);
            ui->PortTonometreupComboBox   ->setCurrentIndex(0);
            EnableComOrNetworkWidgetsAppareilRefraction(ui->PortTonometreupComboBox, N_NULL);
        }
        else
            proc->settings()->setValue(Param_Poste_Tono, box->currentText());
        ui->PortTonometreupComboBox->setEnabled(a>0);
    }
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
        if (ui->LockParamPosteupLabel->pixmap().toImage() == Icons::pxVerrouiller().toImage())
        {
            QString mdp("");
            m_MDPadminverifie = Utils::VerifMDP(proc->MDPAdmin(),"Saisissez le mot de passe Administrateur", mdp, m_MDPadminverifie, this);
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
        bool a = (ui->LockParamPosteupLabel->pixmap().toImage() == Icons::pxDeverouiller().toImage());
        EnableWidgContent(ui->Instrmtsframe,a);
        if (a && ui->FrontoupComboBox       ->currentIndex()==0)    ui->PortFrontoupComboBox->setEnabled(false);
        if (a && ui->RefracteurupComboBox   ->currentIndex()==0)    ui->PortRefracteurupComboBox->setEnabled(false);
        if (a && ui->AutorefupComboBox      ->currentIndex()==0)    ui->PortAutorefupComboBox->setEnabled(false);
        if (a && ui->TonometreupComboBox    ->currentIndex()==0)    ui->PortTonometreupComboBox->setEnabled(false);
        ui->ParamConnexiontabWidget         ->setEnabled(a);
        EnableWidgContent(ui->Imprimanteframe,a);
        EnableWidgContent(ui->VilleDefautframe,a);
        ui->ModifListVillesupPushButton     ->setEnabled(a);
        ui->VillesgroupBox                  ->setEnabled(a);
        ui->ImportDocsgroupBox              ->setEnabled(a);
    }
    else if (obj == ui->LockParamUserupLabel)
    {
        if (ui->LockParamUserupLabel->pixmap().toImage() == Icons::pxVerrouiller().toImage())
        {
            QString mdp("");
            m_MDPuserverifie = Utils::VerifMDP(currentuser()->password(),tr("Saisissez votre mot de passe"), mdp, m_MDPuserverifie, this);
            if (m_MDPuserverifie)
                ui->LockParamUserupLabel->setPixmap(Icons::pxDeverouiller());
        }
        else
            ui->LockParamUserupLabel->setPixmap(Icons::pxVerrouiller());
        bool a = (ui->LockParamUserupLabel->pixmap().toImage() == Icons::pxDeverouiller().toImage());
        ui->ChoixFontupPushButton   ->setEnabled(a);
        ui->ModifDataUserpushButton ->setEnabled(a);
        enableCotations(a);
        RegleCotationsBoutons();        //! pas de changement de ligne ici -> on règle les boutons +/-/modifier à la main
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
        if (ui->LockParamGeneralupLabel->pixmap().toImage() == Icons::pxVerrouiller().toImage())
        {
            QString mdp("");
            m_MDPadminverifie = Utils::VerifMDP(proc->MDPAdmin(),tr("Saisissez le mot de passe Administrateur"), mdp, m_MDPadminverifie, this);
            if (m_MDPadminverifie)
            {
                ui->LockParamGeneralupLabel ->setPixmap(Icons::pxDeverouiller());
                wdg_appareilswdgbuttonframe->wdg_moinsBouton      ->setEnabled(ui->AppareilsConnectesupTableWidget->selectedItems().size()>0);
            }
        }
        else
            ui->LockParamGeneralupLabel->setPixmap(Icons::pxVerrouiller());
        bool a = (ui->LockParamGeneralupLabel->pixmap().toImage() == Icons::pxDeverouiller().toImage());
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
        EnableWidgContent(ui->BackupRestoreframe, db->ModeAccesDataBase() != Utils::Distant && a);
        EnableWidgContent(ui->Languagewidget, a);
    }
}

void dlg_param::EnableFrameServeur(QCheckBox *box, bool a)
{
    if (box == ui->PosteServcheckBox)
    {
        ui->Posteframe                  ->setVisible(a);
        ui->MonoDocsExtupLabel          ->setVisible(a);
        ui->MonoDocupTableWidget        ->setVisible(a);
        ui->Posteframe                  ->setEnabled(a);
        ui->MonoDocsExtupLabel          ->setEnabled(a);
        ui->MonoDocupTableWidget        ->setEnabled(a);
        ui->PosteStockageupLabel        ->setVisible(a && DataBase::I()->ModeAccesDataBase() == Utils::Poste);
        ui->PosteStockageInfoUpLabel    ->setVisible(a && DataBase::I()->ModeAccesDataBase() == Utils::Poste);
        ui->PosteVideoDirupLabel        ->setVisible(a && DataBase::I()->ModeAccesDataBase() == Utils::Poste);
        ui->PosteVideoDirupLineEdit     ->setVisible(a && DataBase::I()->ModeAccesDataBase() == Utils::Poste);
        ui->PosteVideoDirupPushButton   ->setVisible(a && DataBase::I()->ModeAccesDataBase() == Utils::Poste);
    }
    else if (box == ui->LocalServcheckBox)
    {
        ui->Localframe                      ->setVisible(a);
        ui->LocalDocsExtupLabel             ->setVisible(a);
        ui->LocalDocupTableWidget           ->setVisible(a);
        ui->LocalPathStockageupLabel        ->setVisible(a);
        ui->LocalPathStockageupLineEdit     ->setVisible(a);
        ui->LocalPathStockageupPushButton   ->setVisible(a);
        ui->LocalVideoDirupLabel            ->setVisible(a);
        ui->LocalVideoDirupLineEdit         ->setVisible(a);
        ui->LocalVideoDirupPushButton       ->setVisible(a);
        ui->Localframe                      ->setEnabled(a);
        ui->LocalDocsExtupLabel             ->setEnabled(a);
        ui->LocalDocupTableWidget           ->setEnabled(a);
        ui->LocalPathStockageupLabel        ->setEnabled(a);
        ui->LocalPathStockageupLineEdit     ->setEnabled(a);
        ui->LocalPathStockageupPushButton   ->setEnabled(a);
        ui->LocalVideoDirupLabel            ->setEnabled(a);
        ui->LocalVideoDirupLineEdit         ->setEnabled(a);
        ui->LocalVideoDirupPushButton       ->setEnabled(a);
    }
    else if (box == ui->DistantServcheckBox)
    {
        if (a)
            UpMessageBox::Watch(this,tr("Configuration pour un accès distant"),
                                     tr("L'accès distant fonctionne obligatoirement "
                                         "avec un cryptage de données SSL.") + "<br/>" +
                                     tr("Si vous voulez utiliser l'accès distant "
                                         "sur ce poste, il faut vous assurer que:") + "<br/>" +
                                     tr("1. le serveur MySQL est configuré pour le cryptage") + "<br/>" +
                                     tr("2. que les clés clientes SSL sont installées sur ce poste.") + "<br/><br/>" +
                                     tr("Reportez vous à la page internet") + "<br/>" +
                                        "<a href=\'https://www.rufusvision.org/configuration-pour-une-connexion-par-internet---le-cryptage-ssl.html\'>www.rufusvision.org/configuration-pour-une-connexion-par-internet---le-cryptage-ssl.html</a><br/>",
                                     UpDialog::ButtonOK, "www.rufusvision.org/configuration-pour-une-connexion-par-internet---le-cryptage-ssl.html"
                                    );
        ui->Distantframe                ->setVisible(a);
        ui->Distantframe                ->setEnabled(a);
        ui->DistantStockageupLabel      ->setVisible(a);
        ui->DistantStockageupLineEdit   ->setVisible(a);
        ui->DistantStockageupPushButton ->setVisible(a);
        ui->DistantVideoDirupLabel      ->setVisible(a);
        ui->DistantVideoDirupLineEdit   ->setVisible(a);
        ui->DistantVideoDirupPushButton ->setVisible(a);
     }
}

void dlg_param::EnableOKModifPosteButton()
{
    m_modifposte = true;
}

void dlg_param::GestionBanques()
{
    dlg_gestionbanques *Dlg_Banq = new dlg_gestionbanques(this);
    Dlg_Banq->exec();
}

void dlg_param::GestionDatasCurrentUser()
{

    dlg_gestionusers *Dlg_GestUsr = new dlg_gestionusers(Datas::I()->sites->idcurrentsite(), dlg_gestionusers::MODIFUSER, m_MDPuserverifie, this);
    Dlg_GestUsr->setWindowModality(Qt::WindowModal);
    if (Dlg_GestUsr->exec() == QDialog::Accepted)
    {
        m_donneesusermodifiees = true;
        Datas::I()->users->reload(currentuser());
        proc->MAJComptesBancaires(currentuser());
        AfficheParamUser();
        if (!m_MDPuserverifie)
            m_MDPuserverifie = Dlg_GestUsr->isMDPverified();
    }
    else
        m_donneesusermodifiees = false;
    delete Dlg_GestUsr;
}

void dlg_param::GestionUsers()
{
    dlg_gestionusers *Dlg_GestUsr = new dlg_gestionusers(Datas::I()->sites->idcurrentsite(), dlg_gestionusers::ADMIN, m_MDPadminverifie, this);
    Dlg_GestUsr->setWindowModality(Qt::WindowModal);
    if (Dlg_GestUsr->exec()>0)
    {
        m_donneesusermodifiees = true;
        Datas::I()->users   ->initListe();
        proc                ->MAJComptesBancaires(currentuser());
        AfficheParamUser();
    }
    else
        m_donneesusermodifiees = false;
    delete Dlg_GestUsr;
    UpMessageBox::Watch(this, tr("Données utilisateurs modifiées?"),
                              tr("Si vous avez modifié des données d'utilisateurs actuellement connectés,\n"
                                 "chacun de ces utilisateurs doit relancer le programme\n"
                                 "pour pouvoir prendre en compte les modifications apportées!"));
}

void dlg_param::GestionLieux()
{
    dlg_listelieux *gestLieux = new dlg_listelieux(this);
    gestLieux->exec();
    ReconstruitListeLieuxExerciceAllusers();
    delete gestLieux;
}

void dlg_param::ReconstruitListeLieuxExerciceUser(User *user)
{
    /*-------------------- GESTION DES LIEUX D'EXERCICE-------------------------------------------------------*/
    ui->AdressupTableWidget->clear();
    ReconstruitListeLieuxExerciceAllusers();        // (re)remplit la combobox de l'emplacement serveur
    int             ColCount = 4;

    ui->AdressupTableWidget->setColumnCount(ColCount);
    ui->AdressupTableWidget->verticalHeader()->setVisible(false);
    ui->AdressupTableWidget->setGridStyle(Qt::NoPen);
    int li = 0;                                                                                   // Réglage de la largeur et du nombre des colonnes
    ui->AdressupTableWidget->setColumnWidth(li,200);                                              // nom du lieu
    li++;
    ui->AdressupTableWidget->setColumnWidth(li,140);                                              // ville
    li++;
    ui->AdressupTableWidget->setColumnWidth(li,110);                                              // téléphone
    li++;
    ui->AdressupTableWidget->setColumnWidth(li,30);                                              // téléphone
    ui->AdressupTableWidget->FixLargeurTotale();
    if ((!currentuser()->isMedecin()&& !currentuser()->isOrthoptist()) || currentuser()->isRemplacant() || !currentuser()->usenum())
        ui->AdressupTableWidget->setColumnHidden(3,true);

    UpHeaderView *upheader = new UpHeaderView(ui->AdressupTableWidget->horizontalHeader());
    upheader->setVisible(true);
    QStringList list;
    list << tr("Liste des lieux d'exercice");
    QStandardItemModel *mod = new QStandardItemModel(this);
    mod->setHorizontalHeaderLabels(list);
    upheader->setModel(mod);
    upheader->reDim(0,0,ui->AdressupTableWidget->columnCount()-1);

    QMap<Site*,qlonglong> mapsites = Datas::I()->sites->initListeByUser(user->id());
    ui->AdressupTableWidget->setRowCount(mapsites.size());
    int i = 0;
    for (auto it = mapsites.cbegin(); it != mapsites.cend(); ++it)
    {
        Site *sit = qobject_cast<Site*>(it.key());
        if (sit != Q_NULLPTR)
        {
            QTableWidgetItem *pitem1, *pitem2, *pitem3;
            UpPushButton        *AMnumberbutton = new UpPushButton;
            pitem1 = new QTableWidgetItem();
            pitem2 = new QTableWidgetItem();
            pitem3 = new QTableWidgetItem();
            pitem1->setText(sit->nom());
            pitem2->setText(sit->ville());
            pitem3->setText(sit->telephone());
            AMnumberbutton->setIcon(Icons::icSortirDossier());
            AMnumberbutton->setIconSize(QSize(15,15));
            AMnumberbutton->setiD(currentuser()->AMnumberforSite(sit->id()));
            AMnumberbutton->setFixedSize(15,15);
            AMnumberbutton->setFlat(true);
            AMnumberbutton->setFocusPolicy(Qt::NoFocus);
            AMnumberbutton->setData(sit->id());
            QWidget *widg = new QWidget;
            QHBoxLayout *l = new QHBoxLayout();
            l->setAlignment( Qt::AlignCenter );
            l->setContentsMargins(0,0,0,0);
            l->addWidget(AMnumberbutton);
            widg->setLayout(l);
            ui->AdressupTableWidget->setItem(i,0,pitem1);
            ui->AdressupTableWidget->setItem(i,1,pitem2);
            ui->AdressupTableWidget->setItem(i,2,pitem3);
            ui->AdressupTableWidget->setCellWidget(i,3,widg);
            pitem1->setToolTip(sit->coordonnees());
            pitem2->setToolTip(sit->coordonnees());
            pitem3->setToolTip(sit->coordonnees());
            QString tooltiptxt = (AMnumberbutton->iD()>0?
                        tr("Numero AM ") + QString::number(AMnumberbutton->iD()) :
                        tr("Pas de numéro AM enregistré pour ce site")) + "\n" +
                        tr("Cliquez sur le bouton pour modifier le numéro");
            AMnumberbutton->setImmediateToolTip(tooltiptxt);
            if (sit->couleur() != "")
            {
                pitem1->setForeground(QColor("#" + sit->couleur()));
                pitem2->setForeground(QColor("#" + sit->couleur()));
                pitem3->setForeground(QColor("#" + sit->couleur()));
            }
            ui->AdressupTableWidget->setRowHeight(i,int(QFontMetrics(qApp->font()).height()*1.3));
            ui->AdressupTableWidget->FixLargeurTotale(0);
            connect(AMnumberbutton,     &QPushButton::clicked,  this,   [=] {fixAMnumberforSite(AMnumberbutton, mapsites);});
            i++;
        }
    }
}

void dlg_param::fixAMnumberforSite(UpPushButton *AMnumberButton, QMap<Site *, qlonglong> mapsites)
{
    qlonglong AMnumber = 0;
    UpDialog* dlg_ask               = new UpDialog(this);
    int w = 240;
    UpLabel         *lbldebut       = new UpLabel;
    lbldebut        ->setText(tr("Enregistrez le numéro AM (9 chiffres) correspondant à") +
                              "<br/><font color=\"blue\"><b>" + Datas::I()->sites->getById(AMnumberButton->data())->nom() + "</b></font>");
    lbldebut->setAlignment(Qt::AlignCenter);
    QHBoxLayout *hlay = new QHBoxLayout;
    hlay->addSpacerItem(new QSpacerItem(5,5,QSizePolicy::Expanding));

    QStringList listnumAM = QStringList();
    for (auto it = mapsites.cbegin(); it != mapsites.cend(); ++it)
        if (it.value() >0)
            if (!listnumAM.contains(QString::number(it.value())))
                listnumAM << QString::number(it.value());
    if (currentuser()->m_numAM() >0)
        if (!listnumAM.contains(QString::number(currentuser()->m_numAM())))
            listnumAM << QString::number(currentuser()->m_numAM());
    std::sort(listnumAM.begin(), listnumAM.end());

    UpComboBox *combo   = new UpComboBox;
    combo               ->setFixedWidth(w);
    QLineEdit *line     = new QLineEdit;
    QCompleter *comp    = new QCompleter(listnumAM);
    comp                ->setCompletionMode(QCompleter::InlineCompletion);
    line                ->setCompleter(comp);
    line                ->setValidator(new QRegularExpressionValidator(QRegularExpression("[0-9]{9}"), Q_NULLPTR));
    combo               ->addItems(listnumAM);
    combo               ->setLineEdit(line);
    combo               ->setEditable(true);
    combo               ->setCurrentText("");

    hlay                ->addWidget(combo);
    hlay                ->addSpacerItem(new QSpacerItem(5,5,QSizePolicy::Expanding));
    dlg_ask->dlglayout()->insertWidget(0,lbldebut);
    dlg_ask->dlglayout()->insertLayout(1,hlay);
    dlg_ask             ->AjouteLayButtons(UpDialog::ButtonCancel | UpDialog::ButtonOK);
    dlg_ask             ->setWindowTitle(tr("Enregistrement numéro AM"));
    dlg_ask->dlglayout()->setSizeConstraint(QLayout::SetFixedSize);
    connect(dlg_ask->OKButton,  &QPushButton::clicked,  dlg_ask,    [&] {AMnumber = combo->currentText().toLongLong();
        dlg_ask->accept();});
    if (dlg_ask->exec() == QDialog::Accepted)
    {
        if (AMnumber>0)
        {
            mapsites.insert(Datas::I()->sites->getById(AMnumberButton->data()), AMnumber);
            QMap<int, qlonglong> mapid = QMap<int, qlonglong>();
            for (auto it = mapsites.begin(); it != mapsites.end(); ++it)
                mapid.insert(it.key()->id(), it.value());
            currentuser()->setmapUserSites(mapid);
            QString req ="update " TBL_JOINTURESLIEUX " set " CP_AMNUMBER_JOINTSITE " = " +  QString::number(AMnumber) +
                            " where " CP_IDUSER_JOINTSITE  " = " + QString::number(currentuser()->id())  + " and " CP_IDLIEU_JOINTSITE " = " + QString::number(AMnumberButton->data());
            db->StandardSQL(req);
            qDebug() << req;
            AMnumberButton->setiD(AMnumber);
            AMnumberButton->setImmediateToolTip(tr("Numero AM ") + QString::number(AMnumberButton->iD()));
        }
    }
    delete dlg_ask;
}


/*! ReconstruitListeLieuxExerciceAllusers();
*-------------------- GESTION DES LIEUX D'EXRCICE-------------------------------------------------------*/

void dlg_param::ReconstruitListeLieuxExerciceAllusers()
{
    disconnect(ui->EmplacementServeurupComboBox,    QOverload<int>::of(&QComboBox::currentIndexChanged),    this,   &dlg_param::EnregistreEmplacementServeur);
    ui->EmplacementServeurupComboBox->clear();
    foreach (Site* sit, *Datas::I()->sites->sites())
        ui->EmplacementServeurupComboBox->addItem(sit->nom() + " " + sit->ville(), sit->id());
    if (m_parametres->idlieupardefaut() > 0)
        ui->EmplacementServeurupComboBox->setCurrentIndex(ui->EmplacementServeurupComboBox->findData(m_parametres->idlieupardefaut()));
    else if (ui->EmplacementServeurupComboBox->count() > 0)
    {
        ui->EmplacementServeurupComboBox->setCurrentIndex(0);
        EnregistreEmplacementServeur(0);
    }
    connect(ui->EmplacementServeurupComboBox,       QOverload<int>::of(&QComboBox::currentIndexChanged),    this,   &dlg_param::EnregistreEmplacementServeur);
}

void dlg_param::NouvAppareil()
{
    dlg_askappareil = new UpDialog(this);
    dlg_askappareil->setWindowModality(Qt::WindowModal);
    dlg_askappareil->move(QPoint(x()+width()/2,y()+height()/2));
    dlg_askappareil->setFixedSize(400,100);
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
    dlg_askappareil->AjouteLayButtons(UpDialog::ButtonCancel | UpDialog::ButtonOK);
    connect(dlg_askappareil->OKButton,    &QPushButton::clicked,    this,   &dlg_param::EnregistreAppareil);
    dlg_askappareil->exec();
    delete dlg_askappareil;
    dlg_askappareil = Q_NULLPTR;
}

void dlg_param::startImmediateBackup()
{
    if (proc->AutresPostesConnectes())
        return;
    QString dirsauvorigin   = ui->DirBackupuplineEdit->text();
    if (dirsauvorigin == "" || !QDir(dirsauvorigin).exists())
        dirsauvorigin = QDir::homePath();
    QUrl url = Utils::getExistingDirectoryUrl(this, tr("Choisissez le dossier dans lequel vous voulez sauvegarder la base\n"
                                                       "Le nom de dossier ne doit pas contenir d'espace"), QUrl::fromLocalFile(dirsauvorigin), QStringList(), false);
    if (url == QUrl())
        return;
    if (QDir(url.path()).exists())
        proc->ImmediateBackup(url.path(), false, false, this);
}

/*!
 * \brief dlg_param::MAJCotation
 * \param cot  la cotation concernée (déduite par l'appelant via getCotationFromIndex)
 * mise à jour d'une cotation (coche/décoche, montant pratiqué...) : à écrire ultérieurement
 */
void dlg_param::MAJCotation(Cotation *cot)
{
    Q_UNUSED(cot)
}

void dlg_param::SupprAppareil()
{
    if (ui->AppareilsConnectesupTableWidget->selectedItems().size()==0)
        return;
    bool ok;
    QString req = " select list." CP_TITREEXAMEN_APPAREIL ", list." CP_NOMAPPAREIL_APPAREIL " from " TBL_LISTEAPPAREILS " list, " TBL_APPAREILSCONNECTESCENTRE " appcon"
                  " where list." CP_ID_APPAREIL " = appcon." CP_ID_APP
                  " and list." CP_ID_APPAREIL " = " + ui->AppareilsConnectesupTableWidget->selectedItems().at(0)->text();
    QVariantList appdata = db->getFirstRecordFromStandardSelectSQL(req, ok);
    if (!ok || appdata.size()==0)
        return;
    UpMessageBox msgbox(this);
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
        req = "delete from " TBL_APPAREILSCONNECTESCENTRE " where " CP_ID_APP " = "
              + ui->AppareilsConnectesupTableWidget->selectedItems().at(0)->text()
              + " and " CP_IDLIEU_APP " = " + QString::number(Datas::I()->sites->idcurrentsite());
        db->StandardSQL(req);
        proc->settings()->remove(Utils::getBaseFromMode(db->ModeAccesDataBase()) + Dossier_Documents + appdata.at(1).toString());
        Remplir_Tables();
    }
}

/*!
 * \brief dlg_param::cotationUtiliseeParAutre
 * \param cot  la cotation testée
 * vrai si un utilisateur AUTRE que le courant possède cette cotation dans sa table de jointure
 * (jointure choisie selon le type de la cotation).
 */
bool dlg_param::cotationUtiliseeParAutre(Cotation *cot)
{
    if (cot == Q_NULLPTR)
        return false;
    QString jointure, chpIdcotation, chpIduser;
    switch (cot->typcotation())
    {
        case 1: jointure = TBL_JOINTURESCCAM;              chpIdcotation = CP_IDCOTATION_JOINTCCAM;             chpIduser = CP_IDUSER_JOINTCCAM;             break;   //! CCAM
        case 2: jointure = TBL_JOINTURESASSOCIATIONS;      chpIdcotation = CP_IDCOTATION_JOINTASSOCIATIONS;     chpIduser = CP_IDUSER_JOINTASSOCIATIONS;     break;   //! association CCAM
        case 3: jointure = TBL_JOINTURESNGAP;              chpIdcotation = CP_IDCOTATION_JOINTNGAP;             chpIduser = CP_IDUSER_JOINTNGAP;             break;   //! NGAP
        case 4: jointure = TBL_JOINTURESAUTRESCOTATIONS;   chpIdcotation = CP_IDCOTATION_JOINTAUTRESCOTATIONS;  chpIduser = CP_IDUSER_JOINTAUTRESCOTATIONS;  break;   //! autre (hors NGAP/CCAM)
        default: return false;
    }
    bool ok = false;
    QList<QVariantList> l = db->StandardSelectSQL("select 1 from " + jointure
                            + " where " + chpIdcotation + " = " + QString::number(cot->id())
                            + " and " + chpIduser + " <> " + QString::number(currentuser()->id()) + " limit 1", ok);
    return (ok && !l.isEmpty());
}

/*!
 * \brief dlg_param::RegleCotationsBoutons
 * règle l'état des 3 boutons du buttonframe de cotationsUpTableView. Ils ne sont TOUS actifs que si
 * l'utilisateur est son propre parent (un remplaçant ne modifie pas les cotations du parent) :
 * - « + » : toujours (dans ce cas) ;
 * - « modifier » : seulement un « autre » (4) — un acte CCAM a ses montants figés ;
 * - « - » : type 1, 2 ou 4 (pas NGAP) et si aucun autre utilisateur ne l'utilise.
 */
void dlg_param::RegleCotationsBoutons()
{
    const bool sonParent = (currentuser()->idparent() == currentuser()->id());
    Cotation *cot = getCotationFromIndex(ui->cotationsUpTableView->currentIndex());

    wdg_cotationswdgbuttonframe->wdg_plusBouton ->setEnabled(sonParent);
    wdg_cotationswdgbuttonframe->wdg_modifBouton->setEnabled(sonParent && cot != Q_NULLPTR && cot->isnorGAPnorCCAM());
    wdg_cotationswdgbuttonframe->wdg_moinsBouton->setEnabled(sonParent && cot != Q_NULLPTR
                                                             && !cot->isNGAP() && !cotationUtiliseeParAutre(cot));
}

/*!
 * \brief dlg_param::MAJMontantPratique
 * \param cot      la cotation dont le pratiqué change
 * \param montant  le nouveau montant pratiqué
 * met à jour le montant pratiqué de l'utilisateur courant dans la table de jointure correspondant au
 * type de la cotation.
 */
void dlg_param::MAJMontantPratique(Cotation *cot, double montant)
{
    if (cot == Q_NULLPTR)
        return;
    QString jointure, chpIdcotation, chpIduser, chpPratique;
    switch (cot->typcotation())
    {
        case 1: jointure = TBL_JOINTURESCCAM;              chpIdcotation = CP_IDCOTATION_JOINTCCAM;             chpIduser = CP_IDUSER_JOINTCCAM;             chpPratique = CP_MONTANTPRATIQUE_JOINTCCAM;             break;   //! CCAM
        case 2: jointure = TBL_JOINTURESASSOCIATIONS;      chpIdcotation = CP_IDCOTATION_JOINTASSOCIATIONS;     chpIduser = CP_IDUSER_JOINTASSOCIATIONS;     chpPratique = CP_MONTANTPRATIQUE_JOINTASSOCIATIONS;     break;   //! association CCAM
        case 3: jointure = TBL_JOINTURESNGAP;              chpIdcotation = CP_IDCOTATION_JOINTNGAP;             chpIduser = CP_IDUSER_JOINTNGAP;             chpPratique = CP_MONTANTPRATIQUE_JOINTNGAP;             break;   //! NGAP
        case 4: jointure = TBL_JOINTURESAUTRESCOTATIONS;   chpIdcotation = CP_IDCOTATION_JOINTAUTRESCOTATIONS;  chpIduser = CP_IDUSER_JOINTAUTRESCOTATIONS;  chpPratique = CP_MONTANTPRATIQUE_JOINTAUTRESCOTATIONS;  break;   //! autre (hors NGAP/CCAM)
        default: return;
    }
    db->StandardSQL("update " + jointure + " set " + chpPratique + " = " + QString::number(montant)
                    + " where " + chpIdcotation + " = " + QString::number(cot->id())
                    + " and " + chpIduser + " = " + QString::number(currentuser()->id()));
    cot->setmontantpratique(montant);
    m_cotationsmodifiees = true;
}

/*!
 * \brief dlg_param::MenuContextuelCotations
 * clic droit sur la table : sélectionne la rangée sous le curseur et, si l'utilisateur est son propre
 * parent, ouvre un menu (modifier le montant pratiqué / modifier la cotation / supprimer la cotation).
 */
void dlg_param::MenuContextuelCotations()
{
    QModelIndex idx = ui->cotationsUpTableView->indexAt(
        ui->cotationsUpTableView->viewport()->mapFromGlobal(QCursor::pos()));
    if (!idx.isValid())
        return;
    ui->cotationsUpTableView->setCurrentIndex(idx);         //! le clic droit sélectionne la rangée
    Cotation *cot = getCotationFromIndex(idx);
    if (cot == Q_NULLPTR)
        return;
    //! un utilisateur ne modifie que ses propres cotations : pas de menu s'il n'est pas son propre parent
    if (currentuser()->idparent() != currentuser()->id())
        return;

    QMenu menu(this);
    //! modifier le montant pratiqué : édition en place de la case pratiqué (cotation utilisée)
    if (cot->isused())
    {
        QAction *aPrat = menu.addAction(Icons::icEditer(), tr("Modifier le montant pratiqué"));
        connect(aPrat, &QAction::triggered, this, [=] {ui->cotationsUpTableView->edit(m_modelCotations->index(idx.row(), 2));});
    }
    //! modifier la cotation : uniquement un « autre » (4)
    if (cot->isnorGAPnorCCAM())
    {
        QAction *aModif = menu.addAction(Icons::icEditer(), tr("Modifier la cotation"));
        connect(aModif, &QAction::triggered, this, [=] {ModifCotation();});
    }
    //! supprimer la cotation : type 1/2/4 (pas NGAP) et si aucun autre utilisateur ne l'utilise
    if (!cot->isNGAP() && !cotationUtiliseeParAutre(cot))
    {
        QAction *aSuppr = menu.addAction(Icons::icPoubelle(), tr("Supprimer la cotation"));
        connect(aSuppr, &QAction::triggered, this, [=] {supprimeCotation(cot);});
    }
    if (!menu.isEmpty())
        menu.exec(QCursor::pos());
}

void dlg_param::ResetImprimante()
{
    UpMessageBox msgbox(this);
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
    ui->TopMargespinBox->setValue(3);
    EnableOKModifPosteButton();
}

void dlg_param::EnregistreAppareil()
{
    if (!dlg_askappareil) return;
    QString req = "insert into " TBL_APPAREILSCONNECTESCENTRE " (" CP_ID_APP ", " CP_IDLIEU_APP ") Values("
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

void dlg_param::NouvCotation()
{
    dlg_gestioncotations *Dlg_CrrCot = new dlg_gestioncotations(dlg_gestioncotations::Creation, "", this);
    if (Dlg_CrrCot->exec() == QDialog::Accepted)
    {
        Remplir_TableCotations();                       //! la case de la cotation est cochée (jointure du user créée) via isused()
        enableCotations();
        scrollToCotation(Dlg_CrrCot->codeenregistre());  //! amène la table sur la cotation créée/mise à jour
        m_cotationsmodifiees = true;
    }
    delete Dlg_CrrCot;
}

void dlg_param::ModifCotation()
{
    Cotation *cot = getCotationFromIndex(ui->cotationsUpTableView->currentIndex());
    if (cot == Q_NULLPTR)
        return;
    //! le type (association 2 / autre 4) est déterminé par la fiche à partir de la cotation
    dlg_gestioncotations *Dlg_CrrCot = new dlg_gestioncotations(dlg_gestioncotations::Modification, cot->typeacte(), this);
    if (Dlg_CrrCot->exec()>0)
    {
        Remplir_TableCotations();
        enableCotations();
        m_cotationsmodifiees = true;
    }
    delete Dlg_CrrCot;
}

/*!
 * \brief dlg_param::supprimeCotation
 * Retire la cotation sélectionnée de la table de jointure du user correspondant à son type
 * (revient au même qu'un décochage). Puis, si plus personne ne l'utilise, supprime la cotation
 * elle-même de la table cotations (sa ligne disparaît) — SAUF les NGAP (type 3), qui ne sont
 * stockées que là et qu'il faut garder.
 */
void dlg_param::supprimeCotation(Cotation *cot)
{
    if (cot == Q_NULLPTR)
        return;
    //! table de jointure (et ses colonnes idCotation/idUser) correspondant au type de la cotation
    QString jointure, chpIdcotation, chpIduser;
    switch (cot->typcotation())
    {
        case 1: jointure = TBL_JOINTURESCCAM;              chpIdcotation = CP_IDCOTATION_JOINTCCAM;             chpIduser = CP_IDUSER_JOINTCCAM;             break;   //! CCAM
        case 2: jointure = TBL_JOINTURESASSOCIATIONS;      chpIdcotation = CP_IDCOTATION_JOINTASSOCIATIONS;     chpIduser = CP_IDUSER_JOINTASSOCIATIONS;     break;   //! association CCAM
        case 3: jointure = TBL_JOINTURESNGAP;              chpIdcotation = CP_IDCOTATION_JOINTNGAP;             chpIduser = CP_IDUSER_JOINTNGAP;             break;   //! NGAP
        case 4: jointure = TBL_JOINTURESAUTRESCOTATIONS;   chpIdcotation = CP_IDCOTATION_JOINTAUTRESCOTATIONS;  chpIduser = CP_IDUSER_JOINTAUTRESCOTATIONS;  break;   //! autre (hors NGAP/CCAM)
        default: return;
    }
    const QString idcot = QString::number(cot->id());
    //! retrait de la cotation de la jointure du user
    db->StandardSQL("delete from " + jointure + " where " + chpIdcotation + " = " + idcot
                    + " and " + chpIduser + " = " + QString::number(currentuser()->id()));

    //! plus personne ne l'utilise -> on supprime la cotation (sa ligne disparaît), sauf NGAP
    if (!cot->isNGAP())
    {
        bool ok;
        QList<QVariantList> reste = db->StandardSelectSQL("select 1 from " + jointure
                                    + " where " + chpIdcotation + " = " + idcot + " limit 1", ok);
        if (ok && reste.isEmpty())
            db->StandardSQL("delete from " TBL_COTATIONS " where " CP_ID_COTATIONS " = " + idcot);
    }
    Remplir_TableCotations();
    enableCotations();
    m_cotationsmodifiees = true;
}

void dlg_param::ModifMDPAdmin()
{
    dlg_askMDP    = new UpDialog(this);
    dlg_askMDP    ->setWindowModality(Qt::WindowModal);

    UpLineEdit *ConfirmMDP = new UpLineEdit(dlg_askMDP);
    ConfirmMDP->setEchoMode(QLineEdit::Password);
    ConfirmMDP->setObjectName(m_confirmeMDP);
    ConfirmMDP->setValidator(new QRegularExpressionValidator(Utils::rgx_AlphaNumeric_5_12,this));
    ConfirmMDP->setAlignment(Qt::AlignCenter);
    ConfirmMDP->setMaxLength(12);
    dlg_askMDP->dlglayout()->insertWidget(0,ConfirmMDP);

    UpLabel *labelConfirmMDP = new UpLabel();
    labelConfirmMDP->setText(tr("Confirmez le nouveau mot de passe"));
    dlg_askMDP->dlglayout()->insertWidget(0,labelConfirmMDP);

    UpLineEdit *NouvMDP = new UpLineEdit(dlg_askMDP);
    NouvMDP->setEchoMode(QLineEdit::Password);
    NouvMDP->setObjectName(m_nouveauMDP);
    NouvMDP->setValidator(new QRegularExpressionValidator(Utils::rgx_AlphaNumeric_5_12,this));
    NouvMDP->setAlignment(Qt::AlignCenter);
    NouvMDP->setMaxLength(12);
    dlg_askMDP->dlglayout()->insertWidget(0,NouvMDP);

    UpLabel *labelNewMDP = new UpLabel();
    labelNewMDP->setText(tr("Entrez le nouveau mot de passe"));
    dlg_askMDP->dlglayout()->insertWidget(0,labelNewMDP);

    UpLineEdit *AncMDP = new UpLineEdit(dlg_askMDP);
    AncMDP->setEchoMode(QLineEdit::Password);
    AncMDP->setAlignment(Qt::AlignCenter);
    AncMDP->setValidator(new QRegularExpressionValidator(Utils::rgx_AlphaNumeric_3_12,this));
    AncMDP->setObjectName(m_ancienMDP);
    AncMDP->setMaxLength(12);
    dlg_askMDP->dlglayout()->insertWidget(0,AncMDP);
    UpLabel *labelOldMDP = new UpLabel();
    labelOldMDP->setText(tr("Entrez votre mot de passe"));
    dlg_askMDP->dlglayout()->insertWidget(0,labelOldMDP);

    AncMDP->setFocus();

    dlg_askMDP->AjouteLayButtons(UpDialog::ButtonCancel | UpDialog::ButtonOK);
    QList <QWidget*> ListTab;
    ListTab << AncMDP << NouvMDP << ConfirmMDP << dlg_askMDP->OKButton;
    for (int i = 0; i<ListTab.size()-1 ; i++ )
        dlg_askMDP->setTabOrder(ListTab.at(i), ListTab.at(i+1));
    dlg_askMDP    ->setWindowTitle(tr("Mot de passe administrateur"));
    connect(dlg_askMDP->OKButton,    &QPushButton::clicked, this, &dlg_param::EnregistreNouvMDPAdmin);
    dlg_askMDP->dlglayout()->setSizeConstraint(QLayout::SetFixedSize);

    dlg_askMDP->exec();
}

//! Équipe un champ de mot de passe MySQL (onglet « Ce Poste ») : un œil pour
//! l'afficher en clair / le remasquer, et un bouton attenant pour l'enregistrer sur
//! une clé USB (afin de le reporter sur un autre poste).
void dlg_param::ConfigureChampMDPMySQL(UpLineEdit *champ, UpSmallButton *btnUSB)
{
    QAction *oeil = champ->addAction(Icons::icEye(), QLineEdit::TrailingPosition);
    oeil->setToolTip(tr("Afficher / masquer le mot de passe"));
    connect(oeil, &QAction::triggered, this, [=]() {
        const bool masque = (champ->echoMode() == QLineEdit::Password);
        champ->setEchoMode(masque ? QLineEdit::Normal : QLineEdit::Password);
        oeil->setIcon(masque ? Icons::icEyeBarre() : Icons::icEye());   // affiché → œil barré, masqué → œil
    });

    btnUSB->setUpButtonStyle(UpSmallButton::RECORDBUTTON);
    btnUSB->setFixedSize(28, 24);                  // même hauteur que le MDPuplineEdit (UpSmallButton force sinon 45)
    btnUSB->setIconSize(QSize(18, 18));
    btnUSB->setImmediateToolTip(tr("Enregistrer ce mot de passe sur une clé USB"));
    connect(btnUSB, &QPushButton::clicked, this, [=]() {
        const QString mdp = champ->text();
        if (mdp.isEmpty())
        {
            UpMessageBox::Watch(this, tr("Aucun mot de passe à enregistrer"));
            return;
        }
        const QString dossier = QFileDialog::getExistingDirectory(
                    this, tr("Choisissez la clé USB où enregistrer le mot de passe"));
        if (dossier.isEmpty())
            return;
        QFile f(dossier + "/rufus-mdp-mysql.txt");
        if (f.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
        {
            f.write(mdp.toUtf8());
            f.write("\n");
            f.close();
            UpMessageBox::Watch(this, tr("Mot de passe enregistré"),
                                tr("Le mot de passe a été copié sur la clé USB."));
        }
        else
            UpMessageBox::Watch(this, tr("Échec de l'enregistrement"),
                                tr("Impossible d'écrire sur cet emplacement."));
    });
}

void dlg_param::ParamMotifs()
{
    dlg_motifs *Dlg_motifs = new dlg_motifs(this);
    Dlg_motifs->setWindowTitle(tr("Motifs de consultations"));
    Dlg_motifs->exec();
    delete Dlg_motifs;
}

void dlg_param::RecalcAvailablesPorts(bool fromSettings)
{
    QList<UpComboBox*> listportsbox;
    listportsbox << ui->PortFrontoupComboBox << ui->PortAutorefupComboBox << ui->PortRefracteurupComboBox << ui->PortTonometreupComboBox;

    QMap<QString,QString> initmapports = proc->mapPortsCOM();
    if (initmapports.size() >0)
    {
        //! Pour chaque changement de portCOM dans un combobox, on remet à jour la liste des ports disponibles dans les autres combobox
        if (fromSettings)
            for (int l=0; l< listportsbox.size(); ++l)
            {
                QStringList listCOMsets = QStringList();
                if (proc->settings()->value(Param_Poste_PortFronto).toString().contains("COM"))
                    listCOMsets << proc->settings()->value(Param_Poste_PortFronto).toString();
                if (proc->settings()->value(Param_Poste_PortAutoref).toString().contains("COM"))
                    listCOMsets << proc->settings()->value(Param_Poste_PortAutoref).toString();
                if (proc->settings()->value(Param_Poste_PortRefracteur).toString().contains("COM"))
                    listCOMsets << proc->settings()->value(Param_Poste_PortRefracteur).toString();
                if (proc->settings()->value(Param_Poste_PortTono).toString().contains("COM"))
                    listCOMsets << proc->settings()->value(Param_Poste_PortTono).toString();
                for (int l=0; l< listportsbox.size(); ++l)
                {
                    UpComboBox *combobox = listportsbox.at(l);
                    if (!combobox)
                        continue;
                    QString port ("");
                    QMap<QString,QString> mapports = initmapports;
                    if (combobox == ui->PortFrontoupComboBox)
                        port = proc->settings()->value(Param_Poste_PortFronto).toString();
                    else if (combobox == ui->PortAutorefupComboBox)
                        port = proc->settings()->value(Param_Poste_PortAutoref).toString();
                    else if (combobox == ui->PortRefracteurupComboBox)
                        port = proc->settings()->value(Param_Poste_PortRefracteur).toString();
                    else if (combobox == ui->PortTonometreupComboBox)
                        port = proc->settings()->value(Param_Poste_PortTono).toString();
                    for (auto it = initmapports.begin(); it != initmapports.end(); ++it)
                        if (listCOMsets.contains(it.key()) && it.key() != port)
                            mapports.remove(it.key());
                    QStringList listavailablesitems =  QStringList() << N_NULL;
                    for (auto it = mapports.begin(); it != mapports.end(); ++it)
                        listavailablesitems << it.key();
                    if (combobox != ui->PortRefracteurupComboBox)
                        listavailablesitems << BOX;
                    listavailablesitems << DOSSIER_ECHANGE;
                    //! le micmac qui suit sert à remettre à jour la liste des items du combobox sans modifier le currentText() pour ne pas déclencher le signal currentTextChanged()
                    combobox->clearItems(true);
                    combobox->insertItemsRespectCurrent(listavailablesitems);
                }
            }
        else
        {
            for (int l=0; l< listportsbox.size(); ++l)
            {
                UpComboBox *combobox = listportsbox.at(l);
                if (!combobox)
                    continue;
                QMap<QString,QString> mapports = initmapports;
                for (int i=0; i< listportsbox.size(); ++i)
                {
                    if (listportsbox.at(i) != combobox)
                        if (mapports.find(listportsbox.at(i)->currentText()) != mapports.end())
                            mapports.remove(listportsbox.at(i)->currentText());
                }
                QStringList listavailablesitems =  QStringList() << N_NULL;
                for (auto it = mapports.begin(); it != mapports.end(); ++it)
                    listavailablesitems << it.key();
                if (combobox != ui->PortRefracteurupComboBox)
                    listavailablesitems << BOX;
                listavailablesitems << DOSSIER_ECHANGE;
                //! le micmac qui suit sert à remettre à jour la liste des items du combobox sans modifier le currentText() pour ne pas déclencher le signal currentTextChanged()
                combobox->clearItems(true);
                combobox->insertItemsRespectCurrent(listavailablesitems);
            }
        }
    }
    else
    {
        for (int l=0; l< listportsbox.size(); ++l)
        {
            UpComboBox *combobox = listportsbox.at(l);
            if (!combobox)
                continue;
            QString port ("");
            QStringList listavailablesitems =  QStringList() << N_NULL;
            if (fromSettings)
            {
                if (combobox == ui->PortFrontoupComboBox)
                    port = proc->settings()->value(Param_Poste_PortFronto).toString();
                else if (combobox == ui->PortAutorefupComboBox)
                    port = proc->settings()->value(Param_Poste_PortAutoref).toString();
                else if (combobox == ui->PortRefracteurupComboBox)
                    port = proc->settings()->value(Param_Poste_PortRefracteur).toString();
                else if (combobox == ui->PortTonometreupComboBox)
                    port = proc->settings()->value(Param_Poste_PortTono).toString();
            }
            else
                port = combobox->currentText();
            if (port.left(3) == "COM")
                listavailablesitems << port;
            if (combobox != ui->PortRefracteurupComboBox)
                listavailablesitems << BOX;
            listavailablesitems << DOSSIER_ECHANGE;
            //! le micmac qui suit sert à remettre à jour la liste des items du combobox sans modifier le currentText() pour ne pas déclencher le signal currentTextChanged()
            combobox->clearItems(true);
            combobox->insertItemsRespectCurrent(listavailablesitems);
        }
    }
}

void dlg_param::ModifDirBackup()
{
    if (db->ModeAccesDataBase() == Utils::Distant)
        return;
    QString dirsauvorigin   = ui->DirBackupuplineEdit->text();
    if (dirsauvorigin == "" || !QDir(dirsauvorigin).exists())
        dirsauvorigin = db->dirimagerie();
    QUrl url = Utils::getExistingDirectoryUrl(this, tr("Choisissez le dossier dans lequel vous voulez sauvegarder la base\n"
                                                       "Le nom de dossier ne doit pas contenir d'espace"), QUrl::fromLocalFile(dirsauvorigin));
    if (dirsauvorigin == url.path() || url == QUrl())
        return;
    if (!QDir(url.path()).exists())
        return;

    ui->DirBackupuplineEdit ->setText(url.path());
    db->setdirbkup(url.path());
    proc->ParamAutoBackup();
    ui->EffacePrgSauvupPushButton->setEnabled(m_parametres->daysbkup()
                                           && QDir(m_parametres->dirbkup()).exists()
                                           && m_parametres->dirbkup() != ""
                                           && m_parametres->heurebkup() != QTime());
}

void dlg_param::ModifPathDirEchangeMesure(Procedures::TypeAppareil appareil)
{
    /*! il faut utiliser la fonction static QFileDialog::getExistingDirectoryUrl() parce que la QFileDialog implémentée dans Qt ne donne pas accès aux lecteurs réseaux sous linux
     * avec la fonction static, on utilise la boîte de dialog du système
     * bien sûr, il faut paramétrer le fstab sous linux pour que le dossier réseau soit ouvert automatiquement au moment du boot*/
    QString pathappareil;
    QUrl url;
    QString title = tr("Choisissez le dossier d'enregistrement provisoire des mesures de l'appareil\n"
                                "Le nom de dossier ne doit pas contenir d'espace");
    switch (appareil) {
    case Procedures::Fronto:
        pathappareil = proc->settings()->value(Param_Poste_PortFronto_DossierEchange).toString();
        if (pathappareil == "")
        {
            Utils::mkpath(PATH_DIR_FRONTO);
            pathappareil = PATH_DIR_FRONTO;
        }
        if (!QDir(pathappareil).exists())
            pathappareil = QDir::homePath();
        url = Utils::getExistingDirectoryUrl(this, title, QUrl::fromLocalFile(pathappareil), QStringList()<<m_parametres->dirbkup());
        if (url == QUrl())
            return;
        ui->NetworkPathFrontoupLineEdit ->setText(url.path());
        ui->NetworkPathFrontoupLineEdit ->setImmediateToolTip(url.path());
        proc->settings()->setValue(Param_Poste_PortFronto_DossierEchange, url.path());
        break;
    case Procedures::Autoref:
        pathappareil = proc->settings()->value(Param_Poste_PortAutoref_DossierEchange).toString();
        if (pathappareil == "")
        {
            Utils::mkpath(PATH_DIR_AUTOREF);
            pathappareil = PATH_DIR_AUTOREF;
        }
         if (!QDir(pathappareil).exists())
            pathappareil = QDir::homePath();
        url = Utils::getExistingDirectoryUrl(this, title, QUrl::fromLocalFile(pathappareil), QStringList()<<m_parametres->dirbkup());
        if (url == QUrl())
            return;
        ui->NetworkPathAutorefupLineEdit ->setText(url.path());
        ui->NetworkPathAutorefupLineEdit ->setImmediateToolTip(url.path());
        proc->settings()->setValue(Param_Poste_PortAutoref_DossierEchange, url.path());
        break;
    case Procedures::Refracteur:
        pathappareil = proc->settings()->value(Param_Poste_PortRefracteur_DossierEchange).toString();
        if (pathappareil == "")
        {
            Utils::mkpath(PATH_DIR_REFRACTEUR_OUT);
            pathappareil = PATH_DIR_REFRACTEUR_OUT;
        }
        if (!QDir(pathappareil).exists())
            pathappareil = QDir::homePath();
        url = Utils::getExistingDirectoryUrl(this, title, QUrl::fromLocalFile(pathappareil), QStringList()<<m_parametres->dirbkup());
        if (url == QUrl())
            return;
        ui->NetworkPathRefracteurupLineEdit ->setText(url.path());
        ui->NetworkPathRefracteurupLineEdit ->setImmediateToolTip(url.path());
        proc->settings()->setValue(Param_Poste_PortRefracteur_DossierEchange, url.path());
        break;
    case Procedures::Tonometre:
        pathappareil = proc->settings()->value(Param_Poste_PortTono_DossierEchange).toString();
        if (pathappareil == "")
        {
            Utils::mkpath(PATH_DIR_TONO);
            pathappareil = PATH_DIR_TONO;
        }
        if (!QDir(pathappareil).exists())
            pathappareil = QDir::homePath();
        url = Utils::getExistingDirectoryUrl(this, title, QUrl::fromLocalFile(pathappareil), QStringList()<<m_parametres->dirbkup());
        if (url == QUrl())
            return;
        ui->NetworkPathTonoupLineEdit ->setText(url.path());
        ui->NetworkPathTonoupLineEdit ->setImmediateToolTip(url.path());
        proc->settings()->setValue(Param_Poste_PortTono_DossierEchange, url.path());
        break;
    default: break;
    }
}

void dlg_param::ModifPathEchangeReglageRefracteur(Procedures::TypeAppareil appareil)
{
    /*! il faut utiliser la fonction static QFileDialog::getExistingDirectoryUrl() parce que la QFileDialog implémentée dans Qt ne donne pas accès aux lecteurs réseaux sous linux
     * avec la fonction static, on utilise la boîte de dialog du système
     * bien sûr, il faut paramétrer le fstab sous linux pour que le dossier réseau soit ouvert automatiquement au moment du boot*/
    QUrl url = QUrl();
    QString pathappareil;
    QString title = tr("Choisissez le dossier d'enregistrement provisoire des mesures de l'appareil\n"
                                "Le nom de dossier ne doit pas contenir d'espace");
    switch (appareil) {
    case Procedures::Fronto:
        pathappareil = proc->settings()->value(Param_Poste_PortRefracteur_DossierEchange_Fronto).toString();
        if (pathappareil == "")
        {
            Utils::mkpath(PATH_DIR_REFRACTEUR_FRONTOIN);
            pathappareil = PATH_DIR_REFRACTEUR_FRONTOIN;
        }
         if (!QDir(pathappareil).exists())
            pathappareil = QDir::homePath();
        url = Utils::getExistingDirectoryUrl(this, title, QUrl::fromLocalFile(pathappareil), QStringList()<<m_parametres->dirbkup());
        if (url == QUrl())
            return;
        proc->settings()->setValue(Param_Poste_PortRefracteur_DossierEchange_Fronto, url.path());
        ui->NetworkPathEchangeFrontoupLineEdit ->setText(url.path());
        ui->NetworkPathEchangeFrontoupLineEdit ->setImmediateToolTip(url.path());
        break;
    case Procedures::Autoref:
        pathappareil = proc->settings()->value(Param_Poste_PortRefracteur_DossierEchange_Autoref).toString();
        if (pathappareil == "")
        {
            Utils::mkpath(PATH_DIR_REFRACTEUR_AUTOREFIN);
            pathappareil = PATH_DIR_REFRACTEUR_AUTOREFIN;
        }
         if (!QDir(pathappareil).exists())
            pathappareil = QDir::homePath();
        url = Utils::getExistingDirectoryUrl(this, title, QUrl::fromLocalFile(pathappareil), QStringList()<<m_parametres->dirbkup());
        if (url == QUrl())
            return;
        ui->NetworkPathEchangeAutorefupLineEdit ->setText(url.path());
        ui->NetworkPathEchangeAutorefupLineEdit ->setImmediateToolTip(url.path());
        proc->settings()->setValue(Param_Poste_PortRefracteur_DossierEchange_Autoref, url.path());
        break;
    default: break;
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

void dlg_param::ModifBDDVilles(Villes::TownsFrom from)
{
    db                  ->setvillesfrance(from == Villes::DATABASE);
    Datas::I()->villes  ->initListe(from);
}

void dlg_param::DirLocalStockage()
{
    /*! il faut utiliser la fonction static QFileDialog::getExistingDirectoryUrl() parce que la QFileDialog implémentée dans Qt ne donne pas accès aux lecteurs réseaux sous linux
     * avec la fonction static, on utilise la boîte de dialog du système
     * bien sûr, il faut paramétrer le fstab sous linux pour que le dossier réseau soit ouvert automatiquement au moment du boot*/
    QString dir = proc->settings()->value(Utils::getBaseFromMode(Utils::ReseauLocal) + Dossier_Imagerie).toString();
    if (dir == "" || !QDir(dir).exists())
        dir = PATH_DIR_RUFUS;
    QUrl url = Utils::getExistingDirectoryUrl(this, "", QUrl::fromLocalFile(dir), QStringList()<<m_parametres->dirbkup());
    if (url == QUrl())
        return;
    QString path = (url.host()!=""? "//" + url.host() + "/" : "") + url.path();
    ui->LocalPathStockageupLineEdit->setText(path);
    proc->settings()->setValue(Utils::getBaseFromMode(Utils::ReseauLocal) + Dossier_Imagerie, path);
}

void dlg_param::DirDistantStockage()
{
    /*! il faut utiliser la fonction static QFileDialog::getExistingDirectoryUrl() parce que la QFileDialog implémentée dans Qt ne donne pas accès aux lecteurs réseaux sous linux
     * avec la fonction static, on utilise la boîte de dialog du système
     * bien sûr, il faut paramétrer le fstab sous linux pour que le dossier réseau soit ouvert automatiquement au moment du boot*/
    QString dir = proc->settings()->value(Utils::getBaseFromMode(Utils::Distant) + Dossier_Videos).toString();
    if (dir == "" || !QDir(dir).exists())
        dir = PATH_DIR_IMAGERIE;
    QUrl url = Utils::getExistingDirectoryUrl(this, "", QUrl::fromLocalFile(dir), QStringList()<<m_parametres->dirbkup());
    if (url == QUrl())
        return;
    ui->DistantStockageupLineEdit->setText(url.path());
    proc->settings()->setValue(Utils::getBaseFromMode(Utils::Distant) + Dossier_Imagerie, url.path());
}

void dlg_param::PosteVideoDir()
{
    /*! il faut utiliser la fonction static QFileDialog::getExistingDirectoryUrl() parce que la QFileDialog implémentée dans Qt ne donne pas accès aux lecteurs réseaux sous linux
     * avec la fonction static, on utilise la boîte de dialog du système
     * bien sûr, il faut paramétrer le fstab sous linux pour que le dossier réseau soit ouvert automatiquement au moment du boot*/
    QString dir = proc->settings()->value(Utils::getBaseFromMode(Utils::Poste) + Dossier_Imagerie).toString();
    if (dir == "" || !QDir(dir).exists())
        dir = PATH_DIR_IMAGERIE;
    QUrl url = Utils::getExistingDirectoryUrl(this, "", QUrl::fromLocalFile(dir), QStringList() << dir);
    if (url == QUrl())
        return;
    ui->PosteVideoDirupLineEdit->setText(url.path());
    proc->settings()->setValue(Utils::getBaseFromMode(Utils::Poste) + Dossier_Videos, url.path());
}

void dlg_param::LocalVideoDir()
{
    /*! il faut utiliser la fonction static QFileDialog::getExistingDirectoryUrl() parce que la QFileDialog implémentée dans Qt ne donne pas accès aux lecteurs réseaux sous linux
     * avec la fonction static, on utilise la boîte de dialog du système
     * bien sûr, il faut paramétrer le fstab sous linux pour que le dossier réseau soit ouvert automatiquement au moment du boot*/
    QString dir = proc->settings()->value(Utils::getBaseFromMode(Utils::ReseauLocal) + Dossier_Imagerie).toString();
    if (dir == "" || !QDir(dir).exists())
        dir = PATH_DIR_IMAGERIE;
    QUrl url = Utils::getExistingDirectoryUrl(this, "", QUrl::fromLocalFile(dir), QStringList() << dir);
    if (url == QUrl())
        return;
    ui->LocalVideoDirupLineEdit->setText(url.path());
    proc->settings()->setValue(Utils::getBaseFromMode(Utils::ReseauLocal) + Dossier_Videos, url.path());
}

void dlg_param::DistantVideoDir()
{
    /*! il faut utiliser la fonction static QFileDialog::getExistingDirectoryUrl() parce que la QFileDialog implémentée dans Qt ne donne pas accès aux lecteurs réseaux sous linux
     * avec la fonction static, on utilise la boîte de dialog du système
     * bien sûr, il faut paramétrer le fstab sous linux pour que le dossier réseau soit ouvert automatiquement au moment du boot*/
    QString dir = proc->settings()->value(Utils::getBaseFromMode(Utils::Distant) + Dossier_Imagerie).toString();
    if (dir == "" || !QDir(dir).exists())
        dir = PATH_DIR_IMAGERIE;
    QUrl url = Utils::getExistingDirectoryUrl(this, "", QUrl::fromLocalFile(dir), QStringList() << dir);
    if (url == QUrl())
        return;
    ui->DistantVideoDirupLineEdit->setText(url.path());
    proc->settings()->setValue(Utils::getBaseFromMode(Utils::Distant) + Dossier_Videos, url.path());
}

void dlg_param::DossierClesSSL()
{
    QString dir = proc->settings()->value(Utils::getBaseFromMode(Utils::Distant) + Dossier_ClesSSL).toString();
    if (dir == "" || !QDir(dir).exists())
    {
        ui->DossierClesSSLupLineEdit->clear();
        proc->settings()->remove(Utils::getBaseFromMode(Utils::Distant) + Dossier_ClesSSL);
    }
    QUrl url = Utils::getExistingDirectoryUrl(this, "", QUrl::fromLocalFile(dir), QStringList()<<m_parametres->dirbkup(),false);
    if (url == QUrl())
        return;
    ui->DossierClesSSLupLineEdit->setText(url.path());
    proc->settings()->setValue(Utils::getBaseFromMode(Utils::Distant) + Dossier_ClesSSL, url.path());
}

/*! (SERVEUR) Copie vers une clé USB les SEULES clés CLIENT SSL conservées par ce poste
 *  (ca-cert.pem, client-cert.pem, client-key.pem ; jamais les clés serveur), afin de les
 *  déployer sur les postes en accès distant. cf. journal SSL (étape 7, point 3).
 *  L'extraction est volontairement réservée au serveur (poste maîtrisé). */
void dlg_param::ExporterClesSSLversUSB()
{
    const QString source = QString(PATH_DIR_CLESSSL_SERVEUR);
    const QStringList fichiers = QStringList() << "ca-cert.pem" << "client-cert.pem" << "client-key.pem";

    //! Les clés sont récoltées du datadir à l'installation du serveur MySQL (recolterClesClientSSL).
    QStringList manquants;
    for (const QString &f : fichiers)
        if (!QFile::exists(source + "/" + f))
            manquants << f;
    if (!manquants.isEmpty())
    {
        UpMessageBox::Watch(this, tr("Clés client SSL indisponibles"),
                            tr("Les clés client SSL ne sont pas disponibles sur ce serveur :") + "\n"
                            + manquants.join(", ") + "\n\n"
                            + tr("Elles sont créées lors de l'installation du serveur MySQL par Rufus."));
        return;
    }

    //! Destination : la clé USB choisie par l'utilisateur.
    QUrl url = Utils::getExistingDirectoryUrl(this, tr("Sélectionnez la clé USB de destination"),
                                              QUrl::fromLocalFile(QDir::homePath()), QStringList()<<m_parametres->dirbkup(), false);
    if (url == QUrl())
        return;
    //! Sous-dossier dédié « SSLKeys » dans l'emplacement choisi : les clés ne se perdent pas au
    //! milieu d'autres fichiers, et le poste distant n'a qu'à pointer ce dossier.
    const QString dest = url.path() + "/SSLKeys";
    if (!QDir().mkpath(dest))
    {
        UpMessageBox::Watch(this, tr("Dossier inaccessible"),
                            tr("Impossible de créer le sous-dossier SSLKeys dans l'emplacement choisi."));
        return;
    }

    QStringList echecs;
    for (const QString &f : fichiers)
    {
        const QString cible = dest + "/" + f;
        QFile::remove(cible);                       //! QFile::copy échoue si la cible existe déjà
        if (!QFile::copy(source + "/" + f, cible))
            echecs << f;
    }
    if (!echecs.isEmpty())
    {
        UpMessageBox::Watch(this, tr("Export incomplet"),
                            tr("Certains fichiers n'ont pas pu être copiés :") + "\n" + echecs.join(", "));
        return;
    }

    UpMessageBox::Watch(this, tr("Clés client SSL exportées"),
                        tr("Les clés client SSL ont été copiées sur :") + "\n" + dest + "\n\n"
                        + tr("Déployez-les dans le dossier des clés SSL de chaque poste en accès distant."));
}

/*! (ACCÈS DISTANT) Copie vers une clé USB les clés SSL d'accès au serveur DISTANT — celles que ce
 *  poste utilise pour joindre cette base (dossier renseigné via « … » → Dossier_ClesSSL) — afin de
 *  les déployer sur un AUTRE poste qui doit accéder au même serveur distant.
 *  À distinguer de l'export du Poste serveur (ExporterClesSSLversUSB), qui exporte, lui, les clés du
 *  serveur que CE poste héberge : un même poste peut détenir les DEUX jeux de clés. */
void dlg_param::ExporterClesSSLDistantversUSB()
{
    const QString source = proc->settings()->value(Utils::getBaseFromMode(Utils::Distant) + Dossier_ClesSSL).toString();
    if (source.isEmpty() || !QDir(source).exists())
    {
        UpMessageBox::Watch(this, tr("Dossier des clés SSL non renseigné"),
                            tr("Le dossier des clés SSL d'accès distant n'est pas renseigné sur ce poste.") + "\n\n"
                            + tr("Indiquez-le d'abord avec le bouton de sélection du dossier des clés SSL."));
        return;
    }

    //! Clés CLIENT requises pour l'accès distant : client-key.pem + client-cert.pem (la CA
    //! ca.pem / ca-cert.pem est facultative — on l'inclut si elle est présente).
    const QStringList obligatoires = QStringList() << "client-key.pem" << "client-cert.pem";
    QStringList manquants;
    for (const QString &f : obligatoires)
        if (!QFile::exists(source + "/" + f))
            manquants << f;
    if (!manquants.isEmpty())
    {
        UpMessageBox::Watch(this, tr("Clés client SSL indisponibles"),
                            tr("Le dossier des clés SSL ne contient pas les clés nécessaires :") + "\n"
                            + manquants.join(", "));
        return;
    }

    //! Destination : la clé USB choisie par l'utilisateur.
    QUrl url = Utils::getExistingDirectoryUrl(this, tr("Sélectionnez la clé USB de destination"),
                                              QUrl::fromLocalFile(QDir::homePath()), QStringList()<<m_parametres->dirbkup(), false);
    if (url == QUrl())
        return;
    //! Sous-dossier dédié « SSLKeys » dans l'emplacement choisi (même principe que l'export serveur).
    const QString dest = url.path() + "/SSLKeys";
    if (!QDir().mkpath(dest))
    {
        UpMessageBox::Watch(this, tr("Dossier inaccessible"),
                            tr("Impossible de créer le sous-dossier SSLKeys dans l'emplacement choisi."));
        return;
    }

    //! On copie les clés client + la CA si elle existe (sous l'un ou l'autre nom).
    const QStringList aCopier = QStringList() << "client-key.pem" << "client-cert.pem" << "ca.pem" << "ca-cert.pem";
    QStringList echecs;
    for (const QString &f : aCopier)
    {
        if (!QFile::exists(source + "/" + f))
            continue;                               //! ca.pem / ca-cert.pem facultatifs
        const QString cible = dest + "/" + f;
        QFile::remove(cible);                       //! QFile::copy échoue si la cible existe déjà
        if (!QFile::copy(source + "/" + f, cible))
            echecs << f;
    }
    if (!echecs.isEmpty())
    {
        UpMessageBox::Watch(this, tr("Export incomplet"),
                            tr("Certains fichiers n'ont pas pu être copiés :") + "\n" + echecs.join(", "));
        return;
    }

    UpMessageBox::Watch(this, tr("Clés client SSL exportées"),
                        tr("Les clés SSL d'accès distant ont été copiées sur :") + "\n" + dest + "\n\n"
                        + tr("Déployez-les sur l'autre poste en accès distant, puis indiquez-y leur dossier."));
}

/*! (CE POSTE / serveur) Génère DE NOUVELLES clés SSL pour le serveur. Opération DESTRUCTIVE :
 *  les clés actuellement déployées sur les postes en accès distant cesseront de fonctionner tant
 *  qu'on ne leur aura pas transmis les nouvelles. D'où l'avertissement sévère et la confirmation
 *  par le mot de passe administrateur. Réutilise MySQLInstaller::regenererClesSSL() (arrêt serveur
 *  → suppression des certificats → redémarrage [auto-régénération] → réextraction) puis relance
 *  Rufus (la connexion a été coupée par le redémarrage du serveur). */
void dlg_param::CreerClesSSL()
{
    UpMessageBox msgbox(this);
    msgbox.setText(tr("Créer de nouvelles clés SSL ?"));
    //! Texte en HTML : le QLabel d'UpMessageBox ne rend le riche QUE si la chaîne COMMENCE par une
    //! balise (Qt::mightBeRichText). On structure donc tout en <p> (et le gras/rouge en <span>) ;
    //! CalcSize mesure correctement car convertPlainText() retransforme les blocs HTML en lignes.
    msgbox.setInformativeText(
        "<p>" + tr("Vous allez générer de NOUVELLES clés SSL pour ce serveur.") + "</p>"
        + "<p><span style=\"color:#c00000;\"><b>"
        + tr("ATTENTION : plus aucun poste en accès distant ne pourra se connecter tant que vous ne "
             "lui aurez pas transmis les NOUVELLES clés.")
        + "</b></span></p>"
        + "<p>" + tr("Après la génération, exportez les nouvelles clés (bouton « Exporter les clés "
             "client SSL ») et déployez-les sur chaque poste distant.") + "</p>"
        + "<p>" + tr("Le serveur MySQL sera redémarré et Rufus relancé. Continuer ?") + "</p>");
    msgbox.setIcon(UpMessageBox::Warning);
    UpSmallButton *Annul = new UpSmallButton(); Annul->setText(tr("Annuler"));
    UpSmallButton *OKb   = new UpSmallButton(); OKb  ->setText(tr("Générer de nouvelles clés"));
    msgbox.addButton(Annul, UpSmallButton::CLOSEBUTTON);
    msgbox.addButton(OKb,   UpSmallButton::STARTBUTTON);
    msgbox.exec();
    if (msgbox.clickedButton() != OKb)
        return;

    //! Confirmation par le mot de passe administrateur Rufus (geste irréversible pour l'accès distant).
    QString mdp;
    if (!Utils::VerifMDP(proc->MDPAdmin(), tr("Saisissez le mot de passe Administrateur"), mdp, false, this))
        return;

    //! La régénération REDÉMARRE le serveur MySQL → la connexion Qt est coupée. On ARRÊTE d'abord
    //! les timers de fond (heartbeat poste connecté, etc.) : sinon l'un d'eux tente une écriture SQL
    //! pendant le redémarrage et déclenche un « Enregistrement impossible » orphelin (affiché
    //! derrière dlg_param, donc bloqué). On relance Rufus juste après, donc inutile de les rétablir.
    proc->SuspendreTimersFond();

    if (MySQLInstaller().regenererClesSSL())
    {
        UpMessageBox::Watch(this, tr("Nouvelles clés SSL générées"),
            tr("De nouvelles clés SSL ont été générées.") + "\n"
            + tr("Exportez-les puis déployez-les sur les postes en accès distant.") + "\n\n"
            + tr("Rufus va redémarrer."));
        QProcess::startDetached(QApplication::applicationFilePath(), QApplication::arguments().mid(1));
        exit(0);
    }
    else
        UpMessageBox::Watch(this, tr("Génération impossible"),
            tr("Les clés SSL n'ont pas pu être générées."));
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
    if (ui->LocalServcheckBox->isChecked() && db->ModeAccesDataBase() == Utils::ReseauLocal)
    {
        bool DirStockageAVerifier = false;
        if (ui->LocalDocupTableWidget->rowCount()>0)
        {
            for (int i=0; i<ui->LocalDocupTableWidget->rowCount(); i++)
            {
                UpLineEdit *line = qobject_cast<UpLineEdit*>(ui->LocalDocupTableWidget->cellWidget(i,2));
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
                UpLineEdit *line = qobject_cast<UpLineEdit*>(ui->DistantDocupTableWidget->cellWidget(i,2));
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
    if (proc->RestaureBase(false, false, true, this))
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
        A = "<font color=\"green\"><b>" + A + "</b></font>";
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
        UpLabel *lbl = qobject_cast<UpLabel *>(obj);
        if (lbl != Q_NULLPTR)
        {
            UpLabel* upLabelObj = qobject_cast<UpLabel*>(obj);
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
            QPushButton* Button = qobject_cast<QPushButton*>(obj);
            Button->setIcon(Icons::icFermeAppuye());
        }
    }
    if(event->type() == QEvent::MouseMove)
        if (obj == ui->FermepushButton)
        {
            QPushButton* Button = qobject_cast<QPushButton*>(obj);
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
        Remplir_TableCotations();
        wdg_cotationswdgbuttonframe->wdg_modifBouton->setEnabled(false);
        wdg_cotationswdgbuttonframe->wdg_moinsBouton->setEnabled(false);
    }
    else
        ui->Cotationswidget->setVisible(false);
    ReconstruitListeLieuxExerciceUser(currentuser());
}

void dlg_param::ConnectSignals()
{
    ConfigureChampMDPMySQL(ui->MDPMonouplineEdit,    ui->MDPMonoUSBupSmallButton);
    ConfigureChampMDPMySQL(ui->MDPLocaluplineEdit,   ui->MDPLocalUSBupSmallButton);
    ConfigureChampMDPMySQL(ui->MDPDistantuplineEdit, ui->MDPDistantUSBupSmallButton);

    //! Garde-fou : le champ du mode de connexion COURANT contient le mot de passe qui a réellement
    //! servi à se connecter (il a donc fonctionné). Si l'utilisateur le modifie et quitte le champ,
    //! on l'avertit — une faute de frappe ici couperait ce poste de la base à la prochaine ouverture.
    //! Ne concerne QUE le mode courant : pour les autres modes, un mot de passe erroné n'a pas d'effet
    //! immédiat et l'utilisateur peut légitimement en saisir un qu'il ne peut pas vérifier tout de suite.
    UpLineEdit *champMDPcourant = nullptr;
    switch (db->ModeAccesDataBase())
    {
        case Utils::Poste:       champMDPcourant = ui->MDPMonouplineEdit;    break;
        case Utils::ReseauLocal: champMDPcourant = ui->MDPLocaluplineEdit;   break;
        case Utils::Distant:     champMDPcourant = ui->MDPDistantuplineEdit; break;
        default:                 break;
    }
    if (champMDPcourant != nullptr)
    {
        const QString mdpConnexion = MySQLInstaller::motDePasseStockePourMode(db->ModeAccesDataBase());
        connect(champMDPcourant, &QLineEdit::editingFinished, this, [=]() {
            if (m_alerteMDPencours)                         return;   //! le message vole le focus → éviter une 2e alerte
            if (champMDPcourant->text() == mdpConnexion)    return;   //! mot de passe inchangé : rien à signaler
            m_alerteMDPencours = true;
            //! Deux boutons : Oups (rétablir le mot de passe qui marche) et OK (confirmer le changement).
            //! Le framework impose OK le plus à droite et Oups juste à sa gauche → titresboutonslist[0]
            //! est affecté au bouton le plus à gauche (Oups), [1] au suivant (OK).
            //! Seul un clic explicite sur OK conserve la modification ; tout le reste (Oups, fermeture
            //! de la fenêtre) rétablit le mot de passe de connexion — le choix sûr par défaut.
            if (UpMessageBox::Question(this,
                                       tr("Modifier le mot de passe de connexion ?"),
                                       tr("C'est avec ce mot de passe que ce poste est actuellement connecté à la base : il est donc correct.") + "\n"
                                       + tr("Le modifier risque d'empêcher ce poste de se reconnecter."),
                                       UpDialog::ButtonOK | UpDialog::ButtonOups,
                                       QStringList() << tr("Oups") << tr("oui, je suis sûr\nde vouloir changer le mot de passe"))
                != UpSmallButton::STARTBUTTON)                //! STARTBUTTON = style du bouton OK
                champMDPcourant->setText(mdpConnexion);     //! Oups / fermeture → on restaure le mot de passe qui fonctionne
            m_alerteMDPencours = false;
        });
    }
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

    connect(ui->PosteVideoDirupPushButton,          &QPushButton::clicked,                  this,   &dlg_param::PosteVideoDir);
    connect(ui->LocalVideoDirupPushButton,          &QPushButton::clicked,                  this,   &dlg_param::LocalVideoDir);
    connect(ui->DistantVideoDirupPushButton,        &QPushButton::clicked,                  this,   &dlg_param::DistantVideoDir);

    connect(ui->DossierCLesSSLupPushButton,         &QPushButton::clicked,                  this,   &dlg_param::DossierClesSSL);
    connect(ui->ExportClesSSLPosteupPushButton,     &QPushButton::clicked,                  this,   &dlg_param::ExporterClesSSLversUSB);
    connect(ui->ExportClesSSLDistantupPushButton,   &QPushButton::clicked,                  this,   &dlg_param::ExporterClesSSLDistantversUSB);
    connect(ui->CreerClesSSLPosteupPushButton,      &QPushButton::clicked,                  this,   &dlg_param::CreerClesSSL);
    //! Recréer le mot de passe de la base (si l'ancien aléatoire est perdu). Protégé par le mot de passe
    //! Administrateur ; réservé au local + socle conforme (contrôlé dans recreerMotDePasseApresVerifAdmin).
    connect(ui->RecreerMDPMonoupPushButton,         &QPushButton::clicked,                  this,   [=] {MySQLInstaller().recreerMotDePasseApresVerifAdmin(this);});
    connect(ui->AppareilsConnectesupTableWidget,    &QTableWidget::itemSelectionChanged,    this,   &dlg_param::EnableSupprAppareilBouton);
    connect(ui->AutorefupComboBox,                  QOverload<int>::of(&QComboBox::currentIndexChanged),
                                                                                            this,   [=] (int a) {ClearPortsComboBox(ui->AutorefupComboBox,a);});
    connect(ui->TonometreupComboBox,                QOverload<int>::of(&QComboBox::currentIndexChanged),
                                                                                            this,   [=] (int a) {ClearPortsComboBox(ui->TonometreupComboBox,a);});
    connect(ui->FrontoupComboBox,                   QOverload<int>::of(&QComboBox::currentIndexChanged),
                                                                                            this,   [=] (int a) {ClearPortsComboBox(ui->FrontoupComboBox,a);});
    connect(ui->RefracteurupComboBox,               QOverload<int>::of(&QComboBox::currentIndexChanged),
                                                                                            this,   [=] (int a) {ClearPortsComboBox(ui->RefracteurupComboBox,a);});
    ui->cotationsUpTableView    ->setMouseTracking(true);   //! nécessaire pour le signal entered() de la vue
    connect(ui->cotationsUpTableView,               &QAbstractItemView::entered,            this,   [=] (QModelIndex idx) {AfficheToolTip(idx);});
    //! textChanged (et non textEdited) : la table doit suivre aussi bien la frappe que le choix dans le
    //! QCompleter (qui insère le texte sans émettre textEdited)
    connect(ui->ChercheCotationupLineEdit,          &QLineEdit::textChanged,                this,   &dlg_param::scrollToCotation);
    //! clic droit sur la table des cotations : menu contextuel (modifier / supprimer)
    ui->cotationsUpTableView                        ->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->cotationsUpTableView,               &QWidget::customContextMenuRequested,   this,   &dlg_param::MenuContextuelCotations);
    //! double-clic sur la case « pratiqué » (seule éditable) -> édition en place du montant pratiqué
    ui->cotationsUpTableView                        ->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
    connect(ui->ParamMotifspushButton,              &QPushButton::clicked,                  this,   &dlg_param::ParamMotifs);
    connect(this,                                   &dlg_param::click,                      this,   &dlg_param::EnableModif);

    foreach (QLineEdit *line, ui->PosteParamtab->findChildren<QLineEdit*>())
        connect(line,   &QLineEdit::textEdited,                                             this,   &dlg_param::EnableOKModifPosteButton);
    foreach (QComboBox *box, ui->PosteParamtab->findChildren<QComboBox*>())
        connect(box,    QOverload<int>::of(&QComboBox::currentIndexChanged),                this,   &dlg_param::EnableOKModifPosteButton);
    foreach (QRadioButton *butt, ui->PosteParamtab->findChildren<QRadioButton*>())
        connect(butt,   &QRadioButton::clicked,                                             this,   &dlg_param::EnableOKModifPosteButton);
    foreach (QCheckBox *box, ui->PosteParamtab->findChildren<QCheckBox*>())
        connect(box,    &QCheckBox::clicked,                                                this,   &dlg_param::EnableOKModifPosteButton);
    foreach (QSpinBox *spin, ui->PosteParamtab->findChildren<QSpinBox*>())
        connect(spin,   QOverload<int>::of(&QSpinBox::valueChanged),                        this,   &dlg_param::EnableOKModifPosteButton);
    foreach(QRadioButton *butt, ui->JourSauvegardeframe->findChildren<QRadioButton*>())
        connect(butt,                               &QPushButton::clicked,                  this,   &dlg_param::ModifDateBackup);
    connect(ui->HeureBackuptimeEdit,                &QTimeEdit::timeChanged,                this,   &dlg_param::ModifHeureBackup);
    connect(ui->DirBackuppushButton,                &QPushButton::clicked,                  this,   &dlg_param::ModifDirBackup);
    connect(ui->ImmediatBackupupPushButton,         &QPushButton::clicked,                  this,   &dlg_param::startImmediateBackup);
    connect(ui->RestaurBaseupPushButton,            &QPushButton::clicked,                  this,   &dlg_param::RestaureBase);
    connect(ui->ReinitBaseupPushButton,             &QPushButton::clicked,                  proc,   &Procedures::ReinitBase);
    connect(ui->EffacePrgSauvupPushButton,          &QPushButton::clicked,                  this,   &dlg_param::EffaceProgrammationDataBackup);

    connect(ui->NetworkPathFrontoupPushButton,      &QPushButton::clicked,                  this,   [=] {ModifPathDirEchangeMesure(Procedures::Fronto);});
    connect(ui->NetworkPathAutorefupPushButton,     &QPushButton::clicked,                  this,   [=] {ModifPathDirEchangeMesure(Procedures::Autoref);});
    connect(ui->NetworkPathRefracteurupPushButton,  &QPushButton::clicked,                  this,   [=] {ModifPathDirEchangeMesure(Procedures::Refracteur);});
    connect(ui->NetworkPathTonoupPushButton,        &QPushButton::clicked,                  this,   [=] {ModifPathDirEchangeMesure(Procedures::Tonometre);});

    connect(ui->NetworkPathEchangeFrontoupPushButton,       &QPushButton::clicked,          this,   [=] {ModifPathEchangeReglageRefracteur(Procedures::Fronto);});
    connect(ui->NetworkPathEchangeAutorefupPushButton,      &QPushButton::clicked,          this,   [=] {ModifPathEchangeReglageRefracteur(Procedures::Autoref);});

    connect(ui->ParamCOMFrontoupPushButton,                 &QPushButton::clicked,          this,   [=] {ReglePortCOM_dlg(Procedures::Fronto);});
    connect(ui->ParamCOMAutorefupPushButton,                &QPushButton::clicked,          this,   [=] {ReglePortCOM_dlg(Procedures::Autoref);});
    connect(ui->ParamCOMRefracteurupPushButton,             &QPushButton::clicked,          this,   [=] {ReglePortCOM_dlg(Procedures::Refracteur);});
    connect(ui->ParamCOMTonoupPushButton,                   &QPushButton::clicked,          this,   [=] {ReglePortCOM_dlg(Procedures::Tonometre);});
}

bool dlg_param::CotationsModifiees() const
{
    return m_cotationsmodifiees;
}

bool dlg_param::DataUserModifiees() const
{
    return m_donneesusermodifiees;
}

void dlg_param::enableCotations(bool enable)
{
    bool autormodif = enable
                      && (currentuser()->isAlterneResponsableEtAssistant() || currentuser()->isResponsable() || currentuser()->isAssistant())
                      && !currentuser()->isRemplacant();  // les remplaçants ne peuvent pas modifier les actes
    if (m_modelCotations == Q_NULLPTR)
        return;
    //! modèle/vue : la case à cocher est portée par l'item de la colonne 0 ; on active ou non son
    //! caractère cochable selon les droits de l'utilisateur
    for (int row = 0; row < m_modelCotations->rowCount(); ++row)
    {
        QStandardItem *itacte = m_modelCotations->item(row, 0);
        if (itacte == Q_NULLPTR)
            continue;
        Qt::ItemFlags f = itacte->flags();
        f.setFlag(Qt::ItemIsUserCheckable, autormodif);
        itacte->setFlags(f);
    }
    wdg_cotationswdgbuttonframe->setEnabled(enable);   //! table + boutons cotations suivent le verrou de l'onglet user
}

void dlg_param::EnableComOrNetworkWidgetsAppareilRefraction(UpComboBox *combo, QString txtport)
{
    if (combo == Q_NULLPTR)
        return;
    Procedures::TypesAppareils devicesCOM = proc->devicesCOM();
    Procedures::TypesAppareils devicesLAN = proc->devicesLAN();
    QString currportText = txtport;
    combo->setImmediateToolTip("");
    if (combo == ui->PortFrontoupComboBox)
    {
        if (currportText == DOSSIER_ECHANGE)
        {
            ui->NetworkPathFrontoupLineEdit     ->setVisible(true);
            ui->NetworkPathFrontoupPushButton   ->setVisible(true);
            ui->ParamCOMFrontoupPushButton      ->setVisible(false);
            QString path = proc->settings()->value(Param_Poste_PortFronto_DossierEchange).toString();
            if (path == "" || !QDir(path).exists())
            {
                Utils::mkpath(PATH_DIR_FRONTO);
                path = PATH_DIR_FRONTO;
            }
            ui->NetworkPathFrontoupLineEdit ->setText(path);
            proc->settings()->setValue(Param_Poste_PortFronto_DossierEchange, path);
            ui->NetworkPathFrontoupLineEdit ->setImmediateToolTip(path);
            proc->settings()->remove(Param_Poste_PortFronto_COM_baudrate);
            proc->settings()->remove(Param_Poste_PortFronto_COM_databits);
            proc->settings()->remove(Param_Poste_PortFronto_COM_parity);
            proc->settings()->remove(Param_Poste_PortFronto_COM_stopBits);
            proc->settings()->remove(Param_Poste_PortFronto_COM_flowControl);
            devicesCOM.setFlag(Procedures::Fronto, false);
            devicesLAN.setFlag(Procedures::Fronto, true);
        }
        else if (currportText.left(3) == "COM")
        {
            ui->NetworkPathFrontoupLineEdit     ->setVisible(false);
            ui->NetworkPathFrontoupPushButton   ->setVisible(false);
            ui->ParamCOMFrontoupPushButton      ->setVisible(true);
            combo->setImmediateToolTip(ToolTipPortCOM(Procedures::Fronto));
            proc->settings()->remove(Param_Poste_PortFronto_DossierEchange);
            proc->settings()->setValue(Param_Poste_PortFronto, currportText);
            if (combo->valeuravant().left(3) != "COM")
            {
                QMap<QString, QString> map = proc->DefaultSerialSettings(ui->FrontoupComboBox->currentText());
                proc->RegleSerialSettings(Procedures::Fronto, map);
                combo->setImmediateToolTip(ToolTipPortCOM(Procedures::Fronto));
            }
        }
        else
        {
            ui->NetworkPathFrontoupLineEdit     ->setVisible(false);
            ui->NetworkPathFrontoupPushButton   ->setVisible(false);
            ui->ParamCOMFrontoupPushButton      ->setVisible(false);
            proc->settings()->remove(Param_Poste_PortFronto_DossierEchange);
            proc->settings()->remove(Param_Poste_PortFronto_COM_baudrate);
            proc->settings()->remove(Param_Poste_PortFronto_COM_databits);
            proc->settings()->remove(Param_Poste_PortFronto_COM_parity);
            proc->settings()->remove(Param_Poste_PortFronto_COM_stopBits);
            proc->settings()->remove(Param_Poste_PortFronto_COM_flowControl);
            if (currportText != BOX)
                proc->settings()->remove(Param_Poste_PortFronto);
            else
                proc->settings()->setValue(Param_Poste_PortFronto, BOX);
            devicesCOM.setFlag(Procedures::Fronto, false);
            devicesLAN.setFlag(Procedures::Fronto, false);
            proc->setdevicesCOM(devicesCOM);
            proc->setdevicesLAN(devicesLAN);
        }
    }
    else if (combo == ui->PortAutorefupComboBox)
    {
        if (currportText == DOSSIER_ECHANGE)
        {
            ui->NetworkPathAutorefupLineEdit     ->setVisible(true);
            ui->NetworkPathAutorefupPushButton   ->setVisible(true);
            ui->ParamCOMAutorefupPushButton      ->setVisible(false);
            QString path = proc->settings()->value(Param_Poste_PortAutoref_DossierEchange).toString();
            if (path == "" || !QDir(path).exists())
            {
                Utils::mkpath(PATH_DIR_AUTOREF);
                path = PATH_DIR_AUTOREF;
            }
            ui->NetworkPathAutorefupLineEdit ->setText(path);
            proc->settings()->setValue(Param_Poste_PortAutoref_DossierEchange, path);
            ui->NetworkPathAutorefupLineEdit ->setImmediateToolTip(path);
            proc->settings()->remove(Param_Poste_PortAutoref_COM_baudrate);
            proc->settings()->remove(Param_Poste_PortAutoref_COM_databits);
            proc->settings()->remove(Param_Poste_PortAutoref_COM_parity);
            proc->settings()->remove(Param_Poste_PortAutoref_COM_stopBits);
            proc->settings()->remove(Param_Poste_PortAutoref_COM_flowControl);
            devicesCOM.setFlag(Procedures::Autoref, false);
            devicesLAN.setFlag(Procedures::Autoref, true);
            proc->setdevicesCOM(devicesCOM);
            proc->setdevicesLAN(devicesLAN);
        }
        else if (currportText.left(3) == "COM")
        {
            ui->NetworkPathAutorefupLineEdit     ->setVisible(false);
            ui->NetworkPathAutorefupPushButton   ->setVisible(false);
            ui->ParamCOMAutorefupPushButton      ->setVisible(true);
            combo->setImmediateToolTip(ToolTipPortCOM(Procedures::Autoref));
            proc->settings()->setValue(Param_Poste_PortAutoref, currportText);
            proc->settings()->remove(Param_Poste_PortAutoref_DossierEchange);
            if (combo->valeuravant().left(3) != "COM")
            {
                QMap<QString, QString> map = proc->DefaultSerialSettings(ui->AutorefupComboBox->currentText());
                proc->RegleSerialSettings(Procedures::Autoref, map);
                combo->setImmediateToolTip(ToolTipPortCOM(Procedures::Autoref));
            }
        }
        else
        {
            ui->NetworkPathAutorefupLineEdit     ->setVisible(false);
            ui->NetworkPathAutorefupPushButton   ->setVisible(false);
            ui->ParamCOMAutorefupPushButton      ->setVisible(false);
            proc->settings()->remove(Param_Poste_PortAutoref_DossierEchange);
            proc->settings()->remove(Param_Poste_PortAutoref_COM_baudrate);
            proc->settings()->remove(Param_Poste_PortAutoref_COM_databits);
            proc->settings()->remove(Param_Poste_PortAutoref_COM_parity);
            proc->settings()->remove(Param_Poste_PortAutoref_COM_stopBits);
            proc->settings()->remove(Param_Poste_PortAutoref_COM_flowControl);
            if (currportText != BOX)
                proc->settings()->remove(Param_Poste_PortAutoref);
            else
                proc->settings()->setValue(Param_Poste_PortAutoref, BOX);
            devicesCOM.setFlag(Procedures::Autoref, false);
            devicesLAN.setFlag(Procedures::Autoref, false);
        }
    }
    else if (combo == ui->PortRefracteurupComboBox)
    {
        if (currportText == DOSSIER_ECHANGE)
        {
            ui->NetworkPathRefracteurupLineEdit         ->setVisible(true);
            ui->NetworkPathRefracteurupPushButton       ->setVisible(true);
            ui->NetworkPathEchangeAutorefupLineEdit     ->setVisible(true);
            ui->NetworkPathEchangeAutorefupPushButton   ->setVisible(true);
            ui->NetworkPathEchangeFrontoupLineEdit      ->setVisible(true);
            ui->NetworkPathEchangeFrontoupPushButton    ->setVisible(true);
            ui->ParamCOMRefracteurupPushButton          ->setVisible(false);
            QString path = proc->settings()->value(Param_Poste_PortRefracteur_DossierEchange).toString();
            if (path == "" || !QDir(path).exists())
            {
                Utils::mkpath(PATH_DIR_REFRACTEUR_OUT);
                path = PATH_DIR_REFRACTEUR_OUT;
            }
            ui->NetworkPathRefracteurupLineEdit ->setText(path);
            proc->settings()->setValue(Param_Poste_PortRefracteur_DossierEchange, path);
            path = proc->settings()->value(Param_Poste_PortRefracteur_DossierEchange_Autoref).toString();
            if (path == "" || !QDir(path).exists())
            {
                Utils::mkpath(PATH_DIR_REFRACTEUR_AUTOREFIN);
                path = PATH_DIR_REFRACTEUR_AUTOREFIN;
            }
            ui->NetworkPathEchangeAutorefupLineEdit ->setText(path);
            proc->settings()->setValue(Param_Poste_PortRefracteur_DossierEchange_Autoref, path);
            ui->NetworkPathEchangeAutorefupLineEdit ->setImmediateToolTip(path);
            path = proc->settings()->value(Param_Poste_PortRefracteur_DossierEchange_Fronto).toString();
            if (path == "" || !QDir(path).exists())
            {
                Utils::mkpath(PATH_DIR_REFRACTEUR_FRONTOIN);
                path = PATH_DIR_REFRACTEUR_FRONTOIN;
            }
            ui->NetworkPathEchangeFrontoupLineEdit  ->setText(path);
            proc->settings()->setValue(Param_Poste_PortRefracteur_DossierEchange_Fronto, path);
            ui->NetworkPathEchangeFrontoupLineEdit  ->setImmediateToolTip(path);

            proc->settings()->remove(Param_Poste_PortRefracteur_COM_baudrate);
            proc->settings()->remove(Param_Poste_PortRefracteur_COM_databits);
            proc->settings()->remove(Param_Poste_PortRefracteur_COM_parity);
            proc->settings()->remove(Param_Poste_PortRefracteur_COM_stopBits);
            proc->settings()->remove(Param_Poste_PortRefracteur_COM_flowControl);
            devicesCOM.setFlag(Procedures::Refracteur, false);
            devicesLAN.setFlag(Procedures::Refracteur, true);
            proc->setdevicesCOM(devicesCOM);
            proc->setdevicesLAN(devicesLAN);

        }
        else if (currportText.left(3) == "COM")
        {
            ui->NetworkPathRefracteurupLineEdit         ->setVisible(false);
            ui->NetworkPathRefracteurupPushButton       ->setVisible(false);
            ui->NetworkPathEchangeAutorefupLineEdit     ->setVisible(false);
            ui->NetworkPathEchangeAutorefupPushButton   ->setVisible(false);
            ui->NetworkPathEchangeFrontoupLineEdit      ->setVisible(false);
            ui->NetworkPathEchangeFrontoupPushButton    ->setVisible(false);
            ui->ParamCOMRefracteurupPushButton          ->setVisible(true);
            combo->setImmediateToolTip(ToolTipPortCOM(Procedures::Refracteur));
            proc->settings()->remove(Param_Poste_PortRefracteur_DossierEchange);
            proc->settings()->remove(Param_Poste_PortRefracteur_DossierEchange_Autoref);
            proc->settings()->remove(Param_Poste_PortRefracteur_DossierEchange_Fronto);
            proc->settings()->setValue(Param_Poste_PortRefracteur, currportText);
            if (combo->valeuravant().left(3) != "COM")
            {
                QMap<QString, QString> map = proc->DefaultSerialSettings(ui->RefracteurupComboBox->currentText());
                proc->RegleSerialSettings(Procedures::Refracteur, map);
                combo->setImmediateToolTip(ToolTipPortCOM(Procedures::Refracteur));
            }
        }
        else
        {
            ui->NetworkPathRefracteurupLineEdit         ->setVisible(false);
            ui->NetworkPathRefracteurupPushButton       ->setVisible(false);
            ui->NetworkPathEchangeAutorefupLineEdit     ->setVisible(false);
            ui->NetworkPathEchangeAutorefupPushButton   ->setVisible(false);
            ui->NetworkPathEchangeFrontoupLineEdit      ->setVisible(false);
            ui->NetworkPathEchangeFrontoupPushButton    ->setVisible(false);
            ui->ParamCOMRefracteurupPushButton          ->setVisible(false);
            proc->settings()->remove(Param_Poste_PortRefracteur);
            proc->settings()->remove(Param_Poste_PortRefracteur_DossierEchange);
            proc->settings()->remove(Param_Poste_PortRefracteur_DossierEchange_Autoref);
            proc->settings()->remove(Param_Poste_PortRefracteur_DossierEchange_Fronto);
            proc->settings()->remove(Param_Poste_PortRefracteur_COM_baudrate);
            proc->settings()->remove(Param_Poste_PortRefracteur_COM_databits);
            proc->settings()->remove(Param_Poste_PortRefracteur_COM_parity);
            proc->settings()->remove(Param_Poste_PortRefracteur_COM_stopBits);
            proc->settings()->remove(Param_Poste_PortRefracteur_COM_flowControl);
            devicesCOM.setFlag(Procedures::Refracteur, false);
            devicesLAN.setFlag(Procedures::Refracteur, false);
            proc->setdevicesCOM(devicesCOM);
            proc->setdevicesLAN(devicesLAN);
        }
    }
    else if (combo == ui->PortTonometreupComboBox)
    {
        if (currportText == DOSSIER_ECHANGE)
        {
            ui->NetworkPathTonoupLineEdit     ->setVisible(true);
            ui->NetworkPathTonoupPushButton   ->setVisible(true);
            ui->ParamCOMTonoupPushButton      ->setVisible(false);
            QString path = proc->settings()->value(Param_Poste_PortTono_DossierEchange).toString();
            if (path == "" || !QDir(path).exists())
            {
                Utils::mkpath(PATH_DIR_TONO);
                path = PATH_DIR_TONO;
            }
            ui->NetworkPathTonoupLineEdit ->setText(path);
            proc->settings()->setValue(Param_Poste_PortTono_DossierEchange, path);
            ui->NetworkPathTonoupLineEdit ->setImmediateToolTip(path);
            proc->settings()->remove(Param_Poste_PortTono_COM_baudrate);
            proc->settings()->remove(Param_Poste_PortTono_COM_databits);
            proc->settings()->remove(Param_Poste_PortTono_COM_parity);
            proc->settings()->remove(Param_Poste_PortTono_COM_stopBits);
            proc->settings()->remove(Param_Poste_PortTono_COM_flowControl);
            devicesCOM.setFlag(Procedures::Tonometre, false);
            devicesLAN.setFlag(Procedures::Tonometre, true);
            proc->setdevicesCOM(devicesCOM);
            proc->setdevicesLAN(devicesLAN);
        }
        else if (currportText.left(3) == "COM")
        {
            ui->NetworkPathTonoupLineEdit     ->setVisible(false);
            ui->NetworkPathTonoupPushButton   ->setVisible(false);
            ui->ParamCOMTonoupPushButton      ->setVisible(true);
            combo->setImmediateToolTip(ToolTipPortCOM(Procedures::Tonometre));
            proc->settings()->remove(Param_Poste_PortTono_DossierEchange);
            proc->settings()->setValue(Param_Poste_PortTono, currportText);
            if (combo->valeuravant().left(3) != "COM")
            {
                QMap<QString, QString> map = proc->DefaultSerialSettings(ui->TonometreupComboBox->currentText());
                proc->RegleSerialSettings(Procedures::Tonometre, map);
                combo->setImmediateToolTip(ToolTipPortCOM(Procedures::Tonometre));
            }
        }
        else
        {
            ui->NetworkPathTonoupLineEdit     ->setVisible(false);
            ui->NetworkPathTonoupPushButton   ->setVisible(false);
            ui->ParamCOMTonoupPushButton      ->setVisible(false);
            proc->settings()->remove(Param_Poste_PortTono_DossierEchange);
            proc->settings()->remove(Param_Poste_PortTono_COM_baudrate);
            proc->settings()->remove(Param_Poste_PortTono_COM_databits);
            proc->settings()->remove(Param_Poste_PortTono_COM_parity);
            proc->settings()->remove(Param_Poste_PortTono_COM_stopBits);
            proc->settings()->remove(Param_Poste_PortTono_COM_flowControl);
            if (currportText != BOX)
                proc->settings()->remove(Param_Poste_PortTono);
            else
                proc->settings()->setValue(Param_Poste_PortTono, BOX);
            devicesCOM.setFlag(Procedures::Tonometre, false);
            devicesLAN.setFlag(Procedures::Tonometre, false);
            proc->setdevicesCOM(devicesCOM);
            proc->setdevicesLAN(devicesLAN);
        }
    }
}

void dlg_param::EnableWidgContent(QWidget *widg, bool a)
{
    QList<QWidget*> listwidg = widg->findChildren<QWidget*>();
    for (int i=0; i<listwidg.size(); i++)
        listwidg.at(i)->setEnabled(a);
    if (widg == ui->BackupRestoreframe)
    {
        ui->ModifBaselabel->setVisible(db->ModeAccesDataBase() == Utils::Distant);
        if (db->ModeAccesDataBase() != Utils::Distant)
            ui->EffacePrgSauvupPushButton->setEnabled(m_parametres->daysbkup()
                                                   && m_parametres->dirbkup() != ""
                                                   && m_parametres->heurebkup() != QTime()
                                                   && a);
    }
}

void dlg_param::EnregistreNouvMDPAdmin()
{
    if (dlg_askMDP != Q_NULLPTR)
    {
        // Vérifier la cohérence
        QString anc, nouv, confirm;
        UpMessageBox msgbox(this);
        msgbox.setText(tr("Erreur"));
        msgbox.setIcon(UpMessageBox::Warning);
        UpSmallButton OKBouton("OK");
        msgbox.addButton(&OKBouton, UpSmallButton::STARTBUTTON);
        anc         = dlg_askMDP->findChild<UpLineEdit*>(m_ancienMDP)->text();
        nouv        = dlg_askMDP->findChild<UpLineEdit*>(m_nouveauMDP)->text();
        confirm     = dlg_askMDP->findChild<UpLineEdit*>(m_confirmeMDP)->text();
        //qDebug() << anc << nouv << confirm;

        if (anc == "")
        {
            Utils::playAlarm();
            msgbox.setInformativeText(tr("Ancien mot de passe requis"));
            dlg_askMDP->findChild<UpLineEdit*>(m_ancienMDP)->setFocus();
            msgbox.exec();
            return;
        }
        if (Utils::calcSHA1(anc) != proc->MDPAdmin() && anc != proc->MDPAdmin())
        {
            Utils::playAlarm();
            msgbox.setInformativeText(tr("Le mot de passe que vous voulez modifier n'est pas le bon\n"));
            dlg_askMDP->findChild<UpLineEdit*>(m_ancienMDP)->setFocus();
            msgbox.exec();
            return;
        }
        if (!Utils::RegularExpressionMatches(Utils::rgx_AlphaNumeric_5_12, nouv) || nouv == "")
        {
            Utils::playAlarm();
            msgbox.setInformativeText(tr("Le nouveau mot de passe n'est pas conforme\n(au moins 5 caractères - chiffres ou lettres non accentuées -\n"));
            dlg_askMDP->findChild<UpLineEdit*>(m_nouveauMDP)->setFocus();
            msgbox.exec();
            return;
        }
        if (nouv != confirm)
        {
            Utils::playAlarm();
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

void dlg_param::ReglePortCOM_dlg(Procedures::TypeAppareil appareil)
{
    UpDialog *Com_dlg   = new UpDialog(this);
    Com_dlg             ->setWindowModality(Qt::WindowModal);
    QString title(""), port(""), baudrate(""),databits(""),parity(""),stopbits(""),flowcontrol(""), nomphysiqueduport("");
;
    switch (appareil) {
    case Procedures::Fronto :
        title = ui->FrontoupComboBox    ->currentText();
        port        = ui->PortFrontoupComboBox->currentText();
        baudrate    = Param_Poste_PortFronto_COM_baudrate;
        databits    = Param_Poste_PortFronto_COM_databits;
        parity      = Param_Poste_PortFronto_COM_parity;
        stopbits    = Param_Poste_PortFronto_COM_stopBits;
        flowcontrol = Param_Poste_PortFronto_COM_flowControl;
        break;
    case Procedures::Autoref :
        title = ui->AutorefupComboBox   ->currentText();
        port        = ui->PortAutorefupComboBox->currentText();
        baudrate    = Param_Poste_PortAutoref_COM_baudrate;
        databits    = Param_Poste_PortAutoref_COM_databits;
        parity      = Param_Poste_PortAutoref_COM_parity;
        stopbits    = Param_Poste_PortAutoref_COM_stopBits;
        flowcontrol = Param_Poste_PortAutoref_COM_flowControl;
        break;
    case Procedures::Refracteur :
        title = ui->RefracteurupComboBox->currentText();
        port        = ui->PortRefracteurupComboBox->currentText();
        baudrate    = Param_Poste_PortRefracteur_COM_baudrate;
        databits    = Param_Poste_PortRefracteur_COM_databits;
        parity      = Param_Poste_PortRefracteur_COM_parity;
        stopbits    = Param_Poste_PortRefracteur_COM_stopBits;
        flowcontrol = Param_Poste_PortRefracteur_COM_flowControl;
        break;
    case Procedures::Tonometre :
        title = ui->TonometreupComboBox ->currentText();
        port        = ui->PortTonometreupComboBox->currentText();
        baudrate    = Param_Poste_PortTono_COM_baudrate;
        databits    = Param_Poste_PortTono_COM_databits;
        parity      = Param_Poste_PortTono_COM_parity;
        stopbits    = Param_Poste_PortTono_COM_stopBits;
        flowcontrol = Param_Poste_PortTono_COM_flowControl;
        break;
    default: break;
    }
    QMap<QString, QString> mapports = proc->mapPortsCOM();
    auto it = (mapports.find(port));
    if (it != mapports.end())
        nomphysiqueduport = it.value();

    QVBoxLayout     *gbllayout          = new QVBoxLayout();
    UpLabel         *lbltitle           = new UpLabel(Com_dlg, title);
    UpLabel         *lblport            = new UpLabel(Com_dlg, port);
    UpLabel         *lblnomphysport     = new UpLabel(Com_dlg, nomphysiqueduport);
    UpLabel         *lblbaud            = new UpLabel(Com_dlg, tr("Débit"));
    UpLabel         *lbldatabits        = new UpLabel(Com_dlg, tr("Bits de donnés"));
    UpLabel         *lblparity          = new UpLabel(Com_dlg, tr("Parité"));
    UpLabel         *lblstopbits        = new UpLabel(Com_dlg, tr("Bits d'arrêt"));
    UpLabel         *lblflowcontrol     = new UpLabel(Com_dlg, tr("Contrôle de flux"));
    UpComboBox      *combobaud          = new UpComboBox(Com_dlg);
    UpComboBox      *combodatabits      = new UpComboBox(Com_dlg);
    UpComboBox      *comboparity        = new UpComboBox(Com_dlg);
    UpComboBox      *combostopbits      = new UpComboBox(Com_dlg);
    UpComboBox      *comboflowcontrol   = new UpComboBox(Com_dlg);
    QHBoxLayout     *baudlay            = new QHBoxLayout();
    QHBoxLayout     *databitslay        = new QHBoxLayout();
    QHBoxLayout     *paritylay          = new QHBoxLayout();
    QHBoxLayout     *stopbitslay        = new QHBoxLayout();
    QHBoxLayout     *flowcontrollay     = new QHBoxLayout();

    int width           = 170;
    combobaud           ->setFixedWidth(width);
    combodatabits       ->setFixedWidth(width);
    comboparity         ->setFixedWidth(width);
    combostopbits       ->setFixedWidth(width);
    comboflowcontrol    ->setFixedWidth(width);
    lbltitle            ->setAlignment(Qt::AlignCenter);
    lblport             ->setAlignment(Qt::AlignCenter);
    lblnomphysport      ->setAlignment(Qt::AlignCenter);

    baudlay         ->addWidget(lblbaud);
    baudlay         ->addWidget(lblport);
    baudlay         ->addSpacerItem(new QSpacerItem(20,0,QSizePolicy::Expanding));
    baudlay         ->addWidget(combobaud);
    databitslay     ->addWidget(lbldatabits);
    databitslay     ->addSpacerItem(new QSpacerItem(20,0,QSizePolicy::Expanding));
    databitslay     ->addWidget(combodatabits);
    paritylay       ->addWidget(lblparity);
    paritylay       ->addSpacerItem(new QSpacerItem(20,0,QSizePolicy::Expanding));
    paritylay       ->addWidget(comboparity);
    stopbitslay     ->addWidget(lblstopbits);
    stopbitslay     ->addSpacerItem(new QSpacerItem(20,0,QSizePolicy::Expanding));
    stopbitslay     ->addWidget(combostopbits);
    flowcontrollay  ->addWidget(lblflowcontrol);
    flowcontrollay  ->addSpacerItem(new QSpacerItem(20,0,QSizePolicy::Expanding));
    flowcontrollay  ->addWidget(comboflowcontrol);

    gbllayout       ->addWidget(lbltitle);
    gbllayout       ->addWidget(lblport);
    gbllayout       ->addWidget(lblnomphysport);
    gbllayout       ->addSpacerItem(new QSpacerItem(0,30,QSizePolicy::Expanding));
    gbllayout       ->addLayout(baudlay);
    gbllayout       ->addLayout(databitslay);
    gbllayout       ->addLayout(paritylay);
    gbllayout       ->addLayout(stopbitslay);
    gbllayout       ->addLayout(flowcontrollay);

    Com_dlg->AjouteLayButtons(UpDialog::ButtonOups | UpDialog::ButtonCancel | UpDialog::ButtonOK);
    Com_dlg->setStageCount(1);
    Com_dlg->OupsButton->setImmediateToolTip(tr("Revenir aux réglages\npar défaut pour cet appareil"));

    Com_dlg->dlglayout()->insertLayout(0,gbllayout);
    Com_dlg->dlglayout()->setSizeConstraint(QLayout::SetFixedSize);

    combobaud       ->addItems(Utils::listKeyEnumeratorSP(BAUDRATE));
    combodatabits   ->addItems(Utils::listKeyEnumeratorSP(DATABITS));
    combostopbits   ->addItems(Utils::listKeyEnumeratorSP(STOPBITS));
    comboparity     ->addItems(Utils::listKeyEnumeratorSP(PARITY));
    comboflowcontrol->addItems(Utils::listKeyEnumeratorSP(FLOWCONTROL));

    //!REGLAGE DES VALEURS

    //! BAUD
    QString val = proc->settings()->value(baudrate).toString();
    if (val != QVariant())
        combobaud->setCurrentText(val);
    //! DATABITS
    val = proc->settings()->value(databits).toString();
    if (val != QVariant())
        combodatabits->setCurrentText(val);
    //! PARITY
    val = proc->settings()->value(parity).toString();
    if (val != QVariant())
        comboparity->setCurrentText(val);
    //! STOPBITS
    val = proc->settings()->value(stopbits).toString();
    if (val != QVariant())
        combostopbits->setCurrentText(val);
    //! FLOWCONTROL
    val = proc->settings()->value(flowcontrol).toString();
    if (val != QVariant())
        comboflowcontrol->setCurrentText(val);

    connect(Com_dlg->OKButton,   &QPushButton::clicked,  Com_dlg, [=]
    {
        QMap<QString, QString> map;
        map[PORT]       = port;
        map[BAUDRATE]   = combobaud         ->currentText();
        map[DATABITS]   = combodatabits     ->currentText();
        map[PARITY]     = comboparity       ->currentText();
        map[STOPBITS]   = combostopbits     ->currentText();
        map[FLOWCONTROL]= comboflowcontrol  ->currentText();
        proc->RegleSerialSettings(appareil, map);
        proc->Ouverture_Ports_Series(this);
        proc->Ouverture_Fichiers_Echange(this);
        if (appareil == Procedures::Fronto)
            ui->PortFrontoupComboBox->setImmediateToolTip(ToolTipPortCOM(appareil));
        else if (appareil == Procedures::Tonometre)
            ui->PortTonometreupComboBox->setImmediateToolTip(ToolTipPortCOM(appareil));
        else if (appareil == Procedures::Autoref)
            ui->PortAutorefupComboBox->setImmediateToolTip(ToolTipPortCOM(appareil));
        else if (appareil == Procedures::Refracteur)
            ui->PortRefracteurupComboBox->setImmediateToolTip(ToolTipPortCOM(appareil));
        Com_dlg->accept();
    });
    connect(Com_dlg->OupsButton,   &QPushButton::clicked,  Com_dlg, [=]
    {
        QMap<QString, QString> map = proc->DefaultSerialSettings(title);
        combobaud           ->setCurrentText(map[BAUDRATE]);
        combodatabits       ->setCurrentText(map[DATABITS]);
        comboparity         ->setCurrentText(map[PARITY]);
        combostopbits       ->setCurrentText(map[STOPBITS]);
        comboflowcontrol    ->setCurrentText(map[FLOWCONTROL]);
    });
    Com_dlg->exec();
    delete Com_dlg;
}

QString dlg_param::ToolTipPortCOM(Procedures::TypeAppareil appareil)
{
    QString tooltip(""), baudrate(""),databits(""),parity(""),stopbits(""),flowcontrol(""), port (""), nomphysiqueduport("");
    switch (appareil) {
    case Procedures::Fronto :
        port        = proc->settings()->value(Param_Poste_PortFronto).toString();
        baudrate    = Param_Poste_PortFronto_COM_baudrate;
        databits    = Param_Poste_PortFronto_COM_databits;
        parity      = Param_Poste_PortFronto_COM_parity;
        stopbits    = Param_Poste_PortFronto_COM_stopBits;
        flowcontrol = Param_Poste_PortFronto_COM_flowControl;
        break;
    case Procedures::Autoref :
        port        = proc->settings()->value(Param_Poste_PortAutoref).toString();
        baudrate    = Param_Poste_PortAutoref_COM_baudrate;
        databits    = Param_Poste_PortAutoref_COM_databits;
        parity      = Param_Poste_PortAutoref_COM_parity;
        stopbits    = Param_Poste_PortAutoref_COM_stopBits;
        flowcontrol = Param_Poste_PortAutoref_COM_flowControl;
        break;
    case Procedures::Refracteur :
        port        = proc->settings()->value(Param_Poste_PortRefracteur).toString();;
        baudrate    = Param_Poste_PortRefracteur_COM_baudrate;
        databits    = Param_Poste_PortRefracteur_COM_databits;
        parity      = Param_Poste_PortRefracteur_COM_parity;
        stopbits    = Param_Poste_PortRefracteur_COM_stopBits;
        flowcontrol = Param_Poste_PortRefracteur_COM_flowControl;
        break;
    case Procedures::Tonometre :
        port        = proc->settings()->value(Param_Poste_PortTono).toString();;
        baudrate    = Param_Poste_PortTono_COM_baudrate;
        databits    = Param_Poste_PortTono_COM_databits;
        parity      = Param_Poste_PortTono_COM_parity;
        stopbits    = Param_Poste_PortTono_COM_stopBits;
        flowcontrol = Param_Poste_PortTono_COM_flowControl;
        break;
    default: break;
    }

    QMap<QString, QString> mapports = proc->mapPortsCOM();
    auto it = (mapports.find(port));
    if (it != mapports.end())
        nomphysiqueduport = port + " - " + it.value();

    //! BAUD
    QString keystring = proc->settings()->value(baudrate).toString();
    QMetaEnum metaenum  = Utils::enumeratorSP(BAUDRATE);
    int value = metaenum.keyToValue(keystring.toLocal8Bit().data());
    if (value == -1)
        if (proc->setSerialPortSettingsValueFromIndex(BAUDRATE, keystring, appareil))
            value = 0;
    baudrate = ((keystring != QString() && value != -1)?    keystring : tr("Invalide"));

    //! DATABITS
    keystring = proc->settings()->value(databits).toString();
    metaenum  = Utils::enumeratorSP(DATABITS);
    value = metaenum.keyToValue(keystring.toLocal8Bit().data());
    if (value == -1)
        if (proc->setSerialPortSettingsValueFromIndex(DATABITS, keystring, appareil))
            value = 0;
    databits = ((keystring != QString() && value != -1)?    keystring : tr("Invalide"));

    //! PARITY
    keystring = proc->settings()->value(parity).toString();
    metaenum  = Utils::enumeratorSP(PARITY);
    value = metaenum.keyToValue(keystring.toLocal8Bit().data());
    if (value == -1)
        if (proc->setSerialPortSettingsValueFromIndex(PARITY, keystring, appareil))
            value = 0;
    parity = ((keystring != QString() && value != -1)?    keystring : tr("Invalide"));

    //! STOPBITS
    keystring = proc->settings()->value(stopbits).toString();
    metaenum  = Utils::enumeratorSP(STOPBITS);
    value = metaenum.keyToValue(keystring.toLocal8Bit().data());
    if (value == -1)
        if (proc->setSerialPortSettingsValueFromIndex(STOPBITS, keystring, appareil))
            value = 0;
    stopbits = ((keystring != QString() && value != -1)?    keystring : tr("Invalide"));

    //! FLOWCONTROL
    keystring = proc->settings()->value(flowcontrol).toString();
    metaenum  = Utils::enumeratorSP(FLOWCONTROL);
    value = metaenum.keyToValue(keystring.toLocal8Bit().data());
    if (value == -1)
        if (proc->setSerialPortSettingsValueFromIndex(FLOWCONTROL, keystring, appareil))
            value = 0;
    flowcontrol = ((keystring != QString() && value != -1)?    keystring : tr("Invalide"));

    if (baudrate != "" && databits != "" && parity != "" && stopbits != "" && flowcontrol != "")
    {
        if (nomphysiqueduport != "")
            nomphysiqueduport += "\n";

        baudrate    = "Baudrate - " + baudrate;
        databits    = "DataBits - " + databits;
        parity      = "Parity - " + parity;
        stopbits    = "StopBits - " + stopbits;
        flowcontrol = "FlowControl - " + flowcontrol;
        tooltip = tr("Réglé sur") + "\n" + nomphysiqueduport + baudrate + "\n" + databits + "\n" + parity + "\n" + stopbits + "\n" + flowcontrol;
    }
    return tooltip;
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
              " where list." CP_ID_APPAREIL " = appcon." CP_ID_APP " and " CP_IDLIEU_APP " = " + QString::number(Datas::I()->sites->idcurrentsite()) +
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
        connect(line5a,                    &UpLineEdit::textChanged,         this,   &dlg_param::EnableOKModifPosteButton);
        connect(line5b,                    &UpLineEdit::textChanged,         this,   &dlg_param::EnableOKModifPosteButton);
        connect(line5c,                    &UpLineEdit::textChanged,         this,   &dlg_param::EnableOKModifPosteButton);
        connect(line5a,                    &UpLineEdit::TextModified,       this,   [=] (QString txt) {EnregDossierStockageApp(line5a, txt);}); // ne pas supprimer - permet de supprimer manuellement un dossier d'échange pour le remplacer par une valeur nulle p.e.
        connect(line5b,                    &UpLineEdit::TextModified,       this,   [=] (QString txt) {EnregDossierStockageApp(line5b, txt);}); // ne pas supprimer - permet de supprimer manuellement un dossier d'échange pour le remplacer par une valeur nulle p.e.
        connect(line5c,                    &UpLineEdit::TextModified,       this,   [=] (QString txt) {EnregDossierStockageApp(line5c, txt);}); // ne pas supprimer - permet de supprimer manuellement un dossier d'échange pour le remplacer par une valeur nulle p.e.
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
        connect(dossbouton,     &QPushButton::clicked,  this,   [=] {ChoixDossierEchangeAppareilImagerie(dossbouton);});
        connect(dossbouton1,    &QPushButton::clicked,  this,   [=] {ChoixDossierEchangeAppareilImagerie(dossbouton1);});
        connect(dossbouton2,    &QPushButton::clicked,  this,   [=] {ChoixDossierEchangeAppareilImagerie(dossbouton2);});

        pItem6->setText(Applist.at(i).at(3).toString());                             // Format
        ui->AppareilsConnectesupTableWidget->setItem(i,col,pItem6);

        ui->AppareilsConnectesupTableWidget->setRowHeight(i,int(fm.height()*1.3));
        ui->MonoDocupTableWidget->setRowHeight(i,int(fm.height()*1.3));
        ui->LocalDocupTableWidget->setRowHeight(i,int(fm.height()*1.3));
        ui->DistantDocupTableWidget->setRowHeight(i,int(fm.height()*1.3));
    }

    m_listeappareils.clear();
    req = "select " CP_NOMAPPAREIL_APPAREIL " from " TBL_LISTEAPPAREILS
          " where " CP_ID_APPAREIL " not in (select " CP_ID_APP " from " TBL_APPAREILSCONNECTESCENTRE " where " CP_IDLIEU_APP " = " + QString::number(Datas::I()->sites->idcurrentsite()) + ")";
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
            if (UpMessageBox::Question(this,
                                       tr("Vous n'avez pas spécifié de port de communication pour l'autorefractomètre ") + ui->AutorefupComboBox->currentText() + " !",
                                       tr("Voulez-vous le garder quand même?"),
                                       UpDialog::ButtonCancel | UpDialog::ButtonSuppr,
                                       QStringList() << tr("Confirmer") << tr("Corriger"))
               == UpSmallButton::SUPPRBUTTON)
            {
                ui->ParamtabWidget->setCurrentWidget(ui->PosteParamtab);
                ui->PortAutorefupComboBox->setFocus();
                return false;
            }
        }
        if (ui->AutorefupComboBox->currentIndex()==0 && ui->PortAutorefupComboBox->currentIndex() > 0)
        {
            UpMessageBox::Watch(this,tr("Vous avez spécifié un port COM pour l'autorefractomètre sans sélectionner de machine !"));
            ui->ParamtabWidget->setCurrentWidget(ui->PosteParamtab);
            ui->AutorefupComboBox->setFocus();
            return false;
        }
        if (ui->AutorefupComboBox->currentIndex()>0 && ui->PortAutorefupComboBox->currentText() == DOSSIER_ECHANGE && !QDir(ui->NetworkPathAutorefupLineEdit->text()).exists())
        {
            UpMessageBox::Watch(this,tr("Vous n'avez pas spécifié de dossier réseau valide pour l'autorefractomètre") + " !");
            ui->ParamtabWidget->setCurrentWidget(ui->PosteParamtab);
            ui->PortAutorefupComboBox->setFocus();
            return false;
        }

        if (ui->FrontoupComboBox->currentIndex()>0 && ui->PortFrontoupComboBox->currentIndex() == 0)
        {
            if (UpMessageBox::Question(this,
                                       tr("Vous n'avez pas spécifié de port de communication pour le frontofocomètre ") + ui->FrontoupComboBox->currentText() + " !",
                                       tr("Voulez-vous le garder quand même?"),
                                       UpDialog::ButtonCancel | UpDialog::ButtonSuppr,
                                       QStringList() << tr("Confirmer") << tr("Corriger"))
               == UpSmallButton::SUPPRBUTTON)
            {
                ui->ParamtabWidget->setCurrentWidget(ui->PosteParamtab);
                ui->PortFrontoupComboBox->setFocus();
                return false;
            }
        }
        if (ui->FrontoupComboBox->currentIndex()==0 && ui->PortFrontoupComboBox->currentIndex() > 0)
        {
            UpMessageBox::Watch(this,tr("Vous avez spécifié un port COM pour le frontofocotomètre sans sélectionner de machine !"));
            ui->ParamtabWidget->setCurrentWidget(ui->PosteParamtab);
            ui->FrontoupComboBox->setFocus();
            return false;
        }
        if (ui->FrontoupComboBox->currentIndex()>0 && ui->PortFrontoupComboBox->currentText() == DOSSIER_ECHANGE && !QDir(ui->NetworkPathFrontoupLineEdit->text()).exists())
        {
            UpMessageBox::Watch(this,tr("Vous n'avez pas spécifié de dossier réseau valide pour le frontofocomètre") + " !");
            ui->ParamtabWidget->setCurrentWidget(ui->PosteParamtab);
            ui->PortFrontoupComboBox->setFocus();
            return false;
        }

        if (ui->RefracteurupComboBox->currentIndex()>0 && ui->PortRefracteurupComboBox->currentIndex() == 0)
        {
            if (UpMessageBox::Question(this,
                                       tr("Vous n'avez pas spécifié de port de communication pour le refracteur ") + ui->RefracteurupComboBox->currentText() + " !",
                                       tr("Voulez-vous le garder quand même?"),
                                       UpDialog::ButtonCancel | UpDialog::ButtonSuppr,
                                       QStringList() << tr("Confirmer") << tr("Corriger"))
               == UpSmallButton::SUPPRBUTTON)
            {
                ui->ParamtabWidget->setCurrentWidget(ui->PosteParamtab);
                ui->PortRefracteurupComboBox->setFocus();
                return false;
            }
        }
        if (ui->RefracteurupComboBox->currentIndex()==0 && ui->PortRefracteurupComboBox->currentIndex() < 0)
        {
            UpMessageBox::Watch(this,tr("Vous avez spécifié un port COM pour le réfracteur sans sélectionner de machine !"));
            ui->ParamtabWidget->setCurrentWidget(ui->PosteParamtab);
            ui->RefracteurupComboBox->setFocus();
            return false;
        }
        if (ui->RefracteurupComboBox->currentIndex()>0 && ui->PortRefracteurupComboBox->currentText() == DOSSIER_ECHANGE && !QDir(ui->NetworkPathRefracteurupLineEdit->text()).exists())
        {
            UpMessageBox::Watch(this,tr("Vous n'avez pas spécifié de dossier réseau valide pour le refracteur") + " !");
            ui->ParamtabWidget->setCurrentWidget(ui->PosteParamtab);
            ui->PortRefracteurupComboBox->setFocus();
            return false;
        }

        if (ui->TonometreupComboBox->currentIndex()>0 && ui->PortTonometreupComboBox->currentIndex() == 0)
        {
            if (UpMessageBox::Question(this,
                                       tr("Vous n'avez pas spécifié de port de communication pour le tonomètre ") + ui->TonometreupComboBox->currentText() + " !",
                                       tr("Voulez-vous le garder quand même?"),
                                       UpDialog::ButtonCancel | UpDialog::ButtonSuppr,
                                       QStringList() << tr("Confirmer") << tr("Corriger"))
                    == UpSmallButton::SUPPRBUTTON)
            {
                ui->ParamtabWidget->setCurrentWidget(ui->PosteParamtab);
                ui->PortTonometreupComboBox->setFocus();
                return false;
            }
        }
        if (ui->TonometreupComboBox->currentIndex()==0 && ui->PortTonometreupComboBox->currentIndex() > 0)
        {
            UpMessageBox::Watch(this,tr("Vous avez spécifié un port COM pour le tonomètre sans sélectionner de machine !"));
            ui->ParamtabWidget->setCurrentWidget(ui->PosteParamtab);
            ui->TonometreupComboBox->setFocus();
            return false;
        }
        if (ui->TonometreupComboBox->currentIndex()>0 && ui->PortTonometreupComboBox->currentText() == DOSSIER_ECHANGE && !QDir(ui->NetworkPathTonoupLineEdit->text()).exists())
        {
            UpMessageBox::Watch(this,tr("Vous n'avez pas spécifié de dossier réseau valide pour le tonomètre") + " !");
            ui->ParamtabWidget->setCurrentWidget(ui->PosteParamtab);
            ui->PortTonometreupComboBox->setFocus();
            return false;
        }

        //! Un mot de passe MySQL manquant NE bloque PLUS la sortie. Il peut légitimement manquer :
        //! quand le poste s'est connecté avec le mot de passe générique, il ne connaît pas encore le
        //! mot de passe aléatoire de la base sécurisée, donc le champ est vide. Bloquer empêchait
        //! d'enregistrer TOUTES les autres modifications (appareils, impression…). On se contente
        //! d'informer.
        //! Les champs MDP sont pré-remplis à l'ouverture depuis motDePasseStockePourMode(mode) : un
        //! champ vide signifie donc « pas de mot de passe pour ce mode ». On retire alors la ligne
        //! correspondante du .dbkey (supprimerMotDePassePourMode) — sans effacer le fichier ni les
        //! autres modes — au lieu de conserver une valeur que l'utilisateur a délibérément effacée.
        QStringList modesSansMDP;
        if (ui->PosteServcheckBox->isChecked()   && ui->MDPMonouplineEdit->text().isEmpty())    modesSansMDP << tr("Monoposte");
        if (ui->LocalServcheckBox->isChecked()   && ui->MDPLocaluplineEdit->text().isEmpty())   modesSansMDP << tr("Réseau local");
        if (ui->DistantServcheckBox->isChecked() && ui->MDPDistantuplineEdit->text().isEmpty()) modesSansMDP << tr("Accès distant");
        if (!modesSansMDP.isEmpty())
            UpMessageBox::Watch(this, tr("Mot de passe MySQL non renseigné"),
                                tr("Aucun mot de passe MySQL n'a été indiqué pour : %1.").arg(modesSansMDP.join(", ")) + "\n"
                                + tr("Vos autres modifications sont enregistrées.") + "\n"
                                + tr("Vous pourrez renseigner le mot de passe plus tard depuis un poste disposant du mot de passe de la base sécurisée."));

        //! Mot de passe MySQL. Décocher un mode = on ne l'utilise plus (ACTIVE=NO), mais on CONSERVE
        //! son mot de passe dans .dbkey comme les autres réglages du mode (serveur, port) : le poste
        //! pourra recocher le mode plus tard sans avoir à ressaisir le mot de passe.
        //! On ne retire la ligne du .dbkey que sur un mode actif dont le champ a été vidé volontairement.
        QString Base = Utils::getBaseFromMode(Utils::Poste);
        if (ui->PosteServcheckBox->isChecked())
        {
            proc->settings()->setValue(Base + Param_Active,"YES");
            if (!ui->MDPMonouplineEdit->text().isEmpty())
                MySQLInstaller::stockerMotDePassePourMode(Utils::Poste, ui->MDPMonouplineEdit->text());
            else                                            //! champ vidé sur un mode actif → on retire sa ligne du .dbkey
                MySQLInstaller::supprimerMotDePassePourMode(Utils::Poste);
        }
        else
            proc->settings()->setValue(Base + Param_Active,"NO");   //! mode abandonné : on garde le mot de passe stocké
        proc->settings()->setValue(Base + Param_Port,ui->SQLPortPostecomboBox->currentText());

        Base = Utils::getBaseFromMode(Utils::ReseauLocal);
        if (ui->LocalServcheckBox->isChecked())
        {
            proc->settings()->setValue(Base + Param_Active,"YES");
            if (!ui->MDPLocaluplineEdit->text().isEmpty())
                MySQLInstaller::stockerMotDePassePourMode(Utils::ReseauLocal, ui->MDPLocaluplineEdit->text());
            else                                             //! champ vidé sur un mode actif → on retire sa ligne du .dbkey
                MySQLInstaller::supprimerMotDePassePourMode(Utils::ReseauLocal);
        }
        else
            proc->settings()->setValue(Base + Param_Active,"NO");   //! mode abandonné : on garde le mot de passe stocké
        proc->settings()->setValue(Base + Param_Serveur,Utils::calcIP(ui->EmplacementLocaluplineEdit->text(), false));
        db->setadresseserveurlocal(ui->EmplacementLocaluplineEdit->text());
        proc->settings()->setValue(Base + Param_Port,ui->SQLPortLocalcomboBox->currentText());

        Base = Utils::getBaseFromMode(Utils::Distant);
        if (ui->DistantServcheckBox->isChecked())
        {
            proc->settings()->setValue(Base + Param_Active,"YES");
            if (!ui->MDPDistantuplineEdit->text().isEmpty())
                MySQLInstaller::stockerMotDePassePourMode(Utils::Distant, ui->MDPDistantuplineEdit->text());
            else                                               //! champ vidé sur un mode actif → on retire sa ligne du .dbkey
                MySQLInstaller::supprimerMotDePassePourMode(Utils::Distant);
        }
        else
            proc->settings()->setValue(Base + Param_Active,"NO");   //! mode abandonné : on garde le mot de passe stocké
        //if (Utils::rgx_IPV4.exactMatch(ui->EmplacementDistantuplineEdit->text()))
        if (!Utils::RegularExpressionMatches(Utils::rgx_IPV4, ui->EmplacementDistantuplineEdit->text()))
            proc->settings()->setValue(Base + Param_Serveur, Utils::calcIP(ui->EmplacementDistantuplineEdit->text(), false));
        else
            proc->settings()->setValue(Base + Param_Serveur, ui->EmplacementDistantuplineEdit->text());
        db->setadresseserveurdistant(ui->EmplacementDistantuplineEdit->text());

        proc->settings()->setValue(Base + Param_Port,                       ui->SQLPortDistantcomboBox->currentText());
        proc->settings()->setValue(Imprimante_TailleEnTete,                 ui->EntetespinBox->value());
        proc->settings()->setValue(Imprimante_TailleEnTeteALD,              ui->EnteteALDspinBox->value());
        proc->settings()->setValue(Imprimante_TaillePieddePage,             ui->PiedDePagespinBox->value());
        proc->settings()->setValue(Imprimante_TailleTopMarge,               ui->TopMargespinBox->value());
        proc->settings()->setValue(Imprimante_ApercuAvantImpression,        (ui->ApercuImpressioncheckBox->isChecked()? "YES" : "NO"));
        proc->settings()->setValue(Imprimante_OrdoAvecDupli,                (ui->OrdoAvecDuplicheckBox->isChecked()? "YES" : "NO"));

        if (ui->PrioritaireImportDocscheckBox->isChecked())
            proc->settings()->setValue(Utils::getBaseFromMode(Utils::ReseauLocal) + PrioritaireGestionDocs,YESimport);
        else if (ui->NonImportDocscheckBox->isChecked())
            proc->settings()->setValue(Utils::getBaseFromMode(Utils::ReseauLocal) + PrioritaireGestionDocs,NOimport);
        else
            proc->settings()->setValue(Utils::getBaseFromMode(Utils::ReseauLocal) + PrioritaireGestionDocs,NORMimport);

        if (ui->FrontoupComboBox->currentText() == N_NULL)
        {
            proc->settings()->remove(Param_Poste_Fronto);
            ui->PortFrontoupComboBox->setCurrentIndex(0);
        }
        else
             proc->settings()->setValue(Param_Poste_Fronto,ui->FrontoupComboBox->currentText());
        if (ui->PortFrontoupComboBox->currentText() == N_NULL)
        {
            proc->settings()->remove(Param_Poste_PortFronto);
            proc->settings()->remove(Param_Poste_PortFronto_DossierEchange);
            proc->settings()->remove(Param_Poste_PortFronto_COM_baudrate);
            proc->settings()->remove(Param_Poste_PortFronto_COM_databits);
            proc->settings()->remove(Param_Poste_PortFronto_COM_parity);
            proc->settings()->remove(Param_Poste_PortFronto_COM_stopBits);
            proc->settings()->remove(Param_Poste_PortFronto_COM_flowControl);
        }
        else
        {
            proc->settings()->setValue(Param_Poste_PortFronto,ui->PortFrontoupComboBox->currentText());
            if (ui->PortFrontoupComboBox->currentText() == DOSSIER_ECHANGE)
            {
                proc->settings()->setValue(Param_Poste_PortFronto_DossierEchange, ui->NetworkPathFrontoupLineEdit->text());
                proc->settings()->remove(Param_Poste_PortFronto_COM_baudrate);
                proc->settings()->remove(Param_Poste_PortFronto_COM_databits);
                proc->settings()->remove(Param_Poste_PortFronto_COM_parity);
                proc->settings()->remove(Param_Poste_PortFronto_COM_stopBits);
                proc->settings()->remove(Param_Poste_PortFronto_COM_flowControl);
            }
            else
                proc->settings()->remove(Param_Poste_PortFronto_DossierEchange);
        }

        if (ui->AutorefupComboBox->currentText() == N_NULL)
        {
            proc->settings()->remove(Param_Poste_Autoref);
            ui->PortAutorefupComboBox->setCurrentIndex(0);
        }
        else
            proc->settings()->setValue(Param_Poste_Autoref,ui->AutorefupComboBox->currentText());
        if (ui->PortAutorefupComboBox->currentText() == N_NULL)
        {
            proc->settings()->remove(Param_Poste_PortAutoref);
            proc->settings()->remove(Param_Poste_PortAutoref_DossierEchange);
            proc->settings()->remove(Param_Poste_PortAutoref_COM_baudrate);
            proc->settings()->remove(Param_Poste_PortAutoref_COM_databits);
            proc->settings()->remove(Param_Poste_PortAutoref_COM_parity);
            proc->settings()->remove(Param_Poste_PortAutoref_COM_stopBits);
            proc->settings()->remove(Param_Poste_PortAutoref_COM_flowControl);
        }
        else
        {
            proc->settings()->setValue(Param_Poste_PortAutoref,ui->PortAutorefupComboBox->currentText());
            if (ui->PortAutorefupComboBox->currentText() == DOSSIER_ECHANGE)
            {
                proc->settings()->setValue(Param_Poste_PortAutoref_DossierEchange, ui->NetworkPathAutorefupLineEdit->text());
                proc->settings()->remove(Param_Poste_PortAutoref_COM_baudrate);
                proc->settings()->remove(Param_Poste_PortAutoref_COM_databits);
                proc->settings()->remove(Param_Poste_PortAutoref_COM_parity);
                proc->settings()->remove(Param_Poste_PortAutoref_COM_stopBits);
                proc->settings()->remove(Param_Poste_PortAutoref_COM_flowControl);
            }
            else
                proc->settings()->remove(Param_Poste_PortAutoref_DossierEchange);
        }

        if (ui->RefracteurupComboBox->currentText() == N_NULL)
        {
            proc->settings()->remove(Param_Poste_Refracteur);
            ui->PortRefracteurupComboBox->setCurrentIndex(0);
        }
        else
            proc->settings()->setValue(Param_Poste_Refracteur,ui->RefracteurupComboBox->currentText());
        if (ui->PortRefracteurupComboBox->currentText() == N_NULL)
        {
            proc->settings()->remove(Param_Poste_PortRefracteur);
            proc->settings()->remove(Param_Poste_PortRefracteur_DossierEchange);
            proc->settings()->remove(Param_Poste_PortRefracteur_DossierEchange_Autoref);
            proc->settings()->remove(Param_Poste_PortRefracteur_DossierEchange_Fronto);
            proc->settings()->remove(Param_Poste_PortRefracteur_COM_baudrate);
            proc->settings()->remove(Param_Poste_PortRefracteur_COM_databits);
            proc->settings()->remove(Param_Poste_PortRefracteur_COM_parity);
            proc->settings()->remove(Param_Poste_PortRefracteur_COM_stopBits);
            proc->settings()->remove(Param_Poste_PortRefracteur_COM_flowControl);
        }
        else
        {
            proc->settings()->setValue(Param_Poste_PortRefracteur,ui->PortRefracteurupComboBox->currentText());
            if (ui->PortRefracteurupComboBox->currentText() == DOSSIER_ECHANGE)
            {
                proc->settings()->setValue(Param_Poste_PortRefracteur_DossierEchange, ui->NetworkPathRefracteurupLineEdit->text());
                proc->settings()->setValue(Param_Poste_PortRefracteur_DossierEchange_Fronto, ui->NetworkPathEchangeFrontoupLineEdit->text());
                proc->settings()->setValue(Param_Poste_PortRefracteur_DossierEchange_Autoref, ui->NetworkPathEchangeAutorefupLineEdit->text());
                proc->settings()->remove(Param_Poste_PortRefracteur_COM_baudrate);
                proc->settings()->remove(Param_Poste_PortRefracteur_COM_databits);
                proc->settings()->remove(Param_Poste_PortRefracteur_COM_parity);
                proc->settings()->remove(Param_Poste_PortRefracteur_COM_stopBits);
                proc->settings()->remove(Param_Poste_PortRefracteur_COM_flowControl);
            }
            else
            {
                proc->settings()->remove(Param_Poste_PortRefracteur_DossierEchange);
                proc->settings()->remove(Param_Poste_PortRefracteur_DossierEchange_Fronto);
                proc->settings()->remove(Param_Poste_PortRefracteur_DossierEchange_Autoref);
            }
        }

        if (ui->TonometreupComboBox->currentText() == N_NULL)
        {
            proc->settings()->remove(Param_Poste_PortTono);
            proc->settings()->remove(Param_Poste_PortTono_DossierEchange);
            ui->PortTonometreupComboBox->setCurrentIndex(0);
        }
        else
            proc->settings()->setValue(Param_Poste_Tono,ui->TonometreupComboBox->currentText());
        if (ui->PortTonometreupComboBox->currentText() == N_NULL)
        {
            proc->settings()->remove(Param_Poste_PortTono);
            proc->settings()->remove(Param_Poste_PortTono_DossierEchange);
            proc->settings()->remove(Param_Poste_PortTono_COM_baudrate);
            proc->settings()->remove(Param_Poste_PortTono_COM_databits);
            proc->settings()->remove(Param_Poste_PortTono_COM_parity);
            proc->settings()->remove(Param_Poste_PortTono_COM_stopBits);
            proc->settings()->remove(Param_Poste_PortTono_COM_flowControl);
        }
        else
        {
            proc->settings()->setValue(Param_Poste_PortTono,ui->PortTonometreupComboBox->currentText());
            if (ui->PortTonometreupComboBox->currentText() == DOSSIER_ECHANGE)
            {
                proc->settings()->setValue(Param_Poste_PortTono_DossierEchange, ui->NetworkPathTonoupLineEdit->text());
                proc->settings()->remove(Param_Poste_PortTono_COM_baudrate);
                proc->settings()->remove(Param_Poste_PortTono_COM_databits);
                proc->settings()->remove(Param_Poste_PortTono_COM_parity);
                proc->settings()->remove(Param_Poste_PortTono_COM_stopBits);
                proc->settings()->remove(Param_Poste_PortTono_COM_flowControl);
            }
            else
                proc->settings()->remove(Param_Poste_PortTono_DossierEchange);
        }
        proc->settings()->setValue(Ville_Defaut,wdg_VilleDefautlineEdit->text());
        proc->settings()->setValue(CodePostal_Defaut,wdg_CPDefautlineEdit->text());
        m_modifposte = false;
    }
    return true;
}

/*!
 * \brief dlg_param::Remplir_TableCotations
 * Remplit cotationsUpTableView (UpTableView) à neuf avec les cotations de la map de référence.
 * Chaque ligne porte sa Cotation (rufusitem) et est cochée si l'utilisateur courant l'utilise
 * (used). 3 colonnes : acte, montant conventionnel, montant pratiqué.
 */
void dlg_param::Remplir_TableCotations()
{
    //! map de référence (loadCotations) + marquage used() de l'utilisateur courant (loadUserCotations)
    if (m_cotations == Q_NULLPTR)
        m_cotations = new Cotations(this);
    m_cotations         ->loadCotations();
    m_cotations         ->loadUserCotations(currentuser());

    if (m_modelCotations != Q_NULLPTR)
        delete m_modelCotations;
    m_modelCotations = new UpStandardItemModel(this);
    const QStringList entetes = QStringList() << tr("Acte") << tr("Conventionnel") << tr("Pratiqué");
    for (int c = 0; c < entetes.size(); ++c)
    {
        QStandardItem *h = new QStandardItem(entetes.at(c));
        h               ->setEditable(false);
        m_modelCotations->setHorizontalHeaderItem(c, h);
    }

    //! le montant pratiqué n'est éditable que si le user est son propre parent (il ne modifie pas les
    //! cotations d'un parent qu'il remplace) et pour une cotation qu'il utilise (sa jointure existe)
    const bool sonParent = (currentuser()->idparent() == currentuser()->id());

    int row = 0;
    QStringList listeActes;                             //! pour le QCompleter de la zone de recherche
    for (auto it = m_cotations->cotations()->constBegin(); it != m_cotations->cotations()->constEnd(); ++it)
    {
        Cotation *cot = const_cast<Cotation*>(it.value());
        if (cot == Q_NULLPTR)
            continue;
        listeActes << cot->typeacte();
        UpStandardItem *itacte = new UpStandardItem(cot->typeacte(), cot);
        itacte          ->setCheckable(true);
        itacte          ->setCheckState(cot->isused() ? Qt::Checked : Qt::Unchecked);   //! cochée si utilisée par le user
        itacte          ->setEditable(false);
        UpStandardItem *itconv = new UpStandardItem(QLocale().toString(cot->montantconventionnel(), 'f', 2), cot);
        itconv          ->setEditable(false);
        itconv          ->setTextAlignment(Qt::AlignRight);
        UpStandardItem *itprat = new UpStandardItem(QLocale().toString(cot->montantpratique(), 'f', 2), cot);
        itprat          ->setEditable(sonParent && cot->isused());   //! case pratiqué éditable en place
        itprat          ->setTextAlignment(Qt::AlignRight);
        m_modelCotations->setItem(row, 0, itacte);
        m_modelCotations->setItem(row, 1, itconv);
        m_modelCotations->setItem(row, 2, itprat);
        ++row;
    }

    QItemSelectionModel *sel = ui->cotationsUpTableView->selectionModel();   //! à détruire après setModel (bugs d'affichage à la réinitialisation)
    ui->cotationsUpTableView            ->setModel(m_modelCotations);
    if (sel != Q_NULLPTR)
        delete sel;
    ui->cotationsUpTableView            ->verticalHeader()->setVisible(false);
    ui->cotationsUpTableView            ->setColumnWidth(0,200);
    ui->cotationsUpTableView            ->setColumnWidth(1,100);
    ui->cotationsUpTableView            ->setColumnWidth(2,100);
    ui->cotationsUpTableView            ->FixLargeurTotale();

    //! complétion de la zone de recherche sur les Typeactes affichés (motif « commence par »)
    QCompleter *ancienCompleter = ui->ChercheCotationupLineEdit->completer();
    QCompleter *completer       = new QCompleter(listeActes, this);
    completer                   ->setCaseSensitivity(Qt::CaseInsensitive);
    completer                   ->setFilterMode(Qt::MatchStartsWith);
    ui->ChercheCotationupLineEdit       ->setCompleter(completer);
    if (ancienCompleter != Q_NULLPTR)
        ancienCompleter         ->deleteLater();        //! évite l'accumulation à chaque remplissage

    //! item modifié : colonne 2 (pratiqué) -> update du montant pratiqué dans la jointure ; colonne 0
    //! (coche/décoche) -> MAJCotation (à écrire)
    connect(m_modelCotations, &QStandardItemModel::itemChanged, this, [=] (QStandardItem *it) {
        UpStandardItem *upit = dynamic_cast<UpStandardItem*>(it);
        if (upit == Q_NULLPTR)
            return;
        Cotation *cot = qobject_cast<Cotation*>(upit->rufusitem());
        if (it->column() == 2)                          //! case pratiqué éditée
        {
            const double montant = QLocale().toDouble(it->text());
            MAJMontantPratique(cot, montant);
            m_modelCotations->blockSignals(true);       //! reformatage sans redéclencher itemChanged
            it->setText(QLocale().toString(montant, 'f', 2));
            m_modelCotations->blockSignals(false);
        }
        else
            MAJCotation(cot);
    });
    //! changement de ligne en surbrillance : on règle l'état des boutons (+/-/modifier)
    connect(ui->cotationsUpTableView->selectionModel(), &QItemSelectionModel::currentRowChanged, this, [=] {RegleCotationsBoutons();});
}

/*!
 * \brief dlg_param::getCotationFromIndex
 * \param idx  index d'une cellule de cotationsUpTableView
 * rend la Cotation portée par la ligne (via le rufusitem de la colonne 0)
 */
Cotation* dlg_param::getCotationFromIndex(QModelIndex idx)
{
    UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_modelCotations->item(idx.row()));
    if (itm != Q_NULLPTR)
        return qobject_cast<Cotation*>(itm->rufusitem());
    return Q_NULLPTR;
}
