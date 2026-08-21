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

#include "dlg_listesites.h"

dlg_listesites::dlg_listesites(QWidget *parent)
    : UpDialog(parent)
{
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

    connect(wdg_buttonframe,        &WidgetButtonFrame::choix,  this,   &dlg_listesites::ChoixButtonFrame);
    connect(wdg_couleurpushbutt,    &QPushButton::clicked,      this,   &dlg_listesites::ModifCouleur);
    setEnregPosition(true);
    setSaveGeometry(Nom_fiche_GestionLieux);
}

dlg_listesites::~dlg_listesites()
{
}

void dlg_listesites::AfficheDetails(QModelIndex idx, QModelIndex)
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
    int nlieux = db->StandardSelectSQL("select " CP_IDUSER_JOINTSITE " from " TBL_JOINTURESLIEUX " where " CP_IDLIEU_JOINTSITE " = " + QString::number(sit->id()), m_ok).size();
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

Site* dlg_listesites::getSiteFromIndex(QModelIndex idx)
{
    UpStandardItem *upitem = dynamic_cast<UpStandardItem *>(m_model->itemFromIndex(idx));
    if (upitem == nullptr)
        return nullptr;
    Site *sit = qobject_cast<Site *>(upitem->rufusitem());
    return sit;
}

int dlg_listesites::getRowFromSite(Site *sit)
{
    int row = -1;
    for (int i=0; i<m_model->rowCount(); i++)
    {
        UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_model->item(i));
        if(itm)
        {
            Site* sits = qobject_cast<Site*>(itm->rufusitem());
            if (sit->id() == sits->id())
            {
                row = i;
                break;
            }
        }
    }
    return row;
}

void dlg_listesites::ChoixButtonFrame()
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



void dlg_listesites::CreerLieu()
{
    dlg_identificationsite dlg(nullptr, dlg_identificationsite::Nouv, this);
    if (dlg.exec() != QDialog::Accepted)
        return;
    Site *sit = Datas::I()->sites->CreationSite(dlg.listbinds(), this);
    ReconstruitModel();
    wdg_tblview             ->selectRow(getRowFromSite(sit));
}

void dlg_listesites::ModifLieu()
{
    Site *sit = getSiteFromIndex(wdg_tblview->currentIndex());
    if (sit == nullptr)
        return;
    dlg_identificationsite dlg(sit, dlg_identificationsite::Modif, this);
    if (dlg.exec() != QDialog::Accepted)
        return;
    db                      ->UpdateTable(TBL_LIEUXEXERCICE, dlg.listbinds(), " where " CP_ID_SITE " = " + QString::number(sit->id()), tr("Impossible de modifier le site"));
    sit = Datas::I()->sites->getById(sit->id(), true);
    ReconstruitModel();
    wdg_tblview             ->selectRow(getRowFromSite(sit));
}

void dlg_listesites::ModifCouleur()
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




void dlg_listesites::SupprLieu()
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
        Datas::I()->sites->SupprimeSite(sit, this);
        ReconstruitModel();
        wdg_tblview->selectRow(0);
    }
}


void dlg_listesites::ReconstruitModel()
{
    wdg_tblview->disconnect();
    wdg_tblview->selectionModel()->disconnect();
    if (m_model != nullptr)
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
    connect(wdg_tblview->selectionModel(),   &QItemSelectionModel::currentRowChanged, this,  &dlg_listesites::AfficheDetails);
    connect(wdg_tblview,    &QAbstractItemView::entered,       this,   [=, this] (QModelIndex idx) {
            Site *sit = getSiteFromIndex(idx);
            if (sit)
                QToolTip::showText(cursor().pos(), sit->coordonnees());
    } );
}

