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

#include "dlg_gestionusers.h"
#include "ui_dlg_gestionusers.h"

dlg_gestionusers::dlg_gestionusers(int idlieu, UserMode mode, bool mdpverified, QWidget *parent) :
    UpDialog(PATH_FILE_INI, "PositionsFiches/PositionGestionUsers", parent),
    ui(new Ui::dlg_gestionusers)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    setWindowTitle(tr("Gestion des utilisateurs"));

    m_MDPverified            = mdpverified;

    m_idlieu                 = idlieu;

    gNouvMDP        = "nouv";
    gAncMDP         = "anc";
    gConfirmMDP     = "confirm";

    gLoginupLineEdit        = "LoginupLineEdit";
    gMDPupLineEdit          = "MDPupLineEdit";
    gConfirmMDPupLineEdit   = "ConfirmMDPupLineEdit";

    AjouteLayButtons(UpDialog::ButtonClose);

    wdg_buttonframe = new WidgetButtonFrame(ui->ListUserstableWidget);
    wdg_buttonframe->AddButtons(WidgetButtonFrame::Plus | WidgetButtonFrame::Modifier | WidgetButtonFrame::Moins);

    QVBoxLayout *usrLayout = new QVBoxLayout;
    usrLayout        ->addWidget(wdg_buttonframe->widgButtonParent());
    usrLayout        ->addWidget(ui->InactifspushButton);
    QHBoxLayout *play = new QHBoxLayout;
    play        ->addItem(usrLayout);
    play        ->addWidget(ui->Principalframe);
    int marge   = 10;
    play        ->setContentsMargins(marge,marge,marge,marge);
    play        ->setSpacing(marge);
    dlglayout() ->insertLayout(0,play);

    ReconstruitListeLieuxExercice();

    gLibActiv               = tr("Activité libérale");
    gNoLibActiv             = tr("Activité non libérale");

    ui->NomuplineEdit           ->setValidator(new QRegExpValidator(Utils::rgx_rx,this));
    ui->PrenomuplineEdit        ->setValidator(new QRegExpValidator(Utils::rgx_rx,this));
    ui->AutreSoignantupLineEdit ->setValidator(new QRegExpValidator(Utils::rgx_rx,this));
    ui->AutreFonctionuplineEdit ->setValidator(new QRegExpValidator(Utils::rgx_rx,this));
    ui->MailuplineEdit          ->setValidator(new QRegExpValidator(Utils::rgx_mail,this));
    ui->PortableuplineEdit      ->setValidator(new QRegExpValidator(Utils::rgx_telephone,this));
    ui->RPPSupLineEdit          ->setValidator(new QRegExpValidator(Utils::rgx_telephone,this));
    ui->NumCOupLineEdit         ->setValidator(new QRegExpValidator(Utils::rgx_telephone,this));

    QStringList ListTitres;
    ListTitres                      << tr("Docteur") << tr("Professeur");
    ui->TitreupcomboBox             ->insertItems(0,ListTitres);
    ui->TitreupcomboBox             ->setCurrentText("");
    ui->TitreupcomboBox->lineEdit() ->setMaxLength(15);

    ui->AnnulupSmallButton          ->setUpButtonStyle(UpSmallButton::CANCELBUTTON);
    ui->AnnulupSmallButton          ->setText(tr("Annuler"));
    ui->OKupSmallButton             ->setUpButtonStyle(UpSmallButton::RECORDBUTTON);
    ui->OKupSmallButton             ->setText(tr("Enregistrer"));
    ui->InactifspushButton          ->setEnabled(Datas::I()->users->inactifs()->size()>0);

    ui->ModifMDPUserupLabel->setToolTip(tr("Modifier le mot de passe"));

    connect(ui->AnnulupSmallButton,             &QPushButton::clicked,                  this,   &dlg_gestionusers::Annulation);
    connect(ui->OKupSmallButton,                &QPushButton::clicked,                  this,   &dlg_gestionusers::EnregistreUser);
    connect(ui->OPHupRadioButton,               &QRadioButton::clicked,                 this,   &dlg_gestionusers::RegleAffichage);
    connect(ui->OrthoptistupRadioButton,        &QRadioButton::clicked,                 this,   &dlg_gestionusers::RegleAffichage);
    connect(ui->AutreSoignantupRadioButton,     &QRadioButton::clicked,                 this,   &dlg_gestionusers::RegleAffichage);
    connect(ui->AutreFonctionupRadioButton,     &QRadioButton::clicked,                 this,   &dlg_gestionusers::RegleAffichage);
    connect(ui->SecretaireupRadioButton,        &QRadioButton::clicked,                 this,   &dlg_gestionusers::RegleAffichage);
    connect(ui->ComptaLiberalupRadioButton,     &QRadioButton::clicked,                 this,   &dlg_gestionusers::RegleAffichage);
    connect(ui->ComptaLiberalSELupRadioButton,  &QRadioButton::clicked,                 this,   &dlg_gestionusers::RegleAffichage);
    connect(ui->ComptaNoLiberalupRadioButton,   &QRadioButton::clicked,                 this,   &dlg_gestionusers::RegleAffichage);
    connect(ui->ComptaRemplaupRadioButton,      &QRadioButton::clicked,                 this,   &dlg_gestionusers::RegleAffichage);
    connect(ui->OPTAMupRadioButton,             &QRadioButton::clicked,                 this,   &dlg_gestionusers::RegleAffichage);
    connect(ui->ResponsableupRadioButton,       &QRadioButton::clicked,                 this,   &dlg_gestionusers::RegleAffichage);
    connect(ui->ResponsableLes2upRadioButton,   &QRadioButton::clicked,                 this,   &dlg_gestionusers::RegleAffichage);
    connect(ui->AssistantupRadioButton,         &QRadioButton::clicked,                 this,   &dlg_gestionusers::RegleAffichage);
    connect(ui->SocieteComptableupRadioButton,  &QRadioButton::clicked,                 this,   &dlg_gestionusers::RegleAffichage);
    connect(ui->NeutreupRadioButton,            &QRadioButton::clicked,                 this,   &dlg_gestionusers::RegleAffichage);
    connect(ui->GererCompteuppushButton,        &QPushButton::clicked,                  this,   &dlg_gestionusers::GestionComptes);
    connect(ui->InactivUsercheckBox,            &QCheckBox::clicked,                    this,   [=] {ui->OKupSmallButton->setEnabled(true);});
    connect(ui->InactifspushButton,             &QPushButton::clicked,                  this,   &dlg_gestionusers::Inactifs);
    connect(ui->CotationupRadioButton,          &QRadioButton::clicked,                 this,   &dlg_gestionusers::RegleAffichage);
    connect(ui->AGAupRadioButton,               &QRadioButton::clicked,                 this,   &dlg_gestionusers::RegleAffichage);
    connect(ui->MedecincheckBox,                &QCheckBox::clicked,                    this,   &dlg_gestionusers::RegleAffichage);
    connect(ui->ModifMDPUserupLabel,            QOverload<int>::of(&UpLabel::clicked),  this,   &dlg_gestionusers::ModifMDP);
    connect(ui->Secteur1upRadioButton,          &QRadioButton::clicked,                 this,   &dlg_gestionusers::RegleAffichage);
    connect(ui->Secteur2upRadioButton,          &QRadioButton::clicked,                 this,   &dlg_gestionusers::RegleAffichage);
    connect(ui->Secteur3upRadioButton,          &QRadioButton::clicked,                 this,   &dlg_gestionusers::RegleAffichage);
    connect(ui->GestLieuxpushButton,            &QPushButton::clicked,                  this,   &dlg_gestionusers::GestLieux);

    connect(CloseButton,                        &QPushButton::clicked,                  this,   &dlg_gestionusers::FermeFiche);
    connect(wdg_buttonframe,                    &WidgetButtonFrame::choix,              this,   &dlg_gestionusers::ChoixButtonFrame);
    QList<UpLineEdit*> listline  = findChildren<UpLineEdit*>();
    for (int i=0; i<listline.size(); i++)
        connect(listline.at(i),                 &QLineEdit::textEdited,                 this,   [=] {ui->OKupSmallButton->setEnabled(true);});
    QList<UpComboBox*> listcombo     = findChildren<UpComboBox*>();
    for (int i=0; i<listcombo.size(); i++)
        connect(listcombo.at(i),                QOverload<int>::of(&QComboBox::currentIndexChanged),
                                                                                        this,   [=] {ui->OKupSmallButton->setEnabled(true);});
    foreach (UpRadioButton* butt, findChildren<UpRadioButton*>())
    {

        /* on a un pb de bug avec Qt 5.10 et les clicks sur les Qradiobutton et le slot qu'ils déclenchent Slot_RegleAffichage()
         * certains widget qui doivent être masqués le sont de façon aleatoire et se comportent alors comme s'ils étaient masqués quand on essaie d'agir dessus
         *  Je n'ai pas trouvé la parade sinon en désactivant dans l'eventfilter le click de souris sur les Qradiobutton
         * (j'ai mis pour ça la propiété toggleable à false).
         * J'en ai eu assez de chercher et j'ai résolu comme ça. Je crois avoir à peu près tout essayé.
         * donc, les radiobutton de cette fiche ne sont accessibles qu'au clavier et ça marche parfaitement comme ça.
         * Ce bug ne se produit que dans cette fiche....
        */
        //qDebug() << listbutton.at(i)->objectName();
        //listbutton.at(i)->setToggleable(false); // pour contourner un bug d'affichage de Qt...
        connect(butt,                           &QPushButton::clicked,                  this,   [=] {ui->OKupSmallButton->setEnabled(true);});
    }

    RemplirTableWidget(Datas::I()->users->userconnected()->id());

    RegleAffichage();

    dlglayout()->setSizeConstraint(QLayout::SetFixedSize);

    ui->CotationupRadioButton       ->setChecked(true);
    ui->CotationupRadioButton       ->setToggleable(false);
    ui->CotationupRadioButton       ->setImmediateToolTip(tr("Fonction indisponible\npour le moment"));
    ui->OKupSmallButton             ->setEnabled(false);
    setConfig(mode);
}

dlg_gestionusers::~dlg_gestionusers()
{
    delete ui;
}


void dlg_gestionusers::setConfig(enum UserMode mode)
{
    m_usermode = mode;
    switch (mode) {
    case PREMIERUSER:
        ui->SecretaireupRadioButton         ->setEnabled(false);
        ui->AutreFonctionupRadioButton      ->setEnabled(false);
        ui->AutreSoignantupRadioButton      ->setEnabled(false);
        ui->AutreSoignantupLineEdit         ->setVisible(false);
        ui->AutreFonctionuplineEdit         ->setVisible(false);
        ui->SocieteComptableupRadioButton   ->setEnabled(false);
        ui->NeutreupRadioButton             ->setEnabled(false);
        ui->AssistantupRadioButton          ->setEnabled(false);
        ui->ResponsableLes2upRadioButton    ->setEnabled(false);
        ui->ComptaLiberalupRadioButton      ->setChecked(true);
        ui->ComptaRemplaupRadioButton       ->setEnabled(false);
        ui->ComptaNoLiberalupRadioButton    ->setEnabled(false);
        ui->InactivUsercheckBox             ->setVisible(false);
        ui->Principalframe                  ->setEnabled(true);
        ui->AnnulupSmallButton              ->setVisible(false);
        ui->TitreupcomboBox                 ->setCurrentIndex(0);
        ui->CotationupRadioButton           ->setChecked(true);
        ui->OPHupRadioButton                ->setChecked(true);
        ui->ComptaLiberalupRadioButton      ->setChecked(true);
        ui->ResponsableupRadioButton        ->setChecked(true);
        CloseButton                         ->setVisible(false);
        wdg_buttonframe->widgButtonParent() ->setVisible(false);
        ui->InactifspushButton              ->setVisible(false);
        ui->ModifMDPUserupLabel             ->setVisible(false);
        ui->Principalframe                  ->setEnabled(true);
        RegleAffichage();
        break;
    case MODIFUSER:
        ui->FonctiongroupBox                ->setEnabled(false);
        ui->ModeExercicegroupBox            ->setEnabled(false);
        ui->ComptagroupBox                  ->setEnabled(false);
        ui->CotationupRadioButton           ->setEnabled(false);
        ui->SecteurgroupBox                 ->setEnabled(false);
        ui->OPTAMupRadioButton              ->setEnabled(false);
        ui->AGAupRadioButton                ->setEnabled(false);
        CloseButton                         ->setVisible(false);
        ui->InactivUsercheckBox             ->setVisible(false);
        ui->ModifMDPUserupLabel             ->setVisible(true);
        wdg_buttonframe->widgButtonParent() ->setVisible(false);
        ui->InactifspushButton              ->setVisible(false);
        break;
    case ADMIN:
        ui->ModifMDPUserupLabel             ->setVisible(false);
        ui->Principalframe                  ->setEnabled(false);
        wdg_buttonframe                     ->setEnabled(true);
        ui->ListUserstableWidget            ->setEnabled(true);
        break;
    }
}

void dlg_gestionusers::Annulation()
{
    if (m_mode == Creer)
    {
        int id = m_userencours->id();
        Datas::I()->users->SupprimeUser(m_userencours);
        while (Datas::I()->comptes->initListeComptesByIdUser(id).size() > 0)
        {
            int idcompteasupprimer = Datas::I()->comptes->initListeComptesByIdUser(id).firstKey();
            Datas::I()->comptes->SupprimeCompte(Datas::I()->comptes->getById(idcompteasupprimer));
        }
        RemplirTableWidget(Datas::I()->users->userconnected()->id());
        ui->Principalframe->setEnabled(false);
        wdg_buttonframe->setEnabled(true);
        ui->ListUserstableWidget->setEnabled(true);
        m_mode = Modifier;
    }
    else
    {
        if (m_usermode == PREMIERUSER || m_usermode == MODIFUSER)
            reject();
        if (ui->ListUserstableWidget->selectedItems().size()>0)
        {
            int idUser = ui->ListUserstableWidget->item(ui->ListUserstableWidget->selectedItems().at(0)->row(),0)->text().toInt();
            AfficheParamUser(idUser);
        }
        else if (AfficheParamUser(Datas::I()->users->userconnected()->id()))
        {
            int row = ui->ListUserstableWidget->findItems(QString::number(Datas::I()->users->userconnected()->id()), Qt::MatchExactly).at(0)->row();
            ui->ListUserstableWidget->selectRow(row);
        }
        ui->Principalframe->setEnabled(false);
        wdg_buttonframe->setEnabled(true);
        ui->ListUserstableWidget->setEnabled(true);
    }
}

void dlg_gestionusers::ChoixButtonFrame()
{
    switch (wdg_buttonframe->Choix()) {
    case WidgetButtonFrame::Plus:
        CreerUser();
        break;
    case WidgetButtonFrame::Modifier:
        ModifUser();
        break;
    case WidgetButtonFrame::Moins:
        SupprUser();
        break;
    }
}

void dlg_gestionusers::CreerUser()
{
    dlg_ask                     = new UpDialog(this);
    QVBoxLayout *lay            = new QVBoxLayout();
    UpLabel *label              = new UpLabel();
    UpLabel *label2             = new UpLabel();
    UpLabel *label3             = new UpLabel();
    UpLineEdit *Line            = new UpLineEdit();
    UpLineEdit *Line2           = new UpLineEdit();
    UpLineEdit *Line3           = new UpLineEdit();

    dlg_ask                        ->setWindowModality(Qt::WindowModal);
    dlg_ask                        ->move(QPoint(x()+width()/2,y()+height()/2));
    dlg_ask                        ->setFixedSize(300,300);
    dlg_ask                        ->setWindowTitle("");

    Line                        ->setObjectName(gLoginupLineEdit);
    Line2                       ->setObjectName(gMDPupLineEdit);
    Line3                       ->setObjectName(gConfirmMDPupLineEdit);
    Line                        ->setValidator(new QRegExpValidator(Utils::rgx_AlphaNumeric_5_15));
    Line2                       ->setValidator(new QRegExpValidator(Utils::rgx_AlphaNumeric_5_12));
    Line3                       ->setValidator(new QRegExpValidator(Utils::rgx_AlphaNumeric_5_12));
    Line                        ->setAlignment(Qt::AlignCenter);
    Line2                       ->setAlignment(Qt::AlignCenter);
    Line3                       ->setAlignment(Qt::AlignCenter);
    Line                        ->setMaxLength(15);
    Line2                       ->setMaxLength(12);
    Line3                       ->setMaxLength(12);
    Line                        ->setFixedHeight(20);
    Line2                       ->setFixedHeight(20);
    Line3                       ->setFixedHeight(20);
    Line2                       ->setEchoMode(QLineEdit::Password);
    Line3                       ->setEchoMode(QLineEdit::Password);
    label                       ->setMinimumHeight(46);
    label2                      ->setMinimumHeight(46);
    label3                      ->setFixedHeight(16);
    label                       ->setAlignment(Qt::AlignCenter);
    label2                      ->setAlignment(Qt::AlignCenter);
    label3                      ->setAlignment(Qt::AlignCenter);

    lay                         ->setSpacing(2);

    label                       ->setText(tr("Choisissez un login pour le nouvel utilisateur\n- mini 5 maxi 15 caractères -\n- pas de caractères spéciaux ou accentués -"));
    label2                      ->setText(tr("Choisissez un mot de passe\n- mini 5 maxi 12 caractères -\n- pas de caractères spéciaux ou accentués -"));
    label3                      ->setText(tr("Confirmez le mot de passe"));

    dlg_ask                     ->AjouteLayButtons(UpDialog::ButtonCancel | UpDialog::ButtonOK);
    connect(dlg_ask->OKButton,  &QPushButton::clicked,  this,   &dlg_gestionusers::EnregistreNouvUser);

    lay                         ->addWidget(label);
    lay                         ->addWidget(Line);
    lay                         ->addWidget(label2);
    lay                         ->addWidget(Line2);
    lay                         ->addWidget(label3);
    lay                         ->addWidget(Line3);
    lay                         ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding));
    lay->setContentsMargins(5,5,5,5);
    lay->setSpacing(5);

    dlg_ask->dlglayout()           ->insertLayout(0,lay);
    dlg_ask->dlglayout()           ->setSizeConstraint(QLayout::SetFixedSize);

    Line                        ->setFocus();
    dlg_ask->exec();
    delete dlg_ask;
    dlg_ask = Q_NULLPTR;
}

void dlg_gestionusers::EnregistreNouvMDP()
{
    if (gAskMDP)
    {
        // Vérifier la cohérence
        QString anc, nouv, confirm;
        UpMessageBox msgbox(this);
        msgbox.setText(tr("Erreur"));
        msgbox.setIcon(UpMessageBox::Warning);
        UpSmallButton OKBouton("OK");
        msgbox.addButton(&OKBouton, UpSmallButton::STARTBUTTON);
        anc         = gAskMDP->findChild<UpLineEdit*>(gAncMDP)->text();
        nouv        = gAskMDP->findChild<UpLineEdit*>(gNouvMDP)->text();
        confirm     = gAskMDP->findChild<UpLineEdit*>(gConfirmMDP)->text();

        if (anc == "")
        {
            QSound::play(NOM_ALARME);
            msgbox.setInformativeText(tr("Ancien mot de passe requis"));
            gAskMDP->findChild<UpLineEdit*>(gAncMDP)->setFocus();
            msgbox.exec();
            return;
        }
        if (Utils::calcSHA1(anc) != m_userencours->password())
        {
            QSound::play(NOM_ALARME);
            msgbox.setInformativeText(tr("Le mot de passe que vous voulez modifier n'est pas bon\n"));
            gAskMDP->findChild<UpLineEdit*>(gAncMDP)->setFocus();
            msgbox.exec();
            return;
        }
        if (!Utils::rgx_AlphaNumeric_5_15.exactMatch(nouv) || nouv == "")
        {
            QSound::play(NOM_ALARME);
            msgbox.setInformativeText(tr("Le nouveau mot de passe n'est pas conforme\n(au moins 5 caractères - chiffres ou lettres non accentuées -\n"));
            gAskMDP->findChild<UpLineEdit*>(gNouvMDP)->setFocus();
            msgbox.exec();
            return;
        }
        if (nouv != confirm)
        {
            QSound::play(NOM_ALARME);
            msgbox.setInformativeText("Les mots de passe ne correspondent pas\n");
            gAskMDP->findChild<UpLineEdit*>(gNouvMDP)->setFocus();
            msgbox.exec();
            return;
        }
        msgbox.setText(tr("Modifications enregistrées"));
        msgbox.setInformativeText(tr("Le nouveau mot de passe a été enregistré avec succès"));
        // Enregitrer le nouveau MDP de la base
        QString shanouv = Utils::calcSHA1(nouv);
        db->StandardSQL("update " TBL_UTILISATEURS " set " CP_MDP_USR " = '" + shanouv + "' where " CP_ID_USR " = " + ui->idUseruplineEdit->text());
        ui->MDPuplineEdit->setText(nouv);
        m_userencours->setpassword(shanouv);
        gAskMDP->done(0);
        msgbox.exec();
    }
}

void dlg_gestionusers::EnregistreUser()
{
    if (!VerifFiche()) return;
    QString titre = (ui->OPHupRadioButton->isChecked()?       "'" + Utils::correctquoteSQL(ui->TitreupcomboBox->currentText()) + "'" : "null");
    QString actif = (ui->InactivUsercheckBox->isChecked()?  "1" : "null");
    QString req = "update " TBL_UTILISATEURS " set "
            CP_NOM_USR " = '"           + Utils::correctquoteSQL(Utils::trimcapitilize(ui->NomuplineEdit->text()))        + "',\n"
            CP_PRENOM_USR " = "         + ((ui->SocieteComptableupRadioButton->isChecked() || ui->NeutreupRadioButton->isChecked())? "null" : "'" + Utils::correctquoteSQL(Utils::trimcapitilize(ui->PrenomuplineEdit->text())) + "'") + ",\n"
            CP_PORTABLE_USR " = '"      + ui->PortableuplineEdit->text()   + "',\n"
            CP_MAIL_USR " = '"          + ui->MailuplineEdit->text()       + "',\n"
            CP_POLICEECRAN_USR " = '"   POLICEPARDEFAUT "',\n"
            CP_POLICEATTRIBUT_USR " = 'Regular',\n"
            CP_TITRE_USR " = "          + titre + ",\n"
            CP_ISDESACTIVE_USR " = "    + actif + ",\n";
    if (ui->OPHupRadioButton->isChecked())
    {
        req += CP_FONCTION_USR " = '"   + tr("Médecin") + "',\n"
               CP_SPECIALITE_USR " = '" + tr("Ophtalmologiste") + "',\n"
               CP_IDSPECIALITE_USR " = 15,\n"
               CP_SOIGNANTSTATUS_USR " = 1,\n"
               CP_ISMEDECIN_USR " = 1,\n"
               CP_NUMCO_USR " = '"      + Utils::correctquoteSQL(ui->NumCOupLineEdit->text()) +"',\n "
               CP_NUMPS_USR " = "       + ((ui->RPPSupLineEdit->text().toInt()==0 || !db->parametres()->cotationsfrance())? "null" : QString::number(ui->RPPSupLineEdit->text().toInt())) + ",\n"
               CP_RESPONSABLEACTES_USR " = ";
        if (ui->ResponsableupRadioButton->isChecked())
            req += "1,\n";
        else if (ui->ResponsableLes2upRadioButton->isChecked())
            req += "2,\n";
        else if (ui->AssistantupRadioButton->isChecked())
            req += "3,\n";
        if (ui->AssistantupRadioButton->isChecked())
        {
            req += CP_DROITS_USR " = '" OPHTAASSISTANT "', \n"
                   CP_IDCOMPTEPARDEFAUT_USR " = null,\n"
                   CP_IDEMPLOYEUR_USR " = null,\n"
                   CP_ENREGHONORAIRES_USR " = null,\n"
                   CP_ISAGA_USR " = null,\n";
        }
        else
        {
            if (ui->ComptaLiberalupRadioButton->isChecked())
            {
                req += CP_DROITS_USR " = '" OPHTALIBERAL "', \n"
                       CP_IDCOMPTEPARDEFAUT_USR " = " + ui->CompteParDefautcomboBox->currentData().toString() + ",\n"
                       CP_ENREGHONORAIRES_USR " = 1,\n"
                       CP_IDEMPLOYEUR_USR " = null,\n";
                if (ui->AGAupRadioButton->isChecked())
                    req += CP_ISAGA_USR " = 1,\n";
                else
                    req += CP_ISAGA_USR " = null,\n";
            }
            else if (ui->ComptaLiberalSELupRadioButton->isChecked())
            {
                req += CP_DROITS_USR " = '" OPHTASEL "', \n"
                       CP_IDCOMPTEPARDEFAUT_USR " = " + ui->CompteParDefautcomboBox->currentData().toString() + ",\n"
                       CP_ENREGHONORAIRES_USR " = 5,\n"
                       CP_IDEMPLOYEUR_USR " = " + ui->EmployeurcomboBox->currentData().toString() + ",\n";
                if (ui->AGAupRadioButton->isChecked())
                    req += CP_ISAGA_USR " = 1,\n";
                else
                    req += CP_ISAGA_USR " = null,\n";
            }
            else if (ui->ComptaNoLiberalupRadioButton->isChecked())
                req += CP_DROITS_USR " = '" OPHTASALARIE "', \n"
                       CP_IDCOMPTEPARDEFAUT_USR " = null,\n"
                       CP_IDEMPLOYEUR_USR " = " + ui->EmployeurcomboBox->currentData().toString() + ",\n"
                       CP_ENREGHONORAIRES_USR " = 2,\n"
                       CP_ISAGA_USR " = null,\n";
            else if (ui->ComptaRemplaupRadioButton->isChecked())
                req += CP_DROITS_USR " = '" OPHTAREMPLACANT "', \n"
                       CP_IDCOMPTEPARDEFAUT_USR " = null,\n"
                       CP_ENREGHONORAIRES_USR " = 3,\n"
                       CP_IDEMPLOYEUR_USR " = null,\n"
                       CP_ISAGA_USR " = null,\n";
        }
        req += ((ui->CotationupRadioButton->isVisible() && ui->CotationupRadioButton->isChecked())?   CP_COTATION_USR " = 1,\n" : CP_COTATION_USR " = null,\n");
    }
    else if (ui->OrthoptistupRadioButton->isChecked())
    {
        req += CP_FONCTION_USR " = '" + tr("Orthoptiste") + "',\n"
               CP_SPECIALITE_USR " = '" + tr("Orthoptiste") + "',\n"
               CP_IDSPECIALITE_USR " = null,\n"
               CP_SOIGNANTSTATUS_USR " = 2,\n"
               CP_ISMEDECIN_USR " = null,\n"
               CP_NUMCO_USR " = null,\n "
               CP_NUMPS_USR " = "       + ((ui->RPPSupLineEdit->text().toInt()==0 || !db->parametres()->cotationsfrance())? "null" : QString::number(ui->RPPSupLineEdit->text().toInt())) + ",\n"
               CP_RESPONSABLEACTES_USR " = ";
        if (ui->ResponsableupRadioButton->isChecked())
            req += "1,\n";
        else if (ui->ResponsableLes2upRadioButton->isChecked())
            req += "2,\n";
        else if (ui->AssistantupRadioButton->isChecked())
            req += "3,\n";
        if (ui->AssistantupRadioButton->isChecked())
        {
            req += CP_DROITS_USR " = '" ORTHOASSISTANT "', \n"
                   CP_IDCOMPTEPARDEFAUT_USR " = null,\n"
                   CP_ENREGHONORAIRES_USR " = null,\n"
                   CP_IDEMPLOYEUR_USR " = null,\n"
                   CP_ISAGA_USR " = null,\n";
        }
        else
        {
            if (ui->ComptaLiberalupRadioButton->isChecked())
            {
                req += CP_DROITS_USR " = '" ORTHOLIBERAL "', \n"
                       CP_IDCOMPTEPARDEFAUT_USR " = " + ui->CompteParDefautcomboBox->currentData().toString() + ",\n"
                       CP_ENREGHONORAIRES_USR " = 1,\n"
                       CP_IDEMPLOYEUR_USR " = null,\n";
                if (ui->AGAupRadioButton->isChecked())
                    req += CP_ISAGA_USR " = 1,\n";
                else
                    req += CP_ISAGA_USR " = null,\n";
            }
            else if (ui->ComptaLiberalSELupRadioButton->isChecked())
            {
                req += CP_DROITS_USR " = '" ORTHOLIBERAL "', \n"
                       CP_IDCOMPTEPARDEFAUT_USR " = " + ui->CompteParDefautcomboBox->currentData().toString() + ",\n"
                       CP_ENREGHONORAIRES_USR " = 5,\n"
                       CP_IDEMPLOYEUR_USR " = " + ui->EmployeurcomboBox->currentData().toString() + ",\n";
                if (ui->AGAupRadioButton->isChecked())
                    req += CP_ISAGA_USR " = 1,\n";
                else
                    req += CP_ISAGA_USR " = null,\n";
            }
            else if (ui->ComptaNoLiberalupRadioButton->isChecked())
                req += CP_DROITS_USR " = '" ORTHOSALARIE "', \n"
                       CP_IDCOMPTEPARDEFAUT_USR " = null,\n"
                       CP_IDEMPLOYEUR_USR " = " + ui->EmployeurcomboBox->currentData().toString() + ",\n"
                       CP_ENREGHONORAIRES_USR " = 2,\n"
                       CP_ISAGA_USR " = null,\n";
            else if (ui->ComptaRemplaupRadioButton->isChecked())
                req += CP_DROITS_USR " = '" ORTHOREMPLACANT "', \n"
                       CP_IDCOMPTEPARDEFAUT_USR " = null,\n"
                       CP_ENREGHONORAIRES_USR " = 3,\n"
                       CP_IDEMPLOYEUR_USR " = null,\n"
                       CP_ISAGA_USR " = null,\n";
        }
        req += ((ui->CotationupRadioButton->isVisible() && ui->CotationupRadioButton->isChecked())?   CP_COTATION_USR " = 1,\n" : CP_COTATION_USR " = null,\n");
    }
    else if (ui->AutreSoignantupRadioButton->isChecked())
    {
        req += CP_FONCTION_USR " = '" + Utils::correctquoteSQL(ui->AutreSoignantupLineEdit->text()) + "',\n"
               CP_SPECIALITE_USR " = '" + Utils::correctquoteSQL(ui->AutreSoignantupLineEdit->text()) + "',\n"
               CP_IDSPECIALITE_USR " = null,\n"
               CP_SOIGNANTSTATUS_USR " = 3,\n"
               CP_ISMEDECIN_USR " = " + (ui->MedecincheckBox->isChecked()? "1" : "null") + ",\n"
               CP_NUMCO_USR " = " + (ui->MedecincheckBox->isChecked()? (ui->NumCOupLineEdit->text()==""? "null" : "'" + ui->NumCOupLineEdit->text() + "'") : "null") + ",\n "
               CP_NUMPS_USR " = " + ((ui->RPPSupLineEdit->text().toInt()==0 || !db->parametres()->cotationsfrance())? "null" : QString::number(ui->RPPSupLineEdit->text().toInt())) + ",\n"
               CP_RESPONSABLEACTES_USR " = ";
        if (ui->ResponsableupRadioButton->isChecked())
            req += "1,\n";
        else if (ui->ResponsableLes2upRadioButton->isChecked())
            req += "2,\n";
        else if (ui->AssistantupRadioButton->isChecked())
            req += "3,\n";
        if (ui->AssistantupRadioButton->isChecked())
        {
            req += CP_DROITS_USR " = '" AUTRESOIGNANTASSISTANT "', \n"
                   CP_IDCOMPTEPARDEFAUT_USR " = null,\n"
                   CP_ENREGHONORAIRES_USR " = null,\n"
                   CP_IDEMPLOYEUR_USR " = null,\n"
                   CP_ISAGA_USR " = null,\n";
        }
        else
        {
            if (ui->ComptaLiberalupRadioButton->isChecked())
            {
                req += CP_DROITS_USR " = '" AUTRESOIGNANTLIBERAL "', \n"
                       CP_IDCOMPTEPARDEFAUT_USR " = " + ui->CompteParDefautcomboBox->currentData().toString() + ",\n"
                       CP_ENREGHONORAIRES_USR " = 1,\n"
                       CP_IDEMPLOYEUR_USR " = null,\n";
                if (ui->AGAupRadioButton->isChecked())
                    req += CP_ISAGA_USR " = 1,\n";
                else
                    req += CP_ISAGA_USR " = null,\n";
            }
            else if (ui->ComptaLiberalSELupRadioButton->isChecked())
            {
                req += CP_DROITS_USR " = '" AUTRESOIGNANTLIBERAL "', \n"
                       CP_IDCOMPTEPARDEFAUT_USR " = " + ui->CompteParDefautcomboBox->currentData().toString() + ",\n"
                       CP_ENREGHONORAIRES_USR " = 5,\n"
                       CP_IDEMPLOYEUR_USR " = " + ui->EmployeurcomboBox->currentData().toString() + ",\n";
                if (ui->AGAupRadioButton->isChecked())
                    req += CP_ISAGA_USR " = 1,\n";
                else
                    req += CP_ISAGA_USR " = null,\n";
            }
            else if (ui->ComptaNoLiberalupRadioButton->isChecked())
                req += CP_DROITS_USR " = '" AUTRESOIGNANTSALARIE "', \n"
                       CP_IDCOMPTEPARDEFAUT_USR " = null,\n"
                       CP_IDEMPLOYEUR_USR " = " + ui->EmployeurcomboBox->currentData().toString() + ",\n"
                       CP_ENREGHONORAIRES_USR " = 2,\n"
                       CP_ISAGA_USR " = null,\n";
            else if (ui->ComptaRemplaupRadioButton->isChecked())
                req += CP_DROITS_USR " = '" AUTRESOIGNANTREMPLACANT "', \n"
                       CP_IDCOMPTEPARDEFAUT_USR " = null,\n"
                       CP_ENREGHONORAIRES_USR " = 3,\n"
                       CP_IDEMPLOYEUR_USR " = null,\n"
                       CP_ISAGA_USR " = null,\n";
        }
        req += ((ui->CotationupRadioButton->isVisible() && ui->CotationupRadioButton->isChecked())?   CP_COTATION_USR " = 1,\n" : CP_COTATION_USR " = null,\n");
    }
    else if (ui->AutreFonctionupRadioButton->isChecked())
        req += CP_FONCTION_USR " = '" + Utils::correctquoteSQL(ui->AutreFonctionuplineEdit->text()) + "',\n"
               CP_SPECIALITE_USR " = '" + Utils::correctquoteSQL(ui->AutreFonctionuplineEdit->text()) + "',\n"
               CP_IDSPECIALITE_USR " = null,\n"
               CP_SOIGNANTSTATUS_USR " = 4,\n"
               CP_ISMEDECIN_USR " = null,\n"
               CP_ISAGA_USR " = null,\n"
               CP_RESPONSABLEACTES_USR " = null,\n"
               CP_IDCOMPTEPARDEFAUT_USR " = null,\n"
               CP_COTATION_USR " = null,\n"
               CP_ENREGHONORAIRES_USR " = null,\n"
               CP_IDEMPLOYEUR_USR " = null,\n"
               CP_NUMCO_USR " = null,\n "
               CP_NUMPS_USR " = null,\n "
               CP_DROITS_USR " = '" AUTREFONCTION "',\n";
    else if (ui->SecretaireupRadioButton->isChecked())
        req += CP_FONCTION_USR " = '" + tr("Secrétaire") + "',\n"
               CP_SPECIALITE_USR " = '" + tr("Secrétaire médicale") + "',\n"
               CP_IDSPECIALITE_USR " = null,\n"
               CP_SOIGNANTSTATUS_USR " = 4,\n"
               CP_ISMEDECIN_USR " = null,\n"
               CP_ISAGA_USR " = null,\n"
               CP_RESPONSABLEACTES_USR " = null,\n"
               CP_IDCOMPTEPARDEFAUT_USR " = null,\n"
               CP_COTATION_USR " = null,\n"
               CP_ENREGHONORAIRES_USR " = null,\n"
               CP_IDEMPLOYEUR_USR " = null,\n"
               CP_NUMCO_USR " = null,\n "
               CP_NUMPS_USR " = null,\n "
               CP_DROITS_USR " = '" SECRETAIRE "',\n";
    else if (ui->SocieteComptableupRadioButton->isChecked())
        req += CP_FONCTION_USR " = '" + tr("Société") + "',\n"
               CP_SPECIALITE_USR " = '" + tr("Société") + "',\n"
               CP_IDSPECIALITE_USR " = null,\n"
               CP_SOIGNANTSTATUS_USR " = 5,\n"
               CP_ISMEDECIN_USR " = null,\n"
               CP_ISAGA_USR " = null,\n"
               CP_RESPONSABLEACTES_USR " = null,\n"
               CP_IDCOMPTEPARDEFAUT_USR " = " + ui->CompteParDefautcomboBox->currentData().toString() + ",\n"
               CP_COTATION_USR " = null,\n"
               CP_ENREGHONORAIRES_USR " = null,\n"
               CP_IDEMPLOYEUR_USR " = null,\n"
               CP_NUMCO_USR " = null,\n "
               CP_NUMPS_USR " = null,\n "
               CP_DROITS_USR " = '" SOCIETECOMPTABLE "',\n";
    else if (ui->NeutreupRadioButton->isChecked())
        req += CP_FONCTION_USR " = null,\n"
               CP_SPECIALITE_USR " = null,\n"
               CP_IDSPECIALITE_USR " = null,\n"
               CP_SOIGNANTSTATUS_USR " = 6,\n"
               CP_ISMEDECIN_USR " = null,\n"
               CP_ISAGA_USR " = null,\n"
               CP_RESPONSABLEACTES_USR " = null,\n"
               CP_IDCOMPTEPARDEFAUT_USR " = null,\n"
               CP_COTATION_USR " = null,\n"
               CP_ENREGHONORAIRES_USR " = null,\n"
               CP_IDEMPLOYEUR_USR " = null,\n"
               CP_NUMCO_USR " = null,\n "
               CP_NUMPS_USR " = null,\n "
               CP_DROITS_USR " = '" NEUTRE "',\n";
    QString secteur = "null";
    QString Optam   = "null";
    if ((ui->OPHupRadioButton->isChecked() ||(ui->AutreSoignantupRadioButton->isChecked() && ui->MedecincheckBox->isChecked()))
        && !ui->AssistantupRadioButton->isChecked() && !ui->ComptaRemplaupRadioButton->isChecked() && ui->CotationupRadioButton->isChecked())
    {
        if (db->parametres()->cotationsfrance())
        {
            if (ui->Secteur1upRadioButton       ->isChecked())      secteur = "1";
            else if (ui->Secteur2upRadioButton  ->isChecked())      secteur = "2";
            else if (ui->Secteur3upRadioButton  ->isChecked())      secteur = "3";
            if (ui->OPTAMupRadioButton->isChecked() && (ui->Secteur1upRadioButton->isChecked() || ui->Secteur2upRadioButton->isChecked()))
                Optam = "1";
        }
    }
    req += CP_SECTEUR_USR " = " + secteur + ",\n";
    req += CP_ISOPTAM_USR " = " + Optam + "\n";

    req +=  " where " CP_ID_USR " = " + ui->idUseruplineEdit->text();
    //qDebug()<<req;
    db->StandardSQL(req);
    int idlieu=-1;
    db->SupprRecordFromTable(ui->idUseruplineEdit->text().toInt(), "idUser", TBL_JOINTURESLIEUX);
    if (!m_neutre)
        for(int i=0; i< ui->AdressupTableWidget->rowCount(); i++)
        {
            UpRadioButton *butt = static_cast<UpRadioButton*>(ui->AdressupTableWidget->cellWidget(i,0));
            if (butt->isChecked())
            {
                idlieu = butt->iD();
                db->StandardSQL("insert into " TBL_JOINTURESLIEUX "(iduser, idlieu) values (" + ui->idUseruplineEdit->text() + ", " + QString::number(idlieu) + ")");
            }
        }

    req = "update " TBL_COMPTES " set " CP_PARTAGE_COMPTES " = ";
    db->StandardSQL(req + (ui->SocieteComptableupRadioButton->isChecked()? "1" : "null") + " where " CP_IDUSER_COMPTES " = " +  ui->idUseruplineEdit->text());
    setDataCurrentUser(ui->idUseruplineEdit->text().toInt()); // reactualise l'item user correspondant
    RemplirTableWidget(ui->idUseruplineEdit->text().toInt());

    if (m_usermode == PREMIERUSER)
    {
        done(ui->idUseruplineEdit->text().toInt());
        return;
    }
    else if (m_usermode == MODIFUSER)
    {
        accept();
        return;
    }
    else if (m_mode == Creer)
    {
        m_mode = Modifier;
        ui->Principalframe->setEnabled(false);
        wdg_buttonframe->setEnabled(true);
        ui->ListUserstableWidget->setEnabled(true);
        wdg_buttonframe->wdg_moinsBouton->setEnabled(true);
    }
    else
    {
        ui->Principalframe->setEnabled(false);
        wdg_buttonframe->setEnabled(true);
        ui->ListUserstableWidget->setEnabled(true);
        wdg_buttonframe->wdg_moinsBouton->setEnabled(ui->ListUserstableWidget->findItems(ui->idUseruplineEdit->text(),Qt::MatchExactly).at(0)->foreground() != m_color);
    }
    Datas::I()->users->initListe();
    ui->OKupSmallButton->setEnabled(false);
}

void dlg_gestionusers::EnregistreNouvUser()
{
    if (!dlg_ask) return;
    QString msg = "";
    UpLineEdit *Loginline       = dlg_ask->findChild<UpLineEdit*>(gLoginupLineEdit);
    UpLineEdit *MDPline         = dlg_ask->findChild<UpLineEdit*>(gMDPupLineEdit);
    UpLineEdit *ConfirmMDPline  = dlg_ask->findChild<UpLineEdit*>(gConfirmMDPupLineEdit);
    QString login               = Loginline->text();
    QString mdp                 = MDPline->text();
    bool a = true;
    while (a) {
        a = false;
        if (login == "")
        {
            msg = tr("Vous avez oublié d'indiquer le login");
            Loginline->setFocus();
            continue;
        }
        if (mdp == "")
        {
            msg = tr("Vous avez oublié d'indiquer le mot de passe");
            MDPline->setFocus();
            continue;
        }
        QString nomadmin = NOM_ADMINISTRATEUR;
        QString loginsql = LOGIN_SQL;
        if (login.toUpper() == nomadmin.toUpper() || login.toUpper() == loginsql.toUpper())
        {
            msg = tr("Ce login est réservé");
            Loginline->setFocus();
            Loginline->selectAll();
            a = false;
        }
        for (auto it = Datas::I()->users->actifs()->constBegin(); it != Datas::I()->users->actifs()->constEnd(); ++it)
        {
            User* usr = const_cast<User*>(it.value());
            if (usr->login().toUpper() == login.toUpper())
            {
                msg = tr("Ce login est déjà utilisé");
                Loginline->setFocus();
                Loginline->selectAll();
                break;
            }
            if (msg != "")
                continue;
        }
        if (!Utils::rgx_AlphaNumeric_5_12.exactMatch(mdp))
        {
            msg = tr("Le mot de passe n'est pas conforme.") + "\n" +
                  tr("Au moins 5 caractères - uniquement des chifres ou des lettres - max. 12 caractères.");
            MDPline->setFocus();
            continue;
        }
        if (mdp != ConfirmMDPline->text())
        {
            msg = tr("Les mots de passe ne correspondent pas");
            MDPline->setFocus();
            continue;
        }
    }
    if (msg != "")
    {
        QSound::play(NOM_ALARME);
        UpMessageBox::Watch(this,tr("Erreur"), msg);
        return;
    }
    dlg_ask->accept();
    m_mode                          = Creer;
    db->locktable(TBL_UTILISATEURS);
    db->StandardSQL("insert into " TBL_UTILISATEURS " (" CP_LOGIN_USR ", " CP_NOM_USR "," CP_MDP_USR ", " CP_POLICEECRAN_USR ", " CP_POLICEATTRIBUT_USR ")"
                  " VALUES ('" + Utils::correctquoteSQL(login) + "', '" + Utils::correctquoteSQL(Utils::capitilize(login)) + "', '" + Utils::calcSHA1(mdp) + "', '" POLICEPARDEFAUT "', '" POLICEATTRIBUTPARDEFAUT "')");
    QString req = "select " CP_ID_USR " from " TBL_UTILISATEURS " where " CP_LOGIN_USR " = '" + login + "' and " CP_MDP_USR " = '" + Utils::calcSHA1(mdp) + "'";
    int idUser = db->getFirstRecordFromStandardSelectSQL(req,m_ok).at(0).toInt();
    db->unlocktables();
    Datas::I()->users->initListe();
    RemplirTableWidget(idUser);
    wdg_buttonframe                 ->setEnabled(false);
    ui->ListUserstableWidget        ->setEnabled(false);
    ui->Principalframe              ->setEnabled(true);
    ui->ComptagroupBox              ->setEnabled(true);
    setDataCurrentUser(idUser);
    ui->OPHupRadioButton            ->setChecked(true);
    ui->ResponsableupRadioButton    ->setChecked(true);
    ui->ComptaLiberalupRadioButton  ->setChecked(true);
    ui->CotationupRadioButton       ->setChecked(true);
    ui->Secteur1upRadioButton       ->setChecked(true);
    RegleAffichage();
}

void dlg_gestionusers::FermeFiche()
{
    if (ui->OKupSmallButton->isEnabled())
    {
        UpMessageBox msgbox(this);
        UpSmallButton OKBouton(tr("Enregistrer les modifications"));
        UpSmallButton AnnulBouton(tr("Annnuler"));
        msgbox.setText(tr("Modification des données"));
        msgbox.setInformativeText(tr("Vous avez modifié les données de ") + ui->LoginuplineEdit->text() + ".\n"
                                  + tr("Voulez-vous enregistrer ces modifications?") + "\n");
        msgbox.setIcon(UpMessageBox::Info);
        msgbox.addButton(&AnnulBouton, UpSmallButton::CANCELBUTTON);
        msgbox.addButton(&OKBouton, UpSmallButton::STARTBUTTON);
        msgbox.exec();
        if (msgbox.clickedButton()==&OKBouton)
        {
            if (VerifFiche())
            {
                ui->OKupSmallButton->click();
                reject();
            }
            else
                return;
        }
        else
            return;
    }
    reject();
}

// ----------------------------------------------------------------------------------
// Ouverture de la fiche dlg_gestioncomptes
// ----------------------------------------------------------------------------------
void dlg_gestionusers::GestionComptes()
{
    bool modif      = false;
    bool verifempl  = ui->Employeurwidget->isVisible();
    QString empl    = ui->EmployeurcomboBox->currentText();
    bool verifcpta  = ui->CompteComptawidget->isVisible();
    QString cptcpta = ui->CompteParDefautcomboBox->currentText();

    bool comptedesociete    = ui->SocieteComptableupRadioButton->isChecked();
    dlg_gestioncomptes *Dlg_GestComptes = new dlg_gestioncomptes(m_userencours, comptedesociete, this);
    Dlg_GestComptes ->exec();
    delete Dlg_GestComptes;
    if (verifempl)
        if (ui->EmployeurcomboBox->currentText() != empl)
            modif   = true;
    if (verifcpta)
    {
        CalcListitemsCompteComptacomboBox(m_userencours, ui->SocieteComptableupRadioButton->isChecked());
        if (ui->CompteParDefautcomboBox->currentText() != cptcpta)
            modif   = true;
    }
    ui->OKupSmallButton->setEnabled(modif);
}

bool dlg_gestionusers::isMDPverified() const
{
    return m_MDPverified;
}

void dlg_gestionusers::ModifUser()
{
    ui->ListUserstableWidget        ->setEnabled(false);
    wdg_buttonframe                 ->setEnabled(false);
    ui->Principalframe              ->setEnabled(true);
    ui->ModeExercicegroupBox        ->setEnabled(true);
    ui->SecteurgroupBox             ->setEnabled(true);
    ui->OPTAMupRadioButton          ->setEnabled(true);
    ui->OKupSmallButton             ->setEnabled(false);
    ui->AGAupRadioButton            ->setEnabled(true);
    m_mode                          = Modifier;
}

void dlg_gestionusers::GestLieux()
{
    QString mdp("");
    m_MDPverified = Utils::VerifMDP(DataBase::I()->getMDPAdmin(), tr("Saisissez le mot de passe Administrateur"), mdp, m_MDPverified, this);
    if (!m_MDPverified)
            return;
    dlg_listelieux *gestLieux = new dlg_listelieux(this);
    gestLieux->exec();
    ReconstruitListeLieuxExercice();
    delete gestLieux;
    Datas::I()->sites->initListe();
    int idUser = ui->ListUserstableWidget->item(ui->ListUserstableWidget->selectedItems().at(0)->row(),0)->text().toInt();
    QList<QVariantList> listlieux = db->StandardSelectSQL("select idlieu from " TBL_JOINTURESLIEUX " where iduser = " + QString::number(idUser), m_ok);
    QList<int> idlieuxlist;
    for (int k=0; k< listlieux.size(); k++)
        idlieuxlist << listlieux.at(k).at(0).toInt();
    for (int i=0; i<ui->AdressupTableWidget->rowCount(); ++i)
    {
        UpRadioButton *butt = static_cast<UpRadioButton*>(ui->AdressupTableWidget->cellWidget(i,0));
        butt->setChecked(idlieuxlist.contains(butt->iD()));
    }
}

void dlg_gestionusers::ModifMDP()
{
    gAskMDP    = new UpDialog(this);
    gAskMDP    ->setWindowModality(Qt::WindowModal);
    gAskMDP    ->move(QPoint(x()+width()/2,y()+height()/2));

    UpLineEdit *ConfirmMDP = new UpLineEdit(gAskMDP);
    ConfirmMDP->setEchoMode(QLineEdit::Password);
    ConfirmMDP->setObjectName(gConfirmMDP);
    ConfirmMDP->setValidator(new QRegExpValidator(Utils::rgx_AlphaNumeric_5_12,this));
    ConfirmMDP->setAlignment(Qt::AlignCenter);
    ConfirmMDP->setMaxLength(12);
    gAskMDP->dlglayout()->insertWidget(0,ConfirmMDP);
    UpLabel *labelConfirmMDP = new UpLabel();
    labelConfirmMDP->setText(tr("Confirmez le nouveau mot de passe"));
    gAskMDP->dlglayout()->insertWidget(0,labelConfirmMDP);
    UpLineEdit *NouvMDP = new UpLineEdit(gAskMDP);
    NouvMDP->setEchoMode(QLineEdit::Password);
    NouvMDP->setObjectName(gNouvMDP);
    NouvMDP->setValidator(new QRegExpValidator(Utils::rgx_AlphaNumeric_5_12,this));
    NouvMDP->setAlignment(Qt::AlignCenter);
    NouvMDP->setMaxLength(12);
    gAskMDP->dlglayout()->insertWidget(0,NouvMDP);
    UpLabel *labelNewMDP = new UpLabel();
    labelNewMDP->setText(tr("Entrez le nouveau mot de passe"));
    gAskMDP->dlglayout()->insertWidget(0,labelNewMDP);
    UpLineEdit *AncMDP = new UpLineEdit(gAskMDP);
    AncMDP->setEchoMode(QLineEdit::Password);
    AncMDP->setAlignment(Qt::AlignCenter);
    AncMDP->setValidator(new QRegExpValidator(Utils::rgx_AlphaNumeric_3_12,this));
    AncMDP->setObjectName(gAncMDP);
    AncMDP->setMaxLength(12);
    gAskMDP->dlglayout()->insertWidget(0,AncMDP);
    UpLabel *labelOldMDP = new UpLabel();
    labelOldMDP->setText(tr("Ancien mot de passe"));
    gAskMDP->dlglayout()->insertWidget(0,labelOldMDP);
    AncMDP->setFocus();

    gAskMDP->AjouteLayButtons(UpDialog::ButtonCancel | UpDialog::ButtonOK);
    QList <QWidget*> ListTab;
    ListTab << AncMDP << NouvMDP << ConfirmMDP << gAskMDP->OKButton;
    for (int i = 0; i<ListTab.size()-1 ; i++ )
        gAskMDP->setTabOrder(ListTab.at(i), ListTab.at(i+1));
    gAskMDP    ->setWindowTitle(tr("Mot de passe utilisateur"));
    connect(gAskMDP->OKButton,    &QPushButton::clicked, this, &dlg_gestionusers::EnregistreNouvMDP);
    gAskMDP->dlglayout()->setSizeConstraint(QLayout::SetFixedSize);

    gAskMDP->exec();
}

void dlg_gestionusers::RegleAffichage()
{
    /* Valeurs de soignant
     * 1 = ophtalmo
     * 2 = Orthoptiste
     * 3 = autre
     * 4 = non soignant
     * 5 = societe comptable
     * 6 = neutre
     */
    DefinitLesVariables();

    ui->RPPSlabel                   ->setVisible(m_responsable && db->parametres()->cotationsfrance());
    ui->RPPSupLineEdit              ->setVisible(m_responsable && db->parametres()->cotationsfrance());
    ui->ModeExercicegroupBox        ->setVisible(m_soignant);
    ui->CotationupRadioButton       ->setVisible(m_soignantnonremplacant);
    ui->SecteurgroupBox             ->setVisible(m_medecin && m_soignantnonremplacant && db->parametres()->cotationsfrance());
    ui->OPTAMupRadioButton          ->setVisible(m_medecin && m_soignantnonremplacant && (ui->Secteur1upRadioButton->isChecked() || ui->Secteur2upRadioButton->isChecked()) && db->parametres()->cotationsfrance());
    ui->NumCOlabel                  ->setVisible(m_medecin);
    ui->NumCOupLineEdit             ->setVisible(m_medecin);
    ui->TitreupcomboBox             ->setVisible(m_medecin);
    ui->Titrelabel                  ->setVisible(m_medecin);
    ui->AutreSoignantupLineEdit     ->setVisible(m_autresoignant);
    ui->MedecincheckBox             ->setVisible(m_autresoignant);
    ui->AutreSoignantupLineEdit     ->setVisible(ui->AutreSoignantupRadioButton->isChecked());
    ui->AutreFonctionuplineEdit     ->setVisible(ui->AutreFonctionupRadioButton->isChecked());

    ui->Comptawidget                ->setVisible(m_responsable || m_soccomptable);
    ui->ComptagroupBox              ->setVisible(m_responsable);
    ui->ComptaNoLiberalupRadioButton->setImmediateToolTip("");
    if (ui->ListUserstableWidget    ->currentItem()!=Q_NULLPTR)
    {
        bool u = ExisteEmployeur(ui->ListUserstableWidget->item(ui->ListUserstableWidget->currentRow(),0)->text().toInt());
        QString avert = (u? "" :
                         tr("Vous ne pouvez pas déclarer cet utilisateur en tant qu'employé\n"
                           "parce qu'il n'y a aucun utilisateur susceptible d'être employeur\n"
                           "enregistré dans la base."));
        ui->ComptaNoLiberalupRadioButton  ->setImmediateToolTip(avert);
        ui->ComptaNoLiberalupRadioButton  ->setEnabled(u);
    }
    ui->GererCompteuppushButton     ->setVisible(m_respliberal || m_soccomptable || m_respliberalSEL);
    ui->Employeurwidget             ->setVisible(m_respsalarie || m_respliberalSEL);
    ui->CompteComptawidget          ->setVisible(m_respliberal || m_soccomptable || m_respliberalSEL);
    ui->AGAupRadioButton            ->setVisible(m_respliberal || m_respliberalSEL);

    ui->Prenomlabel                 ->setVisible(!m_soccomptable && !m_neutre);
    ui->PrenomuplineEdit            ->setVisible(!m_soccomptable && !m_neutre);
    ui->AdressupTableWidget         ->setVisible(true);
    ui->GestLieuxpushButton         ->setVisible(true);

    if (m_respliberal || m_soccomptable || m_respliberalSEL)
        CalcListitemsCompteComptacomboBox(m_userencours, m_soccomptable);
    if (m_respsalarie || m_respliberalSEL)
    {
        User *usr = Datas::I()->users->getById(ui->idUseruplineEdit->text().toInt());
        if (usr)
        CalcListitemsEmployeurcomboBox(usr);
    }
    ui->OKupSmallButton->setEnabled(true);
}

void dlg_gestionusers::SupprUser()
{
    int idUser = m_userencours->id();
    if (db->StandardSelectSQL("select " CP_ID_USR " from " TBL_UTILISATEURS
                  " where " CP_ID_USR " <> " + QString::number(idUser) +
                  " and (" CP_SOIGNANTSTATUS_USR " = 1 or " CP_SOIGNANTSTATUS_USR " = 2 or " CP_SOIGNANTSTATUS_USR " = 3)"
                  " and (" CP_ENREGHONORAIRES_USR " = 1 or " CP_ENREGHONORAIRES_USR " = 2 or " CP_ENREGHONORAIRES_USR " = 4)", m_ok).size()==0)
    {
        UpMessageBox::Watch(this,tr("Impossible de supprimer ") + ui->ListUserstableWidget->selectedItems().at(1)->text() +
                                     tr(" parce que c'est le seul soignant enregistré dans la base."
                                     " Vous devez d'abord créer un autre utilisateur avant de pouvoir supprimer ")
                                     + ui->ListUserstableWidget->selectedItems().at(1)->text());
        return;
    }
    // si l'utilisateur est une société comptable ou s'il est employeur, on vérifie s'il a des employés et on bloque la suppression du compte si c'est le cas
    if (m_userencours->isSocComptable() || m_userencours->isLiberal())
        if (db->StandardSelectSQL("select " CP_ID_USR " from " TBL_UTILISATEURS " where " CP_IDEMPLOYEUR_USR " = " + QString::number(m_userencours->id()), m_ok).size()>0)
        {
            UpMessageBox::Watch(this, tr("Impossible de supprimer ce compte d'utilisateur!"), tr("cet utilisateur est enregistré comme employeur d'autres utilisateurs"));
            return;
        }
    UpMessageBox msgbox(this);
    UpPushButton OKBouton;
    UpPushButton AnnulBouton;
    QString vamourir = ui->ListUserstableWidget->selectedItems().at(1)->text();
    if (m_userencours == Datas::I()->users->userconnected())
    {
        msgbox.setText("Tentative de suicide");
        msgbox.setInformativeText("Hum " + vamourir
                                  + ", " + tr("êtes vous bien sûr de vouloir faire ça?\n"
                                    "Si vous le faites, le programme se fermera immédiatement après votre disparition"));
        msgbox.setIconPixmap(Icons::pxSuicide().scaledToWidth(150)); //WARNING : icon scaled : pxSuicide w150

        OKBouton.setText(tr("Non, vous avez raison, je vais rester encore un peu"));
        AnnulBouton.setText(tr("Oui, je veux partir"));
    }
    else
    {
        msgbox.setText(tr("Suppression d'un utilisateur"));
        msgbox.setInformativeText(tr("Etes vous bien sûr de vouloir supprimer ")
                                  + vamourir + "?");
        msgbox.setIconPixmap(Icons::pxKiller().scaledToWidth(150)); //WARNING : icon scaled : pxKiller w150
        OKBouton.setText(tr("Garder ") + vamourir);
        AnnulBouton.setText(tr("Oui, supprimer ") + vamourir);
    }
    msgbox.addButton(&AnnulBouton);
    msgbox.addButton(&OKBouton);
    msgbox.exec();
    if (msgbox.clickedpushbutton()==&AnnulBouton)
    {
        foreach (int idcpt, m_userencours->listecomptesbancaires(false))
        {
            QString icpt = QString::number(idcpt);
            if (db->StandardSelectSQL("select " CP_ID_LIGNRECETTES " from " TBL_RECETTES " where " CP_IDCPTEVIREMENT_LIGNRECETTES " = " + icpt, m_ok).size()==0)
                if (db->StandardSelectSQL("select " CP_ID_ARCHIVESCPT " from " TBL_ARCHIVESBANQUE " where " CP_IDCOMPTE_ARCHIVESCPT " = " + icpt, m_ok).size()==0)
                    if (db->StandardSelectSQL("select " CP_ID_DEPENSES " from " TBL_DEPENSES " where " CP_COMPTE_DEPENSES " = " + icpt, m_ok).size()==0)
                        if (db->StandardSelectSQL("select " CP_ID_REMCHEQ " from " TBL_REMISECHEQUES " where " CP_IDCOMPTE_REMCHEQ " = " + icpt, m_ok).size()==0)
                            if (db->StandardSelectSQL("select " CP_ID_LIGNCOMPTES " from " TBL_LIGNESCOMPTES " where " CP_IDCOMPTE_LIGNCOMPTES " = " + icpt, m_ok).size()==0)
                                Datas::I()->comptes->SupprimeCompte(Datas::I()->comptes->getById(idcpt));
        }
        db->SupprRecordFromTable(idUser, CP_IDUSER_COTATIONS , TBL_COTATIONS);
        db->StandardSQL("delete from " TBL_JOINTURESLIEUX " where iduser not in (select " CP_ID_USR " from " TBL_UTILISATEURS ")");

//        QString req = "select user, host from mysql.user where user like '" + ui->ListUserstableWidget->selectedItems().at(1)->text() + "%'";
//        QList<QVariantList> listusr = db->StandardSelectSQL(req, m_ok);
//        if (listusr.size()>0)
//            for (int i=0; i<listusr.size(); i++)
//                db->StandardSQL("drop user '" + listusr.at(i).at(0).toString() + "'@'" + listusr.at(i).at(1).toString() + "'");
        if (m_userencours == Datas::I()->users->userconnected())
        {
            UpMessageBox::Watch(this, tr("Cool ") + vamourir + "...", tr("Votre suicide s'est parfaitement déroulé et le programme va maintenant se fermer"));
            Datas::I()->users->SupprimeUser(m_userencours);
            exit(0);
        }
        Datas::I()->users->SupprimeUser(m_userencours);
        RemplirTableWidget(Datas::I()->users->userconnected()->id());
    }
}

void dlg_gestionusers::CalcListitemsCompteComptacomboBox(User *usr, bool soccomptable)
{
    QList<Compte*> listcomptes;
    foreach (Compte *cpt, *Datas::I()->comptes->comptes())
        if (cpt->idUser() == usr->id() && !cpt->isDesactive())
        {
            if (!soccomptable)
            {
                if (!cpt->isPartage())
                    listcomptes << cpt;
            }
            else
                listcomptes << cpt;
        }
    ui->CompteParDefautcomboBox->clear();
    foreach (Compte *cpt, listcomptes)
        ui->CompteParDefautcomboBox->insertItem(0, cpt->intitulecompte() + " - " + cpt->nomabrege(), cpt->id());
    if (usr->idcomptepardefaut() > 0)
        ui->CompteParDefautcomboBox->setCurrentIndex(ui->CompteParDefautcomboBox->findData(usr->idcomptepardefaut()));
}

void dlg_gestionusers::CalcListitemsEmployeurcomboBox(User *usr)
{
    ui->EmployeurcomboBox->clear();
    for (auto it = Datas::I()->users->comptablesActes()->begin(); it != Datas::I()->users->comptablesActes()->end(); ++it)
    {
        User* usrcpt = it.value();
        if (usrcpt)
            ui->EmployeurcomboBox->insertItem(0, usrcpt->grandnom(), usrcpt->id());
    }
    User* usremployer = Datas::I()->users->getById(usr->idemployeur());
    if (usremployer)
    {
        int idx = ui->EmployeurcomboBox->findData(usremployer->id());
        ui->EmployeurcomboBox->setCurrentIndex(idx);
    }
}

bool  dlg_gestionusers::AfficheParamUser(int idUser)
{
    if (db->StandardSelectSQL("select " CP_ID_USR " from " TBL_UTILISATEURS " where " CP_ID_USR " = " + QString::number(idUser), m_ok).size() == 0)
        return false;
    setDataCurrentUser(idUser);

    /* Valeurs de soignant
     * 1 = ophtalmo
     * 2 = Orthoptiste
     * 3 = autre soignant
     * 4 = non soignant
     * 5 = societe comptable
     */
    bool ophtalmo       = m_userencours->isOpthalmo();
    bool orthoptist     = m_userencours->isOrthoptist();
    bool autresoignant  = m_userencours->isAutreSoignant();
    bool soignant       = m_userencours->isSoignant();
    bool soccomptable   = m_userencours->isSocComptable();
    bool neutre         = m_userencours->isNeutre();
    bool medecin        = m_userencours->isMedecin();

    bool assistant      = m_userencours->isAssistant();
    bool responsable    = m_userencours->isResponsable();
    bool responsableles2= m_userencours->isResponsableOuAssistant();

    bool liberal        = m_userencours->isLiberal();
    bool liberalSEL     = m_userencours->isLiberalSEL();
    bool pasliberal     = m_userencours->isSoignantSalarie() || m_userencours->isLiberalSEL();
    bool remplacant     = m_userencours->isRemplacant();

    bool cotation       = m_userencours->useCotationsActes();

    ui->RPPSlabel                   ->setVisible(soignant && !assistant && db->parametres()->cotationsfrance());
    ui->RPPSupLineEdit              ->setVisible(soignant && !assistant && db->parametres()->cotationsfrance());
    ui->ModeExercicegroupBox        ->setVisible(soignant);
    ui->CotationupRadioButton       ->setVisible(soignant && !assistant && !remplacant);
    ui->NumCOlabel                  ->setVisible(medecin);
    ui->NumCOupLineEdit             ->setVisible(medecin);
    ui->SecteurgroupBox             ->setVisible(medecin && !assistant && !remplacant && cotation && db->parametres()->cotationsfrance());
    ui->OPTAMupRadioButton          ->setVisible(medecin && !assistant && !remplacant && cotation && (m_userencours->secteurconventionnel() == 1 || m_userencours->secteurconventionnel() == 2) && db->parametres()->cotationsfrance());
    ui->TitreupcomboBox             ->setVisible(medecin);
    ui->Titrelabel                  ->setVisible(medecin);
    ui->AutreSoignantupLineEdit     ->setVisible(autresoignant);
    ui->MedecincheckBox             ->setVisible(autresoignant);
    ui->AutreFonctionuplineEdit     ->setVisible(m_userencours->isAutreFonction());

    ui->Comptawidget                ->setVisible((soignant && !assistant) || soccomptable);
    ui->ComptagroupBox              ->setVisible(soignant && !assistant);
    ui->ComptaNoLiberalupRadioButton->setEnabled(ExisteEmployeur(idUser));
    ui->GererCompteuppushButton     ->setVisible((soignant && !assistant && liberal) || soccomptable);
    ui->Employeurwidget             ->setVisible(m_userencours->isSoignantSalarie() || m_userencours->isLiberalSEL());
    ui->CompteComptawidget          ->setVisible((soignant && !assistant && liberal) || soccomptable);
    ui->AGAupRadioButton            ->setVisible(soignant && !assistant && liberal);

    ui->Prenomlabel                 ->setVisible(!soccomptable && !neutre);
    ui->PrenomuplineEdit            ->setVisible(!soccomptable && !neutre);

    if ((soignant && !assistant && (liberal || liberalSEL)) || soccomptable)
        CalcListitemsCompteComptacomboBox(m_userencours, soccomptable);
    if (m_userencours->isSoignantSalarie() || m_userencours->isLiberalSEL())
        CalcListitemsEmployeurcomboBox(m_userencours);

    ui->idUseruplineEdit            ->setText(QString::number(m_userencours->id()));
    ui->LoginuplineEdit             ->setText(m_userencours->login());
    ui->MDPuplineEdit               ->setText(m_userencours->password());
    ui->NomuplineEdit               ->setText(m_userencours->nom());
    ui->PrenomuplineEdit            ->setText(m_userencours->prenom());

    QList<QVariantList> listlieux = db->StandardSelectSQL("select idlieu from " TBL_JOINTURESLIEUX " where iduser = " + QString::number(idUser), m_ok);
    QList<int> idlieuxlist;
    for (int k=0; k< listlieux.size(); k++)
        idlieuxlist << listlieux.at(k).at(0).toInt();
    for (int i=0; i<ui->AdressupTableWidget->rowCount(); ++i)
    {
        UpRadioButton *butt = static_cast<UpRadioButton*>(ui->AdressupTableWidget->cellWidget(i,0));
        butt->setChecked(idlieuxlist.contains(butt->iD()));
    }
    ui->PortableuplineEdit          ->setText(m_userencours->portable());
    ui->MailuplineEdit              ->setText(m_userencours->mail());
    ui->RPPSupLineEdit              ->setText(QString::number(m_userencours->NumPS()));
    ui->NumCOupLineEdit             ->setText(m_userencours->numOrdre());
    ui->InactivUsercheckBox         ->setChecked(m_userencours->isDesactive());

    ui->CotationupRadioButton         ->setChecked(true);

    ui->ComptaLiberalupRadioButton    ->setChecked(liberal);
    ui->ComptaNoLiberalupRadioButton  ->setChecked(pasliberal);
    ui->ComptaRemplaupRadioButton     ->setChecked(remplacant);

    ui->AGAupRadioButton              ->setChecked(ophtalmo && m_userencours->isAGA());

    ui->ResponsableupRadioButton      ->setChecked(responsable);
    ui->ResponsableLes2upRadioButton  ->setChecked(responsableles2);
    ui->AssistantupRadioButton        ->setChecked(assistant);
    if (medecin)
    {
        ui->Secteur1upRadioButton   ->setChecked(false);
        ui->Secteur2upRadioButton   ->setChecked(false);
        ui->Secteur3upRadioButton   ->setChecked(false);
        if  (db->parametres()->cotationsfrance())
        {
            ui->RPPSupLineEdit      ->setText(QString::number(m_userencours->NumPS()));
            switch (m_userencours->secteurconventionnel()) {
            case 1:     ui->Secteur1upRadioButton         ->setChecked(true);     break;
            case 2:     ui->Secteur2upRadioButton         ->setChecked(true);     break;
            case 3:     ui->Secteur3upRadioButton         ->setChecked(true);     break;
            default:
                break;
            }
        }
        else
            ui->RPPSupLineEdit      ->clear();
        ui->TitreupcomboBox         ->setCurrentText(m_userencours->titre());
        ui->OPTAMupRadioButton      ->setChecked(m_userencours->isOPTAM());
    }
    if (ophtalmo)
    {
        ui->NumCOupLineEdit             ->setText(m_userencours->numOrdre());
        ui->OPHupRadioButton            ->setChecked(true);
        ui->AutreSoignantupLineEdit     ->clear();
        ui->AutreFonctionuplineEdit     ->clear();
    }
    else if (orthoptist)
    {
        ui->NumCOupLineEdit             ->clear();
        if  (db->parametres()->cotationsfrance())
            ui->RPPSupLineEdit          ->setText(QString::number(m_userencours->NumPS()));
        else
            ui->RPPSupLineEdit          ->clear();
        ui->OrthoptistupRadioButton     ->setChecked(true);
        ui->AutreSoignantupLineEdit     ->clear();
        ui->AutreFonctionuplineEdit     ->clear();
    }
    else if (autresoignant)
    {
        ui->NumCOupLineEdit             ->clear();
        if  (db->parametres()->cotationsfrance())
            ui->RPPSupLineEdit          ->setText(QString::number(m_userencours->NumPS()));
        else
            ui->RPPSupLineEdit          ->clear();
        ui->AutreSoignantupRadioButton  ->setChecked(true);
        ui->AutreSoignantupLineEdit     ->setText(m_userencours->fonction());
        ui->MedecincheckBox             ->setChecked(medecin);
        ui->AutreFonctionuplineEdit     ->clear();
    }
    else if (soccomptable)
    {
        ui->NumCOupLineEdit             ->clear();
        ui->RPPSupLineEdit              ->clear();
        ui->AutreSoignantupLineEdit     ->clear();
        ui->AutreFonctionuplineEdit     ->clear();
        ui->SocieteComptableupRadioButton  ->setChecked(true);
    }
    else if (neutre)
    {
        ui->NumCOupLineEdit             ->clear();
        ui->RPPSupLineEdit              ->clear();
        ui->AutreSoignantupLineEdit     ->clear();
        ui->AutreFonctionuplineEdit     ->clear();
        ui->NeutreupRadioButton         ->setChecked(true);
    }
    else if (!soignant)
    {
        ui->NumCOupLineEdit             ->clear();
        ui->RPPSupLineEdit              ->clear();
        if (m_userencours->isSecretaire())
        {
            ui->SecretaireupRadioButton   ->setChecked(true);
            ui->AutreFonctionuplineEdit ->clear();
        }
        else
        {
            ui->AutreFonctionupRadioButton  ->setChecked(true);
            ui->AutreFonctionuplineEdit     ->setText(m_userencours->fonction());
        }
        ui->AutreSoignantupLineEdit     ->setVisible(false);
        ui->AutreSoignantupLineEdit     ->clear();
    }
    else
    {
        ui->ComptagroupBox              ->setVisible(true);
        ui->ModeExercicegroupBox        ->setVisible(true);
        ui->AGAupRadioButton            ->setVisible(true);
        ui->AGAupRadioButton            ->setChecked(m_userencours->isAGA());
        ui->SecteurgroupBox             ->setVisible(db->parametres()->cotationsfrance());
        ui->RPPSlabel                   ->setVisible(db->parametres()->cotationsfrance());
        ui->RPPSupLineEdit              ->setVisible(db->parametres()->cotationsfrance());
        ui->AutreSoignantupLineEdit     ->setVisible(false);
        ui->AutreFonctionuplineEdit     ->setVisible(false);
    }
    wdg_buttonframe->wdg_moinsBouton->setEnabled(ui->ListUserstableWidget->findItems(QString::number(idUser),Qt::MatchExactly).at(0)->foreground() != m_color
                                                 && ui->ListUserstableWidget->rowCount()>1);
    RegleAffichage();
    return true;
}

void   dlg_gestionusers::DefinitLesVariables()
{
    m_ophtalmo       = ui->OPHupRadioButton                ->isChecked();
    m_orthoptist     = ui->OrthoptistupRadioButton         ->isChecked();
    m_autresoignant  = ui->AutreSoignantupRadioButton      ->isChecked();
    m_autrenonsoignant  = ui->AutreFonctionupRadioButton   ->isChecked();
    m_medecin        = (ui->MedecincheckBox->isChecked() && ui->AutreSoignantupRadioButton->isChecked())
                   || ui->OPHupRadioButton->isChecked();
    m_soccomptable   = ui->SocieteComptableupRadioButton   ->isChecked();
    m_neutre         = ui->NeutreupRadioButton             ->isChecked();

    m_assistant      = ui->AssistantupRadioButton          ->isChecked();

    m_liberal        = ui->ComptaLiberalupRadioButton      ->isChecked();
    m_liberalSEL     = ui->ComptaLiberalSELupRadioButton   ->isChecked();
    m_pasliberal     = ui->ComptaNoLiberalupRadioButton    ->isChecked();
    m_retrocession   = ui->ComptaRemplaupRadioButton       ->isChecked();

    m_cotation       = ui->CotationupRadioButton           ->isChecked();
    m_soignant       = (m_ophtalmo || m_orthoptist|| m_autresoignant);
    m_responsable    = m_soignant && !m_assistant;
    m_respsalarie    = m_responsable && m_pasliberal;
    m_respliberal    = m_responsable && m_liberal;
    m_respliberalSEL = m_responsable && m_liberalSEL;
    m_soignantnonremplacant = m_responsable && !m_retrocession;
}

bool dlg_gestionusers::ExisteEmployeur(int iduser)
{
    return (db->StandardSelectSQL("select " CP_ID_USR " from " TBL_UTILISATEURS
                      " where (((" CP_SOIGNANTSTATUS_USR " = 1 or " CP_SOIGNANTSTATUS_USR " = 2 or " CP_SOIGNANTSTATUS_USR " = 3) and " CP_ENREGHONORAIRES_USR " = 1) or " CP_SOIGNANTSTATUS_USR " = 5)"
                                                                                                                                                                                                 " and " CP_ID_USR " <> " + QString::number(iduser), m_ok).size()>0);
}

void dlg_gestionusers::Inactifs()
{
    auto calclistusers = [&] (QStandardItemModel *model)
    {
        QList<User*> listuser;
        for (int i = 0; i<model->rowCount(); ++i)
        {
            UpStandardItem * item = dynamic_cast<UpStandardItem*>(model->item(i));
            if (item->checkState() == Qt::Checked)
                listuser << dynamic_cast<User*>(item->item());
        }
        if (listuser.size() > 0)
        {
            for (auto it = listuser.constBegin(); it != listuser.constEnd();)
            {
                User *usr = const_cast<User*>(*it);
                ItemsList::update(usr, CP_ISDESACTIVE_USR, false);
                Datas::I()->users->recalcStatut(usr);
                ++it;
            }
        }
    };
    UpDialog *dlg_listinactifs = new UpDialog(this);
    dlg_listinactifs->setWindowModality(Qt::WindowModal);
    QTableView *wdg_bigtable = new QTableView(dlg_listinactifs);

    //! Remplissage de la table
    QStandardItemModel  *m_model = new QStandardItemModel;
    UpStandardItem      *pitem;
    QItemSelectionModel *m_selectionmodel = new QItemSelectionModel(m_model);

    pitem   = new UpStandardItem(tr("Utilisateurs"));
    pitem->setEditable(false);
    m_model->setHorizontalHeaderItem(0,pitem);
    for (auto itusr = Datas::I()->users->inactifs()->begin(); itusr != Datas::I()->users->inactifs()->end();)
    {
        pitem   = new UpStandardItem(itusr.value()->login(), itusr.value());
        pitem->setCheckState(Qt::Unchecked);
        pitem->setCheckable(true);
        m_model->appendRow(pitem);
        ++ itusr;
    }
    wdg_bigtable->setModel(m_model);
    wdg_bigtable->setSelectionModel(m_selectionmodel);

    QFontMetrics fm(qApp->font());
    int hauteurligne = int(fm.height()*1.1);
    for (int i=0; i<m_model->rowCount(); i++)
        wdg_bigtable->setRowHeight(i,hauteurligne);
    wdg_bigtable->horizontalHeader()->setFixedHeight(hauteurligne);
    wdg_bigtable->setColumnWidth(0,180);

    wdg_bigtable->verticalHeader()->setVisible(false);
    wdg_bigtable->setFocusPolicy(Qt::StrongFocus);
    wdg_bigtable->setSelectionMode(QAbstractItemView::SingleSelection);
    wdg_bigtable->setGridStyle(Qt::NoPen);
    wdg_bigtable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    wdg_bigtable->setColumnWidth(0,300);
    wdg_bigtable->setFixedWidth(wdg_bigtable->columnWidth(0)+2);

    wdg_bigtable->setMinimumHeight(hauteurligne*20);
    wdg_bigtable->setSizeIncrement(0,hauteurligne);
    wdg_bigtable->setMouseTracking(true);
    dlg_listinactifs->dlglayout()->insertWidget(0,wdg_bigtable);

    dlg_listinactifs->AjouteLayButtons();
    UpLabel *label = new UpLabel();
    label->setText(tr("Cochez les utilisateurs\nque vous souhaitez réactiver"));
    dlg_listinactifs->AjouteWidgetLayButtons(label, false);
    dlg_listinactifs->setFixedWidth(wdg_bigtable->width() + dlglayout()->contentsMargins().left()*2);

    dlg_listinactifs->setWindowTitle(tr("Utilisateurs inactifs"));

    connect(dlg_listinactifs->OKButton, &QPushButton::clicked, dlg_listinactifs, [=] {  calclistusers(m_model);
                                                                                        dlg_listinactifs->accept();
                                                                                        RemplirTableWidget(m_userencours->id()); });
    dlg_listinactifs->exec();
    delete dlg_listinactifs;
}

void dlg_gestionusers::setDataCurrentUser(int id)
{
    m_userencours = Datas::I()->users->getById(id, Item::Update);
    if (m_userencours == Q_NULLPTR)
        return;
    m_userencours->setlistecomptesbancaires(Datas::I()->comptes->initListeComptesByIdUser(id));
    Datas::I()->users->CalcCompteEncaissementActes(m_userencours);
}

void dlg_gestionusers::ReconstruitListeLieuxExercice()
{
    ui->AdressupTableWidget->clear();
    int             ColCount = 4;
    ui->AdressupTableWidget->setColumnCount(ColCount);
    ui->AdressupTableWidget->verticalHeader()->setVisible(false);
    ui->AdressupTableWidget->setGridStyle(Qt::NoPen);
    int li = 0;                                                                                   // Réglage de la largeur et du nombre des colonnes
    ui->AdressupTableWidget->setColumnWidth(li,20);                                               // checkbox
    li++;
    ui->AdressupTableWidget->setColumnWidth(li,180);                                              // nom du lieu
    li++;
    ui->AdressupTableWidget->setColumnWidth(li,140);                                              // ville
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
    upheader->reDim(0,0,3);

    QList<QVariantList> listadress = db->StandardSelectSQL("select " CP_ID_SITE ", " CP_NOM_SITE ", " CP_ADRESSE1_SITE ", " CP_ADRESSE2_SITE ", " CP_ADRESSE3_SITE ", "
                                                           CP_CODEPOSTAL_SITE ", " CP_VILLE_SITE ", " CP_TELEPHONE_SITE", " CP_COULEUR_SITE " from " TBL_LIEUXEXERCICE, m_ok);
    ui->AdressupTableWidget->setRowCount(listadress.size());
    for (int i=0; i< listadress.size(); i++)
    {
        QString data ("");
        if (listadress.at(i).at(1).toString()!="")
            data += listadress.at(i).at(1).toString();
        if (listadress.at(i).at(2).toString()!="")
            data += (data != ""? "\n" : "") + listadress.at(i).at(2).toString();
        if (listadress.at(i).at(3).toString()!="")
            data += (data != ""? "\n" : "") + listadress.at(i).at(3).toString();
        if (listadress.at(i).at(4).toString()!="")
            data += (data != ""? "\n" : "") + listadress.at(i).at(4).toString();
        if (listadress.at(i).at(5).toString()!="")
            data += (data != ""? "\n" : "") + listadress.at(i).at(5).toString();
        if (listadress.at(i).at(6).toString()!="")
            data += (data != ""? " " : "") + listadress.at(i).at(6).toString();
        if (listadress.at(i).at(7).toString()!="")
            data += (data != ""? "\nTel: " : "Tel: ") + listadress.at(i).at(7).toString();

        UpRadioButton *buttn = new UpRadioButton();
        buttn->setImmediateToolTip(data);
        buttn->setiD(listadress.at(i).at(0).toInt());
        buttn->setAutoExclusive(false);
        connect(buttn, &QPushButton::clicked, this, [=]
        {
            int idlieu=-1;
            db->SupprRecordFromTable(ui->idUseruplineEdit->text().toInt(), "idUser", TBL_JOINTURESLIEUX);
            for(int i=0; i< ui->AdressupTableWidget->rowCount(); i++)
            {
                UpRadioButton *butt = static_cast<UpRadioButton*>(ui->AdressupTableWidget->cellWidget(i,0));
                if (butt->isChecked())
                {
                    idlieu = butt->iD();
                    db->StandardSQL("insert into " TBL_JOINTURESLIEUX "(iduser, idlieu) values (" + ui->idUseruplineEdit->text() + ", " + QString::number(idlieu) + ")");
                }
            }
            ui->OKupSmallButton->setEnabled(true);
        });
        ui->AdressupTableWidget->setCellWidget(i,0,buttn);

        QTableWidgetItem *pitem1, *pitem2, *pitem3;
        pitem1 = new QTableWidgetItem();
        pitem2 = new QTableWidgetItem();
        pitem3 = new QTableWidgetItem();
        pitem1->setText(listadress.at(i).at(1).toString());
        pitem2->setText(listadress.at(i).at(6).toString());
        pitem3->setText(listadress.at(i).at(7).toString());
        ui->AdressupTableWidget->setItem(i,1,pitem1);
        ui->AdressupTableWidget->setItem(i,2,pitem2);
        ui->AdressupTableWidget->setItem(i,3,pitem3);
        pitem1->setToolTip(data);
        pitem2->setToolTip(data);
        pitem3->setToolTip(data);
        if (listadress.at(i).at(8).toString() != "")
        {
            pitem1->setTextColor(QColor("#" + listadress.at(i).at(8).toString()));
            pitem2->setTextColor(QColor("#" + listadress.at(i).at(8).toString()));
            pitem3->setTextColor(QColor("#" + listadress.at(i).at(8).toString()));
        }
        ui->AdressupTableWidget->setRowHeight(i,int(QFontMetrics(qApp->font()).height()*1.3));
    }
}

void dlg_gestionusers::RemplirTableWidget(int iduser)
{
    ui->ListUserstableWidget->disconnect();
    QTableWidgetItem *pitem0, *pitem1;
    QFontMetrics fm(qApp->font());
    ui->ListUserstableWidget->clearContents();
    ui->ListUserstableWidget->setColumnCount(2);
    ui->ListUserstableWidget->setColumnWidth(0,0);
    ui->ListUserstableWidget->setColumnWidth(1, ui->ListUserstableWidget->width()-2);
    ui->ListUserstableWidget->verticalHeader()->setVisible(false);
    ui->ListUserstableWidget->setHorizontalHeaderLabels(QStringList()<<""<<"Login");
    ui->ListUserstableWidget->setGridStyle(Qt::NoPen);
    QList<User*> usrlist;
    for (auto it = Datas::I()->users->actifs()->constBegin(); it != Datas::I()->users->actifs()->constEnd(); ++it)
    {
        User* usr = const_cast<User*>(it.value());
        if (usr->login() != NOM_ADMINISTRATEUR)
            usrlist << usr;
    }
    if (usrlist.size() == 0)
        return;
    ui->ListUserstableWidget->setRowCount(usrlist.size());
    ui->InactivUsercheckBox ->setVisible(usrlist.size() > 1);
    int i = 0;
    foreach (User *usr, usrlist)
    {
        pitem0 = new QTableWidgetItem;
        pitem1 = new QTableWidgetItem;
        QList<QVariantList> actlst = db->StandardSelectSQL("select count(" CP_ID_ACTES ") from " TBL_ACTES
                                                           " where " CP_IDUSER_ACTES " = " + QString::number(usr->id()) +
                                                           " or " CP_IDUSERCREATEUR_ACTES " = " + QString::number(usr->id()), m_ok);
        if (actlst.size()>0)
        {
            int nbactes = actlst.at(0).at(0).toInt();
            if (nbactes>0)
            {
                pitem0->setForeground(m_color);
                pitem1->setForeground(m_color);
            }
        }
        pitem0->setText(QString::number(usr->id()));
        pitem1->setText(usr->login());
        ui->ListUserstableWidget->setItem(i,0, pitem0);
        ui->ListUserstableWidget->setItem(i,1, pitem1);
        ui->ListUserstableWidget->setRowHeight(i, int(fm.height()*1.3));
        ++i;
    }
    connect(ui->ListUserstableWidget, &QTableWidget::currentItemChanged , this, [=](QTableWidgetItem *pitem) {AfficheParamUser(ui->ListUserstableWidget->item(pitem->row(),0)->text().toInt());});
    if (iduser < 0 || ui->ListUserstableWidget->findItems(QString::number(iduser), Qt::MatchExactly).size() == 0)
        ui->ListUserstableWidget->setCurrentItem(ui->ListUserstableWidget->item(0,1));
    else
        ui->ListUserstableWidget->setCurrentItem(ui->ListUserstableWidget->findItems(QString::number(iduser), Qt::MatchExactly).at(0));
}

bool dlg_gestionusers::VerifFiche()
{
    DefinitLesVariables();
    if (ui->LoginuplineEdit->text().isEmpty())
    {
        UpMessageBox::Watch(this,tr("Vous n'avez pas spécifié de login!"));
        this->ui->LoginuplineEdit->setFocus();
        return false;
    }
    if (ui->MDPuplineEdit->text().isEmpty())
    {
        UpMessageBox::Watch(this,tr("Vous n'avez pas spécifié de mot de passe!"));
        this->ui->MDPuplineEdit->setFocus();
        return false;
    }
    if ((ui->OPHupRadioButton->isChecked() || (ui->MedecincheckBox->isChecked() && ui->AutreSoignantupRadioButton->isChecked())) && ui->TitreupcomboBox->currentText() == "")
    {
        UpMessageBox::Watch(this,tr("Vous n'avez pas spécifié le titre!"));
        this->ui->NomuplineEdit->setFocus();
        return false;
    }
    if (ui->NomuplineEdit->text().isEmpty())
    {
        UpMessageBox::Watch(this,tr("Vous n'avez pas spécifié de nom!"));
        this->ui->NomuplineEdit->setFocus();
        return false;
    }
    if (ui->PrenomuplineEdit->text().isEmpty() && !ui->SocieteComptableupRadioButton->isChecked() && !ui->NeutreupRadioButton->isChecked())
    {
        UpMessageBox::Watch(this,tr("Vous n'avez pas spécifié de prénom!"));
        this->ui->PrenomuplineEdit->setFocus();
        return false;
    }
    QList<QRadioButton*> listbouton = ui->FonctiongroupBox->findChildren<QRadioButton*>();
    bool a = false;
    for (int i=0; i<listbouton.size();i++)
        if (listbouton.at(i)->isChecked())
        {
            a = true;
            break;
        }
    if (!a)
    {
        UpMessageBox::Watch(this,tr("Vous n'avez pas spécifié la fonction!"));
        return false;
    }
    QList<UpRadioButton*> listbut = ui->AdressupTableWidget->findChildren<UpRadioButton*>();
    a = false;
    for (int i=0; i<listbut.size();i++)
        if (listbut.at(i)->isChecked())
        {
            a = true;
            break;
        }
    if (!a)
    {
        UpMessageBox::Watch(this,tr("Vous n'avez spécifié aucun lieu d'exercice!"));
        return false;
    }

    if (m_soignant)
    {
        QList<QRadioButton*> listbouton = ui->ModeExercicegroupBox->findChildren<QRadioButton*>();
        bool a = false;
        for (int i=0; i<listbouton.size();i++)
            if (listbouton.at(i)->isChecked())
            {
                a = true;
                break;
            }
        if (!a)
        {
            UpMessageBox::Watch(this,tr("Vous n'avez pas spécifié le mode d'exercice!"));
            return false;
        }
    }
    if (m_responsable)
    {
        QList<QRadioButton*> listbouton = ui->ComptagroupBox->findChildren<QRadioButton*>();
        bool a = false;
        for (int i=0; i<listbouton.size();i++)
            if (listbouton.at(i)->isChecked())
            {
                a = true;
                break;
            }
        if (!a)
        {
            UpMessageBox::Watch(this,tr("Vous n'avez pas spécifié les renseignements comptables!"));
            return false;
        }
    }
    if (m_medecin && ui->NumCOupLineEdit->text().isEmpty() && db->parametres()->cotationsfrance())
    {
        UpMessageBox::Watch(this,tr("Vous n'avez pas spécifié le n° de l'Ordre!"));
        this->ui->NumCOupLineEdit->setFocus();
        return false;
    }
    if (ui->AutreFonctionupRadioButton->isChecked() && ui->AutreFonctionuplineEdit->text().isEmpty())
    {
        UpMessageBox::Watch(this,tr("Vous n'avez pas spécifié la fonction!"));
        this->ui->AutreFonctionuplineEdit->setFocus();
        return false;
    }
    if (m_responsable && ui->RPPSupLineEdit->text().isEmpty() && db->parametres()->cotationsfrance())
    {
        UpMessageBox::Watch(this,tr("Vous n'avez pas spécifié le RPPS!"));
        this->ui->RPPSupLineEdit->setFocus();
        return false;
    }
    if (m_medecin && m_cotation && db->parametres()->cotationsfrance() && !ui->ComptaRemplaupRadioButton->isChecked())
    {
        QList<QRadioButton*> listb = ui->SecteurgroupBox->findChildren<QRadioButton*>();
        bool a = false;
        for (int i=0; i<listb.size();i++)
            if (listb.at(i)->isChecked())
            {
                a = true;
                break;
            }
        if (!a)
        {
            UpMessageBox::Watch(this,tr("Vous n'avez pas spécifié le secteur conventionnel!"));
            return false;
        }
    }
    if (m_respliberal || m_soccomptable || m_respliberalSEL)
        if (ui->CompteParDefautcomboBox->currentIndex()==-1)
        {
            UpMessageBox::Watch(this,tr("Vous avez oublié de spécifier un compte bancaire pour l'enregistrement de la comptabilité!"));
            return false;
        }
    if (m_pasliberal)
        if (ui->EmployeurcomboBox->currentIndex()==-1)
        {
            UpMessageBox::Watch(this,tr("Vous avez oublié de spécifier un employeur pour cet utilisateur non libéral!"));
            return false;
        }
    return true;
}
