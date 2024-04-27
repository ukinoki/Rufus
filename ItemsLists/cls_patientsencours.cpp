/* (C) 2020 LAINE SERGE
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
    map_all = new QMap<int, PatientEnCours*>();
}

QMap<int,  PatientEnCours *>* PatientsEnCours::patientsencours() const
{
    return map_all;
}

/*! charge les données du patient corresondant à l'id *
 * \brief PatientsEnCours::getById
 * \param id l'id du patient recherché
 * \return PatientEnCours* le patient correspondant à l'id
 */
PatientEnCours* PatientsEnCours::getById(int id)
{
    PatientEnCours *pat = Q_NULLPTR;
    QMap<int, PatientEnCours*>::const_iterator itpat = map_all->constFind(id);
    if (itpat == map_all->constEnd())
    {
        pat = DataBase::I()->loadPatientEnCoursById(id);
        if (pat != Q_NULLPTR)
            add(map_all, pat);
        auto it = map_all->constFind(id);
        return (it != map_all->cend()? const_cast<PatientEnCours*>(it.value()) : Q_NULLPTR);
    }
    else
        return itpat.value();
}

void PatientsEnCours::initListeAll()
{
    QList<PatientEnCours*> listpatencours = DataBase::I()->loadPatientsenCoursAll();
    epurelist(map_all, &listpatencours);
    addList(map_all, &listpatencours, Item::Update);
}

void PatientsEnCours::SupprimePatientEnCours(PatientEnCours *pat)
{
    Supprime(map_all, pat);
}

void PatientsEnCours::updatePatientEnCours(PatientEnCours *pat)
{
    if (pat == Q_NULLPTR)
        return;
    pat->setData(DataBase::I()->loadPatientEnCoursDataById(pat->id()));
}


PatientEnCours* PatientsEnCours::CreationPatient(int idPat, User* usr , QString Statut, QTime heureStatut, QTime heureRDV,
                                                 QTime heureArrivee, QString Motif, QString Message, int idActeAPayer, QString PosteExamen,
                                                 int idUserEnCours, int idSalDat)

{
    QString iduser          = (!usr ?                       "null" : QString::number(usr->id()) );
    QString statut          = (Statut == ""?                "null" : "'" + Utils::correctquoteSQL(Statut) + "'");
    QString heurestatut     = (heureStatut == QTime()?      "null" : "'" + heureStatut.toString("hh:mm:ss") + "'");
    QString heurerdv        = (heureRDV == QTime()?         "null" : "'" + heureRDV.toString("hh:mm:ss") + "'");
    QString heurearrivee    = (heureArrivee == QTime()?     "null" : "'" + heureArrivee.toString("hh:mm:ss") + "'");
    QString motif           = (Motif == ""?                 "null" : "'" + Utils::correctquoteSQL(Motif) + "'");
    QString message         = (Message == ""?               "null" : "'" + Utils::correctquoteSQL(Message) + "'");
    QString idacteapayer    = (idActeAPayer == 0?           "null" : QString::number(idActeAPayer));
    QString posteexamen     = (PosteExamen == ""?           "null" : "'" + Utils::correctquoteSQL(PosteExamen) + "'");
    QString iduserencours   = (idUserEnCours == 0?          "null" : QString::number(idUserEnCours));
    QString idsaldat        = (idSalDat == 0?               "null" : QString::number(idSalDat));

    QString req = "INSERT INTO " TBL_SALLEDATTENTE
                        " (" CP_IDPAT_SALDAT ","
                             CP_IDUSERSUPERVISEUR_SALDAT ","
                             CP_STATUT_SALDAT ","
                             CP_HEURESTATUT_SALDAT ","
                             CP_HEURERDV_SALDAT ","
                             CP_HEUREARRIVEE_SALDAT ","
                             CP_MOTIF_SALDAT ","
                             CP_MESSAGE_SALDAT ","
                             CP_IDACTEAPAYER_SALDAT ","
                             CP_POSTEEXAMEN_SALDAT ","
                             CP_IDUSERENCOURSEXAM_SALDAT ","
                             CP_IDSALDAT_SALDAT ")"
                        " VALUES (" +   QString::number(idPat) + "," +
                                        iduser + "," +
                                        statut + "," +
                                        heurestatut   + "," +
                                        heurerdv   + "," +
                                        heurearrivee   + "," +
                                        motif + "," +
                                        message + "," +
                                        idacteapayer + "," +
                                        posteexamen    + "," +
                                        iduserencours  + "," +
                                        idsaldat +")";
    QString MsgErreur = tr("Impossible de mettre ce dossier en salle d'attente");
    DataBase::I()->locktables(QStringList() << TBL_SALLEDATTENTE);
    if (!DataBase::I()->StandardSQL(req, MsgErreur))
    {
        DataBase::I()->unlocktables();
        return Q_NULLPTR;
    }
    // Récupération de l'idPatient créé ------------------------------------
    int idpat = DataBase::I()->selectMaxFromTable("idPat", TBL_SALLEDATTENTE, m_ok, tr("Impossible de sélectionner les enregistrements"));
    if (!m_ok)
    {
        DataBase::I()->unlocktables();
        return Q_NULLPTR;
    }
    DataBase::I()->unlocktables();
    PatientEnCours *pat = new PatientEnCours();
    pat->setid(idpat);
    pat->setidusersuperviseur(usr->id());
    if (Statut != "")
        pat->setstatut(Statut);
    if (heureStatut != QTime())
        pat->setheurestatut(heureStatut);
    if (heureRDV != QTime())
        pat->setheurerdv(heureRDV);
    if (heureArrivee != QTime())
        pat->setheurearrivee(heureArrivee);
    if (Motif != "")
        pat->setmotif(Motif);
    if (Message != "")
        pat->setmessage(Message);
    if (idActeAPayer != 0)
        pat->setidacteapayer(idActeAPayer);
    if (PosteExamen != "")
        pat->setposteexamen(PosteExamen);
    if (idUserEnCours != 0)
        pat->setiduserencoursexam(idUserEnCours);
    if (idSalDat != 0)
        pat->setidsaldat(idSalDat);
    add(map_all, pat);
    return pat;
}
