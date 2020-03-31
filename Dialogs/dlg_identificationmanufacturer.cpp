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

#include "dlg_identificationmanufacturer.h"
#include "icons.h"
#include "ui_dlg_identificationmanufacturer.h"

dlg_identificationmanufacturer::dlg_identificationmanufacturer(Mode mode, Manufacturer *man, QWidget *parent) :
    UpDialog(PATH_FILE_INI, "PositionsFiches/PositionIdentManuFacturer", parent),
    ui(new Ui::dlg_identificationmanufacturer)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    if (man != Q_NULLPTR)
        m_currentmanufacturer          = man;
    m_mode                      = mode;
    wdg_villeCP                 = new VilleCPWidget(Datas::I()->villes, ui->Principalframe);
    wdg_CPlineedit              = wdg_villeCP->ui->CPlineEdit;
    wdg_villelineedit           = wdg_villeCP->ui->VillelineEdit;
    wdg_villeCP                 ->move(10,156);
    m_nommanufacturer           = "";

    dlglayout()     ->insertWidget(0,ui->CorgroupBox);
    dlglayout()     ->insertWidget(0,ui->Principalframe);
    AjouteLayButtons(UpDialog::ButtonCancel | UpDialog::ButtonOK);
    AjouteWidgetLayButtons(ui->idManufacturerlabel, false);
    dlglayout()     ->setSizeConstraint(QLayout::SetFixedSize);
    QFont font = qApp->font();
    font.setBold(true);
    ui->NomlineEdit->setFont(font);

    setWindowIcon(Icons::icDoctor());
    connect(wdg_villeCP, &VilleCPWidget::villecpmodified, this, &dlg_identificationmanufacturer::EnableOKpushButton);

    AfficheDatasManufacturer();
    if (m_mode == Creation)
        ui->idManufacturerlabel  ->setVisible(false);

    ui->NomlineEdit         ->setValidator(new QRegExpValidator(Utils::rgx_rx,this));
    ui->MaillineEdit        ->setValidator(new QRegExpValidator(Utils::rgx_mail,this));
    ui->Adresse1lineEdit    ->setValidator(new QRegExpValidator(Utils::rgx_adresse,this));
    ui->Adresse2lineEdit    ->setValidator(new QRegExpValidator(Utils::rgx_adresse,this));
    ui->Adresse3lineEdit    ->setValidator(new QRegExpValidator(Utils::rgx_adresse,this));
    ui->TellineEdit         ->setValidator(new QRegExpValidator(Utils::rgx_telephone,this));
    ui->PortablelineEdit    ->setValidator(new QRegExpValidator(Utils::rgx_telephone,this));
    ui->CorNomlineEdit      ->setValidator(new QRegExpValidator(Utils::rgx_rx,this));
    ui->CorPrenomlineEdit   ->setValidator(new QRegExpValidator(Utils::rgx_rx,this));
    ui->CorStatutlineEdit   ->setValidator(new QRegExpValidator(Utils::rgx_rx,this));
    ui->CorMaillineEdit     ->setValidator(new QRegExpValidator(Utils::rgx_mail,this));
    ui->CorTelephonelineEdit->setValidator(new QRegExpValidator(Utils::rgx_telephone,this));

    installEventFilter(this);
    ui->MaillineEdit->installEventFilter(this);
    ui->NomlineEdit->setFocus();

    OKButton->disconnect();
    connect (OKButton,                  &QPushButton::clicked,              this,           &dlg_identificationmanufacturer::OKpushButtonClicked);
    connect (ui->NomlineEdit,           &UpLineEdit::TextModified,          this,           [=] {Majuscule(ui->NomlineEdit);});
    connect (ui->Adresse1lineEdit,      &UpLineEdit::TextModified,          this,           [=] {Majuscule(ui->Adresse1lineEdit);});
    connect (ui->Adresse2lineEdit,      &UpLineEdit::TextModified,          this,           [=] {Majuscule(ui->Adresse2lineEdit);});
    connect (ui->Adresse3lineEdit,      &UpLineEdit::TextModified,          this,           [=] {Majuscule(ui->Adresse3lineEdit);});
    connect (ui->ActifcheckBox,         &QCheckBox::clicked,                this,           &dlg_identificationmanufacturer::EnableOKpushButton);
    connect (ui->NomlineEdit,           &QLineEdit::textEdited,             this,           &dlg_identificationmanufacturer::EnableOKpushButton);
    connect (ui->Adresse1lineEdit,      &QLineEdit::textEdited,             this,           &dlg_identificationmanufacturer::EnableOKpushButton);
    connect (ui->Adresse2lineEdit,      &QLineEdit::textEdited,             this,           &dlg_identificationmanufacturer::EnableOKpushButton);
    connect (ui->Adresse3lineEdit,      &QLineEdit::textEdited,             this,           &dlg_identificationmanufacturer::EnableOKpushButton);
    connect (ui->TellineEdit,           &QLineEdit::textEdited,             this,           &dlg_identificationmanufacturer::EnableOKpushButton);
    connect (ui->PortablelineEdit,      &QLineEdit::textEdited,             this,           &dlg_identificationmanufacturer::EnableOKpushButton);
    connect (ui->MaillineEdit,          &QLineEdit::textEdited,             this,           &dlg_identificationmanufacturer::EnableOKpushButton);
    connect (ui->CorNomlineEdit,        &QLineEdit::textEdited,             this,           &dlg_identificationmanufacturer::EnableOKpushButton);
    connect (ui->CorPrenomlineEdit,     &QLineEdit::textEdited,             this,           &dlg_identificationmanufacturer::EnableOKpushButton);
    connect (ui->CorStatutlineEdit,     &QLineEdit::textEdited,             this,           &dlg_identificationmanufacturer::EnableOKpushButton);
    connect (ui->CorMaillineEdit,       &QLineEdit::textEdited,             this,           &dlg_identificationmanufacturer::EnableOKpushButton);
    connect (ui->CorTelephonelineEdit,  &QLineEdit::textEdited,             this,           &dlg_identificationmanufacturer::EnableOKpushButton);
    connect (ui->DelCorupPushButton,    &QPushButton::clicked,              this,           [=]
                                                                                            {
                                                                                                ui->CorNomlineEdit      ->clear();
                                                                                                ui->CorPrenomlineEdit   ->clear();
                                                                                                ui->CorStatutlineEdit   ->clear();
                                                                                                ui->CorMaillineEdit     ->clear();
                                                                                                ui->CorTelephonelineEdit->clear();
                                                                                                EnableOKpushButton();
                                                                                            });
    OKButton->setEnabled(false);
    OKButton->setText(tr("Enregistrer"));
    CancelButton->setText(tr("Annuler"));
    setStageCount(1);

    QCompleter *profcpl = new QCompleter(Datas::I()->correspondants->autresprofessions(), this);
    profcpl             ->setCaseSensitivity(Qt::CaseInsensitive);
    profcpl             ->setCompletionMode(QCompleter::InlineCompletion);
}

dlg_identificationmanufacturer::~dlg_identificationmanufacturer()
{
}

void    dlg_identificationmanufacturer:: EnableOKpushButton()
{
    bool a  = ui->NomlineEdit->text() != ""
           && wdg_CPlineedit->text() != ""
           && wdg_villelineedit->text() != "";
    OKButton->setEnabled(a);
    OKButton->setShortcut(a? QKeySequence("Meta+Return") : QKeySequence());
}

void dlg_identificationmanufacturer::Majuscule(QLineEdit *ledit)
{
    ledit->setText(Utils::trimcapitilize(ledit->text(),false));
    OKButton->setEnabled(true);
}

void    dlg_identificationmanufacturer::OKpushButtonClicked()
{
    QString ManNom;
    ManNom      = Utils::correctquoteSQL(Utils::trimcapitilize(ui->NomlineEdit->text(),true));

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
    if (ManNom == "")
    {
        UpMessageBox::Watch(this,tr("Vous devez spécifier un nom!"));
        ui->NomlineEdit->setFocus();
        return;
    }

    UpLineEdit* line = dynamic_cast<UpLineEdit*>(focusWidget());
    if (line != Q_NULLPTR)
        if (line == ui->NomlineEdit || line == ui->Adresse1lineEdit || line == ui->Adresse2lineEdit || line == ui->Adresse3lineEdit)
            line->setText(Utils::trimcapitilize(line->text()));

    // D - On vérifie ensuite si ce correspondant existe déjà
    bool ok;
    QString requete = "select " CP_ID_MANUFACTURER " from " TBL_MANUFACTURERS
            " where " CP_NOM_MANUFACTURER " LIKE '" + ManNom + "%'";
    QVariantList mandata = db->getFirstRecordFromStandardSelectSQL(requete,ok, tr("Impossible d'interroger la table des correspondants!"));
    if (!ok)
    {
        reject();
        return;
    }

    if (mandata.size() > 0)
    {
        switch (m_mode) {
        case Modification:
            if (mandata.at(0).toInt() != m_currentmanufacturer->id())
            {
                UpMessageBox::Watch(this,tr("Ce fabricant existe déjà!"));
                delete  m_currentmanufacturer;
                m_currentmanufacturer = Datas::I()->manufacturers->getById(mandata.at(0).toInt());
                OKButton->setEnabled(false);
                AfficheDatasManufacturer();
                disconnect (OKButton,   &QPushButton::clicked,  this,   &dlg_identificationmanufacturer::OKpushButtonClicked);
                connect(OKButton,       &QPushButton::clicked,  this,   &dlg_identificationmanufacturer::accept);
                return;
            }
            break;
        case Creation:
            {
                UpMessageBox::Watch(this,tr("Ce fabricant existe déjà!"));
                m_currentmanufacturer = Datas::I()->manufacturers->getById(mandata.at(0).toInt());
                OKButton->setEnabled(false);
                m_mode = Modification;
                AfficheDatasManufacturer();
                disconnect (OKButton,   &QPushButton::clicked,  this,   &dlg_identificationmanufacturer::OKpushButtonClicked);
                connect(OKButton,       &QPushButton::clicked,  this,   &dlg_identificationmanufacturer::accept);
                return;
            }
        }
    }

    m_listbinds[CP_NOM_MANUFACTURER]          = ManNom.toUpper();
    m_listbinds[CP_ADRESSE1_MANUFACTURER]     = Utils::trimcapitilize(ui->Adresse1lineEdit->text().toUpper(),true);
    m_listbinds[CP_ADRESSE2_MANUFACTURER]     = Utils::trimcapitilize(ui->Adresse2lineEdit->text(),true);
    m_listbinds[CP_ADRESSE3_MANUFACTURER]     = Utils::trimcapitilize(ui->Adresse3lineEdit->text(),true);
    m_listbinds[CP_CODEPOSTAL_MANUFACTURER]   = wdg_CPlineedit->text();
    m_listbinds[CP_VILLE_MANUFACTURER]        = wdg_villelineedit->text();
    m_listbinds[CP_TELEPHONE_MANUFACTURER]    = ui->TellineEdit->text();
    m_listbinds[CP_FAX_MANUFACTURER]          = ui->FaxlineEdit->text();
    m_listbinds[CP_PORTABLE_MANUFACTURER]     = ui->PortablelineEdit->text();
    m_listbinds[CP_WEBSITE_MANUFACTURER]      = ui->WebsiteineEdit->text();
    m_listbinds[CP_MAIL_MANUFACTURER]         = ui->MaillineEdit->text();
    m_listbinds[CP_CORNOM_MANUFACTURER]       = Utils::trimcapitilize(ui->CorNomlineEdit->text(), true);
    m_listbinds[CP_CORPRENOM_MANUFACTURER]    = Utils::trimcapitilize(ui->CorPrenomlineEdit->text(), true);
    m_listbinds[CP_CORSTATUT_MANUFACTURER]    = Utils::trimcapitilize(ui->CorStatutlineEdit->text(), true);
    m_listbinds[CP_CORMAIL_MANUFACTURER]      = ui->CorMaillineEdit->text();
    m_listbinds[CP_CORTELEPHONE_MANUFACTURER] = ui->CorTelephonelineEdit->text();
    m_listbinds[CP_INACTIF_MANUFACTURER]      = (ui->ActifcheckBox->isChecked()? QVariant(QVariant::String) : "1");
    accept();
}

/*-------------------------------------------------------------------------------------
 Interception du focusOut
-------------------------------------------------------------------------------------*/
bool dlg_identificationmanufacturer::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::FocusOut)
    {
        if (obj == ui->MaillineEdit)
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
-- Afficher la fiche du fabricant
--------------------------------------------------------------------------------------------*/
void dlg_identificationmanufacturer::AfficheDatasManufacturer()
{
    if (m_mode == Modification)
    {
        m_nommanufacturer       = m_currentmanufacturer->nom();
        ui->ActifcheckBox       ->setChecked(m_currentmanufacturer->isactif());
        ui->NomlineEdit         ->setText(m_nommanufacturer);
        ui->idManufacturerlabel ->setText(tr("Fabricant n° ") + QString::number(m_currentmanufacturer->id()));

        ui->Adresse1lineEdit    ->setText(m_currentmanufacturer->adresse1());
        ui->Adresse2lineEdit    ->setText(m_currentmanufacturer->adresse2());
        ui->Adresse3lineEdit    ->setText(m_currentmanufacturer->adresse3());
        QString CP              = m_currentmanufacturer->codepostal();
        wdg_CPlineedit          ->completer()->setCurrentRow(wdg_villeCP->villes()->ListeCodesPostaux().indexOf(CP)); // ce micmac est nécessaire à cause d'un bug de QCompleter en mode InLineCompletion
                                                                                                // il faut synchroniser à la main le QCompleter et le QlineEdit au premier affichage
        wdg_CPlineedit          ->setText(CP);
        wdg_villelineedit       ->setText(m_currentmanufacturer->ville());
        ui->TellineEdit         ->setText(m_currentmanufacturer->telephone());
        ui->PortablelineEdit    ->setText(m_currentmanufacturer->portable());
        ui->MaillineEdit        ->setText(m_currentmanufacturer->mail());

        ui->CorNomlineEdit      ->setText(m_currentmanufacturer->cornom());
        ui->CorPrenomlineEdit   ->setText(m_currentmanufacturer->corprenom());
        ui->CorStatutlineEdit   ->setText(m_currentmanufacturer->corstatut());
        ui->CorMaillineEdit     ->setText(m_currentmanufacturer->cormail());
        ui->CorTelephonelineEdit->setText(m_currentmanufacturer->cortelephone());
    }
    else if (m_mode == Creation)
    {
        wdg_CPlineedit              ->setText(Procedures::CodePostalParDefaut());
        wdg_villelineedit           ->setText(Procedures::VilleParDefaut());
    }
}
