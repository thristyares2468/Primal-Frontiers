# Local official-agent process-mode test only. Never allocates PlayFab capacity.
[CmdletBinding()]
param(
    [Parameter(Mandatory=$true)][string]$PackagedServerRoot,
    [string]$AgentRoot='C:\UnrealSource\Dependencies\LocalMultiplayerAgent_v0.9.6',
    [string]$WorkRoot='C:\UnrealSource\TestRuns',
    [string]$ProjectRoot='',
    [string]$EngineRoot='C:\Program Files\Epic Games\UE_5.8',
    [ValidateRange(1024,65535)][int]$GamePort=17996,
    [ValidateRange(1024,65535)][int]$AgentPort=56011,
    # The official v0.9.6 mock lacks only the optional startup /gsdkinfo metrics route.
    [switch]$AllowLegacyMetrics404,
    # Existing UCrowdManager warning is allowed only AFTER this world's BeginTearingDown.
    [switch]$AllowKnownCrowdTeardownWarning,
    [ValidateRange(60,300)][int]$TimeoutSeconds=150
)
Set-StrictMode -Version Latest
$ErrorActionPreference='Stop'
$agent=$null;$server=$null;$probe=$null;$runRoot=$null;$summary=$null;$exitCode=2
function Measure-OwnedProcess($Process,[string]$Prefix) {
    if(!$Process -or $Process.HasExited){return}
    try{$Process.Refresh();$summary[$Prefix+'PeakWorkingGiB']=[Math]::Max([double]$summary[$Prefix+'PeakWorkingGiB'],[Math]::Round($Process.WorkingSet64/1GB,3));$summary[$Prefix+'PeakPrivateGiB']=[Math]::Max([double]$summary[$Prefix+'PeakPrivateGiB'],[Math]::Round($Process.PrivateMemorySize64/1GB,3))}
    catch [InvalidOperationException]{if(!$Process.HasExited){throw}}
}
try {
    if(!$ProjectRoot){$ProjectRoot=Split-Path -Parent $PSScriptRoot}
    foreach($path in @($PackagedServerRoot,$AgentRoot,$WorkRoot,$ProjectRoot,$EngineRoot))
    {if($path -match '["\x00-\x1f]'){throw 'Unsupported path characters'}}
    $package=[IO.Path]::GetFullPath($PackagedServerRoot);$agentSource=[IO.Path]::GetFullPath($AgentRoot)
    $editor=Join-Path $EngineRoot 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe'
    $project=Join-Path $ProjectRoot 'PrimalFrontier.uproject'
    foreach($file in @((Join-Path $package 'PrimalFrontier\Binaries\Win64\PrimalFrontierServer.exe'),(Join-Path $agentSource 'MockVmAgent.exe'),$project,$editor))
    {if(!(Test-Path -LiteralPath $file -PathType Leaf)){throw 'Required package, official agent or compiled probe project missing'}}
    if(@(Get-ChildItem -LiteralPath $package -Recurse -File -Filter '*.pdb').Count){throw 'Stage with -nodebuginfo; retain debug symbols separately'}
    $signature=Get-AuthenticodeSignature -LiteralPath (Join-Path $agentSource 'MockVmAgent.exe')
    if($signature.Status -ne 'Valid' -or $signature.SignerCertificate.Subject -notmatch 'Microsoft Corporation'){throw 'Official Microsoft agent signature validation failed'}
    if(@(Get-Process -Name UnrealEditor,UnrealEditor-Cmd -ErrorAction SilentlyContinue).Count){throw 'Close existing Unreal sessions before isolated lifecycle test'}
    if($GamePort -eq $AgentPort -or @(Get-NetUDPEndpoint -LocalPort $GamePort -ErrorAction SilentlyContinue).Count -or
        @(Get-NetTCPConnection -LocalPort $AgentPort -State Listen -ErrorAction SilentlyContinue).Count){throw 'Diagnostic port occupied/conflicting'}
    $name='M15GSDK_'+[DateTime]::UtcNow.ToString('yyyyMMdd_HHmmss')+'_'+[Guid]::NewGuid().ToString('N').Substring(0,8)
    $runRoot=Join-Path ([IO.Path]::GetFullPath($WorkRoot)) $name
    $agentDir=Join-Path $runRoot 'Agent';$report=Join-Path $ProjectRoot ('Saved\AutomationReports\'+$name+'Probe')
    $null=New-Item -ItemType Directory -Path $agentDir
    $null=New-Item -ItemType Directory -Path $report
    Get-ChildItem -LiteralPath $agentSource -Force | Copy-Item -Destination $agentDir -Recurse
    $archive=Join-Path $runRoot 'Server.zip'
    Add-Type -AssemblyName System.IO.Compression.FileSystem
    [IO.Compression.ZipFile]::CreateFromDirectory($package,$archive,[IO.Compression.CompressionLevel]::Fastest,$false)
    $serverLog=Join-Path $runRoot 'Server.log';$probeLog=Join-Path $runRoot 'Probe.log'
    $userDir=(Join-Path $runRoot 'ServerUser').Replace('\','/')+'/'
    $config=Get-Content -LiteralPath (Join-Path $agentDir 'MultiplayerSettings.json') -Raw | ConvertFrom-Json
    $config.RunContainer=$false;$config.OutputFolder=Join-Path $runRoot 'Output'
    # Cold Editor clients on this machine can need ~80s before their probe starts.
    $config.NumHeartBeatsForActivateResponse=5;$config.NumHeartBeatsForTerminateResponse=110;$config.AgentListeningPort=$AgentPort
    $config.AssetDetails[0].MountPath='';$config.AssetDetails[0].LocalFilePath=$archive
    $config.PortMappingsList[0][0].NodePort=$GamePort;$config.PortMappingsList[0][0].GamePort.Name='gameport'
    $config.PortMappingsList[0][0].GamePort.Number=$GamePort;$config.PortMappingsList[0][0].GamePort.Protocol='UDP'
    $config.ProcessStartParameters.StartGameCommand='PrimalFrontier\Binaries\Win64\PrimalFrontierServer.exe /Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld -PFPlayFabHosted -nullrhi -unattended -nosound -NoSaveConfig -MULTIHOME=127.0.0.1 -LogCmds="LogPFHosting Verbose" -UserDir="'+$userDir+'" -abslog="'+$serverLog+'"'
    $config.TitleId='6B7B6';$config.BuildId=[Guid]::NewGuid().ToString();$config.Region='EastUs' # Mock metadata only.
    $config.SessionConfig.SessionId=[Guid]::NewGuid().ToString();$config.SessionConfig.SessionCookie='local-lifecycle-only';$config.SessionConfig.InitialPlayers=@()
    $config | ConvertTo-Json -Depth 12 | Set-Content -LiteralPath (Join-Path $agentDir 'MultiplayerSettings.json') -Encoding UTF8
    $summary=[ordered]@{Run=$name;Passed=$false;Failure=$null;Map='/Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld';GamePort=$GamePort;AgentPort=$AgentPort;AgentExit=$null;ServerExit=$null;ProbeExit=$null;ProbeVerdict=$null;CleanupFailures=@();ServerPeakWorkingGiB=0;ServerPeakPrivateGiB=0;Standby=$false;Active=$false;Terminating=$false;Healthy=$false;ProbeReport=$report;Limit='Offline process-mode agent only; gameplay admission CLOSED, no cloud allocation/durable restore/container/voice claim'}
    $exitCode=1;$watch=[Diagnostics.Stopwatch]::StartNew()
    $summary.AgentPeakWorkingGiB=0;$summary.AgentPeakPrivateGiB=0;$summary.ProbePeakWorkingGiB=0;$summary.ProbePeakPrivateGiB=0
    $summary.ArchiveSHA256=(Get-FileHash -LiteralPath $archive -Algorithm SHA256).Hash
    $summary.PackageExeSHA256=(Get-FileHash -LiteralPath (Join-Path $package 'PrimalFrontier\Binaries\Win64\PrimalFrontierServer.exe') -Algorithm SHA256).Hash
    $agent=Start-Process -FilePath (Join-Path $agentDir 'MockVmAgent.exe') -WorkingDirectory $agentDir -WindowStyle Hidden -PassThru -RedirectStandardOutput (Join-Path $runRoot 'Agent.stdout.log') -RedirectStandardError (Join-Path $runRoot 'Agent.stderr.log')
    $null=$agent.Handle
    while($watch.Elapsed.TotalSeconds -lt $TimeoutSeconds) {
        if(!$server){
            $candidate=Get-CimInstance Win32_Process -Filter "Name='PrimalFrontierServer.exe'" | Where-Object {$_.CommandLine -and $_.CommandLine.Contains($serverLog)} | Select-Object -First 1
            if($candidate){try{$server=[Diagnostics.Process]::GetProcessById($candidate.ProcessId);$null=$server.Handle}catch{throw 'Owned server exited before its handle could be captured'}}
        }
        Measure-OwnedProcess $server 'Server';Measure-OwnedProcess $agent 'Agent';Measure-OwnedProcess $probe 'Probe'
        $logText=if(Test-Path -LiteralPath $serverLog){Get-Content -LiteralPath $serverLog -Raw}else{''}
        if(!$probe -and $logText -match '\[PFHosting\] StandingBy;'){
            $probeUser=(Join-Path $runRoot 'ProbeUser').Replace('\','/')+'/'
            $probeArguments='"'+$project+'" /Engine/Maps/Entry -game -nullrhi -unattended -nosplash -nosound -NoLiveCoding -NoSaveConfig -PFRunHostingAdmissionProbe -PFHostingProbePort='+$GamePort+' -UserDir="'+$probeUser+'" -ExecCmds="Automation RunTests PF.Hosting.ClosedAdmissionLive" -TestExit="Automation Test Queue Empty" -ReportExportPath="'+$report+'" -abslog="'+$probeLog+'"'
            $probe=Start-Process -FilePath $editor -ArgumentList $probeArguments -WorkingDirectory $ProjectRoot -WindowStyle Hidden -PassThru;$null=$probe.Handle
        }
        if($agent.HasExited -and (!$server -or $server.HasExited) -and (!$probe -or $probe.HasExited)){break}
        Start-Sleep -Milliseconds 250
    }
    if(!$agent.HasExited -or !$server -or !$server.HasExited -or !$probe -or !$probe.HasExited){throw 'Lifecycle/owned process deadline or startup failure; inspect retained agent/server logs'}
    $agent.WaitForExit();$server.WaitForExit();$probe.WaitForExit()
    $summary.AgentExit=$agent.ExitCode;$summary.ServerExit=$server.ExitCode;$summary.ProbeExit=$probe.ExitCode
    $agentText=Get-Content -LiteralPath (Join-Path $runRoot 'Agent.stdout.log') -Raw
    $logText=Get-Content -LiteralPath $serverLog -Raw
    $summary.Standby=$logText -match '\[PFHosting\] StandingBy;';$summary.Active=$logText -match '\[PFHosting\] Active;'
    $summary.Terminating=$logText -match '\[PFHosting\] Terminating; active=1'
    $summary.Healthy=($logText -match '\[PFHosting\] Health callback healthy=1\b' -and $logText -notmatch '\[PFHosting\] Health callback healthy=0\b')
    $summary.AgentStandbyActiveObserved=($agentText -match 'CurrentGameState:\s*StandingBy\b' -and $agentText -match 'CurrentGameState:\s*Active\b')
    $errorLines=@([regex]::Matches($logText,'(?im)^.*\bError:.*$') | ForEach-Object {$_.Value})
    $metricsErrors=@($errorLines | Where-Object {$_ -match 'LogPlayFabGSDK: Error: Received non-success code from Agent when sending info\.  Status Code: 404\s*$'})
    $summary.KnownMockMetrics404=($AllowLegacyMetrics404 -and $metricsErrors.Count -eq 1 -and $agentText -match '/v1/metrics/[^\s]+/gsdkinfo' -and $agentText -match 'Request finished[^\r\n]*\b404\b')
    $summary.ServerRawErrors=$errorLines.Count;$summary.ServerRawWarnings=[regex]::Matches($logText,'(?im)^.*\bWarning:').Count
    $summary.UnexpectedServerErrors=$errorLines.Count-$(if($summary.KnownMockMetrics404){1}else{0})
    $terminationWarnings=[regex]::Matches($logText,'(?im)^.*LogPlayFabGSDK: Warning: Received Termination State\s*$')
    $summary.TerminationCommandObserved=($terminationWarnings.Count -eq 1 -and $summary.Terminating)
    $crowdWarnings=[regex]::Matches($logText,'(?im)^.*LogCrowdFollowing: Warning: Unable to find RecastNavMesh instance while trying to create UCrowdManager instance\s*$')
    $teardownAt=$logText.IndexOf('BeginTearingDown for /Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld',[StringComparison]::Ordinal)
    $summary.KnownCrowdTeardownWarning=($AllowKnownCrowdTeardownWarning -and $crowdWarnings.Count -eq 1 -and $teardownAt -ge 0 -and
        $crowdWarnings[0].Index -gt $teardownAt -and $summary.TerminationCommandObserved)
    $summary.UnexpectedServerWarnings=$summary.ServerRawWarnings-$(if($summary.TerminationCommandObserved){1}else{0})-$(if($summary.KnownCrowdTeardownWarning){1}else{0})
    $summary.MemorySamplesValid=($summary.AgentPeakWorkingGiB -gt 0 -and $summary.ServerPeakWorkingGiB -gt 0 -and $summary.ProbePeakWorkingGiB -gt 0 -and
        $summary.AgentPeakPrivateGiB -gt 0 -and $summary.ServerPeakPrivateGiB -gt 0 -and $summary.ProbePeakPrivateGiB -gt 0)
    $index=Join-Path $report 'index.json'
    $verdictText=& powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File (Join-Path $PSScriptRoot 'RunNativeAutomation.ps1') -ProjectRoot $ProjectRoot -ExistingReport $index -RecordedProcessExitCode $probe.ExitCode -ExpectedFilter PF.Hosting.ClosedAdmissionLive
    $verdictExit=$LASTEXITCODE;$summary.ProbeVerdict=($verdictText -join "`n") | ConvertFrom-Json
    if($agent.ExitCode -ne 0 -or $server.ExitCode -ne 0 -or !$summary.Standby -or !$summary.Active -or !$summary.Terminating -or !$summary.Healthy -or !$summary.AgentStandbyActiveObserved -or !$summary.TerminationCommandObserved -or $verdictExit -ne 0 -or
        $summary.UnexpectedServerErrors -ne 0 -or $summary.UnexpectedServerWarnings -ne 0 -or !$summary.MemorySamplesValid -or $logText -match '(?im)\bFatal error:|\bEnsure condition failed:'){throw 'Lifecycle/health/admission/exit/log contract failed; inspect retained evidence'}
    $summary.Passed=$true;$summary.Result=if($summary.KnownMockMetrics404 -or $summary.KnownCrowdTeardownWarning){'PASS_WITH_KNOWN_LOCAL_MESSAGES'}else{'PASS'};$exitCode=0
}
catch {if($summary){$summary.Failure=$_.Exception.Message};Write-Error ('[PrimalAgentTools] Local GSDK test stopped: '+$_.Exception.Message) -ErrorAction Continue}
finally {
    # Only handles obtained for this unique run are touched; never name-wide termination.
    foreach($owned in @($probe,$server,$agent)){if($owned){try{if(!$owned.HasExited){$owned.Kill()};if(!$owned.WaitForExit(5000) -and $summary){$summary.CleanupFailures+=$owned.Id}}catch{if(!$owned.HasExited -and $summary){$summary.CleanupFailures+=$owned.Id}}finally{$owned.Dispose()}}}
    if($summary -and $summary.CleanupFailures.Count){$summary.Passed=$false;$exitCode=1}
    if($summary -and !$summary.Passed){$summary.Result='FAIL'}
    if($summary -and $runRoot){$summary | ConvertTo-Json -Depth 10 | Set-Content -LiteralPath (Join-Path $runRoot 'run-summary.json') -Encoding UTF8;Write-Output ('[PrimalAgentTools] Lifecycle evidence: '+$runRoot)}
}
exit $exitCode
