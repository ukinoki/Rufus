/* (C) 2025 LAINE SERGE
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

#ifndef MYSQLINSTALLER_H
#define MYSQLINSTALLER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QHash>
#include <QMap>
#include <QDateTime>
#include <QList>
#include <QPair>
#include <QDialog>
#include <QLabel>
#include <QProgressBar>

#include "updialog.h"
#include "upmessagebox.h"
#include "upcheckbox.h"
#include "uplabel.h"
#include "uplineedit.h"
#include "upsmallbutton.h"
#include "macros.h"

/*! Config MySQL distante : version à installer, seuil minimal accepté, URLs de téléchargement. */
struct MySQLRemoteConfig {
    QString version;       /*!< version cible à installer / proposer en mise à jour */
    QString minVersion;    /*!< version minimale acceptée en mode Verify (seuil) */
    QString winUrl;        /*!< URL de téléchargement Windows */
    QString macArm64Url;   /*!< URL de téléchargement macOS Apple Silicon */
    QString macX86Url;     /*!< URL de téléchargement macOS Intel */
};

/*! MySQLProgressDialog — petite fiche de progression (téléchargement / installation).
    * total <= 0 -> barre animée (indéterminée) ; sinon pourcentage.
    * setProgress affiche en plus le volume « X / Y Mo » (gros DMG macOS ~550 Mo).
*/
class MySQLProgressDialog : public QDialog {
    Q_OBJECT
public:
    explicit MySQLProgressDialog(const QString& operation, QWidget* parent = nullptr);

    void setProgress(qint64 received, qint64 total);   /*!< octets -> barre + détail « X / Y Mo » */
    void setProgressBar(qint64 done, qint64 total);    /*!< fraction -> barre SEULE (pas de volume) */

private:
    QLabel*       m_label;
    QProgressBar* m_progress;
    QLabel*       m_detail;   /*!< volume téléchargé « X / Y Mo » */
};

/*! MySQLInstallerDialog — la fiche de l'installeur (dérive d'UpDialog).
    * Haut : titre + sous-titre. Milieu : saisie identifiant + mot de passe (2 UpLineEdit, plus une
      confirmation en mode création). Bas : checklist des 7 critères (affichage seul, cochée par le
      moteur MySQLInstaller via checkStep).
    * Une même fiche, plusieurs contextes : les configurer*() règlent titre / sous-titre / libellé du
      bouton OK et vident les champs (saisie utilisateur Rufus, compte admin MySQL, nouvel utilisateur).
    * Deux usages : exec() (modal) pour SAISIR des identifiants ; show() pendant que le moteur déroule
      les étapes et coche les cases.
*/
class MySQLInstallerDialog : public UpDialog {
    Q_OBJECT
public:
    explicit MySQLInstallerDialog(QWidget* parent = nullptr);

    /*! Code de résultat de exec() quand l'utilisateur clique « Supprimer MySQL » (QDialog réserve 0/1). */
    enum DialogResult { ResultSupprimerMySQL = 2 };

    /*! ── Configuration selon le contexte (titre / sous-titre / bouton OK + reset des champs) ─────────── */
    void configurerCreateUserRufus(const QString& minVersion);                    /*!< contexte : saisie du futur utilisateur Rufus */
    void configurerVerifyAdminMySQL();                                            /*!< contexte : saisie d'un compte ADMIN MySQL existant */
    void passerEnConfiguration(const QString& titre, const QString& sousTitre);   /*!< bascule en « paramétrage en cours » (champs figés, boutons masqués) */
    void masquerSaisieUtilisateur();                                              /*!< cache la ligne login/mdp (réinstallation lors d'une migration) */

    /*! ── Saisie ─────────────────────────────────────────────────────────────────────────────────────── */
    QString login() const;      /*!< texte courant du champ login */
    QString password() const;   /*!< texte courant du champ mot de passe */
    bool    validerSaisie();    /*!< login ET mdp renseignés ? (message + false sinon) */

    /*! ── Checklist ──────────────────────────────────────────────────────────────────────────────────── */
    void checkStep(int i);                              /*!< coche la case i + repaint */
    void setMinVersion(const QString& v);               /*!< re-libelle l'étape 0 */
    bool wasCancelled() const { return m_cancelled; }   /*!< l'utilisateur a-t-il annulé ? */
    void reject() override;                             /*!< marque l'annulation (slot public, comme QDialog) */

private:
    QString baseStepLabel(int i) const;           /*!< libellé de base de la case i */
    void    applyStepLabel(int i);                /*!< (ré)applique le libellé i (base + detail) */
    void    configurer(const QString& titre, const QString& sousTitre,
                       const QString& okLabel);   /*!< applique titre/sous-titre/bouton + vide les champs */

    UpLabel*       m_title         = nullptr;
    UpLabel*       m_subtitle      = nullptr;
    UpLabel*       m_loginLbl      = nullptr;
    UpLabel*       m_mdpLbl        = nullptr;
    UpLineEdit*    m_login         = nullptr;
    UpLineEdit*    m_mdp           = nullptr;
    UpLabel*       m_mdpConfirmLbl = nullptr;           /*!< confirmation du mot de passe (création de compte) */
    UpLineEdit*    m_mdpConfirm    = nullptr;
    bool           m_confirmMdpRequis = false;          /*!< mode CRÉATION : validerSaisie() exige mdp == confirmation */
    UpSmallButton* m_btnSupprMySQL = nullptr;           /*!< « Supprimer MySQL » (mode Verify) */
    UpCheckBox*    m_steps[7];                          /*!< 0..5 = config ; 6 = clés SSL (accès distant) */
    QString        m_minVersion = VERSION_MYSQL_MINI;   /*!< seuil commun (8.0.14), écrasé par setMinVersion() */
    bool           m_cancelled  = false;
};

/*! MySQLInstaller — moteur d'installation, de vérification et de sécurisation du serveur MySQL de Rufus.
    Garantit au démarrage qu'un serveur MySQL conforme est prêt (installé, configuré, sécurisé), et fournit
    l'entretien qui suit (mot de passe, clés SSL). Tout est LOCAL : les données ne quittent jamais le cabinet.

    * POINT D'ENTRÉE — run() (synchrone), cf. initialisation Rufus.txt § II.1 :
        * MySQL absent                -> faireCreate() : installe MySQL, crée les comptes, configure.
        * MySQL présent               -> avertissement d'effacement, AskMdpLoginMySQL(), et si le serveur
          porte déjà une base Rufus (isBaseRufus) offre de la sauvegarder AVANT de rien détruire.
            * socle conforme + compte admin -> faireReutiliser() : on garde le serveur.
            * sinon -> reinstallerSocleMySQLpourMigration() : désinstallation puis serveur neuf.
        * renvoie true si un MySQL conforme est prêt.
    * ÉTAPES DE CONFIG — executerEtapesConfig(), affichées dans la checklist de MySQLInstallerDialog :
        * 0 connexion admin   1 PATH   2 dossier partagé   3 secure_file_priv
        * 4 lecture/écriture   5 privilèges                6 clés SSL (accès distant)
    * COMPTES & MOTS DE PASSE (le cœur « sécurité ») :
        * comptes techniques adminrufus (local/LAN) et adminrufusSSL (accès distant, REQUIRE SSL).
        * chaque poste connaît le mot de passe ALÉATOIRE de son mode (MONO/LAN/WAN) via .dbkey (une ligne
          CLE=mdp par mode) ; cache mémoire s_cacheMDP.
        * gaxt78iy = mot de passe GÉNÉRIQUE de secours, conservé comme 2e mot de passe (RETAIN CURRENT
          PASSWORD) 30 j après la sécurisation puis effacé -> laisse aux autres postes (et à la 1re
          connexion, cache caching_sha2 « froid ») le temps de récupérer l'aléatoire.
        * securiser* posent l'aléatoire ; JAMAIS depuis un poste distant. connecterAvecCandidats essaie
          [aléatoire, gaxt78iy] et retient celui qui marche.
    * CLÉS SSL (accès distant chiffré) : contrôle d'expiration au démarrage, (ré)extraction / régénération,
      conservation lors d'une migration de socle.
    * ENTRETIEN AU DÉMARRAGE — entretienApresConnexion() : sécurise au besoin puis efface gaxt78iy à
      échéance ; helpers d'avertissement / récupération quand un poste n'a pas l'aléatoire.
    * PLATEFORMES : Windows / macOS / Linux(Ubuntu). Le paramétrage root est regroupé en UNE élévation par
      plateforme (prepareCreateMode*) pour ne demander le mot de passe système qu'une fois.
*/
class MySQLInstaller : public QObject {
    Q_OBJECT
public:
    explicit MySQLInstaller(QObject* parent = nullptr);

    bool run();   /*!< point d'entrée synchrone : true si MySQL conforme prêt */

    QString loginRufus() const { return m_loginRufus; }   /*!< login du futur utilisateur Rufus (vide si run() a échoué) */
    QString mdpRufus()   const { return m_mdpRufus; }     /*!< son mot de passe EN CLAIR */
    bool    baseRufusTrouvee() const { return m_baseRufusTrouvee; }   /*!< une base Rufus était présente sur le serveur effacé */

    /*! ── Mot de passe aléatoire / .dbkey (helpers statiques) ─────────────────────────────────────────── */
    static QString     genererMotDePasse();                                                    /*!< mot de passe aléatoire fort (12 car., [A-Za-z0-9]) */
    static QString     motDePasseSQL();                                                        /*!< mdp du mode courant (.dbkey), repli gaxt78iy ; caché, sans UI */
    static void        setMotDePasseSQL(const QString& mdp);                                   /*!< met à jour le mdp du mode courant EN MÉMOIRE (cache) */
    static QStringList motsDePasseSQLCandidats();                                              /*!< mdp à essayer dans l'ordre : [aléatoire du mode, gaxt78iy] */
    static QString     connecterAvecCandidats(const QString& basename);                        /*!< connexion adminrufus en essayant tous les candidats ; retient celui qui marche */
    static void        stockerMotDePasse(const QString& mdp);                                  /*!< écrit le mdp du mode courant dans .dbkey + cache */
    static QString     motDePasseStockePourMode(Utils::ModeAcces mode);                        /*!< mdp brut stocké pour un mode ("" si aucun) */
    static void        stockerMotDePassePourMode(Utils::ModeAcces mode, const QString& mdp);   /*!< écrit le mdp d'un mode explicite */
    static void        supprimerMotDePassePourMode(Utils::ModeAcces mode);                     /*!< oublie le mdp d'un mode abandonné */

    /*! ── Sécurisation de la base ─────────────────────────────────────────────────────────────────────── */
    bool securiserBaseSiNecessaire();     /*!< pose l'aléatoire si la base ne l'a pas encore (gaxt78iy retenu 2e mdp) ; no-op si déjà fait / socle non compatible */
    bool poserEtSauvegarderAleatoire();   /*!< génère + sauve + pose un aléatoire sur adminrufus/SSL (local, socle conforme) */

    /*! Pose `aleatoire` sur tous les comptes adminrufus (le mot de passe courant restant en 2e), puis
     * appelle l'entretien ci-dessous. true si tous les buts atteints, *detailresult but par but. */
    bool securiserAdminrufusEtMdp(const QString& aleatoire,
                                     QMap<QString, bool>* detailresult = nullptr);

    /*! ENTRETIEN des comptes, SANS toucher à aucun mot de passe existant : crée les adminrufus
     * restreints au LAN s'ils manquent, redonne les privilèges (donc SYSTEM_USER), retire
     * adminrufus@'%'. C'est ce qu'on appelle à chaque démarrage. */
    bool entretienComptesAdminrufusLAN(const QString& mdpCourant);

    /*! ── État du socle / du serveur ──────────────────────────────────────────────────────────────────── */
    static bool socleMySQLConforme();                   /*!< socle MySQL conforme à Rufus (>= 8.0.14 et pas MariaDB) */
    static bool socleLocalConforme();                   /*!< idem, mais relevé SANS connexion (serveur LOCAL) : pour les décisions qui précèdent la connexion */
    static bool serveurLocalPresent();                  /*!< un serveur MySQL local est-il installé ? (avant toute connexion) */
    void        verifierEtReparerConfigMonoposte();     /*!< monoposte : vérif silencieuse de la config, propose de réparer sinon */
    bool        reinstallerSocleMySQLpourMigration();   /*!< migration DESTRUCTIVE du socle (après sauvegarde validée) */

    /*! ── Clés SSL (accès distant chiffré) ────────────────────────────────────────────────────────────── */
    bool        sauvegarderClesSSLMigration();       /*!< copie les .pem du datadir vers un stash (avant désinstall) */
    void        restaurerClesSSLMigration();         /*!< réinjecte les .pem dans le nouveau datadir + redémarre */
    static bool clesSSLServeurPresentes();           /*!< la copie CLIENT exportable des clés existe-t-elle ? */
    QDateTime   dateExpirationCertSSL();             /*!< date d'expiration du certificat serveur (invalide si SSL inactif) */
    bool        extraireClesSSLDepuisDatadir();      /*!< réextrait les clés client du datadir SANS redémarrer */
    bool        regenererClesSSL();                  /*!< DESTRUCTIF : régénère les certs serveur (invalide les clés déjà distribuées) */
    void        controlerClesSSLMonoposte();         /*!< monoposte au démarrage : réextrait ou régénère selon l'état des clés */
    void        avertirExpirationClesSSLDistant();   /*!< accès distant : prévient si les certs SSL approchent de l'expiration */

    /*! ── Entretien au démarrage (après connexion) ────────────────────────────────────────────────────── */
    static void      entretienApresConnexion();                                       /*!< sécurise au besoin puis efface gaxt78iy à échéance */
    static bool      adminrufusEstSecurise();                                         /*!< adminrufus a-t-il un 2e mot de passe (base déjà sécurisée) ? */
    static bool      unCompteLANaPerduSystemUser();                                   /*!< un compte adminrufus local a-t-il perdu SYSTEM_USER (à régulariser) ? */
    static QDateTime dateSecurisation();                                              /*!< date de sécurisation (password_last_changed d'adminrufus) */
    static QString   posteSecurisation();                                             /*!< nom du poste ayant posé l'aléatoire (vide si version antérieure) */
    static void      supprimerGaxt78iySiEchue();                                      /*!< efface gaxt78iy si la deadline est passée (et si ce poste a l'aléatoire) */
    void             avertirEffacementImminent();                                     /*!< informe que l'effacement de gaxt78iy approche (informatif) */
    void             proposerRecuperationAleatoire();                                 /*!< base sécurisée mais poste sur gaxt78iy : propose de saisir l'aléatoire */
    void             suggererSecurisationDepuisLocal();                               /*!< poste distant sur base non sécurisée : invite à sécuriser depuis le LAN */
    bool             recreerMotDePasseApresVerifAdmin(QWidget *parent = Q_NULLPTR);   /*!< recrée l'aléatoire quand plus aucun poste ne l'a (protégé par mdp Admin) */

    /*! ── Compte de secours (et suppression de root) ──────────────────────────────────────────────────── */
    static bool rootExiste();                                            /*!< un compte 'root' subsiste-t-il sur ce serveur ? */
    static bool compteDeSecoursExiste();                                 /*!< secoursrufus est-il en place sur TOUS les hosts LAN ? */
    static bool creerCompteDeSecours(QWidget* parent = Q_NULLPTR);       /*!< demande le mdp confidentiel, crée secoursrufus sur les hosts LAN, l'éprouve puis supprime root */
    static void controlerCompteDeSecours();                              /*!< à chaque démarrage (poste serveur) : met en place le secours quand l'utilisateur n°1 se connecte, et supprime root */
    bool        restaurerAvecMotDePasseDeSecours(QWidget* parent = Q_NULLPTR);   /*!< dernier recours : le mdp de secours réécrit un aléatoire neuf sur adminrufus (+ .dbkey) */

    /*! ── Récupération du mot de passe du cabinet ─────────────────────────────────────────────────────── */
    enum class IssueMdp {
        Obtenu,         /*!< mot de passe récupéré et éprouvé */
        Annule,         /*!< annuler et sortir */
        Inconnu,        /*!< je n'ai aucun mot de passe */
        Reinitialiser,  /*!< repartir d'une base neuve */
        EchecSaisie     /*!< mot de passe saisi ou importé, mais il n'ouvre pas la base */
    };

    /*! Saisie ou clé USB, puis épreuve du mot de passe. titre/corps vides → message d'échec de
     * connexion ; monoposte → 5 boutons au lieu de 3 (cf. initialisation Rufus.txt § II.2). */
    static IssueMdp RecupererMotDePasseMySQL(QWidget *parent, const QString &titre = QString(),
                                             const QString &corps = QString(),
                                             bool monoposte = false);

    enum class CreateUserResult { Ok, NoCreateUserRight, Error };   /*!< résultat de createUserAvecAdmin() */

private:
    static QHash<QString,QString> s_cacheMDP;                                /*!< cache mémoire des mdp par mode (cf. motDePasseSQL) */
    QString                       m_login;                                   /*!< = LOGIN_SQL (compte SQL technique) */
    QString                       m_password;                                /*!< mot de passe aléatoire d'adminrufus */
    QString                       m_createUserErr;                           /*!< sortie SQL du dernier createUser() échoué (diag) */
    QString                       m_secureFilePrivErr;                       /*!< diagnostic du dernier ensureSecureFilePriv() échoué */
    QString                       m_loginRufus;                              /*!< login du futur utilisateur Rufus (saisi) */
    QString                       m_mdpRufus;                                /*!< son mot de passe EN CLAIR (saisi) */
    QString                       m_brewPrefix;                              /*!< préfixe Homebrew (cache) */
    MySQLInstallerDialog*         m_dialog = nullptr;                        /*!< la fiche de l'installeur */
    bool                          m_freshInstall = false;                    /*!< MySQL vient d'être installé */
    bool                          m_baseRufusTrouvee = false;                /*!< une base Rufus était présente sur le serveur effacé */
    bool                          m_serveurInjoignable = false;              /*!< dernier tryConnectAs : le serveur n'a pas répondu (ERROR 2002/2003/2005) */
    bool                          m_comptesDejaCrees = false;                /*!< adminrufus déjà créé (mode Verify) */
    QString                       m_initLog = "/tmp/rufus_mysql_init.log";   /*!< journal d'init du datadir (macOS) */
    MySQLRemoteConfig             m_remoteConfig;                            /*!< config distante (chargée une seule fois) */
    bool                          m_remoteConfigLoaded = false;

    /*! ── Phases de run() ─────────────────────────────────────────────────────────────────────────────── */
    bool faireCreate(const MySQLRemoteConfig& cfg);             /*!< chemin création : installe MySQL + crée adminrufus + config */
    bool reinstallerSocleMySQL(const MySQLRemoteConfig& cfg);   /*!< section install de faireCreate, sans saisie (migration) */

    bool          isMariaDB();                                                                         /*!< le serveur local est-il MariaDB ? (incompatible) */
    bool          offrirSauvegardeAvantEffacement();                                                   /*!< prévient que les données du serveur vont disparaître ; false = renoncer */
    QStringList   AskMdpLoginMySQL();                                                                  /*!< { mdp, login } d'un compte admin MySQL éprouvé, vide si l'utilisateur n'en a pas */
    static bool   isBaseRufus(const QStringList& log);                                                 /*!< une base Rufus est-elle présente sur ce serveur ? */
    bool          offrirSauvegardeBaseRufus(const QStringList& log);                                   /*!< base Rufus trouvée : sauvegarder / effacer / renoncer */
    bool          faireReutiliser(const MySQLRemoteConfig& cfg, bool effacerTout);                     /*!< réutilise un MySQL existant : compte admin -> comptes Rufus -> config */
    void          effacerToutesBasesUtilisateur(const QString& adminLogin, const QString& adminMdp);   /*!< supprime toutes les bases non système */

    bool executerEtapesConfig();   /*!< déroule les étapes de config + coche la checklist ; true si tout OK */
    void cleanupDialog();          /*!< ferme et détruit m_dialog */

    /*! ── Multi-plateforme ────────────────────────────────────────────────────────────────────────────── */
    QString sharedFolderPath();   /*!< dossier partagé (/Users/Shared, C:/Users/Public…) */

    /*! ── Pré-requis (droits admin) ───────────────────────────────────────────────────────────────────── */
    bool isAdminUser();          /*!< processus élevé / compte administrateur ? */
    bool assurerDroitsAdmin();   /*!< garantit les droits admin (élévation UAC sinon) ; À N'APPELER qu'avant une (dés)installation */
#if defined(Q_OS_WIN)
    bool isVCRedist2022Installed();                               /*!< Windows : VC++ Redistributable 2022 présent ? */
    bool installVCRedist2022();                                   /*!< Windows : installe VC++ 2022 */
    bool relancerEnAdministrateur();                              /*!< Windows : relance Rufus élevé via l'invite UAC (« runas ») */
    void registerWindowsUninstaller(const QString& base,
                                       const QString& progData,
                                       const QString& version);   /*!< Windows : déclare MySQL dans « Applications » + script de désinstall */
#endif
#if defined(Q_OS_LINUX)
    bool isUbuntuVersionSupported();   /*!< Ubuntu > 22.04 ? */
#endif

    /*! ── MySQL (install / version / démarrage) ───────────────────────────────────────────────────────── */
    bool    isMySQLInstalled();                                      /*!< MySQL installé (binaire/service) ? */
    bool    ensureMysqlInPath();                                     /*!< mysql présent dans le PATH (l'ajoute sinon) */
    QString getMySQLServerVersion();                                 /*!< version du SERVEUR (mysqld), pas du client */
    QString serverVersionString();                                   /*!< VERSION() brute du serveur EN COURS ("" si injoignable) */
    bool    installMySQL();                                          /*!< installe MySQL (3 plateformes) */
    bool    uninstallMySQL();                                        /*!< désinstalle MySQL + config Rufus */
    bool    installFromDmg(const QString& dmgPath);                  /*!< macOS : installe depuis le .dmg Oracle */
    QString downloadOracleDmg();                                     /*!< macOS : télécharge le .dmg Oracle */
    bool    initOracleDataDir();                                     /*!< macOS : initialise le datadir Oracle si le pkg ne l'a pas fait */
    QString oracleInitStartScript() const;                           /*!< fragment shell root idempotent : init datadir Oracle + démarre le serveur */
    bool    downloadFile(const QString& url, const QString& dest,
                         const QString& label);                      /*!< télécharge url -> dest avec barre de progression */
    bool    hasNetworkAccess();                                      /*!< accès réseau (WAN) présent ? (silencieux) */
    bool    checkDownloadConnectivity(const QString& downloadUrl);   /*!< pré-requis réseau avant téléchargement (message explicite si KO) */
    void    avertirTelechargementImpossible();                       /*!< invite à installer MySQL soi-même puis relancer si le téléchargement échoue */
    bool    startMySQL();                                            /*!< démarre le serveur */
    bool    waitForMySQL(int maxSeconds = 30);                       /*!< attend que le serveur réponde */

    /*! ── Identifiants / connexion ────────────────────────────────────────────────────────────────────── */
    bool isServerRunning();                                        /*!< le serveur répond-il ? */
    bool tryConnect();                                             /*!< connexion adminrufus (mdp courant) */
    bool tryConnectAs(const QString& login, const QString& mdp);   /*!< connexion avec login/mdp arbitraires (compte admin saisi) */
    bool checkPrivileges(QStringList& outMissing);                 /*!< adminrufus a-t-il tous les privilèges requis ? (manquants -> outMissing) */
    bool createUser();                                             /*!< crée adminrufus/SSL (mode création) */

    /*! Crée adminrufus/SSL via le compte admin MySQL fourni ; distingue le manque de CREATE USER des autres erreurs. */
    CreateUserResult createUserAvecAdmin(const QString& adminLogin,
                                         const QString& adminMdp);

    /*! ── Dossier partagé (/Users/Shared) ─────────────────────────────────────────────────────────────── */
    bool setupSharedFolder();      /*!< dossier partagé existant + partagé (crée/partage sinon) */
    bool ensureSecureFilePriv();   /*!< secure_file_priv = dossier partagé (my.cnf) */
    bool testSharedFolderRW();     /*!< mysql lit ET écrit un fichier test dans le dossier partagé */
    bool dossierImagerieOuvertATous();  /*!< dossier d'imagerie inscriptible par les autres comptes (postes du réseau, mysql) */
    bool partageImageriePresent();      /*!< dossier d'imagerie visible des postes du réseau */
#if defined(Q_OS_WIN)
    QString windowsPartageImagerieScript() const;   /*!< fragment PowerShell créant le partage du dossier d'imagerie */
#endif
#if defined(Q_OS_LINUX)
    bool    prepareCreateModeLinux();                                                 /*!< Linux : tout le paramétrage root du mode Create en UNE élévation */
    QString linuxFolderSambaScript(const QString& path, const QString& user) const;   /*!< fragment shell dossier + Samba (réutilisé) */
    QString linuxForceMysqlCnfScript() const;                                         /*!< fragment shell remettant l'aiguillage my.cnf sur mysql-server */
#endif
#if defined(Q_OS_MACOS)
    bool prepareCreateModeMacOS();   /*!< macOS : tout le paramétrage root du mode Create en UNE invite admin */
#endif
    QList<QPair<QString, QString>> rufusCnfVars();                                               /*!< variables [mysqld] requises par Rufus (source unique) */
    QString                        getCnfVar(const QString& key);                                /*!< lit une variable de my.cnf */
    QString                        writeCnfToTemp(const QList<QPair<QString, QString>>& vars);   /*!< prépare un my.cnf temporaire (clés=valeurs) sans élévation */
    QString                        getCnfPath();                                                 /*!< chemin de my.cnf */
    void                           restartMySQL();                                               /*!< redémarre le serveur */

    /*! ── Config distante ─────────────────────────────────────────────────────────────────────────────── */
    static MySQLRemoteConfig defaultMySQLConfig();   /*!< config MySQL par défaut (repli) */
    MySQLRemoteConfig        fetchRemoteConfig();    /*!< config JSON distante -> repli sur défaut */

    /*! ── Helpers d'exécution ─────────────────────────────────────────────────────────────────────────── */
    bool        askYesNo(const QString& title, const QString& text);     /*!< question Oui/Non (UpMessageBox::Question) */
    QString     runCmd(const QString& cmd, int timeoutMs = 30000);       /*!< exécute une commande, renvoie sa sortie */
    QString     runCmdFull(const QString& cmd, int timeoutMs = 30000);   /*!< comme runCmd, sortie complète (stdout + stderr) */
    static QString argsServeurCourant();                                 /*!< arguments du client mysql vers le serveur du mode courant */
    QStringList lignesResultat(const QString& sortie);                   /*!< lignes de DONNÉES d'une sortie mysql (sans « [Warning] … password … ») */

    static void supprimerCompteMySQL(const QString& login);   /*!< supprime un compte MySQL sur TOUS ses hosts (erreur SQL muette : l'appelant constate) */

    /*! Exécute avec droits admin. stdinData (Linux) :
     *  passé sur l'entrée standard du processus élevé (ex. mot de passe smbpasswd)
     *  — jamais sur disque, ni en argument, ni dans les logs. */
    bool    runCmdElevated(const QString& cmd, const QString& stdinData = {});
    void    runLongOp(const QString& cmd, const QString& label,
                      int timeoutMs = 360000);           /*!< commande longue avec fiche d'attente */
    void    runLongOpProgress(const QString& cmd, const QString& label,
                              int timeoutMs = 360000);   /*!< commande longue avec barre de % (la commande émet « PROGRESS f t ») */
    QString getBrewPrefix();                             /*!< préfixe Homebrew (macOS) */
    QString mysqlBin(const QString& binary);             /*!< chemin d'un binaire MySQL (mysql, mysqld…) */
    bool    isOracleInstall();                           /*!< installation Oracle (vs Homebrew) ? */
    QString oraclePrefix() const;                        /*!< préfixe d'une install Oracle ("" sinon) */
};

#endif // MYSQLINSTALLER_H
