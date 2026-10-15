# Disposable PowerShell contract tests: no Unreal, network, credentials or personal saves.
[CmdletBinding()]
param()
Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
$fixture = Join-Path ([IO.Path]::GetTempPath()) ('PFSetup_' + [Guid]::NewGuid().ToString('N'))
$checks = [Collections.Generic.List[string]]::new()
function Expect-Refusal([string]$Name, [scriptblock]$Action, [string]$Pattern) {
    $refused = $false
    try { & $Action | Out-Null } catch { if ($_.Exception.Message -notmatch $Pattern) { throw }; $refused = $true }
    if (!$refused) { throw "Expected refusal: $Name" }
    $checks.Add($Name)
}
try {
    $root = Join-Path $fixture 'Project With Spaces'
    $engine = Join-Path $fixture 'Engine With Spaces'
    New-Item -ItemType Directory -Path (Join-Path $root 'Saved\Persistence'),(Join-Path $engine 'Engine\Binaries\Win64') -Force | Out-Null
    Set-Content -LiteralPath (Join-Path $root 'PrimalFrontier.uproject') -Value '{}'
    Set-Content -LiteralPath (Join-Path $engine 'Engine\Binaries\Win64\UnrealEditor.exe') -Value 'Fixture; never execute'
    $launcher = Join-Path $PSScriptRoot 'StartPrivateDevelopmentServer.ps1'
    $planParameters = @{ ProjectRoot=$root; EngineRoot=$engine; Slot='DevSetupTest' }
    foreach ($file in @('GetMultiplayerReadiness.ps1','StartPrivateDevelopmentServer.ps1','TestMultiplayerSetup.ps1')) {
        $parseTokens = $null; $parseErrors = $null
        $null = [Management.Automation.Language.Parser]::ParseFile((Join-Path $PSScriptRoot $file),[ref]$parseTokens,[ref]$parseErrors)
        if ($parseErrors.Count) { throw "Parse failed: $file" }
    }
    $checks.Add('SyntaxAllThree')
    $plan = & $launcher @planParameters -Mode New | ConvertFrom-Json
    if ($plan.Result -ne 'PLAN_ONLY' -or $plan.Arguments -notcontains '-MULTIHOME=127.0.0.1' -or $plan.Arguments -notcontains '-nullrhi' -or $plan.Arguments -contains '-PFLoadSave' -or $plan.Arguments[0] -ne ('"' + (Join-Path $root 'PrimalFrontier.uproject') + '"')) { throw 'New launch plan invalid' }
    $checks.Add('NewLocalhostNullRHIQuotedPath')
    if (@(Get-ChildItem -LiteralPath (Join-Path $root 'Saved\Persistence') -File).Count) { throw 'Plan created a save' }
    $checks.Add('NewPlanCreatesNoSave')
    Expect-Refusal 'LoadMissingRefused' { & $launcher @planParameters -Mode Load } 'missing slot'
    $save = Join-Path $root 'Saved\Persistence\DevSetupTest.a.pfs'
    [IO.File]::WriteAllBytes($save,[byte[]](1,2,3,4))
    $before = (Get-FileHash -LiteralPath $save).Hash
    Expect-Refusal 'NewExistingRefused' { & $launcher @planParameters -Mode New } 'existing slot'
    $plan = & $launcher @planParameters -Mode Load | ConvertFrom-Json
    if ($plan.Arguments -notcontains '-PFLoadSave' -or $plan.Result -ne 'PLAN_ONLY') { throw 'Load plan invalid' }
    $checks.Add('LoadFlagNoCorruptSaveCertification')
    if ((Get-FileHash -LiteralPath $save).Hash -ne $before) { throw 'Fixture save mutated' }
    $checks.Add('SaveBytesUnchanged')
    Move-Item -LiteralPath $save -Destination (Join-Path $root 'Saved\Persistence\DevSetupTest.b.pfs')
    $plan = & $launcher @planParameters -Mode Load | ConvertFrom-Json
    if ($plan.Arguments -notcontains '-PFLoadSave') { throw 'Backup-only load plan invalid' }
    $checks.Add('BackupGenerationOnlyLoadPlan')
    Expect-Refusal 'ReservedSlotRefused' { & $launcher -ProjectRoot $root -EngineRoot $engine -Mode New -Slot 'Identity_private' } 'Reserved'
    Expect-Refusal 'InjectedSlotRefused' { & $launcher -ProjectRoot $root -EngineRoot $engine -Mode New -Slot 'bad -ExecCmds=quit' } 'validate'
    Expect-Refusal 'MissingEngineRefused' { & $launcher -ProjectRoot $root -EngineRoot (Join-Path $fixture 'Missing') -Mode New -Slot 'Other' } 'missing'
    Expect-Refusal 'InvalidPortRefused' { & $launcher @planParameters -Mode Load -Port 0 } 'validate'
    [pscustomobject]@{TestName='PF.Multiplayer.SetupContracts';TimestampUtc=[DateTime]::UtcNow.ToString('o');Map='No level required';Result='PASS';Passed=$checks.Count;Tests=$checks;Details='Plans/refusals only. No Unreal launch or cloud/SQL/audio test.'} | ConvertTo-Json -Depth 4
} finally {
    # Only this invocation's GUID-owned disposable fixture; never a caller-provided path.
    $resolved = [IO.Path]::GetFullPath($fixture)
    $tempPrefix = [IO.Path]::GetFullPath([IO.Path]::GetTempPath()).TrimEnd('\') + '\'
    if (!$resolved.StartsWith($tempPrefix,[StringComparison]::OrdinalIgnoreCase) -or [IO.Path]::GetFileName($resolved) -notmatch '^PFSetup_[a-f0-9]{32}$') { throw 'Fixture cleanup path refused' }
    if (Test-Path -LiteralPath $resolved) { Remove-Item -LiteralPath $resolved -Recurse -Force }
}
