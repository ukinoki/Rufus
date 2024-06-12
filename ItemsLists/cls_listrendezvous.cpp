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

#include "cls_listrendezvous.h"

ListRendezVous::ListRendezVous(QObject *parent) : ItemsList(parent)
{
    map_all     = new QMap<int, RendezVous*>();
}

QMap<int, RendezVous*>* ListRendezVous::listRendezVous() const
{
    return map_all;
}

RendezVous* ListRendezVous::getByIdPatient(int id)
{
    QMap<int, RendezVous*>::const_iterator itrdv = map_all->constFind(id);
    if( itrdv == map_all->constEnd() )
        return Q_NULLPTR;
    return itrdv.value();
}

void ListRendezVous::SupprimeRendezVous(RendezVous* rdv)
{
    map_all->remove(rdv->idpatient());
    delete rdv;
    rdv = Q_NULLPTR;
}

void ListRendezVous::AddRendezVous(RendezVous* rdv)
{
    if (rdv == Q_NULLPTR)
        return;
    RendezVous * exrdv = getByIdPatient(rdv->idpatient());
    if (exrdv != Q_NULLPTR)
        SupprimeRendezVous(exrdv);
    map_all->insert(rdv->idpatient(), rdv);
}

