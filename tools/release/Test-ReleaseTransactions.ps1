# Real package transaction/failure acceptance. SPDX-License-Identifier: GPL-3.0-or-later
[CmdletBinding()]
param([Parameter(Mandatory)][string]$DeliveryDirectory, [string]$OutputDirectory='out/audit')
. (Join-Path $PSScriptRoot 'ReleaseCommon.ps1')
function Require([bool]$Condition,[string]$Message) { if (!$Condition) { throw $Message } }
function Require-Failure([scriptblock]$Action) {
    $failed=$false
    try { & $Action | Out-Null } catch { $failed=$true }
    Require $failed 'Expected failure was accepted.'
}
$root=Join-Path ([IO.Path]::GetFullPath($OutputDirectory)) ('m6-transactions-' + [guid]::NewGuid())
New-Item -ItemType Directory -Path $root -Force | Out-Null
$delivery=Get-Content -LiteralPath (Join-Path $DeliveryDirectory 'delivery.json') -Raw | ConvertFrom-Json
$name="civic89-$($delivery.version)-windows-$($delivery.architecture)"
$archive=Join-Path ([IO.Path]::GetFullPath($DeliveryDirectory)) "$name.zip"
$hash=(Get-FileHash -LiteralPath $archive -Algorithm SHA256).Hash
$install=Join-Path $root 'portable'
$update=Join-Path $PSScriptRoot 'Update-Portable.ps1'
& $update -InstallRoot $install -Archive $archive -ExpectedSha256 $hash
$first=Read-PortablePointer $install
$sentinels=@('User city.cty','User city.c89') | ForEach-Object { Join-Path $install $_ }
foreach ($sentinel in $sentinels) { [IO.File]::WriteAllText($sentinel,'user city must survive') }
& $update -InstallRoot $install -Archive $archive -ExpectedSha256 $hash
$second=Read-PortablePointer $install
Require ($second.target -ne $first.target -and $second.previous -eq $first.target) 'Second install lost rollback state.'
& $update -InstallRoot $install -Rollback
$rolled=Read-PortablePointer $install
Require ($rolled.target -eq $first.target -and $rolled.previous -eq $second.target) 'Rollback did not restore the earlier complete package.'
foreach ($sentinel in $sentinels) {
    Require ((Get-Content -LiteralPath $sentinel -Raw) -eq 'user city must survive') 'Update touched user data.'
}
$before=Get-Content -LiteralPath (Join-Path $install 'current.json') -Raw
Require-Failure { & $update -InstallRoot $install -Archive $archive -ExpectedSha256 ('0' * 64) }
Require ((Get-Content -LiteralPath (Join-Path $install 'current.json') -Raw) -eq $before) 'Hash rejection altered the active pointer.'

# Simulate a Windows sharing violation at the publication point. The full new
# package may remain staged, but the launcher must still select the old package.
$locked=[IO.File]::Open((Join-Path $install 'current.json'),[IO.FileMode]::Open,[IO.FileAccess]::Read,[IO.FileShare]::Read)
$generationsBefore=@(Get-ChildItem -LiteralPath (Join-Path $install 'versions') -Directory -Filter 'civic89-*').Count
try { Require-Failure { & $update -InstallRoot $install -Archive $archive -ExpectedSha256 $hash } }
finally { $locked.Dispose() }
if (@(Get-ChildItem -LiteralPath (Join-Path $install 'versions') -Directory -Filter 'civic89-*').Count -ne $generationsBefore + 1) {
    throw 'Sharing-violation acceptance did not reach pointer publication.'
}
Require ((Get-Content -LiteralPath (Join-Path $install 'current.json') -Raw) -eq $before) 'Failed publication altered the active pointer.'
$null=Test-ReleaseTree (Join-Path $install (Read-PortablePointer $install).target)

$damaged=Join-Path $root 'damaged'
Expand-ReleaseArchive $archive $damaged
[IO.File]::AppendAllText((Join-Path $damaged 'res/tools.json'),'tamper')
Require-Failure { $null=Test-ReleaseTree $damaged }
$badZip=Join-Path $root 'damaged.zip'
[IO.Compression.ZipFile]::CreateFromDirectory($damaged,$badZip)
$badHash=(Get-FileHash -LiteralPath $badZip -Algorithm SHA256).Hash
Require-Failure { & $update -InstallRoot $install -Archive $badZip -ExpectedSha256 $badHash }
Require ((Get-Content -LiteralPath (Join-Path $install 'current.json') -Raw) -eq $before) 'Damaged package altered the active pointer.'
if ([version]($delivery.version.Split('-')[0]) -ge [version]'0.9.0') {
    # Reject even internally consistent manifests that omit M9 provenance or ship user settings.
    foreach ($member in @('assets/graphics-catalogue.json','graphics.cfg')) {
        $variant=Join-Path $root ('graphics-' + [guid]::NewGuid())
        Expand-ReleaseArchive $archive $variant
        $memberPath=Join-Path $variant $member
        if ($member -eq 'graphics.cfg') { [IO.File]::WriteAllText($memberPath,"1 1`n") }
        else { Remove-Item -LiteralPath $memberPath }
        $manifest=Get-Content -LiteralPath (Join-Path $variant 'release-manifest.json') -Raw | ConvertFrom-Json
        $manifest.files=@(Get-ReleaseFiles $variant)
        Write-ReleaseJson (Join-Path $variant 'release-manifest.json') $manifest
        Require-Failure { $null=Test-ReleaseTree $variant }
    }
}
foreach ($path in @('../escape.txt','C:/escape.txt','folder\\escape.txt','CON.txt','folder/file.','res/tools.json:stream')) {
    $zipPath=Join-Path $root ('unsafe-' + [guid]::NewGuid() + '.zip')
    $zip=[IO.Compression.ZipFile]::Open($zipPath,[IO.Compression.ZipArchiveMode]::Create)
    try { $null=$zip.CreateEntry($path) } finally { $zip.Dispose() }
    Require-Failure { Expand-ReleaseArchive $zipPath (Join-Path $root ('extract-' + [guid]::NewGuid())) }
}
Require (!(Test-Path -LiteralPath (Join-Path $root 'escape.txt'))) 'ZIP traversal escaped the staging directory.'
$duplicate=Join-Path $root 'duplicate.zip'
$zip=[IO.Compression.ZipFile]::Open($duplicate,[IO.Compression.ZipArchiveMode]::Create)
try { $null=$zip.CreateEntry('COPYING'); $null=$zip.CreateEntry('copying') } finally { $zip.Dispose() }
Require-Failure { Expand-ReleaseArchive $duplicate (Join-Path $root 'duplicate-extract') }
foreach ($sentinel in $sentinels) {
    Require ((Get-Content -LiteralPath $sentinel -Raw) -eq 'user city must survive') 'Failed update touched user data.'
}
Write-Output 'Portable install, repeated delivery, rollback, locked publication, bad checksum, tampering and hostile ZIP paths passed.'
