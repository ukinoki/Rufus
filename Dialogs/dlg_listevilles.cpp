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

#include "dlg_listevilles.h"
#include "icons.h"


dlg_listevilles::dlg_listevilles(QWidget *parent) :
    UpDialog(PATH_FILE_INI, "PositionsFiches/PositionVilles", parent)
{
    Datas::I()->villes->initListe(Villes::CUSTOM);
    m_mapvilles= Datas::I()->villes->villes();
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
    setWindowModality(Qt::WindowModal);
    wdg_tblview         = new UpTableView();
    wdg_buttonframe     = new WidgetButtonFrame(wdg_tblview);
    wdg_buttonframe     ->AddButtons(WidgetButtonFrame::Plus | WidgetButtonFrame::Modifier | WidgetButtonFrame::Moins);
    wdg_buttonframe     ->addSearchLine();

    AjouteLayButtons(UpDialog::ButtonCancel|UpDialog::ButtonOK);
    CancelButton    ->disconnect();
    dlglayout()     ->insertWidget(0,wdg_buttonframe->widgButtonParent());
    dlglayout()     ->setSizeConstraint(QLayout::SetFixedSize);
    setWindowTitle(tr("Liste des villes"));

    // Mise en forme de la table
    wdg_tblview->setPalette(QPalette(Qt::white));
    wdg_tblview->setGridStyle(Qt::NoPen);
    wdg_tblview->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    wdg_tblview->setEditTriggers(QAbstractItemView::NoEditTriggers);
    wdg_tblview->setSelectionMode(QAbstractItemView::SingleSelection);

    wdg_tblview->setMouseTracking(true);
    wdg_tblview->setContextMenuPolicy(Qt::CustomContextMenu);
    wdg_tblview->horizontalHeader()->setVisible(true);
    wdg_tblview->horizontalHeader()->setIconSize(QSize(30,30));
    wdg_tblview->setFixedHeight(435);

    m_complListVilles               = new QCompleter(this);
    m_complListVilles               ->setModel(new QStringListModel(Datas::I()->villes->ListeNomsVilles()));
    m_complListVilles               ->setCaseSensitivity(Qt::CaseInsensitive);
    m_complListVilles               ->setCompletionMode(QCompleter::PopupCompletion);
    m_complListVilles               ->setFilterMode(Qt::MatchStartsWith);
    wdg_buttonframe->searchline()   ->setCompleter(m_complListVilles);


    connect (CancelButton,                  &QPushButton::clicked,      this,   &dlg_listevilles::reject);
    connect (OKButton,                      &QPushButton::clicked,      this,   &dlg_listevilles::accept);

    connect (wdg_buttonframe,               &WidgetButtonFrame::choix,  this,   &dlg_listevilles::ChoixButtonFrame);
    connect (wdg_buttonframe->searchline(), &QLineEdit::textEdited,     this,   [=] (QString txt) { txt = Utils::trimcapitilize(txt, false, true);
                                                                                                    wdg_buttonframe->searchline()->setText(txt);
                                                                                                    for (int i = 0; i < Datas::I()->villes->ListeNomsVilles().size(); ++i)
                                                                                                    {
                                                                                                        QString nomville = Datas::I()->villes->ListeNomsVilles().at(i);
                                                                                                        if (nomville.toUpper().startsWith(txt.toUpper()))
                                                                                                        {
                                                                                                            QList<QStandardItem *> listitm = m_model->findItems(nomville,Qt::MatchExactly,1);
                                                                                                            if (listitm.size())
                                                                                                            {
                                                                                                                UpStandardItem *itm = dynamic_cast<UpStandardItem*>(listitm.at(0));
                                                                                                                if (itm)
                                                                                                                {
                                                                                                                    Ville *ville = dynamic_cast<Ville*>(itm->item());
                                                                                                                    if (ville)
                                                                                                                        selectcurrentVille(ville, QAbstractItemView::PositionAtCenter);
                                                                                                                }
                                                                                                            }
                                                                                                            break;
                                                                                                        }
                                                                                                    }
                                                                                                    });
    connect(m_complListVilles,    QOverload<const QString &>::of(&QCompleter::activated), this, [=] (QString s) {
        for (auto it = Datas::I()->villes->villes()->constBegin(); it != Datas::I()->villes->villes()->constEnd(); ++it)
        {
            Ville *ville = const_cast<Ville*>(it.value());
            if (ville->nom() == s)
            {
                selectcurrentVille(ville, QAbstractItemView::PositionAtCenter);
                break;
            }
        }
                                                                                                  });

    RemplirTableView();
    if (m_model->rowCount()>0)
    {
        Ville *ville = getVilleFromIndex(m_listnomsproxymodel->index(0,0));
        selectcurrentVille(ville);
    }
}

dlg_listevilles::~dlg_listevilles()
{
}

// ----------------------------------------------------------------------------------
// Recherche de doublon après création ou modification de ville
// ----------------------------------------------------------------------------------
bool dlg_listevilles::ChercheDoublon(QString cp, QString nom)
{
    bool a = false;
    for (int i=0;  i<m_model->rowCount(); i++)
    {
        UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_model->item(i));
        if (itm)
        {
            Ville *ville = dynamic_cast<Ville*>(itm->item());
            if (ville)
            {
                if (ville->codepostal().toUpper() == cp.toUpper() && ville->nom().toUpper() == nom.toUpper())
                {
                    a = true;
                    QString comment = tr("Il existe déjà une ville portant ce nom avec ce code postal");
                    UpMessageBox::Watch(dlg_ask, comment);
                    break;
                }
            }
        }
    }
    return a;
}

void dlg_listevilles::ChoixButtonFrame()
{
    switch (wdg_buttonframe->Choix()) {
    case WidgetButtonFrame::Plus:
        EnregistreNouvelleVille();
        break;
    case WidgetButtonFrame::Modifier:
        ModifieVille(m_currentville);
        break;
    case WidgetButtonFrame::Moins:
        SupprimmVille(m_currentville);
        break;
    }
}

void dlg_listevilles::ChoixMenuContextuel(QString choix)
{
    if (choix  == "Modifier")
        ModifieVille(m_currentville);
    else if (choix  == "Supprimer")
        SupprimmVille(m_currentville);
    else if (choix  == "Creer")
        EnregistreNouvelleVille();
}

// ------------------------------------------------------------------------------------------
// renvoie la ville correspondant à l'index
// ------------------------------------------------------------------------------------------
Ville* dlg_listevilles::getVilleFromIndex(QModelIndex idx)
{
    int row = m_listCPsproxymodel->mapToSource(m_listnomsproxymodel->mapToSource(idx)).row();
    UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_model->item(row));
    if (itm)
        return dynamic_cast<Ville*>(itm->item());
    else
        return Q_NULLPTR;
}

void dlg_listevilles::MenuContextuel()
{
    QMenu *menuContextuel = new QMenu(this);
    QAction *pAction_Modif;
    QAction *pAction_Suppr;
    QAction *pAction_Creer;

    pAction_Creer                = menuContextuel->addAction(Icons::icCreer(), tr("Créer une ville"));
    connect (pAction_Creer,      &QAction::triggered, this,   [=] {ChoixMenuContextuel("Creer");});

    QModelIndex idx   = wdg_tblview->indexAt(wdg_tblview->viewport()->mapFromGlobal(cursor().pos()));
    m_currentville = getVilleFromIndex(idx);
    if (m_currentville)
    {
        pAction_Modif                = menuContextuel->addAction(Icons::icEditer(), tr("Modifier cette ville"));
        pAction_Suppr                = menuContextuel->addAction(Icons::icPoubelle(), tr("Supprimer cette ville"));
        connect (pAction_Modif,      &QAction::triggered, this,   [=] {ChoixMenuContextuel("Modifier");});
        connect (pAction_Suppr,      &QAction::triggered, this,   [=] {ChoixMenuContextuel("Supprimer");});
    }
    // ouvrir le menu
    menuContextuel->exec(cursor().pos());
    delete menuContextuel;
}

// ----------------------------------------------------------------------------------
// Remplissage de la tablewidgetavec les villes de la base.
// ----------------------------------------------------------------------------------
void dlg_listevilles::RemplirTableView()
{
    wdg_tblview->disconnect();
    wdg_tblview->selectionModel()->disconnect();
    if (m_model != Q_NULLPTR)
        delete m_model;
    m_model = new UpStandardItemModel();
    QStandardItem *pitem0   = new QStandardItem(Icons::icImprimer(),"");
    pitem0->setEditable(false);
    pitem0->setTextAlignment(Qt::AlignCenter);
    m_model->setHorizontalHeaderItem(0,pitem0);
    QStandardItem *pitem1   = new QStandardItem(tr("LOCALITÉS"));
    pitem1->setEditable(false);
    pitem1->setTextAlignment(Qt::AlignLeft);
    m_model->setHorizontalHeaderItem(1,pitem1);

    m_model->setRowCount(m_mapvilles->size());
    m_model->setColumnCount(2);
    for (int i=0; i<m_mapvilles->size(); i++)
    {
        Ville *ville = m_mapvilles->values().at(i);
        setVilleToRow(ville, i);
    }
    QItemSelectionModel *m = wdg_tblview->selectionModel(); // il faut détruire le selectionModel pour éviter des bugs d'affichage quand on réinitialise le modèle
    delete m;

    if (m_listCPsproxymodel != Q_NULLPTR)
        delete m_listCPsproxymodel;
    m_listCPsproxymodel = new QSortFilterProxyModel();
    m_listCPsproxymodel->setSourceModel(m_model);
    m_listCPsproxymodel->sort(0);

    if (m_listnomsproxymodel != Q_NULLPTR)
        delete m_listnomsproxymodel;
    m_listnomsproxymodel = new QSortFilterProxyModel();
    m_listnomsproxymodel->setSourceModel(m_listCPsproxymodel);
    m_listnomsproxymodel->sort(1);
    m_listnomsproxymodel->setFilterKeyColumn(1);

    wdg_tblview->setModel(m_listnomsproxymodel);
    wdg_tblview->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    QHeaderView *verticalHeader = wdg_tblview->verticalHeader();
    verticalHeader->setSectionResizeMode(QHeaderView::Fixed);
    verticalHeader->setDefaultSectionSize(int(QFontMetrics(qApp->font()).height()*1.6));
    verticalHeader->setVisible(false);
    wdg_tblview->setColumnWidth(0,60);      // Code postal
    wdg_tblview->setColumnWidth(1,280);     // Ville
    wdg_tblview->FixLargeurTotale();
    wdg_buttonframe->widgButtonParent()->setFixedWidth(wdg_tblview->width());
    connect(wdg_tblview,     &QAbstractItemView::doubleClicked,         this,   [&] (QModelIndex idx) {
        Ville *ville = getVilleFromIndex(idx);
        if (ville)
            m_currentville = ville;
    });
    connect (wdg_tblview,    &QWidget::customContextMenuRequested,      this,   &dlg_listevilles::MenuContextuel);
    connect (wdg_tblview->selectionModel(),
             &QItemSelectionModel::currentRowChanged,
             this,
             [&] (QModelIndex idx)
    {
        m_currentville = getVilleFromIndex(idx);
        if (m_currentville)
        {
            wdg_buttonframe->searchline()->setText(m_currentville->nom());
            wdg_buttonframe->searchline()->selectAll();
            wdg_buttonframe->searchline()->setFocus();
        }
        wdg_buttonframe->wdg_moinsBouton->setEnabled(m_currentville != Q_NULLPTR);
        wdg_buttonframe->wdg_modifBouton->setEnabled(m_currentville != Q_NULLPTR);
    });
    m_currentville = Q_NULLPTR;
    wdg_buttonframe->wdg_moinsBouton->setEnabled(m_mapvilles->size()>0);
    wdg_buttonframe->wdg_modifBouton->setEnabled(m_mapvilles->size()>0);
    wdg_buttonframe->wdg_plusBouton->setEnabled(true);
}

void dlg_listevilles::selectcurrentVille(Ville *ville, QAbstractItemView::ScrollHint hint)
{
    if (!ville)
        return;
    if (wdg_tblview->selectionModel())
        if (wdg_tblview->selectionModel()->selectedIndexes().size() >0)
            wdg_tblview->selectionModel()->clear();
    m_currentville = ville;
    bool itemfound = false;
    for (int i=0; i<m_listnomsproxymodel->rowCount(); i++)
    {
        QModelIndex nomproxyindex = m_listnomsproxymodel->index(i,1);
        QModelIndex CPindex = m_listnomsproxymodel->mapToSource(nomproxyindex);
        QModelIndex sourceindex = m_listCPsproxymodel->mapToSource(CPindex);
        UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_model->itemFromIndex(sourceindex));
        if (itm)
        {
            Ville *vill = dynamic_cast<Ville*>(itm->item());
            if (vill)
                if (m_currentville == vill)
                {
                    itemfound = true;
                    QItemSelectionModel *model = wdg_tblview->selectionModel();
                    if (model)
                        model->select(nomproxyindex,QItemSelectionModel::SelectCurrent);
                    wdg_tblview->scrollTo(nomproxyindex, hint);
                    OKButton->setEnabled(true);
                    break;
                }
        }
    }
    wdg_buttonframe->wdg_moinsBouton->setEnabled(itemfound);
    wdg_buttonframe->wdg_modifBouton->setEnabled(itemfound);
}

void dlg_listevilles::setVilleToRow(Ville *ville, int row)
{
    if(!ville)
        return;
    if (row < 0 || row > m_model->rowCount()-1)
        return;
    UpStandardItem *pitem0 = new UpStandardItem(ville->codepostal(), ville);
    m_model->setItem(row,0,pitem0);
    UpStandardItem *pitem1 = new UpStandardItem(ville->nom(), ville);
    m_model->setItem(row,1, pitem1);
}

// ----------------------------------------------------------------------------------
// Supprime ville
// ----------------------------------------------------------------------------------
void dlg_listevilles::SupprimmVille(Ville* ville)
{
    if (!ville)
        return;

    QString Msg;
    Msg = tr("Etes vous sûr de vouloir supprimer l'association ") + "\n" + ville->codepostal() + " " + ville->nom() + "?";
    UpMessageBox msgbox(this);
    msgbox.setText("Euuhh... " + Datas::I()->users->userconnected()->login() + "?");
    msgbox.setInformativeText(Msg);
    msgbox.setIcon(UpMessageBox::Warning);
    UpSmallButton OKBouton;
    OKBouton.setImmediateToolTip(tr("Supprimer ") + ville->codepostal() + " " + ville->nom());
    UpSmallButton NoBouton;
    NoBouton.setImmediateToolTip(tr("Annuler"));
    msgbox.addButton(&NoBouton,UpSmallButton::CANCELBUTTON);
    msgbox.addButton(&OKBouton, UpSmallButton::STARTBUTTON);
    msgbox.setDefaultButton(&NoBouton);
    msgbox.exec();
    if (msgbox.clickedButton()  == &OKBouton)
    {
        int row = m_model->getRowFromItem(ville);
        QString nom ("");
        QString cp("");
        QModelIndex idx = m_model->index(row,0);
        idx = m_listCPsproxymodel->mapFromSource(idx);
        idx = m_listnomsproxymodel->mapFromSource(idx);
        if (m_model->rowCount()>1)
        {
            int idxrow = idx.row();
            if (idx.row() > 1)
                idxrow -= 1;
            idx = m_listnomsproxymodel->index(idxrow,1);
            Ville *ville = getVilleFromIndex(idx);
            if (ville)
            {
                nom = ville->nom();
                cp  = ville->codepostal();
            }
        }
        Datas::I()->villes->SupprimeVille(ville);
        if (Datas::I()->villes->villes()->size() >0)
        {
            QList<Ville*> listville = Datas::I()->villes->getVilleByCodePostalEtNom(cp, nom);
            if (listville.size() >0)
                ville = listville.at(0);
            RemplirTableView();
            QStringListModel *model = dynamic_cast<QStringListModel*> (m_complListVilles->completionModel());
            if (model)
                model->setStringList(Datas::I()->villes->ListeNomsVilles());
            if (ville != Q_NULLPTR)
                selectcurrentVille(ville);
        }
    }
}

void dlg_listevilles::ModifieVille(Ville *ville)
{
    dialogville(ville->codepostal(), ville->nom());
    connect(dlg_ask->OKButton,    &QPushButton::clicked, this, [=]  {
                                                                        if (!ChercheDoublon(Utils::trimcapitilize(cpline->text()), Utils::trimcapitilize(nomline->text())))
                                                                        {
                                                                            ItemsList::update(ville, CP_CP_AUTRESVILLES,  Utils::trimcapitilize(cpline->text().toUpper()));
                                                                            ItemsList::update(ville, CP_NOM_AUTRESVILLES, Utils::trimcapitilize(nomline->text()));
                                                                            Datas::I()->villes->ReinitMaps();
                                                                            dlg_ask->close();
                                                                            RemplirTableView();
                                                                            QStringListModel *model = dynamic_cast<QStringListModel*> (m_complListVilles->model());
                                                                            if (model)
                                                                                model->setStringList(Datas::I()->villes->ListeNomsVilles());
                                                                            selectcurrentVille(ville);
                                                                            wdg_buttonframe->searchline()->setText(ville->nom());
                                                                            wdg_buttonframe->searchline()->selectAll();
                                                                            wdg_buttonframe->searchline()->setFocus();
                                                                        }
                                                                    });
    dlg_ask->exec();
}

void dlg_listevilles::EnregistreNouvelleVille()
{
    dialogville("","");
    connect(dlg_ask->OKButton,    &QPushButton::clicked, this, [=]  {
                                                                        Ville * ville = Datas::I()->villes->enregistreNouvelleVille(Utils::trimcapitilize(cpline->text()), Utils::trimcapitilize(nomline->text()));
                                                                        if (ville != Q_NULLPTR)
                                                                        {
                                                                            RemplirTableView();
                                                                            QStringListModel *model = dynamic_cast<QStringListModel*> (m_complListVilles->model());
                                                                            if (model)
                                                                                model->setStringList(Datas::I()->villes->ListeNomsVilles());
                                                                            selectcurrentVille(ville);
                                                                            dlg_ask->close();
                                                                            wdg_buttonframe->searchline()->setText(ville->nom());
                                                                            wdg_buttonframe->searchline()->selectAll();
                                                                            wdg_buttonframe->searchline()->setFocus();
                                                                        }
                                                                    });
    dlg_ask->exec();
}

void dlg_listevilles::dialogville(QString cp, QString nom)
{
    if (dlg_ask != Q_NULLPTR)
        delete dlg_ask;
    dlg_ask                 = new UpDialog(this);
    dlg_ask                 ->setWindowModality(Qt::WindowModal);

    QHBoxLayout *cplay      = new QHBoxLayout;
    cpline                  = new UpLineEdit();
    cpline                  ->setValidator(new QRegExpValidator(Utils::rgx_CP,this));
    cpline                  ->setAlignment(Qt::AlignCenter);
    cpline                  ->setMaxLength(10);
    cpline                  ->setText(cp);
    cpline                  ->setFixedWidth(60);
    cplay                   ->insertWidget(0,cpline);
    cplay                   ->insertSpacerItem(0,new QSpacerItem(20,0,QSizePolicy::Expanding, QSizePolicy::Expanding));
    UpLabel *labelCP        = new UpLabel();
    labelCP                 ->setAlignment(Qt::AlignRight);
    labelCP                 ->setText(tr("Code postal (facultatif)"));
    cplay                   ->insertWidget(0,labelCP);
    dlg_ask->dlglayout()    ->insertLayout(0,cplay);

    dlg_ask->dlglayout()    ->insertSpacerItem(0,new QSpacerItem(0,10,QSizePolicy::Expanding, QSizePolicy::Expanding));

    QHBoxLayout *nomlay     = new QHBoxLayout;
    nomline                 = new UpLineEdit();
    nomline                 ->setValidator(new QRegExpValidator(Utils::rgx_ville,this));
    nomline                 ->setAlignment(Qt::AlignCenter);
    nomline                 ->setMaxLength(45);
    nomline                 ->setText(nom);
    nomline                 ->setFixedWidth(280);
    nomlay                  ->insertWidget(0,nomline);
    nomlay                  ->insertSpacerItem(0,new QSpacerItem(20,0,QSizePolicy::Expanding, QSizePolicy::Expanding));
    UpLabel *labelVille     = new UpLabel();
    labelVille              ->setAlignment(Qt::AlignRight);
    labelVille              ->setText(tr("Nom"));
    nomlay                  ->insertWidget(0,labelVille);
    dlg_ask->dlglayout()    ->insertLayout(0,nomlay);

    dlg_ask->dlglayout()    ->insertSpacerItem(0,new QSpacerItem(0,10,QSizePolicy::Expanding, QSizePolicy::Expanding));

    dlg_ask                 ->AjouteLayButtons(UpDialog::ButtonCancel | UpDialog::ButtonOK);
    dlg_ask                 ->setWindowTitle(tr("Enregistrement d'une localité"));
    dlg_ask->dlglayout()    ->setSizeConstraint(QLayout::SetFixedSize);
    connect (nomline,       &QLineEdit::textEdited,                  this,   [=] {nomline->setText(Utils::trimcapitilize(nomline->text(),false));;});
    connect (dlg_ask->CancelButton, &QPushButton::clicked,  dlg_ask, &QDialog::reject);
    nomline                 ->setFocus();
}

