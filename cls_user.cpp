#include "cls_user.h"

int User::ROLE_NON_RENSEIGNE = -1;
int User::ROLE_VIDE = -2;
int User::ROLE_INDETERMINE = -3;


User User::create()
{

}

/*!
 *  \brief Calcul de l'âge
 *
 *  Methode qui permet ????
 *
 *  \param datedenaissance : la date de naissance
 *  \param Sexe : le sexe de la personne [""]
 *  \param datedujour : la date du jour [Date du jour]
 *
 *  \return un object contenant :
 * toString : une chaine de caractères ( ex: 2 ans 3 mois )
 * annee : l'age brut de la personne
 * mois :
 * icone : l'icone à utiliser [man women, girl, boy, kid, baby]
 * formule : une valeur parmi [l'enfant, la jeune, le jeune, madame, monsieur]
 *
 */
QMap<QString,QVariant> User::CalculAge(QDate datedenaissance)
{
    return User::CalculAge(datedenaissance, "", QDate::currentDate());
}
QMap<QString,QVariant> User::CalculAge(QDate datedenaissance, QDate datedujour)
{
    return User::CalculAge(datedenaissance, "", datedujour);
}
QMap<QString,QVariant> User::CalculAge(QDate datedenaissance, QString Sexe, QDate datedujour)
{
    QMap<QString,QVariant>  Age;
    int         AnneeNaiss, MoisNaiss, JourNaiss;
    int         AnneeCurrent, MoisCurrent, JourCurrent;
    int         AgeAnnee, AgeMois;
    int         FormuleMoisJourNaissance, FormuleMoisJourAujourdhui;

    AnneeNaiss                  = datedenaissance.toString("yyyy").toInt();
    MoisNaiss                   = datedenaissance.toString("MM").toInt();
    JourNaiss                   = datedenaissance.toString("dd").toInt();
    AnneeCurrent                = datedujour.toString("yyyy").toInt();
    MoisCurrent                 = datedujour.toString("MM").toInt();
    JourCurrent                 = datedujour.toString("dd").toInt();
    FormuleMoisJourNaissance    = (MoisNaiss*100) + JourNaiss;
    FormuleMoisJourAujourdhui   = (MoisCurrent*100) + JourCurrent;
    AgeAnnee                    = AnneeCurrent - AnneeNaiss;
    AgeMois                     = MoisCurrent - MoisNaiss;
    if (FormuleMoisJourAujourdhui < FormuleMoisJourNaissance)   AgeAnnee --;
    if (JourNaiss > JourCurrent)                                AgeMois --;
    if (AgeMois < 0)                                            AgeMois = AgeMois + 12;

    Age["annee"] = AgeAnnee;
    Age["mois"]  = AgeMois;

    // On formate l'âge pour l'affichage
    switch (AgeAnnee) {
    case 0:
        if (datedenaissance.daysTo(datedujour) > 31)
            Age["toString"]               = QString::number(AgeMois) + " mois";
        else
            Age["toString"]               = QString::number(datedenaissance.daysTo(datedujour)) + " jours";
        break;
    case 1: case 2: case 3: case 4:
        Age["toString"]                    = QString::number(AgeAnnee) + " an";
        if (AgeAnnee > 1) Age["toString"]  = Age["toString"].toString() + "s";
        if (AgeMois > 0)  Age["toString"]  = Age["toString"].toString() + " " + QString::number(AgeMois) + " mois";
        break;
    default:
        Age["toString"]                    = QString::number(AgeAnnee) + " ans";
        break;
    }

    // On cherche l'icone correspondant au mieux à la personne
    QString img = "silhouette";
    if (AgeAnnee < 2)                       img = "baby";
    else if (AgeAnnee < 8)                  img = "kid";
    else if (AgeAnnee < 16 && Sexe == "M")  img = "boy";
    else if (AgeAnnee < 16 && Sexe == "F")  img = "girl";
    else if (Sexe =="M")                    img = "man";
    else if (Sexe =="F")                    img = "women";
    Age["icone"] = img;

    // On cherche la formule de polistesse correspondant au mieux à la personne
    QString formule = "";
    if (AgeAnnee < 11)                  formule = "l'enfant";
    else if (AgeAnnee < 18) {
        if (Sexe == "F")                formule = "la jeune";
        if (Sexe == "M")                formule = "le jeune";
    }
    else {
        if (Sexe == "F")                formule = "madame";
        if (Sexe == "M")                formule = "monsieur";
    }
    Age["formule"] = formule;

    return Age;
}





User::User(QObject *parent) : QObject(parent)
{}
User::User(QString login, QString password, QJsonObject data, QObject *parent) :QObject(parent),
    m_login(login), m_password(password)
{
    setData(data);

}

void User::setData(QJsonObject data)
{
    if( data.isEmpty() )
        return;

    setDataInt(data, "id");

    setDataString(data, "prenom");
    setDataString(data, "nom");
    setDataString(data, "titre");
    setDataString(data, "login");
    setDataString(data, "droits");
    setDataString(data, "fonction");
    setDataString(data, "mail");
    setDataString(data, "specialite");
    setDataString(data, "numCO");
    setDataString(data, "portable");
    setDataString(data, "web");
    setDataString(data, "memo");
    setDataString(data, "policeEcran");
    setDataString(data, "policeAttribut");
    setDataString(data, "nomUserEncaissHonoraires");
    setDataString(data, "nomCompteEncaissHonoraires");
    setDataString(data, "nomCompteParDefaut");
    setDataString(data, "status");
    setDataString(data, "loginComptable");


    setDataInt(data, "soignant");
    setDataInt(data, "m_responsableActes");
    setDataInt(data, "userenreghonoraires");
    setDataInt(data, "userccam");
    setDataInt(data, "numPS");
    setDataInt(data, "noSpecialite");
    setDataInt(data, "idCompteParDefaut");
    setDataInt(data, "poste");
    setDataInt(data, "employeur");
    setDataInt(data, "medecin");
    setDataInt(data, "idCompteEncaissHonoraires");
    setDataInt(data, "enregHonoraires");
    setDataInt(data, "secteur");

    setDataBool(data, "AGA");
    setDataBool(data, "desactive");
    setDataBool(data, "OPTAM");
    setDataBool(data, "cotation");

    setDataDateTime(data, "dateDerniereConnexion");

    //Adresse de travailgetIdUserComptable
    setDataInt(data, "idLieu");
    setDataString(data, "nomLieu");
    setDataString(data, "adresse1");
    setDataString(data, "adresse2");
    setDataString(data, "adresse3");
    setDataInt(data, "codePostal");
    setDataString(data, "ville");
    setDataString(data, "telephone");
    setDataString(data, "fax");

    //TODO : User : finir init des proprietées
}
void User::setDataString(QJsonObject data, QString key)
{
    const char* thisKey = ("m_"+key).toStdString().c_str();
    if( data.contains(key) )
        this->property(thisKey).setValue( data[key].toString() );
}
void User::setDataInt(QJsonObject data, QString key)
{
    const char* thisKey = ("m_"+key).toStdString().c_str();
    if( data.contains(key) )
        this->property(thisKey).setValue( data[key].toInt() );
}
void User::setDataBool(QJsonObject data, QString key)
{
    const char* thisKey = ("m_"+key).toStdString().c_str();
    if( data.contains(key) )
        this->property(thisKey).setValue( data[key].toBool() );
}
void User::setDataDateTime(QJsonObject data, QString key)
{
    const char* thisKey = ("m_"+key).toStdString().c_str();
    if( data.contains(key) )
    {
        int64_t time = data[key].toInt();
        QDateTime dt;
        dt.setMSecsSinceEpoch( time );
        this->property(thisKey).setValue( dt );
    }
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
int User::getIdLieu() const { return m_idLieu; }
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


QString User::getNomlieu() const { return m_nomLieu; }
QString User::getAdresse1() const { return m_adresse1; }
QString User::getAdresse2() const { return m_adresse2; }
QString User::getAdresse3() const { return m_adresse3; }
int User::getCodePostal() const { return m_codePostal; }
QString User::getVille() const { return m_ville; }
QString User::getTelephone() const { return m_telephone; }


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
