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

    setDataInt(data, CP_ID_DATAOPHTA, m_idsubjectif);
    setDataInt(data, CP_IDPATIENT_DATAOPHTA, m_idpat);
    setDataString(data, CP_MESURE_DATAOPHTA, m_mesure);
    setDataString(data, CP_DISTANCE_DATAOPHTA, m_mesure);
    setDataDouble(data, CP_K1OD_DATAOPHTA, m_K1OD);
    setDataDouble(data, CP_K2OD_DATAOPHTA, m_K2OD);
    setDataInt(data, CP_AXEKOD_DATAOPHTA, m_axeKOD);
    setDataDouble(data, CP_K1OG_DATAOPHTA, m_K1OG);
    setDataDouble(data, CP_K2OG_DATAOPHTA, m_K2OG);
    setDataInt(data, CP_AXEKOG_DATAOPHTA, m_axeKOG);
    setDataString(data, CP_MODEMESUREKERATO_DATAOPHTA, m_origineK);
    setDataDate(data, CP_DATEKERATO_DATAOPHTA, m_dateK);
    setDataDouble(data, CP_SPHEREOD_DATAOPHTA, m_sphereOD);
    setDataDouble(data, CP_CYLINDREOD_DATAOPHTA, m_cylindreOD);
    setDataInt(data, CP_AXECYLINDREOD_DATAOPHTA, m_axecylindreOD);
    setDataString(data, CP_AVLOD_DATAOPHTA, m_AVLOD);
    setDataDouble(data, CP_ADDVPOD_DATAOPHTA, m_addVPOD);
    setDataString(data, CP_AVPOD_DATAOPHTA, m_AVPOD);
    setDataDate(data, CP_DATEREFRACTIONOD_DATAOPHTA, m_daterefOD);
    setDataDouble(data, CP_SPHEREOG_DATAOPHTA, m_sphereOG);
    setDataDouble(data, CP_CYLINDREOG_DATAOPHTA, m_cylindreOG);
    setDataInt(data, CP_AXECYLINDREOG_DATAOPHTA, m_axecylindreOG);
    setDataString(data, CP_AVLOG_DATAOPHTA, m_AVLOG);
    setDataDouble(data, CP_ADDVPOG_DATAOPHTA, m_addVPOG);
    setDataString(data, CP_AVPOG_DATAOPHTA, m_AVPOG);
    setDataDate(data, CP_DATEREFRACTIONOG_DATAOPHTA, m_daterefOG);
    setDataInt(data, CP_ECARTIP_DATAOPHTA, m_ecartIP);
    setDataDouble(data, CP_DIOTRIESK1OD_DATAOPHTA, m_dioptriesK1OD);
    setDataDouble(data, CP_DIOTRIESK2OD_DATAOPHTA, m_dioptriesK2OD);
    setDataDouble(data, CP_DIOTRIESK1OG_DATAOPHTA, m_dioptriesK1OG);
    setDataDouble(data, CP_DIOTRIESK2OG_DATAOPHTA, m_dioptriesK2OG);

    setDataInt(data, CP_ID_DATAOPHTA "A", m_idautoref);
    setDataDouble(data, CP_SPHEREOD_DATAOPHTA "A", m_sphereODautoref);
    setDataDouble(data, CP_CYLINDREOD_DATAOPHTA "A", m_cylindreODautoref);
    setDataInt(data, CP_AXECYLINDREOD_DATAOPHTA "A", m_axecylindreODautoref);
    setDataDate(data, CP_DATEREFRACTIONOD_DATAOPHTA "A", m_daterefODautoref);
    setDataDouble(data, CP_SPHEREOG_DATAOPHTA "A", m_sphereOGautoref);
    setDataDouble(data, CP_CYLINDREOG_DATAOPHTA "A", m_cylindreOGautoref);
    setDataInt(data, CP_AXECYLINDREOG_DATAOPHTA "A", m_axecylindreOGautoref);
    setDataDate(data, CP_DATEREFRACTIONOG_DATAOPHTA "A", m_daterefOGautoref);
    setDataInt(data, CP_ECARTIP_DATAOPHTA "A", m_ecartIPautoref);
    m_data = data;
}

