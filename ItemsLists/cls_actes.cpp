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

#include "cls_actes.h"

Actes::Actes(QObject *parent) : ItemsList(parent)
{
    m_actes = new QMap<int, Acte*>();
}

QMap<int, Acte *> *Actes::actes() const
{
    return m_actes;
}

/*!
 * \brief Actes::initListe
 * Charge l'ensemble des actes
 * et les ajoute à la classe Actes
 */
void Actes::initListeByPatient(Patient *pat)
{
    clearAll();
    QMap<int, Acte*> listActes = DataBase::I()->loadActesByPat(pat);
    QMap<int, Acte*>::const_iterator itact;
    for( itact = listActes.constBegin(); itact != listActes.constEnd(); ++itact )
    {
        Acte *act = const_cast<Acte*>(*itact);
        add( act );
    }
}

void Actes::add(Acte *acte)
{
    if( m_actes->contains(acte->id()) )
        return;
    m_actes->insert(acte->id(), acte);
}

void Actes::addList(QList<Acte*> listActes)
{
    QList<Acte*>::const_iterator it;
    for( it = listActes.constBegin(); it != listActes.constEnd(); ++it )
        add( *it );
}

void Actes::clearAll()
{
    for( QMap<int, Acte*>::const_iterator itact = m_actes->constBegin(); itact != m_actes->constEnd(); ++itact)
        delete itact.value();
    m_actes->clear();
}

void Actes::remove(Acte *acte)
{
    if (acte == Q_NULLPTR)
        return;
    m_actes->remove(acte->id());
    delete acte;
}

Acte* Actes::getById(int id, ADDTOLIST add)
{
    QMap<int, Acte*>::const_iterator itact = m_actes->find(id);
    if( itact == m_actes->constEnd() )
    {
        Acte * act = Q_NULLPTR;
        if (add == AddToList)
            act = DataBase::I()->loadActeById(id);
        return act;
    }
    return itact.value();
}

void Actes::reloadActe(Acte* acte)
{
    acte->setData(DataBase::I()->loadActeAllData(acte->id()));
}
