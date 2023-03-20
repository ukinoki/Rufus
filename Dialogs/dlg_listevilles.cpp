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
    UpDialog(PATH_FILE_INI, "PositionsFiches/PositionCommentaires", parent)
{
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

    wdg_tblview->installEventFilter(this);

    QCompleter *complListVilles     = new QCompleter(new QStringListModel(Datas::I()->villes->ListeNomsVilles()));
    complListVilles                 ->setCaseSensitivity(Qt::CaseInsensitive);
    complListVilles                 ->setCompletionMode(QCompleter::PopupCompletion);
    complListVilles                 ->setFilterMode(Qt::MatchStartsWith);
    wdg_buttonframe->searchline()   ->setCompleter(complListVilles);



    connect (CancelButton,                  &QPushButton::clicked,      this,   &dlg_listevilles::Annulation);
    connect (wdg_buttonframe,               &WidgetButtonFrame::choix,  this,   &dlg_listevilles::ChoixButtonFrame);
    connect(wdg_buttonframe->searchline(),  &QLineEdit::textEdited,     this,   [=] (QString txt) { txt = Utils::trimcapitilize(txt, false, true);
                                                                                                    wdg_buttonframe->searchline()->setText(txt);
                                                                                                    for (auto it = Datas::I()->villes->villes()->constBegin(); it != Datas::I()->villes->villes()->constEnd(); ++it)
                                                                                                    {
                                                                                                        Ville *ville = const_cast<Ville*>(it.value());
                                                                                                        if (ville->nom().toUpper().startsWith(txt.toUpper()))
                                                                                                        {
                                                                                                            selectcurrentVille(ville, QAbstractItemView::PositionAtCenter);
                                                                                                            break;
                                                                                                        }
                                                                                                    }
                                                                                                    });

    RemplirTableView();
    if (m_model->rowCount()>0)
    {
        Ville *ville = getVilleFromIndex(m_model->index(0,0));
        selectcurrentVille(ville);
    }
}

dlg_listevilles::~dlg_listevilles()
{
}

void dlg_listevilles::keyPressEvent(QKeyEvent * event )
{
    switch (event->key()) {
    case Qt::Key_Escape:
    {
        Annulation();
        break;
    }
    default:
        break;
    }
}

// ----------------------------------------------------------------------------------
// Clic sur le bouton ANNULER.
// L'action depend de ce qu'on est en train de faire (creation modife, selection)
// ----------------------------------------------------------------------------------
void dlg_listevilles::Annulation()
{
    wdg_tblview->setEditTriggers(QAbstractItemView::NoEditTriggers);
    if (m_mode == Creation || m_mode == Modification)
    {
        int row = m_model->getRowFromItem(m_currentville);
        if (m_mode == Modification && row < m_model->rowCount())
        {
            QModelIndex idx = m_model->index(row,1);
            wdg_tblview->closePersistentEditor(idx);
        }
        else if (m_mode == Creation)
        {
            m_model->removeRow(row);
            if(m_model->rowCount() > 0 && row < m_model->rowCount())
                selectcurrentVille(getVilleFromIndex(m_model->index(row,1)));
        }
        else
            RemplirTableView();
        ConfigMode(Selection);
    }
    else
        reject();
}

// ----------------------------------------------------------------------------------
// Recherche de doublon après création ou modification de ville
// ----------------------------------------------------------------------------------
bool dlg_listevilles::ChercheDoublon(QString str, int row)
{
    bool a = false;
    switch (m_mode) {
    case Creation:
    case Modification:
        for (int i=0;  i<m_model->rowCount(); i++)
        {
            UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_model->item(i));
            if (itm)
            {
                Ville *ville = dynamic_cast<Ville*>(itm->item());
                if (ville)
                {
                    if (ville->codepostal().toUpper() == str.toUpper() && i != row)
                    {
                        a = true;
                        QString comment = tr("Il existe déjà une ville portant ce nom avec ce code postal");
                        UpMessageBox::Watch(this, comment);
                        break;
                    }
                }
            }
        }
        break;
    default:
        break;
    }
    return a;
}

void dlg_listevilles::ChoixButtonFrame()
{
    switch (wdg_buttonframe->Choix()) {
    case WidgetButtonFrame::Plus:
        ConfigMode(Creation, m_currentville);
        break;
    case WidgetButtonFrame::Modifier:
        ConfigMode(Modification,m_currentville);
        break;
    case WidgetButtonFrame::Moins:
        SupprimmVille(m_currentville);
        break;
    }
}

void dlg_listevilles::ChoixMenuContextuel(QString choix)
{
    if (choix  == "Modifier")
        ConfigMode(Modification,m_currentville);
    else if (choix  == "Supprimer")
        SupprimmVille(m_currentville);

    else if (choix  == "Creer")
        ConfigMode(Creation,m_currentville);
}

// ----------------------------------------------------------------------------------
// Configuration de la fiche en fonction du mode de fonctionnement
// ----------------------------------------------------------------------------------
void dlg_listevilles::ConfigMode(Mode mode, Ville *ville)
{
    m_mode = mode;
    wdg_buttonframe->setEnabled(m_mode == Selection);

    if (mode == Selection)
    {
        wdg_tblview->setEnabled(true);
        wdg_tblview->setStyleSheet("");
        wdg_buttonframe->wdg_modifBouton->setEnabled(false);
        wdg_buttonframe->wdg_moinsBouton->setEnabled(false);

        CancelButton->setUpButtonStyle(UpSmallButton::CANCELBUTTON);
        CancelButton->setImmediateToolTip(tr("Annuler et fermer la fiche"));
        if (ville)
            selectcurrentVille(ville, QAbstractItemView::PositionAtCenter);
        OKButton->setEnabled(true);
    }

    if (mode == Modification)
    {
        if (!ville)
            return;
        for (int i=0; i<m_model->rowCount(); ++i)
        {
            UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_model->item(i,0));
            if (itm)
            {
                Ville *coms = dynamic_cast<Ville*>(itm->item());
                if (coms)
                    if (coms == ville)
                    {
                        m_model->item(i,1)->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
                        wdg_tblview->setFocus();
                        wdg_tblview->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
                        wdg_tblview->openPersistentEditor(m_model->index(i,1));
                        i = m_model->rowCount();
                    }
            }
        }
        wdg_tblview->setEnabled(true);
        wdg_tblview->setStyleSheet("");

        CancelButton->setUpButtonStyle(UpSmallButton::BACKBUTTON);
        CancelButton->setImmediateToolTip(tr("Revenir au mode\nsélection de commentaire"));
        OKButton->setImmediateToolTip(tr("Enregistrer"));
        OKButton->setEnabled(false);
    }
    else if (mode == Creation)
    {
        int row = 0;
        if (ville)
            row = m_model->getRowFromItem(ville);
        m_model->insertRow(row);
        m_currentville = new Ville();
        m_currentville->setnom(tr("Nouvelle ville"));
        setVilleToRow(m_currentville, row);
        m_model->item(row,1)->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        wdg_tblview->scrollTo(m_model->index(row,1), QAbstractItemView::EnsureVisible);
        wdg_tblview->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
        wdg_tblview->setCurrentIndex(m_model->index(row,1));
        wdg_tblview->openPersistentEditor(m_model->index(row,1));

        CancelButton->setUpButtonStyle(UpSmallButton::BACKBUTTON);
        CancelButton->setEnabled(true);
        CancelButton->setImmediateToolTip(tr("Revenir au mode\nsélection de commentaire"));
        OKButton->setEnabled(false);
        OKButton->setImmediateToolTip(tr("Enregistrer\nle commentaire"));
    }
}

// ----------------------------------------------------------------------------------
// Modification du commentaire dans la base.
// ----------------------------------------------------------------------------------
bool dlg_listevilles::EnregistreVille(Ville *ville)
{
    int row = m_model->getRowFromItem(ville);
    UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_model->item(row,0));
    if (!itm)
        return false;
    UpStandardItem *itmdef = dynamic_cast<UpStandardItem*>(m_model->item(row,1));
    if (!itmdef)
        return false;
    wdg_tblview->closePersistentEditor(m_model->index(row,1));
    qApp->focusWidget()->clearFocus();   //permet de déclencher le focusout du delegate qui va lancer le signal commiData

    // recherche de l'enregistrement modifié
    // controle validate des champs
    if (ChercheDoublon(ville->nom(),row))
    {
        wdg_tblview->openPersistentEditor(m_model->index(row,1));
        return false;
    }
    m_listbinds[CP_CP_AUTRESVILLES] = Utils::trim(itm->text());
    m_listbinds[CP_NOM_AUTRESVILLES] = Utils::trim(itmdef->text());
    if (m_mode == Creation)
    {
        delete  ville;
        m_currentville = Datas::I()->villes->enregistreNouvelleVille(Utils::trim(itm->text()),Utils::trim(itmdef->text()));
        setVilleToRow(m_currentville, row);
    }
    else if (m_mode == Modification)
    {
        DataBase::I()->UpdateTable(TBL_AUTRESVILLES, m_listbinds, "where " CP_ID_COMLUN " = " + QString::number(ville->id()));
        //m_currentville = Datas::I()->villes->getById(idcom, true);
    }
    if (m_currentville)
    {
        m_model->sort(1);
        selectcurrentVille(m_currentville);
    }
    return true;
}

// ------------------------------------------------------------------------------------------
// renvoie le commentaire correspondant à l'index
// ------------------------------------------------------------------------------------------
Ville* dlg_listevilles::getVilleFromIndex(QModelIndex idx)
{
    int row = idx.row();
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
    QAction *pAction_Recopier;

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
        pAction_Recopier             = menuContextuel->addAction(Icons::icCopy(), tr("Recopier cette ville"));
        connect (pAction_Recopier,   &QAction::triggered, this,   [=] {ChoixMenuContextuel("Recopier");});
    }
    // ouvrir le menu
    menuContextuel->exec(cursor().pos());
    delete menuContextuel;
}

// ----------------------------------------------------------------------------------
// Remplissage de la tablewidgetavec les commentaires de la base.
// ----------------------------------------------------------------------------------
void dlg_listevilles::RemplirTableView()
{
    wdg_tblview->disconnect();
    wdg_tblview->selectionModel()->disconnect();
    if (m_model != Q_NULLPTR)
        delete m_model;
    m_model = new UpStandardItemModel();
    UpLineDelegate *line = new UpLineDelegate();
    wdg_tblview->setItemDelegateForColumn(1,line);
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
        setVilleToRow(ville, i, false);
    }
    QItemSelectionModel *m = wdg_tblview->selectionModel(); // il faut détruire le selectionModel pour éviter des bugs d'affichage quand on réinitialise le modèle
    wdg_tblview->setModel(m_model);
    delete m;
    if (m_model->rowCount()>0)
    {
        m_model->sort(1);
        wdg_tblview->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
        QHeaderView *verticalHeader = wdg_tblview->verticalHeader();
        verticalHeader->setSectionResizeMode(QHeaderView::Fixed);
        verticalHeader->setDefaultSectionSize(int(QFontMetrics(qApp->font()).height()*1.6));
        verticalHeader->setVisible(false);
        wdg_tblview->setColumnWidth(0,30);      // Check
        wdg_tblview->setColumnWidth(1,380);     // Resumé
        wdg_tblview->setColumnWidth(2,30);      // DefautIcon
        wdg_tblview->setColumnWidth(3,30);      // PublicIcon
        wdg_tblview->FixLargeurTotale();
        wdg_buttonframe->widgButtonParent()->setFixedWidth(wdg_tblview->width());
        ConfigMode(Selection);
        connect(wdg_tblview,     &QAbstractItemView::doubleClicked,         this,   [&] (QModelIndex idx) {
                                                                                                            Ville *ville = getVilleFromIndex(idx);
                                                                                                            if (ville)
                                                                                                            {
                                                                                                                m_currentville = ville;
                                                                                                                ConfigMode(Modification,m_currentville);
                                                                                                            }
                                                                                                          });
        connect (wdg_tblview,    &QWidget::customContextMenuRequested,      this,   &dlg_listevilles::MenuContextuel);
        connect (wdg_tblview->selectionModel(),                   &QItemSelectionModel::currentRowChanged,          this,   [&] (QModelIndex idx) {
                                                                                                                    m_currentville = getVilleFromIndex(idx);
                                                                                                                    if (m_currentville)
                                                                                                                    {
                                                                                                                        OKButton->setEnabled(true);
                                                                                                                    }
                                                                                                                });
        m_currentville = Q_NULLPTR;
    }
    else
        ConfigMode(Creation);
}

void dlg_listevilles::selectcurrentVille(Ville *com, QAbstractItemView::ScrollHint hint)
{
    if (wdg_tblview->selectionModel())
        if (wdg_tblview->selectionModel()->selectedIndexes().size() >0)
            wdg_tblview->selectionModel()->clear();
    m_currentville = com;
    for (int i=0; i<m_model->rowCount(); i++)
    {
        UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_model->item(i));
        if (itm)
        {
            Ville *coms = dynamic_cast<Ville*>(itm->item());
            if (coms)
                if (m_currentville == coms)
                {
                    QModelIndex idx = m_model->index(i,1);
                    wdg_tblview->selectionModel()->select(idx,QItemSelectionModel::SelectCurrent);
                    wdg_tblview->scrollTo(idx, hint);
                    OKButton->setEnabled(true);
                    break;
                }
        }
    }
}

void dlg_listevilles::setVilleToRow(Ville *ville, int row, bool resizecolumn)
{
    if(!ville)
        return;
    if (row < 0 || row > m_model->rowCount()-1)
        return;
    QFont disabledFont = qApp->font();
    disabledFont.setItalic(true);

    UpStandardItem *pitem0 = new UpStandardItem(ville->codepostal(), ville);
    m_model->setItem(row,0,pitem0);
    UpStandardItem *pitem1 = new UpStandardItem(ville->nom(), ville);
    pitem1->setFlags(Qt::NoItemFlags);
    m_model->setItem(row,1, pitem1);
    if(!resizecolumn)
        return;

    //! la suite est obligatoire pour contourner un bug d'affichage sous MacOS
    wdg_tblview->setColumnWidth(0,60);      // codepostal
    wdg_tblview->setColumnWidth(1,380);     // Ville
}

// ----------------------------------------------------------------------------------
// Supprime commentaire
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
        if (row > -1 && row < m_model->rowCount())
        {
            m_model->removeRow(row);
            Datas::I()->villes->SupprimeVille(ville);
        }
        if (m_model->rowCount() == 0)
            ConfigMode(Creation);
        else
        {
            ConfigMode(Selection);
            if (row >= m_model->rowCount())
                row = m_model->rowCount()-1;
            ville = getVilleFromIndex(m_model->index(row,0));
            if (ville)
                selectcurrentVille(ville);
        }
    }
}

