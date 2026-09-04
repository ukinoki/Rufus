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

#include <QHeaderView>

#include "dlg_choixcotation.h"
#include "cls_datas.h"
#include "database.h"
#include "upstandarditem.h"

dlg_choixcotation::dlg_choixcotation(QWidget *parent) : UpDialog(parent)
{
    setWindowTitle(tr("Rechercher une cotation"));

    wdg_table = new QTableView(this);
    wdg_table   ->setEditTriggers(QAbstractItemView::NoEditTriggers);
    wdg_table   ->setSelectionMode(QAbstractItemView::SingleSelection);
    wdg_table   ->setSelectionBehavior(QAbstractItemView::SelectRows);
    wdg_table   ->setWordWrap(true);
    wdg_table   ->verticalHeader()->setVisible(false);
    wdg_table   ->setMinimumSize(700, 400);

    m_model = new QStandardItemModel(this);
    RemplitTable();

    /*! le filtre porte sur le descriptif, la colonne que l'utilisateur lit pour trouver son acte */
    m_proxy = new QSortFilterProxyModel(this);
    m_proxy     ->setSourceModel(m_model);
    m_proxy     ->setFilterKeyColumn(ColDescriptif);
    m_proxy     ->setFilterCaseSensitivity(Qt::CaseInsensitive);
    wdg_table   ->setModel(m_proxy);

    wdg_table   ->horizontalHeader()->setSectionResizeMode(ColDescriptif, QHeaderView::Stretch);
    wdg_table   ->setColumnWidth(ColCotation, 90);
    for (int col : {ColNonOptam, ColOptam, ColPratique})
        wdg_table   ->setColumnWidth(col, 100);
    if (!DataBase::I()->parametres()->cotationsfrance())
    {
        wdg_table   ->setColumnHidden(ColNonOptam, true);
        wdg_table   ->setColumnHidden(ColOptam, true);
    }
    wdg_table   ->resizeRowsToContents();

    wdg_buttonframe = new WidgetButtonFrame(wdg_table);
    wdg_buttonframe ->AddButtons(WidgetButtonFrame::Buttons());   //! aucun bouton : seule la ligne de recherche est voulue
    wdg_buttonframe ->addSearchLine();
    AjouteLayButtons(UpDialog::ButtonCancel | UpDialog::ButtonOK);
    dlglayout() ->insertWidget(0, wdg_buttonframe->widgButtonParent());

    OKButton    ->setEnabled(false);
    connect(wdg_buttonframe->searchline(),  &QLineEdit::textEdited,     this,   [=, this] (QString txt) {
        m_proxy     ->setFilterFixedString(txt);
        wdg_table   ->resizeRowsToContents();
        OKButton    ->setEnabled(wdg_table->selectionModel()->hasSelection());
    });
    connect(wdg_table->selectionModel(),    &QItemSelectionModel::selectionChanged, this,   [=, this] {
        OKButton    ->setEnabled(wdg_table->selectionModel()->hasSelection());
    });
    connect(wdg_table,                      &QAbstractItemView::doubleClicked,      this,   &dlg_choixcotation::RetientCotation);
    connect(OKButton,                       &QPushButton::clicked,      this,   [=, this] {
        RetientCotation(wdg_table->currentIndex());
    });

    wdg_buttonframe->searchline()->setFocus();
    setEnregPosition(true);
    setSaveGeometry(Nom_fiche_ChoixCotation);
}

/*!
 * \brief dlg_choixcotation::RemplitTable
 * Une ligne par cotation, la cotation elle-même portée par la première colonne. Le montant pratiqué ne
 * vaut que pour celles que l'utilisateur pratique ; les codes seulement présents dans la CCAM n'en ont pas.
 */
void dlg_choixcotation::RemplitTable()
{
    m_model     ->clear();
    m_model     ->setHorizontalHeaderLabels(QStringList()
                    << tr("Cotation") << tr("Descriptif") << tr("Non OPTAM") << tr("OPTAM") << tr("Pratiqué"));

    auto ajoute = [&] (Cotation *c) {
        QList<QStandardItem*> ligne;
        ligne << new UpStandardItem(c->typeacte(), c);
        ligne << new QStandardItem(c->descriptif());
        ligne << new QStandardItem(QLocale().toString(c->montantnonoptam(), 'f', 2));
        ligne << new QStandardItem(QLocale().toString(c->montantoptam(), 'f', 2));
        ligne << new QStandardItem(c->isused()? QLocale().toString(c->montantpratique(), 'f', 2) : QString());
        for (int col : {ColNonOptam, ColOptam, ColPratique})
            ligne.at(col)->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        ligne.at(ColCotation)->setTextAlignment(Qt::AlignLeft | Qt::AlignTop);
        m_model ->appendRow(ligne);
    };

    QStringList codesvus;
    for (Cotation *c : *Datas::I()->cotations->cotations())
    {
        ajoute(c);
        codesvus << c->typeacte();
    }
    Datas::I()->cotations->initListeCCAM();
    for (Cotation *c : *Datas::I()->cotations->ccam())
        if (!codesvus.contains(c->typeacte()))
            ajoute(c);
}

void dlg_choixcotation::RetientCotation(QModelIndex idx)
{
    if (!idx.isValid())
        return;
    const QModelIndex source = m_proxy->mapToSource(idx.siblingAtColumn(ColCotation));
    UpStandardItem *item = dynamic_cast<UpStandardItem*>(m_model->itemFromIndex(source));
    if (item == nullptr || !item->hasrufusitem())
        return;
    m_cotation = qobject_cast<Cotation*>(item->rufusitem());
    accept();
}
