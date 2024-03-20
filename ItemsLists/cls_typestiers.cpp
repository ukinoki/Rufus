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
        return false;
    *m_typestiers << typetiers;
    return true;
}

void TypesTiers::addList(QList<TypeTiers*> listTypesTiers)
{
    foreach (TypeTiers* trs, listTypesTiers)
        if (trs != Q_NULLPTR)
            add( trs );
}

void TypesTiers::initListe()
{
    qDeleteAll(*m_typestiers);
    m_typestiers->clear();
    addList(DataBase::I()->loadTypesTiers());
}

