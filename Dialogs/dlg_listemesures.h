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

#ifndef DLG_LISTEMESURES_H
#define DLG_LISTEMESURES_H

#include "procedures.h"
#include <QTableView>


class dlg_listemesures : public UpDialog
{
    Q_OBJECT

public:
    explicit                dlg_listemesures(int *IdPatient, QString mode, QWidget *parent = Q_NULLPTR);
    ~dlg_listemesures();
    QTableView              *tabLM;
    QStandardItemModel      *gmodele;
    QString                 IdRefractAOuvrir();

private:
    int             gMode;
    enum gMode {Supprimer, Recuperer};
    int             gidPatient;
    Procedures      *proc;
    void            DetruireLaMesure(int IdRefract);
    int             Nombre_Mesure_Selected();
    DataBase        *db;
    void            RemplirTableView();
    QString         itemSelectionne;

private slots:
    void            Slot_Validation();
    void            Slot_Item_Liste_Clicked(QModelIndex);

};
#endif // DLG_LISTEMESURES_H
