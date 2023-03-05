/* (C) 2022 LAINE SERGE
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

#include "dlg_listetypesinterventions.h"

dlg_listetypesinterventions::dlg_listetypesinterventions(TypeIntervention *typ, QWidget *parent) :
    UpDialog(PATH_FILE_INI, "PositionsFiches/PositionTypesInterventions", parent)
{
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    setWindowModality(Qt::WindowModal);

    wdg_tblview = new UpTableView();
    wdg_buttonframe = new WidgetButtonFrame(wdg_tblview);
    wdg_buttonframe->AddButtons(WidgetButtonFrame::Plus | WidgetButtonFrame::Modifier | WidgetButtonFrame::Moins);
    wdg_buttonframe->addSearchLine();

    dlglayout()->insertWidget(0,wdg_buttonframe->widgButtonParent());
    dlglayout()     ->setSizeConstraint(QLayout::SetFixedSize);
    AjouteLayButtons(UpDialog::ButtonCancel|UpDialog::ButtonOK);
    CancelButton    ->disconnect();

    setSizeGripEnabled(false);

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

    connect (OKButton,          &QPushButton::clicked,      this,   &dlg_listetypesinterventions::Validation);
    connect (CancelButton,      &QPushButton::clicked,      this,   &dlg_listetypesinterventions::Annulation);
    connect (wdg_buttonframe,   &WidgetButtonFrame::choix,  this,   &dlg_listetypesinterventions::ChoixButtonFrame);
    connect(wdg_buttonframe->searchline(),  &QLineEdit::textEdited,     this,   [=] (QString txt) { txt = Utils::trimcapitilize(txt, false, true);
                                                                                                    wdg_buttonframe->searchline()->setText(txt);
                                                                                                    for (auto it = Datas::I()->typesinterventions->typeinterventions()->constBegin(); it != Datas::I()->typesinterventions->typeinterventions()->constEnd(); ++it)
                                                                                                    {
                                                                                                        TypeIntervention *typ = const_cast<TypeIntervention*>(it.value());
                                                                                                        if (typ->typeintervention().toUpper().startsWith(txt.toUpper()))
                                                                                                        {
                                                                                                            selectcurrenttype(typ, QAbstractItemView::PositionAtCenter);
                                                                                                            break;
                                                                                                        }
                                                                                                    }
                                                                                                    });

    RemplirTableView();
    wdg_buttonframe->searchline()->setCompleter(Datas::I()->typesinterventions->completer());
    wdg_buttonframe->wdg_modifBouton->setEnabled(false);
    wdg_buttonframe->wdg_moinsBouton->setEnabled(false);
    wdg_buttonframe->searchline()->setFocus();
    selectcurrenttype(typ);
}

dlg_listetypesinterventions::~dlg_listetypesinterventions()
{
}

void dlg_listetypesinterventions::keyPressEvent(QKeyEvent * event )
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
void dlg_listetypesinterventions::Annulation()
{
    wdg_tblview->setEditTriggers(QAbstractItemView::NoEditTriggers);
    if (m_mode == Creation || m_mode == Modification)
    {
        int row = m_model->getRowFromItem(m_currenttype);
        if (m_mode == Modification && row < m_model->rowCount())
        {
            QModelIndex idx = m_model->index(row,0);
            wdg_tblview->closePersistentEditor(idx);
            m_model->setData(idx, m_currenttype->typeintervention());
            QModelIndex idx2 = m_model->index(row,1);
            wdg_tblview->closePersistentEditor(idx2);
            m_model->setData(idx2, m_currenttype->codeCCAM());
            EnableButtons(m_currenttype);
        }
        else if (m_mode == Creation)
        {
            m_model->removeRow(row);
            if(m_model->rowCount() > 0 && row < m_model->rowCount())
                selectcurrenttype(getTypeFromIndex(m_model->index(row,0)));
        }
        else
            RemplirTableView();
        ConfigMode(Selection);
    }
    else
        reject();
}

// ----------------------------------------------------------------------------------
// Recherche de doublon après création ou modification de type
// ----------------------------------------------------------------------------------
bool dlg_listetypesinterventions::ChercheDoublon(QString str, int row)
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
                TypeIntervention *typ = dynamic_cast<TypeIntervention*>(itm->item());
                if (typ)
                {
                    if (typ->typeintervention().toUpper() == str.toUpper() && i != row)
                    {
                        a = true;
                        UpMessageBox::Watch(this, tr("Il existe déjà un type d'intervention portant ce nom"));
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

void dlg_listetypesinterventions::ChoixButtonFrame()
{
    switch (wdg_buttonframe->Choix()) {
    case WidgetButtonFrame::Plus:
        ConfigMode(Creation, m_currenttype);
        break;
    case WidgetButtonFrame::Modifier:
        ConfigMode(Modification,m_currenttype);
        break;
    case WidgetButtonFrame::Moins:
        Supprimetype(m_currenttype);
        break;
    }
}

void dlg_listetypesinterventions::ChoixMenuContextuel(QString choix)
{
    if (choix  == "Modifier")
        ConfigMode(Modification,m_currenttype);
    else if (choix  == "Supprimer")
        Supprimetype(m_currenttype);
    else if (choix  == "Creer")
        ConfigMode(Creation, m_currenttype);
}

// ----------------------------------------------------------------------------------
// Configuration de la fiche en fonction du mode de fonctionnement
// ----------------------------------------------------------------------------------
void dlg_listetypesinterventions::ConfigMode(Mode mode, TypeIntervention *typ)
{
    m_mode = mode;
    wdg_buttonframe->setEnabled(m_mode == Selection);
    wdg_buttonframe->searchline()->setReadOnly(m_mode != Selection);

    if (mode == Selection)
    {
        EnableLines();
        wdg_tblview->setStyleSheet("");
        wdg_buttonframe->wdg_modifBouton->setEnabled(false);
        wdg_buttonframe->wdg_moinsBouton->setEnabled(false);

        CancelButton->setUpButtonStyle(UpSmallButton::CANCELBUTTON);
        CancelButton->setImmediateToolTip(tr("Annuler et fermer la fiche"));
        OKButton->setImmediateToolTip(tr("Enregistrer\net fermer"));
        if (typ)
            selectcurrenttype(typ, QAbstractItemView::PositionAtCenter);

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
    else if (mode == Modification)
    {
        if (!typ)
            return;
        wdg_buttonframe->searchline()->setText("");
        DisableLines();
        for (int i=0; i<m_model->rowCount(); ++i)
        {
            UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_model->item(i,0));
            if (itm)
            {
                TypeIntervention *mcs = dynamic_cast<TypeIntervention*>(itm->item());
                if (mcs)
                    if (mcs == typ)
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
        CancelButton->setImmediateToolTip(tr("Revenir au mode\nsélection de type d'intervention"));
        OKButton->setImmediateToolTip(tr("Enregistrer"));
        OKButton->setEnabled(false);
    }
    else if (mode == Creation)
    {
        wdg_buttonframe->searchline()->setText("");
        if (m_model->rowCount() > 0)
            DisableLines();
        int row = 0;
        if (typ)
            row = m_model->getRowFromItem(typ);
        m_model->insertRow(row);
        m_currenttype = new TypeIntervention();
        m_currenttype->settypeintervention(tr("Nouveau type d'intervention"));
        settypeToRow(m_currenttype, row);
        m_model->item(row,0)->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        m_model->item(row,1)->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        wdg_tblview->scrollTo(m_model->index(row,0), QAbstractItemView::EnsureVisible);
        wdg_tblview->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
        wdg_tblview->openPersistentEditor(m_model->index(row,0));

        CancelButton->setUpButtonStyle(UpSmallButton::BACKBUTTON);
        CancelButton->setEnabled(true);
        CancelButton->setImmediateToolTip(tr("Revenir au mode\nsélection de type d'intervention"));
        OKButton->setEnabled(false);
        OKButton->setImmediateToolTip(tr("Enregistrer\nle type d'intervention"));
    }
}

void dlg_listetypesinterventions::DisableLines()
{
    for (int i=0; i<m_model->rowCount(); i++)
    {
        QStandardItem *itm = m_model->item(i,0);
        if (itm)
            itm->setFlags(Qt::NoItemFlags);
        QStandardItem *itm1 = m_model->item(i,1);
        if (itm1)
            itm1->setFlags(Qt::NoItemFlags);
    }
}

void dlg_listetypesinterventions::EnableButtons(TypeIntervention *typ)
{
    wdg_buttonframe->wdg_modifBouton->setEnabled(true);
    wdg_buttonframe->wdg_modifBouton->setEnabled(typ);
    bool isused = false;
    if (typ)
    {
        wdg_buttonframe->wdg_modifBouton->setEnabled(typ);
        isused = Datas::I()->typesinterventions->isThisTypUsed(typ);
    }
    wdg_buttonframe->wdg_moinsBouton->setEnabled(!isused);
}

void dlg_listetypesinterventions::EnableLines(int row)
{
    auto enableline = [&] (int i)
    {
        QStandardItem *itm = m_model->item(i,0);
        if (itm)
            itm->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        QStandardItem *itm1 = m_model->item(i,1);
        if (itm1)
            itm1->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    };
    if (row == -1)
        for (int i=0; i<m_model->rowCount(); i++)
            enableline(i);
    else
        enableline(row);
}

// ----------------------------------------------------------------------------------
// Modification du mot clé dans la base.
// ----------------------------------------------------------------------------------
bool dlg_listetypesinterventions::EnregistreType(TypeIntervention *typ)
{
    int row = m_model->getRowFromItem(typ);
    UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_model->item(row));
    if (!itm)
        return false;
    wdg_tblview->closePersistentEditor(m_model->index(row,0));
    wdg_tblview->closePersistentEditor(m_model->index(row,1));
    qApp->focusWidget()->clearFocus();   //permet de déclencher le focusout du delegate qui va lancer le signal commiData
    m_model->setData(m_model->index(row,0),m_textdelegate);
    m_model->setData(m_model->index(row,1),m_ccamdelegate);
    QString TypeIntervention = Utils::capitilize(m_textdelegate.left(80), true);
    QString CCAM             = Utils::capitilize(m_ccamdelegate.left(15), true);
    // recherche de l'enregistrement modifié
    // controle validate des champs
    if (ChercheDoublon(TypeIntervention,row))
    {
        wdg_tblview->openPersistentEditor(m_model->index(row,0));
        return false;
    }
    m_listbinds[CP_TYPEINTERVENTION_TYPINTERVENTION ]   = TypeIntervention;
    m_listbinds[CP_CODECCAM_TYPINTERVENTION ]           = CCAM;
    int idtyp = typ->id();
    if (m_mode == Creation)
    {
        delete typ;
        m_currenttype = Datas::I()->typesinterventions->CreationTypeIntervention(m_listbinds);
    }
    else if (m_mode == Modification)
    {
        DataBase::I()->UpdateTable(TBL_TYPESINTERVENTIONS, m_listbinds, "where " CP_ID_TYPINTERVENTION " = " + QString::number(typ->id()));
        m_currenttype = Datas::I()->typesinterventions->getById(idtyp, true);
    }
    settypeToRow(m_currenttype, row);
    if (m_currenttype)
    {
        m_model->sort(1);
        selectcurrenttype(m_currenttype);
    }
    return true;
}

TypeIntervention* dlg_listetypesinterventions::getTypeFromIndex(QModelIndex idx)
{
    int row = idx.row();
    UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_model->item(row));
    if (itm)
        return dynamic_cast<TypeIntervention*>(itm->item());
    else
        return Q_NULLPTR;
}

QList<int> dlg_listetypesinterventions::listtypDepart() const
{
    return m_listidtypesdepart;
}

void dlg_listetypesinterventions::MenuContextuel()
{
    QMenu *menuContextuel = new QMenu(this);
    QAction *pAction_Modif;
    QAction *pAction_Suppr;
    QAction *pAction_Creer;
    pAction_Creer                = menuContextuel->addAction(Icons::icCreer(), tr("Créer un typr d'intervention"));
    connect (pAction_Creer,      &QAction::triggered, this,   [&] {ChoixMenuContextuel("Creer");});

    QModelIndex idx   = wdg_tblview->indexAt(wdg_tblview->viewport()->mapFromGlobal(cursor().pos()));
    m_currenttype = getTypeFromIndex(idx);
    bool isused = false;
    if (m_currenttype)
    {
        pAction_Modif                = menuContextuel->addAction(Icons::icEditer(), tr("Modifier ce type d'intervention"));
        connect (pAction_Modif,      &QAction::triggered, this,   [&] {ChoixMenuContextuel("Modifier");});
        isused = Datas::I()->typesinterventions->isThisTypUsed(m_currenttype);
        if (isused)
        {
            pAction_Suppr                = menuContextuel->addAction(Icons::icPoubelle(), tr("Supprimer ce type d'intervention"));
            connect (pAction_Suppr,      &QAction::triggered, this,   [&] {ChoixMenuContextuel("Supprimer");});
        }
    }
    // ouvrir le menu
    menuContextuel->exec(cursor().pos());
}

void dlg_listetypesinterventions::RemplirTableView()
{
    wdg_tblview->disconnect();
    wdg_tblview->selectionModel()->disconnect();
    UpLineDelegate *linetxt = new UpLineDelegate();
    connect(linetxt,   &UpLineDelegate::textEdited, this, [&] {OKButton->setEnabled(true);});
    connect(linetxt,   &UpLineDelegate::commitData, this, [&](QWidget *editor) {
                                                                        UpLineEdit *line = qobject_cast<UpLineEdit*>(editor);
                                                                        if (line)
                                                                            m_textdelegate = line->text();
                                                                      });
    UpLineDelegate *lineccam = new UpLineDelegate();
    connect(lineccam,   &UpLineDelegate::textEdited, this, [&] {OKButton->setEnabled(true);});
    connect(lineccam,   &UpLineDelegate::commitData, this, [&](QWidget *editor) {
                                                                        UpLineEdit *line = qobject_cast<UpLineEdit*>(editor);
                                                                        if(line)
                                                                            m_ccamdelegate = line->text();
                                                                      });
    wdg_tblview->setItemDelegateForColumn(0,linetxt);
    wdg_tblview->setItemDelegateForColumn(1,lineccam);
    m_listidtypesdepart.clear();
    for (auto it = Datas::I()->typesinterventions->typeinterventions()->constBegin(); it != Datas::I()->typesinterventions->typeinterventions()->constEnd() ;++it )
        if (it.value())
            m_listidtypesdepart << it.value()->id();
    if (m_model == Q_NULLPTR)
        delete m_model;
    m_model = new UpStandardItemModel(this);

    QStandardItem *pitem1   = new QStandardItem(tr("TITRE"));
    pitem1->setEditable(false);
    pitem1->setTextAlignment(Qt::AlignLeft);
    m_model->setHorizontalHeaderItem(0,pitem1);
    QStandardItem *pitem2   = new QStandardItem(tr("CCAM"));
    pitem2->setEditable(false);
    pitem2->setTextAlignment(Qt::AlignLeft);
    m_model->setHorizontalHeaderItem(1,pitem2);
    m_model->setRowCount(Datas::I()->typesinterventions->typeinterventions()->size());
    m_model->setColumnCount(2);
    for (int i=0; i<Datas::I()->typesinterventions->typeinterventions()->size(); i++)
    {
        TypeIntervention *typ = Datas::I()->typesinterventions->typeinterventions()->values().at(i);
        if (typ)
            settypeToRow(typ,i, false);
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
        wdg_tblview->setColumnWidth(0,270);
        wdg_tblview->setColumnWidth(1,130);
        wdg_tblview->FixLargeurTotale();
        wdg_buttonframe->widgButtonParent()->setFixedWidth(wdg_tblview->width());
        ConfigMode(Selection);
        connect (wdg_tblview,                   &QWidget::customContextMenuRequested,   this,   &dlg_listetypesinterventions::MenuContextuel);
        connect (wdg_tblview->selectionModel(), &QItemSelectionModel::currentRowChanged,this,   [&] (QModelIndex idx) {
                                                                                                                        m_currenttype = getTypeFromIndex(idx);
                                                                                                                        if (m_currenttype)
                                                                                                                        {
                                                                                                                            OKButton->setEnabled(true);
                                                                                                                            EnableButtons(m_currenttype);
                                                                                                                            wdg_buttonframe->searchline()->setText(m_currenttype->typeintervention());
                                                                                                                        }
                                                                                                                        });
        connect(wdg_tblview,                    &QAbstractItemView::doubleClicked,      this,   [=] (QModelIndex idx) {
            TypeIntervention *typ = getTypeFromIndex(idx);
            if (typ)
            {
                m_currenttype = typ;
                if (m_mode == Selection)
                    ConfigMode(Modification, m_currenttype);
            }
        });
        wdg_tblview->selectionModel()->clear();
    }
    else
        ConfigMode(Creation);
}

void dlg_listetypesinterventions::selectcurrenttype(TypeIntervention *typ, QAbstractItemView::ScrollHint hint)
{
    wdg_tblview->selectionModel()->clear();
    m_currenttype = typ;
    if (!m_currenttype)
        EnableButtons();
    else for (int i=0; i<m_model->rowCount(); i++)
    {
        UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_model->item(i));
        if (itm)
        {
            TypeIntervention *mcs = dynamic_cast<TypeIntervention*>(itm->item());
            if (mcs)
                if (mcs == m_currenttype)
                {
                    QModelIndex idx = m_model->index(i,0);
                    wdg_tblview->selectionModel()->select(idx,QItemSelectionModel::SelectCurrent);
                    wdg_tblview->scrollTo(idx, hint);
                    OKButton->setEnabled(true);
                    EnableButtons(m_currenttype);
                    break;
                }
        }
    }
}

void dlg_listetypesinterventions::settypeToRow(TypeIntervention *typ, int row, bool resizecolumn)
{
    if (!typ)
        return;
    if (row < 0 || row > m_model->rowCount()-1)
        return;
    UpStandardItem *itemtyp = new UpStandardItem(typ->typeintervention(), typ);
    m_model->setItem(row,0,itemtyp);
    UpStandardItem *itemccam = new UpStandardItem(typ->codeCCAM(), typ);
    m_model->setItem(row,1,itemccam);
    QList<QStandardItem *> items;
    items << itemtyp << itemccam;
    QModelIndex index = m_model->index(row, 0, QModelIndex());
    m_model->setData(index, typ->typeintervention());
    QModelIndex index2 = m_model->index(row, 1, QModelIndex());
    m_model->setData(index2, typ->codeCCAM());
    if(!resizecolumn)
        return;

    //! la suite est obligatoire pour contourner un bug d'affichage sous MacOS
    wdg_tblview->setColumnWidth(0,270);
    wdg_tblview->setColumnWidth(1,130);
}

void dlg_listetypesinterventions::Supprimetype(TypeIntervention *typ)
{
    UpMessageBox msgbox(this);
    msgbox.setText(tr("Suppression de type d'intervention!"));
    msgbox.setInformativeText(tr("Voulez vous vraiment supprimer le type d'intervention\n") + typ->typeintervention());
    msgbox.setIcon(UpMessageBox::Warning);
    UpSmallButton AnnulBouton(tr("Annuler"));
    UpSmallButton OKBouton(tr("Supprimer le type d'intervention"));
    msgbox.addButton(&AnnulBouton, UpSmallButton::CANCELBUTTON);
    msgbox.addButton(&OKBouton, UpSmallButton::SUPPRBUTTON);
    msgbox.setDefaultButton(&AnnulBouton);
    msgbox.exec();
    if (msgbox.clickedButton() == &OKBouton)
    {
        int row = m_model->getRowFromItem(typ);
        if (row > -1 && row < m_model->rowCount())
        {
            m_model->removeRow(row);
            Datas::I()->typesinterventions->SupprimeTypeIntervention(typ);
        }
        if (m_model->rowCount() == 0)
            ConfigMode(Creation);
        else
        {
            ConfigMode(Selection);
            if (row >= m_model->rowCount())
                row = m_model->rowCount()-1;
            typ = getTypeFromIndex(m_model->index(row,0));
            if (typ)
                selectcurrenttype(typ);
        }
    }
}

void dlg_listetypesinterventions::Validation()
{
    if (m_mode == Modification || m_mode == Creation)
    {
        wdg_tblview->setEditTriggers(QAbstractItemView::NoEditTriggers);
        if (EnregistreType(m_currenttype))
        {
            ConfigMode(Selection, m_currenttype);
            selectcurrenttype(m_currenttype);
        }
    }
    else if (m_mode == Selection)
    {
        accept();
    }
}




