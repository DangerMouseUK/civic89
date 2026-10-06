# SPDX-License-Identifier: GPL-3.0-or-later
[CmdletBinding()]
param([Parameter(Mandatory)][string]$Directory, [switch]$NoSmoke)
. (Join-Path $PSScriptRoot 'ReleaseCommon.ps1')
$manifest=Test-ReleaseTree $Directory
if (!$NoSmoke) { Invoke-PackagedSmoke $Directory }
Write-Output "Verified $($manifest.version) $($manifest.architecture): $(@($manifest.files).Count) files."
