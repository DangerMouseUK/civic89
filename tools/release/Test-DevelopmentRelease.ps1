# Real-delivery publication acceptance. SPDX-License-Identifier: GPL-3.0-or-later
[CmdletBinding()]
param([Parameter(Mandatory)][string[]]$DeliveryDirectory,
    [Parameter(Mandatory)][string]$OutputDirectory)
. (Join-Path $PSScriptRoot 'ReleaseCommon.ps1')
$OutputDirectory = [IO.Path]::GetFullPath($OutputDirectory)
if (Test-Path -LiteralPath $OutputDirectory) { throw 'Test output must be fresh.' }
New-Item -ItemType Directory -Path $OutputDirectory | Out-Null
$notes = Join-Path $PSScriptRoot '../../packaging/DEVELOPMENT_RELEASE_NOTES.md'
$delivery = Get-Content -LiteralPath (Join-Path $DeliveryDirectory[0] 'delivery.json') -Raw | ConvertFrom-Json
$parameters = @{ExpectedCommit=$delivery.commit;Tag='playtest-acceptance';NotesFile=$notes;VerifyOnly=$true}
$publisher = Join-Path $PSScriptRoot 'New-DevelopmentRelease.ps1'
& $publisher @parameters -DeliveryDirectory $DeliveryDirectory -OutputDirectory (Join-Path $OutputDirectory 'valid')

function Assert-Rejected([string]$Name, [scriptblock]$Action, [string]$Reason) {
    try { & $Action | Out-Null }
    catch {
        if (!$_.Exception.Message.Contains($Reason)) { throw }
        Write-Output "Rejected $Name"
        return
    }
    throw "Accepted invalid release: $Name"
}
function Copy-Delivery([string]$Name) {
    $copy = Join-Path $OutputDirectory $Name
    New-Item -ItemType Directory -Path $copy | Out-Null
    Get-ChildItem -LiteralPath $DeliveryDirectory[0] -File | Copy-Item -Destination $copy
    return $copy
}

$wrong = $parameters.Clone(); $wrong.ExpectedCommit = '0000000000000000000000000000000000000000'
Assert-Rejected 'wrong commit' { & $publisher @wrong -DeliveryDirectory $DeliveryDirectory[0] `
    -OutputDirectory (Join-Path $OutputDirectory 'wrong-commit') } 'Wrong development delivery identity'
Assert-Rejected 'duplicate architecture' { & $publisher @parameters -DeliveryDirectory @($DeliveryDirectory[0],$DeliveryDirectory[0]) `
    -OutputDirectory (Join-Path $OutputDirectory 'duplicate-arch') } 'Duplicate delivery architecture'

# Beta approval is scoped to one version and cannot turn a stable/dev tag into a beta.
$policy = (Get-Content -LiteralPath (Join-Path $PSScriptRoot '../../packaging/release-gates.json') -Raw | ConvertFrom-Json).beta
Assert-BetaReleasePolicy $policy.version "v$($policy.version)" $policy
foreach ($field in @('approved','brandReviewApproved','assetInvestigationReviewed','unsignedAccepted','physicalAcceptanceDeferred')) {
    $unapproved = $policy | ConvertTo-Json | ConvertFrom-Json
    $unapproved.$field = $false
    Assert-Rejected "missing beta decision: $field" { Assert-BetaReleasePolicy $policy.version "v$($policy.version)" $unapproved } 'lacks the recorded owner decisions'
}
Assert-Rejected 'approval for a different beta' { Assert-BetaReleasePolicy '0.9.0-beta.999' 'v0.9.0-beta.999' $policy } 'lacks the recorded owner decisions'
Assert-Rejected 'stable version using beta policy' { Assert-BetaReleasePolicy '0.9.0' 'v0.9.0' $policy } 'Beta tag must match'
if ($delivery.version -eq $policy.version) {
    $beta = $parameters.Clone(); $beta.Beta = $true; $beta.Tag = "v$($delivery.version)"
    & $publisher @beta -DeliveryDirectory $DeliveryDirectory -OutputDirectory (Join-Path $OutputDirectory 'valid-beta')
    $wrongTag = $beta.Clone(); $wrongTag.Tag = 'v0.9.0-beta.999'
    Assert-Rejected 'beta tag/version mismatch' { & $publisher @wrongTag -DeliveryDirectory $DeliveryDirectory `
        -OutputDirectory (Join-Path $OutputDirectory 'wrong-beta-tag') } 'Beta tag must match'
    if ($DeliveryDirectory.Count -eq 1) {
        $upload = $beta.Clone(); $upload.Remove('VerifyOnly')
        Assert-Rejected 'incomplete beta architecture upload' { & $publisher @upload -DeliveryDirectory $DeliveryDirectory `
            -OutputDirectory (Join-Path $OutputDirectory 'incomplete-beta') } 'requires x64 and ARM64'
    }
}

$tampered = Copy-Delivery 'tampered-input'
$setup = Get-ChildItem -LiteralPath $tampered -Filter '*-setup.exe'
[IO.File]::AppendAllText($setup.FullName, 'tampered')
Assert-Rejected 'tampered installer' { & $publisher @parameters -DeliveryDirectory $tampered `
    -OutputDirectory (Join-Path $OutputDirectory 'tampered-output') } 'Delivery checksum mismatch'

$missing = Copy-Delivery 'missing-input'
$sums = @(Get-Content -LiteralPath (Join-Path $missing 'SHA256SUMS.txt'))
[IO.File]::WriteAllLines((Join-Path $missing 'SHA256SUMS.txt'), $sums[0..1])
Assert-Rejected 'missing checksum' { & $publisher @parameters -DeliveryDirectory $missing `
    -OutputDirectory (Join-Path $OutputDirectory 'missing-output') } 'Missing delivery checksum'

$source = Copy-Delivery 'source-input'
$sourceName = "civic89-$($delivery.version)-source.zip"
$sourcePath = Join-Path $source $sourceName
$zip = [IO.Compression.ZipFile]::Open($sourcePath, [IO.Compression.ZipArchiveMode]::Update)
try {
    $entry = $zip.GetEntry("civic89-$($delivery.version)/source-provenance.json")
    $writer = [IO.StreamWriter]::new($entry.Open())
    try { $writer.BaseStream.SetLength(0); $writer.Write('{"schema":1,"product":"Civic 89","version":"' + $delivery.version + '","commit":"' + ('0' * 40) + '"}') }
    finally { $writer.Dispose() }
} finally { $zip.Dispose() }
$hash = (Get-FileHash -LiteralPath $sourcePath -Algorithm SHA256).Hash.ToLowerInvariant()
$sums = @(Get-Content -LiteralPath (Join-Path $source 'SHA256SUMS.txt') | ForEach-Object {
    if ($_.EndsWith("  $sourceName")) { "$hash  $sourceName" } else { $_ }
})
[IO.File]::WriteAllLines((Join-Path $source 'SHA256SUMS.txt'), $sums)
Assert-Rejected 'mismatched source with valid outer checksum' { & $publisher @parameters -DeliveryDirectory $source `
    -OutputDirectory (Join-Path $OutputDirectory 'source-output') } 'Wrong source identity'
Write-Output 'Development release verification and rejection acceptance passed.'
