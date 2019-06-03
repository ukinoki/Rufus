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

#include "cls_patientencours.h"

PatientEnCours::PatientEnCours(QJsonObject data, QObject *parent) : Item(parent)
{
    setData(data);
}

void PatientEnCours::setData(QJsonObject data)
{
    if( data.isEmpty() )
        return;
    setDataInt(data, "id", m_id);
    setDataInt(data, "iduser", m_iduser);
    setDataString(data, "statut", m_statut);
    setDataTime(data, "heurestatut", m_heurestatut);
    setDataTime(data, "heurerdv", m_heurerdv);
    setDataTime(data, "heurerarrivee", m_heurearrivee);
    setDataString(data, "motif", m_motif);
    setDataString(data, "message", m_message);
    setDataInt(data, "idacteapayer", m_idacteapayer);
    setDataString(data, "posteexamen", m_posteexamen);
    setDataInt(data, "iduserencoursexam", m_iduserencoursexam);
    setDataInt(data, "idsaldat", m_idsaldat);
    m_data = data;
}


