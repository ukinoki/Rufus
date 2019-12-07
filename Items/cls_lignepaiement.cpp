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

#include "cls_lignepaiement.h"

LignePaiement::LignePaiement(QJsonObject data, QObject *parent) : Item(parent)
{
    setData(data);
}

int LignePaiement::idrecette() const            { return m_stringid.split(TCPMSG_Separator).at(1).toInt(); }
int LignePaiement::idacte() const               { return m_stringid.split(TCPMSG_Separator).at(0).toInt(); }
double LignePaiement::paye() const              { return m_paye; }
QString LignePaiement::monnaie() const          { return m_monnaie; }

void LignePaiement::setData(QJsonObject data)
{
    if( data.isEmpty() )
        return;
    setDataString(data, "stringid", m_stringid);
    setDataDouble(data, "paye", m_paye);
    setDataString(data, "monnaie", m_monnaie);
    m_data = data;
}
