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

#include "cls_patient.h"

//GETTER | SETTER
bool Patient::isAllLoaded() const
{
    return m_isAllLoaded;
}

int Patient::id() const
{
    return m_id;
}
QString Patient::nom() const
{
    return m_nom;
}
QString Patient::prenom() const
{
    return m_prenom;
}
QString Patient::sexe() const
{
    return m_sexe;
}


QMap<int, Acte *> *Patient::actes() const
{
    return m_actes;
}
void Patient::setActes(QMap<int, Acte *> *actes)
{
    m_actes = actes;
}




Patient::Patient(QJsonObject data, QObject *parent) : Item(parent)
{
    setData(data);
}

void Patient::setData(QJsonObject data)
{
    if( data.isEmpty() )
        return;

    setDataBool(data, "isAllLoaded", m_isAllLoaded);

    setDataInt(data, "id", m_id);

    setDataString(data, "nom", m_nom);
    setDataString(data, "prenom", m_prenom);
    setDataString(data, "sexe", m_sexe);

    setDataDateTime(data, "dateDeNaissance", m_dateDeNaissance);
}

/*!
 * \brief Patient::addActe
 * ajout un acte au patient
 * \param acte
*/
void Patient::addActe(Acte *acte)
{
    if( m_actes->contains(acte->id()) )
        return;
    m_actes->insert(acte->id(), acte);
}
