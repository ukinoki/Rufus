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

#include "cls_sites.h"

/*
 * GETTER
*/
QMap<int, Site *> *Sites::sites() const
{
    return m_sites;
}

/*!
 * \brief Sites::Sites
 * Initialise la map m_sites
 */
Sites::Sites(QObject *parent) : ItemsList(parent)
{
    m_sites = new QMap<int, Site*>();
}

void Sites::addList(QList<Site*> listSites)
{
    QList<Site*>::const_iterator it;
    for( it = listSites.constBegin(); it != listSites.constEnd(); ++it )
    {
        Site* sit = const_cast<Site*>(*it);
        add( m_sites, sit );
    }
}

/*!
 * \brief Sites::getById
 * \param id l'id du site recherché
 * \return Q_NULLPTR si aucun site trouvé
 * \return Site* le site correspondant à l'id
 */
Site* Sites::getById(int id)
{
    QMap<int, Site*>::const_iterator itsit = m_sites->find(id);
    Site *result = Q_NULLPTR;
    if( itsit!= m_sites->constEnd() )
        result = itsit.value();
    return result;
}


/*!
 * \brief Sites::initListe sites
 * Charge l'ensemble des sites
 * et les ajoute à la classe Sites
 */
void Sites::initListe()
{
    clearAll(m_sites);
    addList(DataBase::I()->loadSitesAll());
}
