# Default: read-only launch plan. Opt-in -Launch starts localhost only; not a cloud package.
[CmdletBinding()]
param(
    [Parameter(Mandatory)][ValidateSet('New','Load')][string]$Mode,
    [Parameter(Mandatory)][ValidatePattern('^[A-Za-z0-9_]{1,64}$')][string]$Slot,
    [ValidateRange(1024,65535)][int]$Port = 7777,
    [string]$ProjectRoot = (Split-Path -Parent $PSScriptRoot),
    [string]$EngineRoot = 'C:\Program Files\Epic Games\UE_5.8',
    [switch]$Launch
)
Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
if ($Slot -match '^(Identity_|Metadata_)') { throw 'Reserved slot prefix' }
$root = [IO.Path]::GetFullPath($ProjectRoot)
$engine = [IO.Path]::GetFullPath($EngineRoot)
if ($root -match '["\x00-\x1f]' -or $engine -match '["\x00-\x1f]') { throw 'Unsafe path characters' }
$project = Join-Path $root 'PrimalFrontier.uproject'
$exe = Join-Path $engine 'Engine\Binaries\Win64\UnrealEditor.exe'
foreach ($file in @($project,$exe)) {
    if (!(Test-Path -LiteralPath $file -PathType Leaf)) { throw 'Required project/editor file missing' }
}
$saveRoot = Join-Path $root 'Saved\Persistence'
$generations = @(@('a','b') | ForEach-Object { Join-Path $saveRoot ($Slot + '.' + $_ + '.pfs') })
$found = @($generations | Where-Object { Test-Path -LiteralPath $_ -PathType Leaf })
if ($Mode -eq 'New' -and $found.Count) { throw 'New refuses an existing slot; use Load or a different name' }
if ($Mode -eq 'Load' -and !$found.Count) { throw 'Load refuses a missing slot; no fresh-world fallback' }
$map = '/Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld'
$arguments = @(('"' + $project + '"'), $map, '-server', '-game', '-nullrhi', '-nosound', '-log', '-NoLiveCoding', '-MULTIHOME=127.0.0.1', "-port=$Port", "-PFSaveSlot=$Slot")
if ($Mode -eq 'Load') { $arguments += '-PFLoadSave' }
$plan = [ordered]@{
    TestName = 'PF.Multiplayer.PrivateServerPlan'; TimestampUtc = [DateTime]::UtcNow.ToString('o')
    Map = $map; Result = 'PLAN_ONLY'; Mode = $Mode; Slot = $Slot
    Executable = $exe; Arguments = $arguments; Endpoint = "127.0.0.1:$Port"
    Details = 'Localhost only. File presence is not save validity; Unreal validates on load. No SQL or audible voice. Confirm IpNetDriver listening and successful load in Saved/Logs before joining. Normal logout can save; no timed autosave or crash-save guarantee.'
}
if ($Launch) {
    if (@(Get-Process UnrealEditor,UnrealEditor-Cmd -ErrorAction SilentlyContinue).Count) { throw 'Close existing Unreal editors before starting this private test' }
    if (@(Get-NetUDPEndpoint -LocalPort $Port -ErrorAction SilentlyContinue).Count) { throw 'UDP port already occupied' }
    $process = Start-Process -FilePath $exe -ArgumentList ($arguments -join ' ') -WorkingDirectory $root -WindowStyle Hidden -PassThru
    $plan.Result = 'PROCESS_STARTED_NOT_VERIFIED'
    $plan.ProcessId = $process.Id
}
[pscustomobject]$plan | ConvertTo-Json -Depth 4
