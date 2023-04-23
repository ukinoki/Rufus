Add-Type -AssemblyName System.Windows.Forms
Import-Module ".\MenuShell.dll"


 ##     ## ######## #### ##       #### ######## #### ########  ######  
 ##     ##    ##     ##  ##        ##     ##     ##  ##       ##    ## 
 ##     ##    ##     ##  ##        ##     ##     ##  ##       ##       
 ##     ##    ##     ##  ##        ##     ##     ##  ######    ######  
 ##     ##    ##     ##  ##        ##     ##     ##  ##             ## 
 ##     ##    ##     ##  ##        ##     ##     ##  ##       ##    ## 
  #######     ##    #### ######## ####    ##    #### ########  ######  


function TargetIsOlderOrNothing {
    Param( $source="", $target="", $info=$null)
	$result=$true
	# Exist
	if(Test-Path -Path $target) {
	    $fileSource=Get-ChildItem -Path $source
		$fileTarget=Get-ChildItem -Path $target
        # Is newer
  	    if( $fileSource.LastWriteTime -le $fileTarget.LastWriteTime ) {
		    $result=$false
        }
    }
	return $result
}

function CopyFile {
	Param( $source="", $target="", $info = $null)
    if ( TargetIsOlderOrNothing -source $source -target $target ) {
        Copy-Item $source $target
    }
}


function CreateDirectory {
    Param ($strPath = "")
    If ((Test-Path -Path $strPath -IsValid) -And -Not (Test-Path -Path $strPath)) {
        New-Item -type Directory -Path $strPath -Force > $null
    }
}

Function OpenFile
{
    Param($folder='C:\', $filename="", $filter='All (*.*)| *.*', $title = 'Open File')
	
#    [System.Reflection.Assembly]::LoadWithPartialName("System.windows.forms") | Out-Null

    $OpenFileDialog = New-Object System.Windows.Forms.OpenFileDialog
    $OpenFileDialog.initialDirectory = $folder
    $OpenFileDialog.filter = $filter
    $OpenFileDialog.Title = $title
    $OpenFileDialog.Filename = $filename
	
    $OpenFileDialog.ShowDialog() | Out-Null
    return $OpenFileDialog.filename
} 


Function GetDirectory {
    Param($initialDirectory = "", $title = "Select folder", $description = "Select a folder")
	
    $result = $initialDirectory
    $folderDialog = New-Object System.Windows.Forms.FolderBrowserDialog
    $folderDialog.Description = $description
    #$folderDialog.Title = $title
    $folderDialog.ShowNewFolderButton = $true
    $folderDialog.rootfolder = "MyComputer"
    $folderDialog.SelectedPath = $initialDirectory

    if ($folderDialog.ShowDialog() -eq "OK") {
        $result = $folderDialog.SelectedPath
    }
    return $result
}

Function GetFolderFromFileName {
    Param($folder = "", $filter = "", $title = "Select folder", $description = "Select a folder")
    $result = $folder
    $fileList = Get-Childitem -Path $folder -Filter $filter -File -Recurse -ErrorAction SilentlyContinue
    if ( $filelist.length -gt 1 ) {
        $options = New-Object System.Collections.Generic.List[System.Management.Automation.Host.ChoiceDescription]
        foreach ( $file in $fileList ) {
            $name = $file.DirectoryName
            $opt = New-Object System.Management.Automation.Host.ChoiceDescription($name)
            $options.Add($opt)
        }
        $option = $host.ui.PromptForChoice($title, $message, $options.ToArray(), 0)
        $result = $options[$option].Label
    }
    elseif ( $filelist.length -eq 1 ) {
        $result = $filelist[0].DirectoryName
    }
    else {   
        $result = GetDirectory -initialDirectory  $DeployDir -title $title -description $message
    }
    return $result
}


  ######   ##     ## #### 
 ##    ##  ##     ##  ##  
 ##        ##     ##  ##  
 ##   #### ##     ##  ##  
 ##    ##  ##     ##  ##  
 ##    ##  ##     ##  ##  
  ######    #######  #### 

function GetStateWindow {
    Param($title = "")
    [MenuShell.WindowThread]$wt = New-Object MenuShell.WindowThread("MaterialStatusForm",$title)
    [MenuShell.MaterialStatusForm]$info=$wt.CreateWindow()
    $info.Refresh()
    return $info
}

function WriteStatusFormInfo {
    Param([MenuShell.MaterialStatusForm]$info, $text = "")
    if ($text -ne "" -And $info) {$info.AddDateLineStdOut($text) }
   }
   
   function WriteStatusFormError {
    Param([MenuShell.MaterialStatusForm]$info, $text = "")
    if ($text -ne "" -And $info) {$info.AddDateLineStdErr($text) }
   }

function  ShowEndMsg {
    Param([MenuShell.MaterialStatusForm]$info)
    if ( $info.Abort ) {
        WriteStatusFormError $info "**********************************************************"
        WriteStatusFormError $info "            ABORTED  CLOSE THIS WINDOW"
        WriteStatusFormError $info "**********************************************************"
        $info.Message = "   ******* ABORTED ********"
    } else {
        WriteStatusFormInfo $info "**********************************************************"
        WriteStatusFormInfo $info "            END  CLOSE THIS WINDOW"
        WriteStatusFormInfo $info "**********************************************************"
        $info.Message = "   ******* END ********"
    }
    $info.DisableAbort()
    $info.CanClose = $true
    
    #Write-Output "Waiting for closing window"
    $info.Refresh()
    $info.WaitForWindow()
}

Function ShowException {
    Param( $info = $null , $obj )
    $text = $obj.Exception.Message + "`r`n" + $obj.Exception.ItemName + "`r`n" + $obj.InvocationInfo.PositionMessage

    if( $null -eq $info ) {
        MsgBox  -Text $text -Title "ERROR"
    } else {
        WriteStatusFormError $info "$text"
        $info.AbortDisableAbort = $true
        $info.DesActivaAbortar()
        $info.CanClose = $true
    }
}


##     ##  ######   ######   ########   #######  ##     ## 
###   ### ##    ## ##    ##  ##     ## ##     ##  ##   ##  
#### #### ##       ##        ##     ## ##     ##   ## ##   
## ### ##  ######  ##   #### ########  ##     ##    ###    
##     ##       ## ##    ##  ##     ## ##     ##   ## ##   
##     ## ##    ## ##    ##  ##     ## ##     ##  ##   ##  
##     ##  ######   ######   ########   #######  ##     ## 

# OK 0
# OKCancel 1
# AbortRetryIgnore 2
# YesNoCancel 3
# YesNo 4
# RetryCancel 5

# None 0
# Hand 16
# Error 16
# Stop 16
# Question 32
# Exclamation 48
# Warning 48
# Asterisk 64
# Information 64



function MsgBoxWindows {
    Param($Text='MessageBox', $Title='Title', $Button ='OK', $Icon = 'Error')
    $texto =  $Text
    $titulo =  $Title
    [System.Windows.MessageBox]::Show($texto, $titulo, $Button, $Icon)
}

function MsgBox {
    Param($Text='MessageBox', $Title='Title', $Button ='OK', $Icon = 'Error', $Lang = 'en')
    $texto =  $Text
    $titulo =  $Title
    [MenuShell.MaterialMessageBox]::Show($texto, $titulo, $Button, $Icon, $Lang)
}

function MsgBoxInfo {
    Param($Text='MessageBox', $Title='Information', $Button ='OK', $Icon = 'Information', $Lang = 'en')
    $result = MsgBox $Text $Title $Button $Icon $Lang
}

function MsgBoxWarning {
    Param($Text='MessageBox', $Title='Warning', $Button ='OK', $Icon = 'Exclamation', $Lang = 'en')
    $result = MsgBox $Text $Title $Button $Icon $Lang
}

function MsgBoxYesNo {
    Param($Text='MessageBox', $Title='Title', $Button ='YesNo', $Icon = 'Question', $Lang = 'en')
    MsgBox $Text $Title $Button $Icon $Lang
}

function MsgBoxYesNoCancel {
    Param($Text='MessageBox', $Title='Title', $Button ='YesNoCancel', $Icon = 'Question', $Lang = 'en')
    MsgBox $Text $Title $Button $Icon $Lang
}

function MsgBoxAbortRetryIgnore {
    Param($Text='MessageBox', $Title='Title', $Button ='AbortRetryIgnore', $Icon = 'Exclamation', $Lang = 'en')
    MsgBox $Text $Title $Button $Icon $Lang
}


 ########  ##     ## ######## ##     ##  ######  
 ##     ## ##     ## ##       ##     ## ##    ## 
 ##     ## ##     ## ##       ##     ## ##       
 ########  ##     ## ######   ##     ##  ######  
 ##   ##   ##     ## ##       ##     ##       ## 
 ##    ##  ##     ## ##       ##     ## ##    ## 
 ##     ##  #######  ##        #######   ######  

Function TestEnviron {
    $QTDirectory = "C:\qt"
    $BuildDir = "..\..\build"
    $EnvironPS1 = $BuildDir + "\Environ.ps1"

    If ( -Not (Test-Path -Path $EnvironPS1) ) {

        $DeployDir = $BuildDir + "\Deploy"
        $MicrosoftDir = $BuildDir + "\Microsoft"
        $OutputDir = $BuildDir + "\Output"
        $WinDeployQtDir = "C:\Qt\6.4.0\msvc2019_64\bin"

        # Create if not exist
        CreateDirectory $DeployDir
        CreateDirectory $MicrosoftDir
        CreateDirectory $OutputDir

        # find Rufus.exe
        $title = "Select Deploy folder"
        $description = "Select folder containing Release version of Rufus.exe"
        $filter = "Rufus.exe"
        $folder = "..\..\.."
        $DeployDir = GetFolderFromFileName -folder $folder -filter $filter -title $title -description $description

        # find Windeployqt.exe
        $title = "Select windeployqt folder"
        $description = "Select folder containing Qt windeployqt.exe"
        $filter = "windeployqt.exe"
        $folder = $QTDirectory
        $WinDeployQtDir = GetFolderFromFileName -folder $folder -filter $filter -title $title -description $description

        # find vc_redist.x64.exe
        $title = "Select windeployqt folder"
        $description = "Select folder containing vc_redist.x64.exe"
        $filter = "vc_redist.x64.exe"
        $folder = $MicrosoftDir
        $MicrosoftDir = GetFolderFromFileName -folder $folder -filter $filter -title $title -description $description

        $line = "# Change your vars if necessary"
        Write-Output $line | Out-File -FilePath $EnvironPS1
        $line = '$DeployDir="' + $DeployDir + '"'
        Write-Output $line | Out-File -FilePath $EnvironPS1 -Append
        $line = '$MicrosoftDir="' + $MicrosoftDir + '"'
        Write-Output $line | Out-File -FilePath $EnvironPS1 -Append
        $line = '$OutputDir="' + $OutputDir + '"'
        Write-Output $line | Out-File -FilePath $EnvironPS1 -Append
        $line = '$WinDeployQtDir="' + $WinDeployQtDir + '"'
        Write-Output $line | Out-File -FilePath $EnvironPS1 -Append
    }

    . $EnvironPS1


    # Download vc_redist.x64.exe
    $fileRedist = $MicrosoftDir + "\vc_redist.x64.exe"
    If ( -Not (Test-Path $fileRedist)) {
        $uri = "https://aka.ms/vs/17/release/vc_redist.x64.exe"
        $GetResponse = Invoke-WebRequest -Uri $uri -OutFile $fileRedist
    }
}



Function CheckRelease {
    $title = "Release"
    $description = "Select Release version of Rufus.exe"
    $filter = "Rufus|Rufus.exe"
    $folder = "..\..\.."
    if ( $global:RufusExe.length -gt 0 ) {
        $folder = [io.path]::GetDirectoryName($global:RufusExe)
    } 
    $global:RufusExe = OpenFile -folder $folder -filter $filter -title $title -description $description
    ShowEnviron
}

Function CheckMySQL {
    $title = "MySQL / MariaDB"
    $description = "Select mysql.exe"
    $filter = "MySQL|mysql.exe"
    $folder = "..\..\.."
    if ( $global:MysqlExe.length -gt 0 ) {
        $folder = [io.path]::GetDirectoryName($global:MysqlExe)
    } 
    $global:MysqlExe = OpenFile -folder $folder -filter $filter -title $title -description $description

    $description = "Select mysqldump.exe"
    $filter = "MySQL|mysqldump.exe"
    $folder = "..\..\.."
    if ( $global:MysqlDumpExe.length -gt 0 ) {
        $folder = [io.path]::GetDirectoryName($global:MysqlDumpExe)
    } 
    $global:MysqlDumpExe = OpenFile -folder $folder -filter $filter -title $title -description $description
    
    $description = "Select mysql driver DLL"
    $filter = "MariaDB|libmariadb*.dll|MySQL|libmysql*.dll|Other|*.dll"
    $folder = "..\..\.."
    if ( $global:MysqlDll.length -gt 0 ) {
        $folder = [io.path]::GetDirectoryName($global:MysqlDll)
    } 
    $global:MysqlDll = OpenFile -folder $folder -filter $filter -title $title -description $description
    
    ShowEnviron
}


Function CheckWindeployqt {
    $title = "WindeployQt"
    $description = "Select WindeployQt.exe"
    $filter = "WindeployQt|WindeployQt.exe"
    $folder = $global:QTDirectory
    if ( $global:WinDeployQtExe.length -gt 0 ) {
        $folder = [io.path]::GetDirectoryName($global:WinDeployQtExe)
    } 
    $global:WinDeployQtExe = OpenFile -folder $folder -filter $filter -title $title -description $description
    ShowEnviron
}

Function CheckInnoSetup {
    $title = "Inno Setup Compiler"
    $description = "Select ISCC.exe"
    $filter = "ISCC|iscc.exe"
    $folder = "$Env:ProgramFiles(x86)"
    if ( $global:ISCCExe.length -gt 0 ) {
        $folder = [io.path]::GetDirectoryName($ISCCExe)
    } 
    $global:ISCCExe = OpenFile -folder $folder -filter $filter -title $title -description $description
    ShowEnviron
}


function SaveEnviron {
    $line = "# Change your vars if necessary (at your own risk)"
    Write-Output $line | Out-File -FilePath $global:EnvironPS1

    $lines = New-Object System.Collections.Generic.List[System.String]
    $lines.Add('$global:DeployDir="' + $global:DeployDir + '"')
    $lines.Add('$global:MicrosoftDir="' + $global:MicrosoftDir + '"')
    $lines.Add('$global:OutputDir="' + $global:OutputDir + '"')
    $lines.Add('$global:RufusExe="' + $global:RufusExe + '"')
    $lines.Add('$global:WinDeployQtExe="' + $global:WinDeployQtExe + '"')
    $lines.Add('$global:ISCCExe="' + $global:ISCCExe + '"')
    $lines.Add('$global:MysqlExe="' + $global:MysqlExe + '"')
    $lines.Add('$global:MysqlDumpExe="' + $global:MysqlDumpExe + '"')
    $lines.Add('$global:MysqlDll="' + $global:MysqlDll + '"')
    foreach( $line in $lines ) {
        Write-Output $line | Out-File -FilePath $EnvironPS1 -Append
    }
    MsgBoxInfo $global:EnvironPS1
}

function ShowEnviron {
    $lines = ""
    $lines += '$global:DeployDir="' + $global:DeployDir + '"' + $global:crlf
    $lines += '$global:MicrosoftDir="' + $global:MicrosoftDir + '"' + $global:crlf
    $lines += '$global:OutputDir="' + $global:OutputDir + '"' + $global:crlf
    $lines += '$global:RufusExe="' + $global:RufusExe + '"' + $global:crlf
    $lines += '$global:WinDeployQtExe="' + $global:WinDeployQtExe + '"' + $global:crlf
    $lines += '$global:ISCCExe="' + $global:ISCCExe + '"' + $global:crlf
    $lines += '$global:MysqlExe="' + $global:MysqlExe + '"' + $global:crlf
    $lines += '$global:MysqlDumpExe="' + $global:MysqlDumpExe + '"' + $global:crlf
    $lines += '$global:MysqlDll="' + $global:MysqlDll + '"' + $global:crlf
    MsgBoxInfo $lines
}


  ######   ######## ##    ## ######## ########     ###    ######## ######## 
 ##    ##  ##       ###   ## ##       ##     ##   ## ##      ##    ##       
 ##        ##       ####  ## ##       ##     ##  ##   ##     ##    ##       
 ##   #### ######   ## ## ## ######   ########  ##     ##    ##    ######   
 ##    ##  ##       ##  #### ##       ##   ##   #########    ##    ##       
 ##    ##  ##       ##   ### ##       ##    ##  ##     ##    ##    ##       
  ######   ######## ##    ## ######## ##     ## ##     ##    ##    ######## 

Function GenerateRufusSetup {
    Param($windowTitle = "")
    $info = GetStateWindow $windowTitle
    $info.Refresh()
    try {
        # Copy Rufus.exe
        $RufusDeploy="$global:DeployDir\Rufus.exe"
        WriteStatusFormInfo $info "Copying $RufusExe"
        CopyFile -source $global:RufusExe -target $RufusDeploy
        WriteStatusFormInfo $info "Done"

        # Run windeployqt
        $info.exeWorkingDirectory = $global:BuildDir
        $Parms = "Deploy\Rufus.exe"
        $prg = $global:WinDeployQtExe
        WriteStatusFormInfo $info "Running $prg $Parms"
        $info.Execute($prg, $Parms)
        WriteStatusFormInfo $info "Done"

        # Check vc_redist.x64.exe (if copied by windeployqt)
        $fileRedistDeployed = $global:DeployDir + "\vc_redist.x64.exe"
        $fileRedist = $global:MicrosoftDir + "\vc_redist.x64.exe"
        If ( Test-Path $fileRedistDeployed) {
            Move-Item -Path $fileRedistDeployed -Destination $fileRedist
        }
        If ( -Not (Test-Path $fileRedist)) {
            WriteStatusFormInfo $info "$fileRedist doesn't exist. Downloading..."
            $uri = "https://aka.ms/vs/17/release/vc_redist.x64.exe"
            $GetResponse = Invoke-WebRequest -Uri $uri -OutFile $fileRedist
            WriteStatusFormInfo $info "Done"
        }

        # Copy folders
        $list=@("assets")
        foreach( $item in $list) {
            $folderName= $item
            If ( Test-Path "$global:ProjectDir\$folderName") {
                CreateDirectory "$global:DeployDir\$folderName"
                Copy-Item -Path "$global:ProjectDir\$folderName" -Destination "$global:DeployDir\$folderName" -Recurse -Force
            }
        }

        # Copy mysql.dll or mariadb.dll
        $list=@($global:MysqlDll)
        foreach( $item in $list) {
            $fileApp= $item
            If ( Test-Path "$fileApp") {
                Copy-Item -Path "$fileApp" -Destination "$global:DeployDir" -Recurse -Force
            }
        }

        # Copy mysql.exe and mysqldump.exe
        CreateDirectory "$global:DeployDir\Applications"
        $list=@($global:MysqlExe,$global:MysqlDumpExe)
        foreach( $item in $list) {
            $fileApp= $item
            If ( Test-Path "$fileApp") {
                Copy-Item -Path "$fileApp" -Destination "$global:DeployDir\Applications" -Recurse -Force
            }
        }

        # Run InnoSetup
        CopyFile -source "RufusVision.iss" -target "$global:BuildDir\RufusVision.iss"
        $Parms = "RufusVision.iss"
        $prg=$global:ISCCExe
        WriteStatusFormInfo $info "Running $prg $Parms"
        $info.Execute($prg, $Parms)
        WriteStatusFormInfo $info "Done"
    }
    catch {
        ShowException -info $info -obj $_
    }
    ShowEndMsg $info
}

  ######  ########    ###    ########  ########    
 ##    ##    ##      ## ##   ##     ##    ##       
 ##          ##     ##   ##  ##     ##    ##       
  ######     ##    ##     ## ########     ##       
       ##    ##    ######### ##   ##      ##       
 ##    ##    ##    ##     ## ##    ##     ##       
  ######     ##    ##     ## ##     ##    ##       


#Set conveniences
$global:crlf="`r`n"

#Set default variables
$global:QTDirectory = "C:\qt"
$global:ProjectDir = "..\.."
$global:BuildDir = "..\..\build"
$global:EnvironPS1 = $BuildDir + "\Environ.ps1"

$global:DeployDir = $BuildDir + "\Deploy"
$global:MicrosoftDir = $BuildDir + "\Microsoft"
$global:OutputDir = $BuildDir + "\Output"

# Create default folders if not exist
CreateDirectory $global:DeployDir
CreateDirectory $global:MicrosoftDir
CreateDirectory $global:OutputDir


$global:RufusExe = ""
$global:WinDeployQtExe = ""
$global:ISCCExe = ""
$global:MysqlExe = ""
$global:MysqlDumpExe = ""
$global:MysqlDll = ""


If ( Test-Path -Path $EnvironPS1 ) {
    . $global:EnvironPS1
}


 ##     ## ######## ##    ## ##     ## 
 ###   ### ##       ###   ## ##     ## 
 #### #### ##       ####  ## ##     ## 
 ## ### ## ######   ## ## ## ##     ## 
 ##     ## ##       ##  #### ##     ## 
 ##     ## ##       ##   ### ##     ## 
 ##     ## ######## ##    ##  #######  


$list = New-Object "System.Collections.Generic.List[MenuShell.MenuOption]"

$opt = New-Object MenuShell.MenuOption("1", "Set Release")
$list.add($opt)
$opt = New-Object MenuShell.MenuOption("2", "MySQL/MariaDB")
$list.add($opt)
$opt = New-Object MenuShell.MenuOption("3", "Set WindeployQt")
$list.add($opt)
$opt = New-Object MenuShell.MenuOption("4", "Set Inno Setup")
$list.add($opt)
$opt = New-Object MenuShell.MenuOption("7", "Show Environ")
$list.add($opt)
$opt = New-Object MenuShell.MenuOption("8", "Save Environ")
$list.add($opt)
$opt = New-Object MenuShell.MenuOption("9", "Generate Rufus setup")
$list.add($opt)

$title = "Rufus build utility"
$exit=0
while ( $exit -eq 0 ) {
    $menu = New-Object MenuShell.MaterialListBoxMenu($title, "Select option", $list, 150)
    $dialogResult = $menu.ShowDialog()
    if ($dialogResult -eq "OK") {
        $result = $menu.OptionSelected
        $OPC = $result
        try {
            switch ($result) {
                "1" { CheckRelease }
                "2" { CheckMySQL }
                "3" { CheckWindeployqt }
                "4" { CheckInnoSetup }
                "7" { ShowEnviron }
                "8" { SaveEnviron }
                "9" { GenerateRufusSetup $title }
            }
        }
        catch {
            ShowException -info $null -obj $_
        }
    }
    else {
        $exit = 1
    }
}

