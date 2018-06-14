#include "cls_user.h"
#include "log.h"

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

    setDataInt(data, "id", m_id);

    setDataString(data, "prenom", m_prenom);
    setDataString(data, "nom", m_nom);
    setDataString(data, "titre", m_titre);
    setDataString(data, "login", m_login);
    setDataString(data, "droits", m_droits);
    setDataString(data, "fonction", m_fonction);
    setDataString(data, "mail", m_mail);
    setDataString(data, "specialite", m_specialite);
    setDataString(data, "numCO", m_numCO);
    setDataString(data, "portable", m_portable);
    setDataString(data, "web", m_web);
    setDataString(data, "memo", m_memo);
    setDataString(data, "policeEcran", m_policeEcran);
    setDataString(data, "policeAttribut", m_policeAttribut);
    setDataString(data, "nomUserEncaissHonoraires", m_nomUserEncaissHonoraires);
    setDataString(data, "nomCompteEncaissHonoraires", m_nomCompteEncaissHonoraires);
    setDataString(data, "nomCompteParDefaut", m_nomCompteParDefaut);
    setDataString(data, "status", m_status);
    setDataString(data, "loginComptable", m_loginComptable);


    setDataInt(data, "soignant", m_soignant);
    setDataInt(data, "m_responsableActes", m_responsableActes);
    setDataInt(data, "userenreghonoraires", m_userenreghonoraires);
    setDataInt(data, "userccam", m_userccam);
    setDataInt(data, "numPS", m_numPS);
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
}

QString User::getLogin() const { return m_login; }
QString User::getPassword() const { return m_password; }

int User::id() const { return m_id; }
void User::setId( int value ) { m_id = value; }

QString User::getNom() const { return m_nom; }
QString User::getPrenom() const { return m_prenom; }
int User::getSoignant() const { return m_soignant; }
int User::getResponsableactes() const { return m_responsableActes; }
int User::getUserenreghonoraires() const { return m_userenreghonoraires; }
int User::getUserccam() const { return m_userccam; }
int User::getEnregHonoraires() const { return m_enregHonoraires; }
QString User::getTitre() const { return m_titre; }
int User::getNoSpecialite() const { return m_noSpecialite; }
QString User::getSpecialite() const { return m_specialite; }
int User::getNumPS() const { return m_numPS; }
QString User::getNumCO() const { return m_numCO; }
bool User::getAGA() const { return m_AGA; }
int User::getEmployeur() const { return m_employeur; }
int User::getIdCompteEncaissHonoraires() const { return m_idCompteEncaissHonoraires; }
QString User::getNomUserEncaissHonoraires() const { return m_nomUserEncaissHonoraires; }
QString User::getNomCompteEncaissHonoraires() const { return m_nomCompteEncaissHonoraires; }
QString User::getFonction() const { return m_fonction; }
int User::getIdUserActeSuperviseur() const { return m_idUserActeSuperviseur; }
int User::getIdUserParent() const { return m_idUserParent; }
int User::getIdUserComptable() const { return m_idUserCompta; }
int User::getSecteur() const { return m_secteur; }
int User::getIdCompteParDefaut() const { return m_idCompteParDefaut; }
QString User::getMail() const { return m_mail; }
QString User::getLoginComptable() const { return m_loginComptable; }
QString User::getNomCompteParDefaut() const { return m_nomCompteParDefaut; }
QString User::getPortable() const { return m_portable; }


QString User::getStatus() const { return m_status; }


Etablissement* User::getEtablissement() const { return m_etablissement; }
void User::setEtablissement(Etablissement *etablissement)
{
    m_etablissement = etablissement;
}


bool User::isOPTAM() { return m_OPTAM; }
bool User::isCotation() { return m_cotation; }

bool User::isSecretaire() { return m_droits == SECRETAIRE; }
bool User::isAutreFonction() { return m_droits == AUTREFONCTION; }
bool User::isMedecin() { return m_medecin == 1; }
bool User::isOpthalmo() { return m_soignant == 1; }
bool User::isOrthoptist() { return m_soignant == 2; }
bool User::isAutreSoignant() { return m_soignant == 3; }
bool User::isNonSoignant() { return m_soignant == 4; }
bool User::isSocComptable() { return m_soignant == 5; }
bool User::isSoignant() { return isOpthalmo() || isOrthoptist() || isAutreSoignant(); }
bool User::isLiberal() { return isSoignant() && m_enregHonoraires == 1; }
bool User::isSalarie() { return isSoignant() && m_enregHonoraires == 2; }
bool User::isRemplacant() { return isSoignant() && m_enregHonoraires == 3; }
bool User::isSansCompta() { return m_enregHonoraires == 4; }
bool User::isResponsable() { return isSoignant() &&m_idUserActeSuperviseur == 1; }
bool User::isResponsableEtAssistant() { return isSoignant() &&m_idUserActeSuperviseur == 2; }
bool User::isAssistant() { return isSoignant() &&m_idUserActeSuperviseur == 3; }
//bool User::isAssistant() { return isSoignant() && m_idUserActeSuperviseur != m_id; }

bool User::isDesactive() { return m_desactive; }


