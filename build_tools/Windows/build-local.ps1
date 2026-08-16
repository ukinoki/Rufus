<#
.SYNOPSIS
    Construit RufusSetup.exe EN LOCAL sous Windows — équivalent du job « windows » de
    .github/workflows/release.yml, pour itérer sans attendre la CI.

.DESCRIPTION
    Chaîne complète : (pilote QMySQL si absent) -> qmake + nmake -> windeployqt + assets
    -> Inno Setup (RufusVision.iss) -> Rufus-<version>-Setup.exe à la racine du dépôt.

    Le gros du temps, c'est la compilation Qt. Quand on ne touche QUE l'installeur
    (RufusVision.iss), utiliser -InstallerOnly : on réutilise le dossier Deploy\ déjà
    produit et on ne relance QU'Inno Setup → quelques secondes.

.PARAMETER Version
    Numéro de version (= date de publication, format jj-MM-aaaa). Défaut : date du jour.

.PARAMETER QtRoot
    Racine de Qt (ex. C:\Qt\6.10.2\msvc2022_64). Auto-détecté sinon (QT_ROOT_DIR ou C:\Qt\...).

.PARAMETER InstallerOnly
    Ne refait QUE l'installeur (ISCC) à partir du Deploy\ existant. Pas de qmake/nmake.

.PARAMETER SkipQMySQL
    Ne pas (re)construire le pilote QMySQL même s'il est absent.

.PARAMETER Run
    Lance l'installeur produit à la fin.

.EXAMPLE
    .\build_tools\Windows\build-local.ps1                 # build complet
    .\build_tools\Windows\build-local.ps1 -InstallerOnly  # juste l'installeur (rapide)
    .\build_tools\Windows\build-local.ps1 -Run            # build complet + lance le Setup
#>
[CmdletBinding()]
param(
    [string]$Version = (Get-Date -Format 'dd-MM-yyyy'),
    [string]$QtRoot,
    [switch]$InstallerOnly,
    [switch]$SkipQMySQL,
    [switch]$Run
)

$ErrorActionPreference = 'Stop'

# Racine du dépôt = deux niveaux au-dessus de ce script (build_tools\Windows\..\..).
$RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$Deploy   = Join-Path $RepoRoot 'Deploy'
$Setup    = Join-Path $RepoRoot "Rufus-$Version-Setup.exe"
$Iss      = Join-Path $PSScriptRoot 'RufusVision.iss'
Set-Location $RepoRoot

function Trouver-QtRoot {
    param([string]$Hint)
    if ($Hint)               { return $Hint }
    if ($env:QT_ROOT_DIR)    { return $env:QT_ROOT_DIR }
    $candidats = @('C:\Qt\6.10.2\msvc2022_64')
    $candidats += Get-ChildItem 'C:\Qt\6.*\msvc2022_64' -Directory -ErrorAction SilentlyContinue |
                  Sort-Object FullName -Descending | ForEach-Object FullName
    foreach ($c in $candidats) {
        if ($c -and (Test-Path (Join-Path $c 'bin\qmake.exe'))) { return $c }
    }
    throw "Qt introuvable. Passe -QtRoot 'C:\Qt\6.10.2\msvc2022_64' ou définis QT_ROOT_DIR."
}

# Quelle lib client le pilote qsqlmysql.dll réclame-t-il VRAIMENT ? Le pilote est lié, à la
# compilation, soit au connecteur MariaDB (libmariadb.dll), soit au client Oracle (libmysql.dll),
# et il l'exige par son nom exact. On lit sa table d'imports avec dumpbin (présent avec MSVC).
function Detecter-ClientRequis {
    param([string]$Plugin)
    if (-not (Test-Path $Plugin)) { return $null }
    if (-not (Get-Command dumpbin -ErrorAction SilentlyContinue)) { return $null }
    $deps = (& dumpbin /dependents $Plugin 2>$null) -join "`n"
    $m = [regex]::Match($deps, '(?im)^\s*(lib(?:mariadb|mysql)\w*\.dll)\s*$')
    if ($m.Success) { return $m.Groups[1].Value }
    return $null
}

# Localise un DLL par nom dans les emplacements plausibles : Qt (bin / plugins), cache CI, et
# les installations MySQL standard (pour libmysql.dll), puis en dernier recours récursif sous Qt.
function Trouver-Dll {
    param([string]$Nom, [string]$QtRoot)
    $spots = @(
        (Join-Path $QtRoot "bin\$Nom"),
        (Join-Path $QtRoot "plugins\sqldrivers\$Nom"),
        (Join-Path $RepoRoot "qmysql-cache-win\$Nom")
    )
    foreach ($base in @("${env:ProgramFiles}\MySQL", "${env:ProgramFiles(x86)}\MySQL")) {
        if (Test-Path $base) {
            $spots += Get-ChildItem $base -Recurse -Filter $Nom -ErrorAction SilentlyContinue | ForEach-Object FullName
        }
    }
    foreach ($s in $spots) { if ($s -and (Test-Path $s)) { return $s } }
    $hit = Get-ChildItem -Path $QtRoot -Recurse -Filter $Nom -ErrorAction SilentlyContinue | Select-Object -First 1
    if ($hit) { return $hit.FullName }
    return $null
}

# Charge l'environnement MSVC (cl, nmake) dans la session courante si nmake n'est pas déjà là.
function Importer-MSVC {
    if (Get-Command nmake -ErrorAction SilentlyContinue) { return }
    $vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
    if (-not (Test-Path $vswhere)) {
        throw "vswhere introuvable : installe Visual Studio 2022 avec l'outillage C++ (Desktop development with C++)."
    }
    $vsPath = & $vswhere -latest -products * `
                 -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
                 -property installationPath
    if (-not $vsPath) { throw "Visual Studio avec l'outillage C++ x64 introuvable." }
    $vcvars = Join-Path $vsPath 'VC\Auxiliary\Build\vcvars64.bat'
    if (-not (Test-Path $vcvars)) { throw "vcvars64.bat introuvable sous $vsPath." }
    Write-Host "Chargement de l'environnement MSVC x64…"
    cmd /c "`"$vcvars`" && set" | ForEach-Object {
        if ($_ -match '^([^=]+)=(.*)$') { Set-Item -Path ("env:" + $matches[1]) -Value $matches[2] }
    }
}

# ── Build complet (sauf si -InstallerOnly) ───────────────────────────────────
if (-not $InstallerOnly) {
    $QtRoot = Trouver-QtRoot -Hint $QtRoot
    Write-Host "Qt        : $QtRoot"
    $env:QT_ROOT_DIR = $QtRoot
    $env:PATH        = "$QtRoot\bin;$env:PATH"
    Importer-MSVC

    # Pilote QMySQL (le Qt officiel Windows ne le livre pas). Construit une seule fois.
    if (-not $SkipQMySQL) {
        $plugin = Join-Path $QtRoot 'plugins\sqldrivers\qsqlmysql.dll'
        if (-not (Test-Path $plugin)) {
            Write-Host "Pilote QMySQL absent → construction…"
            & (Join-Path $PSScriptRoot 'build-qmysql-windows.ps1')
            if ($LASTEXITCODE -ne 0) { throw "La construction du pilote QMySQL a échoué." }
        } else {
            Write-Host "Pilote QMySQL : déjà présent."
        }
    }

    Write-Host "Compilation (qmake + nmake, Release)…"
    & qmake RufusQt6.pro CONFIG+=release
    if ($LASTEXITCODE -ne 0) { throw "qmake a échoué." }
    & nmake
    if ($LASTEXITCODE -ne 0) { throw "nmake a échoué." }

    Write-Host "Déploiement (windeployqt + assets + pilote MySQL)…"
    Remove-Item $Deploy -Recurse -Force -ErrorAction SilentlyContinue
    New-Item -ItemType Directory -Force -Path $Deploy | Out-Null
    # Choisir le Rufus.exe FRAÎCHEMENT compilé : le plus RÉCENT, en excluant Deploy\ et les dossiers
    # de build QtCreator (build-*). Sinon un vieux Rufus.exe (p.ex. d'un build QtCreator) pouvait
    # être empaqueté à la place du neuf — et AUCUNE modification du code ne se voyait.
    $exe = Get-ChildItem -Recurse -Filter Rufus.exe -ErrorAction SilentlyContinue |
           Where-Object { $_.FullName -notlike '*\Deploy\*' -and $_.FullName -notlike '*build-*' } |
           Sort-Object LastWriteTime -Descending | Select-Object -First 1
    if (-not $exe) { throw "Rufus.exe introuvable après nmake." }
    Write-Host ("Rufus.exe empaqueté : {0}  (compilé le {1})" -f $exe.FullName, $exe.LastWriteTime) -ForegroundColor Cyan
    Copy-Item $exe.FullName "$Deploy\Rufus.exe"
    & "$QtRoot\bin\windeployqt.exe" --release --compiler-runtime --sql "$Deploy\Rufus.exe"
    if ($LASTEXITCODE -ne 0) { throw "windeployqt a échoué." }
    Copy-Item -Recurse -Force assets "$Deploy\assets"
    # Traductions et clients SQL : mêmes emplacements que la CI (release.yml), lus par
    # cheminQmLangue et Procedures::setDirSQLExecutable.
    New-Item -ItemType Directory -Force -Path "$Deploy\Locale" | Out-Null
    Copy-Item "$RepoRoot\rufus_*.qm" "$Deploy\Locale\" -Force
    $mysqlSrc = Get-ChildItem -Path 'C:\tools\mysql','C:\Program Files\MySQL' -Recurse `
                  -Filter mysqldump.exe -ErrorAction SilentlyContinue | Select-Object -First 1
    if (-not $mysqlSrc) { throw "mysql.exe / mysqldump.exe introuvables : l'installeur ne pourrait ni sauvegarder ni restaurer." }
    New-Item -ItemType Directory -Force -Path "$Deploy\Applications" | Out-Null
    Copy-Item "$($mysqlSrc.DirectoryName)\mysql.exe","$($mysqlSrc.DirectoryName)\mysqldump.exe" `
              "$Deploy\Applications\" -Force
    Write-Host "Traductions -> Deploy\Locale ; clients SQL -> Deploy\Applications ($($mysqlSrc.DirectoryName))"
    # Lib client SQL dont DÉPEND réellement le pilote (libmariadb.dll de notre build, ou libmysql.dll
    # du client Oracle) : on détecte la dépendance exacte, on la localise, et on la copie à côté de
    # Rufus.exe. Sans elle, qsqlmysql.dll ne se charge pas et Rufus ne voit pas MySQL.
    $client = Detecter-ClientRequis (Join-Path $Deploy 'sqldrivers\qsqlmysql.dll')
    if (-not $client) { $client = 'libmariadb.dll' }   # défaut : notre build standard
    $clientPath = Trouver-Dll $client $QtRoot
    if (-not $clientPath) {
        throw "Lib client SQL « $client » (réclamée par qsqlmysql.dll) introuvable. " +
              "Place-la dans $QtRoot\bin, ou construis le pilote MySQL (relance sans -SkipQMySQL)."
    }
    Copy-Item $clientPath "$Deploy\$client" -Force
    Write-Host "Lib client SQL : $client  (depuis $clientPath)"
    # Rufus n'utilise que MySQL : ne garder que ce pilote SQL.
    if (Test-Path "$Deploy\sqldrivers") {
        Get-ChildItem "$Deploy\sqldrivers" -Filter 'qsql*.dll' |
            Where-Object { $_.Name -ne 'qsqlmysql.dll' } | Remove-Item -Force
    }
}

# ── Installeur (toujours) ────────────────────────────────────────────────────
if (-not (Test-Path (Join-Path $Deploy 'Rufus.exe'))) {
    throw "Deploy\Rufus.exe absent : lance d'abord un build COMPLET (sans -InstallerOnly)."
}
$iscc = Get-Command iscc -ErrorAction SilentlyContinue
$isccPath = if ($iscc) { $iscc.Source } else { 'C:\Program Files (x86)\Inno Setup 6\ISCC.exe' }
if (-not (Test-Path $isccPath)) {
    throw "Inno Setup (ISCC.exe) introuvable. Installe-le : choco install innosetup -y  (ou https://jrsoftware.org/isdl.php)."
}
Write-Host "Inno Setup → RufusSetup.exe (version $Version)…"
& $isccPath "/DMyAppVersion=$Version" "/DDeploySrc=$Deploy" $Iss
if ($LASTEXITCODE -ne 0) { throw "ISCC a échoué." }

# ISCC produit RufusSetup.exe dans Output\ à côté du .iss : on le récupère à la racine, renommé.
$built = Get-ChildItem -Path $RepoRoot -Recurse -Filter RufusSetup.exe -ErrorAction SilentlyContinue |
         Sort-Object LastWriteTime -Descending | Select-Object -First 1
if (-not $built) { throw "RufusSetup.exe introuvable après ISCC." }
Copy-Item $built.FullName $Setup -Force

Write-Host ""
Write-Host "OK -> $Setup" -ForegroundColor Green
if ($Run) { Start-Process $Setup }
