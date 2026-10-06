# Civic 89 packaging helpers. SPDX-License-Identifier: GPL-3.0-or-later
Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

function Assert-ReleaseVersion([string]$Version) {
    if ($Version -notmatch '^\d+\.\d+\.\d+(?:-[0-9A-Za-z]+(?:[.-][0-9A-Za-z]+)*)?$') { throw 'Invalid release version.' }
}

function Assert-RelativeReleasePath([string]$Path) {
    if (!$Path -or $Path.Contains('\') -or $Path.StartsWith('/') -or $Path.Length -gt 200) { throw "Unsafe package path: $Path" }
    foreach ($part in $Path.Split('/')) {
        if (!$part -or $part -in @('.','..') -or $part -match '[:<>"|?*\x00-\x1f]' -or
            $part -match '[. ]$' -or $part -match '^(CON|PRN|AUX|NUL|COM[0-9]|LPT[0-9])(?:\.|$)') {
            throw "Unsafe package path: $Path"
        }
    }
}

function Get-PeArchitecture([string]$Path) {
    $stream = [IO.File]::OpenRead($Path)
    $reader = [IO.BinaryReader]::new($stream)
    try {
        if ($reader.ReadUInt16() -ne 0x5a4d) { throw "Not a Windows executable: $Path" }
        $stream.Position = 0x3c
        $offset = $reader.ReadUInt32()
        if ($offset -gt $stream.Length - 6) { throw 'Invalid PE header offset.' }
        $stream.Position = $offset
        if ($reader.ReadUInt32() -ne 0x00004550) { throw 'Invalid PE signature.' }
        switch ($reader.ReadUInt16()) {
            0x8664 { return 'x64' }
            0xaa64 { return 'arm64' }
            default { throw "Unsupported PE architecture: $Path" }
        }
    } finally { $reader.Dispose(); $stream.Dispose() }
}

function Write-ReleaseJson([string]$Path, $Value) {
    [IO.File]::WriteAllText($Path, ($Value | ConvertTo-Json -Depth 12) + "`n", [Text.UTF8Encoding]::new($false))
}

function Get-ReleaseFiles([string]$Root) {
    $Root = [IO.Path]::GetFullPath($Root)
    foreach ($file in Get-ChildItem -LiteralPath $Root -Recurse -Force) {
        if ($file.Attributes -band [IO.FileAttributes]::ReparsePoint) { throw 'Package contains a reparse point.' }
        if ($file.PSIsContainer) { continue }
        $relative = [IO.Path]::GetRelativePath($Root, $file.FullName).Replace('\','/')
        Assert-RelativeReleasePath $relative
        if ($relative -eq 'release-manifest.json') { continue }
        [ordered]@{ path=$relative; bytes=$file.Length; sha256=(Get-FileHash -LiteralPath $file.FullName -Algorithm SHA256).Hash.ToLowerInvariant() }
    }
}

function Test-ReleaseTree([string]$Root, [bool]$Installed = $false) {
    $Root = [IO.Path]::GetFullPath($Root)
    if ((Get-Item -LiteralPath $Root).Attributes -band [IO.FileAttributes]::ReparsePoint) { throw 'Release root is a reparse point.' }
    $manifest = Get-Content -LiteralPath (Join-Path $Root 'release-manifest.json') -Raw | ConvertFrom-Json
    if ($manifest.schema -ne 1 -or $manifest.product -ne 'Civic 89' -or $manifest.architecture -notin @('x64','arm64') -or
        $manifest.distribution -notin @('development','public') -or $manifest.commit -notmatch '^[a-f0-9]{40}$') { throw 'Invalid release manifest identity.' }
    Assert-ReleaseVersion $manifest.version
    $expected = @{}
    foreach ($entry in $manifest.files) {
        Assert-RelativeReleasePath $entry.path
        if ($entry.path -eq 'release-manifest.json' -or $expected.ContainsKey($entry.path) -or
            $entry.sha256 -notmatch '^[a-f0-9]{64}$' -or $entry.bytes -lt 0) { throw 'Invalid or duplicate manifest entry.' }
        $expected[$entry.path] = $entry
    }
    $actual = @(Get-ReleaseFiles $Root)
    if ($Installed) {
        # Inno owns its generated uninstaller. A top-level user save is not an
        # application input; all original package members remain verified.
        $actual = @($actual | Where-Object {
            $expected.ContainsKey($_.path) -or ($_.path -notmatch '^unins\d{3}\.(exe|dat|msg)$' -and $_.path -notmatch '^[^/]+\.(cty|c89)$')
        })
    }
    if ($expected.Count -ne $actual.Count) { throw 'Package file count does not match manifest.' }
    foreach ($entry in $actual) {
        if (!$expected.ContainsKey($entry.path) -or $expected[$entry.path].sha256 -ne $entry.sha256 -or
            $expected[$entry.path].bytes -ne $entry.bytes) { throw "Package checksum mismatch: $($entry.path)" }
        if ([IO.Path]::GetExtension($entry.path) -in @('.exe','.dll')) {
            $actualArchitecture=Get-PeArchitecture (Join-Path $Root $entry.path)
            if ($actualArchitecture -ne $manifest.architecture) { throw "Mixed package architectures: $($entry.path) is $actualArchitecture; expected $($manifest.architecture)." }
            if ([IO.Path]::GetFileName($entry.path) -match '(_tests|runner|asan|140d|debug)') { throw 'Development binary in package.' }
        }
        if ($entry.path -match '(?i)(\.pdb$|\.(cty|c89)\.tmp|(^|/)(audio\.cfg|ui\.cfg|display\.cfg|autosave\.(cty|c89)|civic89\.log)$)') {
            throw 'Private/development files in package.'
        }
    }
    foreach ($required in @('civic89.exe','build-info.json','COPYING','__README_OG','NOTICE.md','AUTHORS.md','README.md',
        'res/tools.json','scenarios/snro.666','licenses/msvc/Redist.txt','vcruntime140.dll','msvcp140.dll','SDL3.dll')) {
        if (!$expected.ContainsKey($required)) { throw "Missing package requirement: $required" }
    }
    $build = Get-Content -LiteralPath (Join-Path $Root 'build-info.json') -Raw | ConvertFrom-Json
    if ($build.commit -ne $manifest.commit -or $build.version -ne $manifest.version -or $build.architecture -ne $manifest.architecture) {
        throw 'Package/build identity mismatch.'
    }
    $identity = (Get-Item -LiteralPath (Join-Path $Root 'civic89.exe')).VersionInfo
    if ($identity.ProductName -ne 'Civic 89' -or $identity.ProductVersion -ne $manifest.version -or
        $identity.FileVersion -ne "$($manifest.version)+$($manifest.commit.Substring(0,12))") { throw 'Wrong executable product/version/commit resources.' }
    return $manifest
}

function Invoke-PackagedSmoke([string]$Root) {
    $exe = Join-Path ([IO.Path]::GetFullPath($Root)) 'civic89.exe'
    # M6 deliveries have only the original smoke marker. Keep old-version install
    # and rollback usable while requiring the Enhanced path in M7 and later.
    $build = Get-Content -LiteralPath (Join-Path $Root 'build-info.json') -Raw | ConvertFrom-Json
    Assert-ReleaseVersion $build.version
    $requiresEnhanced = [version]($build.version.Split('-')[0]) -ge [version]'0.7.0'
    $testDirectory = Join-Path ([IO.Path]::GetTempPath()) ('civic89-launch-' + [guid]::NewGuid())
    New-Item -ItemType Directory -Path $testDirectory | Out-Null
    $oldVideo = $env:SDL_VIDEODRIVER; $oldRender = $env:SDL_RENDER_DRIVER; $oldAudio = $env:SDL_AUDIODRIVER
    try {
        $env:SDL_VIDEODRIVER='dummy'; $env:SDL_RENDER_DRIVER='software'; $env:SDL_AUDIODRIVER='dummy'
        # Exclude developer PATH DLLs. Package-local CRT/SDL must suffice.
        $oldPath=$env:Path; $env:Path="$env:SystemRoot\System32;$env:SystemRoot"
        try {
            $launch = @{ FilePath=$exe; ArgumentList='--smoke-test'; WorkingDirectory=$testDirectory; WindowStyle='Hidden'; PassThru=$true;
                RedirectStandardOutput=(Join-Path $testDirectory 'stdout.txt'); RedirectStandardError=(Join-Path $testDirectory 'stderr.txt') }
            $process = Start-Process @launch
            if (!$process.WaitForExit(60000)) { $process.Kill(); throw 'Packaged smoke test timed out.' }
            $process.WaitForExit()
            $output = Get-Content -LiteralPath (Join-Path $testDirectory 'stdout.txt') -Raw
            if ($process.ExitCode -ne 0 -or $output -notmatch 'Packaged startup, scenario, render, save/reload and shutdown passed' -or
                ($requiresEnhanced -and $output -notmatch 'Enhanced ruleset, tagged save/load and Classic export passed')) {
                $errors = Get-Content -LiteralPath (Join-Path $testDirectory 'stderr.txt') -Raw
                throw "Packaged smoke failed ($($process.ExitCode)): $errors"
            }
        } finally { $env:Path=$oldPath }
    } finally {
        $env:SDL_VIDEODRIVER=$oldVideo; $env:SDL_RENDER_DRIVER=$oldRender; $env:SDL_AUDIODRIVER=$oldAudio
        # A unique, explicitly checked task-owned directory; never user save data.
        if ([IO.Path]::GetFullPath($testDirectory).StartsWith([IO.Path]::GetTempPath(), [StringComparison]::OrdinalIgnoreCase) -and
            [IO.Path]::GetFileName($testDirectory).StartsWith('civic89-launch-')) { Remove-Item -LiteralPath $testDirectory -Recurse -Force }
    }
}

function Expand-ReleaseArchive([string]$Archive, [string]$Destination) {
    if (Test-Path -LiteralPath $Destination) { throw 'Archive destination must not exist.' }
    $zip=[IO.Compression.ZipFile]::OpenRead($Archive)
    try {
        $names=@{}; [long]$size=0
        if ($zip.Entries.Count -gt 4000) { throw 'Package has too many entries.' }
        foreach ($entry in $zip.Entries) {
            $name=$entry.FullName.TrimEnd('/')
            Assert-RelativeReleasePath $name
            if ($names.ContainsKey($name)) { throw 'Duplicate archive path.' }
            $names[$name]=$true
            if (($entry.ExternalAttributes -shr 16 -band 0xf000) -eq 0xa000) { throw 'Archive symlinks are not allowed.' }
            $size += $entry.Length
            if ($size -gt 268435456) { throw 'Package exceeds the 256 MiB expanded limit.' }
        }
        New-Item -ItemType Directory -Path $Destination | Out-Null
        $root=[IO.Path]::GetFullPath($Destination) + [IO.Path]::DirectorySeparatorChar
        foreach ($entry in $zip.Entries) {
            $target=[IO.Path]::GetFullPath((Join-Path $Destination $entry.FullName))
            if (!$target.StartsWith($root,[StringComparison]::OrdinalIgnoreCase)) { throw 'Archive escapes destination.' }
            if ($entry.FullName.EndsWith('/')) { New-Item -ItemType Directory -Path $target -Force | Out-Null; continue }
            New-Item -ItemType Directory -Path ([IO.Path]::GetDirectoryName($target)) -Force | Out-Null
            [IO.Compression.ZipFileExtensions]::ExtractToFile($entry,$target,$false)
        }
    } finally { $zip.Dispose() }
}

function Read-PortablePointer([string]$Root) {
    foreach ($path in @($Root,(Join-Path $Root 'versions'),(Join-Path $Root 'current.json'))) {
        if ((Get-Item -LiteralPath $path).Attributes -band [IO.FileAttributes]::ReparsePoint) { throw 'Portable installation contains a reparse point.' }
    }
    $state=Get-Content -LiteralPath (Join-Path $Root 'current.json') -Raw | ConvertFrom-Json
    if ($state.schema -ne 1 -or $state.product -ne 'Civic 89') { throw 'Invalid portable installation pointer.' }
    foreach ($target in @($state.target,$state.previous)) {
        if (!$target) { continue }
        Assert-RelativeReleasePath $target
        if ($target -notmatch '^versions/civic89-\d+\.\d+\.\d+(?:-[0-9A-Za-z]+(?:[.-][0-9A-Za-z]+)*)?-(x64|arm64)-[a-f0-9]{8}$') {
            throw 'Invalid portable release target.'
        }
        $directory=Join-Path $Root $target
        if ((Get-Item -LiteralPath $directory).Attributes -band [IO.FileAttributes]::ReparsePoint) { throw 'Portable target is a reparse point.' }
    }
    if (!$state.target) { throw 'Portable pointer has no selected release.' }
    return $state
}
