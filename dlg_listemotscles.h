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

#ifndef DLG_LISTEMOTSCLES_H
#define DLG_LISTEMOTSCLES_H

#include "upcheckbox.h"
#include "uplabel.h"
#include "uplineedit.h"
#include "procedures.h"
#include "widgetbuttonframe.h"

class dlg_listemotscles : public UpDialog
{
    Q_OBJECT

public:
    explicit dlg_listemotscles(Procedures *Proc, int idpat, QWidget *parent = Q_NULLPTR);
    ~dlg_listemotscles();
    QStringList         listMCDepart();

private:
    int                 idpat;
    int                 gMode;
    enum gMode {Creation, Modif};
    QTableView          *tabMC;
    Procedures          *proc;

    QHBoxLayout         *editlayout;
    UpDialog            *gAskDialog;
    QStandardItemModel  *gmodele;
    QItemSelectionModel *gselection;
    QStringList         glistMC, glistidMCdepart;

    void                DisableLines();
    void                RemplirTableView();
    WidgetButtonFrame   *widgButtons;
    void                CreationModifMC(enum gMode);
    void                SupprMC();

private slots:
    void                Slot_ChoixButtonFrame(int);
    void                Slot_Enablebuttons();
    void                Slot_OK();
    void                Slot_VerifMC();

};

#endif // DLG_LISTEMOTSCLES_H
