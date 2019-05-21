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

#include "cls_comptes.h"

Comptes::Comptes(QObject * parent) : ItemsList(parent)
{
    m_comptes = new QMap<int, Compte*>();
}

Comptes::~Comptes()
{
    clearAll();
    delete m_comptes;
}

QMap<int, Compte*>* Comptes::comptes() const
{
    return m_comptes;
}

void Comptes::add(Compte *compte)
{
    if( m_comptes->contains(compte->id()) )
        return;
    m_comptes->insert(compte->id(), compte);
}

void Comptes::addList(QList<Compte*> listCompte)
{
    QList<Compte*>::const_iterator it;
    for( it = listCompte.constBegin(); it != listCompte.constEnd(); ++it )
        add( *it );
}

void Comptes::clearAll()
{
    QList<Compte*> listcpts;
    QMap<int, Compte*>::const_iterator itcpt;
    for( itcpt = m_comptes->constBegin(); itcpt != m_comptes->constEnd(); ++itcpt)
        delete itcpt.value();
    m_comptes->clear();
}

void Comptes::removeCompte(Compte* cpt)
{
    if (cpt == Q_NULLPTR)
        return;
    QMap<int, Compte*>::const_iterator itcpt;
    m_comptes->find(cpt->id());
    if( itcpt != m_comptes->constEnd() )
        m_comptes           ->remove(cpt->id());
    delete cpt;
}

Compte* Comptes::getById(int id)
{
    QMap<int, Compte*>::const_iterator itcpt = m_comptes->find(id);
    if( itcpt == m_comptes->constEnd() )
        return Q_NULLPTR;
    return itcpt.value();
}

void Comptes::reloadCompte(Compte *compte)
{
    compte->setData(DataBase::I()->loadCompteById(compte->id()));
}

void Comptes::initListe()
{
    QList<Compte*> listcomptes;
    listcomptes = DataBase::I()->loadComptesAll();
    QList<Compte*>::const_iterator itcomptes;
    for( itcomptes = listcomptes.constBegin(); itcomptes != listcomptes.constEnd(); ++itcomptes )
    {
        Compte* cpt = const_cast<Compte*>(*itcomptes);
        add(cpt);
    }
}

