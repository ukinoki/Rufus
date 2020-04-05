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
#include "dlg_identificationtiers.h"
#include "ui_dlg_identificationtiers.h"

#include "dlg_identificationtiers.h"
#include "icons.h"
#include "ui_dlg_identificationtiers.h"

dlg_identificationtiers::dlg_identificationtiers(Mode mode, Tiers *trs, QWidget *parent) :
    UpDialog(PATH_FILE_INI, "PositionsFiches/PositionIdentTIERS", parent),
    ui(new Ui::dlg_identificationtiers)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    if (trs != Q_NULLPTR)
        m_currenttiers          = trs;
    m_mode                      = mode;
    wdg_villeCP                 = new VilleCPWidget(Datas::I()->villes, ui->Principalframe);
    wdg_CPlineedit              = wdg_villeCP->ui->CPlineEdit;
    wdg_villelineedit           = wdg_villeCP->ui->VillelineEdit;
    wdg_villeCP                 ->move(10,156);
    m_nomTiers                  = "";

    dlglayout()     ->insertWidget(0,ui->Principalframe);
    AjouteLayButtons(UpDialog::ButtonCancel | UpDialog::ButtonOK);
    AjouteWidgetLayButtons(ui->idTierslabel, false);
    dlglayout()     ->setSizeConstraint(QLayout::SetFixedSize);
    QFont font = qApp->font();
    font.setBold(true);
    ui->NomlineEdit->setFont(font);

    setWindowIcon(Icons::icDoctor());
    connect(wdg_villeCP, &VilleCPWidget::villecpmodified, this, &dlg_identificationtiers::EnableOKpushButton);

    AfficheDatasTiers();
    if (m_mode == Creation)
        ui->idTierslabel  ->setVisible(false);

    ui->NomlineEdit         ->setValidator(new QRegExpValidator(Utils::rgx_rx,this));
    ui->MaillineEdit        ->setValidator(new QRegExpValidator(Utils::rgx_mail,this));
    ui->Adresse1lineEdit    ->setValidator(new QRegExpValidator(Utils::rgx_adresse,this));
    ui->Adresse2lineEdit    ->setValidator(new QRegExpValidator(Utils::rgx_adresse,this));
    ui->Adresse3lineEdit    ->setValidator(new QRegExpValidator(Utils::rgx_adresse,this));
    ui->TellineEdit         ->setValidator(new QRegExpValidator(Utils::rgx_telephone,this));

    installEventFilter(this);
    ui->MaillineEdit->installEventFilter(this);
    ui->NomlineEdit->setFocus();

    OKButton->disconnect();
    connect (OKButton,                  &QPushButton::clicked,              this,           &dlg_identificationtiers::OKpushButtonClicked);
    connect (ui->NomlineEdit,           &UpLineEdit::TextModified,          this,           [=] {Majuscule(ui->NomlineEdit);});
    connect (ui->Adresse1lineEdit,      &UpLineEdit::TextModified,          this,           [=] {Majuscule(ui->Adresse1lineEdit);});
    connect (ui->Adresse2lineEdit,      &UpLineEdit::TextModified,          this,           [=] {Majuscule(ui->Adresse2lineEdit);});
    connect (ui->Adresse3lineEdit,      &UpLineEdit::TextModified,          this,           [=] {Majuscule(ui->Adresse3lineEdit);});
    connect (ui->NomlineEdit,           &QLineEdit::textEdited,             this,           &dlg_identificationtiers::EnableOKpushButton);
    connect (ui->Adresse1lineEdit,      &QLineEdit::textEdited,             this,           &dlg_identificationtiers::EnableOKpushButton);
    connect (ui->Adresse2lineEdit,      &QLineEdit::textEdited,             this,           &dlg_identificationtiers::EnableOKpushButton);
    connect (ui->Adresse3lineEdit,      &QLineEdit::textEdited,             this,           &dlg_identificationtiers::EnableOKpushButton);
    connect (ui->TellineEdit,           &QLineEdit::textEdited,             this,           &dlg_identificationtiers::EnableOKpushButton);
    connect (ui->MaillineEdit,          &QLineEdit::textEdited,             this,           &dlg_identificationtiers::EnableOKpushButton);
    OKButton->setEnabled(false);
    OKButton->setText(tr("Enregistrer"));
    CancelButton->setText(tr("Annuler"));
    setStageCount(1);

    QCompleter *profcpl = new QCompleter(Datas::I()->correspondants->autresprofessions(), this);
    profcpl             ->setCaseSensitivity(Qt::CaseInsensitive);
    profcpl             ->setCompletionMode(QCompleter::InlineCompletion);
}

dlg_identificationtiers::~dlg_identificationtiers()
{
}

void    dlg_identificationtiers:: EnableOKpushButton()
{
    bool a  = ui->NomlineEdit->text() != ""
           && wdg_CPlineedit->text() != ""
           && wdg_villelineedit->text() != "";
    OKButton->setEnabled(a);
    OKButton->setShortcut(a? QKeySequence("Meta+Return") : QKeySequence());
}

void dlg_identificationtiers::Majuscule(QLineEdit *ledit)
{
    ledit->setText(Utils::trimcapitilize(ledit->text(),false));
    OKButton->setEnabled(true);
}

void    dlg_identificationtiers::OKpushButtonClicked()
{
    QString trsom;
    trsom      = Utils::correctquoteSQL(Utils::trimcapitilize(ui->NomlineEdit->text(),true));

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
    if (trsom == "")
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
    QString requete = "select " CP_ID_TIERS " from " TBL_TIERS
            " where " CP_NOM_TIERS " LIKE '" + trsom + "%'";
    QVariantList trsdata = db->getFirstRecordFromStandardSelectSQL(requete,ok, tr("Impossible d'interroger la table des correspondants!"));
    if (!ok)
    {
        reject();
        return;
    }

    if (trsdata.size() > 0)
    {
        switch (m_mode) {
        case Modification:
            if (trsdata.at(0).toInt() != m_currenttiers->id())
            {
                UpMessageBox::Watch(this,tr("Ce fabricant existe déjà!"));
                delete  m_currenttiers;
                m_currenttiers = Datas::I()->tierspayants->getById(trsdata.at(0).toInt());
                OKButton->setEnabled(false);
                AfficheDatasTiers();
                disconnect (OKButton,   &QPushButton::clicked,  this,   &dlg_identificationtiers::OKpushButtonClicked);
                connect(OKButton,       &QPushButton::clicked,  this,   &dlg_identificationtiers::accept);
                return;
            }
            break;
        case Creation:
            {
                UpMessageBox::Watch(this,tr("Ce fabricant existe déjà!"));
                m_currenttiers = Datas::I()->tierspayants->getById(trsdata.at(0).toInt());
                OKButton->setEnabled(false);
                m_mode = Modification;
                AfficheDatasTiers();
                disconnect (OKButton,   &QPushButton::clicked,  this,   &dlg_identificationtiers::OKpushButtonClicked);
                connect(OKButton,       &QPushButton::clicked,  this,   &dlg_identificationtiers::accept);
                return;
            }
        }
    }

    m_listbinds[CP_NOM_TIERS]          = trsom.toUpper();
    m_listbinds[CP_ADRESSE1_TIERS]     = Utils::trimcapitilize(ui->Adresse1lineEdit->text().toUpper(),true);
    m_listbinds[CP_ADRESSE2_TIERS]     = Utils::trimcapitilize(ui->Adresse2lineEdit->text(),true);
    m_listbinds[CP_ADRESSE3_TIERS]     = Utils::trimcapitilize(ui->Adresse3lineEdit->text(),true);
    m_listbinds[CP_CODEPOSTAL_TIERS]   = wdg_CPlineedit->text();
    m_listbinds[CP_VILLE_TIERS]        = wdg_villelineedit->text();
    m_listbinds[CP_TELEPHONE_TIERS]    = ui->TellineEdit->text();
    m_listbinds[CP_FAX_TIERS]          = ui->FaxlineEdit->text();
    m_listbinds[CP_WEB_TIERS]          = ui->WebsiteineEdit->text();
    m_listbinds[CP_MAIL_TIERS]         = ui->MaillineEdit->text();
    if (m_mode == Creation)
         m_currenttiers = Datas::I()->tierspayants->CreationTiers(m_listbinds);
    else if (m_mode == Modification)
        DataBase::I()->UpdateTable(TBL_TIERS, m_listbinds, " where " CP_ID_TIERS " = " + QString::number(m_currenttiers->id()),tr("Impossible de modifier le dossier"));

    accept();
}

/*-------------------------------------------------------------------------------------
 Interception du focusOut
-------------------------------------------------------------------------------------*/
bool dlg_identificationtiers::eventFilter(QObject *obj, QEvent *event)
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
void dlg_identificationtiers::AfficheDatasTiers()
{
    if (m_mode == Modification)
    {
        m_nomTiers       = m_currenttiers->nom();
        ui->NomlineEdit         ->setText(m_nomTiers);
        ui->idTierslabel ->setText(tr("Tiers n° ") + QString::number(m_currenttiers->id()));

        ui->Adresse1lineEdit    ->setText(m_currenttiers->adresse1());
        ui->Adresse2lineEdit    ->setText(m_currenttiers->adresse2());
        ui->Adresse3lineEdit    ->setText(m_currenttiers->adresse3());
        QString CP              = m_currenttiers->codepostal();
        wdg_CPlineedit          ->completer()->setCurrentRow(wdg_villeCP->villes()->ListeCodesPostaux().indexOf(CP)); // ce micmac est nécessaire à cause d'un bug de QCompleter en mode InLineCompletion
                                                                                                // il faut synchroniser à la main le QCompleter et le QlineEdit au premier affichage
        wdg_CPlineedit          ->setText(CP);
        wdg_villelineedit       ->setText(m_currenttiers->ville());
        ui->TellineEdit         ->setText(m_currenttiers->telephone());
        ui->MaillineEdit        ->setText(m_currenttiers->mail());

    }
    else if (m_mode == Creation)
    {
        wdg_CPlineedit              ->setText(Procedures::CodePostalParDefaut());
        wdg_villelineedit           ->setText(Procedures::VilleParDefaut());
    }
}
