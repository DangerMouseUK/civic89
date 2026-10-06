# Civic 89 offline portable update transaction. SPDX-License-Identifier: GPL-3.0-or-later
[CmdletBinding(DefaultParameterSetName='Install')]
param(
    [Parameter(Mandatory)][string]$InstallRoot,
    [Parameter(Mandatory,ParameterSetName='Install')][string]$Archive,
    [Parameter(Mandatory,ParameterSetName='Install')][ValidatePattern('^[a-fA-F0-9]{64}$')][string]$ExpectedSha256,
    [Parameter(Mandatory,ParameterSetName='Rollback')][switch]$Rollback
)
. (Join-Path $PSScriptRoot 'ReleaseCommon.ps1')
$InstallRoot=[IO.Path]::GetFullPath($InstallRoot)
if ($InstallRoot -eq [IO.Path]::GetPathRoot($InstallRoot)) { throw 'Choose a dedicated installation directory.' }
New-Item -ItemType Directory -Path $InstallRoot -Force | Out-Null
if ((Get-Item -LiteralPath $InstallRoot).Attributes -band [IO.FileAttributes]::ReparsePoint) { throw 'Install root cannot be a reparse point.' }
# Serialize updates, including the first install and rollback. A crash releases the lock.
$lock=[IO.File]::Open((Join-Path $InstallRoot 'update.lock'),[IO.FileMode]::OpenOrCreate,[IO.FileAccess]::ReadWrite,[IO.FileShare]::None)
try {
    $pointer=Join-Path $InstallRoot 'current.json'
    $current=$null
    if (Test-Path -LiteralPath $pointer) { $current=Read-PortablePointer $InstallRoot }
    if ($Rollback) {
        if (!$current -or !$current.previous) { throw 'There is no previous release to restore.' }
        $null=Test-ReleaseTree (Join-Path $InstallRoot $current.previous)
        $target=$current.previous
    } else {
        $Archive=[IO.Path]::GetFullPath($Archive)
        if ((Get-FileHash -LiteralPath $Archive -Algorithm SHA256).Hash -ne $ExpectedSha256) { throw 'Downloaded archive checksum mismatch.' }
        $versions=Join-Path $InstallRoot 'versions'
        New-Item -ItemType Directory -Path $versions -Force | Out-Null
        if ((Get-Item -LiteralPath $versions).Attributes -band [IO.FileAttributes]::ReparsePoint) { throw 'Versions directory cannot be a reparse point.' }
        $stage=Join-Path $versions ('.stage-' + [guid]::NewGuid())
        Expand-ReleaseArchive $Archive $stage
        $manifest=Test-ReleaseTree $stage
        $native=[Runtime.InteropServices.RuntimeInformation]::OSArchitecture.ToString()
        if ($manifest.architecture -eq 'arm64' -and $native -ne 'Arm64') { throw 'ARM64 package requires ARM64 Windows.' }
        # Launch before publication; the existing release remains selected on failure.
        Invoke-PackagedSmoke $stage
        $generation=[guid]::NewGuid().ToString('N').Substring(0,8)
        $target="versions/civic89-$($manifest.version)-$($manifest.architecture)-$generation"
        $destination=Join-Path $InstallRoot $target
        if (Test-Path -LiteralPath $destination) { throw 'The release generation already exists.' }
        Move-Item -LiteralPath $stage -Destination $destination
    }
    $state=[ordered]@{schema=1;product='Civic 89';target=$target;previous=$(if ($current) { $current.target } else { $null })}
    $temporary=Join-Path $InstallRoot ('current-' + [guid]::NewGuid() + '.tmp')
    Write-ReleaseJson $temporary $state
    # Flushed same-directory pointer publication. A crash leaves either old or new
    # complete package selected; neither user saves nor the previous release move.
    $stream=[IO.File]::Open($temporary,[IO.FileMode]::Open,[IO.FileAccess]::ReadWrite,[IO.FileShare]::None)
    try { $stream.Flush($true) } finally { $stream.Dispose() }
    if ($current) { [IO.File]::Replace($temporary,$pointer,(Join-Path $InstallRoot 'current.backup.json')) }
    else { [IO.File]::Move($temporary,$pointer) }
    Write-Output "Selected $target. Previous release and user data are preserved."
} finally { $lock.Dispose() }
