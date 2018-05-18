/* (C) 2016 LAINE SERGE
This file is part of Rufus.

Rufus is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Rufus is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Rufus.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef dlg_listecorrespondants_H
#define dlg_listecorrespondants_H

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
    explicit dlg_listecorrespondants(Procedures *Proc, QSqlQuery Quer, QWidget *parent = Q_NULLPTR);
    ~dlg_listecorrespondants();
    bool                getListeModifiee();

private:
    QSqlQuery           quer;
    Procedures          *proc;
    QTreeView           *treeCor;

    QVBoxLayout         *globallayout;
    UpLineEdit          *ChercheUplineEdit;
    UpLabel             *label;
    QStandardItemModel  *gmodele;
    bool                ListeModifiee;

    WidgetButtonFrame   *widgButtons;
    void                EnregistreNouveauCorresp();
    void                ModifCorresp();
    void                SupprCorresp();
    dlg_identificationcorresp   *Dlg_IdentCorresp;
    void                ReconstruitListeCorrespondants(bool AvecRecalcul = false);

private slots:
    void                Slot_ChoixButtonFrame(int);
    void                Slot_ChercheCor(QString);
    void                Slot_Enablebuttons();
    void                Slot_Reponse();
};

#endif // dlg_listecorrespondants_H
