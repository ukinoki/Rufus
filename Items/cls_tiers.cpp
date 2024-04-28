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
    resetdatas();
    setData(data);
}

QString Tiers::nom() const         { return m_nom; }
QString Tiers::adresse1() const    { return m_adresse1; }
QString Tiers::adresse2() const    { return m_adresse2; }
QString Tiers::adresse3() const    { return m_adresse3; }
QString Tiers::codepostal() const  { return m_codepostal; }
QString Tiers::ville() const       { return m_ville; }
QString Tiers::telephone() const   { return m_telephone; }
QString Tiers::fax() const         { return m_fax; }
QString Tiers::mail() const        { return m_mail; }
QString Tiers::website() const     { return m_web; }

void Tiers::setData(QJsonObject data)
{
    if( data.isEmpty() )
        return;
    setDataInt(data,     CP_ID_TIERS,        m_id);
    setDataString(data,  CP_NOM_TIERS,       m_nom);
    setDataString(data,  CP_ADRESSE1_TIERS,  m_adresse1);
    setDataString(data,  CP_ADRESSE2_TIERS,  m_adresse2);
    setDataString(data,  CP_ADRESSE3_TIERS,  m_adresse3);
    setDataString(data,  CP_CODEPOSTAL_TIERS,m_codepostal);
    setDataString(data,  CP_VILLE_TIERS,     m_ville);
    setDataString(data,  CP_TELEPHONE_TIERS, m_telephone);
    setDataString(data,  CP_FAX_TIERS,       m_fax);
    setDataString(data,  CP_MAIL_TIERS,      m_mail);
    setDataString(data,  CP_WEB_TIERS,       m_web);
    m_data = data;
}

/*! comme son nom l'indique */
void Tiers::resetdatas()
{
    QJsonObject data;
    data[CP_ID_TIERS]           = "";
    data[CP_NOM_TIERS]          = "";
    data[CP_ADRESSE1_TIERS]     = "";
    data[CP_ADRESSE2_TIERS]     = "";
    data[CP_ADRESSE3_TIERS]     = "";
    data[CP_CODEPOSTAL_TIERS]   = "";
    data[CP_VILLE_TIERS]        = "";
    data[CP_TELEPHONE_TIERS]    = "";
    data[CP_FAX_TIERS]          = "";
    setData(data);
}

QString Tiers::tooltip() const
{
    QString ttip = "";
    if (telephone() != "")
        ttip += tr("Telephone:") + " " + telephone();
    if (adresse1() != "")
        ttip += "\n" + adresse1() + " " + codepostal() + " " + ville();
    return ttip;
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

