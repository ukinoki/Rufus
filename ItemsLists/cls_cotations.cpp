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

#include "cls_cotations.h"


Cotations::Cotations(QObject *parent) : ItemsList(parent)
{
    m_cotations = new QMap<int, Cotation*>();
}

QMap<int, Cotation *> *Cotations::cotations() const
{
    return m_cotations;
}

void Cotations::addList(QList<Cotation*> listcot)
{
    QList<Cotation*>::const_iterator it;
    for( it = listcot.constBegin(); it != listcot.constEnd(); ++it )
    {
        Cotation* item = const_cast<Cotation*>(*it);
        add( m_cotations, item->id(), item );
    }
}

/*!
 * \brief Cotationss::initListeByUser
 * Charge l'ensemble des cotations pour le user
 * et les ajoute à la classe Correspondants
 */
void Cotations::initListeByUser(int iduser)
{
    clearAll(m_cotations);
    addList(DataBase::I()->loadCotationsByUser(iduser));
}

