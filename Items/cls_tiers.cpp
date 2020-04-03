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

#include "cls_tiers.h"

Tiers::Tiers(QJsonObject data, QObject *parent) : Item(parent)
{
    setData(data);
}

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
    Utils::setDataInt(data, "id", m_id);
    Utils::setDataString(data, "nomtiers", m_nomtiers);
    Utils::setDataString(data, "adressetiers", m_adressetiers);
    Utils::setDataString(data, "codepostaltiers", m_codepostaltiers);
    Utils::setDataString(data, "villetiers", m_villetiers);
    Utils::setDataString(data, "telephonetiers", m_telephonetiers);
    Utils::setDataString(data, "faxtiers", m_faxtiers);
    m_data = data;
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
    Utils::setDataString(data, "typetiers", m_typetiers);
}

