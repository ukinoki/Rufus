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

#include "cls_patientsencours.h"

/*!
 * \brief PatientsEnCours::PatientsEnCours
 * Initialise la map Patient
 */

PatientsEnCours::PatientsEnCours(QObject *parent) : ItemsList(parent)
{
    m_patientsencours = new QMap<int, PatientEnCours*>();
}

QMap<int,  PatientEnCours *>* PatientsEnCours::patientsencours() const
{
    return m_patientsencours;
}

/*! charge les données du patient corresondant à l'id *
 * \brief PatientsEnCours::getById
 * \param id l'id du patient recherché
 * \return PatientEnCours* le patient correspondant à l'id
 */
PatientEnCours* PatientsEnCours::getById(int id)
{
    PatientEnCours *pat = Q_NULLPTR;
    QMap<int, PatientEnCours*>::const_iterator itpat = m_patientsencours->find(id);
    if (itpat == m_patientsencours->constEnd())
    {
        pat = DataBase::I()->loadPatientEnCoursById(id);
        if (pat != Q_NULLPTR)
            add(pat);
    }
    else
        pat = itpat.value();
    return pat;
}

/*!
 * \brief PatientsEnCours::addPatient
 * Cette fonction va ajouter le patient passé en paramètre
 * \param patient le patient que l'on veut ajouter
 * \return true si le patient est ajouté
 * \return false si le paramètre patient est un Q_NULLPTR
 * \return false si le patient est déjà présent
 */

bool PatientsEnCours::add(PatientEnCours *patient)
{
    if( patient == Q_NULLPTR)
        return false;
    QMap<int, PatientEnCours*>::const_iterator itpat = m_patientsencours->find(patient->id());
    if (itpat != m_patientsencours->constEnd())
        return false;
    m_patientsencours->insert(patient->id(), patient);
    return true;
}

void PatientsEnCours::addList(QList<PatientEnCours*> listPatientsEnCours)
{
    QList<PatientEnCours*>::const_iterator it;
    for( it = listPatientsEnCours.constBegin(); it != listPatientsEnCours.constEnd(); ++it )
        add( *it );
}

void PatientsEnCours::remove(PatientEnCours* patient)
{
    if (patient == Q_NULLPTR)
        return;
    m_patientsencours->remove(patient->id());
    delete patient;
}
void PatientsEnCours::clearAll()
{
    for( QMap<int, PatientEnCours*>::const_iterator itpat = m_patientsencours->constBegin(); itpat != m_patientsencours->constEnd(); ++itpat)
        delete itpat.value();
    m_patientsencours->clear();
}

void PatientsEnCours::initListeAll()
{
    clearAll();
    QList<PatientEnCours*> listpatienteneours = DataBase::I()->loadPatientsenCoursAll();
    addList(listpatienteneours);
}

void PatientsEnCours::SupprimePatientEnCours(PatientEnCours *pat)
{
    //!. Suppression du patient
    DataBase::I()->SupprRecordFromTable(pat->id(), "idPat", TBL_SALLEDATTENTE);
    remove(pat);
}

void PatientsEnCours::updatePatientEnCours(PatientEnCours *pat)
{
    pat->setData(DataBase::I()->loadPatientEnCoursDataById(pat->id()));
}

void PatientsEnCours::updatePatientEnCoursData(PatientEnCours *pat, QString nomchamp, QVariant value)
{
    QString newvalue;
    if (nomchamp == CP_IDPATSALDAT)
    {
        pat->setid(value.toInt());
        newvalue = (value.toInt() != 0? value.toString() : "null");

    }
    else if (nomchamp == CP_IDUSERSALDAT)
    {
        pat->setiduser(value.toInt());
        newvalue = (value.toInt() != 0? value.toString() : "null");

    }
    else if (nomchamp == CP_STATUTSALDAT)
    {
        pat->setstatut(value.toString());
        newvalue = (value.toString() != ""? "'" + Utils::correctquoteSQL(value.toString()) + "'" : "null");
    }
    else if (nomchamp == CP_HEURESTATUTSALDAT)
    {
        pat->setheurestatut(value.toTime());
        newvalue = (value.toTime().isValid()? "'" + value.toTime().toString("HH:mm:ss") + "'" : "null");
    }
    else if (nomchamp == CP_HEUREARRIVEESALDAT)
    {
        pat->setheurerarrivee(value.toTime());
        newvalue = (value.toTime().isValid()? "'" + value.toTime().toString("HH:mm:ss") + "'" : "null");

    }
    else if (nomchamp == CP_HEURERDVSALDAT)
    {
        pat->setheurerdv(value.toTime());
        newvalue = (value.toTime().isValid()? "'" + value.toTime().toString("HH:mm:ss") + "'" : "null");

    }
    else if (nomchamp == CP_MOTIFSALDAT)
    {
        pat->setmotif(value.toString());
        newvalue = (value.toString() != ""? "'" + Utils::correctquoteSQL(value.toString()) + "'" : "null");
    }
    else if (nomchamp == CP_MESSAGESALDAT)
    {
        pat->setmessage(value.toString());
        newvalue = (value.toString() != ""? "'" + Utils::correctquoteSQL(value.toString()) + "'" : "null");
    }
    else if (nomchamp == CP_IDACTEAPAYERSALDAT)
    {
        pat->setidacteapayer(value.toInt());
        newvalue = (value.toInt() != 0? value.toString() : "null");

    }
    else if (nomchamp == CP_POSTEEXAMENSALDAT)
    {
        pat->setmessage(value.toString());
        newvalue = (value.toString() != ""? "'" + Utils::correctquoteSQL(value.toString()) + "'" : "null");
    }
    else if (nomchamp == CP_IDUSERENCOURSEXAMSALDAT)
    {
        pat->setiduserencoursexam(value.toInt());
        newvalue = (value.toInt() != 0? value.toString() : "null");

    }
    else if (nomchamp == CP_IDSALDATSALDAT)
    {
        pat->setidsaldat(value.toInt());
        newvalue = (value.toInt() != 0? value.toString() : "null");

    }
    QString requete = "UPDATE " TBL_SALLEDATTENTE " SET " + nomchamp + " = " + newvalue + " WHERE idPat = " + QString::number(pat->id());
    DataBase::I()->StandardSQL(requete);
}

PatientEnCours* PatientsEnCours::CreationPatient(int idPat, int idUser, QString Statut, QString Motif, int idActeAPayer, int idUserEnCours, QString PosteExamen, QString Message, QTime heureRDV)
{
    bool ok;
    QString idacteapayer    = (idActeAPayer == 0?   "null" : QString::number(idActeAPayer));
    QString iduser          = (idUser == 0?         QString::number(DataBase::I()->getUserConnected()->getIdUserActeSuperviseur()) : QString::number(idUser));
    QString iduserencours   = (idUserEnCours == 0?  "null" : QString::number(idUserEnCours));
    QString posteexamen     = (PosteExamen == ""?   "null" : "'" + Utils::correctquoteSQL(PosteExamen) + "'");
    QString message         = (Message == ""?       "null" : "'" + Utils::correctquoteSQL(Message) + "'");
    QString motif           = (Motif == ""?         "null" : "'" + Utils::correctquoteSQL(Motif) + "'");
    QString statut          = (Statut == ""?        "null" : "'" + Utils::correctquoteSQL(Statut) + "'");
    QString heurerdv        = (heureRDV == QTime()? "null" : "'" + heureRDV.toString("hh::mm::ss") + "'");
    QString req =     "INSERT INTO " TBL_SALLEDATTENTE
                        " (idPat, idUser, Motif, Statut, HeureStatut, idActeAPayer, idUserEnCoursExam, PosteExamen, Message, heureRDV)"
                        " VALUES (" + QString::number(idPat) + "," +
                                     iduser + "," +
                                     motif + "," +
                                     statut + ",'" +
                                     QTime::currentTime().toString("HH:mm") +"'," +
                                     idacteapayer   + "," +
                                     iduserencours  + "," +
                                     posteexamen    + "," +
                                     message        + "," +
                                     heurerdv       + ")";
    qDebug() << req;
    QString MsgErreur           = tr("Impossible de mettre ce dossier en salle d'attente");
    DataBase::I()->locktables(QStringList() << TBL_SALLEDATTENTE);
    if (!DataBase::I()->StandardSQL(req, MsgErreur))
    {
        DataBase::I()->unlocktables();
        return Q_NULLPTR;
    }
    // Récupération de l'idPatient créé ------------------------------------
    int idpat = DataBase::I()->selectMaxFromTable("idPat", TBL_SALLEDATTENTE, ok, tr("Impossible de sélectionner les enregistrements"));
    DataBase::I()->unlocktables();
    if (!ok ||  idpat == 0)
        return Q_NULLPTR;
    PatientEnCours *pat =   DataBase::I()->loadPatientEnCoursById(idpat);
    add(pat);
    return pat;
}
