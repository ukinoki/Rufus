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

#include "cls_banques.h"

Banques::Banques(QObject *parent) : ItemsList(parent)
{
    m_banques = new QMap<int, Banque*>();
}

QMap<int, Banque *> *Banques::banques() const
{
    return m_banques;
}

/*!
 * \brief Banques::initListeBanques
 * Charge l'ensemble des banques
 * et les ajoute à la classe Banques
 */
void Banques::initListe()
{
    clearAll();
    QList<Banque*> listbanques = DataBase::I()->loadBanques();
    addList(listbanques);
}

void Banques::add(Banque *banque)
{
    if( m_banques->contains(banque->id()) )
        return;
    m_banques->insert(banque->id(), banque);
}

void Banques::addList(QList<Banque*> listbanques)
{
    QList<Banque*>::const_iterator it;
    for( it = listbanques.constBegin(); it != listbanques.constEnd(); ++it )
        add( *it );
}

void Banques::clearAll()
{
    for( QMap<int, Banque*>::const_iterator itbq = m_banques->constBegin(); itbq != m_banques->constEnd(); ++itbq)
        delete itbq.value();
    m_banques->clear();
}

void Banques::remove(Banque *banq)
{
    if (banq == Q_NULLPTR)
        return;
    m_banques->remove(banq->id());
    delete banq;
}

Banque* Banques::getById(int id)
{
    QMap<int, Banque*>::const_iterator itcpt = m_banques->find(id);
    if( itcpt == m_banques->constEnd() )
        return Q_NULLPTR;
    return itcpt.value();
}
