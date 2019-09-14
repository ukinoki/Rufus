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
    if (cor != Q_NULLPTR)
    {
        m_correspondant  = cor;
        if (!m_correspondant->isallloaded())
            Datas::I()->correspondants->loadAllData(m_correspondant);
    }
    m_mode               = mode;
    m_onlydoctors         = quelesmedecins;
    wdg_villeCP     = new VilleCPWidget(Datas::I()->villes, ui->Principalframe);
    wdg_CPlineedit      = wdg_villeCP->ui->CPlineEdit;
    wdg_villelineedit   = wdg_villeCP->ui->VillelineEdit;
    wdg_villeCP     ->move(10,224);
    m_nomcor         = "";
    m_prenomcor      = "";
    m_modifdatascor           = false;

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
    connect(wdg_villeCP, &VilleCPWidget::villecpmodified, this, &dlg_identificationcorresp::EnableOKpushButton);

    AfficheDossierAlOuverture();
    if (m_mode == Creation)
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
    listtab << ui->NomlineEdit << ui->PrenomlineEdit << ui->MradioButton << ui->FradioButton << ui->Adresse1lineEdit << ui->Adresse2lineEdit << ui->Adresse3lineEdit << wdg_CPlineedit << wdg_villelineedit
            << ui->TellineEdit << ui->PortablelineEdit << ui->MaillineEdit << ui->FaxlineEdit;
    for (int i = 0; i<listtab.size()-1 ; i++ )
        setTabOrder(listtab.at(i), listtab.at(i+1));

    installEventFilter(this);
    ui->MaillineEdit->installEventFilter(this);
    ui->NomlineEdit->setFocus();

    OKButton->disconnect();
    connect (OKButton,                  &QPushButton::clicked,              this,           &dlg_identificationcorresp::OKpushButtonClicked);
    connect (ui->NomlineEdit,           &QLineEdit::editingFinished,        this,           [=] {Majuscule(ui->NomlineEdit);});
    connect (ui->PrenomlineEdit,        &QLineEdit::editingFinished,        this,           [=] {Majuscule(ui->PrenomlineEdit);});
    connect (ui->Adresse1lineEdit,      &QLineEdit::editingFinished,        this,           [=] {Majuscule(ui->Adresse1lineEdit);});
    connect (ui->Adresse2lineEdit,      &QLineEdit::editingFinished,        this,           [=] {Majuscule(ui->Adresse2lineEdit);});
    connect (ui->Adresse3lineEdit,      &QLineEdit::editingFinished,        this,           [=] {Majuscule(ui->Adresse3lineEdit);});
    connect (ui->NomlineEdit,           &QLineEdit::textEdited,             this,           &dlg_identificationcorresp::EnableOKpushButton);
    connect (ui->PrenomlineEdit,        &QLineEdit::textEdited,             this,           &dlg_identificationcorresp::EnableOKpushButton);
    connect (ui->Adresse1lineEdit,      &QLineEdit::textEdited,             this,           &dlg_identificationcorresp::EnableOKpushButton);
    connect (ui->Adresse2lineEdit,      &QLineEdit::textEdited,             this,           &dlg_identificationcorresp::EnableOKpushButton);
    connect (ui->Adresse3lineEdit,      &QLineEdit::textEdited,             this,           &dlg_identificationcorresp::EnableOKpushButton);
    connect (ui->TellineEdit,           &QLineEdit::textEdited,             this,           &dlg_identificationcorresp::EnableOKpushButton);
    connect (ui->PortablelineEdit,      &QLineEdit::textEdited,             this,           &dlg_identificationcorresp::EnableOKpushButton);
    connect (ui->MaillineEdit,          &QLineEdit::textEdited,             this,           &dlg_identificationcorresp::EnableOKpushButton);
    connect (ui->AutreupLineEdit,       &QLineEdit::textEdited,             this,           &dlg_identificationcorresp::EnableOKpushButton);
    connect (ui->MradioButton,          &QRadioButton::clicked,             this,           &dlg_identificationcorresp::EnableOKpushButton);
    connect (ui->FradioButton,          &QRadioButton::clicked,             this,           &dlg_identificationcorresp::EnableOKpushButton);
    connect (ui->SpecomboBox,           QOverload<int>::of(&QComboBox::currentIndexChanged),
                                                                            this,           &dlg_identificationcorresp::EnableOKpushButton);

    connect(ui->MGradioButton,          &QRadioButton::toggled,             this,           &dlg_identificationcorresp::RegleAffichage);
    connect(ui->SperadioButton,         &QRadioButton::toggled,             this,           &dlg_identificationcorresp::RegleAffichage);
    connect(ui->AutreradioButton,       &QRadioButton::toggled,             this,           &dlg_identificationcorresp::RegleAffichage);

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

void    dlg_identificationcorresp::EnableOKpushButton()
{
    bool a  = ui->NomlineEdit->text() != ""
           && ui->PrenomlineEdit->text() != ""
           && (ui->MradioButton->isChecked() || ui->FradioButton->isChecked())
           && wdg_CPlineedit->text() != ""
           && wdg_villelineedit->text() != "";
    OKButton->setEnabled(a);
    OKButton->setShortcut(a? QKeySequence("Meta+Return") : QKeySequence());
}

void dlg_identificationcorresp::Majuscule(QLineEdit *ledit)
{
    ledit->setText(Utils::trimcapitilize(ledit->text(),false));
    OKButton->setEnabled(true);
}

void    dlg_identificationcorresp::OKpushButtonClicked()
{
    QString CorNom, CorPrenom;
    CorNom      = Utils::correctquoteSQL(Utils::trimcapitilize(ui->NomlineEdit->text(),true));
    CorPrenom   = Utils::correctquoteSQL(Utils::trimcapitilize(ui->PrenomlineEdit->text(),true));

    if (wdg_CPlineedit->text() == "" && wdg_villelineedit->text() == "")
    {
        UpMessageBox::Watch(this,tr("Vous n'avez indiqué ni la ville ni le code postal!"));
        wdg_CPlineedit->setFocus();
        return;
    }

    if (wdg_CPlineedit->text() == "" || wdg_villelineedit->text() == "")
    {
        if (wdg_CPlineedit->text() == "")
        {
            UpMessageBox::Watch(this,tr("Il manque le code postal"));
            wdg_CPlineedit->setFocus();
            return;
        }
        if (wdg_villelineedit->text() == "")
        {
            UpMessageBox::Watch(this,tr("Il manque le nom de la ville"));
            wdg_villelineedit->setFocus();
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
            if (m_onlydoctors)
                m_onlydoctors = (cordata.at(2).toInt()==1);
            OKButton->setEnabled(false);
            m_mode = Modification;
            AfficheDossierAlOuverture();
            disconnect (OKButton,   &QPushButton::clicked,  this,   &dlg_identificationcorresp::OKpushButtonClicked);
            connect(OKButton,       &QPushButton::clicked,  this,   &dlg_identificationcorresp::accept);
            QList<QLineEdit *> listline = findChildren<QLineEdit *>();
            for (int i = 0;i<listline.size();i++) listline.at(i)->setEnabled(false);
            ui->SpegroupBox->setEnabled(false);
            ui->SexegroupBox->setEnabled(false);
            return;
        }
    }
    int idcor = 0;

    if (m_mode == Creation)
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
                + "', '" + Utils::correctquoteSQL(Utils::trimcapitilize(wdg_villelineedit->text(),true))
                + "', '" + wdg_CPlineedit->text()
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
    else if (m_mode == Modification)
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
                + "', CorVille = '" + Utils::correctquoteSQL(Utils::trimcapitilize(wdg_villelineedit->text(),true))
                + "', CorCodePostal = '" + wdg_CPlineedit->text()
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
    m_modifdatascor = true;
    Datas::I()->correspondants->loadAllData(m_correspondant);
    Flags::I()->MAJflagCorrespondants();
    accept();
}

bool dlg_identificationcorresp::identcorrespondantmodifiee() const
{
    return m_modifdatascor;
}

Correspondant* dlg_identificationcorresp::correspondantrenvoye() const
{
    return m_correspondant;
}

void dlg_identificationcorresp::RegleAffichage()
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
    ui->AutreradioButton->setVisible(!m_onlydoctors);
    ui->AutreupLineEdit   ->setVisible(!m_onlydoctors);
    if (m_mode == Modification)
    {
        m_nomcor                 = m_correspondant->nom();
        m_prenomcor              = m_correspondant->prenom();
        ui->NomlineEdit         ->setText(m_nomcor);
        ui->PrenomlineEdit      ->setText(m_prenomcor);
        // pour decocher les 2 radiobutton sexe il faut d'abord leur retirer la propriétét AutoExclusive
        ui->MradioButton        ->setAutoExclusive(false);
        ui->FradioButton        ->setAutoExclusive(false);
        ui->MradioButton        ->setChecked(false);
        ui->FradioButton        ->setChecked(false);
        ui->MradioButton        ->setAutoExclusive(true);
        ui->FradioButton        ->setAutoExclusive(true);
        m_sexecor                    = m_correspondant->sexe();
        if (m_sexecor == "M") ui->MradioButton->setChecked(true);
        if (m_sexecor == "F") ui->FradioButton->setChecked(true);
        ui->idDossierlabel      ->setText(tr("Correspondant n° ") + QString::number(m_correspondant->id()));

        ui->Adresse1lineEdit    ->setText(m_correspondant->adresse1());
        ui->Adresse2lineEdit    ->setText(m_correspondant->adresse2());
        ui->Adresse3lineEdit    ->setText(m_correspondant->adresse3());
        QString CP              = m_correspondant->codepostal();
        wdg_CPlineedit              ->completer()->setCurrentRow(wdg_villeCP->villes()->getListCodePostal().indexOf(CP)); // ce micmac est nécessaire à cause d'un bug de QCompleter en mode InLineCompletion
                                                                                                // il faut synchroniser à la main le QCompleter et le QlineEdit au premier affichage
        wdg_CPlineedit              ->setText(CP);
        wdg_villelineedit           ->setText(m_correspondant->ville());
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
    else if (m_mode == Creation)
    {
        wdg_CPlineedit              ->setText(Procedures::CodePostalParDefaut());
        wdg_villelineedit           ->setText(Procedures::VilleParDefaut());
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

