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
#include <optional>
#include <functional>

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





/*! MySQLInstallerDialog — la fiche de l'installeur (dérive d'UpDialog). Purement informative : titre,
    sous-titre et checklist des 7 critères, cochée en direct par MySQLInstaller (checkStep). Aucun bouton
    ni saisie — le moteur déroule ses étapes pendant qu'elle est affichée par show().
*/
class MySQLInstallerDialog : public UpDialog {
    Q_OBJECT
public:
    explicit MySQLInstallerDialog(QWidget* parent = nullptr);

    void passerEnConfiguration(const QString& titre, const QString& sousTitre);     /*!< titre et sous-titre de l'étape en cours */

    /*! ── Checklist ──────────────────────────────────────────────────────────────────────────────────── */
    void checkStep(int i);                                                          /*!< coche la case i + repaint */
    void setMinVersion(const QString& v);                                           /*!< re-libelle l'étape 0 */

private:
    QString baseStepLabel(int i) const;                                             /*!< libellé de base de la case i */
    void    applyStepLabel(int i);                                                  /*!< (ré)applique le libellé i (base + detail) */

    UpLabel*       m_title         = nullptr;
    UpLabel*       m_subtitle      = nullptr;
    UpCheckBox*    m_steps[7];                                                      /*!< 0..5 = config ; 6 = clés SSL (accès distant) */
    QString        m_minVersion = VERSION_MYSQL_MINI;                               /*!< seuil commun (8.0.14), écrasé par setMinVersion() */
};





/*! MySQLInstaller — moteur d'installation, de vérification et de sécurisation du serveur MySQL de Rufus.
    Garantit au démarrage qu'un serveur MySQL conforme est prêt (installé, configuré, sécurisé), et fournit
    l'entretien qui suit (mot de passe, clés SSL). Tout est LOCAL : les données ne quittent jamais le cabinet.

    * POINT D'ENTRÉE — run(desinstaller, installer) (synchrone) : exécute le diagnostic posé par
      l'appelant, le paramétrage pour Rufus étant fait dans tous les cas.
        * desinstaller -> reinstallerSocleMySQLpourMigration() : purge puis serveur neuf.
        * installer    -> faireCreate() : installe MySQL, crée les comptes, configure.
        * ni l'un ni l'autre -> on garde le serveur en place et on le paramètre.
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
    explicit MySQLInstaller(QWidget* parent = nullptr);   /*!< parent des fiches et messages de l'installeur */

    bool run(bool desinstaller, bool installer);                                /*!< point d'entrée synchrone : exécute le diagnostic posé par l'appelant */
    QStringList FindMdpLoginMySQL(bool sansQuestion = false);                   /*!< { mdp, login } d'un compte admin MySQL éprouvé ; sansQuestion : l'appelant a déjà demandé */
    static bool isBaseRufus(const QStringList& log);                            /*!< une base Rufus est-elle présente sur ce serveur ? */

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
    static bool socleMySQLConforme();                                           /*!< socle MySQL conforme à Rufus (>= 8.0.14 et pas MariaDB) */
    static bool socleLocalConforme();                                           /*!< idem, mais relevé SANS connexion (serveur LOCAL) : pour les décisions qui précèdent la connexion */
    static bool serveurLocalPresent();                                          /*!< un serveur MySQL local est-il installé ? (avant toute connexion) */
    void        verifierEtReparerConfigMonoposte();                             /*!< monoposte : vérif silencieuse de la config, propose de réparer sinon */
    bool        reinstallerSocleMySQLpourMigration(const QStringList& log);     /*!< migration DESTRUCTIVE du socle (log = admin de l'ancien serveur, pour hériter de son imagerie) */
    QStringList SqlLog() {return m_isconnectlogvalid?
                                      QStringList()<<m_login<<m_password :
                                      QStringList();}                           /*!< log de connexion éprouvé (adminrufus / adminrufusSSL) ; vide si pas de connexion */

    /*! ── Clés SSL (accès distant chiffré) ────────────────────────────────────────────────────────────── */
    bool        sauvegarderClesSSLMigration();                                  /*!< copie les .pem du datadir vers un stash (avant désinstall) */
    void        restaurerClesSSLMigration();                                    /*!< réinjecte les .pem dans le nouveau datadir + redémarre */
    static bool      clesSSLServeurPresentes();                                 /*!< le serveur a-t-il des certificats en service ? */
    static QDateTime dateExpirationCertSSL();                                   /*!< date d'expiration du certificat serveur (invalide si SSL inactif) */
    bool        exporterClesClientSSL(const QString& dest);                     /*!< copie les clés client du datadir vers dest */
    bool        corrigerDroitsClesSSL(const QString& dossier);                  /*!< rend les clés d'un dossier à l'utilisateur courant */
    bool        regenererClesSSL();                                             /*!< DESTRUCTIF : régénère les certs serveur (invalide les clés déjà distribuées) */
    void        controlerClesSSLMonoposte();                                    /*!< monoposte au démarrage : réextrait ou régénère selon l'état des clés */
    void        avertirExpirationClesSSLDistant();                              /*!< accès distant : prévient si les certs SSL approchent de l'expiration */

    /*! ── Entretien au démarrage (après connexion) ────────────────────────────────────────────────────── */
    static void      entretienApresConnexion(QWidget *parent = nullptr);            /*!< sécurise au besoin puis efface gaxt78iy à échéance */
    static bool      adminrufusEstSecurise();                                       /*!< adminrufus a-t-il un 2e mot de passe (base déjà sécurisée) ? */
    static bool      unCompteLANaPerduSystemUser();                                 /*!< un compte adminrufus local a-t-il perdu SYSTEM_USER (à régulariser) ? */
    static QDateTime dateSecurisation();                                            /*!< date de sécurisation (password_last_changed d'adminrufus) */
    static QString   posteSecurisation();                                           /*!< nom du poste ayant posé l'aléatoire (vide si version antérieure) */
    static void      supprimerGaxt78iySiEchue(QWidget *parent = nullptr);           /*!< efface gaxt78iy si la deadline est passée (et si ce poste a l'aléatoire) */
    void             avertirEffacementImminent();                                   /*!< informe que l'effacement de gaxt78iy approche (informatif) */
    void             proposerRecuperationAleatoire();                               /*!< base sécurisée mais poste sur gaxt78iy : propose de saisir l'aléatoire */
    void             suggererSecurisationDepuisLocal();                             /*!< poste distant sur base non sécurisée : invite à sécuriser depuis le LAN */
    bool             recreerMotDePasseApresVerifAdmin(QWidget *parent = nullptr);   /*!< recrée l'aléatoire quand plus aucun poste ne l'a (protégé par mdp Admin) */

    /*! ── Compte de secours (et suppression de root) ──────────────────────────────────────────────────── */
    static std::optional<bool> rootExiste();                                    /*!< un compte 'root' subsiste-t-il ? nullopt si la requête échoue */
    static bool compteDeSecoursExiste();                                        /*!< secoursrufus est-il en place sur TOUS les hosts LAN ? */
    static bool creerCompteDeSecours(QWidget* parent = nullptr);                /*!< demande le mdp confidentiel, crée secoursrufus sur les hosts LAN, l'éprouve puis supprime root */
    static void controlerCompteDeSecours(QWidget *parent = nullptr);            /*!< à chaque mise à jour de la base : met en place le secours s'il manque, et supprime root */
    bool        restaurerAvecMotDePasseDeSecours(QWidget* parent = nullptr);    /*!< dernier recours : le mdp de secours réécrit un aléatoire neuf sur adminrufus (+ .dbkey) */

    /*! ── Récupération du mot de passe du cabinet ─────────────────────────────────────────────────────── */
    enum class IssueMdp {
        Obtenu,         /*!< mot de passe récupéré et éprouvé */
        Echec,          /*!< pas de mdp récupéré */
        Secours,        /*!< tenter la récupération par le mot de passe de secours */
        idMySQL,        /*!< rconnexion récupérée à partir d'un idMySQL*/
    };

    /*! Insiste jusqu'à rétablir la connexion ou renoncer ; rend le dernier mode tenté et, par ok, s'il a
     * abouti (cf. initialisation Rufus.txt § II.2). */
    static IssueMdp RecupererMotDePasseMySQL(QWidget *parent, bool &ok, const QString &titre = QString(),
                                             const QString &corps = QString());

    enum class CreateUserResult { Ok, NoCreateUserRight, Error };           /*!< résultat de createUserAvecAdmin() */

private:
    static QHash<QString,QString> s_cacheMDP;                                /*!< cache mémoire des mdp par mode (cf. motDePasseSQL) */
    QString                       m_login;                                   /*!< = LOGIN_SQL (compte SQL technique) */
    QString                       m_password;                                /*!< mot de passe aléatoire d'adminrufus */
    bool                          m_isconnectlogvalid = false;               /*!< true si m_login/m_password représsentent un user MySQL possédant tous les droits valide */
    QString                       m_createUserErr;                           /*!< sortie SQL du dernier createUser() échoué (diag) */
    QString                       m_secureFilePrivErr;                       /*!< diagnostic du dernier ensureSecureFilePriv() échoué */
    QString                       m_brewPrefix;                              /*!< préfixe Homebrew (cache) */
    QWidget*                      m_parent = nullptr;                        /*!< fiche appelante : parent de toutes les fenêtres de l'installeur */
    MySQLInstallerDialog*         m_dialog = nullptr;                        /*!< la fiche de l'installeur */
    bool                          m_freshInstall = false;                    /*!< MySQL vient d'être installé */
    bool                          m_serveurInjoignable = false;              /*!< dernier tryConnectAs : le serveur n'a pas répondu (ERROR 2002/2003/2005) */
    bool                          m_comptesDejaCrees = false;                /*!< adminrufus déjà créé (mode Verify) */
    QString                       m_initLog = "/tmp/rufus_mysql_init.log";   /*!< journal d'init du datadir (macOS) */
    MySQLRemoteConfig             m_remoteConfig;                            /*!< config distante (chargée une seule fois) */
    bool                          m_remoteConfigLoaded = false;
    QString                       m_dossierPartageForce;                     /*!< dossier d'imagerie hérité d'une ancienne base (sinon défaut) */
    QString                       m_ancienDossierImagerie;                   /*!< dossier source connu à recopier (héritage impossible) */
    bool                          m_avertirImagerieAMigrer = false;          /*!< prévenir l'utilisateur de recopier lui-même son imagerie */

    /*! ── Phases de run() ─────────────────────────────────────────────────────────────────────────────── */
    bool terminerRun(bool ok);                                              /*!< sortie commune de run() : aléatoire à noter + compte de secours */
    /*! Joue le SQL en mysql_native_password, le rejoue avec le plugin par défaut si le serveur ne le
     * charge pas (MySQL 8.4). Renvoie la sortie du client mysql. */
    static QString executerAvecRepliPlugin(const std::function<QString(const QString&)>& sqlAvecAuth,
                                           const std::function<QString(const QString&)>& executer);

    bool faireCreate(const MySQLRemoteConfig& cfg);                         /*!< chemin création : installe MySQL + crée adminrufus + config */
    bool reinstallerSocleMySQL(const MySQLRemoteConfig& cfg);               /*!< section install de faireCreate, sans saisie (migration) */
    void avertirPostesAnciensAMettreAJour();                                /*!< serveur neuf : prévient qu'un ancien poste devra être mis à jour */

    /*! ── Imagerie d'une ancienne base : héritage du dossier ou avertissement ─────────────────────────── */
    QString lireSecureFilePriv(const QStringList& log);                     /*!< secure_file_priv de l'ancien serveur (vide si NULL/illisible) */
    void    preserverDossierImagerieAncienneBase(const QStringList& log);   /*!< avant la purge : hérite du dossier d'imagerie ou mémorise qu'il faut avertir */
    bool    configurerEtapesDossierPartage(bool silencieux);                /*!< étapes 3-5 : dossier partagé + secure_file_priv + test R/W */
    void    avertirImagerieAMigrer();                                       /*!< invite l'utilisateur à recopier lui-même son imagerie */

    bool          isMariaDB();                                              /*!< le serveur local est-il MariaDB ? (incompatible) */
    void          effacerToutesBasesUtilisateur(const QString& adminLogin, const QString& adminMdp);  /*!< supprime toutes les bases non système */

    bool executerEtapesConfig();                                            /*!< déroule les étapes de config + coche la checklist ; true si tout OK */
    void cleanupDialog();                                                   /*!< ferme et détruit m_dialog */

    /*! ── Multi-plateforme ────────────────────────────────────────────────────────────────────────────── */
    QString sharedFolderPath();                                             /*!< dossier partagé (/Users/Shared, C:/Users/Public…) */

    /*! ── Pré-requis (droits admin) ───────────────────────────────────────────────────────────────────── */
    bool isAdminUser();                                                     /*!< processus élevé / compte administrateur ? */
    bool assurerDroitsAdmin();                                              /*!< garantit les droits admin (élévation UAC sinon) ; À N'APPELER qu'avant une (dés)installation */
#if defined(Q_OS_WIN)
    bool isVCRedist2022Installed();                                         /*!< Windows : VC++ Redistributable 2022 présent ? */
    bool installVCRedist2022();                                             /*!< Windows : installe VC++ 2022 */
    bool relancerEnAdministrateur();                                        /*!< Windows : relance Rufus élevé via l'invite UAC (« runas ») */
    void registerWindowsUninstaller(const QString& base,
                                       const QString& progData,
                                       const QString& version);             /*!< Windows : déclare MySQL dans « Applications » + script de désinstall */
#endif
#if defined(Q_OS_LINUX)
    bool isUbuntuVersionSupported();   /*!< Ubuntu > 22.04 ? */
#endif

    /*! ── MySQL (install / version / démarrage) ───────────────────────────────────────────────────────── */
    bool    isMySQLInstalled();                                             /*!< MySQL installé (binaire/service) ? */
    bool    ensureMysqlInPath();                                            /*!< mysql présent dans le PATH (l'ajoute sinon) */
    QString getMySQLServerVersion();                                        /*!< version du SERVEUR (mysqld), pas du client */
    QString serverVersionString();                                          /*!< VERSION() brute du serveur EN COURS ("" si injoignable) */
    bool    installMySQL();                                                 /*!< installe MySQL (3 plateformes) */
    bool    uninstallMySQL();                                               /*!< désinstalle MySQL + config Rufus */
    bool    installFromDmg(const QString& dmgPath);                         /*!< macOS : installe depuis le .dmg Oracle */
    QString downloadOracleDmg();                                            /*!< macOS : télécharge le .dmg Oracle */
    bool    initOracleDataDir();                                            /*!< macOS : initialise le datadir Oracle si le pkg ne l'a pas fait */
    QString oracleInitStartScript() const;                                  /*!< fragment shell root idempotent : init datadir Oracle + démarre le serveur */
    bool    downloadFile(const QString& url, const QString& dest,
                         const QString& label);                             /*!< télécharge url -> dest avec barre de progression */
    bool    hasNetworkAccess();                                             /*!< accès réseau (WAN) présent ? (silencieux) */
    bool    checkDownloadConnectivity(const QString& downloadUrl);          /*!< pré-requis réseau avant téléchargement (message explicite si KO) */
    void    avertirTelechargementImpossible();                              /*!< invite à installer MySQL soi-même puis relancer si le téléchargement échoue */
    bool    startMySQL();                                                   /*!< démarre le serveur */
    bool    waitForMySQL(int maxSeconds = 30);                              /*!< attend que le serveur réponde */

    /*! ── Identifiants / connexion ────────────────────────────────────────────────────────────────────── */
    bool isServerRunning();                                                 /*!< le serveur répond-il ? */
    bool tryConnect();                                                      /*!< connexion adminrufus (mdp courant) */
    bool tryConnectAs(const QString& login, const QString& mdp);            /*!< connexion avec login/mdp arbitraires (compte admin saisi) */
    bool checkPrivileges(QStringList& outMissing);                          /*!< adminrufus a-t-il tous les privilèges requis ? (manquants -> outMissing) */
    bool createUser();                                                      /*!< crée adminrufus/SSL (mode création) */

    /*! Crée adminrufus/SSL via le compte admin MySQL fourni ; distingue le manque de CREATE USER des autres erreurs. */
    CreateUserResult createUserAvecAdmin(const QString& adminLogin,
                                         const QString& adminMdp);

    /*! ── Dossier partagé (/Users/Shared) ─────────────────────────────────────────────────────────────── */
    bool setupSharedFolder();                                               /*!< dossier partagé existant + partagé (crée/partage sinon) */
    bool ensureSecureFilePriv();                                            /*!< secure_file_priv = dossier partagé (my.cnf) */
    bool testSharedFolderRW();                                              /*!< mysql lit ET écrit un fichier test dans le dossier partagé */
    bool droitsDossierPartageConformes();                                   /*!< dossier partagé traversable par mysql, et jamais inscriptible par tous */
public:
    void    retirerEcriturePourTous();                                      /*!< retire en silence le w des « autres » sous le dossier partagé */
    QString mysqlBin(const QString& binary);                                /*!< chemin d'un binaire MySQL (mysql, mysqldump…), le nom nu s'il n'est qu'au PATH */
private:
    bool partageImageriePresent();                                          /*!< dossier d'imagerie visible des postes du réseau */
    bool partageImagerieNomPresent();                                       /*!< un partage de ce nom existe, quel que soit son dossier */
#if defined(Q_OS_WIN)
    QString windowsPartageImagerieScript(const QString& path) const;        /*!< fragment PowerShell créant le partage du dossier d'imagerie */
#endif
#if defined(Q_OS_LINUX)
    bool    prepareCreateModeLinux();                                                 /*!< Linux : tout le paramétrage root du mode Create en UNE élévation */
    QString linuxFolderSambaScript(const QString& path, const QString& user) const;   /*!< fragment shell dossier + Samba (réutilisé) */
    QString linuxForceMysqlCnfScript() const;                                         /*!< fragment shell remettant l'aiguillage my.cnf sur mysql-server */
#endif
#if defined(Q_OS_MACOS)
    bool prepareCreateModeMacOS();                                          /*!< macOS : tout le paramétrage root du mode Create en UNE invite admin */
#endif
    QList<QPair<QString, QString>> rufusCnfVars();                                               /*!< variables [mysqld] requises par Rufus (source unique) */
    QString                        getCnfVar(const QString& key);                                /*!< lit une variable de my.cnf */
    QString                        writeCnfToTemp(const QList<QPair<QString, QString>>& vars);   /*!< prépare un my.cnf temporaire (clés=valeurs) sans élévation */
    QString                        getCnfPath();                                                 /*!< chemin de my.cnf */
    void                           restartMySQL();                                               /*!< redémarre le serveur */

    /*! ── Config distante ─────────────────────────────────────────────────────────────────────────────── */
    static MySQLRemoteConfig defaultMySQLConfig();                          /*!< config MySQL par défaut (repli) */
    MySQLRemoteConfig        fetchRemoteConfig();                           /*!< config JSON distante -> repli sur défaut */

    /*! ── Helpers d'exécution ─────────────────────────────────────────────────────────────────────────── */
    bool        askYesNo(const QString& title, const QString& text);        /*!< question Oui/Non (UpMessageBox::Question) */
    QString     runCmd(const QString& cmd, int timeoutMs = 30000);          /*!< exécute une commande, renvoie sa sortie */
    QString     runCmdFull(const QString& cmd, int timeoutMs = 30000);      /*!< comme runCmd, sortie complète (stdout + stderr) */
    static QString argsServeurCourant();                                    /*!< arguments du client mysql vers le serveur du mode courant */
    QStringList lignesResultat(const QString& sortie);                      /*!< lignes de DONNÉES d'une sortie mysql (sans « [Warning] … password … ») */

    static void supprimerCompteMySQL(const QString& login);                 /*!< supprime un compte MySQL sur TOUS ses hosts (erreur SQL muette : l'appelant constate) */

    /*! Exécute avec droits admin. stdinData (Linux) :
     *  passé sur l'entrée standard du processus élevé (ex. mot de passe smbpasswd)
     *  — jamais sur disque, ni en argument, ni dans les logs. */
    bool    runCmdElevated(const QString& cmd, const QString& stdinData = {});
    void    runLongOp(const QString& cmd, const QString& label,
                      int timeoutMs = 360000);                              /*!< commande longue avec fiche d'attente */
    void    runLongOpProgress(const QString& cmd, const QString& label,
                              int timeoutMs = 360000);                      /*!< commande longue avec barre de % (la commande émet « PROGRESS f t ») */
    QString getBrewPrefix();                                                /*!< préfixe Homebrew (macOS) */
    bool    isOracleInstall();                                              /*!< installation Oracle (vs Homebrew) ? */
    QString oraclePrefix() const;                                           /*!< préfixe d'une install Oracle ("" sinon) */
};

#endif // MYSQLINSTALLER_H
