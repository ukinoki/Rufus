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
 * \return false si le paramètre patient est un nullptr
 * \return false si le patient est déjà présent
 */
bool Patients::addPatient(Patient *patient)
{
    if( patient == nullptr)
        return false;

    if( m_patients->contains(patient->id()) )
        return false;

    m_patients->insert(patient->id(), patient);

    return true;
}

/*!
 * \brief Patients::getPatientById
 * \param id l'id du patient recherché
 * \return nullptr si aucun patient trouvé
 * \return Patient* le patient correspondant à l'id
 */
Patient* Patients::getPatientById(int id)
{
    QMap<int, Patient*>::const_iterator patient = m_patients->find(id);
    if( patient == m_patients->constEnd() )
        return nullptr;
    return patient.value();
}
