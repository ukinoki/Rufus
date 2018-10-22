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

#include "dlg_identificationcorresp.h"
#include "icons.h"
#include "ui_dlg_identificationcorresp.h"

dlg_identificationcorresp::dlg_identificationcorresp(QString CreationModification, bool quelesmedecins, int idCorresp, Procedures *procAPasser, QWidget *parent) :
    UpDialog(QDir::homePath() + NOMFIC_INI, "PositionsFiches/PositionIdentCorrespondant", parent),
    ui(new Ui::dlg_identificationcorresp)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    proc                = procAPasser;
    db                  = DataBase::getInstance()->getDataBase();
    gidCor              = idCorresp;
    lCreatModif         = CreationModification;
    OnlyDoctors         = quelesmedecins;
    VilleCPwidg     = new VilleCPWidget(proc->getVilles(), ui->Principalframe, NOM_ALARME);
    CPlineEdit      = VilleCPwidg->ui->CPlineEdit;
    VillelineEdit   = VilleCPwidg->ui->VillelineEdit;
    VilleCPwidg     ->move(10,224);
    gNomCor         = "";
    gPrenomCor      = "";
    modif           = false;

    QVBoxLayout *globallay  = dynamic_cast<QVBoxLayout*>(layout());
    globallay               ->insertWidget(0,ui->SpegroupBox);
    globallay               ->insertWidget(0,ui->Principalframe);
    AjouteLayButtons(UpDialog::ButtonCancel | UpDialog::ButtonOK);;
    AjouteWidgetLayButtons(ui->idDossierlabel, false);
    globallay               ->setSizeConstraint(QLayout::SetFixedSize);
    QFont font = qApp->font();
    font.setBold(true);
    ui->PrenomlineEdit->setFont(font);
    ui->NomlineEdit->setFont(font);

    setWindowIcon(Icons::icDoctor());
    ReconstruitListeSpecialites();
    connect(VilleCPwidg, &VilleCPWidget::villecpmodified, this, &dlg_identificationcorresp::Slot_EnableOKpushButton);

    AfficheDossierAlOuverture();
    if (lCreatModif == "Creation")
    {
        ui->SpecomboBox     ->setVisible(false);
        ui->AutreupLineEdit ->setVisible(false);
        ui->idDossierlabel  ->setVisible(false);
    }

    ui->NomlineEdit         ->setValidator(new QRegExpValidator(Utils::rgx_rx,this));
    ui->PrenomlineEdit      ->setValidator(new QRegExpValidator(Utils::rgx_rx,this));
    ui->MaillineEdit        ->setValidator(new QRegExpValidator(Utils::rgx_mail,this));
    ui->Adresse1lineEdit    ->setValidator(new QRegExpValidator(Utils::rgx_adresse,this));
    ui->Adresse2lineEdit    ->setValidator(new QRegExpValidator(Utils::rgx_adresse,this));
    ui->Adresse3lineEdit    ->setValidator(new QRegExpValidator(Utils::rgx_adresse,this));
    ui->TellineEdit         ->setValidator(new QRegExpValidator(Utils::rgx_telephone,this));
    ui->PortablelineEdit    ->setValidator(new QRegExpValidator(Utils::rgx_telephone,this));

    QList <QWidget *> listtab;
    listtab << ui->NomlineEdit << ui->PrenomlineEdit << ui->MradioButton << ui->FradioButton << ui->Adresse1lineEdit << ui->Adresse2lineEdit << ui->Adresse3lineEdit << CPlineEdit << VillelineEdit
            << ui->TellineEdit << ui->PortablelineEdit << ui->MaillineEdit << ui->FaxlineEdit;
    for (int i = 0; i<listtab.size()-1 ; i++ )
        setTabOrder(listtab.at(i), listtab.at(i+1));

    installEventFilter(this);
    ui->NomlineEdit->setFocus();

    connect (OKButton,                  SIGNAL(clicked()),                  this,           SLOT (Slot_OKpushButtonClicked()));

    connect (ui->NomlineEdit,           SIGNAL(editingFinished()),          this,           SLOT (Slot_Majuscule()));
    connect (ui->PrenomlineEdit,        SIGNAL(editingFinished()),          this,           SLOT (Slot_Majuscule()));
    connect (ui->Adresse1lineEdit,      SIGNAL(editingFinished()),          this,           SLOT (Slot_Majuscule()));
    connect (ui->Adresse2lineEdit,      SIGNAL(editingFinished()),          this,           SLOT (Slot_Majuscule()));
    connect (ui->Adresse3lineEdit,      SIGNAL(editingFinished()),          this,           SLOT (Slot_Majuscule()));
    connect (ui->NomlineEdit,           SIGNAL(textEdited(QString)),        this,           SLOT (Slot_EnableOKpushButton()));
    connect (ui->PrenomlineEdit,        SIGNAL(textEdited(QString)),        this,           SLOT (Slot_EnableOKpushButton()));
    connect (ui->Adresse1lineEdit,      SIGNAL(textEdited(QString)),        this,           SLOT (Slot_EnableOKpushButton()));
    connect (ui->Adresse2lineEdit,      SIGNAL(textEdited(QString)),        this,           SLOT (Slot_EnableOKpushButton()));
    connect (ui->Adresse3lineEdit,      SIGNAL(textEdited(QString)),        this,           SLOT (Slot_EnableOKpushButton()));
    connect (ui->TellineEdit,           SIGNAL(textEdited(QString)),        this,           SLOT (Slot_EnableOKpushButton()));
    connect (ui->PortablelineEdit,      SIGNAL(textEdited(QString)),        this,           SLOT (Slot_EnableOKpushButton()));
    connect (ui->MaillineEdit,          SIGNAL(textEdited(QString)),        this,           SLOT (Slot_EnableOKpushButton()));
    connect (ui->AutreupLineEdit,       SIGNAL(textEdited(QString)),        this,           SLOT (Slot_EnableOKpushButton()));
    connect (ui->MradioButton,          SIGNAL(clicked()),                  this,           SLOT (Slot_EnableOKpushButton()));
    connect (ui->FradioButton,          SIGNAL(clicked()),                  this,           SLOT (Slot_EnableOKpushButton()));
    connect (ui->SpecomboBox,           SIGNAL(currentIndexChanged(int)),   this,           SLOT (Slot_EnableOKpushButton()));

    connect(ui->MGradioButton,          SIGNAL(toggled(bool)),              this,           SLOT (Slot_RegleAffichage()));
    connect(ui->SperadioButton,         SIGNAL(toggled(bool)),              this,           SLOT (Slot_RegleAffichage()));
    connect(ui->AutreradioButton,       SIGNAL(toggled(bool)),              this,           SLOT (Slot_RegleAffichage()));

    OKButton->setEnabled(false);
    OKButton->setText(tr("Enregistrer"));
    CancelButton->setText(tr("Annuler"));

    QSqlQuery quer("select distinct corautreprofession from " NOM_TABLE_CORRESPONDANTS " where corautreprofession is not NULL", db);
    if (quer.size()>0)
    {
        QStringList listprof;
        quer.first();
        for (int i=0; i<quer.size(); i++)
            listprof << quer.value(0).toString();
        QCompleter *profcpl = new QCompleter(listprof);
        profcpl             ->setCaseSensitivity(Qt::CaseInsensitive);
        profcpl             ->setCompletionMode(QCompleter::InlineCompletion);
        ui->AutreupLineEdit->setCompleter(profcpl);
    }
}

dlg_identificationcorresp::~dlg_identificationcorresp()
{
}

/*-----------------------------------------------------------------------------------------------------------------
-- Traitement des SLOTS / actions associees a chaque objet du formulaire et aux menus -----------------------------
-----------------------------------------------------------------------------------------------------------------*/

void    dlg_identificationcorresp::Slot_EnableOKpushButton()
{
    OKButton->setEnabled(true);
    OKButton->setShortcut(QKeySequence("Meta+Return"));
}

void dlg_identificationcorresp::Slot_Majuscule()
{
    QLineEdit* Line = static_cast<QLineEdit*>(sender());
    if (Line->text() != proc->MajusculePremiereLettre(Line->text()))
    {
        Line->setText(proc->MajusculePremiereLettre(Line->text(),false));
        OKButton->setEnabled(true);
    }
}

void    dlg_identificationcorresp::Slot_OKpushButtonClicked()
{
    QString CorNom, CorPrenom;
    CorNom      = proc->CorrigeApostrophe(proc->MajusculePremiereLettre(ui->NomlineEdit->text(),true));
    CorPrenom   = proc->CorrigeApostrophe(proc->MajusculePremiereLettre(ui->PrenomlineEdit->text(),true));


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
    // B - On vérifie ensuite que la saisie est complète
    if (CorNom == "")
    {
        UpMessageBox::Watch(this,tr("Vous devez spécifier un nom!"));
        ui->NomlineEdit->setFocus();
        return;
    }
    if (CorPrenom == "")
    {
        UpMessageBox::Watch(this,tr("Vous devez spécifier un prénom!"));
        ui->PrenomlineEdit->setFocus();
        return;
    }

    if (!ui->MGradioButton->isChecked() && !ui->SperadioButton->isChecked() && !ui->AutreradioButton->isChecked())
    {
        UpMessageBox::Watch(this,tr("Vous devez spécifier l'activité de ce correspondant en cochant une des cases\n") +
                                    ui->MGradioButton->text() + ", " + ui->SperadioButton->text() + tr(" ou ") + ui->AutreradioButton->text());
        ui->MGradioButton->setFocus();
        return;
    }

    // D - On vérifie ensuite si ce correspondant existe déjà
    QString requete = "select idcor, corspecialite, cormedecin from " NOM_TABLE_CORRESPONDANTS
            " where CorNom LIKE '" + CorNom + "%' and CorPrenom LIKE '" + CorPrenom + "%'";
    QSqlQuery IdentCorQuery (requete,db);
    if (DataBase::getInstance()->traiteErreurRequete(IdentCorQuery,requete, "Impossible d'interroger la table des patients!"))
    {
        reject();
        return;
    }

    if (IdentCorQuery.size() > 0)
    {
        IdentCorQuery.first();
        if (IdentCorQuery.value(0).toInt() != gidCor)
        {
            UpMessageBox::Watch(this,tr("Ce correspondant existe déjà!"));
            gidCor = IdentCorQuery.value(0).toInt();
            if (OnlyDoctors)
                OnlyDoctors = (IdentCorQuery.value(2).toInt()==1);
            OKButton->setEnabled(false);
            lCreatModif = "Modification";
            AfficheDossierAlOuverture();
            disconnect (OKButton, SIGNAL(clicked()), this, SLOT (Slot_OKpushButtonClicked()));
            connect(OKButton, SIGNAL(clicked(bool)),this,SLOT(accept()));
            QList<QLineEdit *> listline = findChildren<QLineEdit *>();
            for (int i = 0;i<listline.size();i++) listline.at(i)->setEnabled(false);
            ui->SpegroupBox->setEnabled(false);
            ui->SexegroupBox->setEnabled(false);
            return;
        }
    }

    if (lCreatModif == "Creation")
    {
        // D - le dossier n'existe pas, on le crée
        QString gSexeCor = "";
        if (ui->MradioButton->isChecked()) gSexeCor = "M";
        if (ui->FradioButton->isChecked()) gSexeCor = "F";
        QString insrequete = "INSERT INTO " NOM_TABLE_CORRESPONDANTS
                " (CORNom, CORPrenom, CorSexe, CORAdresse1, CORAdresse2, CORAdresse3, CorVille, CorCodePostal,"
                " CorTelephone, CorPortable, CorFax, CorMail,CorMedecin,CorSpecialite,CorAutreProfession) "
                " VALUES "
                " ('" + CorNom + "', '" + CorPrenom + "', '" + gSexeCor
                + "', '" + proc->CorrigeApostrophe(proc->MajusculePremiereLettre(ui->Adresse1lineEdit->text(),true))
                + "', '" + proc->CorrigeApostrophe(proc->MajusculePremiereLettre(ui->Adresse2lineEdit->text(),true))
                + "', '" + proc->CorrigeApostrophe(proc->MajusculePremiereLettre(ui->Adresse3lineEdit->text(),true))
                + "', '" + proc->CorrigeApostrophe(proc->MajusculePremiereLettre(VillelineEdit->text(),true))
                + "', '" + CPlineEdit->text()
                + "', '" + ui->TellineEdit->text()
                + "', '" + ui->PortablelineEdit->text()
                + "', '" + ui->FaxlineEdit->text()
                + "', '" + proc->CorrigeApostrophe(ui->MaillineEdit->text());
        if (ui->MGradioButton->isChecked())
            insrequete += "',1, 0,null);";
        else if (ui->SperadioButton->isChecked())
            insrequete += "',1, " + ui->SpecomboBox->currentData().toString() + ",null);";
        else if (ui->AutreradioButton->isChecked())
            insrequete += "',null,null,'" + proc->CorrigeApostrophe(ui->AutreupLineEdit->text()) + "');";
        QSqlQuery InsertCorQuery (insrequete,db);
        if (DataBase::getInstance()->traiteErreurRequete(InsertCorQuery,insrequete,tr("Impossible de créer le dossier")))
            reject();
        insrequete = "select max(idcor) from " NOM_TABLE_CORRESPONDANTS;
        QSqlQuery CorQuery (insrequete,db);
        if (CorQuery.seek(0))
            gidCor = CorQuery.value(0).toInt();
        proc->initListeCorrespondants();
        accept();
    }
    if (lCreatModif == "Modification")
    {
        // D - Il n'existe pas de dossier similaire, on le modifie
        QString gSexeCor = "";
        if (ui->MradioButton->isChecked()) gSexeCor = "M";
        if (ui->FradioButton->isChecked()) gSexeCor = "F";
        QString Modifrequete = "update " NOM_TABLE_CORRESPONDANTS
                " set CorNom = '" + CorNom + "', CorPrenom = '" + CorPrenom + "', CorSexe = '" + gSexeCor
                + "', CorAdresse1 = '" + proc->CorrigeApostrophe(proc->MajusculePremiereLettre(ui->Adresse1lineEdit->text(),true))
                + "', CorAdresse2 = '" + proc->CorrigeApostrophe(proc->MajusculePremiereLettre(ui->Adresse2lineEdit->text(),true))
                + "', CorAdresse3 = '" + proc->CorrigeApostrophe(proc->MajusculePremiereLettre(ui->Adresse3lineEdit->text(),true))
                + "', CorVille = '" + proc->CorrigeApostrophe(proc->MajusculePremiereLettre(VillelineEdit->text(),true))
                + "', CorCodePostal = '" + CPlineEdit->text()
                + "', CorTelephone = '" + ui->TellineEdit->text()
                + "', CorPortable = '" + ui->PortablelineEdit->text()
                + "', CorFax = '" + ui->FaxlineEdit->text()
                + "', CorMail = '" + proc->CorrigeApostrophe(ui->MaillineEdit->text()) + "'";
        if (ui->MGradioButton->isChecked())
            Modifrequete += ", CorMedecin = 1, CorSpecialite = 0, CorautreProfession = NULL";
        else if (ui->SperadioButton->isChecked())
            Modifrequete += ", CorMedecin = 1, CorSpecialite = " + ui->SpecomboBox->currentData().toString() + ", CorautreProfession = NULL";
        else if (ui->AutreradioButton->isChecked())
            Modifrequete += ", CorMedecin = 1, CorSpecialite = null, CorautreProfession = '" + proc->CorrigeApostrophe(proc->MajusculePremiereLettre(ui->AutreupLineEdit->text())) + "'";
        Modifrequete += " where idCor =" + QString::number(gidCor);
        //qDebug() <<  Modifrequete;
        QSqlQuery ModifCorQuery (Modifrequete,db);
        DataBase::getInstance()->traiteErreurRequete(ModifCorQuery,Modifrequete,tr("Impossible de modifier le dossier"));
        if (CorNom != gNomCor || CorPrenom != gPrenomCor)
            modif = true;
        proc->initListeCorrespondants();
        accept();
    }
}

bool dlg_identificationcorresp::IdentModified()
{
    return modif;
}

void dlg_identificationcorresp::Slot_RegleAffichage()
{
    ui->SpecomboBox->setVisible(ui->SperadioButton->isChecked());
    ui->AutreupLineEdit->setVisible(ui->AutreradioButton->isChecked());
    OKButton->setEnabled(true);
}

/*-------------------------------------------------------------------------------------
 Interception du focusOut
-------------------------------------------------------------------------------------*/
bool dlg_identificationcorresp::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress && !obj->inherits("QPushButton") )
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        // Return ou Enter - On va au Tab Suivant -----------------------------------------------------------------------------------------------------------------------------
        if ((keyEvent->key()==Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) && keyEvent->modifiers() == Qt::NoModifier)
            return QWidget::focusNextChild();
    }
    return QWidget::eventFilter(obj, event);
}


/*--------------------------------------------------------------------------------------------
-- Afficher la fiche du correspondant
--------------------------------------------------------------------------------------------*/
void dlg_identificationcorresp::AfficheDossierAlOuverture()
{
    ui->AutreradioButton->setVisible(!OnlyDoctors);
    ui->AutreupLineEdit   ->setVisible(!OnlyDoctors);
    if (lCreatModif == "Modification")
    {
        QString requete = "SELECT idCor, CORNom, CORPrenom, CorSexe, CORAdresse1, CORAdresse2, CORAdresse3, CorVille, CorCodePostal,"
                          " CorTelephone, CorPortable, CorFax, CorMail, CorMedecin, CorSpecialite, CorAutreProfession FROM " NOM_TABLE_CORRESPONDANTS
                            " WHERE idCor = " + QString::number(gidCor);
        if (OnlyDoctors)
            requete += " and CorMedecin = 1";
        QSqlQuery AfficheDossierQuery (requete,db);
        if (DataBase::getInstance()->traiteErreurRequete(AfficheDossierQuery,requete,tr("Impossible de retrouver le dossier de ce correspondant")))
            return;
        if (AfficheDossierQuery.size() == 0)           // Aucune mesure trouvee pour ces criteres
            return;
        AfficheDossierQuery     .first();
        //UpMessageBox::Watch(this,requete);
        gNomCor                 = AfficheDossierQuery.value(1).toString();
        gPrenomCor              = AfficheDossierQuery.value(2).toString();
        ui->NomlineEdit         ->setText(gNomCor);
        ui->PrenomlineEdit      ->setText(gPrenomCor);
        // pour decocher les 2 radiobutton sexe il faut d'abord leur retirer la propriétét AutoExclusive
        ui->MradioButton        ->setAutoExclusive(false);
        ui->FradioButton        ->setAutoExclusive(false);
        ui->MradioButton        ->setChecked(false);
        ui->FradioButton        ->setChecked(false);
        ui->MradioButton        ->setAutoExclusive(true);
        ui->FradioButton        ->setAutoExclusive(true);
        Sexe                    = AfficheDossierQuery.value(3).toString();
        if (Sexe == "M") ui->MradioButton->setChecked(true);
        if (Sexe == "F") ui->FradioButton->setChecked(true);
        ui->idDossierlabel      ->setText(tr("Correspondant n° ") + QString::number(gidCor));

        ui->Adresse1lineEdit    ->setText(AfficheDossierQuery.value(4).toString());
        ui->Adresse2lineEdit    ->setText(AfficheDossierQuery.value(5).toString());
        ui->Adresse3lineEdit    ->setText(AfficheDossierQuery.value(6).toString());
        QString CP              = AfficheDossierQuery.value(8).toString();
        CPlineEdit              ->completer()->setCurrentRow(VilleCPwidg->villes()->getListCodePostal().indexOf(CP)); // ce micmac est nécessaire à cause d'un bug de QCompleter en mode InLineCompletion
                                                                                                // il faut synchroniser à la main le QCompleter et le QlineEdit au premier affichage
        CPlineEdit              ->setText(CP);
        VillelineEdit           ->setText(AfficheDossierQuery.value(7).toString());
        ui->TellineEdit         ->setText(AfficheDossierQuery.value(9).toString());
        ui->PortablelineEdit    ->setText(AfficheDossierQuery.value(10).toString());
        ui->MaillineEdit        ->setText(AfficheDossierQuery.value(12).toString());
        if (!AfficheDossierQuery.value(14).isNull()
                && AfficheDossierQuery.value(14).toInt() == 0)                                  //c'est un généraliste
        {
            ui->MGradioButton   ->setChecked(true);
            ui->SpecomboBox     ->setVisible(false);
            ui->AutreupLineEdit ->setVisible(false);
        }
        else if (AfficheDossierQuery.value(13).toInt() == 1)                                    // ce n'est pas un généralliste mais un spécialiste
        {
            ui->SperadioButton  ->setChecked(true);
            ui->SpecomboBox     ->setVisible(true);
            int idx             = ui->SpecomboBox->findData(AfficheDossierQuery.value(14).toInt());
            ui->SpecomboBox     ->setCurrentIndex(idx);
            ui->AutreupLineEdit ->setVisible(false);
        }
        else                                                                                    // ce n'est pas un médecin
        {
            ui->SpecomboBox     ->setVisible(false);
            ui->AutreradioButton->setChecked(true);
            ui->AutreupLineEdit ->setVisible(true);
            ui->AutreupLineEdit ->setText(AfficheDossierQuery.value(15).toString());
        }
    }
    else if (lCreatModif == "Creation")
    {
        CPlineEdit              ->setText(proc->getCodePostalParDefaut());
        VillelineEdit           ->setText(proc->getVilleParDefaut());
    }
}

void dlg_identificationcorresp::ReconstruitListeSpecialites()
{
    ui->SpecomboBox->clear();
    QStringList ListSpec;
    QString req = "SELECT idspecialite, nomspecialite FROM " NOM_TABLE_SPECIALITES " order by nomspecialite";
    QSqlQuery ListSpecQuery (req,db);
    DataBase::getInstance()->traiteErreurRequete(ListSpecQuery,req,"");
    for (int i = 0; i < ListSpecQuery.size(); i++)
    {
        ListSpecQuery.seek(i);
        ListSpec << ListSpecQuery.value(1).toString();
        ui->SpecomboBox->insertItem(i, ListSpecQuery.value(1).toString(), ListSpecQuery.value(0).toInt());
    }
    QCompleter *speccompl   = new QCompleter(ListSpec);
    speccompl               ->setCaseSensitivity(Qt::CaseInsensitive);
    speccompl               ->setCompletionMode(QCompleter::InlineCompletion);
    ui->SpecomboBox         ->setCompleter(speccompl);
}

