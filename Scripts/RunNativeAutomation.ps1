# Native Editor automation only. Live server/client and rendered/manual gates are separate.
[CmdletBinding(DefaultParameterSetName = 'Run')]
param(
    [Parameter(ParameterSetName = 'Run')]
    [ValidatePattern('^PF(?:\.[A-Za-z0-9_]+)+(?:\+PF(?:\.[A-Za-z0-9_]+)+)*$')]
    [ValidateLength(1, 200)]
    [string]$TestFilter = 'PF.Persistence',
    [Parameter(ParameterSetName = 'Run')]
    [ValidatePattern('^[A-Za-z0-9_]+$')][ValidateLength(1, 32)]
    [string]$Label = 'Native',
    [Parameter(ParameterSetName = 'Run')]
    [ValidateRange(30, 1800)][int]$TimeoutSeconds = 300,
    [Parameter(ParameterSetName = 'Run')]
    [string]$EngineRoot = 'C:\Program Files\Epic Games\UE_5.8',
    [string]$ProjectRoot = '',
    [Parameter(Mandatory = $true, ParameterSetName = 'Inspect')]
    [string]$ExistingReport,
    [Parameter(Mandatory = $true, ParameterSetName = 'Inspect')]
    [int]$RecordedProcessExitCode
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

function Get-ReportVerdict([string]$Path, [int]$ProcessExitCode) {
    $reasons = @()
    $tests = @()
    if ($ProcessExitCode -ne 0) { $reasons += "Process exited $ProcessExitCode" }
    try {
        if (-not (Test-Path -LiteralPath $Path -PathType Leaf)) { throw 'Missing automation index.json' }
        $report = Get-Content -LiteralPath $Path -Raw | ConvertFrom-Json
        foreach ($field in @('tests', 'succeeded', 'succeededWithWarnings', 'failed', 'notRun', 'inProcess')) {
            if ($null -eq $report.PSObject.Properties[$field]) { throw "Missing report field: $field" }
        }
        foreach ($field in @('succeeded', 'succeededWithWarnings', 'failed', 'notRun', 'inProcess')) {
            $value = $report.$field
            if (($value -isnot [int] -and $value -isnot [long]) -or $value -lt 0) {
                throw 'Invalid report count'
            }
        }
        $tests = @($report.tests | ForEach-Object {
            foreach ($field in @('fullTestPath', 'state', 'errors', 'warnings')) {
                if ($null -eq $_.PSObject.Properties[$field]) { throw "Missing test field: $field" }
            }
            if ($_.fullTestPath -notmatch '^PF(?:\.[A-Za-z0-9_]+)+$') { throw 'Unexpected test identifier' }
            foreach ($field in @('errors', 'warnings')) {
                $value = $_.$field
                if (($value -isnot [int] -and $value -isnot [long]) -or $value -lt 0) {
                    throw 'Invalid test count'
                }
            }
            [pscustomobject]@{ Name = $_.fullTestPath; State = $_.state; Errors = [int]$_.errors; Warnings = [int]$_.warnings }
        })
        if ($tests.Count -eq 0) { $reasons += 'No tests executed' }
        if (@($tests | Select-Object -ExpandProperty Name -Unique).Count -ne $tests.Count) {
            $reasons += 'Duplicate test identifiers'
        }
        if ([int]$report.failed -ne 0 -or [int]$report.notRun -ne 0 -or [int]$report.inProcess -ne 0) {
            $reasons += 'Report contains failed, unrun or incomplete tests'
        }
        if ([int]$report.succeededWithWarnings -ne 0) { $reasons += 'Report contains test warnings' }
        if ([int]$report.succeeded -ne $tests.Count) { $reasons += 'Successful count does not match test records' }
        if (@($tests | Where-Object { $_.State -ne 'Success' -or $_.Errors -ne 0 -or $_.Warnings -ne 0 }).Count -gt 0) {
            $reasons += 'A test is not a clean Success'
        }
    }
    catch { $reasons += 'Missing, malformed or unsupported automation report' }
    return [pscustomobject]@{
        Passed = ($reasons.Count -eq 0); Reasons = @($reasons); ProcessExitCode = $ProcessExitCode
        TestCount = $tests.Count; Tests = @($tests)
    }
}

function Get-SafeFullPath([string]$Path) {
    if ($Path -match '["\x00-\x1f]') { throw 'Path contains unsupported quote/control characters' }
    return [IO.Path]::GetFullPath($Path)
}

try {
    if ([string]::IsNullOrWhiteSpace($ProjectRoot)) { $ProjectRoot = Split-Path -Parent $PSScriptRoot }
    $projectDirectory = Get-SafeFullPath $ProjectRoot
    $projectFile = Join-Path $projectDirectory 'PrimalFrontier.uproject'
    if (-not (Test-Path -LiteralPath $projectFile -PathType Leaf)) { throw 'PrimalFrontier.uproject not found' }
    $reportRoot = Join-Path $projectDirectory 'Saved\AutomationReports'
    if ($PSCmdlet.ParameterSetName -eq 'Inspect') {
        $reportFile = Get-SafeFullPath $ExistingReport
        if (-not $reportFile.StartsWith($reportRoot + '\', [StringComparison]::OrdinalIgnoreCase) -or
            [IO.Path]::GetFileName($reportFile) -ne 'index.json') { throw 'Inspect only project-local automation index.json reports' }
        $verdict = Get-ReportVerdict $reportFile $RecordedProcessExitCode
        $verdict | ConvertTo-Json -Depth 5
        if ($verdict.Passed) { exit 0 } else { exit 1 }
    }

    # Preserve running user sessions rather than launch a second Editor on the low-memory host.
    if (@(Get-Process -Name UnrealEditor, UnrealEditor-Cmd -ErrorAction SilentlyContinue).Count -gt 0) {
        throw 'An Unreal Editor process is running; finish that session before launching native automation'
    }
    $editor = Join-Path (Get-SafeFullPath $EngineRoot) 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe'
    if (-not (Test-Path -LiteralPath $editor -PathType Leaf)) { throw 'UnrealEditor-Cmd.exe not found' }
    $runName = 'Automation_' + $Label + '_' + [DateTime]::UtcNow.ToString('yyyyMMdd_HHmmssfff') + '_' + [Guid]::NewGuid().ToString('N').Substring(0, 8)
    $runDirectory = Join-Path $reportRoot $runName
    $logDirectory = Join-Path $projectDirectory 'Saved\Logs'
    $null = New-Item -ItemType Directory -Path $runDirectory -Force
    $null = New-Item -ItemType Directory -Path $logDirectory -Force
    $logFile = Join-Path $logDirectory ('PF' + $runName + '.log')
    $arguments = '"{0}" -nullrhi -unattended -nosplash -nosound -NoLiveCoding -NoSaveConfig -ExecCmds="Automation RunTests {1}" -TestExit="Automation Test Queue Empty" -ReportExportPath="{2}" -abslog="{3}"' -f $projectFile, $TestFilter, $runDirectory, $logFile
    $process = Start-Process -FilePath $editor -ArgumentList $arguments -WorkingDirectory $projectDirectory -WindowStyle Hidden -PassThru
    $watch = [Diagnostics.Stopwatch]::StartNew()
    $peakWorking = 0L; $peakPrivate = 0L; $timedOut = $false
    try {
        while (-not $process.HasExited) {
            $process.Refresh()
            $peakWorking = [Math]::Max($peakWorking, $process.WorkingSet64)
            $peakPrivate = [Math]::Max($peakPrivate, $process.PrivateMemorySize64)
            if ($watch.Elapsed.TotalSeconds -ge $TimeoutSeconds) { $timedOut = $true; $process.Kill(); break }
            Start-Sleep -Milliseconds 500
        }
        $process.WaitForExit()
        $verdict = Get-ReportVerdict (Join-Path $runDirectory 'index.json') $process.ExitCode
        if ($timedOut) { $verdict.Passed = $false; $verdict.Reasons += 'Owned automation process timed out' }
        # Log severity counts require review; never dump raw lines, login URLs, credentials or save data.
        $logText = if (Test-Path -LiteralPath $logFile) { Get-Content -LiteralPath $logFile -Raw } else { '' }
        $fatalCount = [regex]::Matches($logText, '(?im)\bFatal error:|\bEnsure condition failed:').Count
        if ($fatalCount -gt 0) { $verdict.Passed = $false; $verdict.Reasons += 'Fatal/ensure found in log' }
        $summary = [ordered]@{
            SchemaVersion = 1; RunName = $runName; Filter = $TestFilter; NullRHI = $true
            Verdict = $verdict; TimedOut = $timedOut; Seconds = [Math]::Round($watch.Elapsed.TotalSeconds, 2)
            SampledPeakWorkingGiB = [Math]::Round($peakWorking / 1GB, 3)
            SampledPeakPrivateGiB = [Math]::Round($peakPrivate / 1GB, 3)
            LogWarningLines = [regex]::Matches($logText, '(?im)^.*\bWarning:').Count
            LogErrorLines = [regex]::Matches($logText, '(?im)^.*\bError:').Count
            LogFatalOrEnsureCount = $fatalCount; LogReviewRequired = ($logText -match '(?im)\bWarning:|\bError:')
            Report = (Join-Path $runDirectory 'index.json'); Log = $logFile
            Limit = 'Native automation only; report pass does not certify raw log review, rendering or manual gameplay'
        }
        $summary | ConvertTo-Json -Depth 7 | Set-Content -LiteralPath (Join-Path $runDirectory 'run-summary.json') -Encoding UTF8
        $summary | ConvertTo-Json -Depth 7
        if ($verdict.Passed) { exit 0 } else { exit 1 }
    }
    finally {
        if (-not $process.HasExited) { $process.Kill(); $process.WaitForExit() }
        $process.Dispose()
    }
}
catch {
    Write-Error ('[PrimalAgentTools] Native automation refused: ' + $_.Exception.Message) -ErrorAction Continue
    exit 2
}
