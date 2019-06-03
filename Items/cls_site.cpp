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

#include "cls_site.h"

Site::Site(QJsonObject data, QObject *parent) : Item(parent)
{
    setData( data );
}

void Site::setData(QJsonObject data)
{
    if( data.isEmpty() )
        return;


    setDataInt(data, "idLieu", m_id);
    setDataString(data, "nomLieu", m_nom);
    setDataString(data, "adresse1", m_adresse1);
    setDataString(data, "adresse2", m_adresse2);
    setDataString(data, "adresse3", m_adresse3);
    setDataInt(data, "codePostal", m_codePostal);
    setDataString(data, "ville", m_ville);
    setDataString(data, "telephone", m_telephone);
    setDataString(data, "fax", m_fax);
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
