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

TiersPayants::TiersPayants()
{
    m_tierspayants = new QMap<int, Tiers *>();
}

QMap<int, Tiers *> *TiersPayants::tierspayants() const
{
    return m_tierspayants;
}

void TiersPayants::add(Tiers *Tiers)
{
    if( m_tierspayants->contains(Tiers->id()) )
        return;
    m_tierspayants->insert(Tiers->id(), Tiers);
}

void TiersPayants::addList(QList<Tiers*> listTiersPayants)
{
    QList<Tiers*>::const_iterator it;
    for( it = listTiersPayants.constBegin(); it != listTiersPayants.constEnd(); ++it )
        add( *it );
}

Tiers* TiersPayants::getById(int id)
{
    QMap<int, Tiers*>::const_iterator itcpt = m_tierspayants->find(id);
    if( itcpt == m_tierspayants->constEnd() )
        return Q_NULLPTR;
    return itcpt.value();
}

void TiersPayants::clearAll()
{
    for( QMap<int, Tiers*>::const_iterator ittrs = m_tierspayants->constBegin(); ittrs != m_tierspayants->constEnd(); ++ittrs)
        delete ittrs.value();
    m_tierspayants->clear();
}

void TiersPayants::remove(Tiers *tiers)
{
    if (tiers == Q_NULLPTR)
        return;
    while( m_tierspayants->contains(tiers->id()) )
        m_tierspayants->remove(tiers->id());
    delete tiers;
}

/*!
 * \brief TiersPayants::initListe
 * Charge l'ensemble des tiers payants
 * et les ajoute à la classe Tiers
 */
void TiersPayants::initListe()
{
    clearAll();
    QList<Tiers*> listtiers = DataBase::getInstance()->loadTiersPayants();
    QList<Tiers*>::const_iterator ittrs;
    for( ittrs = listtiers.constBegin(); ittrs != listtiers.constEnd(); ++ittrs )
    {
        Tiers *trs = const_cast<Tiers*>(*ittrs);
        add( trs );
    }
}
