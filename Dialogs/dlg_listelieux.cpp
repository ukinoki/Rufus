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

#include "dlg_listelieux.h"

dlg_listelieux::dlg_listelieux(QWidget *parent)
    : UpDialog(PATH_FILE_INI, "PositionsFiches/PositionLieux", parent)
{
    setWindowModality(Qt::WindowModal);
    db              = DataBase::I();
    AjouteLayButtons(UpDialog::ButtonClose);
    connect(CloseButton, &QPushButton::clicked, this, &QDialog::reject);

    wdg_tblview = new UpTableView(this);
    // Mise en forme de la table
    wdg_tblview->setPalette(QPalette(Qt::white));
    wdg_tblview->setGridStyle(Qt::NoPen);
    wdg_tblview->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    wdg_tblview->setEditTriggers(QAbstractItemView::NoEditTriggers);
    wdg_tblview->setSelectionMode(QAbstractItemView::SingleSelection);
    wdg_tblview->setSelectionBehavior(QAbstractItemView::SelectRows);
    wdg_tblview->setMouseTracking(true);
    wdg_tblview->setContextMenuPolicy(Qt::CustomContextMenu);
    wdg_tblview->horizontalHeader()->setVisible(true);

    wdg_adressuplbl = new UpLabel();
    wdg_adressuplbl->setFixedWidth(240);
    wdg_couleurpushbutt = new UpPushButton();
    wdg_couleurpushbutt->setFixedHeight(35);
    wdg_couleurpushbutt->setText(tr("modifier la couleur du texte"));
    if (Datas::I()->sites->sites()->size() == 0)
        Datas::I()->sites->initListe();
    wdg_buttonframe = new WidgetButtonFrame(wdg_tblview);
    wdg_buttonframe->AddButtons(WidgetButtonFrame::Plus | WidgetButtonFrame::Modifier | WidgetButtonFrame::Moins);
    wdg_buttonframe->widgButtonParent()->setFixedWidth(240);

    QVBoxLayout *vlay   = new QVBoxLayout();
    QHBoxLayout *hlay   = new QHBoxLayout();
    vlay    ->addWidget(wdg_adressuplbl);
    vlay    ->addSpacerItem(new QSpacerItem(5,5,QSizePolicy::Expanding,QSizePolicy::Expanding));
    vlay    ->addWidget(wdg_couleurpushbutt);
    hlay    ->addWidget(wdg_buttonframe->widgButtonParent());
    hlay    ->addSpacerItem(new QSpacerItem(5,5,QSizePolicy::Expanding,QSizePolicy::Expanding));
    hlay    ->addLayout(vlay);
    dlglayout()     ->insertLayout(0,hlay);
    dlglayout()     ->setSizeConstraint(QLayout::SetFixedSize);
    ReconstruitModel();

    connect(wdg_buttonframe,        &WidgetButtonFrame::choix,  this,   &dlg_listelieux::ChoixButtonFrame);
    connect(wdg_couleurpushbutt,    &QPushButton::clicked,      this,   &dlg_listelieux::ModifCouleur);
}

dlg_listelieux::~dlg_listelieux()
{
}

void dlg_listelieux::AfficheDetails(QModelIndex idx, QModelIndex)
{
    Site * sit = getSiteFromIndex(idx);
    if (!sit)
    {
        wdg_buttonframe->wdg_moinsBouton->setEnabled(false);
        return;
    }
    wdg_adressuplbl->setText(sit->coordonnees());
    wdg_adressuplbl->setStyleSheet("color:#" + (sit->couleur() != ""? sit->couleur() : "000000b"));
    QString ttip = "";
    int nlieux = db->StandardSelectSQL("select iduser from " TBL_JOINTURESLIEUX " where idlieu = " + QString::number(sit->id()), m_ok).size();
    if (nlieux == 0)
        nlieux = db->StandardSelectSQL("select " CP_IDLIEU_ACTES " from " TBL_ACTES " where " CP_IDLIEU_ACTES " = " + QString::number(sit->id()), m_ok).size();
    if (nlieux == 0)
    {
        if (sit->id() == m_idlieuserveur)
            ttip = tr("Vous ne pouvez pas supprimer ce site car il héberge le serveur") + "\n"
                    + tr ("Pour modifier ce paramètre, modifier le choix dans la box \"Emplacement du serveur\"") + "\n"
                    + tr ("Menu Edition/Paramètres - Onglet Général");
    }
    if (nlieux > 0)
        ttip = tr("Vous ne pouvez pas supprimer ce site car il est utilisé");
    wdg_buttonframe->wdg_moinsBouton->setEnabled(ttip == "");
    wdg_buttonframe->wdg_moinsBouton->setImmediateToolTip(ttip, true);
}

Site* dlg_listelieux::getSiteFromIndex(QModelIndex idx)
{
    UpStandardItem *upitem = dynamic_cast<UpStandardItem *>(m_model->itemFromIndex(idx));
    if (upitem == Q_NULLPTR)
        return Q_NULLPTR;
    Site *sit = qobject_cast<Site *>(upitem->item());
    return sit;
}

int dlg_listelieux::getRowFromSite(Site *sit)
{
    int row = -1;
    for (int i=0; i<m_model->rowCount(); i++)
    {
        UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_model->item(i));
        if(itm)
        {
            Site* sits = qobject_cast<Site*>(itm->item());
            if (sit->id() == sits->id())
            {
                row = i;
                break;
            }
        }
    }
    return row;
}

void dlg_listelieux::ChoixButtonFrame()
{
    switch (wdg_buttonframe->Choix()) {
    case WidgetButtonFrame::Plus:
        CreerLieu();
        break;
    case WidgetButtonFrame::Modifier:
        ModifLieu();
        break;
    case WidgetButtonFrame::Moins:
        SupprLieu();
        break;
    }
}

void dlg_listelieux::CreerLieu()
{
    ModifLieuxDialog(Nouv);
    dlg_lieu->exec();
    delete  dlg_lieu;
    dlg_lieu = Q_NULLPTR;
}

void dlg_listelieux::enregNouvLieu()
{
    if (ValidationFiche())
    {
        m_listbinds[CP_NOM_SITE]          = Utils::trimcapitilize(wdg_nomlineedit->text(), true);
        m_listbinds[CP_ADRESSE1_SITE]     = Utils::trimcapitilize(wdg_adress1lineedit->text().toUpper(),true);
        m_listbinds[CP_ADRESSE2_SITE]     = Utils::trimcapitilize(wdg_adress2lineedit->text(),true);
        m_listbinds[CP_ADRESSE3_SITE]     = Utils::trimcapitilize(wdg_adress3lineedit->text(),true);
        m_listbinds[CP_CODEPOSTAL_SITE]   = wdg_CPlineedit->text().toInt();
        m_listbinds[CP_VILLE_SITE]        = Utils::trimcapitilize(wdg_villelineedit->text(),true);
        m_listbinds[CP_TELEPHONE_SITE]    = wdg_tellineedit->text();
        m_listbinds[CP_FAX_SITE]          = wdg_faxlineedit->text();
        m_listbinds[CP_COULEUR_SITE]      = str_nouvcolor;

        Site *sit = Datas::I()->sites->CreationSite(m_listbinds);
        ReconstruitModel();
        dlg_lieu->accept();
        wdg_tblview->selectRow(getRowFromSite(sit));
    }
}

void dlg_listelieux::ModifCouleur()
{
    if (!wdg_tblview->selectionModel()->hasSelection())
        return;
    Site *sit = getSiteFromIndex(wdg_tblview->currentIndex());
    if (!sit)
        return;
    int row = wdg_tblview->currentIndex().row();
    QString couleurenreg = sit->couleur();
    QColor colordep = QColor("#FF" + couleurenreg);
    QColor colorfin = Utils::SelectCouleur(colordep, this);
    if (!colorfin.isValid())
        return;
    QString couleur = colorfin.name();
    QString fontcolor = "color:" + couleur;
    wdg_adressuplbl->setStyleSheet(fontcolor);
    couleur = couleur.replace("#","");
    ItemsList::update(sit, CP_COULEUR_SITE, couleur);
    ReconstruitModel();
    wdg_tblview->selectRow(row);
}

void dlg_listelieux::ModifLieuxDialog(Mode mode)
{
    auto modifcouleur = [&]
        {
            if (!wdg_tblview->selectionModel()->hasSelection())
                return;
            Site *sit = getSiteFromIndex(wdg_tblview->currentIndex());
            if (!sit)
                return;
            QString couleurenreg = sit->couleur();
            QColor colordep = QColor("#FF" + couleurenreg);
            QColor colorfin = Utils::SelectCouleur(colordep, this);
            if (!colorfin.isValid())
                return;
            QString couleur = colorfin.name();
            QString fontcolor = "color:" + couleur;
            wdg_adressuplbl->setStyleSheet(fontcolor);
            str_nouvcolor = couleur.replace("#","");
            dlg_lieu->OKButton->setEnabled(true);
        };
    auto nouvcouleur = [&]
        {
            QColor colordep = QColor(0xFF,0xFF,0xFF);
            QColor colorfin = Utils::SelectCouleur(colordep, this);
            if (!colorfin.isValid())
                return;
            QString couleur = colorfin.name();
            str_nouvcolor = couleur.replace("#","");
            dlg_lieu->OKButton->setEnabled(true);
        };

    dlg_lieu = new UpDialog(this);
    dlg_lieu->AjouteLayButtons(UpDialog::ButtonCancel | UpDialog::ButtonOK);
    dlg_lieu->setWindowModality(Qt::WindowModal);
    QVBoxLayout *laylbl = new QVBoxLayout();
    QVBoxLayout *layledit = new QVBoxLayout();
    QHBoxLayout *laycom = new QHBoxLayout();
    QVBoxLayout *lay = qobject_cast<QVBoxLayout*>(dlg_lieu->layout());

    UpLabel *lblnom = new UpLabel(dlg_lieu, tr("Nom de la structure"));
    UpLabel *lbladr1 = new UpLabel(dlg_lieu, tr("Adresse1"));
    UpLabel *lbladr2 = new UpLabel(dlg_lieu, tr("Adresse2"));
    UpLabel *lbladr3 = new UpLabel(dlg_lieu, tr("Adresse3"));
    UpLabel *lblcp = new UpLabel(dlg_lieu, tr("Code postal"));
    UpLabel *lblville = new UpLabel(dlg_lieu, tr("Ville"));
    UpLabel *lbltel = new UpLabel(dlg_lieu, tr("Telephone"));
    UpLabel *lblfax = new UpLabel(dlg_lieu, tr("Fax"));
    wdg_nouvcouleurpushbutt = new UpPushButton();
    wdg_nouvcouleurpushbutt->setFixedHeight(35);

    int h = 22;
    lblnom  ->setFixedHeight(h);
    lbladr1 ->setFixedHeight(h);
    lbladr2 ->setFixedHeight(h);
    lbladr3 ->setFixedHeight(h);
    lblcp   ->setFixedHeight(h);
    lblville->setFixedHeight(h);
    lbltel  ->setFixedHeight(h);
    lblfax  ->setFixedHeight(h);

    laylbl->addWidget(lblnom);
    laylbl->addWidget(lbladr1);
    laylbl->addWidget(lbladr2);
    laylbl->addWidget(lbladr3);
    laylbl->addWidget(lblcp);
    laylbl->addWidget(lblville);
    laylbl->addWidget(lbltel);
    laylbl->addWidget(lblfax);
    laylbl->addSpacerItem(new QSpacerItem(5,5,QSizePolicy::Expanding,QSizePolicy::Expanding));

    wdg_nomlineedit   = new UpLineEdit(dlg_lieu);
    wdg_adress1lineedit  = new UpLineEdit(dlg_lieu);
    wdg_adress2lineedit  = new UpLineEdit(dlg_lieu);
    wdg_adress3lineedit  = new UpLineEdit(dlg_lieu);
    wdg_CPlineedit    = new UpLineEdit(dlg_lieu);
    wdg_villelineedit = new UpLineEdit(dlg_lieu);
    wdg_tellineedit   = new UpLineEdit(dlg_lieu);
    wdg_faxlineedit   = new UpLineEdit(dlg_lieu);

    wdg_nomlineedit    ->setFixedWidth(240);       // NomLieu
    wdg_adress1lineedit   ->setFixedWidth(240);       // Adresse1
    wdg_adress2lineedit   ->setFixedWidth(240);       // Adresse2
    wdg_adress3lineedit   ->setFixedWidth(240);       // Adresse3
    wdg_CPlineedit     ->setFixedWidth(90);        // CP
    wdg_villelineedit  ->setFixedWidth(240);       // Ville
    wdg_tellineedit    ->setFixedWidth(120);       // Telephone
    wdg_faxlineedit    ->setFixedWidth(120);       // Fax

    wdg_nomlineedit    ->setMaxLength(80);
    wdg_adress1lineedit   ->setMaxLength(45);
    wdg_adress2lineedit   ->setMaxLength(45);
    wdg_adress3lineedit   ->setMaxLength(45);
    wdg_CPlineedit     ->setMaxLength(9);
    wdg_villelineedit  ->setMaxLength(45);
    wdg_tellineedit    ->setMaxLength(17);
    wdg_faxlineedit    ->setMaxLength(17);

    wdg_nomlineedit    ->setValidator(new QRegularExpressionValidator(Utils::rgx_ville));
    wdg_adress1lineedit   ->setValidator(new QRegularExpressionValidator(Utils::rgx_adresse));
    wdg_adress2lineedit   ->setValidator(new QRegularExpressionValidator(Utils::rgx_adresse));
    wdg_adress3lineedit   ->setValidator(new QRegularExpressionValidator(Utils::rgx_adresse));
    wdg_CPlineedit     ->setValidator(new QRegularExpressionValidator(Utils::rgx_CP));
    wdg_villelineedit  ->setValidator(new QRegularExpressionValidator(Utils::rgx_ville));
    wdg_tellineedit    ->setValidator(new QRegularExpressionValidator(Utils::rgx_telephone));
    wdg_faxlineedit    ->setValidator(new QRegularExpressionValidator(Utils::rgx_telephone));

    layledit->addWidget(wdg_nomlineedit);
    layledit->addWidget(wdg_adress1lineedit);
    layledit->addWidget(wdg_adress2lineedit);
    layledit->addWidget(wdg_adress3lineedit);
    layledit->addWidget(wdg_CPlineedit);
    layledit->addWidget(wdg_villelineedit);
    layledit->addWidget(wdg_tellineedit);
    layledit->addWidget(wdg_faxlineedit);
    layledit->addSpacerItem(new QSpacerItem(5,5,QSizePolicy::Expanding,QSizePolicy::Expanding));

    for (int i=0; i< dlg_lieu->findChildren<UpLineEdit*>().size(); i++)
        connect(dlg_lieu->findChildren<UpLineEdit*>().at(i), &QLineEdit::textEdited, this, [=]{    dlg_lieu->OKButton->setEnabled(true);});
    dlg_lieu->OKButton->setEnabled(false);

    laycom->addLayout(laylbl);
    laylbl->addSpacerItem(new QSpacerItem(10,10,QSizePolicy::Fixed,QSizePolicy::Fixed));
    laycom->addLayout(layledit);

    lay     ->insertLayout(0,laycom);
    lay     ->insertWidget(1, wdg_nouvcouleurpushbutt);
    lay     ->setSizeConstraint(QLayout::SetFixedSize);
    if (mode == Modif)
    {
        wdg_nouvcouleurpushbutt->setText(tr("modifier la couleur du texte"));
        connect(wdg_nouvcouleurpushbutt,    &QPushButton::clicked, modifcouleur);
        connect(dlg_lieu->OKButton,     &QPushButton::clicked, this, &dlg_listelieux::enregModifLieu);
    }
    else if (mode == Nouv)
    {
        wdg_nouvcouleurpushbutt->setText(tr("choisir la couleur du texte"));
        connect(wdg_nouvcouleurpushbutt,    &QPushButton::clicked, nouvcouleur);
        connect(dlg_lieu->OKButton,     &QPushButton::clicked, this, &dlg_listelieux::enregNouvLieu);
    }
}

void dlg_listelieux::ModifLieu()
{
    ModifLieuxDialog(Modif);
    Site * sit = getSiteFromIndex(wdg_tblview->currentIndex());
    if (sit == Q_NULLPTR)
        return;
    wdg_nomlineedit    ->setText(sit->nom());
    wdg_adress1lineedit->setText(sit->adresse1());
    wdg_adress2lineedit->setText(sit->adresse2());
    wdg_adress3lineedit->setText(sit->adresse3());
    wdg_CPlineedit     ->setText(sit->codepostal());
    wdg_villelineedit  ->setText(sit->ville());
    wdg_tellineedit    ->setText(sit->telephone());
    wdg_faxlineedit    ->setText(sit->fax());
    dlg_lieu->exec();
    delete  dlg_lieu;
    dlg_lieu = Q_NULLPTR;
}

void dlg_listelieux::enregModifLieu()
{
    if (!wdg_tblview->selectionModel()->hasSelection())
        return;
    Site *sit = getSiteFromIndex(wdg_tblview->selectionModel()->currentIndex());
    if (!sit)
        return;
    if (ValidationFiche())
    {
        m_listbinds[CP_NOM_SITE]          = Utils::trimcapitilize(wdg_nomlineedit->text(), true);
        m_listbinds[CP_ADRESSE1_SITE]     = Utils::trimcapitilize(wdg_adress1lineedit->text().toUpper(),true);
        m_listbinds[CP_ADRESSE2_SITE]     = Utils::trimcapitilize(wdg_adress2lineedit->text(),true);
        m_listbinds[CP_ADRESSE3_SITE]     = Utils::trimcapitilize(wdg_adress3lineedit->text(),true);
        m_listbinds[CP_CODEPOSTAL_SITE]   = Utils::trim(wdg_CPlineedit->text());
        m_listbinds[CP_VILLE_SITE]        = Utils::trimcapitilize(wdg_villelineedit->text(),true);
        m_listbinds[CP_TELEPHONE_SITE]    = Utils::trim(wdg_tellineedit->text());
        m_listbinds[CP_FAX_SITE]          = Utils::trim(wdg_faxlineedit->text());
        m_listbinds[CP_COULEUR_SITE]      = str_nouvcolor;

        DataBase::I()->UpdateTable(TBL_LIEUXEXERCICE, m_listbinds, " where " CP_ID_SITE " = " + QString::number(sit->id()),tr("Impossible de modifier le site"));
        sit = Datas::I()->sites->getById(sit->id(), true);
        ReconstruitModel();
        dlg_lieu->accept();
        wdg_tblview->selectRow(getRowFromSite(sit));
    }
}

void dlg_listelieux::SupprLieu()
{
    Site * sit = getSiteFromIndex(wdg_tblview->currentIndex());
    if (!sit)
        return;
    QString lieu = sit->nom();
    if (UpMessageBox::Question(this,tr("Suppression d'un lieu de soins"),tr("voulez vous vraiment supprimer") + "\n" + lieu + " ?") == UpSmallButton::STARTBUTTON)
    {
        int row = getRowFromSite(sit);
        m_model->takeRow(row);
        UpSystemTrayIcon::I()->showMessage(tr("Messages"), lieu + " supprimé", Icons::icSunglasses(), 3000);
        Datas::I()->sites->SupprimeSite(sit);
        ReconstruitModel();
        wdg_tblview->selectRow(0);
    }
}

bool dlg_listelieux::ValidationFiche()
{
    QString Msg = tr("Vous n'avez pas spécifié ");
    if (wdg_nomlineedit->text() == QString() || wdg_nomlineedit->text() == tr("non défini"))
    {
        Msg += tr("le nom de la structure de soins");
        wdg_nomlineedit->setFocus();
        UpMessageBox::Watch(dlg_lieu,Msg);
        return false;
    }
    if (wdg_CPlineedit->text().toInt() ==  0)
    {
        Msg += tr("le code postal");
        wdg_CPlineedit->setFocus();
        UpMessageBox::Watch(dlg_lieu,Msg);
        return false;
    }
    if (wdg_villelineedit->text() == QString())
    {
        Msg += tr("la ville");
        wdg_villelineedit->setFocus();
        UpMessageBox::Watch(dlg_lieu,Msg);
        return false;
    }
    return true;
}

void dlg_listelieux::ReconstruitModel()
{
    wdg_tblview->disconnect();
    wdg_tblview->selectionModel()->disconnect();
    if (m_model != Q_NULLPTR)
        delete m_model;
    m_model = new UpStandardItemModel();
    QStandardItem *pitem0   = new QStandardItem(tr("Structure de soins"));
    pitem0->setEditable(false);
    pitem0->setTextAlignment(Qt::AlignCenter);
    m_model->setHorizontalHeaderItem(0,pitem0);
    m_model->setRowCount(Datas::I()->sites->sites()->size());
    m_model->setColumnCount(1);

    foreach (Site* sit, *Datas::I()->sites->sites())
    {
        UpStandardItem *pitem0 = new UpStandardItem(sit->nom()==""? tr("non défini") : sit->nom(), sit);
        if (sit->couleur() != "")
            pitem0->setForeground(QBrush(QColor("#" + sit->couleur())));
        m_model->appendRow(QList<QStandardItem*>() << pitem0);
    }
    QItemSelectionModel *m = wdg_tblview->selectionModel(); // il faut détruire le selectionModel pour éviter des bugs d'affichage quand on réinitialise le modèle
    wdg_tblview->setModel(m_model);
    delete m;;
    m_model->sort(0);
    wdg_tblview->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    QHeaderView *verticalHeader = wdg_tblview->verticalHeader();
    verticalHeader->setSectionResizeMode(QHeaderView::Fixed);
    verticalHeader->setDefaultSectionSize(int(QFontMetrics(qApp->font()).height()*1.6));
    verticalHeader->setVisible(false);
    wdg_tblview->setColumnWidth(0,240);      // NomLieu
    wdg_tblview->FixLargeurTotale();
    wdg_buttonframe->widgButtonParent()->setFixedWidth(wdg_tblview->width());

    int h = int(QFontMetrics(qApp->font()).height()*1.1);
    for (int i=0; i < m_model->rowCount(); i++)
        wdg_tblview->setRowHeight(i, h);
    m_idlieuserveur = -1;
    m_idlieuserveur = db->parametres()->idlieupardefaut();
    connect(wdg_tblview->selectionModel(),   &QItemSelectionModel::currentRowChanged, this,  &dlg_listelieux::AfficheDetails);
    connect(wdg_tblview,    &QAbstractItemView::entered,       this,   [=] (QModelIndex idx) {
            Site *sit = getSiteFromIndex(idx);
            if (sit)
                QToolTip::showText(cursor().pos(), sit->coordonnees());
    } );
}

