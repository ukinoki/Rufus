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
#include "dlg_listemanufacturers.h"
#include "icons.h"

dlg_listemanufacturers::dlg_listemanufacturers(QWidget *parent) :
    UpDialog(PATH_FILE_INI, "PositionsFiches/ListeManufacturers",parent)
{
    setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    setWindowTitle(tr("Liste des fabricants"));

    wdg_itemstree = new QTreeView(this);
    wdg_itemstree ->setFixedWidth(280);
    wdg_itemstree ->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    wdg_itemstree ->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    wdg_itemstree ->setFocusPolicy(Qt::StrongFocus);
    wdg_itemstree ->setEditTriggers(QAbstractItemView::NoEditTriggers);
    wdg_itemstree ->setMouseTracking(true);
    wdg_itemstree ->header()->setVisible(false);

    ReconstruitTreeViewManufacturers();

    wdg_buttonframe         = new WidgetButtonFrame(wdg_itemstree);
    wdg_buttonframe         ->AddButtons(WidgetButtonFrame::Plus | WidgetButtonFrame::Modifier | WidgetButtonFrame::Moins);
    wdg_buttonframe         ->addSearchLine();

    AjouteLayButtons(UpDialog::ButtonOK);

    dlglayout()->insertWidget(0,wdg_buttonframe->widgButtonParent());
    setFixedWidth(wdg_itemstree->width() + dlglayout()->contentsMargins().right() + dlglayout()->contentsMargins().left());

    connect(OKButton,                       &QPushButton::clicked,      this,   &QDialog::reject);
    connect(wdg_buttonframe->searchline(),  &QLineEdit::textEdited,     this,   [=] (QString txt) { txt = Utils::trimcapitilize(txt, false, true);
                                                                                                    wdg_buttonframe->searchline()->setText(txt);
                                                                                                    ReconstruitTreeViewManufacturers(txt);});
    connect(wdg_buttonframe,                &WidgetButtonFrame::choix,  this,   &dlg_listemanufacturers::ChoixButtonFrame);

    wdg_buttonframe->wdg_modifBouton    ->setEnabled(false);
    wdg_buttonframe->wdg_moinsBouton    ->setEnabled(false);
    wdg_buttonframe->searchline()       ->setFocus();
}

dlg_listemanufacturers::~dlg_listemanufacturers()
{
}

void dlg_listemanufacturers::Enablebuttons(QModelIndex idx)
{
    Manufacturer *man = getmanufacturerFromIndex(idx);
    wdg_buttonframe->wdg_modifBouton->setEnabled(man != Q_NULLPTR);
    if (man != Q_NULLPTR)
    {
        bool existmaufacturer = false;
        foreach (IOL* iol, *Datas::I()->iols->iols())
        {
            if (iol->idmanufacturer() == man->id())
            {
                existmaufacturer = true;
                break;
            }
        }
        wdg_buttonframe->wdg_moinsBouton->setEnabled(!existmaufacturer);
    }
    else
        wdg_buttonframe->wdg_moinsBouton->setEnabled(false);
}

void dlg_listemanufacturers::ChoixButtonFrame()
{
    Manufacturer *man = Q_NULLPTR;
    if (wdg_itemstree->selectionModel()->hasSelection())
        man = getmanufacturerFromIndex(wdg_itemstree->currentIndex());
    switch (wdg_buttonframe->Choix()) {
    case WidgetButtonFrame::Plus:
        EnregistreNouveauManufacturer();
        break;
    case WidgetButtonFrame::Modifier:
        if (man)
            ModifManufacturer(man);
        break;
    case WidgetButtonFrame::Moins:
        if (man)
            SupprManufacturer(man);
        break;
    }
}

bool dlg_listemanufacturers::listemanufacturersmodifiee() const
{
    return m_listemodifiee;
}

// ------------------------------------------------------------------------------------------
// Enregistre un nouveau correpondant
// ------------------------------------------------------------------------------------------
void dlg_listemanufacturers::EnregistreNouveauManufacturer()
{
    dlg_identificationmanufacturer *Dlg_IdentManufacturer    = new dlg_identificationmanufacturer(dlg_identificationmanufacturer::Creation, Q_NULLPTR, this);
    Dlg_IdentManufacturer->setWindowModality(Qt::WindowModal);
    if (Dlg_IdentManufacturer->exec() == QDialog::Accepted)
    {
        Manufacturer * man = Datas::I()->manufacturers->getById(Dlg_IdentManufacturer->idcurrentmanufacturer(), true);
        m_listemodifiee = true;
        ReconstruitTreeViewManufacturers();
        if (man)
            scrollToManufacturer(man);
    }
    delete Dlg_IdentManufacturer;
}

// ------------------------------------------------------------------------------------------
// renvoie le correpondant correspodant à l'index
// ------------------------------------------------------------------------------------------
Manufacturer* dlg_listemanufacturers::getmanufacturerFromIndex(QModelIndex idx )
{
    UpStandardItem *it = dynamic_cast<UpStandardItem*>(m_model->itemFromIndex(idx));
    if (it != Q_NULLPTR)
        return qobject_cast<Manufacturer *>(it->item());
    else
        return Q_NULLPTR;
}

// ------------------------------------------------------------------------------------------
// Modifie un fabricant
// ------------------------------------------------------------------------------------------
void dlg_listemanufacturers::ModifManufacturer(Manufacturer *man)
{
    if (man == Q_NULLPTR)
        return;
    dlg_identificationmanufacturer *Dlg_IdentManufacturer = new dlg_identificationmanufacturer(dlg_identificationmanufacturer::Modification, man, this);
    Dlg_IdentManufacturer->setWindowModality(Qt::WindowModal);
    if (Dlg_IdentManufacturer->exec() == QDialog::Accepted)
    {
        man =  Datas::I()->manufacturers->getById(Dlg_IdentManufacturer->idcurrentmanufacturer());
        if (man != Q_NULLPTR)
        {
            int idman = man->id();
            m_listemodifiee = true;
            ReconstruitTreeViewManufacturers();
            man = Datas::I()->manufacturers->getById(idman, true);
            if (man)
                scrollToManufacturer(man);
        }
    }
    delete Dlg_IdentManufacturer;
}

void dlg_listemanufacturers::scrollToManufacturer(Manufacturer *man)
{
    if (man != Q_NULLPTR)
    {
        int id = man->id();
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
// Supprime un fabricant
// ------------------------------------------------------------------------------------------
void dlg_listemanufacturers::SupprManufacturer(Manufacturer *man)
{
    if (!man) return;
    QString Msg;
    Msg = tr("Etes vous sûr de vouloir supprimer la fiche") + "\n " +
            man->nom() + "?" +
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
        Datas::I()->manufacturers->SupprimeManufacturer(getmanufacturerFromIndex(wdg_itemstree->currentIndex()));
        m_listemodifiee = true;
        ReconstruitTreeViewManufacturers();
    }
}

void dlg_listemanufacturers::ReconstruitTreeViewManufacturers(QString filtre)
{
    wdg_itemstree->disconnect();
    wdg_itemstree->selectionModel()->disconnect();
    if (m_model != Q_NULLPTR)
        delete m_model;
    m_model = new QStandardItemModel(this);

    UpStandardItem *pitem;

    for (auto it = Datas::I()->manufacturers->manufacturers()->constBegin(); it != Datas::I()->manufacturers->manufacturers()->constEnd(); ++it)
    {
        Manufacturer *man = const_cast<Manufacturer*>(it.value());
        if (man->nom().toUpper().startsWith(filtre.toUpper()))
        {
            pitem   = new UpStandardItem(man->nom(), man);
            pitem   ->setEditable(false);
            if (!man->isactif())
                pitem ->setForeground(QBrush(QColor(Qt::gray)));
            m_model->appendRow(pitem);
        }
    }
    m_model->sort(0, Qt::AscendingOrder);
    wdg_itemstree     ->setModel(m_model);
    if (m_model->rowCount()>0)
    {
        m_model->sort(0);
        connect(wdg_itemstree,    &QAbstractItemView::entered,          this,   [=] (QModelIndex idx) {
                                                                                                        Manufacturer * man = getmanufacturerFromIndex(idx);
                                                                                                        if (man)
                                                                                                            QToolTip::showText(cursor().pos(), man->tooltip());
                                                                                                      } );
        connect(wdg_itemstree->selectionModel(),    &QItemSelectionModel::currentChanged,          this,   &dlg_listemanufacturers::Enablebuttons);
        connect(wdg_itemstree,    &QAbstractItemView::doubleClicked,    this,   [=] (QModelIndex idx) { ModifManufacturer(getmanufacturerFromIndex(idx)); });
    }
}
