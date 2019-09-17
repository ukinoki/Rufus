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

#include "gbl_datas.h"
#include "dlg_listecorrespondants.h"
#include "icons.h"

dlg_listecorrespondants::dlg_listecorrespondants(QWidget *parent) :
    UpDialog(parent)
{
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::WindowMinMaxButtonsHint);

    m_model         = new QStandardItemModel(this);

    setModal(true);
    setWindowTitle(tr("Liste des correspondants"));

    wdg_correspstree = new QTreeView(this);
    wdg_correspstree ->setFixedWidth(320);
    wdg_correspstree ->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    wdg_correspstree ->setFocusPolicy(Qt::StrongFocus);
    wdg_correspstree ->setEditTriggers(QAbstractItemView::NoEditTriggers);
    wdg_correspstree ->setAnimated(true);
    wdg_correspstree ->setIndentation(10);
    wdg_correspstree ->setMouseTracking(true);
    wdg_correspstree ->header()->setVisible(false);

    ReconstruitTreeViewCorrespondants(true);

    wdg_buttonframe         = new WidgetButtonFrame(wdg_correspstree);
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
                                                                                                    ReconstruitTreeViewCorrespondants(false, txt);});
    connect(wdg_buttonframe,        &WidgetButtonFrame::choix,  this,   &dlg_listecorrespondants::ChoixButtonFrame);

    wdg_buttonframe->wdg_modifBouton    ->setEnabled(false);
    wdg_buttonframe->wdg_moinsBouton    ->setEnabled(false);
    wdg_chercheuplineedit           ->setFocus();
}

dlg_listecorrespondants::~dlg_listecorrespondants()
{
}

void dlg_listecorrespondants::Enablebuttons()
{
    wdg_buttonframe->wdg_modifBouton->setEnabled(getCorrespondantFromIndex(wdg_correspstree->selectionModel()->selectedIndexes().at(0)) != Q_NULLPTR);
    wdg_buttonframe->wdg_moinsBouton->setEnabled(getCorrespondantFromIndex(wdg_correspstree->selectionModel()->selectedIndexes().at(0)) != Q_NULLPTR);
}


void dlg_listecorrespondants::ChoixButtonFrame()
{
    switch (wdg_buttonframe->Choix()) {
    case WidgetButtonFrame::Plus:
        EnregistreNouveauCorresp();
        break;
    case WidgetButtonFrame::Modifier:
        if (wdg_correspstree->selectionModel()->selectedIndexes().size()==0)
            return;
        ModifCorresp(getCorrespondantFromIndex(wdg_correspstree->selectionModel()->selectedIndexes().at(0)));
        break;
    case WidgetButtonFrame::Moins:
        SupprCorresp();
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
    Dlg_IdentCorresp    = new dlg_identificationcorresp(dlg_identificationcorresp::Creation, onlydoctors);
    if (Dlg_IdentCorresp->exec()>0)
    {
        m_listemodifiee = true;
        ReconstruitTreeViewCorrespondants();
    }
    delete Dlg_IdentCorresp;
}

// ------------------------------------------------------------------------------------------
// renvoie le correpondant correspodant à l'index
// ------------------------------------------------------------------------------------------
Correspondant* dlg_listecorrespondants::getCorrespondantFromIndex(QModelIndex idx )
{
    UpStandardItem *it = dynamic_cast<UpStandardItem*>(m_model->itemFromIndex(idx));
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
    Dlg_IdentCorresp    = new dlg_identificationcorresp(dlg_identificationcorresp::Modification, onlydoctors, cor);
    if (Dlg_IdentCorresp->exec()>0)
    {
        m_listemodifiee = true;
        ReconstruitTreeViewCorrespondants(true);
    }
    delete Dlg_IdentCorresp;
}

// ------------------------------------------------------------------------------------------
// Supprime un correpondant
// ------------------------------------------------------------------------------------------
void dlg_listecorrespondants::SupprCorresp()
{
    if (wdg_correspstree->selectionModel()->selectedIndexes().size() == 0) return;
    QString Msg;
    Msg = tr("Etes vous sûr de vouloir supprimer la fiche") + "\n " +
            m_model->itemFromIndex(wdg_correspstree->selectionModel()->selectedIndexes().at(0))->text() + "?" +
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
        Datas::I()->correspondants->SupprimeCorrespondant(getCorrespondantFromIndex(wdg_correspstree->selectionModel()->selectedIndexes().at(0)));
        m_listemodifiee = true;
        ReconstruitTreeViewCorrespondants(true);
    }
}

QList<UpStandardItem*> dlg_listecorrespondants::ListeMetiers()
{
    QList<UpStandardItem*> listmetiers;
    QStringList list;
    UpStandardItem *metieritem;
    QString metier  = "";
    foreach(Correspondant *cor, Datas::I()->correspondants->correspondants()->values())
    {
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
    wdg_correspstree->disconnect();
    m_model->clear();

    UpStandardItem *pitem;
    foreach(UpStandardItem *item, ListeMetiers())
        m_model->appendRow(item);

    foreach(Correspondant *cor, Datas::I()->correspondants->correspondants()->values())
    {
        if (cor->nomprenom().startsWith(filtre))
        {
            pitem   = new UpStandardItem(cor->nomprenom());
            pitem->setitem(cor);
            pitem   ->setEditable(false);
            QList<QStandardItem *> listitems = m_model->findItems(Utils::trimcapitilize(cor->metier(), true, false));
            if (listitems.size()>0)
                listitems.at(0)->appendRow(pitem);
        }
    }
    for (int i=0; i<m_model->rowCount();i++)
        if (!m_model->item(i)->hasChildren())
        {
            m_model->removeRow(i);
            i--;
        }
    wdg_correspstree     ->setModel(m_model);
    wdg_correspstree     ->expandAll();
    if (m_model->rowCount()>0)
    {
        m_model->sort(0);
        m_model->sort(1);
        connect(wdg_correspstree,    &QAbstractItemView::entered,        this,   [=] (QModelIndex idx)
                                                                            {
                                                                                if (!m_model->itemFromIndex(idx)->hasChildren())
                                                                                    QToolTip::showText(cursor().pos(), getCorrespondantFromIndex(idx)->adresseComplete());
                                                                            } );
        connect(wdg_correspstree,    &QAbstractItemView::pressed,        this,   &dlg_listecorrespondants::Enablebuttons);
        connect(wdg_correspstree,    &QAbstractItemView::doubleClicked,  this,   [=] (QModelIndex idx)
                                                                            {
                                                                                if (!m_model->itemFromIndex(idx)->hasChildren())
                                                                                    ModifCorresp(getCorrespondantFromIndex(idx));
                                                                            });
    }
}
