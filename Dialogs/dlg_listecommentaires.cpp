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


dlg_listecommentaires::dlg_listecommentaires(QList<CommentLunet*> listecommentaires, QWidget *parent) :
    UpDialog(PATH_FILE_INI, "PositionsFiches/PositionCommentaires", parent)
{
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
    setWindowModality(Qt::WindowModal);
    wdg_tblview         = new UpTableView();
    wdg_comtxt          = new UpTextEdit();
    wdg_publicchkbox    = new UpCheckBox();
    wdg_buttonframe     = new WidgetButtonFrame(wdg_tblview);
    wdg_buttonframe     ->AddButtons(WidgetButtonFrame::Plus | WidgetButtonFrame::Modifier | WidgetButtonFrame::Moins);

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

    wdg_publicchkbox->setText(tr("Afficher les commentaires publics\ndes autres utilisateurs"));
    wdg_publicchkbox->setChecked(currentuser()->affichecommentslunettespublics());
    buttonslayout()->insertWidget(0, wdg_publicchkbox);

    connect (OKButton,          &QPushButton::clicked,      this,   &dlg_listecommentaires::Validation);
    connect (CancelButton,      &QPushButton::clicked,      this,   &dlg_listecommentaires::Annulation);
    connect (wdg_comtxt,        &QTextEdit::textChanged,    this,   [&] { OKButton->setEnabled(true);});
    connect (wdg_comtxt,        &UpTextEdit::dblclick,      this,   &dlg_listecommentaires::dblClicktextEdit);
    connect (wdg_buttonframe,   &WidgetButtonFrame::choix,  this,   &dlg_listecommentaires::ChoixButtonFrame);
    connect (wdg_publicchkbox,  &QCheckBox::clicked,        this,   [&](bool a) {AfficheCommentsPublics(a);});

    m_listcommentaires = listecommentaires;
    RemplirTableView();
    if (m_model->rowCount()>0)
    {
        CommentLunet *com = getCommentFromIndex(m_model->index(0,0));
        selectcurrentComment(com);
    }
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
// Affiche ou non les commentaires publics des autres utilisateurs
// ----------------------------------------------------------------------------------
void dlg_listecommentaires::AfficheCommentsPublics(bool a)
{
    ItemsList::update(currentuser(), CP_AFFICHECOMMENTSPUBLICS_USR,a);
    for (int j=0; j<m_model->rowCount(); j++)
    {
        CommentLunet *com = getCommentFromIndex(m_model->index(j,0));
        if (com)
            if (com->iduser() != currentuser()->id())
                wdg_tblview->setRowHidden(m_model->getRowFromItem(com),!a);
    }
}

// ----------------------------------------------------------------------------------
// Clic sur le bouton ANNULER.
// L'action depend de ce qu'on est en train de faire (creation modife, selection)
// ----------------------------------------------------------------------------------
void dlg_listecommentaires::Annulation()
{
    wdg_tblview->setEditTriggers(QAbstractItemView::NoEditTriggers);
    if (m_mode == Creation || m_mode == Modification)
    {
        int row = m_model->getRowFromItem(m_currentcomment);
        if (m_mode == Modification && row < m_model->rowCount())
        {
            QModelIndex idx = m_model->index(row,1);
            wdg_tblview->closePersistentEditor(idx);
            m_model->setData(idx, m_currentcomment->resume());
            EnableButtons(m_currentcomment);
        }
        else if (m_mode == Creation)
        {
            m_model->removeRow(row);
            if(m_model->rowCount() > 0 && row < m_model->rowCount())
                selectcurrentComment(getCommentFromIndex(m_model->index(row,1)));
        }
        else
            RemplirTableView();
        ConfigMode(Selection);
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
                CommentLunet *com = qobject_cast<CommentLunet*>(itm->item());
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
        int row = m_model->getRowFromItem(m_currentcomment);
        if (row== -1)
            return;
        UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_model->item(row,2));
        if (itm)
        {
            if(itm->data(Qt::DecorationRole) == QPixmap())
                itm->setData(Icons::pxBlackCheck().scaled(15,15),Qt::DecorationRole);
            else
                itm->setData(QPixmap(),Qt::DecorationRole);
        }
        if (m_mode == Selection)
            ItemsList::update(m_currentcomment, CP_PARDEFAUT_COMLUN,!m_currentcomment->isdefaut());
    }
    else if (choix  == "Public")
    {
        int row = m_model->getRowFromItem(m_currentcomment);
        if (row== -1)
            return;
        UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_model->item(row,3));
        if (itm)
        {
            if(itm->data(Qt::DecorationRole) == QPixmap())
                itm->setData(Icons::pxBlackCheck().scaled(15,15),Qt::DecorationRole);
            else
                itm->setData(QPixmap(),Qt::DecorationRole);
        }
        if (m_mode == Selection)
            ItemsList::update(m_currentcomment, CP_PUBLIC_COMLUN,!m_currentcomment->ispublic());
    }
    else if (choix  == "Creer")
    {
        ConfigMode(Creation,m_currentcomment);
    }
    else if (choix  == "Recopier")
    {
        // Creation du Document dans la table
        m_listbinds[CP_TEXT_COMLUN]     = m_currentcomment->texte();
        m_listbinds[CP_RESUME_COMLUN]   = m_currentcomment->resume();
        m_listbinds[CP_IDUSER_COMLUN]   = currentuser()->id();
        m_listbinds[CP_PARDEFAUT_COMLUN]= (m_currentcomment->isdefaut()?  "1" : QVariant());
        m_listbinds[CP_PUBLIC_COMLUN]   = QVariant();
        m_currentcomment = Q_NULLPTR;
        m_currentcomment = Datas::I()->commentslunets->CreationCommentLunet(m_listbinds);
        RemplirTableView();
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
            selectcurrentComment(com, QAbstractItemView::PositionAtCenter);

        int nbCheck = 0;
        for (int i =0 ; i < m_model->rowCount(); i++)
        {
            UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_model->item(i));
            if (itm)
                if (itm->checkState() == Qt::Checked)
                    nbCheck ++;
        }
        OKButton->setEnabled(true);
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
                CommentLunet *coms = qobject_cast<CommentLunet*>(itm->item());
                if (coms)
                    if (coms == com)
                    {
                        m_model->item(i,1)->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
                        wdg_tblview->setFocus();
                        wdg_tblview->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
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
            row = m_model->getRowFromItem(com);
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
        if (m_currentcomment)
            if (m_currentcomment->iduser() == currentuser()->id())
                ConfigMode(Modification,m_currentcomment);
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
    }
}

void dlg_listecommentaires::EnableButtons(CommentLunet* com)
{
    wdg_buttonframe->wdg_modifBouton->setEnabled(com);
    if (com)
        wdg_buttonframe->wdg_moinsBouton->setEnabled(com->iduser() == currentuser()->id());
    else
        wdg_buttonframe->wdg_moinsBouton->setEnabled(false);
}

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
    }
}

// ----------------------------------------------------------------------------------
// Modification du commentaire dans la base.
// ----------------------------------------------------------------------------------
bool dlg_listecommentaires::EnregistreCommentaire(CommentLunet *com)
{
    int row = m_model->getRowFromItem(com);
    UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_model->item(row));
    if (!itm)
        return false;
    UpStandardItem *itmdef = dynamic_cast<UpStandardItem*>(m_model->item(row,2));
    if (!itmdef)
        return false;
    wdg_tblview->closePersistentEditor(m_model->index(row,1));
    qApp->focusWidget()->clearFocus();   //permet de déclencher le focusout du delegate qui va lancer le signal commiData

    QString resume = Utils::trimcapitilize(m_textdelegate).left(50);
    // recherche de l'enregistrement modifié
    // controle validate des champs
    if (ChercheDoublon(resume,row))
    {
        wdg_tblview->openPersistentEditor(m_model->index(row,1));
        return false;
    }
    bool pardefaut = (itmdef->data(Qt::DecorationRole) != QPixmap());
    m_listbinds[CP_TEXT_COMLUN]     = wdg_comtxt->toPlainText();
    m_listbinds[CP_RESUME_COMLUN]   = resume;
    m_listbinds[CP_IDUSER_COMLUN]   = com->iduser();
    m_listbinds[CP_PARDEFAUT_COMLUN]= (pardefaut?  "1" : QVariant());
    m_listbinds[CP_PUBLIC_COMLUN]   = QVariant();
    int idcom = com->id();
    if (m_mode == Creation)
    {
        delete  com;
        m_currentcomment = Datas::I()->commentslunets->CreationCommentLunet(m_listbinds);
        setCommentToRow(m_currentcomment, row);
    }
    else if (m_mode == Modification)
    {
        DataBase::I()->UpdateTable(TBL_COMMENTAIRESLUNETTES, m_listbinds, "where " CP_ID_COMLUN " = " + QString::number(idcom));
        m_currentcomment = Datas::I()->commentslunets->getById(idcom, true);
    }
    if (m_currentcomment)
    {
        m_model->sort(1);
        selectcurrentComment(m_currentcomment);
    }
    return true;
}

// ------------------------------------------------------------------------------------------
// renvoie le commentaire correspondant à l'index
// ------------------------------------------------------------------------------------------
CommentLunet* dlg_listecommentaires::getCommentFromIndex(QModelIndex idx)
{
    int row = idx.row();
    UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_model->item(row));
    if (itm)
        return qobject_cast<CommentLunet*>(itm->item());
    else
        return Q_NULLPTR;
}

void dlg_listecommentaires::MenuContextuel()
{
    QMenu *menuContextuel = new QMenu(this);
    QAction *pAction_Modif;
    QAction *pAction_Suppr;
    QAction *pAction_Creer;
    QAction *pAction_ParDefaut;
    QAction *pAction_Public;
    QAction *pAction_Recopier;

    pAction_Creer                = menuContextuel->addAction(Icons::icCreer(), tr("Créer un commentaire"));
    connect (pAction_Creer,      &QAction::triggered, this,   [=] {ChoixMenuContextuel("Creer");});

    QModelIndex idx   = wdg_tblview->indexAt(wdg_tblview->viewport()->mapFromGlobal(cursor().pos()));
    m_currentcomment = getCommentFromIndex(idx);
    if (m_currentcomment)
    {
        if (m_currentcomment->iduser() == currentuser()->id())
        {
            pAction_Modif                = menuContextuel->addAction(Icons::icEditer(), tr("Modifier ce commentaire"));
            pAction_Suppr                = menuContextuel->addAction(Icons::icPoubelle(), tr("Supprimer ce commentaire"));
            menuContextuel->addSeparator();
            if (m_currentcomment->isdefaut())
                pAction_ParDefaut               = menuContextuel->addAction(Icons::icBlackCheck(), tr("Par défaut"));
            else
                pAction_ParDefaut               = menuContextuel->addAction("Par défaut") ;
            if (m_currentcomment->ispublic() && m_currentcomment->iduser() == currentuser()->id())
                pAction_Public                  = menuContextuel->addAction(Icons::icBlackCheck(), tr("Public"));
            else
                pAction_Public                  = menuContextuel->addAction("Public") ;
            pAction_ParDefaut->setToolTip(tr("si cette option est cochée\nle commentaire sera systématiquement imprimé"));
            connect (pAction_ParDefaut,  &QAction::triggered, this,   [=] {ChoixMenuContextuel("ParDefaut");});
            connect (pAction_Public,     &QAction::triggered, this,   [=] {ChoixMenuContextuel("Public");});
            connect (pAction_Modif,      &QAction::triggered, this,   [=] {ChoixMenuContextuel("Modifier");});
            connect (pAction_Suppr,      &QAction::triggered, this,   [=] {ChoixMenuContextuel("Supprimer");});
        }
        else
        {
            pAction_Recopier             = menuContextuel->addAction(Icons::icCopy(), tr("Recopier ce commentaire"));
            connect (pAction_Recopier,   &QAction::triggered, this,   [=] {ChoixMenuContextuel("Recopier");});
        }
    }
    // ouvrir le menu
    menuContextuel->exec(cursor().pos());
    delete menuContextuel;
}

// ----------------------------------------------------------------------------------
// Remplissage de la tablewidgetavec les commentaires de la base.
// ----------------------------------------------------------------------------------
void dlg_listecommentaires::RemplirTableView()
{
    wdg_tblview->disconnect();
    wdg_tblview->selectionModel()->disconnect();
    if (m_model != Q_NULLPTR)
        delete m_model;
    m_model = new UpStandardItemModel();
    UpLineDelegate *line = new UpLineDelegate();
    connect(line,   &UpLineDelegate::textEdited, this, [=] { OKButton->setEnabled(true);});
    connect(line,   &UpLineDelegate::commitData, this, [=]  (QWidget *editor)
    {
        UpLineEdit *line = qobject_cast<UpLineEdit*>(editor);
        if (line)
            m_textdelegate = line->text();
    });
    wdg_tblview->setItemDelegateForColumn(1,line);
    QStandardItem *pitem0   = new QStandardItem(Icons::icImprimer(),"");
    pitem0->setEditable(false);
    pitem0->setTextAlignment(Qt::AlignCenter);
    m_model->setHorizontalHeaderItem(0,pitem0);
    QStandardItem *pitem1   = new QStandardItem(tr("TITRES DES COMMENTAIRES"));
    pitem1->setEditable(false);
    pitem1->setTextAlignment(Qt::AlignLeft);
    m_model->setHorizontalHeaderItem(1,pitem1);
    QStandardItem *pitem2   = new QStandardItem(Icons::pxBlackCheck().scaled(15,15),"");    // par défaut
    pitem2->setEditable(false);
    m_model->setHorizontalHeaderItem(2,pitem2);
    QStandardItem *pitem3   = new QStandardItem(Icons::icFamily(),"");    // public
    pitem3->setEditable(false);
    m_model->setHorizontalHeaderItem(3,pitem3);
    m_model->setRowCount(Datas::I()->commentslunets->commentaires()->size());
    m_model->setColumnCount(4);
    for (int i=0; i<Datas::I()->commentslunets->commentaires()->size(); i++)
    {
        CommentLunet *com = Datas::I()->commentslunets->commentaires()->values().at(i);
        setCommentToRow(com, i, false);
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
        AfficheCommentsPublics(currentuser()->affichecommentslunettespublics());
        connect(wdg_tblview,     &QAbstractItemView::entered,               this,   [=] (QModelIndex idx) {
                                                                                                            CommentLunet *com = getCommentFromIndex(idx);
                                                                                                            if (com)
                                                                                                                QToolTip::showText(cursor().pos(),com->tooltip());
                                                                                                            } );
        connect(wdg_tblview,     &QAbstractItemView::doubleClicked,         this,   [&] (QModelIndex idx) {
                                                                                                            CommentLunet *com = getCommentFromIndex(idx);
                                                                                                            if (com)
                                                                                                            {
                                                                                                                m_currentcomment = com;
                                                                                                                if(com->iduser() == currentuser()->id() && m_mode == Selection)
                                                                                                                    ConfigMode(Modification,m_currentcomment);
                                                                                                            }
                                                                                                          });
        connect (wdg_tblview,    &QWidget::customContextMenuRequested,      this,   &dlg_listecommentaires::MenuContextuel);
        connect (wdg_tblview->selectionModel(),                   &QItemSelectionModel::currentRowChanged,          this,   [&] (QModelIndex idx) {
                                                                                                                    m_currentcomment = getCommentFromIndex(idx);
                                                                                                                    if (m_currentcomment)
                                                                                                                    {
                                                                                                                        OKButton->setEnabled(true);
                                                                                                                        EnableButtons(m_currentcomment);
                                                                                                                    }
                                                                                                                });
        m_currentcomment = Q_NULLPTR;
    }
    else
        ConfigMode(Creation);
}

void dlg_listecommentaires::selectcurrentComment(CommentLunet *com, QAbstractItemView::ScrollHint hint)
{
    if (wdg_tblview->selectionModel())
        if (wdg_tblview->selectionModel()->selectedIndexes().size() >0)
            wdg_tblview->selectionModel()->clear();
    m_currentcomment = com;
    if (!m_currentcomment)
        EnableButtons();
    else for (int i=0; i<m_model->rowCount(); i++)
    {
        UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_model->item(i));
        if (itm)
        {
            CommentLunet *coms = qobject_cast<CommentLunet*>(itm->item());
            if (coms)
                if (m_currentcomment == coms)
                {
                    QModelIndex idx = m_model->index(i,1);
                    wdg_tblview->selectionModel()->select(idx,QItemSelectionModel::SelectCurrent);
                    wdg_tblview->scrollTo(idx, hint);
                    OKButton->setEnabled(true);
                    EnableButtons(m_currentcomment);
                    break;
                }
        }
    }
}

void dlg_listecommentaires::setCommentToRow(CommentLunet *com, int row, bool resizecolumn)
{
    if(!com)
        return;
    if (row < 0 || row > m_model->rowCount()-1)
        return;
    QFont disabledFont = qApp->font();
    disabledFont.setItalic(true);

    UpStandardItem *pitem0 = new UpStandardItem("", com);
    pitem0->setCheckable(true);
    if (m_listcommentaires.contains(com))
        pitem0->setCheckState(Qt::Checked);
    m_model->setItem(row,0,pitem0);
    QModelIndex index = m_model->index(row, 1, QModelIndex());
    m_model->setData(index, com->resume());
    if (com->iduser() != currentuser()->id())
        {
            m_model->itemFromIndex(index)->setFont(disabledFont);
            m_model->itemFromIndex(index)->setForeground(QBrush(QColor(0,0,140)));
        }
    UpStandardItem *pitem1 = new UpStandardItem("", com);
    if (com->isdefaut() && com->iduser() == currentuser()->id())                   // par défaut
        pitem1->setData(Icons::pxBlackCheck().scaled(15,15),Qt::DecorationRole);
    else
        pitem1->setData(QPixmap(),Qt::DecorationRole);
    pitem1->setFlags(Qt::NoItemFlags);
    m_model->setItem(row,2, pitem1);
    UpStandardItem *pitem2 = new UpStandardItem("", com);
    if (com->ispublic())                                                            // public
        pitem2->setData(Icons::pxBlackCheck().scaled(15,15),Qt::DecorationRole);
    else
        pitem2->setData(QPixmap(),Qt::DecorationRole);
    pitem2->setFlags(Qt::NoItemFlags);
    m_model->setItem(row,3, pitem2);
    if(!resizecolumn)
        return;

    //! la suite est obligatoire pour contourner un bug d'affichage sous MacOS
    wdg_tblview->setColumnWidth(0,30);      // Check
    wdg_tblview->setColumnWidth(1,380);     // Resumé
    wdg_tblview->setColumnWidth(2,30);      // DefautIcon
    wdg_tblview->setColumnWidth(3,30);      // Public
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
    UpMessageBox msgbox(this);
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
        int row = m_model->getRowFromItem(com);
        if (row > -1 && row < m_model->rowCount())
        {
            m_model->removeRow(row);
            Datas::I()->commentslunets->SupprimeCommentLunet(com);
        }
        if (m_model->rowCount() == 0)
            ConfigMode(Creation);
        else
        {
            ConfigMode(Selection);
            if (row >= m_model->rowCount())
                row = m_model->rowCount()-1;
            com = getCommentFromIndex(m_model->index(row,0));
            if (com)
                selectcurrentComment(com);
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
        if (EnregistreCommentaire(m_currentcomment))
        {
            ConfigMode(Selection, m_currentcomment);
            selectcurrentComment(m_currentcomment);
        }
    }
    else if (m_mode == Selection)
    {
        m_listcommentaires.clear();
        for (int i =0 ; i < m_model->rowCount(); i++)
        {
            UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_model->item(i));
            if (itm)
                if (itm->checkState() == Qt::Checked)
            {
                CommentLunet *com = qobject_cast<CommentLunet*>(itm->item());
                if (com)
                {
                    m_listcommentaires << com;
                }
            }
        }
        accept();
    }
}

