#!/usr/bin/env bash
#
#  creer-appimage-installeur.sh
#  ----------------------------
#  Fabrique l'AppImage-installeur de Rufus, en local, comme le fait le workflow
#  GitHub. Reprend ton AppRun (l'installeur, dans ce même dossier) et le fusionne
#  avec Rufus pour produire UNE AppImage qui, lancée sur un poste, s'installe toute
#  seule (libs + menus).
#
#  Usage :
#     ./creer-appimage-installeur.sh                 # compile Rufus puis fabrique
#     ./creer-appimage-installeur.sh /chemin/Rufus   # part d'un binaire déjà compilé
#
#  Prérequis : Qt (qmake dans le PATH) + « sudo apt install libfuse2 ».
#  Mémo complet : LISEZMOI-creer-appimage.md
#
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO="$(cd "${SCRIPT_DIR}/../.." && pwd)"
WORK="${REPO}/build-appimage"          # dossier jetable (recréé à chaque fois)
APPDIR="${WORK}/AppDir"
TOOLS="${HOME}/.cache/rufus-appimage-tools"   # outils téléchargés une fois

# Version = date de sortie, lue dans rufus.cpp : setApplicationVersion("10-06-2026/1")
VER="$(grep -oP 'setApplicationVersion\("\K[^"/]+' "${REPO}/rufus.cpp" | head -n1)"

# Qt : nécessaire pour compiler ET pour embarquer Qt dans l'AppImage. On cherche qmake dans
# l'ordre : 1) variable QMAKE si tu l'as posée ; 2) qmake/qmake6 dans le PATH ; 3) repli sur
# l'install Qt Creator standard ~/Qt/<version>/gcc_64/bin (on prend la version la plus récente).
QMAKE="${QMAKE:-$(command -v qmake6 || command -v qmake || true)}"
if [ -z "${QMAKE}" ]; then
    QMAKE="$(ls -d "${HOME}"/Qt/*/gcc_64/bin/qmake 2>/dev/null | sort -V | tail -n1)"
fi
if [ -z "${QMAKE}" ]; then
    echo "ERREUR : qmake introuvable. Indique Qt, par exemple :" >&2
    echo '   export PATH="$HOME/Qt/6.10.2/gcc_64/bin:$PATH"' >&2
    exit 1
fi
QT_ROOT_DIR="$("${QMAKE}" -query QT_INSTALL_PREFIX)"
echo "Rufus ${VER} | qmake ${QMAKE}"

# ── 1. Le binaire Rufus : fourni en argument, sinon on compile ───────────────
rm -rf "${WORK}"
mkdir -p "${APPDIR}/usr/bin" "${APPDIR}/usr/share/applications" \
         "${APPDIR}/usr/share/icons/hicolor/256x256/apps" "${APPDIR}/usr/Locale"

if [ -n "${1:-}" ]; then
    cp "$1" "${APPDIR}/usr/bin/Rufus"
else
    echo "Compilation (qmake + make)…"
    ( mkdir -p "${WORK}/obj" && cd "${WORK}/obj" && "${QMAKE}" "${REPO}/RufusQt6.pro" CONFIG+=release && make -j"$(nproc)" )
    cp "$(find "${WORK}/obj" -maxdepth 2 -type f -executable -name Rufus | head -n1)" "${APPDIR}/usr/bin/Rufus"
fi

# ── 2. Icône + raccourci .desktop + traductions ──────────────────────────────
cp "${SCRIPT_DIR}/rufus.png" "${APPDIR}/usr/share/icons/hicolor/256x256/apps/rufus.png"
cat > "${APPDIR}/usr/share/applications/rufus.desktop" <<'EOF'
[Desktop Entry]
Type=Application
Name=Rufus
Exec=Rufus
Icon=rufus
Comment=Logiciel de gestion de cabinet d'Ophtalmologie
Categories=Office;
Terminal=false
EOF
cp "${REPO}"/rufus_*.qm "${APPDIR}/usr/Locale/" 2>/dev/null || echo "(pas de traductions rufus_*.qm)"

# ── 3. Les 3 outils AppImage (téléchargés une fois, mis en cache) ────────────
mkdir -p "${TOOLS}"
get() { [ -f "$2" ] || { echo "Téléchargement $(basename "$2")…"; wget -q "$1" -O "$2"; }; chmod +x "$2"; }
get https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage                     "${TOOLS}/linuxdeploy.AppImage"
get https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage "${TOOLS}/linuxdeploy-plugin-qt-x86_64.AppImage"
get https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-x86_64.AppImage                       "${TOOLS}/appimagetool.AppImage"

# ── 4. Embarquer Qt + le pilote MySQL (linuxdeploy) ──────────────────────────
export APPIMAGE_EXTRACT_AND_RUN=1                 # tourne sans FUSE
export PATH="${TOOLS}:${PATH}"                    # pour trouver le plugin qt
export QMAKE                                      # le plugin qt localise Qt via qmake
export EXTRA_QT_PLUGINS="sqldrivers"              # force le pilote MySQL (chargé en dlopen)
"${TOOLS}/linuxdeploy.AppImage" \
    --appdir "${APPDIR}" \
    --executable "${APPDIR}/usr/bin/Rufus" \
    --desktop-file "${APPDIR}/usr/share/applications/rufus.desktop" \
    --icon-file "${APPDIR}/usr/share/icons/hicolor/256x256/apps/rufus.png" \
    --plugin qt

# ── 5. Remplacer l'AppRun par le nôtre (l'installeur) ────────────────────────
# (linuxdeploy en a posé un en symlink vers le binaire : on l'écrase par le nôtre.)
rm -f "${APPDIR}/AppRun"
cp "${SCRIPT_DIR}/AppRun" "${APPDIR}/AppRun"
chmod +x "${APPDIR}/AppRun"
cp "${SCRIPT_DIR}/rufus.png" "${APPDIR}/rufus.png"   # appimagetool veut l'icône à la racine

# ── 6. Empaqueter ────────────────────────────────────────────────────────────
OUT="${REPO}/Rufus-${VER}-x86_64.AppImage"
ARCH=x86_64 "${TOOLS}/appimagetool.AppImage" "${APPDIR}" "${OUT}"
chmod +x "${OUT}"
echo
echo "======================================================================"
echo "  AppImage créée : ${OUT}"
echo "======================================================================"
