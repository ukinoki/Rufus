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
#include "log.h"
#include "database.h"

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
    setDataInt(data, "userenreghonoraires", m_userenreghonoraires);
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
    setDataBool(data, "cotation", m_cotation);

    setDataDateTime(data, "dateDerniereConnexion", m_dateDerniereConnexion);
    m_data = data;
}



QString User::getLogin() const                      { return m_login; }
QString User::getPassword() const                   { return m_password; }
void User::setPassword(QString psswd)               { m_password = psswd; }

QString User::getNom() const                        { return m_nom; }
QString User::getPrenom() const                     { return m_prenom; }
int User::getSoignant() const                       { return m_soignant; }                  //! 1 = ophtalmo 2 = orthoptiste 3 = autre 4 = Non soignant 5 = societe comptable
int User::getResponsableactes() const               { return m_responsableActes; }
int User::getUserenreghonoraires() const            { return m_userenreghonoraires; }       //! 1 = liberal 2 = salarie 3 = retrocession 4 = pas de comptabilite
int User::getUserccam() const                       { return m_userccam; }
int User::getEnregHonoraires() const                { return m_enregHonoraires; }
QString User::getTitre() const                      { return m_titre; }
int User::getNoSpecialite() const                   { return m_noSpecialite; }
QString User::getSpecialite() const                 { return m_specialite; }
qlonglong User::getNumPS() const                    { return m_numPS; }
QString User::getNumCO() const                      { return m_numCO; }
bool User::isAGA() const                            { return m_AGA; }
int User::getEmployeur() const                      { return m_employeur; }
int User::getIdCompteEncaissHonoraires() const      { return m_idCompteEncaissHonoraires; }
QString User::getFonction() const                   { return m_fonction; }

int User::getIdUserActeSuperviseur() const          { return m_idUserActeSuperviseur; }
void User::setIdUserActeSuperviseur(int idusr)      { m_idUserActeSuperviseur = idusr; }
bool User::ishisownsupervisor()                     { return (m_idUserActeSuperviseur == m_id); }

int User::getIdUserParent() const                   { return m_idUserParent; }
void User::setIdUserParent(int idusr)               { m_idUserParent = idusr; }
int User::getIdUserComptable() const                { return m_idUserComptable; }
void User::setIdUserComptable(int idusr)            { m_idUserComptable = idusr; }

User *User::getUserSuperviseur() const              { return m_userSuperviseur; }
void User::setUserSuperviseur(User *usr)            { m_userSuperviseur = usr; }
User *User::getUserParent() const                   { return m_userParent; }
void User::setUserParent(User *usr)                 { m_userParent = usr; }
User *User::getUserComptable() const                { return m_userComptable; }
void User::setUserComptable(User *usr)              { m_userComptable = usr; }

int User::getSecteur() const                        { return m_secteur; }
int User::getIdCompteParDefaut() const              { return m_idCompteParDefaut; }
QString User::getMail() const                       { return m_mail; }
QString User::getPortable() const                   { return m_portable; }

Site* User::getSite() const                         { return m_Site; }
void User::setSite(Site *Site)                      { m_Site = Site; }

QList<Compte*>* User::getComptes(bool avecdesactive) const
{
    return (avecdesactive? m_comptesall : m_comptes);
}

void User::setComptes(QList<Compte *> *comptes)
{
    if (comptes == Q_NULLPTR)
        return;
    if (m_comptes != Q_NULLPTR)
        m_comptes->clear();
    else
        m_comptes = new QList<Compte*>();       //! si on le laisse à Q_NULLPTR, le append() qui suit plantera le prg
    if (m_comptesall != Q_NULLPTR)
        m_comptesall->clear();
    else
        m_comptesall = new QList<Compte*>();       //! si on le laisse à Q_NULLPTR, le append() qui suit plantera le prg
    for( QList<Compte*>::const_iterator itcpt = comptes->constBegin(); itcpt != comptes->constEnd(); ++itcpt )
    {
        Compte *cpt = const_cast<Compte*>(*itcpt);
        m_comptesall->append(cpt);
        if (!cpt->isDesactive())
            m_comptes->append(cpt);
    }
}

int User::getTypeCompta() const                     { return m_typeCompta; }
void User::setTypeCompta(int typeCompta )           { m_typeCompta = typeCompta; }

bool User::isOPTAM()                                { return m_OPTAM; }
bool User::isCotation()                             { return m_cotation; }

bool User::isSecretaire()                           { return m_droits == SECRETAIRE; }
bool User::isAutreFonction()                        { return m_droits == AUTREFONCTION; }
bool User::isMedecin()                              { return m_medecin == 1; }
bool User::isOpthalmo()                             { return m_soignant == 1; }
bool User::isOrthoptist()                           { return m_soignant == 2; }
bool User::isAutreSoignant()                        { return m_soignant == 3; }
bool User::isNonSoignant()                          { return m_soignant == 4; }
bool User::isSocComptable()                         { return m_soignant == 5; }
bool User::isComptable()                            { return isLiberal() || isSocComptable(); }
bool User::isSoignant()                             { return isOpthalmo() || isOrthoptist() || isAutreSoignant(); }
bool User::isLiberal()                              { return isSoignant() && m_enregHonoraires == 1; }
bool User::isSalarie()                              { return isSoignant() && m_enregHonoraires == 2; }
bool User::isRemplacant()                           { return isSoignant() && m_enregHonoraires == 3; }
bool User::isSansCompta()                           { return m_enregHonoraires == 4; }
bool User::isResponsable()                          { return isSoignant() && m_responsableActes == 1; }
bool User::isResponsableEtAssistant()               { return isSoignant() && m_responsableActes == 2; }
bool User::isAssistant()                            { return isSoignant() && m_responsableActes == 3; }
//bool User::isAssistant() { return isSoignant() && m_idUserActeSuperviseur != m_id; }
bool User::isDesactive()                            { return m_desactive; }


/*!
 * \brief User::getStatus
 * génére un résumé des informations de l'utilisateur sur la session courante.
 * \return Chaine de caractères
 */
QString User::getStatus() const
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
        strSup = m_userSuperviseur->getLogin();
    str += tr("superviseur") + "\t\t= " + strSup + "\n";

    QString strParent = "";
    if( m_idUserParent == User::ROLE_NON_RENSEIGNE )                    // le user est soignant, assistant, travaille pour plusieurs superviseurs
        strParent = tr("sans objet");
    else if( m_idUserParent == User::ROLE_VIDE )                        // le user est un administratif
        strParent = tr("sans objet");
    else if( m_idUserParent == User::ROLE_INDETERMINE )                 // jamais utilisé
        strParent = tr("indéterminé");
    else if( m_userParent )
        strParent = m_userParent->getLogin();
    str += tr("parent") + "\t\t= " + strParent + "\n";

    QString strComptable = "";
    if( m_idUserComptable == User::ROLE_NON_RENSEIGNE )
        strComptable = tr("sans objet");
    else if( m_idUserComptable == User::ROLE_VIDE )
        strComptable = tr("sans objet");
    else if( m_idUserComptable == User::ROLE_INDETERMINE )
        strComptable = tr("indéterminé");
    else if( m_userComptable )
        strComptable = m_userComptable->getLogin();
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


