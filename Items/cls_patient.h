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
 * \brief classe Patient
 * l'ensemble des informations concernant un patient
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
    QString nomcomplet() const          { return nom() + " "  + prenom(); }
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
    QString adressecomplete() const     {
                                          QString adress ("");
                                          if (adresse1() != "")
                                              adress += adresse1();
                                          if (adresse2() != "" && adress != "")
                                              adress += + "\n" + adresse2();
                                          if (adresse3() != "" && adress != "")
                                              adress += + "\n" + adresse3();
                                          if (ville() != "" && adress != "")
                                              adress += + "\n" + codepostal() + " " + ville();
                                          if (telephone() != "" && adress != "")
                                              adress += + "\n" + telephone();
                                          if (portable() != "" && adress != "")
                                              adress += + "\n" + portable();
                                          return adress;
                                        }
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
    void setid(int id)                              { m_id = id;
                                                      m_data[CP_IDPAT_PATIENTS] = id; }
    void  setnom(QString str = "")                  { m_nom = str;
                                                      m_data[CP_NOM_PATIENTS] = str; }
    void  setprenom(QString str = "")               { m_prenom = str;
                                                      m_data[CP_PRENOM_PATIENTS] = str; }
    void  setdatedenaissance(QDate date = QDate())  { m_dateDeNaissance = date;
                                                      m_data[CP_DDN_PATIENTS] = date.toString("yyyy-MM-dd"); }
    void  setsexe(QString str = "")                 { m_sexe = str;
                                                      m_data[CP_SEXE_PATIENTS] = str; }
    void  setdatecreation(QDate date = QDate())     { m_datecreation = date;
                                                      m_data[CP_DATECREATION_PATIENTS] = date.toString("yyyy-MM-dd"); }
    void  setidcreateur(int id = 0)                 { m_idcreateur = id;
                                                      m_data[CP_IDCREATEUR_PATIENTS] = id; }

    // Social data
    void setadresse1(QString str = "")              { m_adresse1 = str;
                                                      m_data[CP_ADRESSE1_DSP] = str; }
    void setadresse2(QString str = "")              { m_adresse2 = str;
                                                      m_data[CP_ADRESSE2_DSP] = str; }
    void setadresse3(QString str = "")              { m_adresse3 = str;
                                                      m_data[CP_ADRESSE3_DSP] = str; }
    void setcodepostal(QString str = "")            { m_codepostal = str;
                                                      m_data[CP_CODEPOSTAL_DSP] = str; }
    void setville(QString str = "")                 { m_ville = str;
                                                      m_data[CP_VILLE_DSP] = str; }
    void settelephone(QString str = "")             { m_telephone = str;
                                                      m_data[CP_TELEPHONE_DSP] = str; }
    void setportable(QString str = "")              { m_portable = str;
                                                      m_data[CP_PORTABLE_DSP] = str; }
    void setmail(QString str = "")                  { m_mail = str;
                                                      m_data[CP_MAIL_DSP] = str; }
    void setNNI(qlonglong str = 0)                  { m_NNI = str;
                                                      m_data[CP_NNI_DSP] = str; }
    void setald(bool logic = false)                 { m_ALD = logic;
                                                      m_data[CP_ALD_DSP] = logic; }
    void setcmu(bool logic = false)                 { m_CMU = logic;
                                                      m_data[CP_CMU_DSP] = logic; }
    void setprofession(QString str = "")            { m_profession = str;
                                                      m_data[CP_PROFESSION_DSP] = str; }

    // Medical data
    void  setmg(int id = 0)                         { m_idmg = id;
                                                      m_data[CP_IDMG_RMP] = id; }
    void  setspe1(int id = 0)                       { m_idspe1 = id;
                                                      m_data[CP_IDSPE1_RMP] = id; }
    void  setspe2(int id = 0)                       { m_idspe2 = id;
                                                      m_data[CP_IDSPE2_RMP] = id; }
    void  setatcdtsoph(QString atcdts = "")         { m_atcdtsophtalmos = atcdts;
                                                      m_data[CP_ATCDTSOPH_RMP] = atcdts; }
    void  setatcdtsgen(QString atcdts = "")         { m_atcdtspersos = atcdts;
                                                      m_data[CP_ATCDTSPERSOS_RMP] = atcdts; }
    void  setatcdtsfam(QString atcdts = "")         { m_atcdtsfamiliaux = atcdts;
                                                      m_data[CP_ATCDTSFAMLXS_RMP] = atcdts; }
    void  settraitemntsoph(QString tts = "")        { m_traitementoph = tts;
                                                      m_data[CP_TRAITMTOPH_RMP] = tts; }
    void  settraitemntsgen(QString tts = "")        { m_traitementgen = tts;
                                                      m_data[CP_TRAITMTGEN_RMP] = tts; }
    void  setimportant(QString imp = "")            { m_important = imp;
                                                      m_data[CP_IMPORTANT_RMP] = imp; }
    void  setresume(QString rsm = "")               { m_resume = rsm;
                                                      m_data[CP_RESUME_RMP] = rsm; }
    void  settabac(QString tbc = "")                { m_tabac = tbc;
                                                      m_data[CP_TABAC_RMP] = tbc; }
    void  setautrestoxiques(QString tox = "")       { m_toxiques = tox;
                                                      m_data[CP_AUTRESTOXIQUES_RMP] = tox; }
    /*!
     * \brief resetdatas
     */
    void resetdatas();
    bool isnull() const                             { return m_id == 0; }
};

#endif // CLS_PATIENT_H
