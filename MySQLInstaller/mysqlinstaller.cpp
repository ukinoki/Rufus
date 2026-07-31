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

#include "mysqlinstaller.h"

#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QProcess>
#include <QEventLoop>
#include <QTimer>
#include <QElapsedTimer>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QLineEdit>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QHostInfo>
#include <QTcpSocket>
#include <QSettings>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRandomGenerator>
#include "database.h"           /*!< DataBase::I()->ModeAccesDataBase() : mode de connexion courant */

#if defined(Q_OS_WIN)
#  define WIN32_LEAN_AND_MEAN
#  define NOMINMAX
#  include <windows.h>
#  include <shellapi.h>   /*!< ShellExecuteEx (élévation UAC « runas ») */
#endif

/*! Arguments de transport des clients mysql/mysqladmin pour les connexions AVEC IDENTIFIANTS. nbp est
 *  toujours mono-poste/local : on force TCP vers 127.0.0.1:3306, comme le pilote Qt de Rufus (et MySQL
 *  Workbench). Sinon ces clients passent par le socket Unix local (hôte « localhost ») : un compte
 *  'user'@'127.0.0.1', ou masqué par un anonyme ''@'localhost', est refusé alors qu'il se connecte très
 *  bien en TCP. (À ne PAS utiliser pour « mysql -u root » via pkexec en mode Create, qui repose
 *  volontairement sur l'authentification socket d'Ubuntu.) */
#define LOCAL_TCP_ARGS "--protocol=TCP -h 127.0.0.1 -P 3306"

/*! ═══ MySQLProgressDialog (porté de ProgressDialog) ══════════════════════════════════════════════ */
MySQLProgressDialog::MySQLProgressDialog(const QString& operation, QWidget* parent)
    : QDialog(parent, Qt::Dialog | Qt::WindowTitleHint | Qt::CustomizeWindowHint)
{
    setWindowTitle(tr("MySQL Installer"));
    setFixedWidth(420);
    setModal(true);

    auto* root = new QVBoxLayout(this);
    root    ->setContentsMargins(28, 28, 28, 28);
    root    ->setSpacing(18);

    m_label = new QLabel(operation);
    m_label ->setStyleSheet("font-size: 14px; font-weight: 600; color: #1C1B18;");
    m_label ->setWordWrap(true);
    m_label ->setAlignment(Qt::AlignCenter);
    root    ->addWidget(m_label);

    m_progress = new QProgressBar();
    m_progress ->setRange(0, 0);   /*!< indéterminé (barre animée) */
    m_progress ->setFixedHeight(10);
    m_progress ->setTextVisible(false);
    m_progress ->setStyleSheet(R"(
        QProgressBar {
            border: none; border-radius: 5px;
            background: #D3D1C7;
        }
        QProgressBar::chunk {
            border-radius: 5px;
            background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
                stop:0 #7F77DD, stop:1 #534AB7);
        }
    )");
    root       ->addWidget(m_progress);

    /*! Détail sous la barre : taille téléchargée (rassure sur un gros téléchargement, ~550 Mo pour le
     *  DMG macOS). Vide tant qu'on n'a pas de progression chiffrée. */
    m_detail = new QLabel(QString());
    m_detail ->setStyleSheet("font-size: 12px; color: #6B6A63;");
    m_detail ->setAlignment(Qt::AlignCenter);
    root     ->addWidget(m_detail);
}

/*!
 * \brief MySQLProgressDialog::setProgress
 * Progression en pourcentage + volume « X / Y Mo » sous la barre.
 * \param received  octets déjà reçus
 * \param total     taille totale (<= 0 → barre animée indéterminée)
 */
void MySQLProgressDialog::setProgress(qint64 received, qint64 total)
{
    if (total <= 0) {                 /*!< taille inconnue → barre animée */
        m_progress ->setRange(0, 0);
        m_detail   ->clear();
        return;
    }
    m_progress ->setRange(0, 100);
    m_progress ->setValue(int(received * 100 / total));

    /*! « X / Y Mo » : un gros téléchargement (centaines de Mo) qui n'affiche rien paraît figé. */
    const double mo = 1024.0 * 1024.0;
    m_detail ->setText(tr("%1 / %2 Mo").arg(received / mo, 0, 'f', 0).arg(total / mo, 0, 'f', 0));
}

/*!
 * \brief MySQLProgressDialog::setProgressBar
 * Progression en FRACTION (ex. fichiers extraits) : remplit la barre SANS volume en Mo (ces valeurs ne
 * sont pas des octets → « 0 / 0 Mo » serait trompeur).
 * \param done   nombre d'éléments traités
 * \param total  nombre total (<= 0 → barre animée)
 */
void MySQLProgressDialog::setProgressBar(qint64 done, qint64 total)
{
    if (total <= 0) {                 /*!< total inconnu → barre animée */
        m_progress ->setRange(0, 0);
        m_detail   ->clear();
        return;
    }
    m_progress ->setRange(0, 100);
    m_progress ->setValue(int(done * 100 / total));
    m_detail   ->clear();
}

/*! ═══ MySQLInstallerDialog : checklist des 6 critères (composants Rufus up*) ═════════════════════ */
MySQLInstallerDialog::MySQLInstallerDialog(QWidget* parent)
    : UpDialog(parent)
{
    setWindowTitle(tr("Préparation de MySQL pour Rufus"));

    int row = 0;

    /*! Titre + sous-titre (libellés pilotés par les configurer*()). */
    m_title = new UpLabel(this, tr("Préparation de MySQL pour Rufus"));
    m_title     ->setStyleSheet("font-size: 16px; font-weight: 700; color: #1C1B18;");
    m_title     ->setWordWrap(true);
    dlglayout() ->insertWidget(row++, m_title);

    m_subtitle = new UpLabel(this, QString());
    m_subtitle  ->setStyleSheet("font-size: 12px; color: #1C1B18;");
    m_subtitle  ->setWordWrap(true);
    dlglayout() ->insertWidget(row++, m_subtitle);

    /*! Saisie : identifiant + mot de passe (mêmes validateurs que dlg_paramconnexion). */
    m_loginLbl = new UpLabel(this, tr("Identifiant :"));
    dlglayout() ->insertWidget(row++, m_loginLbl);
    m_login = new UpLineEdit(this);
    m_login     ->setValidator(new QRegularExpressionValidator(Utils::rgx_AlphaNumeric_5_15, this));
    dlglayout() ->insertWidget(row++, m_login);

    m_mdpLbl = new UpLabel(this, tr("Mot de passe :"));
    dlglayout() ->insertWidget(row++, m_mdpLbl);
    m_mdp = new UpLineEdit(this);
    m_mdp       ->setValidator(new QRegularExpressionValidator(Utils::rgx_AlphaNumeric_5_12, this));
    m_mdp       ->setEchoMode(QLineEdit::Password);
    dlglayout() ->insertWidget(row++, m_mdp);

    /*! Confirmation du mot de passe : seulement à la CRÉATION d'un compte (voir configurer*()). Masquée
     *  par défaut ; affichée par configurerCreateUserRufus / configurerNewUserRufus. */
    m_mdpConfirmLbl = new UpLabel(this, tr("Confirmez le mot de passe :"));
    dlglayout()     ->insertWidget(row++, m_mdpConfirmLbl);
    m_mdpConfirm = new UpLineEdit(this);
    m_mdpConfirm    ->setValidator(new QRegularExpressionValidator(Utils::rgx_AlphaNumeric_5_12, this));
    m_mdpConfirm    ->setEchoMode(QLineEdit::Password);
    dlglayout()     ->insertWidget(row++, m_mdpConfirm);
    m_mdpConfirmLbl ->setVisible(false);
    m_mdpConfirm    ->setVisible(false);

    /*! Les 7 cases (affichage seul) insérées entre la saisie et les boutons. */
    for (int i = 0; i < 7; i++) {
        m_steps[i] = new UpCheckBox();
        m_steps[i]  ->setToggleable(false);   /*!< pilotée par l'engine, non cliquable */
        dlglayout() ->insertWidget(row++, m_steps[i]);
        applyStepLabel(i);
    }

    /*! Boutons « Annuler » + « OK » (libellé d'OK ajusté par configurer*()). AjouteLayButtons relie déjà
     *  Annuler à reject() ; on relie OK à accept(). */
    AjouteLayButtons(UpDialog::ButtonCancel | UpDialog::ButtonOK);
    if (OKButton)
        connect(OKButton, &QPushButton::clicked, this, &MySQLInstallerDialog::accept);

    /*! « Supprimer MySQL » (mode Verify) : désinstalle le MySQL existant pour repartir neuf. Placé à
     *  gauche (loin d'OK/Annuler) ; masqué par défaut, affiché par configurerVerifyAdminMySQL(). Le clic
     *  clôt exec() avec ResultSupprimerMySQL. */
    m_btnSupprMySQL = new UpSmallButton(tr("Supprimer MySQL"), this);
    m_btnSupprMySQL ->setVisible(false);
    AjouteWidgetLayButtons(m_btnSupprMySQL, false);   /*!< false = inséré tout à gauche */
    connect(m_btnSupprMySQL, &QPushButton::clicked, this,
            [this]{ done(ResultSupprimerMySQL); });
}

/*! ── Configuration selon le contexte ──────────────────────────────────────────── */

/*!
 * \brief MySQLInstallerDialog::configurer
 * Base commune aux configurer*() : applique titre / sous-titre / libellé du bouton OK et remet les
 * champs en état interactif (éditables, vidés).
 * \param titre      titre de la fiche
 * \param sousTitre  texte explicatif sous le titre
 * \param okLabel    libellé du bouton OK selon le contexte
 */
void MySQLInstallerDialog::configurer(const QString& titre,
                                      const QString& sousTitre,
                                      const QString& okLabel)
{
    m_title    ->setText(titre);
    m_subtitle ->setText(sousTitre);
    if (OKButton) { OKButton->setText(okLabel); OKButton->show(); }
    if (CancelButton) CancelButton->show();

    /*! Mode interactif : champs éditables (restaure l'état après un « paramétrage en cours »). */
    if (m_login)      { m_login->setEnabled(true);      m_login->clear(); }
    if (m_mdp)        { m_mdp->setEnabled(true);        m_mdp->clear(); }
    if (m_mdpConfirm) { m_mdpConfirm->setEnabled(true); m_mdpConfirm->clear(); }
    if (m_login) m_login->setFocus();

    /*! « Supprimer MySQL » n'a de sens qu'en mode Verify : masqué ici, ré-affiché par configurerVerifyAdminMySQL(). */
    if (m_btnSupprMySQL) m_btnSupprMySQL->setVisible(false);
}

/*!
 * \brief MySQLInstallerDialog::passerEnConfiguration
 * Bascule en mode « paramétrage en cours » : plus aucune saisie ni clic attendus. Login/mdp (déjà
 * choisis) restent affichés mais GRISÉS, boutons OK/Annuler (et « Supprimer MySQL ») masqués ; seule la
 * checklist se coche en direct.
 * \param titre      titre affiché en haut de la fiche
 * \param sousTitre  sous-titre explicatif
 */
void MySQLInstallerDialog::passerEnConfiguration(const QString& titre,
                                                 const QString& sousTitre)
{
    m_title    ->setText(titre);
    m_subtitle ->setText(sousTitre);
    if (m_login)      m_login->setEnabled(false);
    if (m_mdp)        m_mdp->setEnabled(false);
    if (m_mdpConfirm) m_mdpConfirm->setEnabled(false);
    if (OKButton)        OKButton->hide();
    if (CancelButton)    CancelButton->hide();
    if (m_btnSupprMySQL) m_btnSupprMySQL->setVisible(false);
    unsetCursor();
    QApplication::processEvents();
}

/*!
 * \brief appliquerValidateursRufus
 * Impose le format d'un identifiant Rufus à CRÉER (5-15 / 5-12 caractères alphanumériques).
 * \param login   champ identifiant
 * \param mdp     champ mot de passe
 * \param parent  parent Qt des validateurs
 */
static void appliquerValidateursRufus(UpLineEdit* login, UpLineEdit* mdp, QObject* parent)
{
    login ->setValidator(new QRegularExpressionValidator(Utils::rgx_AlphaNumeric_5_15, parent));
    mdp   ->setValidator(new QRegularExpressionValidator(Utils::rgx_AlphaNumeric_5_12, parent));
}

void MySQLInstallerDialog::configurerCreateUserRufus(const QString& minVersion)
{
    setMinVersion(minVersion);
    configurer(tr("Création de la base Rufus"),
               tr("Choisissez l'identifiant et le mot de passe que vous utiliserez "
                  "dans Rufus."),
               tr("Installer"));
    appliquerValidateursRufus(m_login, m_mdp, this);   /*!< user Rufus → format imposé */

    /*! CRÉATION d'un compte → champ de confirmation du mot de passe visible ET exigé à la validation. */
    m_confirmMdpRequis = true;
    if (m_mdpConfirmLbl) m_mdpConfirmLbl->setVisible(true);
    if (m_mdpConfirm)    { m_mdpConfirm->setVisible(true);
                           m_mdpConfirm->setValidator(new QRegularExpressionValidator(Utils::rgx_AlphaNumeric_5_12, this)); }
}

void MySQLInstallerDialog::masquerSaisieUtilisateur()
{
    m_confirmMdpRequis = false;
    if (m_loginLbl)      m_loginLbl->setVisible(false);
    if (m_login)         m_login->setVisible(false);
    if (m_mdpLbl)        m_mdpLbl->setVisible(false);
    if (m_mdp)           m_mdp->setVisible(false);
    if (m_mdpConfirmLbl) m_mdpConfirmLbl->setVisible(false);
    if (m_mdpConfirm)    m_mdpConfirm->setVisible(false);
}

void MySQLInstallerDialog::configurerVerifyAdminMySQL()
{
    configurer(tr("Connexion à MySQL"),
               tr("Un serveur MySQL existe déjà. Saisissez l'identifiant et le mot "
                  "de passe d'un compte MySQL administrateur (capable de créer des "
                  "utilisateurs)."),
               tr("Se connecter"));

    /*! Compte MySQL EXISTANT : aucun format imposé (ex. « root » — 4 car. —, ou mdp non alphanumérique)
     *  → on RETIRE les validateurs. */
    m_login ->setValidator(nullptr);
    m_mdp   ->setValidator(nullptr);

    /*! On SAISIT un mot de passe existant (pas de création) → pas de confirmation. */
    m_confirmMdpRequis = false;
    if (m_mdpConfirmLbl) m_mdpConfirmLbl->setVisible(false);
    if (m_mdpConfirm)    m_mdpConfirm->setVisible(false);

    /*! Le choix « réinstaller / effacer » est fait en amont (boîte « que faire de MySQL ? ») : cette
     *  fiche ne sert plus qu'à saisir les identifiants. */
    if (m_btnSupprMySQL) m_btnSupprMySQL->setVisible(false);

    /*! La version MySQL est déjà connue : l'étape 0 sert de statut de connexion. */
    m_steps[0] ->setText(tr("Connexion OK"));
}

void MySQLInstallerDialog::configurerNewUserRufus()
{
    configurer(tr("Compte utilisateur Rufus"),
               tr("Choisissez l'identifiant et le mot de passe que vous utiliserez "
                  "dans Rufus."),
               tr("Créer le compte"));
    appliquerValidateursRufus(m_login, m_mdp, this);   /*!< user Rufus → format imposé */

    /*! CRÉATION d'un compte → champ de confirmation du mot de passe visible ET exigé à la validation. */
    m_confirmMdpRequis = true;
    if (m_mdpConfirmLbl) m_mdpConfirmLbl->setVisible(true);
    if (m_mdpConfirm)    { m_mdpConfirm->setVisible(true);
                           m_mdpConfirm->setValidator(new QRegularExpressionValidator(Utils::rgx_AlphaNumeric_5_12, this)); }
}

QString MySQLInstallerDialog::login() const
{
    return m_login ? m_login ->text() : QString();
}

QString MySQLInstallerDialog::password() const
{
    return m_mdp ? m_mdp ->text() : QString();
}

/*!
 * \brief MySQLInstallerDialog::validerSaisie
 * Vérifie que login et mot de passe sont renseignés, et — en mode création — que le mot de passe et sa
 * confirmation coïncident. Affiche un message et renvoie false sinon.
 */
bool MySQLInstallerDialog::validerSaisie()
{
    if (login().isEmpty() || password().isEmpty()) {
        UpMessageBox::Watch(this, tr("Saisie incomplète"),
            tr("Veuillez renseigner un identifiant et un mot de passe."));
        return false;
    }

    /*! Confirmation : uniquement à la CRÉATION (m_confirmMdpRequis). On vérifie que les deux saisies
     *  coïncident — pas question d'enregistrer un mot de passe mal tapé. NB : on ne teste PAS isVisible()
     *  (validerSaisie() est appelée APRÈS exec(), fiche déjà masquée → faux). */
    if (m_confirmMdpRequis && m_mdpConfirm
        && password() != m_mdpConfirm->text()) {
        UpMessageBox::Watch(this, tr("Confirmation incorrecte"),
            tr("Le mot de passe et sa confirmation ne sont pas identiques."));
        m_mdpConfirm ->clear();
        m_mdpConfirm ->setFocus();
        return false;
    }
    return true;
}

QString MySQLInstallerDialog::baseStepLabel(int i) const
{
    switch (i) {
    case 0: return tr("MySQL %1 (ou ultérieur) installé").arg(m_minVersion);
    case 1: return tr("Variable d'environnement MySQL (PATH) configurée");
    case 2: return tr("Dossier partagé créé et partagé");
    case 3: return tr("secure_file_priv configuré");
    case 4: return tr("Lecture / écriture MySQL vérifiée");
    case 5: return tr("Droits de l'utilisateur confirmés");
    case 6: return tr("Clés SSL pour l'accès distant");
    }
    return {};
}

void MySQLInstallerDialog::applyStepLabel(int i)
{
    if (i < 0 || i > 6) return;
    QString label = baseStepLabel(i);
    if (!m_stepDetail[i].isEmpty())
        label += " : " + m_stepDetail[i];
    m_steps[i] ->setText(label);
}

/*!
 * \brief MySQLInstallerDialog::checkStep
 * Coche la case i de la checklist, force le rafraîchissement immédiat et marque une courte pause pour
 * qu'on voie la coche se poser.
 * \param i  index de la case (0..6)
 */
void MySQLInstallerDialog::checkStep(int i)
{
    if (i < 0 || i > 6) return;
    m_steps[i] ->setChecked(true);

    /*! Peinture SYNCHRONE : sous Windows, après une étape qui bloque longuement le thread UI (typiquement
     *  secure_file_priv, qui redémarre le service MySQL via un appel élevé), processEvents() seul ne
     *  rafraîchit pas toujours la case → les dernières coches ne se voyaient pas. repaint() force
     *  l'affichage immédiat. macOS repeignait déjà correctement. */
    m_steps[i] ->repaint();
    QApplication::processEvents();

    /*! Pause pour qu'on VOIE chaque coche se poser (comme dlg_identificationuser). DANS checkStep (pas
     *  entre les appels) → s'applique AUSSI à la dernière coche avant fermeture. Utils::Pause traite les
     *  événements (l'UI reste vivante). */
    Utils::Pause(450);
}

void MySQLInstallerDialog::uncheckAllSteps()
{
    for (int i = 0; i < 7; i++)
        m_steps[i] ->setChecked(false);
    QApplication::processEvents();
}

void MySQLInstallerDialog::setStepDetail(int i, const QString& detail)
{
    if (i < 0 || i > 6) return;
    m_stepDetail[i] = detail;
    applyStepLabel(i);
}

void MySQLInstallerDialog::setMinVersion(const QString& v)
{
    if (v.isEmpty() || v == m_minVersion) return;
    m_minVersion = v;
    applyStepLabel(0);
}

void MySQLInstallerDialog::reject()
{
    m_cancelled = true;
    UpDialog::reject();
}

/*! ═══ Helpers d'exécution dépendants de la plateforme ════════════════════════════════════════════════
    Déclarés tôt : utilisés par de nombreuses méthodes plus bas. */
static inline QString shellProgram()
{
#if defined(Q_OS_WIN)
    return "cmd.exe";
#else
    return "/bin/bash";
#endif
}

static inline QStringList shellArgs(const QString& cmd)
{
#if defined(Q_OS_WIN)
    return QStringList{"/C", cmd};
#else
    return QStringList{"-c", cmd};
#endif
}

/*! Redirection « rien » pour masquer stderr selon la plateforme. */
static inline QString NUL()
{
#if defined(Q_OS_WIN)
    return "2>nul";
#else
    return "2>/dev/null";
#endif
}

/*!
 * \brief startShellProcess
 * Démarre « cmd » dans le shell système. Sous Windows, la ligne va telle quelle à cmd.exe, encadrée
 * d'une paire de guillemets que cmd /C retire — sans quoi une commande mêlant chemin et arguments
 * quotés serait corrompue.
 * \param p    processus à démarrer
 * \param cmd  ligne de commande
 */
static inline void startShellProcess(QProcess& p, const QString& cmd)
{
#if defined(Q_OS_WIN)
    p.setProgram("cmd.exe");
    p.setNativeArguments("/C \"" + cmd + "\"");
    p.start();
#else
    p.start(shellProgram(), shellArgs(cmd));
#endif
}

/*!
 * \brief waitProcessResponsive
 * Attend la fin de « p » SANS geler l'interface : Qt continue de repeindre (événements de peinture
 * seulement, pas les clics → aucune réentrance). Évite le « (l'application ne répond pas) ».
 * \param p          processus attendu
 * \param timeoutMs  délai max avant kill (0 = illimité)
 */
static void waitProcessResponsive(QProcess& p, int timeoutMs)
{
    QElapsedTimer t;
    t.start();
    while (p.state() != QProcess::NotRunning) {
        if (p.waitForFinished(50))
            break;
        if (timeoutMs > 0 && t.elapsed() > timeoutMs) {
            p.kill();
            p.waitForFinished(2000);
            break;
        }
        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents, 40);
    }
}

/*!
 * \brief versionAtLeast
 * Compare deux versions « pointées » composant par composant ; true si ver >= minVer (version
 * vide/inconnue = inférieure).
 * \param ver     version à tester (ex. « 8.0.40 »)
 * \param minVer  seuil minimal (ex. « 8.0.14 »)
 */
static bool versionAtLeast(const QString& ver, const QString& minVer)
{
    const QStringList a = ver.split('.', Qt::SkipEmptyParts);
    const QStringList b = minVer.split('.', Qt::SkipEmptyParts);
    const int n = qMax(a.size(), b.size());
    for (int i = 0; i < n; ++i) {
        const int va = (i < a.size()) ? a.at(i).toInt() : 0;
        const int vb = (i < b.size()) ? b.at(i).toInt() : 0;
        if (va != vb) return va > vb;
    }
    return true;   /*!< égalité exacte => au moins la version minimale */
}

/*!
 * \brief seuilVersionMySQL
 * Seuil minimal de version MySQL exigé, COMMUN à tous les OS (VERSION_MYSQL_MINI = 8.0.14) : minimum
 * fonctionnel des fonctions de sécurité (double mot de passe). Ce qui compte est la version du serveur,
 * pas la plateforme qui l'héberge.
 */
static QString seuilVersionMySQL()
{
    return VERSION_MYSQL_MINI;
}

/*! ═══ Mot de passe aléatoire (helpers statiques) ═════════════════════════════════════════════════ */

/*!
 * \brief MySQLInstaller::genererMotDePasse
 * Mot de passe alphanumérique fort (12 caractères, [A-Za-z0-9]) tiré d'un générateur semé de façon
 * sécurisée. 12 car. (et non 24) pour tenir dans la contrainte de saisie 5-12 des champs login/mdp ;
 * 62^12 ≈ 3·10^21 reste largement assez fort pour ce compte.
 */
QString MySQLInstaller::genererMotDePasse()
{
    static const QString alphabet =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    QRandomGenerator gen = QRandomGenerator::securelySeeded();
    QString pwd;
    pwd.reserve(12);
    for (int i = 0; i < 12; ++i)
        pwd.append(alphabet.at(int(gen.bounded(uint(alphabet.size())))));
    return pwd;
}

/*! Le mot de passe MySQL vit dans ~/.rufus/.dbkey et non dans Rufus.ini, pour survivre à la suppression
 *  de Rufus.ini. Une ligne par mode d'accès (MONO= / LAN= / WAN=), un poste pouvant se connecter à
 *  plusieurs bases. Rétro-compat : ligne brute ou « MDPSQL=… » = entrée MONO. Repli gaxt78iy si le mode
 *  courant n'a pas d'entrée. */

/*! Cache mémoire des mots de passe, par clé de mode (MONO/LAN/WAN). */
QHash<QString,QString> MySQLInstaller::s_cacheMDP;

/*!
 * \brief cleDepuisMode
 * Clé .dbkey correspondant à un mode d'accès (MONO / LAN / WAN).
 * \param mode  mode d'accès
 */
static QString cleDepuisMode(Utils::ModeAcces mode)
{
    switch (mode) {
    case Utils::ReseauLocal: return "LAN";
    case Utils::Distant:     return "WAN";
    case Utils::Poste:
    default:                 return "MONO";     /*!< monoposte (poste itinérant) : clé fixe */
    }
}

/*! Clé du mode de connexion courant (porté par DataBase). */
static QString cleModeCourant()
{
    return cleDepuisMode(DataBase::I()->ModeAccesDataBase());
}

/*!
 * \brief lireDBKey
 * Lit tout le .dbkey dans une table clé→mdp (clés MONO/LAN/WAN). Gère les formats hérités (ligne brute
 * ou « MDPSQL=… » → MONO).
 */
static QHash<QString,QString> lireDBKey()
{
    QHash<QString,QString> table;
    QFile f(PATH_FILE_DBKEY);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return table;
    const QStringList lignes = QString::fromUtf8(f.readAll()).split('\n');
    f.close();
    for (const QString &ligne : lignes) {
        const QString l = ligne.trimmed();
        if (l.isEmpty() || l.startsWith('['))     /*!< ignore lignes vides et sections INI */
            continue;
        const int eq = l.indexOf('=');
        if (eq > 0) {
            QString cle = l.left(eq).trimmed().toUpper();
            const QString val = l.mid(eq + 1).trimmed();
            if (cle == "MDPSQL")                  /*!< ancien format INI → monoposte */
                cle = "MONO";
            if ((cle == "MONO" || cle == "LAN" || cle == "WAN") && !val.isEmpty())
                table.insert(cle, val);
        } else if (!table.contains("MONO")) {
            table.insert("MONO", l);              /*!< ancienne ligne brute = monoposte */
        }
    }
    return table;
}

/*!
 * \brief MySQLInstaller::motDePasseSQL
 * Mot de passe du MODE COURANT, résolu une fois par mode (lecture .dbkey) puis mis en cache. Fonction
 * PURE (aucune connexion, aucune UI). Repli gaxt78iy si pas d'entrée.
 */
QString MySQLInstaller::motDePasseSQL()
{
    const QString cle = cleModeCourant();
    auto it = s_cacheMDP.constFind(cle);
    if (it != s_cacheMDP.constEnd())
        return it.value();                        /*!< déjà résolu en mémoire : pas d'accès disque */

    QString mdp = lireDBKey().value(cle);
    if (mdp.isEmpty())
        mdp = QString(MDP_SQL);                   /*!< absent → repli legacy gaxt78iy */
    s_cacheMDP.insert(cle, mdp);
    return mdp;
}

/*!
 * \brief MySQLInstaller::setMotDePasseSQL
 * Met à jour, EN MÉMOIRE, le mdp du mode courant (sans toucher au disque : cf. stockerMotDePasse).
 * \param mdp  mot de passe à mettre en cache
 */
void MySQLInstaller::setMotDePasseSQL(const QString& mdp)
{
    s_cacheMDP.insert(cleModeCourant(), mdp);
}

/*!
 * \brief MySQLInstaller::motsDePasseSQLCandidats
 * Mots de passe à essayer dans le mode courant, dans l'ordre : celui que ce poste connaît (.dbkey du
 * mode) puis gaxt78iy.
 */
QStringList MySQLInstaller::motsDePasseSQLCandidats()
{
    QStringList candidats;
    candidats << motDePasseSQL();                 /*!< .dbkey du mode (ou déjà gaxt78iy si absent) */
    const QString legacy = QString(MDP_SQL);
    if (!candidats.contains(legacy))
        candidats << legacy;                      /*!< gaxt78iy en dernier repli */
    return candidats;
}

/*!
 * \brief ecrireDBKey
 * Écrit toute la table clé→mdp dans le .dbkey (une ligne CLE=mdp par mode présent).
 * \param table  table clé→mot de passe à écrire
 */
static void ecrireDBKey(const QHash<QString,QString>& table)
{
    QDir().mkpath(PATH_DIR_RUFUSKEY);             /*!< dossier caché ~/.rufus (créé au besoin) */
    QFile f(PATH_FILE_DBKEY);
    if (f.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        for (const QString &k : { QStringLiteral("MONO"), QStringLiteral("LAN"), QStringLiteral("WAN") })
            if (table.contains(k))
                f.write(QString("%1=%2\n").arg(k, table.value(k)).toUtf8());
        f.close();
    }
}

/*!
 * \brief MySQLInstaller::connecterAvecCandidats
 * Connexion en cascade : essaie les mots de passe candidats (aléatoire .dbkey PUIS gaxt78iy) et mémorise
 * celui qui marche. Protège contre l'échec de la TOUTE PREMIÈRE connexion avec l'aléatoire fraîchement
 * posé par la sécurisation.
 * \param basename  base à laquelle se connecter
 */
QString MySQLInstaller::connecterAvecCandidats(const QString& basename)
{
    QString err;
    const QStringList candidats = motsDePasseSQLCandidats();
    for (const QString &mdp : candidats)
    {
        err = DataBase::I()->connectToDataBase(basename, LOGIN_SQL, mdp);
        if (err.isEmpty())
        {
            setMotDePasseSQL(mdp);                 /*!< mémorise le mdp qui fonctionne */
            break;
        }
        /*! Un aléatoire refusé n'efface PAS le .dbkey : c'est souvent l'unique copie du mot de passe du
         *  cabinet, et un refus peut être passager. Le détruire pouvait verrouiller la base. */
    }
    return err;
}

/*!
 * \brief MySQLInstaller::estErreurAuthentification
 * Refus d'authentification (mauvais mdp : MySQL 1045 / « Access denied ») vs serveur injoignable.
 * \param erreur  message d'erreur de connexion à classer
 */
bool MySQLInstaller::estErreurAuthentification(const QString& erreur)
{
    return erreur.contains("denied", Qt::CaseInsensitive)
        || erreur.contains("1045");
}

/*!
 * \brief MySQLInstaller::stockerMotDePasse
 * Stocke le mot de passe du MODE COURANT (cf. stockerMotDePassePourMode).
 * \param mdp  mot de passe à stocker
 */
void MySQLInstaller::stockerMotDePasse(const QString& mdp)
{
    stockerMotDePassePourMode(DataBase::I()->ModeAccesDataBase(), mdp);
}

/*!
 * \brief MySQLInstaller::motDePasseStockePourMode
 * Valeur BRUTE stockée pour un mode (pour configurer/afficher), "" si aucune.
 * \param mode  mode d'accès interrogé
 */
QString MySQLInstaller::motDePasseStockePourMode(Utils::ModeAcces mode)
{
    return lireDBKey().value(cleDepuisMode(mode));
}

/*!
 * \brief MySQLInstaller::stockerMotDePassePourMode
 * Stocke le mot de passe d'un mode explicite dans le .dbkey en PRÉSERVANT les autres modes, puis met à
 * jour le cache.
 * \param mode  mode d'accès concerné
 * \param mdp   mot de passe à stocker
 */
void MySQLInstaller::stockerMotDePassePourMode(Utils::ModeAcces mode, const QString& mdp)
{
    const QString cle = cleDepuisMode(mode);
    QHash<QString,QString> table = lireDBKey();   /*!< entrées existantes (autres modes) */
    table.insert(cle, mdp);
    ecrireDBKey(table);
    s_cacheMDP.insert(cle, mdp);                  /*!< garde le cache cohérent avec le disque */
}

/*!
 * \brief MySQLInstaller::supprimerMotDePassePourMode
 * Retire l'entrée d'un mode du .dbkey (mode de connexion abandonné) et du cache.
 * \param mode  mode d'accès à oublier
 */
void MySQLInstaller::supprimerMotDePassePourMode(Utils::ModeAcces mode)
{
    const QString cle = cleDepuisMode(mode);
    s_cacheMDP.remove(cle);
    QHash<QString,QString> table = lireDBKey();
    if (!table.contains(cle))
        return;                                   /*!< rien à retirer */
    table.remove(cle);
    ecrireDBKey(table);
}

/*! ═══ Config MySQL distante ═══════════════════════════════════════════════════════════════════════ */
MySQLRemoteConfig MySQLInstaller::defaultMySQLConfig()
{
    MySQLRemoteConfig c;
    c.version     = "8.4.2";
    /*! Seuil minimal accepté : 8.0.14, commun à tous les OS (VERSION_MYSQL_MINI). La version POSÉE en cas
     *  d'install est le 8.4.2 LTS (c.version), mais un serveur déjà >= 8.0.14 est accepté tel quel (le 8.0
     *  d'apt suffit, il sait déjà le double mot de passe). */
    c.minVersion  = seuilVersionMySQL();
    /*! Binaires auto-hébergés sur notre Release GitHub : URLs permanentes et téléchargeables en
     *  automatique, là où dev.mysql.com bloque et renomme à chaque point-release. Le JSON distant peut
     *  surcharger ces valeurs sans recompiler. */
    c.winUrl      = "https://github.com/ukinoki/mysqlinstaller-for-rufus/releases/download/mysql-8.4.2/mysql-8.4.2-winx64.zip";
    c.macArm64Url = "https://github.com/ukinoki/mysqlinstaller-for-rufus/releases/download/mysql-8.4.2/mysql-8.4.2-macos14-arm64.dmg";
    c.macX86Url   = "https://github.com/ukinoki/mysqlinstaller-for-rufus/releases/download/mysql-8.4.2/mysql-8.4.2-macos14-x86_64.dmg";
    return c;
}

/*!
 * \brief MySQLInstaller::fetchRemoteConfig
 * Charge la config MySQL distante la première fois puis renvoie le cache. Timeout 5 s ; en cas d'échec,
 * utilise defaultMySQLConfig().
 */
MySQLRemoteConfig MySQLInstaller::fetchRemoteConfig()
{
    if (m_remoteConfigLoaded)
        return m_remoteConfig;

    m_remoteConfig = defaultMySQLConfig();    /*!< pré-remplir avec les valeurs de fallback */

    const QString configUrl =
        "https://raw.githubusercontent.com/ukinoki/mysqlinstaller-for-rufus/main/mysql_config.json";

    QNetworkAccessManager nam;
    QNetworkRequest req{QUrl(configUrl)};
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                     QNetworkRequest::NoLessSafeRedirectPolicy);
    req.setAttribute(QNetworkRequest::Http2AllowedAttribute, false);
    req.setHeader(QNetworkRequest::UserAgentHeader, "MySQLInstaller/1.0");
    QNetworkReply* reply = nam.get(req);

    QEventLoop loop;
    QTimer timeout;
    timeout.setSingleShot(true);
    QObject::connect(reply,   &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QObject::connect(&timeout, &QTimer::timeout,        &loop, &QEventLoop::quit);
    timeout.start(5000);
    loop.exec();

    if (reply->error() == QNetworkReply::NoError) {
        const QByteArray data = reply->readAll();
        const QJsonObject obj = QJsonDocument::fromJson(data).object();
        if (!obj.isEmpty()) {
            if (obj.contains("mysql_version"))  m_remoteConfig.version     = obj["mysql_version"].toString();
            if (obj.contains("min_version"))     m_remoteConfig.minVersion  = obj["min_version"].toString();
#if defined(Q_OS_LINUX)
            /*! Sous Linux, un seuil dédié (plus tolérant) prime s'il est défini. */
            if (obj.contains("min_version_linux")) m_remoteConfig.minVersion = obj["min_version_linux"].toString();
#endif
            if (obj.contains("win_url"))         m_remoteConfig.winUrl      = obj["win_url"].toString();
            if (obj.contains("mac_arm64_url"))   m_remoteConfig.macArm64Url = obj["mac_arm64_url"].toString();
            if (obj.contains("mac_x86_url"))     m_remoteConfig.macX86Url   = obj["mac_x86_url"].toString();
        }
    }
    reply->deleteLater();
    m_remoteConfigLoaded = true;
    return m_remoteConfig;
}

/*! ═════════════════════════════════════════════════════════════════════════════════════════════════ */
MySQLInstaller::MySQLInstaller(QObject* parent)
    : QObject(parent)
{}

/*! ── Multi-plateforme : dossier partagé ───────────────────────────────────────── */
QString MySQLInstaller::sharedFolderPath()
{
#if defined(Q_OS_WIN)
    return "C:/Users/Public";   /*!< slashes avant : acceptés par Qt et MySQL sous Windows */
#else
    return "/Users/Shared";
#endif
}

/*! ── Pré-requis : droits administrateur ───────────────────────────────────────── */

/*!
 * \brief MySQLInstaller::isAdminUser
 * Le processus tourne-t-il avec les droits administrateur (jeton élevé sous Windows, root ou groupe
 * sudo/admin sous Linux/macOS) ?
 */
bool MySQLInstaller::isAdminUser()
{
#if defined(Q_OS_WIN)
    /*! Le processus est-il élevé ? On interroge directement le jeton d'accès via l'API Win32
     *  (TokenElevation), fiable — contrairement à un appel PowerShell via cmd.exe dont les guillemets
     *  imbriqués étaient mal transmis et faisaient échouer la détection. */
    BOOL elevated = FALSE;
    HANDLE token  = nullptr;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token)) {
        TOKEN_ELEVATION elevation;
        DWORD cb = sizeof(elevation);
        if (GetTokenInformation(token, TokenElevation, &elevation,
                                sizeof(elevation), &cb))
            elevated = elevation.TokenIsElevated;
        CloseHandle(token);
    }
    return elevated;
#elif defined(Q_OS_LINUX)
    /*! root, ou membre du groupe « sudo » (Ubuntu) / « admin ». */
    if (runCmd("id -u 2>/dev/null").trimmed() == "0")
        return true;
    const QStringList groups =
        runCmd("id -Gn 2>/dev/null").split(QRegularExpression("\\s+"),
                                           Qt::SkipEmptyParts);
    return groups.contains("sudo") || groups.contains("admin");
#else
    /*! Les administrateurs macOS sont membres du groupe « admin » (gid 80). */
    const QStringList groups =
        runCmd("id -Gn 2>/dev/null").split(QRegularExpression("\\s+"),
                                           Qt::SkipEmptyParts);
    return groups.contains("admin");
#endif
}

#if defined(Q_OS_WIN)
/*!
 * \brief MySQLInstaller::relancerEnAdministrateur
 * Relance l'exécutable Rufus courant AVEC élévation, via le verbe « runas » de ShellExecuteEx (invite
 * UAC ; sur un compte standard, l'invite permet de SAISIR un compte administrateur). true si l'instance
 * élevée a démarré (UAC acceptée) ; false si annulation/échec.
 */
bool MySQLInstaller::relancerEnAdministrateur()
{
    const QString exe = QDir::toNativeSeparators(QCoreApplication::applicationFilePath());
    const std::wstring exeW = exe.toStdWString();

    SHELLEXECUTEINFOW sei;
    ZeroMemory(&sei, sizeof(sei));
    sei.cbSize = sizeof(sei);
    sei.fMask  = SEE_MASK_NOCLOSEPROCESS;
    sei.lpVerb = L"runas";              /*!< déclenche l'élévation UAC */
    sei.lpFile = exeW.c_str();
    sei.nShow  = SW_SHOWNORMAL;

    if (ShellExecuteExW(&sei)) {
        if (sei.hProcess)
            CloseHandle(sei.hProcess);
        return true;                    /*!< instance élevée lancée */
    }
    return false;                       /*!< annulation (UAC) ou échec */
}
#endif

#if defined(Q_OS_LINUX)
/*! Ubuntu 22.04 (LTS) ou ultérieure (selon /etc/os-release). */
bool MySQLInstaller::isUbuntuVersionSupported()
{
    const QString ver =
        runCmd(". /etc/os-release 2>/dev/null; echo $VERSION_ID").trimmed();
    const QStringList p = ver.split('.');
    if (p.size() < 2)
        return false;
    const int major = p[0].toInt();
    const int minor = p[1].toInt();
    return (major > 22) || (major == 22 && minor >= 4);   /*!< ≥ 22.04 */
}
#endif

#if defined(Q_OS_WIN)
/*! ── Windows : Visual C++ Redistributable 2022 (x64) ──────────────────────────── */
bool MySQLInstaller::isVCRedist2022Installed()
{
    /*! Clé posée par le runtime VC++ 14.x (2015-2022, binairement compatibles). */
    const QStringList keys = {
        "HKLM\\SOFTWARE\\Microsoft\\VisualStudio\\14.0\\VC\\Runtimes\\X64",
        "HKLM\\SOFTWARE\\WOW6432Node\\Microsoft\\VisualStudio\\14.0\\VC\\Runtimes\\X64"
    };
    for (const QString& key : keys) {
        const QString out = runCmd("reg query " + key + " /v Installed " + NUL());
        if (out.contains("0x1"))
            return true;
    }
    return false;
}

bool MySQLInstaller::installVCRedist2022()
{
    const QString url = "https://aka.ms/vs/17/release/vc_redist.x64.exe";
    const QString exe = QDir::tempPath() + "/vc_redist.x64.exe";

    runLongOp(QString("curl -L -o \"%1\" \"%2\"").arg(exe, url),
              tr("Téléchargement de Visual C++ Redistributable 2022…"), 600000);
    if (!QFile::exists(exe))
        return false;

    runLongOp(QString("\"%1\" /install /quiet /norestart").arg(exe),
              tr("Installation de Visual C++ Redistributable 2022…"), 300000);
    QFile::remove(exe);
    return isVCRedist2022Installed();
}

/*!
 * \brief MySQLInstaller::registerWindowsUninstaller
 * Rend MySQL désinstallable depuis « Applications et fonctionnalités » (script PowerShell + entrée de
 * registre).
 * \param base      dossier d'installation de MySQL
 * \param progData  dossier de données (ProgramData)
 * \param version   version installée (affichée dans le gestionnaire)
 */
void MySQLInstaller::registerWindowsUninstaller(const QString& base,
                                                const QString& progData,
                                                const QString& version)
{
    const QString nbase = QDir::toNativeSeparators(base);
    const QString nprog = QDir::toNativeSeparators(progData);
    const QString psPath = base + "/uninstall_rufus.ps1";
    const QString nps    = QDir::toNativeSeparators(psPath);
    const QString binDir = nbase + "\\bin";

    /*! 1. Script de désinstallation (PowerShell, ré-élève si lancé sans droits). */
    QFile f(psPath);
    if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream ts(&f);
        ts << "$ErrorActionPreference = 'SilentlyContinue'\r\n"
           << "$id = [Security.Principal.WindowsIdentity]::GetCurrent()\r\n"
           << "$pr = New-Object Security.Principal.WindowsPrincipal($id)\r\n"
           << "if (-not $pr.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)) {\r\n"
           << "    Start-Process powershell -Verb RunAs -ArgumentList "
              "\"-NoProfile -ExecutionPolicy Bypass -File `\"$PSCommandPath`\"\"\r\n"
           << "    exit\r\n"
           << "}\r\n"
           << "Set-Location $env:SystemDrive\\\r\n"
           << "net stop MySQL\r\n"
           << "sc.exe delete MySQL\r\n"
           << "$bin = '" << binDir << "'\r\n"
           << "$p = [Environment]::GetEnvironmentVariable('Path','Machine')\r\n"
           << "if ($p) { [Environment]::SetEnvironmentVariable('Path', "
              "(($p -split ';' | Where-Object { $_ -and $_ -ne $bin }) -join ';'), 'Machine') }\r\n"
           << "Remove-Item -LiteralPath '" << nbase << "' -Recurse -Force\r\n"
           << "Remove-Item -LiteralPath '" << nprog << "' -Recurse -Force\r\n"
           << "Remove-Item -Path 'HKLM:\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion"
              "\\Uninstall\\MySQLForRufus' -Recurse -Force\r\n";
        f.close();
    }

    /*! 2. Entrée « Applications et fonctionnalités ». */
    QSettings reg("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion"
                  "\\Uninstall\\MySQLForRufus", QSettings::NativeFormat);
    reg.setValue("DisplayName",     QString("MySQL %1 (pour Rufus)").arg(version));
    reg.setValue("DisplayVersion",  version);
    reg.setValue("Publisher",       "Rufus");
    reg.setValue("InstallLocation", nbase);
    reg.setValue("DisplayIcon",     binDir + "\\mysqld.exe");
    reg.setValue("UninstallString",
        QString("powershell -NoProfile -ExecutionPolicy Bypass -File \"%1\"").arg(nps));
    reg.setValue("NoModify", 1);
    reg.setValue("NoRepair", 1);

    /*! Taille installée (Ko) → colonne « Taille » du gestionnaire d'applications. */
    auto dirSizeKB = [](const QString& path) -> quint64 {
        quint64 total = 0;
        QDirIterator it(path, QDir::Files | QDir::NoSymLinks,
                        QDirIterator::Subdirectories);
        while (it.hasNext()) { it.next(); total += quint64(it.fileInfo().size()); }
        return total / 1024;
    };
    reg.setValue("EstimatedSize",
                 uint(dirSizeKB(base) + dirSizeKB(progData)));
}
#endif  /*! Q_OS_WIN */

/*! ═══ run() : point d'entrée synchrone (pré-requis + détection + install éventuelle, puis les 6 critères) ═══ */
void MySQLInstaller::cleanupDialog()
{
    if (m_dialog) {
        m_dialog->close();
        delete m_dialog;
        m_dialog = nullptr;
    }
}

/*!
 * \brief MySQLInstaller::assurerDroitsAdmin
 * Garantit les droits administrateur, requis UNIQUEMENT pour (dés)installer MySQL. Si le processus n'est
 * pas élevé, propose l'élévation UAC (Windows) — l'instance élevée prend le relais (exit de la courante).
 * true si on a (ou vient d'obtenir) les droits, false si refus.
 */
bool MySQLInstaller::assurerDroitsAdmin()
{
    if (isAdminUser())
        return true;
#if defined(Q_OS_WIN)
    if (askYesNo(tr("Droits administrateur requis"),
            tr("Pour installer ou désinstaller MySQL (le moteur de base de données "
               "de Rufus), Windows demande les droits administrateur.\n\nRufus doit "
               "donc être relancé en mode administrateur. Voulez-vous le faire "
               "maintenant ?\n\nUne fenêtre Windows vous demandera alors un compte "
               "administrateur de l'ordinateur."))) {
        if (relancerEnAdministrateur())
            exit(0);                    /*!< l'instance élevée prend le relais */
        /*! UAC refusée / échec : on bascule sur la consigne manuelle. */
        UpMessageBox::Watch(nullptr, tr("Élévation refusée"),
            tr("Rufus n'a pas pu obtenir les droits administrateur.\n\n"
               "Faites un clic droit sur l'application puis « Exécuter en tant "
               "qu'administrateur », et relancez."));
    }
    return false;
#else
    UpMessageBox::Watch(nullptr, tr("Droits administrateur requis"),
        tr("Pour installer ou désinstaller MySQL (le moteur de base de données de "
           "Rufus), macOS demande un compte administrateur.\n\nConnectez-vous avec "
           "un compte administrateur (ou demandez à un administrateur de l'exécuter), "
           "puis relancez."));
    return false;
#endif
}

/*!
 * \brief MySQLInstaller::run
 * Point d'entrée synchrone : pré-requis plateforme, détection de MySQL, puis création complète
 * (faireCreate) ou réutilisation de l'existant (faireReutiliser) selon le choix de l'utilisateur.
 * Renvoie true si un MySQL conforme à Rufus est prêt.
 */
bool MySQLInstaller::run()
{
    /*! On ne vérifie PAS les droits administrateur ici : ils ne sont requis que pour (DÉS)INSTALLER
     *  MySQL. La vérification (+ offre d'élévation UAC) se fait juste avant ces opérations, via
     *  assurerDroitsAdmin() — sinon le mode Verify (simple connexion à un MySQL présent) serait bloqué
     *  pour rien sur un compte non-administrateur. */

#if defined(Q_OS_WIN)
    /*! Windows : MySQL dépend de Visual C++ Redistributable 2022. On le vérifie et l'installe AVANT toute
     *  opération MySQL. */
    if (!isVCRedist2022Installed()) {
        if (!installVCRedist2022()) {
            UpMessageBox::Watch(nullptr,
                tr("Visual C++ Redistributable requis"),
                tr("L'installation de Microsoft Visual C++ Redistributable 2022 a "
                   "échoué.\nVérifiez votre connexion Internet et relancez."));
            return false;
        }
    }
#elif defined(Q_OS_LINUX)
    /*! Linux : ce programme cible Ubuntu (≥ 22.04 LTS). */
    if (!isUbuntuVersionSupported()) {
        UpMessageBox::Watch(nullptr,
            tr("Version d'Ubuntu non compatible"),
            tr("Ce programme nécessite Ubuntu 22.04 ou une version ultérieure."));
        return false;
    }
#endif

    /*! On ne vérifie PAS l'accès réseau ici : il n'est nécessaire que pour TÉLÉCHARGER MySQL (mode
     *  Create), cas déjà couvert par checkDownloadConnectivity() dans faireCreate(). Le vérifier dès
     *  run() bloquerait pour rien un poste hors-ligne qui a DÉJÀ MySQL (Verify) ou une base complète. */

    /*! Config distante (version cible + seuil minimal). La checklist affiche le seuil dans la case « MySQL
     *  ≥ <min> installé ». fetchRemoteConfig() se replie sur les valeurs par défaut si le réseau manque. */
    const MySQLRemoteConfig cfg = fetchRemoteConfig();

    const bool installed = isMySQLInstalled();

    /*! A. Aucun MySQL → CRÉATION complète. */
    if (!installed) {
        if (!askYesNo(tr("Installation de MySQL"),
                tr("Pour installer Rufus, il est nécessaire d'installer une base de "
                   "données MySQL sur cet ordinateur.\n\nVoulez-vous l'installer "
                   "maintenant ?")))
            return false;
        if (!assurerDroitsAdmin())   /*!< l'installation exige les droits admin */
            return false;
        return faireCreate(cfg);
    }

    /*! MySQL DÉJÀ présent → on demande quoi en faire. Plus de « mode Verify » automatique : on reste en
     *  logique Create, avec ou sans (ré)installation, avec ou sans effacement des données. */
    if (!isServerRunning()) startMySQL();

    /*! Une base Rufus COMPLÈTE existe déjà ? On propose de la CONSERVER, la SUPPRIMER ou ANNULER.
     *  Conserver/Annuler → retour à PremierDemarrage() (return false) ; Supprimer → on retire les bases
     *  Rufus puis on continue le flux. */
    if (baseRufusComplete()) {
        UpMessageBox msgbox(nullptr);
        msgbox.setIcon(UpMessageBox::Quest);
        msgbox.setText(tr("Une base Rufus existe déjà"));
        msgbox.setInformativeText(tr(
            "Une base de données Rufus existe déjà sur cet ordinateur.\n\n"
            "Pour vous y connecter, choisissez « Conserver » puis, depuis l'écran de "
            "premier démarrage, « Base patients existante sur le serveur ».\n\n"
            "Voulez-vous la conserver ou la supprimer ?"));
        UpSmallButton* bAnnuler   = new UpSmallButton(tr("Annuler"));
        UpSmallButton* bSupprimer = new UpSmallButton(tr("Supprimer la base Rufus existante"));
        UpSmallButton* bConserver = new UpSmallButton(tr("Conserver la base existante"));
        msgbox.addButton(bAnnuler,   UpSmallButton::CANCELBUTTON);
        msgbox.addButton(bSupprimer, UpSmallButton::OUPSBUTTON);
        msgbox.addButton(bConserver, UpSmallButton::STARTBUTTON);
        msgbox.exec();
        if (msgbox.clickedButton() != bSupprimer)
            return false;                       /*!< Conserver / Annuler → PremierDemarrage() */

        /*! Suppression : action IRRÉVERSIBLE (données patients) → confirmation explicite. */
        if (!askYesNo(tr("Supprimer la base Rufus ?"),
                tr("Cette opération supprimera DÉFINITIVEMENT la base de données Rufus "
                   "(patients, comptabilité, ophtalmologie, imagerie) présente sur ce "
                   "serveur.\n\nCette action est IRRÉVERSIBLE. Voulez-vous continuer ?")))
            return false;                       /*!< non confirmé → PremierDemarrage() */

        /*! On retire les bases Rufus via le mot de passe qui a permis la détection (LOGIN_SQL +
         *  motDePasseSQL(), cf. baseRufusComplete), puis on continue. */
        const QString mdp = motDePasseSQL();
        const QString dropSql =
            "DROP DATABASE IF EXISTS " DB_RUFUS ";"
            "DROP DATABASE IF EXISTS " DB_COMPTA ";"
            "DROP DATABASE IF EXISTS " DB_OPHTA ";"
            "DROP DATABASE IF EXISTS " DB_IMAGES ";";
        runCmdFull(QString("\"%1\" " LOCAL_TCP_ARGS " -u \"%2\" -p\"%3\" -e \"%4\" 2>&1")
            .arg(mysqlBin("mysql"), QString(LOGIN_SQL), mdp, dropSql));
    }

    /*! Compatibilité DÉTECTÉE par Rufus (on ne demande pas à l'utilisateur de juger) : MySQL >= 8.0.14 et
     *  PAS MariaDB. */
    const QString ver = getMySQLServerVersion();
    const bool compatible = versionAtLeast(ver, seuilVersionMySQL()) && !isMariaDB();

    forever {
        switch (demanderQueFaireMySQL(compatible)) {

        case QueFaireMySQL::Annuler:
            return false;                       /*!< retour au menu précédent */

        case QueFaireMySQL::Reinstaller: {
            /*! Sert aux DEUX impasses : version incompatible, et version compatible dont on n'a pas le
             *  mot de passe. On désinstalle puis on relance Rufus, qui ne trouvera plus de serveur et
             *  installera du neuf — aucune connexion requise, aucune logique en double. */
            offrirSauvegardeAvantEffacement();  /*!< (quitte Rufus s'il veut sauvegarder) */
            if (!assurerDroitsAdmin()) continue;
            MySQLProgressDialog* clean = new MySQLProgressDialog(
                tr("Désinstallation de MySQL…"));
            clean->show();
            QApplication::processEvents();
            const bool desinstalle = uninstallMySQL();
            clean->close();
            delete clean;
            if (!desinstalle) {
                /*! Annulation (mot de passe administrateur refusé) ou échec : MySQL est TOUJOURS en place.
                 *  On NE wipe PAS ~/.rufus, on NE relance PAS, et on N'affiche PAS « MySQL supprimé » (ce
                 *  serait mentir) : retour au menu « MySQL est déjà présent ». */
                continue;
            }
            /*! On efface aussi ~/.rufus : il contient le .dbkey (mots de passe MySQL) — devenu
             *  inutile/trompeur puisqu'on repart de zéro — ainsi que les clés SSL et le journal. Sans ça,
             *  le poste réessaierait l'ancien mot de passe au redémarrage. */
            QDir(PATH_DIR_RUFUSKEY).removeRecursively();
            UpMessageBox::Watch(nullptr, tr("MySQL supprimé"),
                tr("MySQL a été supprimé. Rufus va redémarrer pour installer un serveur neuf."));
            /*! On relance AVEC le drapeau « -installMySQL » : au redémarrage, Connexion_A_La_Base() va
             *  DIRECTEMENT à l'installation d'un serveur neuf + base vierge, sans repasser par le carrefour
             *  « Aucun serveur » ni le choix vierge/existante (ergonomie après un « tout effacer et
             *  réinstaller » délibéré). Drapeau posé À CE SEUL endroit. */
            Utils::Redemarrage();
            return true;                        /*!< jamais atteint */
        }

        case QueFaireMySQL::Effacer:
            offrirSauvegardeAvantEffacement();
            if (faireReutiliser(cfg, /*effacerTout=*/true)) return true;
            break;                              /*!< échec/annulation → retour au choix */

        case QueFaireMySQL::Conserver:
            if (faireReutiliser(cfg, /*effacerTout=*/false)) return true;
            break;                              /*!< échec/annulation → retour au choix */
        }
    }
}

/*! ═══ Boîte « MySQL est déjà présent… que voulez-vous faire ? » ═══════════════════════════════════ */
MySQLInstaller::QueFaireMySQL MySQLInstaller::demanderQueFaireMySQL(bool compatible)
{
    UpMessageBox msgbox(nullptr);
    msgbox.setIcon(UpMessageBox::Quest);
    msgbox.setText(tr("MySQL est déjà présent sur cet ordinateur"));

    UpSmallButton* bAnnuler = new UpSmallButton(tr("Annuler (retour au menu)"));

    if (compatible) {
        msgbox.setInformativeText(tr(
            "Cette version de MySQL est compatible avec Rufus.\n\n"
            "Que voulez-vous faire ?"));
        UpSmallButton* bEffacer   = new UpSmallButton(
            tr("L'utiliser en EFFAÇANT toutes les données déjà présentes"));
        UpSmallButton* bConserver = new UpSmallButton(
            tr("L'utiliser en CONSERVANT les données déjà présentes"));
        /*! Échappatoire pour une base compatible dont on n'a PAS le mot de passe : « Effacer » et
         *  « Conserver » exigent de se connecter (drop / configuration). Sans identifiants valides, on
         *  serait bloqué → on offre la réinstallation COMPLÈTE (désinstall + install neuve), sans
         *  connexion. Réutilise le chemin Reinstaller. */
        UpSmallButton* bReinstall = new UpSmallButton(
            tr("Réinstaller entièrement MySQL\n(si le mot de passe est perdu)"));
        msgbox.addButton(bAnnuler,   UpSmallButton::CANCELBUTTON);
        msgbox.addButton(bReinstall, UpSmallButton::OUPSBUTTON);
        msgbox.addButton(bEffacer,   UpSmallButton::NOBUTTON);
        msgbox.addButton(bConserver, UpSmallButton::STARTBUTTON);
        msgbox.exec();
        if (msgbox.clickedButton() == bEffacer)   return QueFaireMySQL::Effacer;
        if (msgbox.clickedButton() == bConserver) return QueFaireMySQL::Conserver;
        if (msgbox.clickedButton() == bReinstall) return QueFaireMySQL::Reinstaller;
        return QueFaireMySQL::Annuler;
    }

    msgbox.setInformativeText(tr(
        "Cette version de MySQL n'est PAS compatible avec Rufus (MariaDB, ou version "
        "antérieure à 8.0.14).\n\nQue voulez-vous faire ?"));
    UpSmallButton* bReinstall = new UpSmallButton(
        tr("Tout effacer et réinstaller MySQL"));
    msgbox.addButton(bAnnuler,   UpSmallButton::CANCELBUTTON);
    msgbox.addButton(bReinstall, UpSmallButton::STARTBUTTON);
    msgbox.exec();
    if (msgbox.clickedButton() == bReinstall) return QueFaireMySQL::Reinstaller;
    return QueFaireMySQL::Annuler;
}

/*!
 * \brief MySQLInstaller::isMariaDB
 * Le serveur MySQL local est-il MariaDB ? (incompatible : pas de RETAIN / double mot de passe.)
 */
bool MySQLInstaller::isMariaDB()
{
    /*! Fiable : la chaîne VERSION() du serveur en cours contient « MariaDB » si c'en est un. Le client
     *  mysql posé par l'installeur étant un vrai MySQL, l'interroger LUI dirait « pas MariaDB » même face
     *  à un serveur MariaDB — d'où la priorité au serveur (cf. serverVersionString). */
    const QString srv = serverVersionString();
    if (!srv.isEmpty())
        return srv.contains("mariadb", Qt::CaseInsensitive);

    /*! Repli (serveur injoignable) : binaire mysqld puis client. Timeout court (blocage AppImage). */
    QString out = runCmd("\"" + mysqlBin("mysqld") + "\" --version " + NUL(), 5000);
    if (out.contains("mariadb", Qt::CaseInsensitive)) return true;
    out = runCmd("\"" + mysqlBin("mysql") + "\" --version " + NUL(), 5000);
    return out.contains("mariadb", Qt::CaseInsensitive);
}

/*!
 * \brief MySQLInstaller::offrirSauvegardeAvantEffacement
 * Avertit que des données NON-Rufus vont être effacées ; propose d'arrêter Rufus pour les sauvegarder
 * soi-même (→ exit), sinon on continue.
 */
void MySQLInstaller::offrirSauvegardeAvantEffacement()
{
    UpMessageBox msgbox(m_dialog);
    msgbox.setIcon(UpMessageBox::Warning);
    msgbox.setText(tr("Des données vont être effacées"));
    msgbox.setInformativeText(tr(
        "Cette opération va supprimer des données présentes sur ce serveur MySQL "
        "qui ne sont PAS gérées par Rufus.\n\n"
        "Si ces données vous importent, arrêtez Rufus pour les sauvegarder vous-même "
        "avant de poursuivre."));
    UpSmallButton* bArreter   = new UpSmallButton(
        tr("Arrêter Rufus pour sauvegarder d'abord"));
    UpSmallButton* bContinuer = new UpSmallButton(
        tr("Continuer (effacer les données)"));
    msgbox.addButton(bArreter,   UpSmallButton::CANCELBUTTON);
    msgbox.addButton(bContinuer, UpSmallButton::STARTBUTTON);
    msgbox.exec();
    if (msgbox.clickedButton() == bContinuer)
        return;
    exit(0);   /*!< « Arrêter » : on quitte pour laisser l'utilisateur sauvegarder */
}

/*!
 * \brief MySQLInstaller::effacerToutesBasesUtilisateur
 * Supprime toutes les bases NON système (les bases Rufus seront recréées vierges par RestaureBase).
 * Utilisé pour le choix « Effacer ».
 * \param adminLogin  compte admin MySQL utilisé pour le DROP
 * \param adminMdp    son mot de passe
 */
void MySQLInstaller::effacerToutesBasesUtilisateur(const QString& adminLogin,
                                                   const QString& adminMdp)
{
    const QString out = runCmdFull(
        QString("\"%1\" " LOCAL_TCP_ARGS " -u \"%2\" -p\"%3\" -N -B -e "
                "\"SHOW DATABASES;\" 2>&1")
            .arg(mysqlBin("mysql"), adminLogin, adminMdp));
    QString sql;
    for (const QString& db : lignesResultat(out)) {
        if (db.startsWith("ERROR", Qt::CaseInsensitive))
            continue;
        if (db == "mysql" || db == "information_schema"
            || db == "performance_schema" || db == "sys")
            continue;
        sql += "DROP DATABASE IF EXISTS `" + db + "`;";
    }
    if (sql.isEmpty())
        return;
    runCmdFull(QString("\"%1\" " LOCAL_TCP_ARGS " -u \"%2\" -p\"%3\" -e \"%4\" 2>&1")
        .arg(mysqlBin("mysql"), adminLogin, adminMdp, sql));
}

/*!
 * \brief inviterANoterMotDePasse
 * Invite à CONSERVER le mot de passe (aléatoire) de connexion à la base : nécessaire pour brancher un
 * autre poste au cabinet ou dépanner l'ordinateur. Appelée après chaque création/rotation du mot de
 * passe (install neuve, réutilisation, migration, sécurisation).
 * \param mdp  mot de passe à afficher / faire noter
 */
static void inviterANoterMotDePasse(const QString& mdp)
{
    /*! Le mot de passe est mis en évidence (propre ligne, centré, gras, rouge, police agrandie) pour
     *  faciliter sa transcription. InfoText est interprété en HTML par UpMessageBox (UpTextEdit::setHtml) ;
     *  le mot de passe est purement alphanumérique (cf. genererMotDePasse), donc sans risque d'injection.
     *  Le <b> sert aussi à CalcSize (correction de hauteur 1.2), réservant la place de la police agrandie. */
    UpMessageBox::Watch(nullptr,
        QObject::tr("Notez le mot de passe de la base de données"),
        QObject::tr("Rufus a créé un mot de passe de connexion à votre base de données patients.\n"
                    "Conservez-le en lieu sûr (sur papier ou sur une clé USB) : il est nécessaire "
                    "pour connecter un autre poste au cabinet, ou pour dépanner cet ordinateur.")
        + "\n\n" + QObject::tr("Mot de passe :")
        + "\n\n" + "<p align=\"center\"><b><span style=\"color:#c00000; font-size:14pt;\">"
                 + mdp + "</span></b></p>"
        + "\n\n" + QObject::tr("Vous pourrez aussi l'enregistrer sur une clé USB à tout moment "
                               "depuis Paramètres ▸ onglet « Ce poste »."));
}

/*!
 * \brief recolterClesClientSSL
 * Copie les SEULES clés CLIENT auto-générées par MySQL vers PATH_DIR_CLESSSL_SERVEUR, d'où elles seront
 * exportées vers les postes distants — jamais les clés serveur ni ca-key. Best-effort.
 * \param datadir  répertoire de données MySQL où sont déposés les .pem
 */
[[maybe_unused]] static bool recolterClesClientSSL(const QString& datadir)  /*!< appelée selon l'OS (Windows ici) */
{
    const QString dest = PATH_DIR_CLESSSL_SERVEUR;
    if (!QDir().mkpath(dest))
        return false;
    /*! { fichier dans le datadir, nom de destination } — UNIQUEMENT les clés client. */
    const char* paires[3][2] = {
        { "ca.pem",          "ca-cert.pem"     },   /*!< ca.pem -> ca-cert.pem */
        { "client-cert.pem", "client-cert.pem" },
        { "client-key.pem",  "client-key.pem"  },
    };
    bool ok = true;
    for (int i = 0; i < 3; ++i) {
        const QString src = datadir + "/" + QString::fromLatin1(paires[i][0]);
        const QString dst = dest    + "/" + QString::fromLatin1(paires[i][1]);
        if (!QFile::exists(src)) { ok = false; continue; }
        QFile::remove(dst);                          /*!< QFile::copy échoue si la cible existe déjà */
        if (!QFile::copy(src, dst)) ok = false;
    }
    return ok;
}

/*!
 * \brief mysqlDataDir
 * Répertoire de données (datadir) du MySQL installé par Rufus, selon l'OS — là où sont déposés les
 * certificats SSL auto-générés. Utilisé par la conservation des clés lors d'un remplacement de socle.
 */
[[maybe_unused]] static QString mysqlDataDir()
{
#if defined(Q_OS_WIN)
    return "C:/ProgramData/MySQL/MySQL Server 8.4/Data";
#elif defined(Q_OS_MACOS)
    return "/usr/local/mysql/data";
#else
    return "/var/lib/mysql";
#endif
}

/*!
 * \brief avertirSecurisationMiseEnPlace
 * Message affiché quand CE poste vient de SÉCURISER la base (pose du mot de passe aléatoire). Prévient
 * que les AUTRES postes du réseau devront être mis à jour dans le délai avant la purge de gaxt78iy.
 * Rédigé au conditionnel (« s'il existe d'autres postes… ») : inoffensif en monoposte pur.
 */
static void avertirSecurisationMiseEnPlace()
{
    UpMessageBox::Watch(nullptr,
        QObject::tr("Sécurisation de la base de données"),
        QObject::tr("IMPORTANT : un mot de passe sécurisé vient d'être mis en place.") + "\n\n"
        + QObject::tr("S'il existe d'autres postes sur le réseau local qui utilisent Rufus, "
                      "ils devront IMPÉRATIVEMENT être mis à jour vers cette nouvelle version "
                      "de Rufus dans un délai d'un mois.") + "\n"
        + QObject::tr("Sans cette mise à jour, au-delà de ce délai, ils ne pourront plus "
                      "utiliser Rufus."));
}

/*!
 * \brief confirmerSuppressionGaxt78iy
 * Demande l'AUTORISATION de supprimer le mot de passe générique (gaxt78iy) une fois la deadline atteinte.
 * true si l'utilisateur confirme, false s'il préfère reporter (reproposé à chaque connexion).
 */
static bool confirmerSuppressionGaxt78iy()
{
    UpMessageBox msgbox(nullptr);
    msgbox.setIcon(UpMessageBox::Warning);
    msgbox.setText(QObject::tr("Suppression du mot de passe générique"));
    msgbox.setInformativeText(
        QObject::tr("IMPORTANT : le mot de passe générique d'accès à la base de données va "
                    "être supprimé.") + "\n\n"
        + QObject::tr("S'il existe d'autres postes sur le réseau local qui utilisent une "
                      "version plus ancienne du programme, ils doivent IMPÉRATIVEMENT être "
                      "mis à jour vers cette nouvelle version pour pouvoir continuer à "
                      "utiliser Rufus."));
    UpSmallButton* bAnnuler   = new UpSmallButton(QObject::tr("Annuler et reporter cette suppression"));
    UpSmallButton* bSupprimer = new UpSmallButton(QObject::tr("OK, supprimer le mot de passe"));
    msgbox.addButton(bAnnuler,   UpSmallButton::CANCELBUTTON);
    msgbox.addButton(bSupprimer, UpSmallButton::STARTBUTTON);
    msgbox.exec();
    return msgbox.clickedButton() == bSupprimer;
}

/*! Message affiché juste APRÈS la suppression effective du mot de passe générique. */
static void avertirSuppressionGaxt78iyEffectuee()
{
    UpMessageBox::Watch(nullptr,
        QObject::tr("Mot de passe générique supprimé"),
        QObject::tr("IMPORTANT : le mot de passe générique d'accès à la base de données vient "
                    "d'être supprimé.") + "\n\n"
        + QObject::tr("S'il existe d'autres postes sur le réseau local qui utilisent une "
                      "version plus ancienne du programme, ils doivent IMPÉRATIVEMENT être "
                      "mis à jour vers cette nouvelle version pour pouvoir continuer à "
                      "utiliser Rufus."));
}

/*!
 * \brief demanderNouvelUtilisateurRufus
 * Petite fiche DÉDIÉE à la saisie du futur utilisateur Rufus (identifiant / mot de passe / confirmation),
 * après la (ré)installation. On n'utilise PAS MySQLInstallerDialog (sa checklist n'a plus de sens ici).
 * true si validé.
 * \param outLogin  reçoit l'identifiant saisi
 * \param outMdp    reçoit le mot de passe saisi
 * \param parent    fenêtre parente
 */
static bool demanderNouvelUtilisateurRufus(QString& outLogin, QString& outMdp, QWidget* parent)
{
    /*! Design repris de dlg_gestionusers::CreerUser() : libellés explicatifs centrés au-dessus de chaque
     *  champ (login / mot de passe / confirmation), champs centrés, mots de passe masqués. */
    UpDialog dlg(parent);
    dlg.setWindowModality(Qt::WindowModal);
    dlg.setFixedSize(300, 300);
    dlg.setWindowTitle("");

    UpLabel*    label  = new UpLabel();
    UpLabel*    label2 = new UpLabel();
    UpLabel*    label3 = new UpLabel();
    UpLineEdit* Line   = new UpLineEdit();
    UpLineEdit* Line2  = new UpLineEdit();
    UpLineEdit* Line3  = new UpLineEdit();

    Line  ->setValidator(new QRegularExpressionValidator(Utils::rgx_AlphaNumeric_5_15, &dlg));
    Line2 ->setValidator(new QRegularExpressionValidator(Utils::rgx_AlphaNumeric_5_12, &dlg));
    Line3 ->setValidator(new QRegularExpressionValidator(Utils::rgx_AlphaNumeric_5_12, &dlg));
    Line  ->setAlignment(Qt::AlignCenter);
    Line2 ->setAlignment(Qt::AlignCenter);
    Line3 ->setAlignment(Qt::AlignCenter);
    Line  ->setMaxLength(15);
    Line2 ->setMaxLength(12);
    Line3 ->setMaxLength(12);
    Line  ->setFixedHeight(20);
    Line2 ->setFixedHeight(20);
    Line3 ->setFixedHeight(20);
    Line2 ->setEchoMode(QLineEdit::Password);
    Line3 ->setEchoMode(QLineEdit::Password);
    label  ->setMinimumHeight(46);
    label2 ->setMinimumHeight(46);
    label3 ->setFixedHeight(16);
    label  ->setAlignment(Qt::AlignCenter);
    label2 ->setAlignment(Qt::AlignCenter);
    label3 ->setAlignment(Qt::AlignCenter);

    label  ->setText(QObject::tr("Choisissez un login pour le nouvel utilisateur\n- mini 5 maxi 15 caractères -\n- pas de caractères spéciaux ou accentués -"));
    label2 ->setText(QObject::tr("Choisissez un mot de passe\n- mini 5 maxi 12 caractères -\n- pas de caractères spéciaux ou accentués -"));
    label3 ->setText(QObject::tr("Confirmez le mot de passe"));

    QVBoxLayout* lay = new QVBoxLayout();
    lay ->setContentsMargins(5, 5, 5, 5);
    lay ->setSpacing(5);
    lay ->addWidget(label);
    lay ->addWidget(Line);
    lay ->addWidget(label2);
    lay ->addWidget(Line2);
    lay ->addWidget(label3);
    lay ->addWidget(Line3);
    lay ->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding));

    dlg.AjouteLayButtons(UpDialog::ButtonCancel | UpDialog::ButtonOK);
    QObject::connect(dlg.OKButton,     &UpSmallButton::clicked, &dlg, &UpDialog::accept);
    QObject::connect(dlg.CancelButton, &UpSmallButton::clicked, &dlg, &UpDialog::reject);

    dlg.dlglayout()->insertLayout(0, lay);   /*!< contenu AU-DESSUS de la barre de boutons */
    Line->setFocus();

    forever {
        if (dlg.exec() != QDialog::Accepted)
            return false;
        const QString login = Line->text().trimmed();
        const QString mdp   = Line2->text();
        if (login.isEmpty() || mdp.isEmpty()) {
            UpMessageBox::Watch(&dlg, QObject::tr("Saisie incomplète"),
                QObject::tr("Veuillez renseigner un identifiant et un mot de passe."));
            continue;
        }
        if (mdp != Line3->text()) {
            UpMessageBox::Watch(&dlg, QObject::tr("Mots de passe différents"),
                QObject::tr("Le mot de passe et sa confirmation ne correspondent pas."));
            continue;
        }
        outLogin = login;
        outMdp   = mdp;
        return true;
    }
}

/*!
 * \brief MySQLInstaller::faireReutiliser
 * RÉUTILISE un MySQL existant compatible (choix Effacer/Conserver) : demande un compte admin MySQL, crée
 * adminrufus/SSL, (option : efface les bases non-Rufus), déroule la config, puis la saisie du futur
 * utilisateur Rufus.
 * \param effacerTout  true = supprime aussi les bases non-Rufus (choix « Effacer »)
 */
bool MySQLInstaller::faireReutiliser(const MySQLRemoteConfig& /*cfg*/, bool effacerTout)
{
    m_dialog = new MySQLInstallerDialog();
    m_dialog->configurerVerifyAdminMySQL();

    /*! Le clic sur OK fait TOUT d'un trait, sans jamais fermer ni rouvrir la fiche : vérifie la connexion
     *  admin (coche la case 0) puis enchaîne la config (cases 1 à 6). accept() seulement à la toute fin,
     *  quand tout est coché — sinon la fiche se fermerait/rouvrirait entre deux cases (clignotement).
     *  Connexion refusée → fiche laissée ouverte pour réessayer ; erreur bloquante → reject(). */
    QString adminLogin, adminMdp;
    bool configReussie = false;
    QObject::disconnect(m_dialog->OKButton, &QPushButton::clicked, nullptr, nullptr);
    connect(m_dialog->OKButton, &QPushButton::clicked, m_dialog, [&] {
        if (!m_dialog->validerSaisie()) return;
        if (!tryConnectAs(m_dialog->login(), m_dialog->password())) {
            UpMessageBox::Watch(m_dialog, tr("Connexion impossible"),
                tr("Connexion refusée avec cet identifiant / mot de passe. Réessayez."));
            return;                                 /*!< fiche ouverte : on réessaie */
        }
        m_dialog->checkStep(0);

        adminLogin = m_dialog->login();
        adminMdp   = m_dialog->password();
        m_login    = LOGIN_SQL;
        m_password = genererMotDePasse();
        if (!isServerRunning()) startMySQL();

        const CreateUserResult r = createUserAvecAdmin(adminLogin, adminMdp);
        if (r == CreateUserResult::NoCreateUserRight) {
            UpMessageBox::Watch(m_dialog, tr("Droits insuffisants"),
                tr("Le compte MySQL « %1 » n'a pas le droit de créer des utilisateurs "
                   "(CREATE USER). Réessayez avec un compte administrateur MySQL "
                   "(par ex. root).").arg(adminLogin));
            m_dialog->reject();                     /*!< → retour à la boîte de choix */
            return;
        }
        if (r != CreateUserResult::Ok) {
            UpMessageBox::Watch(m_dialog, tr("Erreur"),
                tr("Impossible de créer les comptes Rufus."));
            m_dialog->reject();
            return;
        }
        /*! adminrufus/adminrufusSSL créés : on mémorise leur mot de passe aléatoire. */
        stockerMotDePasse(m_password);
        m_comptesDejaCrees = true;

        /*! EFFACER : on supprime aussi les bases NON-Rufus (données étrangères) ; les bases Rufus sont
         *  (re)créées vierges par RestaureBase. CONSERVER : on n'y touche pas. */
        if (effacerTout)
            effacerToutesBasesUtilisateur(adminLogin, adminMdp);

        if (!executerEtapesConfig()) { m_dialog->reject(); return; }

        configReussie = true;
        m_dialog->accept();                         /*!< tout est coché : on ferme enfin */
    });

    m_dialog->exec();
    if (!configReussie) { cleanupDialog(); return false; }

    /*! Saisie du futur utilisateur applicatif Rufus (2e étape) : on FERME d'abord la fiche d'installation
     *  (avec sa checklist) puis on ouvre une petite fiche dédiée — plus de cases sans rapport. */
    cleanupDialog();
    if (!demanderNouvelUtilisateurRufus(m_loginRufus, m_mdpRufus, nullptr))
        return false;

    return true;
}

/*!
 * \brief MySQLInstaller::faireCreate
 * Chemin « création » : MySQL absent (ou trop vieux après nettoyage). Saisie du futur utilisateur Rufus,
 * installation de MySQL, création d'adminrufus (root/pkexec) puis configuration. true si la base Rufus
 * est prête.
 * \param cfg  config distante (version cible, seuil, URLs de téléchargement)
 */
bool MySQLInstaller::faireCreate(const MySQLRemoteConfig& cfg)
{
    m_dialog = new MySQLInstallerDialog();
    m_dialog->configurerCreateUserRufus(cfg.minVersion);
    m_dialog->show();
    QApplication::processEvents();

    /*! Saisie du futur utilisateur applicatif Rufus. */
    forever {
        if (m_dialog->exec() != QDialog::Accepted) { cleanupDialog(); return false; }
        if (m_dialog->validerSaisie()) break;
    }
    m_loginRufus = m_dialog->login();
    m_mdpRufus   = m_dialog->password();

    /*! Le login/mdp Rufus est choisi : la fiche passe en « paramétrage en cours » (champs grisés, plus de
     *  bouton OK). Installation puis config SANS clic — seule une éventuelle saisie du code administrateur
     *  (élévation système) peut être demandée. On la ré-affiche pour que la checklist se coche visiblement. */
    m_dialog->passerEnConfiguration(
        tr("Installation de MySQL"),
        tr("Téléchargement et installation du serveur MySQL en cours…"));
    m_dialog->show();
    QApplication::processEvents();

    /*! Comptes Rufus techniques : login fixe + mot de passe aléatoire (fixé AVANT
     *  createUser/prepareCreateMode* qui s'en servent). */
    m_login    = LOGIN_SQL;
    m_password = genererMotDePasse();

    /*! Pré-requis réseau : sans accès WAN ou si le lien de téléchargement ne se résout pas, l'installation
     *  est impossible. */
    QString dlUrl;
#if defined(Q_OS_WIN)
    dlUrl = cfg.winUrl;
#elif defined(Q_OS_MACOS)
    dlUrl = (runCmd("uname -m 2>/dev/null").trimmed() == "arm64")
                ? cfg.macArm64Url : cfg.macX86Url;
#else
    /*! Linux (apt) : à défaut d'URL directe, on contrôle au moins la résolution de l'hôte officiel MySQL. */
    dlUrl = cfg.winUrl;
#endif
    if (!checkDownloadConnectivity(dlUrl)) { cleanupDialog(); return false; }

    if (!installMySQL()) {
        /*! installMySQL() affiche déjà un message détaillé en cas d'échec. */
        cleanupDialog();
        return false;
    }
    m_freshInstall = true;
    startMySQL();
    m_dialog->checkStep(0);

    /*! MySQL est installé : on l'annonce et on bascule sur le libellé « paramétrage ». */
    m_dialog->passerEnConfiguration(
        tr("Serveur MySQL installé"),
        tr("Paramétrage de l'installation pour Rufus en cours…"));

    if (!executerEtapesConfig()) { cleanupDialog(); return false; }

    /*! Comptes adminrufus/adminrufusSSL créés avec le mot de passe aléatoire : on le mémorise dans le
     *  fichier caché pour les connexions ultérieures. */
    stockerMotDePasse(m_password);

    cleanupDialog();
    return true;
}

/*!
 * \brief MySQLInstaller::reinstallerSocleMySQL
 * Réinstallation du SOCLE MySQL pour une MIGRATION : installe le serveur puis crée adminrufus/adminrufusSSL
 * (aléatoire + gaxt78iy en 2e), SANS nouvel utilisateur applicatif ni base vierge — les bases Rufus sont
 * restaurées ENSUITE par RestaureBase. C'est la section « install » de faireCreate(), sans la saisie.
 * \param cfg  config distante (version cible, seuil, URLs de téléchargement)
 */
bool MySQLInstaller::reinstallerSocleMySQL(const MySQLRemoteConfig& cfg)
{
    m_dialog = new MySQLInstallerDialog();
    m_dialog->configurerCreateUserRufus(cfg.minVersion);
    m_dialog->masquerSaisieUtilisateur();   /*!< migration : pas de saisie d'un nouvel utilisateur */
    m_dialog->passerEnConfiguration(
        tr("Réinstallation de MySQL"),
        tr("Installation du serveur MySQL en cours…"));
    m_dialog->show();
    QApplication::processEvents();

    m_login    = LOGIN_SQL;
    m_password = genererMotDePasse();

    QString dlUrl;
#if defined(Q_OS_WIN)
    dlUrl = cfg.winUrl;
#elif defined(Q_OS_MACOS)
    dlUrl = (runCmd("uname -m 2>/dev/null").trimmed() == "arm64") ? cfg.macArm64Url : cfg.macX86Url;
#else
    dlUrl = cfg.winUrl;
#endif
    if (!checkDownloadConnectivity(dlUrl)) { cleanupDialog(); return false; }

    if (!installMySQL())          { cleanupDialog(); return false; }
    m_freshInstall = true;
    startMySQL();
    m_dialog->checkStep(0);       /*!< « MySQL X installé » (cf. faireCreate/faireVerify) */
    if (!executerEtapesConfig())  { cleanupDialog(); return false; }
    stockerMotDePasse(m_password);
    cleanupDialog();
    inviterANoterMotDePasse(m_password);
    return true;
}

/*!
 * \brief MySQLInstaller::sauvegarderClesSSLMigration
 * Met les .pem du datadir à l'abri dans ~/.rufus avant une désinstallation, qui détruit le datadir. On
 * garde TOUTE la chaîne pour que le nouveau serveur présente le même CA : les clés déjà distribuées aux
 * postes distants restent alors valides. true si au moins le CA est sauvé.
 */
bool MySQLInstaller::sauvegarderClesSSLMigration()
{
    const QString datadir = mysqlDataDir();
    const QString stash   = QString(PATH_DIR_RUFUSKEY) + "/ssl_migration";
    const QStringList pem  = QStringList()
        << "ca.pem" << "ca-key.pem" << "server-cert.pem"
        << "server-key.pem" << "client-cert.pem" << "client-key.pem";
#if defined(Q_OS_WIN)
    /*! Datadir lisible par le processus (élevé) : copie directe en C++. */
    QDir().mkpath(stash);
    for (const QString& f : pem) {
        const QString dst = stash + "/" + f;
        QFile::remove(dst);
        if (QFile::exists(datadir + "/" + f))
            QFile::copy(datadir + "/" + f, dst);
    }
#else
    /*! Datadir root-only (mysql/_mysql, clés en 0600) : copie ÉLEVÉE puis restitution à l'utilisateur. */
    const QString owner = QString::fromLocal8Bit(qgetenv("USER"));
    QString sh = "DATA='" + datadir + "'; STASH='" + stash + "'; OWNER='" + owner + "'\n"
                 "mkdir -p \"$STASH\"\n";
    for (const QString& f : pem)
        sh += "[ -f \"$DATA/" + f + "\" ] && cp -f \"$DATA/" + f + "\" \"$STASH/" + f + "\"\n";
    sh += "[ -n \"$OWNER\" ] && chown -R \"$OWNER\" \"$STASH\"\n";
    runCmdElevated(sh);
#endif
    return QFile::exists(stash + "/ca.pem");
}

/*!
 * \brief MySQLInstaller::restaurerClesSSLMigration
 * Réinjecte les .pem conservés dans le NOUVEAU datadir, fixe droits/propriétaire, redémarre MySQL (→ il
 * sert le MÊME CA) et réactualise la copie CLIENT rendue à l'utilisateur (PATH_DIR_CLESSSL_SERVEUR). Ne
 * fait rien si aucun stash n'existe. Nettoie le stash ensuite.
 */
void MySQLInstaller::restaurerClesSSLMigration()
{
    const QString datadir = mysqlDataDir();
    const QString stash   = QString(PATH_DIR_RUFUSKEY) + "/ssl_migration";
    if (!QFile::exists(stash + "/ca.pem"))
        return;
    const QStringList pem  = QStringList()
        << "ca.pem" << "ca-key.pem" << "server-cert.pem"
        << "server-key.pem" << "client-cert.pem" << "client-key.pem";
    const QString sslDest = QString(PATH_DIR_CLESSSL_SERVEUR);
#if defined(Q_OS_WIN)
    runCmdElevated("net stop MySQL");
    for (const QString& f : pem) {
        const QString src = stash + "/" + f;
        if (!QFile::exists(src)) continue;
        const QString dst = datadir + "/" + f;
        QFile::remove(dst);
        QFile::copy(src, dst);
    }
    runCmdElevated("net start MySQL");
    waitForMySQL(15);
    recolterClesClientSSL(datadir);            /*!< la copie client reflète les ANCIENNES clés */
#else
    const QString user = QString::fromLocal8Bit(qgetenv("USER"));
    QString sh = "DATA='" + datadir + "'; STASH='" + stash + "'; SSLDEST='" + sslDest + "'; USERN='" + user + "'\n";
  #if defined(Q_OS_MACOS)
    sh += "OWNER=$(id -u _mysql >/dev/null 2>&1 && echo _mysql || echo mysql)\n"
          "PLIST=/Library/LaunchDaemons/com.oracle.oss.mysql.mysqld.plist\n"
          "[ -f \"$PLIST\" ] && launchctl unload \"$PLIST\" 2>/dev/null\n";
  #else
    sh += "OWNER=mysql\n"
          "systemctl stop mysql 2>/dev/null\n";
  #endif
    for (const QString& f : pem)
        sh += "[ -f \"$STASH/" + f + "\" ] && cp -f \"$STASH/" + f + "\" \"$DATA/" + f + "\"\n";
    sh += "for k in ca-key.pem server-key.pem client-key.pem; do chmod 600 \"$DATA/$k\" 2>/dev/null; done\n"
          "for c in ca.pem server-cert.pem client-cert.pem; do chmod 644 \"$DATA/$c\" 2>/dev/null; done\n"
          "chown \"$OWNER\":\"$OWNER\" \"$DATA\"/ca.pem \"$DATA\"/ca-key.pem \"$DATA\"/server-cert.pem "
          "\"$DATA\"/server-key.pem \"$DATA\"/client-cert.pem \"$DATA\"/client-key.pem 2>/dev/null\n"
          /*! réactualiser la copie CLIENT rendue à l'utilisateur (mêmes clés qu'avant la migration) */
          "mkdir -p \"$SSLDEST\"\n"
          "cp -f \"$STASH/ca.pem\"          \"$SSLDEST/ca-cert.pem\"     2>/dev/null\n"
          "cp -f \"$STASH/client-cert.pem\" \"$SSLDEST/client-cert.pem\" 2>/dev/null\n"
          "cp -f \"$STASH/client-key.pem\"  \"$SSLDEST/client-key.pem\"  2>/dev/null\n"
          "[ -n \"$USERN\" ] && { chown \"$USERN\" \"$(dirname \"$SSLDEST\")\"; chown -R \"$USERN\" \"$SSLDEST\"; }\n"
          "chmod 600 \"$SSLDEST/client-key.pem\" 2>/dev/null\n";
  #if defined(Q_OS_MACOS)
    sh += "[ -f \"$PLIST\" ] && launchctl load -w \"$PLIST\" 2>/dev/null || "
          "/usr/local/mysql/support-files/mysql.server restart 2>/dev/null\n";
  #else
    sh += "systemctl start mysql 2>/dev/null\n";
  #endif
    runCmdElevated(sh);
    waitForMySQL(15);
#endif
    QDir(stash).removeRecursively();
}

/*! ── SSL — contrôle et (re)génération des clés (serveur / monoposte) ──────────── */

/*!
 * \brief MySQLInstaller::clesSSLServeurPresentes
 * La copie CLIENT exportable (récoltée du datadir) est-elle présente ? Ce sont les 3 fichiers qu'on
 * distribue aux postes distants.
 */
bool MySQLInstaller::clesSSLServeurPresentes()
{
    const QString d = QString(PATH_DIR_CLESSSL_SERVEUR);
    return QFile::exists(d + "/ca-cert.pem")
        && QFile::exists(d + "/client-cert.pem")
        && QFile::exists(d + "/client-key.pem");
}

/*!
 * \brief MySQLInstaller::dateExpirationCertSSL
 * Date d'expiration du certificat SERVEUR, lue via l'état SSL du serveur (aucun accès au datadir requis).
 * QDateTime INVALIDE si SSL inactif / certificat absent / illisible.
 */
QDateTime MySQLInstaller::dateExpirationCertSSL()
{
    bool ok = false;
    const QVariantList r = DataBase::I()->getFirstRecordFromStandardSelectSQL(
        "SHOW STATUS LIKE 'Ssl_server_not_after'", ok);
    if (!ok || r.size() < 2)
        return QDateTime();
    QString v = r.at(1).toString().trimmed();   /*!< p.ex. « Apr 14 12:00:00 2034 GMT » */
    if (v.isEmpty())
        return QDateTime();
    v.remove(" GMT").remove(" UTC");
    v = v.simplified();                          /*!< « Apr 1 … » (jour sur 1 chiffre) → un seul espace */
    QDateTime dt = QLocale(QLocale::C).toDateTime(v, "MMM d HH:mm:ss yyyy");
    if (dt.isValid()) dt.setTimeSpec(Qt::UTC);
    return dt;
}

/*!
 * \brief MySQLInstaller::extraireClesSSLDepuisDatadir
 * (Ré)extrait les clés CLIENT du datadir vers PATH_DIR_CLESSSL_SERVEUR SANS arrêter le serveur (les .pem
 * restent lisibles pendant qu'il tourne → AUCUNE coupure). Cas d'usage : la copie exportable a été
 * effacée par erreur alors que le serveur a toujours ses certificats. true si la copie client est là.
 */
bool MySQLInstaller::extraireClesSSLDepuisDatadir()
{
    const QString datadir = mysqlDataDir();
#if defined(Q_OS_WIN)
    recolterClesClientSSL(datadir);   /*!< datadir lisible par le process élevé → copie C++ directe */
#else
    /*! Datadir root-only (mysql/_mysql, client-key.pem en 0600) : extraction ÉLEVÉE puis restitution. */
    const QString sslDest = QString(PATH_DIR_CLESSSL_SERVEUR);
    const QString user    = QString::fromLocal8Bit(qgetenv("USER"));
    QString sh =
        "DATA='" + datadir + "'; SSLDEST='" + sslDest + "'; USERN='" + user + "'\n"
        "mkdir -p \"$SSLDEST\"\n"
        "cp -f \"$DATA/ca.pem\"          \"$SSLDEST/ca-cert.pem\"     2>/dev/null\n"
        "cp -f \"$DATA/client-cert.pem\" \"$SSLDEST/client-cert.pem\" 2>/dev/null\n"
        "cp -f \"$DATA/client-key.pem\"  \"$SSLDEST/client-key.pem\"  2>/dev/null\n"
        "[ -n \"$USERN\" ] && { chown \"$USERN\" \"$(dirname \"$SSLDEST\")\"; chown -R \"$USERN\" \"$SSLDEST\"; }\n"
        "chmod 600 \"$SSLDEST/client-key.pem\" 2>/dev/null\n";
    runCmdElevated(sh);
#endif
    return clesSSLServeurPresentes();
}

/*!
 * \brief MySQLInstaller::regenererClesSSL
 * DESTRUCTIF : supprime les certificats du datadir et laisse MySQL les recréer au redémarrage. Invalide
 * toutes les clés déjà distribuées, et coupe la connexion : l'appelant doit prévenir et relancer Rufus.
 */
bool MySQLInstaller::regenererClesSSL()
{
    const QString datadir = mysqlDataDir();
    const QStringList pem  = QStringList()
        << "ca.pem" << "ca-key.pem" << "server-cert.pem" << "server-key.pem"
        << "client-cert.pem" << "client-key.pem" << "private_key.pem" << "public_key.pem";
#if defined(Q_OS_WIN)
    runCmdElevated("net stop MySQL");
    for (const QString& f : pem)
        QFile::remove(datadir + "/" + f);
    runCmdElevated("net start MySQL");
    waitForMySQL(15);
    /*! Laisser à mysqld le temps de régénérer les certificats avant de les récolter. */
    for (int i = 0; i < 20 && !QFile::exists(datadir + "/ca.pem"); ++i)
        Utils::Pause(1000);
    recolterClesClientSSL(datadir);
#else
    const QString sslDest = QString(PATH_DIR_CLESSSL_SERVEUR);
    const QString user    = QString::fromLocal8Bit(qgetenv("USER"));
    QString sh = "DATA='" + datadir + "'; SSLDEST='" + sslDest + "'; USERN='" + user + "'\n";
  #if defined(Q_OS_MACOS)
    sh += "PLIST=/Library/LaunchDaemons/com.oracle.oss.mysql.mysqld.plist\n"
          "[ -f \"$PLIST\" ] && launchctl unload \"$PLIST\" 2>/dev/null\n";
  #else
    sh += "systemctl stop mysql 2>/dev/null\n";
  #endif
    for (const QString& f : pem)
        sh += "rm -f \"$DATA/" + f + "\"\n";
  #if defined(Q_OS_MACOS)
    sh += "[ -f \"$PLIST\" ] && launchctl load -w \"$PLIST\" 2>/dev/null || "
          "/usr/local/mysql/support-files/mysql.server restart 2>/dev/null\n";
  #else
    sh += "systemctl start mysql 2>/dev/null\n";
  #endif
    /*! Attendre la régénération automatique des certificats (auto_generate_certs) puis récolter. */
    sh += "for i in $(seq 1 20); do [ -f \"$DATA/ca.pem\" ] && break; sleep 1; done\n"
          "mkdir -p \"$SSLDEST\"\n"
          "cp -f \"$DATA/ca.pem\"          \"$SSLDEST/ca-cert.pem\"     2>/dev/null\n"
          "cp -f \"$DATA/client-cert.pem\" \"$SSLDEST/client-cert.pem\" 2>/dev/null\n"
          "cp -f \"$DATA/client-key.pem\"  \"$SSLDEST/client-key.pem\"  2>/dev/null\n"
          "[ -n \"$USERN\" ] && { chown \"$USERN\" \"$(dirname \"$SSLDEST\")\"; chown -R \"$USERN\" \"$SSLDEST\"; }\n"
          "chmod 600 \"$SSLDEST/client-key.pem\" 2>/dev/null\n";
    runCmdElevated(sh);
    waitForMySQL(15);
#endif
    return clesSSLServeurPresentes();
}

/*!
 * \brief MySQLInstaller::controlerClesSSLMonoposte
 * Rattrape au démarrage des clés SSL effacées, jamais créées ou expirées. Cas bénin (copie client
 * manquante) : réextraction silencieuse ; cas destructif (certificats absents ou expirés) : régénération,
 * donc consentement puis relance.
 */
void MySQLInstaller::controlerClesSSLMonoposte()
{
    if (DataBase::I()->ModeAccesDataBase() != Utils::Poste)
        return;

    const QDateTime exp   = dateExpirationCertSSL();
    const bool      expire = exp.isValid() && exp <= QDateTime::currentDateTimeUtc();
    const bool      presentes = clesSSLServeurPresentes();

    if (presentes && !expire)
        return;                                   /*!< tout va bien : rien */

    /*! Cas BÉNIN : la copie client exportable manque, mais le serveur a des certificats VALIDES (« effacée
     *  par erreur » / « jamais récoltée » — typiquement un ancien Rufus + MySQL 8 déjà présent). On
     *  RÉEXTRAIT simplement, sans toucher au serveur (aucune coupure, aucune relance). */
    if (!presentes && !expire)
    {
        if (extraireClesSSLDepuisDatadir())
            return;
        /*! Échec → le datadir n'a pas (ou plus) de certificats : on bascule sur la régénération. */
    }

    /*! Cas DESTRUCTIF : certificats expirés, ou aucun certificat. La régénération crée de NOUVELLES clés →
     *  les postes distants déjà configurés devront recevoir les nouvelles. On avertit sévèrement et on
     *  n'agit que sur consentement. */
    UpMessageBox msgbox(Q_NULLPTR);
    msgbox.setText(expire ? tr("Certificats SSL expirés") : tr("Clés SSL absentes"));
    msgbox.setInformativeText(
        (expire ? tr("Les certificats SSL du serveur ont expiré : l'accès distant ne fonctionne plus.")
                : tr("Le serveur ne dispose pas de clés SSL pour l'accès distant.")) + "\n\n"
        + tr("Rufus peut générer de NOUVELLES clés SSL.") + "\n"
        + tr("ATTENTION : les postes en accès distant déjà configurés ne pourront plus se connecter "
             "tant que vous ne leur aurez pas transmis les NOUVELLES clés.") + "\n"
        + tr("Le serveur MySQL sera redémarré et Rufus relancé."));
    msgbox.setIcon(UpMessageBox::Warning);
    UpSmallButton *Plus = new UpSmallButton(); Plus->setText(tr("Plus tard"));
    UpSmallButton *Gen  = new UpSmallButton(); Gen ->setText(tr("Générer de nouvelles clés"));
    msgbox.addButton(Plus, UpSmallButton::CLOSEBUTTON);
    msgbox.addButton(Gen,  UpSmallButton::STARTBUTTON);
    msgbox.exec();
    if (msgbox.clickedButton() != Gen)
        return;                                   /*!< reporté : on retentera au prochain démarrage */

    if (regenererClesSSL())
    {
        UpMessageBox::Watch(Q_NULLPTR, tr("Nouvelles clés SSL générées"),
            tr("De nouvelles clés SSL ont été générées.") + "\n"
            + tr("Transmettez-les aux postes en accès distant : menu Édition / Paramètres / Ce poste "
                 "→ « Exporter les clés client SSL ».") + "\n\n"
            + tr("Rufus va redémarrer."));
        Utils::Redemarrage();
    }
    UpMessageBox::Watch(Q_NULLPTR, tr("Génération impossible"),
        tr("Les clés SSL n'ont pas pu être générées."));
}

/*!
 * \brief MySQLInstaller::avertirExpirationClesSSLDistant
 * ACCÈS DISTANT : avertissement PROACTIF d'expiration des clés SSL. Indispensable car le serveur (machine
 * dédiée qui ne lance jamais Rufus) ne déclenche pas controlerClesSSLMonoposte() → sans ce rappel côté
 * client, l'accès distant tomberait sans préavis. La régénération se fait sur le SERVEUR : ici on alerte.
 */
void MySQLInstaller::avertirExpirationClesSSLDistant()
{
    if (DataBase::I()->ModeAccesDataBase() != Utils::Distant)
        return;
    const QDateTime exp = dateExpirationCertSSL();
    if (!exp.isValid())
        return;                                   /*!< statut indisponible → on n'invente rien */
    const qint64 jours = QDateTime::currentDateTimeUtc().daysTo(exp);
    if (jours > 60)
        return;                                   /*!< encore large (certs valides ~10 ans) → silence */

    const QString quand = (jours <= 0)
        ? tr("ont expiré")
        : tr("expireront le %1 (dans %2 jours)")
              .arg(QLocale().toString(exp.date(), "dd MMMM yyyy")).arg(jours);
    UpMessageBox::Watch(Q_NULLPTR, tr("Clés SSL d'accès distant"),
        tr("Les clés SSL qui sécurisent cet accès distant %1.").arg(quand) + "\n\n"
        + tr("Sans renouvellement, la connexion à distance cessera de fonctionner.") + "\n"
        + tr("Faites générer de nouvelles clés SUR LE POSTE SERVEUR (menu Édition / Paramètres / "
             "Ce poste → « Créer de nouvelles clés SSL »), puis faites-vous transmettre les nouvelles "
             "clés et indiquez leur dossier ici."));
}

/*!
 * \brief MySQLInstaller::reinstallerSocleMySQLpourMigration
 * Orchestration destructive de la migration du socle : (droits admin) → désinstallation de l'ancien MySQL
 * → réinstallation + adminrufus. À N'APPELER QU'APRÈS une sauvegarde VALIDÉE (cf. Procedures). true si le
 * nouveau socle est prêt.
 */
bool MySQLInstaller::reinstallerSocleMySQLpourMigration()
{
    if (!assurerDroitsAdmin())
        return false;
    const MySQLRemoteConfig cfg = fetchRemoteConfig();
    /*! SSL (étape 7, point 4) : conserver les clés AVANT la désinstallation (qui détruit le datadir). */
    const bool clesConservees = sauvegarderClesSSLMigration();
    uninstallMySQL();
    if (!reinstallerSocleMySQL(cfg))
        return false;
    /*! Réinjecter les anciennes clés (même CA) : les postes distants existants restent valides. */
    if (clesConservees)
        restaurerClesSSLMigration();
    return true;
}

/*!
 * \brief MySQLInstaller::socleLocalConforme
 * Socle conforme (>= VERSION_MYSQL_MINI, pas MariaDB) relevé SANS connexion, sur le serveur LOCAL.
 * Pour les décisions qui PRÉCÈDENT la connexion — après, c'est socleMySQLConforme().
 */
bool MySQLInstaller::socleLocalConforme()
{
    MySQLInstaller m;
    return versionAtLeast(m.getMySQLServerVersion(), seuilVersionMySQL()) && !m.isMariaDB();
}

/*!
 * \brief MySQLInstaller::socleMySQLConforme
 * Idem, mais lu par la connexion Qt ouverte (SELECT VERSION()) : vaut donc aussi pour un serveur distant.
 */
bool MySQLInstaller::socleMySQLConforme()
{
    bool ok = false;
    QVariantList rv = DataBase::I()->getFirstRecordFromStandardSelectSQL("SELECT VERSION()", ok);
    if (!ok || rv.isEmpty())
        return false;
    const QString sv = rv.at(0).toString();
    if (sv.contains("MariaDB", Qt::CaseInsensitive))
        return false;
    QRegularExpression re(R"((\d+\.\d+\.\d+))");
    const auto mv = re.match(sv);
    return mv.hasMatch() && versionAtLeast(mv.captured(1), seuilVersionMySQL());
}

/*!
 * \brief MySQLInstaller::executerEtapesConfig
 * Étapes de configuration après installation (PATH, dossier partagé, secure_file_priv, droits…), en
 * cochant la checklist. Les comptes ne sont créés ici que s'ils ne l'ont pas déjà été (m_comptesDejaCrees).
 * true si tout est validé.
 */
bool MySQLInstaller::executerEtapesConfig()
{
    /*! Login = LOGIN_SQL. m_password a déjà été fixé (aléatoire) par l'appelant. */
    m_login = LOGIN_SQL;

    if (!isServerRunning()) startMySQL();

#if defined(Q_OS_MACOS)
    /*! macOS, installation neuve : regrouper TOUT le paramétrage root (my.cnf, /etc/paths.d, dossier +
     *  partage SMB, redémarrage) en UNE seule invite admin, AVANT les étapes ci-dessous qui, sinon,
     *  élèveraient chacune séparément. */
    if (m_freshInstall)
        prepareCreateModeMacOS();
#endif

    /*! ── Étape 2 : le chemin de mysql est dans la variable d'environnement PATH ─ */
    if (!ensureMysqlInPath()) {
        UpMessageBox::Watch(m_dialog, tr("PATH non configuré"),
            tr("Impossible d'ajouter le chemin de mysql à la variable PATH."));
        return false;
    }
    if (m_dialog->wasCancelled()) return false;
    m_dialog->checkStep(1);

#if defined(Q_OS_LINUX)
    /*! Installation neuve sous Linux : regrouper TOUT le paramétrage root en UNE seule élévation, pour ne
     *  demander le mot de passe système qu'une fois. */
    if (m_freshInstall)
        prepareCreateModeLinux();   /*!< (affiche son propre indicateur + attend mysqld) */
#endif

    /*! Installation neuve : créer l'utilisateur — les étapes suivantes ont besoin d'une connexion valide.
     *  (Sous Linux, createUser() court-circuite si prepareCreateModeLinux l'a créé.) En mode VERIFY, les
     *  comptes ont déjà été créés par createUserAvecAdmin() → on ne les recrée pas. */
    if (m_freshInstall && !m_comptesDejaCrees && !createUser()) {
        UpMessageBox::Watch(m_dialog, tr("Création d'utilisateur impossible"),
            tr("Impossible de créer l'utilisateur '%1'.").arg(m_login)
            + (m_createUserErr.trimmed().isEmpty()
                   ? QString()
                   : "\n\n" + tr("Détail :") + "\n" + m_createUserErr.trimmed().left(1500)));
        return false;
    }

    /*! ── Étape 3 : le dossier partagé existe et est partagé ──────────────── */
    if (!setupSharedFolder()) {
        UpMessageBox::Watch(m_dialog, tr("Dossier partagé impossible"),
            tr("Impossible de créer ou de partager le dossier %1.")
            .arg(sharedFolderPath()));
        return false;
    }
    if (m_dialog->wasCancelled()) return false;
    m_dialog->checkStep(2);

    /*! ── Étape 4 : secure_file_priv pointe sur le dossier partagé ────────── */
    if (!ensureSecureFilePriv()) {
        UpMessageBox::Watch(m_dialog, tr("secure_file_priv impossible"),
            tr("Impossible de configurer secure_file_priv sur %1.")
            .arg(sharedFolderPath())
            + (m_secureFilePrivErr.trimmed().isEmpty()
                   ? QString()
                   : "\n\n" + tr("Détail :") + "\n" + m_secureFilePrivErr.trimmed()));
        return false;
    }
    if (m_dialog->wasCancelled()) return false;
    m_dialog->checkStep(3);

    /*! ── Étape 5 : mysql lit et écrit dans le dossier partagé (fichier test) ───────
     *  L'étape 4 a déjà prouvé que le serveur applique secure_file_priv : un échec ici a donc une cause
     *  réelle (privilège FILE, droits du dossier), pas le « Full Disk Access » qu'on accusait à tort. */
    if (!testSharedFolderRW()) {
        if (!tryConnect()) {
            UpMessageBox::Watch(m_dialog, tr("Connexion impossible"),
                tr("Connexion impossible avec le login « %1 ».\n"
                   "Vérifiez le login et le mot de passe.").arg(m_login));
            return false;
        }
        UpMessageBox::Watch(m_dialog, tr("Écriture impossible"),
            tr("Le serveur MySQL ne parvient pas à écrire dans %1.\n\n"
               "Vérifiez que le compte « %2 » possède le privilège FILE et que les "
               "droits du dossier autorisent l'écriture.").arg(sharedFolderPath(), m_login));
        return false;
    }
    if (m_dialog->wasCancelled()) return false;
    m_dialog->checkStep(4);

    /*! ── Étape 6 : privilèges ALL + GRANT OPTION ─────────────────────────── */
    QStringList missing;
    if (!checkPrivileges(missing)) {
        UpMessageBox::Watch(m_dialog, tr("Privilèges manquants"),
            tr("Privilège(s) manquant(s) pour « %1 » : %2")
            .arg(m_login, missing.join(", ")));
        return false;
    }
    if (m_dialog->wasCancelled()) return false;
    m_dialog->checkStep(5);

    /*! ── Étape 7 : clés SSL pour l'accès distant ───────────────────────────
     *  Certificats auto-générés par MySQL dans le datadir ; on récolte la copie CLIENT (exportable) si elle
     *  n'a pas déjà été faite à l'installation. BEST-EFFORT : l'accès distant est optionnel, on ne fait pas
     *  échouer la config s'il manque (controlerClesSSLMonoposte() rattrapera au démarrage). */
    if (!clesSSLServeurPresentes())
        extraireClesSSLDepuisDatadir();
    m_dialog->checkStep(6);

    /*! ── Toutes les étapes de configuration sont validées ───────────────────── */
    return true;
}

/*!
 * \brief MySQLInstaller::uninstallMySQL
 * Désinstalle MySQL et la configuration ajoutée par l'outil. NON destructif pour le dossier partagé
 * /Users/Shared (susceptible de contenir les données de l'utilisateur). Une seule élévation par plateforme.
 */
bool MySQLInstaller::uninstallMySQL()
{
#if defined(Q_OS_WIN)
    /*! Ni la version ni le nom du service ne sont codés en dur (« MySQL80 », « MySQL Server 8.0 »… selon
     *  les postes) : on découvre le service réel par son ImagePath, puis on balaie les installations
     *  serveur de toute version, le PATH machine et la clé de désinstallation.
     *  (Aucun guillemet double DANS $ps : quotes simples ou [char]34.) */
    const QString ps =
        "$ErrorActionPreference='SilentlyContinue';"
        "$base=$null;"
        "$svc=Get-CimInstance Win32_Service | "
            "Where-Object {$_.PathName -like '*mysqld.exe*'} | Select-Object -First 1;"
        "if($svc){"
            "$exe=$svc.PathName.Trim([char]34);"
            "$i=$exe.ToLower().IndexOf('mysqld.exe');"
            "if($i -ge 0){$exe=$exe.Substring(0,$i+10)};"
            "$base=Split-Path (Split-Path ($exe.Trim().Trim([char]34)) -Parent) -Parent;"
            "Stop-Service -Name $svc.Name -Force;"
            "sc.exe stop $svc.Name | Out-Null;"
            "sc.exe delete $svc.Name | Out-Null"
        "};"
        "Start-Sleep -Seconds 2;"
        "Get-Process mysqld -ErrorAction SilentlyContinue | Stop-Process -Force;"
        "Start-Sleep -Seconds 1;"
        "if($base -and (Test-Path $base)){Remove-Item -LiteralPath $base -Recurse -Force};"
        "Get-ChildItem 'C:\\Program Files\\MySQL' -Directory -Filter 'MySQL Server *' "
            "-ErrorAction SilentlyContinue | Remove-Item -Recurse -Force;"
        "Get-ChildItem 'C:\\ProgramData\\MySQL' -Directory -Filter 'MySQL Server *' "
            "-ErrorAction SilentlyContinue | Remove-Item -Recurse -Force;"
        "$p=[Environment]::GetEnvironmentVariable('Path','Machine');"
        "if($p){[Environment]::SetEnvironmentVariable('Path',"
            "(($p -split ';' | Where-Object {$_ -and ($_ -notlike '*MySQL Server*')}) -join ';'),"
            "'Machine')};"
        "Remove-Item -Path 'HKLM:\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion"
        "\\Uninstall\\MySQLForRufus' -Recurse -Force";
    /*! On exécute via un FICHIER .ps1 (-File), PAS en ligne (-Command "…") : un script long truffé de
     *  | { } ' passé à cmd.exe → powershell casse l'imbrication de guillemets en silence. On écrit donc le
     *  script dans le dossier temporaire et on le lance par chemin. */
    const QString script = QDir::toNativeSeparators(QDir::tempPath() + "/rufus_uninstall_mysql.ps1");
    {
        QFile f(script);
        if (f.open(QIODevice::WriteOnly | QIODevice::Text))
            f.write(ps.toUtf8());
    }
    runCmdFull("powershell -NoProfile -ExecutionPolicy Bypass -File \"" + script + "\"", 300000);
    QFile::remove(script);
    return !isMySQLInstalled();
#elif defined(Q_OS_LINUX)
    /*! On retire le partage [Rufus] mais pas samba ni le dossier partagé, dont l'utilisateur peut se
     *  servir. Motifs de purge CIBLÉS : ni 'libmysqlclient.*' ni 'mysql.*', filets trop larges qui
     *  emportaient mysql-workbench ou php-mysql. */
    const QString script =
        "systemctl stop mysql mysqld mariadb 2>/dev/null;"
        "for pat in 'mysql-server.*' 'mysql-client.*' 'mysql-common' "
                   "'mysql-community-*' 'mariadb.*'; do "
          "DEBIAN_FRONTEND=noninteractive apt-get purge -y \"$pat\" 2>/dev/null || true; "
        "done;"
        "DEBIAN_FRONTEND=noninteractive apt-get autoremove -y --purge 2>/dev/null || true;"
        "rm -rf /etc/mysql /var/lib/mysql /var/log/mysql /var/lib/mysql-files "
               "/var/lib/mysql-keyring;"
        "rm -f /etc/profile.d/mysql.sh;"
        "rm -f /etc/apparmor.d/disable/usr.sbin.mysqld;"
        "[ -f /etc/apparmor.d/usr.sbin.mysqld ] && "
          "apparmor_parser -r /etc/apparmor.d/usr.sbin.mysqld 2>/dev/null;"
        "systemctl reload apparmor 2>/dev/null || true;"
        "ufw delete allow 3306 2>/dev/null || true;"
        "SMB=/etc/samba/smb.conf; if [ -f \"$SMB\" ]; then "
          "sed -i '/^\\[Rufus\\]/,/^\\[/ { /^\\[Rufus\\]/d; /^\\[/!d }' \"$SMB\";"
          "sed -i '/server min protocol = NT1/d' \"$SMB\";"
          "systemctl restart smbd 2>/dev/null || true; "
        "fi";
    runCmdElevated(script);
    return !isMySQLInstalled();
#else
    /*! macOS (installeur Oracle .dmg) : arrêt du démon, suppression du pkg (toutes versions via glob),
     *  config et reçus pkgutil. Le compte système _mysql (natif à macOS) est conservé. */
    const QString script =
        "launchctl bootout system "
          "/Library/LaunchDaemons/com.oracle.oss.mysql.mysqld.plist 2>/dev/null || "
        "launchctl unload -w "
          "/Library/LaunchDaemons/com.oracle.oss.mysql.mysqld.plist 2>/dev/null;"
        "pkill -f /usr/local/mysql/bin/mysqld 2>/dev/null; sleep 2;"
        "rm -f /Library/LaunchDaemons/com.oracle.oss.mysql.mysqld.plist;"
        "rm -rf /Library/PreferencePanes/MySQL.prefPane;"
        "rm -rf /usr/local/mysql /usr/local/mysql-*;"   /*!< symlink + dossier(s) versionné(s) */
        "rm -f /etc/my.cnf /etc/paths.d/mysql /tmp/mysql.sock /tmp/mysql.sock.lock;"
        "for p in com.mysql.launchd com.mysql.mysql com.mysql.prefpane "
                 "com.oracle.oss.mysql.mysqld; do pkgutil --forget \"$p\" 2>/dev/null; done";
    runCmdElevated(script);
    return !isMySQLInstalled();
#endif
}

/*! ── MySQL : détection, installation, démarrage ───────────────────────────────── */

/*!
 * \brief MySQLInstaller::isMySQLInstalled
 * Un SERVEUR MySQL (ou MariaDB) est-il installé sur ce poste ? (service/binaire/pkg selon l'OS.)
 */
bool MySQLInstaller::isMySQLInstalled()
{
#if defined(Q_OS_WIN)
    if (!oraclePrefix().isEmpty())
        return true;
    if (runCmd("sc query MySQL " + NUL()).contains("SERVICE_NAME", Qt::CaseInsensitive))
        return true;
    return runCmd("mysql --version " + NUL()).contains("mysql", Qt::CaseInsensitive);
#elif defined(Q_OS_LINUX)
    /*! Détecter le SERVEUR (mysql-server / mariadb-server), PAS le client seul. */
    if (!runCmd("dpkg -l 2>/dev/null | "
                "grep -E '^ii +(mysql-server|mariadb-server)'").trimmed().isEmpty())
        return true;
    if (QFile::exists("/usr/sbin/mysqld") || QFile::exists("/usr/sbin/mariadbd"))
        return true;
    return false;
#else
    QString brewList = runCmd("brew list --formula 2>/dev/null");
    if (brewList.contains(QRegularExpression("\\bmysql(@8\\.4)?\\b")))
        return true;
    if (!oraclePrefix().isEmpty())
        return true;
    QString ver = runCmd("mysql --version 2>/dev/null");
    return ver.contains("mysql", Qt::CaseInsensitive);
#endif
}

bool MySQLInstaller::isOracleInstall()
{
    return !oraclePrefix().isEmpty();
}

/*!
 * \brief MySQLInstaller::serveurLocalPresent
 * Enveloppe STATIQUE de isMySQLInstalled() : pré-contrôle « y a-t-il un serveur ? » au démarrage
 * (Procedures::Connexion_A_La_Base) sans exposer le détail d'implémentation.
 */
bool MySQLInstaller::serveurLocalPresent()
{
    return MySQLInstaller().isMySQLInstalled();
}

/*!
 * \brief MySQLInstaller::verifierEtReparerConfigMonoposte
 * MONOPOSTE : vérification SILENCIEUSE de la config serveur à chaque démarrage, puis réparation SUR
 * CONSENTEMENT si nécessaire. Seul le monoposte corrige la config (un client ne peut pas toucher au serveur).
 */
void MySQLInstaller::verifierEtReparerConfigMonoposte()
{
    if (DataBase::I()->ModeAccesDataBase() != Utils::Poste)
        return;                                   /*!< correction réservée au poste qui héberge la base */

    m_login    = LOGIN_SQL;
    m_password = motDePasseSQL();

    /*! Deux signaux sûrs, pour rien coûter à un démarrage normal : le dossier partagé a disparu, ou
     *  secure_file_priv ne pointe plus sur un dossier valide. Le PATH du client n'est pas testé ici (faux
     *  positifs), et les contrôles lourds sont refaits par executerEtapesConfig() pendant la réparation. */
    QStringList problemes;
    if (!QDir(sharedFolderPath()).exists())
        problemes << tr("le dossier partagé est introuvable");
    if (!DataBase::I()->dirsecure_file_priv())
        problemes << tr("la variable serveur secure_file_priv n'est pas correctement positionnée");

    if (problemes.isEmpty())
        return;                                   /*!< cas courant : tout est conforme, RIEN (silencieux) */

    /*! Anomalie détectée : on PROPOSE de corriger (peut demander le mdp administrateur du poste). */
    UpMessageBox msgbox(Q_NULLPTR);
    msgbox.setText(tr("Configuration du serveur MySQL à corriger"));
    msgbox.setInformativeText(
        tr("La configuration du serveur MySQL présente une ou plusieurs anomalies :") + "\n• "
        + problemes.join("\n• ") + "\n\n"
        + tr("Rufus peut tenter de les corriger maintenant. Cette opération peut demander le mot de "
             "passe administrateur de l'ordinateur.") + "\n"
        + tr("Vous pouvez aussi continuer : Rufus fonctionnera, mais certaines fonctions (imagerie, "
             "sauvegarde) pourraient être perturbées."));
    msgbox.setIcon(UpMessageBox::Warning);
    UpSmallButton *Annul = new UpSmallButton(); Annul->setText(tr("Plus tard"));
    UpSmallButton *Rep   = new UpSmallButton(); Rep->setText(tr("Corriger maintenant"));
    msgbox.addButton(Annul, UpSmallButton::CLOSEBUTTON);
    msgbox.addButton(Rep,   UpSmallButton::STARTBUTTON);
    msgbox.exec();
    if (msgbox.clickedButton() != Rep)
        return;

    /*! Réparation : on REJOUE les étapes de config (PATH, dossier partagé, secure_file_priv,
     *  lecture/écriture, privilèges) SANS réinstaller ni recréer d'utilisateur (m_freshInstall=false). */
    m_dialog = new MySQLInstallerDialog();
    m_dialog->passerEnConfiguration(tr("Correction de la configuration MySQL"),
                                    tr("Vérification et correction de la configuration en cours…"));
    m_dialog->show();
    QApplication::processEvents();
    m_freshInstall = false;
    const bool repare = executerEtapesConfig();
    cleanupDialog();

    if (repare)
        UpMessageBox::Watch(Q_NULLPTR, tr("Configuration corrigée"),
            tr("La configuration du serveur MySQL a été corrigée."));
    else
        UpMessageBox::Watch(Q_NULLPTR, tr("Correction incomplète"),
            tr("Certaines anomalies de configuration n'ont pas pu être corrigées.") + "\n"
            + tr("Rufus continue ; certaines fonctions (imagerie, sauvegarde) peuvent être affectées."));
}

/*!
 * \brief MySQLInstaller::ensureMysqlInPath
 * Vérifie que le dossier de l'exécutable mysql figure dans le PATH ; sinon l'y ajoute de façon
 * persistante (écriture privilégiée ; admin requis).
 */
bool MySQLInstaller::ensureMysqlInPath()
{
    QString mysqlPath = mysqlBin("mysql");
    if (!QDir::isAbsolutePath(mysqlPath)) {
#if defined(Q_OS_WIN)
        mysqlPath = runCmd("where mysql " + NUL())
                        .split('\n', Qt::SkipEmptyParts).value(0).trimmed();
#else
        mysqlPath = runCmd("command -v mysql " + NUL()).trimmed();
#endif
    }
    const QString binDir = QFileInfo(mysqlPath).absolutePath();
    if (binDir.isEmpty() || binDir == ".")
        return false;

#if defined(Q_OS_WIN)
    const QString winBin = QString(binDir).replace('/', '\\');
    auto inMachinePath = [this, &binDir, &winBin]() {
        const QString path = runCmd("powershell -NoProfile -Command "
            "\"[Environment]::GetEnvironmentVariable('Path','Machine')\"");
        return path.contains(winBin, Qt::CaseInsensitive)
            || path.contains(binDir, Qt::CaseInsensitive);
    };
    if (inMachinePath())
        return true;

    runCmdElevated(QString("powershell -NoProfile -Command "
        "\"[Environment]::SetEnvironmentVariable('Path',"
        "[Environment]::GetEnvironmentVariable('Path','Machine')+';%1','Machine')\"")
        .arg(winBin));
    return inMachinePath();
#elif defined(Q_OS_LINUX)
    /*! Sous Ubuntu, apt installe mysql dans /usr/bin (déjà dans le PATH). Au cas où, on persiste via
     *  /etc/profile.d/mysql.sh. */
    auto inLoginPath = [this, &binDir]() {
        const QStringList dirs =
            runCmd("bash -lc 'echo $PATH' 2>/dev/null").split(':', Qt::SkipEmptyParts);
        return dirs.contains(binDir);
    };
    if (inLoginPath())
        return true;

    runCmdElevated(QString(
        "printf 'export PATH=\"%1:$PATH\"\\n' > /etc/profile.d/mysql.sh && "
        "chmod 644 /etc/profile.d/mysql.sh").arg(binDir));
    return inLoginPath();
#else
    auto inLoginPath = [this, &binDir]() {
        const QStringList dirs =
            runCmd("zsh -lc 'echo $PATH' 2>/dev/null").split(':', Qt::SkipEmptyParts);
        return dirs.contains(binDir);
    };
    if (inLoginPath())
        return true;

    const QString tmp = QDir::tempPath() + "/mysql.path";
    QFile f(tmp);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;
    { QTextStream ts(&f); ts << binDir << '\n'; }
    f.close();

    runCmdElevated(QString("cp '%1' /etc/paths.d/mysql && chmod 644 /etc/paths.d/mysql")
                   .arg(tmp));
    QFile::remove(tmp);
    return inLoginPath();
#endif
}

QString MySQLInstaller::oraclePrefix() const
{
#if defined(Q_OS_WIN)
    /*! Emplacement par défaut de l'installation MySQL 8.4 sous Windows. */
    for (const QString& p : {QString("C:/Program Files/MySQL/MySQL Server 8.4"),
                             QString("C:/Program Files/MySQL/MySQL Server 8.0")})
        if (QFile::exists(p + "/bin/mysql.exe"))
            return p;
    return {};
#else
    if (QFile::exists("/usr/local/mysql/bin/mysql"))
        return "/usr/local/mysql";
    return {};
#endif
}

/*!
 * \brief MySQLInstaller::serverVersionString
 * VERSION() du serveur en cours (ex. « 10.5.8-MariaDB »), vide s'il est injoignable. On interroge le
 * serveur DÉJÀ LANCÉ plutôt que « mysqld --version », qui reste muet jusqu'au timeout sous AppImage
 * (LD_LIBRARY_PATH hérité de Rufus).
 */
QString MySQLInstaller::serverVersionString()
{
    const QRegularExpression re(R"((\d+\.\d+\.\d+[^\s]*))");   /*!< « 8.0.13 », « 10.5.8-MariaDB »… */

    /*! Essaie « SELECT VERSION() » avec chaque mot de passe candidat, via la chaîne de connexion et le
     *  timeout donnés. Renvoie la version trouvée, ou vide. */
    auto interroge = [&](const QString& conn, int timeoutMs) -> QString {
        for (const QString& mdp : motsDePasseSQLCandidats()) {
            const QString out = runCmdFull(
                QString("\"%1\" %2 -u \"%3\" -p\"%4\" -N -B -e \"SELECT VERSION();\" 2>&1")
                    .arg(mysqlBin("mysql"), conn, QString(LOGIN_SQL), mdp), timeoutMs);
            const auto m = re.match(out);
            if (m.hasMatch())
                return m.captured(1);
        }
        return QString();
    };

#if !defined(Q_OS_WIN)
    /*! 1. SOCKET Unix (pas de -h : connexion « localhost »). Sans adresse IP, le serveur ne fait AUCUNE
     *  résolution DNS inverse → réponse IMMÉDIATE. En TCP, un serveur dont le DNS inverse traîne (fréquent
     *  en VM) ne répond qu'au bout de ~30 s — ce qui dépassait le timeout et faisait lire une version fausse. */
    const QString viaSocket = interroge(QStringLiteral("--connect-timeout=5"), 8000);
    if (!viaSocket.isEmpty())
        return viaSocket;
#endif
    /*! 2. Repli TCP (réseau/local, ou socket indisponible). Timeout LONG : si le serveur traîne sur la
     *  résolution DNS inverse, il finit par répondre (~30 s) — lent mais correct, mieux qu'une version fausse. */
    return interroge(QStringLiteral(LOCAL_TCP_ARGS), 35000);
}

/*!
 * \brief MySQLInstaller::getMySQLServerVersion
 * Version du SERVEUR (X.Y.Z), jamais celle du client : c'est elle qui décide de la compatibilité
 * (RETAIN CURRENT PASSWORD, apparu en 8.0.14), et sous Ubuntu le client est souvent plus récent que le
 * serveur en place.
 */
QString MySQLInstaller::getMySQLServerVersion()
{
    /*! 1. Le plus fiable : demander sa version au SERVEUR en cours (cf. serverVersionString). */
    const QString srv = serverVersionString();
    const QRegularExpression reXYZ(R"((\d+\.\d+\.\d+))");
    {
        const auto m = reXYZ.match(srv);
        if (m.hasMatch()) return m.captured(1);
    }

    /*! 2. Repli (serveur injoignable) : binaire serveur mysqld. « --version » affiche la version et quitte
     *  (ne démarre rien). Timeout COURT : sous AppImage ce binaire peut rester bloqué. Chemins usuels où
     *  mysqld n'est pas dans le PATH d'un utilisateur normal (Debian/Ubuntu : /usr/sbin). */
    QStringList candidats;
    candidats << mysqlBin("mysqld");
#if !defined(Q_OS_WIN)
    candidats << "/usr/sbin/mysqld" << "/usr/local/mysql/bin/mysqld";
#endif
    const QRegularExpression reDistrib(R"(Distrib\s+([\d.]+))");  /*!< format MariaDB / anciens MySQL */
    const QRegularExpression reVer(R"(Ver\s+([\d.]+))");          /*!< format MySQL 8.x */
    for (const QString& bin : candidats) {
        const QString out = runCmd("\"" + bin + "\" --version " + NUL(), 5000);
        if (out.trimmed().isEmpty())
            continue;
        auto m = reDistrib.match(out);
        if (m.hasMatch()) return m.captured(1);
        m = reVer.match(out);
        if (m.hasMatch()) return m.captured(1);
    }

    /*! 3. Repli ultime : version du client (mieux que rien). Trompeuse sous Ubuntu, d'où le dernier rang. */
    const QString out = runCmd("\"" + mysqlBin("mysql") + "\" --version " + NUL(), 5000);
    auto m = reDistrib.match(out);
    if (m.hasMatch()) return m.captured(1);
    m = reVer.match(out);
    return m.hasMatch() ? m.captured(1) : QString();
}

/*!
 * \brief MySQLInstaller::securiserBaseSiNecessaire
 * Sécurisation à la volée d'une base existante (monoposte / LAN). Pose l'aléatoire si la base ne l'a pas
 * encore, jamais depuis un poste distant, jamais si le socle ne supporte pas le double mot de passe.
 */
bool MySQLInstaller::securiserBaseSiNecessaire()
{
    /*! 0. JAMAIS depuis un poste en accès DISTANT (WAN) : la sécurisation (poser l'aléatoire, devenir
     *  « propriétaire » du mot de passe) doit être faite par un poste LOCAL — sinon, via la deadline des
     *  30 jours, ce poste distant pourrait verrouiller l'accès des postes locaux sans l'aléatoire. */
    if (DataBase::I()->ModeAccesDataBase() == Utils::Distant)
        return false;

    /*! 1. Base déjà sécurisée → rien. Le juge fait foi côté serveur (2e mot de passe sur adminrufus) ; le
     *  .dbkey local sert de garde-fou. NB : on teste un mot de passe NON VIDE pour le mode courant, pas la
     *  simple existence du fichier — un .dbkey présent mais VIDE ne doit pas bloquer une nouvelle tentative. */
    //qDebug() << cleModeCourant() << lireDBKey().value(cleModeCourant());
    if (adminrufusEstSecurise() || motDePasseSQL() != QString(MDP_SQL))
        return false;

    /*! 2. Le serveur supporte-t-il le double mot de passe (RETAIN CURRENT PASSWORD, MySQL >= 8.0.14, pas
     *  MariaDB) ? Sinon rotater bloquerait les autres postes → on laisse la base telle quelle. */
    if (!socleMySQLConforme())
        return false;

    /*! 3. Pose de l'aléatoire + sauvegarde (cœur commun avec l'auto-réparation). */
    return poserEtSauvegarderAleatoire();
}

/*!
 * \brief MySQLInstaller::poserEtSauvegarderAleatoire
 * Cœur de la sécurisation : génère un aléatoire, l'écrit et le RELIT avant de le poser sur le serveur
 * (jamais de mot de passe posé qu'on ne saurait pas relire), vérifie qu'il a pris, puis l'affiche à
 * noter. Réservé au local + socle conforme. true si OK.
 */
bool MySQLInstaller::poserEtSauvegarderAleatoire()
{
    const QString np = genererMotDePasse();
    if (np.isEmpty())                                   /*!< garde-fou : genererMotDePasse ne renvoie jamais vide */
        return false;

    /*! ORDRE CRITIQUE — on SAUVEGARDE puis on RELIT le mot de passe (DISQUE, pas cache) AVANT de le poser
     *  sur le serveur : si l'écriture échoue, on N'ALTÈRE PAS le serveur (sinon base sécurisée avec un mot
     *  de passe perdu → verrouillage à l'expiration de gaxt78iy). */
    stockerMotDePasse(np);
    if (lireDBKey().value(cleModeCourant()) != np)
    {
        s_cacheMDP.remove(cleModeCourant());            /*!< ne pas laisser un mot de passe fantôme en cache */
        qWarning("Securisation abandonnee : ecriture/relecture du .dbkey impossible.");
        return false;                                   /*!< écriture impossible → on ne sécurise PAS */
    }
    /*! Pose np (avec gaxt78iy en 2e mot de passe) sur TOUS les hosts d'adminrufus/adminrufusSSL. */
    securiserAdminrufusEtMdp(np);   /*!< pose np partout, le générique restant en 2e mot de passe */

    /*! VÉRIFICATION que la sécurisation a pris (retours StandardSQL non fiables). Sinon on ANNULE : retrait
     *  de np du .dbkey, retour à gaxt78iy, aucun message trompeur. La base reste fonctionnelle. */
    if (!adminrufusEstSecurise())
    {
        supprimerMotDePassePourMode(DataBase::I()->ModeAccesDataBase());   /*!< retire np du .dbkey + cache */
        s_cacheMDP.insert(cleModeCourant(), QString(MDP_SQL));             /*!< on reste sur gaxt78iy */
        qWarning("Securisation : les ALTER USER n'ont pas pris cote serveur, annulation.");
        return false;
    }

    inviterANoterMotDePasse(np);
    avertirSecurisationMiseEnPlace();
    return true;
}

/*!
 * \brief hostsLANprives
 * Jeu de hosts LOCAUX/PRIVÉS où adminrufus (compte NON-SSL) est autorisé : loopback + les trois plages
 * privées RFC 1918 (172.16/12 énuméré 172.16.%…172.31.%). Couvre n'importe quel LAN et EXCLUT toute IP
 * publique. Source unique : createUser + securiserAdminrufusEtMdp.
 */
static QStringList hostsLANprives()
{
    QStringList h;
    h << "localhost" << "127.0.0.1" << "10.%" << "192.168.%";
    for (int i = 16; i <= 31; ++i) h << QString("172.%1.%").arg(i);
    return h;
}

/*!
 * \brief MySQLInstaller::entretienComptesAdminrufusLAN
 * Crée les adminrufus restreints au LAN s'ils manquent, redonne les privilèges, retire adminrufus@'%'.
 * Ne touche à aucun mot de passe existant. true si @'%' a disparu.
 * \param mdpCourant  mot de passe de la connexion en cours, donné aux comptes créés
 */
bool MySQLInstaller::entretienComptesAdminrufusLAN(const QString& mdpCourant)
{
    const QString ur = QString(LOGIN_SQL);
    auto exec = [](const QString& q){ DataBase::I()->StandardSQL(q); };

    /*! Création groupée : un seul témoin, et le plus improbable — localhost peut traîner d'une install ancienne. */
    if (!Utils::hostsDuCompteSQL(ur).contains("10.%"))
    {
        /*! Générique encore vivant sur le compte qu'on va supprimer ? On le reconduit, sinon les postes
         *  qui n'ont pas l'aléatoire seraient coupés net. */
        bool ok = false;
        const QVariantList r = DataBase::I()->getFirstRecordFromStandardSelectSQL(
            "SELECT User_attributes->>'$.additional_password' IS NOT NULL"
            " FROM mysql.user WHERE User='" LOGIN_SQL "' AND Host='%'", ok);
        const bool generiqueEncoreLa = ok && !r.isEmpty() && r.at(0).toInt() == 1;

        for (const QString& h : hostsLANprives())
        {
            if (generiqueEncoreLa)
            {
                /*! Comptes neufs : l'échéance du générique, lue sur adminrufusSSL@'%', n'est pas déplacée. */
                exec(QString("CREATE USER IF NOT EXISTS '%1'@'%2' IDENTIFIED BY '%3'").arg(ur, h, QString(MDP_SQL)));
                exec(QString("ALTER USER '%1'@'%2' IDENTIFIED BY '%3' RETAIN CURRENT PASSWORD").arg(ur, h, mdpCourant));
            }
            else
                exec(QString("CREATE USER IF NOT EXISTS '%1'@'%2' IDENTIFIED BY '%3'").arg(ur, h, mdpCourant));
        }
    }

    /*! Redonnés à chaque passage : c'est par eux que revient SYSTEM_USER, qu'une version ancienne révoquait. */
    for (const QString& h : hostsLANprives())
        exec(QString("GRANT ALL PRIVILEGES ON *.* TO '%1'@'%2' WITH GRANT OPTION").arg(ur, h));
    exec(QString("FLUSH PRIVILEGES"));

    /*! @'%' (joignable du WAN) retiré seulement une fois le relais LAN en place. Seul endroit où on le droppe. */
    if (!Utils::hostsDuCompteSQL(ur).contains("10.%"))
        return false;                                            /*!< création ratée : on ne touche à rien */
    exec(QString("DROP USER IF EXISTS '%1'@'%'").arg(ur));
    exec(QString("FLUSH PRIVILEGES"));
    return !Utils::hostsDuCompteSQL(ur).contains("%");
}

/*!
 * \brief MySQLInstaller::securiserAdminrufusEtMdp
 * Pose `aleatoire` sur les comptes adminrufus, le mot de passe courant restant en 2e, puis appelle l'entretien.
 * \param aleatoire     mot de passe à poser
 * \param detailresult  si fourni, résultat but par but
 */
bool MySQLInstaller::securiserAdminrufusEtMdp(const QString& aleatoire,
                                              QMap<QString, bool>* detailresult)
{
    const QString urSSL    = QString(LOGIN_SQL) + "SSL";
    const QString ur       = QString(LOGIN_SQL);
    const QString posteSql = Utils::correctquoteSQL(Utils::hostName());
    const QString attr     = QString(" ATTRIBUTE '{\"securepar\":\"%1\"}'").arg(posteSql);
    auto exec = [](const QString& q){ DataBase::I()->StandardSQL(q); };

    /*! Un seul ALTER : RETAIN garde en 2e le mot de passe courant, le générique, qu'on ne repose donc pas.
     *  securepar voyage dans le même ALTER, isolé il effacerait additional_password. */
    exec(QString("ALTER USER '%1'@'%2' IDENTIFIED BY '%3' RETAIN CURRENT PASSWORD%4").arg(urSSL, "%", aleatoire, attr));
    for (const QString& h : Utils::hostsDuCompteSQL(ur))
        exec(QString("ALTER USER '%1'@'%2' IDENTIFIED BY '%3' RETAIN CURRENT PASSWORD%4").arg(ur, h, aleatoire, attr));
    exec(QString("FLUSH PRIVILEGES"));

    /*! Comptes LAN éventuellement absents : l'entretien s'en charge avec l'aléatoire qu'on vient de poser. */
    const bool entretienOK = entretienComptesAdminrufusLAN(aleatoire);

    QMap<QString, bool> res;
    res["adminrufus@% supprimé"]  = entretienOK;
    res["adminrufusSSL sécurisé"] = adminrufusEstSecurise();
    if (detailresult) *detailresult = res;
    for (bool v : res) if (!v) return false;
    return true;
}

/*!
 * \brief MySQLInstaller::entretienApresConnexion
 * Entretien du mot de passe après toute connexion réussie, selon celui avec lequel ce poste s'est
 * connecté. Chaque étape appelée ici est auto-gardée et ne fait rien hors de son cas.
 */
void MySQLInstaller::entretienApresConnexion()
{
    /*! Si la sécurisation vient d'être posée DANS cet appel, securiser a déjà affiché ses messages (mot de
     *  passe à noter + sécurisation en place) : inutile d'y ajouter aussitôt l'avertissement « générique
     *  bientôt désactivé » (deadline tout juste créée). On le réserve aux démarrages SUIVANTS. */
    bool vientDeSecuriser = MySQLInstaller().securiserBaseSiNecessaire();
    /*! La régularisation des comptes adminrufus (cohérence + Option B) n'est PLUS ici : elle est déclenchée
     *  par la connexion elle-même (DataBase::connectToDataBase → verifierComptesAdminrufus), uniquement
     *  quand on s'est connecté AVEC un aléatoire, en local. */
    supprimerGaxt78iySiEchue();
    if (!vientDeSecuriser)
        MySQLInstaller().avertirEffacementImminent();
    MySQLInstaller().proposerRecuperationAleatoire();
    MySQLInstaller().suggererSecurisationDepuisLocal();
}

/*!
 * \brief MySQLInstaller::avertirEffacementImminent
 * Poste détenant l'ALÉATOIRE, gaxt78iy encore en 2e mot de passe, deadline NON atteinte : prévient que le
 * générique sera bientôt effacé (cf. supprimerGaxt78iySiEchue, qui le retirera une fois la deadline
 * passée). Purement informatif.
 */
void MySQLInstaller::avertirEffacementImminent()
{
    if (motDePasseSQL() == QString(MDP_SQL)) return;   /*!< on n'a que le générique → pas concerné */
    if (!adminrufusEstSecurise())            return;   /*!< gaxt78iy déjà retiré */
    const QDateTime d = dateSecurisation();
    if (!d.isValid())                        return;
    const QDateTime echeance = d.addDays(30);
    if (echeance <= QDateTime::currentDateTime()) return;   /*!< deadline passée → c'est supprimerGaxt78iySiEchue qui agit */

    /*! « Ne plus afficher » est mémorisé PAR BASE (rubrique du mode dans rufus.ini) : un poste peut en
     *  viser plusieurs, chacune avec sa propre échéance. La valeur stockée est la date de sécurisation,
     *  donc l'avis réapparaît si la base est re-sécurisée. */
    const QString cleMasque = Utils::getBaseFromMode(DataBase::I()->ModeAccesDataBase()) + "/AvisGeneriqueMasque";
    const QString signature = d.toString(Qt::ISODate);
    QSettings ini(PATH_FILE_INI, QSettings::IniFormat);
    if (ini.value(cleMasque).toString() == signature) return;   /*!< déjà masqué pour cette sécurisation */

    const int jours = QDateTime::currentDateTime().daysTo(echeance);
    const UpSmallButton::StyleBouton rep = UpMessageBox::Question(nullptr,
        tr("Mot de passe générique bientôt désactivé"),
        tr("Ce poste utilise un mot de passe sécurisé pour accèder au servur de base de données.") + "\n\n" +
        tr("Un mot de passe générique est par ailleurs maintenu") + "\n" +
        tr("pour des raisons de compatibilité avec les versions antérieures de Rufus") + "\n\n" +
        tr("Ce mot de passe générique sera automatiquement désactivé") + "\n" +
        tr("le %1 dans %2 jours").arg(QLocale().toString(echeance.date(), tr("dd MMMM yyyy"))).arg(jours) + "\n\n" +
        tr("Assurez-vous d'ici là que les autres postes qui ont accès à ce serveur ont bien récupéré le mot de passe sécurisé."),
        UpDialog::ButtonCancel | UpDialog::ButtonOK,
        QStringList() << tr("Ne plus afficher ce message") << tr("J'ai compris"));
    if (rep == UpSmallButton::CANCELBUTTON)                      /*!< « Ne plus afficher ce message » */
        ini.setValue(cleMasque, signature);
}

/*!
 * \brief MySQLInstaller::proposerRecuperationAleatoire
 * Poste connecté avec le GÉNÉRIQUE (gaxt78iy) alors que la base est SÉCURISÉE (un aléatoire existe côté
 * serveur) — TOUS modes. On l'informe de l'échéance et propose de récupérer DÈS MAINTENANT l'aléatoire
 * (saisie ou clé USB → .dbkey), avant que gaxt78iy ne soit retiré par un poste local.
 */
void MySQLInstaller::proposerRecuperationAleatoire()
{
    if (motDePasseSQL() != QString(MDP_SQL)) return;   /*!< ce poste détient déjà l'aléatoire */
    if (!adminrufusEstSecurise())            return;   /*!< base pas (encore) sécurisée : rien à récupérer */
    const QDateTime d = dateSecurisation();
    const QDateTime echeance = d.isValid() ? d.addDays(30) : QDateTime();
    const QString dateTxt = echeance.isValid() ? QLocale().toString(echeance.date(), "dd MMMM yyyy")
                                               : tr("prochainement");
    const int jours = echeance.isValid() ? QDateTime::currentDateTime().daysTo(echeance) : 0;

    QString corps = tr("Ce poste utilise encore le mot de passe générique (de mise en route),");
    if (echeance.isValid())
        corps += " " + tr("qui sera désactivé le %1 (dans %2 jours).").arg(dateTxt).arg(jours);
    else
        corps += " " + tr("qui sera prochainement désactivé.");
    /*! Si l'attribut securepar est présent, on NOMME le poste détenteur : le praticien sait où aller
     *  chercher le mot de passe (bouton clé USB de Paramètres) au lieu de deviner. Sinon (base sécurisée
     *  par une version antérieure, avant cet attribut), on le dit clairement et on oriente vers n'importe
     *  quel poste à jour, plutôt que de laisser croire à un oubli. */
    const QString poste = posteSecurisation();
    corps += "\n\n";
    if (!poste.isEmpty())
        corps += tr("Le mot de passe sécurisé du cabinet a été mis en place depuis le poste « %1 ».").arg(poste) + "\n" +
                 tr("Récupérez-le sur ce poste (copié sur une clé USB, ou via le menu Édition / Paramètres) "
                    "et enregistrez-le ici : sans lui, cet accès cessera de fonctionner.");
    else
        corps += tr("Le poste qui a mis en place ce mot de passe n'est pas identifié "
                    "(sécurisation faite par une version antérieure de Rufus).") + "\n" +
                 tr("Récupérez-le depuis n'importe quel poste à jour du cabinet (copié sur une clé USB, "
                    "ou via le menu Édition / Paramètres) et enregistrez-le ici : sans lui, cet accès "
                    "cessera de fonctionner.");

    //! un poste distant ne recrée jamais l'aléatoire : pas de 3e bouton pour lui
    const bool distant = (DataBase::I()->ModeAccesDataBase() == Utils::Distant);

    forever
    {
        UpMessageBox msgbox(nullptr);
        msgbox.setText(tr("Mot de passe du cabinet à récupérer"));
        msgbox.setInformativeText(corps);
        msgbox.setIcon(UpMessageBox::Warning);
        UpSmallButton *Annul   = new UpSmallButton(tr("Continuer avec le\nmot de passe temporaire"));
        UpSmallButton *Saisir  = new UpSmallButton(tr("Renseigner le nouveau\nmot de passe"));
        UpSmallButton *Recreer = distant ? nullptr
                                         : new UpSmallButton(tr("Le mot de passe est égaré,\nen créer un nouveau"));
        msgbox.addButton(Annul, UpSmallButton::CLOSEBUTTON);
        if (Recreer)
            msgbox.addButton(Recreer, UpSmallButton::EDITBUTTON);
        msgbox.addButton(Saisir, UpSmallButton::STARTBUTTON);
        msgbox.exec();

        //! aucun poste ne détient plus l'aléatoire : on en recrée un, protégé par le mdp Administrateur
        if (Recreer && msgbox.clickedButton() == Recreer)
        {
            recreerMotDePasseApresVerifAdmin();
            return;
        }
        if (msgbox.clickedButton() != Saisir)
            return;

        //! message adapté : le générique fonctionne encore, ce n'est pas un échec de connexion
        const IssueMdp issue = RecupererMotDePasseMySQL(nullptr,
                tr("Récupérer le mot de passe du cabinet"),
                tr("Saisissez le mot de passe sécurisé du cabinet, ou importez-le depuis la clé USB sur "
                   "laquelle il a été copié depuis un poste à jour."));
        if (issue != IssueMdp::EchecSaisie)   //! obtenu, ou renoncé
            return;

        if (distant)   //! il ne peut ni réessayer utilement ni en créer un : on l'oriente et on sort
        {
            UpMessageBox::Information(nullptr,
                tr("Mot de passe non récupéré"),
                tr("Ce poste continue avec le mot de passe générique, qui sera désactivé.") + "\n\n" +
                tr("Connectez-vous depuis un poste du réseau local ou depuis le serveur pour "
                   "récupérer ou recréer le mot de passe sécurisé du cabinet."));
            return;
        }
    }
}

/*!
 * \brief MySQLInstaller::RecupererMotDePasseMySQL
 * Propose de saisir le mot de passe du cabinet ou de l'importer d'une clé USB, puis l'éprouve avant de
 * l'enregistrer dans le .dbkey.
 * \param parent     fenêtre parente
 * \param titre      titre de la boîte, vide → « base sécurisée »
 * \param corps      texte de la boîte, vide → message d'échec de connexion
 * \param monoposte  ajoute « je n'ai aucun mot de passe » et « réinitialiser le programme »
 */
MySQLInstaller::IssueMdp
MySQLInstaller::RecupererMotDePasseMySQL(QWidget *parent, const QString &titre,
                                         const QString &corps, bool monoposte)
{
    UpMessageBox msgbox(parent);
    msgbox.setIcon(UpMessageBox::Warning);
    msgbox.setText(!titre.isEmpty() ? titre : tr("Base de données sécurisée"));
    msgbox.setInformativeText(!corps.isEmpty() ? corps
                              : tr("Aucun mot de passe connu ne permet de se connecter à cette base : "
                                   "elle a été sécurisée sur un autre poste.\n\n"
                                   "Vous pouvez récupérer le mot de passe du cabinet copié sur une clé USB "
                                   "depuis un poste qui fonctionne, ou le saisir si vous le connaissez."));
    UpSmallButton *AnnulBouton  = new UpSmallButton(tr("Annuler"));
    UpSmallButton *SaisirBouton = new UpSmallButton(tr("Saisir le mot de passe"));
    UpSmallButton *USBBouton    = new UpSmallButton(tr("Importer depuis une clé USB"));
    UpSmallButton *PasserBouton = nullptr;
    UpSmallButton *ReinitBouton = nullptr;
    msgbox.addButton(AnnulBouton, UpSmallButton::CLOSEBUTTON);
    if (monoposte)   //! un client ne répare ni ne remplace la base des autres
    {
        PasserBouton = new UpSmallButton(tr("Je n'ai aucun mot de passe"));
        ReinitBouton = new UpSmallButton(tr("Réinitialiser le programme"));
        msgbox.addButton(PasserBouton, UpSmallButton::SKIPBUTTON);
        msgbox.addButton(ReinitBouton, UpSmallButton::OUPSBUTTON);
    }
    msgbox.addButton(SaisirBouton, UpSmallButton::KEYBOARDBUTTON);
    msgbox.addButton(USBBouton,    UpSmallButton::RECORDBUTTON);
    msgbox.exec();

    if (PasserBouton && msgbox.clickedButton() == PasserBouton)
        return IssueMdp::Inconnu;
    if (ReinitBouton && msgbox.clickedButton() == ReinitBouton)
        return IssueMdp::Reinitialiser;

    QString mdp;
    if (msgbox.clickedButton() == USBBouton)
    {
        const QString fichier = QFileDialog::getOpenFileName(
                    parent, tr("Sélectionnez le fichier du mot de passe sur la clé USB"));
        if (fichier.isEmpty())
            return IssueMdp::EchecSaisie;
        QFile f(fichier);
        if (f.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            mdp = QString::fromUtf8(f.readAll()).trimmed();
            f.close();
        }
        //! un mdp tient sur UNE ligne : tout le reste (un .dbkey pointé par erreur…) est refusé
        if (mdp.contains('\n') || mdp.contains('\r')
            || mdp.startsWith("MONO=",   Qt::CaseInsensitive)
            || mdp.startsWith("LAN=",    Qt::CaseInsensitive)
            || mdp.startsWith("WAN=",    Qt::CaseInsensitive)
            || mdp.startsWith("MDPSQL=", Qt::CaseInsensitive))
            mdp.clear();
        if (mdp.isEmpty())
        {
            UpMessageBox::Watch(parent, tr("Fichier illisible"),
                                tr("Ce fichier ne contient pas un mot de passe valide."));
            return IssueMdp::EchecSaisie;
        }
    }
    else if (msgbox.clickedButton() == SaisirBouton)
    {
        if (!Utils::SaisirMDP(tr("Entrez le mot de passe MySQL du cabinet :"), mdp, parent))
            return IssueMdp::EchecSaisie;
    }
    else
        return IssueMdp::Annule;

    //! éprouvé avant d'écrire le .dbkey : une saisie erronée effacerait l'ancien mot de passe
    if (!DataBase::I()->connectToDataBase(DB_RUFUS, LOGIN_SQL, mdp).isEmpty())
    {
        UpMessageBox::Watch(parent, tr("Mot de passe incorrect"),
            tr("Ce mot de passe ne permet pas de se connecter à la base de données."));
        return IssueMdp::EchecSaisie;
    }
    stockerMotDePasse(mdp);
    return IssueMdp::Obtenu;
}

/*!
 * \brief MySQLInstaller::recreerMotDePasseApresVerifAdmin
 * Recréation MANUELLE du mot de passe aléatoire (bouton « le mot de passe est égaré ») quand PLUS AUCUN
 * poste ne le détient (ex. .dbkey perdu). Protégée par le mot de passe ADMINISTRATEUR de Rufus. Réservé au
 * LOCAL + socle conforme. true si un nouvel aléatoire a été posé.
 * \param parent  fenêtre parente des boîtes de dialogue
 */
bool MySQLInstaller::recreerMotDePasseApresVerifAdmin(QWidget *parent)
{
    if (DataBase::I()->ModeAccesDataBase() == Utils::Distant)
    {
        UpMessageBox::Watch(parent, tr("Impossible depuis un poste distant"),
            tr("La recréation du mot de passe doit se faire depuis un poste du réseau local ou le serveur."));
        return false;
    }
    if (!socleMySQLConforme())
    {
        UpMessageBox::Watch(parent, tr("Impossible"),
            tr("Le serveur MySQL ne prend pas en charge cette opération (version trop ancienne)."));
        return false;
    }
    /*! Verrou : on exige le mot de passe Administrateur de Rufus avant de rebattre le mot de passe de la
     *  base (opération sensible : elle invalide l'ancien aléatoire pour TOUS les postes). */
    QString saisi;
    if (!Utils::VerifMDP(DataBase::I()->getMDPAdmin(), tr("Saisissez le mot de passe Administrateur"), saisi, false, parent))
        return false;   /*!< mauvais mot de passe / annulation */
    return poserEtSauvegarderAleatoire();   /*!< nouvel aléatoire sur tous les hosts + sauvegarde + message */
}

/*!
 * \brief MySQLInstaller::suggererSecurisationDepuisLocal
 * Poste DISTANT connecté avec le GÉNÉRIQUE sur une base ENCORE NON sécurisée : on ne sécurise JAMAIS
 * depuis un poste distant (garde-fou n°0), ce qui laissait ce cas sans message → on SUGGÈRE de sécuriser
 * depuis un poste local ou le serveur. Purement informatif. Réservé à MySQL >= 8.0.14.
 */
void MySQLInstaller::suggererSecurisationDepuisLocal()
{
    if (DataBase::I()->ModeAccesDataBase() != Utils::Distant) return;   /*!< uniquement en accès distant */
    if (motDePasseSQL() != QString(MDP_SQL))                  return;   /*!< ce poste détient déjà l'aléatoire */
    if (adminrufusEstSecurise())                             return;   /*!< base déjà sécurisée → proposerRecuperationAleatoire s'en charge */

    /*! MySQL >= 8.0.14 requis pour la sécurisation (double mot de passe). En deçà, ne rien suggérer :
     *  l'avis « serveur à mettre à jour » couvre déjà ce cas (sécuriser serait de toute façon impossible). */
    bool ok = false;
    QVariantList rv = DataBase::I()->getFirstRecordFromStandardSelectSQL("SELECT VERSION()", ok);
    if (!ok || rv.isEmpty())                         return;
    const QString sv = rv.at(0).toString();
    if (sv.contains("MariaDB", Qt::CaseInsensitive)) return;
    QRegularExpression re(R"((\d+\.\d+\.\d+))");
    const auto mv = re.match(sv);
    if (!mv.hasMatch() || !versionAtLeast(mv.captured(1), seuilVersionMySQL())) return;

    /*! Simple INFORMATION, un seul bouton OK : à ce stade AUCUN mot de passe aléatoire n'existe encore
     *  (contrairement à proposerRecuperationAleatoire, où la base EST sécurisée). Rien à récupérer ici — on
     *  invite juste à faire la sécurisation depuis un poste local ou le serveur. */
    UpMessageBox::Information(nullptr,
        tr("Base de données non sécurisée"),
        tr("Ce poste se connecte au serveur avec le mot de passe générique de mise en route.") + "\n\n" +
        tr("Pour sécuriser cet accès, connectez-vous depuis un poste du réseau local ou depuis le serveur :") + "\n" +
        tr("la création du mot de passe sécurisé s'y fera automatiquement.") + "\n\n" +
        tr("Cette sécurisation ne peut pas se faire depuis un poste distant."));
}

/*!
 * \brief MySQLInstaller::adminrufusEstSecurise
 * adminrufus a-t-il un 2e mot de passe ? (base sécurisée). User_attributes existe depuis MySQL 8.0.21 ;
 * nos serveurs cibles (8.0.3x apt / 8.4.x Oracle) le fournissent.
 */
bool MySQLInstaller::adminrufusEstSecurise()
{
    /*! On interroge adminrufusSSL@'%' (et NON adminrufus@'%') : référence STABLE. L'Option B supprime
     *  adminrufus@'%' (restriction au LAN) mais laisse adminrufusSSL@'%' intact ; or les deux sont sécurisés
     *  EN MÊME TEMPS. Interroger adminrufus@'%' ferait croire, après l'Option B, que la base n'est plus
     *  sécurisée (et gaxt78iy ne serait jamais purgé). */
    bool ok = false;
    const QVariantList r = DataBase::I()->getFirstRecordFromStandardSelectSQL(
        "SELECT User_attributes->>'$.additional_password' IS NOT NULL FROM mysql.user WHERE User='" LOGIN_SQL "SSL' AND Host='%'", ok);
    return ok && !r.isEmpty() && r.at(0).toInt() == 1;
}

/*!
 * \brief MySQLInstaller::unCompteLANaPerduSystemUser
 * true si AU MOINS un compte adminrufus de plage LOCALE (host ≠ '%') a PERDU SYSTEM_USER (revoke d'une
 * version antérieure). Les entrées LAN étant créées ensemble, leur statut est homogène → une requête
 * suffit. Sert à re-déclencher la régularisation même après l'Option B.
 */
bool MySQLInstaller::unCompteLANaPerduSystemUser()
{
    bool ok = false;
    const QVariantList r = DataBase::I()->getFirstRecordFromStandardSelectSQL(
        "SELECT COUNT(*) FROM mysql.user u WHERE u.User='" LOGIN_SQL "' AND u.Host<>'%'"
        " AND NOT EXISTS (SELECT 1 FROM mysql.global_grants g"
        " WHERE g.USER=u.User AND g.HOST=u.Host AND g.PRIV='SYSTEM_USER')", ok);
    return ok && !r.isEmpty() && r.at(0).toInt() > 0;
}

/*!
 * \brief MySQLInstaller::dateSecurisation
 * Date de sécurisation = dernier changement du mot de passe d'adminrufus.
 */
QDateTime MySQLInstaller::dateSecurisation()
{
    /*! adminrufusSSL@'%' : référence stable (cf. adminrufusEstSecurise) — adminrufus@'%' peut avoir été
     *  supprimé par l'Option B. Les deux comptes sont (re)sécurisés ensemble, donc la date fait foi. */
    bool ok = false;
    QVariantList r = DataBase::I()->getFirstRecordFromStandardSelectSQL(
        "SELECT password_last_changed FROM mysql.user WHERE User='" LOGIN_SQL "SSL' AND Host='%'", ok);
    if (ok && !r.isEmpty())
        return r.at(0).toDateTime();
    return QDateTime();
}

/*!
 * \brief MySQLInstaller::posteSecurisation
 * Nom du poste qui a posé le mot de passe sécurisé (gravé dans User_attributes par
 * securiserAdminrufusEtMdp). Indique OÙ récupérer le mot de passe. Vide pour une base sécurisée par une
 * version antérieure (attribut pas encore posé) → l'appelant se rabat sur une formulation générique.
 */
QString MySQLInstaller::posteSecurisation()
{
    bool ok = false;
    QVariantList r = DataBase::I()->getFirstRecordFromStandardSelectSQL(
        "SELECT User_attributes->>'$.securepar' FROM mysql.user WHERE User='" LOGIN_SQL "SSL' AND Host='%'", ok);
    if (ok && !r.isEmpty())
        return r.at(0).toString();
    return QString();
}

/*!
 * \brief MySQLInstaller::supprimerGaxt78iySiEchue
 * Supprime gaxt78iy (le 2e mot de passe) si la deadline (sécurisation + 30 j) est passée. Garde-fou : on
 * ne le fait QUE si ce poste détient le vrai aléatoire (sinon il se couperait l'accès). En accès DISTANT
 * on ne supprime jamais : on avertit seulement que l'échéance est dépassée.
 */
void MySQLInstaller::supprimerGaxt78iySiEchue()
{
    if (motDePasseSQL() == QString(MDP_SQL))   /*!< on n'a que gaxt78iy → ne pas droper */
        return;
    if (!adminrufusEstSecurise())              /*!< déjà droppé, ou jamais sécurisé */
        return;
    const QDateTime d = dateSecurisation();
    if (!d.isValid() || d.addDays(30) > QDateTime::currentDateTime())
        return;                                /*!< deadline pas encore atteinte */

    /*! Poste DISTANT : il ne touche JAMAIS aux comptes adminrufus → il ne supprime PAS le générique (c'est
     *  à un poste local ou au serveur de le faire). L'échéance étant déjà DÉPASSÉE, on avertit qu'elle l'est
     *  depuis X jours et que la désactivation doit se faire depuis un poste local. */
    if (DataBase::I()->ModeAccesDataBase() == Utils::Distant)
    {
        const int joursDepasse = d.addDays(30).daysTo(QDateTime::currentDateTime());
        UpMessageBox::Watch(nullptr,
            tr("Mot de passe générique à désactiver"),
            tr("Ce poste utilise un mot de passe sécurisé pour accéder à la base de données.") + "\n\n" +
            tr("Un mot de passe générique de compatibilité est cependant toujours actif, alors que sa "
               "date d'échéance est dépassée depuis %1 jours.").arg(joursDepasse) + "\n\n" +
            tr("Sa désactivation ne peut pas se faire depuis un poste distant : connectez-vous depuis un "
               "poste du réseau local ou depuis le serveur pour la déclencher."));
        return;
    }

    /*! Deadline atteinte : on DEMANDE l'autorisation avant de supprimer gaxt78iy (un poste resté sur une
     *  ancienne version perdrait l'accès). Si l'utilisateur reporte, on ne supprime rien : la demande se
     *  représentera à la prochaine connexion. */
    if (!confirmerSuppressionGaxt78iy())
        return;

    /*! Purge de gaxt78iy sur TOUS les hosts (comme la pose de l'aléatoire) : sinon le générique subsisterait
     *  sur @'localhost' / @'192.168.%' et resterait un accès valide en local/LAN. */
    for (const QString& h : Utils::hostsDuCompteSQL(QString(LOGIN_SQL)))
        DataBase::I()->StandardSQL(QString("ALTER USER '" LOGIN_SQL "'@'%1' DISCARD OLD PASSWORD").arg(h));
    for (const QString& h : Utils::hostsDuCompteSQL(QString(LOGIN_SQL) + "SSL"))
        DataBase::I()->StandardSQL(QString("ALTER USER '" LOGIN_SQL "SSL'@'%1' DISCARD OLD PASSWORD").arg(h));
    DataBase::I()->StandardSQL("FLUSH PRIVILEGES");
    avertirSuppressionGaxt78iyEffectuee();
}

QString MySQLInstaller::downloadOracleDmg()
{
    const MySQLRemoteConfig cfg = fetchRemoteConfig();

    const QString arch    = runCmd("uname -m 2>/dev/null").trimmed();
    const QString baseUrl = (arch == "arm64") ? cfg.macArm64Url : cfg.macX86Url;

    /*! Oracle nomme ses DMG avec un suffixe « macosNN » (version de macOS de compilation). On construit une
     *  liste de candidats et on retient le premier réellement servi par Oracle. */
    QStringList candidates;
    candidates << baseUrl;
    QRegularExpression macRe(R"(macos\d+)");
    if (macRe.match(baseUrl).hasMatch()) {
        for (const QString& tag : {QStringLiteral("macos26"),
                                   QStringLiteral("macos15"),
                                   QStringLiteral("macos14"),
                                   QStringLiteral("macos13")}) {
            const QString alt = QString(baseUrl).replace(macRe, tag);
            if (!candidates.contains(alt)) candidates << alt;
        }
    }

    QString url = baseUrl;   /*!< dernier recours si aucune sonde n'aboutit */
    for (const QString& cand : candidates) {
        /*! -r 0-0 : ne télécharge qu'un octet ; -L : suit la redirection vers le miroir CDN ; on inspecte
         *  le code HTTP final (200 ou 206 = disponible). */
        const QString code = runCmdFull(
            QString("curl -sSL -r 0-0 -o /dev/null -w '%{http_code}' '%1' 2>/dev/null")
                .arg(cand)).trimmed();
        if (code.endsWith("200") || code.endsWith("206")) { url = cand; break; }
    }

    const QString fileName = url.section('/', -1);
    QString tmpDmg         = QDir::tempPath() + "/" + fileName;

    /*! Téléchargement avec BARRE DE PROGRESSION RÉELLE : downloadFile() utilise curl (RAPIDE — QNAM
     *  plafonnait, très lent sur macOS) et pilote MySQLProgressDialog::setProgress en sondant la taille du
     *  fichier qui grossit (repli QNAM si curl absent). Le DMG macOS pèse ~550 Mo ; sans progression
     *  chiffrée, l'utilisateur croit l'application figée. */
    downloadFile(url, tmpDmg, tr("Téléchargement de MySQL %1 (Oracle)…").arg(cfg.version));

    if (!QFile::exists(tmpDmg) || QFileInfo(tmpDmg).size() < 1'000'000LL) {
        QFile::remove(tmpDmg);
        return {};
    }
    return tmpDmg;
}

/*!
 * \brief MySQLInstaller::installFromDmg
 * macOS : monte le .dmg Oracle, localise le .pkg, l'installe + initialise le datadir + démarre le serveur
 * dans UNE seule élévation (osascript). true si l'install Oracle est en place.
 * \param dmgPath  chemin du .dmg téléchargé
 */
bool MySQLInstaller::installFromDmg(const QString& dmgPath)
{
    /*! Montage */
    QString mountOut = runCmdFull(
        QString("hdiutil attach -nobrowse '%1' 2>&1").arg(dmgPath));
    QString volumePath;
    for (const QString& line : mountOut.split('\n', Qt::SkipEmptyParts)) {
        if (line.contains("/Volumes/")) {
            volumePath = line.section('\t', -1).trimmed();
            break;
        }
    }
    if (volumePath.isEmpty()) { QFile::remove(dmgPath); return false; }

    /*! Localiser le .pkg */
    QString pkgPath;
    for (const QString& f : QDir(volumePath).entryList({"*.pkg"}, QDir::Files)) {
        pkgPath = volumePath + "/" + f; break;
    }
    if (pkgPath.isEmpty()) {
        runCmd(QString("hdiutil detach '%1' -force 2>/dev/null").arg(volumePath));
        QFile::remove(dmgPath);
        return false;
    }

    /*! Journal d'init (réinitialisé ici : installation neuve). */
    { QFile f(m_initLog);
      if (f.open(QIODevice::WriteOnly | QIODevice::Text))
          QTextStream(&f) << "== installFromDmg : installer + init + start ==\n"
                          << "pkg = " << pkgPath << "\n"; }

    /*! Script shell temporaire : installateur PUIS init du datadir + démarrage du serveur, le tout dans la
     *  MÊME élévation → une seule invite de mot de passe. */
    QString scriptPath = QDir::tempPath() + "/mysql_oracle_install.sh";
    {
        QFile s(scriptPath);
        s.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream ts(&s);
        ts << "#!/bin/sh\n"
           << "installer -pkg '" << pkgPath << "' -target /\n"
           << oracleInitStartScript();
    }
    runCmd("chmod +x '" + scriptPath + "'");

    /*! Installation avec élévation */
    MySQLProgressDialog* dlg = new MySQLProgressDialog(
        tr("Installation et configuration de MySQL…\n"
           "(Autorisez l'opération dans la fenêtre qui s'affiche)"));
    dlg->show();
    QApplication::processEvents();

    QProcess proc;
    QEventLoop loop;
    QTimer timeout;
    timeout.setSingleShot(true);
    QObject::connect(&proc,    &QProcess::finished, &loop, &QEventLoop::quit);
    QObject::connect(&timeout, &QTimer::timeout,    [&]{ proc.kill(); loop.quit(); });
    proc.start("osascript", {"-e",
        QString("do shell script \"%1\" with administrator privileges").arg(scriptPath)});
    timeout.start(180000);
    loop.exec();
    dlg->close();
    delete dlg;

    QFile::remove(scriptPath);
    runCmd(QString("hdiutil detach '%1' -force 2>/dev/null").arg(volumePath));
    QFile::remove(dmgPath);

    waitForMySQL(20);             /*!< le script a aussi initialisé et démarré le serveur */
    return isOracleInstall();
}

/*!
 * \brief MySQLInstaller::oracleInitStartScript
 * Fragment shell (exécuté en ROOT) qui, depuis MySQL 8.4.x, supplée le pkg Oracle macOS — lequel
 * n'initialise plus automatiquement /usr/local/mysql/data — puis démarre le serveur et récolte les clés
 * client SSL.
 */
QString MySQLInstaller::oracleInitStartScript() const
{
    /*! SSL (étape 7, points 2 et 5) : la récolte des clés CLIENT se fait ICI, côté ROOT, car le datadir (et
     *  client-key.pem en 0600) appartient à _mysql et n'est pas lisible par l'utilisateur. On copie les 3
     *  clés client vers le dossier du serveur, puis on les rend à l'utilisateur (chown). */
    const QString sslDest  = QString(PATH_DIR_CLESSSL_SERVEUR);
    const QString sslOwner = QString::fromLocal8Bit(qgetenv("USER"));   /*!< utilisateur lançant Rufus */
    return QStringLiteral(
        "PREFIX=/usr/local/mysql; DATA=\"$PREFIX/data\"\n"
        /*! Compte exécutant mysqld : « _mysql » (créé par le pkg), repli « mysql ». */
        "OWNER=$(id -u _mysql >/dev/null 2>&1 && echo _mysql || echo mysql)\n"
        "{\n"
        "  echo \"-- init+start (owner=$OWNER) --\"\n"
        "  if [ ! -f \"$DATA/mysql.ibd\" ]; then\n"
        "    id \"$OWNER\" || echo \"(compte $OWNER introuvable)\"\n"
        "    rm -rf \"$DATA\"; mkdir -p \"$DATA\"; chown -R \"$OWNER\":\"$OWNER\" \"$DATA\"\n"
        /*! TMPDIR de session = /var/folders/<user>/T (privé, inaccessible à _mysql → InnoDB échoue errno
         *  13). On force /tmp (sticky) pour l'env ET via --tmpdir. */
        "    export TMPDIR=/tmp\n"
        "    \"$PREFIX/bin/mysqld\" --no-defaults --initialize-insecure "
        "--user=\"$OWNER\" --basedir=\"$PREFIX\" --datadir=\"$DATA\" --tmpdir=/tmp\n"
        "    echo \"mysqld --initialize rc=$?\"\n"
        "    chown -R \"$OWNER\":\"$OWNER\" \"$DATA\"\n"
        "  else\n"
        "    echo 'datadir déjà initialisé'\n"
        "  fi\n"
        /*! Démarrage (root) : LaunchDaemon si présent (persiste au redémarrage), sinon mysql.server
         *  (TMPDIR=/tmp en renfort contre l'errno 13). */
        "  PLIST=/Library/LaunchDaemons/com.oracle.oss.mysql.mysqld.plist\n"
        "  if [ -f \"$PLIST\" ]; then echo '-- launchctl load --'; launchctl load -w \"$PLIST\"; "
        "else echo '-- mysql.server start --'; TMPDIR=/tmp \"$PREFIX/support-files/mysql.server\" start; fi\n"
        /*! Récolte des SEULES clés client (ca.pem -> ca-cert.pem, client-cert.pem, client-key.pem), puis
         *  restitution à l'utilisateur. JAMAIS server-*.pem ni ca-key.pem. */
        "  echo '-- recolte cles client SSL --'\n"
        "  SSLDEST='%2'; SSLOWNER='%3'\n"
        "  mkdir -p \"$SSLDEST\"\n"
        "  [ -f \"$DATA/ca.pem\" ]          && cp -f \"$DATA/ca.pem\"          \"$SSLDEST/ca-cert.pem\"\n"
        "  [ -f \"$DATA/client-cert.pem\" ] && cp -f \"$DATA/client-cert.pem\" \"$SSLDEST/client-cert.pem\"\n"
        "  [ -f \"$DATA/client-key.pem\" ]  && cp -f \"$DATA/client-key.pem\"  \"$SSLDEST/client-key.pem\"\n"
        "  [ -n \"$SSLOWNER\" ] && { chown \"$SSLOWNER\" \"$(dirname \"$SSLDEST\")\"; chown -R \"$SSLOWNER\" \"$SSLDEST\"; }\n"
        "  chmod 600 \"$SSLDEST/client-key.pem\" 2>/dev/null\n"
        "} >> '%1' 2>&1\n"
        "chmod 644 '%1' 2>/dev/null\n").arg(m_initLog).arg(sslDest).arg(sslOwner);
}

/*!
 * \brief MySQLInstaller::initOracleDataDir
 * Initialise (si besoin) le datadir Oracle PUIS démarre le serveur, en UNE élévation.
 */
bool MySQLInstaller::initOracleDataDir()
{
    const QString prefix = oraclePrefix();          /*!< /usr/local/mysql (ou vide) */
    const QString data   = prefix + "/data";

    {
        QFile f(m_initLog);
        if (f.open(QIODevice::WriteOnly | QIODevice::Text))
            QTextStream(&f) << "== initOracleDataDir ==\n"
                            << "prefix = " << (prefix.isEmpty() ? "(vide)" : prefix) << "\n";
    }
    if (prefix.isEmpty()) {
        QFile f(m_initLog);
        if (f.open(QIODevice::Append | QIODevice::Text))
            QTextStream(&f) << "ERREUR : /usr/local/mysql/bin/mysql introuvable "
                               "(pkg Oracle non détecté).\n";
        return false;
    }

    runCmdElevated(oracleInitStartScript());

    const bool ok = QFileInfo::exists(data + "/mysql.ibd");
    if (ok) waitForMySQL(20);            /*!< le script root a démarré le serveur */
    return ok;
}

/*!
 * \brief MySQLInstaller::installMySQL
 * Installe MySQL selon l'OS : archive ZIP scriptée (Windows), apt-get via pkexec (Linux), .dmg Oracle
 * (macOS). Récolte au passage les clés client SSL. true si le serveur est installé.
 */
bool MySQLInstaller::installMySQL()
{
#if defined(Q_OS_WIN)
    /*! ── Installation Windows par archive ZIP (entièrement scriptable) ───────── */
    const MySQLRemoteConfig cfg = fetchRemoteConfig();
    const QString version  = cfg.version;
    const QString url      = cfg.winUrl;
    const QString zipName  = url.section('/', -1);
    const QString innerDir = zipName.chopped(4);   /*!< retire ".zip" → dossier racine du zip */
    const QString zipPath  = QDir::tempPath() + "/" + zipName;
    const QString extract  = QDir::tempPath() + "/mysql_extract";

    const QString base     = "C:/Program Files/MySQL/MySQL Server 8.4";
    const QString progData = "C:/ProgramData/MySQL/MySQL Server 8.4";
    const QString dataDir  = progData + "/Data";
    const QString cnfPath  = progData + "/my.ini";
    const QString mysqld   = base + "/bin/mysqld.exe";

    /*! Lit le journal d'erreur le plus récent du serveur (datadir/*.err). */
    auto lastErrLog = [&]() -> QString {
        const auto errs = QDir(dataDir).entryInfoList(
            {"*.err"}, QDir::Files, QDir::Time);
        if (errs.isEmpty()) return {};
        QFile lf(errs.first().absoluteFilePath());
        if (!lf.open(QIODevice::ReadOnly | QIODevice::Text)) return {};
        return QString::fromLocal8Bit(lf.readAll()).right(1500);
    };

    /*! 0. Nettoyage défensif (autorise les ré-essais). */
    runCmdFull("net stop MySQL " + NUL());
    runCmdFull("sc delete MySQL " + NUL());
    runCmdFull("taskkill /F /IM mysqld.exe " + NUL());
    /*! Laisser Windows libérer les verrous de fichiers avant de supprimer. */
    runCmd("powershell -NoProfile -Command \"Start-Sleep -Seconds 2\"");
    runCmd(QString("powershell -NoProfile -Command \""
        "Remove-Item -LiteralPath '%1','%2','%3' -Recurse -Force "
        "-ErrorAction SilentlyContinue\"")
        .arg(QDir::toNativeSeparators(base),
             QDir::toNativeSeparators(progData),
             QDir::toNativeSeparators(extract)));

    /*! 1. Téléchargement de l'archive ZIP (~250 Mo) avec barre de progression. */
    downloadFile(url, zipPath, tr("Téléchargement de MySQL %1…").arg(version));
    if (!QFile::exists(zipPath) || QFileInfo(zipPath).size() < 1'000'000LL) {
        QFile::remove(zipPath);
        avertirTelechargementImpossible();
        return false;
    }

    /*! 2. Extraction entrée par entrée (avec barre de progression) puis déplacement. */
    const QString extractPs  = QDir::tempPath() + "/mysql_extract.ps1";
    const QString extractLog = QDir::tempPath() + "/mysql_extract.log";
    QFile::remove(extractLog);
    {
        QFile f(extractPs);
        if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream ts(&f);
            ts << "$ErrorActionPreference = 'Stop'\r\n"
               << "$log = '" << QDir::toNativeSeparators(extractLog) << "'\r\n"
               << "try {\r\n"
               << "  Add-Type -AssemblyName System.IO.Compression.FileSystem\r\n"
               << "  $zipPath = '" << QDir::toNativeSeparators(zipPath) << "'\r\n"
               << "  $dest    = '" << QDir::toNativeSeparators(extract) << "'\r\n"
               << "  $base    = '" << QDir::toNativeSeparators(base)    << "'\r\n"
               << "  if (Test-Path $dest) { Remove-Item -LiteralPath $dest -Recurse -Force }\r\n"
               << "  $zip = [System.IO.Compression.ZipFile]::OpenRead($zipPath)\r\n"
               << "  $total = $zip.Entries.Count\r\n"
               << "  $i = 0\r\n"
               << "  foreach ($e in $zip.Entries) {\r\n"
               << "    $i++\r\n"
               << "    $target = Join-Path $dest $e.FullName\r\n"
               << "    if ($e.FullName.EndsWith('/')) {\r\n"
               << "      if (-not (Test-Path $target)) { New-Item -ItemType Directory -Force -Path $target | Out-Null }\r\n"
               << "    } else {\r\n"
               << "      $dir = Split-Path $target -Parent\r\n"
               << "      if (-not (Test-Path $dir)) { New-Item -ItemType Directory -Force -Path $dir | Out-Null }\r\n"
               << "      [System.IO.Compression.ZipFileExtensions]::ExtractToFile($e, $target, $true)\r\n"
               << "    }\r\n"
               << "    if (($i % 100) -eq 0 -or $i -eq $total) { Write-Output (\"PROGRESS {0} {1}\" -f $i, $total) }\r\n"
               << "  }\r\n"
               << "  $zip.Dispose()\r\n"
               << "  New-Item -ItemType Directory -Force -Path (Split-Path $base -Parent) | Out-Null\r\n"
               << "  if (Test-Path $base) { Remove-Item -LiteralPath $base -Recurse -Force }\r\n"
               << "  Move-Item -LiteralPath (Join-Path $dest '" << innerDir << "') -Destination $base -Force\r\n"
               << "  if (-not (Test-Path (Join-Path $base 'bin\\mysqld.exe'))) {\r\n"
               << "    ('mysqld.exe absent. Contenu extrait : ' + ((Get-ChildItem -LiteralPath $dest -Name) -join ', ')) | Out-File -FilePath $log -Encoding utf8 -Force\r\n"
               << "  }\r\n"
               << "} catch {\r\n"
               << "  \"$($_.Exception.Message)\" | Out-File -FilePath $log -Encoding utf8 -Force\r\n"
               << "}\r\n";
            f.close();
        }
    }
    runLongOpProgress(
        QString("powershell -NoProfile -ExecutionPolicy Bypass -File \"%1\"")
            .arg(QDir::toNativeSeparators(extractPs)),
        tr("Extraction des fichiers MySQL…"), 600000);
    QFile::remove(extractPs);
    QFile::remove(zipPath);
    if (!QFile::exists(mysqld)) {
        QString detail;
        QFile lf(extractLog);
        if (lf.open(QIODevice::ReadOnly | QIODevice::Text))
            detail = QString::fromUtf8(lf.readAll()).trimmed().left(1500);
        QFile::remove(extractLog);
        UpMessageBox::Watch(nullptr, tr("Extraction échouée"),
            tr("L'archive MySQL n'a pas pu être extraite (mysqld.exe introuvable).\n\n"
               "Détail : %1").arg(detail.isEmpty() ? tr("(aucun détail)") : detail));
        return false;
    }
    QFile::remove(extractLog);

    /*! 3. Fichier de configuration minimal (basedir + datadir). */
    QDir().mkpath(progData);
    {
        QFile f(cnfPath);
        if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
            UpMessageBox::Watch(nullptr, tr("Configuration échouée"),
                tr("Impossible d'écrire %1.").arg(QDir::toNativeSeparators(cnfPath)));
            return false;
        }
        QTextStream ts(&f);
        ts << "[mysqld]\n"
           << "basedir=" << base    << "\n"
           << "datadir=" << dataDir << "\n"
           << "port=3306\n"
           /*! MySQL 8.4 désactive mysql_native_password par défaut : on le réactive, car createUser() crée
            *  adminrufus dans ce plugin (le driver Qt ne sait pas authentifier caching_sha2_password en TCP
            *  non chiffré). Cf. createUser(). */
           << "mysql_native_password=ON\n";
    }

    /*! 4. Initialisation du datadir (crée root@localhost SANS mot de passe). */
    runLongOp(QString("\"%1\" --defaults-file=\"%2\" --initialize-insecure --console")
              .arg(QDir::toNativeSeparators(mysqld), QDir::toNativeSeparators(cnfPath)),
              tr("Initialisation de la base de données,\n"
                 "cela peut prendre quelques instants…"), 300000);
    if (!QFile::exists(dataDir + "/mysql")) {     /*!< schéma système créé ? */
        UpMessageBox::Watch(nullptr, tr("Initialisation échouée"),
            tr("L'initialisation du datadir MySQL a échoué.\n\n%1").arg(lastErrLog()));
        return false;
    }

    /*! 5. Enregistrement et démarrage du service Windows. */
    runCmdElevated(QString("\"%1\" --install MySQL --defaults-file=\"%2\"")
                   .arg(QDir::toNativeSeparators(mysqld),
                        QDir::toNativeSeparators(cnfPath)));
    runCmdElevated("net start MySQL");

    if (!isMySQLInstalled()) {
        UpMessageBox::Watch(nullptr, tr("Installation incomplète"),
            tr("Les fichiers MySQL sont en place mais l'installation n'est pas "
               "détectée correctement."));
        return false;
    }
    if (!waitForMySQL(30)) {
        UpMessageBox::Watch(nullptr, tr("Démarrage du service échoué"),
            tr("MySQL est installé mais le service n'a pas démarré.\n\n%1")
            .arg(lastErrLog()));
        return false;
    }

    /*! SSL (étape 7, point 2) : récolte des clés client auto-générées dans le datadir, pour pouvoir les
     *  exporter ensuite vers les postes distants. Best-effort. */
    recolterClesClientSSL(dataDir);

    /*! Rendre MySQL désinstallable depuis « Applications et fonctionnalités ». */
    registerWindowsUninstaller(base, progData, version);
    return true;
#elif defined(Q_OS_LINUX)
    /*! Installation via apt-get (droits root → pkexec), avec barre de progression RÉELLE : apt écrit son
     *  avancement (0-100) sur APT::Status-Fd. */
    const QString aptScript = QDir::tempPath() + "/mysql_apt_install.sh";
    /*! SSL (étape 7, points 2 et 5) : récolte des clés CLIENT, côté ROOT (le datadir /var/lib/mysql et
     *  client-key.pem en 0600 appartiennent à mysql), puis restitution à l'utilisateur. UNIQUEMENT les clés
     *  client ; jamais server-*.pem ni ca-key.pem. */
    const QString sslDest  = QString(PATH_DIR_CLESSSL_SERVEUR);
    const QString sslOwner = QString::fromLocal8Bit(qgetenv("USER"));
    {
        QFile f(aptScript);
        if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream ts(&f);
            ts << "#!/bin/sh\n"
               << "apt-get update >/dev/null 2>&1\n"
               << "DEBIAN_FRONTEND=noninteractive apt-get -o APT::Status-Fd=1 "
                  "install -y mysql-server 2>/dev/null | "
                  "awk -F: '/^(pmstatus|dlstatus)/ "
                  "{ printf \"PROGRESS %d 100\\n\", $3; fflush() }'\n"
               << "DATA=/var/lib/mysql\n"
               << "SSLDEST='" << sslDest << "'; SSLOWNER='" << sslOwner << "'\n"
               << "mkdir -p \"$SSLDEST\"\n"
               << "[ -f \"$DATA/ca.pem\" ]          && cp -f \"$DATA/ca.pem\"          \"$SSLDEST/ca-cert.pem\"\n"
               << "[ -f \"$DATA/client-cert.pem\" ] && cp -f \"$DATA/client-cert.pem\" \"$SSLDEST/client-cert.pem\"\n"
               << "[ -f \"$DATA/client-key.pem\" ]  && cp -f \"$DATA/client-key.pem\"  \"$SSLDEST/client-key.pem\"\n"
               << "[ -n \"$SSLOWNER\" ] && { chown \"$SSLOWNER\" \"$(dirname \"$SSLDEST\")\"; chown -R \"$SSLOWNER\" \"$SSLDEST\"; }\n"
               << "chmod 600 \"$SSLDEST/client-key.pem\" 2>/dev/null\n";
            f.close();
        }
    }
    runLongOpProgress("pkexec sh '" + aptScript + "'",
                      tr("Installation de MySQL via apt-get…"), 900000);
    QFile::remove(aptScript);
    return isMySQLInstalled();
#else
    QString dmg = downloadOracleDmg();
    if (dmg.isEmpty()) { avertirTelechargementImpossible(); return false; }
    /*! installFromDmg installe le pkg PUIS initialise le datadir et démarre le serveur dans la même
     *  élévation. */
    if (!installFromDmg(dmg)) return false;

    /*! L'init a-t-elle réussi ? (mysql.ibd = dictionnaire de données InnoDB.) */
    if (!QFileInfo::exists("/usr/local/mysql/data/mysql.ibd")) {
        QString detail;
        QFile lf(m_initLog);
        if (lf.open(QIODevice::ReadOnly | QIODevice::Text))
            detail = QString::fromLocal8Bit(lf.readAll()).trimmed();
        UpMessageBox::Watch(nullptr, tr("Initialisation impossible"),
            tr("MySQL est installé mais la base de données n'a pas pu être "
               "initialisée (%1/data).\n\nLe serveur ne peut pas démarrer.\n\n"
               "Détail (%2) :\n%3")
            .arg(oraclePrefix(), m_initLog,
                 detail.isEmpty() ? tr("(journal indisponible)")
                                  : detail.right(1500)));
        return false;
    }
    return true;
#endif
}

bool MySQLInstaller::startMySQL()
{
    /*! Déjà démarré ? (isServerRunning gère le cas auth_socket d'Ubuntu.) */
    if (isServerRunning())
        return true;

#if defined(Q_OS_WIN)
    runCmdElevated("net start MySQL");
#elif defined(Q_OS_LINUX)
    runCmdElevated("systemctl start mysql");
#else
    if (isOracleInstall()) {
        /*! Démon système macOS : démarrage (et init du datadir si nécessaire) en UNE élévation idempotente. */
        initOracleDataDir();
    } else {
        runCmdFull("brew services start mysql@8.4 2>&1", 15000);
    }
#endif
    return waitForMySQL(30);
}

bool MySQLInstaller::waitForMySQL(int maxSeconds)
{
    for (int i = 0; i < maxSeconds; i++) {
        QEventLoop loop;
        QTimer::singleShot(1000, &loop, &QEventLoop::quit);
        loop.exec();
        if (isServerRunning())
            return true;
    }
    return false;
}

void MySQLInstaller::restartMySQL()
{
#if defined(Q_OS_WIN)
    /*! Service Windows : arrêt puis démarrage (l'app est déjà élevée). */
    runCmdElevated("net stop MySQL & net start MySQL");
#elif defined(Q_OS_LINUX)
    runCmdElevated("systemctl restart mysql");
#else
    if (isOracleInstall()) {
        /*! Démon système (/Library/LaunchDaemons) : le redémarrage exige root. */
        const QString plist = "/Library/LaunchDaemons/com.oracle.oss.mysql.mysqld.plist";
        if (QFile::exists(plist))
            runCmdElevated(
                QString("launchctl unload '%1'; launchctl load -w '%1'").arg(plist));
        else
            runCmdElevated("/usr/local/mysql/support-files/mysql.server restart");
    } else {
        runLongOp("brew services restart mysql@8.4 2>&1",
                  tr("Redémarrage de MySQL…"), 30000);
    }
#endif
    waitForMySQL(15);
}

/*! ── Dossier partagé /Users/Shared : secure_file_priv + test lecture/écriture ─── */
QList<QPair<QString, QString>> MySQLInstaller::rufusCnfVars()
{
    QList<QPair<QString, QString>> vars = {
        qMakePair(QStringLiteral("secure_file_priv"), sharedFolderPath()),
        qMakePair(QStringLiteral("sql_mode"),
                  QStringLiteral("STRICT_TRANS_TABLES,NO_ENGINE_SUBSTITUTION")),
    };
#if defined(Q_OS_LINUX)
    vars << qMakePair(QStringLiteral("bind-address"), QStringLiteral("*"));
#endif
    return vars;
}

bool MySQLInstaller::ensureSecureFilePriv()
{
    const QString target = sharedFolderPath();
    const QList<QPair<QString, QString>> vars = rufusCnfVars();

    /*! On lit la variable TELLE QUE LE SERVEUR L'APPLIQUE : c'est la seule preuve que le my.cnf a bien
     *  été lu (un fichier correct mais hors de la chaîne de config validait à tort).
     *  normChemin : « \ » → « / » puis cleanPath, pour comparer des chemins qui diffèrent par leurs
     *  séparateurs (dont ceux doublés par le client mysql en mode -B). */
    auto normChemin = [](QString s) { return QDir::cleanPath(s.replace('\\', '/')); };
    QString liveVu;     /*!< dernière valeur lue côté serveur (conservée pour le diagnostic) */
    auto serveurOk = [&]() -> bool {
        const QString out = runCmdFull(QString(
            "\"%1\" " LOCAL_TCP_ARGS " -u \"%2\" -p\"%3\" -N -B -e "
            "\"SELECT @@GLOBAL.secure_file_priv;\" 2>&1")
            .arg(mysqlBin("mysql"), m_login, m_password));
        /*! runCmdFull fusionne stderr : le client mysql lancé avec -p<motdepasse> imprime TOUJOURS
         *  « [Warning] Using a password on the command line… » AVANT le résultat. La valeur est donc sur la
         *  DERNIÈRE ligne non vide ; comparer la sortie brute échouerait à cause de cette ligne parasite. */
        const QStringList lignes = out.split('\n', Qt::SkipEmptyParts);
        liveVu = lignes.isEmpty() ? QString() : lignes.last().trimmed();
        return !liveVu.isEmpty() && liveVu.compare("NULL", Qt::CaseInsensitive) != 0
            && normChemin(liveVu).compare(normChemin(target), Qt::CaseInsensitive) == 0;
    };
    /*! Diagnostic posé sur CHAQUE échec : on saura, à l'écran, POURQUOI ça a raté (valeur réelle du serveur
     *  vs attendue, et ce que contient le fichier de config réellement lu). */
    auto noteEchec = [&](const QString& etape) {
        m_secureFilePrivErr =
            tr("Étape : %1").arg(etape) + "\n"
          + tr("Serveur renvoie : « %1 »").arg(liveVu.isEmpty()
                ? tr("(aucune réponse / connexion impossible)") : liveVu) + "\n"
          + tr("Attendu : « %1 »").arg(target) + "\n"
          + tr("Fichier %1").arg(getCnfPath()) + "\n"
          + tr("contient : « %1 »").arg(getCnfVar("secure_file_priv"));
    };
    m_secureFilePrivErr.clear();

    /*! Déjà bon ? Le FICHIER doit contenir nos réglages ET le SERVEUR doit les appliquer réellement. */
    bool fichierOk = true;
    for (const auto& kv : vars)
        if (getCnfVar(kv.first) != kv.second) { fichierOk = false; break; }
    if (fichierOk && serveurOk())
        return true;

    const QString tmp = writeCnfToTemp(vars);
    if (tmp.isEmpty()) {
        m_secureFilePrivErr = tr("Écriture du my.ini temporaire impossible.");
        return false;
    }
    const QString path = getCnfPath();
    bool ok;

#if defined(Q_OS_LINUX)
    /*! Copie + redémarrage en UNE SEULE élévation (pkexec). */
    ok = runCmdElevated(QString(
        "cp '%1' '%2' && chmod 644 '%2' && systemctl restart mysql")
        .arg(tmp, path));
    QFile::remove(tmp);
    if (ok) waitForMySQL(20);
#elif defined(Q_OS_WIN)
    ok = runCmdElevated(QString("copy /Y \"%1\" \"%2\"")
                        .arg(QString(tmp).replace('/', '\\'),
                             QString(path).replace('/', '\\')));
    QFile::remove(tmp);
    if (ok) restartMySQL();
#else
    ok = runCmdElevated(QString("cp '%1' '%2' && chmod 644 '%2'").arg(tmp, path));
    QFile::remove(tmp);
    if (ok) restartMySQL();
#endif

    if (!ok) {
        noteEchec(tr("copie du my.ini / redémarrage du serveur (commande élevée en échec)"));
        return false;
    }
    /*! Preuve finale sur le SERVEUR VIVANT (après redémarrage), pas sur le fichier : si le serveur ne lit
     *  toujours pas le my.cnf, on le sait ICI (et on échoue honnêtement) au lieu de glisser vers une fausse
     *  fenêtre Full Disk Access. */
    if (!serveurOk()) {
        noteEchec(tr("vérification de la variable serveur après redémarrage"));
        return false;
    }
    return true;
}

/*!
 * \brief MySQLInstaller::testSharedFolderRW
 * Vérifie que mysql sait ÉCRIRE puis RELIRE un fichier dans le dossier partagé.
 */
bool MySQLInstaller::testSharedFolderRW()
{
    const QString sub   = sharedFolderPath() + "/.mysql_rwtest";
    const QString token = "MYSQLD_RW_OK";
    const QString file  = sub + "/probe.txt";   /*!< écrit ET relu par le serveur */

    QDir().mkpath(sub);
#if !defined(Q_OS_WIN)
    /*! macOS : rendre le sous-dossier inscriptible par le compte _mysql (0777). */
    runCmd("chmod 777 '" + sub + "'");
#endif

    /*! ── Écriture par le serveur ─────────────────────────────────────────────── */
    QFile::remove(file);   /*!< INTO OUTFILE refuse un fichier existant */
    runCmdFull(QString(
        "\"%1\" " LOCAL_TCP_ARGS " -u \"%2\" -p\"%3\" -N -B -e \"SELECT '%4' INTO OUTFILE '%5';\" 2>&1")
        .arg(mysqlBin("mysql"), m_login, m_password, token, file));

    /*! ── Relecture par le serveur (et non par l'app) ─────────────────────────── */
    const QString out = runCmdFull(QString(
        "\"%1\" " LOCAL_TCP_ARGS " -u \"%2\" -p\"%3\" -N -B -e \"SELECT LOAD_FILE('%4');\" 2>&1")
        .arg(mysqlBin("mysql"), m_login, m_password, file));

    /*! ── Nettoyage ───────────────────────────────────────────────────────────── */
    QFile::remove(file);
    QDir().rmdir(sub);

    /*! Écriture ET lecture réussies si le jeton est revenu intact. */
    return out.contains(token);
}

/*!
 * \brief MySQLInstaller::getCnfVar
 * Lit la valeur d'une clé dans la section [mysqld] de my.cnf (sans connexion).
 * \param key  nom de la variable recherchée
 */
QString MySQLInstaller::getCnfVar(const QString& key)
{
    QFile f(getCnfPath());
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return {};
    QTextStream ts(&f);
    bool inMysqld = false;
    QString value;
    while (!ts.atEnd()) {
        const QString line = ts.readLine().trimmed();
        if (line == "[mysqld]")   { inMysqld = true;  continue; }
        if (line.startsWith('[')) { inMysqld = false; continue; }
        if (inMysqld) {
            const int eq = line.indexOf('=');
            /*! MySQL accepte « secure_file_priv » et « secure-file-priv ». */
            if (eq > 0) {
                const QString k = line.left(eq).trimmed().replace('-', '_');
                if (k == QString(key).replace('-', '_'))
                    value = line.mid(eq + 1).trimmed();
            }
        }
    }
    f.close();
    /*! Retirer d'éventuels guillemets (fréquents sous Windows) et un slash final. */
    if (value.size() >= 2 && value.startsWith('"') && value.endsWith('"'))
        value = value.mid(1, value.size() - 2);
    if (value.endsWith('/')) value.chop(1);
    return value;
}

/*! ── Identifiants ─────────────────────────────────────────────────────────────── */
bool MySQLInstaller::isServerRunning()
{
    QString bin = mysqlBin("mysqladmin");
    QString out = runCmdFull(QString("\"%1\" ping 2>&1").arg(bin));
    return out.contains("mysqld is alive", Qt::CaseInsensitive)
        || out.contains("Access denied",   Qt::CaseInsensitive);
}

bool MySQLInstaller::tryConnect()
{
    QString out = runCmdFull(
        QString("\"%1\" " LOCAL_TCP_ARGS " -u \"%2\" -p\"%3\" ping 2>&1")
            .arg(mysqlBin("mysqladmin"), m_login, m_password));
    return out.contains("mysqld is alive");
}

/*!
 * \brief MySQLInstaller::tryConnectAs
 * Comme tryConnect() mais avec des identifiants arbitraires (compte admin saisi).
 * \param login  identifiant à tester
 * \param mdp    mot de passe à tester
 */
bool MySQLInstaller::tryConnectAs(const QString& login, const QString& mdp)
{
    const QString out = runCmdFull(
        QString("\"%1\" " LOCAL_TCP_ARGS " -u \"%2\" -p\"%3\" ping 2>&1")
            .arg(mysqlBin("mysqladmin"), login, mdp));
    return out.contains("mysqld is alive");
}

/*!
 * \brief MySQLInstaller::baseRufusComplete
 * true ssi adminrufus se connecte avec motDePasseSQL() ET la base Rufus (DB_RUFUS) existe avec au moins
 * une table → base Rufus complète.
 */
bool MySQLInstaller::baseRufusComplete()
{
    const QString mdp = motDePasseSQL();

    /*! adminrufus se connecte-t-il ? */
    const QString ping = runCmdFull(
        QString("\"%1\" " LOCAL_TCP_ARGS " -u \"%2\" -p\"%3\" ping 2>&1")
            .arg(mysqlBin("mysqladmin"), QString(LOGIN_SQL), mdp));
    if (!ping.contains("mysqld is alive"))
        return false;

    /*! La base DB_RUFUS existe-t-elle ? */
    const QString dbs = runCmdFull(
        QString("\"%1\" " LOCAL_TCP_ARGS " -u \"%2\" -p\"%3\" -N -B -e "
                "\"SHOW DATABASES LIKE '%4';\" 2>&1")
            .arg(mysqlBin("mysql"), QString(LOGIN_SQL), mdp, QString(DB_RUFUS)));
    bool dbFound = false;
    for (const QString& l : lignesResultat(dbs))
        if (l == QString(DB_RUFUS)) { dbFound = true; break; }
    if (!dbFound)
        return false;

    /*! …et contient-elle au moins une table ? Seules les vraies lignes comptent : l'avertissement du
     *  client suffisait à faire passer une base vide pour complète. */
    const QString tables = runCmdFull(
        QString("\"%1\" " LOCAL_TCP_ARGS " -u \"%2\" -p\"%3\" -N -B -e "
                "\"SHOW TABLES FROM %4;\" 2>&1")
            .arg(mysqlBin("mysql"), QString(LOGIN_SQL), mdp, QString(DB_RUFUS)));
    for (const QString& t : lignesResultat(tables))
        if (!t.startsWith("ERROR", Qt::CaseInsensitive))
            return true;
    return false;
}

bool MySQLInstaller::checkPrivileges(QStringList& outMissing)
{
    static const QStringList REQUIRED = {
        "SELECT", "INSERT", "UPDATE", "DELETE", "CREATE", "DROP",
        "RELOAD", "SHUTDOWN", "PROCESS", "FILE", "REFERENCES", "INDEX",
        "ALTER", "SHOW DATABASES", "SUPER", "CREATE TEMPORARY TABLES",
        "LOCK TABLES", "EXECUTE", "REPLICATION SLAVE", "REPLICATION CLIENT",
        "CREATE VIEW", "SHOW VIEW", "CREATE ROUTINE", "ALTER ROUTINE",
        "CREATE USER", "EVENT", "TRIGGER", "CREATE TABLESPACE",
        "CREATE ROLE", "DROP ROLE"
    };

    /*! SHOW GRANTS sans clause FOR = celui du compte RÉELLEMENT utilisé par la connexion, quel que soit
     *  son host. Depuis qu'adminrufus n'existe plus en @'%', le nommer en dur ne rendait aucune ligne :
     *  tous les privilèges étaient signalés manquants. */
    QString raw = runCmdFull(
        QString("\"%1\" " LOCAL_TCP_ARGS " -u \"%2\" -p\"%3\" -N -B -e "
                "\"SHOW GRANTS;\" 2>&1")
            .arg(mysqlBin("mysql"), m_login, m_password));

    QStringList grantedPrivs;
    bool hasGrantOption = false;

    for (const QString& line : raw.split('\n', Qt::SkipEmptyParts)) {
        QString upper = line.trimmed().toUpper();
        if (!upper.startsWith("GRANT ")) continue;
        int onPos = upper.indexOf(" ON ");
        if (onPos < 0) continue;
        QString privPart = upper.mid(6, onPos - 6).trimmed();
        if (privPart == "ALL PRIVILEGES") {
            grantedPrivs = REQUIRED;
        } else {
            for (const QString& p : privPart.split(','))
                grantedPrivs << p.trimmed();
        }
        if (upper.contains("WITH GRANT OPTION"))
            hasGrantOption = true;
    }
    grantedPrivs.removeDuplicates();

    outMissing.clear();
    for (const QString& priv : REQUIRED)
        if (!grantedPrivs.contains(priv, Qt::CaseInsensitive))
            outMissing << priv;

    if (!hasGrantOption) outMissing << "WITH GRANT OPTION";
    return outMissing.isEmpty();
}

bool MySQLInstaller::createUser()
{
#if defined(Q_OS_LINUX)
    /*! Déjà créé (ex. par prepareCreateModeLinux) ? On court-circuite pour ne pas redemander le mot de
     *  passe système. */
    if (tryConnect())
        return true;
#endif

    /*! DOUBLE mot de passe : l'aléatoire en principal, gaxt78iy conservé en 2e (RETAIN) pour qu'un
     *  poste réseau puisse encore se connecter tant que la purge des 30 j n'a pas eu lieu.
     *  Plugin : mysql_native_password d'abord (le driver Qt ne fait pas caching_sha2 en TCP clair), avec
     *  repli sur le plugin par défaut, ce plugin étant désactivé d'origine sur MySQL 8.4 (ERROR 1524). */
    const QString sslLogin = QString(LOGIN_SQL "SSL");
    const QString legacy   = QString(MDP_SQL);
    /*! adminrufus (NON-SSL) n'est créé QUE sur les hosts LOCAUX/PRIVÉS (jamais @'%', joignable du WAN via
     *  redirection de ports). adminrufusSSL@'%' (SSL) sert l'accès distant. Double mot de passe partout :
     *  aléatoire (m_password) primaire + gaxt78iy en 2e (bootstrap réseau, 30 j). */
    auto sqlAvecAuth = [&](const QString& auth) {
        QString sql;
        for (const QString& h : hostsLANprives())
        {
            sql += QString("CREATE USER IF NOT EXISTS '%1'@'%2' %3 '%4';").arg(m_login, h, auth, legacy);
            sql += QString("ALTER USER '%1'@'%2' %3 '%4';").arg(m_login, h, auth, legacy);
            sql += QString("ALTER USER '%1'@'%2' %3 '%4' RETAIN CURRENT PASSWORD;").arg(m_login, h, auth, m_password);
            sql += QString("GRANT ALL PRIVILEGES ON *.* TO '%1'@'%2' WITH GRANT OPTION;").arg(m_login, h);
        }
        sql += QString("CREATE USER IF NOT EXISTS '%1'@'%' %2 '%3' REQUIRE SSL;").arg(sslLogin, auth, legacy);
        sql += QString("ALTER USER '%1'@'%' %2 '%3' REQUIRE SSL;").arg(sslLogin, auth, legacy);
        sql += QString("ALTER USER '%1'@'%' %2 '%3' RETAIN CURRENT PASSWORD;").arg(sslLogin, auth, m_password);
        sql += QString("GRANT ALL PRIVILEGES ON *.* TO '%1'@'%' WITH GRANT OPTION;").arg(sslLogin);
        sql += "FLUSH PRIVILEGES;\n";
        return sql;
    };

#if defined(Q_OS_LINUX)
    /*! Sur Ubuntu, root@localhost utilise auth_socket : « mysql -u root » ne fonctionne QUE lancé en tant
     *  que root. On exécute donc mysql via pkexec et on transmet le SQL (qui contient le mot de passe) par
     *  l'ENTRÉE STANDARD. (MySQL d'apt = 8.0, mysql_native_password actif → pas de repli nécessaire ici.) */
    const QString sql = sqlAvecAuth("IDENTIFIED WITH mysql_native_password BY");
    QProcess p;
    p.setProcessChannelMode(QProcess::MergedChannels);
    p.start("pkexec", QStringList{ mysqlBin("mysql"), "-u", "root" });
    if (!p.waitForStarted(60000))
        return false;
    p.write(sql.toUtf8());
    p.closeWriteChannel();
    waitProcessResponsive(p, 60000);
    const QString out = QString::fromLocal8Bit(p.readAll());
    m_createUserErr = out;
    return !out.contains("ERROR", Qt::CaseInsensitive)
        && !out.contains("not authorized", Qt::CaseInsensitive)
        && !out.contains("dismissed",      Qt::CaseInsensitive);
#else
    auto executer = [&](const QString& sqlTexte) {
        return runCmdFull(QString("\"%1\" -u root -e \"%2\" 2>&1").arg(mysqlBin("mysql"), sqlTexte));
    };
    QString out = executer(sqlAvecAuth("IDENTIFIED WITH mysql_native_password BY"));
    const QString up = out.toUpper();
    /*! Plugin mysql_native_password indisponible (MySQL 8.4 par défaut) → repli sur le plugin serveur. */
    if (up.contains("ERROR 1524")
        || (up.contains("NOT LOADED") && up.contains("MYSQL_NATIVE_PASSWORD")))
        out = executer(sqlAvecAuth("IDENTIFIED BY"));
    m_createUserErr = out;
    return !out.contains("ERROR", Qt::CaseInsensitive);
#endif
}

/*!
 * \brief MySQLInstaller::createUserAvecAdmin
 * Crée adminrufus/adminrufusSSL (avec m_password) en se connectant via le compte ADMINISTRATEUR MySQL
 * fourni (« mysql -u <admin> -p<mdp> », pas de root/pkexec). Distingue l'absence du droit CREATE USER
 * (NoCreateUserRight) des autres erreurs.
 * \param adminLogin  compte admin MySQL utilisé pour la création
 * \param adminMdp    son mot de passe
 */
MySQLInstaller::CreateUserResult
MySQLInstaller::createUserAvecAdmin(const QString& adminLogin, const QString& adminMdp)
{
    const QString sslLogin = QString(LOGIN_SQL "SSL");
    const QString legacy   = QString(MDP_SQL);
    /*! Même schéma que createUser : aléatoire en principal, gaxt78iy retenu en 2e, plugin
     *  mysql_native_password avec repli (ERROR 1524 sur MySQL 8.4). */
    /*! adminrufus (NON-SSL) uniquement sur les hosts LOCAUX/PRIVÉS (jamais @'%' — cf. createUser) ;
     *  adminrufusSSL@'%' (SSL) pour l'accès distant. Double mot de passe : aléatoire + gaxt78iy (2e). */
    auto sqlAvecAuth = [&](const QString& auth) {
        QString sql;
        for (const QString& h : hostsLANprives())
        {
            sql += QString("CREATE USER IF NOT EXISTS '%1'@'%2' %3 '%4';").arg(m_login, h, auth, legacy);
            sql += QString("ALTER USER '%1'@'%2' %3 '%4';").arg(m_login, h, auth, legacy);
            sql += QString("ALTER USER '%1'@'%2' %3 '%4' RETAIN CURRENT PASSWORD;").arg(m_login, h, auth, m_password);
            sql += QString("GRANT ALL PRIVILEGES ON *.* TO '%1'@'%2' WITH GRANT OPTION;").arg(m_login, h);
        }
        sql += QString("CREATE USER IF NOT EXISTS '%1'@'%' %2 '%3' REQUIRE SSL;").arg(sslLogin, auth, legacy);
        sql += QString("ALTER USER '%1'@'%' %2 '%3' REQUIRE SSL;").arg(sslLogin, auth, legacy);
        sql += QString("ALTER USER '%1'@'%' %2 '%3' RETAIN CURRENT PASSWORD;").arg(sslLogin, auth, m_password);
        sql += QString("GRANT ALL PRIVILEGES ON *.* TO '%1'@'%' WITH GRANT OPTION;").arg(sslLogin);
        sql += "FLUSH PRIVILEGES;";
        return sql;
    };
    auto executer = [&](const QString& sql) {
        return runCmdFull(QString("\"%1\" " LOCAL_TCP_ARGS " -u \"%2\" -p\"%3\" -e \"%4\" 2>&1")
                              .arg(mysqlBin("mysql"), adminLogin, adminMdp, sql));
    };

    QString out = executer(sqlAvecAuth("IDENTIFIED WITH mysql_native_password BY"));
    QString up  = out.toUpper();
    /*! Plugin mysql_native_password indisponible (MySQL 8.4 par défaut) → repli plugin serveur. */
    if (up.contains("ERROR 1524")
        || (up.contains("NOT LOADED") && up.contains("MYSQL_NATIVE_PASSWORD")))
    {
        out = executer(sqlAvecAuth("IDENTIFIED BY"));
        up  = out.toUpper();
    }
    m_createUserErr = out;   /*!< conservé pour le diagnostic, comme dans createUser() */

    if (!out.contains("ERROR", Qt::CaseInsensitive))
        return CreateUserResult::Ok;

    /*! Manque du droit de créer des utilisateurs ? MySQL renvoie typiquement ERROR 1227 (« you need the
     *  CREATE USER privilege »), ou ERROR 1044/1142 (accès refusé sur l'objet / privilège manquant). */
    const bool accessDeniedCreateUser =
        up.contains("ACCESS DENIED") && up.contains("CREATE USER");
    if (accessDeniedCreateUser
        || up.contains("ERROR 1227")
        || up.contains("ERROR 1044")
        || up.contains("ERROR 1142"))
        return CreateUserResult::NoCreateUserRight;

    return CreateUserResult::Error;
}

/*! ═══ COMPTE DE SECOURS — et suppression de root ═══════════════════════════════════════════════════
 *  --initialize-insecure laisse root@localhost sans mot de passe : on le remplace par un compte connu
 *  du seul praticien, puis on supprime root, que Rufus n'utilise plus après l'installation.
 *  Ne protège pas du vol de la machine : le datadir se lit sans aucun compte.
 */

/*!
 * \brief MySQLInstaller::supprimerCompteMySQL
 * Supprime un compte sur tous ses hosts. Erreur muette : supprimer un compte système exige SYSTEM_USER,
 * et l'appelant constate lui-même le résultat.
 * \param login  compte à supprimer
 */
void MySQLInstaller::supprimerCompteMySQL(const QString& login)
{
    const QStringList hosts = Utils::hostsDuCompteSQL(login);
    if (hosts.isEmpty())
        return;
    for (const QString& h : hosts)
        DataBase::I()->StandardSQL(QString("DROP USER IF EXISTS '%1'@'%2'").arg(login, h), "");
    DataBase::I()->StandardSQL("FLUSH PRIVILEGES", "");
}

bool MySQLInstaller::rootExiste()           { return !Utils::hostsDuCompteSQL("root").isEmpty(); }
bool MySQLInstaller::compteDeSecoursExiste(){ return !Utils::hostsDuCompteSQL(LOGIN_SQL_SECOURS).isEmpty(); }

/*!
 * \brief demanderMotDePasseDeSecours
 * Saisie du mot de passe de secours, alphanumérique 5-12 caractères pour être retapable des années plus tard.
 * \param parent            fiche parente
 * \param avecConfirmation  true = création (mot de passe + confirmation)
 * \param outMdp            mot de passe saisi
 */
static bool demanderMotDePasseDeSecours(QWidget* parent, bool avecConfirmation, QString& outMdp)
{
    UpDialog dlg(parent);
    dlg.setWindowModality(Qt::WindowModal);
    dlg.setWindowTitle("");

    UpLabel*    label  = new UpLabel();
    UpLabel*    label2 = new UpLabel();
    UpLineEdit* Line   = new UpLineEdit();
    UpLineEdit* Line2  = new UpLineEdit();

    Line  ->setValidator(new QRegularExpressionValidator(Utils::rgx_AlphaNumeric_5_12, &dlg));
    Line2 ->setValidator(new QRegularExpressionValidator(Utils::rgx_AlphaNumeric_5_12, &dlg));
    Line  ->setAlignment(Qt::AlignCenter);
    Line2 ->setAlignment(Qt::AlignCenter);
    Line  ->setMaxLength(12);
    Line2 ->setMaxLength(12);
    Line  ->setFixedHeight(20);
    Line2 ->setFixedHeight(20);
    Line  ->setEchoMode(QLineEdit::Password);
    Line2 ->setEchoMode(QLineEdit::Password);
    label ->setAlignment(Qt::AlignCenter);
    label2->setAlignment(Qt::AlignCenter);
    label2->setFixedHeight(16);

    label->setText(avecConfirmation
        ? QObject::tr("Choisissez un mot de passe de SECOURS pour votre base de données.\n\n"
                      "Il ne servira qu'à reprendre la main sur votre base si tous les autres\n"
                      "mots de passe sont perdus. Il n'est enregistré NULLE PART :\n"
                      "vous seul le connaissez.\n\n"
                      "Choisissez quelque chose que vous retrouverez dans dix ans\n"
                      "(le nom de votre premier chien, de votre rue d'enfance…)\n"
                      "- mini 5 maxi 12 caractères -\n"
                      "- pas de caractères spéciaux ou accentués -")
        : QObject::tr("Saisissez le mot de passe de SECOURS choisi lors de l'installation\n"
                      "de votre base de données."));
    label2->setText(QObject::tr("Confirmez le mot de passe"));
    label2->setVisible(avecConfirmation);
    Line2 ->setVisible(avecConfirmation);

    QVBoxLayout* lay = new QVBoxLayout();
    lay ->setContentsMargins(5, 5, 5, 5);
    lay ->setSpacing(5);
    lay ->addWidget(label);
    lay ->addWidget(Line);
    lay ->addWidget(label2);
    lay ->addWidget(Line2);

    dlg.AjouteLayButtons(UpDialog::ButtonCancel | UpDialog::ButtonOK);
    QObject::connect(dlg.OKButton,     &UpSmallButton::clicked, &dlg, &UpDialog::accept);
    QObject::connect(dlg.CancelButton, &UpSmallButton::clicked, &dlg, &UpDialog::reject);
    dlg.dlglayout()->insertLayout(0, lay);
    dlg.dlglayout()->setSizeConstraint(QLayout::SetFixedSize);
    Line->setFocus();

    forever {
        if (dlg.exec() != QDialog::Accepted)
            return false;
        const QString mdp = Line->text();
        if (mdp.isEmpty()) {
            UpMessageBox::Watch(&dlg, QObject::tr("Saisie incomplète"),
                QObject::tr("Veuillez renseigner un mot de passe."));
            continue;
        }
        if (avecConfirmation && mdp != Line2->text()) {
            UpMessageBox::Watch(&dlg, QObject::tr("Mots de passe différents"),
                QObject::tr("Le mot de passe et sa confirmation ne correspondent pas."));
            continue;
        }
        outMdp = mdp;
        return true;
    }
}

/*!
 * \brief MySQLInstaller::creerCompteDeSecours
 * Crée secoursrufus en monoposte, le teste et seulement alors — supprime root.
 * \param parent
 */
bool MySQLInstaller::creerCompteDeSecours(QWidget* parent)
{
    QString mdp;
    if (!demanderMotDePasseDeSecours(parent, /*avecConfirmation=*/true, mdp))
        return false;

    MySQLInstaller m;
    /*! Les deux formes, comme adminrufus, selon ce que le serveur résout pour 127.0.0.1. */
    auto sqlAvecAuth = [&](const QString& auth) {
        QString sql;
        for (const QString& h : { QString("localhost"), QString("127.0.0.1") })
        {
            sql += QString("CREATE USER IF NOT EXISTS '%1'@'%2' %3 '%4';").arg(QString(LOGIN_SQL_SECOURS), h, auth, mdp);
            sql += QString("ALTER USER '%1'@'%2' %3 '%4';").arg(QString(LOGIN_SQL_SECOURS), h, auth, mdp);
            sql += QString("GRANT ALL PRIVILEGES ON *.* TO '%1'@'%2' WITH GRANT OPTION;").arg(QString(LOGIN_SQL_SECOURS), h);
        }
        sql += "FLUSH PRIVILEGES;";
        return sql;
    };
    auto executer = [&](const QString& sql) {
        return m.runCmdFull(QString("\"%1\" " LOCAL_TCP_ARGS " -u \"%2\" -p\"%3\" -e \"%4\" 2>&1")
                            .arg(m.mysqlBin("mysql"), QString(LOGIN_SQL), motDePasseSQL(), sql));
    };
    /*! native d'abord, repli silencieux : le plugin est désactivé d'origine sur MySQL 8.4. */
    QString out = executer(sqlAvecAuth("IDENTIFIED WITH mysql_native_password BY"));
    const QString up = out.toUpper();
    if (up.contains("ERROR 1524")
        || (up.contains("NOT LOADED") && up.contains("MYSQL_NATIVE_PASSWORD")))
        out = executer(sqlAvecAuth("IDENTIFIED BY"));

    /*! Éprouvé avant de supprimer root : sinon un échec silencieux laisserait le poste sans l'un ni l'autre. */
    if (!m.tryConnectAs(LOGIN_SQL_SECOURS, mdp))
    {
        UpMessageBox::Watch(parent, tr("Compte de secours non créé"),
            tr("Rufus n'a pas pu créer le compte de secours sur le serveur MySQL.") + "\n" +
            tr("Rien n'a été modifié ; Rufus réessaiera au prochain démarrage.") + "\n\n" + out);
        return false;
    }

    /*! Suppression de root, vérification (relecture de mysql.user)*/
    supprimerCompteMySQL("root");
    if (rootExiste())
    {
        UpMessageBox::Watch(parent, tr("Mot de passe de secours enregistré"),
            tr("Votre mot de passe de secours est en place.") + "\n\n" +
            tr("En revanche, Rufus n'a pas pu supprimer le compte « root » de MySQL"));
        return true;
    }
    UpMessageBox::Watch(parent, tr("Mot de passe de secours enregistré"),
        tr("Votre mot de passe de secours est en place") + "\n\n" +
        tr("Ne l'oubliez pas : il n'est écrit nulle part, et c'est lui qui vous permettra de "
           "récupérer l'accès à votre base si tous les autres mots de passe sont perdus."));
    return true;
}

/*!
 * \brief MySQLInstaller::controlerCompteDeSecours
 * Met en place le compte de secours puis supprime root, à chaque démarrage du poste hôte. Sur une base
 * ancienne, attend l'utilisateur n°1 : lui seul choisit ce mot de passe.
 * \param iduser  id de l'utilisateur Rufus qui vient de s'identifier
 */
void MySQLInstaller::controlerCompteDeSecours()
{
    if (compteDeSecoursExiste())
    {
        if (!rootExiste())
            supprimerCompteMySQL("root");    /*!< secours déjà en place, root réapparu (mise à jour du serveur) */
        return;
    }
    creerCompteDeSecours();
}

/*!
 * \brief MySQLInstaller::restaurerAvecMotDePasseDeSecours
 * \param parent  fiche parente des boîtes affichées
 */
bool MySQLInstaller::restaurerAvecMotDePasseDeSecours(QWidget* parent)
{
    QString mdp;
    if (!demanderMotDePasseDeSecours(parent, /*avecConfirmation=*/false, mdp))
        return false;

    if (!tryConnectAs(LOGIN_SQL_SECOURS, mdp))
    {
        UpMessageBox::Watch(parent, tr("Mot de passe de secours refusé"),
            tr("Ce mot de passe n'ouvre pas de session sur le serveur MySQL de ce poste.") + "\n" +
            tr("Vérifiez qu'il s'agit bien du mot de passe de secours choisi à l'installation "
               "de la base, sur CET ordinateur."));
        return false;
    }

    /*! Voie normale de recréation des comptes : seul le compte qui l'exécute change. */
    m_login    = LOGIN_SQL;
    m_password = genererMotDePasse();
    if (createUserAvecAdmin(LOGIN_SQL_SECOURS, mdp) != CreateUserResult::Ok)
    {
        UpMessageBox::Watch(parent, tr("Rétablissement impossible"),
            tr("La connexion de secours fonctionne, mais Rufus n'a pas pu réécrire les comptes "
               "d'accès à la base.") + "\n" + m_createUserErr);
        return false;
    }
    stockerMotDePasse(m_password);
    inviterANoterMotDePasse(m_password);
    return true;
}

#if defined(Q_OS_LINUX)
/*!
 * \brief MySQLInstaller::prepareCreateModeLinux
 * Mode Create (Linux) : exécute TOUT le paramétrage root en UNE SEULE élévation pkexec, pour ne demander
 * le mot de passe système qu'une fois.
 */
bool MySQLInstaller::prepareCreateModeLinux()
{
    const QString path = sharedFolderPath();
    const QString user = runCmd("id -un 2>/dev/null").trimmed();
    QDir().mkpath(path);

    /*! my.cnf préparé hors élévation (mêmes variables que ensureSecureFilePriv). */
    const QString cnfTmp = writeCnfToTemp(rufusCnfVars());
    const QString cnfDst = getCnfPath();
    if (cnfTmp.isEmpty())
        return false;

    /*! 1. SQL de création des DEUX utilisateurs (adminrufus + adminrufusSSL) avec un DOUBLE mot de passe :
     *  on pose d'abord gaxt78iy (MDP_SQL, %3 — littéral, pas un secret), puis on bascule sur l'aléatoire
     *  $PW (placeholder printf %s) EN CONSERVANT gaxt78iy en 2e. INDISPENSABLE : les autres postes se
     *  connectent d'abord avec gaxt78iy avant de récupérer l'aléatoire. Sans ce 2e mot de passe, le
     *  court-circuit « if (tryConnect()) return true; » de createUser() priverait adminrufus de gaxt78iy. */
    const QString sslLogin = QString(LOGIN_SQL "SSL");
    const QString legacy   = QString(MDP_SQL);
    /*! mysql_native_password : cf. createUser() — indispensable pour que le driver Qt (TCP en clair) puisse
     *  se connecter. Actif par défaut sur le MySQL 8.0 d'apt. %1=adminrufus, %2=adminrufusSSL, %3=gaxt78iy ;
     *  %s (printf) = $PW (aléatoire), 2 occurrences. */
    const QString userSql = QString(
        "printf \"CREATE USER IF NOT EXISTS '%1'@'%%' IDENTIFIED WITH mysql_native_password BY '%3'; "
        "ALTER USER '%1'@'%%' IDENTIFIED WITH mysql_native_password BY '%3'; "
        "ALTER USER '%1'@'%%' IDENTIFIED WITH mysql_native_password BY '%s' RETAIN CURRENT PASSWORD; "
        "GRANT ALL PRIVILEGES ON *.* TO '%1'@'%%' WITH GRANT OPTION; "
        "CREATE USER IF NOT EXISTS '%2'@'%%' IDENTIFIED WITH mysql_native_password BY '%3' REQUIRE SSL; "
        "ALTER USER '%2'@'%%' IDENTIFIED WITH mysql_native_password BY '%3' REQUIRE SSL; "
        "ALTER USER '%2'@'%%' IDENTIFIED WITH mysql_native_password BY '%s' RETAIN CURRENT PASSWORD; "
        "GRANT ALL PRIVILEGES ON *.* TO '%2'@'%%' WITH GRANT OPTION; "
        "FLUSH PRIVILEGES;\\n\" \"$PW\" \"$PW\" | mysql -u root; ")
        .arg(m_login, sslLogin, legacy);

    /*! 3. Copie de my.cnf en place + redémarrage du serveur. */
    const QString cnfFragment = QString(
        "cp '%1' '%2' && chmod 644 '%2' && systemctl restart mysql; ")
        .arg(cnfTmp, cnfDst);

    const QString script =
        "IFS= read -r PW\n"
        /*! Journal de support /tmp/rufus_prepare.log : conserve la sortie de l'install. Le mot de passe n'y
         *  apparaît JAMAIS. */
        "exec >/tmp/rufus_prepare.log 2>&1\n"
        "echo '### createUser ###'; "
        + userSql
        + " echo \"### createUser rc=$? ###\"; "
          "echo '### folderSamba ###'; "
        + linuxFolderSambaScript(path, user)
        + "; echo \"### folderSamba rc=$? ###\"; "
          "echo '### cnf ###'; "
        + cnfFragment
        + " echo \"### cnf rc=$? ###\"; echo '### DONE ###'\n";

    /*! Indicateur d'activité pendant l'opération. */
    MySQLProgressDialog* dlg = new MySQLProgressDialog(
        tr("Préparation du serveur…\n"
           "Cela peut durer plusieurs minutes."));
    dlg->show();
    QApplication::processEvents();

    const bool ok = runCmdElevated(script, m_password + "\n");
    QFile::remove(cnfTmp);
    waitForMySQL(20);                 /*!< le serveur redémarre en fin de script */

    dlg->close();
    delete dlg;
    return ok;
}

/*!
 * \brief MySQLInstaller::linuxFolderSambaScript
 * Fragment shell (root) du paramétrage dossier + Samba (mot de passe Samba dans $PW). Réutilisé par
 * setupSharedFolder() et prepareCreateModeLinux().
 * \param path  dossier partagé (/Users/Shared)
 * \param user  compte Ubuntu propriétaire / utilisateur Samba
 */
QString MySQLInstaller::linuxFolderSambaScript(const QString& path,
                                               const QString& user) const
{
    return QString(
        /*! Arborescence Rufus + droits : chmod -R 755, chown -R sur tout /Users. */
        "mkdir -p '%1/Rufus/Imagerie'; chmod -R 755 '%1'; chown -R %2 /Users; "
        /*! AppArmor : DÉSACTIVER le profil mysqld (sinon lecture des images bloquée). */
        "mkdir -p /etc/apparmor.d/disable; "
        "if [ -f /etc/apparmor.d/usr.sbin.mysqld ]; then "
          "ln -sf /etc/apparmor.d/usr.sbin.mysqld /etc/apparmor.d/disable/; "
          "apparmor_parser -R /etc/apparmor.d/usr.sbin.mysqld 2>/dev/null || true; "
        "fi; "
        /*! pare-feu : ouvrir le port MySQL */
        "ufw allow 3306 || true; "
        /*! Samba + wsdd : installés ENSEMBLE si l'un manque. */
        "if ! dpkg -s samba >/dev/null 2>&1 || ! dpkg -s wsdd >/dev/null 2>&1; then "
          "apt-get update </dev/null; "
          "DEBIAN_FRONTEND=noninteractive apt-get install -y samba wsdd </dev/null; "
        "fi; "
        /*! protocole NT1/SMB1 (appareils de mesure anciens) */
        "grep -q 'server min protocol' /etc/samba/smb.conf 2>/dev/null || "
          "sed -i '/^\\[global\\]/a server min protocol = NT1' /etc/samba/smb.conf; "
        /*! partage [Rufus] -> /Users/Shared/Rufus */
        "grep -q '^\\[Rufus\\]' /etc/samba/smb.conf 2>/dev/null || "
          "printf '\\n[Rufus]\\n   comment = Rufus\\n   path = %1/Rufus\\n"
          "   browseable = yes\\n   read only = no\\n   guest ok = yes\\n"
          "   create mask = 0755\\n   directory mask = 0755\\n' >> /etc/samba/smb.conf; "
        /*! utilisateur Samba = compte Ubuntu ; mot de passe = $PW. */
        "printf '%s\\n%s\\n' \"$PW\" \"$PW\" | smbpasswd -s -a '%2' >/dev/null 2>&1 || true; "
        "systemctl restart smbd 2>/dev/null || service smbd restart 2>/dev/null || true; "
        /*! wsdd : activer le service */
        "systemctl enable --now wsdd 2>/dev/null || true"
        ).arg(path, user);
}
#endif

#if defined(Q_OS_MACOS)
/*!
 * \brief MySQLInstaller::prepareCreateModeMacOS
 * Mode Create (macOS) : exécute TOUT le paramétrage root en UNE SEULE invite admin.
 */
bool MySQLInstaller::prepareCreateModeMacOS()
{
    const QString path = sharedFolderPath();            /*!< /Users/Shared */
    QDir().mkpath(path + "/Rufus/Imagerie");

    /*! my.cnf préparé hors élévation (mêmes variables que ensureSecureFilePriv) + réactivation de
     *  mysql_native_password : sur MySQL 8.4 (installé ici sur macOS) ce plugin est DÉSACTIVÉ par défaut,
     *  or createUser() en a besoin pour que le driver Qt se connecte en TCP non chiffré. Sans danger : on
     *  n'écrit jamais ce my.cnf sur un serveur 8.0 préexistant (réservé à l'installation neuve 8.4). */
    QList<QPair<QString, QString>> cnfVars = rufusCnfVars();
    cnfVars << qMakePair(QStringLiteral("mysql_native_password"), QStringLiteral("ON"));
    const QString cnfTmp = writeCnfToTemp(cnfVars);
    const QString cnfDst = getCnfPath();                 /*!< /etc/my.cnf (Oracle) */
    if (cnfTmp.isEmpty())
        return false;

    /*! Chemin du binaire mysql → fichier temporaire copié ensuite dans /etc/paths.d. */
    QString mysqlPath = mysqlBin("mysql");
    if (!QDir::isAbsolutePath(mysqlPath))
        mysqlPath = runCmd("command -v mysql " + NUL()).trimmed();
    const QString binDir  = QFileInfo(mysqlPath).absolutePath();
    const QString pathTmp = QDir::tempPath() + "/mysql.path";
    { QFile f(pathTmp);
      if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
          QTextStream ts(&f); ts << binDir << '\n';
      } }

    /*! (Re)démarrage du serveur : démon launchd si présent, sinon mysql.server. */
    const QString plist = "/Library/LaunchDaemons/com.oracle.oss.mysql.mysqld.plist";
    const QString restart = QFile::exists(plist)
        ? QString("launchctl unload '%1' 2>/dev/null; launchctl load -w '%1'").arg(plist)
        : QString("TMPDIR=/tmp '%1/support-files/mysql.server' restart").arg(oraclePrefix());

    const QString script = QString(
        "cp '%1' '%2' && chmod 644 '%2'\n"                                   /*!< my.cnf */
        "cp '%3' /etc/paths.d/mysql && chmod 644 /etc/paths.d/mysql\n"       /*!< PATH */
        "mkdir -p '%4/Rufus/Imagerie'; chmod -R 755 '%4/Rufus'\n"            /*!< dossier */
        /*! Partage SMB de /Users/Shared (lecture/écriture invité), pour Windows. */
        "launchctl enable system/com.apple.smbd 2>/dev/null; "
        "launchctl kickstart -k system/com.apple.smbd 2>/dev/null; "
        "sharing -a '%4' -n 'Partagé' -s 001 -g 000 2>/dev/null; "
        "launchctl kickstart -k system/com.apple.smbd 2>/dev/null\n"
        "%5\n")                                                             /*!< restart */
        .arg(cnfTmp, cnfDst, pathTmp, path, restart);

    MySQLProgressDialog* dlg = new MySQLProgressDialog(
        tr("Préparation du serveur…\n"
           "Cela peut durer plusieurs minutes."));
    dlg->show();
    QApplication::processEvents();

    const bool ok = runCmdElevated(script);
    QFile::remove(cnfTmp);
    QFile::remove(pathTmp);
    waitForMySQL(20);                 /*!< le serveur vient d'être redémarré */

    dlg->close();
    delete dlg;
    return ok;
}
#endif

/*! ── Dossier partagé : existe et est partagé ──────────────────────────────────── */
bool MySQLInstaller::setupSharedFolder()
{
    const QString path = sharedFolderPath();
    if (!QDir(path).exists())
        QDir().mkpath(path);

#if defined(Q_OS_WIN)
    /*! C:\Users\Public existe par défaut et est accessible en lecture/écriture à tous les comptes
     *  Windows : aucun partage supplémentaire à configurer. */
    return QDir(path).exists();
#elif defined(Q_OS_LINUX)
    /*! Déjà configuré ? Vérifications NON privilégiées (aucune invite pkexec). */
    auto alreadyConfigured = [&]() -> bool {
        if (!QDir(path + "/Rufus/Imagerie").exists())
            return false;
        if (QFileInfo::exists("/etc/apparmor.d/usr.sbin.mysqld")
            && !QFileInfo("/etc/apparmor.d/disable/usr.sbin.mysqld").isSymLink())
            return false;
        QFile smb("/etc/samba/smb.conf");
        if (!smb.open(QIODevice::ReadOnly | QIODevice::Text))
            return false;
        const QString smbContent = QString::fromUtf8(smb.readAll());
        if (!smbContent.contains("[Rufus]"))
            return false;
        if (!smbContent.contains("NT1"))
            return false;
        if (!runCmd("dpkg -s wsdd 2>/dev/null").contains("Status: install ok"))
            return false;
        return true;
    };
    if (alreadyConfigured())
        return true;

    /*! Sinon, tout le paramétrage privilégié en une seule élévation (pkexec). */
    const QString user = runCmd("id -un 2>/dev/null").trimmed();
    runCmdElevated("IFS= read -r PW\n" + linuxFolderSambaScript(path, user),
                   m_password + "\n");
    return QDir(path).exists();
#else
    /*! Déjà partagé ? (lecture seule, aucune élévation) */
    if (runCmd("sharing -l 2>/dev/null").contains(path))
        return true;

    /*! Activer SMB et déclarer le partage — opérations privilégiées (une invite macOS). */
    runCmdElevated(
        "launchctl enable system/com.apple.smbd; "
        "launchctl kickstart -k system/com.apple.smbd; "
        "sharing -a '/Users/Shared' -n 'Partagé' -s 001 -g 000; "
        "launchctl kickstart -k system/com.apple.smbd");

    return runCmd("sharing -l 2>/dev/null").contains(path);
#endif
}

/*! ── Variables MySQL ──────────────────────────────────────────────────────────── */
QString MySQLInstaller::writeCnfToTemp(const QList<QPair<QString, QString>>& vars)
{
    const QString path = getCnfPath();
    QFile   f(path);
    QStringList lines;
    bool inMysqld = false;
    QStringList remaining;                 /*!< clés pas encore rencontrées */
    for (const auto& kv : vars) remaining << kv.first;

    if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream ts(&f);
        while (!ts.atEnd()) {
            QString line    = ts.readLine();
            QString trimmed = line.trimmed();
            if (trimmed == "[mysqld]")
                { inMysqld = true;  lines << line; continue; }
            if (trimmed.startsWith('[') && trimmed != "[mysqld]")
                inMysqld = false;
            bool replaced = false;
            if (inMysqld) {
                const int eq = trimmed.indexOf('=');
                if (eq > 0) {
                    const QString lineKey =
                        trimmed.left(eq).trimmed().replace('-', '_');
                    for (const auto& kv : vars) {
                        if (lineKey == QString(kv.first).replace('-', '_')) {
                            lines << kv.first + " = " + kv.second;
                            remaining.removeAll(kv.first);
                            replaced = true;
                            break;
                        }
                    }
                }
            }
            if (!replaced) lines << line;
        }
        f.close();
    }

    /*! Clés non présentes : insérées juste après [mysqld] (créé si absent). */
    if (!remaining.isEmpty()) {
        QStringList toInsert;
        for (const auto& kv : vars)
            if (remaining.contains(kv.first))
                toInsert << kv.first + " = " + kv.second;
        int idx = -1;
        for (int i = 0; i < lines.size(); i++)
            if (lines[i].trimmed() == "[mysqld]") { idx = i; break; }
        if (idx >= 0) {
            for (int j = toInsert.size() - 1; j >= 0; --j)
                lines.insert(idx + 1, toInsert[j]);
        } else {
            for (int j = toInsert.size() - 1; j >= 0; --j)
                lines.prepend(toInsert[j]);
            lines.prepend("[mysqld]");
        }
    }

    const QString tmp = QDir::tempPath() + "/mysql_conf.new";
    QFile out(tmp);
    if (!out.open(QIODevice::WriteOnly | QIODevice::Text))
        return {};
    {
        QTextStream ts(&out);
        for (const QString& l : lines) ts << l << '\n';
    }
    out.close();
    return tmp;
}

QString MySQLInstaller::getCnfPath()
{
#if defined(Q_OS_WIN)
    for (const QString& p : {QString("C:/ProgramData/MySQL/MySQL Server 8.4/my.ini"),
                             QString("C:/ProgramData/MySQL/MySQL Server 8.0/my.ini")})
        if (QFile::exists(p)) return p;
    return "C:/ProgramData/MySQL/MySQL Server 8.4/my.ini";
#elif defined(Q_OS_LINUX)
    /*! Ubuntu (apt) : la section [mysqld] vit dans mysql.conf.d/mysqld.cnf. */
    for (const QString& p : {QString("/etc/mysql/mysql.conf.d/mysqld.cnf"),
                             QString("/etc/mysql/my.cnf")})
        if (QFile::exists(p)) return p;
    return "/etc/mysql/mysql.conf.d/mysqld.cnf";
#else
    /*! Installation Oracle (.dmg) : son mysqld, lancé par launchd SANS --defaults-file, lit la chaîne de
     *  config par défaut où /etc/my.cnf vient en PREMIER. On y écrit DONC nos réglages, sans se laisser
     *  détourner par un Homebrew présent (un Mac de dev a souvent brew, mais NOTRE serveur est l'Oracle —
     *  écrire dans le my.cnf de brew laisserait le serveur Oracle aveugle : cause de secure_file_priv=NULL
     *  et de mysql_native_password désactivé). */
    if (isOracleInstall())
        return "/etc/my.cnf";
    const QString prefix = getBrewPrefix();     /*!< MySQL installé via Homebrew (brew install mysql) */
    if (!prefix.isEmpty()) return prefix + "/etc/my.cnf";
    for (auto& p : {QString("/etc/my.cnf"),
                    QString("/etc/mysql/my.cnf"),
                    QString("/usr/local/mysql/etc/my.cnf"),
                    QString("/usr/local/etc/my.cnf")})
        if (QFile::exists(p)) return p;
    return "/opt/homebrew/etc/my.cnf";
#endif
}

/*! ── Helpers ──────────────────────────────────────────────────────────────────── */

/*!
 * \brief MySQLInstaller::askYesNo
 * Question Oui/Non via UpMessageBox::Question (boutons « Non »/« Oui »). true = « Oui ».
 * \param title  titre de la boîte
 * \param text   texte de la question
 */
bool MySQLInstaller::askYesNo(const QString& title, const QString& text)
{
    const UpSmallButton::StyleBouton rep = UpMessageBox::Question(
        m_dialog, title, text,
        UpDialog::ButtonCancel | UpDialog::ButtonOK,
        QStringList() << tr("Non") << tr("Oui"));
    return rep == UpSmallButton::STARTBUTTON;
}

QString MySQLInstaller::runCmd(const QString& cmd, int timeoutMs)
{
    QProcess p;
    startShellProcess(p, cmd);
    waitProcessResponsive(p, timeoutMs);
    return p.readAllStandardOutput().trimmed();
}

QString MySQLInstaller::runCmdFull(const QString& cmd, int timeoutMs)
{
    QProcess p;
    p.setProcessChannelMode(QProcess::MergedChannels);
    startShellProcess(p, cmd);
    waitProcessResponsive(p, timeoutMs);
    return p.readAllStandardOutput().trimmed();
}

/*!
 * \brief MySQLInstaller::lignesResultat
 * Lignes de données d'une sortie du client mysql : son « [Warning] Using a password… », fusionné par
 * runCmdFull, se ferait sinon passer pour un résultat.
 * \param sortie  sortie brute de runCmdFull
 */
QStringList MySQLInstaller::lignesResultat(const QString& sortie)
{
    QStringList lignes;
    for (const QString& ligne : sortie.split('\n', Qt::SkipEmptyParts)) {
        const QString l = ligne.trimmed();
        if (l.isEmpty())
            continue;
        /*! Diagnostics du client : jamais un nom de base ni de table. */
        if (l.contains("[Warning]") || l.contains("[ERROR]") || l.contains("[Note]"))
            continue;
        lignes << l;
    }
    return lignes;
}

bool MySQLInstaller::runCmdElevated(const QString& cmd, const QString& stdinData)
{
#if defined(Q_OS_WIN)
    /*! L'application Windows tourne déjà en tant qu'administrateur. */
    Q_UNUSED(stdinData);
    const QString out = runCmdFull(cmd);
    return !out.contains("Access is denied", Qt::CaseInsensitive)
        && !out.contains("denied",           Qt::CaseInsensitive);
#elif defined(Q_OS_LINUX)
    /*! Élévation via pkexec (invite graphique PolicyKit). La commande est déposée dans un script
     *  temporaire pour éviter les soucis de guillemets. */
    const QString scriptPath = QDir::tempPath() + "/mysqlinstaller_priv.sh";
    QFile s(scriptPath);
    if (!s.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;
    { QTextStream ts(&s); ts << "#!/bin/sh\n" << cmd << '\n'; }
    s.close();
    runCmd("chmod +x '" + scriptPath + "'");

    QString out;
    if (stdinData.isEmpty()) {
        out = runCmdFull("pkexec sh '" + scriptPath + "' 2>&1", 900000);
    } else {
        /*! stdinData transmis à l'entrée standard du processus élevé. */
        QProcess p;
        p.setProcessChannelMode(QProcess::MergedChannels);
        p.start("pkexec", QStringList{"sh", scriptPath});
        if (p.waitForStarted(60000)) {
            p.write(stdinData.toUtf8());
            p.closeWriteChannel();
            waitProcessResponsive(p, 900000);
            out = QString::fromLocal8Bit(p.readAll());
        }
    }
    QFile::remove(scriptPath);
    return !out.contains("Authentication failed", Qt::CaseInsensitive)
        && !out.contains("not authorized",        Qt::CaseInsensitive)
        && !out.contains("dismissed",             Qt::CaseInsensitive);
#else
    Q_UNUSED(stdinData);
    /*! Exécute « cmd » avec les droits administrateur via une seule invite macOS. */
    const QString scriptPath = QDir::tempPath() + "/mysqlinstaller_priv.sh";
    QFile s(scriptPath);
    if (!s.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;
    {
        QTextStream ts(&s);
        ts << "#!/bin/sh\n" << cmd << '\n';
    }
    s.close();
    runCmd("chmod +x '" + scriptPath + "'");

    /*! Timeout LARGE (15 min) : l'invite de mot de passe administrateur est interactive. Avec le défaut de
     *  runCmdFull (30 s), si l'invite tardait ou que saisie + opération dépassaient 30 s,
     *  waitProcessResponsive tuait osascript EN PLEIN MILIEU : la sortie tronquée ne contenant ni « User
     *  canceled » ni « execution error », on renvoyait « succès » à tort alors que le script (ex.
     *  désinstallation) n'avait pas fini → « MySQL toujours présent » (bug uninstall fantôme). */
    const QString out = runCmdFull(QString(
        "osascript -e 'do shell script \"%1\" with administrator privileges' 2>&1")
        .arg(scriptPath), 900000);
    QFile::remove(scriptPath);

    /*! Succès = l'utilisateur n'a pas annulé l'invite et osascript n'a pas échoué. */
    return !out.contains("User canceled", Qt::CaseInsensitive)
        && !out.contains("execution error", Qt::CaseInsensitive);
#endif
}

void MySQLInstaller::runLongOp(const QString& cmd, const QString& label, int timeoutMs)
{
    MySQLProgressDialog* dlg = new MySQLProgressDialog(label);
    dlg->show();
    QApplication::processEvents();

    QProcess proc;
    QEventLoop loop;
    QTimer timeout;
    timeout.setSingleShot(true);
    QObject::connect(&proc,    &QProcess::finished, &loop, &QEventLoop::quit);
    QObject::connect(&timeout, &QTimer::timeout,    [&]{ proc.kill(); loop.quit(); });
    startShellProcess(proc, cmd);
    if (timeoutMs > 0) timeout.start(timeoutMs);
    loop.exec();

    dlg->close();
    delete dlg;
}

/*!
 * \brief MySQLInstaller::runLongOpProgress
 * Variante de runLongOp() avec barre de progression réelle : la commande doit émettre sur sa sortie
 * standard des lignes « PROGRESS <fait> <total> ».
 * \param cmd        commande à exécuter
 * \param label      texte affiché dans la fiche
 * \param timeoutMs  délai max avant kill
 */
void MySQLInstaller::runLongOpProgress(const QString& cmd, const QString& label,
                                       int timeoutMs)
{
    MySQLProgressDialog dlg(label);
    dlg.show();
    QApplication::processEvents();

    QProcess proc;
    proc.setProcessChannelMode(QProcess::MergedChannels);
    QObject::connect(&proc, &QProcess::readyReadStandardOutput, [&]{
        while (proc.canReadLine()) {
            const QString line = QString::fromLocal8Bit(proc.readLine()).trimmed();
            if (line.startsWith("PROGRESS")) {
                const QStringList p = line.split(' ', Qt::SkipEmptyParts);
                if (p.size() >= 3)
                    dlg.setProgressBar(p.at(1).toLongLong(), p.at(2).toLongLong());
            }
        }
        QApplication::processEvents();
    });

    QEventLoop loop;
    QTimer timeout;
    timeout.setSingleShot(true);
    QObject::connect(&proc,    &QProcess::finished, &loop, &QEventLoop::quit);
    QObject::connect(&timeout, &QTimer::timeout,    [&]{ proc.kill(); loop.quit(); });
    startShellProcess(proc, cmd);
    if (timeoutMs > 0) timeout.start(timeoutMs);
    loop.exec();
}

/*!
 * \brief MySQLInstaller::downloadFile
 * Téléchargement HTTP(S) robuste : curl d'abord (RAPIDE, barre réelle), repli QNetworkAccessManager.
 * \param url    adresse à télécharger
 * \param dest   fichier de destination
 * \param label  texte affiché dans la fiche de progression
 */
bool MySQLInstaller::downloadFile(const QString& url, const QString& dest,
                                  const QString& label)
{
    /*! ── Tentative 1 : curl (RAPIDE) + barre réelle par sondage de la taille du fichier ──
     *  curl sature la ligne là où QNetworkAccessManager plafonne (TRÈS lent sur macOS). On le lance en
     *  arrière-plan (QProcess) et on pilote la barre en SONDANT la taille du fichier qui grossit — inutile
     *  de parser la barre texte de curl (sur stderr). Taille totale via un HEAD préalable (best effort ;
     *  sinon barre animée). Repli QNAM ci-dessous si curl est absent ou échoue. */
    {
        MySQLProgressDialog dlg(label);
        dlg.show();
        QApplication::processEvents();

        qint64 total = 0;
        {
            QProcess head;
            head.start("curl", {"-sIL", "--connect-timeout", "15", url});
            if (head.waitForFinished(20000)) {
                const QString h = QString::fromUtf8(head.readAllStandardOutput());
                QRegularExpression re("(?i)content-length:\\s*(\\d+)");   /*!< dernière valeur = après redirections */
                auto it = re.globalMatch(h);
                while (it.hasNext()) total = it.next().captured(1).toLongLong();
            }
        }

        QFile::remove(dest);
        QProcess proc;
        proc.start("curl", {"-fSL", "--connect-timeout", "20", "-o",
                            QDir::toNativeSeparators(dest), url});
        if (proc.waitForStarted(5000)) {           /*!< curl disponible → on l'utilise */
            QEventLoop loop;
            QObject::connect(&proc, &QProcess::finished, &loop, &QEventLoop::quit);
            QTimer poll;
            QObject::connect(&poll, &QTimer::timeout, &dlg,
                             [&]{ dlg.setProgress(QFileInfo(dest).size(), total); });
            poll.start(250);
            loop.exec();
            poll.stop();
            dlg.setProgress(QFileInfo(dest).size(), total);
            if (proc.exitStatus() == QProcess::NormalExit && proc.exitCode() == 0
                && QFileInfo(dest).size() > 0)
                return true;
        }
    }

    /*! ── Tentative 2 : QNetworkAccessManager (repli si curl indisponible / échoué) ───────── */
    {
        QFile file(dest);
        if (file.open(QIODevice::WriteOnly)) {
            MySQLProgressDialog dlg(label);
            dlg.show();
            QApplication::processEvents();

            QNetworkAccessManager nam;
            QNetworkRequest req{QUrl(url)};
            req.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                             QNetworkRequest::NoLessSafeRedirectPolicy);
            /*! Forcer HTTP/1.1 : le HTTP/2 de Qt fait parfois échouer un gros téléchargement en cours de
             *  route avec le CDN MySQL. */
            req.setAttribute(QNetworkRequest::Http2AllowedAttribute, false);
            req.setHeader(QNetworkRequest::UserAgentHeader, "MySQLInstaller/1.0");
            QNetworkReply* reply = nam.get(req);

            QEventLoop loop;
            /*! WATCHDOG anti-blocage : sans lui, si tous les octets arrivent (barre à 100 %) mais que la
             *  connexion ne se referme pas proprement (FIN retardé/perdu sur un lien lent), finished() n'est
             *  JAMAIS émis et loop.exec() reste figé — c'est le « figé à 100 % » observé. On (ré)arme un
             *  délai à chaque octet reçu ; passé ce délai SANS progrès, on abandonne (reply->abort() →
             *  finished en erreur → repli sur curl, qui a son propre timeout). */
            QTimer watchdog;
            watchdog.setSingleShot(true);
            QObject::connect(&watchdog, &QTimer::timeout, reply, &QNetworkReply::abort);
            QObject::connect(reply, &QNetworkReply::readyRead, [&]{
                file.write(reply->readAll());
            });
            QObject::connect(reply, &QNetworkReply::downloadProgress,
                             [&](qint64 r, qint64 t){
                watchdog.start(120000);     /*!< 120 s sans le moindre octet → on lâche QNAM */
                dlg.setProgress(r, t);
            });
            QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
            watchdog.start(120000);
            loop.exec();
            watchdog.stop();

            file.write(reply->readAll());      /*!< reliquat éventuel */
            file.close();
            const bool ok = (reply->error() == QNetworkReply::NoError);
            reply->deleteLater();
            if (ok && QFileInfo(dest).size() > 0)
                return true;
        }
    }

    /*! ── Tentative 3 : ultime repli curl bloquant (barre animée, TLS du système) ─────────── */
    QFile::remove(dest);
    runLongOp(QString("curl -fSL -o \"%1\" \"%2\"")
              .arg(QDir::toNativeSeparators(dest), url), label, 1800000);
    return QFile::exists(dest) && QFileInfo(dest).size() > 0;
}

/*!
 * \brief MySQLInstaller::hasNetworkAccess
 * Accès réseau (WAN) présent ? Connexion TCP/443 vers une IP publique fiable, SANS DNS.
 */
bool MySQLInstaller::hasNetworkAccess()
{
    const QStringList ips = {"1.1.1.1", "8.8.8.8"};
    for (const QString& ip : ips) {
        QTcpSocket sock;
        sock.connectToHost(ip, 443);
        if (sock.waitForConnected(3000)) {
            sock.abort();
            return true;
        }
    }
    return false;
}

/*!
 * \brief MySQLInstaller::checkDownloadConnectivity
 * Pré-requis réseau juste avant un téléchargement d'installation MySQL : accès WAN + résolution DNS de
 * l'hôte du lien. Affiche un message et renvoie false si l'un manque.
 * \param downloadUrl  lien de téléchargement à contrôler
 */
bool MySQLInstaller::checkDownloadConnectivity(const QString& downloadUrl)
{
    /*! 1. Accès WAN. */
    if (!hasNetworkAccess()) {
        UpMessageBox::Watch(nullptr, tr("Pas d'accès réseau"),
            tr("Absence d'accès réseau. Le programme ne peut pas télécharger "
               "le fichier d'installation de MySQL.\n\nFermeture du programme."));
        return false;
    }

    /*! 2. Résolution DNS de l'hôte du lien de téléchargement. */
    const QString host = QUrl(downloadUrl).host();
    const QHostInfo info = QHostInfo::fromName(host);
    if (info.error() != QHostInfo::NoError || info.addresses().isEmpty()) {
        UpMessageBox::Watch(nullptr, tr("Lien de téléchargement introuvable"),
            tr("Impossible de résoudre le lien de téléchargement. Le programme "
               "ne peut pas télécharger le fichier d'installation de MySQL.\n\n"
               "Fermeture du programme."));
        return false;
    }
    return true;
}

/*!
 * \brief MySQLInstaller::avertirTelechargementImpossible
 * Alarme commune quand le téléchargement automatique de MySQL n'aboutit pas (site inaccessible, blocage
 * CDN, coupure…). On explique à l'utilisateur qu'il peut installer MySQL lui-même et que Rufus prendra le
 * relais au prochain lancement.
 */
void MySQLInstaller::avertirTelechargementImpossible()
{
    UpMessageBox::Watch(nullptr, tr("Téléchargement de MySQL impossible"),
        tr("Rufus n'a pas réussi à télécharger MySQL : le site n'est pas accessible.") + "\n\n" +
        tr("Vous pouvez installer MySQL vous-même : téléchargez-le et installez-le sans rien configurer.") + "\n" +
        tr("Notez simplement le login et le mot de passe de l'utilisateur que vous avez créé.") + "\n" +
        tr("Relancez ensuite Rufus : il détectera la présence du serveur") + "\n" +
        tr("et vous demandera ce login et ce mot de passe.") + "\n" +
        tr("Avec ces renseignements, il se connectera au serveur") + "\n" +
        tr("et le configurera automatiquement pour son usage."));
}

QString MySQLInstaller::getBrewPrefix()
{
#if defined(Q_OS_WIN) || defined(Q_OS_LINUX)
    return {};   /*!< pas de Homebrew sous Windows/Linux */
#else
    if (m_brewPrefix.isNull()) {
        QProcess p;
        p.start("/bin/bash", {"-c", "brew --prefix mysql@8.4 2>/dev/null"});
        p.waitForFinished(10000);
        m_brewPrefix = p.readAllStandardOutput().trimmed();
        if (m_brewPrefix.isEmpty()) {
            for (auto& path : {QString("/opt/homebrew/opt/mysql@8.4"),
                                QString("/usr/local/opt/mysql@8.4")})
                if (QDir(path).exists()) { m_brewPrefix = path; break; }
        }
    }
    return m_brewPrefix;
#endif
}

QString MySQLInstaller::mysqlBin(const QString& binary)
{
#if defined(Q_OS_WIN)
    const QString oracle = oraclePrefix();
    if (!oracle.isEmpty()) {
        const QString full = oracle + "/bin/" + binary + ".exe";
        if (QFile::exists(full)) return full;
    }
    return binary;   /*!< supposé présent dans le PATH */
#else
    QString oracle = oraclePrefix();
    if (!oracle.isEmpty()) {
        QString full = oracle + "/bin/" + binary;
        if (QFile::exists(full)) return full;
    }
    QString prefix = getBrewPrefix();
    if (!prefix.isEmpty()) {
        QString full = prefix + "/bin/" + binary;
        if (QFile::exists(full)) return full;
    }
    return binary;
#endif
}
