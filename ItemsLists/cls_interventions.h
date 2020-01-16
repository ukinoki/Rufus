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

#ifndef INTERVENTIONS_H
#define INTERVENTIONS_H

#include "cls_intervention.h"
#include "cls_itemslist.h"

class Interventions : public ItemsList
{
private:
    QMap<int, Intervention*> *map_interventions = Q_NULLPTR;    //!< la liste des interventions
    int m_idpat = 0;                                            //!< l'id du patient concerné par la map

public:
    explicit Interventions(QObject *parent = Q_NULLPTR);

    QMap<int, Intervention*> *interventions() const;
    int idpatient() const { return m_idpat ; }

    Intervention* getById(int id);
    void initListebyUserId(int id);

    //!> actions sur les enregistrements
    void                SupprimeIntervention(Intervention *intervention);
    Intervention*       CreationIntervention(QHash<QString, QVariant> sets);
};

#endif // INTERVENTIONS_H
