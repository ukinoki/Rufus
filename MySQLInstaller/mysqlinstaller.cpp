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
#include <QJsonDocument>
#include <QJsonObject>
#include <QRandomGenerator>

#if defined(Q_OS_WIN)
#  define WIN32_LEAN_AND_MEAN
#  define NOMINMAX
#  include <windows.h>
#  include <shellapi.h>   // ShellExecuteEx (élévation UAC « runas »)
#endif

// Arguments de transport des clients mysql/mysqladmin pour les connexions
// AVEC IDENTIFIANTS. nbp est toujours mono-poste/local : on force TCP vers
// 127.0.0.1:3306, exactement comme s'y connecte le pilote Qt de Rufus (et comme
// MySQL Workbench). Sans cela, ces clients passent par le socket Unix local
// (hôte « localhost ») : un compte défini pour 'user'@'127.0.0.1' ou masqué par
// un compte anonyme ''@'localhost' se voit alors refusé, alors qu'il se connecte
// très bien en TCP. (À ne PAS utiliser pour « mysql -u root » via pkexec en mode
// Create, qui repose volontairement sur l'authentification socket d'Ubuntu.)
#define LOCAL_TCP_ARGS "--protocol=TCP -h 127.0.0.1 -P 3306"

// ═════════════════════════════════════════════════════════════════════════════
//  MySQLProgressDialog (porté de ProgressDialog)
// ═════════════════════════════════════════════════════════════════════════════
MySQLProgressDialog::MySQLProgressDialog(const QString& operation, QWidget* parent)
    : QDialog(parent, Qt::Dialog | Qt::WindowTitleHint | Qt::CustomizeWindowHint)
{
    setWindowTitle(tr("MySQL Installer"));
    setFixedWidth(420);
    setModal(true);

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(28, 28, 28, 28);
    root->setSpacing(18);

    m_label = new QLabel(operation);
    m_label->setStyleSheet("font-size: 14px; font-weight: 600; color: #1C1B18;");
    m_label->setWordWrap(true);
    m_label->setAlignment(Qt::AlignCenter);
    root->addWidget(m_label);

    m_progress = new QProgressBar();
    m_progress->setRange(0, 0);   // indéterminé (barre animée)
    m_progress->setFixedHeight(10);
    m_progress->setTextVisible(false);
    m_progress->setStyleSheet(R"(
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
    root->addWidget(m_progress);
}

void MySQLProgressDialog::setProgress(qint64 received, qint64 total)
{
    if (total <= 0) {                 // taille inconnue → barre animée
        m_progress->setRange(0, 0);
        return;
    }
    m_progress->setRange(0, 100);
    m_progress->setValue(int(received * 100 / total));
}

// ═════════════════════════════════════════════════════════════════════════════
//  MySQLInstallerDialog : checklist des 6 critères (composants Rufus up*)
// ═════════════════════════════════════════════════════════════════════════════
MySQLInstallerDialog::MySQLInstallerDialog(QWidget* parent)
    : UpDialog(parent)
{
    setWindowTitle(tr("Préparation de MySQL pour Rufus"));

    int row = 0;

    // Titre + sous-titre (libellés pilotés par les méthodes configurer*()).
    m_title = new UpLabel(this, tr("Préparation de MySQL pour Rufus"));
    m_title->setStyleSheet("font-size: 16px; font-weight: 700; color: #1C1B18;");
    m_title->setWordWrap(true);
    dlglayout()->insertWidget(row++, m_title);

    m_subtitle = new UpLabel(this, QString());
    m_subtitle->setStyleSheet("font-size: 12px; color: #1C1B18;");
    m_subtitle->setWordWrap(true);
    dlglayout()->insertWidget(row++, m_subtitle);

    // Saisie : identifiant + mot de passe (mêmes validateurs que dlg_paramconnexion).
    UpLabel* loginLbl = new UpLabel(this, tr("Identifiant :"));
    dlglayout()->insertWidget(row++, loginLbl);
    m_login = new UpLineEdit(this);
    m_login->setValidator(new QRegularExpressionValidator(Utils::rgx_AlphaNumeric_5_15, this));
    dlglayout()->insertWidget(row++, m_login);

    UpLabel* mdpLbl = new UpLabel(this, tr("Mot de passe :"));
    dlglayout()->insertWidget(row++, mdpLbl);
    m_mdp = new UpLineEdit(this);
    m_mdp->setValidator(new QRegularExpressionValidator(Utils::rgx_AlphaNumeric_5_12, this));
    m_mdp->setEchoMode(QLineEdit::Password);
    dlglayout()->insertWidget(row++, m_mdp);

    // Les 6 cases (affichage seul) insérées entre la saisie et les boutons.
    for (int i = 0; i < 6; i++) {
        m_steps[i] = new UpCheckBox();
        m_steps[i]->setToggleable(false);   // pilotée par l'engine, non cliquable
        dlglayout()->insertWidget(row++, m_steps[i]);
        applyStepLabel(i);
    }

    // Boutons « Annuler » + « OK » (le libellé d'OK est ajusté par configurer*()).
    // AjouteLayButtons relie déjà Annuler à reject() ; on relie OK à accept().
    AjouteLayButtons(UpDialog::ButtonCancel | UpDialog::ButtonOK);
    if (OKButton)
        connect(OKButton, &QPushButton::clicked, this, &MySQLInstallerDialog::accept);

    // Bouton « Supprimer MySQL » (mode Verify uniquement) : permet de tout remettre
    // à zéro — désinstaller le MySQL existant pour repartir sur une base neuve.
    // Placé à gauche (loin des boutons OK/Annuler) ; masqué par défaut, affiché par
    // configurerVerifyAdminMySQL(). Le clic clôt exec() avec ResultSupprimerMySQL.
    m_btnSupprMySQL = new UpSmallButton(tr("Supprimer MySQL"), this);
    m_btnSupprMySQL->setVisible(false);
    AjouteWidgetLayButtons(m_btnSupprMySQL, false);   // false = inséré tout à gauche
    connect(m_btnSupprMySQL, &QPushButton::clicked, this,
            [this]{ done(ResultSupprimerMySQL); });
}

// ── Configuration selon le contexte ──────────────────────────────────────────
void MySQLInstallerDialog::configurer(const QString& titre,
                                      const QString& sousTitre,
                                      const QString& okLabel)
{
    m_title->setText(titre);
    m_subtitle->setText(sousTitre);
    if (OKButton) OKButton->setText(okLabel);
    if (m_login) m_login->clear();
    if (m_mdp)   m_mdp->clear();
    if (m_login) m_login->setFocus();
    // « Supprimer MySQL » n'a de sens qu'en mode Verify : masqué par défaut, ré-
    // affiché explicitement par configurerVerifyAdminMySQL().
    if (m_btnSupprMySQL) m_btnSupprMySQL->setVisible(false);
}

//  Format imposé pour un identifiant Rufus à CRÉER (5-15 / 5-12 alphanumériques).
static void appliquerValidateursRufus(UpLineEdit* login, UpLineEdit* mdp, QObject* parent)
{
    login->setValidator(new QRegularExpressionValidator(Utils::rgx_AlphaNumeric_5_15, parent));
    mdp  ->setValidator(new QRegularExpressionValidator(Utils::rgx_AlphaNumeric_5_12, parent));
}

void MySQLInstallerDialog::configurerCreateUserRufus(const QString& minVersion)
{
    setMinVersion(minVersion);
    configurer(tr("Création de la base Rufus"),
               tr("Choisissez l'identifiant et le mot de passe que vous utiliserez "
                  "dans Rufus."),
               tr("Installer"));
    appliquerValidateursRufus(m_login, m_mdp, this);   // user Rufus → format imposé
}

void MySQLInstallerDialog::configurerVerifyAdminMySQL()
{
    configurer(tr("Connexion à MySQL"),
               tr("Un serveur MySQL existe déjà. Saisissez l'identifiant et le mot "
                  "de passe d'un compte MySQL administrateur (capable de créer des "
                  "utilisateurs)."),
               tr("Se connecter"));
    // Compte MySQL EXISTANT : aucun format imposé (ex. « root » — 4 car. —, ou mot de
    // passe non alphanumérique) → on RETIRE les validateurs.
    m_login->setValidator(nullptr);
    m_mdp  ->setValidator(nullptr);
    // Seul contexte où l'on propose de tout réinitialiser (désinstaller MySQL).
    if (m_btnSupprMySQL) m_btnSupprMySQL->setVisible(true);
}

void MySQLInstallerDialog::configurerNewUserRufus()
{
    configurer(tr("Compte utilisateur Rufus"),
               tr("Choisissez l'identifiant et le mot de passe que vous utiliserez "
                  "dans Rufus."),
               tr("Créer le compte"));
    appliquerValidateursRufus(m_login, m_mdp, this);   // user Rufus → format imposé
}

QString MySQLInstallerDialog::login() const
{
    return m_login ? m_login->text() : QString();
}

QString MySQLInstallerDialog::password() const
{
    return m_mdp ? m_mdp->text() : QString();
}

bool MySQLInstallerDialog::validerSaisie()
{
    if (login().isEmpty() || password().isEmpty()) {
        UpMessageBox::Watch(this, tr("Saisie incomplète"),
            tr("Veuillez renseigner un identifiant et un mot de passe."));
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
    }
    return {};
}

void MySQLInstallerDialog::applyStepLabel(int i)
{
    if (i < 0 || i > 5) return;
    QString label = baseStepLabel(i);
    if (!m_stepDetail[i].isEmpty())
        label += " : " + m_stepDetail[i];
    m_steps[i]->setText(label);
}

void MySQLInstallerDialog::checkStep(int i)
{
    if (i < 0 || i > 5) return;
    m_steps[i]->setChecked(true);
    QApplication::processEvents();
}

void MySQLInstallerDialog::uncheckAllSteps()
{
    for (int i = 0; i < 6; i++)
        m_steps[i]->setChecked(false);
    QApplication::processEvents();
}

void MySQLInstallerDialog::setStepDetail(int i, const QString& detail)
{
    if (i < 0 || i > 5) return;
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

// ═════════════════════════════════════════════════════════════════════════════
//  Helpers d'exécution dépendants de la plateforme (déclarés tôt : utilisés par
//  de nombreuses méthodes plus bas).
// ═════════════════════════════════════════════════════════════════════════════
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

//  Redirection « rien » pour masquer stderr selon la plateforme.
static inline QString NUL()
{
#if defined(Q_OS_WIN)
    return "2>nul";
#else
    return "2>/dev/null";
#endif
}

//  Démarre « cmd » dans le shell système.
//
//  Sous Windows, on passe la ligne de commande TELLE QUELLE à cmd.exe via
//  setNativeArguments() (QProcess ré-échapperait sinon les guillemets selon les
//  règles du runtime C, différentes de cmd.exe). De plus, on encadre toute la
//  commande d'une paire de guillemets externe : cmd.exe /C retire le premier et
//  le dernier guillemet dès qu'il y en a plus de deux ; sans cet encadrement,
//  une commande combinant un chemin quoté ET des arguments quotés (ex.
//  « "…mysqladmin.exe" -u "login" -p"pass" ») serait corrompue. Avec
//  « /C "…" », cmd retire la paire externe et exécute l'intérieur verbatim.
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

//  Attend la fin de « p » SANS geler l'interface : on laisse Qt rafraîchir la
//  fenêtre (événements de peinture uniquement, pas les clics → aucune réentrance)
//  pendant que la commande tourne. Évite le « (l'application ne répond pas) ».
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

//  Compare deux numéros de version « pointés » (ex. « 8.4.8 » vs « 8.4.3 »).
//  Renvoie true si « ver » est supérieur OU égal à « minVer », composant par
//  composant. Une version vide/inconnue est considérée comme inférieure.
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
    return true;   // égalité exacte => au moins la version minimale
}

// ═════════════════════════════════════════════════════════════════════════════
//  Mot de passe aléatoire (helpers statiques)
// ═════════════════════════════════════════════════════════════════════════════
//  Mot de passe alphanumérique fort (12 caractères, [A-Za-z0-9]), tiré d'un
//  générateur semé de façon sécurisée. 12 car. (et non 24) pour tenir dans la
//  contrainte de saisie 5-12 des champs login/mdp (mêmes que rufus.utilisateurs) :
//  62^12 ≈ 3·10^21 → reste largement assez fort pour ce compte.
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

//  Lit le mot de passe MySQL dans le fichier CACHÉ PATH_FILE_DBKEY (~/.rufus/.dbkey),
//  PAS dans Rufus.ini : il doit survivre à une réinitialisation par suppression de
//  Rufus.ini. Repli sur MDP_SQL (mot de passe historique) si absent/vide (base non
//  encore migrée).
//  Le fichier ne contient QUE le mot de passe BRUT (aucune étiquette « MDPSQL= »,
//  aucune section) : un contenu opaque ne révèle pas à quoi il sert. On garde
//  toutefois la lecture de l'ancien format INI (« [Connexion]/MDPSQL=… ») pour
//  rester compatible avec d'éventuels fichiers déjà écrits.
QString MySQLInstaller::motDePasseSQL()
{
    QFile f(PATH_FILE_DBKEY);
    if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        const QString contenu = QString::fromUtf8(f.readAll()).trimmed();
        f.close();
        // Ancien format QSettings/INI : détecté par la présence d'un « = » (le mot
        // de passe aléatoire, alphanumérique, n'en contient jamais).
        if (contenu.contains('=')) {
            QSettings sets(PATH_FILE_DBKEY, QSettings::IniFormat);
            const QString mdp = sets.value(Param_MDPSQL).toString();
            if (!mdp.isEmpty())
                return mdp;
        } else if (!contenu.isEmpty()) {
            return contenu;                   // format actuel : mot de passe brut
        }
    }
    return QString(MDP_SQL);                   // absent/vide → repli legacy
}

//  Stocke le mot de passe dans le fichier caché PATH_FILE_DBKEY (hors ~/Documents/Rufus).
//  Écriture du mot de passe BRUT, sans clé ni section (cf. motDePasseSQL).
void MySQLInstaller::stockerMotDePasse(const QString& mdp)
{
    QDir().mkpath(PATH_DIR_RUFUSKEY);     // dossier caché ~/.rufus (créé au besoin)
    QFile f(PATH_FILE_DBKEY);
    if (f.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        f.write(mdp.toUtf8());
        f.write("\n");
        f.close();
    }
}

// ═════════════════════════════════════════════════════════════════════════════
//  Config MySQL distante
// ═════════════════════════════════════════════════════════════════════════════
MySQLRemoteConfig MySQLInstaller::defaultMySQLConfig()
{
    MySQLRemoteConfig c;
    c.version     = "8.4.9";
    // Seuil minimal accepté en mode Verify. Sous Linux on est plus tolérant :
    // Rufus fonctionne bien avec MySQL 8.0 (la version fournie par apt), inutile
    // d'imposer 8.4. Sous Windows/macOS on installe 8.4, donc seuil 8.4.3.
#if defined(Q_OS_LINUX)
    c.minVersion  = "8.0";
#else
    c.minVersion  = "8.4.3";
#endif
    c.winUrl      = "https://dev.mysql.com/get/Downloads/MySQL-8.4/mysql-8.4.9-winx64.zip";
    c.macArm64Url = "https://dev.mysql.com/get/Downloads/MySQL-8.4/mysql-8.4.9-macos14-arm64.dmg";
    c.macX86Url   = "https://dev.mysql.com/get/Downloads/MySQL-8.4/mysql-8.4.9-macos14-x86_64.dmg";
    return c;
}

//  Charge la config MySQL distante la première fois, puis renvoie le cache.
//  Timeout 5 s ; en cas d'échec, utilise defaultMySQLConfig().
MySQLRemoteConfig MySQLInstaller::fetchRemoteConfig()
{
    if (m_remoteConfigLoaded)
        return m_remoteConfig;

    m_remoteConfig = defaultMySQLConfig();    // pré-remplir avec les valeurs de fallback

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
            // Sous Linux, un seuil dédié (plus tolérant) prime s'il est défini.
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

// ═════════════════════════════════════════════════════════════════════════════
MySQLInstaller::MySQLInstaller(QObject* parent)
    : QObject(parent)
{}

// ─────────────────────────────────────────────────────────────────────────────
//  Multi-plateforme : dossier partagé
// ─────────────────────────────────────────────────────────────────────────────
QString MySQLInstaller::sharedFolderPath()
{
#if defined(Q_OS_WIN)
    return "C:/Users/Public";   // slashes avant : acceptés par Qt et MySQL sous Windows
#else
    return "/Users/Shared";
#endif
}

// ─────────────────────────────────────────────────────────────────────────────
//  Pré-requis : droits administrateur
// ─────────────────────────────────────────────────────────────────────────────
bool MySQLInstaller::isAdminUser()
{
#if defined(Q_OS_WIN)
    // Le processus est-il élevé ? On interroge directement le jeton d'accès via
    // l'API Win32 (TokenElevation), fiable — contrairement à un appel PowerShell
    // passé par cmd.exe, dont les guillemets imbriqués étaient mal transmis et
    // faisaient systématiquement échouer la détection.
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
    // root, ou membre du groupe « sudo » (Ubuntu) / « admin ».
    if (runCmd("id -u 2>/dev/null").trimmed() == "0")
        return true;
    const QStringList groups =
        runCmd("id -Gn 2>/dev/null").split(QRegularExpression("\\s+"),
                                           Qt::SkipEmptyParts);
    return groups.contains("sudo") || groups.contains("admin");
#else
    // Les administrateurs macOS sont membres du groupe « admin » (gid 80).
    const QStringList groups =
        runCmd("id -Gn 2>/dev/null").split(QRegularExpression("\\s+"),
                                           Qt::SkipEmptyParts);
    return groups.contains("admin");
#endif
}

#if defined(Q_OS_WIN)
//  Relance l'exécutable Rufus courant AVEC élévation, via le verbe « runas » de
//  ShellExecuteEx : Windows affiche l'invite UAC. Sur un compte standard, l'invite
//  permet de SAISIR un compte administrateur — c'est exactement le comportement
//  demandé. true si l'instance élevée a démarré (UAC acceptée) ; false si l'utilisateur
//  a annulé (ERROR_CANCELLED) ou en cas d'échec.
bool MySQLInstaller::relancerEnAdministrateur()
{
    const QString exe = QDir::toNativeSeparators(QCoreApplication::applicationFilePath());
    const std::wstring exeW = exe.toStdWString();

    SHELLEXECUTEINFOW sei;
    ZeroMemory(&sei, sizeof(sei));
    sei.cbSize = sizeof(sei);
    sei.fMask  = SEE_MASK_NOCLOSEPROCESS;
    sei.lpVerb = L"runas";              // déclenche l'élévation UAC
    sei.lpFile = exeW.c_str();
    sei.nShow  = SW_SHOWNORMAL;

    if (ShellExecuteExW(&sei)) {
        if (sei.hProcess)
            CloseHandle(sei.hProcess);
        return true;                    // instance élevée lancée
    }
    return false;                       // annulation (UAC) ou échec
}
#endif

#if defined(Q_OS_LINUX)
//  Ubuntu 22.04 (LTS) ou ultérieure (selon /etc/os-release).
bool MySQLInstaller::isUbuntuVersionSupported()
{
    const QString ver =
        runCmd(". /etc/os-release 2>/dev/null; echo $VERSION_ID").trimmed();
    const QStringList p = ver.split('.');
    if (p.size() < 2)
        return false;
    const int major = p[0].toInt();
    const int minor = p[1].toInt();
    return (major > 22) || (major == 22 && minor >= 4);   // ≥ 22.04
}
#endif

#if defined(Q_OS_WIN)
// ─────────────────────────────────────────────────────────────────────────────
//  Windows : Visual C++ Redistributable 2022 (x64)
// ─────────────────────────────────────────────────────────────────────────────
bool MySQLInstaller::isVCRedist2022Installed()
{
    // Clé posée par le runtime VC++ 14.x (2015-2022, binairement compatibles).
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

//  Rend MySQL désinstallable depuis « Applications et fonctionnalités ».
void MySQLInstaller::registerWindowsUninstaller(const QString& base,
                                                const QString& progData,
                                                const QString& version)
{
    const QString nbase = QDir::toNativeSeparators(base);
    const QString nprog = QDir::toNativeSeparators(progData);
    const QString psPath = base + "/uninstall_rufus.ps1";
    const QString nps    = QDir::toNativeSeparators(psPath);
    const QString binDir = nbase + "\\bin";

    // 1. Script de désinstallation (PowerShell, ré-élève si lancé sans droits).
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

    // 2. Entrée « Applications et fonctionnalités ».
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

    // Taille installée (Ko) → colonne « Taille » du gestionnaire d'applications.
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
#endif  // Q_OS_WIN

// ═════════════════════════════════════════════════════════════════════════════
//  run() : point d'entrée synchrone. Phase 1 (pré-requis + détection + install
//  éventuelle), puis phase 2 (les 6 critères). Renvoie true si MySQL conforme.
// ═════════════════════════════════════════════════════════════════════════════
void MySQLInstaller::cleanupDialog()
{
    if (m_dialog) {
        m_dialog->close();
        delete m_dialog;
        m_dialog = nullptr;
    }
}

//  Garantit les droits administrateur, requis UNIQUEMENT pour installer ou
//  désinstaller MySQL. Si le processus n'est pas élevé, propose l'élévation UAC
//  (Windows) : l'utilisateur peut saisir un compte administrateur à cet instant ;
//  l'instance élevée prend alors le relais (exit de l'instance courante). Renvoie
//  true si on a (ou vient d'obtenir) les droits, false si l'utilisateur refuse.
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
            exit(0);                    // l'instance élevée prend le relais
        // UAC refusée / échec : on bascule sur la consigne manuelle.
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

bool MySQLInstaller::run()
{
    // NB : on ne vérifie PAS les droits administrateur ici. Ils ne sont requis que
    // pour INSTALLER ou DÉSINSTALLER MySQL. La vérification (et l'offre d'élévation
    // UAC) se fait donc juste avant ces opérations, via assurerDroitsAdmin() — pas
    // en tête de run(), sinon le mode Verify (qui ne fait que se connecter à un
    // MySQL déjà présent) serait bloqué pour rien sur un compte non-administrateur.

#if defined(Q_OS_WIN)
    // Windows : MySQL dépend de Visual C++ Redistributable 2022. On le vérifie et
    // on l'installe AVANT toute opération MySQL.
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
    // Linux : ce programme cible Ubuntu (≥ 22.04 LTS).
    if (!isUbuntuVersionSupported()) {
        UpMessageBox::Watch(nullptr,
            tr("Version d'Ubuntu non compatible"),
            tr("Ce programme nécessite Ubuntu 22.04 ou une version ultérieure."));
        return false;
    }
#endif

    // NB : on ne vérifie PAS l'accès réseau ici. Il n'est nécessaire que pour
    // TÉLÉCHARGER MySQL (mode Create), et ce cas est déjà couvert par
    // checkDownloadConnectivity() (appelé dans faireCreate(), qui appelle lui-même
    // hasNetworkAccess()). Vérifier le réseau dès run() bloquerait inutilement un
    // poste hors-ligne qui a DÉJÀ MySQL (mode Verify) ou une base Rufus complète.

    // Config distante (version cible + seuil minimal). La checklist affiche le
    // seuil dans le libellé de la case « MySQL ≥ <min> installé ».
    // fetchRemoteConfig() se replie sur les valeurs par défaut si le réseau manque.
    const MySQLRemoteConfig cfg = fetchRemoteConfig();

    const bool installed = isMySQLInstalled();

    // ════════════════════════════════════════════════════════════════════════
    //  A. Aucun MySQL → CRÉATION complète.
    // ════════════════════════════════════════════════════════════════════════
    if (!installed) {
        if (!askYesNo(tr("Installation de MySQL"),
                tr("Pour installer Rufus, il est nécessaire d'installer une base de "
                   "données MySQL sur cet ordinateur.\n\nVoulez-vous l'installer "
                   "maintenant ?")))
            return false;
        if (!assurerDroitsAdmin())   // l'installation exige les droits admin
            return false;
        return faireCreate(cfg);
    }

    // ════════════════════════════════════════════════════════════════════════
    //  MySQL présent : version conforme ?
    // ════════════════════════════════════════════════════════════════════════
    const QString ver = getMySQLVersion();
    if (!versionAtLeast(ver, cfg.minVersion)) {
        // Version trop ancienne : NETTOYAGE COMPLET (askUpdateConfirmation avertit
        // de la perte des données) puis on rejoint le chemin de création.
        if (!askUpdateConfirmation(ver, cfg.version))
            return false;
        if (!assurerDroitsAdmin())   // désinstallation + réinstallation = droits admin
            return false;
        MySQLProgressDialog* clean = new MySQLProgressDialog(
            tr("Nettoyage de l'ancienne installation de MySQL…"));
        clean->show();
        QApplication::processEvents();
        uninstallMySQL();
        clean->close();
        delete clean;
        UpMessageBox::Information(nullptr, tr("Désinstallation terminée"),
            tr("MySQL a été désinstallé de cet ordinateur."));
        return faireCreate(cfg);
    }

    // ── B1 : une base Rufus COMPLÈTE existe déjà ? Ce n'est pas le rôle de nbp. ─
    if (!isServerRunning()) startMySQL();
    if (baseRufusComplete()) {
        UpMessageBox::Information(nullptr, tr("Une base Rufus existe déjà"),
            tr("Un serveur MySQL contient déjà une base Rufus complète sur cet "
               "ordinateur.\n\nPour vous y connecter, utilisez « Base patients "
               "existante sur le serveur » depuis l'écran de premier démarrage."));
        return false;
    }

    // ── B2 : MySQL présent mais pas encore une base Rufus → demander un compte
    //         administrateur MySQL pour créer les comptes Rufus. ──────────────
    m_dialog = new MySQLInstallerDialog();
    m_dialog->configurerVerifyAdminMySQL();
    m_dialog->setMinVersion(cfg.minVersion);
    m_dialog->show();
    QApplication::processEvents();

    QString adminLogin, adminMdp;
    forever {
        const int res = m_dialog->exec();

        // L'utilisateur veut TOUT remettre à zéro : désinstaller le MySQL existant
        // (et ses données) pour repartir sur une installation neuve par Rufus.
        if (res == MySQLInstallerDialog::ResultSupprimerMySQL) {
            if (askYesNo(tr("Supprimer MySQL"),
                    tr("Cette opération va SUPPRIMER complètement MySQL de cet "
                       "ordinateur, ainsi que TOUTES les données qu'il contient. "
                       "Elle est IRRÉVERSIBLE.\n\nVoulez-vous continuer ?"))) {
                if (!assurerDroitsAdmin()) {   // la suppression exige les droits admin
                    m_dialog->show();          // élévation refusée → retour à la fiche
                    QApplication::processEvents();
                    continue;
                }
                cleanupDialog();
                MySQLProgressDialog* clean = new MySQLProgressDialog(
                    tr("Suppression de MySQL en cours…"));
                clean->show();
                QApplication::processEvents();
                uninstallMySQL();
                clean->close();
                delete clean;
                UpMessageBox::Information(nullptr, tr("Désinstallation terminée"),
                    tr("MySQL a été désinstallé de cet ordinateur.\n\nRufus va "
                       "maintenant installer une base neuve."));
                // MySQL absent → on rejoint le chemin de création complet.
                return faireCreate(cfg);
            }
            // Annulation de la suppression : on ré-affiche la fiche Verify.
            m_dialog->show();
            QApplication::processEvents();
            continue;
        }

        if (res != QDialog::Accepted) { cleanupDialog(); return false; }
        adminLogin = m_dialog->login();
        adminMdp   = m_dialog->password();
        if (!m_dialog->validerSaisie()) continue;
        if (tryConnectAs(adminLogin, adminMdp)) break;
        UpMessageBox::Watch(m_dialog, tr("Connexion impossible"),
            tr("Connexion refusée avec cet identifiant / mot de passe. Réessayez."));
    }

    // La saisie modale a masqué la fiche : on la ré-affiche pour que la checklist
    // se coche visiblement pendant la configuration.
    m_dialog->show();
    QApplication::processEvents();

    // Comptes Rufus techniques : login fixe LOGIN_SQL + mot de passe aléatoire.
    m_login    = LOGIN_SQL;
    m_password = genererMotDePasse();
    if (!isServerRunning()) startMySQL();

    const CreateUserResult r = createUserAvecAdmin(adminLogin, adminMdp);
    if (r == CreateUserResult::NoCreateUserRight) {
        UpMessageBox::Watch(m_dialog, tr("Droits insuffisants"),
            tr("Le compte MySQL « %1 » n'a pas le droit de créer des utilisateurs "
               "(CREATE USER). Utilisez un compte administrateur MySQL (par ex. "
               "root).").arg(adminLogin));
        cleanupDialog();
        return false;
    }
    if (r != CreateUserResult::Ok) {
        UpMessageBox::Watch(m_dialog, tr("Erreur"),
            tr("Impossible de créer les comptes Rufus."));
        cleanupDialog();
        return false;
    }
    // adminrufus/adminrufusSSL créés : on mémorise leur mot de passe aléatoire.
    stockerMotDePasse(m_password);
    m_comptesDejaCrees = true;

    m_dialog->checkStep(0);
    if (!executerEtapesConfig()) { cleanupDialog(); return false; }

    // Saisie du futur utilisateur applicatif Rufus (2e étape).
    m_dialog->configurerNewUserRufus();
    forever {
        if (m_dialog->exec() != QDialog::Accepted) { cleanupDialog(); return false; }
        if (m_dialog->validerSaisie()) break;
    }
    m_loginRufus = m_dialog->login();
    m_mdpRufus   = m_dialog->password();

    cleanupDialog();
    return true;
}

// ═════════════════════════════════════════════════════════════════════════════
//  Chemin « création » : MySQL absent (ou trop vieux après nettoyage). Saisie du
//  futur utilisateur Rufus, installation de MySQL, création d'adminrufus
//  (root/pkexec) puis configuration. true si la base Rufus est prête.
// ═════════════════════════════════════════════════════════════════════════════
bool MySQLInstaller::faireCreate(const MySQLRemoteConfig& cfg)
{
    m_dialog = new MySQLInstallerDialog();
    m_dialog->configurerCreateUserRufus(cfg.minVersion);
    m_dialog->show();
    QApplication::processEvents();

    // Saisie du futur utilisateur applicatif Rufus.
    forever {
        if (m_dialog->exec() != QDialog::Accepted) { cleanupDialog(); return false; }
        if (m_dialog->validerSaisie()) break;
    }
    m_loginRufus = m_dialog->login();
    m_mdpRufus   = m_dialog->password();

    // La saisie modale (exec()) a masqué la fiche : on la ré-affiche pour que la
    // checklist se coche visiblement pendant l'installation et la configuration.
    m_dialog->show();
    QApplication::processEvents();

    // Comptes Rufus techniques : login fixe + mot de passe aléatoire (fixé AVANT
    // createUser/prepareCreateMode* qui s'en servent).
    m_login    = LOGIN_SQL;
    m_password = genererMotDePasse();

    // Pré-requis réseau : sans accès WAN ou si le lien de téléchargement ne se
    // résout pas, l'installation est impossible.
    QString dlUrl;
#if defined(Q_OS_WIN)
    dlUrl = cfg.winUrl;
#elif defined(Q_OS_MACOS)
    dlUrl = (runCmd("uname -m 2>/dev/null").trimmed() == "arm64")
                ? cfg.macArm64Url : cfg.macX86Url;
#else
    // Linux (apt) : à défaut d'URL directe, on contrôle au moins la résolution de
    // l'hôte officiel MySQL.
    dlUrl = cfg.winUrl;
#endif
    if (!checkDownloadConnectivity(dlUrl)) { cleanupDialog(); return false; }

    if (!installMySQL()) {
        // installMySQL() affiche déjà un message détaillé en cas d'échec.
        cleanupDialog();
        return false;
    }
    m_freshInstall = true;
    startMySQL();
    m_dialog->checkStep(0);

    if (!executerEtapesConfig()) { cleanupDialog(); return false; }

    // Comptes adminrufus/adminrufusSSL créés avec le mot de passe aléatoire : on le
    // mémorise dans le fichier caché pour les connexions ultérieures.
    stockerMotDePasse(m_password);

    cleanupDialog();
    return true;
}

// ═════════════════════════════════════════════════════════════════════════════
//  Étapes de configuration post-install/verify. true si toutes validées.
//  En mode CREATE (absent) : les comptes sont créés ici par createUser()
//  (root/pkexec). En mode VERIFY (B2) : ils l'ont déjà été par createUserAvecAdmin()
//  → on NE recrée PAS (garde m_comptesDejaCrees).
// ═════════════════════════════════════════════════════════════════════════════
bool MySQLInstaller::executerEtapesConfig()
{
    // Login = LOGIN_SQL. m_password a déjà été fixé (aléatoire) par l'appelant.
    m_login = LOGIN_SQL;

    if (!isServerRunning()) startMySQL();

#if defined(Q_OS_MACOS)
    // macOS, installation neuve : regrouper TOUT le paramétrage root (my.cnf,
    // /etc/paths.d, dossier + partage SMB, redémarrage) en UNE seule invite admin,
    // AVANT les étapes ci-dessous qui, sinon, élèveraient chacune séparément.
    if (m_freshInstall)
        prepareCreateModeMacOS();
#endif

    // ── Étape 2 : le chemin de mysql est dans la variable d'environnement PATH ─
    if (!ensureMysqlInPath()) {
        UpMessageBox::Watch(m_dialog, tr("PATH non configuré"),
            tr("Impossible d'ajouter le chemin de mysql à la variable PATH."));
        return false;
    }
    if (m_dialog->wasCancelled()) return false;
    m_dialog->checkStep(1);

#if defined(Q_OS_LINUX)
    // Installation neuve sous Linux : regrouper TOUT le paramétrage root en UNE
    // seule élévation, pour ne demander le mot de passe système qu'une fois.
    if (m_freshInstall)
        prepareCreateModeLinux();   // (affiche son propre indicateur + attend mysqld)
#endif

    // Installation neuve : créer l'utilisateur — les étapes suivantes ont besoin
    // d'une connexion valide. (Sous Linux, createUser() court-circuite si
    // prepareCreateModeLinux l'a créé.) En mode VERIFY (B2), les comptes ont déjà
    // été créés par createUserAvecAdmin() → on ne les recrée pas.
    if (m_freshInstall && !m_comptesDejaCrees && !createUser()) {
        UpMessageBox::Watch(m_dialog, tr("Création d'utilisateur impossible"),
            tr("Impossible de créer l'utilisateur '%1'.").arg(m_login));
        return false;
    }

    // ── Étape 3 : le dossier partagé existe et est partagé ────────────────
    if (!setupSharedFolder()) {
        UpMessageBox::Watch(m_dialog, tr("Dossier partagé impossible"),
            tr("Impossible de créer ou de partager le dossier %1.")
            .arg(sharedFolderPath()));
        return false;
    }
    // Révèle le chemin du dossier partagé en face de la case (une fois cochée).
    m_dialog->setStepDetail(2, QDir::toNativeSeparators(sharedFolderPath()));
    if (m_dialog->wasCancelled()) return false;
    m_dialog->checkStep(2);

    // ── Étape 4 : secure_file_priv pointe sur le dossier partagé ──────────
    if (!ensureSecureFilePriv()) {
        UpMessageBox::Watch(m_dialog, tr("secure_file_priv impossible"),
            tr("Impossible de configurer secure_file_priv sur %1.")
            .arg(sharedFolderPath()));
        return false;
    }
    if (m_dialog->wasCancelled()) return false;
    m_dialog->checkStep(3);

    // ── Étape 5 : mysql lit et écrit dans le dossier (fichier test) ───────
    while (!testSharedFolderRW()) {
        if (!tryConnect()) {
            UpMessageBox::Watch(m_dialog, tr("Connexion impossible"),
                tr("Connexion impossible avec le login « %1 ».\n"
                   "Vérifiez le login et le mot de passe.").arg(m_login));
            return false;
        }
#if defined(Q_OS_MACOS)
        // macOS : l'échec vient en général de l'absence de « Full Disk Access ».
        if (!guideMysqldFullDiskAccess()) {
            UpMessageBox::Watch(m_dialog, tr("Écriture impossible"),
                tr("mysql ne parvient pas à écrire dans %1.\n"
                   "Accordez l'accès complet au disque à mysqld, ou vérifiez le "
                   "privilège FILE de « %2 ».").arg(sharedFolderPath(), m_login));
            return false;
        }
        restartMySQL();   // appliquer l'accès nouvellement accordé au démon
#else
        // Windows / Linux : pas de notion de « Full Disk Access ».
        UpMessageBox::Watch(m_dialog, tr("Écriture impossible"),
            tr("mysql ne parvient pas à écrire dans %1.\n"
               "Vérifiez les droits du dossier et le privilège FILE de « %2 ».")
            .arg(sharedFolderPath(), m_login));
        return false;
#endif
    }
    if (m_dialog->wasCancelled()) return false;
    m_dialog->checkStep(4);

    // ── Étape 6 : privilèges ALL + GRANT OPTION ───────────────────────────
    QStringList missing;
    if (!checkPrivileges(missing)) {
        UpMessageBox::Watch(m_dialog, tr("Privilèges manquants"),
            tr("Privilège(s) manquant(s) pour « %1 » : %2")
            .arg(m_login, missing.join(", ")));
        return false;
    }
    if (m_dialog->wasCancelled()) return false;
    m_dialog->checkStep(5);

    // ── Toutes les étapes de configuration sont validées ─────────────────────
    return true;
}

// Désinstalle MySQL et la configuration ajoutée par l'outil. NON destructif pour
// le dossier partagé /Users/Shared (susceptible de contenir les données de
// l'utilisateur). Une seule élévation par plateforme.
bool MySQLInstaller::uninstallMySQL()
{
#if defined(Q_OS_WIN)
    // L'application tourne déjà en administrateur : arrêt + suppression du service,
    // des dossiers (binaires + données), de l'entrée PATH et de la clé de registre.
    const QString base = "C:/Program Files/MySQL/MySQL Server 8.4";
    const QString prog = "C:/ProgramData/MySQL/MySQL Server 8.4";
    const QString bin  = QDir::toNativeSeparators(base + "/bin");
    const QString ps =
        "$ErrorActionPreference='SilentlyContinue';"
        "net stop MySQL;"
        "sc.exe delete MySQL;"
        "$bin='" + bin + "';"
        "$p=[Environment]::GetEnvironmentVariable('Path','Machine');"
        "if($p){[Environment]::SetEnvironmentVariable('Path',"
        "(($p -split ';' | Where-Object {$_ -and $_ -ne $bin}) -join ';'),'Machine')};"
        "Remove-Item -LiteralPath '" + QDir::toNativeSeparators(base) + "' -Recurse -Force;"
        "Remove-Item -LiteralPath '" + QDir::toNativeSeparators(prog) + "' -Recurse -Force;"
        "Remove-Item -Path 'HKLM:\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion"
        "\\Uninstall\\MySQLForRufus' -Recurse -Force";
    runCmdFull("powershell -NoProfile -ExecutionPolicy Bypass -Command \"" + ps + "\"",
               300000);
    return !isMySQLInstalled();
#elif defined(Q_OS_LINUX)
    // apt purge (par motif individuel) + suppression données/config. On RETIRE le
    // partage [Rufus] de Samba mais on NE purge PAS samba/wsdd ni le dossier
    // partagé (l'utilisateur peut s'en servir par ailleurs).
    const QString script =
        "systemctl stop mysql mysqld mariadb 2>/dev/null;"
        "for pat in 'mysql-server.*' 'mysql-client.*' 'libmysqlclient.*' "
                   "'mysql-common' 'mysql.*' 'mariadb.*'; do "
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
    // macOS (installeur Oracle .dmg) : arrêt du démon, suppression du pkg (toutes
    // versions via glob), config et reçus pkgutil. Le compte système _mysql (natif
    // à macOS) est conservé.
    const QString script =
        "launchctl bootout system "
          "/Library/LaunchDaemons/com.oracle.oss.mysql.mysqld.plist 2>/dev/null || "
        "launchctl unload -w "
          "/Library/LaunchDaemons/com.oracle.oss.mysql.mysqld.plist 2>/dev/null;"
        "pkill -f /usr/local/mysql/bin/mysqld 2>/dev/null; sleep 2;"
        "rm -f /Library/LaunchDaemons/com.oracle.oss.mysql.mysqld.plist;"
        "rm -rf /Library/PreferencePanes/MySQL.prefPane;"
        "rm -rf /usr/local/mysql /usr/local/mysql-*;"   // symlink + dossier(s) versionné(s)
        "rm -f /etc/my.cnf /etc/paths.d/mysql /tmp/mysql.sock /tmp/mysql.sock.lock;"
        "for p in com.mysql.launchd com.mysql.mysql com.mysql.prefpane "
                 "com.oracle.oss.mysql.mysqld; do pkgutil --forget \"$p\" 2>/dev/null; done";
    runCmdElevated(script);
    return !isMySQLInstalled();
#endif
}

// ─────────────────────────────────────────────────────────────────────────────
//  MySQL : détection, installation, démarrage
// ─────────────────────────────────────────────────────────────────────────────
bool MySQLInstaller::isMySQLInstalled()
{
#if defined(Q_OS_WIN)
    if (!oraclePrefix().isEmpty())
        return true;
    if (runCmd("sc query MySQL " + NUL()).contains("SERVICE_NAME", Qt::CaseInsensitive))
        return true;
    return runCmd("mysql --version " + NUL()).contains("mysql", Qt::CaseInsensitive);
#elif defined(Q_OS_LINUX)
    // Détecter le SERVEUR (mysql-server / mariadb-server), PAS le client seul.
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

//  Vérifie que le dossier de l'exécutable mysql figure dans la variable PATH.
//  Sinon l'y ajoute de façon persistante (écriture privilégiée ; admin requis).
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
    // Sous Ubuntu, apt installe mysql dans /usr/bin (déjà dans le PATH). Au cas
    // où, on persiste via /etc/profile.d/mysql.sh.
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
    // Emplacement par défaut de l'installation MySQL 8.4 sous Windows.
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

QString MySQLInstaller::getMySQLVersion()
{
    QString out = runCmd("\"" + mysqlBin("mysql") + "\" --version " + NUL());
    QRegularExpression re(R"(Distrib\s+([\d.]+))");
    auto m = re.match(out);
    if (m.hasMatch()) return m.captured(1);
    re = QRegularExpression(R"(Ver\s+([\d.]+))");
    m = re.match(out);
    return m.hasMatch() ? m.captured(1) : QString();
}

QString MySQLInstaller::downloadOracleDmg()
{
    const MySQLRemoteConfig cfg = fetchRemoteConfig();

    const QString arch    = runCmd("uname -m 2>/dev/null").trimmed();
    const QString baseUrl = (arch == "arm64") ? cfg.macArm64Url : cfg.macX86Url;

    // Oracle nomme ses DMG avec un suffixe « macosNN » correspondant à la version
    // de macOS contre laquelle le binaire est compilé. On construit une liste de
    // candidats et on retient la première réellement servie par Oracle.
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

    QString url = baseUrl;   // dernier recours si aucune sonde n'aboutit
    for (const QString& cand : candidates) {
        // -r 0-0 : ne télécharge qu'un octet ; -L : suit la redirection vers le
        // miroir CDN ; on inspecte le code HTTP final (200 ou 206 = disponible).
        const QString code = runCmdFull(
            QString("curl -sSL -r 0-0 -o /dev/null -w '%{http_code}' '%1' 2>/dev/null")
                .arg(cand)).trimmed();
        if (code.endsWith("200") || code.endsWith("206")) { url = cand; break; }
    }

    const QString fileName = url.section('/', -1);
    QString tmpDmg         = QDir::tempPath() + "/" + fileName;

    runLongOp(
        QString("curl -fSL --progress-bar -o '%1' '%2' 2>&1").arg(tmpDmg, url),
        tr("Téléchargement de MySQL %1 (Oracle)…").arg(cfg.version),
        600000);

    if (!QFile::exists(tmpDmg) || QFileInfo(tmpDmg).size() < 1'000'000LL) {
        QFile::remove(tmpDmg);
        return {};
    }
    return tmpDmg;
}

bool MySQLInstaller::installFromDmg(const QString& dmgPath)
{
    // Montage
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

    // Localiser le .pkg
    QString pkgPath;
    for (const QString& f : QDir(volumePath).entryList({"*.pkg"}, QDir::Files)) {
        pkgPath = volumePath + "/" + f; break;
    }
    if (pkgPath.isEmpty()) {
        runCmd(QString("hdiutil detach '%1' -force 2>/dev/null").arg(volumePath));
        QFile::remove(dmgPath);
        return false;
    }

    // Journal d'init (réinitialisé ici : installation neuve).
    { QFile f(m_initLog);
      if (f.open(QIODevice::WriteOnly | QIODevice::Text))
          QTextStream(&f) << "== installFromDmg : installer + init + start ==\n"
                          << "pkg = " << pkgPath << "\n"; }

    // Script shell temporaire : installateur PUIS init du datadir + démarrage du
    // serveur, le tout dans la MÊME élévation → une seule invite de mot de passe.
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

    // Installation avec élévation
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

    waitForMySQL(20);             // le script a aussi initialisé et démarré le serveur
    return isOracleInstall();
}

// Fragment shell (exécuté en ROOT) qui, depuis MySQL 8.4.x, supplée le pkg Oracle
// macOS — lequel n'initialise plus automatiquement /usr/local/mysql/data.
QString MySQLInstaller::oracleInitStartScript() const
{
    return QStringLiteral(
        "PREFIX=/usr/local/mysql; DATA=\"$PREFIX/data\"\n"
        // Compte exécutant mysqld : « _mysql » (créé par le pkg), repli « mysql ».
        "OWNER=$(id -u _mysql >/dev/null 2>&1 && echo _mysql || echo mysql)\n"
        "{\n"
        "  echo \"-- init+start (owner=$OWNER) --\"\n"
        "  if [ ! -f \"$DATA/mysql.ibd\" ]; then\n"
        "    id \"$OWNER\" || echo \"(compte $OWNER introuvable)\"\n"
        "    rm -rf \"$DATA\"; mkdir -p \"$DATA\"; chown -R \"$OWNER\":\"$OWNER\" \"$DATA\"\n"
        // TMPDIR de session = /var/folders/<user>/T (privé, inaccessible à _mysql →
        // InnoDB échoue errno 13). On force /tmp (sticky) pour l'env ET via --tmpdir.
        "    export TMPDIR=/tmp\n"
        "    \"$PREFIX/bin/mysqld\" --no-defaults --initialize-insecure "
        "--user=\"$OWNER\" --basedir=\"$PREFIX\" --datadir=\"$DATA\" --tmpdir=/tmp\n"
        "    echo \"mysqld --initialize rc=$?\"\n"
        "    chown -R \"$OWNER\":\"$OWNER\" \"$DATA\"\n"
        "  else\n"
        "    echo 'datadir déjà initialisé'\n"
        "  fi\n"
        // Démarrage (root) : LaunchDaemon si présent (persiste au redémarrage),
        // sinon mysql.server (TMPDIR=/tmp en renfort contre l'errno 13).
        "  PLIST=/Library/LaunchDaemons/com.oracle.oss.mysql.mysqld.plist\n"
        "  if [ -f \"$PLIST\" ]; then echo '-- launchctl load --'; launchctl load -w \"$PLIST\"; "
        "else echo '-- mysql.server start --'; TMPDIR=/tmp \"$PREFIX/support-files/mysql.server\" start; fi\n"
        "} >> '%1' 2>&1\n"
        "chmod 644 '%1' 2>/dev/null\n").arg(m_initLog);
}

// Initialise (si besoin) le datadir Oracle PUIS démarre le serveur, en UNE élévation.
bool MySQLInstaller::initOracleDataDir()
{
    const QString prefix = oraclePrefix();          // /usr/local/mysql (ou vide)
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
    if (ok) waitForMySQL(20);            // le script root a démarré le serveur
    return ok;
}

bool MySQLInstaller::installMySQL()
{
#if defined(Q_OS_WIN)
    // ── Installation Windows par archive ZIP (entièrement scriptable) ─────────
    const MySQLRemoteConfig cfg = fetchRemoteConfig();
    const QString version  = cfg.version;
    const QString url      = cfg.winUrl;
    const QString zipName  = url.section('/', -1);
    const QString innerDir = zipName.chopped(4);   // retire ".zip" → dossier racine du zip
    const QString zipPath  = QDir::tempPath() + "/" + zipName;
    const QString extract  = QDir::tempPath() + "/mysql_extract";

    const QString base     = "C:/Program Files/MySQL/MySQL Server 8.4";
    const QString progData = "C:/ProgramData/MySQL/MySQL Server 8.4";
    const QString dataDir  = progData + "/Data";
    const QString cnfPath  = progData + "/my.ini";
    const QString mysqld   = base + "/bin/mysqld.exe";

    //  Lit le journal d'erreur le plus récent du serveur (datadir/*.err).
    auto lastErrLog = [&]() -> QString {
        const auto errs = QDir(dataDir).entryInfoList(
            {"*.err"}, QDir::Files, QDir::Time);
        if (errs.isEmpty()) return {};
        QFile lf(errs.first().absoluteFilePath());
        if (!lf.open(QIODevice::ReadOnly | QIODevice::Text)) return {};
        return QString::fromLocal8Bit(lf.readAll()).right(1500);
    };

    // 0. Nettoyage défensif (autorise les ré-essais).
    runCmdFull("net stop MySQL " + NUL());
    runCmdFull("sc delete MySQL " + NUL());
    runCmdFull("taskkill /F /IM mysqld.exe " + NUL());
    // Laisser Windows libérer les verrous de fichiers avant de supprimer.
    runCmd("powershell -NoProfile -Command \"Start-Sleep -Seconds 2\"");
    runCmd(QString("powershell -NoProfile -Command \""
        "Remove-Item -LiteralPath '%1','%2','%3' -Recurse -Force "
        "-ErrorAction SilentlyContinue\"")
        .arg(QDir::toNativeSeparators(base),
             QDir::toNativeSeparators(progData),
             QDir::toNativeSeparators(extract)));

    // 1. Téléchargement de l'archive ZIP (~250 Mo) avec barre de progression.
    downloadFile(url, zipPath, tr("Téléchargement de MySQL %1…").arg(version));
    if (!QFile::exists(zipPath) || QFileInfo(zipPath).size() < 1'000'000LL) {
        QFile::remove(zipPath);
        UpMessageBox::Watch(nullptr, tr("Téléchargement échoué"),
            tr("Impossible de télécharger MySQL %1 depuis dev.mysql.com.\n"
               "Vérifiez votre connexion Internet.").arg(version));
        return false;
    }

    // 2. Extraction entrée par entrée (avec barre de progression) puis déplacement.
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

    // 3. Fichier de configuration minimal (basedir + datadir).
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
           << "port=3306\n";
    }

    // 4. Initialisation du datadir (crée root@localhost SANS mot de passe).
    runLongOp(QString("\"%1\" --defaults-file=\"%2\" --initialize-insecure --console")
              .arg(QDir::toNativeSeparators(mysqld), QDir::toNativeSeparators(cnfPath)),
              tr("Initialisation de la base de données,\n"
                 "cela peut prendre quelques instants…"), 300000);
    if (!QFile::exists(dataDir + "/mysql")) {     // schéma système créé ?
        UpMessageBox::Watch(nullptr, tr("Initialisation échouée"),
            tr("L'initialisation du datadir MySQL a échoué.\n\n%1").arg(lastErrLog()));
        return false;
    }

    // 5. Enregistrement et démarrage du service Windows.
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

    // Rendre MySQL désinstallable depuis « Applications et fonctionnalités ».
    registerWindowsUninstaller(base, progData, version);
    return true;
#elif defined(Q_OS_LINUX)
    // Installation via apt-get (droits root → pkexec), avec barre de progression
    // RÉELLE : apt écrit son avancement (0-100) sur APT::Status-Fd.
    const QString aptScript = QDir::tempPath() + "/mysql_apt_install.sh";
    {
        QFile f(aptScript);
        if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream ts(&f);
            ts << "#!/bin/sh\n"
               << "apt-get update >/dev/null 2>&1\n"
               << "DEBIAN_FRONTEND=noninteractive apt-get -o APT::Status-Fd=1 "
                  "install -y mysql-server 2>/dev/null | "
                  "awk -F: '/^(pmstatus|dlstatus)/ "
                  "{ printf \"PROGRESS %d 100\\n\", $3; fflush() }'\n";
            f.close();
        }
    }
    runLongOpProgress("pkexec sh '" + aptScript + "'",
                      tr("Installation de MySQL via apt-get…"), 900000);
    QFile::remove(aptScript);
    return isMySQLInstalled();
#else
    QString dmg = downloadOracleDmg();
    if (dmg.isEmpty()) return false;
    // installFromDmg installe le pkg PUIS initialise le datadir et démarre le
    // serveur dans la même élévation.
    if (!installFromDmg(dmg)) return false;

    // L'init a-t-elle réussi ? (mysql.ibd = dictionnaire de données InnoDB.)
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
    // Déjà démarré ? (isServerRunning gère le cas auth_socket d'Ubuntu.)
    if (isServerRunning())
        return true;

#if defined(Q_OS_WIN)
    runCmdElevated("net start MySQL");
#elif defined(Q_OS_LINUX)
    runCmdElevated("systemctl start mysql");
#else
    if (isOracleInstall()) {
        // Démon système macOS : démarrage (et init du datadir si nécessaire) en
        // UNE élévation idempotente.
        initOracleDataDir();
    } else {
        runCmdFull("brew services start mysql@8.4 2>&1", 15000);
    }
#endif
    return waitForMySQL(30);
}

void MySQLInstaller::stopMySQL()
{
    if (!isServerRunning())   // gère le cas auth_socket d'Ubuntu
        return;
#if defined(Q_OS_WIN)
    runCmdElevated("net stop MySQL");
#elif defined(Q_OS_LINUX)
    runCmdElevated("systemctl stop mysql");
#else
    if (isOracleInstall()) {
        QString plist = "/Library/LaunchDaemons/com.oracle.oss.mysql.mysqld.plist";
        if (QFile::exists(plist))
            runCmdFull(QString("launchctl unload '%1' 2>&1").arg(plist), 15000);
        else
            runCmdFull("/usr/local/mysql/support-files/mysql.server stop 2>&1", 15000);
    } else {
        runCmdFull("brew services stop mysql@8.4 2>&1", 15000);
    }
#endif
    QEventLoop loop;
    QTimer::singleShot(2000, &loop, &QEventLoop::quit);
    loop.exec();
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
    // Service Windows : arrêt puis démarrage (l'app est déjà élevée).
    runCmdElevated("net stop MySQL & net start MySQL");
#elif defined(Q_OS_LINUX)
    runCmdElevated("systemctl restart mysql");
#else
    if (isOracleInstall()) {
        // Démon système (/Library/LaunchDaemons) : le redémarrage exige root.
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

// ─────────────────────────────────────────────────────────────────────────────
//  Dossier partagé /Users/Shared : secure_file_priv + test lecture/écriture
// ─────────────────────────────────────────────────────────────────────────────
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

    // Déjà toutes configurées ? → aucune élévation.
    bool allOk = true;
    for (const auto& kv : vars)
        if (getCnfVar(kv.first) != kv.second) { allOk = false; break; }
    if (allOk)
        return true;

    const QString tmp = writeCnfToTemp(vars);
    if (tmp.isEmpty())
        return false;
    const QString path = getCnfPath();
    bool ok;

#if defined(Q_OS_LINUX)
    // Copie + redémarrage en UNE SEULE élévation (pkexec).
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

    if (!ok)
        return false;
    return getCnfVar("secure_file_priv") == target;
}

//  Vérifie que mysql sait ÉCRIRE puis RELIRE un fichier dans /Users/Shared.
bool MySQLInstaller::testSharedFolderRW()
{
    const QString sub   = sharedFolderPath() + "/.mysql_rwtest";
    const QString token = "MYSQLD_RW_OK";
    const QString file  = sub + "/probe.txt";   // écrit ET relu par le serveur

    QDir().mkpath(sub);
#if !defined(Q_OS_WIN)
    // macOS : rendre le sous-dossier inscriptible par le compte _mysql (0777).
    runCmd("chmod 777 '" + sub + "'");
#endif

    // ── Écriture par le serveur ───────────────────────────────────────────────
    QFile::remove(file);   // INTO OUTFILE refuse un fichier existant
    runCmdFull(QString(
        "\"%1\" " LOCAL_TCP_ARGS " -u \"%2\" -p\"%3\" -N -B -e \"SELECT '%4' INTO OUTFILE '%5';\" 2>&1")
        .arg(mysqlBin("mysql"), m_login, m_password, token, file));

    // ── Relecture par le serveur (et non par l'app) ───────────────────────────
    const QString out = runCmdFull(QString(
        "\"%1\" " LOCAL_TCP_ARGS " -u \"%2\" -p\"%3\" -N -B -e \"SELECT LOAD_FILE('%4');\" 2>&1")
        .arg(mysqlBin("mysql"), m_login, m_password, file));

    // ── Nettoyage ─────────────────────────────────────────────────────────────
    QFile::remove(file);
    QDir().rmdir(sub);

    // Écriture ET lecture réussies si le jeton est revenu intact.
    return out.contains(token);
}

//  Guide l'utilisateur pour accorder l'« Accès complet au disque » au binaire
//  mysqld, puis attend qu'il demande un ré-essai. Renvoie false si l'utilisateur
//  annule.
bool MySQLInstaller::guideMysqldFullDiskAccess()
{
    const QString mysqld = mysqlBin("mysqld");

    forever {
        const UpSmallButton::StyleBouton rep = UpMessageBox::Question(m_dialog,
            tr("Accès complet au disque requis pour mysqld"),
            tr("mysqld ne parvient pas à écrire dans /Users/Shared. Sur macOS, un "
               "démon doit disposer de l'« Accès complet au disque » pour y accéder.\n\n"
               "Pour l'autoriser :\n"
               "  1. Ouvrez les Réglages Système ▸ Confidentialité ▸ Accès complet au disque.\n"
               "  2. Cliquez sur le bouton « + ».\n"
               "  3. Glissez mysqld (révélé dans le Finder) :\n\n"
               "        %1\n\n"
               "  4. Activez l'interrupteur en face de mysqld.\n"
               "  5. Revenez ici et cliquez sur « Réessayer ».").arg(mysqld),
            UpDialog::ButtonCancel | UpDialog::ButtonOK,
            QStringList() << tr("Annuler") << tr("Réessayer"));

        if (rep != UpSmallButton::STARTBUTTON)
            return false;             // « Annuler »

        // Ouvre les Réglages Système et révèle mysqld dans le Finder, puis
        // réaffiche la boîte pour le ré-essai.
        runCmd("open 'x-apple.systempreferences:com.apple.preference.security"
               "?Privacy_AllFiles' 2>/dev/null");
        runCmd("open -R '" + mysqld + "' 2>/dev/null");
        return true;                  // « Réessayer »
    }
}

//  Lit la valeur d'une clé dans la section [mysqld] de my.cnf (sans connexion).
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
            // MySQL accepte « secure_file_priv » et « secure-file-priv ».
            if (eq > 0) {
                const QString k = line.left(eq).trimmed().replace('-', '_');
                if (k == QString(key).replace('-', '_'))
                    value = line.mid(eq + 1).trimmed();
            }
        }
    }
    f.close();
    // Retirer d'éventuels guillemets (fréquents sous Windows) et un slash final.
    if (value.size() >= 2 && value.startsWith('"') && value.endsWith('"'))
        value = value.mid(1, value.size() - 2);
    if (value.endsWith('/')) value.chop(1);
    return value;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Identifiants
// ─────────────────────────────────────────────────────────────────────────────
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

//  Comme tryConnect() mais avec des identifiants arbitraires (compte admin saisi).
bool MySQLInstaller::tryConnectAs(const QString& login, const QString& mdp)
{
    const QString out = runCmdFull(
        QString("\"%1\" " LOCAL_TCP_ARGS " -u \"%2\" -p\"%3\" ping 2>&1")
            .arg(mysqlBin("mysqladmin"), login, mdp));
    return out.contains("mysqld is alive");
}

//  true ssi adminrufus se connecte avec motDePasseSQL() ET la base Rufus
//  (DB_RUFUS) existe avec au moins une table → base Rufus complète.
bool MySQLInstaller::baseRufusComplete()
{
    const QString mdp = motDePasseSQL();

    // adminrufus se connecte-t-il ?
    const QString ping = runCmdFull(
        QString("\"%1\" " LOCAL_TCP_ARGS " -u \"%2\" -p\"%3\" ping 2>&1")
            .arg(mysqlBin("mysqladmin"), QString(LOGIN_SQL), mdp));
    if (!ping.contains("mysqld is alive"))
        return false;

    // La base DB_RUFUS existe-t-elle ?
    const QString dbs = runCmdFull(
        QString("\"%1\" " LOCAL_TCP_ARGS " -u \"%2\" -p\"%3\" -N -B -e "
                "\"SHOW DATABASES LIKE '%4';\" 2>&1")
            .arg(mysqlBin("mysql"), QString(LOGIN_SQL), mdp, QString(DB_RUFUS)));
    bool dbFound = false;
    for (const QString& line : dbs.split('\n', Qt::SkipEmptyParts))
        if (line.trimmed() == QString(DB_RUFUS)) { dbFound = true; break; }
    if (!dbFound)
        return false;

    // …et contient-elle au moins une table ?
    const QString tables = runCmdFull(
        QString("\"%1\" " LOCAL_TCP_ARGS " -u \"%2\" -p\"%3\" -N -B -e "
                "\"SHOW TABLES FROM %4;\" 2>&1")
            .arg(mysqlBin("mysql"), QString(LOGIN_SQL), mdp, QString(DB_RUFUS)));
    for (const QString& line : tables.split('\n', Qt::SkipEmptyParts)) {
        const QString t = line.trimmed();
        if (!t.isEmpty() && !t.startsWith("ERROR", Qt::CaseInsensitive))
            return true;
    }
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

    QString raw = runCmdFull(
        QString("\"%1\" " LOCAL_TCP_ARGS " -u \"%2\" -p\"%3\" -N -B -e "
                "\"SHOW GRANTS FOR '%2'@'%';\" 2>&1")
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
    // Déjà créé (ex. par prepareCreateModeLinux) ? On court-circuite pour ne pas
    // redemander le mot de passe système.
    if (tryConnect())
        return true;
#endif

    // Crée DEUX comptes avec le MÊME mot de passe aléatoire, en @'%' (réseau) :
    //   • adminrufus       → ALL PRIVILEGES … WITH GRANT OPTION ;
    //   • adminrufusSSL    → idem + REQUIRE SSL.
    const QString sslLogin = QString(LOGIN_SQL "SSL");
    // CREATE USER IF NOT EXISTS ne met PAS à jour le mot de passe d'un compte
    // déjà présent : on ajoute donc un ALTER USER … IDENTIFIED BY … pour IMPOSER
    // le mot de passe aléatoire (celui stocké dans ~/.rufus/.dbkey), que le
    // compte existe déjà ou non — sinon adminrufus garderait un ancien mot de
    // passe et la connexion ultérieure échouerait.
    const QString sql = QString(
        "CREATE USER IF NOT EXISTS '%1'@'%' IDENTIFIED BY '%2';"
        "ALTER USER '%1'@'%' IDENTIFIED BY '%2';"
        "GRANT ALL PRIVILEGES ON *.* TO '%1'@'%' WITH GRANT OPTION;"
        "CREATE USER IF NOT EXISTS '%3'@'%' IDENTIFIED BY '%2' REQUIRE SSL;"
        "ALTER USER '%3'@'%' IDENTIFIED BY '%2' REQUIRE SSL;"
        "GRANT ALL PRIVILEGES ON *.* TO '%3'@'%' WITH GRANT OPTION;"
        "FLUSH PRIVILEGES;\n").arg(m_login, m_password, sslLogin);

#if defined(Q_OS_LINUX)
    // Sur Ubuntu, root@localhost utilise auth_socket : « mysql -u root » ne
    // fonctionne QUE lancé en tant que root. On exécute donc mysql via pkexec, et
    // on transmet le SQL (qui contient le mot de passe) par l'ENTRÉE STANDARD.
    QProcess p;
    p.setProcessChannelMode(QProcess::MergedChannels);
    p.start("pkexec", QStringList{ mysqlBin("mysql"), "-u", "root" });
    if (!p.waitForStarted(60000))
        return false;
    p.write(sql.toUtf8());
    p.closeWriteChannel();
    waitProcessResponsive(p, 60000);
    const QString out = QString::fromLocal8Bit(p.readAll());
    return !out.contains("ERROR", Qt::CaseInsensitive)
        && !out.contains("not authorized", Qt::CaseInsensitive)
        && !out.contains("dismissed",      Qt::CaseInsensitive);
#else
    const QString out = runCmdFull(
        QString("\"%1\" -u root -e \"%2\" 2>&1").arg(mysqlBin("mysql"), sql));
    return !out.contains("ERROR", Qt::CaseInsensitive);
#endif
}

//  Crée adminrufus/adminrufusSSL @'%' (avec m_password) en se connectant au serveur
//  via le compte ADMINISTRATEUR MySQL fourni (login/mdp). Sur les TROIS plateformes
//  on passe directement par « mysql -u <admin> -p<mdp> » (pas de root/pkexec ici).
//  Distingue l'absence du droit CREATE USER (NoCreateUserRight) des autres erreurs.
MySQLInstaller::CreateUserResult
MySQLInstaller::createUserAvecAdmin(const QString& adminLogin, const QString& adminMdp)
{
    const QString sslLogin = QString(LOGIN_SQL "SSL");
    // ALTER USER après CREATE USER IF NOT EXISTS : impose le mot de passe même si
    // le compte adminrufus / adminrufusSSL existait déjà (sinon il garderait son
    // ancien mot de passe et la connexion via le mot de passe stocké échouerait).
    const QString sql = QString(
        "CREATE USER IF NOT EXISTS '%1'@'%' IDENTIFIED BY '%2';"
        "ALTER USER '%1'@'%' IDENTIFIED BY '%2';"
        "GRANT ALL PRIVILEGES ON *.* TO '%1'@'%' WITH GRANT OPTION;"
        "CREATE USER IF NOT EXISTS '%3'@'%' IDENTIFIED BY '%2' REQUIRE SSL;"
        "ALTER USER '%3'@'%' IDENTIFIED BY '%2' REQUIRE SSL;"
        "GRANT ALL PRIVILEGES ON *.* TO '%3'@'%' WITH GRANT OPTION;"
        "FLUSH PRIVILEGES;").arg(m_login, m_password, sslLogin);

    const QString out = runCmdFull(
        QString("\"%1\" " LOCAL_TCP_ARGS " -u \"%2\" -p\"%3\" -e \"%4\" 2>&1")
            .arg(mysqlBin("mysql"), adminLogin, adminMdp, sql));

    if (!out.contains("ERROR", Qt::CaseInsensitive))
        return CreateUserResult::Ok;

    // Manque du droit de créer des utilisateurs ? MySQL renvoie typiquement :
    //   ERROR 1227 (42000): Access denied; you need (at least one of) the CREATE
    //                       USER privilege(s) for this operation
    //   ERROR 1044/1142 : accès refusé sur l'objet / privilège manquant.
    const QString up = out.toUpper();
    const bool accessDeniedCreateUser =
        up.contains("ACCESS DENIED") && up.contains("CREATE USER");
    if (accessDeniedCreateUser
        || up.contains("ERROR 1227")
        || up.contains("ERROR 1044")
        || up.contains("ERROR 1142"))
        return CreateUserResult::NoCreateUserRight;

    return CreateUserResult::Error;
}

#if defined(Q_OS_LINUX)
//  Mode Create (Linux) : exécute TOUT le paramétrage root en UNE SEULE élévation
//  pkexec, pour ne demander le mot de passe système qu'une fois.
bool MySQLInstaller::prepareCreateModeLinux()
{
    const QString path = sharedFolderPath();
    const QString user = runCmd("id -un 2>/dev/null").trimmed();
    QDir().mkpath(path);

    // my.cnf préparé hors élévation (mêmes variables que ensureSecureFilePriv).
    const QString cnfTmp = writeCnfToTemp(rufusCnfVars());
    const QString cnfDst = getCnfPath();
    if (cnfTmp.isEmpty())
        return false;

    // 1. SQL de création des DEUX utilisateurs (adminrufus + adminrufusSSL),
    //    même mot de passe = $PW (placeholder printf %s rempli par "$PW"), en @'%'.
    const QString sslLogin = QString(LOGIN_SQL "SSL");
    // ALTER USER après CREATE USER IF NOT EXISTS : impose le mot de passe même si
    // le compte existe déjà (CREATE USER IF NOT EXISTS ne le réécrit pas). Quatre
    // placeholders printf %s → quatre "$PW".
    const QString userSql = QString(
        "printf \"CREATE USER IF NOT EXISTS '%1'@'%%' IDENTIFIED BY '%s'; "
        "ALTER USER '%1'@'%%' IDENTIFIED BY '%s'; "
        "GRANT ALL PRIVILEGES ON *.* TO '%1'@'%%' WITH GRANT OPTION; "
        "CREATE USER IF NOT EXISTS '%2'@'%%' IDENTIFIED BY '%s' REQUIRE SSL; "
        "ALTER USER '%2'@'%%' IDENTIFIED BY '%s' REQUIRE SSL; "
        "GRANT ALL PRIVILEGES ON *.* TO '%2'@'%%' WITH GRANT OPTION; "
        "FLUSH PRIVILEGES;\\n\" \"$PW\" \"$PW\" \"$PW\" \"$PW\" | mysql -u root; ")
        .arg(m_login, sslLogin);

    // 3. Copie de my.cnf en place + redémarrage du serveur.
    const QString cnfFragment = QString(
        "cp '%1' '%2' && chmod 644 '%2' && systemctl restart mysql; ")
        .arg(cnfTmp, cnfDst);

    const QString script =
        "IFS= read -r PW\n"
        // Journal de support /tmp/rufus_prepare.log : conserve la sortie de
        // l'install. Le mot de passe n'y apparaît JAMAIS.
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

    // Indicateur d'activité pendant l'opération.
    MySQLProgressDialog* dlg = new MySQLProgressDialog(
        tr("Préparation du serveur…\n"
           "Cela peut durer plusieurs minutes."));
    dlg->show();
    QApplication::processEvents();

    const bool ok = runCmdElevated(script, m_password + "\n");
    QFile::remove(cnfTmp);
    waitForMySQL(20);                 // le serveur redémarre en fin de script

    dlg->close();
    delete dlg;
    return ok;
}

//  Fragment shell (root) du paramétrage dossier + Samba (mot de passe Samba dans $PW).
QString MySQLInstaller::linuxFolderSambaScript(const QString& path,
                                               const QString& user) const
{
    return QString(
        // Arborescence Rufus + droits : chmod -R 755, chown -R sur tout /Users.
        "mkdir -p '%1/Rufus/Imagerie'; chmod -R 755 '%1'; chown -R %2 /Users; "
        // — AppArmor : DÉSACTIVER le profil mysqld (sinon lecture des images bloquée). —
        "mkdir -p /etc/apparmor.d/disable; "
        "if [ -f /etc/apparmor.d/usr.sbin.mysqld ]; then "
          "ln -sf /etc/apparmor.d/usr.sbin.mysqld /etc/apparmor.d/disable/; "
          "apparmor_parser -R /etc/apparmor.d/usr.sbin.mysqld 2>/dev/null || true; "
        "fi; "
        // — pare-feu : ouvrir le port MySQL —
        "ufw allow 3306 || true; "
        // — Samba + wsdd : installés ENSEMBLE si l'un manque. —
        "if ! dpkg -s samba >/dev/null 2>&1 || ! dpkg -s wsdd >/dev/null 2>&1; then "
          "apt-get update </dev/null; "
          "DEBIAN_FRONTEND=noninteractive apt-get install -y samba wsdd </dev/null; "
        "fi; "
        // — protocole NT1/SMB1 (appareils de mesure anciens) —
        "grep -q 'server min protocol' /etc/samba/smb.conf 2>/dev/null || "
          "sed -i '/^\\[global\\]/a server min protocol = NT1' /etc/samba/smb.conf; "
        // — partage [Rufus] -> /Users/Shared/Rufus —
        "grep -q '^\\[Rufus\\]' /etc/samba/smb.conf 2>/dev/null || "
          "printf '\\n[Rufus]\\n   comment = Rufus\\n   path = %1/Rufus\\n"
          "   browseable = yes\\n   read only = no\\n   guest ok = yes\\n"
          "   create mask = 0755\\n   directory mask = 0755\\n' >> /etc/samba/smb.conf; "
        // — utilisateur Samba = compte Ubuntu ; mot de passe = $PW. —
        "printf '%s\\n%s\\n' \"$PW\" \"$PW\" | smbpasswd -s -a '%2' >/dev/null 2>&1 || true; "
        "systemctl restart smbd 2>/dev/null || service smbd restart 2>/dev/null || true; "
        // — wsdd : activer le service —
        "systemctl enable --now wsdd 2>/dev/null || true"
        ).arg(path, user);
}
#endif

#if defined(Q_OS_MACOS)
//  Mode Create (macOS) : exécute TOUT le paramétrage root en UNE SEULE invite admin.
bool MySQLInstaller::prepareCreateModeMacOS()
{
    const QString path = sharedFolderPath();            // /Users/Shared
    QDir().mkpath(path + "/Rufus/Imagerie");

    // my.cnf préparé hors élévation (mêmes variables que ensureSecureFilePriv).
    const QString cnfTmp = writeCnfToTemp(rufusCnfVars());
    const QString cnfDst = getCnfPath();                 // /etc/my.cnf (Oracle)
    if (cnfTmp.isEmpty())
        return false;

    // Chemin du binaire mysql → fichier temporaire copié ensuite dans /etc/paths.d.
    QString mysqlPath = mysqlBin("mysql");
    if (!QDir::isAbsolutePath(mysqlPath))
        mysqlPath = runCmd("command -v mysql " + NUL()).trimmed();
    const QString binDir  = QFileInfo(mysqlPath).absolutePath();
    const QString pathTmp = QDir::tempPath() + "/mysql.path";
    { QFile f(pathTmp);
      if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
          QTextStream ts(&f); ts << binDir << '\n';
      } }

    // (Re)démarrage du serveur : démon launchd si présent, sinon mysql.server.
    const QString plist = "/Library/LaunchDaemons/com.oracle.oss.mysql.mysqld.plist";
    const QString restart = QFile::exists(plist)
        ? QString("launchctl unload '%1' 2>/dev/null; launchctl load -w '%1'").arg(plist)
        : QString("TMPDIR=/tmp '%1/support-files/mysql.server' restart").arg(oraclePrefix());

    const QString script = QString(
        "cp '%1' '%2' && chmod 644 '%2'\n"                                   // my.cnf
        "cp '%3' /etc/paths.d/mysql && chmod 644 /etc/paths.d/mysql\n"       // PATH
        "mkdir -p '%4/Rufus/Imagerie'; chmod -R 755 '%4/Rufus'\n"            // dossier
        // Partage SMB de /Users/Shared (lecture/écriture invité), pour Windows.
        "launchctl enable system/com.apple.smbd 2>/dev/null; "
        "launchctl kickstart -k system/com.apple.smbd 2>/dev/null; "
        "sharing -a '%4' -n 'Partagé' -s 001 -g 000 2>/dev/null; "
        "launchctl kickstart -k system/com.apple.smbd 2>/dev/null\n"
        "%5\n")                                                             // restart
        .arg(cnfTmp, cnfDst, pathTmp, path, restart);

    MySQLProgressDialog* dlg = new MySQLProgressDialog(
        tr("Préparation du serveur…\n"
           "Cela peut durer plusieurs minutes."));
    dlg->show();
    QApplication::processEvents();

    const bool ok = runCmdElevated(script);
    QFile::remove(cnfTmp);
    QFile::remove(pathTmp);
    waitForMySQL(20);                 // le serveur vient d'être redémarré

    dlg->close();
    delete dlg;
    return ok;
}
#endif

// ─────────────────────────────────────────────────────────────────────────────
//  Dossier partagé : existe et est partagé
// ─────────────────────────────────────────────────────────────────────────────
bool MySQLInstaller::setupSharedFolder()
{
    const QString path = sharedFolderPath();
    if (!QDir(path).exists())
        QDir().mkpath(path);

#if defined(Q_OS_WIN)
    // C:\Users\Public existe par défaut et est accessible en lecture/écriture à
    // tous les comptes Windows : aucun partage supplémentaire à configurer.
    return QDir(path).exists();
#elif defined(Q_OS_LINUX)
    // Déjà configuré ? Vérifications NON privilégiées (aucune invite pkexec).
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

    // Sinon, tout le paramétrage privilégié en une seule élévation (pkexec).
    const QString user = runCmd("id -un 2>/dev/null").trimmed();
    runCmdElevated("IFS= read -r PW\n" + linuxFolderSambaScript(path, user),
                   m_password + "\n");
    return QDir(path).exists();
#else
    // Déjà partagé ? (lecture seule, aucune élévation)
    if (runCmd("sharing -l 2>/dev/null").contains(path))
        return true;

    // Activer SMB et déclarer le partage — opérations privilégiées (une invite macOS).
    runCmdElevated(
        "launchctl enable system/com.apple.smbd; "
        "launchctl kickstart -k system/com.apple.smbd; "
        "sharing -a '/Users/Shared' -n 'Partagé' -s 001 -g 000; "
        "launchctl kickstart -k system/com.apple.smbd");

    return runCmd("sharing -l 2>/dev/null").contains(path);
#endif
}

// ─────────────────────────────────────────────────────────────────────────────
//  Variables MySQL
// ─────────────────────────────────────────────────────────────────────────────
QString MySQLInstaller::writeCnfToTemp(const QList<QPair<QString, QString>>& vars)
{
    const QString path = getCnfPath();
    QFile   f(path);
    QStringList lines;
    bool inMysqld = false;
    QStringList remaining;                 // clés pas encore rencontrées
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

    // Clés non présentes : insérées juste après [mysqld] (créé si absent).
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
    // Ubuntu (apt) : la section [mysqld] vit dans mysql.conf.d/mysqld.cnf.
    for (const QString& p : {QString("/etc/mysql/mysql.conf.d/mysqld.cnf"),
                             QString("/etc/mysql/my.cnf")})
        if (QFile::exists(p)) return p;
    return "/etc/mysql/mysql.conf.d/mysqld.cnf";
#else
    QString prefix = getBrewPrefix();
    if (!prefix.isEmpty()) return prefix + "/etc/my.cnf";
    for (auto& p : {QString("/etc/my.cnf"),
                    QString("/etc/mysql/my.cnf"),
                    QString("/usr/local/mysql/etc/my.cnf"),
                    QString("/usr/local/etc/my.cnf")})
        if (QFile::exists(p)) return p;
    if (isOracleInstall()) return "/etc/my.cnf";
    return "/opt/homebrew/etc/my.cnf";
#endif
}

// ─────────────────────────────────────────────────────────────────────────────
//  Helpers
// ─────────────────────────────────────────────────────────────────────────────
//  Question Oui/Non via UpMessageBox::Question (boutons « Non »/« Oui »).
//  Le bouton OK (STARTBUTTON) correspond à « Oui ».
bool MySQLInstaller::askYesNo(const QString& title, const QString& text)
{
    const UpSmallButton::StyleBouton rep = UpMessageBox::Question(
        m_dialog, title, text,
        UpDialog::ButtonCancel | UpDialog::ButtonOK,
        QStringList() << tr("Non") << tr("Oui"));
    return rep == UpSmallButton::STARTBUTTON;
}

//  Dialogue de mise à jour nécessaire (mode Verify, version trop ancienne).
bool MySQLInstaller::askUpdateConfirmation(const QString& currentVer,
                                           const QString& targetVer)
{
    const QString text = tr(
        "MySQL %1 est installé, mais la version %2 (ou ultérieure) est "
        "nécessaire.\n\n"
        "La mise à jour effectue un NETTOYAGE COMPLET : MySQL est entièrement "
        "désinstallé puis réinstallé proprement. La base de données existante "
        "sera DÉFINITIVEMENT SUPPRIMÉE.\n\n"
        "Sauvegardez vos données AVANT de poursuivre. Ne confirmez que si elles "
        "sont déjà sauvegardées.")
        .arg(currentVer.isEmpty() ? tr("(version inconnue)") : currentVer,
             targetVer);

    const UpSmallButton::StyleBouton rep = UpMessageBox::Question(
        m_dialog, tr("Mise à jour de MySQL nécessaire"), text,
        UpDialog::ButtonCancel | UpDialog::ButtonOK,
        QStringList() << tr("Annuler")
                      << tr("OK, faire la MAJ, mes données sont sauvegardées"));
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

bool MySQLInstaller::runCmdElevated(const QString& cmd, const QString& stdinData)
{
#if defined(Q_OS_WIN)
    // L'application Windows tourne déjà en tant qu'administrateur.
    Q_UNUSED(stdinData);
    const QString out = runCmdFull(cmd);
    return !out.contains("Access is denied", Qt::CaseInsensitive)
        && !out.contains("denied",           Qt::CaseInsensitive);
#elif defined(Q_OS_LINUX)
    // Élévation via pkexec (invite graphique PolicyKit). La commande est déposée
    // dans un script temporaire pour éviter les soucis de guillemets.
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
        // stdinData transmis à l'entrée standard du processus élevé.
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
    // Exécute « cmd » avec les droits administrateur via une seule invite macOS.
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

    // Timeout LARGE (15 min) : l'invite de mot de passe administrateur est
    // interactive. Avec le défaut de runCmdFull (30 s), si l'invite tardait à
    // s'afficher ou que la saisie du mot de passe + l'opération dépassaient 30 s,
    // waitProcessResponsive tuait osascript EN PLEIN MILIEU : la sortie tronquée ne
    // contenant ni « User canceled » ni « execution error », on renvoyait « succès »
    // à tort alors que le script (ex. désinstallation) n'avait pas fini → « MySQL
    // toujours présent » malgré le message de confirmation (bug uninstall fantôme).
    const QString out = runCmdFull(QString(
        "osascript -e 'do shell script \"%1\" with administrator privileges' 2>&1")
        .arg(scriptPath), 900000);
    QFile::remove(scriptPath);

    // Succès = l'utilisateur n'a pas annulé l'invite et osascript n'a pas échoué.
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

//  Variante de runLongOp() avec barre de progression réelle : la commande doit
//  émettre sur sa sortie standard des lignes « PROGRESS <fait> <total> ».
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
                    dlg.setProgress(p.at(1).toLongLong(), p.at(2).toLongLong());
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

//  Téléchargement HTTP(S) robuste : QNetworkAccessManager (progression réelle),
//  repli automatique sur curl en cas d'échec.
bool MySQLInstaller::downloadFile(const QString& url, const QString& dest,
                                  const QString& label)
{
    // ── Tentative 1 : QNetworkAccessManager (progression réelle) ──────────────
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
            // Forcer HTTP/1.1 : le HTTP/2 de Qt fait parfois échouer un gros
            // téléchargement en cours de route avec le CDN MySQL.
            req.setAttribute(QNetworkRequest::Http2AllowedAttribute, false);
            req.setHeader(QNetworkRequest::UserAgentHeader, "MySQLInstaller/1.0");
            QNetworkReply* reply = nam.get(req);

            QEventLoop loop;
            QObject::connect(reply, &QNetworkReply::readyRead, [&]{
                file.write(reply->readAll());
            });
            QObject::connect(reply, &QNetworkReply::downloadProgress,
                             [&](qint64 r, qint64 t){
                dlg.setProgress(r, t);
                QApplication::processEvents();
            });
            QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
            loop.exec();

            file.write(reply->readAll());      // reliquat éventuel
            file.close();
            const bool ok = (reply->error() == QNetworkReply::NoError);
            reply->deleteLater();
            if (ok && QFileInfo(dest).size() > 0)
                return true;
        }
    }

    // ── Tentative 2 : repli sur curl (TLS du système) ─────────────────────────
    QFile::remove(dest);
    runLongOp(QString("curl -fSL -o \"%1\" \"%2\"")
              .arg(QDir::toNativeSeparators(dest), url), label, 1800000);
    return QFile::exists(dest) && QFileInfo(dest).size() > 0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Accès réseau (WAN) : connexion TCP/443 vers une IP publique fiable, SANS DNS.
// ─────────────────────────────────────────────────────────────────────────────
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

// ─────────────────────────────────────────────────────────────────────────────
//  Pré-requis réseau juste avant un téléchargement d'installation MySQL.
// ─────────────────────────────────────────────────────────────────────────────
bool MySQLInstaller::checkDownloadConnectivity(const QString& downloadUrl)
{
    // 1. Accès WAN.
    if (!hasNetworkAccess()) {
        UpMessageBox::Watch(nullptr, tr("Pas d'accès réseau"),
            tr("Absence d'accès réseau. Le programme ne peut pas télécharger "
               "le fichier d'installation de MySQL.\n\nFermeture du programme."));
        return false;
    }

    // 2. Résolution DNS de l'hôte du lien de téléchargement.
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

QString MySQLInstaller::getBrewPrefix()
{
#if defined(Q_OS_WIN) || defined(Q_OS_LINUX)
    return {};   // pas de Homebrew sous Windows/Linux
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
    return binary;   // supposé présent dans le PATH
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
