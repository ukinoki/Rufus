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

#include "cls_patients.h"

/*
 * GETTER
*/
QMap<int, Patient *> *Patients::getPatients() const
{
    return m_patients;
}


/*!
 * \brief Patients::Patients
 * Initialise la map Patient
 */
Patients::Patients()
{
    m_patients = new QMap<int, Patient*>();
}

/*!
 * \brief Patients::addPatient
 * Cette fonction va ajouter le patient passé en paramètre
 *
 * \param patient le patient que l'on veut ajouter
 * \return true si le patient est ajouté
 * \return false si le paramètre patient est un Q_NULLPTR
 * \return false si le patient est déjà présent
 */
bool Patients::add(Patient *patient)
{
    if( patient == Q_NULLPTR)
        return false;

    if( m_patients->contains(patient->id()) )
        return false;

    m_patients->insert(patient->id(), patient);

    return true;
}

/*!
 * \brief Patients::getById
 * \param id l'id du patient recherché
 * \return Q_NULLPTR si aucun patient trouvé
 * \return Patient* le patient correspondant à l'id
 */
Patient* Patients::getById(int id)
{
    QMap<int, Patient*>::const_iterator patient = m_patients->find(id);
    if( patient == m_patients->constEnd() )
        return Q_NULLPTR;
    return patient.value();
}
