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

#include "cls_correspondants.h"

/*
 * GETTER
*/
QMap<int, Correspondant *> *Correspondants::getCorrespondants() const
{
    return m_Correspondants;
}

QMap<int, Correspondant *> *Correspondants::getMGCorrespondants() const
{
    return m_MGCorrespondants;
}


/*!
 * \brief Correspondants::Correspondants
 * Initialise la map Correspondants
 */
Correspondants::Correspondants()
{
    m_Correspondants = new QMap<int, Correspondant*>();
    m_MGCorrespondants = new QMap<int, Correspondant*>();
}

void Correspondants::VideLesListes()
{
    m_Correspondants->clear();
    m_MGCorrespondants->clear();
}

/*!
 * \brief Correspondants::addCorrespondant
 * Cette fonction va ajouter le correspondant passé en paramètre
 *
 * \param Correspondant le correspondant que l'on veut ajouter
 * \return true si le correspondant est ajouté
 * \return false si le paramètre correspondant est un nullptr
 * \return false si le correspondant est déjà présent
 */
bool Correspondants::addCorrespondant(Correspondant *Correspondant)
{
    if( Correspondant == Q_NULLPTR)
        return false;

    if( m_Correspondants->contains(Correspondant->id()) )
        return false;

    m_Correspondants->insert(Correspondant->id(), Correspondant);

    return true;
}

bool Correspondants::addMGCorrespondant(Correspondant *Correspondant)
{
    if( Correspondant == Q_NULLPTR)
        return false;

    if( m_MGCorrespondants->contains(Correspondant->id()) )
        return false;

    m_MGCorrespondants->insert(Correspondant->id(), Correspondant);

    return true;
}

/*!
 * \brief Correspondants::getCorrespondantById
 * \param id l'id du Correspondant recherché
 * \return nullptr si aucun Correspondant trouvé
 * \return Correspondant* le Correspondant correspondant à l'id
 */
Correspondant* Correspondants::getCorrespondantById(int id)
{
    QMap<int, Correspondant*>::const_iterator Correspondant = m_Correspondants->find(id);
    if( Correspondant == m_Correspondants->constEnd() )
        return Q_NULLPTR;
    return Correspondant.value();
}
