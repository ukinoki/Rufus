#!/bin/bash
# ─────────────────────────────────────────────────────────────────────────────
#  Crée un .dmg d'installation de Rufus pour macOS à partir du bundle .app
#  fraîchement compilé par Qt. Équivalent macOS du setup.exe (Windows) et de
#  l'AppImage (Linux).
#
#  Usage :  ./create-rufus-dmg.sh  <chemin/vers/Rufus.app>  [version]  [dossier_sortie]
#
#  Étapes :
#   1. macdeployqt : embarque les frameworks Qt + plugins (dont le pilote SQL
#      MySQL) dans le bundle, et le rend autonome ;
#   2. hdiutil : fabrique un .dmg compressé contenant l'app + un lien vers
#      /Applications (l'utilisateur n'a qu'à glisser-déposer pour installer).
#
#  Le .dmg n'est PAS signé : à la première ouverture, Gatekeeper affichera un
#  avertissement que l'utilisateur contourne (clic droit → Ouvrir). La signature
#  / notarisation Apple pourra être ajoutée ultérieurement.
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

rm -rf "${STAGE}"
echo "==> OK : ${DMG_PATH}"
