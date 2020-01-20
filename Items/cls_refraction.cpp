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

#include "cls_refraction.h"

Refraction::Refraction(QJsonObject data, QObject *parent) : Item(parent)
{
    if (data == QJsonObject())
    {
        m_data[CP_SPHEREOD_REFRACTIONS]           = 0;
        m_data[CP_CYLINDREOD_REFRACTIONS]         = 0;
        m_data[CP_AXECYLOD_REFRACTIONS]           = 0;
        m_data[CP_ADDVPOD_REFRACTIONS]            = 0;
        m_data[CP_PRISMEOD_REFRACTIONS]           = 0;
        m_data[CP_BASEPRISMEOD_REFRACTIONS]       = 0;
        m_data[CP_RYSEROD_REFRACTIONS]            = 0;
        m_data[CP_SPHEREOG_REFRACTIONS]           = 0;
        m_data[CP_CYLINDREOG_REFRACTIONS]         = 0;
        m_data[CP_AXECYLOG_REFRACTIONS]           = 0;
        m_data[CP_ADDVPOG_REFRACTIONS]            = 0;
        m_data[CP_PRISMEOG_REFRACTIONS]           = 0;
        m_data[CP_BASEPRISMEOG_REFRACTIONS]       = 0;
        m_data[CP_RYSEROG_REFRACTIONS]            = 0;
    }
    setData(data);
}
void Refraction::setData(QJsonObject data)
{
    if( data.isEmpty() )
        return;
    Utils::setDataInt(data, CP_ID_REFRACTIONS, m_id);

    Utils::setDataInt(data, CP_IDPAT_REFRACTIONS, m_idpat);                            //!> l'id du patient
    Utils::setDataInt(data, CP_IDACTE_REFRACTIONS, m_idacte);                          //!> l'id de l'acte aucours duquel la mesure a été faite
    Utils::setDataDate(data, CP_DATE_REFRACTIONS, m_daterefraction);                   //!> la date de la refraction
    m_typemesure = ConvertMesure(data[CP_TYPEMESURE_REFRACTIONS].toString());          //!> le type de mesure effectuée : frontofocometre, autorfractomètre, acuité ou prescription
    m_distance = ConvertDistance(data[CP_DISTANCEMESURE_REFRACTIONS].toString());      //!> la distance de mesure: loin, près, les 2
    if (data.contains(CP_CYCLOPLEGIE_REFRACTIONS))
        m_dilate = (data[CP_CYCLOPLEGIE_REFRACTIONS].toBool()? Dilatation : NoDilatation);
    Utils::setDataBool(data, CP_ODMESURE_REFRACTIONS, m_isODmesure);                   //!> l'OD a été mesuré
    Utils::setDataDouble(data, CP_SPHEREOD_REFRACTIONS, m_sphereOD);                   //!> sphere OD
    Utils::setDataDouble(data, CP_CYLINDREOD_REFRACTIONS, m_cylindreOD);               //!> cylindre OD
    Utils::setDataInt(data, CP_AXECYLOD_REFRACTIONS, m_axecylindreOD);                 //!> axe OD
    Utils::setDataString(data, CP_AVLOD_REFRACTIONS, m_avlOD);                         //!> acuité de loin OD
    Utils::setDataDouble(data, CP_ADDVPOD_REFRACTIONS, m_addVPOD);                     //!> addition de près OD
    Utils::setDataString(data, CP_AVPOD_REFRACTIONS, m_avpPOD);                        //!> acuité de près OD
    Utils::setDataDouble(data, CP_PRISMEOD_REFRACTIONS, m_prismeOD);                   //!> prismeOD
    Utils::setDataInt(data, CP_BASEPRISMEOD_REFRACTIONS, m_baseprismeOD);              //!> base prisme OD en degré
    Utils::setDataString(data, CP_BASEPRISMETEXTOD_REFRACTIONS, m_baseprismetextOD);   //!> base prisme OD en texte (nasal, temporal, supérieur, inférieur
    Utils::setDataBool(data, CP_PRESSONOD_REFRACTIONS, m_haspressonOD);                //!> un presson est utilisé sur l'OD
    Utils::setDataBool(data, CP_DEPOLIOD_REFRACTIONS, m_hasdepoliOD);                  //!> un dépoli est utilisé sur l'OD
    Utils::setDataBool(data, CP_PLANOD_REFRACTIONS, m_hasplanOD);                      //!> un verre plan est utilisé sur l'OD
    Utils::setDataInt(data, CP_RYSEROD_REFRACTIONS, m_ryserOD);                        //!> puissance Ryser OD
    Utils::setDataString(data, CP_FORMULEOD_REFRACTIONS, m_formuleOD);                 //!> formule de réfraction OD
    Utils::setDataBool(data, CP_OGMESURE_REFRACTIONS, m_isOGmesure);                   //!> l'OG a été mesuré
    Utils::setDataDouble(data, CP_SPHEREOG_REFRACTIONS, m_sphereOG);                   //!> sphere OG
    Utils::setDataDouble(data, CP_CYLINDREOG_REFRACTIONS, m_cylindreOG);               //!> cylindre OG
    Utils::setDataInt(data, CP_AXECYLOG_REFRACTIONS, m_axecylindreOG);                 //!> axe OG
    Utils::setDataString(data, CP_AVLOG_REFRACTIONS, m_avlOG);                         //!> acuité de loin OG
    Utils::setDataDouble(data, CP_ADDVPOG_REFRACTIONS, m_addVPOG);                     //!> addition de près OG
    Utils::setDataString(data, CP_AVPOG_REFRACTIONS, m_avpPOG);                        //!> acuité de près OG
    Utils::setDataDouble(data, CP_PRISMEOG_REFRACTIONS, m_prismeOG);                   //!> prismeOG
    Utils::setDataInt(data, CP_BASEPRISMEOG_REFRACTIONS, m_baseprismeOG);              //!> base prisme OG en degré
    Utils::setDataString(data, CP_BASEPRISMETEXTOG_REFRACTIONS, m_baseprismetextOG);   //!> base prisme OG en texte (nasal, temporal, supérieur, inférieur
    Utils::setDataBool(data, CP_PRESSONOG_REFRACTIONS, m_haspressonOG);                //!> un presson est utilisé sur l'OG
    Utils::setDataBool(data, CP_DEPOLIOG_REFRACTIONS, m_hasdepoliOG);                  //!> un dépoli est utilisé sur l'OG
    Utils::setDataBool(data, CP_PLANOG_REFRACTIONS, m_hasplanOG);                      //!> un verre plan est utilisé sur l'OG
    Utils::setDataInt(data, CP_RYSEROG_REFRACTIONS, m_ryserOG);                        //!> puissance Ryser OG
    Utils::setDataString(data, CP_FORMULEOG_REFRACTIONS, m_formuleOG);                 //!> formule de réfraction OG
    Utils::setDataString(data, CP_COMMENTAIREORDO_REFRACTIONS, m_commentaireordo);     //!> commentaire de l'ordonnace de verres
    m_typeverres = ConvertDistance(data[CP_TYPEVERRES_REFRACTIONS].toString());        //!> la distance d'utilisation des verres
    m_oeil = Utils::ConvertCote(data[CP_OEIL_REFRACTIONS].toString());                 //!> l'oeil pour lequel les verres sont prescrits
    Utils::setDataInt(data, CP_MONTURE_REFRACTIONS, m_monture);                        //!> le nombre de montures
    Utils::setDataBool(data, CP_VERRETEINTE_REFRACTIONS, m_isverreteinte);             //!> les verres sont teintés
    Utils::setDataInt(data, CP_PD_REFRACTIONS, m_ecartIP);                             //!> ecart interpuppilaire
    Utils::setDataBool(data, "isODmesure", m_isODmesure);
    Utils::setDataBool(data, "isOGmesure", m_isOGmesure);

    m_data = data;
}

Refraction::Distance Refraction::ConvertDistance(QString distance)
{
    if (distance == "P") return Pres;
    if (distance == "L") return Loin;
    if (distance == "2") return AllDistance;
    return  Inconnu;
}

QString Refraction::ConvertDistance(Refraction::Distance distance)
{
    if (distance == Pres) return "P";
    else if (distance == Loin) return "L";
    else if (distance == AllDistance) return "2";
    return "";
}

Refraction::Mesure Refraction::ConvertMesure(QString Mesure)
{
    if (Mesure == "P") return Fronto;
    if (Mesure == "A") return Autoref;
    if (Mesure == "O") return Prescription;
    if (Mesure == "R") return Acuite;
    return  NoMesure;
}

QString Refraction::ConvertMesure(Refraction::Mesure Mesure)
{
    switch (Mesure) {
    case Fronto:       return "P";
    case Autoref:      return "A";
    case Prescription: return "O";
    case Acuite:       return "R";
    default: return "";
    }
}

void Refraction::setdataclean(Mesure mesure)
{
    m_data[CP_SPHEREOD_REFRACTIONS]           = 0;
    m_data[CP_CYLINDREOD_REFRACTIONS]         = 0;
    m_data[CP_AXECYLOD_REFRACTIONS]           = 0;
    m_data[CP_ADDVPOD_REFRACTIONS]            = 0;
    m_data[CP_PRISMEOD_REFRACTIONS]           = 0;
    m_data[CP_BASEPRISMEOD_REFRACTIONS]       = 0;
    m_data[CP_RYSEROD_REFRACTIONS]            = 0;
    m_data[CP_SPHEREOG_REFRACTIONS]           = 0;
    m_data[CP_CYLINDREOG_REFRACTIONS]         = 0;
    m_data[CP_AXECYLOG_REFRACTIONS]           = 0;
    m_data[CP_ADDVPOG_REFRACTIONS]            = 0;
    m_data[CP_PRISMEOG_REFRACTIONS]           = 0;
    m_data[CP_BASEPRISMEOG_REFRACTIONS]       = 0;
    m_data[CP_RYSEROG_REFRACTIONS]            = 0;
    m_data[CP_TYPEMESURE_REFRACTIONS]         = ConvertMesure(mesure);
}

