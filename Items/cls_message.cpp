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

#include "cls_message.h"

Message::Message(QJsonObject data, QObject *parent) : Item(parent)
{
    setData(data);
}

void Message::setData(QJsonObject data)
{
    if( data.isEmpty() )
        return;

    Utils::setDataInt(data, CP_ID_MSG, m_id);
    Utils::setDataInt(data, CP_IDEMETTEUR_MSG, m_idemetteur);
    Utils::setDataString(data, CP_TEXT_MSG, m_text);
    Utils::setDataInt(data, CP_IDPATIENT_MSG, m_idpatient);
    Utils::setDataBool(data, CP_TACHE_MSG, m_istache);
    Utils::setDataDate(data, CP_DATELIMITE_MSG, m_datelimite);
    Utils::setDataDateTime(data, CP_DATECREATION_MSG, m_datecreation);
    Utils::setDataBool(data, CP_URGENT_MSG, m_isurgent);
    Utils::setDataInt(data, CP_ENREPONSEA_MSG, m_idreponsea);
    Utils::setDataBool(data, CP_ASUPPRIMER_MSG, m_isasupprimer);
    Utils::setDataBool(data, CP_LU_JOINTURESMSG, m_islu);
    Utils::setDataBool(data, CP_FAIT_JOINTURESMSG, m_isfait);
    Utils::setDataInt(data, CP_ID_JOINTURESMSG, m_idjointure);
    Utils::setDataInt(data, CP_IDDESTINATAIRE_JOINTURESMSG, m_iddestinataire);
    m_data = data;
}

void Message::resetdatas()
{
    QJsonObject data;
    data[CP_ID_MSG]             = 0;
    data[CP_IDEMETTEUR_MSG]     = 0;
    data[CP_TEXT_MSG]           = "";
    data[CP_IDPATIENT_MSG]      = 0;
    data[CP_TACHE_MSG]          = false;
    data[CP_DATELIMITE_MSG]     = QDate(1900,1,1).toString("yyyy-MM-dd");
    data[CP_DATECREATION_MSG]   = QDateTime(QDate(1900,1,1),QTime(0,0)).toMSecsSinceEpoch();
    data[CP_URGENT_MSG]         = false;
    data[CP_ENREPONSEA_MSG]     = 0;
    data[CP_ASUPPRIMER_MSG]     = false;
    data[CP_LU_JOINTURESMSG]    = false;
    data[CP_FAIT_JOINTURESMSG]  = false;
    data[CP_ID_JOINTURESMSG]    = 0;
    data[CP_IDDESTINATAIRE_JOINTURESMSG]    = 0;
    setData(data);
}

