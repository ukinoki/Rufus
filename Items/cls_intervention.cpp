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

IOL::IOL(QMap<QString, QVariant> map, QObject *parent) : Item(parent)
{
    resetdatas();
    setData(map);
}

void IOL::setData(QMap<QString, QVariant> map)
{
    if( map.isEmpty() )
        return;
//    qDebug() << "map[CP_MODELNAME_IOLS]" << map[CP_MODELNAME_IOLS];
//    qDebug() << "map[CP_ACD_IOLS]" << map[CP_ACD_IOLS];
//    qDebug() << "map[CP_IDMANUFACTURER_IOLS]" << map[CP_IDMANUFACTURER_IOLS];
//    qDebug() << "map[CP_INACTIF_IOLS]" << map[CP_INACTIF_IOLS];
    m_id                = map[CP_ID_IOLS].toInt();
    m_idmanufacturer    = map[CP_IDMANUFACTURER_IOLS].toInt();
    m_modele            = map[CP_MODELNAME_IOLS].toString();
    m_inactif           = map[CP_INACTIF_IOLS].toBool();
    m_acd               = map[CP_ACD_IOLS].toDouble();
    m_pwrmin            = map[CP_MINPWR_IOLS].toDouble();
    m_pwrmax            = map[CP_MAXPWR_IOLS].toDouble();
    m_pwrstp            = map[CP_PWRSTEP_IOLS].toDouble();
    m_cylmin            = map[CP_MINCYL_IOLS].toDouble();
    m_cylmax            = map[CP_MAXCYL_IOLS].toDouble();
    m_cylstp            = map[CP_CYLSTEP_IOLS].toDouble();
    m_csteAopt          = map[CP_CSTEAOPT_IOLS].toDouble();
    m_csteAEcho         = map[CP_CSTEAECHO_IOLS].toDouble();
    m_haigisa0          = map[CP_HAIGISA0_IOLS].toDouble();
    m_haigisa1          = map[CP_HAIGISA1_IOLS].toDouble();
    m_haigisa2          = map[CP_HAIGISA2_IOLS].toDouble();
    m_holladay          = map[CP_HOLL1_IOLS].toDouble();
    m_diainjecteur      = map[CP_DIAINJECTEUR_IOLS].toDouble();
    m_diaall            = map[CP_DIAALL_IOLS].toDouble();
    m_diaoptique        = map[CP_DIAOPT_IOLS].toDouble();
    m_imgiol            = map[CP_IMG_IOLS].toByteArray();
    m_imageformat       = map[CP_TYPIMG_IOLS].toString();
    m_materiau          = map[CP_MATERIAU_IOLS].toString();
    m_remarque          = map[CP_REMARQUE_IOLS].toString();
    m_precharge         = map[CP_PRECHARGE_IOLS].toBool();
    m_multifocal        = map[CP_MULTIFOCAL_IOLS].toBool();
    m_jaune             = map[CP_JAUNE_IOLS].toBool();
    m_edof              = map[CP_EDOF_IOLS].toBool();
    m_toric             = map[CP_TORIC_IOLS].toBool();
    m_map = map;
}

QString IOL::tooltip() const
{
    QString ttip = modele();
    if (csteAopt() != 0.0)
        ttip += "\ncsteA opt. " + QString::number(csteAopt(), 'f', 1);
    if (csteAEcho() != 0.0)
        ttip += "\ncsteA echo " + QString::number(csteAEcho(), 'f', 1);
    if (acd() != 0.0)
        ttip += "\nACD " + QString::number(acd(), 'f', 2);
    if (haigisa0() != 0.0 && haigisa1() != 0.0 && haigisa2() != 0.0)
        ttip += "\n Haigis a: " + QString::number(haigisa0(), 'f', 4) + " - b: "  + QString::number(haigisa1(), 'f', 4) + " - c: "  + QString::number(haigisa2(), 'f', 4);
    if (materiau() != "")
        ttip += "\n" + tr("materiau") + " " + materiau();
    if (remarque() != "")
        ttip += "\n" + tr("remarque") + " " + remarque();
    return ttip;
}

void IOL::resetdatas()
{
    QMap<QString, QVariant> map;
    map[CP_ID_IOLS]                 = 0;
    map[CP_IDMANUFACTURER_IOLS]     = 0;
    map[CP_MODELNAME_IOLS]          = "";
    map[CP_INACTIF_IOLS]            = false;
    map[CP_ACD_IOLS]                = 0;
    map[CP_MINPWR_IOLS]             = 0;
    map[CP_MAXPWR_IOLS]             = 0;
    map[CP_PWRSTEP_IOLS]            = 0;
    map[CP_MINCYL_IOLS]             = 0;
    map[CP_MAXCYL_IOLS]             = 0;
    map[CP_CYLSTEP_IOLS]            = 0;
    map[CP_CSTEAOPT_IOLS]           = 0;
    map[CP_CSTEAECHO_IOLS]          = 0;
    map[CP_HAIGISA0_IOLS]           = 0;
    map[CP_HAIGISA1_IOLS]           = 0;
    map[CP_HAIGISA2_IOLS]           = 0;
    map[CP_HOLL1_IOLS]              = 0;
    map[CP_DIAINJECTEUR_IOLS]       = 0;
    map[CP_DIAALL_IOLS]             = 0;
    map[CP_DIAOPT_IOLS]             = 0;
    map[CP_IMG_IOLS]                = QVariant();
    map[CP_TYPIMG_IOLS]             = "";
    map[CP_MATERIAU_IOLS]           = "";
    map[CP_REMARQUE_IOLS]           = "";
    map[CP_PRECHARGE_IOLS]          = false;
    map[CP_JAUNE_IOLS]              = false;
    map[CP_MULTIFOCAL_IOLS]         = false;
    map[CP_EDOF_IOLS]               = false;
    map[CP_TORIC_IOLS]              = false;
    setData(map);
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


