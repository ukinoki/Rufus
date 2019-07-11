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

#include "dlg_identificationpatient.h"
#include "icons.h"
#include "ui_dlg_identificationpatient.h"

dlg_identificationpatient::dlg_identificationpatient(Mode mode, Patient *pat, QWidget *parent) :
    UpDialog(QDir::homePath() + FILE_INI, "PositionsFiches/PositionIdentificationPatient", parent),
    ui(new Ui::dlg_identificationpatient)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

    proc                = Procedures::I();
    m_currentpatient    = pat;
    gMode               = mode;
    db                  = DataBase::I();
    ListeCorModifiee    = false;
    QVBoxLayout *vlay       = new QVBoxLayout;
    vlay                    ->setContentsMargins(0,10,0,10);
    vlay                    ->setSpacing(5);
    vlay                    ->insertWidget(0,ui->Createurlabel);
    vlay                    ->insertWidget(0,ui->idPatientlabel);
    vlay                    ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding, QSizePolicy::Expanding));

    dlglayout()             ->insertWidget(0,ui->Principalframe);
    VitaleButton            = new UpSmallButton();
    VitaleButton            ->setIcon(Icons::icVitale());
    VitaleButton            ->setFixedHeight(100);
    VitaleButton            ->setStyleSheet("qproperty-iconSize:120px 100px;");
    AjouteWidgetLayButtons(VitaleButton, true);
    AjouteLayButtons(UpDialog::ButtonCancel | UpDialog::ButtonOK);
    OKButton                ->setText(tr("Enregistrer"));
    CancelButton            ->setText(tr("Annuler"));
    setStageCount(2);
    buttonslayout()         ->insertLayout(0, vlay);
    dlglayout()             ->setSizeConstraint(QLayout::SetFixedSize);

    ui->DDNdateEdit     ->setDateRange(QDate::currentDate().addYears(-105),QDate::currentDate());
    QFont font = qApp->font();
    font.setBold(true);
    ui->PrenomlineEdit->setFont(font);
    ui->NomlineEdit->setFont(font);
    ui->DDNdateEdit->setFont(font);

    UpLineEdit *MGLineEdit = new UpLineEdit(this);
    MGLineEdit          ->setStyleSheet(
    "UpLineEdit {background-color:white; border-style: none;}"
    "UpLineEdit:focus {border-style:none;}");
    MGLineEdit          ->setValidator(new QRegExpValidator(Utils::rgx_rx,this));
    MGLineEdit          ->setMaxLength(90);
    ui->MGupComboBox    ->setLineEdit(MGLineEdit);
    ui->MGupComboBox    ->setMaxVisibleItems(15);
    ui->MGupComboBox    ->setContextMenuPolicy(Qt::CustomContextMenu);
    proc->ReconstruitComboCorrespondants(ui->MGupComboBox, Correspondants::QueLesGeneralistes);
    ui->MGupComboBox    ->setCurrentIndex(-1);

    VilleCPwidg         = new VilleCPWidget(Datas::I()->villes, ui->Principalframe);
    CPlineEdit          = VilleCPwidg->ui->CPlineEdit;
    VillelineEdit       = VilleCPwidg->ui->VillelineEdit;
    VilleCPwidg         ->move(10,254);
    connect(VilleCPwidg, &VilleCPWidget::villecpmodified, this, &dlg_identificationpatient::Slot_EnableOKpushButton);

    AfficheDossierAlOuverture();

    ui->NomlineEdit     ->setValidator(new QRegExpValidator(Utils::rgx_rx,this));
    ui->PrenomlineEdit  ->setValidator(new QRegExpValidator(Utils::rgx_rx,this));
    ui->MaillineEdit    ->setValidator(new QRegExpValidator(Utils::rgx_mail,this));
    ui->Adresse1lineEdit->setValidator(new QRegExpValidator(Utils::rgx_adresse,this));
    ui->Adresse2lineEdit->setValidator(new QRegExpValidator(Utils::rgx_adresse,this));
    ui->Adresse3lineEdit->setValidator(new QRegExpValidator(Utils::rgx_adresse,this));
    ui->TellineEdit     ->setValidator(new QRegExpValidator(Utils::rgx_telephone,this));
    ui->PortablelineEdit->setValidator(new QRegExpValidator(Utils::rgx_telephone,this));
    ui->NNIlineEdit     ->setValidator(new QRegExpValidator(Utils::rgx_NNI,this));

    QList <QWidget *> listtab;
    listtab << ui->NomlineEdit << ui->PrenomlineEdit << ui->DDNdateEdit << ui->MradioButton << ui->FradioButton << ui->Adresse1lineEdit << ui->Adresse2lineEdit << ui->Adresse3lineEdit << CPlineEdit << VillelineEdit
            << ui->TellineEdit << ui->PortablelineEdit << ui->MaillineEdit << ui->NNIlineEdit << ui->ProfessionlineEdit << ui->MGupComboBox << ui->CMUcheckBox << ui->ALDcheckBox;
    for (int i = 0; i<listtab.size()-1 ; i++ )
        setTabOrder(listtab.at(i), listtab.at(i+1));


    installEventFilter(this);

    ui->MGupComboBox        ->installEventFilter(this);
    ui->MaillineEdit        ->installEventFilter(this);
    m_flagcorrespondants    = Flags::I()->flagCorrespondants();
    gTimer                  = new QTimer(this);
    gTimer                  ->start(5000);
    connect (gTimer,                        &QTimer::timeout,                           this,   &dlg_identificationpatient::Slot_VerifMGFlag);

    ui->NomlineEdit->setFocus();

    OKButton    ->disconnect();
    CancelButton->disconnect();
    connect (OKButton,                      SIGNAL(clicked()),                          this,   SLOT (Slot_OKpushButtonClicked()));
    connect (CancelButton,                  SIGNAL(clicked()),                          this,   SLOT (Slot_AnnulpushButtonClicked()));
    connect (ui->ModifierDDNupPushButton,   SIGNAL(clicked()),                          this,   SLOT (Slot_ModifDDN()));

    connect (ui->NomlineEdit,               SIGNAL(editingFinished()),                  this,   SLOT (Slot_Majuscule()));
    connect (ui->PrenomlineEdit,            SIGNAL(editingFinished()),                  this,   SLOT (Slot_Majuscule()));
    connect (ui->Adresse1lineEdit,          SIGNAL(editingFinished()),                  this,   SLOT (Slot_Majuscule()));
    connect (ui->Adresse2lineEdit,          SIGNAL(editingFinished()),                  this,   SLOT (Slot_Majuscule()));
    connect (ui->Adresse3lineEdit,          SIGNAL(editingFinished()),                  this,   SLOT (Slot_Majuscule()));
    connect (ui->ProfessionlineEdit,        SIGNAL(editingFinished()),                  this,   SLOT (Slot_Majuscule()));
    connect (ui->DDNdateEdit,               SIGNAL(dateChanged(QDate)),                 this,   SLOT (Slot_EnableOKpushButton()));
    connect (ui->NomlineEdit,               SIGNAL(textEdited(QString)),                this,   SLOT (Slot_EnableOKpushButton()));
    connect (ui->PrenomlineEdit,            SIGNAL(textEdited(QString)),                this,   SLOT (Slot_EnableOKpushButton()));
    connect (ui->Adresse1lineEdit,          SIGNAL(textEdited(QString)),                this,   SLOT (Slot_EnableOKpushButton()));
    connect (ui->Adresse2lineEdit,          SIGNAL(textEdited(QString)),                this,   SLOT (Slot_EnableOKpushButton()));
    connect (ui->Adresse3lineEdit,          SIGNAL(textEdited(QString)),                this,   SLOT (Slot_EnableOKpushButton()));
    connect (ui->TellineEdit,               SIGNAL(textEdited(QString)),                this,   SLOT (Slot_EnableOKpushButton()));
    connect (ui->PortablelineEdit,          SIGNAL(textEdited(QString)),                this,   SLOT (Slot_EnableOKpushButton()));
    connect (ui->MaillineEdit,              SIGNAL(textEdited(QString)),                this,   SLOT (Slot_EnableOKpushButton()));
    connect (ui->NNIlineEdit,               SIGNAL(textEdited(QString)),                this,   SLOT (Slot_EnableOKpushButton()));
    connect (ui->ProfessionlineEdit,        SIGNAL(textEdited(QString)),                this,   SLOT (Slot_EnableOKpushButton()));
    connect (ui->MradioButton,              SIGNAL(clicked()),                          this,   SLOT (Slot_EnableOKpushButton()));
    connect (ui->FradioButton,              SIGNAL(clicked()),                          this,   SLOT (Slot_EnableOKpushButton()));
    connect (ui->ALDcheckBox,               SIGNAL(clicked()),                          this,   SLOT (Slot_EnableOKpushButton()));
    connect (ui->CMUcheckBox,               SIGNAL(clicked()),                          this,   SLOT (Slot_EnableOKpushButton()));
    connect (ui->MGupComboBox,              QOverload<int>::of(&QComboBox::activated),  this,   &dlg_identificationpatient::ChoixMG);
    connect (ui->MGupComboBox,              SIGNAL(currentTextChanged(QString)) ,       this,   SLOT (Slot_EnableOKpushButton()));
    connect (ui->MGupComboBox,              &QWidget::customContextMenuRequested,       this,   [=] {MenuContextuelMedecin();});

    OKButton  ->setEnabled(false);

    bool f = (QLocale().country() == QLocale::France
                || QLocale().country() == QLocale::FrenchGuiana
                || QLocale().country() == QLocale::FrenchPolynesia
                || QLocale().country() == QLocale::FrenchSouthernTerritories);
    ui->CMUcheckBox->setVisible(f);
    ui->ALDcheckBox->setVisible(f);
}

dlg_identificationpatient::~dlg_identificationpatient()
{
}

/*-----------------------------------------------------------------------------------------------------------------
-- Traitement des SLOTS / actions associees a chaque objet du formulaire  -----------------------------
-----------------------------------------------------------------------------------------------------------------*/
void dlg_identificationpatient::ChoixMG()
{
    OKButton->setEnabled(true);
    Datas::I()->patients->updateCorrespondant(m_currentpatient, DataBase::MG, Datas::I()->correspondants->getById(ui->MGupComboBox->currentData().toInt()));
}

void    dlg_identificationpatient::Slot_EnableOKpushButton()
{
    bool a  = ui->NomlineEdit->text() != ""
           && ui->PrenomlineEdit->text() != ""
           && (ui->MradioButton->isChecked() || ui->FradioButton->isChecked())
           && CPlineEdit->text() != ""
           && VillelineEdit->text() != "";
    OKButton->setEnabled(a);
    OKButton->setShortcut(a? QKeySequence("Meta+Return") : QKeySequence());
}

void dlg_identificationpatient::Slot_Majuscule()
{
    QLineEdit* Line = static_cast<QLineEdit*>(sender());
    Line->setText(Utils::trimcapitilize(Line->text()));
    OKButton->setEnabled(true);
}

void    dlg_identificationpatient::Slot_ModifDDN()
{
    ui->DDNdateEdit->setEnabled(true);
    ui->ModifierDDNupPushButton->setVisible(false);
}

void dlg_identificationpatient::Slot_VerifMGFlag()
{
    int flag = Flags::I()->flagCorrespondants();
    if (m_flagcorrespondants < flag)
    {
        m_flagcorrespondants = flag;
        // on reconstruit la liste des MG
        proc->ReconstruitComboCorrespondants(ui->MGupComboBox, Correspondants::QueLesGeneralistes);
        m_currentpatient = Datas::I()->patients->getById(m_currentpatient->id(), Item::LoadDetails);
        if (m_currentpatient->idmg() > 0 && ui->MGupComboBox->currentData().toInt() != m_currentpatient->idmg())
            ui->MGupComboBox->setCurrentIndex(ui->MGupComboBox->findData(m_currentpatient->idmg()));
        else
            ui->MGupComboBox->setCurrentIndex(-1);
    }
}

void    dlg_identificationpatient::Slot_OKpushButtonClicked()
{
    QString PatNom, PatPrenom, PatDDN, PatCreePar, PatCreeLe;
    PatNom      = Utils::correctquoteSQL(Utils::trimcapitilize(ui->NomlineEdit->text(),true));
    PatPrenom   = Utils::correctquoteSQL(Utils::trimcapitilize(ui->PrenomlineEdit->text(),true));
    PatDDN      = ui->DDNdateEdit->date().toString("yyyy-MM-dd");
    PatCreeLe   = QDateTime::currentDateTime().date().toString("yyyy-MM-dd");
    PatCreePar  = QString::number(db->getUserConnected()->id());

    if (CPlineEdit->text() == "" && VillelineEdit->text() == "")
    {
        UpMessageBox::Watch(this,tr("Vous n'avez indiqué ni la ville ni le code postal!"));
        CPlineEdit->setFocus();
        return;
    }

    if (CPlineEdit->text() == "" || VillelineEdit->text() == "")
    {
        if (CPlineEdit->text() == "")
        {
            UpMessageBox::Watch(this,tr("Il manque le code postal"));
            CPlineEdit->setFocus();
            return;
        }
        if (VillelineEdit->text() == "")
        {
            UpMessageBox::Watch(this,tr("Il manque le nom de la ville"));
            VillelineEdit->setFocus();
            return;
        }
    }
    QString Sexe ("");
    if (ui->MradioButton->isChecked()) Sexe = "M";
    if (ui->FradioButton->isChecked()) Sexe = "F";
    QString ALD = (ui->ALDcheckBox->isChecked()? "1" : "null");
    QString CMU = (ui->CMUcheckBox->isChecked()? "1" : "null");
    QString NNI = (!ui->NNIlineEdit->text().isEmpty()? ui->NNIlineEdit->text() : "null");

    if (gMode == Copie)
    {
        // A - On vérifie qu'une date de naissance a été enregistrée, différente de la date par défaut
        if (ui->DDNdateEdit->date() == QDate(2000,1,1))
        {
            UpMessageBox msgbox;
            UpSmallButton OKBouton(tr("Je confirme"));
            UpSmallButton NoBouton(tr("Annuler"));
            msgbox.setText("Euuhh... " + db->getUserConnected()->getLogin());
            msgbox.setInformativeText(tr("Confirmez vous la date de naissance?") + "\n" + ui->DDNdateEdit->date().toString(tr("d-MMM-yyyy")));
            msgbox.setIcon(UpMessageBox::Warning);
            msgbox.addButton(&NoBouton, UpSmallButton::CANCELBUTTON);
            msgbox.addButton(&OKBouton, UpSmallButton::STARTBUTTON);
            msgbox.exec();
            if (msgbox.clickedButton() != &OKBouton)
                return;
        }

        // B - On vérifie ensuite que la saisie est complète
        if (PatNom == "")
        {
            UpMessageBox::Watch(this,tr("Vous devez spécifier un nom!"));
            ui->NomlineEdit->setFocus();
            return;
        }
        if (PatPrenom == "")
        {
            UpMessageBox::Watch(this,tr("Vous devez spécifier un prénom!"));
            ui->PrenomlineEdit->setFocus();
            return;
        }
        if (Sexe == "")
        {
            UpMessageBox::Watch(this,tr("Vous devez spécifier le sexe!"));
            return;
        }

        // C - On vérifie ensuite si ce dossier existe déjà
        QString requete = "select idPat from " TBL_PATIENTS
                " where PatNom LIKE '" + PatNom + "%' and PatPrenom LIKE '" + PatPrenom + "%' and PatDDN = '" + PatDDN + "'";
        QVariantList patdata = db->getFirstRecordFromStandardSelectSQL(requete, ok,  tr("Impossible d'interroger la table des patients!"));
        if(!ok)
        {
            reject();
            return;
        }

        if (patdata.size() > 0)
        {
            UpMessageBox::Watch(this,tr("Ce dossier existe déjà!"));
            db->loadPatientById(patdata.at(0).toInt(), m_currentpatient, true);
            AfficheDossierAlOuverture();
            disconnect (OKButton, SIGNAL(clicked()), this, SLOT (Slot_OKpushButtonClicked()));
            connect (OKButton, SIGNAL(clicked(bool)),this,SLOT(Slot_AnnulpushButtonClicked()));
            QList<QLineEdit *> listline = findChildren<QLineEdit *>();
            for (int i = 0;i<listline.size();i++) listline.at(i)->setEnabled(false);
            ui->SexegroupBox->setEnabled(false);
            ui->ALDcheckBox->setEnabled(false);
            ui->CMUcheckBox->setEnabled(false);
            VitaleButton->setEnabled(false);
            return;
        }
        // D - le dossier n'existe pas, on le crée
        QHash<QString, QVariant> listbinds;
        listbinds[CP_NOM_PATIENTS]          = ui->NomlineEdit->text();
        listbinds[CP_PRENOM_PATIENTS]       = ui->PrenomlineEdit->text();
        listbinds[CP_DDN_PATIENTS]          = ui->DDNdateEdit->date().toString("yyyy-MM-dd");
        listbinds[CP_SEXE_PATIENTS]         = Sexe;
        m_nouveaupatient = Patients::CreationPatient(listbinds);
        if (m_nouveaupatient == Q_NULLPTR)
            reject();

        // Mise à jour de donneessocialespatients
        ItemsList::update(m_nouveaupatient, CP_ADRESSE1_DSP,    Utils::trimcapitilize(ui->Adresse1lineEdit->text()));
        ItemsList::update(m_nouveaupatient, CP_ADRESSE2_DSP,    Utils::trimcapitilize(ui->Adresse2lineEdit->text()));
        ItemsList::update(m_nouveaupatient, CP_ADRESSE3_DSP,    Utils::trimcapitilize(ui->Adresse3lineEdit->text()));
        ItemsList::update(m_nouveaupatient, CP_CODEPOSTAL_DSP,  CPlineEdit->text());
        ItemsList::update(m_nouveaupatient, CP_VILLE_DSP,       Utils::trimcapitilize(VillelineEdit->text().left(70)));
        ItemsList::update(m_nouveaupatient, CP_TELEPHONE_DSP,   ui->TellineEdit->text());
        ItemsList::update(m_nouveaupatient, CP_PORTABLE_DSP,    ui->PortablelineEdit->text());
        ItemsList::update(m_nouveaupatient, CP_MAIL_DSP,        ui->MaillineEdit->text());
        ItemsList::update(m_nouveaupatient, CP_PROFESSION_DSP,  ui->ProfessionlineEdit->text());
        ItemsList::update(m_nouveaupatient, CP_NNI_DSP,         ui->NNIlineEdit->text());
        ItemsList::update(m_nouveaupatient, CP_ALD_DSP,         ui->ALDcheckBox->isChecked());
        ItemsList::update(m_nouveaupatient, CP_CMU_DSP,         ui->CMUcheckBox->isChecked());
        ItemsList::update(m_nouveaupatient, CP_IDMG_RMP,        ui->MGupComboBox->currentData().toInt());
        accept();
    }
    else if (gMode == Modification)
    {
        // on vérifie si le dossier existe déjà avec les mêmes nom, prénom et DDN
        QString requete = "select idPat from " TBL_PATIENTS
                          " where PatNom LIKE '" + Utils::correctquoteSQL(Utils::trimcapitilize(ui->NomlineEdit->text())) + "%'"
                          " and PatPrenom LIKE '" + Utils::correctquoteSQL(Utils::trimcapitilize(ui->PrenomlineEdit->text())) + "%'"
                          " and PatDDN = '" + ui->DDNdateEdit->date().toString("yyyy-MM-dd") + "'" +
                          " and idpat <> " + QString::number(m_currentpatient->id());
        QVariantList patdata = db->getFirstRecordFromStandardSelectSQL(requete, ok, tr("Impossible d'interroger la table des patients!"));
        if(!ok)
            return;
        if (patdata.size() > 0)
        {
            if (patdata.at(0).toInt() != m_currentpatient->id())
            {
                UpMessageBox::Watch(this,tr("Ce dossier existe déjà!"));
                return;
            }
        }
        //1 - Mise à jour patients
        ItemsList::update(m_currentpatient, CP_NOM_PATIENTS,    ui->NomlineEdit->text());
        ItemsList::update(m_currentpatient, CP_PRENOM_PATIENTS, ui->PrenomlineEdit->text());
        ItemsList::update(m_currentpatient, CP_DDN_PATIENTS,    ui->DDNdateEdit->date());
        if (Sexe != "")
            ItemsList::update(m_currentpatient, CP_SEXE_PATIENTS, Sexe);

        // Mise à jour de donneessocialespatients
        ItemsList::update(m_currentpatient, CP_ADRESSE1_DSP,    Utils::trimcapitilize(ui->Adresse1lineEdit->text()));
        ItemsList::update(m_currentpatient, CP_ADRESSE2_DSP,    Utils::trimcapitilize(ui->Adresse2lineEdit->text()));
        ItemsList::update(m_currentpatient, CP_ADRESSE3_DSP,    Utils::trimcapitilize(ui->Adresse3lineEdit->text()));
        ItemsList::update(m_currentpatient, CP_CODEPOSTAL_DSP,  CPlineEdit->text());
        ItemsList::update(m_currentpatient, CP_VILLE_DSP,       Utils::trimcapitilize(VillelineEdit->text().left(70)));
        ItemsList::update(m_currentpatient, CP_TELEPHONE_DSP,   ui->TellineEdit->text());
        ItemsList::update(m_currentpatient, CP_PORTABLE_DSP,    ui->PortablelineEdit->text());
        ItemsList::update(m_currentpatient, CP_MAIL_DSP,        ui->MaillineEdit->text());
        ItemsList::update(m_currentpatient, CP_PROFESSION_DSP,  ui->ProfessionlineEdit->text());
        ItemsList::update(m_currentpatient, CP_NNI_DSP,         ui->NNIlineEdit->text());
        ItemsList::update(m_currentpatient, CP_ALD_DSP,         ui->ALDcheckBox->isChecked());
        ItemsList::update(m_currentpatient, CP_CMU_DSP,         ui->CMUcheckBox->isChecked());
        ItemsList::update(m_currentpatient, CP_IDMG_RMP,        ui->MGupComboBox->currentData().toInt());

        accept();
    }
    else if (gMode == Creation)
    {
        //1 - Mise à jour patients
        if (Sexe != "")
            ItemsList::update(m_currentpatient, CP_SEXE_PATIENTS, Sexe);
        //2 - Mise à jour de donneessocialespatients
        ItemsList::update(m_currentpatient, CP_ADRESSE1_DSP,    Utils::trimcapitilize(ui->Adresse1lineEdit->text()));
        ItemsList::update(m_currentpatient, CP_ADRESSE2_DSP,    Utils::trimcapitilize(ui->Adresse2lineEdit->text()));
        ItemsList::update(m_currentpatient, CP_ADRESSE3_DSP,    Utils::trimcapitilize(ui->Adresse3lineEdit->text()));
        ItemsList::update(m_currentpatient, CP_CODEPOSTAL_DSP,  CPlineEdit->text());
        ItemsList::update(m_currentpatient, CP_VILLE_DSP,       Utils::trimcapitilize(VillelineEdit->text().left(70)));
        ItemsList::update(m_currentpatient, CP_TELEPHONE_DSP,   ui->TellineEdit->text());
        ItemsList::update(m_currentpatient, CP_PORTABLE_DSP,    ui->PortablelineEdit->text());
        ItemsList::update(m_currentpatient, CP_MAIL_DSP,        ui->MaillineEdit->text());
        ItemsList::update(m_currentpatient, CP_PROFESSION_DSP,  ui->ProfessionlineEdit->text());
        ItemsList::update(m_currentpatient, CP_NNI_DSP,         ui->NNIlineEdit->text());
        ItemsList::update(m_currentpatient, CP_ALD_DSP,         ui->ALDcheckBox->isChecked());
        ItemsList::update(m_currentpatient, CP_CMU_DSP,         ui->CMUcheckBox->isChecked());
        ItemsList::update(m_currentpatient, CP_IDMG_RMP,        ui->MGupComboBox->currentData().toInt());
        accept();
    }
}

void dlg_identificationpatient::MenuContextuelMedecin()
{
    if (ui->MGupComboBox->findText(ui->MGupComboBox->currentText()) != -1)
    {
        gmenuContextuel = new QMenu(this);
        QAction *pAction_IdentPatient = gmenuContextuel->addAction(tr("Modifier les coordonnées de ce médecin"));
        connect (pAction_IdentPatient,      &QAction::triggered,    [=] {ModifCorrespondant();});

        // ouvrir le menu
        gmenuContextuel->exec(cursor().pos());
        delete gmenuContextuel;
    }
}

void dlg_identificationpatient::ModifCorrespondant()
{
    int idcor           = ui->MGupComboBox->currentData().toInt();
    bool onlydoctors = true;
    Dlg_IdentCorresp    = new dlg_identificationcorresp(dlg_identificationcorresp::Modification, onlydoctors, Datas::I()->correspondants->getById(idcor, Item::LoadDetails));
    if (Datas::I()->correspondants->getById(idcor)==Q_NULLPTR)
        Dlg_IdentCorresp->ui->NomlineEdit   ->setText(ui->MGupComboBox->currentText());
    else
        Dlg_IdentCorresp->ui->NomlineEdit   ->setText(Datas::I()->correspondants->getById(idcor)->nom());
    Dlg_IdentCorresp->ui->PrenomlineEdit->setFocus();
    Dlg_IdentCorresp->ui->MGradioButton ->setChecked(true);
    if (Dlg_IdentCorresp->exec()>0)
    {
        proc->ReconstruitComboCorrespondants(ui->MGupComboBox, Correspondants::QueLesGeneralistes);
        ui->MGupComboBox->setCurrentIndex(ui->MGupComboBox->findData(idcor));
        ListeCorModifiee = Dlg_IdentCorresp->identcorrespondantmodifiee();
    }
    delete Dlg_IdentCorresp;
}

void dlg_identificationpatient::Slot_AnnulpushButtonClicked()
{
    if (gMode == Creation)
    {
        UpMessageBox *msgbox = new UpMessageBox(this);
        UpSmallButton OKBouton(tr("Annuler la création"));
        UpSmallButton NoBouton(tr("Revenir à la fiche"));
        msgbox->setText("Euuhh... " + db->getUserConnected()->getLogin());
        msgbox->setInformativeText(tr("Annuler la création de ce dossier ?"));
        msgbox->setIcon(UpMessageBox::Warning);
        msgbox->addButton(&NoBouton, UpSmallButton::OUPSBUTTON);
        msgbox->addButton(&OKBouton, UpSmallButton::CLOSEBUTTON);
        msgbox->exec();
        if (msgbox->clickedButton() == &OKBouton)
            reject();
    }
    else
        reject();
}

/*-------------------------------------------------------------------------------------
 Interception du focusOut
-------------------------------------------------------------------------------------*/
bool dlg_identificationpatient::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::FocusOut)
    {
        if (obj == ui->MGupComboBox)
            MAJMG();
        else if (obj == ui->MaillineEdit)
        {
            ui->MaillineEdit->setText(Utils::trim(ui->MaillineEdit->text()));
            if (ui->MaillineEdit->text()!="")
                if (!Utils::rgx_mailexactmatch.exactMatch(ui->MaillineEdit->text()))
                {
                    UpMessageBox::Watch(this, tr("Adresse mail invalide"));
                    ui->MaillineEdit->setFocus();
                    return true;
                }
        }
    }
    if (event->type() == QEvent::KeyPress && !obj->inherits("QPushButton"))
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        // Return ou Enter - On va au Tab Suivant -----------------------------------------------------------------------------------------------------------------------------
        if ((keyEvent->key()==Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) && keyEvent->modifiers() == Qt::NoModifier)
            return QWidget::focusNextChild();
    }
    return QWidget::eventFilter(obj, event);
}


/*--------------------------------------------------------------------------------------------
-- Afficher les éléments de la tables Patients
--------------------------------------------------------------------------------------------*/
void dlg_identificationpatient::AfficheDossierAlOuverture()
{
    // pour decocher les 2 radiobutton sexe il faut d'abord leur retirer la propriété AutoExclusive
    ui->MradioButton->setAutoExclusive(false);
    ui->FradioButton->setAutoExclusive(false);
    ui->MradioButton->setChecked(false);
    ui->FradioButton->setChecked(false);
    ui->MradioButton->setAutoExclusive(true);
    ui->FradioButton->setAutoExclusive(true);
    if (gMode == Copie)                                             //!> le nouveau dossier n'est pas encore créé (il ne le sera que si la fche est validée), on se contente de recopier les données du patient passsé en paramètre dans la fiche
    {
        if (!m_currentpatient->isalloaded())
            m_currentpatient = Datas::I()->patients->getById(m_currentpatient->id(), Item::LoadDetails);
        ui->NomlineEdit->setText(m_currentpatient->nom());
        ui->Adresse1lineEdit->setText(m_currentpatient->adresse1());
        ui->Adresse2lineEdit->setText(m_currentpatient->adresse2());
        ui->Adresse3lineEdit->setText(m_currentpatient->adresse3());
        CPlineEdit->setText(m_currentpatient->codepostal());
        VillelineEdit->setText(m_currentpatient->ville());
        ui->TellineEdit->setText(m_currentpatient->telephone());
        ui->ModifierDDNupPushButton->setVisible(false);
        int e = ui->MGupComboBox->findData(m_currentpatient->idmg());
        if (e > -1)
            ui->MGupComboBox->setCurrentIndex(e);
        ui->idPatientlabel->setText("");
        ui->Createurlabel->setText("");
    }
    else if (gMode == Creation)                                     //!> le nouveau dossier est créé, on affiche les paramètres définis à sa création (nom, prénom, DDN, idcreateur, datecreation)
    {
        ui->NomlineEdit->setText(m_currentpatient->nom());
        ui->PrenomlineEdit->setText(m_currentpatient->prenom());
        ui->DDNdateEdit->setDate(m_currentpatient->datedenaissance());
        ui->NomlineEdit->setEnabled(false);
        ui->PrenomlineEdit->setEnabled(false);
        ui->DDNdateEdit->setEnabled(false);
        ui->idPatientlabel->setText(tr("Dossier n° ") + QString::number(m_currentpatient->id()));
        ui->Createurlabel->setText(tr("Créé le ") + m_currentpatient->datecreationdossier().toString(tr("d-M-yyyy")) + "\n" +
                                   tr("par ") + Datas::I()->users->getById(m_currentpatient->idcreateur())->getLogin());
        ui->Adresse1lineEdit->clear();
        ui->Adresse2lineEdit->clear();
        ui->Adresse3lineEdit->clear();
        QString CP = proc->getCodePostalParDefaut();
        CPlineEdit          ->completer()->setCurrentRow(VilleCPwidg->villes()->getListCodePostal().indexOf(CP)); // ce micmac est nécessaire à cause d'un bug de QCompleter en mode InLineCompletion
        // il faut synchroniser à la main le QCompleter et le QlineEdit au premier affichage
        CPlineEdit          ->setText(CP);
        VillelineEdit->setText(proc->getVilleParDefaut());
        ui->TellineEdit->clear();
        ui->PortablelineEdit->clear();
        ui->MaillineEdit->clear();
        ui->ProfessionlineEdit->clear();
        ui->ALDcheckBox->setChecked(false);
        ui->CMUcheckBox->setChecked(false);
    }
    else if (gMode == Modification)                                 //!> on ne crée pas de nouveau dossier, on affiche tous les paramètres connus du dossier
    {
        if (!m_currentpatient->isalloaded())
            m_currentpatient = Datas::I()->patients->getById(m_currentpatient->id(), Item::LoadDetails);
        ui->NomlineEdit->setText(m_currentpatient->nom());
        ui->PrenomlineEdit->setText(m_currentpatient->prenom());
        ui->DDNdateEdit->setDate(m_currentpatient->datedenaissance());
        ui->DDNdateEdit->setEnabled(false);
        ui->MradioButton->setChecked(m_currentpatient->sexe() == "M");
        ui->FradioButton->setChecked(m_currentpatient->sexe() == "F");
        ui->idPatientlabel->setText(tr("Dossier n° ") + QString::number(m_currentpatient->id()));
        ui->Createurlabel->setText(tr("Créé le ") + m_currentpatient->datecreationdossier().toString(tr("d-M-yyyy")) + "\n" +
                                   tr("par ") + Datas::I()->users->getById(m_currentpatient->idcreateur())->getLogin());
        ui->Adresse1lineEdit->setText(m_currentpatient->adresse1());
        ui->Adresse2lineEdit->setText(m_currentpatient->adresse2());
        ui->Adresse3lineEdit->setText(m_currentpatient->adresse3());
        QString CP;
        if (m_currentpatient->codepostal() == "")
            CP = proc->getCodePostalParDefaut();
        else
            CP = m_currentpatient->codepostal();
        CPlineEdit          ->completer()->setCurrentRow(VilleCPwidg->villes()->getListCodePostal().indexOf(CP)); // ce micmac est nécessaire à cause d'un bug de QCompleter en mode InLineCompletion
        // il faut synchroniser à la main le QCompleter et le QlineEdit au premier affichage

        CPlineEdit          ->setText(CP);
        if (m_currentpatient->ville() == "")
            VillelineEdit   ->setText(proc->getVilleParDefaut());
        else
            VillelineEdit   ->setText(m_currentpatient->ville());
        ui->TellineEdit     ->setText(m_currentpatient->telephone());
        ui->PortablelineEdit->setText(m_currentpatient->portable());
        ui->MaillineEdit    ->setText(m_currentpatient->mail());
        if (m_currentpatient->NNI() > 0)
            ui->NNIlineEdit->setText(QString::number(m_currentpatient->NNI()));
        ui->ProfessionlineEdit->setText(m_currentpatient->profession());
        ui->ALDcheckBox     ->setChecked(m_currentpatient->isald());
        ui->CMUcheckBox     ->setChecked(m_currentpatient->iscmu());
        int e = ui->MGupComboBox->findData(m_currentpatient->idmg());
        if (e > -1)
            ui->MGupComboBox->setCurrentIndex(e);
    }
}

// ------------------------------------------------------------------------------------------
// Enregistre un nouveau correpondant
// ------------------------------------------------------------------------------------------
int dlg_identificationpatient::EnregistreNouveauCorresp()
{
    int idcor = -1;
    bool onlydoctors = true;
    Dlg_IdentCorresp        = new dlg_identificationcorresp(dlg_identificationcorresp::Creation, onlydoctors);
    Dlg_IdentCorresp->ui->NomlineEdit->setText(ui->MGupComboBox->currentText());
    Dlg_IdentCorresp->ui->PrenomlineEdit->setFocus();
    Dlg_IdentCorresp->ui->MGradioButton->setChecked(true);
    if (Dlg_IdentCorresp->exec()>0)
    {
        ListeCorModifiee = Dlg_IdentCorresp->identcorrespondantmodifiee();
        idcor = Dlg_IdentCorresp->correspondantrenvoye()->id();
    }
    delete Dlg_IdentCorresp;
    return idcor;
}


bool dlg_identificationpatient::listecorrespondantsmodifiee()
{
    return  ListeCorModifiee;
}

Patient* dlg_identificationpatient::getPatient()
{
    return m_nouveaupatient;
}

// ------------------------------------------------------------------------------------------
// Enregistre ou met à jour les coordonnées du médecin traitant
// ------------------------------------------------------------------------------------------
void dlg_identificationpatient::MAJMG()
{
    QString anc = ui->MGupComboBox->getValeurAvant();
    QString nou = Utils::trimcapitilize(ui->MGupComboBox->currentText(),true);
    ui->MGupComboBox->setCurrentText(nou);
    if (anc != nou) OKButton->setEnabled(true);
    int i = ui->MGupComboBox->findText(nou, Qt::MatchExactly);
    if (-1 < i && i < ui->MGupComboBox->count())
        return;
    if (nou != anc)
    {
        if (nou != "")
        {
            UpMessageBox msgbox;
            UpSmallButton OKBouton(tr("Enregistrer le correspondant"));
            UpSmallButton NoBouton(tr("Annuler"));
            msgbox.setText("Euuhh... " + db->getUserConnected()->getLogin());
            msgbox.setInformativeText(tr("Correspondant inconnu! Souhaitez-vous l'enregistrer?"));
            msgbox.setIcon(UpMessageBox::Warning);
            msgbox.addButton(&NoBouton, UpSmallButton::CANCELBUTTON);
            msgbox.addButton(&OKBouton, UpSmallButton::STARTBUTTON);
            msgbox.exec();
            if (msgbox.clickedButton() == &OKBouton)
            {
                int idcor = EnregistreNouveauCorresp();
                if (idcor >= 0)
                {
                    m_flagcorrespondants = Flags::I()->flagCorrespondants();
                    proc->ReconstruitComboCorrespondants(ui->MGupComboBox, Correspondants::QueLesGeneralistes);
                    ui->MGupComboBox->setCurrentIndex(ui->MGupComboBox->findData(idcor));
                    Datas::I()->patients->updateCorrespondant(m_currentpatient, DataBase::MG, Datas::I()->correspondants->getById(ui->MGupComboBox->currentData().toInt()));
                }
                else
                    ui->MGupComboBox->setCurrentText(anc);
            }
            else
                ui->MGupComboBox->setCurrentIndex(ui->MGupComboBox->findText(anc));
            msgbox.close();
        }
        else if (ui->MGupComboBox->getValeurAvant() != "" && gMode != Copie)
            Datas::I()->patients->updateCorrespondant(m_currentpatient, DataBase::MG);
    }
}

