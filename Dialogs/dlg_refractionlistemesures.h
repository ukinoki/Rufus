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

#ifndef DLG_REFRACTIONLISTEMESURES_H
#define DLG_REFRACTIONLISTEMESURES_H

#include "procedures.h"

class dlg_listemesures : public UpDialog
{
    Q_OBJECT

public:
    enum Mode               {Supprimer, Recuperer};   Q_ENUM(Mode)
    explicit                dlg_listemesures(Mode mode, QWidget *parent = Q_NULLPTR);
    ~dlg_listemesures();
    Refraction*             RefractionAOuvrir() const;

private:
    Procedures          *proc   = Procedures::I();
    DataBase            *db     = DataBase::I();
    QTableView          *wdg_bigtable;
    QStandardItemModel  *m_modele;
    Mode                m_mode;
    Refraction*         m_refselectionne;

    void                DetruireLaMesure(Refraction* ref);
    int                 Nombre_Mesure_Selected();
    void                RemplirTableView();
    void                Validation();

private slots:
    void                Slot_ItemClicked(QModelIndex);

};
#endif // DLG_REFRACTIONLISTEMESURES_H
