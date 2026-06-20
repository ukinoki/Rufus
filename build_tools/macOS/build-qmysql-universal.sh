#!/bin/bash
# ─────────────────────────────────────────────────────────────────────────────
#  build-qmysql-universal.sh — construit le pilote Qt « qsqlmysql » en binaire
#  UNIVERSEL (x86_64 + arm64) pour macOS et l'installe dans les plugins de Qt.
#
#  POURQUOI : le Qt officiel pour macOS (celui d'install-qt-action / de l'installeur
#  en ligne) ne LIVRE PAS le pilote MySQL (contrairement à Linux/Windows). Sans lui,
#  Rufus ne peut ni se connecter à MySQL ni sauvegarder/restaurer sur Mac.
#
#  STRATÉGIE (tout depuis les sources, en universel) :
#    1. OpenSSL  -> bibliothèques STATIQUES universelles (libssl.a + libcrypto.a) ;
#    2. MariaDB Connector/C -> libmariadb.dylib universelle, AVEC OpenSSL embarqué
#       statiquement -> lib client AUTONOME (pas de dépendance OpenSSL externe) ;
#    3. pilote Qt qsqlmysql -> universel, lié à cette libmariadb.dylib ;
#    4. installation dans $QT/plugins/sqldrivers (macdeployqt l'embarquera ensuite).
#
#  Peut se lancer en local sur un Mac (pour valider) OU dans la CI. Prérequis :
#  Qt dans le PATH (qmake, qt-cmake), Xcode (clang, lipo), cmake, curl, perl.
#  Vérif finale :  lipo -archs <…>/libqsqlmysql.dylib  ->  « x86_64 arm64 ».
# ─────────────────────────────────────────────────────────────────────────────
set -euo pipefail

# Versions épinglées (lignes stables/LTS).
OSSL_VER="${OSSL_VER:-3.0.16}"
MARIADB_TAG="${MARIADB_TAG:-v3.4.3}"

# Localiser Qt (qmake + qt-cmake). Priorité : PATH ; sinon $QTDIR / $QT_ROOT_DIR ; sinon la
# version la plus récente sous ~/Qt/<ver>/macos/bin (install Qt officielle). En CI, qmake est
# déjà dans le PATH (install-qt-action) → premier cas.
find_qt_bin() {
    if command -v qmake >/dev/null 2>&1; then dirname "$(command -v qmake)"; return 0; fi
    [ -n "${QTDIR:-}" ]        && [ -x "${QTDIR}/bin/qmake" ]        && { echo "${QTDIR}/bin"; return 0; }
    [ -n "${QT_ROOT_DIR:-}" ]  && [ -x "${QT_ROOT_DIR}/bin/qmake" ]  && { echo "${QT_ROOT_DIR}/bin"; return 0; }
    local cand
    cand="$(ls -d "${HOME}"/Qt/*/macos/bin/qmake 2>/dev/null | sort -V | tail -n1)"
    [ -n "${cand}" ] && { dirname "${cand}"; return 0; }
    return 1
}
QT_BIN="$(find_qt_bin)" || {
    echo "ERREUR : qmake introuvable." >&2
    echo "  -> ajoute Qt au PATH, ou exporte QTDIR (ex. export QTDIR=~/Qt/6.10.2/macos)" >&2
    exit 1
}
export PATH="${QT_BIN}:${PATH}"
QMAKE="${QT_BIN}/qmake"
QTCMAKE="${QT_BIN}/qt-cmake"
echo "== Qt détecté : ${QT_BIN}"

# Localiser cmake : PATH, sinon celui embarqué par le Qt officiel (~/Qt/Tools/CMake…).
# qt-cmake appelle cmake → on ajoute aussi son dossier au PATH. En CI, cmake est déjà présent.
find_cmake() {
    command -v cmake >/dev/null 2>&1 && { command -v cmake; return 0; }
    local c
    c="$(ls -d "${HOME}"/Qt/Tools/CMake*/CMake.app/Contents/bin/cmake 2>/dev/null | sort -V | tail -n1)"
    [ -n "${c}" ] && { echo "${c}"; return 0; }
    return 1
}
CMAKE="$(find_cmake)" || {
    echo "ERREUR : cmake introuvable." >&2
    echo "  -> installe-le : 'brew install cmake', OU via le Qt Maintenance Tool" >&2
    echo "     (Developer and Designer Tools > CMake)." >&2
    exit 1
}
export PATH="$(dirname "${CMAKE}"):${PATH}"
echo "== cmake détecté : ${CMAKE}"

QT_PREFIX="$("${QMAKE}" -query QT_INSTALL_PREFIX)"
QT_PLUGINS="$("${QMAKE}" -query QT_INSTALL_PLUGINS)"
QT_VER="$("${QMAKE}" -query QT_VERSION)"
QT_MINOR="${QT_VER%.*}"
ARCHS="x86_64;arm64"
DEPLOY_TARGET="11.0"
JOBS="$(sysctl -n hw.ncpu)"
WORK="$(mktemp -d)"

echo "== Pilote MySQL universel — Qt ${QT_VER}"
echo "   plugins : ${QT_PLUGINS}"
echo "   travail : ${WORK}"

# ── 1. OpenSSL universel (statique) ───────────────────────────────────────────
cd "${WORK}"
curl -fL -o openssl.tar.gz \
    "https://github.com/openssl/openssl/releases/download/openssl-${OSSL_VER}/openssl-${OSSL_VER}.tar.gz"

build_openssl() {                     # $1 = arch (x86_64|arm64), $2 = cible OpenSSL
    local arch="$1" target="$2"
    rm -rf "openssl-${OSSL_VER}"
    tar xf openssl.tar.gz
    ( cd "openssl-${OSSL_VER}"
      ./Configure "${target}" no-shared no-tests \
          -mmacosx-version-min="${DEPLOY_TARGET}" --prefix="${WORK}/ossl-${arch}" >/dev/null
      make -j"${JOBS}" >/dev/null
      make install_sw >/dev/null )
}
echo "-- OpenSSL ${OSSL_VER} (x86_64)…" ; build_openssl x86_64 darwin64-x86_64-cc
echo "-- OpenSSL ${OSSL_VER} (arm64)…"  ; build_openssl arm64  darwin64-arm64-cc

mkdir -p "${WORK}/ossl/lib"
lipo -create "${WORK}/ossl-x86_64/lib/libssl.a"    "${WORK}/ossl-arm64/lib/libssl.a"    -output "${WORK}/ossl/lib/libssl.a"
lipo -create "${WORK}/ossl-x86_64/lib/libcrypto.a" "${WORK}/ossl-arm64/lib/libcrypto.a" -output "${WORK}/ossl/lib/libcrypto.a"
cp -R "${WORK}/ossl-arm64/include" "${WORK}/ossl/include"
echo "   OpenSSL universel : $(lipo -archs "${WORK}/ossl/lib/libssl.a")"

# ── 2. MariaDB Connector/C universel (OpenSSL statique embarqué) ───────────────
cd "${WORK}"
curl -fL -o mariadb.tar.gz \
    "https://github.com/mariadb-corporation/mariadb-connector-c/archive/refs/tags/${MARIADB_TAG}.tar.gz"
tar xf mariadb.tar.gz
MARIADB_SRC="${WORK}/mariadb-connector-c-${MARIADB_TAG#v}"

"${CMAKE}" -S "${MARIADB_SRC}" -B "${WORK}/mariadb-build" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
    -DCMAKE_OSX_ARCHITECTURES="${ARCHS}" \
    -DCMAKE_OSX_DEPLOYMENT_TARGET="${DEPLOY_TARGET}" \
    -DWITH_SSL=OPENSSL \
    -DOPENSSL_ROOT_DIR="${WORK}/ossl" \
    -DOPENSSL_USE_STATIC_LIBS=TRUE \
    -DWITH_EXTERNAL_ZLIB=ON \
    -DWITH_UNIT_TESTS=OFF \
    -DCMAKE_DISABLE_FIND_PACKAGE_ZSTD=ON \
    -DWITH_ZSTD=OFF \
    -DCLIENT_PLUGIN_ZSTD=OFF \
    -DCMAKE_C_FLAGS="-w" \
    -DCMAKE_INSTALL_PREFIX="${WORK}/mariadb" >/dev/null
echo "-- MariaDB Connector/C ${MARIADB_TAG}…"
# On construit TOUTES les cibles (lib + plugins comme dialog.so) : l'install les attend toutes.
"${CMAKE}" --build "${WORK}/mariadb-build" -j"${JOBS}" >/dev/null
"${CMAKE}" --install "${WORK}/mariadb-build" >/dev/null

MARIADB_DYLIB="$(find "${WORK}/mariadb" -name 'libmariadb*.dylib' -type f | head -n1)"
MARIADB_INC="$(dirname "$(find "${WORK}/mariadb" -name 'mysql.h' | head -n1)")"
[ -n "${MARIADB_DYLIB}" ] || { echo "ERREUR : libmariadb.dylib introuvable après build"; exit 1; }
# install_name ABSOLU : pour que macdeployqt puisse suivre la dépendance et l'embarquer.
install_name_tool -id "${MARIADB_DYLIB}" "${MARIADB_DYLIB}"
echo "   libmariadb : ${MARIADB_DYLIB} ($(lipo -archs "${MARIADB_DYLIB}"))"

# ── 3. Pilote Qt qsqlmysql universel ──────────────────────────────────────────
# La chaîne Qt (build « qt_build_repo ») IGNORE -DCMAKE_OSX_ARCHITECTURES et impose x86_64.
# On force donc les DEUX archis au niveau du compilateur ET de l'éditeur de liens
# (-arch x86_64 -arch arm64), ce que la chaîne Qt ne peut pas écraser → binaire universel
# en un seul build (clang dédoublonne les -arch redondants).
cd "${WORK}"
curl -fL -o qtbase.tar.xz \
    "https://download.qt.io/official_releases/qt/${QT_MINOR}/${QT_VER}/submodules/qtbase-everywhere-src-${QT_VER}.tar.xz"
mkdir -p qtbase && tar xf qtbase.tar.xz -C qtbase --strip-components=1

ARCHFLAGS="-arch x86_64 -arch arm64"
echo "-- Pilote Qt qsqlmysql (universel, -arch forcé)…"
"${QTCMAKE}" -S qtbase/src/plugins/sqldrivers -B "${WORK}/sqldrv-build" \
    -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64" \
    -DCMAKE_C_FLAGS="${ARCHFLAGS}" \
    -DCMAKE_CXX_FLAGS="${ARCHFLAGS}" \
    -DCMAKE_SHARED_LINKER_FLAGS="${ARCHFLAGS}" \
    -DCMAKE_MODULE_LINKER_FLAGS="${ARCHFLAGS}" \
    -DMySQL_INCLUDE_DIR="${MARIADB_INC}" \
    -DMySQL_LIBRARY="${MARIADB_DYLIB}" >/dev/null
"${CMAKE}" --build "${WORK}/sqldrv-build" -j"${JOBS}" >/dev/null

DRV="$(find "${WORK}/sqldrv-build" -name 'libqsqlmysql.dylib' -type f | head -n1)"
[ -n "${DRV}" ] || { echo "ERREUR : pilote non construit" >&2; exit 1; }
DRIVER="${QT_PLUGINS}/sqldrivers/libqsqlmysql.dylib"
cp -f "${DRV}" "${DRIVER}"

# Dépendance libmariadb STABLE : le pilote pointait vers le dossier TEMPORAIRE de build
# (éphémère). On copie libmariadb dans le lib/ de Qt et on réécrit la référence du pilote vers
# ce chemin fixe. -> Rufus en dev local trouve la lib ; et macdeployqt suivra ce chemin pour
# l'embarquer dans le bundle. (libmariadb contient déjà OpenSSL en statique : aucune autre dépendance.)
MARIADB_BASE="$(basename "${MARIADB_DYLIB}")"
STABLE_MARIADB="${QT_PREFIX}/lib/${MARIADB_BASE}"
cp -f "${MARIADB_DYLIB}" "${STABLE_MARIADB}"
chmod u+w "${STABLE_MARIADB}"
install_name_tool -id "@rpath/${MARIADB_BASE}" "${STABLE_MARIADB}" 2>/dev/null || true
# Référence RELATIVE depuis le pilote (plugins/sqldrivers) vers Qt/lib : indépendante de
# l'emplacement ABSOLU de Qt -> survit à un déplacement de Qt ET au cache CI (où le chemin
# absolu de Qt peut varier d'un run à l'autre). macdeployqt sait suivre @loader_path.
install_name_tool -change "${MARIADB_DYLIB}" "@loader_path/../../lib/${MARIADB_BASE}" "${DRIVER}" 2>/dev/null || true
echo "   libmariadb : ${STABLE_MARIADB} (réf. pilote : @loader_path/../../lib/${MARIADB_BASE})"

echo "== OK : ${DRIVER}"
echo "   architectures : $(lipo -archs "${DRIVER}")"
echo "   dépendances (otool) :"
otool -L "${DRIVER}" | sed -n '2,8p'
