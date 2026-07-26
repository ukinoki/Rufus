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

#include "cls_cotation.h"

Cotation::Cotation(QJsonObject data, QObject *parent) : Item(parent)
{
    setData(data);
}

void Cotation::setData(QJsonObject data)
{
    if( data.isEmpty() )
        return;

    setDataInt(data, "idcotation", m_id);
    setDataString(data, "typeacte", m_typeacte);
    setDataDouble(data, "montantoptam", m_montantoptam);
    setDataDouble(data, "montantnonoptam", m_montantnonoptam);
    setDataDouble(data, "montantpratique", m_montantpratique);
    setDataInt(data, "ccam", m_typcotation);   //! le type (1/2/3/4) est porté par la colonne CCAM
    settypcotation(m_typcotation);             //! déduit les 4 booléens is*
    setDataInt(data, "frequence", m_frequence);
    setDataString(data, "descriptif", m_descriptif);
    m_data = data;
}
