/* (C) 2016 LAINE SERGE
This file is part of Rufus.

Rufus is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Rufus is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Rufus. If not, see <http://www.gnu.org/licenses/>.
*/

#include "dlg_identificationpatient.h"
#include "icons.h"
#include "ui_dlg_identificationpatient.h"

dlg_identificationpatient::dlg_identificationpatient(QString *CreationModification, int *idPatAPasser, Procedures *procAPasser, QWidget *parent) :
    UpDialog(QDir::homePath() + NOMFIC_INI, "PositionsFiches/PositionIdentificationPatient", parent),
    ui(new Ui::dlg_identificationpatient)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

    proc                = procAPasser;
    gidPatient          = *idPatAPasser;
    lCreatModifCopie    = *CreationModification;
    db                  = DataBase::getInstance()->getDataBase();
    QVBoxLayout *globallay  = dynamic_cast<QVBoxLayout*>(layout());

    QVBoxLayout *vlay       = new QVBoxLayout;
    vlay                    ->setContentsMargins(0,10,0,10);
    vlay                    ->setSpacing(5);
    vlay                    ->insertWidget(0,ui->Createurlabel);
    vlay                    ->insertWidget(0,ui->idPatientlabel);
    vlay                    ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding, QSizePolicy::Expanding));

    globallay               ->insertWidget(0,ui->Principalframe);
    VitaleButton            = new UpSmallButton();
    VitaleButton            ->setIcon(Icons::icVitale());
    VitaleButton            ->setFixedHeight(100);
    VitaleButton            ->setStyleSheet("qproperty-iconSize:120px 100px;");
    AjouteWidgetLayButtons(VitaleButton, true);
    AjouteLayButtons(UpDialog::ButtonCancel | UpDialog::ButtonOK);
    OKButton                ->setText(tr("Enregistrer"));
    CancelButton            ->setText(tr("Annuler"));
    laybuttons              ->insertLayout(0, vlay);
    globallay               ->setSizeConstraint(QLayout::SetFixedSize);

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
    ReconstruitListeMG();
    ui->MGupComboBox    ->setCurrentIndex(-1);

    VilleCPwidg         = new VilleCPWidget(proc->getVilles(), ui->Principalframe, NOM_ALARME);
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

    ui->MGupComboBox    ->installEventFilter(this);
    gflagMG             = proc->GetflagMG();

    gTimer              = new QTimer(this);
    gTimer              ->start(1000);
    connect (gTimer,                            SIGNAL(timeout()),                              this,           SLOT (Slot_VerifMGFlag()));

    ui->NomlineEdit->setFocus();

    connect (OKButton,                          SIGNAL(clicked()),                              this,           SLOT (Slot_OKpushButtonClicked()));
    disconnect (CancelButton,                   SIGNAL(clicked()),                              this,           SLOT (reject()));
    connect (CancelButton,                      SIGNAL(clicked()),                              this,           SLOT (Slot_AnnulpushButtonClicked()));
    connect (ui->ModifierDDNupPushButton,       SIGNAL(clicked()),                              this,           SLOT (Slot_ModifDDN()));

    connect (ui->NomlineEdit,                   SIGNAL(editingFinished()),                      this,           SLOT (Slot_Majuscule()));
    connect (ui->PrenomlineEdit,                SIGNAL(editingFinished()),                      this,           SLOT (Slot_Majuscule()));
    connect (ui->Adresse1lineEdit,              SIGNAL(editingFinished()),                      this,           SLOT (Slot_Majuscule()));
    connect (ui->Adresse2lineEdit,              SIGNAL(editingFinished()),                      this,           SLOT (Slot_Majuscule()));
    connect (ui->Adresse3lineEdit,              SIGNAL(editingFinished()),                      this,           SLOT (Slot_Majuscule()));
    connect (ui->ProfessionlineEdit,            SIGNAL(editingFinished()),                      this,           SLOT (Slot_Majuscule()));
    connect (ui->DDNdateEdit,                   SIGNAL(dateChanged(QDate)),                     this,           SLOT (Slot_EnableOKpushButton()));
    connect (ui->NomlineEdit,                   SIGNAL(textEdited(QString)),                    this,           SLOT (Slot_EnableOKpushButton()));
    connect (ui->PrenomlineEdit,                SIGNAL(textEdited(QString)),                    this,           SLOT (Slot_EnableOKpushButton()));
    connect (ui->Adresse1lineEdit,              SIGNAL(textEdited(QString)),                    this,           SLOT (Slot_EnableOKpushButton()));
    connect (ui->Adresse2lineEdit,              SIGNAL(textEdited(QString)),                    this,           SLOT (Slot_EnableOKpushButton()));
    connect (ui->Adresse3lineEdit,              SIGNAL(textEdited(QString)),                    this,           SLOT (Slot_EnableOKpushButton()));
    connect (ui->TellineEdit,                   SIGNAL(textEdited(QString)),                    this,           SLOT (Slot_EnableOKpushButton()));
    connect (ui->PortablelineEdit,              SIGNAL(textEdited(QString)),                    this,           SLOT (Slot_EnableOKpushButton()));
    connect (ui->MaillineEdit,                  SIGNAL(textEdited(QString)),                    this,           SLOT (Slot_EnableOKpushButton()));
    connect (ui->ProfessionlineEdit,            SIGNAL(textEdited(QString)),                    this,           SLOT (Slot_EnableOKpushButton()));
    connect (ui->MradioButton,                  SIGNAL(clicked()),                              this,           SLOT (Slot_EnableOKpushButton()));
    connect (ui->FradioButton,                  SIGNAL(clicked()),                              this,           SLOT (Slot_EnableOKpushButton()));
    connect (ui->ALDcheckBox,                   SIGNAL(clicked()),                              this,           SLOT (Slot_EnableOKpushButton()));
    connect (ui->CMUcheckBox,                   SIGNAL(clicked()),                              this,           SLOT (Slot_EnableOKpushButton()));
    connect (ui->MGupComboBox,                  QOverload<int>::of(&QComboBox::activated),      [=](int) {ChoixMG();});
    connect (ui->MGupComboBox,                  SIGNAL(currentTextChanged(QString)) ,           this,           SLOT (Slot_EnableOKpushButton()));

    OKButton  ->setEnabled(false);
    gAutorDepart        = true;
    gControleMGCombo    = true;

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
    QString req = "update " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS " set idcormedmg = " + ui->MGupComboBox->currentData().toString() + " where idpat = " + QString::number(gidPatient);
    QSqlQuery (req,db);
}

void    dlg_identificationpatient::Slot_EnableOKpushButton()
{
    OKButton    ->setEnabled(true);
    OKButton    ->setShortcut(QKeySequence("Meta+Return"));
}

void dlg_identificationpatient::Slot_Majuscule()
{
    QLineEdit* Line = static_cast<QLineEdit*>(sender());
    if (Line->text() != proc->MajusculePremiereLettre(Line->text()))
    {
        Line->setText(proc->MajusculePremiereLettre(Line->text(),false));
        OKButton->setEnabled(true);
    }
}

void    dlg_identificationpatient::Slot_ModifDDN()
{
    ui->DDNdateEdit->setEnabled(true);
    ui->ModifierDDNupPushButton->setVisible(false);
}

void dlg_identificationpatient::Slot_VerifMGFlag()
{
    if (gflagMG < proc->GetflagMG())
    {
        gflagMG = proc->GetflagMG();
        // on reconstruit la liste des MG
        ReconstruitListeMG();
        QString req = "select idcormedmg from " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS " where idpat = " + QString::number(gidPatient);
        QSqlQuery quer(req,db);
        if (quer.seek(0))
            ui->MGupComboBox->setCurrentIndex(ui->MGupComboBox->findData(quer.value(0).toInt()));
        else
            ui->MGupComboBox->setCurrentIndex(-1);
    }

}

void    dlg_identificationpatient::Slot_OKpushButtonClicked()
{
    QString PatNom, PatPrenom, PatDDN, PatCreePar, PatCreeLe;
    PatNom      = proc->CorrigeApostrophe(proc->MajusculePremiereLettre(ui->NomlineEdit->text(),true));
    PatPrenom   = proc->CorrigeApostrophe(proc->MajusculePremiereLettre(ui->PrenomlineEdit->text(),true));
    PatDDN      = ui->DDNdateEdit->date().toString("yyyy-MM-dd");
    PatCreeLe   = QDateTime::currentDateTime().date().toString("yyyy-MM-dd");
    PatCreePar  = QString::number(proc->getDataUser()->id());


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
    if (lCreatModifCopie == "Copie")
    {
        // A - On vérifie qu'une date de naissance a été enregistrée, différente de la date par défaut
        if (ui->DDNdateEdit->date() == QDate(2000,1,1))
        {
            UpMessageBox msgbox;
            UpSmallButton *OKBouton = new UpSmallButton();
            OKBouton->setText(tr("Je confirme"));
            UpSmallButton *NoBouton = new UpSmallButton();
            NoBouton->setText(tr("Annuler"));
            msgbox.setText("Euuhh... " + proc->getDataUser()->getLogin());
            msgbox.setInformativeText(tr("Confirmez vous la date de naissance?") + "\n" + ui->DDNdateEdit->date().toString(tr("d-MMM-yyyy")));
            msgbox.setIcon(UpMessageBox::Warning);
            msgbox.addButton(NoBouton, UpSmallButton::CANCELBUTTON);
            msgbox.addButton(OKBouton, UpSmallButton::STARTBUTTON);
            msgbox.exec();
            if (msgbox.clickedButton() != OKBouton)
            {
                delete NoBouton;
                delete OKBouton;
                return;
            }
            delete NoBouton;
            delete OKBouton;
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
        QSqlQuery IdentPatientQuery (requete,db);
        if (DataBase::getInstance()->traiteErreurRequete(IdentPatientQuery,requete, tr("Impossible d'interroger la table des patients!")))
        {
            FermeFiche(Reject);
            return;
        }

        if (IdentPatientQuery.size() > 0)
        {
            UpMessageBox::Watch(this,tr("Ce dossier existe déjà!"));
            IdentPatientQuery.first();
            gidPatient = IdentPatientQuery.value(0).toInt();
            AfficheDossierAlOuverture();
            disconnect (OKButton, SIGNAL(clicked()), this, SLOT (Slot_OKpushButtonClicked()));
            connect(OKButton, SIGNAL(clicked(bool)),this,SLOT(Slot_AnnulpushButtonClicked()));
            QList<QLineEdit *> listline = findChildren<QLineEdit *>();
            for (int i = 0;i<listline.size();i++) listline.at(i)->setEnabled(false);
            ui->SexegroupBox->setEnabled(false);
            ui->ALDcheckBox->setEnabled(false);
            ui->CMUcheckBox->setEnabled(false);
            VitaleButton->setEnabled(false);
            return;
        }
        // D - le dossier n'existe pas, on le crée
        QString gSexePat = "";
        if (ui->MradioButton->isChecked()) gSexePat = "M";
        if (ui->FradioButton->isChecked()) gSexePat = "F";
        QString insrequete = "INSERT INTO " NOM_TABLE_PATIENTS
                    " (PatNom, PatPrenom, PatDDN, PatCreele, PatCreePar, Sexe) "
                    " VALUES "
                    " ('" + PatNom + "', '" + PatPrenom + "', '" + PatDDN + "', NOW(), '" + PatCreePar +"' , '" + gSexePat +"');";
        QSqlQuery InsertPatQuery (insrequete,db);
        if (DataBase::getInstance()->traiteErreurRequete(InsertPatQuery,insrequete,tr("Impossible de créer le dossier")))
        {
            FermeFiche(Reject);
        }
        // Récupération de l'idPatient créé ------------------------------------
        QString recuprequete = "SELECT  idPat FROM " NOM_TABLE_PATIENTS
                    " WHERE PatNom = '" + PatNom + "' AND PatPrenom = '" + PatPrenom + "' AND PatDDN = '" + PatDDN + "'";

        QSqlQuery ChercheIdPatientQuery (recuprequete,db);
        if (DataBase::getInstance()->traiteErreurRequete(ChercheIdPatientQuery,recuprequete,tr("Impossible de sélectionner les enregistrements")))
        {
            FermeFiche(Reject);
        }
        ChercheIdPatientQuery.first();
        gidPatient = ChercheIdPatientQuery.value(0).toInt();              // retourne idPatient
        requete =   "INSERT INTO " NOM_TABLE_DONNEESSOCIALESPATIENTS " (idPat) VALUES ('" + QString::number(gidPatient) + "')";
        QSqlQuery CreeDonneeSocialePatientQuery (requete,db);
        DataBase::getInstance()->traiteErreurRequete(CreeDonneeSocialePatientQuery,requete,tr("Impossible de créer les données sociales"));
        FermeFiche(Accept);
    }
    else if (lCreatModifCopie == "Modification")
    {
        // on vérifie si le dossier existe déjà avec les mêmes nom, prénom et DDN
        QString requete = "select idPat from " NOM_TABLE_PATIENTS
                " where PatNom LIKE '" + proc->CorrigeApostrophe(ui->NomlineEdit->text()) + "%' and PatPrenom LIKE '" +
                proc->CorrigeApostrophe(ui->PrenomlineEdit->text()) + "%' and PatDDN = '" +
                ui->DDNdateEdit->date().toString("yyyy-MM-dd") + "'";
        QSqlQuery IdentPatientQuery (requete,db);
        if (DataBase::getInstance()->traiteErreurRequete(IdentPatientQuery,requete, tr("Impossible d'interroger la table des patients!")))
            return;
        if (IdentPatientQuery.size() > 0)
        {
            IdentPatientQuery.first();
            if (IdentPatientQuery.value(0).toInt() != gidPatient)
            {
                UpMessageBox::Watch(this,tr("Ce dossier existe déjà!"));
                return;
            }
        }
        FermeFiche(Accept);
    }
    else
        FermeFiche(Accept);
}

void    dlg_identificationpatient::Slot_AnnulpushButtonClicked()
{
    gControleMGCombo = false;
    if (lCreatModifCopie == "Creation")
    {
        UpMessageBox msgbox;
        UpSmallButton *OKBouton = new UpSmallButton();
        OKBouton->setText(tr("Annuler la création"));
        UpSmallButton *NoBouton = new UpSmallButton();
        msgbox.setText("Euuhh... " + proc->getDataUser()->getLogin());
        msgbox.setInformativeText(tr("Annuler la création de ce dossier ?"));
        msgbox.setIcon(UpMessageBox::Warning);
        msgbox.addButton(NoBouton, UpSmallButton::CANCELBUTTON);
        msgbox.addButton(OKBouton, UpSmallButton::STARTBUTTON);
        msgbox.exec();
        if (msgbox.clickedButton() == OKBouton)
        {
            QString AnnulRequete = "delete from " NOM_TABLE_PATIENTS " where idPat = " + QString::number(gidPatient);
            QSqlQuery (AnnulRequete,db);
            AnnulRequete = "delete from " NOM_TABLE_DONNEESSOCIALESPATIENTS " where idPat = " + QString::number(gidPatient);
            QSqlQuery (AnnulRequete,db);
            delete OKBouton;
            delete NoBouton;
            FermeFiche(Reject);
        }
        else
        {
            gControleMGCombo = true;
            delete OKBouton;
            delete NoBouton;
        }
    }
    else
        FermeFiche(Reject);
}

/*-------------------------------------------------------------------------------------
 Interception du focusOut
-------------------------------------------------------------------------------------*/
bool dlg_identificationpatient::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::FocusOut)
    {
        if (obj == ui->MGupComboBox)
            if (gControleMGCombo)
                MAJMG();
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
   QString req = "SELECT idPat, PatNom, PatPrenom, PatDDN, Sexe, PatCreele, PatCreePar FROM " NOM_TABLE_PATIENTS
            " WHERE idPat = '" + QString::number(gidPatient) + "'";
    QSqlQuery AfficheDossierQuery (req,db);
    if (DataBase::getInstance()->traiteErreurRequete(AfficheDossierQuery,req, tr("Impossible de retrouver le dossier de ce patient")))
        return;
    if (AfficheDossierQuery.size() == 0)           // Aucune mesure trouvee pour ces criteres
        return;
    AfficheDossierQuery.first();

    gNomPatient = AfficheDossierQuery.value(1).toString();
    gPrenomPatient = AfficheDossierQuery.value(2).toString();
    ui->NomlineEdit->setText(gNomPatient);
    ui->PrenomlineEdit->setText(AfficheDossierQuery.value(2).toString());
    ui->DDNdateEdit->setDate(AfficheDossierQuery.value(3).toDate());
    // pour decocher les 2 radiobutton sexe il faut d'abord leur retirer la propriétét AutoExclusive
    ui->MradioButton->setAutoExclusive(false);
    ui->FradioButton->setAutoExclusive(false);
    ui->MradioButton->setChecked(false);
    ui->FradioButton->setChecked(false);
    ui->MradioButton->setAutoExclusive(true);
    ui->FradioButton->setAutoExclusive(true);
    Sexe = AfficheDossierQuery.value(4).toString();
    if (Sexe == "M") ui->MradioButton->setChecked(true);
    if (Sexe == "F") ui->FradioButton->setChecked(true);
    ui->idPatientlabel->setText(tr("Dossier n° ") + QString::number(gidPatient));


    ui->Createurlabel->setText(tr("Créé le ") + AfficheDossierQuery.value(5).toDate().toString(tr("d-M-yyyy")) + "\n" +
                               tr("par ") + proc->getLogin(AfficheDossierQuery.value(6).toInt()));

    //2 - récupération des données sociales

    req = "SELECT idPat, PatAdresse1, PatAdresse2, PatAdresse3, PatCodePostal, PatVille, PatTelephone, PatPortable, PatMail, PatNNI, PatALD, PatProfession, PatCMU FROM " NOM_TABLE_DONNEESSOCIALESPATIENTS
            " WHERE idPat = '" + QString::number(gidPatient) + "'";
    QSqlQuery DonneesSocialesQuery (req,db);
    if (!DataBase::getInstance()->traiteErreurRequete(DonneesSocialesQuery,req, tr("Impossible de retrouver les données sociales!")))
    {
        if (DonneesSocialesQuery.size() > 0)
        {
            DonneesSocialesQuery.first();
            ui->Adresse1lineEdit->setText(DonneesSocialesQuery.value(1).toString());
            ui->Adresse2lineEdit->setText(DonneesSocialesQuery.value(2).toString());
            ui->Adresse3lineEdit->setText(DonneesSocialesQuery.value(3).toString());
            QString CP;
            if (DonneesSocialesQuery.value(4).toString() == "")
                CP = proc->getCodePostalParDefaut();
            else
                CP = DonneesSocialesQuery.value(4).toString();
            CPlineEdit          ->completer()->setCurrentRow(VilleCPwidg->villes()->getListCodePostal().indexOf(CP)); // ce micmac est nécessaire à cause d'un bug de QCompleter en mode InLineCompletion
                                                                                                // il faut synchroniser à la main le QCompleter et le QlineEdit au premier affichage

            CPlineEdit          ->setText(CP);
            if (DonneesSocialesQuery.value(5).toString() == "")
                VillelineEdit   ->setText(proc->getVilleParDefaut());
            else
                VillelineEdit   ->setText(DonneesSocialesQuery.value(5).toString());
            ui->TellineEdit     ->setText(DonneesSocialesQuery.value(6).toString());
            ui->PortablelineEdit->setText(DonneesSocialesQuery.value(7).toString());
            ui->MaillineEdit    ->setText(DonneesSocialesQuery.value(8).toString());
            if (DonneesSocialesQuery.value(9).toInt() > 0) ui->NNIlineEdit->setText(DonneesSocialesQuery.value(9).toString());
            ui->ProfessionlineEdit->setText(DonneesSocialesQuery.value(11).toString());
            ui->ALDcheckBox     ->setChecked(DonneesSocialesQuery.value(10).toInt() == 1);
            ui->CMUcheckBox     ->setChecked(DonneesSocialesQuery.value(12).toInt() == 1);
        }
        else
        {
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
        }
    }

    // - récupération du médecin traitant
    req  = "select idCorMedMG FROM "
            NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS
            " WHERE idPat = " + QString::number(gidPatient);
    QSqlQuery MGQuery (req,db);
    if (!DataBase::getInstance()->traiteErreurRequete(MGQuery,req, tr("Impossible de retrouver le médecin traitant!")))
    {
        if (MGQuery.size() > 0)
        {
            MGQuery.first();
            int idx = ui->MGupComboBox->findData(MGQuery.value(0).toInt());
            ui->MGupComboBox->setCurrentIndex(idx);
        }
    }
}

// ------------------------------------------------------------------------------------------
// Enregistre un nouveau correpondant
// ------------------------------------------------------------------------------------------
int dlg_identificationpatient::EnregistreNouveauCorresp()
{
    int idcor = -1;
    bool onlydoctors = true;
    Dlg_IdentCorresp        = new dlg_identificationcorresp("Creation",onlydoctors,0, proc, this);
    Dlg_IdentCorresp->ui->NomlineEdit->setText(ui->MGupComboBox->currentText());
    Dlg_IdentCorresp->ui->PrenomlineEdit->setFocus();
    Dlg_IdentCorresp->ui->MGradioButton->setChecked(true);
    if (Dlg_IdentCorresp->exec()>0)
    {
        idcor = Dlg_IdentCorresp->gidCor;
    }
    delete Dlg_IdentCorresp;
    return idcor;
}


void dlg_identificationpatient::FermeFiche(enum CloseReason Cause)
{
    switch (Cause) {
    case Accept:
        if (focusWidget() == ui->MGupComboBox){
            gControleMGCombo = false;
            MAJMG();
        }
        else gAutorDepart = true;

        if (gAutorDepart) accept();
        else {
            gControleMGCombo = true;
            gAutorDepart = true;
        }
        break;
    case Reject:
        reject();
        break;
    default:
        break;
    }
}

// ------------------------------------------------------------------------------------------
// Enregistre ou met à jour les coordonnées du méddecin traitant
// ------------------------------------------------------------------------------------------
void dlg_identificationpatient::MAJMG()
{
    QString anc = ui->MGupComboBox->getValeurAvant();
    QString nou = proc->MajusculePremiereLettre(ui->MGupComboBox->currentText(),true);
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
            UpSmallButton *OKBouton = new UpSmallButton();
            OKBouton->setText(tr("Enregistrer le correspondant"));
            UpSmallButton *NoBouton = new UpSmallButton();
            NoBouton->setText(tr("Annuler"));
            msgbox.setText("Euuhh... " + proc->getDataUser()->getLogin());
            msgbox.setInformativeText(tr("Correspondant inconnu! Souhaitez-vous l'enregistrer?"));
            msgbox.setIcon(UpMessageBox::Warning);
            msgbox.addButton(NoBouton, UpSmallButton::CANCELBUTTON);
            msgbox.addButton(OKBouton, UpSmallButton::STARTBUTTON);
            msgbox.exec();
            if (msgbox.clickedButton() == OKBouton)
            {
                int idcor = EnregistreNouveauCorresp();
                if (idcor >= 0)
                {
                    ReconstruireListMG = true;
                    gflagMG = proc->MAJflagMG();
                    ReconstruitListeMG();
                    ui->MGupComboBox->setCurrentIndex(ui->MGupComboBox->findData(idcor));
                }
                else
                    ui->MGupComboBox->setCurrentText(anc);
            }
            else
                ui->MGupComboBox->setCurrentIndex(ui->MGupComboBox->findText(anc));
            gAutorDepart = false;
            delete OKBouton;
            delete NoBouton;
            msgbox.close();
        }
        else if (ui->MGupComboBox->getValeurAvant() != "")
        {
            QString req = "update " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS " set idcormedmg = null where idpat = " + QString::number(gidPatient);
            QSqlQuery (req,db);
        }
    }
}

/*------------------------------------------------------------------------------------------------------------------------------------
-- Reconstruit la liste des MG correspondants -------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_identificationpatient::ReconstruitListeMG()
{
    ui->MGupComboBox->clear();
    QString req = "SELECT idCor, CorNom, CorPrenom FROM " NOM_TABLE_CORRESPONDANTS " where cormedecin = 1 order by cornom, corprenom";
    QSqlQuery ListMGQuery (req,db);
    DataBase::getInstance()->traiteErreurRequete(ListMGQuery,req,"");
    for (int i = 0; i < ListMGQuery.size(); i++)
    {
        ListMGQuery.seek(i);
        ui->MGupComboBox->insertItem(i, ListMGQuery.value(1).toString() + " " + ListMGQuery.value(2).toString(), ListMGQuery.value(0).toInt());
    }
}
