/* (C) 2018 LAINE SERGE
This file is part of Rufus.

Rufus is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Rufus is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Rufus. If not, see <http://www.gnu.org/licenses/>.
*/

#include "cls_tiers.h"

Tiers::Tiers(QJsonObject data, QObject *parent) : Item(parent)
{
    setData(data);
}

int Tiers::id() const                   { return m_id; }
QString Tiers::nomtiers() const         { return m_nomtiers; }
QString Tiers::adressetiers() const     { return m_adressetiers; }
QString Tiers::codepostaltiers() const  { return m_codepostaltiers; }
QString Tiers::villetiers() const       { return m_villetiers; }
QString Tiers::telephonetiers() const   { return m_telephonetiers; }
QString Tiers::faxtiers() const         { return m_faxtiers; }

void Tiers::setData(QJsonObject data)
{
    if( data.isEmpty() )
        return;
    setDataInt(data, "id", m_id);
    setDataString(data, "nomtiers", m_nomtiers);
    setDataString(data, "adressetiers", m_adressetiers);
    setDataString(data, "codepostaltiers", m_codepostaltiers);
    setDataString(data, "villetiers", m_villetiers);
    setDataString(data, "telephonetiers", m_telephonetiers);
    setDataString(data, "faxtiers", m_faxtiers);
}


TiersPayants::TiersPayants()
{
    m_tierspayants = new QMap<int, Tiers *>();
}

QMap<int, Tiers *> *TiersPayants::tierspayants() const
{
    return m_tierspayants;
}

void TiersPayants::addTiers(Tiers *Tiers)
{
    if( m_tierspayants->contains(Tiers->id()) )
        return;
    m_tierspayants->insert(Tiers->id(), Tiers);
}

void TiersPayants::addTiers(QList<Tiers*> listTiersPayants)
{
    QList<Tiers*>::const_iterator it;
    for( it = listTiersPayants.constBegin(); it != listTiersPayants.constEnd(); ++it )
        addTiers( *it );
}

void TiersPayants::removeTiers(Tiers* Tiers)
{
    m_tierspayants       ->remove(Tiers->id());
}

Tiers* TiersPayants::getTiersById(int id)
{
    QMultiMap<int, Tiers*>::const_iterator itcpt = m_tierspayants->find(id);
    if( itcpt == m_tierspayants->constEnd() )
        return Q_NULLPTR;
    return itcpt.value();
}

TypeTiers::TypeTiers(QJsonObject data, QObject *parent) : Item(parent)
{
    setData(data);
}

QString TypeTiers::typetiers() const    { return m_typetiers; }

void TypeTiers::setData(QJsonObject data)
{
    if( data.isEmpty() )
        return;
    setDataString(data, "typetiers", m_typetiers);
}


TypesTiers::TypesTiers()
{
    m_typestiers = new QList<TypeTiers *>();
}

QList<TypeTiers *> *TypesTiers::typestiers() const
{
    return m_typestiers;
}

void TypesTiers::addTypeTiers(TypeTiers *typetiers)
{
    if( m_typestiers->contains(typetiers) )
        return;
    *m_typestiers << typetiers;
}

void TypesTiers::addTypeTiers(QList<TypeTiers*> listTypesTiers)
{
    QList<TypeTiers*>::const_iterator it;
    for( it = listTypesTiers.constBegin(); it != listTypesTiers.constEnd(); ++it )
        addTypeTiers( *it );
}

void TypesTiers::removeTypeTiers(TypeTiers* typetiers)
{
    while( m_typestiers->contains(typetiers) )
        m_typestiers->removeOne(typetiers);
}

