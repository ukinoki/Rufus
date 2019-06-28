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

    proc            = Procedures::I();
    db              = DataBase::I();
    gmodele         = new QStandardItemModel(this);

    ListeCorModifiee   = false;

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

    ReconstruitTreeViewCorrespondants(true);

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

    dlglayout()->insertWidget(0,widgButtons->widgButtonParent());

    connect(OKButton,           SIGNAL(clicked(bool)),                  this,   SLOT(reject()));
    connect(ChercheUplineEdit,  &QLineEdit::textEdited,                 this,   [=] (QString txt) { txt = Utils::trimcapitilize(txt, false, true);
                                                                                                    ChercheUplineEdit->setText(txt);
                                                                                                    ReconstruitTreeViewCorrespondants(false, txt);});
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
    switch (i) {
    case 1:
        EnregistreNouveauCorresp();
        break;
    case 0:
        if (treeCor->selectionModel()->selectedIndexes().size()==0)
            return;
        ModifCorresp(getCorrespondantFromIndex(treeCor->selectionModel()->selectedIndexes().at(0)));
        break;
    case -1:
        SupprCorresp();
        break;
    default:
        break;
    }
}

bool dlg_listecorrespondants::listecorrespondantsmodifiee()
{
    return ListeCorModifiee;
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
        ListeCorModifiee = true;
        ReconstruitTreeViewCorrespondants();
    }
    delete Dlg_IdentCorresp;
}

// ------------------------------------------------------------------------------------------
// renvoie le correpondant correspodant à l'index
// ------------------------------------------------------------------------------------------
Correspondant* dlg_listecorrespondants::getCorrespondantFromIndex(QModelIndex idx )
{
    return Datas::I()->correspondants->getById(gmodele->itemFromIndex(idx)->data().toMap().value("id").toInt());
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
        ListeCorModifiee = true;
        ReconstruitTreeViewCorrespondants(true);
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
    msgbox.setText("Euuhh... " + db->getUserConnected()->getLogin() + "?");
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
        db->SupprCorrespondant(idCor);
        ListeCorModifiee = true;
        ReconstruitTreeViewCorrespondants(true);
    }
}

QList<QStandardItem*> dlg_listecorrespondants::ListeMetiers()
{
    QList<QStandardItem*> listmetiers;
    QStringList list;
    QStandardItem *metieritem;
    QString metier  = "";
    QMapIterator<int, Correspondant *> itcor(*Datas::I()->correspondants->correspondants());
    while (itcor.hasNext()) {
        Correspondant *cor = const_cast<Correspondant*>(itcor.next().value());
        QString metier  = Utils::trimcapitilize(cor->metier(), true, false);
        if (!list.contains(metier))
        {
            list << metier;
            metieritem  = new QStandardItem(metier);
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
    treeCor->disconnect();
    gmodele->clear();

    QStandardItem *pitem;
    QList<QStandardItem*> listmetiers = ListeMetiers();
    for (int i=0; i<listmetiers.size(); i++)
    {
        gmodele->appendRow(listmetiers.at(i));
        //qDebug() << gmodele->item(i)->text();
    }

    QMapIterator<int, Correspondant *> itcor(*Datas::I()->correspondants->correspondants());
    while (itcor.hasNext()) {
        Correspondant *cor = const_cast<Correspondant*>(itcor.next().value());
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
                                                                            {
                                                                                if (!gmodele->itemFromIndex(idx)->hasChildren())
                                                                                    QToolTip::showText(cursor().pos(), getCorrespondantFromIndex(idx)->adresseComplete());
                                                                            } );
        connect(treeCor,    &QAbstractItemView::pressed,        this,   &dlg_listecorrespondants::Enablebuttons);
        connect(treeCor,    &QAbstractItemView::doubleClicked,  this,   [=] (QModelIndex idx)
                                                                            {
                                                                                if (!gmodele->itemFromIndex(idx)->hasChildren())
                                                                                    ModifCorresp(getCorrespondantFromIndex(idx));
                                                                            });
    }
}
