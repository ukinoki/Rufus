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

#ifndef CLS_PATIENTSENCOURS_H
#define CLS_PATIENTSENCOURS_H


#include <cls_patientencours.h>
#include "database.h"
#include "cls_itemslist.h"

class PatientsEnCours : public ItemsList
{

public:
    explicit PatientsEnCours(QObject *parent = Q_NULLPTR);

    QMap<int, PatientEnCours*> *patientsencours() const;
    PatientEnCours* getById(int id);                                                         /*! charge les données du patient corresondant à l'id
                                                                                             * \brief PatientsEnCours::getById
                                                                                             * \param id l'id du patient recherché
                                                                                             * \return Q_NULLPTR si aucun patient trouvé
                                                                                             * \return PatientEnCours* le patient correspondant à l'id
                                                                                             */

    void initListeAll();          /*! crée une liste de patients */

private:
    QMap<int, PatientEnCours*> *m_patientsencours;                                                        //!< une liste de patients
    void addList(QList<PatientEnCours*> listpatientss);

public:

    //!> actions combinées sur l'item et l'enregistrement correspondant en base de données

    //!> update sur les champs
    //! géré par la fonction static update de cls_itemslist

    //!> actions sur les enregistrements
    void                SupprimePatientEnCours(PatientEnCours *pat);
    PatientEnCours*     CreationPatient(int idPat, int idUser , QString Statut, QTime heureStatut, QTime heureRDV = QTime(),
                                        QTime heureArrivee = QTime(), QString Motif = "", QString Message = "", int idActeAPayer = 0, QString PosteExamen = "",
                                        int idUserEnCours = 0, int idSalDat = 0);

    //!< action sur toutes les données
    void    updatePatientEnCours(PatientEnCours* pat);                                                    //!> met à jour les datas d'un patient à partir des données enregistrées dans la base

};
#endif // CLS_PATIENTSENCOURS_H