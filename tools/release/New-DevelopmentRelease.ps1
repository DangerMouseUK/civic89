# Stage verified deliveries as a private GitHub draft. SPDX-License-Identifier: GPL-3.0-or-later
[CmdletBinding()]
param(
    [Parameter(Mandatory)][string[]]$DeliveryDirectory,
    [Parameter(Mandatory)][string]$ExpectedCommit,
    [Parameter(Mandatory)][string]$Tag,
    [Parameter(Mandatory)][string]$NotesFile,
    [Parameter(Mandatory)][string]$OutputDirectory,
    [string]$Repository = 'DangerMouseUK/civic89',
    [switch]$VerifyOnly
)
. (Join-Path $PSScriptRoot 'ReleaseCommon.ps1')
$PSNativeCommandUseErrorActionPreference = $true
if ($ExpectedCommit -notmatch '^[a-f0-9]{40}$' -or $Tag -notmatch '^playtest-[a-z0-9]+(?:-[a-z0-9]+)*$' -or
    $Repository -notmatch '^[A-Za-z0-9_.-]+/[A-Za-z0-9_.-]+$') { throw 'Invalid commit, playtest tag or repository.' }
$notes = Get-Content -LiteralPath $NotesFile -Raw
if ([string]::IsNullOrWhiteSpace($notes)) { throw 'Release notes are required.' }
$OutputDirectory = [IO.Path]::GetFullPath($OutputDirectory)
if (Test-Path -LiteralPath $OutputDirectory) { throw 'Release output must be a fresh directory.' }
New-Item -ItemType Directory -Path $OutputDirectory | Out-Null

function Get-SourceInventory([string]$Archive, [string]$Version) {
    $zip = [IO.Compression.ZipFile]::OpenRead($Archive)
    try {
        $inventory = @{}; [long]$bytes = 0
        if ($zip.Entries.Count -gt 10000) { throw 'Source archive has too many entries.' }
        foreach ($entry in $zip.Entries) {
            $path = $entry.FullName.TrimEnd('/')
            Assert-RelativeReleasePath $path
            if (!$path.StartsWith("civic89-$Version/", [StringComparison]::Ordinal) -and $path -ne "civic89-$Version") {
                throw 'Wrong source archive root.'
            }
            if ($inventory.ContainsKey($path)) { throw 'Duplicate source archive path.' }
            $bytes += $entry.Length
            if ($bytes -gt 268435456 -or ($entry.ExternalAttributes -shr 16 -band 0xf000) -eq 0xa000) {
                throw 'Oversized source archive or source symlink.'
            }
            $stream = $entry.Open()
            try { $inventory[$path] = [Convert]::ToHexString([Security.Cryptography.SHA256]::HashData($stream)) }
            finally { $stream.Dispose() }
        }
        $entry = $zip.GetEntry("civic89-$Version/source-provenance.json")
        if (!$entry -or $entry.Length -gt 4096) { throw 'Missing or oversized source provenance.' }
        $reader = [IO.StreamReader]::new($entry.Open())
        try { $provenance = $reader.ReadToEnd() | ConvertFrom-Json }
        finally { $reader.Dispose() }
        if ($provenance.schema -ne 1 -or $provenance.product -ne 'Civic 89' -or
            $provenance.version -ne $Version -or $provenance.commit -ne $ExpectedCommit) { throw 'Wrong source identity.' }
        return $inventory
    } finally { $zip.Dispose() }
}

$architectures = @{}; $sourceInventory = $null; $version = $null
$assets = [Collections.Generic.List[string]]::new()
foreach ($directory in $DeliveryDirectory) {
    $directory = [IO.Path]::GetFullPath($directory)
    $delivery = Get-Content -LiteralPath (Join-Path $directory 'delivery.json') -Raw | ConvertFrom-Json
    if ($delivery.schema -ne 1 -or $delivery.product -ne 'Civic 89' -or $delivery.commit -ne $ExpectedCommit -or
        $delivery.distribution -ne 'development' -or $delivery.architecture -notin @('x64','arm64')) { throw 'Wrong development delivery identity.' }
    Assert-ReleaseVersion $delivery.version
    if ($version -and $version -ne $delivery.version) { throw 'Delivery versions differ.' }
    $version = $delivery.version
    if ($architectures.ContainsKey($delivery.architecture)) { throw 'Duplicate delivery architecture.' }
    $architectures[$delivery.architecture] = $true
    $name = "civic89-$version-windows-$($delivery.architecture)"
    $sourceName = "civic89-$version-source.zip"
    $required = @("$name.zip", "$name-setup.exe", $sourceName)
    $checksums = @{}
    foreach ($line in Get-Content -LiteralPath (Join-Path $directory 'SHA256SUMS.txt')) {
        if ($line -notmatch '^([a-f0-9]{64})  ([^/\\]+)$') { throw 'Invalid delivery checksum record.' }
        $hash = $Matches[1]; $filename = $Matches[2]
        if ($filename -notin $required -or $checksums.ContainsKey($filename)) { throw 'Unexpected or duplicate delivery checksum.' }
        $checksums[$filename] = $hash
    }
    if ($checksums.Count -ne $required.Count) { throw 'Missing delivery checksum.' }
    foreach ($filename in $required) {
        if ((Get-FileHash -LiteralPath (Join-Path $directory $filename) -Algorithm SHA256).Hash -ne $checksums[$filename]) {
            throw "Delivery checksum mismatch: $filename"
        }
    }
    $tree = Join-Path $OutputDirectory "verify-$($delivery.architecture)"
    Expand-ReleaseArchive (Join-Path $directory "$name.zip") $tree
    $manifest = Test-ReleaseTree $tree
    if ($manifest.commit -ne $ExpectedCommit -or $manifest.dirty -or $manifest.version -ne $version -or
        $manifest.architecture -ne $delivery.architecture -or $manifest.distribution -ne 'development') { throw 'Wrong portable build identity.' }
    $currentSource = Get-SourceInventory (Join-Path $directory $sourceName) $version
    if ($null -eq $sourceInventory) {
        $sourceInventory = $currentSource
        Copy-Item -LiteralPath (Join-Path $directory $sourceName) -Destination $OutputDirectory
        $assets.Add((Join-Path $OutputDirectory $sourceName))
    } else {
        # Native runners can produce different ZIP compression/headers. Compare
        # every source member before sharing one corresponding-source archive.
        if ($currentSource.Count -ne $sourceInventory.Count) { throw 'Architecture source inventories differ.' }
        foreach ($path in $sourceInventory.Keys) {
            if (!$currentSource.ContainsKey($path) -or $currentSource[$path] -ne $sourceInventory[$path]) {
                throw "Architecture sources differ: $path"
            }
        }
    }
    foreach ($filename in @("$name.zip", "$name-setup.exe")) {
        Copy-Item -LiteralPath (Join-Path $directory $filename) -Destination $OutputDirectory
        $assets.Add((Join-Path $OutputDirectory $filename))
    }
    $metadata = Join-Path $OutputDirectory "delivery-$($delivery.architecture).json"
    Write-ReleaseJson $metadata $delivery
    $assets.Add($metadata)
}
if (!$version) { throw 'At least one delivery is required.' }
$metadata = Join-Path $OutputDirectory 'release.json'
Write-ReleaseJson $metadata ([ordered]@{schema=1;product='Civic 89';version=$version;commit=$ExpectedCommit;
    tag=$Tag;distribution='development';draft=$true;architectures=@($architectures.Keys | Sort-Object)})
$assets.Add($metadata)
$sums = foreach ($asset in $assets | Sort-Object) {
    "$((Get-FileHash -LiteralPath $asset -Algorithm SHA256).Hash.ToLowerInvariant())  $([IO.Path]::GetFileName($asset))"
}
$sumFile = Join-Path $OutputDirectory 'SHA256SUMS.txt'
[IO.File]::WriteAllLines($sumFile, $sums, [Text.UTF8Encoding]::new($false))
$assets.Add($sumFile)
$body = "**Draft development build for personal testing. Unsigned; required physical acceptance and public-release gates remain pending.**`n`n" +
    "Build: ``$version``; commit: ``$ExpectedCommit``; architectures: $(@($architectures.Keys | Sort-Object) -join ', ').`n`n" + $notes
$bodyFile = Join-Path $OutputDirectory 'release-notes.md'
[IO.File]::WriteAllText($bodyFile, $body, [Text.UTF8Encoding]::new($false))
if ($VerifyOnly) { Write-Output "Verified $($assets.Count) release assets: $OutputDirectory"; return }

# A draft is visible to repository writers, not an anonymous public download.
# Authenticate the CI run separately in the workflow; this script never executes
# downloaded binaries and never publishes, replaces assets or moves an old tag.
gh api "repos/$Repository/git/commits/$ExpectedCommit" --silent
$existing = @(gh api "repos/$Repository/releases" --paginate --jq '.[].tag_name')
if ($Tag -in $existing) { throw 'Release tag already has a release; choose a new playtest tag.' }
$refs = @(gh api "repos/$Repository/git/matching-refs/tags/$Tag" | ConvertFrom-Json)
$exact = @($refs | Where-Object { $_.ref -eq "refs/tags/$Tag" })
if ($exact.Count) {
    if ($exact[0].object.type -ne 'commit' -or $exact[0].object.sha -ne $ExpectedCommit) { throw 'Existing tag points elsewhere.' }
} else {
    gh api --method POST "repos/$Repository/git/refs" -f "ref=refs/tags/$Tag" -f "sha=$ExpectedCommit" --silent
}
gh release create $Tag @assets --repo $Repository --verify-tag --draft --prerelease --latest=false `
    --title "Civic 89 $version - $Tag" --notes-file $bodyFile
