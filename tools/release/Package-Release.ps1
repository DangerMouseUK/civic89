# SPDX-License-Identifier: GPL-3.0-or-later
[CmdletBinding()]
param(
    [string]$BuildDirectory = 'out/build/windows-x64-release',
    [string]$OutputDirectory = 'out/releases',
    [string]$Iscc,
    [switch]$Public,
    [string]$SigningThumbprint,
    [string]$SignTool,
    [string]$TimestampUrl = 'https://timestamp.digicert.com'
)
. (Join-Path $PSScriptRoot 'ReleaseCommon.ps1')
$repo = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$BuildDirectory=[IO.Path]::GetFullPath($BuildDirectory)
$OutputDirectory=[IO.Path]::GetFullPath($OutputDirectory)
$build = Get-Content -LiteralPath (Join-Path $BuildDirectory 'build-info.json') -Raw | ConvertFrom-Json
Assert-ReleaseVersion $build.version
$cache=Get-Content -LiteralPath (Join-Path $BuildDirectory 'CMakeCache.txt') -Raw
if ($cache -notmatch '(?m)^CMAKE_CONFIGURATION_TYPES:[^=]+=Release\r?$' -or $cache -match '(?m)^CIVIC89_ENABLE_ASAN:BOOL=ON') {
    throw 'Package only a Release build with AddressSanitizer disabled.'
}
if ($Public) {
    $gates=Get-Content -LiteralPath (Join-Path $repo 'packaging/release-gates.json') -Raw | ConvertFrom-Json
    $pending=@($gates.gates | Where-Object { !$_.passed })
    if ($pending.Count) { throw "Public release gates are unresolved: $($pending.id -join ', ')" }
    if (!$SigningThumbprint -or $build.dirty) { throw 'Public packaging requires a clean signed build.' }
}
if ($SigningThumbprint -and (!$SignTool -or $SigningThumbprint -notmatch '^[0-9a-fA-F]{40}$' -or $TimestampUrl -notmatch '^https://[^"$\s]+$')) {
    throw 'Signing requires SignTool, a 40-digit certificate thumbprint and HTTPS timestamp service.'
}
$revision=& git -C $repo rev-parse HEAD
if ($LASTEXITCODE -ne 0 -or $revision -ne $build.commit) { throw 'Reconfigure the build at the current commit before packaging.' }
$changes=& git -C $repo status --porcelain
if ($LASTEXITCODE -ne 0 -or $changes -or $build.dirty) { throw 'Commit all release inputs and reconfigure a clean build before packaging.' }
if (!$Iscc -or !(Test-Path -LiteralPath $Iscc)) { throw 'Pass the pinned Inno Setup ISCC.exe path to build the complete ZIP + installer deliverables.' }
$name="civic89-$($build.version)-windows-$($build.architecture)"
New-Item -ItemType Directory -Path $OutputDirectory -Force | Out-Null
$delivery=Join-Path $OutputDirectory $name
if (Test-Path -LiteralPath $delivery) { throw 'Release output already exists; use a new output directory.' }
$work=Join-Path $OutputDirectory ('.stage-' + [guid]::NewGuid())
New-Item -ItemType Directory -Path $work | Out-Null
$stage=Join-Path $work 'portable'
& cmake --install $BuildDirectory --config Release --prefix $stage
if ($LASTEXITCODE -ne 0) { throw 'CMake portable install failed.' }

# Bundle only the selected compiler's redistributable CRT, never System32/debug DLLs.
$instance=[regex]::Match($cache,'(?m)^CMAKE_GENERATOR_INSTANCE:[^=]+=([^\r\n]+)').Groups[1].Value.Split(',')[0]
if (!$instance) { throw 'Cannot locate the build compiler instance.' }
$crtVersion=(Get-Content -LiteralPath (Join-Path $instance 'VC/Auxiliary/Build/Microsoft.VCRedistVersion.default.txt') -Raw).Trim()
$crtBase=Join-Path $instance "VC/Redist/MSVC/$crtVersion/$($build.architecture)"
$crtDirectories=@(Get-ChildItem -LiteralPath $crtBase -Directory -Filter '*.CRT')
if ($crtDirectories.Count -ne 1) { throw 'Cannot select the matching MSVC CRT redistributable.' }
Copy-Item -Path (Join-Path $crtDirectories[0].FullName '*.dll') -Destination $stage
$crtNotice=Join-Path $stage 'licenses/msvc'
New-Item -ItemType Directory -Path $crtNotice -Force | Out-Null
Copy-Item -LiteralPath (Join-Path $instance 'Licenses/1033/Redist.txt') -Destination $crtNotice
Copy-Item -LiteralPath (Join-Path $repo 'packaging/MSVC-RUNTIME-NOTICE.md') -Destination $crtNotice
$toolVersion=(Get-Content -LiteralPath (Join-Path $instance 'VC/Auxiliary/Build/Microsoft.VCToolsVersion.default.txt') -Raw).Trim()
$hostArch=if ([Runtime.InteropServices.RuntimeInformation]::ProcessArchitecture.ToString() -eq 'Arm64') { 'arm64' } else { 'x64' }
$dumpTool=Join-Path $instance "VC/Tools/MSVC/$toolVersion/bin/Host$hostArch/$($build.architecture)/link.exe"
& (Join-Path $PSScriptRoot 'Test-PeDependencies.ps1') -Directory $stage -DumpTool $dumpTool

if ($SigningThumbprint) {
    & (Join-Path $PSScriptRoot 'Sign-ReleaseFile.ps1') -Path (Join-Path $stage 'civic89.exe') -Thumbprint $SigningThumbprint -SignTool $SignTool -TimestampUrl $TimestampUrl
}
$manifest=[ordered]@{
    schema=1; product='Civic 89'; version=$build.version; architecture=$build.architecture; commit=$build.commit;
    dirty=$build.dirty; compiler=$build.compiler; vcpkgBaseline=$build.vcpkgBaseline; crtVersion=$crtVersion;
    distribution=$(if ($Public) { 'public' } else { 'development' }); files=@(Get-ReleaseFiles $stage | Sort-Object path)
}
Write-ReleaseJson (Join-Path $stage 'release-manifest.json') $manifest
$null=Test-ReleaseTree $stage
Invoke-PackagedSmoke $stage

# Matching source at the exact commit is delivered alongside the binary artifacts.
$source=Join-Path $work "civic89-$($build.version)-source.zip"
& git -C $repo archive --format=zip "--prefix=civic89-$($build.version)/" "--output=$source" $build.commit
if ($LASTEXITCODE -ne 0) { throw 'Corresponding source archive failed.' }
$sourceZip=[IO.Compression.ZipFile]::Open($source,[IO.Compression.ZipArchiveMode]::Update)
try {
    $entry=$sourceZip.CreateEntry("civic89-$($build.version)/source-provenance.json")
    $entry.LastWriteTime=[DateTimeOffset]::Parse((& git -C $repo show -s --format=%cI $build.commit))
    $writer=[IO.StreamWriter]::new($entry.Open(),[Text.UTF8Encoding]::new($false))
    try { $writer.Write(([ordered]@{schema=1;product='Civic 89';version=$build.version;commit=$build.commit} | ConvertTo-Json) + "`n") }
    finally { $writer.Dispose() }
} finally { $sourceZip.Dispose() }
$zip=Join-Path $work "$name.zip"
[IO.Compression.ZipFile]::CreateFromDirectory($stage,$zip,[IO.Compression.CompressionLevel]::Optimal,$false)
# Verify and launch the extracted ZIP as well as the staging tree.
$extracted=Join-Path $work 'zip-check'
Expand-ReleaseArchive $zip $extracted
$null=Test-ReleaseTree $extracted
Invoke-PackagedSmoke $extracted

$installerArguments=@("/DSourceDir=$stage","/DOutputDir=$work","/DAppVersion=$($build.version)","/DAppArch=$($build.architecture)","/DOutputName=$name-setup")
if ($SigningThumbprint) {
    # Signing happens on the trusted signing host; keys never enter build artifacts.
    $signer=Join-Path $PSScriptRoot 'Sign-ReleaseFile.ps1'
    if ($signer.Contains('$') -or $SignTool.Contains('$')) { throw 'Unsupported signing tool path.' }
    $command='pwsh.exe -NoProfile -File $q' + $signer + '$q -Thumbprint ' + $SigningThumbprint +
        ' -SignTool $q' + $SignTool + '$q -TimestampUrl $q' + $TimestampUrl + '$q -Path $f'
    $installerArguments += @('/DSigned','/Scivic89=' + $command)
}
& $Iscc @installerArguments (Join-Path $repo 'packaging/civic89.iss')
if ($LASTEXITCODE -ne 0) { throw 'Installer compilation failed.' }
$setup=Join-Path $work "$name-setup.exe"
if ($SigningThumbprint) {
    & $SignTool verify /pa /all $setup
    if ($LASTEXITCODE -ne 0) { throw 'Installer signature verification failed.' }
}
$sums=@()
foreach ($file in @($zip,$setup,$source)) { $sums += "$((Get-FileHash -LiteralPath $file -Algorithm SHA256).Hash.ToLowerInvariant())  $([IO.Path]::GetFileName($file))" }
[IO.File]::WriteAllLines((Join-Path $work 'SHA256SUMS.txt'),$sums,[Text.UTF8Encoding]::new($false))
Write-ReleaseJson (Join-Path $work 'delivery.json') ([ordered]@{schema=1;product='Civic 89';version=$build.version;architecture=$build.architecture;commit=$build.commit;distribution=$manifest.distribution})
# Checks are retained for review. Publish the completed delivery directory with one rename.
Move-Item -LiteralPath $work -Destination $delivery
Write-Output "Packaged ZIP, installer, source and SHA-256 inventory: $delivery"
