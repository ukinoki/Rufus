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

#include "cls_compte.h"

Compte::Compte(QJsonObject data, QObject *parent) : Item(parent)
{
    setData(data);
}

int Compte::idUser() const              { return m_iduser; }
int Compte::idBanque() const            { return m_idbanque; }
QString Compte::iban() const            { return m_iban; }
QString Compte::intitulecompte() const  { return m_intitulecompte; }
QString Compte::nomabrege() const       { return m_nomabrege; }
double Compte::solde() const            { return m_solde; }
bool Compte::isDesactive() const        { return m_desactive; }
bool Compte::isPartage() const          { return m_partage; }


void Compte::setData(QJsonObject data)
{
    if( data.isEmpty() )
        return;

    Utils::setDataInt(data, CP_IDCOMPTE_COMPTES, m_id);
    Utils::setDataInt(data, CP_IDUSER_COMPTES, m_iduser);
    Utils::setDataInt(data, CP_IDBANQUE_COMPTES, m_idbanque);
    Utils::setDataString(data, CP_IBAN_COMPTES, m_iban);
    Utils::setDataString(data, CP_INTITULE_COMPTES, m_intitulecompte);
    Utils::setDataString(data, CP_NOMABREGE_COMPTES, m_nomabrege);
    Utils::setDataDouble(data, CP_SOLDE_COMPTES, m_solde);
    Utils::setDataBool(data, CP_DESACTIVE_COMPTES, m_desactive);
    Utils::setDataBool(data, CP_PARTAGE_COMPTES, m_partage);
    m_data = data;
}
