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
    m_patients              = new QMap<int, Patient*>();
    m_patientstable         = new QMap<int, Patient*>();
    m_patientssaldat        = new QMap<int, Patient*>();
    m_currentpatient        = new Patient();
    m_dossierpatientaouvrir = new Patient();
    m_currentpatient        ->resetdatas();
    m_dossierpatientaouvrir ->resetdatas();
    m_full                  = false;
}

bool Patients::isfull()
{
    return m_full;
}

void Patients::setcurrentpatient(int id)
{
    m_currentpatient->resetdatas();
    DataBase::I()->loadPatientById(id, m_currentpatient, Item::LoadDetails);
}

void Patients::setdossierpatientaouvrir(int id)
{
    m_dossierpatientaouvrir->resetdatas();
    DataBase::I()->loadPatientById(id, m_dossierpatientaouvrir, Item::LoadDetails);
}



/*! charge les données du patient corresondant à l'id * \brief Patients::getById
 * \param id l'id du patient recherché
 * \param all =false  -> ne charge que les données d'identité - =true -> charge les données sociales et médicales
 * \return Q_NULLPTR si aucun patient trouvé
 * \return Patient* le patient correspondant à l'id
 */
Patient* Patients::getById(int id, Item::LOADDETAILS loadDetails)
{
    if (id == 0)
        return Q_NULLPTR;
    Patient *pat = Q_NULLPTR;
    QMap<int, Patient*>::const_iterator itpat = m_patients->find(id);
    if (itpat == m_patients->constEnd())
    {
        pat = DataBase::I()->loadPatientById(id, pat, loadDetails);
        if (pat != Q_NULLPTR)
            add(m_patients, pat);
    }
    else
    {
        pat = itpat.value();
        if (loadDetails == Item::LoadDetails)
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
    if (pat == Q_NULLPTR)
        return;
    if (!pat->isalloaded() || upd == Item::ForceUpdate)
    {
        QJsonObject jsonPatient = DataBase::I()->loadPatientAllData(pat->id());
        if( !jsonPatient.isEmpty() )
        {
            pat->resetdatas();
            pat->setData(jsonPatient);
        }
    }
    if (m_patients->find(pat->id()) == m_patients->cend())
        add (m_patients, pat);
}

void Patients::initListeSalDat(QList<int> listidaajouter)
{
    /*! on recrée la liste des patients en cours
     */
    QList<Patient*> listpatients = DataBase::I()->loadPatientsByListId(listidaajouter);
    epurelist(m_patientssaldat, &listpatients);
    addList(m_patientssaldat, &listpatients, Item::ForceUpdate);
}

void Patients::initListeTable(QString nom, QString prenom, bool filtre)
{
    /*! on recrée une liste des patients pour remplir la table
     */
    QList<Patient*> listpatients = DataBase::I()->loadPatientsAll(nom, prenom, filtre);
    m_full = (nom == "" && prenom == "");
    epurelist(m_patientstable, &listpatients);
    addList(m_patientstable, &listpatients, Item::ForceUpdate);
}

void Patients::initListeByDDN(QDate DDN)
{
    QList<Patient*> listpatients = (DDN == QDate()? DataBase::I()->loadPatientsAll() : DataBase::I()->loadPatientsByDDN(DDN));
    m_full = (DDN == QDate());
    epurelist(m_patientstable, &listpatients);
    addList(m_patientstable, &listpatients, Item::ForceUpdate);
}

void Patients::SupprimePatient(Patient *pat)
{
    if (pat == Q_NULLPTR)
        return;
    QString requete;
    //!. Suppression des bilans orthoptiques
    DataBase::I()->StandardSQL("DELETE FROM " TBL_BILANORTHO " WHERE idbilanortho in (SELECT idActe from " TBL_ACTES " where idPat = " + QString::number(pat->id()) + ")");
    //!. Suppression des actes
    DataBase::I()->SupprRecordFromTable(pat->id(), "idPat", TBL_ACTES);
    //!. Suppression des documents émis
    DataBase::I()->SupprRecordFromTable(pat->id(), "idPat", TBL_DOCSEXTERNES);
    //!. Suppression des mots cles utilisés
    DataBase::I()->SupprRecordFromTable(pat->id(), "idPat", TBL_MOTSCLESJOINTURES);

    //! Suppression dans la base OPhtalmologie
    DataBase::I()->SupprRecordFromTable(pat->id(), "idPat", TBL_REFRACTIONS);
    DataBase::I()->SupprRecordFromTable(pat->id(), "idPat", TBL_DONNEES_OPHTA_PATIENTS);
    DataBase::I()->SupprRecordFromTable(pat->id(), "idPat", TBL_BIOMETRIES);
    DataBase::I()->SupprRecordFromTable(pat->id(), "idPat", TBL_TONOMETRIE);

    //!. Suppression du patient
    DataBase::I()->SupprRecordFromTable(pat->id(), "idPat", TBL_PATIENTS);
    DataBase::I()->SupprRecordFromTable(pat->id(), "idPat", TBL_DONNEESSOCIALESPATIENTS);
    DataBase::I()->SupprRecordFromTable(pat->id(), "idPat", TBL_RENSEIGNEMENTSMEDICAUXPATIENTS);
    if (pat != m_currentpatient || pat != m_dossierpatientaouvrir)
        remove(m_patients, pat);
    else
        pat->resetdatas();
}

void Patients::updatePatient(Patient *pat)
{
    if (pat == Q_NULLPTR)
        return;
    pat->setData(DataBase::I()->loadPatientAllData(pat->id()));
}

void Patients::updatePatientData(Patient *pat, QString nomchamp, QVariant value)
{
    if (pat == Q_NULLPTR)
        return;
    QString table, newvalue;
    if (nomchamp == CP_ATCDTSOPH_RMP)
    {
        table = TBL_RENSEIGNEMENTSMEDICAUXPATIENTS;
        pat->setatcdtsoph(value.toString());
        newvalue = (value.toString() != ""? "'" + Utils::correctquoteSQL(value.toString()) + "'" : "null");
    }
    else if (nomchamp == CP_ATCDTSFAMLXS_RMP)
    {
        table = TBL_RENSEIGNEMENTSMEDICAUXPATIENTS;
        pat->setatcdtsfam(value.toString());
        newvalue = (value.toString() != ""? "'" + Utils::correctquoteSQL(value.toString()) + "'" : "null");
    }
    else if (nomchamp == CP_ATCDTSPERSOS_RMP)
    {
        table = TBL_RENSEIGNEMENTSMEDICAUXPATIENTS;
        pat->setatcdtsgen(value.toString());
        newvalue = (value.toString() != ""? "'" + Utils::correctquoteSQL(value.toString()) + "'" : "null");
    }
    else if (nomchamp == CP_TRAITMTGEN_RMP)
    {
        table = TBL_RENSEIGNEMENTSMEDICAUXPATIENTS;
        pat->settraitemntsgen(value.toString());
        newvalue = (value.toString() != ""? "'" + Utils::correctquoteSQL(value.toString()) + "'" : "null");
    }
    else if (nomchamp == CP_TRAITMTOPH_RMP)
    {
        table = TBL_RENSEIGNEMENTSMEDICAUXPATIENTS;
        pat->settraitemntsoph(value.toString());
        newvalue = (value.toString() != ""? "'" + Utils::correctquoteSQL(value.toString()) + "'" : "null");
    }
    else if (nomchamp == CP_TABAC_RMP)
    {
        table = TBL_RENSEIGNEMENTSMEDICAUXPATIENTS;
        pat->settabac(value.toString());
        newvalue = (value.toString() != ""? "'" + Utils::correctquoteSQL(value.toString()) + "'" : "null");
    }
    else if (nomchamp == CP_AUTRESTOXIQUES_RMP)
    {
        table = TBL_RENSEIGNEMENTSMEDICAUXPATIENTS;
        pat->setautrestoxiques(value.toString());
        newvalue = (value.toString() != ""? "'" + Utils::correctquoteSQL(value.toString()) + "'" : "null");
    }
    else if (nomchamp == CP_IMPORTANT_RMP)
    {
        table = TBL_RENSEIGNEMENTSMEDICAUXPATIENTS;
        pat->setimportant(value.toString());
        newvalue = (value.toString() != ""? "'" + Utils::correctquoteSQL(value.toString()) + "'" : "null");
    }
    else if (nomchamp == CP_RESUME_RMP)
    {
        table = TBL_RENSEIGNEMENTSMEDICAUXPATIENTS;
        pat->setresume(value.toString());
        newvalue = (value.toString() != ""? "'" + Utils::correctquoteSQL(value.toString()) + "'" : "null");
    }
    else if (nomchamp == CP_IDMG_RMP)
    {
        table = TBL_RENSEIGNEMENTSMEDICAUXPATIENTS;
        pat->setmg(value.toInt());
        newvalue = (value.toInt() != 0? value.toString() : "null");

    }
    else if (nomchamp == CP_IDSPE1_RMP)
    {
        table = TBL_RENSEIGNEMENTSMEDICAUXPATIENTS;
        pat->setspe1(value.toInt());
        newvalue = (value.toInt() != 0? value.toString() : "null");

    }
    else if (nomchamp == CP_IDSPE2_RMP)
    {
        table = TBL_RENSEIGNEMENTSMEDICAUXPATIENTS;
        pat->setspe2(value.toInt());
        newvalue = (value.toInt() != 0? value.toString() : "null");
    }
    QString requete = "UPDATE " + table + " SET " + nomchamp + " = " + newvalue + " WHERE idPat = " + QString::number(pat->id());
    DataBase::I()->StandardSQL(requete);
}

void Patients::updateCorrespondant(Patient *pat, Correspondant::typecorrespondant type, Correspondant *cor)
{
    if (pat == Q_NULLPTR)
        return;
    int id = (cor != Q_NULLPTR ? cor->id() : 0);
    QString field;
    switch (type) {
    case Correspondant::MG:
        field = CP_IDMG_RMP;
        pat->setmg(id);
        break;
    case Correspondant::Spe1:
        field = CP_IDSPE1_RMP;
        pat->setspe1(id);
        break;
    case Correspondant::Spe2:
        pat->setspe2(id);
        field = CP_IDSPE2_RMP;
    }
    QString idsql = (cor != Q_NULLPTR ? QString::number(cor->id()) : "null");
    DataBase::I()->StandardSQL("update " TBL_RENSEIGNEMENTSMEDICAUXPATIENTS " set " + field + " = " + idsql +
                " where idpat = " + QString::number(pat->id()));
}

Patient* Patients::CreationPatient(QHash<QString, QVariant> sets)
{
    Patient *pat = Q_NULLPTR;
    DataBase::I()->locktables(QStringList() << TBL_PATIENTS << TBL_DONNEESSOCIALESPATIENTS << TBL_RENSEIGNEMENTSMEDICAUXPATIENTS );
    sets[CP_DATECREATION_PATIENTS] = DataBase::I()->ServerDateTime().date();
    sets[CP_IDCREATEUR_PATIENTS]   = DataBase::I()->getUserConnected()->id();
    bool result = DataBase::I()->InsertSQLByBinds(TBL_PATIENTS, sets);
    if (!result)
    {
        UpMessageBox::Watch(Q_NULLPTR,tr("Impossible d'enregistrer ce patient dans la base!"));
        DataBase::I()->unlocktables();
        return pat;
    }
    // Récupération de l'idpatient créé ------------------------------------
    int id = 0;
    QHash<QString, QVariant>::const_iterator itx = sets.find(CP_IDPAT_PATIENTS);
    if (itx != sets.constEnd())
        id = itx.value().toInt();
    else
    {
        bool ok;
        id = DataBase::I()->selectMaxFromTable(CP_IDPAT_PATIENTS, TBL_PATIENTS, ok, tr("Impossible de sélectionner les enregistrements"));
        if (!ok)
        {
            DataBase::I()->unlocktables();
            return Q_NULLPTR;
        }
    }
    if (id == 0)
    {
        DataBase::I()->unlocktables();
        return Q_NULLPTR;
    }
    QJsonObject  data = QJsonObject{};
    data[CP_IDPAT_PATIENTS] = id;
    data[CP_IDCREATEUR_PATIENTS] = DataBase::I()->getUserConnected()->id();
    data[CP_DATECREATION_PATIENTS] = QDate::currentDate().toString("yyyy-MM-dd");
    QString champ;
    QVariant value;
    for (QHash<QString, QVariant>::const_iterator itset = sets.constBegin(); itset != sets.constEnd(); ++itset)
    {
        champ  = itset.key();
        if (champ == CP_NOM_PATIENTS)                   data[champ] = itset.value().toString();
        else if (champ == CP_PRENOM_PATIENTS)           data[champ] = itset.value().toString();
        else if (champ == CP_DDN_PATIENTS)              data[champ] = itset.value().toDate().toString("yyyy-MM-dd");
        else if (champ == CP_SEXE_PATIENTS)             data[champ] = itset.value().toString();
        else if (champ == CP_DATECREATION_PATIENTS)     data[champ] = itset.value().toDate().toString("yyyy-MM-dd");
        else if (champ == CP_IDCREATEUR_PATIENTS)       data[champ] = itset.value().toInt();
        else if (champ == CP_IDLIEU_PATIENTS)           data[champ] = itset.value().toInt();
        else if (champ == CP_COMMENTAIRE_PATIENTS)      data[champ] = itset.value().toString();
    }
    pat = new Patient(data);
    QString req = "INSERT INTO " TBL_DONNEESSOCIALESPATIENTS " (idPat) VALUES ('" + QString::number(pat->id()) + "')";
    DataBase::I()->StandardSQL(req,tr("Impossible de créer les données sociales"));
    req = "INSERT INTO " TBL_RENSEIGNEMENTSMEDICAUXPATIENTS " (idPat) VALUES ('" + QString::number(pat->id()) + "')";
    DataBase::I()->StandardSQL(req,tr("Impossible de créer les renseignements médicaux"));
    DataBase::I()->unlocktables();
    return pat;
}
