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

#include "dlg_gestionusers.h"
#include "ui_dlg_gestionusers.h"

dlg_gestionusers::dlg_gestionusers(int idUser, int idlieu, bool mdpverified, QWidget *parent) :
    UpDialog(QDir::homePath() + NOMFIC_INI, "PositionsFiches/PositionGestionUsers", parent),
    ui(new Ui::dlg_gestionusers)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

    db                      = DataBase::getInstance();
    gMode                   = Modifier;
    MDPverified             = mdpverified;

    gidLieu                 = idlieu;

    gNouvMDP        = "nouv";
    gAncMDP         = "anc";
    gConfirmMDP     = "confirm";

    gLoginupLineEdit        = "LoginupLineEdit";
    gMDPupLineEdit          = "MDPupLineEdit";
    gConfirmMDPupLineEdit   = "ConfirmMDPupLineEdit";
    gcolor                  = QBrush(QColor(Qt::magenta));

    AjouteLayButtons(UpDialog::ButtonClose);

    widgButtons = new WidgetButtonFrame(ui->ListUserstableWidget);
    widgButtons->AddButtons(WidgetButtonFrame::PlusButton | WidgetButtonFrame::ModifButton | WidgetButtonFrame::MoinsButton);

    QHBoxLayout *play = new QHBoxLayout;
    play        ->addWidget(widgButtons->widgButtonParent());
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

    ui->ModifMDPUserupLabel->setToolTip(tr("Modifier le mot de passe"));

    connect(ui->AnnulupSmallButton,             SIGNAL(clicked(bool)),                  this,   SLOT(Slot_Annulation()));
    connect(ui->OKupSmallButton,                SIGNAL(clicked(bool)),                  this,   SLOT(Slot_EnregistreUser()));
    connect(ui->OPHupRadioButton,               SIGNAL(clicked(bool)),                  this,   SLOT(Slot_RegleAffichage()));
    connect(ui->OrthoptistupRadioButton,        SIGNAL(clicked(bool)),                  this,   SLOT(Slot_RegleAffichage()));
    connect(ui->AutreSoignantupRadioButton,     SIGNAL(clicked(bool)),                  this,   SLOT(Slot_RegleAffichage()));
    connect(ui->AutreNonSoignantupRadioButton,  SIGNAL(clicked(bool)),                  this,   SLOT(Slot_RegleAffichage()));
    connect(ui->SecretaireupRadioButton,        SIGNAL(clicked(bool)),                  this,   SLOT(Slot_RegleAffichage()));
    connect(ui->ComptaLiberalupRadioButton,     SIGNAL(clicked(bool)),                  this,   SLOT(Slot_RegleAffichage()));
    connect(ui->ComptaNoLiberalupRadioButton,   SIGNAL(clicked(bool)),                  this,   SLOT(Slot_RegleAffichage()));
    connect(ui->ComptaRemplaupRadioButton,      SIGNAL(clicked(bool)),                  this,   SLOT(Slot_RegleAffichage()));
    connect(ui->NoComptaupRadioButton,          SIGNAL(clicked(bool)),                  this,   SLOT(Slot_RegleAffichage()));
    connect(ui->OPTAMupRadioButton,             SIGNAL(clicked(bool)),                  this,   SLOT(Slot_RegleAffichage()));
    connect(ui->ResponsableupRadioButton,       SIGNAL(clicked(bool)),                  this,   SLOT(Slot_RegleAffichage()));
    connect(ui->ResponsableLes2upRadioButton,   SIGNAL(clicked(bool)),                  this,   SLOT(Slot_RegleAffichage()));
    connect(ui->AssistantupRadioButton,         SIGNAL(clicked(bool)),                  this,   SLOT(Slot_RegleAffichage()));
    connect(ui->SocieteComptableupRadioButton,  SIGNAL(clicked(bool)),                  this,   SLOT(Slot_RegleAffichage()));
    connect(ui->GererCompteuppushButton,        SIGNAL(clicked(bool)),                  this,   SLOT(Slot_GestionComptes()));
    connect(ui->InactivUsercheckBox,            SIGNAL(clicked(bool)),                  this,   SLOT(Slot_EnableOKpushButton()));
    connect(ui->CotationupRadioButton,          SIGNAL(clicked(bool)),                  this,   SLOT(Slot_RegleAffichage()));
    connect(ui->AGAupRadioButton,               SIGNAL(clicked(bool)),                  this,   SLOT(Slot_RegleAffichage()));
    connect(ui->MedecincheckBox,                SIGNAL(clicked(bool)),                  this,   SLOT(Slot_RegleAffichage()));
    connect(ui->ModifMDPUserupLabel,            SIGNAL(clicked(int)),                   this,   SLOT(Slot_ModifMDP()));
    connect(ui->Secteur1upRadioButton,          SIGNAL(clicked(bool)),                  this,   SLOT(Slot_RegleAffichage()));
    connect(ui->Secteur2upRadioButton,          SIGNAL(clicked(bool)),                  this,   SLOT(Slot_RegleAffichage()));
    connect(ui->Secteur3upRadioButton,          SIGNAL(clicked(bool)),                  this,   SLOT(Slot_RegleAffichage()));
    connect(ui->GestLieuxpushButton,            SIGNAL(clicked(bool)),                  this,   SLOT(Slot_GestLieux()));

    connect(CloseButton,                        SIGNAL(clicked(bool)),                  this,   SLOT(Slot_FermeFiche()));
    connect(widgButtons,                        SIGNAL(choix(int)),                     this,   SLOT(Slot_ChoixButtonFrame(int)));
    QList<UpLineEdit*> listline  = findChildren<UpLineEdit*>();
    for (int i=0; i<listline.size(); i++)
        connect(listline.at(i),                 SIGNAL(textEdited(QString)),            this,   SLOT(Slot_EnableOKpushButton()));
    QList<UpComboBox*> listcombo     = findChildren<UpComboBox*>();
    for (int i=0; i<listcombo.size(); i++)
        connect(listcombo.at(i),                SIGNAL(currentIndexChanged(int)),       this,   SLOT(Slot_EnableOKpushButton()));
    QList<UpRadioButton*> listbutton = findChildren<UpRadioButton*>();
    for (int i=0; i<listbutton.size(); i++)
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
        connect(listbutton.at(i),               SIGNAL(clicked(bool)),                  this,   SLOT(Slot_EnableOKpushButton()));
    }

    RemplirTableWidget(idUser);
    gidUserDepart = idUser;

    Slot_RegleAffichage();

    dlglayout()->setSizeConstraint(QLayout::SetFixedSize);

    ui->NoComptaupRadioButton       ->setCheckable(false);
    ui->CotationupRadioButton       ->setChecked(true);
    ui->CotationupRadioButton       ->setEnabled(false);
    ui->CotationupRadioButton       ->setImmediateToolTip(tr("Fonction indisponible\npour le moment"));
    ui->NoComptaupRadioButton       ->setImmediateToolTip(tr("Fonction indisponible\npour le moment"));
    ui->OKupSmallButton             ->setEnabled(false);
}

dlg_gestionusers::~dlg_gestionusers()
{
    delete ui;
}


void dlg_gestionusers::setConfig(enum Mode Style)
{
    Mode = Style;
    switch (Style) {
    case PREMIERUSER:
        ui->SecretaireupRadioButton         ->setEnabled(false);
        ui->AutreNonSoignantupRadioButton   ->setEnabled(false);
        ui->AutreSoignantupRadioButton      ->setEnabled(false);
        ui->AutreSoignantupLineEdit         ->setVisible(false);
        ui->AutreFonctionuplineEdit         ->setVisible(false);
        ui->SocieteComptableupRadioButton   ->setEnabled(false);
        ui->AssistantupRadioButton          ->setEnabled(false);
        ui->ResponsableLes2upRadioButton    ->setEnabled(false);
        ui->ComptaLiberalupRadioButton      ->setChecked(true);
        ui->ComptaRemplaupRadioButton       ->setEnabled(false);
        ui->ComptaNoLiberalupRadioButton    ->setEnabled(false);
        ui->NoComptaupRadioButton           ->setEnabled(false);
        ui->InactivUsercheckBox             ->setVisible(false);
        ui->Principalframe                  ->setEnabled(true);
        ui->AnnulupSmallButton              ->setVisible(false);
        ui->TitreupcomboBox                 ->setCurrentIndex(0);
        ui->CotationupRadioButton           ->setChecked(true);
        ui->OPHupRadioButton                ->setChecked(true);
        ui->ComptaLiberalupRadioButton      ->setChecked(true);
        ui->ResponsableupRadioButton        ->setChecked(true);
        CloseButton                         ->setVisible(false);
        widgButtons->widgButtonParent()     ->setVisible(false);
        ui->ModifMDPUserupLabel             ->setVisible(false);
        ui->Principalframe                  ->setEnabled(true);
        Slot_RegleAffichage();
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
        widgButtons->widgButtonParent()     ->setVisible(false);
        break;
    default:
        ui->ModifMDPUserupLabel             ->setVisible(false);
        ui->Principalframe                  ->setEnabled(false);
        widgButtons                         ->setEnabled(true);
        ui->ListUserstableWidget            ->setEnabled(true);
        break;
    }
}

void dlg_gestionusers::Slot_Annulation()
{
    if (gMode == Creer)
    {
        db->SupprRecordFromTable(DataUser()->id(), "idUser", NOM_TABLE_UTILISATEURS);
        db->SupprRecordFromTable(DataUser()->id(), "idUser", NOM_TABLE_COMPTES);
        int b = -1;
        QList<QVariant> userdata = db->getFirstRecordFromStandardSelectSQL("select idUser from " NOM_TABLE_UTILISATEURS " where iduser = " + QString::number(gidUserDepart),ok);
        if (ok && userdata.size()>0)
            b = gidUserDepart;
        RemplirTableWidget(b);
        ui->Principalframe->setEnabled(false);
        widgButtons->setEnabled(true);
        ui->ListUserstableWidget->setEnabled(true);
        gMode = Modifier;
    }
    else
    {
        if (Mode == PREMIERUSER || Mode == MODIFUSER)
            reject();
        if (ui->ListUserstableWidget->selectedItems().size()>0)
        {
            int idUser = ui->ListUserstableWidget->item(ui->ListUserstableWidget->selectedItems().at(0)->row(),0)->text().toInt();
            AfficheParamUser(idUser);
        }
        else if (AfficheParamUser(gidUserDepart))
        {
            int row = ui->ListUserstableWidget->findItems(QString::number(gidUserDepart), Qt::MatchExactly).at(0)->row();
            ui->ListUserstableWidget->selectRow(row);
        }
        ui->Principalframe->setEnabled(false);
        widgButtons->setEnabled(true);
        ui->ListUserstableWidget->setEnabled(true);
    }
}

void dlg_gestionusers::Slot_ChoixButtonFrame(int i)
{
    switch (i) {
    case 1:
        CreerUser();
        break;
    case 0:
        ModifUser();
        break;
    case -1:
        SupprUser();
        break;
    default:
        break;
    }
}

void dlg_gestionusers::CreerUser()
{
    gAsk                        = new UpDialog(this);
    QVBoxLayout *lay            = new QVBoxLayout();
    UpLabel *label              = new UpLabel();
    UpLabel *label2             = new UpLabel();
    UpLabel *label3             = new UpLabel();
    UpLineEdit *Line            = new UpLineEdit();
    UpLineEdit *Line2           = new UpLineEdit();
    UpLineEdit *Line3           = new UpLineEdit();

    gAsk                        ->setModal(true);
    gAsk                        ->move(QPoint(x()+width()/2,y()+height()/2));
    gAsk                        ->setFixedSize(300,300);
    gAsk                        ->setWindowTitle("");

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

    gAsk                        ->AjouteLayButtons(UpDialog::ButtonOK);
    connect(gAsk->OKButton,     SIGNAL(clicked(bool)),  this,   SLOT(Slot_EnregistreNouvUser()));

    lay                         ->addWidget(label);
    lay                         ->addWidget(Line);
    lay                         ->addWidget(label2);
    lay                         ->addWidget(Line2);
    lay                         ->addWidget(label3);
    lay                         ->addWidget(Line3);
    lay                         ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding));
    lay->setContentsMargins(5,5,5,5);
    lay->setSpacing(5);

    gAsk->dlglayout()           ->insertLayout(0,lay);
    gAsk->dlglayout()           ->setSizeConstraint(QLayout::SetFixedSize);

    Line                        ->setFocus();
    gAsk->exec();
    delete gAsk;
}

void dlg_gestionusers::Slot_EnableOKpushButton()
{
    ui->OKupSmallButton->setEnabled(true);
}

void dlg_gestionusers::Slot_EnregistreNouvMDP()
{
    if (gAskMDP)
    {
        // Vérifier la cohérence
        QString anc, nouv, confirm;
        UpMessageBox msgbox;
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
        if (anc != OtherUser->getPassword())
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
        db->StandardSQL("update " NOM_TABLE_UTILISATEURS " set userMDP = '" + nouv + "' where idUser = " + ui->idUseruplineEdit->text());
        // Enregitrer le nouveau MDP de connexion à MySQL
        db->StandardSQL("set password = '" + nouv + "'");
        QString AdressIP;
        foreach (const QHostAddress &address, QNetworkInterface::allAddresses()) {
            if (address.protocol() == QAbstractSocket::IPv4Protocol && address != QHostAddress(QHostAddress::LocalHost))
                 AdressIP = address.toString();
        }
        QString Domaine;
        QStringList listIP = AdressIP.split(".");
        for (int i=0;i<listIP.size()-1;i++)
            Domaine += listIP.at(i) + ".";
        db->StandardSQL("set password for '" + OtherUser->getLogin() + "'@'" + Domaine + "%' = '" + nouv + "'");
        db->StandardSQL("set password for '" + OtherUser->getLogin() + "SSL'@'%' = '" + nouv + "'");
        ui->MDPuplineEdit->setText(nouv);
        OtherUser->setPassword(nouv);
        gAskMDP->done(0);
        msgbox.exec();
    }
}

void dlg_gestionusers::Slot_EnregistreUser()
{
    if (!VerifFiche()) return;
    QString titre = (ui->OPHupRadioButton->isChecked()?       "'" + Utils::correctquoteSQL(ui->TitreupcomboBox->currentText()) + "'" : "null");
    QString actif = (ui->InactivUsercheckBox->isChecked()?  "1" : "null");
    QString req = "update "         NOM_TABLE_UTILISATEURS
            " set userNom = '"      + Utils::correctquoteSQL(Utils::trimcapitilize(ui->NomuplineEdit->text()))        + "',\n"
            " userPrenom = "        + (ui->SocieteComptableupRadioButton->isChecked()? "null" : "'" + Utils::correctquoteSQL(Utils::trimcapitilize(ui->PrenomuplineEdit->text())) + "'") + ",\n"
            " UserPortable = '"     + ui->PortableuplineEdit->text()   + "',\n"
            " UserMail = '"         + ui->MailuplineEdit->text()       + "',\n"
            " UserPoliceEcran = '" POLICEPARDEFAUT "',\n"
            " UserPoliceAttribut = 'Regular',\n"
            " UserTitre = "         + titre + ",\n"
            " UserDesactive = "     + actif + ",\n";
    if (ui->OPHupRadioButton->isChecked())
    {
        req += " UserFonction = '" + tr("Médecin") + "',\n"
               " UserSpecialite = '" + tr("Ophtalmologiste") + "',\n"
               " UserNoSpecialite = 15,\n"
               " Soignant = 1,\n"
               " Medecin = 1,\n"
               " UserNumCO = '" + Utils::correctquoteSQL(ui->NumCOupLineEdit->text()) +"',\n "
               " UserNumPS = '" + Utils::correctquoteSQL(ui->RPPSupLineEdit->text()) +"',\n "
               " ResponsableActes = ";
        if (ui->ResponsableupRadioButton->isChecked())
            req += "1,\n";
        else if (ui->ResponsableLes2upRadioButton->isChecked())
            req += "2,\n";
        else if (ui->AssistantupRadioButton->isChecked())
            req += "3,\n";
        if (ui->AssistantupRadioButton->isChecked())
        {
            req += " Userdroits = '" OPHTAASSISTANT "', \n"
                   " IdCompteParDefaut = null,\n"
                   " idCompteEncaissHonoraires = null,\n"
                   " UserEmployeur = null,\n"
                   " UserEnregHonoraires = null,\n"
                   " USerAGA = null,\n";
        }
        else
        {
            if (ui->ComptaLiberalupRadioButton->isChecked())
            {
                req += " Userdroits = '" OPHTALIBERAL "', \n"
                       " IdCompteParDefaut = " + ui->CompteComptacomboBox->currentData().toString() + ",\n"
                       " idCompteEncaissHonoraires = " + ui->CompteActescomboBox->currentData().toString() + ",\n"
                       " UserEnregHonoraires = 1,\n"
                       " UserEmployeur = null,\n";
                if (ui->AGAupRadioButton->isChecked())
                    req += " USerAGA = 1,\n";
                else
                    req += " USerAGA = null,\n";
            }
            else if (ui->ComptaNoLiberalupRadioButton->isChecked())
                req += " Userdroits = '" OPHTASALARIE "', \n"
                       " IdCompteParDefaut = null,\n"
                       " idCompteEncaissHonoraires = null,\n"
                       " UserEmployeur = " + ui->EmployeurcomboBox->currentData().toString() + ",\n"
                       " UserEnregHonoraires = 2,\n"
                       " USerAGA = null,\n";
            else if (ui->ComptaRemplaupRadioButton->isChecked())
                req += " Userdroits = '" OPHTAREMPLACANT "', \n"
                       " IdCompteParDefaut = null,\n"
                       " idCompteEncaissHonoraires = null,\n"
                       " UserEnregHonoraires = 3,\n"
                       " UserEmployeur = null,\n"
                       " USerAGA = null,\n";
            else if (ui->NoComptaupRadioButton->isChecked())
                req += " Userdroits = '" OPHTANOCOMPTA "', \n"
                       " IdCompteParDefaut = null,\n"
                       " idCompteEncaissHonoraires = null,\n"
                       " UserEnregHonoraires = 4,\n"
                       " UserEmployeur = null,\n"
                       " USerAGA = null,\n";
        }
        req += ((ui->CotationupRadioButton->isVisible() && ui->CotationupRadioButton->isChecked())?   " UserCCAM = 1,\n" : " UserCCAM = null,\n");
    }
    else if (ui->OrthoptistupRadioButton->isChecked())
    {
        req += " UserFonction = '" + tr("Orthoptiste") + "',\n"
               " UserSpecialite = '" + tr("Orthoptiste") + "',\n"
               " UserNoSpecialite = null,\n"
               " Soignant = 2,\n"
               " Medecin = null,\n"
               " UserNumCO = null,\n "
               " UserNumPS = '" + Utils::correctquoteSQL(ui->RPPSupLineEdit->text()) +"',\n"
               " ResponsableActes = ";
        if (ui->ResponsableupRadioButton->isChecked())
            req += "1,\n";
        else if (ui->ResponsableLes2upRadioButton->isChecked())
            req += "2,\n";
        else if (ui->AssistantupRadioButton->isChecked())
            req += "3,\n";
        if (ui->AssistantupRadioButton->isChecked())
        {
            req += " Userdroits = '" ORTHOASSISTANT "', \n"
                   " IdCompteParDefaut = null,\n"
                   " idCompteEncaissHonoraires = null,\n"
                   " UserEnregHonoraires = null,\n"
                   " UserEmployeur = null,\n"
                   " USerAGA = null,\n";
        }
        else
        {
            if (ui->ComptaLiberalupRadioButton->isChecked())
            {
                req += " Userdroits = '" ORTHOLIBERAL "', \n"
                       " IdCompteParDefaut = " + ui->CompteComptacomboBox->currentData().toString() + ",\n"
                       " idCompteEncaissHonoraires = " + ui->CompteActescomboBox->currentData().toString() + ",\n"
                       " UserEnregHonoraires = 1,\n"
                       " UserEmployeur = null,\n";
                if (ui->AGAupRadioButton->isChecked())
                    req += " USerAGA = 1,\n";
                else
                    req += " USerAGA = null,\n";
            }
            else if (ui->ComptaNoLiberalupRadioButton->isChecked())
                req += " Userdroits = '" ORTHOSALARIE "', \n"
                       " IdCompteParDefaut = null,\n"
                       " idCompteEncaissHonoraires = null,\n"
                       " UserEmployeur = " + ui->EmployeurcomboBox->currentData().toString() + ",\n"
                       " UserEnregHonoraires = 2,\n"
                       " USerAGA = null,\n";
            else if (ui->ComptaRemplaupRadioButton->isChecked())
                req += " Userdroits = '" ORTHOREMPLACANT "', \n"
                       " IdCompteParDefaut = null,\n"
                       " idCompteEncaissHonoraires = null,\n"
                       " UserEnregHonoraires = 3,\n"
                       " UserEmployeur = null,\n"
                       " USerAGA = null,\n";
            else if (ui->NoComptaupRadioButton->isChecked())
                req += " Userdroits = '" ORTHONOCOMPTA "', \n"
                       " IdCompteParDefaut = null,\n"
                       " idCompteEncaissHonoraires = null,\n"
                       " UserEnregHonoraires = 4,\n"
                       " UserEmployeur = null,\n"
                       " USerAGA = null,\n";
        }
        req += ((ui->CotationupRadioButton->isVisible() && ui->CotationupRadioButton->isChecked())?   " UserCCAM = 1,\n" : " UserCCAM = null,\n");
    }
    else if (ui->AutreSoignantupRadioButton->isChecked())
    {
        req += " UserFonction = '" + Utils::correctquoteSQL(ui->AutreSoignantupLineEdit->text()) + "',\n"
               " UserSpecialite = '" + Utils::correctquoteSQL(ui->AutreSoignantupLineEdit->text()) + "',\n"
               " UserNoSpecialite = null,\n"
               " Soignant = 3,\n"
               " Medecin = " + (ui->MedecincheckBox->isChecked()? "1" : "null") + ",\n"
               " UserNumCO = " + (ui->MedecincheckBox->isChecked()? (ui->NumCOupLineEdit->text()==""? "null" : "'" + ui->NumCOupLineEdit->text() + "'") : "null") + ",\n "
               " UserNumPS = " + (ui->RPPSupLineEdit->text()==""? "null" : "'" + ui->RPPSupLineEdit->text() + "'") + ",\n"
               " ResponsableActes = ";
        if (ui->ResponsableupRadioButton->isChecked())
            req += "1,\n";
        else if (ui->ResponsableLes2upRadioButton->isChecked())
            req += "2,\n";
        else if (ui->AssistantupRadioButton->isChecked())
            req += "3,\n";
        if (ui->AssistantupRadioButton->isChecked())
        {
            req += " Userdroits = '" AUTRESOIGNANTASSISTANT "', \n"
                   " IdCompteParDefaut = null,\n"
                   " idCompteEncaissHonoraires = null,\n"
                   " UserEnregHonoraires = null,\n"
                   " UserEmployeur = null,\n"
                   " USerAGA = null,\n";
        }
        else
        {
            if (ui->ComptaLiberalupRadioButton->isChecked())
            {
                req += " Userdroits = '" AUTRESOIGNANTLIBERAL "', \n"
                       " IdCompteParDefaut = " + ui->CompteComptacomboBox->currentData().toString() + ",\n"
                       " idCompteEncaissHonoraires = " + ui->CompteActescomboBox->currentData().toString() + ",\n"
                       " UserEnregHonoraires = 1,\n"
                       " UserEmployeur = null,\n";
                if (ui->AGAupRadioButton->isChecked())
                    req += " USerAGA = 1,\n";
                else
                    req += " USerAGA = null,\n";
            }
            else if (ui->ComptaNoLiberalupRadioButton->isChecked())
                req += " Userdroits = '" AUTRESOIGNANTSALARIE "', \n"
                       " IdCompteParDefaut = null,\n"
                       " idCompteEncaissHonoraires = null,\n"
                       " UserEmployeur = " + ui->EmployeurcomboBox->currentData().toString() + ",\n"
                       " UserEnregHonoraires = 2,\n"
                       " USerAGA = null,\n";
            else if (ui->ComptaRemplaupRadioButton->isChecked())
                req += " Userdroits = '" AUTRESOIGNANTREMPLACANT "', \n"
                       " IdCompteParDefaut = null,\n"
                       " idCompteEncaissHonoraires = null,\n"
                       " UserEnregHonoraires = 3,\n"
                       " UserEmployeur = null,\n"
                       " USerAGA = null,\n";
            else if (ui->NoComptaupRadioButton->isChecked())
                req += " Userdroits = '" AUTRESOIGNANTNOCOMPTA "', \n"
                       " IdCompteParDefaut = null,\n"
                       " idCompteEncaissHonoraires = null,\n"
                       " UserEnregHonoraires = 4,\n"
                       " UserEmployeur = null,\n"
                       " USerAGA = null,\n";
        }
        req += ((ui->CotationupRadioButton->isVisible() && ui->CotationupRadioButton->isChecked())?   " UserCCAM = 1,\n" : " UserCCAM = null,\n");
    }
    else if (ui->AutreNonSoignantupRadioButton->isChecked())
        req += " UserFonction = '" + Utils::correctquoteSQL(ui->AutreFonctionuplineEdit->text()) + "',\n"
               " UserSpecialite = '" + Utils::correctquoteSQL(ui->AutreFonctionuplineEdit->text()) + "',\n"
               " UserNoSpecialite = null,\n"
               " Soignant = 4,\n"
               " Medecin = null,\n"
               " UserAGA = null,\n"
               " ResponsableActes = null,\n"
               " IdCompteParDefaut = null,\n"
               " idCompteEncaissHonoraires = null,\n"
               " UserCCAM = null,\n"
               " UserEnregHonoraires = null,\n"
               " UserEmployeur = null,\n"
               " UserNumCO = null,\n "
               " UserNumPS = null,\n "
               " Userdroits = '" AUTREFONCTION "',\n";
    else if (ui->SecretaireupRadioButton->isChecked())
        req += " UserFonction = '" + tr("Secrétaire") + "',\n"
               " UserSpecialite = '" + tr("Secrétaire médicale") + "',\n"
               " UserNoSpecialite = null,\n"
               " Soignant = 4,\n"
               " Medecin = null,\n"
               " UserAGA = null,\n"
               " ResponsableActes = null,\n"
               " IdCompteParDefaut = null,\n"
               " idCompteEncaissHonoraires = null,\n"
               " UserCCAM = null,\n"
               " UserEnregHonoraires = null,\n"
               " UserEmployeur = null,\n"
               " UserNumCO = null,\n "
               " UserNumPS = null,\n "
               " Userdroits = '" SECRETAIRE "',\n";
    else if (ui->SocieteComptableupRadioButton->isChecked())
        req += " UserFonction = '" + tr("Société") + "',\n"
               " UserSpecialite = '" + tr("Société") + "',\n"
               " UserNoSpecialite = null,\n"
               " Soignant = 5,\n"
               " Medecin = null,\n"
               " UserAGA = null,\n"
               " ResponsableActes = null,\n"
               " IdCompteParDefaut = " + ui->CompteComptacomboBox->currentData().toString() + ",\n"
               " idCompteEncaissHonoraires = null,\n"
               " UserCCAM = null,\n"
               " UserEnregHonoraires = null,\n"
               " UserEmployeur = null,\n"
               " UserNumCO = null,\n "
               " UserNumPS = null,\n "
               " Userdroits = '" SOCIETECOMPTABLE "',\n";
    if (ui->OPHupRadioButton->isChecked() && !ui->AssistantupRadioButton->isChecked() && !ui->ComptaRemplaupRadioButton->isChecked() && ui->CotationupRadioButton->isChecked())
    {
        QString secteur = "null";
        if (ui->Secteur1upRadioButton       ->isChecked())      secteur = "1";
        if (ui->Secteur2upRadioButton       ->isChecked())      secteur = "2";
        if (ui->Secteur3upRadioButton       ->isChecked())      secteur = "3";
        req += " UserSecteur = " + secteur + ",\n";
        QString Optam = ((ui->OPTAMupRadioButton->isChecked() && (ui->Secteur1upRadioButton->isChecked() || ui->Secteur2upRadioButton->isChecked()))? "1" : "null");
        req += " OPTAM = " + Optam + "\n";
    }
    else
    {
        req += " UserSecteur = null,\n";
        req += " OPTAM = null\n";
    }
    req +=  " where idUser = "      + ui->idUseruplineEdit->text();
    //Edit(req);
    db->StandardSQL(req);
    int idlieu=-1;
    db->SupprRecordFromTable(ui->idUseruplineEdit->text().toInt(), "idUser", NOM_TABLE_JOINTURESLIEUX);
    for(int i=0; i< ui->AdressupTableWidget->rowCount(); i++)
    {
        UpRadioButton *butt = static_cast<UpRadioButton*>(ui->AdressupTableWidget->cellWidget(i,0));
        if (butt->isChecked())
        {
            idlieu = butt->iD();
            db->StandardSQL("insert into " NOM_TABLE_JOINTURESLIEUX "(iduser, idlieu) values (" + ui->idUseruplineEdit->text() + ", " + QString::number(idlieu) + ")");
        }
    }

    req = "update " NOM_TABLE_COMPTES " set partage = ";
    db->StandardSQL(req + (ui->SocieteComptableupRadioButton->isChecked()? "1" : "null") + " where iduser = " +  ui->idUseruplineEdit->text());

    if (Mode==PREMIERUSER || Mode == MODIFUSER)
    {
        done(ui->idUseruplineEdit->text().toInt());
        return;
    }
    else if (gMode == Creer)
    {
        //2. On crée 3 comptes avec ce login et ce MDP: local en localhost, réseau en 192.168.1.% et distant en %-SSL et login avec SSL à la fin
        QString AdressIP, MasqueReseauLocal;
        foreach (const QHostAddress &address, QNetworkInterface::allAddresses()) {
            if (address.protocol() == QAbstractSocket::IPv4Protocol && address != QHostAddress(QHostAddress::LocalHost))
                 AdressIP = address.toString();
        }
        QStringList listIP = AdressIP.split(".");
        for (int i=0;i<listIP.size()-1;i++)
            MasqueReseauLocal += QString::number(listIP.at(i).toInt()) + ".";
        MasqueReseauLocal += "%";
        QString login = ui->LoginuplineEdit->text();
        QString MDP = ui->MDPuplineEdit->text();
        db->StandardSQL("create user '" + login + "'@'localhost' identified by '" + MDP + "'");
        db->StandardSQL("create user '" + login + "'@'" + MasqueReseauLocal + "' identified by '" + MDP + "'");
        db->StandardSQL("create user '" + login + "SSL'@'%' identified by '" + MDP + "' REQUIRE SSL");
        db->StandardSQL("grant all on *.* to '" + login + "'@'localhost' identified by '" + MDP + "' with grant option");
        db->StandardSQL("grant all on *.* to '" + login + "SSL'@'%' identified by '" + MDP + "' with grant option");
        db->StandardSQL("grant all on *.* to '" + login + "'@'" + MasqueReseauLocal + "' identified by '" + MDP + "' with grant option");
        gMode = Modifier;
        ui->Principalframe->setEnabled(false);
        widgButtons->setEnabled(true);
        ui->ListUserstableWidget->setEnabled(true);
        widgButtons->moinsBouton->setEnabled(true);
    }
    else
    {
        ui->Principalframe->setEnabled(false);
        widgButtons->setEnabled(true);
        ui->ListUserstableWidget->setEnabled(true);
        widgButtons->moinsBouton->setEnabled(ui->ListUserstableWidget->findItems(ui->idUseruplineEdit->text(),Qt::MatchExactly).at(0)->foreground() != gcolor);
    }
    ui->OKupSmallButton->setEnabled(false);
}

void dlg_gestionusers::Slot_EnregistreNouvUser()
{
    if (!gAsk) return;
    QString msg = "";
    UpLineEdit *Loginline       = gAsk->findChild<UpLineEdit*>(gLoginupLineEdit);
    UpLineEdit *MDPline         = gAsk->findChild<UpLineEdit*>(gMDPupLineEdit);
    UpLineEdit *ConfirmMDPline  = gAsk->findChild<UpLineEdit*>(gConfirmMDPupLineEdit);
    QString login               = Loginline->text();
    QString MDP                 = MDPline->text();

    if (Loginline->text() == "")
    {
        msg = tr("Vous avez oublié d'indiquer le login");
        Loginline->setFocus();
    }
    if (Loginline->text() == NOM_ADMINISTRATEURDOCS)
    {
        msg = tr("Ce login est réservé");
        Loginline->setFocus();
        QTimer::singleShot(0, Loginline, &QLineEdit::selectAll);
    }
    if (MDPline->text() == "")
    {
        msg = tr("Vous avez oublié d'indiquer le mot de passe");
        MDPline->setFocus();
    }
    if (!Utils::rgx_AlphaNumeric_5_15.exactMatch(MDPline->text()))
    {
        msg = tr("Le mot de passe n'est pas conforme.") + "\n" +
              tr("Au moins 5 caractères - uniquement des chifres ou des lettres - max. 5 caractères.");
        MDPline->setFocus();
    }
    if (MDPline->text() != ConfirmMDPline->text())
    {
        msg = tr("Les mots de passe ne correspondent pas");
        MDPline->setFocus();
    }
    if (ui->ListUserstableWidget->findItems(login,Qt::MatchFixedString).size()>0)
    {
        msg = tr("Ce login est déjà utilisé!");
        Loginline->setFocus();
    }
    if (msg != "")
    {
        QSound::play(NOM_ALARME);
        UpMessageBox::Watch(this,msg);
        return;
    }
    gAsk->accept();
    gMode                       = Creer;
    db->StandardSQL("insert into " NOM_TABLE_UTILISATEURS " (UserLogin, UserMDP) VALUES ('" + Utils::correctquoteSQL(login) + "', '" + Utils::correctquoteSQL(MDP) + "')");
    QString req = "select idUser from " NOM_TABLE_UTILISATEURS " where UserLogin = '" + login + "' and UserMDP = '" + MDP + "'";
    int idUser = db->getFirstRecordFromStandardSelectSQL(req,ok).at(0).toInt();
    RemplirTableWidget(idUser);
    widgButtons                     ->setEnabled(false);
    ui->ListUserstableWidget        ->setEnabled(false);
    ui->Principalframe              ->setEnabled(true);
    ui->ComptagroupBox              ->setEnabled(true);
    setDataUser(idUser);
    ui->OPHupRadioButton              ->setChecked(true);
    ui->ResponsableupRadioButton      ->setChecked(true);
    ui->ComptaLiberalupRadioButton    ->setChecked(true);
    ui->CotationupRadioButton         ->setChecked(true);
    ui->Secteur1upRadioButton         ->setChecked(true);
    Slot_RegleAffichage();
}

void dlg_gestionusers::Slot_FermeFiche()
{
    if (ui->OKupSmallButton->isEnabled())
    {
        UpMessageBox msgbox;
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
                emit ui->OKupSmallButton->click();
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
void dlg_gestionusers::Slot_GestionComptes()
{
    bool modif      = false;
    bool verifempl  = ui->Employeurwidget->isVisible();
    QString empl    = ui->EmployeurcomboBox->currentText();
    bool verifact   = ui->CompteActeswidget->isVisible();
    QString cptact  = ui->CompteActescomboBox->currentText();
    bool verifcpta  = ui->CompteComptawidget->isVisible();
    QString cptcpta = ui->CompteComptacomboBox->currentText();

    bool comptedesociete    = ui->SocieteComptableupRadioButton->isChecked();
    bool affichelesolde     = (DataUser()->id()==gidUserDepart);

    Dlg_GestComptes = new dlg_gestioncomptes(DataUser(), comptedesociete, affichelesolde, this);
    Dlg_GestComptes ->setWindowTitle(tr("Comptes bancaires de ") + DataUser()->getLogin());
    Dlg_GestComptes ->exec();
    if (verifempl)
        if (ui->EmployeurcomboBox->currentText() != empl)
            modif   = true;
    if (verifact)
    {
        CalcListitemsCompteActescomboBox(DataUser()->id());
        if (ui->CompteActescomboBox->currentText() != cptact)
            modif   = true;
    }
    if (verifcpta)
    {
        CalcListitemsCompteComptacomboBox(DataUser()->id(), ui->SocieteComptableupRadioButton->isChecked());
        if (ui->CompteComptacomboBox->currentText() != cptcpta)
            modif   = true;
    }
    ui->OKupSmallButton->setEnabled(modif);
}

bool dlg_gestionusers::isMDPverified()
{
    return MDPverified;
}

void dlg_gestionusers::ModifUser()
{
    ui->ListUserstableWidget        ->setEnabled(false);
    widgButtons                     ->setEnabled(false);
    ui->Principalframe              ->setEnabled(true);
    ui->ModeExercicegroupBox        ->setEnabled(true);
    ui->SecteurgroupBox             ->setEnabled(true);
    ui->OPTAMupRadioButton          ->setEnabled(true);
    ui->OKupSmallButton             ->setEnabled(false);
    ui->AGAupRadioButton            ->setEnabled(true);
    ui->CompteActeswidget           ->setEnabled(true);
    gMode                           = Modifier;
}

void dlg_gestionusers::Slot_GestLieux()
{
    MDPverified = Utils::VerifMDP(DataBase::getInstance()->getMDPAdmin(), tr("Saisissez le mot de passe Administrateur"), MDPverified );
    if (!MDPverified)
            return;
    dlg_GestionLieux *gestLieux = new dlg_GestionLieux();
    gestLieux->exec();
    ReconstruitListeLieuxExercice();
    delete gestLieux;
    int idUser = ui->ListUserstableWidget->item(ui->ListUserstableWidget->selectedItems().at(0)->row(),0)->text().toInt();
    QList<QList<QVariant>> listlieux = db->StandardSelectSQL("select idlieu from " NOM_TABLE_JOINTURESLIEUX " where iduser = " + QString::number(idUser), ok);
    QList<int> idlieuxlist;
    for (int k=0; k< listlieux.size(); k++)
        idlieuxlist << listlieux.at(k).at(0).toInt();
    for (int i=0; i<ui->AdressupTableWidget->rowCount(); ++i)
    {
        UpRadioButton *butt = static_cast<UpRadioButton*>(ui->AdressupTableWidget->cellWidget(i,0));
        butt->setChecked(idlieuxlist.contains(butt->iD()));
    }
}

void dlg_gestionusers::Slot_ModifMDP()
{
    gAskMDP    = new UpDialog(this);
    gAskMDP    ->setModal(true);
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

    gAskMDP->AjouteLayButtons(UpDialog::ButtonOK);
    QList <QWidget*> ListTab;
    ListTab << AncMDP << NouvMDP << ConfirmMDP << gAskMDP->OKButton;
    for (int i = 0; i<ListTab.size()-1 ; i++ )
        gAskMDP->setTabOrder(ListTab.at(i), ListTab.at(i+1));
    gAskMDP    ->setWindowTitle(tr("Mot de passe utilisateur"));
    connect(gAskMDP->OKButton,    SIGNAL(clicked(bool)), this, SLOT(Slot_EnregistreNouvMDP()));
    gAskMDP->dlglayout()->setSizeConstraint(QLayout::SetFixedSize);

    gAskMDP->exec();
}

void dlg_gestionusers::Slot_RegleAffichage()
{
    /* Valeurs de soignant
     * 1 = ophtalmo
     * 2 = Orthoptiste
     * 3 = autre
     * 4 = non soignant
     * 5 = societe comptable
     */
    DefinitLesVariables();

    ui->RPPSlabel                   ->setVisible(responsable);
    ui->RPPSupLineEdit              ->setVisible(responsable);
    ui->ModeExercicegroupBox        ->setVisible(soignant);
    ui->CotationupRadioButton       ->setVisible(soigntnonrplct);
    ui->SecteurgroupBox             ->setVisible(medecin && soigntnonrplct);
    ui->OPTAMupRadioButton          ->setVisible(medecin && soigntnonrplct && (ui->Secteur1upRadioButton->isChecked() || ui->Secteur2upRadioButton->isChecked()));
    ui->NumCOlabel                  ->setVisible(medecin);
    ui->NumCOupLineEdit             ->setVisible(medecin);
    ui->TitreupcomboBox             ->setVisible(medecin);
    ui->Titrelabel                  ->setVisible(medecin);
    ui->AutreSoignantupLineEdit     ->setVisible(autresoignant);
    ui->MedecincheckBox             ->setVisible(autresoignant);
    ui->AutreSoignantupLineEdit     ->setVisible(ui->AutreSoignantupRadioButton->isChecked());
    ui->AutreFonctionuplineEdit     ->setVisible(ui->AutreNonSoignantupRadioButton->isChecked());

    ui->Comptawidget                ->setVisible(responsable || soccomptable);
    ui->ComptagroupBox              ->setVisible(responsable);
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
    ui->GererCompteuppushButton     ->setVisible(respliberal || soccomptable);
    ui->Employeurwidget             ->setVisible(respsalarie);
    ui->CompteActeswidget           ->setVisible(respliberal);
    ui->CompteComptawidget          ->setVisible(respliberal || soccomptable);
    ui->AGAupRadioButton            ->setVisible(respliberal);

    ui->Prenomlabel                 ->setVisible(!soccomptable);
    ui->PrenomuplineEdit            ->setVisible(!soccomptable);

    if (respliberal || soccomptable)
        ActualiseRsgnmtBanque(soccomptable);
    if (respsalarie)
    {
        CalcListitemsEmployeurcomboBox(ui->idUseruplineEdit->text().toInt());
        ui->EmployeurcomboBox->setCurrentIndex(0);
    }
    Slot_EnableOKpushButton();
}

void dlg_gestionusers::SupprUser()
{
    int idUser = DataUser()->id();
    if (db->StandardSelectSQL("select iduser from " NOM_TABLE_UTILISATEURS
                  " where iduser <> " + QString::number(idUser) +
                  " and (Soignant = 1 or Soignant = 2 or Soignant = 3)"
                  " and (UserEnregHonoraires = 1 or UserEnregHonoraires = 2 or UserEnregHonoraires = 4)", ok).size()==0)
    {
        UpMessageBox::Watch(this,tr("Impossible de supprimer ") + ui->ListUserstableWidget->selectedItems().at(1)->text() +
                                     tr(" parce que c'est le seul soignant enregistré dans la base."
                                     " Vous devez d'abord créer un autre utilisateur avant de pouvoir supprimer ")
                                     + ui->ListUserstableWidget->selectedItems().at(1)->text());
        return;
    }
    // si l'utilisateur est une société comptable ou s'il est employeur, on vérifie s'il a des employés et on bloque la suppression du compte si c'est le cas
    if (DataUser()->isSocComptable() || DataUser()->isLiberal())
        if (db->StandardSelectSQL("select iduser from " NOM_TABLE_UTILISATEURS " where UserEmployeur = " + QString::number(DataUser()->id()), ok).size()>0)
        {
            UpMessageBox::Watch(this, tr("Impossible de supprimer ce compte d'utilisateur!"), tr("cet utilisateur est enregistré comme employeur d'autres utilisateurs"));
            return;
        }
    UpMessageBox msgbox;
    UpPushButton OKBouton;
    UpPushButton AnnulBouton;
    QString vamourir = ui->ListUserstableWidget->selectedItems().at(1)->text();
    if (idUser == gidUserDepart)
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
        QList<QList<QVariant>> listcpt = db->StandardSelectSQL("select idcompte from " NOM_TABLE_COMPTES " where iduser = " + QString::number(idUser), ok);
        if(listcpt.size()>0)
        {
            for (int i=0; i<listcpt.size();i++)
            {
                QString icpt = listcpt.at(i).at(0).toString();
                if (db->StandardSelectSQL("select idrecette from " NOM_TABLE_RECETTES " where comptevirement = " + icpt, ok).size()==0)
                    if (db->StandardSelectSQL("select idligne from " NOM_TABLE_ARCHIVESBANQUE " where idcompte = " + icpt, ok).size()==0)
                        if (db->StandardSelectSQL("select iddep from " NOM_TABLE_DEPENSES " where compte = " + icpt, ok).size()==0)
                            if (db->StandardSelectSQL("select idremcheq from " NOM_TABLE_REMISECHEQUES " where idcompte = " + icpt, ok).size()==0)
                                if (db->StandardSelectSQL("select idligne from " NOM_TABLE_LIGNESCOMPTES " where idcompte = " + icpt, ok).size()==0)
                                    db->SupprRecordFromTable(icpt.toInt(), "idcompte", NOM_TABLE_COMPTES);
            }
        }
        db->SupprRecordFromTable(idUser, "idUser", NOM_TABLE_COTATIONS);
        db->SupprRecordFromTable(idUser, "idUser", NOM_TABLE_UTILISATEURS);
        db->StandardSQL("delete from " NOM_TABLE_JOINTURESLIEUX " where iduser not in (select iduser from " NOM_TABLE_UTILISATEURS ")");

        QString req = "select user, host from mysql.user where user like '" + ui->ListUserstableWidget->selectedItems().at(1)->text() + "%'";
        QList<QList<QVariant>> listusr = db->StandardSelectSQL(req, ok);
        if (listusr.size()>0)
            for (int i=0; i<listusr.size(); i++)
                db->StandardSQL("drop user '" + listusr.at(i).at(0).toString() + "'@'" + listusr.at(i).at(1).toString() + "'");
        if (gidUserDepart == idUser)
        {
            UpMessageBox::Watch(this, tr("Cool ") + vamourir + "...", tr("Votre suicide s'est parfaitement déroulé et le programme va maintenant se fermer"));
            exit(0);
        }
        int b = (db->StandardSelectSQL("select idUser from " NOM_TABLE_UTILISATEURS " where iduser = " + QString::number(gidUserDepart), ok).size() == 0?
                     -1:
                     gidUserDepart);
        RemplirTableWidget(b);
    }
}

void dlg_gestionusers::ActualiseRsgnmtBanque(bool soccomptable)
{
    CalcListitemsCompteActescomboBox(DataUser()->id());
    CalcListitemsCompteComptacomboBox(DataUser()->id(), soccomptable);
}

void dlg_gestionusers::CalcListitemsCompteActescomboBox(int iduser)
{
    QString user = QString::number(iduser);
    QString req = "select idCompte, IntituleCompte, NomCompteAbrege from \n" NOM_TABLE_BANQUES " as ban, " NOM_TABLE_COMPTES " as comp\n"
                  " where ban.idbanque = comp.idbanque \n"
                  " and (iduser = " + user + " or partage = 1)"
                  " and desactive is null";
    QList<QList<QVariant>> listcpt = db->StandardSelectSQL(req, ok, tr("Impossible de retrouver les comptes de l'utilisateur"));
    if (!ok)
        return;
    ui->CompteActescomboBox->clear();
    for (int i=0; i<listcpt.size(); i++)
        ui->CompteActescomboBox->insertItem(0, listcpt.at(i).at(1).toString() + " - " + listcpt.at(i).at(2).toString(), listcpt.at(i).at(0).toInt());
    QList<QVariant> idcptlst = db->getFirstRecordFromStandardSelectSQL("select idCompteEncaissHonoraires from " NOM_TABLE_UTILISATEURS
                                                                       " where iduser = " + user + " and idCompteEncaissHonoraires is not null", ok);
    if (ok && idcptlst.size()>0)
        ui->CompteActescomboBox->setCurrentIndex(ui->CompteActescomboBox->findData(idcptlst.at(0)));
}

void dlg_gestionusers::CalcListitemsCompteComptacomboBox(int iduser, bool soccomptable)
{
    QString user = QString::number(iduser);
    QString req = "select idCompte, IntituleCompte, NomCompteAbrege from \n" NOM_TABLE_BANQUES " as ban, " NOM_TABLE_COMPTES " as comp\n"
                  " where ban.idbanque = comp.idbanque \n"
                  " and iduser = " + user +
                  " and desactive is null";
    if (!soccomptable)
        req +=    " and partage is null";
    QList<QList<QVariant>> listcpt = db->StandardSelectSQL(req, ok, tr("Impossible de retrouver les comptes de l'utilisateur"));
    if (!ok)
        return;
    ui->CompteComptacomboBox->clear();
    for (int i=0; i<listcpt.size(); i++)
        ui->CompteComptacomboBox->insertItem(0, listcpt.at(i).at(1).toString() + " - " + listcpt.at(i).at(2).toString(), listcpt.at(i).at(0).toInt());
    QList<QVariant> idcptlst = db->getFirstRecordFromStandardSelectSQL("select idcomptepardefaut from " NOM_TABLE_UTILISATEURS
                                                                       " where iduser = " + user + " and idcomptepardefaut is not null", ok);
    if (ok && idcptlst.size()>0)
        ui->CompteComptacomboBox->setCurrentIndex(ui->CompteComptacomboBox->findData(idcptlst.at(0)));
    else
        ui->CompteComptacomboBox->setCurrentIndex(0);
}

void dlg_gestionusers::CalcListitemsEmployeurcomboBox(int iduser)
{
    QString user = QString::number(iduser);
    QString req = "select iduser, UserPrenom, UserNom from " NOM_TABLE_UTILISATEURS
                  " where (Soignant = 5"
                  " or (soignant < 4 and UserEnregHonoraires = 1))"
                  " and iduser <> " + user +
                  " and userdesactive is null";
    //qDebug() << req;
    QList<QList<QVariant>> listusr = db->StandardSelectSQL(req, ok, tr("Impossible de retrouver la liste des employeurs"));
    if (!ok)
        return;
    ui->EmployeurcomboBox->clear();
    for (int i=0; i<listusr.size(); i++)
        ui->EmployeurcomboBox->insertItem(0, (listusr.at(i).at(1).toString() != ""? listusr.at(i).at(1).toString() + " " + listusr.at(i).at(2).toString() : listusr.at(i).at(2).toString()), listusr.at(i).at(0).toInt());
    QList<QVariant> idusrlst = db->getFirstRecordFromStandardSelectSQL("select UserEmployeur from " NOM_TABLE_UTILISATEURS " where iduser = " + user, ok);
    if (ok && idusrlst.size()>0)
        ui->EmployeurcomboBox->setCurrentIndex(ui->EmployeurcomboBox->findData(idusrlst.at(0)));
}

bool  dlg_gestionusers::AfficheParamUser(int idUser)
{
    QString req;
    if (db->StandardSelectSQL("select idUser from " NOM_TABLE_UTILISATEURS " where iduser = " + QString::number(idUser), ok).size() == 0)
        return false;
    setDataUser(idUser);

    /* Valeurs de soignant
     * 1 = ophtalmo
     * 2 = Orthoptiste
     * 3 = autre soignant
     * 4 = non soignant
     * 5 = societe comptable
     */
    bool ophtalmo       = DataUser()->isOpthalmo();
    bool orthoptist     = DataUser()->isOrthoptist();
    bool autresoignant  = DataUser()->isAutreSoignant();
    bool soignant       = DataUser()->isSoignant();
    bool soccomptable   = DataUser()->isSocComptable();
    bool medecin        = DataUser()->isMedecin();

    bool assistant      = DataUser()->isAssistant();
    bool responsable    = DataUser()->isResponsable();
    bool responsableles2= DataUser()->isResponsableEtAssistant();

    bool liberal        = DataUser()->isLiberal();
    bool pasliberal     = DataUser()->isSalarie();
    bool retrocession   = DataUser()->isRemplacant();
    bool pasdecompta    = DataUser()->isSansCompta();

    bool cotation       = DataUser()->isCotation();

    ui->RPPSlabel                   ->setVisible(soignant && !assistant);
    ui->RPPSupLineEdit              ->setVisible(soignant && !assistant);
    ui->ModeExercicegroupBox        ->setVisible(soignant);
    ui->CotationupRadioButton       ->setVisible(soignant && !assistant && !retrocession);
    ui->NumCOlabel                  ->setVisible(medecin);
    ui->NumCOupLineEdit             ->setVisible(medecin);
    ui->SecteurgroupBox             ->setVisible(ophtalmo && !assistant && !retrocession && cotation);
    ui->OPTAMupRadioButton          ->setVisible(ophtalmo && !assistant && !retrocession && cotation && (DataUser()->getSecteur() == 1 || DataUser()->getSecteur() == 2));
    ui->TitreupcomboBox             ->setVisible(medecin);
    ui->Titrelabel                  ->setVisible(medecin);
    ui->AutreSoignantupLineEdit     ->setVisible(autresoignant);
    ui->MedecincheckBox             ->setVisible(autresoignant);
    ui->AutreFonctionuplineEdit     ->setVisible(DataUser()->isAutreFonction());

    ui->Comptawidget                ->setVisible((soignant && !assistant) || soccomptable);
    ui->ComptagroupBox              ->setVisible(soignant && !assistant);
    ui->ComptaNoLiberalupRadioButton->setEnabled(ExisteEmployeur(idUser));
    ui->GererCompteuppushButton     ->setVisible((soignant && !assistant && liberal) || soccomptable);
    ui->Employeurwidget             ->setVisible(soignant && !assistant && pasliberal);
    ui->CompteActeswidget           ->setVisible(soignant && !assistant && liberal);
    ui->CompteComptawidget          ->setVisible((soignant && !assistant && liberal) || soccomptable);
    ui->AGAupRadioButton            ->setVisible(soignant && !assistant && liberal);

    ui->Prenomlabel                 ->setVisible(!soccomptable);
    ui->PrenomuplineEdit            ->setVisible(!soccomptable);

    if ((soignant && !assistant && liberal) || soccomptable)
        ActualiseRsgnmtBanque(soccomptable);
    if (soignant && !assistant && pasliberal)
        CalcListitemsEmployeurcomboBox(idUser);

    ui->idUseruplineEdit            ->setText(QString::number(DataUser()->id()));
    ui->LoginuplineEdit             ->setText(DataUser()->getLogin());
    ui->MDPuplineEdit               ->setText(DataUser()->getPassword());
    if (medecin)
        ui->TitreupcomboBox         ->setCurrentText(DataUser()->getTitre());
    ui->NomuplineEdit               ->setText(DataUser()->getNom());
    ui->PrenomuplineEdit            ->setText(DataUser()->getPrenom());

    QList<QList<QVariant>> listlieux = db->StandardSelectSQL("select idlieu from " NOM_TABLE_JOINTURESLIEUX " where iduser = " + QString::number(idUser), ok);
    QList<int> idlieuxlist;
    for (int k=0; k< listlieux.size(); k++)
        idlieuxlist << listlieux.at(k).at(0).toInt();
    for (int i=0; i<ui->AdressupTableWidget->rowCount(); ++i)
    {
        UpRadioButton *butt = static_cast<UpRadioButton*>(ui->AdressupTableWidget->cellWidget(i,0));
        butt->setChecked(idlieuxlist.contains(butt->iD()));
    }
    ui->PortableuplineEdit          ->setText(DataUser()->getPortable());
    ui->MailuplineEdit              ->setText(DataUser()->getMail());
    ui->RPPSupLineEdit              ->setText(QString::number(DataUser()->getNumPS()));
    ui->NumCOupLineEdit             ->setText(DataUser()->getNumCO());
    ui->InactivUsercheckBox         ->setChecked(DataUser()->isDesactive());

    ui->CotationupRadioButton         ->setChecked(true);
    //ui->CotationupRadioButton         ->setChecked(DataUser().isCotation());

    ui->ComptaLiberalupRadioButton    ->setChecked(liberal);
    ui->ComptaNoLiberalupRadioButton  ->setChecked(pasliberal);
    ui->ComptaRemplaupRadioButton     ->setChecked(retrocession);
    ui->NoComptaupRadioButton         ->setChecked(pasdecompta);

    ui->AGAupRadioButton              ->setChecked(ophtalmo && DataUser()->getAGA());

    ui->ResponsableupRadioButton      ->setChecked(responsable);
    ui->ResponsableLes2upRadioButton  ->setChecked(responsableles2);
    ui->AssistantupRadioButton        ->setChecked(assistant);

    if (ophtalmo)
    {
        ui->NumCOupLineEdit             ->setText(DataUser()->getNumCO());
        ui->RPPSupLineEdit              ->setText(QString::number(DataUser()->getNumPS()));
        ui->OPHupRadioButton            ->setChecked(true);
        ui->AutreSoignantupLineEdit     ->clear();
        ui->AutreFonctionuplineEdit     ->clear();
        switch (DataUser()->getSecteur()) {
        case 1:     ui->Secteur1upRadioButton         ->setChecked(true);     break;
        case 2:     ui->Secteur2upRadioButton         ->setChecked(true);     break;
        case 3:     ui->Secteur3upRadioButton         ->setChecked(true);     break;
        default:
            break;
        }
        ui->OPTAMupRadioButton->setChecked(DataUser()->isOPTAM());
    }
    else if (orthoptist)
    {
        ui->NumCOupLineEdit             ->clear();
        ui->RPPSupLineEdit              ->setText(QString::number(DataUser()->getNumPS()));
        ui->OrthoptistupRadioButton       ->setChecked(true);
        ui->AutreSoignantupLineEdit     ->clear();
        ui->AutreFonctionuplineEdit     ->clear();
    }
    else if (autresoignant)
    {
        ui->NumCOupLineEdit             ->clear();
        ui->RPPSupLineEdit              ->clear();
        ui->AutreSoignantupRadioButton  ->setChecked(true);
        ui->AutreSoignantupLineEdit     ->setText(DataUser()->getFonction());
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
    else if (!soignant)
    {
        ui->NumCOupLineEdit             ->clear();
        ui->RPPSupLineEdit              ->clear();
        if (DataUser()->isSecretaire())
        {
            ui->SecretaireupRadioButton   ->setChecked(true);
            ui->AutreFonctionuplineEdit ->clear();
        }
        else
        {
            ui->AutreNonSoignantupRadioButton ->setChecked(true);
            ui->AutreFonctionuplineEdit     ->setText(DataUser()->getFonction());
        }
        ui->AutreSoignantupLineEdit     ->setVisible(false);
        ui->AutreSoignantupLineEdit     ->clear();
    }
    else
    {
        ui->ComptagroupBox              ->setVisible(true);
        ui->ModeExercicegroupBox        ->setVisible(true);
        ui->AGAupRadioButton              ->setVisible(true);
        ui->AGAupRadioButton              ->setChecked(DataUser()->getAGA());
        ui->SecteurgroupBox             ->setVisible(true);
        ui->RPPSlabel                   ->setVisible(true);
        ui->RPPSupLineEdit              ->setVisible(true);
        ui->AutreSoignantupLineEdit     ->setVisible(false);
        ui->AutreFonctionuplineEdit     ->setVisible(false);
        ui->SecteurgroupBox             ->setVisible(true);
        if (DataUser()->isCotation())
        {
            switch (DataUser()->getSecteur()) {
            case 1:     ui->Secteur1upRadioButton       ->setChecked(true); break;
            case 2:     ui->Secteur2upRadioButton       ->setChecked(true); break;
            case 3:     ui->Secteur3upRadioButton       ->setChecked(true); break;
            default:    break;
            }
        }
    }
    widgButtons->moinsBouton->setEnabled(ui->ListUserstableWidget->findItems(QString::number(idUser),Qt::MatchExactly).at(0)->foreground() != gcolor);
    return true;
}

void   dlg_gestionusers::DefinitLesVariables()
{
    ophtalmo       = ui->OPHupRadioButton                ->isChecked();
    orthoptist     = ui->OrthoptistupRadioButton         ->isChecked();
    autresoignant  = ui->AutreSoignantupRadioButton      ->isChecked();
    medecin        = (ui->MedecincheckBox->isChecked() && ui->AutreSoignantupRadioButton->isChecked())
                   || ui->OPHupRadioButton->isChecked();
    soccomptable   = ui->SocieteComptableupRadioButton   ->isChecked();

    assistant      = ui->AssistantupRadioButton          ->isChecked();

    liberal        = ui->ComptaLiberalupRadioButton      ->isChecked();
    pasliberal     = ui->ComptaNoLiberalupRadioButton    ->isChecked();
    retrocession   = ui->ComptaRemplaupRadioButton       ->isChecked();

    cotation       = ui->CotationupRadioButton           ->isChecked();
    soignant       = (ophtalmo || orthoptist|| autresoignant);
    responsable    = soignant && !assistant;
    respsalarie    = responsable && pasliberal;
    respliberal    = responsable && liberal;
    soigntnonrplct = responsable && !retrocession;
}

bool dlg_gestionusers::ExisteEmployeur(int iduser)
{
    return (db->StandardSelectSQL("select iduser from " NOM_TABLE_UTILISATEURS
                      " where (((Soignant = 1 or Soignant = 2 or Soignant = 3) and UserEnregHonoraires = 1) or Soignant = 5)"
                      " and iduser <> " + QString::number(iduser), ok).size()>0);
}
bool dlg_gestionusers::setDataUser(int id)
{
    QJsonObject data = db->loadUserData(id); //TODO : !!! Chargement du lieu
    if( data.isEmpty() )
        return false;
    OtherUser = new User( data );
    return true;
}

User* dlg_gestionusers::DataUser()
{
    return OtherUser;
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

    QList<QList<QVariant>> listadress = db->StandardSelectSQL("select idLieu, NomLieu, LieuAdresse1, LieuAdresse2, LieuAdresse3,"
                                                              " LieuCodePostal, LieuVille, LieuTelephone from " NOM_TABLE_LIEUXEXERCICE, ok);
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
            db->SupprRecordFromTable(ui->idUseruplineEdit->text().toInt(), "idUser", NOM_TABLE_JOINTURESLIEUX);
            for(int i=0; i< ui->AdressupTableWidget->rowCount(); i++)
            {
                UpRadioButton *butt = static_cast<UpRadioButton*>(ui->AdressupTableWidget->cellWidget(i,0));
                if (butt->isChecked())
                {
                    idlieu = butt->iD();
                    db->StandardSQL("insert into " NOM_TABLE_JOINTURESLIEUX "(iduser, idlieu) values (" + ui->idUseruplineEdit->text() + ", " + QString::number(idlieu) + ")");
                }
            }
            Slot_EnableOKpushButton();
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
    QList<QList<QVariant>> usrlst = db->StandardSelectSQL("select IdUser, UserLogin from " NOM_TABLE_UTILISATEURS " where userlogin <> '" NOM_ADMINISTRATEURDOCS "'",ok);
    ui->ListUserstableWidget->setRowCount(usrlst.size());
    for (int i=0; i<usrlst.size(); i++)
    {
        pitem0 = new QTableWidgetItem;
        pitem1 = new QTableWidgetItem;
        QList<QList<QVariant>> actlst = db->StandardSelectSQL("select count(idActe) from Rufus.Actes where idUser = " + usrlst.at(i).at(0).toString() + " or creepar = " + usrlst.at(i).at(0).toString(), ok);
        if (actlst.size()>0)
        {
            int nbactes = actlst.at(0).at(0).toInt();
            if (nbactes>0)
            {
                pitem0->setForeground(gcolor);
                pitem1->setForeground(gcolor);
            }
        }
        pitem0->setText(usrlst.at(i).at(0).toString());
        pitem1->setText(usrlst.at(i).at(1).toString());
        ui->ListUserstableWidget->setItem(i,0, pitem0);
        ui->ListUserstableWidget->setItem(i,1, pitem1);
        ui->ListUserstableWidget->setRowHeight(i, int(fm.height()*1.3));
    }
    connect(ui->ListUserstableWidget, &QTableWidget::currentItemChanged , this, [=](QTableWidgetItem *pitem) {AfficheParamUser(ui->ListUserstableWidget->item(pitem->row(),0)->text().toInt());});
    if (iduser<0)
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
    if (ui->PrenomuplineEdit->text().isEmpty() && !ui->SocieteComptableupRadioButton->isChecked())
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

    if (soignant)
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
    if (responsable)
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
    if (medecin && ui->NumCOupLineEdit->text().isEmpty())
    {
        UpMessageBox::Watch(this,tr("Vous n'avez pas spécifié le n° de l'Ordre!"));
        this->ui->NumCOupLineEdit->setFocus();
        return false;
    }
    if (ui->AutreNonSoignantupRadioButton->isChecked() && ui->AutreFonctionuplineEdit->text().isEmpty())
    {
        UpMessageBox::Watch(this,tr("Vous n'avez pas spécifié la fonction!"));
        this->ui->AutreFonctionuplineEdit->setFocus();
        return false;
    }
    if (responsable && ui->RPPSupLineEdit->text().isEmpty())
    {
        UpMessageBox::Watch(this,tr("Vous n'avez pas spécifié le RPPS!"));
        this->ui->RPPSupLineEdit->setFocus();
        return false;
    }
    if (medecin && cotation)
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
    if (respliberal)
        if (ui->CompteActescomboBox->currentIndex()==-1)
        {
            UpMessageBox::Watch(this,tr("Vous avez oublié de spécifier un compte bancaire pour l'encaissement des honoraires!"));
            return false;
        }
    if (respliberal || soccomptable)
        if (ui->CompteComptacomboBox->currentIndex()==-1)
        {
            UpMessageBox::Watch(this,tr("Vous avez oublié de spécifier un compte bancaire pour l'enregistrement de la comptabilité!"));
            return false;
        }
    if (pasliberal)
        if (ui->EmployeurcomboBox->currentIndex()==-1)
        {
            UpMessageBox::Watch(this,tr("Vous avez oublié de spécifier un employeur pour cet utilisateur non libéral!"));
            return false;
        }
    return true;
}
