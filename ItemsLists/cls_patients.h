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

#ifndef CLS_PATIENTS_H
#define CLS_PATIENTS_H

#include <cls_patient.h>
#include "cls_itemslist.h"

 /*! La classe patients initialise
  * 2 QMap de patients
    * la liste des patients affichés dans la table ui->PatientsListeTableView de rufus.ui                                           \variable m_patientstable
    * la liste de tous les patients en cours, ceux qui sont inscrits dans la table rufus.salledattente                              \variable m_patientssaldat
  * le patient en cours d'examen sur le poste,                                                                                      \variable m_currentpatient
  * le dossier à ouvrir pour les recherches sur des patients qui ne sont pas le patient courant, par le biais des menus contextuels \variable m_dossierpatientaouvrir
  */

class Patients : public ItemsList
{

public:
    explicit Patients(QObject *parent = Q_NULLPTR);

    //GETTER
    QMap<int, Patient*> *patients()           { return map_patients; }
    QMap<int, Patient*> *patientstable()      { return map_patientstable; }
    QMap<int, Patient*> *patientssaldat()     { return map_patientssaldat; }
    Patient* currentpatient()                 { return m_currentpatient; }
    Patient* dossierpatientaouvrir()          { return m_dossierpatientaouvrir; }
    void setcurrentpatient(Patient *pat);
    void setdossierpatientaouvrir(Patient *pat);

    DonneesOphtaPatient* donneesophtacurrentpatient();
    void setDonneesOphtaCurrentPatient ();
    void actualiseDonneesOphtaCurrentPatient ();


    Patient* getById(int id, Item::LOADDETAILS loadDetails = Item::NoLoadDetails);          /*! charge le patient corresondant à l'id
                                                                                             * \brief Patients::getById
                                                                                             * \param id l'id du patient recherché
                                                                                             * \param loadDetails = NoLoadDetails  -> ne charge que les données d'identité - = LoadDetails -> charge les données sociales et médicales
                                                                                             * \return Q_NULLPTR si aucun patient trouvé
                                                                                             * \return Patient* le patient correspondant à l'id
                                                                                             */
    bool isfull();                                                                          /*! la liste contient tous les patients de la base */
    void loadAll(Patient *pat, Item::UPDATE upd = Item::NoUpdate);                          /*! charge toutes les données d'un  patient si ce n'est pas le cas
                                                                                             * \param upd -> force ou non la recharge depuis la BDD si elles sont déjà chargées
                                                                                             */

    void initListeTable(QString nom = "", QString prenom = "", bool filtre = false);        /*! crée la liste de patients de la table
                                                                                            * \param patnom filtrer sur le nom de patient
                                                                                            * \param patprenom filtrer sur le prénom de patient
                                                                                            * \param le filtre se fait sur des valeurs aprrochantes */

    void initListeSalDat(QList<int> listidpatients);                                       /*! crée la liste de patients en cours (patients en cours d'examen, en salle d'attente ou à l'accueil)
                                                                                            * \param list id = la liste des idpatient en cours */

    void initListeByDDN(QDate DDN = QDate());                                              /*! crée une liste des patients de la table pour une date de naissance
                                                                                            * \param DDN la date de naissance */

    static bool veriftelephone(Patient * pat);                                             /*! vérfie si le patient a au moins un no de tel enregistré */

private:
    /*! > il y a 3 listes de patients:
     * une liste fourre tout m_patients
     * la liste des patients de la table de rechercher
     * la liste des patients en cours de la table salle d'attente
     * et les 2 patients actifs sur le poste: patient en cours d'examen et patient à ouvir (menu contextuel de la table)
    */
    QMap<int, Patient*> *map_patients;                                                      //!< une liste fourre tout
    QMap<int, Patient*> *map_patientstable;                                                 //!< la liste des patients de la table listepatients
    QMap<int, Patient*> *map_patientssaldat;                                                //!< la liste des patients en salle d'attente
    Patient *m_currentpatient           = new Patient();                                    //!> le patient dont le dossier est ouvert
    Patient *m_dossierpatientaouvrir    = Q_NULLPTR;                                        //!> le dossier de patient à ouvrir

    bool m_full;                                                                            //! la liste contient tous les patients de la base


public:

    //!> actions combinées sur l'item et l'enregistrement correspondant en base de données

    //!> actions sur les champs
    void    updateCorrespondant(Patient *pat,                                               //! met à jour un des correspondants d'un patient
                                Correspondant::typecorrespondant type,
                                Correspondant *cor = Q_NULLPTR);


    //!> actions sur les enregistrements
    void                SupprimePatient(Patient *pat);
    static Patient*     CreationPatient(QHash<QString, QVariant> data);

    //!< action sur toutes les données
    static void    updatePatient(Patient* pat);                                             //!> met à jour les datas d'un patient à partir des données enregistrées dans la base
};

#endif // CLS_PATIENTS_H
