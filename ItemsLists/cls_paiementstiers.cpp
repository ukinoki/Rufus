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


#include "cls_paiementstiers.h"
#include "database.h"

PaiementsTiers::PaiementsTiers(QObject *parent) : ItemsList(parent)
{
    m_paiementstiers = new QMap<int, PaiementTiers*>();
}

QMap<int, PaiementTiers *> *PaiementsTiers::paiementstiers() const
{
    return m_paiementstiers;
}

PaiementTiers* PaiementsTiers::getById(int id)
{
    QMap<int, PaiementTiers*>::const_iterator itcpt = m_paiementstiers->find(id);
    if( itcpt == m_paiementstiers->constEnd() )
        return Q_NULLPTR;
    return itcpt.value();
}

/*!
 * \brief PaiementsTiers::initListe
 * Charge l'ensemble des paiements par tiers
 * et les ajoute à la classe PaiementsTiers
 */
void PaiementsTiers::initListe(User* usr)
{
    QList<PaiementTiers*> listpaiements = DataBase::I()->loadPaiementTiersByUser(usr);
    epurelist(m_paiementstiers, &listpaiements);
    addList(m_paiementstiers, &listpaiements);
}
