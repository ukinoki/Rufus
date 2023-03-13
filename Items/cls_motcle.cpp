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

#include "cls_motcle.h"

MotCle::MotCle(QJsonObject data, QObject *parent) : Item(parent)
{
    setData(data);
}

void MotCle::setData(QJsonObject data)
{
    if( data.isEmpty() )
        return;

    Utils::setDataInt(data, CP_ID_MOTCLE, m_id);
    Utils::setDataString(data, CP_TEXT_MOTCLE, m_motcle);
    m_data = data;
}

/*! comme son nom l'indique */
void MotCle::resetdatas()
{
    QJsonObject data;
    data[CP_ID_MOTCLE]          = 0;
    data[CP_TEXT_MOTCLE]        = "";
    setData(data);
}


