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

#ifndef DLG_REFRACTIONLISTEMESURES_H
#define DLG_REFRACTIONLISTEMESURES_H

#include "procedures.h"

class dlg_refractionlistemesures : public UpDialog
{
    Q_OBJECT
public:
    enum Mode               {Supprimer, Recuperer};   Q_ENUM(Mode)
    explicit                dlg_refractionlistemesures(Mode mode, QWidget *parent = Q_NULLPTR);
    ~dlg_refractionlistemesures();
    Refraction*             RefractionAOuvrir() const;

private:
    Procedures          *proc   = Procedures::I();
    DataBase            *db     = DataBase::I();
    QTableView          *wdg_bigtable;
    QStandardItemModel  *m_model = Q_NULLPTR;
    Mode                m_mode;
    Refraction*         m_refselectionne;

    void                DetruireLaMesure(Refraction* ref);
    void                ItemClicked(QModelIndex);
    int                 Nombre_Mesure_Selected();
    void                RemplirTableView();
    void                Validation();
};
#endif // DLG_REFRACTIONLISTEMESURES_H
