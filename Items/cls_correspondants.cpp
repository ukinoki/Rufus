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

#include "cls_correspondants.h"

/*
 * GETTER
*/
QMap<int, Correspondant *> *Correspondants::getCorrespondants() const
{
    return m_Correspondants;
}

/*!
 * \brief Correspondants::Correspondants
 * Initialise la map Correspondants
 */
Correspondants::Correspondants()
{
    m_Correspondants = new QMap<int, Correspondant*>();
}

void Correspondants::clearAll()
{
    QList<Correspondant*> listcors;
    for( QMap<int, Correspondant*>::const_iterator itcor = m_Correspondants->constBegin(); itcor != m_Correspondants->constEnd(); ++itcor)
        delete itcor.value();
    m_Correspondants->clear();
}

void Correspondants::removeCorrespondant(Correspondant *cor)
{
    if (cor == Q_NULLPTR)
        return;
    QMap<int, Correspondant*>::const_iterator itcor;
    m_Correspondants->find(cor->id());
    if( itcor == m_Correspondants->constEnd() )
        return;
    m_Correspondants->remove(cor->id());
    delete cor;
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
bool Correspondants::addCorrespondant(Correspondant *cor)
{
    if( cor == Q_NULLPTR)
        return false;
    if( m_Correspondants->contains(cor->id()) )
        return false;
    m_Correspondants->insert(cor->id(), cor);
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
    QMap<int, Correspondant*>::const_iterator itcor = m_Correspondants->find(id);
    if( itcor == m_Correspondants->constEnd() )
        return Q_NULLPTR;
    return itcor.value();
}
