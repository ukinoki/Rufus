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

#ifndef dlg_listecorrespondants_H
#define dlg_listecorrespondants_H

#include <QToolTip>
#include "dlg_identificationcorresp.h"
#include "uplineedit.h"
#include <QFrame>
#include <QToolTip>
#include "uplabel.h"
#include <QTreeView>
#include "procedures.h"
#include "widgetbuttonframe.h"

class dlg_listecorrespondants : public UpDialog
{
    Q_OBJECT

public:
    explicit dlg_listecorrespondants(QWidget *parent = Q_NULLPTR);
    ~dlg_listecorrespondants();
    bool                getListeModifiee();

private:
    dlg_identificationcorresp   *Dlg_IdentCorresp;
    Procedures              *proc;
    DataBase                *db;
    QTreeView               *treeCor;

    UpLineEdit              *ChercheUplineEdit;
    UpLabel                 *label;
    QStandardItemModel      *gmodele;
    QList<QStandardItem*>   ListeMetiers();                // la liste des metiers
    bool                    ListeModifiee;

    WidgetButtonFrame   *widgButtons;
    void                ChoixButtonFrame(int);
    void                Enablebuttons();
    void                EnregistreNouveauCorresp();
    void                ModifCorresp(int idcor);
    void                SupprCorresp();
    void                ReconstruitListeCorrespondants(QString filtre = "");
};

#endif // dlg_listecorrespondants_H
