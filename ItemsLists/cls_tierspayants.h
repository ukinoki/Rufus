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

#ifndef CLS_TIERSPAYANTS_H
#define CLS_TIERSPAYANTS_H

#include "cls_tiers.h"
#include "cls_itemslist.h"


/*!
 * \brief TierPayants class
 * la liste des différents organsmes de tiers payants
 * CPAM, MGEN, MSA...etc..
 * géré par la table ComptaMedicale.tiers
 */
class TiersPayants : public ItemsList
{
private:
    QMap<int, Tiers*> *map_tierspayants; //!< la liste des tiers payants

public:
    explicit TiersPayants(QObject *parent = Q_NULLPTR);

    QMap<int, Tiers *> *tierspayants() const;

    Tiers* getById(int id);
    void initListe();
};

#endif // TIERSPAYANTS_H
