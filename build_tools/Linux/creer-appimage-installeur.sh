#!/usr/bin/env bash
#
#  creer-appimage-installeur.sh
#  ----------------------------
#  Fabrique EN LOCAL l'AppImage-installeur de Rufus, à l'identique de ce que fait le
#  workflow GitHub (.github/workflows/release.yml) — pour être autonome, sans dépendre
#  de GitHub. Le résultat est UNE SEULE AppImage qui, lancée sur un poste, s'installe
#  toute seule (contrôle/installe les libs nécessaires + pose Rufus dans les menus),
#  grâce au script AppRun embarqué.
#
#  Ce que le script enchaîne :
#    1. compile Rufus (qmake + make) dans un dossier de build séparé ;
#    2. assemble l'arborescence AppDir (binaire + icône + .desktop + traductions) ;
#    3. embarque Qt et le pilote MySQL avec linuxdeploy + son plugin Qt ;
#    4. remplace l'AppRun par le nôtre (celui qui fait l'auto-installation) ;
#    5. empaquette le tout avec appimagetool → Rufus-<version>-x86_64.AppImage.
#
#  Mode d'emploi détaillé : voir LISEZMOI-creer-appimage.md (même dossier).
#
#  Usage :
#    ./creer-appimage-installeur.sh [chemin/vers/un/binaire/Rufus/déjà/compilé]
#
#    - Sans argument : le script compile Rufus lui-même (il faut Qt/qmake accessible).
#    - Avec un binaire déjà compilé en argument : il saute la compilation et empaquette
#      directement ce binaire (pratique si tu as déjà compilé dans Qt Creator).
#
set -euo pipefail

# ── Repères de dossiers ──────────────────────────────────────────────────────
# Le script vit dans build_tools/Linux/ ; la racine du dépôt est deux crans au-dessus.
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO="$(cd "${SCRIPT_DIR}/../.." && pwd)"
PRO="${REPO}/RufusQt6.pro"
[ -f "${PRO}" ] || { echo "ERREUR : ${PRO} introuvable (le dépôt a-t-il bougé ?)." >&2; exit 1; }

# Dossiers de travail. WORK est jetable (recréé à chaque fois) ; TOOLS est un cache
# des outils AppImage téléchargés une fois pour toutes (hors du dépôt).
WORK="${REPO}/build-appimage"
APPDIR="${WORK}/AppDir"
TOOLS="${HOME}/.cache/rufus-appimage-tools"
OUTDIR="$(pwd)"            # l'AppImage finale est déposée là où tu lances le script

echo "==> Dépôt        : ${REPO}"
echo "==> Build        : ${WORK}"
echo "==> Outils (cache): ${TOOLS}"
echo

# ── Version (= date de sortie), lue dans rufus.cpp, source unique ────────────
#  qApp->setApplicationVersion("10-06-2026/1")  ->  10-06-2026
VER="$(grep -oP 'setApplicationVersion\("\K[^"/]+' "${REPO}/rufus.cpp" | head -n1 || true)"
[ -n "${VER}" ] || VER="inconnue"
echo "==> Version Rufus : ${VER}"

# ── Petit utilitaire de téléchargement (wget ou curl) ────────────────────────
telecharge() {   # telecharge <url> <fichier-destination>
    if command -v wget >/dev/null 2>&1; then
        wget -q "$1" -O "$2"
    elif command -v curl >/dev/null 2>&1; then
        curl -fsSL "$1" -o "$2"
    else
        echo "ERREUR : ni wget ni curl ne sont installés (impossible de télécharger les outils)." >&2
        exit 1
    fi
}

# ── 1. Localiser qmake / Qt ──────────────────────────────────────────────────
# Qt est nécessaire DANS TOUS LES CAS : pour compiler (si besoin) ET pour que
# linuxdeploy-plugin-qt sache où trouver Qt à embarquer. On cherche qmake dans
# l'ordre : variable QMAKE, PATH (qmake6 puis qmake), enfin QT_ROOT_DIR/bin.
BIN_FOURNI="${1:-}"

QMAKE="${QMAKE:-}"
if [ -z "${QMAKE}" ]; then
    QMAKE="$(command -v qmake6 || command -v qmake || true)"
fi
if [ -z "${QMAKE}" ] && [ -n "${QT_ROOT_DIR:-}" ] && [ -x "${QT_ROOT_DIR}/bin/qmake" ]; then
    QMAKE="${QT_ROOT_DIR}/bin/qmake"
fi
if [ -z "${QMAKE}" ]; then
    cat >&2 <<'MSG'
ERREUR : qmake introuvable (Qt est nécessaire même avec un binaire déjà compilé,
         pour embarquer Qt dans l'AppImage).
  Ajoute Qt au PATH, par exemple :
      export PATH="$HOME/Qt/6.10.2/gcc_64/bin:$PATH"
  ou indique-le explicitement :
      QMAKE=$HOME/Qt/6.10.2/gcc_64/bin/qmake ./creer-appimage-installeur.sh
MSG
    exit 1
fi
QT_ROOT_DIR="$("${QMAKE}" -query QT_INSTALL_PREFIX)"
echo "==> qmake        : ${QMAKE}"
echo "==> Qt           : ${QT_ROOT_DIR}"

# ── 2. Compiler Rufus (sauf si un binaire est fourni) ────────────────────────
rm -rf "${WORK}"
mkdir -p "${APPDIR}/usr/bin" \
         "${APPDIR}/usr/share/applications" \
         "${APPDIR}/usr/share/icons/hicolor/256x256/apps" \
         "${APPDIR}/usr/Locale"

if [ -n "${BIN_FOURNI}" ]; then
    [ -x "${BIN_FOURNI}" ] || { echo "ERREUR : ${BIN_FOURNI} n'est pas un exécutable." >&2; exit 1; }
    echo "==> Binaire fourni : ${BIN_FOURNI} (compilation sautée)"
    cp "${BIN_FOURNI}" "${APPDIR}/usr/bin/Rufus"
else
    echo "==> Compilation (qmake + make, Release)…"
    ( cd "${WORK}" && "${QMAKE}" "${PRO}" CONFIG+=release && make -j"$(nproc)" )
    # Le binaire produit s'appelle Rufus (TARGET = Rufus dans le .pro). Le « || true »
    # évite l'échec du pipe (head ferme tôt) sous « set -o pipefail ».
    BIN="$(find "${WORK}" -maxdepth 2 -type f -executable \( -name 'Rufus' -o -name 'RufusQt6' \) ! -name '*.so*' | head -n1 || true)"
    [ -n "${BIN}" ] || { echo "ERREUR : binaire Rufus introuvable après compilation." >&2; exit 1; }
    echo "==> Binaire      : ${BIN}"
    cp "${BIN}" "${APPDIR}/usr/bin/Rufus"
fi

# ── 3. Icône, .desktop, traductions ──────────────────────────────────────────
cp "${SCRIPT_DIR}/rufus.png" "${APPDIR}/usr/share/icons/hicolor/256x256/apps/rufus.png"

# .desktop généré ici (comme le workflow) : Exec=Rufus (nom du binaire dans usr/bin).
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

# Traductions : l'appli lit <dir-binaire>/../Locale/rufus_xx.qm (cf. rufus.cpp).
if ls "${REPO}"/rufus_*.qm >/dev/null 2>&1; then
    cp "${REPO}"/rufus_*.qm "${APPDIR}/usr/Locale/"
else
    echo "ATTENTION : aucun rufus_*.qm trouvé — l'AppImage n'aura pas les traductions." >&2
fi

# ── 4. Récupérer les outils AppImage (téléchargés une fois, mis en cache) ─────
mkdir -p "${TOOLS}"
LD="${TOOLS}/linuxdeploy-x86_64.AppImage"
LDQT="${TOOLS}/linuxdeploy-plugin-qt-x86_64.AppImage"
ATOOL="${TOOLS}/appimagetool-x86_64.AppImage"
[ -f "${LD}" ]    || { echo "==> Téléchargement linuxdeploy…";            telecharge "https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage" "${LD}"; }
[ -f "${LDQT}" ]  || { echo "==> Téléchargement linuxdeploy-plugin-qt…";  telecharge "https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage" "${LDQT}"; }
[ -f "${ATOOL}" ] || { echo "==> Téléchargement appimagetool…";           telecharge "https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-x86_64.AppImage" "${ATOOL}"; }
chmod +x "${LD}" "${LDQT}" "${ATOOL}"

# ── 5. Bundling Qt avec linuxdeploy + plugin Qt ──────────────────────────────
# Les outils AppImage doivent pouvoir tourner sans FUSE (fréquent hors CI).
export APPIMAGE_EXTRACT_AND_RUN=1
# Le plugin Qt doit être trouvable par linuxdeploy et localise Qt via qmake.
cp -f "${LDQT}" "${WORK}/linuxdeploy-plugin-qt-x86_64.AppImage"
export PATH="${WORK}:${PATH}"
# Le pilote MySQL (QMYSQL) est chargé dynamiquement (dlopen) : linuxdeploy ne peut pas
# le deviner depuis le binaire, on force donc l'embarquement des sqldrivers.
export EXTRA_QT_PLUGINS="sqldrivers"

# On ne garde QUE le pilote MySQL parmi les sqldrivers. Contrairement au runner CI (jetable),
# ON NE SUPPRIME RIEN dans TON install Qt : on écarte temporairement les autres pilotes, puis
# on les RESTAURE en sortie (même en cas d'erreur) via un trap. Ça évite aussi que le plugin Qt
# échoue sur un pilote aux dépendances absentes (ex. Oracle qsqloci -> libclntsh.so).
STASH=""
restaurer_sqldrivers() {
    [ -n "${STASH}" ] && [ -d "${STASH}" ] || return 0
    mv "${STASH}"/*.so "${SQLDIR}/" 2>/dev/null || true
    rmdir "${STASH}" 2>/dev/null || true
}
if [ -n "${QT_ROOT_DIR:-}" ]; then
    SQLDIR="${QT_ROOT_DIR}/plugins/sqldrivers"
    if [ -d "${SQLDIR}" ] && [ -w "${SQLDIR}" ]; then
        STASH="$(mktemp -d)"
        trap restaurer_sqldrivers EXIT
        for so in "${SQLDIR}"/libqsql*.so; do
            [ -e "${so}" ] || continue
            case "$(basename "${so}")" in
                libqsqlmysql.so) : ;;                 # on garde MySQL
                *) mv "${so}" "${STASH}/" ;;          # on écarte le reste le temps du build
            esac
        done
    elif [ -d "${SQLDIR}" ]; then
        echo "ATTENTION : ${SQLDIR} non modifiable — tous les pilotes SQL seront embarqués." >&2
    fi
fi

echo "==> Bundling Qt (linuxdeploy + plugin qt)…"
QMAKE="${QMAKE:-${QT_ROOT_DIR}/bin/qmake}" \
"${LD}" \
    --appdir "${APPDIR}" \
    --executable "${APPDIR}/usr/bin/Rufus" \
    --desktop-file "${APPDIR}/usr/share/applications/rufus.desktop" \
    --icon-file "${APPDIR}/usr/share/icons/hicolor/256x256/apps/rufus.png" \
    --plugin qt

# ── 6. Injecter NOTRE AppRun (celui qui fait l'auto-installation) ────────────
# ⚠ linuxdeploy a posé AppDir/AppRun en SYMLINK vers usr/bin/Rufus : on le SUPPRIME
# d'abord (sinon cp suit le lien et écrase le binaire).
rm -f "${APPDIR}/AppRun"
cp "${SCRIPT_DIR}/AppRun" "${APPDIR}/AppRun"
chmod +x "${APPDIR}/AppRun"

# Garde-fou : le binaire doit rester un vrai ELF (pas écrasé par le script AppRun).
if ! file -b "${APPDIR}/usr/bin/Rufus" | grep -q '^ELF'; then
    echo "ERREUR : ${APPDIR}/usr/bin/Rufus n'est pas un ELF (binaire écrasé ?)." >&2
    exit 1
fi

# appimagetool veut aussi l'icône à la RACINE de l'AppDir (Icon=rufus du .desktop).
rm -f "${APPDIR}/rufus.png"
cp "${SCRIPT_DIR}/rufus.png" "${APPDIR}/rufus.png"

# ── 7. Empaquetage final ─────────────────────────────────────────────────────
OUT="${OUTDIR}/Rufus-${VER}-x86_64.AppImage"
echo "==> Empaquetage → ${OUT}"
ARCH=x86_64 "${ATOOL}" "${APPDIR}" "${OUT}"
chmod +x "${OUT}"

# Garde-fou : une AppImage complète (binaire + Qt) pèse des dizaines de Mo.
SZ="$(stat -c%s "${OUT}")"
echo "==> Taille       : ${SZ} octets"
if [ "${SZ}" -lt 20000000 ]; then
    echo "ERREUR : AppImage anormalement petite (${SZ} octets) — le bundling Qt a probablement échoué." >&2
    exit 1
fi

echo
echo "======================================================================"
echo "  Terminé : ${OUT}"
echo "  Pour installer Rufus sur ce poste ou un autre : lance simplement"
echo "  cette AppImage (double-clic ou ./Rufus-${VER}-x86_64.AppImage)."
echo "  Elle s'installe toute seule (libs + menus)."
echo "======================================================================"
