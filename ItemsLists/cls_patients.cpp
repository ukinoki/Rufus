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
Patients::Patients(QObject *parent) : ItemsList(parent)
{
    m_patients = new QMap<int, Patient*>();
    m_full  = false;
}

QMap<int,  Patient *> *Patients::patients() const
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
Patient* Patients::getById(int id, LOADDETAILS loadDetails)
{
    Patient *pat = Q_NULLPTR;
    QMap<int, Patient*>::const_iterator itpat = m_patients->find(id);
    if (itpat == m_patients->constEnd())
        pat = DataBase::I()->loadPatientById(id, pat, loadDetails);
    else
    {
        pat = itpat.value();
        if (loadDetails == LoadDetails)
            if (!pat->isalloaded())
            {
                QJsonObject jsonPatient = DataBase::I()->loadAllDataPatientById(id);
                if( jsonPatient.isEmpty() )
                    return Q_NULLPTR;
                else
                    pat->setData(jsonPatient);
            }
    }
    return pat;
}

void Patients::loadAll(Patient *pat)
{
    QMap<int, Patient*>::const_iterator itpat = m_patients->find(pat->id());
    if (itpat == m_patients->constEnd())
        pat = DataBase::I()->loadPatientById(pat->id(), pat, true);
    else if (!itpat.value()->isalloaded())
    {
        QJsonObject jsonPatient = DataBase::I()->loadAllDataPatientById(pat->id());
        if( !jsonPatient.isEmpty() )
            pat->setData(jsonPatient);
    }
}

void Patients::reloadMedicalData(Patient *pat)
{
    QJsonObject jData{};
    jData["id"] = pat->id();
    bool ok;
    DataBase::I()->loadMedicalDataPatient(jData, ok);
    if( !jData.isEmpty() )
        pat->setMedicalData(jData);
}

void Patients::reloadSocialData(Patient *pat)
{
    QJsonObject jData{};
    jData["id"] = pat->id();
    bool ok;
    DataBase::I()->loadSocialDataPatient(jData, ok);
    if( !jData.isEmpty() )
        pat->setSocialData(jData);
}


/*!
 * \brief Patients::addPatient
 * Cette fonction va ajouter le patient passé en paramètre
 * \param patient le patient que l'on veut ajouter
 * \return true si le patient est ajouté
 * \return false si le paramètre patient est un Q_NULLPTR
 * \return false si le patient est déjà présent
 */
bool Patients::add(Patient *patient)
{
    if( patient == Q_NULLPTR)
        return false;
    QMap<int, Patient*>::const_iterator itpat = m_patients->find(patient->id());
    if (itpat != m_patients->constEnd())
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
    if (patient == Q_NULLPTR)
        return;
    m_patients->remove(patient->id());
    delete patient;
}
void Patients::clearAll()
{
    for( QMap<int, Patient*>::const_iterator itpat = m_patients->constBegin(); itpat != m_patients->constEnd(); ++itpat)
        delete itpat.value();
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

