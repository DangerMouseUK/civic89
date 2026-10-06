# Trusted signing stage, separate from unprivileged CI. SPDX-License-Identifier: GPL-3.0-or-later
[CmdletBinding()]
param([Parameter(Mandatory)][string]$DeliveryDirectory, [Parameter(Mandatory)][string]$ExpectedCommit,
    [Parameter(Mandatory)][string]$OutputDirectory, [Parameter(Mandatory)][string]$Iscc,
    [Parameter(Mandatory)][string]$SignTool, [Parameter(Mandatory)][string]$Thumbprint,
    [string]$TimestampUrl='https://timestamp.digicert.com', [switch]$Public)
. (Join-Path $PSScriptRoot 'ReleaseCommon.ps1')
$repo=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$DeliveryDirectory=[IO.Path]::GetFullPath($DeliveryDirectory)
$OutputDirectory=[IO.Path]::GetFullPath($OutputDirectory)
if ($ExpectedCommit -notmatch '^[a-f0-9]{40}$') { throw 'Expected immutable commit is required.' }
$delivery=Get-Content -LiteralPath (Join-Path $DeliveryDirectory 'delivery.json') -Raw | ConvertFrom-Json
if ($delivery.schema -ne 1 -or $delivery.product -ne 'Civic 89' -or $delivery.commit -ne $ExpectedCommit) { throw 'Unexpected delivery identity.' }
Assert-ReleaseVersion $delivery.version
if ($Public) {
    $gates=Get-Content -LiteralPath (Join-Path $repo 'packaging/release-gates.json') -Raw | ConvertFrom-Json
    if (@($gates.gates | Where-Object { !$_.passed }).Count) { throw 'Public release gates remain unresolved.' }
}
$name="civic89-$($delivery.version)-windows-$($delivery.architecture)"
$archive=Join-Path $DeliveryDirectory "$name.zip"
$sourceName="civic89-$($delivery.version)-source.zip"
foreach ($filename in @("$name.zip",$sourceName)) {
    $line=@(Get-Content -LiteralPath (Join-Path $DeliveryDirectory 'SHA256SUMS.txt') | Where-Object { $_.EndsWith("  $filename") })
    if ($line.Count -ne 1 -or $line[0] -notmatch '^([a-f0-9]{64})  ') { throw 'Missing delivery checksum.' }
    if ((Get-FileHash -LiteralPath (Join-Path $DeliveryDirectory $filename) -Algorithm SHA256).Hash -ne $Matches[1]) { throw 'Delivery integrity check failed.' }
}
if (Test-Path -LiteralPath $OutputDirectory) { throw 'Signing output must be a fresh directory.' }
New-Item -ItemType Directory -Path $OutputDirectory | Out-Null
$stage=Join-Path $OutputDirectory 'portable'
Expand-ReleaseArchive $archive $stage
$manifest=Test-ReleaseTree $stage
if ($manifest.commit -ne $ExpectedCommit -or $manifest.dirty -or $manifest.architecture -ne $delivery.architecture -or
    $manifest.version -ne $delivery.version) { throw 'Signed candidates require exact clean build identity.' }
# Authenticate the input through the trusted workflow/run, not through these
# checksums alone. Signing configuration/key material comes from the signing host.
& (Join-Path $PSScriptRoot 'Sign-ReleaseFile.ps1') -Path (Join-Path $stage 'civic89.exe') -Thumbprint $Thumbprint -SignTool $SignTool -TimestampUrl $TimestampUrl
$manifest.distribution=$(if ($Public) { 'public' } else { 'development' })
$manifest.files=@(Get-ReleaseFiles $stage | Sort-Object path)
Write-ReleaseJson (Join-Path $stage 'release-manifest.json') $manifest
$null=Test-ReleaseTree $stage
Invoke-PackagedSmoke $stage
[IO.Compression.ZipFile]::CreateFromDirectory($stage,(Join-Path $OutputDirectory "$name.zip"))
Copy-Item -LiteralPath (Join-Path $DeliveryDirectory $sourceName) -Destination $OutputDirectory
$signer=Join-Path $PSScriptRoot 'Sign-ReleaseFile.ps1'
if ($signer.Contains('$') -or $SignTool.Contains('$') -or $Thumbprint -notmatch '^[0-9a-fA-F]{40}$' -or $TimestampUrl -notmatch '^https://[^"$\s]+$') {
    throw 'Invalid signing command configuration.'
}
$command='pwsh.exe -NoProfile -File $q' + $signer + '$q -Thumbprint ' + $Thumbprint +
    ' -SignTool $q' + $SignTool + '$q -TimestampUrl $q' + $TimestampUrl + '$q -Path $f'
$compilerArgs=@("/DSourceDir=$stage","/DOutputDir=$OutputDirectory","/DAppVersion=$($manifest.version)",
    "/DAppArch=$($manifest.architecture)","/DOutputName=$name-setup",'/DSigned',('/Scivic89=' + $command))
& $Iscc @compilerArgs (Join-Path $repo 'packaging/civic89.iss')
if ($LASTEXITCODE -ne 0) { throw 'Signed installer compilation failed.' }
& $SignTool verify /pa /all (Join-Path $OutputDirectory "$name-setup.exe")
if ($LASTEXITCODE -ne 0) { throw 'Signed installer verification failed.' }
$sums=foreach ($filename in @("$name.zip","$name-setup.exe",$sourceName)) {
    "$((Get-FileHash -LiteralPath (Join-Path $OutputDirectory $filename) -Algorithm SHA256).Hash.ToLowerInvariant())  $filename"
}
[IO.File]::WriteAllLines((Join-Path $OutputDirectory 'SHA256SUMS.txt'),$sums,[Text.UTF8Encoding]::new($false))
$delivery.distribution=$manifest.distribution
Write-ReleaseJson (Join-Path $OutputDirectory 'delivery.json') $delivery
Write-Output "Sealed $($manifest.version) $($manifest.architecture) at $OutputDirectory"
