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
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::WindowMinMaxButtonsHint);

    setModal(true);
    setWindowTitle(tr("Liste des fabricants"));

    wdg_itemstree = new QTreeView(this);
    wdg_itemstree ->setFixedWidth(280);
    wdg_itemstree ->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    wdg_itemstree ->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    wdg_itemstree ->setFocusPolicy(Qt::StrongFocus);
    wdg_itemstree ->setEditTriggers(QAbstractItemView::NoEditTriggers);
    wdg_itemstree ->setMouseTracking(true);
    wdg_itemstree ->header()->setVisible(false);

    ReconstruitTreeViewManufacturers(true);

    wdg_buttonframe         = new WidgetButtonFrame(wdg_itemstree);
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
    setFixedWidth(wdg_itemstree->width() + dlglayout()->contentsMargins().right() + dlglayout()->contentsMargins().left());

    connect(OKButton,               &QPushButton::clicked,      this,   &QDialog::reject);
    connect(wdg_chercheuplineedit,  &QLineEdit::textEdited,     this,   [=] (QString txt) { txt = Utils::trimcapitilize(txt, false, true);
                                                                                                    wdg_chercheuplineedit->setText(txt);
                                                                                                    ReconstruitTreeViewManufacturers(false, txt);});
    connect(wdg_buttonframe,        &WidgetButtonFrame::choix,  this,   &dlg_listemanufacturers::ChoixButtonFrame);

    wdg_buttonframe->wdg_modifBouton    ->setEnabled(false);
    wdg_buttonframe->wdg_moinsBouton    ->setEnabled(false);
    wdg_chercheuplineedit               ->setFocus();
}

dlg_listemanufacturers::~dlg_listemanufacturers()
{
    EnregistrePosition();
}

void dlg_listemanufacturers::Enablebuttons(QModelIndex idx)
{
    Manufacturer *man = getmanufacturerFromIndex(idx);
    wdg_buttonframe->wdg_modifBouton->setEnabled(man != Q_NULLPTR);
    if (man != Q_NULLPTR)
    {
        Datas::I()->iols->initListeByManufacturerId(man->id());
        wdg_buttonframe->wdg_moinsBouton->setEnabled(Datas::I()->iols->iols()->size() == 0);
    }
    else
        wdg_buttonframe->wdg_moinsBouton->setEnabled(false);
}

void dlg_listemanufacturers::ChoixButtonFrame()
{
    Manufacturer *man = Q_NULLPTR;
    if (wdg_itemstree->selectionModel()->selectedIndexes().size())
        man = getmanufacturerFromIndex(wdg_itemstree->selectionModel()->selectedIndexes().at(0));
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
    dlg_identificationmanufacturer *Dlg_IdentManufacturer    = new dlg_identificationmanufacturer(dlg_identificationmanufacturer::Creation);
    if (Dlg_IdentManufacturer->exec()>0)
    {
        Manufacturer * man = Datas::I()->manufacturers->getById(Dlg_IdentManufacturer->idcurrentmanufacturer());
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
        return dynamic_cast<Manufacturer *>(it->item());
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
    dlg_identificationmanufacturer *Dlg_IdentManufacturer = new dlg_identificationmanufacturer(dlg_identificationmanufacturer::Modification, man);
    if (Dlg_IdentManufacturer->exec()>0)
    {
        man =  Datas::I()->manufacturers->getById(Dlg_IdentManufacturer->idcurrentmanufacturer());
        if (man != Q_NULLPTR)
        {
            int idman = man->id();
            m_listemodifiee = true;
            ReconstruitTreeViewManufacturers(true);
            man = Datas::I()->manufacturers->getById(idman);
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
        Datas::I()->manufacturers->SupprimeManufacturer(getmanufacturerFromIndex(wdg_itemstree->selectionModel()->selectedIndexes().at(0)));
        m_listemodifiee = true;
        ReconstruitTreeViewManufacturers();
    }
}

void dlg_listemanufacturers::ReconstruitTreeViewManufacturers(bool reconstruirelaliste, QString filtre)
{
    if (reconstruirelaliste)
        Datas::I()->manufacturers->initListe();
    wdg_itemstree->disconnect();
    wdg_itemstree->selectionModel()->disconnect();
    if (m_model == Q_NULLPTR)
        delete m_model;
    m_model = new QStandardItemModel(this);

    UpStandardItem *pitem;

    foreach(Manufacturer *man, Datas::I()->manufacturers->manufacturers()->values())
    {
        if (man->nom().startsWith(filtre))
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