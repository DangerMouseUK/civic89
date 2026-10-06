# SPDX-License-Identifier: GPL-3.0-or-later
[CmdletBinding()]
param([Parameter(Mandatory)][string]$InstallRoot)
. (Join-Path $PSScriptRoot 'ReleaseCommon.ps1')
$InstallRoot=[IO.Path]::GetFullPath($InstallRoot)
$pointer=Read-PortablePointer $InstallRoot
$directory=Join-Path $InstallRoot $pointer.target
$null=Test-ReleaseTree $directory
Start-Process -FilePath (Join-Path $directory 'civic89.exe') -WorkingDirectory $directory -WindowStyle Normal
