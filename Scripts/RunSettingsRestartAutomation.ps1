# Real sequential game processes; first menu writes only its disposable INI.
[CmdletBinding()]
param([ValidateSet('720','1440')][string]$Resolution='720',
      [ValidateRange(60,600)][int]$TimeoutSeconds=180)
Set-StrictMode -Version Latest
$ErrorActionPreference='Stop'
$runner=Join-Path $PSScriptRoot 'RunControlsAutomation.ps1'
$prepare=& $runner -TestCase SettingsRestartPrepare -Resolution $Resolution -TimeoutSeconds $TimeoutSeconds
$prepareCode=$LASTEXITCODE
$prepared=($prepare -join "`n") | ConvertFrom-Json
if($prepareCode -ne 0){$prepare;exit $prepareCode}
# The first process has exited and strict exact-selection/hash guards passed.
$verify=& $runner -TestCase SettingsRestartVerify -Resolution $Resolution -SettingsSourceRun $prepared.Run -TimeoutSeconds $TimeoutSeconds
$verifyCode=$LASTEXITCODE
$verified=($verify -join "`n") | ConvertFrom-Json
$summary=[ordered]@{Passed=($prepareCode -eq 0 -and $verifyCode -eq 0);Preparation=$prepared;FreshProcess=$verified;RunnerExitCode=$verifyCode}
$summary | ConvertTo-Json -Depth 10 | Set-Content -LiteralPath (Join-Path $prepared.Report 'settings-restart-summary.json') -Encoding UTF8
$summary | ConvertTo-Json -Depth 10
exit $verifyCode
