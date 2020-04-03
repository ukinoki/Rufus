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

#include "cls_site.h"

Site::Site(QJsonObject data, QObject *parent) : Item(parent)
{
    setData( data );
}

void Site::setData(QJsonObject data)
{
    if( data.isEmpty() )
        return;


    Utils::setDataInt(data, CP_ID_SITE, m_id);
    Utils::setDataString(data, CP_NOM_SITE, m_nom);
    Utils::setDataString(data, CP_ADRESSE1_SITE, m_adresse1);
    Utils::setDataString(data, CP_ADRESSE2_SITE, m_adresse2);
    Utils::setDataString(data, CP_ADRESSE3_SITE, m_adresse3);
    Utils::setDataInt(data, CP_CODEPOSTAL_SITE, m_codePostal);
    Utils::setDataString(data, CP_VILLE_SITE, m_ville);
    Utils::setDataString(data, CP_TELEPHONE_SITE, m_telephone);
    Utils::setDataString(data, CP_FAX_SITE, m_fax);
    Utils::setDataString(data, CP_COULEUR_SITE, m_couleur);
    m_data = data;
}


QString Site::nom() const       { return m_nom; }
QString Site::adresse1() const  { return m_adresse1; }
QString Site::adresse2() const  { return m_adresse2; }
QString Site::adresse3() const  { return m_adresse3; }
int Site::codePostal() const    { return m_codePostal; }
QString Site::ville() const     { return m_ville; }
QString Site::telephone() const { return m_telephone; }
QString Site::fax() const       { return m_fax; }
QString Site::couleur() const   { return m_couleur; }

void Site::setnom(const QString &nom)               { m_nom = nom; }
void Site::setadresse1(const QString &adresse1)     { m_adresse1 = adresse1; }
void Site::setadresse2(const QString &adresse2)     { m_adresse2 = adresse2; }
void Site::setadresse3(const QString &adresse3)     { m_adresse3 = adresse3; }
void Site::setcodepostal(int codePostal)            { m_codePostal = codePostal; }
void Site::setville(const QString &ville)           { m_ville = ville; }
void Site::settelephone(const QString &telephone)   { m_telephone = telephone; }
void Site::setfax(const QString &fax)               { m_fax = fax; }
void Site::setcouleur(const QString &couleur)       { m_couleur = couleur; }

QString Site::adresseComplete() const
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
    if (QString::number(m_codePostal) + m_ville != "")
    {
        if (ttip != "") ttip += "\n";
        ttip += QString::number(m_codePostal);
        if (QString::number(m_codePostal) != "" && m_ville != "")
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
