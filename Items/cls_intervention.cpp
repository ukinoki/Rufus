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
    Utils::setDataDate(data, CP_DATE_LIGNPRGOPERATOIRE, m_date);
    Utils::setDataInt(data, CP_IDUSER_LIGNPRGOPERATOIRE, m_iduser);
    Utils::setDataInt(data, CP_IDPATIENT_LIGNPRGOPERATOIRE, m_idpatient);
    Utils::setDataInt(data, CP_IDLIEU_LIGNPRGOPERATOIRE, m_idlieu);
    m_anesth = ConvertModeAnesthesie(data[CP_TYPEANESTH_LIGNPRGOPERATOIRE].toString());
    m_cote   = Utils::ConvertCote(data[CP_COTE_LIGNPRGOPERATOIRE].toString());
    Utils::setDataInt(data, CP_IDIOL_LIGNPRGOPERATOIRE, m_idIOL);
    Utils::setDataDouble(data, CP_PWRIOL_LIGNPRGOPERATOIRE, m_pwrIOL);
    Utils::setDataDouble(data, CP_CYLIOL_LIGNPRGOPERATOIRE, m_cylIOL);
    Utils::setDataString(data, CP_OBSERV_LIGNPRGOPERATOIRE, m_observation);
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
    data[CP_ID_LIGNPRGOPERATOIRE]           = 0;
    data[CP_DATE_LIGNPRGOPERATOIRE]         = "";
    data[CP_IDUSER_LIGNPRGOPERATOIRE]       = 0;
    data[CP_IDPATIENT_LIGNPRGOPERATOIRE]    = 0;
    data[CP_IDLIEU_LIGNPRGOPERATOIRE]       = 0;
    data[CP_TYPEANESTH_LIGNPRGOPERATOIRE]   = "";
    data[CP_COTE_LIGNPRGOPERATOIRE]         = "";
    data[CP_IDIOL_LIGNPRGOPERATOIRE]        = 0;
    data[CP_PWRIOL_LIGNPRGOPERATOIRE]       = 0;
    data[CP_CYLIOL_LIGNPRGOPERATOIRE]       = 0;
    data[CP_OBSERV_LIGNPRGOPERATOIRE]       = 0;

    setData(data);

    m_data = data;
}

IOL::IOL(QJsonObject data, QObject *parent) : Item(parent)
{
    resetdatas();
    setData(data);
}

void IOL::setData(QJsonObject data)
{
    if( data.isEmpty() )
        return;

    Utils::setDataInt(data, CP_ID_IOLS, m_id);
    Utils::setDataInt(data, CP_IDMANUFACTURER_IOLS, m_idmanufacturer);
    Utils::setDataString(data, CP_MODELNAME_IOLS, m_modele);
    m_data = data;
}

void IOL::resetdatas()
{
    QJsonObject data;
    data[CP_ID_IOLS]                = 0;
    data[CP_IDMANUFACTURER_IOLS]    = 0;
    data[CP_MODELNAME_IOLS]         = "";
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
    m_data = data;
}

void TypeIntervention::resetdatas()
{
    QJsonObject data;
    data[CP_ID_TYPINTERVENTION]                 = 0;
    data[CP_TYPEINTERVENTION_TYPINTERVENTION]   = "";
    data[CP_CODECCAM_TYPINTERVENTION]           = "";
    setData(data);
}
