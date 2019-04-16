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


#include "cls_depenses.h"
#include <QDebug>

/*
 * GETTER
*/
QMap<int, Depense *> *Depenses::getDepenses() const
{
    return m_Depenses;
}


/*!
 * \brief Depenses::Depenses
 * Initialise la map Depenses
 */
Depenses::Depenses(QObject *parent) : QObject (parent)
{
    m_Depenses = new QMap<int, Depense*>();
}

void Depenses::clearAll()
{
    QList<Depense*> listdepenses;
    for( QMap<int, Depense*>::const_iterator itdep = m_Depenses->constBegin(); itdep != m_Depenses->constEnd(); ++itdep)
        delete itdep.value();
    m_Depenses->clear();
}

/*!
 * \brief Depenses::addDepense
 * Cette fonction va ajouter la Depense passée en paramètre
 *
 * \param Depense la Depense que l'on veut ajouter
 * \return true si la Depense est ajoutée
 * \return false si le paramètre Depense est un nullptr
 * \return false si la Depense est déjà présent
 */
bool Depenses::addDepense(Depense *Depense)
{
    if( Depense == Q_NULLPTR)
        return false;

    if( m_Depenses->contains(Depense->id()) )
        return false;

    m_Depenses->insert(Depense->id(), Depense);

    return true;
}

/*!
 * \brief Depenses::getDepenseById
 * \param id l'id du Depense recherché
 * \return nullptr si aucune Depense trouvée
 * \return Depense* le Depense Depense à l'id
 */
Depense* Depenses::getDepenseById(int id)
{
    QMap<int, Depense*>::const_iterator Depense = m_Depenses->find(id);
    if( Depense == m_Depenses->constEnd() )
        return Q_NULLPTR;
    return Depense.value();
}

void Depenses::removeDepense(Depense *dep)
{
    if (dep == Q_NULLPTR)
        return;
    QMap<int, Depense*>::const_iterator Depense = m_Depenses->find(dep->id());
    if( Depense == m_Depenses->constEnd() )
        return;
    m_Depenses->remove(dep->id());
    delete dep;
}
