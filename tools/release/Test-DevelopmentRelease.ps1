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
