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
    Utils::setDataInt(data, CP_IDPAT_SALDAT, m_id);
    Utils::setDataInt(data, CP_IDUSER_SALDAT, m_iduser);
    Utils::setDataString(data, CP_STATUT_SALDAT, m_statut);
    Utils::setDataTime(data, CP_HEURESTATUT_SALDAT, m_heurestatut);
    Utils::setDataTime(data, CP_HEURERDV_SALDAT , m_heurerdv);
    Utils::setDataTime(data, CP_HEUREARRIVEE_SALDAT, m_heurearrivee);
    Utils::setDataString(data, CP_MOTIF_SALDAT, m_motif);
    Utils::setDataString(data, CP_MESSAGE_SALDAT, m_message);
    Utils::setDataInt(data, CP_IDACTEAPAYER_SALDAT, m_idacteapayer);
    Utils::setDataString(data, CP_POSTEEXAMEN_SALDAT, m_posteexamen);
    Utils::setDataInt(data, CP_IDUSERENCOURSEXAM_SALDAT, m_iduserencoursexam);
    Utils::setDataInt(data, CP_IDSALDAT_SALDAT, m_idsaldat);
    m_data = data;
}

