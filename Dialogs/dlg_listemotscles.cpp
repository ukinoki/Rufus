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

#include "dlg_listemotscles.h"

dlg_listemotscles::dlg_listemotscles(Procedures *Proc, int idPat, QWidget *parent) :
    UpDialog(QDir::homePath() + NOMFIC_INI, "PositionsFiches/PositionMotsCles", parent)
{
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::WindowMinMaxButtonsHint);
    idpat              = idPat;
    proc               = Proc;

    QVBoxLayout *globallay       = dynamic_cast<QVBoxLayout*>(layout());
    tabMC              = new QTableView();


    RemplirTableView();
    tabMC->verticalHeader()->setVisible(false);
    tabMC->setFocusPolicy(Qt::StrongFocus);
    tabMC->setSelectionMode(QAbstractItemView::SingleSelection);
    tabMC->setGridStyle(Qt::NoPen);
    tabMC->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tabMC->setColumnWidth(0,300);
    tabMC->setFixedWidth(tabMC->columnWidth(0)+2);
    QFontMetrics fm(qApp->font());
    int hauteurligne = int(fm.height()*1.1);
    tabMC->setMinimumHeight(hauteurligne*20);
    tabMC->setSizeIncrement(0,hauteurligne);
    tabMC->setMouseTracking(true);
    widgButtons = new WidgetButtonFrame(tabMC);
    widgButtons->AddButtons(WidgetButtonFrame::PlusButton | WidgetButtonFrame::ModifButton | WidgetButtonFrame::MoinsButton);

    globallay->insertWidget(0,widgButtons->widgButtonParent());

    AjouteLayButtons();
    setFixedWidth(tabMC->width() + globallay->contentsMargins().left()*2);
    setModal(true);
    setSizeGripEnabled(false);
    setWindowTitle(tr("Liste des mots-clés"));

    glistidMCdepart << "-1";

    connect(tabMC,          SIGNAL(pressed(QModelIndex)),   this,   SLOT(Slot_Enablebuttons()));
    connect(OKButton,       SIGNAL(clicked(bool)),          this,   SLOT(Slot_OK()));
    connect(widgButtons,    SIGNAL(choix(int)),             this,   SLOT(Slot_ChoixButtonFrame(int)));
    widgButtons->modifBouton->setEnabled(false);
    widgButtons->moinsBouton->setEnabled(false);
}

dlg_listemotscles::~dlg_listemotscles()
{
}

void dlg_listemotscles::Slot_ChoixButtonFrame(int i)
{
    switch (i) {
    case 1:
        CreationModifMC(Creation);
        break;
    case 0:
        CreationModifMC(Modif);
        break;
    case -1:
        SupprMC();
        break;
    default:
        break;
    }
}

void dlg_listemotscles::CreationModifMC(enum gMode mode)
{
    gAskDialog                      = new UpDialog(this);
    QVBoxLayout *globallay          = dynamic_cast<QVBoxLayout*>(gAskDialog->layout());
    QWidget     *widg               = new QWidget(gAskDialog);
    UpLineEdit  *Line               = new UpLineEdit(gAskDialog);
    QCompleter  *MCListCompleter    = new QCompleter(glistMC);

    widg            ->setLayout(new QVBoxLayout);
    widg->layout()  ->setContentsMargins(0,10,0,0);
    widg->layout()  ->addWidget(Line);
    Line            ->setFixedSize(300,21);
    globallay       ->insertWidget(0,widg);
    globallay       ->setSizeConstraint(QLayout::SetFixedSize);

    gAskDialog      ->AjouteLayButtons();
    gAskDialog      ->setWindowTitle(tr("Entrez un nouveau mot-clé"));

    connect(gAskDialog->OKButton,       SIGNAL(clicked(bool)),this,SLOT(Slot_VerifMC()));

    Line            ->setMaxLength(60);
    MCListCompleter ->setCaseSensitivity(Qt::CaseInsensitive);
    MCListCompleter ->setCompletionMode(QCompleter::InlineCompletion);
    Line            ->setCompleter(MCListCompleter);

    if (mode == Modif)
    {
        Line        ->setText(gmodele->itemFromIndex(gselection->currentIndex())->text());
        gAskDialog  ->setMode("Modif");
    }
    else
        gAskDialog  ->setMode("Creation");
    gAskDialog->exec();
    delete gAskDialog;
}

void dlg_listemotscles::Slot_VerifMC()
{
    QString nouvMC= gAskDialog->findChildren<UpLineEdit*>().at(0)->text();
    if (nouvMC == "")
    {
        proc->Message(tr("Vous n'avez pas rempli le mot clé"),2000,false);
        return;
    }
    if (glistMC.contains(nouvMC, Qt::CaseInsensitive))
    {
        proc->Message(tr("Ce mot-clé existe déjà"),2000,false);
        if (gmodele->findItems(nouvMC, Qt::MatchExactly).size()>0)
            tabMC->scrollTo(gmodele->findItems(nouvMC, Qt::MatchExactly).at(0)->index());
        return;
    }
    if (gAskDialog->mode()=="Creation")
    {
        QString req = "insert into " NOM_TABLE_MOTSCLES " (MotCle) values('" + proc->CorrigeApostrophe(nouvMC) + "')";
        QSqlQuery (req, DataBase::getInstance()->getDataBase());
    }
    else if (gAskDialog->mode()=="Modif")
    {
        QString req = "update " NOM_TABLE_MOTSCLES " set MotCle = '" + proc->CorrigeApostrophe(nouvMC) + "' where MotCle = '"
                   + gmodele->itemFromIndex(gselection->currentIndex())->text() + "'";
        QSqlQuery (req, DataBase::getInstance()->getDataBase());
    }
    QSqlQuery ("delete from " NOM_TABLE_MOTSCLESJOINTURES " where idpat = " + QString::number(idpat), DataBase::getInstance()->getDataBase());
    QStringList listidMc;
    for (int i=0; i< gmodele->rowCount(); i++)
        if(gmodele->item(i,0)->checkState() == Qt::Checked)
            listidMc << gmodele->item(i,0)->accessibleDescription();
    if (listidMc.size()>0)
    {
        QString req = "insert into " NOM_TABLE_MOTSCLESJOINTURES " (idpat, idmotcle) values ";
        req += "(" + QString::number(idpat) + ", " + listidMc.at(0) + ")";
        for (int j=1; j<listidMc.size(); j++)
            req += ", (" + QString::number(idpat) + ", " + listidMc.at(j) + ")";
        QSqlQuery (req,DataBase::getInstance()->getDataBase());
    }
    RemplirTableView();
    gAskDialog->accept();
}

void dlg_listemotscles::SupprMC()
{
    UpMessageBox msgbox;
    msgbox.setText(tr("Suppression de mot clé!"));
    msgbox.setInformativeText(tr("Voulez vous vraiment supprimer le mot-clé\n") + gmodele->itemFromIndex(gselection->currentIndex())->text());
    msgbox.setIcon(UpMessageBox::Warning);
    UpSmallButton AnnulBouton(tr("Annuler"));
    UpSmallButton OKBouton(tr("Supprimer le mot-clé"));
    msgbox.addButton(&AnnulBouton, UpSmallButton::CANCELBUTTON);
    msgbox.addButton(&OKBouton, UpSmallButton::SUPPRBUTTON);
    //msgbox.setDefaultButton(AnnulBouton);
    msgbox.exec();
    if (msgbox.clickedButton() == &OKBouton)
    {
        QString req = "delete from " NOM_TABLE_MOTSCLES " where idmotcle = " + gmodele->itemFromIndex(gselection->currentIndex())->accessibleDescription();
        QSqlQuery (req, DataBase::getInstance()->getDataBase());
        RemplirTableView();
    }
}

void dlg_listemotscles::Slot_Enablebuttons()
{
    widgButtons->modifBouton->setEnabled(gselection->hasSelection());
    widgButtons->moinsBouton->setEnabled(gselection->hasSelection());
}

void dlg_listemotscles::Slot_OK()
{
    QSqlQuery ("delete from " NOM_TABLE_MOTSCLESJOINTURES " where idpat = " + QString::number(idpat), DataBase::getInstance()->getDataBase());
    QStringList listidMc;
    for (int i=0; i< gmodele->rowCount(); i++)
        if(gmodele->item(i,0)->checkState() == Qt::Checked)
            listidMc << gmodele->item(i,0)->accessibleDescription();
    if (listidMc.size()>0)
    {
        QString req = "insert into " NOM_TABLE_MOTSCLESJOINTURES " (idpat, idmotcle) values ";
        req += "(" + QString::number(idpat) + ", " + listidMc.at(0) + ")";
        for (int j=1; j<listidMc.size(); j++)
            req += ", (" + QString::number(idpat) + ", " + listidMc.at(j) + ")";
        QSqlQuery (req,DataBase::getInstance()->getDataBase());
    }
    accept();
}

void dlg_listemotscles::DisableLines()
{
    gselection->clearSelection();
}

QStringList dlg_listemotscles::listMCDepart()
{
    return glistidMCdepart;
}

void dlg_listemotscles::RemplirTableView()
{
     QString req = "select idMotcle from " NOM_TABLE_MOTSCLESJOINTURES " where idpat = " + QString::number(idpat);
     QSqlQuery querMC(req,DataBase::getInstance()->getDataBase());
     QStringList listidMC;
     bool a = glistidMCdepart.contains("-1");
     glistidMCdepart.clear();
     if (querMC.size()>0)
     {
         querMC.first();
         for (int i=0; i<querMC.size(); i++)
         {
             listidMC << querMC.value(0).toString();
             if (a)
                 glistidMCdepart << querMC.value(0).toString();
             querMC.next();
         }
     }
     req = "select idmotcle, motcle from " NOM_TABLE_MOTSCLES " order by motcle";
     QSqlQuery quer(req,DataBase::getInstance()->getDataBase());

     quer.first();

     QStandardItem       *pitem;
     gmodele = dynamic_cast<QStandardItemModel*>(tabMC->model());
     if (gmodele)
         gmodele->clear();
     else
         gmodele = new QStandardItemModel;
     gselection = new QItemSelectionModel(gmodele);

     glistMC.clear();
     pitem   = new QStandardItem(tr("Mot-clé"));
     pitem->setEditable(false);
     gmodele->setHorizontalHeaderItem(0,pitem);

     for (int i=0; i<quer.size(); i++)
     {
         pitem   = new QStandardItem(quer.value(1).toString());
         pitem->setAccessibleDescription(quer.value(0).toString());
         //pitem->setEditable(false);
         pitem->setCheckable(true);
         if (listidMC.contains(quer.value(0).toString()))
             pitem->setCheckState(Qt::Checked);
         else
             pitem->setCheckState(Qt::Unchecked);
         gmodele->appendRow(pitem);
         glistMC << quer.value(1).toString();
         quer.next();
     }
     tabMC->setModel(gmodele);
     tabMC->setSelectionModel(gselection);

     QFontMetrics fm(qApp->font());
     int hauteurligne = int(fm.height()*1.1);
     for (int i=0; i<gmodele->rowCount(); i++)
         tabMC->setRowHeight(i,hauteurligne);
     tabMC->horizontalHeader()->setFixedHeight(hauteurligne);
     tabMC->setColumnWidth(0,300);
}
