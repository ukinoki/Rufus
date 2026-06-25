<#
  DIAGNOSTIC de la désinstallation MySQL (Windows) — exécute la même logique que
  uninstallMySQL() de Rufus, mais avec TOUTES les sorties visibles, pour voir où ça coince.

  À lancer dans un PowerShell *ADMINISTRATEUR* (clic droit ▸ Exécuter en administrateur) :

      powershell -ExecutionPolicy Bypass -File .\build_tools\Windows\test\uninstall-mysql-diag.ps1

  Ne modifie rien tant que tu n'as pas confirmé : il SUPPRIME MySQL (comme Rufus le ferait).
  Colle-moi toute la sortie.
#>
$ErrorActionPreference = 'Continue'

Write-Host "===== 1. Suis-je administrateur ? =====" -ForegroundColor Cyan
$id  = [Security.Principal.WindowsIdentity]::GetCurrent()
$adm = (New-Object Security.Principal.WindowsPrincipal($id)).IsInRole(
         [Security.Principal.WindowsBuiltinRole]::Administrator)
Write-Host "   Élevé administrateur : $adm"
if (-not $adm) {
    Write-Host "   >>> PAS administrateur : relance ce script dans un PowerShell admin." -ForegroundColor Yellow
    return
}

Write-Host "`n===== 2. Service MySQL (ImagePath -> mysqld.exe) =====" -ForegroundColor Cyan
$svc = Get-CimInstance Win32_Service | Where-Object { $_.PathName -like '*mysqld.exe*' } |
       Select-Object -First 1
if (-not $svc) {
    Write-Host "   AUCUN service mysqld trouvé."
} else {
    Write-Host "   Service  : $($svc.Name)   | État : $($svc.State)"
    Write-Host "   PathName : $($svc.PathName)"
    $exe = $svc.PathName.Trim([char]34)
    $i   = $exe.ToLower().IndexOf('mysqld.exe')
    if ($i -ge 0) { $exe = $exe.Substring(0, $i + 10) }
    $base = Split-Path (Split-Path ($exe.Trim().Trim([char]34)) -Parent) -Parent
    Write-Host "   Base déduite : $base"

    Write-Host "   -> Stop-Service $($svc.Name)"
    try { Stop-Service -Name $svc.Name -Force -ErrorAction Stop; Write-Host "      OK" }
    catch { Write-Host "      ÉCHEC : $($_.Exception.Message)" -ForegroundColor Red }

    Write-Host "   -> sc.exe stop / delete"
    sc.exe stop   $svc.Name
    sc.exe delete $svc.Name
}

Start-Sleep -Seconds 2
Write-Host "`n===== 3. Process mysqld encore vivants ? =====" -ForegroundColor Cyan
$procs = Get-Process mysqld -ErrorAction SilentlyContinue
if ($procs) { $procs | Format-Table Id, Path -AutoSize | Out-String | Write-Host }
else        { Write-Host "   aucun" }

Write-Host "`n===== 4. Suppression des dossiers SERVEUR =====" -ForegroundColor Cyan
$dirs = @()
$dirs += Get-ChildItem 'C:\Program Files\MySQL' -Directory -Filter 'MySQL Server *' -ErrorAction SilentlyContinue
$dirs += Get-ChildItem 'C:\ProgramData\MySQL'  -Directory -Filter 'MySQL Server *' -ErrorAction SilentlyContinue
if (-not $dirs) { Write-Host "   (aucun dossier 'MySQL Server *' trouvé)" }
foreach ($dir in $dirs) {
    Write-Host "   Remove : $($dir.FullName)"
    try { Remove-Item -LiteralPath $dir.FullName -Recurse -Force -ErrorAction Stop; Write-Host "      OK" -ForegroundColor Green }
    catch { Write-Host "      ÉCHEC : $($_.Exception.Message)" -ForegroundColor Red }
}

Write-Host "`n===== 5. Vérification finale =====" -ForegroundColor Cyan
Write-Host "   Reste dans C:\Program Files\MySQL :"
Get-ChildItem 'C:\Program Files\MySQL' -ErrorAction SilentlyContinue | Select-Object -ExpandProperty Name | ForEach-Object { Write-Host "     - $_" }
Write-Host "   Reste dans C:\ProgramData\MySQL :"
Get-ChildItem 'C:\ProgramData\MySQL' -ErrorAction SilentlyContinue | Select-Object -ExpandProperty Name | ForEach-Object { Write-Host "     - $_" }
$still = Get-CimInstance Win32_Service | Where-Object { $_.PathName -like '*mysqld.exe*' } | Select-Object -First 1
Write-Host "   Service mysqld restant : $(if ($still) { $still.Name } else { 'aucun' })"
