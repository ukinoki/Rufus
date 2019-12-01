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

#include "cls_posteconnecte.h"

PosteConnecte::PosteConnecte(QJsonObject data, QObject *parent) : Item(parent)
{
    setData(data);
}

void PosteConnecte::setData(QJsonObject data)
{
    if( data.isEmpty() )
        return;

    Utils::setDataInt(data, CP_IDUSER_USRCONNECT, m_id);
    Utils::setDataString(data, CP_NOMPOSTE_USRCONNECT, m_nomposte);
    Utils::setDataString(data, CP_MACADRESS_USRCONNECT, m_macadress);
    Utils::setDataString(data, CP_IPADRESS_USRCONNECT, m_ipadress);
    Utils::setDataBool(data, CP_DISTANT_USRCONNECT, m_accesdistant);
    Utils::setDataInt(data, CP_IDUSERSUPERVISEUR_USRCONNECT, m_idsuperviseur);
    Utils::setDataInt(data, CP_IDUSERPARENT_USRCONNECT, m_idparent);
    Utils::setDataInt(data, CP_IDUSERCOMPTABLE_USRCONNECT, m_idcomptable);
    Utils::setDataInt(data, CP_IDLIEU_USRCONNECT, m_idlieu);
    Utils::setDataInt(data, CP_IDPATENCOURS_USRCONNECT, m_idpatencours);
    Utils::setDataDateTime(data, CP_HEUREDERNIERECONNECTION_USRCONNECT, m_dateheurederniereconnexion);
    Utils::setDataString(data, "stringid", m_stringid);
    m_data = data;
}
