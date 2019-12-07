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

#ifndef CLS_USER_H
#define CLS_USER_H

#include <QMap>
#include <QVariant>
#include <QDate>
#include "cls_item.h"
#include "macros.h"
#include "log.h"

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

    static int COMPTA_AVEC_COTATION_AVEC_COMPTABILITE; //0
    static int COMPTA_SANS_COTATION_SANS_COMPTABILITE; //1
    static int COMPTA_AVEC_COTATION_SANS_COMPTABILITE; //2
    static int COMPTA_SANS_COTATION_AVEC_COMPTABILITE; //3

    enum ENREGISTREMENTHONORAIRES {Liberal, Salarie, Retrocession, NoCompta};                       Q_ENUM(ENREGISTREMENTHONORAIRES)
    enum METIER {Ophtalmo, Orthoptiste, AutreSoignant, NonSoignant, SocieteComptable, NoMetier};    Q_ENUM(METIER)
    enum RESPONSABLE {Responsable, AlterneResponsablePasResponsable, PasResponsable};               Q_ENUM(RESPONSABLE)

private:
    bool m_isAllLoaded = false;

/*!
 * les données figées, ne variant pas d'une session à l'autre
 */

    //!< m_id = Id de l'utilsateur en base

    QString m_login = "";               //!< Identifiant de l'utilisateur
    QString m_password = "";            //!> mot de passe de l'utilisateur
    QString m_prenom = "";              //!< prénom de l'utilisateur
    QString m_nom = "";                 //!< nom de l'utilisateur
    QString m_droits = "";              //!> les droits de l'utilisateur - correspond à l'enum METIER -> Ophtalmo, Orthoptiste, AutreSoignant, NonSoignant, SocieteComptable, NoMetier
    QString m_mail = "";                //!> le mail
    QString m_titre = "";               //!< titre, ex: Docteur
    QString m_fonction = "";            //! la fonction précise de l'utilisateur
    QString m_specialite = "";          //!> la spécialité
    QString m_numCO = "";               //!> le no du conseil de l'ordre
    QString m_portable = "";            //!> le no de téléphone portable
    QString m_web = "";                 //!> le site web
    QString m_memo = "";                //!> memo sur l'utilisateur
    QString m_policeEcran = "";         //!> le choix de police d'écran de l'utilisateur
    QString m_policeAttribut = "";      //! lle choix d'attribut de la police d'écran
    QString m_nomCompteEncaissHonoraires = "";  //!> le compte sur lequel sont encaissés les honoraires

    int m_soignant;
    qlonglong m_numPS;
    int m_noSpecialite;

    int m_poste;
    int m_employeur;
    int m_medecin;

    int m_enregHonoraires;
    int m_secteur;

    int m_typeCompta = COMPTA_AVEC_COTATION_AVEC_COMPTABILITE;

    bool m_AGA;
    bool m_desactive = false;
    bool m_OPTAM;
    bool m_ccam;

    bool m_useCompta;
    int m_responsableActes; //!< 0 : pas responsable
                            //!< 1 : responsable
                            //!< 2 : responsable et assistant
                            //!< 3 : assistant

    QDateTime m_dateDerniereConnexion;

    QList<int> *m_listidcomptesall = Q_NULLPTR;     //! tous les id des comptes de l'utilisateur  y compris ceux qui sont devenus inactifs
    QList<int> *m_listidcomptes = Q_NULLPTR;        //! tous les id des comptes actifs de l'utilisateur
    int m_idCompteParDefaut = 0;                    //! le compte bancaire personnel utilisé pour la comptabilité personnelle
    int m_idCompteEncaissHonoraires = 0;            //! le compte bancaire utilisé pour l'enregistrement des recettes (différent du compte personnel en cas d'exercice en société type SEL)

/*!
 * les données susceptibles de varier d'une session à l'autre
 */

    User *m_userSuperviseur     = Q_NULLPTR;
    int m_idUserSuperviseur = ROLE_INDETERMINE;     //!< son id s'il est responsable de ses actes
                                                    //!< l'id du user assisté s'il est assistant
    User *m_userParent = Q_NULLPTR;
    int m_idUserParent = ROLE_INDETERMINE;          //!< son id s'il n'est pas remplaçant
                                                    //!< l'id du user remplacé s'il est remplacé
    User *m_userComptable = Q_NULLPTR;
    int m_idUserComptable = ROLE_INDETERMINE;       //!< son id s'il est responsable et libéral
                                                    //!< l'id de son employeur s'il est responsable et salarié
                                                    //!< s'il est remplaçant (retrocession) on lui demande qui il remplace et le user comptable devient
                                                    //!< . celui qu'il remplace si celui qu'il remplace est libéral
                                                    //!< . l'employeur de celui qu'il remplace si celui qu'il remplace est salarié

public:
    explicit User(QJsonObject data = {}, QObject *parent = Q_NULLPTR);
    explicit User(QString login, QString password, QJsonObject data = {}, QObject *parent = Q_NULLPTR);

    void setData(QJsonObject data);

    /*!
     * les données figées, ne variant pas d'une session à l'autre =======================================================================================================================
     */

    QString login() const;
    QString password() const;
    void setpassword(QString psswd);

    QString nom() const;
    QString prenom() const;
    METIER metier() const;                                          //!< Ophtalmo, Orthoptiste, AutreSoignant, NonSoignant, SocieteComptable, NoMetier
    ENREGISTREMENTHONORAIRES modeenregistrementhonoraires() const;  //!< Liberal, Salarie, Retrocession, NoCompta
    QString titre() const;
    int numspecialite() const;
    QString specialite() const;
    qlonglong NumPS() const;
    QString numOrdre() const;
    bool isAGA() const;
    int idemployeur() const;
    int idcompteencaissementhonoraires() const;
    void setidcompteencaissementhonoraires(int id);
    QString fonction() const;

    QFont police() const {
                    QString attribut = m_policeAttribut;
                    QFont policeEcran = QFont(m_policeEcran);
                    if (!attribut.contains("Regular",Qt::CaseInsensitive))
                    {
                        if (attribut.contains("Italic",Qt::CaseInsensitive))
                            policeEcran.setStyle(QFont::StyleItalic);
                        if (attribut.contains("Light",Qt::CaseInsensitive))
                            policeEcran.setWeight(QFont::Light);
                        if (attribut.contains("Normal",Qt::CaseInsensitive))
                            policeEcran.setWeight(QFont::Normal);
                        if (attribut.contains("SemiBold",Qt::CaseInsensitive))
                            policeEcran.setWeight(QFont::DemiBold);
                        if (attribut.contains("Bold",Qt::CaseInsensitive))
                            policeEcran.setWeight(QFont::Bold);
                        if (attribut.contains("Black",Qt::CaseInsensitive))
                            policeEcran.setWeight(QFont::Black);
                    }
                    Utils::CalcFontSize(policeEcran);
                    return policeEcran; }
    QString policeattribut() const  { return m_policeAttribut; }

    RESPONSABLE responsableactes() const;

    int secteurconventionnel() const;
    int idcomptepardefaut() const;
    QString mail() const;
    QString portable() const;

    QList<int>*     listecomptesbancaires(bool avecdesactive = false) const;
    void            setlistecomptesbancaires(QMap<int, bool> mapidcomptes);

    int typecompta() const;
    void setTypeCompta(int typeCompta);

    bool isAllLoaded() const;

    bool isOPTAM();
    bool useCCAM();

    bool isSecretaire();
    bool isAutreFonction();
    bool isMedecin();
    bool isOpthalmo();
    bool isOrthoptist();
    bool isAutreSoignant();
    bool isNonSoignant();
    bool isSocComptable();
    bool isSoignant();
    bool isComptable();
    bool isLiberal();
    bool isSalarie();
    bool isRemplacant();
    bool isSansCompta();
    bool isResponsable();
    bool isResponsableOuAssistant();
    bool isAssistant();

    bool isDesactive();

    /*!
     * les données susceptibles de varier d'une session à l'autre  ======================================================================================================================
     */

    User *superviseur() const;
    void setsuperviseur(User *usr);
    int idsuperviseur() const;
    void setidsuperviseur(int idusr);
    bool ishisownsupervisor();

    User *parent() const;
    void setparent(User *usr);
    int idparent() const;
    void setidparent(int idusr);

    User *comptable() const;
    void setcomptable(User *usr);
    int idcomptable() const;
    void setidusercomptable(int idusr);

    QString status() const;
};

#endif // CLS_USER_H
