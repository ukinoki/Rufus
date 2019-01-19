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

#include "dlg_listemesures.h"

dlg_listemesures::dlg_listemesures(int *IdPatient, QString mode, QWidget *parent) :
    UpDialog (QDir::homePath() + NOMFIC_INI, "PositionsFiches/PositionListeMes", parent)
{
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    gidPatient  = *IdPatient;
    proc        = Procedures::I();
    if (mode == "SUPPR")
        gMode       = Supprimer;
    else if (mode == "RECUP")
        gMode       = Recuperer;
    db          = DataBase::getInstance();

    QVBoxLayout *globallay       = dynamic_cast<QVBoxLayout*>(layout());
    tabLM              = new QTableView(this);

    globallay->insertWidget(0,tabLM);

    setModal(true);
    setSizeGripEnabled(false);
    setWindowTitle(tr("Liste des mesures"));

    if (gMode == Recuperer)
    {
        AjouteLayButtons(UpDialog::ButtonCancel | UpDialog::ButtonOK);
        OKButton->setText(tr("Reprendre"));
        tabLM->setSelectionMode(QAbstractItemView::NoSelection);
        connect (OKButton,      SIGNAL(clicked()),  this, SLOT (Slot_Validation()) );
    }
    else
    {
        AjouteLayButtons(UpDialog::ButtonCancel | UpDialog::ButtonSuppr);
        SupprButton->setText(tr("Supprimer"));
        tabLM->setSelectionMode(QAbstractItemView::SingleSelection);
        tabLM->setSelectionBehavior(QAbstractItemView::SelectRows);
        connect (SupprButton,   SIGNAL(clicked()),  this, SLOT (Slot_Validation()) );
    }
    setStageCount(1);

    RemplirTableView();
    tabLM->verticalHeader()->setVisible(false);
    tabLM->setFocusPolicy(Qt::StrongFocus);
    tabLM->setGridStyle(Qt::NoPen);
    tabLM->setEditTriggers(QAbstractItemView::NoEditTriggers);

    tabLM->setColumnWidth(0,125);     // Date
    tabLM->setColumnWidth(1,90);      // Mesure
    tabLM->setColumnWidth(2,280);     // Formule OD
    tabLM->setColumnWidth(3,280);     // Formule OG
    tabLM->setColumnHidden(4,true);   // idRefraction
    tabLM->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);

    int larg = 0;
    for (int i=0; i < gmodele->columnCount(); i++)
        if (!tabLM->isColumnHidden(i))
            larg += tabLM->columnWidth(i);
    tabLM->setFixedWidth(larg+2);
    setFixedWidth(tabLM->width()
                        + globallay->contentsMargins().left()*2);
    QFontMetrics fm(qApp->font());
    int hauteurligne = int(fm.height()*1.1);
    tabLM->setMinimumHeight(hauteurligne*20);
    tabLM->setSizeIncrement(0,hauteurligne);
    tabLM->setMouseTracking(true);

    // UTF-8 Encodage pour compatibilité Windows-Mac
    // BUG QT 5 QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    // BUG QT 5 QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
    // BUG QT 5 QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

    dynamic_cast<QVBoxLayout*>(layout())->insertWidget(0,tabLM);
    connect (tabLM,   SIGNAL(clicked(QModelIndex)),  this, SLOT (Slot_Item_Liste_Clicked(QModelIndex)) );
}

dlg_listemesures::~dlg_listemesures()
{
}

//--------------------------------------------------------------------------------------
// Clic sur Bouton Supprimer ou Reprendre
//--------------------------------------------------------------------------------------
void dlg_listemesures::Slot_Validation()

{
    if (gMode == Recuperer)
        accept();
    else if (gMode == Supprimer)
    {
        QString msg;
        if (tabLM->selectionModel()->selectedRows().size()==1)
            msg = tr("Voulez vous supprimer la mesure sélectionnée?");
        else
            msg = tr("Voulez vous supprimer les mesures sélectionnées?");
        int ret = UpMessageBox::Watch(this,tr("Annulation de mesure"), msg, UpDialog::ButtonCancel|UpDialog::ButtonSuppr);
        if (ret == UpSmallButton::SUPPRBUTTON)
        {
            for (int i =0 ; i<tabLM->selectionModel()->selectedRows().size(); i++)
                DetruireLaMesure(gmodele->item(tabLM->selectionModel()->selectedRows().at(i).row(),4)->text().toInt());
            accept();
        }
    }
}

//--------------------------------------------------------------------------------------
// Click sur une ligne de la liste
//--------------------------------------------------------------------------------------
void dlg_listemesures::Slot_Item_Liste_Clicked(QModelIndex mod)
{
    if (gMode == Supprimer)
        SupprButton->setEnabled(tabLM->selectionModel()->selectedIndexes().size()>0);
    else if (gMode ==  Recuperer)
    {
        QStandardItem *pitem = gmodele->itemFromIndex(mod);
        if (pitem->checkState() == Qt::Checked)
        {
            for (int i =0 ; i < gmodele->rowCount(); i++)
            {
                if (gmodele->item(i,0)->checkState() == Qt::Checked && i != pitem->row())
                    gmodele->item(i,0)->setCheckState(Qt::Unchecked);
            }
            itemSelectionne = gmodele->item(pitem->row(),4)->text();
        }
        OKButton->setEnabled(Nombre_Mesure_Selected() > 0);
    }
}

//---------------------------------------------------------------------------------
// Suppression d'une mesure en base
//----------------------------------------------------------------------------------
void dlg_listemesures::DetruireLaMesure(int IdRefract)
{
    bool ok;
    QString requete = "select idPat, quellemesure from " NOM_TABLE_REFRACTION " WHERE  idRefraction = " + QString::number(IdRefract);
    QList<QVariant> mesuredata = db->getFirstRecordFromStandardSelectSQL(requete,ok);
    if (ok && mesuredata.size() > 0)
    {
        requete = "DELETE  FROM " NOM_TABLE_DONNEES_OPHTA_PATIENTS " WHERE  QuelleMesure = '"
                    + mesuredata.at(1).toString() + "' and idpat = " + mesuredata.at(0).toString();
        db->StandardSQL(requete, tr("Impossible de suppimer cette mesure dans donneesophtapatients!"));
    }
    requete = "DELETE  FROM " NOM_TABLE_REFRACTION " WHERE  idRefraction = " + QString::number(IdRefract);
    db->StandardSQL(requete, tr("Impossible de suppimer cette mesure dans refractions!"));
}

QString dlg_listemesures::IdRefractAOuvrir()
{
    return itemSelectionne;
}

// ----------------------------------------------------------------------------------
// Retourne le nombre de lignes comm selectionnees
// ----------------------------------------------------------------------------------
int dlg_listemesures::Nombre_Mesure_Selected()
{
    int nbCommSelected = 0;
    for (int i =0 ; i < gmodele->rowCount(); i++)
        if (gmodele->item(i,0)->checkState() == Qt::Checked)
            nbCommSelected ++;
    return nbCommSelected;
}

//--------------------------------------------------------------------------------------
// Remplir la liste avec les mesures du patient en cours
//--------------------------------------------------------------------------------------
void dlg_listemesures::RemplirTableView()
{
    bool ok;
    QString zw;
    if (gMode == Recuperer)
        OKButton->setEnabled(false);
    else
        SupprButton->setEnabled(false);

    QString requete = "SELECT  idRefraction, DateRefraction, QuelleMesure, FormuleOD, FormuleOG "
              " FROM "  NOM_TABLE_REFRACTION
              " WHERE  IdPat = " + QString::number(gidPatient);
    QList<QList<QVariant>> refractlist = db->StandardSelectSQL(requete, ok, tr("Impossible de trouver la table des refractions!"));
    if(!ok)
        return;

    QStandardItem       *pitem0, *pitem1, *pitem2, *pitem3, *pitem4;
    gmodele = dynamic_cast<QStandardItemModel*>(tabLM->model());
    if (gmodele)
        gmodele->clear();
    else
        gmodele = new QStandardItemModel;

    pitem0  = new QStandardItem(tr("Date"));
    gmodele ->setHorizontalHeaderItem(0,pitem0);
    pitem1  = new QStandardItem(tr("Mesure"));
    gmodele ->setHorizontalHeaderItem(1,pitem1);
    pitem2  = new QStandardItem(tr("Formule OD"));
    gmodele ->setHorizontalHeaderItem(2,pitem2);
    pitem3  = new QStandardItem(tr("Formule OG"));
    gmodele ->setHorizontalHeaderItem(3,pitem3);
    pitem0  ->setEditable(false);
    pitem1  ->setEditable(false);
    pitem2  ->setEditable(false);
    pitem3  ->setEditable(false);
    for (int i=0; i<refractlist.size(); i++)
    {
        pitem0  = new QStandardItem(refractlist.at(i).at(1).toDate().toString(tr("dd-MMM-yyyy")));
        if (gMode == Recuperer)
            pitem0  ->setCheckable(true);
        QString Mesure = "";
        if (refractlist.at(i).at(2).toString() == "P")   Mesure = tr("Porte");
        if (refractlist.at(i).at(2).toString() == "A")   Mesure = tr("AutoRef");
        if (refractlist.at(i).at(2).toString() == "R")   Mesure = tr("Réfraction");
        if (refractlist.at(i).at(2).toString() == "O")   Mesure = tr("Ordonnance");
        pitem1  = new QStandardItem(Mesure);
        pitem2  = new QStandardItem(refractlist.at(i).at(3).toString());
        pitem3  = new QStandardItem(refractlist.at(i).at(4).toString());
        pitem4  = new QStandardItem(refractlist.at(i).at(0).toString());
        QList<QStandardItem*> listitems;
        listitems << pitem0 << pitem1 << pitem2 << pitem3 << pitem4;
        gmodele ->appendRow(listitems);
    }
    tabLM->setModel(gmodele);

    QFontMetrics fm(qApp->font());
    int hauteurligne = int(fm.height()*1.1);
    for (int j=0; j<gmodele->rowCount(); j++)
        tabLM->setRowHeight(j,hauteurligne);
    tabLM->horizontalHeader()->setFixedHeight(hauteurligne);
}

