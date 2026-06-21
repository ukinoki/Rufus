# ═════════════════════════════════════════════════════════════════════════════
#  EN CLAIR (pour aller vite) :
#  Sous Windows, Qt ne fournit PAS la pièce qui permet à Rufus de parler à MySQL.
#  Ce script la fabrique (qsqlmysql.dll) et la range dans ton Qt, avec sa lib
#  client (libmariadb.dll). On le lance une fois ; ensuite Rufus sait se connecter
#  à MySQL. C'est l'équivalent Windows de build-qmysql-universal.sh (macOS), en
#  plus simple : pas d'universel (x64 seul), et TLS via Schannel (Windows) -> pas
#  d'OpenSSL à construire.
# ─────────────────────────────────────────────────────────────────────────────
#  build-qmysql-windows.ps1 — construit le pilote Qt « qsqlmysql » pour Windows x64.
#
#  STRATÉGIE :
#    1. MariaDB Connector/C -> libmariadb.dll + mariadb.lib (TLS Schannel intégré,
#       plugins d'auth caching_sha2/sha256/native en STATIQUE) ;
#    2. pilote Qt qsqlmysql -> qsqlmysql.dll, lié à cette lib client ;
#    3. installation de qsqlmysql.dll dans Qt\plugins\sqldrivers et de libmariadb.dll
#       dans Qt\bin (windeployqt l'embarquera, et on la copie aussi près de Rufus.exe).
#
#  Prérequis : environnement MSVC chargé (msvc-dev-cmd), Qt dans QT_ROOT_DIR
#  (install-qt-action), cmake + ninja dans le PATH (présents sur les runners GitHub).
# ─────────────────────────────────────────────────────────────────────────────
$ErrorActionPreference = "Stop"

$MARIADB_TAG = "v3.4.3"

# --- Localiser Qt -------------------------------------------------------------
$QtRoot = $env:QT_ROOT_DIR
if (-not $QtRoot) { throw "QT_ROOT_DIR non defini (Qt introuvable)." }
$Qmake   = Join-Path $QtRoot "bin\qmake.exe"
$QtCmake = Join-Path $QtRoot "bin\qt-cmake.bat"
$QtVer    = (& $Qmake -query QT_VERSION).Trim()
$QtMinor  = $QtVer.Substring(0, $QtVer.LastIndexOf('.'))
$QtPlugins = (& $Qmake -query QT_INSTALL_PLUGINS).Trim()
Write-Host "== Qt $QtVer  | plugins: $QtPlugins"

$Work = Join-Path $env:RUNNER_TEMP "qmysql"
New-Item -ItemType Directory -Force -Path $Work | Out-Null
Set-Location $Work

# --- 1. MariaDB Connector/C (DLL) --------------------------------------------
Write-Host "-- MariaDB Connector/C $MARIADB_TAG (source)"
Invoke-WebRequest -UseBasicParsing `
  -Uri "https://github.com/mariadb-corporation/mariadb-connector-c/archive/refs/tags/$MARIADB_TAG.tar.gz" `
  -OutFile "mariadb.tar.gz"
tar xf mariadb.tar.gz
$MariaSrc = Join-Path $Work ("mariadb-connector-c-" + $MARIADB_TAG.TrimStart('v'))

# NB PowerShell : chaque -D... est ENTRE GUILLEMETS, sinon PowerShell peut découper la valeur
# (ex. « =3.5 » devient « =3 » + « .5 » -> CMAKE_POLICY_VERSION_MINIMUM invalide). Les guillemets
# forcent le passage d'un seul bloc à cmake.
cmake -S $MariaSrc -B "$Work\mariadb-build" -G Ninja `
  "-DCMAKE_BUILD_TYPE=Release" `
  "-DCMAKE_POLICY_VERSION_MINIMUM=3.5" `
  "-DWITH_SSL=SCHANNEL" `
  "-DWITH_UNIT_TESTS=OFF" `
  "-DCMAKE_DISABLE_FIND_PACKAGE_ZSTD=ON" `
  "-DWITH_ZSTD=OFF" `
  "-DCLIENT_PLUGIN_ZSTD=OFF" `
  "-DCLIENT_PLUGIN_CACHING_SHA2_PASSWORD=STATIC" `
  "-DCLIENT_PLUGIN_SHA256_PASSWORD=STATIC" `
  "-DCLIENT_PLUGIN_MYSQL_NATIVE_PASSWORD=STATIC" `
  "-DCMAKE_INSTALL_PREFIX=$Work\mariadb"
cmake --build "$Work\mariadb-build" --config Release
cmake --install "$Work\mariadb-build"

$MariaDll = (Get-ChildItem -Recurse -Path "$Work\mariadb" -Filter "libmariadb.dll" | Select-Object -First 1)
$MariaLib = (Get-ChildItem -Recurse -Path "$Work\mariadb" -Include "libmariadb.lib","mariadbclient.lib","mariadb.lib" -File | Select-Object -First 1)
$MariaInc = (Get-ChildItem -Recurse -Path "$Work\mariadb" -Filter "mysql.h" | Select-Object -First 1).Directory.FullName
if (-not $MariaDll) { throw "libmariadb.dll introuvable apres build." }
if (-not $MariaLib) { throw "lib d'import MariaDB (.lib) introuvable apres build." }
Write-Host "   libmariadb.dll : $($MariaDll.FullName)"
Write-Host "   import lib     : $($MariaLib.FullName)"
Write-Host "   include        : $MariaInc"

# --- 2. Pilote Qt qsqlmysql ---------------------------------------------------
Write-Host "-- Sources qtbase $QtVer (pour le pilote sqldrivers)"
Invoke-WebRequest -UseBasicParsing `
  -Uri "https://download.qt.io/official_releases/qt/$QtMinor/$QtVer/submodules/qtbase-everywhere-src-$QtVer.zip" `
  -OutFile "qtbase.zip"
Expand-Archive -Force "qtbase.zip" -DestinationPath "qtbase-extract"
$SqlSrc = (Get-ChildItem -Recurse -Path "qtbase-extract" -Directory -Filter "sqldrivers" |
           Where-Object { Test-Path (Join-Path $_.FullName "CMakeLists.txt") } | Select-Object -First 1).FullName
if (-not $SqlSrc) { throw "Source des sqldrivers introuvable dans qtbase." }

Write-Host "-- Pilote Qt qsqlmysql"
& $QtCmake -S $SqlSrc -B "$Work\sqldrv" -G Ninja `
  "-DMySQL_INCLUDE_DIR=$MariaInc" `
  "-DMySQL_LIBRARY=$($MariaLib.FullName)"
cmake --build "$Work\sqldrv" --config Release

$Driver = (Get-ChildItem -Recurse -Path "$Work\sqldrv" -Filter "qsqlmysql.dll" | Select-Object -First 1)
if (-not $Driver) { throw "qsqlmysql.dll non construit." }

# --- 3. Installation dans Qt --------------------------------------------------
$DestPlugins = Join-Path $QtPlugins "sqldrivers"
New-Item -ItemType Directory -Force -Path $DestPlugins | Out-Null
Copy-Item $Driver.FullName (Join-Path $DestPlugins "qsqlmysql.dll") -Force
# libmariadb.dll dans Qt\bin : windeployqt la voit, et on la rapatriera aussi pres de Rufus.exe.
Copy-Item $MariaDll.FullName (Join-Path $QtRoot "bin\libmariadb.dll") -Force

Write-Host "== OK : qsqlmysql.dll installe dans $DestPlugins"
Write-Host "        libmariadb.dll copie dans $QtRoot\bin"
