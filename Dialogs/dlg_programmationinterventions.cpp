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

#include "dlg_programmationinterventions.h"

dlg_programmationinterventions::dlg_programmationinterventions(QWidget *parent) : UpDialog(QDir::homePath() + FILE_INI, "PositionsFiches/PositionProgramIntervention",parent)
{
    UpTableView *wdg_tableprogramme         = new UpTableView();
    QTreeView *wdg_dates                    = new QTreeView();
    QComboBox *wdg_listmedecinscombo        = new QComboBox();
    QHBoxLayout *choixmedecinLay            = new QHBoxLayout();
    QHBoxLayout *programmLay                = new QHBoxLayout();

    wdg_tableprogramme      ->setFocusPolicy(Qt::NoFocus);
    wdg_tableprogramme      ->setPalette(QPalette(Qt::white));
    wdg_tableprogramme      ->setEditTriggers(QAbstractItemView::NoEditTriggers);
    wdg_tableprogramme      ->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    wdg_tableprogramme      ->verticalHeader()->setVisible(false);
    wdg_tableprogramme      ->setSelectionMode(QAbstractItemView::SingleSelection);
    wdg_tableprogramme      ->setGridStyle(Qt::SolidLine);
    wdg_tableprogramme      ->verticalHeader()->setVisible(false);
    wdg_tableprogramme      ->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel); // sinon on n'a pas de scrollbar vertical
    wdg_tableprogramme      ->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    choixmedecinLay    ->addWidget(wdg_listmedecinscombo);
    choixmedecinLay    ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    choixmedecinLay    ->setSpacing(5);
    choixmedecinLay    ->setContentsMargins(0,0,0,0);

    programmLay    ->addWidget(wdg_dates);
    programmLay    ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    programmLay    ->addWidget(wdg_tableprogramme);
    programmLay    ->setSpacing(5);
    programmLay    ->setContentsMargins(0,0,0,0);
    programmLay    ->setStretch(0,2);
    programmLay    ->setStretch(1,0);
    programmLay    ->setStretch(2,6);

    dlglayout()   ->insertLayout(0, programmLay);
    dlglayout()   ->insertLayout(0, choixmedecinLay);

    AjouteLayButtons(UpDialog::ButtonOK);
    connect(OKButton,     &QPushButton::clicked,    this, [=] {close();});
    setModal(true);
    dlglayout()->setStretch(0,1);
    dlglayout()->setStretch(1,15);

    QStandardItemModel *md_medecins = new QStandardItemModel();
    foreach (User* usr, *Datas::I()->users->actifs())
        if (usr->isMedecin())
        {
            QList<QStandardItem *> items;
            items << new QStandardItem(usr->login())
                  << new QStandardItem(QString::number(usr->id()));
            if (md_medecins->findItems(usr->login()).size()==0)
                md_medecins->appendRow(items);
        }
    md_medecins->sort(0, Qt::DescendingOrder);
    for (int i=0; i< md_medecins->rowCount(); ++i)
    {
        wdg_listmedecinscombo->addItem(md_medecins->item(i,0)->text());
        wdg_listmedecinscombo->setItemData(i, md_medecins->item(i,1)->text());
    }
    Datas::I()->interventions->initListebyUserId(wdg_listmedecinscombo->currentData().toInt());
}

dlg_programmationinterventions::~dlg_programmationinterventions()
{
}
