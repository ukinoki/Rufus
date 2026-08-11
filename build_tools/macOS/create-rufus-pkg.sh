#!/bin/bash
# ─────────────────────────────────────────────────────────────────────────────
#  Crée un .pkg d'installation de Rufus pour macOS à partir du bundle .app.
#  Le .pkg (et non plus un .dmg glisser-déposer) permet un script « preinstall »
#  qui CONTRÔLE le socle MySQL AVANT toute copie → on ne remplace pas l'ancien
#  Rufus si MySQL est trop ancien (cf. build_tools/macOS/preinstall).
#
#  Usage :  ./create-rufus-pkg.sh  <chemin/vers/Rufus.app>  [version]  [dossier_sortie]
#
#  ⚠️ BINAIRE UNIVERSEL (x86_64 + arm64) — compiler en amont avec :
#     qmake RufusQt6.pro CONFIG+=release "QMAKE_APPLE_DEVICE_ARCHS=x86_64 arm64"
#  Vérif :  lipo -archs Rufus.app/Contents/MacOS/Rufus  →  « x86_64 arm64 ».
#
#  SIGNATURE :
#   • Sans Developer ID : l'app est signée AD-HOC (gratuit) — REQUIS pour qu'elle DÉMARRE
#     sur Apple Silicon (macdeployqt invalide la signature de l'éditeur de liens, et arm64
#     exige une signature). Gatekeeper avertit quand même au 1er lancement (dév. non identifié).
#   • export CODESIGN_ID="Developer ID Application: …"         → signe l'app (officiel) ;
#   • export CODESIGN_INSTALLER_ID="Developer ID Installer: …" → signe le .pkg.
# ─────────────────────────────────────────────────────────────────────────────
set -euo pipefail

APP_PATH="${1:?Usage: create-rufus-pkg.sh <Rufus.app> [version] [outdir]}"
VERSION="${2:-dev}"
OUTDIR="${3:-$(pwd)}"
HERE="$(cd "$(dirname "$0")" && pwd)"
REPO="$(cd "${HERE}/../.." && pwd)"   # racine du dépôt (pour les .qm de traduction)

# ── Traductions : copier les .qm dans Rufus.app/Contents/Locale ───────────────
# L'appli les cherche dans applicationDirPath()/../Locale = Contents/Locale (cf. rufus.cpp).
# Rien dans le .pro ne les déploie (ils ne sont que dans DISTFILES) → on le fait ici, AVANT
# macdeployqt et la signature, pour qu'ils soient couverts par la signature du bundle.
echo "==> Traductions (.qm) -> ${APP_PATH}/Contents/Locale"
LOCALE_DIR="${APP_PATH}/Contents/Locale"
mkdir -p "${LOCALE_DIR}"
# (au besoin) régénérer les .qm depuis les .ts, sinon prendre ceux du dépôt
command -v lrelease >/dev/null 2>&1 && ( cd "${REPO}" && lrelease RufusQt6.pro >/dev/null 2>&1 ) || true
cp -f "${REPO}"/rufus_*.qm "${LOCALE_DIR}/" 2>/dev/null || true
ls "${LOCALE_DIR}"/rufus_*.qm >/dev/null 2>&1 || echo "   ⚠ aucun .qm copié (traductions manquantes ?)"

# Le pilote MySQL (libqsqlmysql.dylib) est installé dans les plugins Qt par
# build_tools/macOS/build-qmysql-universal.sh, avec une dépendance STABLE vers libmariadb
# (qui embarque OpenSSL en statique → aucune autre dépendance externe). Chemins de référence
# pour le filet de sécurité post-macdeployqt ci-dessous.
QT_PLUGINS="$(qmake -query QT_INSTALL_PLUGINS 2>/dev/null)"
QT_LIBDIR="$(qmake -query QT_INSTALL_LIBS 2>/dev/null)"
QSQLMYSQL="${QT_PLUGINS}/sqldrivers/libqsqlmysql.dylib"
[ -f "${QSQLMYSQL}" ] || echo "   ⚠ libqsqlmysql.dylib introuvable dans Qt (${QT_PLUGINS}/sqldrivers) — pilote non compilé ?"

echo "==> macdeployqt sur ${APP_PATH}"
macdeployqt "${APP_PATH}" -verbose=1

# ── Filet de sécurité : pilote MySQL + libmariadb bien embarqués et reliés ─────
# On ne dépend pas du bon vouloir de macdeployqt : on garantit que (1) le pilote est dans le
# bundle, (2) libmariadb est dans Contents/Frameworks, (3) le pilote pointe vers cette copie.
BUNDLE_DRV="${APP_PATH}/Contents/PlugIns/sqldrivers/libqsqlmysql.dylib"
BUNDLE_FW="${APP_PATH}/Contents/Frameworks"

if [ ! -f "${BUNDLE_DRV}" ]; then
    echo "   pilote absent du bundle → copie directe depuis Qt"
    mkdir -p "$(dirname "${BUNDLE_DRV}")"
    cp -f "${QSQLMYSQL}" "${BUNDLE_DRV}" 2>/dev/null || true
fi

if [ -f "${BUNDLE_DRV}" ]; then
    # Nom réel de la lib client référencée par le pilote (ex. libmariadb.3.dylib).
    MARIADB_DEP="$(otool -L "${BUNDLE_DRV}" 2>/dev/null | awk '/libmariadb|libmysqlclient/ {print $1; exit}')"
    MARIADB_NAME="$(basename "${MARIADB_DEP:-libmariadb.3.dylib}")"
    if [ ! -f "${BUNDLE_FW}/${MARIADB_NAME}" ]; then
        echo "   libmariadb absente de Frameworks → copie depuis ${QT_LIBDIR}"
        mkdir -p "${BUNDLE_FW}"
        cp -f "${QT_LIBDIR}/${MARIADB_NAME}" "${BUNDLE_FW}/" 2>/dev/null || true
    fi
    chmod u+w "${BUNDLE_FW}/${MARIADB_NAME}" 2>/dev/null || true
    install_name_tool -id "@rpath/${MARIADB_NAME}" "${BUNDLE_FW}/${MARIADB_NAME}" 2>/dev/null || true
    # TOUJOURS réécrire la dépendance du pilote vers la copie de Frameworks. La valeur posée par
    # le script de build (@loader_path/../../lib/…) est juste DANS L'INSTALL QT (plugins -> lib),
    # mais FAUSSE dans le bundle (PlugIns/sqldrivers/../../lib = Contents/lib, inexistant). Ici la
    # cible est Contents/Frameworks : PlugIns/sqldrivers/../../Frameworks. (D'où le « driver not
    # loaded » tant qu'on ne corrigeait pas les chemins en @… commençant par @loader_path.)
    if [ -n "${MARIADB_DEP}" ]; then
        install_name_tool -change "${MARIADB_DEP}" "@loader_path/../../Frameworks/${MARIADB_NAME}" "${BUNDLE_DRV}" 2>/dev/null || true
    fi
    echo "   pilote MySQL : $(lipo -archs "${BUNDLE_DRV}" 2>/dev/null) | dépendance -> $(otool -L "${BUNDLE_DRV}" | awk '/libmariadb|libmysqlclient/{print $1; exit}')"
fi

# ── Élagage des pilotes SQL inutiles ──────────────────────────────────────────
# Rufus n'utilise QUE MySQL (QSqlDatabase::addDatabase("QMYSQL"), aucun usage de SQLite ni
# autre). macdeployqt embarque TOUS les pilotes de Qt (ODBC, PostgreSQL, SQLite…) : on ne
# garde que libqsqlmysql.dylib. À faire AVANT la signature, pour qu'elle porte sur le final.
SQLDRV_DIR="${APP_PATH}/Contents/PlugIns/sqldrivers"
if [ -d "${SQLDRV_DIR}" ]; then
    find "${SQLDRV_DIR}" -name 'libqsql*.dylib' ! -name 'libqsqlmysql.dylib' -delete
    echo "==> pilotes SQL conservés : $(ls "${SQLDRV_DIR}" 2>/dev/null | tr '\n' ' ')"
fi

# ── Clients mysql / mysqldump embarqués (Contents/Applications) ───────────────
# Sauvegarde et restauration passent par ces deux binaires ; sur un poste sans serveur MySQL local
# ils n'existent nulle part ailleurs. Emplacement lu par Procedures::setDirSQLExecutable.
CLIENTS_DIR="${APP_PATH}/Contents/Applications"
MYSQL_SRC=""
for d in /usr/local/mysql/bin /opt/homebrew/opt/mysql-client/bin /opt/homebrew/opt/mysql/bin \
         /usr/local/opt/mysql-client/bin /usr/local/opt/mysql/bin; do
    [ -x "${d}/mysql" ] && [ -x "${d}/mysqldump" ] && { MYSQL_SRC="${d}"; break; }
done
[ -n "${MYSQL_SRC}" ] || { echo "   ✗ mysql / mysqldump introuvables : sauvegarde et restauration impossibles"; exit 1; }

mkdir -p "${CLIENTS_DIR}"
cp -f "${MYSQL_SRC}/mysql" "${MYSQL_SRC}/mysqldump" "${CLIENTS_DIR}/"
chmod u+w "${CLIENTS_DIR}/mysql" "${CLIENTS_DIR}/mysqldump"
echo "==> clients SQL embarqués depuis ${MYSQL_SRC}"

# ── Filet de sécurité : nom du bundle = « Rufus.app » → /Applications/Rufus.app ─
# Le .pro fixe désormais TARGET = Rufus (bundle « Rufus.app »). Ce renommage ne sert donc plus
# que de garde-fou si un ancien build fournissait encore « RufusQt6.app ». AVANT la signature.
if [ "$(basename "${APP_PATH}")" != "Rufus.app" ]; then
    NEWAPP="$(dirname "${APP_PATH}")/Rufus.app"
    rm -rf "${NEWAPP}"
    mv "${APP_PATH}" "${NEWAPP}"
    APP_PATH="${NEWAPP}"
    echo "==> bundle renommé : ${APP_PATH}"
fi

# Signature de l'app (en profondeur).
if [ -n "${CODESIGN_ID:-}" ]; then
    # Developer ID (compte Apple payant) : signature "officielle" (prélude à la notarisation).
    echo "==> codesign app (${CODESIGN_ID})"
    codesign --deep --force --options runtime --sign "${CODESIGN_ID}" "${APP_PATH}"
else
    # Pas de Developer ID : signature AD-HOC (gratuite) — INDISPENSABLE sur Apple Silicon.
    # macdeployqt vient de modifier les binaires (rpaths), ce qui INVALIDE la signature ad-hoc
    # posée par l'éditeur de liens ; sans re-signature, macOS arm64 TUE l'app au lancement
    # (« killed »). L'ad-hoc rend l'app LANÇABLE. (Gatekeeper avertit quand même « développeur
    # non identifié » au 1er lancement → clic droit ▸ Ouvrir, ou Réglages ▸ Ouvrir quand même.)
    echo "==> codesign ad-hoc (pas de Developer ID) — requis pour démarrer sur Apple Silicon"
    codesign --deep --force --sign - "${APP_PATH}"
fi

# Mise en scène : un dossier racine ne contenant que l'app (→ /Applications).
STAGE="$(mktemp -d)/root"
SCRIPTS="$(mktemp -d)"
mkdir -p "${STAGE}"
cp -R "${APP_PATH}" "${STAGE}/"
cp "${HERE}/preinstall" "${SCRIPTS}/preinstall"
chmod +x "${SCRIPTS}/preinstall"
cp "${HERE}/postinstall" "${SCRIPTS}/postinstall"
chmod +x "${SCRIPTS}/postinstall"

PKG="${OUTDIR}/Rufus-${VERSION}.pkg"
echo "==> pkgbuild ${PKG}"

# Empêcher la RELOCALISATION du bundle. Par défaut pkgbuild marque une .app « relocatable » :
# l'installeur la pose là où Spotlight trouve déjà une copie de MÊME identifiant
# (org.rufusvision.rufus — p.ex. un build local Qt Creator) au lieu de /Applications, et l'app
# semble « disparaître ». On force BundleIsRelocatable=false via un component plist.
COMPONENT_PLIST="$(mktemp -d)/component.plist"
pkgbuild --analyze --root "${STAGE}" "${COMPONENT_PLIST}"
python3 - "${COMPONENT_PLIST}" <<'PY'
import plistlib, sys
path = sys.argv[1]
with open(path, "rb") as f:
    data = plistlib.load(f)
for item in data:
    if isinstance(item, dict):
        item["BundleIsRelocatable"] = False
with open(path, "wb") as f:
    plistlib.dump(data, f)
PY

pkgbuild \
    --root "${STAGE}" \
    --component-plist "${COMPONENT_PLIST}" \
    --scripts "${SCRIPTS}" \
    --install-location /Applications \
    --identifier org.rufusvision.rufus \
    --version "${VERSION}" \
    "${PKG}"

# Signature du .pkg (certificat « Developer ID Installer ») si demandée.
if [ -n "${CODESIGN_INSTALLER_ID:-}" ]; then
    echo "==> productsign (${CODESIGN_INSTALLER_ID})"
    SIGNED="${OUTDIR}/Rufus-${VERSION}-signed.pkg"
    productsign --sign "${CODESIGN_INSTALLER_ID}" "${PKG}" "${SIGNED}"
    mv -f "${SIGNED}" "${PKG}"
fi

echo "==> OK : ${PKG}"
