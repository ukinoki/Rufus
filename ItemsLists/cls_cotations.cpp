/* (C) 2020 LAINE SERGE
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


int Cotations::iduser() const
{
    return m_iduser;
}

void Cotations::setiduser(int iduser)
{
    m_iduser = iduser;
}

Cotations::Cotations(QObject *parent) : ItemsList(parent)
{
    map_cotations = new QMap<int, Cotation*>();
}

QMap<int, Cotation *> *Cotations::cotations() const
{
    return map_cotations;
}

/*!
 * \brief Cotationss::initListeByUser
 * Charge l'ensemble des cotations pour le user
 * et les ajoute à la classe Correspondants
 */
void Cotations::initListeByUser(User *usr)
{
    if (usr == Q_NULLPTR)
        return;
    QList<Cotation*> listcotations = DataBase::I()->loadCotationsByUser(usr);
    epurelist(map_cotations, &listcotations);
    addList(map_cotations, &listcotations);
    setiduser(usr->id());
}

