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

dlg_identificationmanufacturer::dlg_identificationmanufacturer(Mode mode, bool quelesmedecins, Correspondant *cor, QWidget *parent) :
    UpDialog(PATH_FILE_INI, "PositionsFiches/PositionIdentCorrespondant", parent),
    ui(new Ui::dlg_identificationmanufacturer)
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
    {
        ui->idManufacturerlabel  ->setVisible(false);
    }

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
    QString CorNom, CorPrenom;
    CorNom      = Utils::correctquoteSQL(Utils::trimcapitilize(ui->NomlineEdit->text(),true));

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

    UpLineEdit* line = dynamic_cast<UpLineEdit*>(focusWidget());
    if (line != Q_NULLPTR)
        if (line == ui->NomlineEdit || line == ui->Adresse1lineEdit || line == ui->Adresse2lineEdit || line == ui->Adresse3lineEdit)
            line->setText(Utils::trimcapitilize(line->text()));

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
            disconnect (OKButton,   &QPushButton::clicked,  this,   &dlg_identificationmanufacturer::OKpushButtonClicked);
            connect(OKButton,       &QPushButton::clicked,  this,   &dlg_identificationmanufacturer::accept);
            QList<QLineEdit *> listline = findChildren<QLineEdit *>();
            for (int i = 0;i<listline.size();i++) listline.at(i)->setEnabled(false);
            ui->CorgroupBox->setEnabled(false);
            return;
        }
    }
    int idcor = 0;

    if (m_mode == Creation)
    {
        // D - le dossier n'existe pas, on le crée
        QString gSexeCor = "";
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
        Modifrequete += " where idCor =" + QString::number(idcor);
        //qDebug() <<  Modifrequete;
        db->StandardSQL(Modifrequete, tr("Impossible de modifier le dossier"));
    }
    m_modifdatascor = true;
    Datas::I()->correspondants->loadAllData(m_correspondant);
    Flags::I()->MAJflagCorrespondants();
    accept();
}

bool dlg_identificationmanufacturer::identcorrespondantmodifiee() const
{
    return m_modifdatascor;
}

Correspondant* dlg_identificationmanufacturer::correspondantrenvoye() const
{
    return m_correspondant;
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
        m_nomcor                 = m_correspondant->nom();
        m_prenomcor              = m_correspondant->prenom();
        ui->NomlineEdit         ->setText(m_nomcor);
        // pour decocher les 2 radiobutton sexe il faut d'abord leur retirer la propriétét AutoExclusive
        m_sexecor                    = m_correspondant->sexe();
        ui->idManufacturerlabel      ->setText(tr("Correspondant n° ") + QString::number(m_correspondant->id()));

        ui->Adresse1lineEdit    ->setText(m_correspondant->adresse1());
        ui->Adresse2lineEdit    ->setText(m_correspondant->adresse2());
        ui->Adresse3lineEdit    ->setText(m_correspondant->adresse3());
        QString CP              = m_correspondant->codepostal();
        wdg_CPlineedit              ->completer()->setCurrentRow(wdg_villeCP->villes()->ListeCodesPostaux().indexOf(CP)); // ce micmac est nécessaire à cause d'un bug de QCompleter en mode InLineCompletion
                                                                                                // il faut synchroniser à la main le QCompleter et le QlineEdit au premier affichage
        wdg_CPlineedit              ->setText(CP);
        wdg_villelineedit           ->setText(m_correspondant->ville());
        ui->TellineEdit         ->setText(m_correspondant->telephone());
        ui->PortablelineEdit    ->setText(m_correspondant->portable());
        ui->MaillineEdit        ->setText(m_correspondant->mail());
    }
    else if (m_mode == Creation)
    {
        wdg_CPlineedit              ->setText(Procedures::CodePostalParDefaut());
        wdg_villelineedit           ->setText(Procedures::VilleParDefaut());
    }
}
