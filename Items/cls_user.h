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

#ifndef CLS_USER_H
#define CLS_USER_H

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

    enum ENREGISTREMENTHONORAIRES {Liberal, LiberalSEL, Salarie, Retrocession, NoCompta};                   Q_ENUM(ENREGISTREMENTHONORAIRES)
    enum METIER {Ophtalmo, Orthoptiste, AutreSoignant, NonSoignant, SocieteComptable, NoMetier, Neutre};    Q_ENUM(METIER)
    enum RESPONSABLE {Responsable, AlterneResponsablePasResponsable, PasResponsable};                       Q_ENUM(RESPONSABLE)

private:

/*!
 * les données figées, ne variant pas d'une session à l'autre
 */

    //!< m_id = Id de l'utilisateur en base

    QString m_login = "";               //!< Identifiant de l'utilisateur
    QString m_password = "";            //!> mot de passe de l'utilisateur
    QString m_prenom = "";              //!< prénom de l'utilisateur
    QString m_nom = "";                 //!< nom de l'utilisateur
    QString m_droits = "";              //!> les droits de l'utilisateur - correspond à l'enum METIER -> Ophtalmo, Orthoptiste, AutreSoignant, NonSoignant, SocieteComptable, NoMetier
    QString m_mail = "";                //!> le mail
    QString m_titre = "";               //!< titre, ex: Docteur
    QString m_fonction = "";            //!< la fonction précise de l'utilisateur
    QString m_specialite = "";          //!> la spécialité
    QString m_numCO = "";               //!> le no du conseil de l'ordre
    QString m_portable = "";            //!> le no de téléphone portable
    QString m_memo = "";                //!> memo sur l'utilisateur
    QString m_policeEcran = "";         //!> le choix de police d'écran de l'utilisateur
    QString m_policeAttribut = "";      //!> le choix d'attribut de la police d'écran
    bool m_affichedocspublics = true;   //!> affiche les docs publics des autres utilisateurs dans la fiche impressions
    bool m_affichecommentslunettespublics = true;   //!> affiche les commentaires publics des autres utilisateurs dans la fiche commentaires lunettes

    int m_soignant;
    qlonglong m_numPS;
    int m_noSpecialite;

    int m_employeur;
    int m_medecin;

    int m_enregHonoraires;
    int m_secteur;

    int m_typeCompta = COMPTA_AVEC_COTATION_AVEC_COMPTABILITE;

    bool m_AGA = false;
    bool m_desactive = false;
    bool m_OPTAM = true;
    bool m_ccam;

    bool m_useCompta;
    int m_responsableActes; //!< 0 : pas responsable
                            //!< 1 : responsable
                            //!< 2 : responsable et assistant
                            //!< 3 : assistant

    QDateTime   m_dateDerniereConnexion;
    QDate       m_datecreationMDP;                  //! la date de creation du mot de passe actuel

    QList<int> m_listidcomptesall;                  //! tous les id des comptes bancaires de l'utilisateur  y compris ceux qui sont devenus inactifs
    QList<int> m_listidcomptes;                     //! tous les id des comptes bancaires actifs de l'utilisateur
    int m_idCompteParDefaut = 0;                    //! le compte bancaire personnel utilisé pour la comptabilité personnelle
    int m_idCompteEncaissHonoraires = 0;            //! le compte bancaire utilisé pour l'enregistrement des recettes (différent du compte personnel en cas d'exercice en société type SEL)

/*!
 * les données susceptibles de varier d'une session à l'autre - ne concernent que l'utilisateur courant
 */

    int m_idUserSuperviseur = ROLE_INDETERMINE;     //!< son id s'il est responsable de ses actes
                                                    //!< l'id du user assisté s'il est assistant
    int m_idUserParent = ROLE_INDETERMINE;          //!< son id s'il n'est pas remplaçant
                                                    //!< l'id du user remplacé s'il est remplaçant
    int m_idUserComptable = ROLE_INDETERMINE;       //!< si le  user est soignant
                                                        //! s'il est responsable de ses actes =>
                                                            //!< son id s'il est libéral
                                                            //!< l'id de son employeur s'il est salarié
                                                            //!< s'il est remplaçant (retrocession) on lui demande qui il remplace et le user comptable devient
                                                            //!< . celui qu'il remplace si celui qu'il remplace est libéral
                                                            //!< . l'employeur de celui qu'il remplace si celui qu'il remplace est salarié
                                                        //! s'il n'est pas responsable de ses actes
                                                            //! -2 = ROLE_VIDE - le comptable de l'acte sera enregistré au moment de l'enregistrement de la cotation
                                                            //! ce sera l'id du user qui enregistrera la cotation
public: 
    explicit User(QJsonObject data = {}, QObject *parent = Q_NULLPTR);
    explicit User(QString login, QString password, QJsonObject data = {}, QObject *parent = Q_NULLPTR);

    void setData(QJsonObject data);

    /*!
     * les données figées, ne variant pas d'une session à l'autre =======================================================================================================================
     */

    void setid(int id)                      { m_id = id;
                                              m_data[CP_ID_USR] = id; }
    QString login() const;
    QString password() const;
    void setpassword(QString psswd)         { m_password = psswd;
                                              m_data[CP_MDP_USR] = psswd;}
    QDate datecreationMDP() const           { return m_datecreationMDP; }
    void setdatecreationMDP(QDate date)     { m_datecreationMDP = date;
                                              m_data[CP_DATECREATIONMDP_USR] = date.toString("yyyy-MM-dd"); }
    QString nom() const;
    QString prenom() const;
    QString grandnom() const {return (m_prenom !=""? m_prenom + " " : "") + m_nom;};
    METIER metier() const;                                          //!< Ophtalmo, Orthoptiste, AutreSoignant, NonSoignant, SocieteComptable, NoMetier
    ENREGISTREMENTHONORAIRES modeenregistrementhonoraires() const;  //!< Liberal, Salarie, Retrocession, NoCompta
    QString titre() const;
    int numspecialite() const;
    QString specialite() const;
    qlonglong NumPS() const;
    QString numOrdre() const;
    bool isadmin() const                    { return m_login == NOM_ADMINISTRATEUR; }
    bool isAGA() const;
    void setAGA(bool aga) {m_AGA = aga;}
    int idemployeur() const;
    int idcompteencaissementhonoraires() const;
    void setidcompteencaissementhonoraires(int id)  { m_idCompteEncaissHonoraires = id;}
    QString fonction() const;

    QFont police() const {
                    QFont policeEcran = QFont(m_policeEcran);
                    if (!m_policeAttribut.contains("Regular",Qt::CaseInsensitive))
                    {
                        if (m_policeAttribut.contains("Italic",Qt::CaseInsensitive))
                            policeEcran.setStyle(QFont::StyleItalic);
                        if (m_policeAttribut.contains("Light",Qt::CaseInsensitive))
                            policeEcran.setWeight(QFont::Light);
                        if (m_policeAttribut.contains("Normal",Qt::CaseInsensitive))
                            policeEcran.setWeight(QFont::Normal);
                        if (m_policeAttribut.contains("SemiBold",Qt::CaseInsensitive))
                            policeEcran.setWeight(QFont::DemiBold);
                        if (m_policeAttribut.contains("Bold",Qt::CaseInsensitive))
                            policeEcran.setWeight(QFont::Bold);
                        if (m_policeAttribut.contains("Black",Qt::CaseInsensitive))
                            policeEcran.setWeight(QFont::Black);
                    }
                    Utils::CalcFontSize(policeEcran);
                    return policeEcran; }
    QString policeattribut() const  { return m_policeAttribut; }
    bool    affichedocspublics() const { return m_affichedocspublics; }
    void    setaffichedocspublics(bool aff)  { m_affichedocspublics = aff; }
    bool    affichecommentslunettespublics() const { return m_affichecommentslunettespublics; }
    void    setaffichecommentslunettespublics(bool aff)  { m_affichecommentslunettespublics = aff; }

    RESPONSABLE responsableactes() const;

    int secteurconventionnel() const;
    void setsecteurconventionnel(int sectconventionnel) {m_secteur = sectconventionnel;}
    int idcomptepardefaut() const;
    QString mail() const;
    QString portable() const;

    QList<int> listecomptesbancaires(bool avecdesactive = false) const;
    void       setlistecomptesbancaires(QMap<int, bool> mapidcomptes);

    int typecompta() const;
    void setTypeCompta(int typeCompta)           { m_typeCompta = typeCompta; }

    bool isOPTAM();
    void setOPTAM(bool optam) {m_OPTAM = optam;}
    bool useCCAM();

    bool isSecretaire();
    bool isAutreFonction();
    bool isMedecin();
    bool isOpthalmo();
    bool isOrthoptist();
    bool isAutreSoignant();
    bool isNonSoignant();
    bool isSocComptable();
    bool isNeutre();
    bool isSoignant();
    bool isComptableActes();
    bool isLiberal();
    bool isLiberalSEL();
    bool isSoignantSalarie();
    bool isRemplacant();
    bool isSansCompta();
    bool isResponsable();
    bool isResponsableOuAssistant();
    bool isAssistant();

    bool isDesactive();
    void setdesactive(bool logic)                 { m_desactive = logic;
                                                    m_data[CP_ISDESACTIVE_USR] = logic; }

    /*!
     * les données susceptibles de varier d'une session à l'autre - ne concerne que le user current ======================================================================================================================
     */

    int idsuperviseur() const                     { return m_idUserSuperviseur; }
    void setidsuperviseur(int idusr)              { m_idUserSuperviseur = idusr; }
    bool ishisownsupervisor()                     { return (m_idUserSuperviseur == m_id); }

    int idparent() const                          { return m_idUserParent; }
    void setidparent(int idusr)                   { m_idUserParent = idusr; }

    int idcomptable() const                       { return m_idUserComptable; }
    void setidcomptable(int idusr)                { m_idUserComptable = idusr; }
};

#endif // CLS_USER_H
