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
#  SIGNATURE (optionnelle) :
#   • export CODESIGN_ID="Developer ID Application: …"  → signe l'app ;
#   • export CODESIGN_INSTALLER_ID="Developer ID Installer: …"  → signe le .pkg.
#  Sans elles, le .pkg n'est pas signé (Gatekeeper averti au 1er lancement).
# ─────────────────────────────────────────────────────────────────────────────
set -euo pipefail

APP_PATH="${1:?Usage: create-rufus-pkg.sh <Rufus.app> [version] [outdir]}"
VERSION="${2:-dev}"
OUTDIR="${3:-$(pwd)}"
HERE="$(cd "$(dirname "$0")" && pwd)"

echo "==> macdeployqt sur ${APP_PATH}"
macdeployqt "${APP_PATH}" -verbose=1

# Signature de l'app (en profondeur) si demandée.
if [ -n "${CODESIGN_ID:-}" ]; then
    echo "==> codesign app (${CODESIGN_ID})"
    codesign --deep --force --options runtime --sign "${CODESIGN_ID}" "${APP_PATH}"
fi

# Mise en scène : un dossier racine ne contenant que l'app (→ /Applications).
STAGE="$(mktemp -d)/root"
SCRIPTS="$(mktemp -d)"
mkdir -p "${STAGE}"
cp -R "${APP_PATH}" "${STAGE}/"
cp "${HERE}/preinstall" "${SCRIPTS}/preinstall"
chmod +x "${SCRIPTS}/preinstall"

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
