/* (C) 2018 LAINE SERGE
This file is part of Rufus.

Rufus is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Rufus is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Rufus. If not, see <http://www.gnu.org/licenses/>.
*/

#include "cls_depenses.h"

/*
 * GETTER
*/
QHash<int, Depense *> *Depenses::getDepenses() const
{
    return m_Depenses;
}


/*!
 * \brief Depenses::Depenses
 * Initialise la map Depenses
 */
Depenses::Depenses(QObject *parent) : QObject (parent)
{
    m_Depenses = new QHash<int, Depense*>();
}

/*!
 * \brief Depenses::addDepense
 * Cette fonction va ajouter la Depense passée en paramètre
 *
 * \param Depense le Depense que l'on veut ajouter
 * \return true si le Depense est ajouté
 * \return false si le paramètre Depense est un nullptr
 * \return false si le Depense est déjà présent
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
    QHash<int, Depense*>::const_iterator Depense = m_Depenses->find(id);
    if( Depense == m_Depenses->constEnd() )
        return Q_NULLPTR;
    return Depense.value();
}
