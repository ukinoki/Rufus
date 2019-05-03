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
    UpDialog(QDir::homePath() + NOMFIC_INI, "PositionsFiches/PositionIdentificationPatient", parent),
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
    proc->ReconstruitComboCorrespondants(ui->MGupComboBox,false);
    ui->MGupComboBox    ->setCurrentIndex(-1);

    VilleCPwidg         = new VilleCPWidget(proc->getVilles(), ui->Principalframe);
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

    connect (OKButton,                      SIGNAL(clicked()),                          this,   SLOT (Slot_OKpushButtonClicked()));
    disconnect (CancelButton,               SIGNAL(clicked()),                          this,   SLOT (reject()));
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
    db->UpdateCorrespondant(m_currentpatient, DataBase::MG, Datas::I()->correspondants->getById(ui->MGupComboBox->currentData().toInt()));
}

void    dlg_identificationpatient::Slot_EnableOKpushButton()
{
    OKButton    ->setEnabled(true);
    OKButton    ->setShortcut(QKeySequence("Meta+Return"));
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
        proc->ReconstruitComboCorrespondants(ui->MGupComboBox,false);
        db->loadMedicalDataPatient(m_currentpatient, ok);
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

        // C - On vérifie ensuite si ce dossier existe déjà
        QString requete = "select idPat from " NOM_TABLE_PATIENTS
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
            m_currentpatient = db->loadPatientById(patdata.at(0).toInt(), true);
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
        m_currentpatient = db->CreationPatient(ui->NomlineEdit->text(), ui->PrenomlineEdit->text(), ui->DDNdateEdit->date(), Sexe);
        if (m_currentpatient == Q_NULLPTR)
            reject();

        // Mise à jour de donneessocialespatients
        QString ALD = (ui->ALDcheckBox->isChecked()? "1" : "null");
        QString CMU = (ui->CMUcheckBox->isChecked()? "1" : "null");
        requete = "UPDATE " NOM_TABLE_DONNEESSOCIALESPATIENTS
                         " SET PatAdresse1 = '" + Utils::correctquoteSQL(Utils::trimcapitilize(ui->Adresse1lineEdit->text())) +
                         "', PatAdresse2 = '" + Utils::correctquoteSQL(Utils::trimcapitilize(ui->Adresse2lineEdit->text())) +
                         "', PatAdresse3 = '" + Utils::correctquoteSQL(Utils::trimcapitilize(ui->Adresse3lineEdit->text())) +
                         "', PatCodePostal = '" + CPlineEdit->text() + "'";
        if (!ui->NNIlineEdit->text().isEmpty())
            requete +=   ", PatNNI = " + ui->NNIlineEdit->text();
        requete +=       ", PatVille = '" + Utils::correctquoteSQL(Utils::trimcapitilize(VillelineEdit->text())).left(70) +
                         "', PatTelephone = '" + ui->TellineEdit->text() +
                         "', PatPortable = '" + ui->PortablelineEdit->text() +
                         "', PatMail = '" + ui->MaillineEdit->text() +
                         "', PatProfession = '" + Utils::correctquoteSQL(Utils::trimcapitilize(ui->ProfessionlineEdit->text(), true)) + "'";
        requete +=       ", PatALD = "  + ALD;
        requete +=       ", PatCMU = " + CMU;
        requete +=       " WHERE idPat = " + QString::number(m_currentpatient->id());

        db->StandardSQL(requete, tr("Impossible d'écrire dans la table des données sociales"));

        // Mise à jour du medecin traitant
        db->UpdateCorrespondant(m_currentpatient, DataBase::MG, Datas::I()->correspondants->getById(ui->MGupComboBox->currentData().toInt()));
        accept();
    }
    else if (gMode == Modification)
    {
        // on vérifie si le dossier existe déjà avec les mêmes nom, prénom et DDN
        QString requete = "select idPat from " NOM_TABLE_PATIENTS
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
        }            // Mise à jour SQL patients
        //1 - Mise à jour patients
        requete =    "UPDATE " NOM_TABLE_PATIENTS
                     " SET PatNom = '" + Utils::correctquoteSQL(ui->NomlineEdit->text()) +
                     "', PatPrenom = '" + Utils::correctquoteSQL(ui->PrenomlineEdit->text()) +
                     "', PatDDN = '" + ui->DDNdateEdit->date().toString("yyyy-MM-dd");
        if (Sexe != "")
            requete += "', Sexe = '" + Sexe;
        requete +=   "' WHERE idPat = " + QString::number(m_currentpatient->id());
        db->StandardSQL(requete,tr("Impossible d'écrire dans la table PATIENTS"));

        // Mise à jour de donneessocialespatients
        requete =   "UPDATE " NOM_TABLE_DONNEESSOCIALESPATIENTS
                " SET PatAdresse1 = '" + Utils::correctquoteSQL(Utils::trimcapitilize(ui->Adresse1lineEdit->text())) +
                "', PatAdresse2 = '" + Utils::correctquoteSQL(Utils::trimcapitilize(ui->Adresse2lineEdit->text())) +
                "', PatAdresse3 = '" + Utils::correctquoteSQL(Utils::trimcapitilize(ui->Adresse3lineEdit->text())) +
                "', PatCodePostal = '" + CPlineEdit->text() + "'";
        if (!ui->NNIlineEdit->text().isEmpty())
            requete += ", PatNNI = " + ui->NNIlineEdit->text();
        requete +=
                ", PatVille = '" + Utils::correctquoteSQL(Utils::trimcapitilize(VillelineEdit->text())).left(70) +
                "', PatTelephone = '" + ui->TellineEdit->text() +
                "', PatPortable = '" + ui->PortablelineEdit->text() +
                "', PatMail = '" + ui->MaillineEdit->text() +
                "', PatProfession = '" + Utils::correctquoteSQL(Utils::trimcapitilize(ui->ProfessionlineEdit->text())) + "'";
        if (ui->ALDcheckBox->isChecked())
            requete += ", PatALD = 1";
        else
            requete += ", PatALD = null";
        if (ui->CMUcheckBox->isChecked())
            requete += ", PatCMU = 1";
        else
            requete += ", PatCMU = null";
        requete += " WHERE idPat = " + QString::number(m_currentpatient->id());
        db->StandardSQL(requete,tr("Impossible d'écrire dans la table des données sociales"));

        // Mise à jour du médecin traitant
        db->UpdateCorrespondant(m_currentpatient, DataBase::MG, Datas::I()->correspondants->getById(ui->MGupComboBox->currentData().toInt()));
        accept();
    }
    else if (gMode == Creation)
    {
        //1 - Mise à jour patients
        if (Sexe != "")
        {
            QString requete =   "UPDATE " NOM_TABLE_PATIENTS " SET Sexe = '" + Sexe + "' WHERE idPat = " + QString::number(m_currentpatient->id());
            db->StandardSQL(requete,"Impossible d'écrire dans la table patients");
        }
        //2 - Mise à jour de donneessocialespatients
        QString requete =   "UPDATE " NOM_TABLE_DONNEESSOCIALESPATIENTS
                " SET PatAdresse1 = '" + Utils::correctquoteSQL(Utils::trimcapitilize(ui->Adresse1lineEdit->text())) +
                "', PatAdresse2 = '" + Utils::correctquoteSQL(Utils::trimcapitilize(ui->Adresse2lineEdit->text())) +
                "', PatAdresse3 = '" + Utils::correctquoteSQL(Utils::trimcapitilize(ui->Adresse3lineEdit->text())) +
                "', PatCodePostal = '" + CPlineEdit->text() + "'";
        if (!ui->NNIlineEdit->text().isEmpty())
            requete += ", PatNNI = " + ui->NNIlineEdit->text();
        requete +=
                ", PatVille = '" + Utils::correctquoteSQL(Utils::trimcapitilize(VillelineEdit->text())).left(70) +
                "', PatTelephone = '" + ui->TellineEdit->text() +
                "', PatPortable = '" + ui->PortablelineEdit->text() +
                "', PatMail = '" + ui->MaillineEdit->text() +
                "', PatProfession = '" + Utils::correctquoteSQL(Utils::trimcapitilize(ui->ProfessionlineEdit->text())) + "'";
        if (ui->ALDcheckBox->isChecked())
            requete += ", PatALD = 1";
        else
            requete += ", PatALD = null";
        if (ui->CMUcheckBox->isChecked())
            requete += ", PatCMU = 1";
        else
            requete += ", PatCMU = null";
        requete += " WHERE idPat = " + QString::number(m_currentpatient->id());

        db->StandardSQL(requete, tr("Impossible d'écrire dans la table des données sociales"));
        //2 - Mise à jour de medecin traitant
        db->UpdateCorrespondant(m_currentpatient, DataBase::MG, Datas::I()->correspondants->getById(ui->MGupComboBox->currentData().toInt()));
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
    Dlg_IdentCorresp    = new dlg_identificationcorresp(dlg_identificationcorresp::Modification, onlydoctors, Datas::I()->correspondants->getById(idcor, true, true));
    if (Datas::I()->correspondants->getById(idcor)==Q_NULLPTR)
        Dlg_IdentCorresp->ui->NomlineEdit   ->setText(ui->MGupComboBox->currentText());
    else
        Dlg_IdentCorresp->ui->NomlineEdit   ->setText(Datas::I()->correspondants->getById(idcor)->nom());
    Dlg_IdentCorresp->ui->PrenomlineEdit->setFocus();
    Dlg_IdentCorresp->ui->MGradioButton ->setChecked(true);
    if (Dlg_IdentCorresp->exec()>0)
    {
        proc->ReconstruitComboCorrespondants(ui->MGupComboBox,false);
        ui->MGupComboBox->setCurrentIndex(ui->MGupComboBox->findData(idcor));
        ListeCorModifiee = Dlg_IdentCorresp->identcorrespondantmodifiee();
    }
    delete Dlg_IdentCorresp;
}

void dlg_identificationpatient::Slot_AnnulpushButtonClicked()
{
    if (gMode == Creation)
    {
        UpMessageBox msgbox;
        UpSmallButton OKBouton(tr("Annuler la création"));
        UpSmallButton NoBouton;
        msgbox.setText("Euuhh... " + db->getUserConnected()->getLogin());
        msgbox.setInformativeText(tr("Annuler la création de ce dossier ?"));
        msgbox.setIcon(UpMessageBox::Warning);
        msgbox.addButton(&NoBouton, UpSmallButton::CANCELBUTTON);
        msgbox.addButton(&OKBouton, UpSmallButton::STARTBUTTON);
        msgbox.exec();
        if (msgbox.clickedButton() == &OKBouton)
        {
            db->StandardSQL("delete from " NOM_TABLE_PATIENTS " where idPat = " + QString::number(m_currentpatient->id()));
            db->StandardSQL("delete from " NOM_TABLE_DONNEESSOCIALESPATIENTS " where idPat = " + QString::number(m_currentpatient->id()));
            db->StandardSQL("delete from " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS " where idPat = " + QString::number(m_currentpatient->id()));
            reject();
        }
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
    bool ok;
    if (!m_currentpatient->isalloaded())
    {
        db->loadSocialDataPatient(m_currentpatient, ok);
        db->loadMedicalDataPatient(m_currentpatient, ok);
    }
    if (gMode == Copie)
    {
        ui->NomlineEdit->setText(m_currentpatient->nom());
        ui->Adresse1lineEdit->setText(m_currentpatient->adresse1());
        ui->Adresse2lineEdit->setText(m_currentpatient->adresse2());
        ui->Adresse3lineEdit->setText(m_currentpatient->adresse3());
        CPlineEdit->setText(m_currentpatient->codepostal());
        VillelineEdit->setText(m_currentpatient->ville());
        ui->TellineEdit->setText(m_currentpatient->telephone());
        ui->idPatientlabel->setVisible(false);
        ui->Createurlabel->setVisible(false);
        ui->ModifierDDNupPushButton->setVisible(false);
        int e = ui->MGupComboBox->findData(m_currentpatient->idmg());
        if (e > -1)
            ui->MGupComboBox->setCurrentIndex(e);
        return;
    }
    if (gMode == Creation)
    {
        ui->NomlineEdit->setEnabled(false);
        ui->PrenomlineEdit->setEnabled(false);
        ui->DDNdateEdit->setEnabled(false);
    }
    else if (gMode == Modification)
        ui->DDNdateEdit->setEnabled(false);

    ui->NomlineEdit->setText(m_currentpatient->nom());
    ui->PrenomlineEdit->setText(m_currentpatient->prenom());
    ui->DDNdateEdit->setDate(m_currentpatient->datedenaissance());
    // pour decocher les 2 radiobutton sexe il faut d'abord leur retirer la propriétét AutoExclusive
    ui->MradioButton->setAutoExclusive(false);
    ui->FradioButton->setAutoExclusive(false);
    ui->MradioButton->setChecked(false);
    ui->FradioButton->setChecked(false);
    ui->MradioButton->setAutoExclusive(true);
    ui->FradioButton->setAutoExclusive(true);
    ui->MradioButton->setChecked(m_currentpatient->sexe() == "M");
    ui->FradioButton->setChecked(m_currentpatient->sexe() == "F");
    ui->idPatientlabel->setText(tr("Dossier n° ") + QString::number(m_currentpatient->id()));


    ui->Createurlabel->setText(tr("Créé le ") + m_currentpatient->datecreationdossier().toString(tr("d-M-yyyy")) + "\n" +
                               tr("par ") + Datas::I()->users->getById(m_currentpatient->idcreateur())->getLogin());
    ui->Adresse1lineEdit->clear();
    ui->Adresse2lineEdit->clear();
    ui->Adresse3lineEdit->clear();
    CPlineEdit->setText(proc->getCodePostalParDefaut());
    VillelineEdit->setText(proc->getVilleParDefaut());
    ui->TellineEdit->clear();
    ui->PortablelineEdit->clear();
    ui->MaillineEdit->clear();
    ui->ProfessionlineEdit->clear();
    ui->ALDcheckBox->setChecked(false);
    ui->CMUcheckBox->setChecked(false);
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
    if (m_currentpatient->NNI() > 0) ui->NNIlineEdit->setText(QString::number(m_currentpatient->NNI()));
    ui->ProfessionlineEdit->setText(m_currentpatient->profession());
    ui->ALDcheckBox     ->setChecked(m_currentpatient->isald());
    ui->CMUcheckBox     ->setChecked(m_currentpatient->iscmu());
    int e = ui->MGupComboBox->findData(m_currentpatient->idmg());
    if (e > -1)
        ui->MGupComboBox->setCurrentIndex(e);
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
    return m_currentpatient;
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
                    proc->ReconstruitComboCorrespondants(ui->MGupComboBox,false);
                    ui->MGupComboBox->setCurrentIndex(ui->MGupComboBox->findData(idcor));
                    db->UpdateCorrespondant(m_currentpatient, DataBase::MG, Datas::I()->correspondants->getById(ui->MGupComboBox->currentData().toInt()));
                }
                else
                    ui->MGupComboBox->setCurrentText(anc);
            }
            else
                ui->MGupComboBox->setCurrentIndex(ui->MGupComboBox->findText(anc));
            msgbox.close();
        }
        else if (ui->MGupComboBox->getValeurAvant() != "" && gMode != Copie)
            db->UpdateCorrespondant(m_currentpatient, DataBase::MG, Q_NULLPTR);
    }
}

