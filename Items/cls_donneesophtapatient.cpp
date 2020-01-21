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

#include "cls_donneesophtapatient.h"

DonneesOphtaPatient::DonneesOphtaPatient()
{

}

void DonneesOphtaPatient::setData(QJsonObject data)
{
    if( data.isEmpty() )
        return;

    Utils::setDataInt(data, CP_ID_DATAOPHTA, m_idsubjectif);
    Utils::setDataInt(data, CP_IDPATIENT_DATAOPHTA, m_idpat);
    Utils::setDataString(data, CP_MESURE_DATAOPHTA, m_mesure);
    Utils::setDataString(data, CP_DISTANCE_DATAOPHTA, m_mesure);
    Utils::setDataDouble(data, CP_K1OD_DATAOPHTA, m_K1OD);
    Utils::setDataDouble(data, CP_K2OD_DATAOPHTA, m_K2OD);
    Utils::setDataInt(data, CP_AXEKOD_DATAOPHTA, m_axeKOD);
    Utils::setDataDouble(data, CP_K1OG_DATAOPHTA, m_K1OG);
    Utils::setDataDouble(data, CP_K2OG_DATAOPHTA, m_K2OG);
    Utils::setDataInt(data, CP_AXEKOG_DATAOPHTA, m_axeKOG);
    Utils::setDataString(data, CP_MODEMESUREKERATO_DATAOPHTA, m_origineK);
    Utils::setDataDate(data, CP_DATEKERATO_DATAOPHTA, m_dateK);
    Utils::setDataDouble(data, CP_SPHEREOD_DATAOPHTA, m_sphereOD);
    Utils::setDataDouble(data, CP_CYLINDREOD_DATAOPHTA, m_cylindreOD);
    Utils::setDataInt(data, CP_AXECYLINDREOD_DATAOPHTA, m_axecylindreOD);
    Utils::setDataString(data, CP_AVLOD_DATAOPHTA, m_AVLOD);
    Utils::setDataDouble(data, CP_ADDVPOD_DATAOPHTA, m_addVPOD);
    Utils::setDataString(data, CP_AVPOD_DATAOPHTA, m_AVPOD);
    Utils::setDataDate(data, CP_DATEREFRACTIONOD_DATAOPHTA, m_daterefOD);
    Utils::setDataDouble(data, CP_SPHEREOG_DATAOPHTA, m_sphereOG);
    Utils::setDataDouble(data, CP_CYLINDREOG_DATAOPHTA, m_cylindreOG);
    Utils::setDataInt(data, CP_AXECYLINDREOG_DATAOPHTA, m_axecylindreOG);
    Utils::setDataString(data, CP_AVLOG_DATAOPHTA, m_AVLOG);
    Utils::setDataDouble(data, CP_ADDVPOG_DATAOPHTA, m_addVPOG);
    Utils::setDataString(data, CP_AVPOG_DATAOPHTA, m_AVPOG);
    Utils::setDataDate(data, CP_DATEREFRACTIONOG_DATAOPHTA, m_daterefOG);
    Utils::setDataInt(data, CP_ECARTIP_DATAOPHTA, m_ecartIP);
    Utils::setDataDouble(data, CP_DIOTRIESK1OD_DATAOPHTA, m_dioptriesK1OD);
    Utils::setDataDouble(data, CP_DIOTRIESK2OD_DATAOPHTA, m_dioptriesK2OD);
    Utils::setDataDouble(data, CP_DIOTRIESK1OG_DATAOPHTA, m_dioptriesK1OG);
    Utils::setDataDouble(data, CP_DIOTRIESK2OG_DATAOPHTA, m_dioptriesK2OG);

    Utils::setDataInt(data, CP_ID_DATAOPHTA "A", m_idautoref);
    Utils::setDataDouble(data, CP_SPHEREOD_DATAOPHTA "A", m_sphereODautoref);
    Utils::setDataDouble(data, CP_CYLINDREOD_DATAOPHTA "A", m_cylindreODautoref);
    Utils::setDataInt(data, CP_AXECYLINDREOD_DATAOPHTA "A", m_axecylindreODautoref);
    Utils::setDataDate(data, CP_DATEREFRACTIONOD_DATAOPHTA "A", m_daterefODautoref);
    Utils::setDataDouble(data, CP_SPHEREOG_DATAOPHTA "A", m_sphereOGautoref);
    Utils::setDataDouble(data, CP_CYLINDREOG_DATAOPHTA "A", m_cylindreOGautoref);
    Utils::setDataInt(data, CP_AXECYLINDREOG_DATAOPHTA "A", m_axecylindreOGautoref);
    Utils::setDataDate(data, CP_DATEREFRACTIONOG_DATAOPHTA "A", m_daterefOGautoref);
    Utils::setDataInt(data, CP_ECARTIP_DATAOPHTA "A", m_ecartIPautoref);
    m_data = data;
}

