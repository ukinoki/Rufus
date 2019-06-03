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

#include "cls_banque.h"

Banque::Banque(QJsonObject data, QObject *parent) : Item(parent)
{
    setData(data);
}

int Banque::CodeBanque() const            { return m_codebanque; }
QString Banque::NomBanque() const         { return m_nombanque; }
QString Banque::NomBanqueAbrege() const   { return m_idbanqueabrege; }

void Banque::setData(QJsonObject data)
{
    if( data.isEmpty() )
        return;
    setDataInt(data, "id", m_id);
    setDataString(data, "nombanque", m_nombanque);
    setDataString(data, "idbanqueabrege", m_idbanqueabrege);
    setDataInt(data, "codebanque", m_codebanque);
    m_data = data;
}


