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

#include "dlg_listecommentaires.h"
#include "icons.h"


dlg_listecommentaires::dlg_listecommentaires(QWidget *parent) :
    UpDialog(PATH_FILE_INI, "PositionsFiches/PositionCommentaires", parent)
{
    //ui->setupUi(this);
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
    wdg_tblview = new UpTableView();
    wdg_comtxt = new UpTextEdit();
    wdg_buttonframe = new WidgetButtonFrame(wdg_tblview);
    wdg_buttonframe->AddButtons(WidgetButtonFrame::Plus | WidgetButtonFrame::Modifier | WidgetButtonFrame::Moins);

    AjouteLayButtons(UpDialog::ButtonCancel|UpDialog::ButtonOK);
    CancelButton    ->disconnect();
    dlglayout()     ->insertWidget(0,wdg_comtxt);
    dlglayout()     ->insertWidget(0,wdg_buttonframe->widgButtonParent());
    dlglayout()     ->setSizeConstraint(QLayout::SetFixedSize);
    setWindowTitle(tr("Liste des commentaires prédéfinis de ") + currentuser()->login());

    // Mise en forme de la table
    wdg_tblview->setPalette(QPalette(Qt::white));
    wdg_tblview->setGridStyle(Qt::NoPen);
    wdg_tblview->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    wdg_tblview->setEditTriggers(QAbstractItemView::NoEditTriggers);
    wdg_tblview->setSelectionMode(QAbstractItemView::SingleSelection);
    //wdg_comtbl->setSelectionBehavior(QAbstractItemView::SelectRows);
    wdg_tblview->setMouseTracking(true);
    wdg_tblview->setContextMenuPolicy(Qt::CustomContextMenu);
    wdg_tblview->horizontalHeader()->setVisible(true);
    wdg_tblview->horizontalHeader()->setIconSize(QSize(30,30));
    wdg_tblview->setFixedHeight(435);

    wdg_comtxt->installEventFilter(this);
    wdg_comtxt->setFixedHeight(80);
    wdg_tblview->installEventFilter(this);

    connect (OKButton,          &QPushButton::clicked,      this,   &dlg_listecommentaires::Validation);
    connect (CancelButton,      &QPushButton::clicked,      this,   &dlg_listecommentaires::Annulation);
    connect (wdg_comtxt,        &QTextEdit::textChanged,    this,   [&] { OKButton->setEnabled(true);});
    connect (wdg_comtxt,        &UpTextEdit::dblclick,      this,   &dlg_listecommentaires::dblClicktextEdit);
    connect (wdg_buttonframe,   &WidgetButtonFrame::choix,  this,   &dlg_listecommentaires::ChoixButtonFrame);

    QList<int> listiduser;
    listiduser << currentuser()->id();
    if (currentuser()->idsuperviseur() != currentuser()->id())
        listiduser << currentuser()->idsuperviseur();
    if ((currentuser()->idparent() != currentuser()->idsuperviseur()) && (currentuser()->idparent() != currentuser()->id()))
        listiduser << currentuser()->idparent();
    Datas::I()->commentslunets->initListeByListUsers(listiduser);
    RemplirTableView();
}

dlg_listecommentaires::~dlg_listecommentaires()
{
}

/* ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Interception des évènements internes -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
bool dlg_listecommentaires::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress)
    {
        if (obj == wdg_comtxt)
        {
            if (wdg_comtxt->toPlainText() != "")
                OKButton->setEnabled(true);
        }
    }
    return QWidget::eventFilter(obj, event);
}

void dlg_listecommentaires::keyPressEvent(QKeyEvent * event )
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
void dlg_listecommentaires::Annulation()
{
    if (m_mode == Creation || m_mode == Modification)
    {
        wdg_tblview->setEditTriggers(QAbstractItemView::NoEditTriggers);
        int row = getRowFromComment(m_currentcomment);
        if (m_mode == Modification && row < m_model->rowCount())
        {
            m_model->setData(m_model->index(row,1), m_currentcomment->resume());
            ConfigMode(Selection, m_currentcomment);
        }
        else if (m_mode == Creation && m_currentcomment)
        {
            RemplirTableView();
            delete m_currentcomment;
            if(m_model->rowCount() > 0 && row < m_model->rowCount())
            {
                wdg_tblview->setCurrentIndex(m_model->index(row,1));
                m_currentcomment = getCommentFromIndex(m_model->index(row,1));
            }
        }
        else
            RemplirTableView();
    }
    else
        reject();
}

// ----------------------------------------------------------------------------------
// Recherche de doublon après création ou modification de document
// ----------------------------------------------------------------------------------
bool dlg_listecommentaires::ChercheDoublon(QString str, int row)
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
                CommentLunet *com = dynamic_cast<CommentLunet*>(itm->item());
                if (com)
                {
                    if (com->texte().toUpper() == str.toUpper() && i != row)
                    {
                        a = true;
                        QString b = " " + tr(" créé par vous");
                        int iduser = com->iduser();
                        QString comment = tr("Il existe déjà un commentaire portant ce nom");
                        if (iduser != currentuser()->id())
                        {
                            if (Datas::I()->users->getById(iduser) != Q_NULLPTR)
                                b = " " + tr("créé par") + " " + Datas::I()->users->getById(iduser)->login();
                            else
                                b = "";
                        }
                        UpMessageBox::Watch(this, comment + b);
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

void dlg_listecommentaires::ChoixButtonFrame()
{
    switch (wdg_buttonframe->Choix()) {
    case WidgetButtonFrame::Plus:
        ConfigMode(Creation, m_currentcomment);
        break;
    case WidgetButtonFrame::Modifier:
        ConfigMode(Modification,m_currentcomment);
        break;
    case WidgetButtonFrame::Moins:
        SupprimmCommentaire(m_currentcomment);
        break;
    }
}

void dlg_listecommentaires::ChoixMenuContextuel(QString choix)
{
    if (choix  == "Modifier")
        ConfigMode(Modification,m_currentcomment);
    else if (choix  == "Supprimer")
        SupprimmCommentaire(m_currentcomment);
    else if (choix  == "ParDefaut")
    {
        ItemsList::update(m_currentcomment, CP_PARDEFAUT_COMLUN,!m_currentcomment->isdefaut());
        int row = getRowFromComment(m_currentcomment);
        setCommentToRow(m_currentcomment, row);
    }
    else if (choix  == "Creer")
    {
        ConfigMode(Creation,m_currentcomment);
    }
}

// ----------------------------------------------------------------------------------
// Configuration de la fiche en fonction du mode de fonctionnement
// ----------------------------------------------------------------------------------
void dlg_listecommentaires::ConfigMode(Mode mode, CommentLunet *com)
{
    m_mode = mode;
    wdg_comtxt->setVisible (m_mode != Selection);
    wdg_buttonframe->setEnabled(m_mode == Selection);

    if (mode == Selection)
    {
        EnableLines();
        wdg_tblview->setEnabled(true);
        wdg_tblview->setFocus();
        wdg_tblview->setStyleSheet("");
        wdg_buttonframe->wdg_modifBouton->setEnabled(false);
        wdg_buttonframe->wdg_moinsBouton->setEnabled(false);
        wdg_comtxt->setEnabled(true);
        wdg_comtxt->clear();
        wdg_comtxt->setFocusPolicy(Qt::NoFocus);
        wdg_comtxt->setStyleSheet("");

        CancelButton->setUpButtonStyle(UpSmallButton::CANCELBUTTON);
        CancelButton->setImmediateToolTip(tr("Annuler et fermer la fiche"));
        OKButton->setImmediateToolTip(tr("Imprimer\nla sélection"));
        if (com)
            selectcurrentComment(com);

        int nbCheck = 0;
        for (int i =0 ; i < m_model->rowCount(); i++)
        {
            UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_model->item(i));
            if (itm)
                if (itm->checkState() == Qt::Checked)
                    nbCheck ++;
        }
        OKButton->setEnabled(nbCheck);
    }

    if (mode == Modification)
    {
        if (!com)
            return;
        DisableLines();
        for (int i=0; i<m_model->rowCount(); ++i)
        {
            UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_model->item(i,0));
            if (itm)
            {
                CommentLunet *coms = dynamic_cast<CommentLunet*>(itm->item());
                if (coms)
                    if (coms == com)
                    {
                        m_model->item(i,1)->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
                        wdg_tblview->setFocus();
                        wdg_tblview->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
                        wdg_tblview->setCurrentIndex(m_model->index(i,1));
                        wdg_tblview->openPersistentEditor(m_model->index(i,1));
                        wdg_comtxt->setText(com->texte());
                        i = m_model->rowCount();
                    }
            }
        }
        wdg_tblview->setEnabled(true);
        wdg_tblview->setStyleSheet("");
        wdg_comtxt->setFocusPolicy(Qt::WheelFocus);
        wdg_comtxt->setStyleSheet("border: 2px solid rgb(251, 51, 61);");

        CancelButton->setUpButtonStyle(UpSmallButton::BACKBUTTON);
        CancelButton->setImmediateToolTip(tr("Revenir au mode\nsélection de commentaire"));
        OKButton->setImmediateToolTip(tr("Enregistrer"));
        OKButton->setEnabled(false);
    }
    else if (mode == Creation)
    {
        if (m_model->rowCount() > 0)
            DisableLines();
        int row = 0;
        if (com)
            row = getRowFromComment(com);
        m_model->insertRow(row);
        m_currentcomment = new CommentLunet();
        m_currentcomment->setresume(tr("Nouveau Commentaire"));
        m_currentcomment->setiduser(currentuser()->id());
        setCommentToRow(m_currentcomment, row);
        m_model->item(row,1)->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        wdg_tblview->scrollTo(m_model->index(row,1), QAbstractItemView::EnsureVisible);
        wdg_tblview->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
        wdg_tblview->setCurrentIndex(m_model->index(row,1));
        wdg_tblview->openPersistentEditor(m_model->index(row,1));

        wdg_comtxt->clear();
        wdg_comtxt->setEnabled(true);
        wdg_comtxt->setFocusPolicy(Qt::WheelFocus);
        wdg_comtxt->setStyleSheet("border: 2px solid rgb(251, 51, 61);");

        CancelButton->setUpButtonStyle(UpSmallButton::BACKBUTTON);
        CancelButton->setEnabled(true);
        CancelButton->setImmediateToolTip(tr("Revenir au mode\nsélection de commentaire"));
        OKButton->setEnabled(false);
        OKButton->setImmediateToolTip(tr("Enregistrer\nle commentaire"));
    }
}

void dlg_listecommentaires::dblClicktextEdit()
{
    if (m_mode == Selection)
    {
        CommentLunet *com = Q_NULLPTR;
        if (wdg_tblview->selectionModel()->hasSelection())
            com = getCommentFromIndex(wdg_tblview->currentIndex());
        if (com)
            if (com->iduser() == currentuser()->id())
                ConfigMode(Modification,com);
    }
}

// --------------------------------------------------------------------------------------------------
// SetEnabled = false et disconnect toute sles lignes des UpTableWidget - SetEnabled = false checkBox
// --------------------------------------------------------------------------------------------------
void dlg_listecommentaires::DisableLines()
{
    for (int i=0; i<m_model->rowCount(); i++)
    {
        QStandardItem *itm = m_model->item(i,0);
        if (itm)
            itm->setFlags(Qt::ItemIsUserCheckable);
        QStandardItem *itm1 = m_model->item(i,1);
        if (itm1)
            itm1->setFlags(Qt::NoItemFlags);
        QStandardItem *itm2 = m_model->item(i,2);
        if (itm2)
            itm2->setFlags(Qt::NoItemFlags);
    }
}

void dlg_listecommentaires::Enablebuttons(QModelIndex idx)
{
    if (wdg_tblview->selectionModel()->hasSelection())
        m_currentcomment = getCommentFromIndex(idx);
    else
        m_currentcomment = Q_NULLPTR;
    wdg_buttonframe->wdg_modifBouton->setEnabled(m_currentcomment);
    if (m_currentcomment)
        wdg_buttonframe->wdg_moinsBouton->setEnabled(m_currentcomment->iduser() == currentuser()->id());
    else
        wdg_buttonframe->wdg_moinsBouton->setEnabled(false);
}

// -------------------------------------------------------------------------------------------
// SetEnabled = true, connect toutes les lignes des UpTableWidget - SetEnabled = true checkBox
// -------------------------------------------------------------------------------------------
void dlg_listecommentaires::EnableLines()
{
    for (int i=0; i<m_model->rowCount(); i++)
    {
        QStandardItem *itm = m_model->item(i,0);
        if (itm)
            itm->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled );
        QStandardItem *itm1 = m_model->item(i,1);
        if (itm1)
            itm1->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        QStandardItem *itm2 = m_model->item(i,2);
        if (itm2)
            itm2->setFlags(Qt::ItemIsEnabled);
    }
}

// ----------------------------------------------------------------------------------
// Modification du commentaire dans la base.
// ----------------------------------------------------------------------------------
void dlg_listecommentaires::EnregistreCommentaire(CommentLunet *com)
{
    int row = getRowFromComment(com);
    UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_model->item(row));
    if (!itm)
    {
        selectcurrentComment(com);
        return;
    }
    UpStandardItem *itmdef = dynamic_cast<UpStandardItem*>(m_model->item(row,2));
    if (!itmdef)
    {
        selectcurrentComment(com);
        return;
    }
    bool pardefaut = (itmdef->data(Qt::DecorationRole) != QPixmap());

    QString resume = itm->text();
    // recherche de l'enregistrement modifié
    // controle validate des champs
    if (ChercheDoublon(resume,row))
    {
        selectcurrentComment(com);
        return;
    }
    m_listbinds[CP_TEXT_COMLUN]     = wdg_comtxt->toPlainText();
    m_listbinds[CP_RESUME_COMLUN]   = m_model->item(row,1)->data(Qt::EditRole).toString();
    m_listbinds[CP_IDUSER_COMLUN]   = com->iduser();
    m_listbinds[CP_PARDEFAUT_COMLUN]= (pardefaut?  "1" : QVariant());
    m_listbinds[CP_PUBLIC_COMLUN]   = QVariant();
    if (m_mode == Creation)
    {
        delete  com;
        m_currentcomment = Datas::I()->commentslunets->CreationCommentLunet(m_listbinds);
    }
    else if (m_mode == Modification)
    {
        DataBase::I()->UpdateTable(TBL_COMMENTAIRESLUNETTES, m_listbinds, "where " CP_ID_COMLUN " = " + QString::number(com->id()));
        m_currentcomment = Datas::I()->commentslunets->getById(com->id(), true);
    }
    RemplirTableView();
    if (m_model->rowCount() > 0)
        selectcurrentComment(com);
}

// ------------------------------------------------------------------------------------------
// renvoie le commentaire correspondant à l'index
// ------------------------------------------------------------------------------------------
CommentLunet* dlg_listecommentaires::getCommentFromIndex(QModelIndex idx )
{
    int row = idx.row();
    UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_model->item(row));
    if (itm)
        return dynamic_cast<CommentLunet*>(itm->item());
    else
        return Q_NULLPTR;
}

// ------------------------------------------------------------------------------------------
// renvoie le row correspondant au commentaire
// ------------------------------------------------------------------------------------------
int dlg_listecommentaires::getRowFromComment(CommentLunet *com)
{
    int row = -1;
    if (!com)
        return row;
    for (int i=0; i<m_model->rowCount(); i++)
    {
        UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_model->item(i));
        if (itm)
        {
            CommentLunet *coms = dynamic_cast<CommentLunet*>(itm->item());
            if (coms == com)
            {
                row = i;
                i = m_model->rowCount();
            }
        }
    }
    return row;
}

void dlg_listecommentaires::MenuContextuel()
{
    QModelIndex idx   = wdg_tblview->indexAt(wdg_tblview->viewport()->mapFromGlobal(cursor().pos()));
    CommentLunet *com = getCommentFromIndex(idx);
    QMenu *menuContextuel = new QMenu(this);
    QAction *pAction_Modif;
    QAction *pAction_Suppr;
    QAction *pAction_Creer;
    QAction *pAction_ParDefaut;

    pAction_Creer                = menuContextuel->addAction(Icons::icCreer(), tr("Créer un commentaire"));
    connect (pAction_Creer,      &QAction::triggered,    [=] {ChoixMenuContextuel("Creer");});

    if (com && com->iduser() == currentuser()->id())
    {
        selectcurrentComment(com);
        pAction_Modif                = menuContextuel->addAction(Icons::icEditer(), tr("Modifier ce commentaire"));
        pAction_Suppr                = menuContextuel->addAction(Icons::icPoubelle(), tr("Supprimer ce commentaire"));
        menuContextuel->addSeparator();
        if (!com->isdefaut())
            pAction_ParDefaut                = menuContextuel->addAction(Icons::icBlackCheck(), tr("Par défaut"));
        else
            pAction_ParDefaut                = menuContextuel->addAction("Par défaut") ;
        pAction_ParDefaut->setToolTip(tr("si cette option est cochée\nle commentaire sera systématiquement imprimé"));
        connect (pAction_ParDefaut,          &QAction::triggered,    [=] {ChoixMenuContextuel("ParDefaut");});
        connect (pAction_Modif,      &QAction::triggered,    [=] {ChoixMenuContextuel("Modifier");});
        connect (pAction_Suppr,      &QAction::triggered,    [=] {ChoixMenuContextuel("Supprimer");});
    }
    // ouvrir le menu
    menuContextuel->exec(cursor().pos());
}

// ----------------------------------------------------------------------------------
// Remplissage de la tablewidgetavec les commentaires de la base.
// ----------------------------------------------------------------------------------
void dlg_listecommentaires::RemplirTableView()
{
    wdg_tblview->disconnect();
    wdg_tblview->selectionModel()->disconnect();
    if (m_model == Q_NULLPTR)
        delete m_model;
    m_model = new QStandardItemModel();
    QStandardItem *pitem0   = new QStandardItem(Icons::icImprimer(),"");
    pitem0->setEditable(false);
    pitem0->setTextAlignment(Qt::AlignCenter);
    m_model->setHorizontalHeaderItem(0,pitem0);
    QStandardItem *pitem1   = new QStandardItem(tr("TITRES DES COMMENTAIRES"));
    pitem0->setEditable(false);
    pitem0->setTextAlignment(Qt::AlignLeft);
    m_model->setHorizontalHeaderItem(1,pitem1);
    QStandardItem *pitem2   = new QStandardItem();
    pitem1->setEditable(false);
    m_model->setHorizontalHeaderItem(2,pitem2);
    m_model->setRowCount(Datas::I()->commentslunets->commentaires()->size());
    m_model->setColumnCount(3);
    for (int i=0; i<Datas::I()->commentslunets->commentaires()->size(); i++)
    {
        CommentLunet *com = Datas::I()->commentslunets->commentaires()->values().at(i);
        setCommentToRow(com, i);
    }
    if (m_model->rowCount()>0)
    {
        for (int i=0; i<Datas::I()->commentslunets->commentaires()->size(); i++)
        {
            CommentLunet *com = Datas::I()->commentslunets->commentaires()->values().at(i);
            setCommentToRow(com, i);
        }
        m_model->sort(1);
        QItemSelectionModel *m = wdg_tblview->selectionModel(); // il faut détruire le selectionModel pour éviter des bugs d'affichage quand on réinitialise le modèle
        wdg_tblview->setModel(m_model);
        delete m;
        UpLineDelegate *line = new UpLineDelegate();
        connect(line,   &UpLineDelegate::textEdited, this, [&] { OKButton->setEnabled(true);});
        wdg_tblview->setItemDelegateForColumn(1,line);
        wdg_tblview->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
        QHeaderView *verticalHeader = wdg_tblview->verticalHeader();
        verticalHeader->setSectionResizeMode(QHeaderView::Fixed);
        verticalHeader->setDefaultSectionSize(int(QFontMetrics(qApp->font()).height()*1.6));
        verticalHeader->setVisible(false);
        wdg_tblview->setColumnWidth(0,30);      // Check
        wdg_tblview->setColumnWidth(1,380);     // Resumé
        wdg_tblview->setColumnWidth(2,30);      // DefautIcon
        wdg_tblview->FixLargeurTotale();
        wdg_buttonframe->widgButtonParent()->setFixedWidth(wdg_tblview->width());
        ConfigMode(Selection);
        connect(wdg_tblview,     &QAbstractItemView::entered,               this,   [=] (QModelIndex idx) {
                                                                                                            CommentLunet *com = getCommentFromIndex(idx);
                                                                                                            if (com)
                                                                                                                QToolTip::showText(cursor().pos(),com->tooltip());
                                                                                                            } );
        connect(wdg_tblview,     &QAbstractItemView::doubleClicked,         this,   [=] (QModelIndex idx) {
                                                                                                            CommentLunet *com = getCommentFromIndex(idx);
                                                                                                            if (com)
                                                                                                            {
                                                                                                                m_currentcomment = com;
                                                                                                                if(com->iduser() == currentuser()->id() && m_mode == Selection)
                                                                                                                    ConfigMode(Modification,m_currentcomment);
                                                                                                            }
                                                                                                          });
        connect (wdg_tblview,    &QWidget::customContextMenuRequested,      this,   &dlg_listecommentaires::MenuContextuel);
        //! ++++ il faut utiliser selectionChanged et pas currentChanged qui n'est pas déclenché quand on clique sur un item alors la table n'a pas le focus et qu'elle n'a aucun item sélectionné
        connect (wdg_tblview->selectionModel(),
                                 &QItemSelectionModel::selectionChanged,    this,   [=] (QItemSelection select) {Enablebuttons(select.indexes().at(0));});
        connect(wdg_tblview,     &QAbstractItemView::clicked,               this,   [=] (QModelIndex idx) {// le bouton OK est enabled quand une case est cochée
                                                                                                            QStandardItem *itm = m_model->itemFromIndex(idx);
                                                                                                            if(itm->isCheckable() && itm->checkState() == Qt::Checked)
                                                                                                                OKButton->setEnabled(true);
                                                                                                            else
                                                                                                            {
                                                                                                                bool ok = false;
                                                                                                                for (int i=0; i<m_model->rowCount(); ++i)
                                                                                                                {
                                                                                                                    QStandardItem *itmc = m_model->item(i);
                                                                                                                    if (itmc)
                                                                                                                        if(itmc->checkState() == Qt::Checked)
                                                                                                                        {
                                                                                                                            ok = true;
                                                                                                                            i = m_model->rowCount();
                                                                                                                        }
                                                                                                                }
                                                                                                                OKButton->setEnabled(ok);
                                                                                                            }
                                                                                                           });
        m_currentcomment = Q_NULLPTR;
        wdg_tblview->selectionModel()->clearCurrentIndex();
    }
    else
        ConfigMode(Creation);
}

void dlg_listecommentaires::selectcurrentComment(CommentLunet *com)
{
    if (!com)
    {
        m_currentcomment = Q_NULLPTR;
        wdg_tblview->selectionModel()->clearSelection();
        Enablebuttons(QModelIndex());
    }
    else for (int i=0; i<m_model->rowCount(); i++)
    {
        UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_model->item(i));
        if (itm)
        {
            CommentLunet *coms = dynamic_cast<CommentLunet*>(itm->item());
            if (coms)
                if (com == coms)
                {
                    QModelIndex idx = m_model->index(i,1);
                    wdg_tblview->selectionModel()->setCurrentIndex(idx,QItemSelectionModel::Select);
                    wdg_tblview->scrollTo(idx, QAbstractItemView::PositionAtCenter);
                    OKButton->setEnabled(true);
                    Enablebuttons(idx);
                    break;
                }
        }
    }
}

void dlg_listecommentaires::setCommentToRow(CommentLunet *com, int row)
{
    if(!com)
        return;
    QFont disabledFont = qApp->font();
    disabledFont.setItalic(true);
    UpStandardItem *pitem0 = new UpStandardItem("", com);
    pitem0->setCheckable(true);
    if (com->isdefaut())
        pitem0->setCheckState(Qt::Checked);
    m_model->setItem(row,0,pitem0);
    QModelIndex index = m_model->index(row, 1, QModelIndex());
    m_model->setData(index, com->resume());
    if (com->iduser() != currentuser()->id())
        m_model->itemFromIndex(index)->setFont(disabledFont);
    UpStandardItem *pitem1 = new UpStandardItem("", com);
    if (com->isdefaut())
        pitem1->setData(Icons::pxBlackCheck().scaled(15,15),Qt::DecorationRole);
    else
        pitem1->setData(QPixmap(),Qt::DecorationRole);
    m_model->setItem(row,2, pitem1);
}

// ----------------------------------------------------------------------------------
// Supprime commentaire
// ----------------------------------------------------------------------------------
void dlg_listecommentaires::SupprimmCommentaire(CommentLunet* com)
{
    if (!com)
        return;

    QString Msg;
    Msg = tr("Etes vous sûr de vouloir supprimer le commentaire") + "\n" + com->resume() + "?";
    UpMessageBox msgbox;
    msgbox.setText("Euuhh... " + currentuser()->login() + "?");
    msgbox.setInformativeText(Msg);
    msgbox.setIcon(UpMessageBox::Warning);
    UpSmallButton OKBouton;
    OKBouton.setImmediateToolTip(tr("Supprimer le commentaire"));
    UpSmallButton NoBouton;
    NoBouton.setImmediateToolTip(tr("Annuler"));
    msgbox.addButton(&NoBouton,UpSmallButton::CANCELBUTTON);
    msgbox.addButton(&OKBouton, UpSmallButton::STARTBUTTON);
    msgbox.setDefaultButton(&NoBouton);
    msgbox.exec();
    if (msgbox.clickedButton()  == &OKBouton)
    {
        int row = getRowFromComment(com);
        if (row > -1 && row < m_model->rowCount())
        {
            m_model->takeRow(getRowFromComment(com));
            Datas::I()->commentslunets->SupprimeCommentLunet(com);
        }
    }
}

// ----------------------------------------------------------------------------------
// Clic sur le bouton OK.
// L'action depend de ce qu'on est en train de faire (creation modife, selection)
// ----------------------------------------------------------------------------------
void dlg_listecommentaires::Validation()
{
    if (m_mode == Modification || m_mode == Creation)
    {
        wdg_tblview->setEditTriggers(QAbstractItemView::NoEditTriggers);
        EnregistreCommentaire(m_currentcomment);
        ConfigMode(Selection, m_currentcomment);
    }
    else if (m_mode == Selection)
    {
        for (int i =0 ; i < m_model->rowCount(); i++)
        {
            UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_model->item(i));
            if (itm)
                if (itm->checkState() == Qt::Checked)
            {
                CommentLunet *com = dynamic_cast<CommentLunet*>(itm->item());
                if (com)
                {
                    m_commentaire       += com->texte() + "\n";
                    m_commentaireresume += " - " + com->resume();
                }
            }
        }
        if (m_commentaireresume != "")
            m_commentaireresume = "\n" + m_commentaireresume;
        accept();
    }
}

