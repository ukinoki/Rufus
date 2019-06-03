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
int Patient::idspe3()const                  { return m_idspe3; }
int Patient::idcornonmg() const             { return m_idcornonmg; }
QString Patient::atcdtspersos()             { return m_atcdtspersos; }
QString Patient::atcdtsfamiliaux()          { return m_atcdtsfamiliaux; }
QString Patient::atcdtsophtalmos()          { return m_atcdtsophtalmos; }
QString Patient::traitementgen()            { return m_traitementgen; }
QString Patient::traitementoph()            { return m_traitementoph; }
QString Patient::tabac()                    { return m_tabac; }
QString Patient::toxiques()                 { return m_toxiques; }
QString Patient::important()                { return m_important; }
QString Patient::resume()                   { return m_resume; }

bool Patient::ismedicalloaded() const       { return m_ismedicalloaded; }
bool Patient::issocialloaded() const        { return m_issocialloaded; }
bool Patient::isalloaded()
{
    return (m_issocialloaded && m_ismedicalloaded);
}

QMap<int, Acte *> *Patient::actes() const
{
    return m_actes;
}
void Patient::setActes(QMap<int, Acte *> *actes)
{
    m_actes = actes;
}

void Patient::setSexe(QString sex)
{
    m_sexe = sex;
}

Patient::Patient(QJsonObject data, QObject *parent) : Item(parent)
{
    setData(data);
}

void Patient::setData(QJsonObject data)
{
    if( data.isEmpty() )
        return;

    setDataInt(data, "id", m_id);

    setDataString(data, "nom", m_nom);
    setDataString(data, "prenom", m_prenom);
    setDataString(data, "sexe", m_sexe);
    setDataDate(data, "datecreation", m_datecreation);
    setDataInt(data, "idcreateur", m_idcreateur);
    setDataDate(data, "dateDeNaissance", m_dateDeNaissance);
    setDataBool(data, "isMedicalLoaded", m_ismedicalloaded);
    setDataBool(data, "isSocialLoaded", m_issocialloaded);
    if (m_issocialloaded)
    {
        setDataString(data, "adresse1", m_adresse1);
        setDataString(data, "adresse2", m_adresse2);
        setDataString(data, "adresse3", m_adresse3);
        setDataString(data, "codepostal", m_codepostal);
        setDataString(data, "ville", m_ville);
        setDataString(data, "telephone", m_telephone);
        setDataString(data, "portable", m_portable);
        setDataString(data, "mail", m_mail);
        setDataLongLongInt(data, "NNI", m_NNI);
        setDataBool(data, "ALD", m_ALD);
        setDataBool(data, "CMU", m_CMU);
        setDataString(data, "profession", m_profession);
    }
    if (m_ismedicalloaded)
    {
        setDataInt(data, "idMG", m_idmg);
        setDataInt(data, "idSpe1", m_idspe1);
        setDataInt(data, "idSpe2", m_idspe2);
        setDataInt(data, "idSpe3", m_idspe3);
        setDataInt(data, "idCornonMG", m_idcornonmg);
        setDataString(data, "AtcdtsPerso", m_atcdtspersos);
        setDataString(data, "TtGeneral", m_traitementgen);
        setDataString(data, "AtcdtsFamiliaux", m_atcdtsfamiliaux);
        setDataString(data, "AtcdstOph", m_atcdtsophtalmos);
        setDataString(data, "Tabac", m_tabac);
        setDataString(data, "Toxiques", m_toxiques);
        setDataString(data, "Important", m_important);
        setDataString(data, "Resume", m_resume);
        setDataString(data, "TtOph", m_traitementoph);
    }
    m_data = data;
}

void Patient::setSocialData(QJsonObject data)
{
    if( data.isEmpty() )
        return;
    setDataString(data, "adresse1", m_adresse1);
    setDataString(data, "adresse2", m_adresse2);
    setDataString(data, "adresse3", m_adresse3);
    setDataString(data, "codepostal", m_codepostal);
    setDataString(data, "ville", m_ville);
    setDataString(data, "telephone", m_telephone);
    setDataString(data, "portable", m_portable);
    setDataString(data, "mail", m_mail);
    setDataLongLongInt(data, "NNI", m_NNI);
    setDataBool(data, "ALD", m_ALD);
    setDataBool(data, "CMU", m_CMU);
    setDataString(data, "profession", m_profession);
    m_issocialloaded = data["isSocialLoaded"].toBool();
}

void Patient::setMedicalData(QJsonObject data)
{
    if( data.isEmpty() )
        return;
    setDataInt(data, "idMG", m_idmg);
    setDataInt(data, "idSpe1", m_idspe1);
    setDataInt(data, "idSpe2", m_idspe2);
    setDataInt(data, "idSpe3", m_idspe3);
    setDataInt(data, "idCornonMG", m_idcornonmg);
    setDataString(data, "AtcdtsPerso", m_atcdtspersos);
    setDataString(data, "TtGeneral", m_traitementgen);
    setDataString(data, "AtcdtsFamiliaux", m_atcdtsfamiliaux);
    setDataString(data, "AtcdstOph", m_atcdtsophtalmos);
    setDataString(data, "Tabac", m_tabac);
    setDataString(data, "Toxiques", m_toxiques);
    setDataString(data, "Important", m_important);
    setDataString(data, "Resume", m_resume);
    setDataString(data, "TtOph", m_traitementoph);
    m_ismedicalloaded = data["isMedicalLoaded"].toBool();
}

/*!
 * \brief Patient::addActe
 * ajout un acte au patient
 * \param acte
*/
void Patient::addActe(Acte *acte)
{
    if( m_actes->contains(acte->id()) )
        return;
    m_actes->insert(acte->id(), acte);
}
