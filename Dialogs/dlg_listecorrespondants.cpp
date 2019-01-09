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

#include "gbl_datas.h"
#include "dlg_listecorrespondants.h"
#include "icons.h"

dlg_listecorrespondants::dlg_listecorrespondants(QWidget *parent) :
    UpDialog(parent)
{
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::WindowMinMaxButtonsHint);

    proc            = Procedures::I();
    globallayout    = dynamic_cast<QVBoxLayout*>(layout());
    gmodele         = new QStandardItemModel;
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

    proc->initListeCorrespondantsAll();
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

    connect(OKButton,           SIGNAL(clicked(bool)),                  this,   SLOT(reject()));
    connect(ChercheUplineEdit,  &QLineEdit::textEdited,                 this,   [=] (QString txt) { txt = Utils::trimcapitilize(txt, true, false);
                                                                                                    ChercheUplineEdit->setText(txt);
                                                                                                    ReconstruitListeCorrespondants(txt);});
    connect(widgButtons,        &WidgetButtonFrame::choix,              this,   [=] (int i) {ChoixButtonFrame(i);});

    widgButtons->modifBouton    ->setEnabled(false);
    widgButtons->moinsBouton    ->setEnabled(false);
    ChercheUplineEdit           ->setFocus();
}

dlg_listecorrespondants::~dlg_listecorrespondants()
{
}

void dlg_listecorrespondants::Enablebuttons()
{
    widgButtons->modifBouton->setEnabled(gmodele->itemFromIndex(treeCor->selectionModel()->selectedIndexes().at(0))->data().toMap()["id"].toInt()>0);
    widgButtons->moinsBouton->setEnabled(gmodele->itemFromIndex(treeCor->selectionModel()->selectedIndexes().at(0))->data().toMap()["id"].toInt()>0);
}


void dlg_listecorrespondants::ChoixButtonFrame(int i)
{
    int idCor;
    switch (i) {
    case 1:
        EnregistreNouveauCorresp();
        break;
    case 0:
        if (treeCor->selectionModel()->selectedIndexes().size()==0)
            return;
        idCor = gmodele->itemFromIndex(treeCor->selectionModel()->selectedIndexes().at(0))->data().toMap()["id"].toInt();
        ModifCorresp(idCor);
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
    Dlg_IdentCorresp    = new dlg_identificationcorresp("Creation", onlydoctors, 0);
    if (Dlg_IdentCorresp->exec()>0)
    {
        ListeModifiee = true;
        proc->initListeCorrespondantsAll();
        ReconstruitListeCorrespondants();
    }
    delete Dlg_IdentCorresp;
}

// ------------------------------------------------------------------------------------------
// Modifie un correpondant
// ------------------------------------------------------------------------------------------
void dlg_listecorrespondants::ModifCorresp(int idcor)
{
    bool onlydoctors    = false;
    Dlg_IdentCorresp    = new dlg_identificationcorresp("Modification", onlydoctors, idcor);
    if (Dlg_IdentCorresp->exec()>0)
    {
        ListeModifiee = true;
        proc->initListeCorrespondantsAll();
        ReconstruitListeCorrespondants();
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
    UpSmallButton NoBouton(tr("Annuler"));
    UpSmallButton OKBouton(tr("Supprimer la fiche"));
    msgbox.addButton(&NoBouton, UpSmallButton::CANCELBUTTON);
    msgbox.addButton(&OKBouton, UpSmallButton::SUPPRBUTTON);
    msgbox.exec();
    if (msgbox.clickedButton() == &OKBouton)
    {
        int idCor   = gmodele->itemFromIndex(treeCor->selectionModel()->selectedIndexes().at(0))->data().toMap()["id"].toInt();
        DataBase::getInstance()->SupprCorrespondant(idCor);
        ListeModifiee = true;
        Datas::I()->correspondants->removeCorrespondant(Datas::I()->correspondants->getCorrespondantById(idCor));
        ReconstruitListeCorrespondants();
    }
}

bool dlg_listecorrespondants::getListeModifiee()
{
    return ListeModifiee;
}

QList<QStandardItem*> dlg_listecorrespondants::ListeMetiers()
{
    QList<QStandardItem*> listmetiers;
    QStringList list;
    QMap<int, Correspondant *> *Correspondants = Datas::I()->correspondants->getCorrespondants();
    QMap<int, Correspondant *>::const_iterator itcorrespondants;
    QStandardItem *metieritem;
    QString metier  = "";
    for( itcorrespondants = Correspondants->constBegin(); itcorrespondants != Correspondants->constEnd(); ++itcorrespondants )
    {
        Correspondant *cor = const_cast<Correspondant*>(*itcorrespondants);
        QString metier  = Utils::trimcapitilize(cor->metier(), true, false);
        if (!list.contains(metier))
        {
            list << metier;
            metieritem  = new QStandardItem();
            metieritem  ->setText(metier);
            metieritem  ->setForeground(QBrush(QColor(Qt::red)));
            metieritem  ->setEditable(false);
            metieritem  ->setEnabled(false);
            listmetiers << metieritem;
        }
    }
    return listmetiers;
}

void dlg_listecorrespondants::ReconstruitListeCorrespondants(QString filtre)
{
    treeCor->disconnect();
    gmodele->clear();

    QMap<int, Correspondant *> *Correspondants = Datas::I()->correspondants->getCorrespondants();
    QStandardItem *pitem;

    QList<QStandardItem*> listmetiers = ListeMetiers();
    for (int i=0; i<listmetiers.size(); i++)
        gmodele->appendRow(listmetiers.at(i));

    QMap<int, Correspondant *>::const_iterator itcor;
    for( itcor = Correspondants->constBegin(); itcor != Correspondants->constEnd(); ++itcor )
    {
        Correspondant *cor = const_cast<Correspondant*>(*itcor);
        if (cor->nomprenom().startsWith(filtre))
        {
            pitem   = new QStandardItem(cor->nomprenom());
            QMap<QString, QVariant> data;
            data["adr"] = cor->adresseComplete();
            data["id"]  = cor->id();
            pitem   ->setData(data);
            pitem   ->setEditable(false);
            QList<QStandardItem *> listitems = gmodele->findItems(Utils::trimcapitilize(cor->metier(), true, false));
            if (listitems.size()>0)
                listitems.at(0)->appendRow(pitem);
        }
    }
    for (int i=0; i<gmodele->rowCount();i++)
        if (!gmodele->item(i)->hasChildren())
        {
            gmodele->removeRow(i);
            i--;
        }
    treeCor     ->setModel(gmodele);
    treeCor     ->expandAll();
    if (gmodele->rowCount()>0)
    {
        gmodele->sort(0);
        gmodele->sort(1);
        connect(treeCor,    &QAbstractItemView::entered,        this,   [=] (QModelIndex idx)
                                                                            {QToolTip::showText(cursor().pos(),gmodele->itemFromIndex(idx)->data().toMap().value("adr").toString());} );
        connect(treeCor,    &QAbstractItemView::pressed,        this,   &dlg_listecorrespondants::Enablebuttons);
        connect(treeCor,    &QAbstractItemView::doubleClicked,  this,   [=] (QModelIndex idx)
                                                                            {
                                                                                int id = gmodele->itemFromIndex(idx)->data().toMap().value("id").toInt();
                                                                                if (id != -1)
                                                                                    ModifCorresp(id);
                                                                            });
    }
}
