/* (C) 2016 LAINE SERGE
This file is part of Rufus.

Rufus is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Rufus is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Rufus. If not, see <http://www.gnu.org/licenses/>.
*/

#include "dlg_listecorrespondants.h"
#include "icons.h"

dlg_listecorrespondants::dlg_listecorrespondants(Procedures *Proc, QSqlQuery Quer, QWidget *parent) :
    UpDialog(parent)
{
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    quer            = Quer;
    proc            = Proc;

    globallayout    = dynamic_cast<QVBoxLayout*>(layout());
    ListeModifiee   = false;


    setModal(true);
    setWindowTitle(tr("Liste des correspondants"));

    treeCor = new QTreeView(this);
    treeCor ->setFixedWidth(320);
    treeCor ->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    treeCor ->setFocusPolicy(Qt::StrongFocus);
    treeCor ->setEditTriggers(QAbstractItemView::NoEditTriggers);
    treeCor ->setAnimated(true);
    treeCor ->setIndentation(10);
    treeCor ->setMouseTracking(true);
    treeCor ->header()->setVisible(false);

    ReconstruitListeCorrespondants();

    widgButtons         = new WidgetButtonFrame(treeCor);
    widgButtons         ->AddButtons(WidgetButtonFrame::PlusButton | WidgetButtonFrame::ModifButton | WidgetButtonFrame::MoinsButton);

    label               = new UpLabel();
    label               ->setFixedSize(21,21);
    label               ->setPixmap(Icons::pxLoupe().scaled(20,20)); //WARNING : icon scaled : pxLoupe 20,20
    ChercheUplineEdit   = new UpLineEdit();
    ChercheUplineEdit   ->setFixedSize(140,25);
    ChercheUplineEdit   ->setStyleSheet(
    "UpLineEdit {background-color:white; border: 1px solid rgb(150,150,150);border-radius: 10px;}"
    "UpLineEdit:focus {border: 2px solid rgb(164, 205, 255);border-radius: 10px;}");
    widgButtons->layButtons()->insertWidget(0,label);
    widgButtons->layButtons()->insertWidget(0,ChercheUplineEdit);
    AjouteLayButtons(UpDialog::ButtonOK);

    globallayout->insertWidget(0,widgButtons->widgButtonParent());

    connect(treeCor,            SIGNAL(pressed(QModelIndex)),           this,   SLOT(Slot_Enablebuttons()));
    connect(treeCor,            SIGNAL(doubleClicked(QModelIndex)),     this,   SLOT(Slot_Reponse()));
    connect(OKButton,           SIGNAL(clicked(bool)),                  this,   SLOT(reject()));
    connect(ChercheUplineEdit,  SIGNAL(textEdited(QString)),            this,   SLOT(Slot_ChercheCor(QString)));
    connect(widgButtons,        SIGNAL(choix(int)),                     this,   SLOT(Slot_ChoixButtonFrame(int)));

    widgButtons->modifBouton    ->setEnabled(false);
    widgButtons->moinsBouton    ->setEnabled(false);
    ChercheUplineEdit           ->setFocus();
}

dlg_listecorrespondants::~dlg_listecorrespondants()
{
}

void dlg_listecorrespondants::Slot_ChercheCor(QString txt)
{
    for (int i=0; i<gmodele->rowCount(); i++)
        for (int k=0; k<gmodele->item(i)->rowCount(); k++)
        {
            QString text = gmodele->item(i)->child(k)->text();
            if( text.startsWith(txt) )
            {
                treeCor->scrollTo(gmodele->item(i)->child(k)->index(), QAbstractItemView::PositionAtCenter);
                treeCor->selectionModel()->select(gmodele->item(i)->child(k)->index(),QItemSelectionModel::Select);
                Slot_Enablebuttons();
                return;
            }
        }
}

void dlg_listecorrespondants::Slot_Enablebuttons()
{
    widgButtons->modifBouton->setEnabled(gmodele->itemFromIndex(treeCor->selectionModel()->selectedIndexes().at(0))->accessibleDescription() != "");
    widgButtons->moinsBouton->setEnabled(gmodele->itemFromIndex(treeCor->selectionModel()->selectedIndexes().at(0))->accessibleDescription() != "");
}


void dlg_listecorrespondants::Slot_Reponse()
{
    ModifCorresp();
}

void dlg_listecorrespondants::Slot_ChoixButtonFrame(int i)
{
    switch (i) {
    case 1:
        EnregistreNouveauCorresp();
        break;
    case 0:
        ModifCorresp();
        break;
    case -1:
        SupprCorresp();
        break;
    default:
        break;
    }
}

// ------------------------------------------------------------------------------------------
// Enregistre un nouveau correpondant
// ------------------------------------------------------------------------------------------
void dlg_listecorrespondants::EnregistreNouveauCorresp()
{
    bool onlydoctors    = false;
    Dlg_IdentCorresp    = new dlg_identificationcorresp("Creation", onlydoctors, 0, proc, this);
    if (Dlg_IdentCorresp->exec()>0)
    {
        ListeModifiee         = true;
        ReconstruitListeCorrespondants(true);
    }
    delete Dlg_IdentCorresp;
}

// ------------------------------------------------------------------------------------------
// Modifie un correpondant
// ------------------------------------------------------------------------------------------
void dlg_listecorrespondants::ModifCorresp()
{
    bool onlydoctors    = false;
    int idCor           = gmodele->itemFromIndex(treeCor->selectionModel()->selectedIndexes().at(0))->accessibleDescription().toInt();
    Dlg_IdentCorresp    = new dlg_identificationcorresp("Modification", onlydoctors, idCor, proc, this);
    if (Dlg_IdentCorresp->exec()>0)
    {
        ListeModifiee         = true;
        ReconstruitListeCorrespondants(true);
    }
    delete Dlg_IdentCorresp;
}

// ------------------------------------------------------------------------------------------
// Supprime un correpondant
// ------------------------------------------------------------------------------------------
void dlg_listecorrespondants::SupprCorresp()
{
    if (treeCor->selectionModel()->selectedIndexes().size() == 0) return;
    QString Msg;
    Msg = tr("Etes vous sûr de vouloir supprimer la fiche") + "\n " +
            gmodele->itemFromIndex(treeCor->selectionModel()->selectedIndexes().at(0))->text() + "?" +
            "\n" + tr("La suppression de cette fiche est IRRÉVERSIBLE.");
    UpMessageBox msgbox;
    msgbox.setText("Euuhh... " + proc->getUserConnected()->getLogin() + "?");
    msgbox.setInformativeText(Msg);
    msgbox.setIcon(UpMessageBox::Warning);
    UpSmallButton *NoBouton = new UpSmallButton();
    NoBouton->setText(tr("Annuler"));
    UpSmallButton *OKBouton = new UpSmallButton();
    OKBouton->setText(tr("Supprimer la fiche"));
    msgbox.addButton(NoBouton, UpSmallButton::CANCELBUTTON);
    msgbox.addButton(OKBouton, UpSmallButton::SUPPRBUTTON);
    msgbox.exec();
    if (msgbox.clickedButton() == OKBouton)
    {
        QString idCor   = gmodele->itemFromIndex(treeCor->selectionModel()->selectedIndexes().at(0))->accessibleDescription();
        QString req     = "delete from " NOM_TABLE_CORRESPONDANTS " where idcor = " + idCor;
        QSqlQuery       (req, DataBase::getInstance()->getDataBase());
        QSqlQuery       ("update " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS " set idcormedmg  = null where idcormedmg  = " + idCor, DataBase::getInstance()->getDataBase());
        QSqlQuery       ("update " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS " set idcormespe1 = null where idcormespe1 = " + idCor, DataBase::getInstance()->getDataBase());
        QSqlQuery       ("update " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS " set idcormespe2 = null where idcormespe2 = " + idCor, DataBase::getInstance()->getDataBase());
        QSqlQuery       ("update " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS " set idcormespe3 = null where idcormespe3 = " + idCor, DataBase::getInstance()->getDataBase());
        ListeModifiee   = true;
        ReconstruitListeCorrespondants(true);
    }
}

bool dlg_listecorrespondants::getListeModifiee()
{
    return ListeModifiee;
}

void dlg_listecorrespondants::ReconstruitListeCorrespondants(bool AvecRecalcul)
{
    QString             idimpraretrouver = "";
    if (AvecRecalcul)
    {
        QString req = "SELECT idCor, CorNom, CorPrenom, nomspecialite as metier, CorAdresse1, CorAdresse2, CorAdresse3, CorCodepostal, CorVille, CorTelephone FROM " NOM_TABLE_CORRESPONDANTS ", " NOM_TABLE_SPECIALITES
                " where cormedecin = 1 and corspecialite = idspecialite"
                " union"
                " SELECT idCor, CorNom, CorPrenom, corautreprofession as metier, CorAdresse1, CorAdresse2, CorAdresse3, CorCodepostal, CorVille, CorTelephone FROM " NOM_TABLE_CORRESPONDANTS
                " where cormedecin <> 1 or cormedecin is null"
                " order by metier, cornom, corprenom";
        QSqlQuery Recalcquer(req,DataBase::getInstance()->getDataBase());
        quer = Recalcquer;
        if (treeCor->selectionModel()->selectedIndexes().size()>0)
        {
            QModelIndex         actifidx;
            actifidx = treeCor->selectionModel()->selectedIndexes().at(0);
            if (!gmodele->itemFromIndex(actifidx)->hasChildren())
                idimpraretrouver = gmodele->itemFromIndex(actifidx)->accessibleDescription();
        }
        gmodele->clear();
    }
    else
        gmodele = new QStandardItemModel;
    quer    .first();
    if (quer.size() > 0)
    {
        QStandardItem *pitem, *metieritem;
        QString metier  = "";
        quer            .first();
        for (int i=0; i<quer.size(); i++)
        {
            if (proc->MajusculePremiereLettre(quer.value(3).toString(), true, false) != metier)
            {
                metier      = proc->MajusculePremiereLettre(quer.value(3).toString(), true, false);
                metieritem  = new QStandardItem();
                metieritem  ->setText(metier);
                metieritem  ->setForeground(QBrush(QColor(Qt::red)));
                metieritem  ->setEditable(false);
                metieritem  ->setEnabled(false);
                gmodele     ->appendRow(metieritem);
            }
            quer.next();
        }
        quer.first();
        for (int i=0; i<quer.size(); i++)
        {
            pitem   = new QStandardItem(quer.value(1).toString() + " " + quer.value(2).toString());
            pitem   ->setAccessibleDescription(quer.value(0).toString());
            QString ttip;
            if (quer.value(4).toString() != "") ttip += quer.value(4).toString();
            if (quer.value(5).toString() != "")
            {
                if (ttip != "") ttip += "\n";
                ttip += quer.value(5).toString();
            }
            if (quer.value(6).toString() != "")
            {
                if (ttip != "") ttip += "\n";
                ttip += quer.value(6).toString();
            }
            if (quer.value(7).toString() + quer.value(8).toString() != "")
            {
                if (ttip != "") ttip += "\n";
                ttip += quer.value(7).toString();
                if (quer.value(7).toString() + quer.value(8).toString() != "")
                    ttip += " ";
                ttip += quer.value(8).toString();
            }
            if (quer.value(9).toString() != "")
            {
                if (ttip != "") ttip += "\n";
                ttip += quer.value(9).toString();
            }
            pitem   ->setData(ttip, Qt::ToolTipRole);
            pitem   ->setEditable(false);
            QList<QStandardItem *> listitems = gmodele->findItems(proc->MajusculePremiereLettre(quer.value(3).toString(), true, false));
            if (listitems.size()>0)
                listitems.at(0)->appendRow(pitem);
            quer.next();
        }
    }
    treeCor->setModel(gmodele);
    treeCor->expandAll();
    if (gmodele->rowCount()>0)
    {
        if (idimpraretrouver != "")
            for (int m = 0; m<gmodele->rowCount(); m++)
            {
                for (int n=0; n<gmodele->item(m)->rowCount(); n++)
                    if (gmodele->item(m)->child(n)->accessibleDescription() == idimpraretrouver)
                    {
                        treeCor->scrollTo(gmodele->item(m)->child(n)->index(), QAbstractItemView::PositionAtCenter);
                        treeCor->setCurrentIndex(gmodele->item(m)->child(n)->index());
                        return;
                    }
            }
    }
}
