# Read-only local inventory. This does not certify packaging, cloud, SQL or audio.
[CmdletBinding()]
param(
    [string]$ProjectRoot = (Split-Path -Parent $PSScriptRoot),
    [string]$EngineRoot = 'C:\Program Files\Epic Games\UE_5.8',
    [string]$BuildVolumePath = '',
    [switch]$CheckGitHubAccess
)
Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
$root = [IO.Path]::GetFullPath($ProjectRoot)
$versionFile = Join-Path $EngineRoot 'Engine\Build\Build.version'
if (!(Test-Path -LiteralPath (Join-Path $root 'PrimalFrontier.uproject') -PathType Leaf)) { throw 'Project file missing' }
if (!(Test-Path -LiteralPath $versionFile -PathType Leaf)) { throw 'Engine Build.version missing' }
$version = Get-Content -LiteralPath $versionFile -Raw | ConvertFrom-Json
$volumes = @(Get-CimInstance Win32_LogicalDisk -Filter 'DriveType=3' | ForEach-Object {
    [pscustomobject]@{ Volume = $_.DeviceID; FreeGiB = [math]::Round($_.FreeSpace / 1GB, 2) }
})
if (!$BuildVolumePath) { $BuildVolumePath = $root }
$buildVolume = [IO.Path]::GetPathRoot([IO.Path]::GetFullPath($BuildVolumePath)).TrimEnd('\')
$selectedVolume = @($volumes | Where-Object Volume -EQ $buildVolume)
$storage = if ($selectedVolume.Count -eq 1 -and $selectedVolume[0].FreeGiB -ge 400) { 'PLANNING_SPACE_AVAILABLE' } else { 'BLOCKED_SPACE' }
$access = 'NOT_CHECKED'
if ($CheckGitHubAccess) {
    if (Get-Command gh -ErrorAction SilentlyContinue) {
        # Only sanitized repository metadata. Never print tokens or full configuration.
        try {
            $null = & gh api repos/EpicGames/UnrealEngine --jq '.name' 2>$null
            $access = if ($LASTEXITCODE -eq 0) { 'PASS' } else { 'UNVERIFIED_AUTH_OR_ACCESS' }
        } catch { $access = 'UNVERIFIED_AUTH_OR_ACCESS' }
    } else { $access = 'GH_NOT_INSTALLED' }
}
$tools = [ordered]@{}
foreach ($name in @('git','gh','ssh','docker','psql','pg_dump')) {
    $tools[$name] = [bool](Get-Command $name -ErrorAction SilentlyContinue)
}
[pscustomobject]@{
    TestName = 'PF.Multiplayer.InfrastructureReadiness'
    TimestampUtc = [DateTime]::UtcNow.ToString('o')
    Map = 'No level required'
    Result = 'INVENTORY_ONLY'
    EngineVersion = "$($version.MajorVersion).$($version.MinorVersion).$($version.PatchVersion)"
    EngineChangelist = $version.Changelist
    EngineDistribution = $(if (Test-Path -LiteralPath (Join-Path $EngineRoot 'Engine\Build\InstalledBuild.txt')) { 'Installed; prior Server target refused' } else { 'Unknown; compile required' })
    ServerTargetPresent = Test-Path -LiteralPath (Join-Path $root 'Source\PrimalFrontierServer.Target.cs')
    SourceAccess = $access
    SourceBuildStorage = $storage
    SelectedBuildVolume = $buildVolume
    PlanningFreeGiB = 400
    Volumes = $volumes
    Tools = $tools
    RunningEditors = @(Get-Process UnrealEditor,UnrealEditor-Cmd -ErrorAction SilentlyContinue).Count
    Details = '400GiB is a conservative planning allowance, not a measured build minimum. No source fetched, service provisioned, firewall changed, database connected or microphone opened.'
} | ConvertTo-Json -Depth 5
