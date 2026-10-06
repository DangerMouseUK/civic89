# Verify the packaged static DLL closure. SPDX-License-Identifier: GPL-3.0-or-later
[CmdletBinding()]
param([Parameter(Mandatory)][string]$Directory,[Parameter(Mandatory)][string]$DumpTool)
$ErrorActionPreference='Stop'
$Directory=[IO.Path]::GetFullPath($Directory)
foreach ($file in Get-ChildItem -LiteralPath $Directory -File | Where-Object { $_.Extension -in @('.exe','.dll') }) {
    $imports=& $DumpTool /dump /dependents $file.FullName
    if ($LASTEXITCODE -ne 0) { throw "Cannot inspect imports for $($file.Name)" }
    foreach ($line in $imports) {
        if ($line -notmatch '^\s+([A-Za-z0-9_.-]+\.dll)\s*$') { continue }
        $dependency=$Matches[1]
        if (Test-Path -LiteralPath (Join-Path $Directory $dependency)) { continue }
        if ($dependency -match '^(api-ms-|ext-ms-)') { continue }
        # Release CRT must be app-local even if the workstation has a global copy.
        if ($dependency -match '^(vcruntime|msvcp|concrt|vccorlib)\d' -or
            !(Test-Path -LiteralPath (Join-Path "$env:SystemRoot/System32" $dependency))) {
            throw "Missing app-local DLL: $($file.Name) imports $dependency"
        }
    }
}
Write-Output 'Static DLL closure resolved entirely to packaged libraries and Windows system APIs.'
