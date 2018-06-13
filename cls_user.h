#ifndef CLS_USER_H
#define CLS_USER_H

#include <QMap>
#include <QObject>
#include <QVariant>
#include <QDate>
#include <QJsonObject>
#include "macros.h"

class User : public QObject
{
    Q_OBJECT
public: //static
    static int ROLE_NON_RENSEIGNE; //-1
    static int ROLE_VIDE; //-2
    static int ROLE_INDETERMINE; //-3

    static User create();
    static QMap<QString,QVariant> CalculAge(QDate datedenaissance);
    static QMap<QString,QVariant> CalculAge(QDate datedenaissance, QDate datedujour);
    static QMap<QString,QVariant> CalculAge(QDate datedenaissance, QString Sexe, QDate datedujour = QDate::currentDate());



private:
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

    //Adresse de travail
    int m_idLieu;
    QString m_nomLieu;
    QString m_adresse1;
    QString m_adresse2;
    QString m_adresse3;
    int m_codePostal;
    QString m_ville;
    QString m_telephone;
    QString m_fax;


    //TODO : User : A vérifier
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
    explicit User(QObject *parent = nullptr);
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
    int getIdUserParent() const;
    int getIdUserComptable() const;
    int getSecteur() const;
    int getIdCompteParDefaut() const;
    QString getMail() const;
    QString getLoginComptable() const;
    QString getNomCompteParDefaut() const;
    QString getPortable() const;


    QString getStatus() const;


    QString getNomlieu() const;
    QString getAdresse1() const;
    QString getAdresse2() const;
    QString getAdresse3() const;
    int getCodePostal() const;
    QString getVille() const;
    QString getTelephone() const;


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

private:
    void setDataString(QJsonObject data, QString key);
    void setDataInt(QJsonObject data, QString key);
    void setDataBool(QJsonObject data, QString key);
    void setDataDateTime(QJsonObject data, QString key);

signals:

public slots:
};

#endif // CLS_USER_H
