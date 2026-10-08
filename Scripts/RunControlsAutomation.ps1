# Isolated rendered controls smoke; human playtests and physical hardware remain separate.
[CmdletBinding()]
param([ValidateSet('720','1440')][string]$Resolution='720',
      [ValidateSet('Controls','Inventory','Survival','Overlays','Reconnect')][string]$TestCase='Controls',
      [ValidateRange(0.75,1.5)][float]$HUDScale=1,
      [switch]$LowShadowDiagnostic,
      [ValidateRange(60,600)][int]$TimeoutSeconds=180)
Set-StrictMode -Version Latest
$ErrorActionPreference='Stop'
$projectRoot=Split-Path -Parent $PSScriptRoot
$exe='C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe'
if(Get-Process UnrealEditor* -ErrorAction SilentlyContinue){throw 'Close the existing Unreal process before this isolated UI test.'}
if(!(Test-Path -LiteralPath $exe)){throw 'Unreal Editor executable unavailable.'}
$run='M11'+$TestCase+$Resolution+'_'+[DateTime]::UtcNow.ToString('yyyyMMdd_HHmmssfff')+'_'+[Guid]::NewGuid().ToString('N').Substring(0,8)
$report=Join-Path $projectRoot ('Saved\AutomationReports\'+$run)
$log=Join-Path $projectRoot ('Saved\Logs\'+$run+'.log')
$width=if($Resolution -eq '720'){1280}else{2560}
$height=if($Resolution -eq '720'){720}else{1440}
$filter=switch($TestCase){'Controls'{'PF.UI.ControlsLive'} 'Inventory'{'PF.UI.InventorySelectionLive'} 'Survival'{'PF.UI.SurvivalFeedbackLive'} 'Overlays'{'PF.UI.ActionOverlaysLive'} 'Reconnect'{'PF.UI.ReconnectProfileLive'}}
# Only the credential fixture writes an identity profile; isolate it from the user's Local profile.
$profileArgument=if($TestCase -eq 'Reconnect'){' -PFIdentityProfile=UI'+[Guid]::NewGuid().ToString('N').Substring(0,12)}else{''}
# Explicit optional renderer diagnostic: halve directional shadow resolution,
# keep the existing page budget, do not persist settings or mute warnings.
$shadowCommands=if($LowShadowDiagnostic){'r.Shadow.Virtual.ResolutionLodBiasDirectional 1,r.Shadow.Virtual.ResolutionLodBiasDirectionalMoving 1,'}else{''}
$arguments='"{0}" /Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld -game -windowed -ForceRes -ResX={1} -ResY={2} -unattended -nosplash -nosound -NoLiveCoding -NoSaveConfig -PFRunControlsUITest -PFControlsEvidence={3} -PFHUDScale={7} -ExecCmds="t.MaxFPS 0,r.VSync 0,{8}Automation RunTests {6}" -TestExit="Automation Test Queue Empty" -ReportExportPath="{4}" -abslog="{5}"' -f (Join-Path $projectRoot 'PrimalFrontier.uproject'),$width,$height,$run,$report,$log,$filter,([string]$HUDScale),$shadowCommands
$arguments+=$profileArgument
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
    $diagnostic=[ordered]@{Run=$run;Resolution="$width x $height";HUDScale=$HUDScale;LowShadowDiagnostic=[bool]$LowShadowDiagnostic;Rendered=$true;RequestedUncapped=$true;TimedOut=$timedOut;EngineExitCode=$code;VerdictExitCode=$verdictCode;Seconds=[Math]::Round($watch.Elapsed.TotalSeconds,2);SampledPeakWorkingGiB=[Math]::Round($working/1GB,3);SampledPeakPrivateGiB=[Math]::Round($private/1GB,3);Report=$report;Log=$log;Screenshots=(Join-Path $projectRoot ('Saved\AutomationReports\ControlsUI\'+$run));Verdict=($inspection -join "`n" | ConvertFrom-Json)}
    # The runner owns its generated diagnostics; retain failures rather than overwrite artifacts.
    New-Item -ItemType Directory -Path $report -Force | Out-Null
    $diagnostic | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath (Join-Path $report 'run-summary.json') -Encoding UTF8
    $diagnostic | ConvertTo-Json -Depth 8
    exit $verdictCode
} finally {
    if($null -ne $process -and !$process.HasExited){Stop-Process -Id $process.Id -ErrorAction SilentlyContinue}
}
