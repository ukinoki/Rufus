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

#include "gbl_datas.h"
#include "dlg_listecorrespondants.h"
#include "icons.h"

dlg_listecorrespondants::dlg_listecorrespondants(QWidget *parent) :
    UpDialog(PATH_FILE_INI, "PositionsFiches/ListeCorrespondants",parent)
{
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::WindowMinMaxButtonsHint);
    setWindowTitle(tr("Liste des correspondants"));

    wdg_itemstree = new QTreeView(this);
    wdg_itemstree ->setFixedWidth(320);
    wdg_itemstree ->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    wdg_itemstree ->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    wdg_itemstree ->setFocusPolicy(Qt::StrongFocus);
    wdg_itemstree ->setEditTriggers(QAbstractItemView::NoEditTriggers);
    wdg_itemstree ->setIndentation(10);
    wdg_itemstree ->setMouseTracking(true);
    wdg_itemstree ->header()->setVisible(false);

    ReconstruitTreeViewCorrespondants(true);

    wdg_buttonframe         = new WidgetButtonFrame(wdg_itemstree);
    wdg_buttonframe         ->AddButtons(WidgetButtonFrame::Plus | WidgetButtonFrame::Modifier | WidgetButtonFrame::Moins);

    wdg_buttonframe->addSearchLine();
    AjouteLayButtons(UpDialog::ButtonCancel | UpDialog::ButtonOK);


    dlglayout()->insertWidget(0,wdg_buttonframe->widgButtonParent());
    setFixedWidth(wdg_itemstree->width() + dlglayout()->contentsMargins().right() + dlglayout()->contentsMargins().left());

    connect(OKButton,                       &QPushButton::clicked,      this,   &QDialog::reject);
    connect(wdg_buttonframe->searchline(),  &QLineEdit::textEdited,     this,   [=] (QString txt) { txt = Utils::trimcapitilize(txt, false, true);
                                                                                                    wdg_buttonframe->searchline()->setText(txt);
                                                                                                    ReconstruitTreeViewCorrespondants(false, txt);});
    connect(wdg_buttonframe,        &WidgetButtonFrame::choix,  this,   &dlg_listecorrespondants::ChoixButtonFrame);

    wdg_buttonframe->wdg_modifBouton    ->setEnabled(false);
    wdg_buttonframe->wdg_moinsBouton    ->setEnabled(false);
    wdg_buttonframe->searchline()       ->setFocus();
}

dlg_listecorrespondants::~dlg_listecorrespondants()
{
}

void dlg_listecorrespondants::Enablebuttons(QModelIndex idx)
{
    wdg_buttonframe->wdg_modifBouton->setEnabled(getCorrespondantFromIndex(idx) != Q_NULLPTR);
    wdg_buttonframe->wdg_moinsBouton->setEnabled(getCorrespondantFromIndex(idx) != Q_NULLPTR);
}


void dlg_listecorrespondants::ChoixButtonFrame()
{
    Correspondant *cor = Q_NULLPTR;
    if (wdg_itemstree->selectionModel()->selectedIndexes().size())
        cor = getCorrespondantFromIndex(wdg_itemstree->selectionModel()->selectedIndexes().at(0));
    switch (wdg_buttonframe->Choix()) {
    case WidgetButtonFrame::Plus:
        EnregistreNouveauCorresp();
        break;
    case WidgetButtonFrame::Modifier:
        if (cor)
            ModifCorresp(cor);
        break;
    case WidgetButtonFrame::Moins:
        if (cor)
        SupprCorresp(cor);
        break;
    }
}

bool dlg_listecorrespondants::listecorrespondantsmodifiee() const
{
    return m_listemodifiee;
}

// ------------------------------------------------------------------------------------------
// Enregistre un nouveau correpondant
// ------------------------------------------------------------------------------------------
void dlg_listecorrespondants::EnregistreNouveauCorresp()
{
    bool onlydoctors    = false;
    dlg_identificationcorresp *Dlg_IdentCorresp    = new dlg_identificationcorresp(dlg_identificationcorresp::Creation, onlydoctors, Q_NULLPTR, this);
    Dlg_IdentCorresp->setWindowModality(Qt::WindowModal);
    if (Dlg_IdentCorresp->exec() == QDialog::Accepted)
    {
        int idcor = Dlg_IdentCorresp->idcurrentcorrespondant();
         m_listemodifiee = true;
        ReconstruitTreeViewCorrespondants(true);
        Correspondant *cor = Q_NULLPTR;
        if (idcor>0)
            cor = Datas::I()->correspondants->getById(idcor);
        if (cor)
            scrollToCorresp(cor);
    }
    delete Dlg_IdentCorresp;
}

// ------------------------------------------------------------------------------------------
// renvoie le correpondant correspodant à l'index
// ------------------------------------------------------------------------------------------
Correspondant* dlg_listecorrespondants::getCorrespondantFromIndex(QModelIndex idx )
{
    UpStandardItem *it = dynamic_cast<UpStandardItem*>(m_correspondantsmodel->itemFromIndex(idx));
    if (it != Q_NULLPTR)
        return dynamic_cast<Correspondant *>(it->item());
    else
        return Q_NULLPTR;
}


// ------------------------------------------------------------------------------------------
// Modifie un correpondant
// ------------------------------------------------------------------------------------------
void dlg_listecorrespondants::ModifCorresp(Correspondant *cor)
{
    if (cor == Q_NULLPTR)
        return;
    bool onlydoctors    = false;
    dlg_identificationcorresp *Dlg_IdentCorresp    = new dlg_identificationcorresp(dlg_identificationcorresp::Modification, onlydoctors, cor, this);
    Dlg_IdentCorresp->setWindowModality(Qt::WindowModal);
    if (Dlg_IdentCorresp->exec() == QDialog::Accepted)
    {
        int idcor = cor->id();
        m_listemodifiee = true;
        ReconstruitTreeViewCorrespondants(true);
        cor = Datas::I()->correspondants->getById(idcor);
        if (cor)
            scrollToCorresp(cor);
    }
    delete Dlg_IdentCorresp;
}

void dlg_listecorrespondants::scrollToCorresp(Correspondant *cor)
{
    if (cor != Q_NULLPTR)
    {
        for (int i=0; i < m_correspondantsmodel->rowCount(); ++i)
        {
            UpStandardItem *itm = dynamic_cast<UpStandardItem *>(m_correspondantsmodel->item(i));
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
                                Correspondant *scor = dynamic_cast<Correspondant*>(childitm->item());
                                if (scor)
                                {
                                    if (scor->id() == cor->id())
                                    {
                                        wdg_itemstree->scrollTo(childitm->index(), QAbstractItemView::PositionAtCenter);
                                        wdg_itemstree->selectionModel()->select(childitm->index(),QItemSelectionModel::Rows | QItemSelectionModel::Select);
                                        j = itm->rowCount();
                                        i = m_correspondantsmodel->rowCount();
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
// Supprime un correpondant
// ------------------------------------------------------------------------------------------
void dlg_listecorrespondants::SupprCorresp(Correspondant *cor)
{
    if (!cor) return;
    QString Msg;
    Msg = tr("Etes vous sûr de vouloir supprimer la fiche") + "\n " +
           cor->nomprenom() + "?" +
            "\n" + tr("La suppression de cette fiche est IRRÉVERSIBLE.");
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
        Datas::I()->correspondants->SupprimeCorrespondant(getCorrespondantFromIndex(wdg_itemstree->selectionModel()->selectedIndexes().at(0)));
        m_listemodifiee = true;
        ReconstruitTreeViewCorrespondants(true);
    }
}

QList<UpStandardItem*> dlg_listecorrespondants::ListeMetiers()
{
    QList<UpStandardItem*> listmetiers;
    QStringList list;
    UpStandardItem *metieritem;
    for (auto it = Datas::I()->correspondants->correspondants()->constBegin(); it != Datas::I()->correspondants->correspondants()->constEnd(); ++it)
    {
        Correspondant *cor = const_cast<Correspondant*>(it.value());
        QString metier  = Utils::trimcapitilize(cor->metier(), true, false);
        if (!list.contains(metier))
        {
            list << metier;
            metieritem  = new UpStandardItem(metier);
            metieritem  ->setForeground(QBrush(QColor(Qt::red)));
            metieritem  ->setEditable(false);
            metieritem  ->setEnabled(false);
            listmetiers << metieritem;
        }
    }
    return listmetiers;
}

void dlg_listecorrespondants::ReconstruitTreeViewCorrespondants(bool reconstruirelaliste, QString filtre)
{
    if (reconstruirelaliste)
        Datas::I()->correspondants->initListe(true);
    wdg_itemstree->disconnect();
    wdg_itemstree->selectionModel()->disconnect();
    if (m_correspondantsmodel == Q_NULLPTR)
        delete m_correspondantsmodel;
    m_correspondantsmodel = new QStandardItemModel(this);

    UpStandardItem *pitem;
    foreach(UpStandardItem *item, ListeMetiers())
        m_correspondantsmodel->appendRow(item);

    for (auto it = Datas::I()->correspondants->correspondants()->constBegin(); it != Datas::I()->correspondants->correspondants()->constEnd(); ++it)
    {
        Correspondant *cor = const_cast<Correspondant*>(it.value());
        if (cor->nomprenom().startsWith(filtre))
        {
            pitem   = new UpStandardItem(cor->nomprenom(), cor);
            pitem   ->setEditable(false);
            QList<QStandardItem *> listitems = m_correspondantsmodel->findItems(Utils::trimcapitilize(cor->metier(), true, false));
            if (listitems.size()>0)
                listitems.at(0)->appendRow(pitem);
        }
    }
    for (int i=0; i<m_correspondantsmodel->rowCount();i++)
        if (!m_correspondantsmodel->item(i)->hasChildren())
        {
            m_correspondantsmodel->removeRow(i);
            i--;
        }
    wdg_itemstree     ->setModel(m_correspondantsmodel);
    wdg_itemstree     ->expandAll();
    wdg_itemstree     ->setItemsExpandable(false);
    if (m_correspondantsmodel->rowCount()>0)
    {
        m_correspondantsmodel->sort(0);
        m_correspondantsmodel->sort(1);
        connect(wdg_itemstree,    &QAbstractItemView::entered,       this,   [=] (QModelIndex idx) { if (!m_correspondantsmodel->itemFromIndex(idx)->hasChildren())
                                                                                                            {
                                                                                                                Correspondant * cor = getCorrespondantFromIndex(idx);
                                                                                                                if (cor)
                                                                                                                    QToolTip::showText(cursor().pos(), cor->adresseComplete());
                                                                                                            }
                                                                                                      } );
        connect(wdg_itemstree->selectionModel(),    &QItemSelectionModel::currentChanged,       this,   &dlg_listecorrespondants::Enablebuttons);
        connect(wdg_itemstree,    &QAbstractItemView::doubleClicked, this,   [=] (QModelIndex idx) { if (!m_correspondantsmodel->itemFromIndex(idx)->hasChildren())
                                                                                                            ModifCorresp(getCorrespondantFromIndex(idx)); });
    }
}
