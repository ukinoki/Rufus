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


TypesTiers::TypesTiers(QObject *parent) : ItemsList(parent)
{
    m_typestiers = new QList<TypeTiers *>();
}

QList<TypeTiers *> *TypesTiers::typestiers() const
{
    return m_typestiers;
}

bool TypesTiers::add(TypeTiers *typetiers)
{
    if (typetiers == Q_NULLPTR)
        return false;
    if( m_typestiers->contains(typetiers) )
    {
        delete typetiers;
        return false;
    }
    *m_typestiers << typetiers;
    return true;
}

void TypesTiers::addList(QList<TypeTiers*> listTypesTiers)
{
    QList<TypeTiers*>::const_iterator it;
    for( it = listTypesTiers.constBegin(); it != listTypesTiers.constEnd(); ++it )
    {
        TypeTiers* trs = const_cast<TypeTiers*>(*it);
        add( trs );
    }
}

void TypesTiers::remove(TypeTiers* typetiers)
{
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
    addList(DataBase::I()->loadTypesTiers());
}

