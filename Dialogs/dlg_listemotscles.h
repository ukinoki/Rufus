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
    explicit dlg_listemotscles(Patient *pat, QWidget *parent = Q_NULLPTR);
    ~dlg_listemotscles();
    QStringList         listMCDepart();
    enum Mode {Creation, Modif};    Q_ENUM(Mode)

private:
    Patient             *gPatientEncours;
    Mode                 gMode;
    QTableView          *tabMC;
    DataBase            *db;

    QHBoxLayout         *editlayout;
    UpDialog            *gAskDialog;
    QStandardItemModel  *gmodele;
    QItemSelectionModel *gselection;
    QStringList         glistMC, glistidMCdepart;

    void                DisableLines();
    void                RemplirTableView();
    WidgetButtonFrame   *widgButtons;
    void                CreationModifMC(enum Mode);
    void                SupprMC();

private slots:
    void                Slot_ChoixButtonFrame(int);
    void                Slot_Enablebuttons();
    void                Slot_OK();
    void                Slot_VerifMC();

};

#endif // DLG_LISTEMOTSCLES_H
