#ifndef CLS_USER_H
#define CLS_USER_H

#include <QMap>
#include <QObject>
#include <QVariant>
#include <QDate>
#include <QJsonObject>
#include "cls_item.h"
#include "cls_site.h"
#include "cls_compte.h"
#include "macros.h"

/*!
 * \brief The User class
 * l'ensemble des informations concernant
 * une personne utilisant le logiciel
 * ex: Praticien, Secretaire, ...
 */

class User : public Item
{
    Q_OBJECT
public: //static
    static int ROLE_NON_RENSEIGNE; //-1
    static int ROLE_VIDE; //-2
    static int ROLE_INDETERMINE; //-3




private:
    bool m_isAllLoaded = false;

    int m_id; //!< Id de l'utilsateur en base

    QString m_login; //!< Identifiant de l'utilisateur
    QString m_password;
    QString m_prenom; //!< prénom de l'utilisateur
    QString m_nom; //!< nom de l'utilisateur
    QString m_droits;
    QString m_mail;
    QString m_titre; //!< titre, ex: Docteur
    QString m_fonction;
    QString m_specialite;
    QString m_numCO;
    QString m_portable;
    QString m_web;
    QString m_memo;
    QString m_policeEcran;
    QString m_policeAttribut;
    QString m_nomUserEncaissHonoraires;
    QString m_nomCompteEncaissHonoraires;
    QString m_nomCompteParDefaut;
    QString m_status;
    QString m_loginComptable;

    int m_soignant;
    int m_responsableActes; //!< 1 : responsable
                            //!< 2 : responsable et assistant
                            //!< 3 : assistant
    int m_userenreghonoraires;
    int m_userccam;
    int m_numPS;
    int m_noSpecialite;
    int m_idCompteParDefaut;
    int m_poste;
    int m_employeur;
    int m_medecin;
    int m_idCompteEncaissHonoraires;
    int m_enregHonoraires;
    int m_secteur;

    bool m_AGA;
    bool m_desactive;
    bool m_OPTAM;
    bool m_cotation;

    QDateTime m_dateDerniereConnexion;

    Site *m_Site = NULL;
    Comptes *m_comptes = NULL;


    //TODO : User : A vérifier Role
    int m_idUserActeSuperviseur = ROLE_INDETERMINE; //!< son id s'il est responsable de ses actes
                                                  //!< un idUser s'il est assistant
    int m_idUserParent = ROLE_INDETERMINE; //!< son id s'il n'est pas remplaçant
                                         //!< un idUser qui correspond au soigant remplacé
    int m_idUserCompta = ROLE_INDETERMINE; //!< son id s'il est responsable et libéral
                                         //!< l'id de son employeur s'il est responsable et salarié
                                         //!< s'il est remplaçant (retrocession) on lui demande qui il remplace et le comptable devient
                                         //!< . celui qu'il remplace si celui qu'il remplace est libéral
                                         //!< . l'employeur de celui qu'il remplace si celui qu'il remplace est salarié



public:
    explicit User(QJsonObject data = {}, QObject *parent = nullptr);
    explicit User(QString login, QString password, QJsonObject data = {}, QObject *parent = nullptr);

    void setData(QJsonObject data);

    QString getLogin() const;
    QString getPassword() const;

    int id() const;
    void setId( int value );

    QString getNom() const;
    QString getPrenom() const;
    int getSoignant() const;
    int getResponsableactes() const;
    int getUserenreghonoraires() const;
    int getUserccam() const;
    int getEnregHonoraires() const;
    QString getTitre() const;
    int getNoSpecialite() const;
    QString getSpecialite() const;
    int getNumPS() const;
    QString getNumCO() const;
    bool getAGA() const;
    int getEmployeur() const;
    int getIdLieu() const;
    int getIdCompteEncaissHonoraires() const;
    QString getNomUserEncaissHonoraires() const;
    QString getNomCompteEncaissHonoraires() const;
    QString getFonction() const;

    int getIdUserActeSuperviseur() const;
    void setIdUserActeSuperviseur(int idUserActeSuperviseur);
    int getIdUserParent() const;
    void setIdUserParent(int idUserParent);
    int getIdUserComptable() const;
    void setIdUserComptable(int idUserCompta);

    int getSecteur() const;
    int getIdCompteParDefaut() const;
    QString getMail() const;
    QString getLoginComptable() const;
    QString getNomCompteParDefaut() const;
    QString getPortable() const;
    QString getNomCompteAbrege() const;


    QString getStatus() const;


    Site* getSite() const;
    void setSite(Site *Site);

    Comptes *getComptes() const;
    void setComptes(Comptes *comptes);

    bool isAllLoaded() const;

    bool isOPTAM();
    bool isCotation();


    bool isSecretaire();
    bool isAutreFonction();
    bool isMedecin();
    bool isOpthalmo();
    bool isOrthoptist();
    bool isAutreSoignant();
    bool isNonSoignant();
    bool isSocComptable();
    bool isSoignant();
    bool isLiberal();
    bool isSalarie();
    bool isRemplacant();
    bool isSansCompta();
    bool isResponsable();
    bool isResponsableEtAssistant();
    bool isAssistant();

    bool isDesactive();


signals:

public slots:
};

#endif // CLS_USER_H
