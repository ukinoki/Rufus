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

/*!
 * \brief The Patient class
 * l'ensemble des informations concernant une patient
 */

class Patient : public Item
{

private: //Données du patient
    //!<m_id = Id du patient en base
    QString m_nom = "";                 //!< Nom du patient
    QString m_prenom = "";              //!< Prénom du patient
    QString m_sexe = "";                //!< Sexe du patient
    QDate m_datecreation = QDate();     //!< Date de creation du dossier
    int m_idcreateur = 0;               //!< id du user qui a créé le dossier

    QDate m_dateDeNaissance = QDate();  //!< Date de naissance du patient

    QString m_adresse1 = "";            //!< 1ere ligne d'adresse du patient
    QString m_adresse2 = "";            //!< 2eme ligne d'adresse du patient
    QString m_adresse3 = "";            //!< 3eme ligne d'adresse du patient
    QString m_codepostal = "";          //!< code postal du patient
    QString m_ville = "";               //!< ville du patient
    QString m_telephone = "";           //!< telephone du patient
    QString m_portable = "";            //!< no portable du patient
    QString m_mail = "";                //!< maildu patient
    qlonglong m_NNI= 0;                 //!< numero national d'identité du patient
    bool m_ALD = false;                 //!< le patient est en ALD
    bool m_CMU = false;                 //!< le patient bénéficie de la CMU
    QString m_profession = "";          //!< profession du patient

    int m_idmg = 0;                     //!< l'id du MG correspondant
    int m_idspe1 = 0;                   //!< l'id du 1er spécialiste correspondant
    int m_idspe2 = 0;                   //!< l'id du 2eme spécialiste correspondant
    int m_idspe3 = 0;                   //!< l'id du 3eme spécialiste correspondant
    int m_idcornonmg;                   //!< l'id d'un correspondant non médecin
    QString m_atcdtspersos = "";        //!< les antécédents généraux
    QString m_atcdtsfamiliaux = "";     //!< les antécédents familiaux
    QString m_atcdtsophtalmos = "";     //!< les antécédents ophtalmos
    QString m_traitementgen = "";       //!< le traitement général
    QString m_traitementoph = "";       //!< le traitement ophtalmo
    QString m_tabac = "";               //!< consommation de tabac /j
    QString m_toxiques = "";            //!< autres toxiques
    QString m_important = "";           //!< points importants du dossier
    QString m_resume = "";              //!< resumé du dossier
    bool m_ismedicalloaded = false;     //!< les renseignements médicaux sont chargés
    bool m_issocialloaded  = false;     //!< les renseignements sociaux sont chargés

public:
    //GETTER | SETTER
    bool ismedicalloaded() const;
    bool issocialloaded() const;
    bool isalloaded();

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
    QString atcdtspersos() const;
    QString atcdtsfamiliaux() const;
    QString atcdtsophtalmos() const;
    QString traitementgen() const;
    QString traitementoph() const;
    QString tabac() const;
    QString toxiques() const;
    QString important() const;
    QString resume() const;

    void setSexe(QString sex);

    explicit Patient(QJsonObject data = {}, QObject *parent = Q_NULLPTR);

    void setData(QJsonObject data = QJsonObject{});
    void setSocialData(QJsonObject data = QJsonObject{});
    void setMedicalData(QJsonObject data = QJsonObject{});

    // basic data
    void  setnom(QString str = "")                  { m_nom = str; }
    void  setprenom(QString str = "")               { m_prenom = str; }
    void  setdatedenaissance(QDate date = QDate())  { m_dateDeNaissance = date; }
    void  setsexe(QString str = "")                 { m_sexe = str; }
    void  setdatecreation(QDate date = QDate())     { m_datecreation = date; }
    void  setidcreateur(int id = 0)                 { m_idcreateur = id;}

    // Social data
    void setadresse1(QString str = "")              { m_adresse1 = str; }
    void setadresse2(QString str = "")              { m_adresse2 = str; }
    void setadresse3(QString str = "")              { m_adresse3 = str; }
    void setcodepostal(QString str = "")            { m_codepostal = str; }
    void setville(QString str = "")                 { m_ville = str; }
    void settelephone(QString str = "")             { m_telephone = str; }
    void setportable(QString str = "")              { m_portable = str; }
    void setmail(QString str = "")                  { m_mail = str; }
    void setNNI(qlonglong str = 0)                  { m_NNI = str; }
    void setald(bool logic = false)                 { m_ALD = logic; }
    void setcmu(bool logic = false)                 { m_CMU = logic; }
    void setprofession(QString str = "")            { m_profession = str; }

    // Medical data
    void  setmg(int id = 0)                         { m_idmg = id; }
    void  setspe1(int id = 0)                       { m_idspe1 = id; }
    void  setspe2(int id = 0)                       { m_idspe2 = id; }
    void  setatcdtsoph(QString atcdts = "")         { m_atcdtsophtalmos = atcdts; }
    void  setatcdtsgen(QString atcdts = "")         { m_atcdtspersos = atcdts; }
    void  setatcdtsfam(QString atcdts = "")         { m_atcdtsfamiliaux = atcdts; }
    void  settraitemntsoph(QString tts = "")        { m_traitementoph = tts; }
    void  settraitemntsgen(QString tts = "")        { m_traitementgen = tts; }
    void  setimportant(QString imp = "")            { m_important = imp; }
    void  setresume(QString rsm = "")               { m_resume = rsm; }
    void  settabac(QString tbc = "")                { m_tabac = tbc; }
    void  setautrestoxiques(QString tox = "")       { m_toxiques = tox; }

    void resetdatas();
    bool isnull() const                             { return m_id == 0; }
};

#endif // CLS_PATIENT_H
