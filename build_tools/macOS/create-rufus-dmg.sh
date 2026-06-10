#!/bin/bash
# ─────────────────────────────────────────────────────────────────────────────
#  Crée un .dmg d'installation de Rufus pour macOS à partir du bundle .app
#  fraîchement compilé par Qt. Équivalent macOS du setup.exe (Windows) et de
#  l'AppImage (Linux).
#
#  Usage :  ./create-rufus-dmg.sh  <chemin/vers/Rufus.app>  [version]  [dossier_sortie]
#
#  ⚠️ BINAIRE UNIVERSEL (x86_64 + arm64) — on NE fait PAS l'impasse sur les Macs
#  Intel. Compiler en amont avec les deux architectures :
#     qmake RufusQt6.pro CONFIG+=release "QMAKE_APPLE_DEVICE_ARCHS=x86_64 arm64"
#     make -j$(sysctl -n hw.ncpu)
#  (Qt macOS officiel est universel ; la tranche absente est cross-compilée.)
#  Vérifier après coup :  lipo -archs Rufus.app/Contents/MacOS/Rufus  →  « x86_64 arm64 ».
#
#  Étapes du script :
#   1. macdeployqt : embarque les frameworks Qt + plugins (dont le pilote SQL
#      MySQL) dans le bundle, et le rend autonome ;
#   2. signature (optionnelle, cf. CODESIGN_ID ci-dessous) ;
#   3. hdiutil : fabrique un .dmg compressé contenant l'app + un lien vers
#      /Applications (l'utilisateur n'a qu'à glisser-déposer pour installer).
#
#  SIGNATURE — les « 2 lignes » : exporter CODESIGN_ID avec ton identité
#  « Developer ID Application » avant de lancer le script, p. ex. :
#     export CODESIGN_ID="Developer ID Application: Prénom Nom (TEAMID)"
#  Sans CODESIGN_ID, le .dmg n'est pas signé : Gatekeeper affichera un avertissement
#  à la 1re ouverture (clic droit → Ouvrir). La notarisation (xcrun notarytool) peut
#  être ajoutée ensuite si besoin d'une ouverture sans aucun avertissement.
# ─────────────────────────────────────────────────────────────────────────────
set -euo pipefail

APP_PATH="${1:?Usage: create-rufus-dmg.sh <Rufus.app> [version] [outdir]}"
VERSION="${2:-dev}"
OUTDIR="${3:-$(pwd)}"

APP_NAME="$(basename "${APP_PATH}" .app)"
DMG_NAME="Rufus-${VERSION}.dmg"
DMG_PATH="${OUTDIR}/${DMG_NAME}"

# macdeployqt doit être dans le PATH (fourni par l'installation Qt).
echo "==> macdeployqt sur ${APP_PATH}"
macdeployqt "${APP_PATH}" -verbose=1

# Signature (optionnelle) — ligne 1/2 : signer l'app universelle en profondeur.
if [ -n "${CODESIGN_ID:-}" ]; then
    echo "==> codesign (${CODESIGN_ID})"
    codesign --deep --force --options runtime \
        --sign "${CODESIGN_ID}" "${APP_PATH}"
fi

# Dossier de mise en scène : l'app + un alias vers /Applications.
STAGE="$(mktemp -d)"
cp -R "${APP_PATH}" "${STAGE}/"
ln -s /Applications "${STAGE}/Applications"

echo "==> Fabrication du dmg ${DMG_PATH}"
rm -f "${DMG_PATH}"
hdiutil create \
    -volname "Rufus ${VERSION}" \
    -srcfolder "${STAGE}" \
    -ov -format UDZO \
    "${DMG_PATH}"

# Signature (optionnelle) — ligne 2/2 : signer le .dmg lui-même.
if [ -n "${CODESIGN_ID:-}" ]; then
    codesign --force --sign "${CODESIGN_ID}" "${DMG_PATH}"
fi

rm -rf "${STAGE}"
echo "==> OK : ${DMG_PATH}"
