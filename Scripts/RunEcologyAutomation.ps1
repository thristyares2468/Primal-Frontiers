# Isolated M13 ecology / existing M6 navigation regression. No personal slots or rendered clients.
[CmdletBinding()]
param(
    [ValidateSet(1,2)][int]$Players=1,
    [ValidateSet('Ecology','Lifecycle','Building','Traversal','Variety','InventoryRace','InventoryOwnership','AutoClimb','AuthoredClimb')][string]$TestCase='Ecology',
    [switch]$SimulateLagLoss,
    [switch]$ManualClimb,
    [switch]$FullHeightClimb,
    [ValidateRange(1024,65535)][int]$Port=17993,
    [ValidateRange(60,300)][int]$TimeoutSeconds=150,
    [string]$EngineRoot='C:\Program Files\Epic Games\UE_5.8'
)
Set-StrictMode -Version Latest
$ErrorActionPreference='Stop'
$root=Split-Path -Parent $PSScriptRoot
$editor=Join-Path $EngineRoot 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe'
if(!(Test-Path -LiteralPath $editor)){throw 'Editor executable unavailable'}
if(Get-Process UnrealEditor,UnrealEditor-Cmd -ErrorAction SilentlyContinue){throw 'Close the existing Unreal process before isolated ecology automation'}
if(Get-NetUDPEndpoint -LocalPort $Port -ErrorAction SilentlyContinue){throw 'Diagnostic port already occupied'}
foreach($path in @($root,$EngineRoot)){if($path -match '["\x00-\x1f]'){throw 'Unsupported path quote/control character'}}
if($FullHeightClimb -and $TestCase -ne 'AuthoredClimb'){throw 'FullHeightClimb requires authored climbing'}
if($ManualClimb -and $TestCase -ne 'AuthoredClimb'){throw 'ManualClimb requires the authored climb fixture'}
$run=$(if($FullHeightClimb){'M14Height'}elseif($ManualClimb){'M14Manual'}elseif($TestCase -eq 'AuthoredClimb'){'M14'}elseif($TestCase -in @('InventoryRace','InventoryOwnership')){'M15'}elseif($TestCase -eq 'Building'){'M11Building'}elseif($TestCase -in @('Traversal','AutoClimb','AuthoredClimb')){'M1Traverse'}else{'M13Ecology'})+$TestCase+$Players+'_'+[DateTime]::UtcNow.ToString('yyyyMMdd_HHmmssfff')+'_'+[Guid]::NewGuid().ToString('N').Substring(0,8)
$slot='AutomationM13Ecology'+[Guid]::NewGuid().ToString('N')
$profile='Eco'+[Guid]::NewGuid().ToString('N').Substring(0,12)
$reports=Join-Path $root 'Saved\AutomationReports'
$report=Join-Path $reports $run
if(Test-Path -LiteralPath $report){throw 'Refusing reused report destination'}
$persistence=Join-Path $root 'Saved\Persistence'
function Get-PersonalSnapshot {
    if(Test-Path -LiteralPath $persistence){
        Get-ChildItem -LiteralPath $persistence -File -Filter '*.pfs' |
            Where-Object {$_.Name -notin @(($slot+'.a.pfs'),($slot+'.b.pfs')) -and !$_.Name.StartsWith('Identity_'+$profile)} |
            Sort-Object FullName | ForEach-Object {$_.FullName+':'+(Get-FileHash -LiteralPath $_.FullName -Algorithm SHA256).Hash}
    }
}
function Get-SettingsSnapshot {
    $config=Join-Path $root 'Saved\Config'
    if(Test-Path -LiteralPath $config){
        Get-ChildItem -LiteralPath $config -Recurse -File |
            Where-Object {$_.Name -in @('GameUserSettings.ini','Scalability.ini')} |
            Sort-Object FullName | ForEach-Object {$_.FullName+':'+(Get-FileHash -LiteralPath $_.FullName -Algorithm SHA256).Hash}
    }
}
foreach($generation in @('a','b')){if(Test-Path -LiteralPath (Join-Path $persistence ($slot+'.'+$generation+'.pfs'))){throw 'Refusing reused diagnostic slot'}}
$before=@(Get-PersonalSnapshot);$settingsBefore=@(Get-SettingsSnapshot)
$null=New-Item -ItemType Directory -Path $report
$filter=if($TestCase -eq 'Ecology'){'PF.Creatures.EcologyLive'}elseif($TestCase -eq 'Building'){'PF.Building.Live'}elseif($TestCase -in @('Traversal','AutoClimb','AuthoredClimb')){'PF.Traversal.Live'}else{'PF.Creatures.Live'}
if($TestCase -eq 'Variety'){$filter='PF.Creatures.VarietyLive'}
if($TestCase -eq 'InventoryRace'){$filter='PF.Inventory.RaceLive'}
if($TestCase -eq 'InventoryOwnership'){$filter='PF.Inventory.OwnershipLive'}
if($TestCase -eq 'AutoClimb'){$filter='PF.Traversal.AutomaticLive'}
if($TestCase -eq 'AuthoredClimb'){$filter='PF.Traversal.AuthoredLive'}
$flag=if($TestCase -eq 'Ecology'){'-PFRunEcologyLiveTests'}elseif($TestCase -eq 'Building'){'-PFRunBuildingLiveTests'}elseif($TestCase -in @('Traversal','AutoClimb','AuthoredClimb')){'-PFRunTraversalLiveTests'}else{'-PFRunCreatureLiveTests'}
if($TestCase -eq 'Variety'){$flag='-PFRunCreatureVarietyLiveTests'}
if($TestCase -eq 'InventoryRace'){$flag='-PFRunInventoryRaceLiveTests'}
if($TestCase -eq 'InventoryOwnership'){$flag='-PFRunInventoryOwnershipLiveTests'}
if($TestCase -eq 'AutoClimb'){$flag='-PFRunAutomaticClimbLiveTests'}
if($TestCase -eq 'AuthoredClimb'){$flag='-PFRunAuthoredClimbLiveTests'}
if($ManualClimb){$flag+=' -PFManualClimb'}
if($FullHeightClimb){$flag+=' -PFFullHeightClimb'}
$lag=if($SimulateLagLoss){75}else{0};$loss=if($SimulateLagLoss){1}else{0}
$map=if($TestCase -eq 'Ecology'){'/Game/PrimalFrontier/Maps/L_M7SurvivalArena'}elseif($TestCase -eq 'Building'){'/Game/PrimalFrontier/Maps/L_M5Building'}elseif($TestCase -in @('Traversal','AutoClimb','AuthoredClimb')){'/Game/PrimalFrontier/Maps/L_M7SurvivalArena'}else{'/Game/PrimalFrontier/Maps/L_M6Creatures'}
if($TestCase -eq 'AuthoredClimb'){$map='/Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld'}
$summary=[ordered]@{Run=$run;Map=$map;TestCase=$TestCase;ManualClimb=[bool]$ManualClimb;FullHeightClimb=[bool]$FullHeightClimb;Players=$Players;NullRHI=$true;HeadlessFrameRate=$(if($TestCase -in @('Traversal','AutoClimb','AuthoredClimb')){120}else{0});ExpectedFilter=$filter;Slot=$slot;ProfilePrefix=$profile;Port=$Port;OutgoingLagMs=$lag;OutgoingLossPercent=$loss;Passed=$false;TimedOut=$false;Processes=@();CleanupFailures=@();Failure=$null}
$runs=@();$watch=[Diagnostics.Stopwatch]::StartNew();$exitCode=1
function Sample-Memory($taskRun) {
    if(!$taskRun.Process.HasExited){
        $taskRun.Process.Refresh()
        if(!$taskRun.Process.HasExited){$taskRun.Working=[Math]::Max($taskRun.Working,$taskRun.Process.WorkingSet64);$taskRun.Private=[Math]::Max($taskRun.Private,$taskRun.Process.PrivateMemorySize64)}
    }
}
try {
    for($i=0;$i -le $Players;$i++){
        $role=if($i -eq 0){'Server'}else{'Client'+$i}
        $name=$run+$role;$taskReport=Join-Path $reports $name;$log=Join-Path $root ('Saved\Logs\PF'+$name+'.log')
        $null=New-Item -ItemType Directory -Path $taskReport
        $url=if($i -eq 0){$summary.Map}else{'127.0.0.1:'+$Port}
        $mode=if($i -eq 0){'-server -game -port='+$Port}else{'-game'}
        $arguments='"{0}" {1} {2} -nullrhi -unattended -nosplash -nosound -NoLiveCoding -NoSaveConfig -PktLag={3} -PktLoss={4} {5} -PFExpectedPlayers={6} -PFSaveSlot={7} -PFIdentityProfile={8} -ExecCmds="Automation RunTests {9}" -TestExit="Automation Test Queue Empty" -ReportExportPath="{10}" -abslog="{11}"' -f (Join-Path $root 'PrimalFrontier.uproject'),$url,$mode,$lag,$loss,$flag,$Players,$slot,($profile+$i),$filter,$taskReport,$log
        # Headless movement transport diagnostic: prevent unbounded NullRHI loops
        # exhausting saved moves. Rendered FPS tests remain uncapped.
        if($TestCase -in @('Traversal','AutoClimb','AuthoredClimb')){$arguments=$arguments.Replace('Automation RunTests '+$filter,'t.MaxFPS 120,Automation RunTests '+$filter);$arguments+=' -ini:GameUserSettings:[/Script/PrimalFrontier.PFGameUserSettings]:FrameRateLimit=120'}
        # M6 navigation-only regression is deliberately outside the persistence map whitelist.
        if($TestCase -ne 'Ecology'){$arguments=$arguments.Replace(' -PFSaveSlot='+$slot,'')}
        $process=Start-Process -FilePath $editor -ArgumentList $arguments -WorkingDirectory $root -WindowStyle Hidden -PassThru
        $taskRun=[pscustomobject]@{Role=$role;Process=$process;Report=$taskReport;Log=$log;Working=0L;Private=0L};$runs+=$taskRun
        if($i -eq 0){
            while($true){
                Sample-Memory $taskRun
                if($process.HasExited){throw 'Server exited before client launch'}
                if((Test-Path -LiteralPath $log) -and ((Get-Content -LiteralPath $log -Raw) -match ('IpNetDriver listening on port '+$Port+'\b'))){break}
                if($watch.Elapsed.TotalSeconds -ge [Math]::Min(45,$TimeoutSeconds)){$summary.TimedOut=$true;throw 'Server readiness deadline exceeded'}
                Start-Sleep -Milliseconds 500
            }
        }
        elseif($i -lt $Players){
            # Finish the first client's startup before launching the second, avoiding
            # concurrent engine AutoSDK backup rotation in the shared project log folder.
            while($true){
                foreach($ownedRun in $runs){Sample-Memory $ownedRun}
                if($process.HasExited){throw 'Client exited before next client launch'}
                if((Test-Path -LiteralPath $log) -and ((Get-Content -LiteralPath $log -Raw) -match 'UEngine::LoadMap Load map complete')){break}
                if($watch.Elapsed.TotalSeconds -ge $TimeoutSeconds){$summary.TimedOut=$true;throw 'Client readiness deadline exceeded'}
                Start-Sleep -Milliseconds 500
            }
        }
    }
    while(@($runs | Where-Object {!$_.Process.HasExited}).Count){
        foreach($taskRun in $runs){Sample-Memory $taskRun}
        if($watch.Elapsed.TotalSeconds -ge $TimeoutSeconds){$summary.TimedOut=$true;throw 'Owned ecology phase deadline exceeded'}
        Start-Sleep -Milliseconds 500
    }
}
catch {$summary.Failure=$_.Exception.Message}
finally {
    foreach($taskRun in $runs){
        try {
            if(!$taskRun.Process.HasExited){$taskRun.Process.Kill()}
            if(!$taskRun.Process.WaitForExit(5000)){$summary.CleanupFailures+=$taskRun.Role}
        } catch {if(!$taskRun.Process.HasExited){$summary.CleanupFailures+=$taskRun.Role}}
    }
    foreach($taskRun in $runs){
        try {
            if(!$taskRun.Process.HasExited){throw 'Owned process could not be stopped'}
            $inspection=& powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File (Join-Path $PSScriptRoot 'RunNativeAutomation.ps1') -ExistingReport (Join-Path $taskRun.Report 'index.json') -RecordedProcessExitCode $taskRun.Process.ExitCode -ExpectedFilter $filter
            $verdictCode=$LASTEXITCODE;$verdict=($inspection -join "`n") | ConvertFrom-Json
            $text=if(Test-Path -LiteralPath $taskRun.Log){Get-Content -LiteralPath $taskRun.Log -Raw}else{''}
            $fatal=[regex]::Matches($text,'(?im)\bFatal error:|\bEnsure condition failed:|\bAssertion failed:').Count
            $contract=if($TestCase -ne 'Ecology'){$true}elseif($taskRun.Role -eq 'Server'){$text.Contains('Ecology server real navigation, dormant/night/day resident and twice-loaded stable save identity verified')}else{$text.Contains('Ecology client authority, restored resident and day phase verified')}
            if($TestCase -in @('InventoryRace','InventoryOwnership')){
                $contract=if($taskRun.Role -eq 'Server'){$text.Contains('Inventory race server verified all client acknowledgements, single winner, split cooldown, stale IDs, unique batches and eleven-food deadline conservation')}else{$text.Contains('Inventory race client verified owned RPC phases, direct mutation refusal, final private inventory and public drop expiry')}
                if($taskRun.Role -ne 'Server'){
                    $lastPhase=if($TestCase -eq 'InventoryOwnership'){14}else{13}
                    foreach($phase in 9..$lastPhase){$contract=$contract -and $text.Contains('Inventory race client acknowledged phase='+$phase+' ')}
                }
                if($TestCase -eq 'InventoryOwnership'){
                    $marker=if($taskRun.Role -eq 'Server'){'Inventory ownership server verified foreign live GUID and malformed action/quantity refusal'}else{'Inventory ownership client verified known foreign IDs or valid-owner malformed requests'}
                    $contract=$contract -and $text.Contains($marker)
                }
            }
            if($TestCase -eq 'AutoClimb'){$contract=if($taskRun.Role -eq 'Server'){$text.Contains('Server automatic real jump/wall contact landing verified')}else{$text.Contains('Owner automatic jump/contact/acknowledged landing verified without F')}}
            if($TestCase -eq 'AuthoredClimb'){$contract=if($taskRun.Role -eq 'Server'){$text.Contains('Authored server real map contact landings and exact costs verified')}else{$text.Contains('Authored owner both real map ledges, server acknowledgements and no item grants verified')}}
            $network=($text -match ('PktLag set to '+$lag+'\b')) -and ($text -match ('PktLoss set to '+$loss+'\b'))
            $passed=$verdictCode -eq 0 -and $verdict.Passed -and $verdict.TestCount -eq 1 -and $fatal -eq 0 -and $contract -and $network
            $summary.Processes+= [ordered]@{Role=$taskRun.Role;Passed=$passed;EngineExit=$taskRun.Process.ExitCode;VerdictExit=$verdictCode;Verdict=$verdict;Contract=$contract;NetworkApplied=$network;FatalEnsureAssertion=$fatal;WorkingGiB=[Math]::Round($taskRun.Working/1GB,3);PrivateGiB=[Math]::Round($taskRun.Private/1GB,3);LogWarnings=[regex]::Matches($text,'(?im)^.*\bWarning:').Count;LogErrors=[regex]::Matches($text,'(?im)^.*\bError:').Count;Log=$taskRun.Log;Report=(Join-Path $taskRun.Report 'index.json')}
        } catch {$summary.Processes+=[ordered]@{Role=$taskRun.Role;Passed=$false;Failure=$_.Exception.Message}}
        finally {$taskRun.Process.Dispose()}
    }
    $summary.PersonalSaveCount=$before.Count
    $summary.PersonalSavesUnchanged=($before -join "`n") -ceq (@(Get-PersonalSnapshot) -join "`n")
    $summary.DefaultSettingsUnchanged=($settingsBefore -join "`n") -ceq (@(Get-SettingsSnapshot) -join "`n")
    $summary.Seconds=[Math]::Round($watch.Elapsed.TotalSeconds,2)
    $summary.Passed=!$summary.Failure -and !$summary.TimedOut -and !$summary.CleanupFailures.Count -and $summary.Processes.Count -eq ($Players+1) -and !@($summary.Processes | Where-Object {!$_.Passed}).Count -and $summary.PersonalSavesUnchanged -and $summary.DefaultSettingsUnchanged
    $summary.Limit='NullRHI fixture only; raw log review, authored open-world activation, rendering, human gameplay, packaged server, FPS/minimum-spec acceptance remain separate'
    $summary | ConvertTo-Json -Depth 10 | Set-Content -LiteralPath (Join-Path $report 'run-summary.json') -Encoding UTF8
    $summary | ConvertTo-Json -Depth 10
    if($summary.Passed){$exitCode=0}
}
exit $exitCode
