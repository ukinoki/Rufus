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
QString Compte::nom() const             { return m_nom; }
double Compte::solde() const            { return m_solde; }
bool Compte::isDesactive() const        { return m_desactive; }
bool Compte::isPartage() const          { return m_partage; }


void Compte::setData(QJsonObject data)
{
    if( data.isEmpty() )
        return;

    setDataInt(data, "id", m_id);
    setDataInt(data, "iduser", m_iduser);
    setDataInt(data, "idbanque", m_idbanque);
    setDataString(data, "IBAN", m_iban);
    setDataString(data, "IntituleCompte", m_intitulecompte);
    setDataString(data, "nom", m_nom);
    setDataDouble(data, "solde", m_solde);
    setDataBool(data, "desactive", m_desactive);
    setDataBool(data, "partage", m_partage);
    m_data = data;
}

void Compte::setSolde(double solde)
{
    m_solde = solde;
}
