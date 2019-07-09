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
    setData(data);
}
void Refraction::setData(QJsonObject data)
{
    if( data.isEmpty() )
        return;
    setDataInt(data, CP_ID_REFRACTIONS, m_id);

    setDataInt(data, CP_IDPAT_REFRACTIONS, m_idpat);                            //!> l'id du patient
    setDataInt(data, CP_IDACTE_REFRACTIONS, m_idacte);                          //!> l'id de l'acte aucours duquel la mesure a été faite
    setDataDate(data, CP_DATE_REFRACTIONS, m_daterefraction);                   //!> la date de la refraction
    QString mesure = data.value( CP_TYPEMESURE_REFRACTIONS ).toString();        //!> le type de mesure effectuée : frontofocometre, autorfractomètre, acuité ou prescription
    if (mesure == "P")
        m_mesure = Porte;
    else if (mesure == "A")
        m_mesure = Autoref;
    else if (mesure == "R")
        m_mesure = Acuite;
    else if (mesure == "O")
        m_mesure = Prescription;
    QString distance = data.value( CP_DISTANCEMESURE_REFRACTIONS ).toString();  //!> la distance de mesure: loin, près, les 2
    if (distance == "L")
        m_distance = Loin;
    else if (distance == "P")
        m_distance = Pres;
    else if (distance == "2")
        m_distance = AllDistance;
    setDataBool(data, CP_CYCLOPLEGIE_REFRACTIONS, m_isdilate);                  //!> examen sous cycloplegie ou non
    setDataBool(data, CP_ODMESURE_REFRACTIONS, m_isODmesure);                   //!> l'OD a été mesuré
    setDataDouble(data, CP_SPHEREOD_REFRACTIONS, m_sphereOD);                   //!> sphere OD
    setDataDouble(data, CP_CYLINDREOD_REFRACTIONS, m_cylindreOD);               //!> cylindre OD
    setDataInt(data, CP_AXECYLOD_REFRACTIONS, m_axecylindreOD);                 //!> axe OD
    setDataString(data, CP_AVLOD_REFRACTIONS, m_avlOD);                         //!> acuité de loin OD
    setDataDouble(data, CP_ADDVPOD_REFRACTIONS, m_addVPOD);                     //!> addition de près OD
    setDataString(data, CP_AVPOD_REFRACTIONS, m_avpPOD);                        //!> acuité de près OD
    setDataDouble(data, CP_PRISMEOD_REFRACTIONS, m_prismeOD);                   //!> prismeOD
    setDataInt(data, CP_BASEPRISMEOD_REFRACTIONS, m_baseprismeOD);              //!> base prisme OD en degré
    setDataString(data, CP_BASEPRISMETEXTOD_REFRACTIONS, m_baseprismetextOD);   //!> base prisme OD en texte (nasal, temporal, supérieur, inférieur
    setDataBool(data, CP_PRESSONOD_REFRACTIONS, m_haspressonOD);                //!> un presson est utilisé sur l'OD
    setDataBool(data, CP_DEPOLIOD_REFRACTIONS, m_hasdepoliOD);                  //!> un dépoli est utilisé sur l'OD
    setDataBool(data, CP_PLANOD_REFRACTIONS, m_hasplanOD);                      //!> un verre plan est utilisé sur l'OD
    setDataInt(data, CP_RYSEROD_REFRACTIONS, m_ryserOD);                        //!> puissance Ryser OD
    setDataString(data, CP_FORMULEOD_REFRACTIONS, m_formuleOD);                 //!> formule de réfraction OD
    setDataBool(data, CP_OGMESURE_REFRACTIONS, m_isOGmesure);                   //!> l'OG a été mesuré
    setDataDouble(data, CP_SPHEREOG_REFRACTIONS, m_sphereOG);                   //!> sphere OG
    setDataDouble(data, CP_CYLINDREOG_REFRACTIONS, m_cylindreOG);               //!> cylindre OG
    setDataInt(data, CP_AXECYLOG_REFRACTIONS, m_axecylindreOG);                 //!> axe OG
    setDataString(data, CP_AVLOG_REFRACTIONS, m_avlOG);                         //!> acuité de loin OG
    setDataDouble(data, CP_ADDVPOG_REFRACTIONS, m_addVPOG);                     //!> addition de près OG
    setDataString(data, CP_AVPOG_REFRACTIONS, m_avpPOG);                        //!> acuité de près OG
    setDataDouble(data, CP_PRISMEOG_REFRACTIONS, m_prismeOG);                   //!> prismeOG
    setDataInt(data, CP_BASEPRISMEOG_REFRACTIONS, m_baseprismeOG);              //!> base prisme OG en degré
    setDataString(data, CP_BASEPRISMETEXTOG_REFRACTIONS, m_baseprismetextOG);   //!> base prisme OG en texte (nasal, temporal, supérieur, inférieur
    setDataBool(data, CP_PRESSONOG_REFRACTIONS, m_haspressonOG);                //!> un presson est utilisé sur l'OG
    setDataBool(data, CP_DEPOLIOG_REFRACTIONS, m_hasdepoliOG);                  //!> un dépoli est utilisé sur l'OG
    setDataBool(data, CP_PLANOG_REFRACTIONS, m_hasplanOG);                      //!> un verre plan est utilisé sur l'OG
    setDataInt(data, CP_RYSEROG_REFRACTIONS, m_ryserOG);                        //!> puissance Ryser OG
    setDataString(data, CP_FORMULEOG_REFRACTIONS, m_formuleOG);                 //!> formule de réfraction OG
    setDataString(data, CP_COMMENTAIREORDO_REFRACTIONS, m_commentaireordo);     //!> commentaire de l'ordonnace de verres
    QString typeverres = data.value( CP_TYPEVERRES_REFRACTIONS ).toString();    //!> la distance d'utilisation des verres
    if (typeverres == "L")
        m_typeverres = Loin;
    else if (typeverres == "P")
        m_typeverres = Pres;
    else if (typeverres == "2")
        m_typeverres = AllDistance;
    QString oeil = data.value(CP_OEIL_REFRACTIONS).toString();                  //!> l'oeil pour lequel les verres sont prescrits
    if (oeil == "D")
        m_oeil = Droit;
    else if (oeil == "G")
        m_oeil = Gauche;
    else if (oeil == "2")
        m_oeil = Les2;
    setDataInt(data, CP_MONTURE_REFRACTIONS, m_monture);                        //!> le nombre de montures
    setDataBool(data, CP_VERRETEINTE_REFRACTIONS, m_isverreteinte);             //!> les verres sont teintés
    setDataInt(data, CP_PD_REFRACTIONS, m_ecartIP);                             //!> ecart interpuppilaire
    m_data = data;
}
