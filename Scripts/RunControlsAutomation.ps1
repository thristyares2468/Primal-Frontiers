# Isolated rendered controls smoke; human playtests and physical hardware remain separate.
[CmdletBinding()]
param([ValidateSet('720','1440')][string]$Resolution='720',
      [ValidateSet('Controls','Inventory','Survival','Overlays','Reconnect','Settings','SettingsApply','Display','WorldMenu','ToolProgression')][string]$TestCase='Controls',
      [ValidateRange(0.75,1.5)][float]$HUDScale=1,
      [switch]$LowShadowDiagnostic,
      [ValidateRange(60,600)][int]$TimeoutSeconds=180)
Set-StrictMode -Version Latest
$ErrorActionPreference='Stop'
$projectRoot=Split-Path -Parent $PSScriptRoot
$exe='C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe'
if(Get-Process UnrealEditor* -ErrorAction SilentlyContinue){throw 'Close the existing Unreal process before this isolated UI test.'}
if(!(Test-Path -LiteralPath $exe)){throw 'Unreal Editor executable unavailable.'}
function Get-PersonalSettingsSnapshot {
    $configRoot=Join-Path $projectRoot 'Saved\Config'
    if(Test-Path -LiteralPath $configRoot){
        Get-ChildItem -LiteralPath $configRoot -Recurse -File |
            Where-Object {$_.Name -in @('GameUserSettings.ini','Scalability.ini')} |
            Sort-Object FullName | ForEach-Object {$_.FullName+':'+(Get-FileHash -LiteralPath $_.FullName -Algorithm SHA256).Hash}
    }
}
$personalSettingsBefore=@(Get-PersonalSettingsSnapshot)
$run=$(if($TestCase -eq 'ToolProgression'){'M12Tool'}else{'M11'+$TestCase})+$Resolution+'_'+[DateTime]::UtcNow.ToString('yyyyMMdd_HHmmssfff')+'_'+[Guid]::NewGuid().ToString('N').Substring(0,8)
$report=Join-Path $projectRoot ('Saved\AutomationReports\'+$run)
$settingsTarget=Join-Path $report 'GameUserSettings.ini'
if(Test-Path -LiteralPath $report){throw 'Refusing reused UI report/settings destination.'}
$log=Join-Path $projectRoot ('Saved\Logs\'+$run+'.log')
$width=if($Resolution -eq '720'){1280}else{2560}
$height=if($Resolution -eq '720'){720}else{1440}
$filter=switch($TestCase){'Controls'{'PF.UI.ControlsLive'} 'Inventory'{'PF.UI.InventorySelectionLive'} 'Survival'{'PF.UI.SurvivalFeedbackLive'} 'Overlays'{'PF.UI.ActionOverlaysLive'} 'Reconnect'{'PF.UI.ReconnectProfileLive'} 'Settings'{'PF.UI.SettingsCancelLive'} 'SettingsApply'{'PF.UI.SettingsApplyLive'} 'Display'{'PF.UI.SettingsDisplayLive'} 'WorldMenu'{'PF.UI.WorldMenuLive'} 'ToolProgression'{'PF.Crafting.ToolProgressionLive'}}
# Every game login saves a server-issued credential, including ordinary UI fixtures.
# Always isolate the profile; never replace the user's default Local reconnect details.
$ownedProfile='UI'+[Guid]::NewGuid().ToString('N').Substring(0,12)
$profileArgument=' -PFIdentityProfile='+$ownedProfile
$ownedWorld='UIWorld_'+[Guid]::NewGuid().ToString('N')
$ownedNames='Metadata_UIWorld_'+[Guid]::NewGuid().ToString('N')
$persistenceRoot=Join-Path $projectRoot 'Saved\Persistence'
function Get-PersonalWorldSnapshot {
    if(Test-Path -LiteralPath $persistenceRoot){
        Get-ChildItem -LiteralPath $persistenceRoot -File -Filter '*.pfs' |
            Where-Object {$_.Name -notin @(($ownedWorld+'.a.pfs'),($ownedWorld+'.b.pfs'),($ownedNames+'.a.pfs'),($ownedNames+'.b.pfs')) -and !$_.Name.StartsWith('Identity_'+$ownedProfile+'_')} |
            Sort-Object FullName | ForEach-Object {$_.FullName+':'+(Get-FileHash -LiteralPath $_.FullName -Algorithm SHA256).Hash}
    }
}
$personalWorldsBefore=@(Get-PersonalWorldSnapshot)
foreach($taskSlot in @($ownedWorld,$ownedNames)){foreach($generation in @('a','b')){if(Test-Path -LiteralPath (Join-Path $persistenceRoot ($taskSlot+'.'+$generation+'.pfs'))){throw 'Refusing reused menu test slot.'}}}
# Explicit optional renderer diagnostic: halve directional shadow resolution,
# keep the existing page budget, do not persist settings or mute warnings.
$shadowCommands=if($LowShadowDiagnostic){'r.Shadow.Virtual.ResolutionLodBiasDirectional 1,r.Shadow.Virtual.ResolutionLodBiasDirectionalMoving 1,'}else{''}
# Apply must own r.VSync through GameUserSettings. A SetByConsole override would
# prevent that supported path; the fixture checks the actual off/unlimited state.
$writesSettings=$TestCase -in @('SettingsApply','Display')
$vsyncCommand=if($writesSettings -or $TestCase -eq 'WorldMenu'){''}else{'r.VSync 0,'}
$arguments='"{0}" /Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld -game -windowed -ForceRes -ResX={1} -ResY={2} -unattended -nosplash -nosound -NoLiveCoding -NoSaveConfig -PFRunControlsUITest -PFControlsEvidence={3} -PFHUDScale={7} -ExecCmds="t.MaxFPS 0,{9}{8}Automation RunTests {6}" -TestExit="Automation Test Queue Empty" -ReportExportPath="{4}" -abslog="{5}"' -f (Join-Path $projectRoot 'PrimalFrontier.uproject'),$width,$height,$run,$report,$log,$filter,([string]$HUDScale),$shadowCommands,$vsyncCommand
$arguments+=$profileArgument
if($TestCase -eq 'ToolProgression'){$arguments+=' -PFRunToolProgressionTests -PFExpectedPlayers=1'}
if($TestCase -eq 'WorldMenu'){
    $arguments=$arguments.Replace(' /Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld',' /Engine/Maps/Entry')
    $arguments+=' -PFRunWorldMenuTest -PFWorldMenuTestSlot='+$ownedWorld+' -PFWorldNamesTestSlot='+$ownedNames
}
# Supported FConfigCacheIni::GetDestIniFilename override: only this run owns the destination.
$arguments+=' -GameUserSettingsINI="'+$settingsTarget+'"'
# Only guarded settings fixtures deliberately save to their disposable destination.
if($writesSettings){
    $arguments=$arguments.Replace(' -NoSaveConfig','')
    $arguments+=if($TestCase -eq 'SettingsApply'){' -PFRunSettingsApplyTest'}else{' -PFRunSettingsDisplayTest'}
}
$process=$null;$working=0L;$private=0L;$timedOut=$false;$watch=[Diagnostics.Stopwatch]::StartNew()
try {
    $process=Start-Process -FilePath $exe -ArgumentList $arguments -WorkingDirectory $projectRoot -WindowStyle Hidden -PassThru
    while(!$process.HasExited){
        $process.Refresh()
        if(!$process.HasExited){$working=[Math]::Max($working,$process.WorkingSet64);$private=[Math]::Max($private,$process.PrivateMemorySize64)}
        if($watch.Elapsed.TotalSeconds -ge $TimeoutSeconds){$timedOut=$true;Stop-Process -Id $process.Id;break}
        Start-Sleep -Milliseconds 500
    }
    $process.WaitForExit()
    $code=if($timedOut){-1}else{$process.ExitCode}
    $inspection=& (Join-Path $PSScriptRoot 'RunNativeAutomation.ps1') -ExistingReport (Join-Path $report 'index.json') -RecordedProcessExitCode $code
    $verdictCode=$LASTEXITCODE
    $personalSettingsAfter=@(Get-PersonalSettingsSnapshot)
    $personalSettingsUnchanged=($personalSettingsBefore -join "`n") -ceq ($personalSettingsAfter -join "`n")
    $personalWorldsAfter=@(Get-PersonalWorldSnapshot)
    $personalWorldsUnchanged=($personalWorldsBefore -join "`n") -ceq ($personalWorldsAfter -join "`n")
    $runnerCode=if($personalSettingsUnchanged -and $personalWorldsUnchanged){$verdictCode}else{1}
    $diagnostic=[ordered]@{Run=$run;Resolution="$width x $height";HUDScale=$HUDScale;LowShadowDiagnostic=[bool]$LowShadowDiagnostic;Rendered=$true;RequestedUncapped=$true;TimedOut=$timedOut;EngineExitCode=$code;VerdictExitCode=$verdictCode;RunnerExitCode=$runnerCode;SettingsTarget=$settingsTarget;DefaultConfigFileCount=$personalSettingsBefore.Count;DefaultConfigsUnchanged=$personalSettingsUnchanged;Seconds=[Math]::Round($watch.Elapsed.TotalSeconds,2);SampledPeakWorkingGiB=[Math]::Round($working/1GB,3);SampledPeakPrivateGiB=[Math]::Round($private/1GB,3);Report=$report;Log=$log;Screenshots=(Join-Path $projectRoot ('Saved\AutomationReports\ControlsUI\'+$run));Verdict=($inspection -join "`n" | ConvertFrom-Json)}
    # The runner owns its generated diagnostics; retain failures rather than overwrite artifacts.
    $diagnostic.PersonalSaveFileCount=$personalWorldsBefore.Count
    $diagnostic.PersonalSavesUnchanged=$personalWorldsUnchanged
    New-Item -ItemType Directory -Path $report -Force | Out-Null
    $diagnostic | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath (Join-Path $report 'run-summary.json') -Encoding UTF8
    $diagnostic | ConvertTo-Json -Depth 8
    exit $runnerCode
} finally {
    if($null -ne $process -and !$process.HasExited){Stop-Process -Id $process.Id -ErrorAction SilentlyContinue}
}
