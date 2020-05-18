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

#include "dlg_listemotscles.h"

dlg_listemotscles::dlg_listemotscles(QWidget *parent) :
    UpDialog(PATH_FILE_INI, "PositionsFiches/PositionMotsCles", parent)
{
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::WindowMinMaxButtonsHint);

    wdg_tblview = new UpTableView();
    wdg_buttonframe = new WidgetButtonFrame(wdg_tblview);
    wdg_buttonframe->AddButtons(WidgetButtonFrame::Plus | WidgetButtonFrame::Modifier | WidgetButtonFrame::Moins);
    wdg_buttonframe->addSearchLine();

    dlglayout()->insertWidget(0,wdg_buttonframe->widgButtonParent());
    AjouteLayButtons(UpDialog::ButtonCancel|UpDialog::ButtonOK);
    CancelButton    ->disconnect();

    setModal(true);
    setSizeGripEnabled(false);
    setWindowTitle(tr("Liste des mots-clés"));

    // Mise en forme de la table
    wdg_tblview->setPalette(QPalette(Qt::white));
    wdg_tblview->setGridStyle(Qt::NoPen);
    wdg_tblview->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    wdg_tblview->setFocusPolicy(Qt::StrongFocus);
    wdg_tblview->setEditTriggers(QAbstractItemView::NoEditTriggers);
    wdg_tblview->setSelectionMode(QAbstractItemView::SingleSelection);
    QFontMetrics fm(qApp->font());
    int hauteurligne = int(fm.height()*1.3);
    wdg_tblview->setMinimumHeight(hauteurligne*20);
    wdg_tblview->setSizeIncrement(0,hauteurligne);
    wdg_tblview->setContextMenuPolicy(Qt::CustomContextMenu);
    wdg_tblview->setMouseTracking(true);

    connect (OKButton,          &QPushButton::clicked,      this,   &dlg_listemotscles::Validation);
    connect (CancelButton,      &QPushButton::clicked,      this,   &dlg_listemotscles::Annulation);
    connect (wdg_buttonframe,   &WidgetButtonFrame::choix,  this,   &dlg_listemotscles::ChoixButtonFrame);
    connect(wdg_buttonframe->searchline(),  &QLineEdit::textEdited,     this,   [=] (QString txt) { txt = Utils::trimcapitilize(txt, false, true);
                                                                                                    wdg_buttonframe->searchline()->setText(txt);
                                                                                                    for (auto it = Datas::I()->motscles->motscles()->constBegin(); it != Datas::I()->motscles->motscles()->constEnd(); ++it)
                                                                                                    {
                                                                                                        MotCle *mc = const_cast<MotCle*>(it.value());
                                                                                                        if (mc->motcle().startsWith(txt))
                                                                                                        {
                                                                                                            selectcurrentMotCle(mc, QAbstractItemView::PositionAtCenter);
                                                                                                            break;
                                                                                                        }
                                                                                                    }
                                                                                                    });

    RemplirTableView();
    wdg_buttonframe->searchline()->setCompleter(Datas::I()->motscles->completer());
    wdg_buttonframe->wdg_modifBouton->setEnabled(false);
    wdg_buttonframe->wdg_moinsBouton->setEnabled(false);
    wdg_buttonframe->searchline()->setFocus();
}

dlg_listemotscles::~dlg_listemotscles()
{
    EnregistrePosition();
}

void dlg_listemotscles::keyPressEvent(QKeyEvent * event )
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
void dlg_listemotscles::Annulation()
{
    int row = getRowFromMotCle(m_currentmotcle);
    wdg_tblview->setEditTriggers(QAbstractItemView::NoEditTriggers);
    if (m_mode == Creation || m_mode == Modification)
    {
        if (m_mode == Modification && row < m_model->rowCount())
        {
            QModelIndex idx = m_model->index(row,1);
            wdg_tblview->closePersistentEditor(idx);
            m_model->setData(idx, m_currentmotcle->motcle());
            EnableButtons(m_currentmotcle);
        }
        else if (m_mode == Creation && m_currentmotcle)
        {
            m_model->removeRow(row);
            delete m_currentmotcle;
            if(m_model->rowCount() > 0 && row < m_model->rowCount())
                selectcurrentMotCle(getMotCleFromIndex(m_model->index(row,1)));
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
bool dlg_listemotscles::ChercheDoublon(QString str, int row)
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
                MotCle *com = dynamic_cast<MotCle*>(itm->item());
                if (com)
                {
                    if (com->motcle().toUpper() == str.toUpper() && i != row)
                    {
                        a = true;
                        UpMessageBox::Watch(this, tr("Il existe déjà un mot-clé portant ce nom"));
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


void dlg_listemotscles::ChoixButtonFrame()
{
    switch (wdg_buttonframe->Choix()) {
    case WidgetButtonFrame::Plus:
        ConfigMode(Creation, m_currentmotcle);
        break;
    case WidgetButtonFrame::Modifier:
        ConfigMode(Modification,m_currentmotcle);
        break;
    case WidgetButtonFrame::Moins:
        SupprimeMotCle(m_currentmotcle);
        break;
    }
}

void dlg_listemotscles::ChoixMenuContextuel(QString choix)
{
    if (choix  == "Modifier")
        ConfigMode(Modification,m_currentmotcle);
    else if (choix  == "Supprimer")
        SupprimeMotCle(m_currentmotcle);
    else if (choix  == "Creer")
        ConfigMode(Creation, m_currentmotcle);
}

// ----------------------------------------------------------------------------------
// Configuration de la fiche en fonction du mode de fonctionnement
// ----------------------------------------------------------------------------------
void dlg_listemotscles::ConfigMode(Mode mode, MotCle *mc)
{
    m_mode = mode;
    wdg_buttonframe->setEnabled(m_mode == Selection);

    if (mode == Selection)
    {
        EnableLines();
        wdg_tblview->setFocus();
        wdg_tblview->setStyleSheet("");
        wdg_buttonframe->wdg_modifBouton->setEnabled(false);
        wdg_buttonframe->wdg_moinsBouton->setEnabled(false);

        CancelButton->setUpButtonStyle(UpSmallButton::CANCELBUTTON);
        CancelButton->setImmediateToolTip(tr("Annuler et fermer la fiche"));
        OKButton->setImmediateToolTip(tr("Imprimer\nla sélection"));
        if (mc)
            selectcurrentMotCle(mc, QAbstractItemView::PositionAtCenter);

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
        if (!mc)
            return;
        DisableLines();
        for (int i=0; i<m_model->rowCount(); ++i)
        {
            UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_model->item(i,0));
            if (itm)
            {
                MotCle *mcs = dynamic_cast<MotCle*>(itm->item());
                if (mcs)
                    if (mcs == mc)
                    {
                        m_model->item(i,1)->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
                        wdg_tblview->setFocus();
                        wdg_tblview->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
                        wdg_tblview->setCurrentIndex(m_model->index(i,1));
                        wdg_tblview->openPersistentEditor(m_model->index(i,1));
                        i = m_model->rowCount();
                    }
            }
        }
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
        if (mc)
            row = getRowFromMotCle(mc);
        m_model->insertRow(row);
        m_currentmotcle = new MotCle();
        m_currentmotcle->setmotcle(tr("Nouveau mot-clé"));
        setMotCleToRow(m_currentmotcle, row);
        m_model->item(row,1)->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        wdg_tblview->scrollTo(m_model->index(row,1), QAbstractItemView::EnsureVisible);
        wdg_tblview->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
        wdg_tblview->openPersistentEditor(m_model->index(row,1));

        CancelButton->setUpButtonStyle(UpSmallButton::BACKBUTTON);
        CancelButton->setEnabled(true);
        CancelButton->setImmediateToolTip(tr("Revenir au mode\nsélection de commentaire"));
        OKButton->setEnabled(false);
        OKButton->setImmediateToolTip(tr("Enregistrer\nle commentaire"));
    }
}

void dlg_listemotscles::DisableLines()
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

void dlg_listemotscles::EnableButtons(MotCle *mc)
{
    wdg_buttonframe->wdg_modifBouton->setEnabled(true);
    wdg_buttonframe->wdg_modifBouton->setEnabled(mc);
    bool isused = false;
    if (mc && m_currentpatient)
        isused = Datas::I()->motscles->isThisMCusedForOtherPatients(mc, m_currentpatient->id());
    wdg_buttonframe->wdg_moinsBouton->setEnabled(!isused);
}

void dlg_listemotscles::EnableLines(int row)
{
    auto enableline = [=] (int i)
    {
        QStandardItem *itm = m_model->item(i,0);
        if (itm)
            itm->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled );
        QStandardItem *itm1 = m_model->item(i,1);
        if (itm1)
            itm1->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    };
    if (row == -1)
        for (int i=0; i<m_model->rowCount(); i++)
            enableline(i);
    else
        enableline(row);
}

// ----------------------------------------------------------------------------------
// Modification du commentaire dans la base.
// ----------------------------------------------------------------------------------
void dlg_listemotscles::EnregistreMotCle(MotCle *mc)
{
    int row = getRowFromMotCle(mc);
    wdg_tblview->closePersistentEditor(m_model->index(row,1));
    UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_model->item(row));
    if (!itm)
    {
        selectcurrentMotCle(mc);
        return;
    }

    QString motcle = m_model->item(row,1)->data(Qt::DisplayRole).toString().left(80);
    // recherche de l'enregistrement modifié
    // controle validate des champs
    if (ChercheDoublon(motcle,row))
    {
        selectcurrentMotCle(mc);
        return;
    }
    m_listbinds[CP_TEXT_MOTCLE]     = motcle;
    int idmc = mc->id();
    if (m_mode == Creation)
    {
        delete mc;
        m_currentmotcle = Datas::I()->motscles->CreationMotCle(m_listbinds);
        setMotCleToRow(m_currentmotcle, row);
    }
    else if (m_mode == Modification)
    {
        DataBase::I()->UpdateTable(TBL_MOTSCLES, m_listbinds, "where " CP_ID_MOTCLE " = " + QString::number(mc->id()));
        m_currentmotcle = Datas::I()->motscles->getById(idmc, true);
    }
    m_model->sort(1);
    if (m_model->rowCount() > 0)
        selectcurrentMotCle(m_currentmotcle);
}

MotCle* dlg_listemotscles::getMotCleFromIndex(QModelIndex idx)
{
    int row = idx.row();
    UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_model->item(row));
    if (itm)
        return dynamic_cast<MotCle*>(itm->item());
    else
        return Q_NULLPTR;
}

int dlg_listemotscles::getRowFromMotCle(MotCle *mc)
{
    int row = -1;
    if (!mc)
        return row;
    for (int i=0; i<m_model->rowCount(); i++)
    {
        UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_model->item(i));
        if(itm)
        {
            MotCle* mcs = dynamic_cast<MotCle*>(itm->item());
            if (mc->id() == mcs->id())
            {
                row = i;
                break;
            }
        }
    }
    return row;
}

QList<int> dlg_listemotscles::listMCDepart() const
{
    return m_listidmotsclesdepart;
}

void dlg_listemotscles::MenuContextuel()
{
    QMenu *menuContextuel = new QMenu(this);
    QAction *pAction_Modif;
    QAction *pAction_Suppr;
    QAction *pAction_Creer;
    pAction_Creer                = menuContextuel->addAction(Icons::icCreer(), tr("Créer un mot-clé"));
    connect (pAction_Creer,      &QAction::triggered,    [=] {ChoixMenuContextuel("Creer");});

    QModelIndex idx   = wdg_tblview->indexAt(wdg_tblview->viewport()->mapFromGlobal(cursor().pos()));
    m_currentmotcle = getMotCleFromIndex(idx);

    if (m_currentmotcle)
    {
        pAction_Modif                = menuContextuel->addAction(Icons::icEditer(), tr("Modifier ce mot-clé"));
        pAction_Suppr                = menuContextuel->addAction(Icons::icPoubelle(), tr("Supprimer ce mot-clé"));

        connect (pAction_Modif,      &QAction::triggered,    [=] {ChoixMenuContextuel("Modifier");});
        connect (pAction_Suppr,      &QAction::triggered,    [=] {ChoixMenuContextuel("Supprimer");});
    }
    // ouvrir le menu
    menuContextuel->exec(cursor().pos());
}

void dlg_listemotscles::RemplirTableView()
{
    wdg_tblview->disconnect();
    wdg_tblview->selectionModel()->disconnect();
    UpLineDelegate *line = new UpLineDelegate();
    connect(line,   &UpLineDelegate::textEdited, [=] {OKButton->setEnabled(true);});
    wdg_tblview->setItemDelegateForColumn(1,line);
    QList<int> listidMC = DataBase::I()->loadListIdMotsClesByPat(m_currentpatient->id());
    m_listidmotsclesdepart.clear();
    for (int i=0; i<listidMC.size(); i++)
        m_listidmotsclesdepart << listidMC.at(i);
    if (m_model == Q_NULLPTR)
        delete m_model;
    m_model = new QStandardItemModel(this);

    QStandardItem *pitem0   = new QStandardItem(Icons::icCheck(),"");
    pitem0->setEditable(false);
    pitem0->setTextAlignment(Qt::AlignCenter);
    m_model->setHorizontalHeaderItem(0,pitem0);
    QStandardItem *pitem1   = new QStandardItem(tr("TITRES DES COMMENTAIRES"));
    pitem0->setEditable(false);
    pitem0->setTextAlignment(Qt::AlignLeft);
    m_model->setHorizontalHeaderItem(1,pitem1);
    m_model->setRowCount(Datas::I()->commentslunets->commentaires()->size());
    m_model->setColumnCount(2);
    for (int i=0; i<Datas::I()->motscles->motscles()->size(); i++)
    {
        MotCle *mc = Datas::I()->motscles->motscles()->values().at(i);
        setMotCleToRow(mc,i);
    }
    if (m_model->rowCount()>0)
    {
        m_model->sort(1);
        QItemSelectionModel *m = wdg_tblview->selectionModel(); // il faut détruire le selectionModel pour éviter des bugs d'affichage quand on réinitialise le modèle
        wdg_tblview->setModel(m_model);
        delete m;

        int hauteurligne = int(QFontMetrics(qApp->font()).height()*1.3);
        wdg_tblview->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
        QHeaderView *verticalHeader = wdg_tblview->verticalHeader();
        verticalHeader->setSectionResizeMode(QHeaderView::Fixed);
        verticalHeader->setDefaultSectionSize(hauteurligne);
        verticalHeader->setVisible(false);
        wdg_tblview->horizontalHeader()->setFixedHeight(hauteurligne);
        wdg_tblview->setColumnWidth(0,30);
        wdg_tblview->setColumnWidth(1,270);
        wdg_tblview->FixLargeurTotale();
        wdg_buttonframe->widgButtonParent()->setFixedWidth(wdg_tblview->width());
        ConfigMode(Selection);
        connect (wdg_tblview,                   &QWidget::customContextMenuRequested,   this,   &dlg_listemotscles::MenuContextuel);
        //! ++++ il faut utiliser selectionChanged et pas currentChanged qui n'est pas déclenché quand on clique sur un item alors la tabnle n'a pas le focus et qu'elle n'a aucun item sélectionné
        connect (wdg_tblview->selectionModel(),                   &QItemSelectionModel::currentRowChanged,          this,   [&] (QModelIndex idx) {
                                                                                                                        m_currentmotcle = getMotCleFromIndex(idx);
                                                                                                                        if (m_currentmotcle)
                                                                                                                        {
                                                                                                                            OKButton->setEnabled(true);
                                                                                                                            EnableButtons(m_currentmotcle);
                                                                                                                        }
                                                                                                                        });
        connect (wdg_tblview,                   &QAbstractItemView::clicked,            this,   [=] (QModelIndex idx) {// le bouton OK est enabled quand une case est cochée
            QStandardItem *itm = m_model->itemFromIndex(idx);
            if (itm)
            {
                if(itm->isCheckable())
                {
                    if (itm->checkState() == Qt::Checked)
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
                }
            }
        });
        connect(wdg_tblview,                    &QAbstractItemView::doubleClicked,      this,   [=] (QModelIndex idx) {
            MotCle *mc = getMotCleFromIndex(idx);
            if (mc)
            {
                m_currentmotcle = mc;
                if (m_mode == Selection)
                    ConfigMode(Modification, m_currentmotcle);
            }
        });
        wdg_tblview->selectionModel()->clearSelection();
    }
    else
        ConfigMode(Creation);
}

void dlg_listemotscles::selectcurrentMotCle(MotCle *mc, QAbstractItemView::ScrollHint hint)
{
    m_currentmotcle = mc;
    if (!m_currentmotcle)
    {
        wdg_tblview->selectionModel()->clearSelection();
        EnableButtons();
    }
    else for (int i=0; i<m_model->rowCount(); i++)
    {
        UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_model->item(i));
        if (itm)
        {
            MotCle *mcs = dynamic_cast<MotCle*>(itm->item());
            if (mcs)
                if (mcs == m_currentmotcle)
                {
                    QModelIndex idx = m_model->index(i,1);
                    wdg_tblview->clearSelection();
                    wdg_tblview->selectionModel()->setCurrentIndex(idx,QItemSelectionModel::Select);
                    wdg_tblview->scrollTo(idx, hint);
                    OKButton->setEnabled(true);
                    EnableButtons(m_currentmotcle);
                    break;
                }
        }
    }
}

void dlg_listemotscles::setMotCleToRow(MotCle *mc, int row)
{
    UpStandardItem *pitem0 = new UpStandardItem("", mc);
    pitem0->setCheckable(true);
    if (m_listidmotsclesdepart.contains(mc->id()))
        pitem0->setCheckState(Qt::Checked);
    else
        pitem0->setCheckState(Qt::Unchecked);
    m_model->setItem(row,0,pitem0);
    QModelIndex index = m_model->index(row, 1, QModelIndex());
    m_model->setData(index, mc->motcle());
}

void dlg_listemotscles::SupprimeMotCle(MotCle *mc)
{
    UpMessageBox msgbox;
    msgbox.setText(tr("Suppression de mot clé!"));
    msgbox.setInformativeText(tr("Voulez vous vraiment supprimer le mot-clé\n") + mc->motcle());
    msgbox.setIcon(UpMessageBox::Warning);
    UpSmallButton AnnulBouton(tr("Annuler"));
    UpSmallButton OKBouton(tr("Supprimer le mot-clé"));
    msgbox.addButton(&AnnulBouton, UpSmallButton::CANCELBUTTON);
    msgbox.addButton(&OKBouton, UpSmallButton::SUPPRBUTTON);
    msgbox.setDefaultButton(&AnnulBouton);
    msgbox.exec();
    if (msgbox.clickedButton() == &OKBouton)
    {
        int row = getRowFromMotCle(mc);
        if (row > -1 && row < m_model->rowCount())
        {
            m_model->removeRow(row);
            Datas::I()->motscles->SupprimeMotCle(mc);
        }
    }
}

void dlg_listemotscles::Validation()
{
    if (m_mode == Modification || m_mode == Creation)
    {
        wdg_tblview->setEditTriggers(QAbstractItemView::NoEditTriggers);
        EnregistreMotCle(m_currentmotcle);
        ConfigMode(Selection, m_currentmotcle);
    }
    else if (m_mode == Selection)
    {
        db->StandardSQL("delete from " TBL_MOTSCLESJOINTURES " where idpat = " + QString::number(m_currentpatient->id()));
        QList<int> listidMc;
        for (int i=0; i< m_model->rowCount(); i++)
            if(m_model->item(i,0)->checkState() == Qt::Checked)
            {
                MotCle* mc = getMotCleFromIndex(m_model->index(i,0));
                if (mc)
                    listidMc << mc->id();
            }
        if (listidMc.size()>0)
        {
            QString req = "insert into " TBL_MOTSCLESJOINTURES " (idpat, idmotcle) values ";
            req += "(" + QString::number(m_currentpatient->id()) + ", " + QString::number(listidMc.at(0)) + ")";
            for (int j=1; j<listidMc.size(); j++)
                req += ", (" + QString::number(m_currentpatient->id()) + ", " + QString::number(listidMc.at(j)) + ")";
            //qDebug() << req;
            db->StandardSQL(req);
        }
        accept();
    }
}

