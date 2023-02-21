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

#include "cls_intervention.h"

Intervention::Intervention(QJsonObject data, QObject *parent) : Item(parent)
{
    resetdatas();
    setData(data);
}

void Intervention::setData(QJsonObject data)
{
    if( data.isEmpty() )
        return;

    Utils::setDataInt(data, CP_ID_LIGNPRGOPERATOIRE, m_id);
    Utils::setDataInt(data, CP_IDSESSION_LIGNPRGOPERATOIRE, m_idsession);
    Utils::setDataInt(data, CP_IDACTE_LIGNPRGOPERATOIRE, m_idacte);
    Utils::setDataInt(data, CP_IDPATIENT_LIGNPRGOPERATOIRE, m_idpatient);
    Utils::setDataTime(data, CP_HEURE_LIGNPRGOPERATOIRE, m_heure);
    m_anesth = ConvertModeAnesthesie(data[CP_TYPEANESTH_LIGNPRGOPERATOIRE].toString());
    m_cote   = Utils::ConvertCote(data[CP_COTE_LIGNPRGOPERATOIRE].toString());
    Utils::setDataInt(data, CP_IDTYPEINTERVENTION_LIGNPRGOPERATOIRE, m_idtypeintervention);
    Utils::setDataInt(data, CP_IDIOL_LIGNPRGOPERATOIRE, m_idIOL);
    Utils::setDataDouble(data, CP_PWRIOL_LIGNPRGOPERATOIRE, m_pwrIOL);
    Utils::setDataDouble(data, CP_CYLIOL_LIGNPRGOPERATOIRE, m_cylIOL);
    Utils::setDataString(data, CP_OBSERV_LIGNPRGOPERATOIRE, m_observation);
    Utils::setDataString(data, CP_INCIDENT_LIGNPRGOPERATOIRE, m_incident);
    m_data = data;
}

Intervention::ModeAnesthesie Intervention::ConvertModeAnesthesie(QString mode)
{
    if (mode == "L") return Locale;
    if (mode == "R") return LocoRegionale;
    if (mode == "G") return Generale;
    return  NoLoSo;
}

QString Intervention::ConvertModeAnesthesie(ModeAnesthesie mode)
{
    switch (mode) {
    case Locale:        return "L";
    case LocoRegionale: return "R";
    case Generale:      return "G";
    default: return "";
    }
}

/*! comme son nom l'indique */
void Intervention::resetdatas()
{
    QJsonObject data;
    data[CP_ID_LIGNPRGOPERATOIRE]                   = 0;
    data[CP_HEURE_LIGNPRGOPERATOIRE]                = "";
    data[CP_IDSESSION_LIGNPRGOPERATOIRE]            = 0;
    data[CP_IDACTE_LIGNPRGOPERATOIRE]               = 0;
    data[CP_IDPATIENT_LIGNPRGOPERATOIRE]            = 0;
    data[CP_TYPEANESTH_LIGNPRGOPERATOIRE]           = "";
    data[CP_IDTYPEINTERVENTION_LIGNPRGOPERATOIRE]   = 0;
    data[CP_COTE_LIGNPRGOPERATOIRE]                 = "";
    data[CP_IDIOL_LIGNPRGOPERATOIRE]                = 0;
    data[CP_PWRIOL_LIGNPRGOPERATOIRE]               = 0;
    data[CP_CYLIOL_LIGNPRGOPERATOIRE]               = 0;
    data[CP_OBSERV_LIGNPRGOPERATOIRE]               = "";
    data[CP_INCIDENT_LIGNPRGOPERATOIRE]             = "";
    setData(data);
}

SessionOperatoire::SessionOperatoire(QJsonObject data, QObject *parent) : Item(parent)
{
    resetdatas();
    setData(data);
}

TypeIntervention::TypeIntervention(QJsonObject data, QObject *parent) : Item(parent)
{
    resetdatas();
    setData(data);
}

void TypeIntervention::setData(QJsonObject data)
{
    if( data.isEmpty() )
        return;
    Utils::setDataInt(data, CP_ID_TYPINTERVENTION, m_id);
    Utils::setDataString(data, CP_TYPEINTERVENTION_TYPINTERVENTION, m_typeintervention);
    Utils::setDataString(data, CP_CODECCAM_TYPINTERVENTION, m_codeCCAM);
    Utils::setDataTime(data, CP_DUREE_TYPINTERVENTION, m_duree);
    m_data = data;
}

void TypeIntervention::resetdatas()
{
    QJsonObject data;
    data[CP_ID_TYPINTERVENTION]                 = 0;
    data[CP_TYPEINTERVENTION_TYPINTERVENTION]   = "";
    data[CP_CODECCAM_TYPINTERVENTION]           = "";
    data[CP_DUREE_TYPINTERVENTION]              = "";
    setData(data);
}


