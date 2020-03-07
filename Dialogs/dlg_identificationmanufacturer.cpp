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
        m_manufacturer          = man;
    m_mode                      = mode;
    wdg_villeCP                 = new VilleCPWidget(Datas::I()->villes, ui->Principalframe);
    wdg_CPlineedit              = wdg_villeCP->ui->CPlineEdit;
    wdg_villelineedit           = wdg_villeCP->ui->VillelineEdit;
    wdg_villeCP                 ->move(10,156);
    m_nommanufacturer           = "";
    m_modifdatasmanufacturer    = false;

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

    AfficheDossierAlOuverture();
    if (m_mode == Creation)
        ui->idManufacturerlabel  ->setVisible(false);

    ui->NomlineEdit         ->setValidator(new QRegExpValidator(Utils::rgx_rx,this));
    ui->MaillineEdit        ->setValidator(new QRegExpValidator(Utils::rgx_mail,this));
    ui->Adresse1lineEdit    ->setValidator(new QRegExpValidator(Utils::rgx_adresse,this));
    ui->Adresse2lineEdit    ->setValidator(new QRegExpValidator(Utils::rgx_adresse,this));
    ui->Adresse3lineEdit    ->setValidator(new QRegExpValidator(Utils::rgx_adresse,this));
    ui->TellineEdit         ->setValidator(new QRegExpValidator(Utils::rgx_telephone,this));
    ui->PortablelineEdit    ->setValidator(new QRegExpValidator(Utils::rgx_telephone,this));

    QList <QWidget *> listtab;
    listtab << ui->NomlineEdit << ui->Adresse1lineEdit << ui->Adresse2lineEdit << ui->Adresse3lineEdit << wdg_CPlineedit << wdg_villelineedit
            << ui->TellineEdit << ui->PortablelineEdit << ui->MaillineEdit << ui->FaxlineEdit;
    for (int i = 0; i<listtab.size()-1 ; i++ )
        setTabOrder(listtab.at(i), listtab.at(i+1));

    installEventFilter(this);
    ui->MaillineEdit->installEventFilter(this);
    ui->NomlineEdit->setFocus();

    OKButton->disconnect();
    connect (OKButton,                  &QPushButton::clicked,              this,           &dlg_identificationmanufacturer::OKpushButtonClicked);
    connect (ui->NomlineEdit,           &UpLineEdit::TextModified,          this,           [=] {Majuscule(ui->NomlineEdit);});
    connect (ui->Adresse1lineEdit,      &UpLineEdit::TextModified,          this,           [=] {Majuscule(ui->Adresse1lineEdit);});
    connect (ui->Adresse2lineEdit,      &UpLineEdit::TextModified,          this,           [=] {Majuscule(ui->Adresse2lineEdit);});
    connect (ui->Adresse3lineEdit,      &UpLineEdit::TextModified,          this,           [=] {Majuscule(ui->Adresse3lineEdit);});
    connect (ui->NomlineEdit,           &QLineEdit::textEdited,             this,           &dlg_identificationmanufacturer::EnableOKpushButton);
    connect (ui->Adresse1lineEdit,      &QLineEdit::textEdited,             this,           &dlg_identificationmanufacturer::EnableOKpushButton);
    connect (ui->Adresse2lineEdit,      &QLineEdit::textEdited,             this,           &dlg_identificationmanufacturer::EnableOKpushButton);
    connect (ui->Adresse3lineEdit,      &QLineEdit::textEdited,             this,           &dlg_identificationmanufacturer::EnableOKpushButton);
    connect (ui->TellineEdit,           &QLineEdit::textEdited,             this,           &dlg_identificationmanufacturer::EnableOKpushButton);
    connect (ui->PortablelineEdit,      &QLineEdit::textEdited,             this,           &dlg_identificationmanufacturer::EnableOKpushButton);
    connect (ui->MaillineEdit,          &QLineEdit::textEdited,             this,           &dlg_identificationmanufacturer::EnableOKpushButton);

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

void    dlg_identificationmanufacturer::EnableOKpushButton()
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
        if (mandata.at(0).toInt() != m_manufacturer->id())
        {
            UpMessageBox::Watch(this,tr("Ce fabricant existe déjà!"));
            m_manufacturer = Datas::I()->manufacturers->getById(mandata.at(0).toInt());
            OKButton->setEnabled(false);
            m_mode = Modification;
            AfficheDossierAlOuverture();
            disconnect (OKButton,   &QPushButton::clicked,  this,   &dlg_identificationmanufacturer::OKpushButtonClicked);
            connect(OKButton,       &QPushButton::clicked,  this,   &dlg_identificationmanufacturer::accept);
            return;
        }
    }

    QHash<QString, QVariant> listbinds;
    listbinds[CP_NOM_MANUFACTURER]          = ManNom.toUpper();
    listbinds[CP_ADRESSE1_MANUFACTURER]     = Utils::trimcapitilize(ui->Adresse1lineEdit->text().toUpper(),true);
    listbinds[CP_ADRESSE2_MANUFACTURER]     = Utils::trimcapitilize(ui->Adresse2lineEdit->text(),true);
    listbinds[CP_ADRESSE3_MANUFACTURER]     = Utils::trimcapitilize(ui->Adresse3lineEdit->text(),true);
    listbinds[CP_CODEPOSTAL_MANUFACTURER]   = wdg_CPlineedit->text();
    listbinds[CP_VILLE_MANUFACTURER]        = wdg_villelineedit->text();
    listbinds[CP_TELEPHONE_MANUFACTURER]    = ui->TellineEdit->text();
    listbinds[CP_FAX_MANUFACTURER]          = ui->FaxlineEdit->text();
    listbinds[CP_PORTABLE_MANUFACTURER]     = ui->PortablelineEdit->text();
    listbinds[CP_WEBSITE_MANUFACTURER]      = ui->WebsiteineEdit->text();
    listbinds[CP_MAIL_MANUFACTURER]         = ui->MaillineEdit->text();
    listbinds[CP_CORNOM_MANUFACTURER]       = Utils::trimcapitilize(ui->CorNomlineEdit->text(), true);
    listbinds[CP_CORPRENOM_MANUFACTURER]    = Utils::trimcapitilize(ui->CorPrenomlineEdit->text(), true);
    listbinds[CP_CORSTATUT_MANUFACTURER]    = Utils::trimcapitilize(ui->CorStatutlineEdit->text(), true);
    listbinds[CP_CORMAIL_MANUFACTURER]      = Utils::trimcapitilize(ui->CorMaillineEdit->text(), true);
    listbinds[CP_CORTELEPHONE_MANUFACTURER] = Utils::trimcapitilize(ui->CorTelephonelineEdit->text(), true);
    if (m_mode == Creation)
        m_manufacturer = Datas::I()->manufacturers->CreationManufacturer(listbinds);
    else if (m_mode == Modification)
        DataBase::I()->UpdateTable(TBL_MANUFACTURERS, listbinds, " where " CP_ID_MANUFACTURER " = " + QString::number(m_manufacturer->id()),tr("Impossible de modifier le dossier"));
    m_modifdatasmanufacturer = true;
    accept();
}

bool dlg_identificationmanufacturer::identcorrespondantmodifiee() const
{
    return m_modifdatasmanufacturer;
}

Manufacturer *dlg_identificationmanufacturer::manufacturerrenvoye() const
{
    return m_manufacturer;
}

void dlg_identificationmanufacturer::RegleAffichage()
{
    OKButton->setEnabled(true);
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
-- Afficher la fiche du correspondant
--------------------------------------------------------------------------------------------*/
void dlg_identificationmanufacturer::AfficheDossierAlOuverture()
{
    if (m_mode == Modification)
    {
        m_nommanufacturer       = m_manufacturer->nom();
        ui->NomlineEdit         ->setText(m_nommanufacturer);
        ui->idManufacturerlabel ->setText(tr("Fabricant n° ") + QString::number(m_manufacturer->id()));

        ui->Adresse1lineEdit    ->setText(m_manufacturer->adresse1());
        ui->Adresse2lineEdit    ->setText(m_manufacturer->adresse2());
        ui->Adresse3lineEdit    ->setText(m_manufacturer->adresse3());
        QString CP              = m_manufacturer->codepostal();
        wdg_CPlineedit          ->completer()->setCurrentRow(wdg_villeCP->villes()->ListeCodesPostaux().indexOf(CP)); // ce micmac est nécessaire à cause d'un bug de QCompleter en mode InLineCompletion
                                                                                                // il faut synchroniser à la main le QCompleter et le QlineEdit au premier affichage
        wdg_CPlineedit          ->setText(CP);
        wdg_villelineedit       ->setText(m_manufacturer->ville());
        ui->TellineEdit         ->setText(m_manufacturer->telephone());
        ui->PortablelineEdit    ->setText(m_manufacturer->portable());
        ui->MaillineEdit        ->setText(m_manufacturer->mail());
    }
    else if (m_mode == Creation)
    {
        wdg_CPlineedit              ->setText(Procedures::CodePostalParDefaut());
        wdg_villelineedit           ->setText(Procedures::VilleParDefaut());
    }
}
