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

#include "cls_banque.h"

Banque::Banque(QJsonObject data, QObject *parent) : Item(parent)
{
    setData(data);
}

int Banque::code() const            { return m_codebanque; }
QString Banque::nom() const         { return m_nombanque; }
QString Banque::nomabrege() const   { return m_idbanqueabrege; }

void Banque::setData(QJsonObject data)
{
    if( data.isEmpty() )
        return;
    Utils::setDataInt(data, CP_ID_BANQUES, m_id);
    Utils::setDataString(data, CP_NOMBANQUE_BANQUES, m_nombanque);
    Utils::setDataString(data, CP_NOMABREGE_BANQUES, m_idbanqueabrege);
    Utils::setDataInt(data, CP_CODE_BANQUES, m_codebanque);
    m_data = data;
}


