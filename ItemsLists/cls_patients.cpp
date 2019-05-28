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
Patient* Patients::getById(int id, Item::LOADDETAILS loadDetails)
{
    Patient *pat = Q_NULLPTR;
    QMap<int, Patient*>::const_iterator itpat = m_patients->find(id);
    if (itpat == m_patients->constEnd())
    {
        pat = DataBase::I()->loadPatientById(id, pat, loadDetails);
        if (pat != Q_NULLPTR)
            add(pat);
    }
    else
    {
        pat = itpat.value();
        if (loadDetails == Item::LoadDetails)
            if (!pat->isalloaded())
            {
                QJsonObject jsonPatient = DataBase::I()->loadPatientAllData(id);
                if( jsonPatient.isEmpty() )
                    return Q_NULLPTR;
                else
                    pat->setData(jsonPatient);
            }
    }
    return pat;
}

void Patients::loadAll(Patient *pat, Item::UPDATE upd)
{
    QMap<int, Patient*>::const_iterator itpat = m_patients->find(pat->id());
    if (itpat == m_patients->constEnd())
        add(pat);
    if (!pat->isalloaded() || upd == Item::ForceUpdate)
    {
        QJsonObject jsonPatient = DataBase::I()->loadPatientAllData(pat->id());
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

void Patients::SupprimePatient(Patient *pat)
{
    //!. Suppression des bilans orthoptiques
    QString requete;
    DataBase::I()->StandardSQL("DELETE FROM " NOM_TABLE_BILANORTHO " WHERE idbilanortho in (SELECT idActe from " NOM_TABLE_ACTES " where idPat = " + QString::number(pat->id()) + ")");
    //!. Suppression des actes
    DataBase::I()->SupprRecordFromTable(pat->id(), "idPat", NOM_TABLE_ACTES);
    //!. Suppression des documents émis
    DataBase::I()->SupprRecordFromTable(pat->id(), "idPat", NOM_TABLE_IMPRESSIONS);
    //!. Suppression des mots cles utilisés
    DataBase::I()->SupprRecordFromTable(pat->id(), "idPat", NOM_TABLE_MOTSCLESJOINTURES);
    //!. Suppression de la salle d'attente
    DataBase::I()->SupprRecordFromTable(pat->id(), "idPat", NOM_TABLE_SALLEDATTENTE);

    //! Suppression dans la base OPhtalmologie
    DataBase::I()->SupprRecordFromTable(pat->id(), "idPat", NOM_TABLE_REFRACTION);
    DataBase::I()->SupprRecordFromTable(pat->id(), "idPat", NOM_TABLE_DONNEES_OPHTA_PATIENTS);
    DataBase::I()->SupprRecordFromTable(pat->id(), "idPat", NOM_TABLE_BIOMETRIES);
    DataBase::I()->SupprRecordFromTable(pat->id(), "idPat", NOM_TABLE_TONOMETRIE);

    //!. Suppression du patient
    DataBase::I()->SupprRecordFromTable(pat->id(), "idPat", NOM_TABLE_PATIENTS);
    DataBase::I()->SupprRecordFromTable(pat->id(), "idPat", NOM_TABLE_DONNEESSOCIALESPATIENTS);
    DataBase::I()->SupprRecordFromTable(pat->id(), "idPat", NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS);
    remove(pat);
}

Patient* Patients::CreationPatient(QString nom, QString prenom, QDate datedenaissance, QString sexe)
{
    bool ok;
    QString req;
    req =   "INSERT INTO " NOM_TABLE_PATIENTS
            " (PatNom, PatPrenom, PatDDN, PatCreele, PatCreePar, Sexe) "
            " VALUES ('" +
            Utils::correctquoteSQL(nom) + "', '" +
            Utils::correctquoteSQL(prenom) + "', '" +
            datedenaissance.toString("yyyy-MM-dd") +
            "', NOW(), '" +
            QString::number(DataBase::I()->getUserConnected()->id()) +"' , '" +
            sexe +
            "');";

    DataBase::I()->locktables(QStringList() << NOM_TABLE_PATIENTS << NOM_TABLE_DONNEESSOCIALESPATIENTS << NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS);
    if (!DataBase::I()->StandardSQL(req, tr("Impossible de créer le dossier")))
    {
        DataBase::I()->unlocktables();
        return Q_NULLPTR;
    }
    // Récupération de l'idPatient créé ------------------------------------
    int idpat = DataBase::I()->selectMaxFromTable("idPat", NOM_TABLE_PATIENTS, ok, tr("Impossible de sélectionner les enregistrements"));
    if (!ok ||  idpat == 0)
    {
        DataBase::I()->unlocktables();
        return Q_NULLPTR;
    }

    Patient *pat = DataBase::I()->loadPatientById(idpat);
    req = "INSERT INTO " NOM_TABLE_DONNEESSOCIALESPATIENTS " (idPat) VALUES ('" + QString::number(pat->id()) + "')";
    DataBase::I()->StandardSQL(req,tr("Impossible de créer les données sociales"));
    req = "INSERT INTO " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS " (idPat) VALUES ('" + QString::number(pat->id()) + "')";
    DataBase::I()->StandardSQL(req,tr("Impossible de créer les renseignements médicaux"));
    DataBase::I()->unlocktables();
    return pat;
}

void Patients::updatePatient(Patient *pat)
{
    pat->setData(DataBase::I()->loadPatientAllData(pat->id()));
}

void Patients::updatePatientData(Patient *pat, QString nomchamp, QVariant value)
{
    QString table, newvalue;
    if (nomchamp == CP_ATCDTSOPHRMP)
    {
        table = NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS;
        pat->setatcdtsoph(value.toString());
        newvalue = "'" + Utils::correctquoteSQL(value.toString()) + "'";
    }
    else if (nomchamp == CP_ATCDTSFAMLXSRMP)
    {
        table = NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS;
        pat->setatcdtsfam(value.toString());
        newvalue = "'" + Utils::correctquoteSQL(value.toString()) + "'";
    }
    else if (nomchamp == CP_ATCDTSPERSOSRMP)
    {
        table = NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS;
        pat->setatcdtsgen(value.toString());
        newvalue = "'" + Utils::correctquoteSQL(value.toString()) + "'";
    }
    else if (nomchamp == CP_TRAITMTGENRMP)
    {
        table = NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS;
        pat->settraitemntsgen(value.toString());
        newvalue = "'" + Utils::correctquoteSQL(value.toString()) + "'";
    }
    else if (nomchamp == CP_TRAITMTOPHRMP)
    {
        table = NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS;
        pat->settraitemntsoph(value.toString());
        newvalue = "'" + Utils::correctquoteSQL(value.toString()) + "'";
    }
    else if (nomchamp == CP_TABACRMP)
    {
        table = NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS;
        pat->settabac(value.toString());
        newvalue = "'" + Utils::correctquoteSQL(value.toString()) + "'";
    }
    else if (nomchamp == CP_AUTRESTOXIQUESRMP)
    {
        table = NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS;
        pat->setautrestoxiques(value.toString());
        newvalue = "'" + Utils::correctquoteSQL(value.toString()) + "'";
    }
    else if (nomchamp == CP_IMPORTANTRMP)
    {
        table = NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS;
        pat->setimportant(value.toString());
        newvalue = "'" + Utils::correctquoteSQL(value.toString()) + "'";
    }
    else if (nomchamp == CP_RESUMERMP)
    {
        table = NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS;
        pat->setresume(value.toString());
        newvalue = "'" + Utils::correctquoteSQL(value.toString()) + "'";
    }
    else if (nomchamp == CP_IDMGRMP)
    {
        table = NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS;
        pat->setmg(value.toInt());
        newvalue = QString::number(value.toInt());
    }
    else if (nomchamp == CP_IDSPE1RMP)
    {
        table = NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS;
        pat->setspe1(value.toInt());
        newvalue = QString::number(value.toInt());
    }
    else if (nomchamp == CP_IDSPE2RMP)
    {
        table = NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS;
        pat->setspe2(value.toInt());
        newvalue = QString::number(value.toInt());
    }
    QString requete = "UPDATE " + table + " SET " + nomchamp + " = " + newvalue + " WHERE idPat = " + QString::number(pat->id());
    DataBase::I()->StandardSQL(requete);
}

void Patients::updateCorrespondant(Patient *pat, DataBase::typecorrespondant type, Correspondant *cor)
{
    int id = (cor != Q_NULLPTR ? cor->id() : 0);
    QString field;
    switch (type) {
    case DataBase::MG:
        field = CP_IDMGRMP;
        pat->setmg(id);
        break;
    case DataBase::Spe1:
        field = CP_IDSPE1RMP;
        pat->setspe1(id);
        break;
    case DataBase::Spe2:
        pat->setspe2(id);
        field = CP_IDSPE2RMP;
    }
    QString idsql = (cor != Q_NULLPTR ? QString::number(cor->id()) : "null");
    DataBase::I()->StandardSQL("update " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS " set " + field + " = " + idsql +
                " where idpat = " + QString::number(pat->id()));
}

