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

#include "database.h"
#include "cls_tiers.h"


/*!
 * \brief TierPayants class
 * la liste des différents organsmes de tiers payants
 * CPAM, MGEN, MSA...etc..
 * géré par la table ComptaMedicale.tiers
 */
class TiersPayants
{
private:
    QMap<int, Tiers*> *m_tierspayants; //!< la liste des tiers payants

public:
    explicit TiersPayants();

    QMap<int, Tiers *> *tierspayants() const;

    void add(Tiers *Tiers);
    void addList(QList<Tiers*> listTierss);
    void remove(Tiers* Tiers);
    Tiers* getById(int id);
    void clearAll();
    void initListe();
};

#endif // TIERSPAYANTS_H
