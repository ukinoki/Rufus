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

#include "cls_correspondant.h"

QString Correspondant::nom() const
{
    return m_nom;
}
QString Correspondant::prenom() const
{
    return m_prenom;
}
QString Correspondant::nomprenom() const
{
    return m_nom + " " + m_prenom;
}
QString Correspondant::sexe() const
{
    return m_sexe;
}
bool Correspondant::isMG() const
{
    return m_generaliste;
}
bool Correspondant::ismedecin() const
{
    return m_medecin;
}
QString Correspondant::adresse1() const
{
    return m_adresse1;
}
QString Correspondant::adresse2() const
{
    return m_adresse2;
}
QString Correspondant::adresse3() const
{
    return m_adresse3;
}
QString Correspondant::metier() const
{
    return m_metier;
}
QString Correspondant::ville() const
{
    return m_ville;
}
QString Correspondant::codepostal() const
{
    return m_codepostal;
}
QString Correspondant::telephone() const
{
    return m_telephone;
}
QString Correspondant::adresseComplete() const
{
    QString ttip;
    if (m_adresse1!= "") ttip += m_adresse1;
    if (m_adresse2 != "")
    {
        if (ttip != "") ttip += "\n";
        ttip += m_adresse2;
    }
    if (m_adresse3 != "")
    {
        if (ttip != "") ttip += "\n";
        ttip += m_adresse3;
    }
    if (m_codepostal + m_ville != "")
    {
        if (ttip != "") ttip += "\n";
        ttip += m_codepostal;
        if (m_codepostal != "" && m_ville != "")
            ttip += " ";
        ttip += m_ville;
    }
    if (m_telephone != "")
    {
        if (ttip != "") ttip += "\n";
        ttip += m_telephone;
    }
    return ttip;
}
QString Correspondant::portable() const
{
    return m_portable;
}
QString Correspondant::fax() const
{
    return m_fax;
}
QString Correspondant::mail() const
{
    return m_mail;
}
int Correspondant::idspecialite() const
{
    return m_idspecialite;
}

Correspondant::Correspondant(QJsonObject data, QObject *parent) : Item(parent)
{
    setData(data);
}

bool Correspondant::isallloaded() const
{
    return m_isAllLoaded;
}

void Correspondant::setData(QJsonObject data)
{
    if( data.isEmpty() )
        return;

    Utils::setDataInt(data,    CP_ID_CORRESP, m_id);
    Utils::setDataBool(data,   CP_ISGENERALISTE, m_generaliste);
    Utils::setDataBool(data,   CP_ISMEDECIN, m_medecin);
    Utils::setDataString(data, CP_NOM_CORRESP, m_nom);
    Utils::setDataString(data, CP_PRENOM_CORRESP, m_prenom);
    Utils::setDataString(data, CP_SEXE_CORRESP, m_sexe);
    Utils::setDataString(data, CP_METIER, m_metier);
    Utils::setDataString(data, CP_ADRESSE1_CORRESP, m_adresse1);
    Utils::setDataString(data, CP_ADRESSE2_CORRESP, m_adresse2);
    Utils::setDataString(data, CP_ADRESSE3_CORRESP, m_adresse3);
    Utils::setDataString(data, CP_CODEPOSTAL_CORRESP, m_codepostal);
    Utils::setDataString(data, CP_VILLE_CORRESP, m_ville);
    Utils::setDataString(data, CP_TELEPHONE_CORRESP, m_telephone);
    Utils::setDataString(data, CP_MAIL_CORRESP, m_mail);
    Utils::setDataString(data, CP_FAX_CORRESP, m_fax);
    Utils::setDataString(data, CP_PORTABLE_CORRESP, m_portable);
    Utils::setDataInt(data,    CP_SPECIALITE_CORRESP, m_idspecialite);

    Utils::setDataBool(data,   CP_ISALLLOADED, m_isAllLoaded);
    m_data = data;
}
