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

// ─────────────────────────────────────────────────────────────────────────────
//  Config MySQL distante (version cible, seuil minimal, URLs de téléchargement).
// ─────────────────────────────────────────────────────────────────────────────
struct MySQLRemoteConfig {
    QString version;       // version cible à installer / proposer en mise à jour
    QString minVersion;    // version minimale acceptée en mode Verify (seuil)
    QString winUrl;
    QString macArm64Url;
    QString macX86Url;
};

// ─────────────────────────────────────────────────────────────────────────────
//  Dialogue de progression interne (porté de l'app autonome ProgressDialog).
//  Barre animée (indéterminée) si total <= 0, sinon pourcentage.
// ─────────────────────────────────────────────────────────────────────────────
class MySQLProgressDialog : public QDialog {
    Q_OBJECT
public:
    explicit MySQLProgressDialog(const QString& operation, QWidget* parent = nullptr);

    //  Mode déterminé (pourcentage). total <= 0 => barre animée (indéterminée).
    void setProgress(qint64 received, qint64 total);

private:
    QLabel*       m_label;
    QProgressBar* m_progress;
};

// ─────────────────────────────────────────────────────────────────────────────
//  Dialogue de l'installeur : titre + sous-titre, saisie d'un identifiant et d'un
//  mot de passe (deux UpLineEdit), PUIS la checklist des 6 critères (affichage
//  seul, pilotée par l'engine via checkStep / uncheckAllSteps). Le même dialogue
//  est réutilisé :
//   • en mode modal (exec()) pour la SAISIE d'identifiants (utilisateur Rufus, ou
//     compte administrateur MySQL) ;
//   • affiché (show()) pendant que l'engine déroule les étapes.
//  Les méthodes configurer*() basculent le titre / sous-titre / libellé du bouton
//  OK selon le contexte et vident les champs.
// ─────────────────────────────────────────────────────────────────────────────
class MySQLInstallerDialog : public UpDialog {
    Q_OBJECT
public:
    explicit MySQLInstallerDialog(QWidget* parent = nullptr);

    //  Code de résultat de exec() quand l'utilisateur clique « Supprimer MySQL »
    //  (mode Verify). QDialog réserve Rejected = 0 et Accepted = 1.
    enum DialogResult { ResultSupprimerMySQL = 2 };

    // ── Configuration selon le contexte (titre / sous-titre / bouton OK + reset) ─
    //  Création de la base : saisie du futur utilisateur applicatif Rufus.
    void configurerCreateUserRufus(const QString& minVersion);
    //  Vérification : un MySQL existe déjà, saisie d'un compte ADMIN MySQL.
    void configurerVerifyAdminMySQL();
    //  Saisie du futur utilisateur applicatif Rufus (2e étape, après config).
    void configurerNewUserRufus();

    // ── Saisie ───────────────────────────────────────────────────────────────
    QString login() const;                          // texte courant du champ login
    QString password() const;                       // texte courant du champ mdp
    //  Vérifie que login ET mot de passe sont renseignés (le format est garanti par
    //  les validateurs). Affiche un UpMessageBox::Watch et renvoie false si vide.
    bool    validerSaisie();

    // ── Checklist ──────────────────────────────────────────────────────────────
    void checkStep(int i);                          // coche la case i + processEvents
    void uncheckAllSteps();                         // décoche tout
    void setStepDetail(int i, const QString& detail);   // ajoute « : detail » au libellé
    void setMinVersion(const QString& v);           // re-libelle l'étape 0
    bool wasCancelled() const { return m_cancelled; }

protected:
    void reject() override;                         // marque l'annulation

private:
    QString baseStepLabel(int i) const;
    void    applyStepLabel(int i);
    void    configurer(const QString& titre, const QString& sousTitre,
                       const QString& okLabel);     // applique + vide les champs

    UpLabel*       m_title         = nullptr;
    UpLabel*       m_subtitle      = nullptr;
    UpLineEdit*    m_login         = nullptr;
    UpLineEdit*    m_mdp           = nullptr;
    UpSmallButton* m_btnSupprMySQL = nullptr;   // « Supprimer MySQL » (mode Verify)
    UpCheckBox* m_steps[6];
    QString     m_stepDetail[6];
    QString     m_minVersion = "8.4.3";
    bool        m_cancelled  = false;
};

// ─────────────────────────────────────────────────────────────────────────────
//  Engine d'installation/vérification de MySQL pour Rufus (porté d'AppController).
//  run() est SYNCHRONE et renvoie true si un MySQL conforme à Rufus est prêt
//  (vérifié OU fraîchement installé+configuré), false si annulation/échec.
// ─────────────────────────────────────────────────────────────────────────────
class MySQLInstaller : public QObject {
    Q_OBJECT
public:
    explicit MySQLInstaller(QObject* parent = nullptr);

    //  Point d'entrée synchrone. true = MySQL conforme prêt ; false = annulation
    //  ou échec.
    bool    run();

    //  Identifiant et mot de passe (EN CLAIR) du futur utilisateur APPLICATIF
    //  Rufus, saisis dans le dialogue. L'appelant les écrit dans rufus.utilisateurs
    //  (en hachant le mot de passe en SHA1). Vides si run() a échoué/annulé.
    QString loginRufus() const { return m_loginRufus; }
    QString mdpRufus()   const { return m_mdpRufus; }

    // ── Mot de passe aléatoire (helpers statiques) ─────────────────────────────
    //  Génère un mot de passe alphanumérique fort (12 caractères, [A-Za-z0-9]).
    static QString genererMotDePasse();
    //  Lit la clé Param_MDPSQL du rufus.ini ; repli sur MDP_SQL si absente/vide.
    static QString motDePasseSQL();
    //  Stocke le mot de passe (clé Param_MDPSQL du rufus.ini).
    static void    stockerMotDePasse(const QString& mdp);

    // Résultat de createUserAvecAdmin().
    enum class CreateUserResult { Ok, NoCreateUserRight, Error };

private:
    QString             m_login;                 // = LOGIN_SQL (compte SQL technique)
    QString             m_password;              // mot de passe aléatoire d'adminrufus
    QString             m_loginRufus;            // login du futur utilisateur Rufus (saisi)
    QString             m_mdpRufus;              // son mot de passe EN CLAIR (saisi)
    QString             m_brewPrefix;
    MySQLInstallerDialog* m_dialog = nullptr;
    bool                m_freshInstall = false;  // true = MySQL vient d'être installé
    bool                m_comptesDejaCrees = false; // adminrufus déjà créé (mode VERIFY)
    // macOS : journal de mysqld --initialize (diagnostic si l'init du datadir échoue).
    QString             m_initLog = "/tmp/rufus_mysql_init.log";
    MySQLRemoteConfig   m_remoteConfig;          // config distante (chargée une seule fois)
    bool                m_remoteConfigLoaded = false;

    // ── Phases de run() ────────────────────────────────────────────────────────
    //  Chemin « création » : MySQL absent (ou trop vieux, après nettoyage). Installe
    //  MySQL, crée adminrufus (root/pkexec) et déroule les étapes de config. Renvoie
    //  true si la base Rufus est prête. La saisie du futur utilisateur Rufus a déjà
    //  été faite par run() avant l'appel.
    bool    faireCreate(const MySQLRemoteConfig& cfg);
    //  Étapes de configuration post-install/verify (PATH, dossier partagé,
    //  secure_file_priv, lecture/écriture, privilèges), avec mise à jour de la
    //  checklist. createUser() n'est invoqué que si m_freshInstall && !m_comptesDejaCrees.
    bool    executerEtapesConfig();      // true si toutes les étapes OK
    void    cleanupDialog();             // ferme et détruit m_dialog

    // ── Multi-plateforme ───────────────────────────────────────────────────────
    QString sharedFolderPath();          // /Users/Shared (macOS) | C:/Users/Public (Windows)

    // ── Pré-requis ─────────────────────────────────────────────────────────────
    bool    isAdminUser();               // compte administrateur / processus élevé ?
#if defined(Q_OS_WIN)
    bool    isVCRedist2022Installed();   // Windows : Visual C++ Redistributable 2022
    bool    installVCRedist2022();
    //  Relance Rufus en tant qu'administrateur via l'invite UAC (verbe « runas ») :
    //  l'utilisateur peut saisir un compte administrateur à cet instant, sans avoir
    //  à relancer manuellement « Exécuter en tant qu'administrateur ». true si
    //  l'instance élevée a été lancée (UAC acceptée), false si refus/annulation.
    bool    relancerEnAdministrateur();
    //  Déclare MySQL dans « Applications et fonctionnalités » + script de désinstall.
    void    registerWindowsUninstaller(const QString& base,
                                       const QString& progData,
                                       const QString& version);
#endif
#if defined(Q_OS_LINUX)
    bool    isUbuntuVersionSupported();  // Ubuntu > 22.04 ?
#endif

    // ── MySQL ──────────────────────────────────────────────────────────────────
    bool    isMySQLInstalled();
    bool    ensureMysqlInPath();         // chemin de mysql présent dans PATH (sinon l'ajoute)
    QString getMySQLVersion();
    bool    installMySQL();
    bool    uninstallMySQL();        // désinstalle MySQL + config Rufus (3 plateformes)
    bool    installFromDmg(const QString& dmgPath);
    QString downloadOracleDmg();
    //  macOS : initialise /usr/local/mysql/data si le pkg Oracle ne l'a pas fait
    //  (cas MySQL 8.4.x). root@localhost sans mot de passe. No-op si déjà fait.
    bool    initOracleDataDir();
    //  Fragment shell (root) idempotent : initialise le datadir Oracle s'il ne
    //  l'est pas (mysqld --initialize-insecure, TMPDIR=/tmp) puis démarre le
    //  serveur (LaunchDaemon, sinon mysql.server). Sortie ajoutée à m_initLog.
    //  Partagé par installFromDmg() (fusionné à l'installateur → 1 seule invite)
    //  et initOracleDataDir() (réparation/démarrage autonome).
    QString oracleInitStartScript() const;
    //  Télécharge url -> dest avec barre de progression (vrai pourcentage).
    bool    downloadFile(const QString& url, const QString& dest,
                         const QString& label);
    //  Accès réseau (WAN) présent ? Connexion TCP vers une IP publique, sans
    //  message (l'appelant décide quoi afficher). Contrôlé au lancement.
    bool    hasNetworkAccess();
    //  Pré-requis réseau avant un téléchargement d'installation. Affiche un
    //  message d'erreur explicite et renvoie false si : aucun accès réseau (WAN),
    //  ou WAN présent mais le lien de téléchargement ne se résout pas (DNS).
    bool    checkDownloadConnectivity(const QString& downloadUrl);
    void    stopMySQL();
    bool    startMySQL();
    bool    waitForMySQL(int maxSeconds = 30);

    // ── Identifiants ───────────────────────────────────────────────────────────
    bool    isServerRunning();
    bool    tryConnect();
    //  Comme tryConnect() mais avec des identifiants arbitraires (compte admin saisi).
    bool    tryConnectAs(const QString& login, const QString& mdp);
    //  true ssi adminrufus se connecte avec motDePasseSQL() ET le schéma Rufus
    //  (base DB_RUFUS avec au moins une table) existe → base Rufus complète.
    bool    baseRufusComplete();
    bool    checkPrivileges(QStringList& outMissing);
    bool    createUser();
    //  Crée adminrufus/adminrufusSSL en se connectant avec le compte admin MySQL
    //  fourni (login/mdp). Distingue le manque du droit CREATE USER des autres
    //  erreurs.
    CreateUserResult createUserAvecAdmin(const QString& adminLogin,
                                         const QString& adminMdp);

    // ── Dossier partagé /Users/Shared ──────────────────────────────────────────
    bool    setupSharedFolder();         // existe + partagé (crée/partage sinon)
    bool    ensureSecureFilePriv();      // secure_file_priv = /Users/Shared (my.cnf)
    bool    testSharedFolderRW();        // mysql lit ET écrit un fichier test
    bool    guideMysqldFullDiskAccess(); // guide l'octroi du FDA à mysqld (ré-essai)
#if defined(Q_OS_LINUX)
    //  Regroupe TOUT le paramétrage root du mode Create en une seule élévation
    //  (utilisateur MySQL + dossier/AppArmor/ufw/Samba/wsdd + my.cnf + restart),
    //  pour ne demander le mot de passe système qu'une fois. Best-effort : les
    //  étapes individuelles (qui vérifient l'état) reprennent ce qui manquerait.
    bool    prepareCreateModeLinux();
    //  Fragment shell du paramétrage dossier+Samba (utilise $PW pour smbpasswd) ;
    //  réutilisé par setupSharedFolder() et prepareCreateModeLinux().
    QString linuxFolderSambaScript(const QString& path, const QString& user) const;
#endif
#if defined(Q_OS_MACOS)
    //  Regroupe TOUT le paramétrage root du mode Create en une seule invite admin
    //  (my.cnf + /etc/paths.d + dossier/partage SMB + redémarrage du serveur), pour
    //  ne demander le mot de passe administrateur qu'une fois. La création de
    //  l'utilisateur reste séparée (mysql -u root n'exige pas root sur macOS).
    bool    prepareCreateModeMacOS();
#endif
    //  Variables [mysqld] requises par Rufus (secure_file_priv, sql_mode, +
    //  bind-address sous Linux). Source unique pour my.cnf.
    QList<QPair<QString, QString>> rufusCnfVars();
    QString getCnfVar(const QString& key);
    //  Prépare une copie de my.cnf avec une ou plusieurs paires clé=valeur
    //  (section [mysqld]) dans un fichier temporaire, SANS élévation. Renvoie le
    //  chemin du temp (ou vide). Permet d'écrire plusieurs variables en une fois
    //  (donc une seule élévation à la copie).
    QString writeCnfToTemp(const QList<QPair<QString, QString>>& vars);
    QString getCnfPath();
    void    restartMySQL();

    // ── Config distante ────────────────────────────────────────────────────────
    static MySQLRemoteConfig defaultMySQLConfig();
    MySQLRemoteConfig        fetchRemoteConfig();   // JSON distant → fallback sur défaut

    // ── Helpers ────────────────────────────────────────────────────────────────
    //  Question Oui/Non (UpMessageBox::Question, boutons « Non »/« Oui »).
    bool    askYesNo(const QString& title, const QString& text);
    //  Dialogue de mise à jour : prévient qu'une MAJ est nécessaire, conseille de
    //  sauvegarder les données. Renvoie true si l'utilisateur confirme.
    bool    askUpdateConfirmation(const QString& currentVer, const QString& targetVer);
    QString runCmd(const QString& cmd, int timeoutMs = 30000);
    QString runCmdFull(const QString& cmd, int timeoutMs = 30000);
    //  Exécution avec droits admin. stdinData (Linux uniquement) : données
    //  fournies sur l'entrée standard du processus élevé (ex. mot de passe pour
    //  smbpasswd) — jamais sur disque, ni en argument, ni dans les logs.
    bool    runCmdElevated(const QString& cmd, const QString& stdinData = {});
    void    runLongOp(const QString& cmd, const QString& label,
                      int timeoutMs = 360000);
    //  Comme runLongOp mais avec barre de % : la commande émet « PROGRESS f t ».
    void    runLongOpProgress(const QString& cmd, const QString& label,
                              int timeoutMs = 360000);
    QString getBrewPrefix();
    QString mysqlBin(const QString& binary);
    bool    isOracleInstall();
    QString oraclePrefix() const;
};

#endif // MYSQLINSTALLER_H
