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

#include "dlg_identificationmanufacturer.h"
#include "icons.h"
#include "ui_dlg_identificationmanufacturer.h"

dlg_identificationmanufacturer::dlg_identificationmanufacturer(Mode mode, Manufacturer *man, QWidget *parent) :
    UpDialog(PATH_FILE_INI, "PositionsFiches/PositionIdentManuFacturer", parent),
    ui(new Ui::dlg_identificationmanufacturer)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    setWindowTitle(tr("Gestion des abricants"));
    if (man != Q_NULLPTR)
        m_currentmanufacturer   = man;
    m_mode                      = mode;
    wdg_villeCP                 = new VilleCPWidget(Datas::I()->villes, ui->Principalframe);
    wdg_CPlineedit              = wdg_villeCP->ui->CPlineEdit;
    wdg_villelineedit           = wdg_villeCP->ui->VillelineEdit;
    wdg_villeCP                 ->move(10,156);
    m_nommanufacturer           = "";
    wdg_buttonframe         = new WidgetButtonFrame(ui->commercialsupTableView);
    wdg_buttonframe         ->AddButtons(WidgetButtonFrame::Plus | WidgetButtonFrame::Modifier | WidgetButtonFrame::Moins);

    dlglayout()     ->insertWidget(0,wdg_buttonframe->widgButtonParent());
    dlglayout()     ->insertWidget(0,ui->Principalframe);
    AjouteLayButtons(UpDialog::ButtonCancel | UpDialog::ButtonOK);
    AjouteWidgetLayButtons(ui->idManufacturerlabel, false);
    dlglayout()     ->setSizeConstraint(QLayout::SetFixedSize);
    dlglayout()     ->setSpacing(5);
    QFont font = qApp->font();
    font.setBold(true);
    ui->NomlineEdit->setFont(font);

    setWindowIcon(Icons::icDoctor());
    connect(wdg_villeCP, &VilleCPWidget::villecpmodified, this, &dlg_identificationmanufacturer::EnableOKpushButton);

    if (m_mode == Creation)
        ui->idManufacturerlabel  ->setVisible(false);

    ui->NomlineEdit         ->setValidator(new QRegularExpressionValidator(rgx_nom,this));
    ui->MaillineEdit        ->setValidator(new QRegularExpressionValidator(Utils::rgx_mail,this));
    ui->Adresse1lineEdit    ->setValidator(new QRegularExpressionValidator(Utils::rgx_adresse,this));
    ui->Adresse2lineEdit    ->setValidator(new QRegularExpressionValidator(Utils::rgx_adresse,this));
    ui->Adresse3lineEdit    ->setValidator(new QRegularExpressionValidator(Utils::rgx_adresse,this));
    ui->TellineEdit         ->setValidator(new QRegularExpressionValidator(Utils::rgx_telephone,this));
    ui->PortablelineEdit    ->setValidator(new QRegularExpressionValidator(Utils::rgx_telephone,this));

    setSizeGripEnabled(false);
    setWindowTitle(tr("Liste des fabricants"));

    ui->commercialsupTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->commercialsupTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->commercialsupTableView->verticalHeader()->setVisible(false);
    ui->commercialsupTableView->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->commercialsupTableView->setFocusPolicy(Qt::StrongFocus);
    ui->commercialsupTableView->setGridStyle(Qt::NoPen);
    ui->commercialsupTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    QStandardItemModel m_manufacturermodel;
    int idcurrenmtmanufacturer = (m_currentmanufacturer? m_currentmanufacturer->id() : 0);
    for (auto it = Datas::I()->manufacturers->manufacturers()->constBegin(); it != Datas::I()->manufacturers->manufacturers()->constEnd(); ++it)
    {
        Manufacturer *man = const_cast<Manufacturer*>(it.value());
        if (man->id() != idcurrenmtmanufacturer)
        {
            QList<QStandardItem *> items;
            items << new QStandardItem(man->nom()) << new QStandardItem(QString::number(man->id()));
            m_manufacturermodel.appendRow(items);
        }
    }
    m_manufacturermodel.sort(0);
    for (int i=0; i <m_manufacturermodel.rowCount(); ++i)
        ui->DistributeurupComboBox->addItem(m_manufacturermodel.item(i)->text(), m_manufacturermodel.item(i,1)->text());

    AfficheDatasManufacturer();

    QList <QWidget *> listtab;
    listtab << ui->NomlineEdit << ui->Adresse1lineEdit << ui->Adresse2lineEdit << ui->Adresse3lineEdit << wdg_CPlineedit << wdg_villelineedit
            << ui->TellineEdit << ui->PortablelineEdit << ui->MaillineEdit << ui->WebsiteineEdit << ui->FaxlineEdit << ui->DistributeurupComboBox;
    for (int i = 0; i<listtab.size()-1 ; i++ )
        setTabOrder(listtab.at(i), listtab.at(i+1));
    installEventFilter(this);
    ui->MaillineEdit->installEventFilter(this);
    ui->NomlineEdit->setFocus();
    ui->Websitelabel->setText("<a href=\"Site Web\">Site Web</a>");
    OKButton->disconnect();
    connect (OKButton,                  &QPushButton::clicked,              this,           [=] {OKpushButtonClicked();});
    connect (ui->NomlineEdit,           &UpLineEdit::TextModified,          this,           [=] {ui->NomlineEdit->setText(ui->NomlineEdit->text().toUpper()); EnableOKpushButton();});
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
    connect (ui->WebsiteineEdit,        &QLineEdit::textEdited,             this,           &dlg_identificationmanufacturer::EnableOKpushButton);
    connect (ui->FaxlineEdit,           &QLineEdit::textEdited,             this,           &dlg_identificationmanufacturer::EnableOKpushButton);
    connect (ui->DistributeurupComboBox->lineEdit(),    &QLineEdit::textEdited,
                                                                            this,           &dlg_identificationmanufacturer::EnableOKpushButton);
    connect (ui->DistributeurupComboBox,                QOverload<int>::of(&QComboBox::currentIndexChanged),
                                                                            this,           &dlg_identificationmanufacturer::EnableOKpushButton);
    connect (ui->Websitelabel,          &QLabel::linkActivated,             this,           [=] {if (ui->WebsiteineEdit->text() != "") QDesktopServices::openUrl(QUrl("http://" + ui->WebsiteineEdit->text()));});
    connect(wdg_buttonframe,            &WidgetButtonFrame::choix,          this,           &dlg_identificationmanufacturer::ChoixButtonFrame);
    OKButton->setEnabled(false);
    OKButton->setText(tr("Enregistrer"));
    CancelButton->setText(tr("Annuler"));
    wdg_buttonframe->wdg_modifBouton->setEnabled(false);
    wdg_buttonframe->wdg_moinsBouton->setEnabled(false);

    setStageCount(1);
}

dlg_identificationmanufacturer::~dlg_identificationmanufacturer()
{
}

void    dlg_identificationmanufacturer:: EnableOKpushButton()
{
    bool a  = ui->NomlineEdit->text() != "";
    OKButton->setEnabled(a);
    OKButton->setShortcut(a? QKeySequence("Meta+Return") : QKeySequence());
}

Commercial* dlg_identificationmanufacturer::getCommercialFromIndex(QModelIndex idx)
{
    Commercial *com = Q_NULLPTR;
    UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_commodel->itemFromIndex(idx));
    if(itm)
        com = qobject_cast<Commercial*>(itm->item());
    return com;
}

void dlg_identificationmanufacturer::Majuscule(QLineEdit *ledit)
{
    ledit->setText(Utils::trimcapitilize(ledit->text(),false));
    OKButton->setEnabled(true);
}

void    dlg_identificationmanufacturer::OKpushButtonClicked(bool acceptalafin)
{
    QString ManNom;
    ManNom      = Utils::correctquoteSQL(Utils::trimcapitilize(ui->NomlineEdit->text(),true));

    // B - On vérifie ensuite que la saisie est complète
    if (ManNom == "")
    {
        UpMessageBox::Watch(this,tr("Vous devez spécifier un nom!"));
        ui->NomlineEdit->setFocus();
        return;
    }

    UpLineEdit* line = qobject_cast<UpLineEdit*>(focusWidget());
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
    m_listbinds[CP_INACTIF_MANUFACTURER]      = (ui->ActifcheckBox->isChecked()? QVariant(QMetaType::fromType<QString>()) : "1");
    m_listbinds[CP_DISTRIBUEPAR_MANUFACTURER] = (ui->DistributeurupComboBox->currentData().toInt()>0? ui->DistributeurupComboBox->currentData().toInt() : QVariant());
    if (m_mode == Creation)
         m_currentmanufacturer = Datas::I()->manufacturers->CreationManufacturer(m_listbinds);
    else if (m_mode == Modification)
        DataBase::I()->UpdateTable(TBL_MANUFACTURERS, m_listbinds, " where " CP_ID_MANUFACTURER " = " + QString::number(m_currentmanufacturer->id()),tr("Impossible de modifier le dossier"));
    if (acceptalafin)
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
                //if (!Utils::rgx_mail.exactMatch(ui->MaillineEdit->text()))
                if (!Utils::RegularExpressionMatches(Utils::rgx_mail, ui->MaillineEdit->text()))
                {
                    UpMessageBox::Watch(this, tr("Adresse mail invalide"));
                    ui->MaillineEdit->setFocus();
                    return true;
                }
        }
    }
    if (event->type() == QEvent::KeyPress  && qobject_cast<QPushButton *>(obj) == Q_NULLPTR )
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        // Return ou Enter - On va au Tab Suivant -----------------------------------------------------------------------------------------------------------------------------
        if ((keyEvent->key()==Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) && keyEvent->modifiers() == Qt::NoModifier)
            return QWidget::focusNextChild();
    }
    return QWidget::eventFilter(obj, event);
}


void dlg_identificationmanufacturer::ChoixButtonFrame()
{
    Commercial *com = Q_NULLPTR;
    if (ui->commercialsupTableView->selectionModel())
        if (ui->commercialsupTableView->selectionModel()->selectedIndexes().size()>0)
            com = getCommercialFromIndex(ui->commercialsupTableView->selectionModel()->selectedIndexes().at(0));
    switch (wdg_buttonframe->Choix()) {
    case WidgetButtonFrame::Plus:
        EnregistreNouveauCommercial();
        break;
    case WidgetButtonFrame::Modifier:
        if (com)
          ModifieCommercial(com);
        break;
    case WidgetButtonFrame::Moins:
        if (com)
          SupprimeCommercial(com);
        break;
    }
}

void dlg_identificationmanufacturer::EnregistreNouveauCommercial()
{
    if (m_mode == Creation)
    {
        if (UpMessageBox::Question(this, tr("Vous devez d'abord enregistrer ce fabricant avant d'enregistrer des personnels"),
                                   tr("Voulez-vous enregistrer ce fabricant?")) != UpSmallButton::STARTBUTTON)
            return;
        else
        {
            OKpushButtonClicked(false);
            if (m_currentmanufacturer)
                m_mode = Modification;
        }
    }
    if (!m_currentmanufacturer)
        return;
    dlg_identificationcommercial *Dlg_Identcommercial    = new dlg_identificationcommercial(dlg_identificationcommercial::Creation, m_currentmanufacturer, this);
    if (Dlg_Identcommercial->exec() == QDialog::Accepted)
    {
        int id = Dlg_Identcommercial->idcurrentcommercial();
        reconstruitCommercialsModel();
        for (int i=0; i<m_commodel->rowCount(); ++i)
        {
            UpStandardItem *itm = dynamic_cast<UpStandardItem *>(m_commodel->item(i));
            if (itm)
                if (itm->item())
                {
                    Commercial *scom = qobject_cast<Commercial*>(itm->item());
                    if (scom)
                        if (scom->id() == id)
                        {
                            ui->commercialsupTableView->scrollTo(itm->index(), QAbstractItemView::PositionAtCenter);
                            ui->commercialsupTableView->selectionModel()->select(itm->index(),QItemSelectionModel::Rows | QItemSelectionModel::Select);
                            i = m_commodel->rowCount();
                        }
                }
        }
        if (ui->commercialsupTableView->selectionModel()->selectedIndexes().size()>0)
        {
            QModelIndex idx = ui->commercialsupTableView->selectionModel()->currentIndex();
            Enablebuttons(idx);
        }
    }
    delete Dlg_Identcommercial;
}

void dlg_identificationmanufacturer::ModifieCommercial(Commercial *com)
{
    if (!com)
        return;
    dlg_identificationcommercial *Dlg_Identcommercial    = new dlg_identificationcommercial(dlg_identificationcommercial::Modification, com, this);
    if (Dlg_Identcommercial->exec()>0)
    {
        int id = Dlg_Identcommercial->idcurrentcommercial();
        reconstruitCommercialsModel();
        for (int i=0; i<m_commodel->rowCount(); ++i)
        {
            UpStandardItem *itm = dynamic_cast<UpStandardItem *>(m_commodel->item(i));
            if (itm)
                if (itm->item())
                {
                    Commercial *scom = qobject_cast<Commercial*>(itm->item());
                    if (scom)
                    {
                        if (scom->id() == id)
                        {
                            ui->commercialsupTableView->scrollTo(itm->index(), QAbstractItemView::PositionAtCenter);
                            ui->commercialsupTableView->selectionModel()->select(itm->index(),QItemSelectionModel::Rows | QItemSelectionModel::Select);
                            i = m_commodel->rowCount();
                        }
                    }
                }
        }
        if (ui->commercialsupTableView->selectionModel()->selectedIndexes().size()>0)
        {
            QModelIndex idx = ui->commercialsupTableView->selectionModel()->currentIndex();
            Enablebuttons(idx);
        }
    }
    delete Dlg_Identcommercial;
}

void dlg_identificationmanufacturer::SupprimeCommercial(Commercial *com)
{
    if (!com) return;
    QString Msg;
    Msg = tr("Etes vous sûr de vouloir supprimer") + "\n " +
            com->nom().toUpper() + " " + com->prenom() + "?" +
            "\n" + tr("La suppression est IRRÉVERSIBLE.");
    UpMessageBox msgbox(this);
    msgbox.setText("Euuhh... " + Datas::I()->users->userconnected()->login() + "?");
    msgbox.setInformativeText(Msg);
    msgbox.setIcon(UpMessageBox::Warning);
    UpSmallButton NoBouton(tr("Annuler"));
    UpSmallButton OKBouton(tr("Supprimer la fiche"));
    msgbox.addButton(&NoBouton, UpSmallButton::CANCELBUTTON);
    msgbox.addButton(&OKBouton, UpSmallButton::SUPPRBUTTON);
    msgbox.exec();
    if (msgbox.clickedButton() == &OKBouton)
    {
        Datas::I()->commercials->SupprimeCommercial(com);
        reconstruitCommercialsModel();
    }
    if (ui->commercialsupTableView->selectionModel()->selectedIndexes().size()>0)
    {
        QModelIndex idx = ui->commercialsupTableView->selectionModel()->currentIndex();
        Enablebuttons(idx);
    }
}

void dlg_identificationmanufacturer::Enablebuttons(QModelIndex idx)
{
    Commercial *com = getCommercialFromIndex(idx);
    wdg_buttonframe->wdg_modifBouton->setEnabled(com != Q_NULLPTR);
    wdg_buttonframe->wdg_moinsBouton->setEnabled(com != Q_NULLPTR);
}

/*--------------------------------------------------------------------------------------------
-- Afficher la fiche du fabricant
--------------------------------------------------------------------------------------------*/
void dlg_identificationmanufacturer::AfficheDatasManufacturer()
{
    if (m_mode == Modification)
    {
        if(!m_currentmanufacturer)
            return;
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
        ui->FaxlineEdit         ->setText(m_currentmanufacturer->fax());
        ui->WebsiteineEdit      ->setText(m_currentmanufacturer->website());
        ui->MaillineEdit        ->setText(m_currentmanufacturer->mail());

        ui->DistributeurupComboBox->setCurrentIndex(ui->DistributeurupComboBox->findData(m_currentmanufacturer->iddistributeur()));
        reconstruitCommercialsModel();
    }
    else if (m_mode == Creation)
    {
        wdg_CPlineedit              ->setText(Procedures::CodePostalParDefaut());
        wdg_villelineedit           ->setText(Procedures::VilleParDefaut());
        ui->DistributeurupComboBox->setCurrentIndex(-1);
    }
}

void dlg_identificationmanufacturer::reconstruitCommercialsModel()
{
    ui->commercialsupTableView->selectionModel()->disconnect();   
    Datas::I()->commercials->initListebyidManufacturer(m_currentmanufacturer->id());
    QItemSelectionModel *m = ui->commercialsupTableView->selectionModel();
    delete m;
    if (m_commodel == Q_NULLPTR)
        delete m_commodel;
    m_commodel = new QStandardItemModel(this);
    for (auto it = Datas::I()->commercials->commercials()->constBegin(); it != Datas::I()->commercials->commercials()->constEnd(); ++it)
    {
        Commercial*com = const_cast<Commercial*>(it.value());
        UpStandardItem *itmnom      = new UpStandardItem(com->nom().toUpper() + " " + com->prenom(), com);
        UpStandardItem *itmstatut   = new UpStandardItem(com->statut(), com);
        UpStandardItem *itmtel      = new UpStandardItem(com->telephone(), com);
        UpStandardItem *itmmail     = new UpStandardItem(com->mail(), com);
        m_commodel->appendRow(QList<QStandardItem*>() << itmnom << itmstatut << itmtel << itmmail);
    }
    QStandardItem *itnom = new QStandardItem();
    itnom->setText(tr("Nom"));
    itnom->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
    m_commodel->setHorizontalHeaderItem(0,itnom);
    QStandardItem *itstatut = new QStandardItem();
    itstatut->setText(tr("Statut"));
    itstatut->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
    m_commodel->setHorizontalHeaderItem(1,itstatut);
    QStandardItem *ittelephone = new QStandardItem();
    ittelephone->setText(tr("Telephone"));
    ittelephone->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
    m_commodel->setHorizontalHeaderItem(2,ittelephone);
    QStandardItem *itmail = new QStandardItem();
    itmail->setText(tr("Mail"));
    itmail->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
    m_commodel->setHorizontalHeaderItem(3,itmail);

    ui->commercialsupTableView->setModel(m_commodel);

    ui->commercialsupTableView->setColumnWidth(0,180);     // NomPrenom
    ui->commercialsupTableView->setColumnWidth(1,150);     // statut
    ui->commercialsupTableView->setColumnWidth(2,120);     // telephone
    ui->commercialsupTableView->setColumnWidth(3,180);     // mail
    ui->commercialsupTableView->setMouseTracking(true);
    ui->commercialsupTableView->FixLargeurTotale();
    wdg_buttonframe->widgButtonParent()->setFixedWidth(ui->commercialsupTableView->width());
    QFontMetrics fm(qApp->font());
    for (int j=0; j<Datas::I()->recettes->recettes()->size(); j++)
        ui->commercialsupTableView->setRowHeight(j,int(fm.height()*1.3));
    ui->commercialsupTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    connect(ui->commercialsupTableView->selectionModel(),   &QItemSelectionModel::currentChanged,   this,   &dlg_identificationmanufacturer::Enablebuttons);
    connect(ui->commercialsupTableView,                     &QAbstractItemView::doubleClicked,      this,   [=] (QModelIndex idx) { if (!m_commodel->itemFromIndex(idx))
                                                                                                                                        ModifieCommercial(getCommercialFromIndex(idx)); });
}
