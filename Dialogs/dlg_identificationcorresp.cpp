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

#include "dlg_identificationcorresp.h"
#include "icons.h"
#include "ui_dlg_identificationcorresp.h"

dlg_identificationcorresp::dlg_identificationcorresp(Mode mode, bool quelesmedecins, Correspondant *cor, QWidget *parent) :
    UpDialog(QDir::homePath() + FILE_INI, "PositionsFiches/PositionIdentCorrespondant", parent),
    ui(new Ui::dlg_identificationcorresp)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    db                  = DataBase::I();
    m_correspondant      = Q_NULLPTR;
    if (cor != Q_NULLPTR)
    {
        m_correspondant  = cor;
        if (!m_correspondant->isallloaded())
            Datas::I()->correspondants->loadAllData(m_correspondant);
    }
    gMode               = mode;
    OnlyDoctors         = quelesmedecins;
    VilleCPwidg     = new VilleCPWidget(Datas::I()->villes, ui->Principalframe);
    CPlineEdit      = VilleCPwidg->ui->CPlineEdit;
    VillelineEdit   = VilleCPwidg->ui->VillelineEdit;
    VilleCPwidg     ->move(10,224);
    gNomCor         = "";
    gPrenomCor      = "";
    modif           = false;

    dlglayout()     ->insertWidget(0,ui->SpegroupBox);
    dlglayout()     ->insertWidget(0,ui->Principalframe);
    AjouteLayButtons(UpDialog::ButtonCancel | UpDialog::ButtonOK);;
    AjouteWidgetLayButtons(ui->idDossierlabel, false);
    dlglayout()     ->setSizeConstraint(QLayout::SetFixedSize);
    QFont font = qApp->font();
    font.setBold(true);
    ui->PrenomlineEdit->setFont(font);
    ui->NomlineEdit->setFont(font);

    setWindowIcon(Icons::icDoctor());
    ReconstruitListeSpecialites();
    connect(VilleCPwidg, &VilleCPWidget::villecpmodified, this, &dlg_identificationcorresp::Slot_EnableOKpushButton);

    AfficheDossierAlOuverture();
    if (gMode == Creation)
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
    ui->MaillineEdit->installEventFilter(this);
    ui->NomlineEdit->setFocus();

    OKButton->disconnect();
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
    setStageCount(1);

    QCompleter *profcpl = new QCompleter(Datas::I()->correspondants->autresprofessions(), this);
    profcpl             ->setCaseSensitivity(Qt::CaseInsensitive);
    profcpl             ->setCompletionMode(QCompleter::InlineCompletion);
    ui->AutreupLineEdit->setCompleter(profcpl);
}

dlg_identificationcorresp::~dlg_identificationcorresp()
{
}

/*-----------------------------------------------------------------------------------------------------------------
-- Traitement des SLOTS / actions associees a chaque objet du formulaire et aux menus -----------------------------
-----------------------------------------------------------------------------------------------------------------*/

void    dlg_identificationcorresp::Slot_EnableOKpushButton()
{
    bool a  = ui->NomlineEdit->text() != ""
           && ui->PrenomlineEdit->text() != ""
           && (ui->MradioButton->isChecked() || ui->FradioButton->isChecked())
           && CPlineEdit->text() != ""
           && VillelineEdit->text() != "";
    OKButton->setEnabled(a);
    OKButton->setShortcut(a? QKeySequence("Meta+Return") : QKeySequence());
}

void dlg_identificationcorresp::Slot_Majuscule()
{
    QLineEdit* Line = static_cast<QLineEdit*>(sender());
    Line->setText(Utils::trimcapitilize(Line->text(),false));
    OKButton->setEnabled(true);
}

void    dlg_identificationcorresp::Slot_OKpushButtonClicked()
{
    QString CorNom, CorPrenom;
    CorNom      = Utils::correctquoteSQL(Utils::trimcapitilize(ui->NomlineEdit->text(),true));
    CorPrenom   = Utils::correctquoteSQL(Utils::trimcapitilize(ui->PrenomlineEdit->text(),true));

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
    bool ok;
    QString requete = "select idcor, corspecialite, cormedecin from " TBL_CORRESPONDANTS
            " where CorNom LIKE '" + CorNom + "%' and CorPrenom LIKE '" + CorPrenom + "%'";
    QVariantList cordata = db->getFirstRecordFromStandardSelectSQL(requete,ok, tr("Impossible d'interroger la table des correspondants!"));
    if (!ok)
    {
        reject();
        return;
    }

    if (cordata.size() > 0)
    {
        if (cordata.at(0).toInt() != m_correspondant->id())
        {
            UpMessageBox::Watch(this,tr("Ce correspondant existe déjà!"));
            m_correspondant->setid(cordata.at(0).toInt());
            if (OnlyDoctors)
                OnlyDoctors = (cordata.at(2).toInt()==1);
            OKButton->setEnabled(false);
            gMode = Modification;
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
    int idcor = 0;

    if (gMode == Creation)
    {
        // D - le dossier n'existe pas, on le crée
        QString gSexeCor = "";
        if (ui->MradioButton->isChecked()) gSexeCor = "M";
        if (ui->FradioButton->isChecked()) gSexeCor = "F";
        db->locktables(QStringList() << TBL_CORRESPONDANTS);
        QString insrequete = "INSERT INTO " TBL_CORRESPONDANTS
                " (CORNom, CORPrenom, CorSexe, CORAdresse1, CORAdresse2, CORAdresse3, CorVille, CorCodePostal,"
                " CorTelephone, CorPortable, CorFax, CorMail,CorMedecin,CorSpecialite,CorAutreProfession) "
                " VALUES "
                " ('" + CorNom + "', '" + CorPrenom + "', '" + gSexeCor
                + "', '" + Utils::correctquoteSQL(Utils::trimcapitilize(ui->Adresse1lineEdit->text(),true))
                + "', '" + Utils::correctquoteSQL(Utils::trimcapitilize(ui->Adresse2lineEdit->text(),true))
                + "', '" + Utils::correctquoteSQL(Utils::trimcapitilize(ui->Adresse3lineEdit->text(),true))
                + "', '" + Utils::correctquoteSQL(Utils::trimcapitilize(VillelineEdit->text(),true))
                + "', '" + CPlineEdit->text()
                + "', '" + ui->TellineEdit->text()
                + "', '" + ui->PortablelineEdit->text()
                + "', '" + ui->FaxlineEdit->text()
                + "', '" + Utils::correctquoteSQL(ui->MaillineEdit->text());
        if (ui->MGradioButton->isChecked())
            insrequete += "',1, 0,null);";
        else if (ui->SperadioButton->isChecked())
            insrequete += "',1, " + ui->SpecomboBox->currentData().toString() + ",null);";
        else if (ui->AutreradioButton->isChecked())
            insrequete += "',null,null,'" + Utils::correctquoteSQL(ui->AutreupLineEdit->text()) + "');";
        if (!db->StandardSQL(insrequete,tr("Impossible de créer le dossier")))
            reject();
        insrequete = "select max(idcor) from " TBL_CORRESPONDANTS;
        QVariantList cordata = db->getFirstRecordFromStandardSelectSQL(insrequete, ok);
        if (ok && cordata.size()>0)
            idcor = cordata.at(0).toInt();
        db->unlocktables();
        m_correspondant = new Correspondant();
        m_correspondant->setid(idcor);
    }
    else if (gMode == Modification)
    {
        idcor = m_correspondant->id();
        // D - Il n'existe pas de dossier similaire, on le modifie
        QString gSexeCor = "";
        if (ui->MradioButton->isChecked()) gSexeCor = "M";
        if (ui->FradioButton->isChecked()) gSexeCor = "F";
        QString Modifrequete = "update " TBL_CORRESPONDANTS
                " set CorNom = '" + CorNom + "', CorPrenom = '" + CorPrenom + "', CorSexe = '" + gSexeCor
                + "', CorAdresse1 = '" + Utils::correctquoteSQL(Utils::trimcapitilize(ui->Adresse1lineEdit->text(),true))
                + "', CorAdresse2 = '" + Utils::correctquoteSQL(Utils::trimcapitilize(ui->Adresse2lineEdit->text(),true))
                + "', CorAdresse3 = '" + Utils::correctquoteSQL(Utils::trimcapitilize(ui->Adresse3lineEdit->text(),true))
                + "', CorVille = '" + Utils::correctquoteSQL(Utils::trimcapitilize(VillelineEdit->text(),true))
                + "', CorCodePostal = '" + CPlineEdit->text()
                + "', CorTelephone = '" + ui->TellineEdit->text()
                + "', CorPortable = '" + ui->PortablelineEdit->text()
                + "', CorFax = '" + ui->FaxlineEdit->text()
                + "', CorMail = '" + Utils::correctquoteSQL(ui->MaillineEdit->text()) + "'";
        if (ui->MGradioButton->isChecked())
            Modifrequete += ", CorMedecin = 1, CorSpecialite = 0, CorautreProfession = NULL";
        else if (ui->SperadioButton->isChecked())
            Modifrequete += ", CorMedecin = 1, CorSpecialite = " + ui->SpecomboBox->currentData().toString() + ", CorautreProfession = NULL";
        else if (ui->AutreradioButton->isChecked())
            Modifrequete += ", CorMedecin = 1, CorSpecialite = null, CorautreProfession = '" + Utils::correctquoteSQL(Utils::trimcapitilize(ui->AutreupLineEdit->text())) + "'";
        Modifrequete += " where idCor =" + QString::number(idcor);
        //qDebug() <<  Modifrequete;
        db->StandardSQL(Modifrequete, tr("Impossible de modifier le dossier"));
    }
    modif = true;
    Datas::I()->correspondants->loadAllData(m_correspondant);
    Flags::I()->MAJflagCorrespondants();
    accept();
}

bool dlg_identificationcorresp::identcorrespondantmodifiee()
{
    return modif;
}

Correspondant* dlg_identificationcorresp::correspondantrenvoye()
{
    return m_correspondant;
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
    if (event->type() == QEvent::FocusOut)
    {
        if (obj == ui->MaillineEdit)
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
    if (gMode == Modification)
    {
        gNomCor                 = m_correspondant->nom();
        gPrenomCor              = m_correspondant->prenom();
        ui->NomlineEdit         ->setText(gNomCor);
        ui->PrenomlineEdit      ->setText(gPrenomCor);
        // pour decocher les 2 radiobutton sexe il faut d'abord leur retirer la propriétét AutoExclusive
        ui->MradioButton        ->setAutoExclusive(false);
        ui->FradioButton        ->setAutoExclusive(false);
        ui->MradioButton        ->setChecked(false);
        ui->FradioButton        ->setChecked(false);
        ui->MradioButton        ->setAutoExclusive(true);
        ui->FradioButton        ->setAutoExclusive(true);
        Sexe                    = m_correspondant->sexe();
        if (Sexe == "M") ui->MradioButton->setChecked(true);
        if (Sexe == "F") ui->FradioButton->setChecked(true);
        ui->idDossierlabel      ->setText(tr("Correspondant n° ") + QString::number(m_correspondant->id()));

        ui->Adresse1lineEdit    ->setText(m_correspondant->adresse1());
        ui->Adresse2lineEdit    ->setText(m_correspondant->adresse2());
        ui->Adresse3lineEdit    ->setText(m_correspondant->adresse3());
        QString CP              = m_correspondant->codepostal();
        CPlineEdit              ->completer()->setCurrentRow(VilleCPwidg->villes()->getListCodePostal().indexOf(CP)); // ce micmac est nécessaire à cause d'un bug de QCompleter en mode InLineCompletion
                                                                                                // il faut synchroniser à la main le QCompleter et le QlineEdit au premier affichage
        CPlineEdit              ->setText(CP);
        VillelineEdit           ->setText(m_correspondant->ville());
        ui->TellineEdit         ->setText(m_correspondant->telephone());
        ui->PortablelineEdit    ->setText(m_correspondant->portable());
        ui->MaillineEdit        ->setText(m_correspondant->mail());
        if (m_correspondant->isMG())                                  //c'est un généraliste
        {
            ui->MGradioButton   ->setChecked(true);
            ui->SpecomboBox     ->setVisible(false);
            ui->AutreupLineEdit ->setVisible(false);
        }
        else if (m_correspondant->ismedecin())                                    // ce n'est pas un généralliste mais un spécialiste
        {
            ui->SperadioButton  ->setChecked(true);
            ui->SpecomboBox     ->setVisible(true);
            int idx             = ui->SpecomboBox->findData(m_correspondant->idspecialite());
            ui->SpecomboBox     ->setCurrentIndex(idx);
            ui->AutreupLineEdit ->setVisible(false);
        }
        else                                                                                    // ce n'est pas un médecin
        {
            ui->SpecomboBox     ->setVisible(false);
            ui->AutreradioButton->setChecked(true);
            ui->AutreupLineEdit ->setVisible(true);
            ui->AutreupLineEdit ->setText(m_correspondant->metier());
        }
    }
    else if (gMode == Creation)
    {
        CPlineEdit              ->setText(Procedures::CodePostalParDefaut());
        VillelineEdit           ->setText(Procedures::VilleParDefaut());
    }
}

void dlg_identificationcorresp::ReconstruitListeSpecialites()
{
    bool ok;
    ui->SpecomboBox->clear();
    QStringList ListSpec;
    QString req = "SELECT idspecialite, nomspecialite FROM " TBL_SPECIALITES " order by nomspecialite";
    QList<QVariantList> speclist = db->StandardSelectSQL(req,ok);
    if (!ok) return;
    for (int i = 0; i < speclist.size(); i++)
    {
        ListSpec << speclist.at(i).at(1).toString();
        ui->SpecomboBox->insertItem(i, speclist.at(i).at(1).toString(), speclist.at(i).at(0).toInt());
    }
    QCompleter *speccompl   = new QCompleter(ListSpec, this);
    speccompl               ->setCaseSensitivity(Qt::CaseInsensitive);
    speccompl               ->setCompletionMode(QCompleter::InlineCompletion);
    ui->SpecomboBox         ->setCompleter(speccompl);
}

