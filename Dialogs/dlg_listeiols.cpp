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

#include "dlg_listeiols.h"

dlg_listeiols::dlg_listeiols(QWidget *parent) :
    UpDialog(parent)
{
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::WindowMinMaxButtonsHint);

    setModal(true);
    setWindowTitle(tr("Liste des IOLs"));

    wdg_iolstree = new QTreeView(this);
    wdg_iolstree ->setFixedWidth(320);
    wdg_iolstree ->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    wdg_iolstree ->setFocusPolicy(Qt::StrongFocus);
    wdg_iolstree ->setEditTriggers(QAbstractItemView::NoEditTriggers);
    wdg_iolstree ->setAnimated(true);
    wdg_iolstree ->setIndentation(10);
    wdg_iolstree ->setMouseTracking(true);
    wdg_iolstree ->header()->setVisible(false);

    ReconstruitTreeViewIOLs(true);

    wdg_buttonframe         = new WidgetButtonFrame(wdg_iolstree);
    wdg_buttonframe         ->AddButtons(WidgetButtonFrame::PlusButton | WidgetButtonFrame::ModifButton | WidgetButtonFrame::MoinsButton);

    wdg_label               = new UpLabel();
    wdg_label               ->setFixedSize(21,21);
    wdg_label               ->setPixmap(Icons::pxLoupe().scaled(20,20)); //WARNING : icon scaled : pxLoupe 20,20
    wdg_chercheuplineedit   = new UpLineEdit();
    wdg_chercheuplineedit   ->setFixedSize(140,25);
    wdg_chercheuplineedit   ->setStyleSheet(
    "UpLineEdit {background-color:white; border: 1px solid rgb(150,150,150);border-radius: 10px;}"
    "UpLineEdit:focus {border: 2px solid rgb(164, 205, 255);border-radius: 10px;}");
    wdg_buttonframe->layButtons()->insertWidget(0,wdg_label);
    wdg_buttonframe->layButtons()->insertWidget(0,wdg_chercheuplineedit);
    AjouteLayButtons(UpDialog::ButtonOK);

    dlglayout()->insertWidget(0,wdg_buttonframe->widgButtonParent());

    connect(OKButton,               &QPushButton::clicked,      this,   &QDialog::reject);
    connect(wdg_chercheuplineedit,  &QLineEdit::textEdited,     this,   [=] (QString txt) { txt = Utils::trimcapitilize(txt, false, true);
                                                                                                    wdg_chercheuplineedit->setText(txt);
                                                                                                    ReconstruitTreeViewIOLs(false, txt);});
    connect(wdg_buttonframe,        &WidgetButtonFrame::choix,  this,   &dlg_listeiols::ChoixButtonFrame);

    wdg_buttonframe->wdg_modifBouton    ->setEnabled(false);
    wdg_buttonframe->wdg_moinsBouton    ->setEnabled(false);
    wdg_chercheuplineedit               ->setFocus();
    QString req = "select " CP_ID_IOLS " from " TBL_IOLS " where " CP_ID_IOLS " in (select " CP_IDIOL_LIGNPRGOPERATOIRE " from " TBL_LIGNESPRGOPERATOIRES ")";
    bool ok;
    QList<QVariantList> listidiols = DataBase::I()->StandardSelectSQL(req,ok);
    if (ok)
        for (int i=0; i<listidiols.size(); ++i)
            m_listidiolsutilises << listidiols.at(i).at(0).toInt();
}

dlg_listeiols::~dlg_listeiols()
{
}

void dlg_listeiols::Enablebuttons()
{
    IOL *iol = getIOLFromIndex(wdg_iolstree->selectionModel()->selectedIndexes().at(0));
    if (iol != Q_NULLPTR)
    {
        wdg_buttonframe->wdg_modifBouton->setEnabled(true);
        wdg_buttonframe->wdg_moinsBouton->setEnabled(m_listidiolsutilises.indexOf(iol->id()) == -1);
    }
    else
    {
        wdg_buttonframe->wdg_modifBouton->setEnabled(false);
        wdg_buttonframe->wdg_moinsBouton->setEnabled(false);
    }
}


void dlg_listeiols::ChoixButtonFrame()
{
    IOL *iol = Q_NULLPTR;
    if (wdg_iolstree->selectionModel()->selectedIndexes().size())
        iol = getIOLFromIndex(wdg_iolstree->selectionModel()->selectedIndexes().at(0));
    switch (wdg_buttonframe->Choix()) {
    case WidgetButtonFrame::Plus:
        EnregistreNouveauIOL();
        break;
    case WidgetButtonFrame::Modifier:
        if (iol)
            ModifIOL(iol);
        break;
    case WidgetButtonFrame::Moins:
        if (iol)
            SupprIOL(iol);
        break;
    }
}

bool dlg_listeiols::listeIOLsmodifiee() const
{
    return m_listemodifiee;
}

// ------------------------------------------------------------------------------------------
// Enregistre un nouveau correpondant
// ------------------------------------------------------------------------------------------
void dlg_listeiols::EnregistreNouveauIOL()
{
    dlg_identificationIOL *Dlg_IdentIOL    = new dlg_identificationIOL(dlg_identificationIOL::Creation);
    if (!Dlg_IdentIOL->initok())
        return;
    if (Dlg_IdentIOL->exec()>0)
    {
        IOL *iol = Datas::I()->iols->CreationIOL(Dlg_IdentIOL->Listbinds());
        m_listemodifiee = true;
        ReconstruitTreeViewIOLs(true);
        if (iol)
            scrollToIOL(iol);
    }
    delete Dlg_IdentIOL;
}
// ------------------------------------------------------------------------------------------
// renvoie l'IOL correspondant à l'index
// ------------------------------------------------------------------------------------------
IOL* dlg_listeiols::getIOLFromIndex(QModelIndex idx )
{
    UpStandardItem *it = dynamic_cast<UpStandardItem*>(m_IOLsmodel->itemFromIndex(idx));
    if (it != Q_NULLPTR)
        return dynamic_cast<IOL *>(it->item());
    else
        return Q_NULLPTR;
}


// ------------------------------------------------------------------------------------------
// Modifie un IOL
// ------------------------------------------------------------------------------------------
void dlg_listeiols::ModifIOL(IOL *iol)
{
    if (iol == Q_NULLPTR)
        return;
    dlg_identificationIOL *Dlg_IdentIOL = new dlg_identificationIOL(dlg_identificationIOL::Modification, iol, Q_NULLPTR, this);
    if (!Dlg_IdentIOL->initok())
        return;
    if (Dlg_IdentIOL->exec()>0)
    {
        DataBase::I()->UpDateIOL(iol->id(), Dlg_IdentIOL->Listbinds());
        if (iol)
        {
            int idiol = iol->id();
            m_listemodifiee = true;
            ReconstruitTreeViewIOLs(true);
            iol = Datas::I()->iols->getById(idiol);
            if (iol)
                scrollToIOL(iol);
        }
    }
    delete Dlg_IdentIOL;
}

void dlg_listeiols::scrollToIOL(IOL *iol)
{
    if (iol != Q_NULLPTR)
    {
        for (int i=0; i < m_IOLsmodel->rowCount(); ++i)
        {
            UpStandardItem *itm = dynamic_cast<UpStandardItem *>(m_IOLsmodel->item(i));
            if (itm)
            {
                if (itm->hasChildren())
                {
                    for (int j=0; j < itm->rowCount(); ++j)
                    {
                        UpStandardItem *childitm = dynamic_cast<UpStandardItem *>(itm->child(j));
                        if (childitm)
                            if (childitm->item())
                            {
                                IOL *siol = dynamic_cast<IOL*>(childitm->item());
                                if (siol)
                                {
                                    if (siol->id() == iol->id())
                                    {
                                        wdg_iolstree->scrollTo(childitm->index(), QAbstractItemView::PositionAtCenter);
                                        wdg_iolstree->selectionModel()->select(childitm->index(),QItemSelectionModel::Rows | QItemSelectionModel::Select);
                                        j = itm->rowCount();
                                        i = m_IOLsmodel->rowCount();
                                    }
                                }
                            }
                    }
                }
            }
        }
    }
}

// ------------------------------------------------------------------------------------------
// Supprime un IOL
// ------------------------------------------------------------------------------------------
void dlg_listeiols::SupprIOL(IOL *iol)
{
    if (!iol) return;
    QString Msg;
    Msg = tr("Etes vous sûr de vouloir supprimer la fiche") + "\n " +
            iol->modele() + "?" +
            "\n" + tr("La suppression de cette fiche est IRRÉVERSIBLE.");
    UpMessageBox msgbox;
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
        Datas::I()->iols->SupprimeIOL(getIOLFromIndex(wdg_iolstree->selectionModel()->selectedIndexes().at(0)));
        m_listemodifiee = true;
        ReconstruitTreeViewIOLs(true);
    }
}

void dlg_listeiols::ReconstruitListeManufacturers()
{
    if (m_manufacturersmodel == Q_NULLPTR)
        delete m_manufacturersmodel;
    m_manufacturersmodel = new QStandardItemModel(this);

    QStringList list;
    UpStandardItem *manufactureritem;
    foreach(IOL *iol, Datas::I()->iols->iols()->values())
    {
        Manufacturer *man = Datas::I()->manufacturers->getById(iol->idmanufacturer());
        if (man != Q_NULLPTR)
        {
            QString fabricant  = man->nom();
            if (!list.contains(fabricant))
            {
                list << fabricant;
                manufactureritem  = new UpStandardItem(fabricant, man);
                manufactureritem  ->setForeground(QBrush(QColor(Qt::red)));
                manufactureritem  ->setEditable(false);
                manufactureritem  ->setEnabled(false);
                m_manufacturersmodel->appendRow(manufactureritem);
            }
        }
    }
    m_manufacturersmodel->sort(0);
}

void dlg_listeiols::ReconstruitTreeViewIOLs(bool reconstruirelaliste, QString filtre)
{
    if (reconstruirelaliste)
    {
        Datas::I()->iols->initListe();
        ReconstruitListeManufacturers();
    }
    wdg_iolstree->disconnect();
    if (m_IOLsmodel == Q_NULLPTR)
        delete m_IOLsmodel;
    m_IOLsmodel = new QStandardItemModel(this);

    UpStandardItem *pitem;
    for (int i=0; i<  m_manufacturersmodel->rowCount(); ++i)
    {
        UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_manufacturersmodel->item(i));
        if (itm != Q_NULLPTR)
        {
            Manufacturer *man = dynamic_cast<Manufacturer*>(itm->item());
            if (man != Q_NULLPTR)
            {
                UpStandardItem *manufactureritem = new UpStandardItem(man->nom(), man);
                manufactureritem  ->setForeground(QBrush(QColor(Qt::red)));
                manufactureritem  ->setEditable(false);
                manufactureritem  ->setEnabled(false);
                m_IOLsmodel->appendRow(manufactureritem);
            }
        }
        m_IOLsmodel->sort(0);
    }
    foreach(IOL *iol, Datas::I()->iols->iols()->values())
    {
        if (iol->modele().startsWith(filtre))
        {
            pitem   = new UpStandardItem(iol->modele(), iol);
            if (!iol->isactif())
                pitem ->setForeground(QBrush(QColor(Qt::darkGray)));
            pitem   ->setEditable(false);
            Manufacturer *man = Datas::I()->manufacturers->getById(iol->idmanufacturer());
            if (man != Q_NULLPTR)
            {
                QString fabricant  = man->nom();
                QList<QStandardItem *> listitems = m_IOLsmodel->findItems(fabricant);
                if (listitems.size()>0)
                    listitems.at(0)->appendRow(pitem);
            }
        }
    }
    for (int i=0; i<m_IOLsmodel->rowCount();i++)
    {
        if (m_IOLsmodel->item(i) != Q_NULLPTR)
            if (!m_IOLsmodel->item(i)->hasChildren())
            {
                m_IOLsmodel->removeRow(i);
                i--;
            }
    }
    wdg_iolstree     ->setModel(m_IOLsmodel);
    wdg_iolstree     ->expandAll();
    if (m_IOLsmodel->rowCount()>0)
    {
        m_IOLsmodel->sort(0);
        m_IOLsmodel->sort(1);
        connect(wdg_iolstree,    &QAbstractItemView::entered,       this,   [=] (QModelIndex idx) { if (!m_IOLsmodel->itemFromIndex(idx)->hasChildren())
                                                                                                        {
                                                                                                            IOL*iol = getIOLFromIndex(idx);
                                                                                                            if (iol)
                                                                                                                QToolTip::showText(cursor().pos(), iol->tooltip());
                                                                                                        }
                                                                                                    } );
        connect(wdg_iolstree,    &QAbstractItemView::pressed,       this,   &dlg_listeiols::Enablebuttons);
        connect(wdg_iolstree,    &QAbstractItemView::doubleClicked, this,   [=] (QModelIndex idx) { if (!m_IOLsmodel->itemFromIndex(idx)->hasChildren())
                                                                                                            ModifIOL(getIOLFromIndex(idx)); });
    }
}
