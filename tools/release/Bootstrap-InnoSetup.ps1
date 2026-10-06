# SPDX-License-Identifier: GPL-3.0-or-later
[CmdletBinding()]
param([Parameter(Mandatory)][string]$Destination)
$ErrorActionPreference = 'Stop'
$Destination = [IO.Path]::GetFullPath($Destination)
if (Test-Path -LiteralPath (Join-Path $Destination 'ISCC.exe')) {
    throw 'Choose an empty tooling destination; the compiler must match the pinned download.'
}
New-Item -ItemType Directory -Path $Destination -Force | Out-Null
$installer = Join-Path $Destination 'innosetup-6.7.3.exe'
Invoke-WebRequest 'https://github.com/jrsoftware/issrc/releases/download/is-6_7_3/innosetup-6.7.3.exe' -OutFile $installer
if ((Get-FileHash -LiteralPath $installer -Algorithm SHA256).Hash -ne '9c73c3bae7ed48d44112a0f48e66742c00090bdb5bef71d9d3c056c66e97b732') {
    throw 'Pinned Inno Setup download checksum mismatch.'
}
$signature = Get-AuthenticodeSignature -LiteralPath $installer
if ($signature.Status -ne 'Valid' -or $signature.SignerCertificate.Subject -notmatch 'Pyrsys B.V.') {
    throw 'Inno Setup publisher signature verification failed.'
}
$process = Start-Process -FilePath $installer -ArgumentList @('/VERYSILENT','/SUPPRESSMSGBOXES','/NORESTART','/SP-','/CURRENTUSER',"/DIR=`"$Destination`"",'/NOICONS') -WindowStyle Hidden -Wait -PassThru
if ($process.ExitCode -ne 0 -or !(Test-Path -LiteralPath (Join-Path $Destination 'ISCC.exe'))) {
    throw "Inno Setup tool installation failed: $($process.ExitCode)"
}
Write-Output (Join-Path $Destination 'ISCC.exe')
