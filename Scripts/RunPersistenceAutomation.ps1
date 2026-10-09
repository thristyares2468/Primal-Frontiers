# Disposable M8 live tests only. Never uses a personal slot/profile or rendered clients.
[CmdletBinding()]
param(
    [ValidateSet(1, 2)][int]$Players = 1,
    [switch]$SimulateLagLoss,
    [switch]$ToolProgression,
    [switch]$WeaponProgression,
    [switch]$Protection,
    [ValidateRange(1024, 65535)][int]$Port = 17989,
    [ValidateRange(30, 600)][int]$TimeoutSeconds = 210,
    [string]$EngineRoot = 'C:\Program Files\Epic Games\UE_5.8',
    [string]$ProjectRoot = ''
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

function Get-SafeFullPath([string]$Path) {
    if ($Path -match '["\x00-\x1f]') { throw 'Unsupported path quote/control characters' }
    return [IO.Path]::GetFullPath($Path)
}

function Update-Memory($Run) {
    try {
        if (-not $Run.Process.HasExited) {
            $Run.Process.Refresh()
            $Run.Working = [Math]::Max($Run.Working, $Run.Process.WorkingSet64)
            $Run.Private = [Math]::Max($Run.Private, $Run.Process.PrivateMemorySize64)
        }
    }
    catch [InvalidOperationException] {
        if (-not $Run.Process.HasExited) { throw }
    }
}

$summaryDirectory = $null
$summary = $null
$exitCode = 2
try {
    if ([string]::IsNullOrWhiteSpace($ProjectRoot)) { $ProjectRoot = Split-Path -Parent $PSScriptRoot }
    $root = Get-SafeFullPath $ProjectRoot
    $projectFile = Join-Path $root 'PrimalFrontier.uproject'
    $editor = Join-Path (Get-SafeFullPath $EngineRoot) 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe'
    $inspector = Join-Path $PSScriptRoot 'RunNativeAutomation.ps1'
    foreach ($required in @($projectFile, $editor, $inspector)) {
        if (-not (Test-Path -LiteralPath $required -PathType Leaf)) { throw 'Required project, engine or verdict tool missing' }
    }
    if (@(Get-Process -Name UnrealEditor, UnrealEditor-Cmd -ErrorAction SilentlyContinue).Count) {
        throw 'Existing Editor session; finish it before live automation'
    }
    if (@(Get-NetUDPEndpoint -LocalPort $Port -ErrorAction SilentlyContinue).Count) {
        throw 'Diagnostic UDP port is already occupied'
    }

    $stamp = [DateTime]::UtcNow.ToString('yyyyMMdd_HHmmssfff') + '_' + [Guid]::NewGuid().ToString('N').Substring(0, 8)
    if(@(@($ToolProgression,$WeaponProgression,$Protection) | Where-Object {$_}).Count -gt 1){throw 'Choose one progression fixture per run'}
    $prefix = $(if($Protection){'M12ProtectionLive'}elseif($WeaponProgression){'M12WeaponLive'}elseif($ToolProgression){'M12ToolLive'}else{'M8Live'}) + $Players + '_' + $stamp
    $testFilter = if($Protection){'PF.Crafting.ProtectionLive'}elseif($WeaponProgression){'PF.Crafting.WeaponProgressionLive'}elseif($ToolProgression){'PF.Crafting.ToolProgressionLive'}else{'PF.Persistence.Live'}
    $testFlag = if($Protection){'-PFRunProtectionTests'}elseif($WeaponProgression){'-PFRunWeaponProgressionTests'}elseif($ToolProgression){'-PFRunToolProgressionTests'}else{'-PFRunPersistenceLiveTests'}
    $slot = 'Automation' + $prefix
    # Profiles are private local test state. Same aliases and endpoint survive both phases.
    $profile = 'PF' + [Guid]::NewGuid().ToString('N').Substring(0, 8)
    $reportRoot = Join-Path $root 'Saved\AutomationReports'
    $logRoot = Join-Path $root 'Saved\Logs'
    $summaryDirectory = Join-Path $reportRoot $prefix
    $null = New-Item -ItemType Directory -Path $summaryDirectory
    $null = New-Item -ItemType Directory -Path $logRoot -Force
    $summary = [ordered]@{
        SchemaVersion = 1; Runner = 'RunPersistenceAutomation'; RunName = $prefix
        Passed = $false; Players = $Players; NullRHI = $true; Port = $Port
        Map = '/Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld'
        SimulatedOutgoingLagMs = $(if ($SimulateLagLoss) { 75 } else { 0 })
        SimulatedOutgoingLossPercent = $(if ($SimulateLagLoss) { 1 } else { 0 })
        PhaseTimeoutSeconds = $TimeoutSeconds; Phases = @(); Failure = $null
        Limit = 'Automation only; raw log review, rendering, manual gameplay, packaged server and 16GB certification remain separate'
    }
    $exitCode = 1
    foreach ($phase in @('Create', 'Restart')) {
        $runs = @()
        $phaseRecord = [ordered]@{ Name = $phase; Passed = $false; TimedOut = $false; CleanupFailures = @(); Seconds = 0; Processes = @() }
        $summary.Phases += $phaseRecord
        $watch = [Diagnostics.Stopwatch]::StartNew()
        try {
            for ($i = 0; $i -le $Players; $i++) {
                $role = if ($i -eq 0) { 'Server' } else { 'Client' + $i }
                $name = $prefix + $phase + $role
                $report = Join-Path $reportRoot $name
                $log = Join-Path $logRoot ('PF' + $name + '.log')
                $null = New-Item -ItemType Directory -Path $report
                $url = if ($i -eq 0) { $summary.Map } else { '127.0.0.1:' + $Port }
                $mode = if ($i -eq 0) { '-server -game -port=' + $Port } else { '-game -PFIdentityProfile=' + $profile + $i }
                $load = if ($phase -eq 'Restart') { '-PFLoadSave' } else { '' }
                $network = if ($SimulateLagLoss) { '-PktLag=75 -PktLoss=1' } else { '-PktLag=0 -PktLoss=0' }
                $arguments = '"{0}" {1} {2} -nullrhi -unattended -nosplash -nosound -NoLiveCoding -NoSaveConfig {3} {9} -PFExpectedPlayers={4} -PFSaveSlot={5} {6} -ExecCmds="Automation RunTests {10}" -TestExit="Automation Test Queue Empty" -ReportExportPath="{7}" -abslog="{8}"' -f $projectFile, $url, $mode, $network, $Players, $slot, $load, $report, $log, $testFlag, $testFilter
                $process = Start-Process -FilePath $editor -ArgumentList $arguments -WorkingDirectory $root -WindowStyle Hidden -PassThru
                $run = [pscustomobject]@{ Name = $name; Role = $role; Process = $process; Report = $report; Log = $log; Working = 0L; Private = 0L }
                $runs += $run
                if ($i -eq 0) {
                    while ($true) {
                        Update-Memory $run
                        if ($process.HasExited) { throw 'Server exited before client launch' }
                        $ready = (Test-Path -LiteralPath $log) -and ((Get-Content -LiteralPath $log -Raw) -match ('IpNetDriver listening on port ' + $Port + '\b'))
                        if ($ready) { break }
                        if ($watch.Elapsed.TotalSeconds -ge [Math]::Min(45, $TimeoutSeconds)) {
                            $phaseRecord.TimedOut = $true; throw 'Server readiness deadline exceeded'
                        }
                        Start-Sleep -Milliseconds 500
                    }
                }
            }
            while (@($runs | Where-Object { -not $_.Process.HasExited }).Count) {
                foreach ($run in $runs) { Update-Memory $run }
                if ($watch.Elapsed.TotalSeconds -ge $TimeoutSeconds) {
                    $phaseRecord.TimedOut = $true; throw 'Owned live phase deadline exceeded'
                }
                Start-Sleep -Milliseconds 500
            }
        }
        finally {
            # Only the handles registered by this phase are ever terminated; no name-wide cleanup.
            foreach ($run in $runs) {
                try {
                    if (-not $run.Process.HasExited) { $run.Process.Kill() }
                    if (-not $run.Process.WaitForExit(5000)) { $phaseRecord.CleanupFailures += $run.Role }
                }
                catch {
                    # An exit racing Kill is harmless; still attempt cleanup of every other handle.
                    if (-not $run.Process.HasExited) { $phaseRecord.CleanupFailures += $run.Role }
                }
            }
            # Retain outcomes for every launched process, including aborted/incomplete phases.
            foreach ($run in $runs) {
                try {
                    if (-not $run.Process.HasExited) {
                        $phaseRecord.Processes += [ordered]@{
                            Run = $run.Name; Role = $run.Role; Passed = $false; ProcessId = $run.Process.Id
                            ReportReasons = @('Owned process cleanup failed; manual termination required')
                            StorageOwnerRole = $false; StorageForeignRole = $false
                        }
                        continue
                    }
                    $index = Join-Path $run.Report 'index.json'
                    $verdictJson = & powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File $inspector -ProjectRoot $root -ExistingReport $index -RecordedProcessExitCode $run.Process.ExitCode
                    $reportExit = $LASTEXITCODE
                    $verdict = ($verdictJson -join "`n") | ConvertFrom-Json
                    $text = if (Test-Path -LiteralPath $run.Log) { Get-Content -LiteralPath $run.Log -Raw } else { '' }
                    $fatal = [regex]::Matches($text, '(?im)\bFatal error:|\bEnsure condition failed:').Count
                    $serverRefusals = [regex]::Matches($text, 'Request action=0 quantity=-1 accepted=0').Count
                    $ownerRole = $text -match ('Storage privacy role owner=1 foreign=0 otherPlayers=' + ($Players - 1) + '\b')
                    $foreignRole = $text -match ('Storage privacy role owner=0 foreign=1 otherPlayers=' + ($Players - 1) + '\b')
                    $networkApplied = $text -match ('PktLag set to ' + $summary.SimulatedOutgoingLagMs + '\b') -and
                        $text -match ('PktLoss set to ' + $summary.SimulatedOutgoingLossPercent + '\b')
                    $contract = if ($run.Role -eq 'Server') { $serverRefusals -eq $Players } else {
                        $text -match 'Invalid inventory RPC acknowledged by server refusal' -and
                        $text -match 'Owner/foreign bag and storage privacy assertions inspected after load/reconnect' -and
                        ($ownerRole -xor $foreignRole)
                    }
                    if($ToolProgression){$contract=if($run.Role -eq 'Server'){$text -match $(if($phase -eq 'Restart'){'Tool restart server restored every owner'}else{'Tool server conservation and save verified'})}else{$text -match $(if($phase -eq 'Restart'){'Tool restart client identity, tier and privacy verified'}else{'Tool owned RPC loop, finite yield and privacy verified'})}}
                    if($WeaponProgression){$contract=if($run.Role -eq 'Server'){$text -match $(if($phase -eq 'Restart'){'Weapon restart server restored every owner'}else{'Weapon server conservation and save verified'})}else{$text -match $(if($phase -eq 'Restart'){'Weapon restart client identity, tier and privacy verified'}else{'Weapon owned RPC loop, tier and privacy verified'})}}
                    if($Protection){$contract=if($run.Role -eq 'Server'){$text -match $(if($phase -eq 'Restart'){'Protection restart server restored every owner'}else{'Protection server damage, conservation and save verified'})}else{$text -match $(if($phase -eq 'Restart'){'Protection restart client identity, health and privacy verified'}else{'Protection owned RPC loop, health and privacy verified'})}}
                    $exactTest = $verdict.TestCount -eq 1 -and @($verdict.Tests | Where-Object { $_.Name -eq $testFilter }).Count -eq 1
                    $entry = [ordered]@{
                        Run = $run.Name; Role = $run.Role; Passed = ($reportExit -eq 0 -and $verdict.Passed -and $exactTest -and $fatal -eq 0 -and $contract -and $networkApplied)
                        EngineExit = $run.Process.ExitCode; ReportVerdictExit = $reportExit
                        Tests = $verdict.Tests; ReportReasons = $verdict.Reasons; FatalEnsure = $fatal
                        ContractEvidence = $contract; NetworkSettingsConfirmed = $networkApplied
                        StorageOwnerRole = ($run.Role -ne 'Server' -and $ownerRole)
                        StorageForeignRole = ($run.Role -ne 'Server' -and $foreignRole)
                        WorkingGiB = [Math]::Round($run.Working / 1GB, 3); PrivateGiB = [Math]::Round($run.Private / 1GB, 3)
                        LogWarnings = [regex]::Matches($text, '(?im)^.*\bWarning:').Count
                        LogErrors = [regex]::Matches($text, '(?im)^.*\bError:').Count
                        LogReviewRequired = ($text -match '(?im)\bWarning:|\bError:')
                        Report = $index; Log = $run.Log
                    }
                    $phaseRecord.Processes += $entry
                    $entry | ConvertTo-Json -Depth 6 -Compress
                }
                finally { $run.Process.Dispose() }
            }
            $phaseRecord.Seconds = [Math]::Round($watch.Elapsed.TotalSeconds, 2)
            $summary | ConvertTo-Json -Depth 9 | Set-Content -LiteralPath (Join-Path $summaryDirectory 'run-summary.json') -Encoding UTF8
        }
        $allPassed = $phaseRecord.Processes.Count -eq ($Players + 1) -and @($phaseRecord.Processes | Where-Object { -not $_.Passed }).Count -eq 0
        $rolesCovered = $Protection -or $WeaponProgression -or $ToolProgression -or (@($phaseRecord.Processes | Where-Object { $_.StorageOwnerRole }).Count -eq 1 -and
            @($phaseRecord.Processes | Where-Object { $_.StorageForeignRole }).Count -eq ($Players - 1))
        if ($phaseRecord.CleanupFailures.Count -or -not $allPassed -or -not $rolesCovered) { throw 'Live report, cleanup or RPC/privacy/profile evidence failed' }
        $phaseRecord.Passed = $true
    }
    $summary.Passed = $true
    $exitCode = 0
}
catch {
    if ($null -ne $summary) { $summary.Failure = $_.Exception.Message }
    Write-Error ('[PrimalAgentTools] Live automation stopped: ' + $_.Exception.Message) -ErrorAction Continue
}
finally {
    if ($null -ne $summaryDirectory -and $null -ne $summary) {
        $summary | ConvertTo-Json -Depth 9 | Set-Content -LiteralPath (Join-Path $summaryDirectory 'run-summary.json') -Encoding UTF8
        Write-Output ('[PrimalAgentTools] Live summary: ' + (Join-Path $summaryDirectory 'run-summary.json'))
    }
}
exit $exitCode
