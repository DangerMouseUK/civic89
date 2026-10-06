# SPDX-License-Identifier: GPL-3.0-or-later
[CmdletBinding()]
param([Parameter(Mandatory)][string]$Path, [Parameter(Mandatory)][string]$Thumbprint,
    [Parameter(Mandatory)][string]$SignTool, [string]$TimestampUrl='https://timestamp.digicert.com')
$ErrorActionPreference='Stop'
if ($Thumbprint -notmatch '^[0-9a-fA-F]{40}$' -or $TimestampUrl -notmatch '^https://[^"$\s]+$') { throw 'Invalid signing configuration.' }
$certificate=Get-Item -LiteralPath "Cert:\CurrentUser\My\$Thumbprint"
if (!$certificate.HasPrivateKey -or $certificate.NotAfter -le (Get-Date) -or
    !($certificate.EnhancedKeyUsageList.ObjectId -contains '1.3.6.1.5.5.7.3.3')) { throw 'A valid code-signing identity is required.' }
& $SignTool sign /sha1 $Thumbprint /s My /fd SHA256 /tr $TimestampUrl /td SHA256 $Path
if ($LASTEXITCODE -ne 0) { throw 'Authenticode signing failed.' }
& $SignTool verify /pa /all $Path
if ($LASTEXITCODE -ne 0) { throw 'Authenticode verification failed.' }
$signature=Get-AuthenticodeSignature -LiteralPath $Path
if ($signature.Status -ne 'Valid' -or $signature.SignerCertificate.Thumbprint -ne $Thumbprint -or !$signature.TimeStamperCertificate) {
    throw 'Signer identity or RFC3161 timestamp verification failed.'
}
