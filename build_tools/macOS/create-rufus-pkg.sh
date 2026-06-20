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

# ── Pilote SQL MySQL : réorienter sa dépendance client AVANT macdeployqt ───────
# macdeployqt embarque les autres pilotes SQL mais LÂCHE libqsqlmysql.dylib s'il ne peut pas
# résoudre sa dépendance vers la lib client (libmariadb/libmysqlclient). On réoriente cette
# dépendance, si elle pointe un chemin inexistant, vers la lib installée par brew → macdeployqt
# pourra alors embarquer le pilote ET sa lib client, et réécrire les chemins tout seul.
QT_PLUGINS="$(qmake -query QT_INSTALL_PLUGINS 2>/dev/null)"
QSQLMYSQL="${QT_PLUGINS}/sqldrivers/libqsqlmysql.dylib"
if [ -n "${QT_PLUGINS}" ] && [ -f "${QSQLMYSQL}" ]; then
    echo "==> Pilote MySQL : vérification de la dépendance client (${QSQLMYSQL})"
    otool -L "${QSQLMYSQL}" 2>/dev/null | awk '/libmariadb|libmysqlclient/ {print $1}' | while read -r dep; do
        [ -f "${dep}" ] && continue   # dépendance déjà résolvable : rien à faire
        base="$(basename "${dep}")"
        real="$(find /opt/homebrew /usr/local -name "${base}" 2>/dev/null | head -n1)"
        if [ -n "${real}" ]; then
            echo "   réoriente ${dep} -> ${real}"
            install_name_tool -change "${dep}" "${real}" "${QSQLMYSQL}" 2>/dev/null || true
        fi
    done
else
    echo "   ⚠ libqsqlmysql.dylib introuvable dans les plugins Qt (${QT_PLUGINS}/sqldrivers)"
fi

echo "==> macdeployqt sur ${APP_PATH}"
macdeployqt "${APP_PATH}" -verbose=1

# Contrôle : le pilote MySQL est-il bien dans le bundle ? (sinon backup/connexion KO)
if [ ! -f "${APP_PATH}/Contents/PlugIns/sqldrivers/libqsqlmysql.dylib" ]; then
    echo "   ⚠ libqsqlmysql.dylib ABSENT du bundle après macdeployqt — tentative de copie directe"
    mkdir -p "${APP_PATH}/Contents/PlugIns/sqldrivers"
    cp -f "${QSQLMYSQL}" "${APP_PATH}/Contents/PlugIns/sqldrivers/" 2>/dev/null || true
fi

# ── Nom du bundle = « Rufus.app » → installé directement dans /Applications ────
# Le .pro ne fixe pas de nom propre sur macOS (les scopes OSX/LINUX ne sont pas des scopes
# qmake valides → bundle « RufusQt6.app » par défaut). On le renomme ici, AVANT la signature,
# pour que l'utilisateur obtienne /Applications/Rufus.app (et non un nom interne de build).
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
pkgbuild \
    --root "${STAGE}" \
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
