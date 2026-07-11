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
    void setProgress(qint64 received, qint64 total);     //  octets → détail « X / Y Mo »
    void setProgressBar(qint64 done, qint64 total);      //  fraction → barre SEULE (pas de volume)

private:
    QLabel*       m_label;
    QProgressBar* m_progress;
    QLabel*       m_detail;       //  taille téléchargée « X / Y Mo » (gros DMG macOS ~550 Mo)
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
    //  Bascule la fiche en mode « paramétrage en cours » : titre dédié, champs
    //  login/mdp désactivés (déjà choisis) et boutons OK/Annuler masqués — la
    //  séquence de configuration se déroule alors sans aucun clic à faire.
    void passerEnConfiguration(const QString& titre, const QString& sousTitre);
    //  Cache la ligne de saisie identifiant/mot de passe (labels + champs) : utilisé pour
    //  la RÉINSTALLATION lors d'une migration, où aucun nouvel utilisateur n'est saisi.
    void masquerSaisieUtilisateur();

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
    UpLabel*       m_loginLbl      = nullptr;
    UpLabel*       m_mdpLbl        = nullptr;
    UpLineEdit*    m_login         = nullptr;
    UpLineEdit*    m_mdp           = nullptr;
    UpLabel*       m_mdpConfirmLbl = nullptr;   // confirmation du mot de passe (création de compte)
    UpLineEdit*    m_mdpConfirm    = nullptr;
    bool           m_confirmMdpRequis = false;  // true en mode CRÉATION : validerSaisie() exige mdp == confirmation
    UpSmallButton* m_btnSupprMySQL = nullptr;   // « Supprimer MySQL » (mode Verify)
    UpCheckBox* m_steps[7];          // 0..5 = config ; 6 = clés SSL pour l'accès distant
    QString     m_stepDetail[7];
    QString     m_minVersion = VERSION_MYSQL_MINI;   // seuil commun (8.0.14), écrasé par setMinVersion()
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
    //  Mot de passe MySQL que CE poste connaît pour le MODE de connexion courant
    //  (DataBase::ModeAccesDataBase : MONO/LAN/WAN). Le .dbkey contient une ligne par
    //  mode (CLE=mdp) ; repli legacy gaxt78iy (MDP_SQL). Résolu une fois par mode puis
    //  gardé en cache mémoire. Fonction PURE : aucune connexion, aucune UI.
    static QString motDePasseSQL();
    //  Met à jour le mot de passe du mode courant EN MÉMOIRE (cache). À appeler quand
    //  la couche connexion a déterminé/récupéré le mdp qui fonctionne réellement.
    static void    setMotDePasseSQL(const QString& mdp);
    //  Liste ORDONNÉE et dédupliquée des mots de passe à essayer pour se connecter
    //  dans le mode courant : [ mdp .dbkey du mode, gaxt78iy ]. La cascade de
    //  connexion (et la récupération en dernier ressort) vit dans la couche connexion.
    static QStringList motsDePasseSQLCandidats();
    //  Connexion à `basename` via adminrufus en ESSAYANT TOUS les mots de passe candidats
    //  (aléatoire .dbkey PUIS gaxt78iy) et en mémorisant celui qui marche. Indispensable
    //  juste après la sécurisation : l'aléatoire fraîchement posé peut ne pas être accepté
    //  à la toute première connexion (cache d'auth caching_sha2 « froid »), alors que
    //  gaxt78iy — conservé comme 2e mot de passe — fonctionne. Renvoie "" si OK, sinon la
    //  dernière erreur. À utiliser pour TOUTE (re)connexion d'identification.
    static QString connecterAvecCandidats(const QString& basename);
    //  Vrai si l'erreur de connexion est un REFUS D'AUTHENTIFICATION (mauvais mot de passe :
    //  MySQL 1045 / « Access denied »), par opposition à un serveur injoignable (réseau).
    static bool    estErreurAuthentification(const QString& erreur);
    //  Stocke le mot de passe du MODE COURANT dans le .dbkey (les autres modes sont
    //  préservés) ET met à jour le cache.
    static void    stockerMotDePasse(const QString& mdp);
    //  Variantes par MODE EXPLICITE : pour configurer plusieurs bases (une par mode)
    //  depuis dlg_param, indépendamment du mode de connexion courant.
    static QString motDePasseStockePourMode(Utils::ModeAcces mode);                   // valeur brute stockée, "" si aucune
    static void    stockerMotDePassePourMode(Utils::ModeAcces mode, const QString& mdp);
    static void    supprimerMotDePassePourMode(Utils::ModeAcces mode);               // mode de connexion abandonné

    //  Sécurisation « à la volée » d'une base EXISTANTE. Si adminrufus n'a pas encore de
    //  2e mot de passe (base encore sur gaxt78iy seul), bascule adminrufus/adminrufusSSL
    //  sur un mot de passe aléatoire en CONSERVANT gaxt78iy comme 2e mot de passe
    //  (ALTER USER … RETAIN CURRENT PASSWORD), puis écrit .dbkey. Passe par la connexion
    //  Qt (DataBase::I()) : fonctionne en monoposte ET en réseau (un poste LAN peut
    //  sécuriser le serveur). No-op si : déjà sécurisée (2e mot de passe présent), ou
    //  serveur ne supportant pas le double mot de passe (MySQL < 8.0.14 / MariaDB).
    bool    securiserBaseSiNecessaire();   // true si la sécurisation vient d'être posée DANS cet appel
    //  Cœur commun : génère un aléatoire, le sauvegarde+relit AVANT de le poser sur tous les hosts
    //  d'adminrufus/adminrufusSSL, vérifie et affiche le mot de passe. Suppose local + socle conforme.
    //  Utilisé par la 1re sécurisation et le bouton « le mot de passe est égaré ».
    bool    poserEtSauvegarderAleatoire();
    //  OPTION B — restreint adminrufus (non-SSL) au réseau local (RFC 1918 + loopback) et supprime
    //  adminrufus@'%' (exposé au WAN), UNIQUEMENT après une connexion de test prouvant qu'une entrée LAN
    //  prend le relais (jamais de verrouillage). adminrufusSSL@'%' (distant, SSL) intact. `mdpAleatoire` =
    //  l'aléatoire éprouvé de la connexion en cours (posé sur les entrées créées). Silencieux, local.
    bool    restreindreAdminrufusAuLAN(const QString& mdpAleatoire);
    //  Rattrape une base « polluée » (1re sécurisation échouée : adminrufusSSL@'%' resté compte système
    //  NON sécurisé, adminrufus@'%' droppé → plus aucune connexion ordinaire ne peut le corriger) en
    //  ouvrant une 2e connexion LOCALE sous adminrufusSSL, qui, LUI, détient SYSTEM_USER. `aleatoire` =
    //  mot de passe à poser (gaxt78iy conservé en 2e mdp). Marche dans tous les cas ; seule exigence : que
    //  la liaison TLS locale s'ouvre (REQUIRE SSL). true si adminrufusSSL@'%' est bien sécurisé au final.
    bool    securiserComptesetMdpViaAdminrufusSSL(const QString& aleatoire);

    //  true si le serveur MySQL courant atteint le seuil commun (VERSION_MYSQL_MINI = 8.0.14)
    //  et n'est pas MariaDB. L'OS du serveur n'entre pas en jeu : seule compte la version MySQL.
    //  Sert au démarrage à décider d'une migration du socle (local) / d'une alerte (réseau).
    static bool      socleMySQLConforme();
    //  true si un SERVEUR MySQL local est présent (binaire/service installé), AVANT toute connexion.
    //  Sert au démarrage MONOPOSTE : pas de serveur → inutile de tenter une connexion, on propose
    //  directement une base vierge. (Enveloppe statique de isMySQLInstalled().)
    static bool      serveurLocalPresent();
    //  MONOPOSTE uniquement : à chaque démarrage, vérification SILENCIEUSE et bon marché de la config
    //  serveur (secure_file_priv, dossier partagé, client mysql dans le PATH). Si tout est conforme →
    //  rien. Sinon → propose (consentement) de RÉPARER en réutilisant executerEtapesConfig() sans
    //  réinstaller. La correction est réservée au monoposte : un client ne peut pas corriger le serveur.
    void      verifierEtReparerConfigMonoposte();
    //  Migration DESTRUCTIVE du socle : désinstalle l'ancien MySQL puis réinstalle + crée
    //  adminrufus. À N'APPELER QU'APRÈS une sauvegarde VALIDÉE (cf. Procedures). true si OK.
    bool             reinstallerSocleMySQLpourMigration();

    //  SSL (étape 7, point 4) — conservation des clés lors d'un remplacement de socle.
    //  sauvegarder… : copie les 6 .pem du datadir vers un stash, AVANT la désinstallation.
    //  restaurer…   : réinjecte les .pem dans le NOUVEAU datadir (même CA -> les postes
    //  distants existants restent valides), redémarre MySQL et réactualise la copie client.
    bool             sauvegarderClesSSLMigration();
    void             restaurerClesSSLMigration();

    //  SSL — contrôle/(re)génération des clés (serveur / monoposte).
    //  clesSSLServeurPresentes() : la copie CLIENT exportable (PATH_DIR_CLESSSL_SERVEUR) existe-t-elle ?
    //  dateExpirationCertSSL()  : date d'expiration du certificat serveur (SHOW STATUS Ssl_server_not_after),
    //                             invalide si SSL inactif / certificat absent.
    //  extraireClesSSLDepuisDatadir() : (ré)extrait les clés client du datadir SANS redémarrer le serveur
    //                             (cas « copie effacée par erreur » alors que le serveur a déjà ses certs).
    //  regenererClesSSL() : DESTRUCTIF — arrête le serveur, supprime les certs du datadir, redémarre (MySQL
    //                             les régénère via auto_generate_certs), réextrait. INVALIDE les clés déjà
    //                             distribuées → l'appelant doit prévenir + RELANCER Rufus (connexion coupée).
    static bool      clesSSLServeurPresentes();
    QDateTime        dateExpirationCertSSL();
    bool             extraireClesSSLDepuisDatadir();
    bool             regenererClesSSL();
    //  MONOPOSTE, à chaque démarrage : contrôle les clés SSL serveur. Copie client absente mais certs
    //  valides → réextraction SILENCIEUSE (sans redémarrage). Certs expirés / absents → message
    //  d'avertissement + (sur consentement) régénération destructive puis RELANCE de Rufus.
    void             controlerClesSSLMonoposte();
    //  ACCÈS DISTANT, à chaque démarrage : prévient si les certificats SSL approchent de leur
    //  expiration (le serveur, souvent dans une armoire, ne lance jamais Rufus et ne le verrait pas).
    //  Sans renouvellement, l'accès distant serait refusé du jour au lendemain. Lecture via
    //  dateExpirationCertSSL() ; la régénération, elle, se fait sur le SERVEUR.
    void             avertirExpirationClesSSLDistant();

    //  À appeler APRÈS toute connexion réussie : sécurise la base au besoin, puis
    //  supprime gaxt78iy si la deadline (sécurisation + 30 j) est passée.
    static void      entretienApresConnexion();
    //  true si adminrufus possède un 2e mot de passe (base déjà sécurisée). Détecté via
    //  mysql.user.User_attributes (sans connaître la valeur du mot de passe).
    static bool      adminrufusEstSecurise();
    //  true si un compte adminrufus de plage LOCALE (host ≠ '%') a PERDU SYSTEM_USER (revoke d'une
    //  version antérieure) → à régulariser. Une seule requête, pas de balayage host par host.
    static bool      unCompteLANaPerduSystemUser();
    //  Date de sécurisation = password_last_changed d'adminrufus (côté serveur, fiable).
    static QDateTime dateSecurisation();
    //  Nom du poste ayant posé le mot de passe sécurisé (grâce à User_attributes.securepar).
    //  Vide si la base a été sécurisée par une version antérieure (attribut absent).
    static QString   posteSecurisation();
    //  Supprime gaxt78iy (DISCARD OLD PASSWORD) si la deadline est passée — UNIQUEMENT si
    //  ce poste détient le vrai mot de passe (sinon il se verrouillerait lui-même).
    static void      supprimerGaxt78iySiEchue();
    //  Poste détenant l'ALÉATOIRE, gaxt78iy encore présent et deadline NON atteinte : simple
    //  avertissement que l'effacement du générique est imminent (informatif, pas d'action).
    void      avertirEffacementImminent();
    //  Poste connecté avec le GÉNÉRIQUE alors que la base est SÉCURISÉE (un aléatoire existe) —
    //  TOUS modes : message d'échéance + bouton « Renseigner le nouveau mot de passe » (saisie / clé
    //  USB → .dbkey), pour récupérer l'aléatoire avant le retrait de gaxt78iy. (Ex-rappel distant,
    //  généralisé : un poste local doit aussi pouvoir récupérer l'aléatoire posé par un autre poste.)
    void      proposerRecuperationAleatoire();
    //  Poste DISTANT connecté avec le GÉNÉRIQUE sur une base NON encore sécurisée : on ne sécurise
    //  jamais depuis un poste distant (cf. securiserBaseSiNecessaire, garde-fou n°0), on se contente
    //  d'INVITER à le faire depuis un poste du réseau local ou le serveur. Simple information, un seul
    //  bouton OK (aucun aléatoire n'existe encore → rien à saisir). Réservé à MySQL >= 8.0.14 (en deçà,
    //  c'est l'avis « serveur à mettre à jour »).
    void      suggererSecurisationDepuisLocal();
    //  Recréation MANUELLE du mot de passe aléatoire (bouton « le mot de passe est égaré »), quand plus
    //  aucun poste ne le détient (ex. .dbkey perdu). Protégée par le mot de passe Administrateur de Rufus.
    //  Réservé au LOCAL + socle conforme. Appelée par proposerRecuperationAleatoire() et par dlg_Param.
    bool      recreerMotDePasseApresVerifAdmin(QWidget *parent = Q_NULLPTR);

    // Résultat de createUserAvecAdmin().
    enum class CreateUserResult { Ok, NoCreateUserRight, Error };

private:
    static QHash<QString,QString> s_cacheMDP;    // cache mémoire des mdp MySQL par mode (MONO/LAN/WAN), cf motDePasseSQL
    QString             m_login;                 // = LOGIN_SQL (compte SQL technique)
    QString             m_password;              // mot de passe aléatoire d'adminrufus
    QString             m_createUserErr;         // sortie SQL du dernier createUser() échoué (diag)
    QString             m_secureFilePrivErr;     // diagnostic du dernier ensureSecureFilePriv() échoué
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
    bool    reinstallerSocleMySQL(const MySQLRemoteConfig& cfg);   // section install de faireCreate, sans saisie d'utilisateur (migration)

    //  MySQL DÉJÀ présent et compatible : que faire de l'instance existante ?
    //  (plus de « mode Verify » automatique — on demande à l'utilisateur).
    enum class QueFaireMySQL { Annuler, Effacer, Conserver, Reinstaller };
    //  Boîte « MySQL est déjà présent… que voulez-vous faire ? ». Si compatible :
    //  Effacer / Conserver / Annuler ; sinon : Réinstaller / Annuler.
    QueFaireMySQL demanderQueFaireMySQL(bool compatible);
    //  Le serveur MySQL local est-il MariaDB ? (incompatible : pas de double mdp.)
    bool    isMariaDB();
    //  Avertit que des données NON-Rufus vont être effacées et propose d'arrêter
    //  Rufus pour les sauvegarder soi-même. Si l'utilisateur choisit d'arrêter, on
    //  quitte le programme (exit) ; sinon la fonction rend la main (on continue).
    void    offrirSauvegardeAvantEffacement();
    //  RÉUTILISE un MySQL existant compatible : demande un compte admin MySQL, crée
    //  adminrufus/SSL, (option : efface les bases non-Rufus), config, puis saisie du
    //  futur utilisateur Rufus. true si prêt ; false (échec/annulation) → retour au choix.
    bool    faireReutiliser(const MySQLRemoteConfig& cfg, bool effacerTout);
    //  Supprime toutes les bases NON système (données étrangères à Rufus incluses).
    void    effacerToutesBasesUtilisateur(const QString& adminLogin, const QString& adminMdp);

    //  Étapes de configuration post-install/verify (PATH, dossier partagé,
    //  secure_file_priv, lecture/écriture, privilèges), avec mise à jour de la
    //  checklist. createUser() n'est invoqué que si m_freshInstall && !m_comptesDejaCrees.
    bool    executerEtapesConfig();      // true si toutes les étapes OK
    void    cleanupDialog();             // ferme et détruit m_dialog

    // ── Multi-plateforme ───────────────────────────────────────────────────────
    QString sharedFolderPath();          // /Users/Shared (macOS) | C:/Users/Public (Windows)

    // ── Pré-requis ─────────────────────────────────────────────────────────────
    bool    isAdminUser();               // compte administrateur / processus élevé ?
    //  Garantit les droits admin (requis SEULEMENT pour installer/désinstaller
    //  MySQL) : si le processus n'est pas élevé, propose l'élévation UAC (Windows)
    //  puis exit() au profit de l'instance élevée. true si on a les droits, false
    //  si l'utilisateur refuse. À n'appeler QUE juste avant une (dés)installation.
    bool    assurerDroitsAdmin();
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
    QString getMySQLServerVersion();     // version du SERVEUR (mysqld), pas du client mysql
    QString serverVersionString();       // VERSION() brute du serveur EN COURS (vide si injoignable)
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
    //  Alarme commune (macOS + Windows) quand le téléchargement de MySQL échoue :
    //  invite l'utilisateur à installer MySQL lui-même puis à relancer Rufus, qui
    //  détectera alors le serveur et le configurera.
    void    avertirTelechargementImpossible();
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
