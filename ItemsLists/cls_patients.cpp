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

/*!
 * \brief Patients::Patients
 * Initialise la map Patient
 */
Patients::Patients()
{
    m_patients = new QList<Patient*>();
    m_full  = false;
}

QList< Patient *> *Patients::patients() const
{
    return m_patients;
}

bool Patients::isfull()
{
    return m_full;
}

/*! charge les données du patient corresondant à l'id * \brief Patients::getById
 * \param id l'id du patient recherché
 * \param all =false  -> ne charge que les données d'identité - =true -> charge les données sociales et médicales
 * \return Q_NULLPTR si aucun patient trouvé
 * \return Patient* le patient correspondant à l'id
 */
Patient* Patients::getById(int id, bool all)
{
    Patient* pat = Q_NULLPTR;
    for (int i=0; i< m_patients->size(); i++)
        if (m_patients->at(i)->id() == id)
        {
            pat = m_patients->at(i);
            if (all)
                if (!pat->isalloaded())
                {
                    bool ok;
                    DataBase::I()->loadMedicalDataPatient(pat, ok);
                    DataBase::I()->loadSocialDataPatient(pat, ok);
                }
            break;
        }
    if (pat == Q_NULLPTR)
    {
        pat = DataBase::I()->loadPatientById(id, all);
        m_patients->append(pat);
    }
    return pat;
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
    if( m_patients->contains(patient) )
        return false;
    m_patients->insert(patient->id(), patient);
    return true;
}

void Patients::addList(QList<Patient*> listpatients)
{
    QList<Patient*>::const_iterator it;
    for( it = listpatients.constBegin(); it != listpatients.constEnd(); ++it )
        add( *it );
}

void Patients::remove(Patient* patient)
{
    m_patients->removeOne(patient);
    delete  patient;
}
void Patients::clearAll()
{
    while (m_patients->size() >0)
        remove(m_patients->at(0));
    m_patients->clear();
}

void Patients::initListeAll(QString nom, QString prenom, bool filtre)
{
    clearAll();
    m_patients  = DataBase::I()->loadPatientsAll(nom, prenom, filtre);
    m_full = (nom == "" && prenom == "");
}

void Patients::initListeByDDN(QDate DDN)
{
    clearAll();
    if (DDN == QDate())
        m_patients = DataBase::I()->loadPatientsAll();
    else
        m_patients = DataBase::I()->loadPatientsByDDN(DDN);
    m_full = (DDN == QDate());
}

