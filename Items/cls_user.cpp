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

#include "cls_user.h"
#include <QMetaEnum>

int User::ROLE_NON_RENSEIGNE = -1;
int User::ROLE_VIDE = -2;
int User::ROLE_INDETERMINE = -3;

User::User(QJsonObject data, QObject *parent) : Item(parent)
{
    setData(data);
}

User::User(QString login, QString password, QJsonObject data, QObject *parent) : Item(parent),
    m_login(login), m_password(password)
{
    setData(data);
}

void User::setData(QJsonObject data)
{
    if( data.isEmpty() )
        return;

    Utils::setDataInt(data, CP_ID_USR, m_id);

    Utils::setDataString(data, CP_PRENOM_USR, m_prenom);
    Utils::setDataString(data, CP_NOM_USR, m_nom);
    Utils::setDataString(data, CP_TITRE_USR, m_titre);
    Utils::setDataString(data, CP_LOGIN_USR, m_login);
    Utils::setDataString(data, CP_DROITS_USR, m_droits);
    Utils::setDataString(data, CP_MDP_USR, m_password);
    Utils::setDataString(data, CP_FONCTION_USR, m_fonction);
    Utils::setDataString(data, CP_MAIL_USR, m_mail);
    Utils::setDataString(data, CP_SPECIALITE_USR, m_specialite);
    Utils::setDataString(data, CP_NUMCO_USR, m_numCO);
    Utils::setDataString(data, CP_PORTABLE_USR, m_portable);
    Utils::setDataString(data, CP_MEMO_USR, m_memo);
    Utils::setDataString(data, CP_POLICEECRAN_USR, m_policeEcran);
    Utils::setDataString(data, CP_POLICEATTRIBUT_USR, m_policeAttribut);

    Utils::setDataInt(data, CP_SOIGNANTSTATUS_USR, m_soignant);
    Utils::setDataInt(data, CP_RESPONSABLEACTES_USR, m_responsableActes);
    Utils::setDataLongLongInt(data, CP_NUMPS_USR, m_numPS);
    Utils::setDataInt(data, CP_IDSPECIALITE_USR, m_noSpecialite);
    Utils::setDataInt(data, CP_IDCOMPTEPARDEFAUT_USR, m_idCompteParDefaut);
    Utils::setDataInt(data, CP_IDEMPLOYEUR_USR, m_employeur);
    Utils::setDataBool(data, CP_ISMEDECIN_USR, m_medecin);
    Utils::setDataInt(data, CP_ENREGHONORAIRES_USR, m_enregHonoraires);
    Utils::setDataInt(data, CP_SECTEUR_USR, m_secteur);

    Utils::setDataBool(data, CP_ISAGA_USR, m_AGA);
    Utils::setDataBool(data, CP_ISDESACTIVE_USR, m_desactive);
    Utils::setDataBool(data, CP_ISOPTAM_USR, m_OPTAM);
    Utils::setDataBool(data, CP_COTATION_USR, m_cotationactes);

    Utils::setDataDateTime(data, CP_DATEDERNIERECONNEXION_USR, m_dateDerniereConnexion);
    Utils::setDataDate(data, CP_DATECREATIONMDP_USR, m_datecreationMDP);
    Utils::setDataBool(data, CP_AFFICHEDOCSPUBLICS_USR, m_affichedocspublics);
    Utils::setDataBool(data, CP_AFFICHECOMMENTSPUBLICS_USR, m_affichecommentslunettespublics);
    m_data = data;
    /*qDebug() << login();

    qDebug() << "m_responsableActes" << m_responsableActes;
    qDebug() << "responsableactes() = " + Utils::EnumDescription(QMetaEnum::fromType<RESPONSABLE>(), responsableactes());
    qDebug() << "isResponsable()" << isResponsable();
    qDebug() << "isResponsableOuAssistant()" << isResponsableOuAssistant();
    qDebug() << "metier() = " + Utils::EnumDescription(QMetaEnum::fromType<METIER>(), metier());
    qDebug() << "isSoignant()" << isSoignant();
    qDebug() << "ismedecin()" << isMedecin();
    qDebug() << "isRemplacant()" << isRemplacant();
    qDebug() << "statutcomptable() = " + Utils::EnumDescription(QMetaEnum::fromType<ENREGISTREMENTHONORAIRES>(), statutcomptable());*/
}

/*!
 * les données figées, ne variant pas d'une session à l'autre =======================================================================================================================
 */

QString User::login() const                      { return m_login; }
QString User::password() const                   { return m_password; }

QString User::nom() const                        { return m_nom; }
QString User::prenom() const                     { return m_prenom; }
User::METIER User::metier() const
{
    switch (m_soignant) {
    case 0: return NoMetier;
    case 1: return Ophtalmo;
    case 2: return Orthoptiste;
    case 3: return AutreSoignant;
    case 4: return NonSoignant;
    case 5: return SocieteComptable;
    case 6: return Neutre;
    }
    return NoMetier;
}
User::RESPONSABLE User::responsableactes() const           /*! Responsable                      = effectue exclusivement des actes sous sa responsabilite
                                                            *  AlterneResponsablePasResponsable = effectue des actes sous sa responsabilite et sous celle des autres users
                                                            *  PasResponsable                   = sans objet (secrétaire, comptable...) ou n'effectue aucun acte sous sa responsabilite */
{
    switch (m_responsableActes) {
    case 0: return PasResponsable;
    case 1: return Responsable;
    case 2: return AlterneResponsablePasResponsable;
    case 3: return PasResponsable;
    }
    return PasResponsable;
}
QString User::titre() const                         { return m_titre; }
int User::numspecialite() const                     { return m_noSpecialite; }
QString User::specialite() const                    { return m_specialite; }
qlonglong User::NumPS() const                       { return m_numPS; }
QString User::numOrdre() const                      { return m_numCO; }
bool User::isAGA() const                            { return m_AGA; }
int User::idemployeur() const                       { return m_employeur; }

QString User::fonction() const                      { return m_fonction; }
int User::secteurconventionnel() const              { return m_secteur; }
QString User::mail() const                          { return m_mail; }
QString User::portable() const                      { return m_portable; }

bool User::isOPTAM()                                { return m_OPTAM; }
bool User::useCotationsActes()                      { return m_cotationactes; }

bool User::isSecretaire()                           { return m_droits == SECRETAIRE; }
bool User::isAutreFonction()                        { return m_droits == AUTREFONCTION; }
bool User::isMedecin()                              { return m_medecin; }
bool User::isOpthalmo()                             { return metier() == Ophtalmo; }
bool User::isOrthoptist()                           { return metier() == Orthoptiste; }
bool User::isAutreSoignant()                        { return metier() == AutreSoignant; }
bool User::isNonSoignant()                          { return metier() == NonSoignant; }
bool User::isSocComptable()                         { return metier() == SocieteComptable; }
bool User::isNeutre()                               { return metier() == Neutre; }
bool User::isSoignant()                             { return isOpthalmo() || isOrthoptist() || isAutreSoignant(); }
bool User::isLiberal()                              { return statutcomptable() == Liberal; }
bool User::isLiberalSEL()                           { return statutcomptable() == LiberalSEL; }
bool User::isSoignantSalarie()                      { return isSoignant() && statutcomptable() == Salarie; }
bool User::isRemplacant()                           { return statutcomptable() == Remplacant; }
bool User::isResponsable()                          { return isSoignant() && responsableactes() == Responsable; }
bool User::isAlterneResponsableEtAssistant()        { return isSoignant() && responsableactes() == AlterneResponsablePasResponsable; }
bool User::isAssistant()                            { return isSoignant() && responsableactes() == PasResponsable; }
bool User::isDesactive()                            { return m_desactive; }


bool User::isComptableActes()                       { return isLiberal() || isSocComptable(); }
User::STATUT_COMPTABLE User::statutcomptable() const
{
    switch (m_enregHonoraires) {
    case 0: return NoCompta;
    case 1: return Liberal;
    case 2: return Salarie;
    case 3: return Remplacant;
    case 4: return NoCompta;
    case 5: return LiberalSEL;
    }
    return NoCompta;
}
int User::idcomptepardefaut() const                 { return m_idCompteParDefaut; }
int User::idcompteencaissementhonoraires() const    { return m_idCompteEncaissHonoraires; }
QList<int> User::listecomptesbancaires(bool avecdesactive) const
{
    return (avecdesactive? m_listidcomptesall : m_listidcomptes);
}
void User::setlistecomptesbancaires(QMap<int, bool> mapidcomptes)
{
    if (!m_listidcomptes.isEmpty())
        m_listidcomptes.clear();
    if (!m_listidcomptesall.isEmpty())
        m_listidcomptesall.clear();
    for (auto it = mapidcomptes.constBegin(); it!= mapidcomptes.constEnd(); ++it)
    {
        m_listidcomptesall << it.key();
        if (!mapidcomptes.value(it.key()))
            m_listidcomptes.append(it.key());
    }
}

