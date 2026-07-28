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
    setDataInt(data,    CP_ID_COTATIONS,                m_id);
    setDataString(data, CP_TYPEACTE_COTATIONS,          m_typeacte);
    setDataDouble(data, CP_MONTANTOPTAM_COTATIONS,      m_montantoptam);
    setDataDouble(data, CP_MONTANTNONOPTAM_COTATIONS,   m_montantnonoptam);
    setDataDouble(data, CP_MONTANTPRATIQUE_JOINTCOTATIONS, m_montantpratique);  //! le pratiqué vient des jointures, plus de cotations.MontantPratique
    setDataInt(data,    CP_TYPECOTATION_COTATIONS,      m_typcotation);   //! le type (1/2/3/4) est porté par la colonne Typecotation
    setDataInt(data,    CP_FREQUENCE_COTATIONS,         m_frequence);
    setDataString(data, CP_TIP_COTATIONS,               m_descriptif);

    m_data = data;
}
