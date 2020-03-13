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

#ifndef DLG_LISTEIOLS_H
#define DLG_LISTEIOLS_H

#include "updialog.h"
#include <QHeaderView>
#include <QTreeView>
#include "gbl_datas.h"
#include "widgetbuttonframe.h"

class dlg_listeiols : public UpDialog
{
    Q_OBJECT
public:
    explicit                dlg_listeiols(QWidget *parent = Q_NULLPTR);
    ~dlg_listeiols();
    bool                    listeIOLsmodifiee() const;

private:
    bool                    m_listemodifiee = false;

    QStandardItemModel      *m_model;
    UpLabel                 *wdg_label;
    QTreeView               *wdg_iolstree;
    UpLineEdit              *wdg_chercheuplineedit;
    WidgetButtonFrame       *wdg_buttonframe;

    void                    ChoixButtonFrame();
    void                    Enablebuttons();
    void                    EnregistreNouveauIOL();
    IOL*                    getIOLFromIndex(QModelIndex idx);
    QList<UpStandardItem*>  ListeIOLs();                // la liste des iols
    void                    ModifIOL(IOL *iol);
    void                    SupprIOL();
    void                    ReconstruitTreeViewIOLs(bool reconstruirelaliste = false, QString filtre = "");
};

#endif // DLG_LISTEIOLS_H
