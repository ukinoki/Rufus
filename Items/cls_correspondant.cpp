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

#include "cls_correspondant.h"

int Correspondant::id() const
{
    return m_id;
}
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
int Correspondant::specialite() const
{
    return m_specialite;
}

Correspondant::Correspondant(QJsonObject data, QObject *parent) : Item(parent)
{
    setData(data);
}

bool Correspondant::isAllLoaded() const
{
    return m_isAllLoaded;
}

void Correspondant::setData(QJsonObject data)
{
    if( data.isEmpty() )
        return;

    setDataInt(data,    "id", m_id);
    setDataBool(data,   "generaliste", m_generaliste);
    setDataBool(data,   "medecin", m_medecin);
    setDataString(data, "nom", m_nom);
    setDataString(data, "prenom", m_prenom);
    setDataString(data, "sexe", m_sexe);
    setDataString(data, "metier", m_metier);
    setDataString(data, "adresse1", m_adresse1);
    setDataString(data, "adresse2", m_adresse2);
    setDataString(data, "adresse3", m_adresse3);
    setDataString(data, "codepostal", m_codepostal);
    setDataString(data, "ville", m_ville);
    setDataString(data, "telephone", m_telephone);
    setDataString(data, "mail", m_mail);
    setDataString(data, "fax", m_fax);
    setDataString(data, "portable", m_portable);
    setDataInt(data,    "specialite", m_specialite);

    setDataBool(data,   "isAllLoaded", m_isAllLoaded);
}

