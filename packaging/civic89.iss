; Civic 89 installer. SPDX-License-Identifier: GPL-3.0-or-later
#ifndef SourceDir
  #error SourceDir is required
#endif
#ifndef AppVersion
  #error AppVersion is required
#endif
[Setup]
AppId={{A892DA65-E29E-4C10-B294-18D2F37ACB06}
AppName=Civic 89
AppVersion={#AppVersion}
AppPublisher=Civic 89 contributors
AppPublisherURL=https://github.com/DangerMouseUK/civic89
AppSupportURL=https://github.com/DangerMouseUK/civic89/issues
DefaultDirName={localappdata}\Programs\Civic 89\{#AppArch}
DefaultGroupName=Civic 89
PrivilegesRequired=lowest
MinVersion=10.0.22000
#if AppArch == "arm64"
ArchitecturesAllowed=arm64
ArchitecturesInstallIn64BitMode=arm64
#else
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
#endif
OutputDir={#OutputDir}
OutputBaseFilename={#OutputName}
SetupIconFile=..\assets\branding\civic89.ico
UninstallDisplayIcon={app}\civic89.exe
LicenseFile={#SourceDir}\COPYING
InfoBeforeFile={#SourceDir}\NOTICE.md
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
CloseApplications=yes
RestartApplications=no
Uninstallable=yes
#ifdef Signed
SignTool=civic89
SignedUninstaller=yes
#else
SignedUninstaller=no
#endif

[Tasks]
Name: "desktopicon"; Description: "Create a desktop shortcut"; GroupDescription: "Shortcuts:"; Flags: unchecked

[Files]
Source: "{#SourceDir}\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\Civic 89"; Filename: "{app}\civic89.exe"; WorkingDir: "{app}"
Name: "{autodesktop}\Civic 89"; Filename: "{app}\civic89.exe"; WorkingDir: "{app}"; Tasks: desktopicon

; No user-data files, registry preferences, file associations or recursive
; uninstall deletion. User-created saves and %APPDATA%\Civic89 survive removal.
