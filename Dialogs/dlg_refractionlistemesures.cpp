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

#include "dlg_refractionlistemesures.h"

dlg_refractionlistemesures::dlg_refractionlistemesures(Mode mode, QWidget *parent) :
    UpDialog (PATH_FILE_INI, "PositionsFiches/PositionListeMes", parent)
{
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    setWindowModality(Qt::WindowModal);
    m_mode       = mode;

    wdg_bigtable = new QTableView(this);

    dlglayout()->insertWidget(0,wdg_bigtable);

    setSizeGripEnabled(false);
    setWindowTitle(tr("Liste des mesures"));

    if (m_mode == Recuperer)
    {
        AjouteLayButtons(UpDialog::ButtonCancel | UpDialog::ButtonOK);
        OKButton->setText(tr("Reprendre"));
        wdg_bigtable->setSelectionMode(QAbstractItemView::NoSelection);
        connect (OKButton,      &QPushButton::clicked,  this, &dlg_refractionlistemesures::Validation);
    }
    else
    {
        AjouteLayButtons(UpDialog::ButtonCancel | UpDialog::ButtonSuppr);
        SupprButton->setText(tr("Supprimer"));
        wdg_bigtable->setSelectionMode(QAbstractItemView::MultiSelection);
        wdg_bigtable->setSelectionBehavior(QAbstractItemView::SelectRows);
        connect (SupprButton,   &QPushButton::clicked,  this, &dlg_refractionlistemesures::Validation);
    }
    setStageCount(1);

    RemplirTableView();
    wdg_bigtable->verticalHeader()->setVisible(true);
    wdg_bigtable->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    wdg_bigtable->setFocusPolicy(Qt::StrongFocus);
    wdg_bigtable->setGridStyle(Qt::NoPen);
    wdg_bigtable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    wdg_bigtable->setColumnWidth(0,135);     // Date
    wdg_bigtable->setColumnWidth(1,90);      // Mesure
    wdg_bigtable->setColumnWidth(2,280);     // Formule OD
    wdg_bigtable->setColumnWidth(3,280);     // Formule OG
    wdg_bigtable->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);

    int larg = 0;
    for (int i=0; i < m_model->columnCount(); i++)
        if (!wdg_bigtable->isColumnHidden(i))
            larg += wdg_bigtable->columnWidth(i);
    wdg_bigtable->setFixedWidth(larg+2+wdg_bigtable->verticalHeader()->width());
    setFixedWidth(wdg_bigtable->width()
                        + dlglayout()->contentsMargins().left()*2);
    QFontMetrics fm(qApp->font());
    int hauteurligne = int(fm.height()*1.1);
    wdg_bigtable->setMinimumHeight(hauteurligne*20);
    wdg_bigtable->setSizeIncrement(0,hauteurligne);
    wdg_bigtable->setMouseTracking(true);
    dlglayout()->insertWidget(0,wdg_bigtable);
    connect (wdg_bigtable,   &QAbstractItemView::clicked,  this, &dlg_refractionlistemesures::ItemClicked);
}

dlg_refractionlistemesures::~dlg_refractionlistemesures()
{
}

//--------------------------------------------------------------------------------------
// Clic sur Bouton Supprimer ou Reprendre
//--------------------------------------------------------------------------------------
void dlg_refractionlistemesures::Validation()

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
            {
                UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_model->item(wdg_bigtable->selectionModel()->selectedRows().at(i).row(),0));
                if (itm != Q_NULLPTR)
                    DetruireLaMesure(Datas::I()->refractions->getById(itm->item()->id()));
            }
            accept();
        }
    }
}

//--------------------------------------------------------------------------------------
// Click sur une ligne de la liste
//--------------------------------------------------------------------------------------
void dlg_refractionlistemesures::ItemClicked(QModelIndex mod)
{
    if (m_mode == Supprimer)
        SupprButton->setEnabled(wdg_bigtable->selectionModel()->selectedIndexes().size()>0);
    else if (m_mode ==  Recuperer)
    {
        UpStandardItem *pitem = dynamic_cast<UpStandardItem*>(m_model->itemFromIndex(mod));
        if (pitem == Q_NULLPTR)
            return;
        if (pitem->checkState() == Qt::Checked)
        {
            for (int i =0 ; i < m_model->rowCount(); i++)
            {
                if (m_model->item(i,0)->checkState() == Qt::Checked && i != pitem->row())
                    m_model->item(i,0)->setCheckState(Qt::Unchecked);
            }
            m_refselectionne = qobject_cast<Refraction*>(pitem->item());
        }
        OKButton->setEnabled(Nombre_Mesure_Selected() > 0);
    }
}

//---------------------------------------------------------------------------------
// Suppression d'une mesure en base
//----------------------------------------------------------------------------------
void dlg_refractionlistemesures::DetruireLaMesure(Refraction *ref)
{
    if (ref == Q_NULLPTR)
        return;
    QString mesure = Refraction::ConvertMesure(ref->typemesure());
    if (mesure != "")
    {
        QString req ("");
        switch (ref->typemesure()) {
        case Refraction::Acuite:
            req = "DELETE  FROM " TBL_DONNEES_OPHTA_PATIENTS " WHERE " CP_MESURE_DATAOPHTA " = '"
                    + mesure + "' and " CP_IDPATIENT_DATAOPHTA " = " + QString::number(ref->idpat());
            db->StandardSQL(req, tr("Impossible de suppimer cette mesure dans donneesophtapatients!"));
            break;
        case Refraction::Autoref:
            req = "update " TBL_DONNEES_OPHTA_PATIENTS " set "
                    CP_SPHEREOD_DATAOPHTA           " = null, "
                    CP_CYLINDREOD_DATAOPHTA         " = null, "
                    CP_AXECYLINDREOD_DATAOPHTA      " = null, "
                    CP_DATEREFRACTIONOD_DATAOPHTA   " = null, "
                    CP_SPHEREOG_DATAOPHTA           " = null, "
                    CP_CYLINDREOG_DATAOPHTA         " = null, "
                    CP_AXECYLINDREOG_DATAOPHTA      " = null, "
                    CP_DATEREFRACTIONOG_DATAOPHTA   " = null, "
                    CP_ECARTIP_DATAOPHTA            " = null "
                    " WHERE " CP_MESURE_DATAOPHTA " = '" + mesure + "' and " CP_IDPATIENT_DATAOPHTA " = " + QString::number(ref->idpat());
            db->StandardSQL(req, tr("Impossible de suppimer cette mesure dans donneesophtapatients!"));
            break;
        default:
            break;
        }
    }
    Datas::I()->refractions->SupprimeRefraction(ref);
    Datas::I()->patients->actualiseDonneesOphtaCurrentPatient();
}

Refraction* dlg_refractionlistemesures::RefractionAOuvrir() const
{
    return m_refselectionne;
}

// ----------------------------------------------------------------------------------
// Retourne le nombre de lignes comm selectionnees
// ----------------------------------------------------------------------------------
int dlg_refractionlistemesures::Nombre_Mesure_Selected()
{
    int nb = 0;
    for (int i =0 ; i < m_model->rowCount(); i++)
        if (m_model->item(i,0)->checkState() == Qt::Checked)
            nb ++;
    return nb;
}

//--------------------------------------------------------------------------------------
// Remplir la liste avec les mesures du patient en cours
//--------------------------------------------------------------------------------------
void dlg_refractionlistemesures::RemplirTableView()
{
    if (m_mode == Recuperer)
        OKButton->setEnabled(false);
    else
        SupprButton->setEnabled(false);

    UpStandardItem       *pitem0, *pitem1, *pitem2, *pitem3;
    if (m_model != Q_NULLPTR)
        delete m_model;
    m_model = new QStandardItemModel(this);

    pitem0  = new UpStandardItem(tr("Date"));
    m_model ->setHorizontalHeaderItem(0,pitem0);
    pitem1  = new UpStandardItem(tr("Mesure"));
    m_model ->setHorizontalHeaderItem(1,pitem1);
    pitem2  = new UpStandardItem(tr("Formule OD"));
    m_model ->setHorizontalHeaderItem(2,pitem2);
    pitem3  = new UpStandardItem(tr("Formule OG"));
    m_model ->setHorizontalHeaderItem(3,pitem3);
    pitem0  ->setEditable(false);
    pitem1  ->setEditable(false);
    pitem2  ->setEditable(false);
    pitem3  ->setEditable(false);
    foreach (Refraction *ref, *Datas::I()->refractions->refractions())
    {
        pitem0  = new UpStandardItem(ref->daterefraction().toString(tr("dd-MMM-yyyy")), ref);
        if (m_mode == Recuperer)
            pitem0  ->setCheckable(true);
        QString Mesure = "";
        if (ref->typemesure() == Refraction::Fronto)             Mesure = tr("Porte");
        else if (ref->typemesure() == Refraction::Autoref)      Mesure = tr("AutoRef");
        else if (ref->typemesure() == Refraction::Acuite)       Mesure = tr("Réfraction");
        else if (ref->typemesure() == Refraction::Prescription) Mesure = tr("Ordonnance");
        pitem1  = new UpStandardItem(Mesure, ref);
        pitem2  = new UpStandardItem(ref->formuleOD(), ref);
        pitem3  = new UpStandardItem(ref->formuleOG(), ref);
        QList<QStandardItem*> listitems;
        listitems << pitem0 << pitem1 << pitem2 << pitem3;
        m_model ->appendRow(listitems);
    }
    wdg_bigtable->setModel(m_model);

    QFontMetrics fm(qApp->font());
    int hauteurligne = int(fm.height()*1.1);
    for (int j=0; j<m_model->rowCount(); j++)
        wdg_bigtable->setRowHeight(j,hauteurligne);
    wdg_bigtable->horizontalHeader()->setFixedHeight(hauteurligne);
}

