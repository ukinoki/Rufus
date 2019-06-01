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

void PaiementsTiers::add(PaiementTiers *PaiementTiers)
{
    if( m_paiementstiers->contains(PaiementTiers->id()) )
        return;
    m_paiementstiers->insert(PaiementTiers->id(), PaiementTiers);
}

void PaiementsTiers::addList(QList<PaiementTiers*> listpaiementtiers)
{
    QList<PaiementTiers*>::const_iterator it;
    for( it = listpaiementtiers.constBegin(); it != listpaiementtiers.constEnd(); ++it )
        add( *it );
}

PaiementTiers* PaiementsTiers::getById(int id)
{
    QMap<int, PaiementTiers*>::const_iterator itcpt = m_paiementstiers->find(id);
    if( itcpt == m_paiementstiers->constEnd() )
        return Q_NULLPTR;
    return itcpt.value();
}

void PaiementsTiers::clearAll()
{
    for( QMap<int, PaiementTiers*>::const_iterator itmtf = m_paiementstiers->constBegin(); itmtf != m_paiementstiers->constEnd(); ++itmtf)
        delete itmtf.value();
    m_paiementstiers->clear();
}

void PaiementsTiers::remove(PaiementTiers *PaiementTiers)
{
    if (PaiementTiers == Q_NULLPTR)
        return;
    m_paiementstiers->remove(PaiementTiers->id());
    delete PaiementTiers;
}

/*!
 * \brief PaiementsTiers::initListe
 * Charge l'ensemble des paiements par tiers
 * et les ajoute à la classe PaiementsTiers
 */
void PaiementsTiers::initListe(User* usr)
{
    clearAll();
    addList(DataBase::I()->loadPaiementTiersByUser(usr));
}
