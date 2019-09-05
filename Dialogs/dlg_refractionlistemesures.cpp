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

#include "dlg_refractionlistemesures.h"

dlg_listemesures::dlg_listemesures(QString mode, QWidget *parent) :
    UpDialog (QDir::homePath() + FILE_INI, "PositionsFiches/PositionListeMes", parent)
{
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    if (mode == "SUPPR")
        m_mode       = Supprimer;
    else if (mode == "RECUP")
        m_mode       = Recuperer;

    wdg_bigtable              = new QTableView(this);

    dlglayout()->insertWidget(0,wdg_bigtable);

    setModal(true);
    setSizeGripEnabled(false);
    setWindowTitle(tr("Liste des mesures"));

    if (m_mode == Recuperer)
    {
        AjouteLayButtons(UpDialog::ButtonCancel | UpDialog::ButtonOK);
        OKButton->setText(tr("Reprendre"));
        wdg_bigtable->setSelectionMode(QAbstractItemView::NoSelection);
        connect (OKButton,      SIGNAL(clicked()),  this, SLOT (Slot_Validation()) );
    }
    else
    {
        AjouteLayButtons(UpDialog::ButtonCancel | UpDialog::ButtonSuppr);
        SupprButton->setText(tr("Supprimer"));
        wdg_bigtable->setSelectionMode(QAbstractItemView::MultiSelection);
        wdg_bigtable->setSelectionBehavior(QAbstractItemView::SelectRows);
        connect (SupprButton,   SIGNAL(clicked()),  this, SLOT (Slot_Validation()) );
    }
    setStageCount(1);

    RemplirTableView();
    wdg_bigtable->verticalHeader()->setVisible(false);
    wdg_bigtable->setFocusPolicy(Qt::StrongFocus);
    wdg_bigtable->setGridStyle(Qt::NoPen);
    wdg_bigtable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    wdg_bigtable->setColumnWidth(0,125);     // Date
    wdg_bigtable->setColumnWidth(1,90);      // Mesure
    wdg_bigtable->setColumnWidth(2,280);     // Formule OD
    wdg_bigtable->setColumnWidth(3,280);     // Formule OG
    wdg_bigtable->setColumnHidden(4,true);   // idRefraction
    wdg_bigtable->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);

    int larg = 0;
    for (int i=0; i < m_modele->columnCount(); i++)
        if (!wdg_bigtable->isColumnHidden(i))
            larg += wdg_bigtable->columnWidth(i);
    wdg_bigtable->setFixedWidth(larg+2);
    setFixedWidth(wdg_bigtable->width()
                        + dlglayout()->contentsMargins().left()*2);
    QFontMetrics fm(qApp->font());
    int hauteurligne = int(fm.height()*1.1);
    wdg_bigtable->setMinimumHeight(hauteurligne*20);
    wdg_bigtable->setSizeIncrement(0,hauteurligne);
    wdg_bigtable->setMouseTracking(true);
    dlglayout()->insertWidget(0,wdg_bigtable);
    connect (wdg_bigtable,   SIGNAL(clicked(QModelIndex)),  this, SLOT (Slot_Item_Liste_Clicked(QModelIndex)) );
}

dlg_listemesures::~dlg_listemesures()
{
}

//--------------------------------------------------------------------------------------
// Clic sur Bouton Supprimer ou Reprendre
//--------------------------------------------------------------------------------------
void dlg_listemesures::Slot_Validation()

{
    if (m_mode == Recuperer)
        accept();
    else if (m_mode == Supprimer)
    {
        QString msg;
        if (wdg_bigtable->selectionModel()->selectedRows().size()==1)
            msg = tr("Voulez vous supprimer la mesure sélectionnée?");
        else
            msg = tr("Voulez vous supprimer les mesures sélectionnées?");
        int ret = UpMessageBox::Watch(this,tr("Annulation de mesure"), msg, UpDialog::ButtonCancel|UpDialog::ButtonSuppr);
        if (ret == UpSmallButton::SUPPRBUTTON)
        {
            for (int i =0 ; i<wdg_bigtable->selectionModel()->selectedRows().size(); i++)
                DetruireLaMesure(Datas::I()->refractions->getById(m_modele->item(wdg_bigtable->selectionModel()->selectedRows().at(i).row(),4)->text().toInt()));
            accept();
        }
    }
}

//--------------------------------------------------------------------------------------
// Click sur une ligne de la liste
//--------------------------------------------------------------------------------------
void dlg_listemesures::Slot_Item_Liste_Clicked(QModelIndex mod)
{
    if (m_mode == Supprimer)
        SupprButton->setEnabled(wdg_bigtable->selectionModel()->selectedIndexes().size()>0);
    else if (m_mode ==  Recuperer)
    {
        QStandardItem *pitem = m_modele->itemFromIndex(mod);
        if (pitem->checkState() == Qt::Checked)
        {
            for (int i =0 ; i < m_modele->rowCount(); i++)
            {
                if (m_modele->item(i,0)->checkState() == Qt::Checked && i != pitem->row())
                    m_modele->item(i,0)->setCheckState(Qt::Unchecked);
            }
            m_idrefselectionne = m_modele->item(pitem->row(),4)->text().toInt();
        }
        OKButton->setEnabled(Nombre_Mesure_Selected() > 0);
    }
}

//---------------------------------------------------------------------------------
// Suppression d'une mesure en base
//----------------------------------------------------------------------------------
void dlg_listemesures::DetruireLaMesure(Refraction *ref)
{
    if (ref == Q_NULLPTR)
        return;
    QString mesure = "";
    switch (ref->typemesure()) {
    case Refraction::Fronto:
        mesure = "P";
        break;
    case Refraction::Autoref:
        mesure = "A";
        break;
    case Refraction::Acuite:
        mesure = "R";
        break;
    case Refraction::Prescription:
        mesure = "O";
        break;
    default:
        break;
    }
    if (mesure != "")
    {
        QString requete = "DELETE  FROM " TBL_DONNEES_OPHTA_PATIENTS " WHERE  QuelleMesure = '"
                    + mesure + "' and idpat = " + QString::number(ref->idpat());
        db->StandardSQL(requete, tr("Impossible de suppimer cette mesure dans donneesophtapatients!"));
    }
    Datas::I()->refractions->SupprimeRefraction(ref);
}

int dlg_listemesures::idRefractionAOuvrir() const
{
    return m_idrefselectionne;
}

// ----------------------------------------------------------------------------------
// Retourne le nombre de lignes comm selectionnees
// ----------------------------------------------------------------------------------
int dlg_listemesures::Nombre_Mesure_Selected()
{
    int nbCommSelected = 0;
    for (int i =0 ; i < m_modele->rowCount(); i++)
        if (m_modele->item(i,0)->checkState() == Qt::Checked)
            nbCommSelected ++;
    return nbCommSelected;
}

//--------------------------------------------------------------------------------------
// Remplir la liste avec les mesures du patient en cours
//--------------------------------------------------------------------------------------
void dlg_listemesures::RemplirTableView()
{
    QString zw;
    if (m_mode == Recuperer)
        OKButton->setEnabled(false);
    else
        SupprButton->setEnabled(false);

    QStandardItem       *pitem0, *pitem1, *pitem2, *pitem3, *pitem4;
    m_modele = dynamic_cast<QStandardItemModel*>(wdg_bigtable->model());
    if (m_modele)
        m_modele->clear();
    else
        m_modele = new QStandardItemModel(this);

    pitem0  = new QStandardItem(tr("Date"));
    m_modele ->setHorizontalHeaderItem(0,pitem0);
    pitem1  = new QStandardItem(tr("Mesure"));
    m_modele ->setHorizontalHeaderItem(1,pitem1);
    pitem2  = new QStandardItem(tr("Formule OD"));
    m_modele ->setHorizontalHeaderItem(2,pitem2);
    pitem3  = new QStandardItem(tr("Formule OG"));
    m_modele ->setHorizontalHeaderItem(3,pitem3);
    pitem0  ->setEditable(false);
    pitem1  ->setEditable(false);
    pitem2  ->setEditable(false);
    pitem3  ->setEditable(false);
    foreach (Refraction *ref, *Datas::I()->refractions->refractions())
    {
        pitem0  = new QStandardItem(ref->daterefraction().toString(tr("dd-MMM-yyyy")));
        if (m_mode == Recuperer)
            pitem0  ->setCheckable(true);
        QString Mesure = "";
        if (ref->typemesure() == Refraction::Fronto)             Mesure = tr("Porte");
        else if (ref->typemesure() == Refraction::Autoref)      Mesure = tr("AutoRef");
        else if (ref->typemesure() == Refraction::Acuite)       Mesure = tr("Réfraction");
        else if (ref->typemesure() == Refraction::Prescription) Mesure = tr("Ordonnance");
        pitem1  = new QStandardItem(Mesure);
        pitem2  = new QStandardItem(ref->formuleOD());
        pitem3  = new QStandardItem(ref->formuleOG());
        pitem4  = new QStandardItem(QString::number(ref->id()));
        QList<QStandardItem*> listitems;
        listitems << pitem0 << pitem1 << pitem2 << pitem3 << pitem4;
        m_modele ->appendRow(listitems);
    }
    wdg_bigtable->setModel(m_modele);

    QFontMetrics fm(qApp->font());
    int hauteurligne = int(fm.height()*1.1);
    for (int j=0; j<m_modele->rowCount(); j++)
        wdg_bigtable->setRowHeight(j,hauteurligne);
    wdg_bigtable->horizontalHeader()->setFixedHeight(hauteurligne);
}

