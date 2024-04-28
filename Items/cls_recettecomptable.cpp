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

#include "cls_recettecomptable.h"

RecetteComptable::RecetteComptable(QJsonObject data, QObject *parent) : Item(parent)
{
    setData(data);
}

void RecetteComptable::setData(QJsonObject data)
{
    if( data.isEmpty() )
        return;
    setDataInt(data, CP_ID_LIGNRECETTES, m_id);
    setDataInt(data, CP_IDUSER_LIGNRECETTES, m_iduser);
    setDataDate(data, CP_DATE_LIGNRECETTES, m_date);
    setDataDate(data, CP_DATEENREGISTREMENT_LIGNRECETTES , m_dateenregistrement);
    setDataDouble(data, CP_MONTANT_LIGNRECETTES, m_montant);
    setDataString(data, CP_MODEPAIEMENT_LIGNRECETTES, m_modepaiement);
    setDataString(data, CP_TIREURCHEQUE_LIGNRECETTES, m_nomtireurchq);
    setDataInt(data, CP_IDCPTEVIREMENT_LIGNRECETTES, m_idcompte);
    setDataString(data, CP_BANQUECHEQUE_LIGNRECETTES, m_nombanque);
    setDataBool(data, CP_TIERSPAYANT_LIGNRECETTES, m_tierspayant);
    setDataString(data, CP_NOMPAYEUR_LIGNRECETTES , m_nompayeur);
    setDataDouble(data, CP_COMMISSION_LIGNRECETTES, m_commission);
    setDataInt(data, CP_IDREMISECHQ_LIGNRECETTES, m_idremise);
    setDataBool(data, CP_CHQENATTENTE_LIGNRECETTES, m_chqenattente);
    setDataInt(data, CP_IDUSERENREGISTREUR_LIGNRECETTES, m_iduserenregistreur);
    setDataInt(data, CP_TYPERECETTE_LIGNRECETTES, m_typerecette);
    m_data = data;
}
