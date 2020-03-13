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

    m_model         = new QStandardItemModel(this);

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
}

dlg_listeiols::~dlg_listeiols()
{
}

void dlg_listeiols::Enablebuttons()
{
    wdg_buttonframe->wdg_modifBouton->setEnabled(getIOLFromIndex(wdg_iolstree->selectionModel()->selectedIndexes().at(0)) != Q_NULLPTR);
    wdg_buttonframe->wdg_moinsBouton->setEnabled(getIOLFromIndex(wdg_iolstree->selectionModel()->selectedIndexes().at(0)) != Q_NULLPTR);
}


void dlg_listeiols::ChoixButtonFrame()
{
    switch (wdg_buttonframe->Choix()) {
    case WidgetButtonFrame::Plus:
        EnregistreNouveauIOL();
        break;
    case WidgetButtonFrame::Modifier:
        if (wdg_iolstree->selectionModel()->selectedIndexes().size()==0)
            return;
        ModifIOL(getIOLFromIndex(wdg_iolstree->selectionModel()->selectedIndexes().at(0)));
        break;
    case WidgetButtonFrame::Moins:
        SupprIOL();
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
}

// ------------------------------------------------------------------------------------------
// renvoie le correpondant correspodant à l'index
// ------------------------------------------------------------------------------------------
IOL* dlg_listeiols::getIOLFromIndex(QModelIndex idx )
{
    UpStandardItem *it = dynamic_cast<UpStandardItem*>(m_model->itemFromIndex(idx));
    if (it != Q_NULLPTR)
        return dynamic_cast<IOL *>(it->item());
    else
        return Q_NULLPTR;
}


// ------------------------------------------------------------------------------------------
// Modifie un correpondant
// ------------------------------------------------------------------------------------------
void dlg_listeiols::ModifIOL(IOL *iol)
{
    if (iol == Q_NULLPTR)
        return;
}

// ------------------------------------------------------------------------------------------
// Supprime un correpondant
// ------------------------------------------------------------------------------------------
void dlg_listeiols::SupprIOL()
{
    if (wdg_iolstree->selectionModel()->selectedIndexes().size() == 0) return;
    QString Msg;
    Msg = tr("Etes vous sûr de vouloir supprimer la fiche") + "\n " +
            m_model->itemFromIndex(wdg_iolstree->selectionModel()->selectedIndexes().at(0))->text() + "?" +
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

QList<UpStandardItem*> dlg_listeiols::ListeIOLs()
{
    QList<UpStandardItem*> listmanufacturers;
    QStringList list;
    UpStandardItem *manufactureritem;
    QString metier  = "";
    foreach(IOL *iol, Datas::I()->iols->alls()->values())
    {
        Manufacturer *man = Datas::I()->manufacturers->getById(iol->idmanufacturer());
        if (man != Q_NULLPTR)
        {
            QString fabricant  = Utils::trimcapitilize(man->nom(), true, false);
            if (!list.contains(fabricant))
            {
                list << fabricant;
                manufactureritem  = new UpStandardItem(fabricant);
                manufactureritem  ->setForeground(QBrush(QColor(Qt::red)));
                manufactureritem  ->setEditable(false);
                manufactureritem  ->setEnabled(false);
                listmanufacturers << manufactureritem;
            }
        }
    }
    return listmanufacturers;
}

void dlg_listeiols::ReconstruitTreeViewIOLs(bool reconstruirelaliste, QString filtre)
{
    if (reconstruirelaliste)
        Datas::I()->iols->initListe();
    wdg_iolstree->disconnect();
    m_model->clear();

    UpStandardItem *pitem;
    foreach(UpStandardItem *item, ListeIOLs())
        m_model->appendRow(item);

    foreach(IOL *iol, Datas::I()->iols->alls()->values())
    {
        if (iol->modele().startsWith(filtre))
        {
            pitem   = new UpStandardItem(iol->modele(), iol);
            pitem   ->setEditable(false);
            Manufacturer *man = Datas::I()->manufacturers->getById(iol->idmanufacturer());
            if (man != Q_NULLPTR)
            {
                QString fabricant  = Utils::trimcapitilize(man->nom(), true, false);
                QList<QStandardItem *> listitems = m_model->findItems(fabricant);
                if (listitems.size()>0)
                    listitems.at(0)->appendRow(pitem);
            }
        }
    }
    for (int i=0; i<m_model->rowCount();i++)
        if (!m_model->item(i)->hasChildren())
        {
            m_model->removeRow(i);
            i--;
        }
    wdg_iolstree     ->setModel(m_model);
    wdg_iolstree     ->expandAll();
    if (m_model->rowCount()>0)
    {
        m_model->sort(0);
        m_model->sort(1);
        connect(wdg_iolstree,    &QAbstractItemView::pressed,       this,   &dlg_listeiols::Enablebuttons);
        connect(wdg_iolstree,    &QAbstractItemView::doubleClicked, this,   [=] (QModelIndex idx) { if (!m_model->itemFromIndex(idx)->hasChildren())
                                                                                                            ModifIOL(getIOLFromIndex(idx)); });
    }
}
