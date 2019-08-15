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

#include "cls_tierspayants.h"

TiersPayants::TiersPayants(QObject *parent) : ItemsList(parent)
{
    map_tierspayants = new QMap<int, Tiers *>();
}

QMap<int, Tiers *> *TiersPayants::tierspayants() const
{
    return map_tierspayants;
}

Tiers* TiersPayants::getById(int id)
{
    QMap<int, Tiers*>::const_iterator itcpt = map_tierspayants->find(id);
    if( itcpt == map_tierspayants->constEnd() )
        return Q_NULLPTR;
    return itcpt.value();
}

/*!
 * \brief TiersPayants::initListe
 * Charge l'ensemble des tiers payants
 * et les ajoute à la classe Tiers
 */
void TiersPayants::initListe()
{
    QList<Tiers*> listtiers = DataBase::I()->loadTiersPayants();
    epurelist(map_tierspayants, &listtiers);
    addList(map_tierspayants, &listtiers);
}
