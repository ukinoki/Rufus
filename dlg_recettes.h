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

#ifndef DLG_RECETTES_H
#define DLG_RECETTES_H



#include <QLocale>
#include "procedures.h"
#include "uptablewidget.h"

class dlg_recettes : public UpDialog
{
    Q_OBJECT

public:
    explicit dlg_recettes(QDate *DateDebut, QDate *Datefin, Procedures *ProcAPasser, QSqlQuery BilanRec, QWidget *parent = Q_NULLPTR);
    ~dlg_recettes();

private:
    Procedures                  *proc;
    QDate                       Debut, Fin;
    QSqlQuery                   gBilan;
    QCheckBox                   *gSupervcheckBox;
    UpComboBox                  *gSupervbox;
    UpTableWidget               *gBigTable;
    UpLabel                     *TotalMontantlbl, *TotalReclbl, *DetailReclbl;
    void                        CalculeTotal();
    void                        DefinitArchitetureTable();
    void                        RemplitLaTable();

private slots:
    void                        Slot_FiltreTable();
    void                        Slot_ImprimeEtat();
};

#endif // DLG_RECETTES_H
