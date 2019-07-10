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

#include "cls_refractions.h"

Refractions::Refractions(QObject *parent) : ItemsList(parent)
{
    m_refractions = new QMap<int, Refraction*>();
}

QMap<int, Refraction *>* Refractions::refractions() const
{
    return m_refractions;
}

void Refractions::addList(QList<Refraction*> listRefractions)
{
    QList<Refraction*>::const_iterator it;
    for( it = listRefractions.constBegin(); it != listRefractions.constEnd(); ++it )
    {
        Refraction* ref = const_cast<Refraction*>(*it);
        add( m_refractions, ref );
    }
}

Refraction* Refractions::getById(int id)
{
    QMap<int, Refraction*>::const_iterator itref = m_refractions->find(id);
    if( itref == m_refractions->constEnd() )
    {
        Refraction * ref = DataBase::I()->loadRefractionById(id);
        if (ref != Q_NULLPTR)
            add( m_refractions, ref );
        return ref;
    }
    return itref.value();
}

/*!
 * \brief Refractions::initListeByPatId
 * Charge l'ensemble des documments accessibles à l'utilisateur en cours
 * et les ajoute à la classe Refractions
 */
void Refractions::initListebyPatId(int id)
{
    clearAll(m_refractions);
    addList(DataBase::I()->loadRefractionByPatId(id));
}


void Refractions::SupprimeRefraction(Refraction* ref)
{
    Supprime(m_refractions, ref);
}

