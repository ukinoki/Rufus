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

#include "cls_typestiers.h"


TypesTiers::TypesTiers()
{
    m_typestiers = new QList<TypeTiers *>();
}

QList<TypeTiers *> *TypesTiers::typestiers() const
{
    return m_typestiers;
}

void TypesTiers::add(TypeTiers *typetiers)
{
    if( m_typestiers->contains(typetiers) )
        return;
    *m_typestiers << typetiers;
}

void TypesTiers::addList(QList<TypeTiers*> listTypesTiers)
{
    QList<TypeTiers*>::const_iterator it;
    for( it = listTypesTiers.constBegin(); it != listTypesTiers.constEnd(); ++it )
        add( *it );
}

void TypesTiers::remove(TypeTiers* typetiers)
{
    while( m_typestiers->contains(typetiers) )
        m_typestiers->removeOne(typetiers);
    delete  typetiers;
}
void TypesTiers::clearAll()
{
    while (m_typestiers->size() >0)
        remove(m_typestiers->at(0));
    m_typestiers->clear();
}

/*!
 * \brief TypesTiers::initListe
 * Charge l'ensemble des types de tiers payants
 * et les ajoute à la classe TypesTiers
 */
void TypesTiers::initListe()
{
    clearAll();
    QList<TypeTiers*> listtypes = DataBase::getInstance()->loadTypesTiers();
    QList<TypeTiers*>::const_iterator ittyp;
    for( ittyp = listtypes.constBegin(); ittyp != listtypes.constEnd(); ++ittyp )
    {
        TypeTiers *typ = const_cast<TypeTiers*>(*ittyp);
        add( typ );
    }
}

