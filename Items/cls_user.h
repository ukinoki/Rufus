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

/*!
 * \brief The User class
 * l'ensemble des informations concernant
 * une personne utilisant le logiciel
 * ex: Praticien, Secretaire, ...

 * IL existe 3 sortes d'utilisateurs
    * les utilisateurs soignants : médecin, orthoptiste, autres
    * les utilisateurs administratis: secrétaire, société comptable (une société comptable est une société qui va encaisser les honoraires  pour les redistribuer ensuite)
    * autres : utilisateur n'ayant accès ni aux dossiers médicaux, ni à la comptabilité :  sert pour un poste d'imageire par exemple pour retourver l'id d'un patient au moment d'entrer son dossier dans l'appareil d'imagerie


 * Sur la plan de la comptabilité, il existe 3 situations distinctes pour chaque utilisateur
    * la comptabilités des actes effectués
    * la comptabilité des recettes spéciales et des dépenses.
    * Pas de comptabilité
    * On distingue 4 types d'utilisateur sur le plan comptable:
        * l'utilisateur encaisse les honoraires liés aux actes : soignant libéral ou société comptable *///-> iscomptableactes()
            /*! ce type d'utilisateur est son propre comptableactes
                * encaisse des honoraires pour les actes,
                * encaisse des recettes spéciales
                * gère ses dépenses
        * l'utilisateur n'encaisse pas d'honoraires mais a une comptabilité des dépenses et des recettes spéciales : utilisateur libéral en société d'exercice libéral *///-> iscomptablnonactes()
            /*! ce type d'utilisateur n'a pas de comptabilités des actes -> !iscomptableactes() mais a un comptable pour les actes qu'il effectue, son employeur (un libéral ou une société comptable)
                * encaisse des recettes spéciales
                * gère ses dépenses
        * l'utilisateur ne tient pas de comptabilité
            * secrétaire, toute autre forme de personnel administratif ou autre
            * il n'a aucune forme de comptabilité
        * l'utilisateur est soigant remplaçant
            * le comptable de ses actes devient le comptables actes du libéral qu'il remplace (le libéral qu'il remplacee ou la société comptable qui emploie le libéral qu'il remplace)
            * il n'a ni compatbilité des actes, ni comptabilité des dépenses ou des recettes spéciales
        * un collaborateur a le même statut qu'un libéral

* Sur le plan du personnal soignant,
    * en ce qui concerne la situation vis à vis des actes médicaux
        * l'utilisateur peut être
            * responsable de ses actes : il est son propre superviseur
            * non responsable de ses actes (cas d'une orthoptiste en travail aidé)
                * il ne cote pas d'actes
                    * son superviseur est le médecin qu'il aide
                    * il n'a pas de supperviseur s'il aide plusieurs médecins
            * alterner les 2 : orthotpiste en travail aidé par moment et en responsable pour de la rééducation à d'autres
    * s'il est remplaçant
        * son parent est le soignant qu'il remplace
        * son statut de responsable ou non est celui du soignant qu'il remplace
    * s'il est collaborateur
        * il est son propre parent

 * c'est la fonction Procedures::DefinitRoleUser() qui, à l'ouveture d'une session Rufus va définir, pour l'utilisateur qui vient de ce connecter, son satut en définissant
    * le statut médical
        * isSoigant() ou isAutreSoignant()
            * si oui
                * isLiberal()
                * ou isLiberalSEL()
                * ou isRemplaçant()
                * ou isCollaborateur()

                * idSuperviseur()
                * idParent()
            * si non
                *.isAdministratif()
                * isSecrétaire()
                * isSocComptable()
    * le statut comptable
        * iscomptableactes()
        * iscomptablesnonactes()


*/
class User : public Item
{
    Q_OBJECT
public: //static
    static int ROLE_NON_RENSEIGNE; //-1
    static int ROLE_VIDE; //-2
    static int ROLE_INDETERMINE; //-3

    enum Compta {
        ComptaLess          = 0x0,
        ComptaNoMedical     = 0x1,
        ComptaMedicalActs   = 0x2
    };
    Q_DECLARE_FLAGS(ModeCompta, Compta)
    Q_ENUM(Compta)
    enum STATUT_COMPTABLE {Liberal, LiberalSEL, Salarie, Remplacant, NoCompta};                             Q_ENUM(STATUT_COMPTABLE)
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
    qlonglong m_numPS= 0;
    int m_noSpecialite;

    int m_employeur;
    int m_medecin;

    int m_enregHonoraires;
    int m_secteur;

    bool m_AGA = false;
    bool m_desactive = false;
    bool m_OPTAM = true;
    bool m_cotationactes = true;

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

    int m_idSuperviseur = ROLE_INDETERMINE;         //!< son id s'il est responsable de ses actes
                                                    //!< l'id du user assisté s'il est assistant
    int m_idParent = ROLE_INDETERMINE;              //!< son id s'il n'est pas remplaçant
                                                    //!< l'id du user remplacé s'il est remplaçant
    int m_idComptableActes = ROLE_INDETERMINE;      //!< si le  user est soignant
                                                            //! s'il est responsable de ses actes =>
                                                            //!< son id s'il est libéral
                                                            //!< l'id de son employeur s'il est salarié
                                                            //!< s'il est remplaçant (retrocession) on lui demande qui il remplace et le user comptable devient
                                                                //!< . celui qu'il remplace si celui qu'il remplace est libéral
                                                                //!< . l'employeur de celui qu'il remplace si celui qu'il remplace est salarié
                                                    //! s'il n'est pas responsable de ses actes
                                                            //! -2 = ROLE_VIDE - le comptable de l'acte sera enregistré au moment de l'enregistrement de la cotation
                                                            //! ce sera l'id du user qui enregistrera la cotation
    int m_idComptableNonActes = ROLE_INDETERMINE;   //!< si le  user est soignant
                                                            //!< son id s'il est libéral ou liberalSEL
                                                            //!< s'il est remplaçant ou collaborateur
                                                                //!< ROLE_VIDE
                                                    //!< si le  user n'est pas soignant
                                                            //!< son id s'il est une sociétécomptable
                                                            //!< sinon ROLE_VIDE


    ModeCompta m_modecompta = ComptaLess;
    STATUT_COMPTABLE statutcomptable() const;  //!< Liberal, Salarie, Remplaçant, Collaborateur, NoCompta
    RESPONSABLE responsableactes() const;

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

    int secteurconventionnel() const;
    void setsecteurconventionnel(int sectconventionnel) {m_secteur = sectconventionnel;}
    int idcomptepardefaut() const;
    QString mail() const;
    QString portable() const;

    QList<int> listecomptesbancaires(bool avecdesactive = false) const;
    void       setlistecomptesbancaires(QMap<int, bool> mapidcomptes);

    bool isOPTAM();
    void setOPTAM(bool optam) {m_OPTAM = optam;}
    bool useCotationsActes();

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

    bool isResponsable();
    bool isResponsableOuAssistant();
    bool isAssistant();

    bool isDesactive();
    void setdesactive(bool logic)               { m_desactive = logic;
                                                  m_data[CP_ISDESACTIVE_USR] = logic; }

    ModeCompta setmodecomptable()               {
                                                    if(isLiberal() || isSocComptable() || isLiberalSEL())
                                                        m_modecompta.setFlag(ComptaNoMedical);
                                                    if(isLiberal() || isSocComptable())
                                                        m_modecompta.setFlag(ComptaMedicalActs);
                                                    return m_modecompta;
                                                }
    ModeCompta modecomptable() const            { return m_modecompta;  }

    /*!
     * les données susceptibles de varier d'une session à l'autre - ne concerne que le user current ======================================================================================================================
     */

    int idsuperviseur() const                     { return m_idSuperviseur; }
    void setidsuperviseur(int idusr)              { m_idSuperviseur = idusr; }
    bool ishisownsupervisor()                     { return (m_idSuperviseur == m_id); }

    int idparent() const                          { return m_idParent; }
    void setidparent(int idusr)                   { m_idParent = idusr; }

    int idcomptableactes() const                       { return m_idComptableActes; }
    void setidcomptableactes(int idusr)                { m_idComptableActes = idusr; }
};

Q_DECLARE_OPERATORS_FOR_FLAGS(User::ModeCompta)

#endif // CLS_USER_H
