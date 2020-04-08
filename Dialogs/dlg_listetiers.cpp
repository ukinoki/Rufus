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
    UpDialog(parent)
{
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::WindowMinMaxButtonsHint);

    setModal(true);
    setWindowTitle(tr("Liste des tiers payants"));

    wdg_tierstree = new QTreeView(this);
    wdg_tierstree ->setFixedWidth(240);
    wdg_tierstree ->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    wdg_tierstree ->setFocusPolicy(Qt::StrongFocus);
    wdg_tierstree ->setEditTriggers(QAbstractItemView::NoEditTriggers);
    wdg_tierstree ->setMouseTracking(true);
    wdg_tierstree ->header()->setVisible(false);

    ReconstruitTreeViewtiers();

    wdg_buttonframe         = new WidgetButtonFrame(wdg_tierstree);
    wdg_buttonframe         ->AddButtons(WidgetButtonFrame::PlusButton | WidgetButtonFrame::ModifButton | WidgetButtonFrame::MoinsButton);

    wdg_label               = new UpLabel();
    wdg_label               ->setFixedSize(21,21);
    wdg_label               ->setPixmap(Icons::pxLoupe().scaled(20,20)); //WARNING : icon scaled : pxLoupe 20,20
    wdg_chercheuplineedit   = new UpLineEdit();
    wdg_chercheuplineedit   ->setFixedSize(120,25);
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
                                                                                                    ReconstruitTreeViewtiers(false, txt);});
    connect(wdg_buttonframe,        &WidgetButtonFrame::choix,  this,   &dlg_listetiers::ChoixButtonFrame);

    wdg_buttonframe->wdg_modifBouton    ->setEnabled(false);
    wdg_buttonframe->wdg_moinsBouton    ->setEnabled(false);
    wdg_chercheuplineedit               ->setFocus();
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
    if (wdg_tierstree->selectionModel()->selectedIndexes().size())
        trs = getTiersFromIndex(wdg_tierstree->selectionModel()->selectedIndexes().at(0));
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
    dlg_identificationtiers *Dlg_IdentTiers    = new dlg_identificationtiers(dlg_identificationtiers::Creation);
    if (Dlg_IdentTiers->exec()>0)
    {
        Tiers * trs = Dlg_IdentTiers->currentTiers();
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
    if (Dlg_IdentTiers->exec()>0)
    {
        trs =  Dlg_IdentTiers->currentTiers();
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
                        wdg_tierstree->scrollTo(itm->index(), QAbstractItemView::PositionAtCenter);
                        wdg_tierstree->selectionModel()->select(itm->index(),QItemSelectionModel::Rows | QItemSelectionModel::Select);
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
        Datas::I()->tierspayants->SupprimeTiers(getTiersFromIndex(wdg_tierstree->selectionModel()->selectedIndexes().at(0)));
        m_listemodifiee = true;
        ReconstruitTreeViewtiers();
    }
}

void dlg_listetiers::ReconstruitTreeViewtiers(bool reconstruirelaliste, QString filtre)
{
    if (reconstruirelaliste)
        Datas::I()->tierspayants->initListe();
    wdg_tierstree->disconnect();
    wdg_tierstree->selectionModel()->disconnect();
    if (m_model == Q_NULLPTR)
        delete m_model;
    m_model = new QStandardItemModel(this);

    UpStandardItem *pitem;

    foreach(Tiers *trs, Datas::I()->tierspayants->tierspayants()->values())
    {
        if (trs->nom().startsWith(filtre))
        {
            pitem   = new UpStandardItem(trs->nom(), trs);
            pitem   ->setEditable(false);
            m_model->appendRow(pitem);
        }
    }
    m_model->sort(0, Qt::AscendingOrder);
    wdg_tierstree     ->setModel(m_model);
    if (m_model->rowCount()>0)
    {
        m_model->sort(0);
        connect(wdg_tierstree,    &QAbstractItemView::entered,          this,   [=] (QModelIndex idx)   {
                                                                                                            Tiers * trs = getTiersFromIndex(idx);
                                                                                                            if (trs)
                                                                                                                QToolTip::showText(cursor().pos(), trs->tooltip());
                                                                                                        } );
        connect(wdg_tierstree->selectionModel(),    &QItemSelectionModel::currentChanged,          this,   &dlg_listetiers::Enablebuttons);
        connect(wdg_tierstree,    &QAbstractItemView::doubleClicked,    this,   [=] (QModelIndex idx) { ModifTiers(getTiersFromIndex(idx)); });
    }
}
