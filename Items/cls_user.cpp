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

#include "cls_user.h"
#include <QMetaEnum>

int User::ROLE_NON_RENSEIGNE = -1;
int User::ROLE_VIDE = -2;
int User::ROLE_INDETERMINE = -3;

int User::COMPTA_AVEC_COTATION_AVEC_COMPTABILITE = 0; //0
int User::COMPTA_SANS_COTATION_SANS_COMPTABILITE = 1; //1
int User::COMPTA_AVEC_COTATION_SANS_COMPTABILITE = 2; //2
int User::COMPTA_SANS_COTATION_AVEC_COMPTABILITE = 3; //3

bool User::isAllLoaded() const
{
    return m_isAllLoaded;
}

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

    setDataBool(data, "isAllLoaded", m_isAllLoaded);

    setDataInt(data, "id", m_id);

    setDataString(data, "prenom", m_prenom);
    setDataString(data, "nom", m_nom);
    setDataString(data, "titre", m_titre);
    setDataString(data, "login", m_login);
    setDataString(data, "droits", m_droits);
    setDataString(data, "password", m_password);
    setDataString(data, "fonction", m_fonction);
    setDataString(data, "mail", m_mail);
    setDataString(data, "specialite", m_specialite);
    setDataString(data, "numCO", m_numCO);
    setDataString(data, "portable", m_portable);
    setDataString(data, "web", m_web);
    setDataString(data, "memo", m_memo);
    setDataString(data, "policeEcran", m_policeEcran);
    setDataString(data, "policeAttribut", m_policeAttribut);
    setDataString(data, "nomCompteEncaissHonoraires", m_nomCompteEncaissHonoraires);

    setDataInt(data, "soignant", m_soignant);
    setDataInt(data, "responsableActes", m_responsableActes);
    setDataInt(data, "userccam", m_userccam);
    setDataLongLongInt(data, "numPS", m_numPS);
    setDataInt(data, "noSpecialite", m_noSpecialite);
    setDataInt(data, "idCompteParDefaut", m_idCompteParDefaut);
    setDataInt(data, "poste", m_poste);
    setDataInt(data, "employeur", m_employeur);
    setDataInt(data, "medecin", m_medecin);
    setDataInt(data, "idCompteEncaissHonoraires", m_idCompteEncaissHonoraires);
    setDataInt(data, "enregHonoraires", m_enregHonoraires);
    setDataInt(data, "secteur", m_secteur);

    setDataBool(data, "AGA", m_AGA);
    setDataBool(data, "desactive", m_desactive);
    setDataBool(data, "OPTAM", m_OPTAM);
    setDataBool(data, "ccam", m_ccam);

    setDataDateTime(data, "dateDerniereConnexion", m_dateDerniereConnexion);
    m_data = data;
    /*qDebug() << login();
    qDebug() << "m_responsableActes" << m_responsableActes;
    qDebug() << "responsableactes() = " + Utils::EnumDescription(QMetaEnum::fromType<RESPONSABLE>(), responsableactes());
    qDebug() << "isResponsable()" << isResponsable();
    qDebug() << "isResponsableOuAssistant()" << isResponsableOuAssistant();
    qDebug() << "metier() = " + Utils::EnumDescription(QMetaEnum::fromType<METIER>(), metier());
    qDebug() << "isSoignant()" << isSoignant();
    qDebug() << "isRemplacant()" << isRemplacant();
    qDebug() << "modeenregistrementhonoraires() = " + Utils::EnumDescription(QMetaEnum::fromType<ENREGISTREMENTHONORAIRES>(), modeenregistrementhonoraires());*/
}



QString User::login() const                      { return m_login; }
QString User::password() const                   { return m_password; }
void User::setPassword(QString psswd)            { m_password = psswd; }

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
    }
    return NoMetier;
}
User::RESPONSABLE User::responsableactes() const           /*! 1 = effectue exclusivement des actes sous sa responsabilite
                                                               *  2 = effectue des actes sous sa responsabilite et sous celle des autres users
                                                               *  3 = n'effectue aucun acte sous sa responsabilite */
{
    switch (m_responsableActes) {
    case 0: return PasResponsable;
    case 1: return Responsable;
    case 2: return AlterneResponsablePasResponsable;
    case 3: return PasResponsable;
    }
    return PasResponsable;
}
int User::getUserccam() const                       { return m_userccam; }
User::ENREGISTREMENTHONORAIRES User::modeenregistrementhonoraires() const
{
    switch (m_enregHonoraires) {
    case 0: return NoCompta;
    case 1: return Liberal;
    case 2: return Salarie;
    case 3: return Retrocession;
    case 4: return NoCompta;
    }
    return NoCompta;
}
QString User::titre() const                         { return m_titre; }
int User::numspecialite() const                     { return m_noSpecialite; }
QString User::specialite() const                    { return m_specialite; }
qlonglong User::getNumPS() const                    { return m_numPS; }
QString User::numOrdre() const                      { return m_numCO; }
bool User::isAGA() const                            { return m_AGA; }
int User::idemployeur() const                       { return m_employeur; }
int User::idCompteEncaissHonoraires() const         { return m_idCompteEncaissHonoraires; }
QString User::fonction() const                      { return m_fonction; }

int User::idSuperviseurActes() const                { return m_idUserActeSuperviseur; }
void User::setIdUserSuperviseur(int idusr)          { m_idUserActeSuperviseur = idusr; }
bool User::ishisownsupervisor()                     { return (m_idUserActeSuperviseur == m_id); }

int User::idparent() const                          { return m_idUserParent; }
void User::setIdUserParent(int idusr)               { m_idUserParent = idusr; }
int User::idcomptable() const                       { return m_idUserComptable; }
void User::setIdUserComptable(int idusr)            { m_idUserComptable = idusr; }

User *User::superviseur() const                     { return m_userSuperviseur; }
void User::setUserSuperviseur(User *usr)            { m_userSuperviseur = usr; }
User *User::userparent() const                      { return m_userParent; }
void User::setUserParent(User *usr)                 { m_userParent = usr; }
User *User::comptable() const                       { return m_userComptable; }
void User::setUserComptable(User *usr)              { m_userComptable = usr; }

int User::secteurconventionnel() const              { return m_secteur; }
int User::idcompteParDefaut() const                 { return m_idCompteParDefaut; }
QString User::mail() const                          { return m_mail; }
QString User::portable() const                      { return m_portable; }

Site* User::sitedetravail() const                   { return m_Site; }
void User::setSite(Site *Site)                      { m_Site = Site; }

QList<Compte*>* User::comptesbancaires(bool avecdesactive) const
{
    return (avecdesactive? m_comptesall : m_comptes);
}

void User::setComptes(QList<Compte *> comptes)
{
    if (m_comptes != Q_NULLPTR)
        m_comptes->clear();
    else
        m_comptes = new QList<Compte*>();       //! si on le laisse à Q_NULLPTR, le append() qui suit plantera le prg
    if (m_comptesall != Q_NULLPTR)
        m_comptesall->clear();
    else
        m_comptesall = new QList<Compte*>();       //! si on le laisse à Q_NULLPTR, le append() qui suit plantera le prg
    foreach (Compte* cpt, comptes)
    {
        m_comptesall->append(cpt);
        if (!cpt->isDesactive())
            m_comptes->append(cpt);
    }
}

int User::typecompta() const                        { return m_typeCompta; }
void User::setTypeCompta(int typeCompta )           { m_typeCompta = typeCompta; }

bool User::isOPTAM()                                { return m_OPTAM; }
bool User::useCCAM()                                { return m_ccam; }

bool User::isSecretaire()                           { return m_droits == SECRETAIRE; }
bool User::isAutreFonction()                        { return m_droits == AUTREFONCTION; }
bool User::isMedecin()                              { return m_medecin == 1; }
bool User::isOpthalmo()                             { return metier() == Ophtalmo; }
bool User::isOrthoptist()                           { return metier() == Orthoptiste; }
bool User::isAutreSoignant()                        { return metier() == AutreSoignant; }
bool User::isNonSoignant()                          { return metier() == NonSoignant; }
bool User::isSocComptable()                         { return metier() == SocieteComptable; }
bool User::isComptable()                            { return isLiberal() || isSocComptable(); }
bool User::isSoignant()                             { return isOpthalmo() || isOrthoptist() || isAutreSoignant(); }
bool User::isLiberal()                              { return isSoignant() && modeenregistrementhonoraires() == Liberal; }
bool User::isSalarie()                              { return isSoignant() && modeenregistrementhonoraires() == Salarie; }
bool User::isRemplacant()                           { return isSoignant() && modeenregistrementhonoraires() == Retrocession; }
bool User::isSansCompta()                           { return modeenregistrementhonoraires() == NoCompta; }
bool User::isResponsable()                          { return isSoignant() && responsableactes() == Responsable; }
bool User::isResponsableOuAssistant()               { return isSoignant() && responsableactes() == AlterneResponsablePasResponsable; }
bool User::isAssistant()                            { return isSoignant() && responsableactes() == PasResponsable; }
bool User::isDesactive()                            { return m_desactive; }


/*!
 * \brief User::getStatus
 * génére un résumé des informations de l'utilisateur sur la session courante.
 * \return Chaine de caractères
 */
QString User::Status() const
{
    QString str = "" +
            tr("utilisateur") + "\t\t= " + m_login  + "\n"
          + tr("Site") + "\t\t= " + m_Site->nom()  + "\n";

    //qDebug() << "superviseur " << m_idUserActeSuperviseur;
    //qDebug() << "parent " << m_idUserParent;
    //qDebug() << "comptable " << m_idUserComptable;
    QString strSup = "";
    if( m_idUserActeSuperviseur == User::ROLE_NON_RENSEIGNE )           // le user est soignant, assistant et travaille pour plusieurs superviseurs
        strSup = tr("tout le monde");
    else if( m_idUserActeSuperviseur == User::ROLE_VIDE )               // le user est un administratif
        strSup = tr("sans objet");
    else if( m_idUserActeSuperviseur == User::ROLE_INDETERMINE )        // jamais utilisé
        strSup = tr("indéterminé");
    else if( m_userSuperviseur )
        strSup = m_userSuperviseur->login();
    str += tr("superviseur") + "\t\t= " + strSup + "\n";

    QString strParent = "";
    if( m_idUserParent == User::ROLE_NON_RENSEIGNE )                    // le user est soignant, assistant, travaille pour plusieurs superviseurs
        strParent = tr("sans objet");
    else if( m_idUserParent == User::ROLE_VIDE )                        // le user est un administratif
        strParent = tr("sans objet");
    else if( m_idUserParent == User::ROLE_INDETERMINE )                 // jamais utilisé
        strParent = tr("indéterminé");
    else if( m_userParent )
        strParent = m_userParent->login();
    str += tr("parent") + "\t\t= " + strParent + "\n";

    QString strComptable = "";
    if( m_idUserComptable == User::ROLE_NON_RENSEIGNE )
        strComptable = tr("sans objet");
    else if( m_idUserComptable == User::ROLE_VIDE )
        strComptable = tr("sans objet");
    else if( m_idUserComptable == User::ROLE_INDETERMINE )
        strComptable = tr("indéterminé");
    else if( m_userComptable )
        strComptable = m_userComptable->login();
    str += tr("comptable") + "\t\t= " + strComptable + "\n";
    if( m_userComptable )
        str += tr("cpte banque") + "\t= " + m_nomCompteEncaissHonoraires + "\n";

    QString strCompta = "";
    if( m_typeCompta == User::COMPTA_AVEC_COTATION_AVEC_COMPTABILITE )
        strCompta = "avec cotation et comptabilité";
    else if( m_typeCompta == User::COMPTA_SANS_COTATION_SANS_COMPTABILITE )
        strCompta = "sans cotation ni comptabilité";
    else if( m_typeCompta == User::COMPTA_AVEC_COTATION_SANS_COMPTABILITE )
        strCompta = "avec cotation sans comptabilité";
    else if( m_typeCompta == User::COMPTA_SANS_COTATION_AVEC_COMPTABILITE )
        strCompta = "sans cotation avec comptabilité";
    str += tr("comptabilité") + "\t= " + strCompta;

    return str;
}


