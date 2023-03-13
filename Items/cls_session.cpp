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

#include "cls_session.h"

int Session::ROLE_NON_RENSEIGNE = -1;
int Session::ROLE_VIDE = -2;
int Session::ROLE_INDETERMINE = -3;

Session::Session(QJsonObject data, QObject *parent) : Item(parent)
{
    setData(data);
}

void Session::setData(QJsonObject data)
{
    if( data.isEmpty() )
        return;

    Utils::setDataInt(data, CP_ID_SESSIONS, m_id);
    Utils::setDataInt(data, CP_IDUSER_SESSIONS, m_iduser);
    Utils::setDataInt(data, CP_IDSUPERVISEUR_SESSIONS, m_idUserSuperviseur);
    Utils::setDataInt(data, CP_IDPARENT_SESSIONS, m_idUserParent);
    Utils::setDataInt(data, CP_IDCOMPTABLE_SESSIONS, m_idUserComptable);
    Utils::setDataInt(data, CP_IDLIEU_SESSIONS, m_idlieu);
    Utils::setDataDateTime(data,CP_DATEDEBUT_SESSIONS, m_datedebut);
    Utils::setDataDateTime(data,CP_DATEFIN_SESSIONS, m_datefin);
    m_data = data;
}

