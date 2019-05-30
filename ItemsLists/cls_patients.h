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

#ifndef CLS_PATIENTS_H
#define CLS_PATIENTS_H

#include <cls_patient.h>
#include "database.h"
#include "cls_itemslist.h"

class Patients : public ItemsList
{

public:
    explicit Patients(QObject *parent = Q_NULLPTR);

    //GETTER
    QMap<int, Patient*> *patients() const;


    Patient* getById(int id, Item::LOADDETAILS loadDetails = Item::NoLoadDetails);          /*! charge les données du patient corresondant à l'id
                                                                                             * \brief Patients::getById
                                                                                             * \param id l'id du patient recherché
                                                                                             * \param loadDetails = NoLoadDetails  -> ne charge que les données d'identité - = LoadDetails -> charge les données sociales et médicales
                                                                                             * \return Q_NULLPTR si aucun patient trouvé
                                                                                             * \return Patient* le patient correspondant à l'id
                                                                                             */
    bool isfull();                                                                          /*! la liste contient tous les patients de la base */
    bool add(Patient *patient);
    void addList(QList<Patient*> listpatientss);
    void loadAll(Patient *pat, Item::UPDATE upd = Item::NoUpdate);                          /*! charge toutes les données d'un  patient si ce n'est pas le cas
                                                                                             * \param upd force ou non la recharge depuis la BDD si elles sont déjà chargées
                                                                                             */
    void reloadMedicalData(Patient* pat);                                                   //!> recharge les données médicales d'un patient
    void reloadSocialData(Patient* pat);                                                    //!> recharge les données sociales d'un patient
    void remove(Patient* patient);
    void clearAll();
    void initListeAll(QString nom = "", QString prenom = "", bool filtre = false);          /*! crée une liste de patients
                                                                                            * \param patnom filtrer sur le nom de patient
                                                                                            * \param patprenom filtrer sur le prénom de patient
                                                                                            * \param le filtre se fait sur des valeurs aprrochantes */

    void initListeByDDN(QDate DDN = QDate());                                               /*! crée une liste de tous les patients pour une date de naissance
                                                                                            * \param DDN la date de naissance */
private:
    QMap<int, Patient*> *m_patients;                                                        //!< une liste de patients
    bool m_full;                                                                            //! la liste contient tous les patients de la base


public:


    //!> actions combinées sur l'item et l'enregistrement correspondant en base de données

    //!> actions sur les champs
    void    updatePatientData(Patient *pat, QString nomchamp, QVariant value = QVariant()); //! met à jour la valeur d'un champ de la table et sa propriété correspondante pour le patient
    void    updateCorrespondant(Patient *pat,                                               //! met à jour un des correspondants d'un patient
                                DataBase::typecorrespondant type,
                                Correspondant *cor = Q_NULLPTR);


    //!> actions sur les enregistrements
    void        SupprimePatient(Patient *pat);
    Patient*    CreationPatient(QString nom, QString prenom, QDate datedenaissance, QString sexe = "");

    //!< action sur toutes les données
    void    updatePatient(Patient* pat);                                                    //!> met à jour les datas d'un patient à partir des données enregistrées dans la base


};

#endif // CLS_PATIENTS_H
