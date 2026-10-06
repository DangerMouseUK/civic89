# Windows per-user install/upgrade/uninstall acceptance. SPDX-License-Identifier: GPL-3.0-or-later
[CmdletBinding()]
param([Parameter(Mandatory)][string]$DeliveryDirectory,[string]$OutputDirectory='out/audit')
. (Join-Path $PSScriptRoot 'ReleaseCommon.ps1')
$DeliveryDirectory=[IO.Path]::GetFullPath($DeliveryDirectory)
$delivery=Get-Content -LiteralPath (Join-Path $DeliveryDirectory 'delivery.json') -Raw | ConvertFrom-Json
$setup=Join-Path $DeliveryDirectory "civic89-$($delivery.version)-windows-$($delivery.architecture)-setup.exe"
# Never adopt or uninstall a user's existing installed Civic 89.
$key='HKCU:\Software\Microsoft\Windows\CurrentVersion\Uninstall\{A892DA65-E29E-4C10-B294-18D2F37ACB06}_is1'
if (Test-Path -LiteralPath $key) { throw 'Existing Civic 89 installer registration found; acceptance requires a clean test user.' }
$root=Join-Path ([IO.Path]::GetFullPath($OutputDirectory)) ('m6-installer-' + [guid]::NewGuid())
New-Item -ItemType Directory -Path $root -Force | Out-Null
$destination=Join-Path $root 'installed'
try {
    foreach ($iteration in 1..2) {
        $process=Start-Process -FilePath $setup -ArgumentList @('/VERYSILENT','/SUPPRESSMSGBOXES','/NORESTART','/SP-',"/DIR=`"$destination`"",'/NOICONS',"/LOG=`"$(Join-Path $root "install-$iteration.log")`"") -WindowStyle Hidden -Wait -PassThru
        if ($process.ExitCode -ne 0) { throw "Installer failed: $($process.ExitCode)" }
        $null=Test-ReleaseTree -Root $destination -Installed $true
        Invoke-PackagedSmoke $destination
        if ($iteration -eq 1) { [IO.File]::WriteAllText((Join-Path $destination 'user-save.cty'),'preserve me') }
        if ((Get-Content -LiteralPath (Join-Path $destination 'user-save.cty') -Raw) -ne 'preserve me') { throw 'Upgrade changed the user city.' }
    }
    $uninstaller=Join-Path $destination 'unins000.exe'
    $process=Start-Process -FilePath $uninstaller -ArgumentList @('/VERYSILENT','/SUPPRESSMSGBOXES','/NORESTART',"/LOG=`"$(Join-Path $root 'uninstall.log')`"") -WindowStyle Hidden -Wait -PassThru
    if ($process.ExitCode -ne 0 -or (Test-Path -LiteralPath (Join-Path $destination 'civic89.exe')) -or
        !(Test-Path -LiteralPath (Join-Path $destination 'user-save.cty')) -or (Test-Path -LiteralPath $key)) { throw 'Uninstall did not remove binaries/preserve the user file.' }
    Write-Output 'Per-user install, upgrade, installed-app launch and uninstall/user-file retention passed.'
} finally {
    # On failure remove only this acceptance installation through its uninstaller.
    $uninstaller=Join-Path $destination 'unins000.exe'
    if (Test-Path -LiteralPath $uninstaller) {
        $null=Start-Process -FilePath $uninstaller -ArgumentList @('/VERYSILENT','/SUPPRESSMSGBOXES','/NORESTART') -WindowStyle Hidden -Wait -PassThru
    }
}
