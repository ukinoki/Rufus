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

#include "dlg_listetiers.h"

dlg_listetiers::dlg_listetiers(QWidget *parent) :
    UpDialog(PATH_FILE_INI, "PositionsFiches/ListeTiers",parent)
{
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::WindowMinMaxButtonsHint);
    setWindowTitle(tr("Liste des tiers payants"));

    wdg_itemstree = new QTreeView(this);
    wdg_itemstree ->setFixedWidth(240);
    wdg_itemstree ->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    wdg_itemstree ->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    wdg_itemstree ->setFocusPolicy(Qt::StrongFocus);
    wdg_itemstree ->setEditTriggers(QAbstractItemView::NoEditTriggers);
    wdg_itemstree ->setMouseTracking(true);
    wdg_itemstree ->header()->setVisible(false);

    ReconstruitTreeViewtiers();

    wdg_buttonframe         = new WidgetButtonFrame(wdg_itemstree);
    wdg_buttonframe         ->AddButtons(WidgetButtonFrame::Plus | WidgetButtonFrame::Modifier | WidgetButtonFrame::Moins);
    wdg_buttonframe         ->addSearchLine();

    AjouteLayButtons(UpDialog::ButtonOK);

    dlglayout()->insertWidget(0,wdg_buttonframe->widgButtonParent());
    setFixedWidth(wdg_itemstree->width() + dlglayout()->contentsMargins().right() + dlglayout()->contentsMargins().left());

    connect(OKButton,                       &QPushButton::clicked,      this,   &QDialog::reject);
    connect(wdg_buttonframe->searchline(),  &QLineEdit::textEdited,     this,   [=] (QString txt) { txt = Utils::trimcapitilize(txt, false, true);
                                                                                                    wdg_buttonframe->searchline()->setText(txt);
                                                                                                    ReconstruitTreeViewtiers(false, txt);});
    connect(wdg_buttonframe,        &WidgetButtonFrame::choix,  this,   &dlg_listetiers::ChoixButtonFrame);

    wdg_buttonframe->wdg_modifBouton    ->setEnabled(false);
    wdg_buttonframe->wdg_moinsBouton    ->setEnabled(false);
    wdg_buttonframe->searchline()       ->setFocus();
}

dlg_listetiers::~dlg_listetiers()
{
}

void dlg_listetiers::Enablebuttons(QModelIndex idx)
{
    Tiers *trs = getTiersFromIndex(idx);
    wdg_buttonframe->wdg_modifBouton->setEnabled(trs != Q_NULLPTR);
    if (trs != Q_NULLPTR)
    {
        bool ok = Datas::I()->tierspayants->isUtilise(trs->nom());
        wdg_buttonframe->wdg_moinsBouton->setEnabled(!ok);
    }
    else
        wdg_buttonframe->wdg_moinsBouton->setEnabled(false);
}

void dlg_listetiers::ChoixButtonFrame()
{
    Tiers *trs = Q_NULLPTR;
    if (wdg_itemstree->selectionModel()->selectedIndexes().size())
        trs = getTiersFromIndex(wdg_itemstree->selectionModel()->selectedIndexes().at(0));
    switch (wdg_buttonframe->Choix()) {
    case WidgetButtonFrame::Plus:
        EnregistreNouveauTiers();
        break;
    case WidgetButtonFrame::Modifier:
        if (trs)
            ModifTiers(trs);
        break;
    case WidgetButtonFrame::Moins:
        if (trs)
            SupprTiers(trs);
        break;
    }
}

bool dlg_listetiers::listetiersmodifiee() const
{
    return m_listemodifiee;
}

// ------------------------------------------------------------------------------------------
// Enregistre un nouveau correpondant
// ------------------------------------------------------------------------------------------
void dlg_listetiers::EnregistreNouveauTiers()
{
    dlg_identificationtiers *Dlg_IdentTiers    = new dlg_identificationtiers(dlg_identificationtiers::Creation, Q_NULLPTR, this);
    if (Dlg_IdentTiers->exec() == QDialog::Accepted)
    {
        Tiers * trs = Datas::I()->tierspayants->getById(Dlg_IdentTiers->idcurrentTiers());
        m_listemodifiee = true;
        ReconstruitTreeViewtiers();
        if (trs)
            scrollToTiers(trs);
    }
    delete Dlg_IdentTiers;
}

// ------------------------------------------------------------------------------------------
// renvoie le correpondant correspodant à l'index
// ------------------------------------------------------------------------------------------
Tiers* dlg_listetiers::getTiersFromIndex(QModelIndex idx )
{
    UpStandardItem *it = dynamic_cast<UpStandardItem*>(m_model->itemFromIndex(idx));
    if (it != Q_NULLPTR)
        return dynamic_cast<Tiers *>(it->item());
    else
        return Q_NULLPTR;
}

// ------------------------------------------------------------------------------------------
// Modifie un tiers payant
// ------------------------------------------------------------------------------------------
void dlg_listetiers::ModifTiers(Tiers *trs)
{
    if (trs == Q_NULLPTR)
        return;
    dlg_identificationtiers *Dlg_IdentTiers = new dlg_identificationtiers(dlg_identificationtiers::Modification, trs);
    if (Dlg_IdentTiers->exec() == QDialog::Accepted)
    {
        trs =  Datas::I()->tierspayants->getById(Dlg_IdentTiers->idcurrentTiers());
        if (trs != Q_NULLPTR)
        {
            int idtrs = trs->id();
            m_listemodifiee = true;
            ReconstruitTreeViewtiers(true);
            trs = Datas::I()->tierspayants->getById(idtrs);
            if (trs)
                scrollToTiers(trs);
        }
    }
    delete Dlg_IdentTiers;
}

void dlg_listetiers::scrollToTiers(Tiers *trs)
{
    if (trs != Q_NULLPTR)
    {
        int id = trs->id();
        for (int i=0; i < m_model->rowCount(); ++i)
        {
            UpStandardItem *itm = dynamic_cast<UpStandardItem *>(m_model->item(i));
            if (itm)
            {
                if (itm->item() != Q_NULLPTR)
                {
                    if (itm->item()->id() == id)
                    {
                        wdg_itemstree->scrollTo(itm->index(), QAbstractItemView::PositionAtCenter);
                        wdg_itemstree->selectionModel()->select(itm->index(),QItemSelectionModel::Rows | QItemSelectionModel::Select);
                        i = m_model->rowCount();
                    }
                }
            }
        }
    }
}
// ------------------------------------------------------------------------------------------
// Supprime un tiers payant
// ------------------------------------------------------------------------------------------
void dlg_listetiers::SupprTiers(Tiers *trs)
{
    if (!trs) return;
    QString Msg;
    Msg = tr("Etes vous sûr de vouloir supprimer la fiche") + "\n " +
            trs->nom() + "?" +
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
        Datas::I()->tierspayants->SupprimeTiers(getTiersFromIndex(wdg_itemstree->selectionModel()->selectedIndexes().at(0)));
        m_listemodifiee = true;
        ReconstruitTreeViewtiers();
    }
}

void dlg_listetiers::ReconstruitTreeViewtiers(bool reconstruirelaliste, QString filtre)
{
    if (reconstruirelaliste)
        Datas::I()->tierspayants->initListe();
    wdg_itemstree->disconnect();
    wdg_itemstree->selectionModel()->disconnect();
    if (m_model == Q_NULLPTR)
        delete m_model;
    m_model = new QStandardItemModel(this);

    UpStandardItem *pitem;

    for (auto it = Datas::I()->tierspayants->tierspayants()->constBegin(); it != Datas::I()->tierspayants->tierspayants()->constEnd(); ++it)
    {
        Tiers *trs = const_cast<Tiers*>(it.value());
        if (trs->nom().startsWith(filtre))
        {
            pitem   = new UpStandardItem(trs->nom(), trs);
            pitem   ->setEditable(false);
            m_model->appendRow(pitem);
        }
    }
    m_model->sort(0, Qt::AscendingOrder);
    wdg_itemstree     ->setModel(m_model);
    if (m_model->rowCount()>0)
    {
        m_model->sort(0);
        connect(wdg_itemstree,    &QAbstractItemView::entered,          this,   [=] (QModelIndex idx)   {
                                                                                                            Tiers * trs = getTiersFromIndex(idx);
                                                                                                            if (trs)
                                                                                                                QToolTip::showText(cursor().pos(), trs->tooltip());
                                                                                                        } );
        connect(wdg_itemstree->selectionModel(),    &QItemSelectionModel::currentChanged,          this,   &dlg_listetiers::Enablebuttons);
        connect(wdg_itemstree,    &QAbstractItemView::doubleClicked,    this,   [=] (QModelIndex idx) { ModifTiers(getTiersFromIndex(idx)); });
    }
}
