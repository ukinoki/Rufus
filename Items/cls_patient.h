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

#ifndef CLS_PATIENT_H
#define CLS_PATIENT_H

#include "cls_item.h"
#include "cls_acte.h"

/*!
 * \brief The Patient class
 * l'ensemble des informations concernant une patient
 */

class Patient : public Item
{

private: //Données du patient
    int m_id;                       //!< Id du patient en base
    QString m_nom;                  //!< Nom du patient
    QString m_prenom;               //!< Prénom du patient
    QString m_sexe;                 //!< Sexe du patient
    QDate m_datecreation;           //!< Date de creation du dossier
    int m_idcreateur;               //!< id du user qui a créé le dossier

    QDate m_dateDeNaissance;        //!< Date de naissance du patient


    QString m_adresse1;             //!< 1ere ligne d'adresse du patient
    QString m_adresse2;             //!< 2eme ligne d'adresse du patient
    QString m_adresse3;             //!< 3eme ligne d'adresse du patient
    QString m_codepostal;           //!< code postal du patient
    QString m_ville;                //!< ville du patient
    QString m_telephone;            //!< telephone du patient
    QString m_portable;             //!< no portable du patient
    QString m_mail;                 //!< maildu patient
    qlonglong m_NNI;                //!< numero national d'identité du patient
    bool m_ALD;                     //!< le patient est en ALD
    bool m_CMU;                     //!< le patient bénéficie de la CMU
    QString m_profession;           //!< profession du patient

    int m_idmg;                     //!< l'id du MG correspondant
    int m_idspe1;                   //!< l'id du 1er spécialiste correspondant
    int m_idspe2;                   //!< l'id du 2eme spécialiste correspondant
    int m_idspe3;                   //!< l'id du 3eme spécialiste correspondant
    int m_idcornonmg;               //!< l'id d'un correspondant non médecin
    QString m_atcdtspersos;         //!< les antécédents généraux
    QString m_atcdtsfamiliaux;      //!< les antécédents familiaux
    QString m_atcdtsophtalmos;      //!< les antécédents ophtalmos
    QString m_traitementgen;        //!< le traitement général
    QString m_traitementoph;        //!< le traitement ophtalmo
    QString m_tabac;                //!< consommation de tabac /j
    QString m_toxiques;             //!< autres toxiques
    QString m_gencorresp;           //!< nom du generaliste correspondant (plus utilisé)
    QString m_important;            //!< points importants du dossier
    QString m_resume;               //!< resumé du dossier
    bool    m_ismedicalloaded;      //!< les renseignements médicaux sont chargés
    bool    m_issocialloaded;       //!< les renseignements sociaux sont chargés

    QMap<int, Acte*> *m_actes;      //!< ensemble des actes du patient

public:
    //GETTER | SETTER
    bool ismedicalloaded() const;
    bool issocialloaded() const;
    bool isalloaded();

    int     id() const;
    QString nom() const;
    QString prenom() const;
    QString sexe() const;
    QDate   datecreationdossier() const;
    QDate   datedenaissance() const;
    int     idcreateur() const;

    // Social data
    QString adresse1() const;
    QString adresse2() const;
    QString adresse3() const;
    QString codepostal() const;
    QString ville() const;
    QString telephone() const;
    QString portable() const;
    QString mail() const;
    qlonglong NNI() const;
    bool isald() const;
    bool iscmu() const;
    QString profession() const;

    // Medical data
    int idmg() const;
    int idspe1() const;
    int idspe2() const;
    int idspe3()const;
    int idcornonmg() const;
    QString atcdtspersos();
    QString atcdtsfamiliaux();
    QString atcdtsophtalmos();
    QString traitementgen();
    QString traitementoph();
    QString tabac();
    QString toxiques();
    QString gencorresp();
    QString important();
    QString resume();

    void setActes(QMap<int, Acte *> *actes);
    QMap<int, Acte *> *actes() const;
    void setSexe(QString sex);

    explicit Patient(QJsonObject data = {}, QObject *parent = Q_NULLPTR);

    void setData(QJsonObject data);
    void setSocialData(QJsonObject data);
    void setMedicalData(QJsonObject data);
    void addActe(Acte *acte);

    // Medical data
    void    setmg(int id) {m_idmg = id;}
    void    setspe1(int id) {m_idspe1 = id;}
    void    setspe2(int id) {m_idspe2 = id;}
};

#endif // CLS_PATIENT_H
