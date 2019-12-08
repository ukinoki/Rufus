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

#include "cls_patient.h"

Patient::Patient(QJsonObject data, QObject *parent) : Item(parent)
{
    resetdatas();
    setData(data);
}

//GETTER | SETTER
QString Patient::nom() const                { return m_nom; }
QString Patient::prenom() const             { return m_prenom; }
QString Patient::sexe() const               { return m_sexe; }
QDate Patient::datedenaissance() const      { return m_dateDeNaissance; }
QDate Patient::datecreationdossier() const  { return m_datecreation; }
int Patient::idcreateur() const             { return m_idcreateur; }

// Social data
QString Patient::adresse1() const           { return m_adresse1; }
QString Patient::adresse2() const           { return m_adresse2; }
QString Patient::adresse3() const           { return m_adresse3; }
QString Patient::codepostal() const         { return m_codepostal; }
QString Patient::ville() const              { return m_ville; }
QString Patient::telephone() const          { return m_telephone; }
QString Patient::portable() const           { return m_portable; }
QString Patient::mail() const               { return m_mail; }
qlonglong Patient::NNI() const              { return m_NNI; }
bool Patient::isald() const                 { return m_ALD; }
bool Patient::iscmu() const                 { return m_CMU; }
QString Patient::profession() const         { return m_profession; }

// Medical data
int Patient::idmg() const                   { return m_idmg; }
int Patient::idspe1() const                 { return m_idspe1; }
int Patient::idspe2() const                 { return m_idspe2; }
int Patient::idspe3() const                 { return m_idspe3; }
int Patient::idcornonmg() const             { return m_idcornonmg; }
QString Patient::atcdtspersos() const       { return m_atcdtspersos; }
QString Patient::atcdtsfamiliaux() const    { return m_atcdtsfamiliaux; }
QString Patient::atcdtsophtalmos() const    { return m_atcdtsophtalmos; }
QString Patient::traitementgen() const      { return m_traitementgen; }
QString Patient::traitementoph() const      { return m_traitementoph; }
QString Patient::tabac() const              { return m_tabac; }
QString Patient::toxiques() const           { return m_toxiques; }
QString Patient::important() const          { return m_important; }
QString Patient::resume() const             { return m_resume; }

bool Patient::ismedicalloaded() const       { return m_ismedicalloaded; }
bool Patient::issocialloaded() const        { return m_issocialloaded; }
bool Patient::isalloaded()
{
    return (m_issocialloaded && m_ismedicalloaded);
}

void Patient::setSexe(QString sex)
{
    m_sexe = sex;
}

void Patient::setData(QJsonObject data)
{
    if( data.isEmpty() )
        return;

    Utils::setDataInt(data, CP_IDPAT_PATIENTS, m_id);
    Utils::setDataString(data, CP_NOM_PATIENTS, m_nom);
    Utils::setDataString(data, CP_PRENOM_PATIENTS, m_prenom);
    Utils::setDataString(data, CP_SEXE_PATIENTS, m_sexe);
    Utils::setDataDate(data, CP_DATECREATION_PATIENTS, m_datecreation);
    Utils::setDataInt(data, CP_IDCREATEUR_PATIENTS, m_idcreateur);
    Utils::setDataDate(data, CP_DDN_PATIENTS, m_dateDeNaissance);
    Utils::setDataBool(data, CP_ISMEDICALlOADED, m_ismedicalloaded);
    Utils::setDataBool(data, CP_ISSOCIALlOADED, m_issocialloaded);
    if (m_issocialloaded)
    {
        Utils::setDataString(data, CP_ADRESSE1_DSP, m_adresse1);
        Utils::setDataString(data, CP_ADRESSE2_DSP, m_adresse2);
        Utils::setDataString(data, CP_ADRESSE3_DSP, m_adresse3);
        Utils::setDataString(data, CP_CODEPOSTAL_DSP, m_codepostal);
        Utils::setDataString(data, CP_VILLE_DSP, m_ville);
        Utils::setDataString(data, CP_TELEPHONE_DSP, m_telephone);
        Utils::setDataString(data, CP_PORTABLE_DSP, m_portable);
        Utils::setDataString(data, CP_MAIL_DSP, m_mail);
        Utils::setDataLongLongInt(data, CP_NNI_DSP, m_NNI);
        Utils::setDataBool(data, CP_ALD_DSP, m_ALD);
        Utils::setDataBool(data, CP_CMU_DSP, m_CMU);
        Utils::setDataString(data, CP_PROFESSION_DSP, m_profession);
    }
    if (m_ismedicalloaded)
    {
        Utils::setDataInt(data, CP_IDMG_RMP, m_idmg);
        Utils::setDataInt(data, CP_IDSPE1_RMP, m_idspe1);
        Utils::setDataInt(data, CP_IDSPE2_RMP, m_idspe2);
        Utils::setDataInt(data, CP_IDSPE3_RMP, m_idspe3);
        Utils::setDataInt(data, CP_IDCORNONMED_RMP, m_idcornonmg);
        Utils::setDataString(data, CP_ATCDTSPERSOS_RMP, m_atcdtspersos);
        Utils::setDataString(data, CP_TRAITMTGEN_RMP, m_traitementgen);
        Utils::setDataString(data, CP_ATCDTSFAMLXS_RMP, m_atcdtsfamiliaux);
        Utils::setDataString(data, CP_ATCDTSOPH_RMP, m_atcdtsophtalmos);
        Utils::setDataString(data, CP_TABAC_RMP, m_tabac);
        Utils::setDataString(data, CP_AUTRESTOXIQUES_RMP, m_toxiques);
        Utils::setDataString(data, CP_IMPORTANT_RMP, m_important);
        Utils::setDataString(data, CP_RESUME_RMP, m_resume);
        Utils::setDataString(data, CP_TRAITMTOPH_RMP, m_traitementoph);
    }
    m_data = data;
}

void Patient::setSocialData(QJsonObject data)
{
    if( data.isEmpty() )
        return;
    Utils::setDataString(data, CP_ADRESSE1_DSP, m_adresse1);
    Utils::setDataString(data, CP_ADRESSE2_DSP, m_adresse2);
    Utils::setDataString(data, CP_ADRESSE3_DSP, m_adresse3);
    Utils::setDataString(data, CP_CODEPOSTAL_DSP, m_codepostal);
    Utils::setDataString(data, CP_VILLE_DSP, m_ville);
    Utils::setDataString(data, CP_TELEPHONE_DSP, m_telephone);
    Utils::setDataString(data, CP_PORTABLE_DSP, m_portable);
    Utils::setDataString(data, CP_MAIL_DSP, m_mail);
    Utils::setDataLongLongInt(data, CP_NNI_DSP, m_NNI);
    Utils::setDataBool(data, CP_ALD_DSP, m_ALD);
    Utils::setDataBool(data, CP_CMU_DSP, m_CMU);
    Utils::setDataString(data, CP_PROFESSION_DSP, m_profession);
    Utils::setDataBool(data, CP_ISSOCIALlOADED, m_issocialloaded);
}

void Patient::setMedicalData(QJsonObject data)
{
    if( data.isEmpty() )
        return;
    Utils::setDataInt(data, CP_IDMG_RMP, m_idmg);
    Utils::setDataInt(data, CP_IDSPE1_RMP, m_idspe1);
    Utils::setDataInt(data, CP_IDSPE2_RMP, m_idspe2);
    Utils::setDataInt(data, CP_IDSPE3_RMP, m_idspe3);
    Utils::setDataInt(data, CP_IDCORNONMED_RMP, m_idcornonmg);
    Utils::setDataString(data, CP_ATCDTSPERSOS_RMP, m_atcdtspersos);
    Utils::setDataString(data, CP_TRAITMTGEN_RMP, m_traitementgen);
    Utils::setDataString(data, CP_ATCDTSFAMLXS_RMP, m_atcdtsfamiliaux);
    Utils::setDataString(data, CP_ATCDTSOPH_RMP, m_atcdtsophtalmos);
    Utils::setDataString(data, CP_TABAC_RMP, m_tabac);
    Utils::setDataString(data, CP_AUTRESTOXIQUES_RMP, m_toxiques);
    Utils::setDataString(data, CP_IMPORTANT_RMP, m_important);
    Utils::setDataString(data, CP_RESUME_RMP, m_resume);
    Utils::setDataString(data, CP_TRAITMTOPH_RMP, m_traitementoph);
    Utils::setDataBool(data, CP_ISMEDICALlOADED, m_ismedicalloaded);
}

/*! comme son nom l'indique */
void Patient::resetdatas()
{
    QJsonObject data;
    data[CP_IDPAT_PATIENTS]        = 0;
    data[CP_NOM_PATIENTS]          = "";
    data[CP_PRENOM_PATIENTS]       = "";
    data[CP_DDN_PATIENTS]          = QDate(1900,1,1).toString("yyyy-MM-dd");
    data[CP_SEXE_PATIENTS]         = "";
    data[CP_DATECREATION_PATIENTS] = QDate(1900,1,1).toString("yyyy-MM-dd");
    data[CP_IDCREATEUR_PATIENTS]   = 0;
    data[CP_IDMG_RMP]              = 0;
    data[CP_IDSPE1_RMP]            = 0;
    data[CP_IDSPE2_RMP]            = 0;
    data[CP_IDSPE3_RMP]            = 0;
    data[CP_IDCORNONMED_RMP]       = 0;
    data[CP_ATCDTSPERSOS_RMP]      = "";
    data[CP_TRAITMTGEN_RMP]        = "";
    data[CP_ATCDTSFAMLXS_RMP]      = "";
    data[CP_ATCDTSOPH_RMP]         = "";
    data[CP_TABAC_RMP]             = "";
    data[CP_AUTRESTOXIQUES_RMP]    = "";
    data[CP_GENCORRESPONDANT_RMP]  = "";
    data[CP_IMPORTANT_RMP]         = "";
    data[CP_RESUME_RMP]            = "";
    data[CP_TRAITMTOPH_RMP]        = "";
    data[CP_ISMEDICALlOADED]       = true;
    data[CP_ADRESSE1_DSP]          = "";
    data[CP_ADRESSE2_DSP]          = "";
    data[CP_ADRESSE3_DSP]       = "";
    data[CP_CODEPOSTAL_DSP]     = "";
    data[CP_VILLE_DSP]          = "";
    data[CP_TELEPHONE_DSP]      = "";
    data[CP_PORTABLE_DSP]       = "";
    data[CP_MAIL_DSP]           = "";
    data[CP_NNI_DSP]            = "";
    data[CP_ALD_DSP]            = false;
    data[CP_CMU_DSP]            = false;
    data[CP_PROFESSION_DSP]     = "";
    data[CP_ISSOCIALlOADED]     = true;
    setData(data);

    data[CP_ISMEDICALlOADED]    = false;
    data[CP_ISSOCIALlOADED]     = false;
    Utils::setDataBool(data, CP_ISSOCIALlOADED, m_issocialloaded);
    Utils::setDataBool(data, CP_ISMEDICALlOADED, m_ismedicalloaded);

    m_data = data;
}
