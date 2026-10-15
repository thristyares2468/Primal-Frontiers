# Rebuild the actual historical writer in a source-only temporary project.
# Never checks out old Content or downloads LFS/engine/services. Candidates do not replace frozen fixtures.
[CmdletBinding()]
param([string]$EngineRoot='C:\Program Files\Epic Games\UE_5.8')
Set-StrictMode -Version Latest
$ErrorActionPreference='Stop'
$projectRoot=Split-Path -Parent $PSScriptRoot
if(@(Get-Process UnrealEditor,UnrealEditor-Cmd -ErrorAction SilentlyContinue).Count){throw 'Close existing Unreal sessions before isolated writer verification'}
$revision=(& git -C $projectRoot rev-parse c3d221f).Trim()
if($LASTEXITCODE -ne 0 -or $revision -notmatch '^[a-f0-9]{40}$'){throw 'Historical writer revision missing'}
$stamp=[DateTime]::UtcNow.ToString('yyyyMMdd_HHmmssfff')+'_'+[Guid]::NewGuid().ToString('N').Substring(0,8)
$candidate=Join-Path $projectRoot ('Saved\AutomationReports\LegacyM8Writer_'+$stamp)
$tempBase=[IO.Path]::GetFullPath((Join-Path ([IO.Path]::GetTempPath()) 'PFWriterFixtures'))
$buildRoot=Join-Path $tempBase $stamp
if((Test-Path -LiteralPath $candidate) -or (Test-Path -LiteralPath $buildRoot)){throw 'Unique fixture destination already exists'}
$null=New-Item -ItemType Directory -Path $candidate
$null=New-Item -ItemType Directory -Path $buildRoot
$archive=Join-Path $candidate 'historical-source.zip'
& git -C $projectRoot archive --format=zip "--output=$archive" $revision Source Config Plugins/PrimalAgentTools PrimalFrontier.uproject
if($LASTEXITCODE -ne 0){throw 'Historical source export failed'}
Expand-Archive -LiteralPath $archive -DestinationPath $buildRoot
$null=New-Item -ItemType Directory -Path (Join-Path $buildRoot 'Content')
$writerPaths=@('Source/PrimalFrontier/Persistence/PFWorldSaveFormat.cpp','Source/PrimalFrontier/Persistence/PFWorldSaveData.h','Source/PrimalFrontier/Persistence/PFPlayerSaveFormat.cpp','Source/PrimalFrontier/Persistence/PFPlayerSaveFormat.h','Source/PrimalFrontier/Persistence/PFSaveFileStore.cpp','Source/PrimalFrontier/Inventory/PFItemCatalog.cpp','Source/PrimalFrontier/Building/PFBuildingCatalog.cpp','Source/PrimalFrontier/Crafting/PFCraftingCatalog.cpp','Source/PrimalFrontier/Creatures/PFCreatureCatalog.cpp')
$hashes=[ordered]@{}
foreach($path in $writerPaths){$hashes[$path]=(Get-FileHash -LiteralPath (Join-Path $buildRoot $path) -Algorithm SHA256).Hash.ToLowerInvariant()}
$harness=Join-Path $projectRoot 'Tests\Fixtures\LegacyM8Writer.cpp'
Copy-Item -LiteralPath $harness -Destination (Join-Path $buildRoot 'Source\PrimalFrontier\Tests\PFLegacyWriterExportTests.cpp')
# Temporary launch configuration only. Historical production source remains byte exact.
Add-Content -LiteralPath (Join-Path $buildRoot 'Config\DefaultEngine.ini') -Value "`n[/Script/EngineSettings.GameMapsSettings]`nEditorStartupMap=/Engine/Maps/Entry`nGameDefaultMap=/Engine/Maps/Entry`nGlobalDefaultGameMode=/Script/Engine.GameModeBase"
$buildLog=Join-Path $candidate 'WriterEditorBuild.log'
& (Join-Path $EngineRoot 'Engine\Build\BatchFiles\Build.bat') PrimalFrontierEditor Win64 Development "-Project=$buildRoot\PrimalFrontier.uproject" -WaitMutex -NoHotReloadFromIDE -MaxParallelActions=4 "-Log=$buildLog"
if($LASTEXITCODE -ne 0){throw "Historical writer build failed; evidence retained in $candidate"}
& (Join-Path $PSScriptRoot 'RunNativeAutomation.ps1') -ProjectRoot $buildRoot -EngineRoot $EngineRoot -TestFilter PF.Persistence.LegacyWriterExport -Label LegacyWriterExport -TimeoutSeconds 180
if($LASTEXITCODE -ne 0){throw "Historical writer test failed; temporary project/evidence retained at $buildRoot"}
$nativeSummary=Get-ChildItem -LiteralPath (Join-Path $buildRoot 'Saved\AutomationReports') -Filter run-summary.json -Recurse | Select-Object -First 1
$native=Get-Content -LiteralPath $nativeSummary.FullName -Raw | ConvertFrom-Json
if(!$native.Verdict.Passed -or $native.LogWarningLines -ne 0 -or $native.LogErrorLines -ne 0 -or $native.LogFatalOrEnsureCount -ne 0){throw "Historical writer raw log gate failed; temporary evidence retained at $buildRoot"}
foreach($path in $writerPaths){if((Get-FileHash -LiteralPath (Join-Path $buildRoot $path) -Algorithm SHA256).Hash.ToLowerInvariant() -ne $hashes[$path]){throw 'Historical production writer changed during generation'}}
$fixturePath=Join-Path $buildRoot 'Saved\AutomationReports\LegacyWriter\fixture.json'
$fixture=Get-Content -LiteralPath $fixturePath -Raw | ConvertFrom-Json
$payload=[Convert]::FromBase64String($fixture.payloadBase64);$envelope=[Convert]::FromBase64String($fixture.envelopeBase64)
$sha=[Security.Cryptography.SHA256]::Create()
try{$payloadHash=([BitConverter]::ToString($sha.ComputeHash($payload))).Replace('-','').ToLowerInvariant();$envelopeHash=([BitConverter]::ToString($sha.ComputeHash($envelope))).Replace('-','').ToLowerInvariant()}finally{$sha.Dispose()}
$md5=[Security.Cryptography.MD5]::Create()
try{$payloadMd5=([BitConverter]::ToString($md5.ComputeHash($payload))).Replace('-','').ToLowerInvariant();$envelopeMd5=([BitConverter]::ToString($md5.ComputeHash($envelope))).Replace('-','').ToLowerInvariant()}finally{$md5.Dispose()}
$manifest=[ordered]@{Fixture='SyntheticM8WorldV1';WriterCommit=$revision;Engine=(Get-Content -LiteralPath (Join-Path $EngineRoot 'Engine\Build\Build.version') -Raw | ConvertFrom-Json);SourceArchiveSha256=(Get-FileHash -LiteralPath $archive -Algorithm SHA256).Hash.ToLowerInvariant();ProductionSourcesSha256=$hashes;HarnessSha256=(Get-FileHash -LiteralPath $harness -Algorithm SHA256).Hash.ToLowerInvariant();PayloadSha256=$payloadHash;EnvelopeSha256=$envelopeHash;PayloadMd5=$payloadMd5;EnvelopeMd5=$envelopeMd5;PayloadBytes=$payload.Length;EnvelopeBytes=$envelope.Length;SavedUtc=$fixture.savedUtc;CapturedUtc=$fixture.capturedUtc;Generation=$fixture.generation;Limit='Historical production source compiled now with installed engine and added synthetic harness, not an archived historical executable or private save. Temporary launch config uses Engine Entry; no historical Content.'}
$manifest | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath (Join-Path $candidate 'manifest.json') -Encoding UTF8
Copy-Item -LiteralPath $fixturePath -Destination (Join-Path $candidate 'fixture.json')
Copy-Item -LiteralPath (Join-Path $buildRoot 'Saved\AutomationReports') -Destination (Join-Path $candidate 'writer-reports') -Recurse
Copy-Item -LiteralPath (Join-Path $buildRoot 'Saved\Logs') -Destination (Join-Path $candidate 'writer-logs') -Recurse
# Validate the resolved owned target before any recursive cleanup. Never remove the temp base.
$resolved=(Resolve-Path -LiteralPath $buildRoot).Path
if(!$resolved.StartsWith($tempBase+[IO.Path]::DirectorySeparatorChar,[StringComparison]::OrdinalIgnoreCase) -or [IO.Path]::GetFileName($resolved) -ne $stamp){throw 'Refuse cleanup outside the explicitly created writer root'}
Remove-Item -LiteralPath $resolved -Recurse -Force
[pscustomobject]@{Passed=$true;WriterCommit=$revision;Candidate=$candidate;PayloadBytes=$payload.Length;EnvelopeBytes=$envelope.Length;TemporaryProjectRemoved=$true} | ConvertTo-Json
