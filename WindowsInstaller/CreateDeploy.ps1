function CreateDirectory {
    Param ($strPath)
	If ((Test-Path -Path $strPath -IsValid) -and !(Test-Path -Path $strPath)){
        New-Item -type Directory -Path $strPath -Force > $null
	}
}

$QTDirectory ="C:\qt"

CreateDirectory "Deploy"
CreateDirectory "Microsoft"
CreateDirectory "Output"

# Download vc_redist.x64.exe
$fileRedist = ".\Microsoft\vc_redist.x64.exe"
If (! Test-Path $fileRedist){
    $GetResponse=Invoke-WebRequest -Uri "https://aka.ms/vs/17/release/vc_redist.x64.exe" -OutFile $fileRedist
}
# find Rufus.exe
$fileList = Get-Childitem –Path ".." -Include Rufus.exe -File -Recurse -ErrorAction SilentlyContinue
foreach( $file in $fileList ) {

}

# find Windeployqt.exe
$fileList = Get-Childitem –Path $QTDirectory -Include windeployqt.exe -File -Recurse -ErrorAction SilentlyContinue

