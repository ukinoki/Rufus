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

#include "dlg_identificationpatient.h"
#include "icons.h"
#include "ui_dlg_identificationpatient.h"

dlg_identificationpatient::dlg_identificationpatient(Mode mode, Patient *pat, QWidget *parent) :
    UpDialog(PATH_FILE_INI, "PositionsFiches/PositionIdentificationPatient", parent),
    ui(new Ui::dlg_identificationpatient)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    setWindowTitle(tr("Identification de") + " " + pat->prenom().toUpper() + " " + pat->nom().toUpper());

    m_currentpatient        = pat;
    m_mode                  = mode;
    QVBoxLayout *vlay       = new QVBoxLayout;
    vlay                    ->setContentsMargins(0,10,0,10);
    vlay                    ->setSpacing(5);
    vlay                    ->insertWidget(0,ui->Createurlabel);
    vlay                    ->insertWidget(0,ui->idPatientlabel);
    vlay                    ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding, QSizePolicy::Expanding));

    dlglayout()             ->insertWidget(0,ui->Principalframe);
    widg_vitalebutton       = new UpSmallButton();
    widg_vitalebutton       ->setIcon(Icons::icVitale());
    widg_vitalebutton       ->setFixedHeight(100);
    widg_vitalebutton       ->setStyleSheet("qproperty-iconSize:120px 100px;");
    AjouteWidgetLayButtons(widg_vitalebutton, true);
    AjouteLayButtons(UpDialog::ButtonCancel | UpDialog::ButtonOK);
    OKButton                ->setText(tr("Enregistrer"));
    CancelButton            ->setText(tr("Annuler"));
    setStageCount(2);
    buttonslayout()         ->insertLayout(0, vlay);
    dlglayout()             ->setSizeConstraint(QLayout::SetFixedSize);
    QDate                   m_currentdate = db->ServerDate();
    ui->DDNdateEdit     ->setDateRange(m_currentdate.addYears(-105),m_currentdate);
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
    Procedures::ReconstruitComboCorrespondants(ui->MGupComboBox, Correspondants::QueLesGeneralistes);
    ui->MGupComboBox    ->setCurrentIndex(-1);

    wdg_villeCP         = new VilleCPWidget(Datas::I()->villes, ui->Principalframe);
    wdg_CPlineedit          = wdg_villeCP->ui->CPlineEdit;
    wdg_villelineedit       = wdg_villeCP->ui->VillelineEdit;
    wdg_villeCP         ->move(10,254);
    connect(wdg_villeCP, &VilleCPWidget::villecpmodified, this, &dlg_identificationpatient::EnableOKpushButton);

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
    listtab << ui->NomlineEdit << ui->PrenomlineEdit << ui->DDNdateEdit
            << ui->MradioButton << ui->FradioButton << ui->Adresse1lineEdit
            << ui->Adresse2lineEdit << ui->Adresse3lineEdit;
    if (db->parametres()->villesfrance() == true)
            listtab << wdg_CPlineedit;
    listtab << wdg_villelineedit
            << ui->TellineEdit << ui->PortablelineEdit << ui->MaillineEdit
            << ui->NNIlineEdit << ui->ProfessionlineEdit << ui->MGupComboBox
            << ui->CMUcheckBox << ui->ALDcheckBox;
    for (int i = 0; i<listtab.size()-1 ; i++ )
        setTabOrder(listtab.at(i), listtab.at(i+1));

    installEventFilter(this);

    ui->MGupComboBox        ->installEventFilter(this);
    ui->MaillineEdit        ->installEventFilter(this);
    m_flagcorrespondants    = Flags::I()->flagCorrespondants();
    t_timer                  = new QTimer(this);
    t_timer                  ->start(5000);
    connect (t_timer,                        &QTimer::timeout,                           this,   &dlg_identificationpatient::VerifMGFlag);

    ui->NomlineEdit->setFocus();

    OKButton    ->disconnect();
    CancelButton->disconnect();
    connect (OKButton,                      &QPushButton::clicked,                      this,   &dlg_identificationpatient::OKpushButtonClicked);
    connect (CancelButton,                  &QPushButton::clicked,                      this,   &dlg_identificationpatient::AnnulpushButtonClicked);
    connect (ui->ModifierDDNupPushButton,   &QPushButton::clicked,                      this,   &dlg_identificationpatient::ModifDDN);

    connect (ui->NomlineEdit,               &UpLineEdit::TextModified,                  this,   [=] {Majuscule(ui->NomlineEdit);});
    connect (ui->PrenomlineEdit,            &UpLineEdit::TextModified,                  this,   [=] {Majuscule(ui->PrenomlineEdit);});
    connect (ui->Adresse1lineEdit,          &UpLineEdit::TextModified,                  this,   [=] {Majuscule(ui->Adresse1lineEdit);});
    connect (ui->Adresse2lineEdit,          &UpLineEdit::TextModified,                  this,   [=] {Majuscule(ui->Adresse2lineEdit);});
    connect (ui->Adresse3lineEdit,          &UpLineEdit::TextModified,                  this,   [=] {Majuscule(ui->Adresse3lineEdit);});
    connect (ui->ProfessionlineEdit,        &UpLineEdit::TextModified,                  this,   [=] {Majuscule(ui->ProfessionlineEdit);});
    connect (ui->DDNdateEdit,               &QDateEdit::dateChanged,                    this,   &dlg_identificationpatient::EnableOKpushButton);
    connect (ui->NomlineEdit,               &QLineEdit::textEdited,                     this,   &dlg_identificationpatient::EnableOKpushButton);
    connect (ui->PrenomlineEdit,            &QLineEdit::textEdited,                     this,   &dlg_identificationpatient::EnableOKpushButton);
    connect (ui->Adresse1lineEdit,          &QLineEdit::textEdited,                     this,   &dlg_identificationpatient::EnableOKpushButton);
    connect (ui->Adresse2lineEdit,          &QLineEdit::textEdited,                     this,   &dlg_identificationpatient::EnableOKpushButton);
    connect (ui->Adresse3lineEdit,          &QLineEdit::textEdited,                     this,   &dlg_identificationpatient::EnableOKpushButton);
    connect (ui->TellineEdit,               &QLineEdit::textEdited,                     this,   &dlg_identificationpatient::EnableOKpushButton);
    connect (ui->PortablelineEdit,          &QLineEdit::textEdited,                     this,   &dlg_identificationpatient::EnableOKpushButton);
    connect (ui->MaillineEdit,              &QLineEdit::textEdited,                     this,   &dlg_identificationpatient::EnableOKpushButton);
    connect (ui->NNIlineEdit,               &QLineEdit::textEdited,                     this,   &dlg_identificationpatient::EnableOKpushButton);
    connect (ui->ProfessionlineEdit,        &QLineEdit::textEdited,                     this,   &dlg_identificationpatient::EnableOKpushButton);
    connect (ui->MradioButton,              &QRadioButton::clicked,                     this,   &dlg_identificationpatient::EnableOKpushButton);
    connect (ui->FradioButton,              &QRadioButton::clicked,                     this,   &dlg_identificationpatient::EnableOKpushButton);
    connect (ui->ALDcheckBox,               &QRadioButton::clicked,                     this,   &dlg_identificationpatient::EnableOKpushButton);
    connect (ui->CMUcheckBox,               &QRadioButton::clicked,                     this,   &dlg_identificationpatient::EnableOKpushButton);
    connect (ui->MGupComboBox,              QOverload<int>::of(&QComboBox::activated),  this,   &dlg_identificationpatient::ChoixMG);
    connect (ui->MGupComboBox,              &QComboBox::currentTextChanged,             this,   &dlg_identificationpatient::EnableOKpushButton);
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

void dlg_identificationpatient::ChoixMG()
{
    OKButton->setEnabled(true);
    Datas::I()->patients->updateCorrespondant(m_currentpatient, Correspondant::MG, Datas::I()->correspondants->getById(ui->MGupComboBox->currentData().toInt()));
}

void    dlg_identificationpatient::EnableOKpushButton()
{
    if (OKButton->isEnabled())
        return;
    bool a  = ui->NomlineEdit->text() != ""
           && ui->PrenomlineEdit->text() != ""
           && (ui->MradioButton->isChecked() || ui->FradioButton->isChecked())
           && wdg_villeCP->isValid();
    if (!wdg_villeCP->RechercheCP())
        Majuscule(wdg_villelineedit);
    Majuscule(ui->NomlineEdit);
    Majuscule(ui->PrenomlineEdit);
    Majuscule(ui->Adresse1lineEdit);
    Majuscule(ui->Adresse2lineEdit);
    Majuscule(ui->Adresse3lineEdit);
    Majuscule(ui->ProfessionlineEdit);

    OKButton->setEnabled(a);
    OKButton->setShortcut(a? QKeySequence("Meta+Return") : QKeySequence());
}

void dlg_identificationpatient::Majuscule(QLineEdit* ledit)
{
    ledit->setText(Utils::trimcapitilize(ledit->text(),false));
    OKButton->setEnabled(true);
}

void    dlg_identificationpatient::ModifDDN()
{
    ui->DDNdateEdit->setEnabled(true);
    ui->ModifierDDNupPushButton->setVisible(false);
}

void dlg_identificationpatient::VerifMGFlag()
{
    int flag = Flags::I()->flagCorrespondants();
    if (m_flagcorrespondants < flag)
    {
        m_flagcorrespondants = flag;
        // on reconstruit la liste des MG
        Datas::I()->correspondants->initListe();
        Procedures::ReconstruitComboCorrespondants(ui->MGupComboBox, Correspondants::QueLesGeneralistes);
        Datas::I()->patients->loadAll(m_currentpatient, Item::Update);
        if (m_currentpatient->idmg() > 0 && ui->MGupComboBox->currentData().toInt() != m_currentpatient->idmg())
            ui->MGupComboBox->setCurrentIndex(ui->MGupComboBox->findData(m_currentpatient->idmg()));
        else
            ui->MGupComboBox->setCurrentIndex(-1);
    }
}

void    dlg_identificationpatient::OKpushButtonClicked()
{
    QString PatNom, PatPrenom, PatDDN, PatCreePar, PatCreeLe;
    PatNom      = Utils::correctquoteSQL(Utils::trimcapitilize(ui->NomlineEdit->text(),true));
    PatPrenom   = Utils::correctquoteSQL(Utils::trimcapitilize(ui->PrenomlineEdit->text(),true));
    PatDDN      = ui->DDNdateEdit->date().toString("yyyy-MM-dd");
    PatCreeLe   = QDateTime::currentDateTime().date().toString("yyyy-MM-dd");
    PatCreePar  = QString::number(Datas::I()->users->userconnected()->id());

    if (!wdg_villeCP->isValid(true))
        return;
    QString Sexe ("");
    if (ui->MradioButton->isChecked()) Sexe = "M";
    if (ui->FradioButton->isChecked()) Sexe = "F";

    if (m_mode == Copie)
    {
        // A - On vérifie qu'une date de naissance a été enregistrée, différente de la date par défaut
        if (ui->DDNdateEdit->date() == QDate(2000,1,1))
        {
            UpMessageBox msgbox(this);
            UpSmallButton OKBouton(tr("Je confirme"));
            UpSmallButton NoBouton(tr("Annuler"));
            msgbox.setText("Euuhh... " + Datas::I()->users->userconnected()->login());
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
        UpLineEdit* line = dynamic_cast<UpLineEdit*>(focusWidget());
        if (line != Q_NULLPTR)
            if (line == ui->PrenomlineEdit || line == ui->NomlineEdit || line == ui->Adresse1lineEdit || line == ui->Adresse2lineEdit || line == ui->Adresse3lineEdit || line == ui->ProfessionlineEdit)
                line->setText(Utils::trimcapitilize(line->text()));

        // C - On vérifie ensuite si ce dossier existe déjà
        QString requete = "select idPat from " TBL_PATIENTS
                " where PatNom LIKE '" + PatNom + "%' and PatPrenom LIKE '" + PatPrenom + "%' and PatDDN = '" + PatDDN + "'";
        QVariantList patdata = db->getFirstRecordFromStandardSelectSQL(requete, m_ok,  tr("Impossible d'interroger la table des patients!"));
        if(!m_ok)
        {
            reject();
            return;
        }

        if (patdata.size() > 0)
        {
            UpMessageBox::Watch(this,tr("Ce dossier existe déjà!"));
            Datas::I()->patients->loadAll(m_currentpatient, Item::Update);
            AfficheDossierAlOuverture();
            disconnect  (OKButton, &QPushButton::clicked,   this,   &dlg_identificationpatient::OKpushButtonClicked);
            connect     (OKButton, &QPushButton::clicked,   this,   &dlg_identificationpatient::AnnulpushButtonClicked);
            QList<QLineEdit *> listline = findChildren<QLineEdit *>();
            for (int i = 0;i<listline.size();i++) listline.at(i)->setEnabled(false);
            ui->SexegroupBox->setEnabled(false);
            ui->ALDcheckBox->setEnabled(false);
            ui->CMUcheckBox->setEnabled(false);
            widg_vitalebutton->setEnabled(false);
            return;
        }
        // D - le dossier n'existe pas, on le crée
        QHash<QString, QVariant> listbinds;
        listbinds[CP_NOM_PATIENTS]          = ui->NomlineEdit->text();
        listbinds[CP_PRENOM_PATIENTS]       = ui->PrenomlineEdit->text();
        listbinds[CP_DDN_PATIENTS]          = ui->DDNdateEdit->date().toString("yyyy-MM-dd");
        listbinds[CP_SEXE_PATIENTS]         = Sexe;
        m_currentpatient = Patients::CreationPatient(listbinds);
        if (m_currentpatient == Q_NULLPTR)
            reject();

        // Mise à jour de donneessocialespatients
        ItemsList::update(m_currentpatient, CP_ADRESSE1_DSP,    Utils::trimcapitilize(ui->Adresse1lineEdit->text()));
        ItemsList::update(m_currentpatient, CP_ADRESSE2_DSP,    Utils::trimcapitilize(ui->Adresse2lineEdit->text()));
        ItemsList::update(m_currentpatient, CP_ADRESSE3_DSP,    Utils::trimcapitilize(ui->Adresse3lineEdit->text()));
        ItemsList::update(m_currentpatient, CP_CODEPOSTAL_DSP,  wdg_CPlineedit->text());
        ItemsList::update(m_currentpatient, CP_VILLE_DSP,       Utils::trimcapitilize(wdg_villelineedit->text().left(70)));
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
    else if (m_mode == Modification)
    {
        // on vérifie si le dossier existe déjà avec les mêmes nom, prénom et DDN
        QString requete = "select idPat from " TBL_PATIENTS
                          " where PatNom LIKE '" + Utils::correctquoteSQL(Utils::trimcapitilize(ui->NomlineEdit->text())) + "%'"
                          " and PatPrenom LIKE '" + Utils::correctquoteSQL(Utils::trimcapitilize(ui->PrenomlineEdit->text())) + "%'"
                          " and PatDDN = '" + ui->DDNdateEdit->date().toString("yyyy-MM-dd") + "'" +
                          " and idpat <> " + QString::number(m_currentpatient->id());
        QVariantList patdata = db->getFirstRecordFromStandardSelectSQL(requete, m_ok, tr("Impossible d'interroger la table des patients!"));
        if(!m_ok)
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
        ItemsList::update(m_currentpatient, CP_CODEPOSTAL_DSP,  wdg_CPlineedit->text());
        ItemsList::update(m_currentpatient, CP_VILLE_DSP,       Utils::trimcapitilize(wdg_villelineedit->text().left(70)));
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
    else if (m_mode == Creation)
    {
        //1 - Mise à jour patients
        if (Sexe != "")
            ItemsList::update(m_currentpatient, CP_SEXE_PATIENTS, Sexe);
        //2 - Mise à jour de donneessocialespatients
        ItemsList::update(m_currentpatient, CP_ADRESSE1_DSP,    Utils::trimcapitilize(ui->Adresse1lineEdit->text()));
        ItemsList::update(m_currentpatient, CP_ADRESSE2_DSP,    Utils::trimcapitilize(ui->Adresse2lineEdit->text()));
        ItemsList::update(m_currentpatient, CP_ADRESSE3_DSP,    Utils::trimcapitilize(ui->Adresse3lineEdit->text()));
        ItemsList::update(m_currentpatient, CP_CODEPOSTAL_DSP,  wdg_CPlineedit->text());
        ItemsList::update(m_currentpatient, CP_VILLE_DSP,       Utils::trimcapitilize(wdg_villelineedit->text().left(70)));
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
        m_menucontextuel = new QMenu(this);
        QAction *pAction_IdentPatient = m_menucontextuel->addAction(tr("Modifier les coordonnées de ce médecin"));
        connect (pAction_IdentPatient,      &QAction::triggered, this,    [=] {ModifCorrespondant();});

        // ouvrir le menu
        m_menucontextuel->exec(cursor().pos());
        delete m_menucontextuel;
    }
}

void dlg_identificationpatient::ModifCorrespondant()
{
    int idcor           = ui->MGupComboBox->currentData().toInt();
    bool onlydoctors = true;
    dlg_identificationcorresp *Dlg_IdentCorresp = new dlg_identificationcorresp(dlg_identificationcorresp::Modification, onlydoctors, Datas::I()->correspondants->getById(idcor, Item::LoadDetails), this);
    if (Datas::I()->correspondants->getById(idcor)==Q_NULLPTR)
        Dlg_IdentCorresp->ui->NomlineEdit   ->setText(ui->MGupComboBox->currentText());
    else
        Dlg_IdentCorresp->ui->NomlineEdit   ->setText(Datas::I()->correspondants->getById(idcor)->nom());
    Dlg_IdentCorresp->ui->PrenomlineEdit->setFocus();
    Dlg_IdentCorresp->ui->MGradioButton ->setChecked(true);
    Dlg_IdentCorresp->setWindowModality(Qt::WindowModal);
    if (Dlg_IdentCorresp->exec() == QDialog::Accepted)
    {
        Procedures::ReconstruitComboCorrespondants(ui->MGupComboBox, Correspondants::QueLesGeneralistes);
        ui->MGupComboBox->setCurrentIndex(ui->MGupComboBox->findData(idcor));
    }
    delete Dlg_IdentCorresp;
}

void dlg_identificationpatient::AnnulpushButtonClicked()
{
    if (m_mode == Creation)
    {
        UpMessageBox msgbox(this);
        UpSmallButton OKBouton(tr("Annuler la création"));
        UpSmallButton NoBouton(tr("Revenir à la fiche"));
        msgbox.setText("Euuhh... " + Datas::I()->users->userconnected()->login());
        msgbox.setInformativeText(tr("Annuler la création de ce dossier ?"));
        msgbox.setIcon(UpMessageBox::Warning);
        msgbox.addButton(&NoBouton, UpSmallButton::OUPSBUTTON);
        msgbox.addButton(&OKBouton, UpSmallButton::CLOSEBUTTON);
        msgbox.exec();
        if (msgbox.clickedButton() == &OKBouton)
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
                if (!Utils::rgx_mail.exactMatch(ui->MaillineEdit->text()))
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
    if (m_mode == Copie)                                             //!> le nouveau dossier n'est pas encore créé (il ne le sera que si la fche est validée), on se contente de recopier les données du patient passsé en paramètre dans la fiche
    {
        if (!m_currentpatient->isalloaded())
            Datas::I()->patients->loadAll(m_currentpatient, Item::Update);
        ui->NomlineEdit->setText(m_currentpatient->nom());
        ui->Adresse1lineEdit->setText(m_currentpatient->adresse1());
        ui->Adresse2lineEdit->setText(m_currentpatient->adresse2());
        ui->Adresse3lineEdit->setText(m_currentpatient->adresse3());
        wdg_CPlineedit->setText(m_currentpatient->codepostal());
        wdg_villelineedit->setText(m_currentpatient->ville());
        ui->TellineEdit->setText(m_currentpatient->telephone());
        ui->ModifierDDNupPushButton->setVisible(false);
        int e = ui->MGupComboBox->findData(m_currentpatient->idmg());
        if (e > -1)
            ui->MGupComboBox->setCurrentIndex(e);
        ui->idPatientlabel->setText("");
        ui->Createurlabel->setText("");
    }
    else if (m_mode == Creation)                                     //!> le nouveau dossier est créé, on affiche les paramètres définis à sa création (nom, prénom, DDN, idcreateur, datecreation)
    {
        ui->NomlineEdit->setText(m_currentpatient->nom());
        ui->PrenomlineEdit->setText(m_currentpatient->prenom());
        ui->DDNdateEdit->setDate(m_currentpatient->datedenaissance());
        ui->NomlineEdit->setEnabled(false);
        ui->PrenomlineEdit->setEnabled(false);
        ui->DDNdateEdit->setEnabled(false);
        ui->idPatientlabel->setText(tr("Dossier n° ") + QString::number(m_currentpatient->id()));
        QString textcreateur (tr("Créé le ") + m_currentpatient->datecreationdossier().toString(tr("d-M-yyyy")));
        if (Datas::I()->users->getById(m_currentpatient->idcreateur()) != Q_NULLPTR)
            textcreateur += "\n" + tr("par ") + Datas::I()->users->getById(m_currentpatient->idcreateur())->login();
        ui->Createurlabel->setText(textcreateur);
        ui->Adresse1lineEdit->clear();
        ui->Adresse2lineEdit->clear();
        ui->Adresse3lineEdit->clear();
        QString CP = Procedures::CodePostalParDefaut();
        wdg_CPlineedit          ->completer()->setCurrentRow(wdg_villeCP->villes()->ListeCodesPostaux().indexOf(CP)); // ce micmac est nécessaire à cause d'un bug de QCompleter en mode InLineCompletion
        // il faut synchroniser à la main le QCompleter et le QlineEdit au premier affichage
        wdg_CPlineedit          ->setText(CP);
        wdg_villelineedit->setText(Procedures::VilleParDefaut());
        ui->TellineEdit->clear();
        ui->PortablelineEdit->clear();
        ui->MaillineEdit->clear();
        ui->ProfessionlineEdit->clear();
        ui->ALDcheckBox->setChecked(false);
        ui->CMUcheckBox->setChecked(false);
    }
    else if (m_mode == Modification)                                 //!> on ne crée pas de nouveau dossier, on affiche tous les paramètres connus du dossier
    {
        if (!m_currentpatient->isalloaded())
            Datas::I()->patients->loadAll(m_currentpatient, Item::Update);
        ui->NomlineEdit->setText(m_currentpatient->nom());
        ui->PrenomlineEdit->setText(m_currentpatient->prenom());
        ui->DDNdateEdit->setDate(m_currentpatient->datedenaissance());
        ui->DDNdateEdit->setEnabled(false);
        ui->MradioButton->setChecked(m_currentpatient->sexe() == "M");
        ui->FradioButton->setChecked(m_currentpatient->sexe() == "F");
        ui->idPatientlabel->setText(tr("Dossier n° ") + QString::number(m_currentpatient->id()));
        QString textcreateur (tr("Créé le ") + m_currentpatient->datecreationdossier().toString(tr("d-M-yyyy")));
        if (Datas::I()->users->getById(m_currentpatient->idcreateur()) != Q_NULLPTR)
            textcreateur += "\n" + tr("par ") + Datas::I()->users->getById(m_currentpatient->idcreateur())->login();
        ui->Createurlabel->setText(textcreateur);
        ui->Adresse1lineEdit->setText(m_currentpatient->adresse1());
        ui->Adresse2lineEdit->setText(m_currentpatient->adresse2());
        ui->Adresse3lineEdit->setText(m_currentpatient->adresse3());
        QString CP;
        CP = m_currentpatient->codepostal();
        wdg_CPlineedit          ->completer()->setCurrentRow(wdg_villeCP->villes()->ListeCodesPostaux().indexOf(CP)); // ce micmac est nécessaire à cause d'un bug de QCompleter en mode InLineCompletion
        // il faut synchroniser à la main le QCompleter et le QlineEdit au premier affichage

        wdg_CPlineedit          ->setText(CP);
        wdg_villelineedit   ->setText(m_currentpatient->ville());
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
    dlg_identificationcorresp *Dlg_IdentCorresp        = new dlg_identificationcorresp(dlg_identificationcorresp::Creation, onlydoctors, Q_NULLPTR, this);
    Dlg_IdentCorresp->ui->NomlineEdit->setText(ui->MGupComboBox->currentText());
    Dlg_IdentCorresp->ui->PrenomlineEdit->setFocus();
    Dlg_IdentCorresp->ui->MGradioButton->setChecked(true);
    Dlg_IdentCorresp->setWindowModality(Qt::WindowModal);
    if (Dlg_IdentCorresp->exec() == QDialog::Accepted)
        idcor = Dlg_IdentCorresp->idcurrentcorrespondant();
    delete Dlg_IdentCorresp;
    return idcor;
}


// ------------------------------------------------------------------------------------------
// Enregistre ou met à jour les coordonnées du médecin traitant
// ------------------------------------------------------------------------------------------
void dlg_identificationpatient::MAJMG()
{
    QString anc = ui->MGupComboBox->valeuravant();
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
            UpMessageBox msgbox(this);
            UpSmallButton OKBouton(tr("Enregistrer le correspondant"));
            UpSmallButton NoBouton(tr("Annuler"));
            msgbox.setText("Euuhh... " + Datas::I()->users->userconnected()->login());
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
                    Procedures::ReconstruitComboCorrespondants(ui->MGupComboBox, Correspondants::QueLesGeneralistes);
                    ui->MGupComboBox->setCurrentIndex(ui->MGupComboBox->findData(idcor));
                    Datas::I()->patients->updateCorrespondant(m_currentpatient, Correspondant::MG, Datas::I()->correspondants->getById(ui->MGupComboBox->currentData().toInt()));
                }
                else
                    ui->MGupComboBox->setCurrentText(anc);
            }
            else
                ui->MGupComboBox->setCurrentIndex(ui->MGupComboBox->findText(anc));
            msgbox.close();
        }
        else if (ui->MGupComboBox->valeuravant() != "" && m_mode != Copie)
            Datas::I()->patients->updateCorrespondant(m_currentpatient, Correspondant::MG);
    }
}

