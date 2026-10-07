Download the **windows-x64.zip** for an Intel/AMD Windows 11 PC, or
**windows-arm64.zip** for an ARM Windows 11 PC. Extract the **whole ZIP** into a
folder, then double-click **civic89.exe**. Visual Studio, vcpkg, SDL and PowerShell
are not needed to play. The matching **setup.exe** is an alternative per-user
installer; use either the portable ZIP or the installer.

Keep the DLLs, `res`, `images` and `scenarios` folders beside the executable.
F7 starts a city. The dashboard's Files panel opens/saves cities and imports/exports
copies. Start with a fresh city or a copy of a current save.

This is faithful original gameplay with modern Windows presentation. Existing
51,360-byte `.cty` saves and M7 `.c89` saves remain supported; historical 27,120-byte
city files are unsupported. Scenario exports reopen as ordinary cities. No gameplay
expansion is included. In M9 (`0.9.0-dev`) candidates, Settings → Readability →
Graphics switches between original Classic pixels and optional 2x edge-refined
Enhanced pixels. All original buildings, palettes, footprints and animation frames
remain. This preference is independent of city/save identity and does not convert
saves. M8 (`0.8.0-dev`) candidates retain their original graphics.

Please try launching and playing, saving/reopening a city, cancelling the native
file picker, hearing audio, Alt+Tab/minimise/restore, resizing/fullscreen and readable
UI scale on your actual screen(s). Report the Windows version, architecture, screen
resolution/scaling, game version and steps to reproduce any issue. Physical checks remain pending. Public beta approval and stable-release
follow-ups are documented in project/decisions/0010_PUBLIC_BETA.md; this template
still describes a private development draft. Windows may show a warning
because this testing build is unsigned; check the release identity before choosing
to run it.

For M9, compare both graphics options while playing the same city. Check animated
buildings, moving sprites, construction previews, minimap and overlays at different
zoom levels; switch repeatedly, save/reopen and try fullscreen. At 1080p and 4K,
report stutters or unreadable details and include your graphics adapter. M8/M9 are now merged; their physical acceptance remains pending. The public
beta gathers that evidence without claiming the checks have passed.

`SHA256SUMS.txt` covers the packages, matching source and identity files. The source
ZIP corresponds to the embedded build commit. This draft is for the repository
owner/collaborators; sign into that GitHub account on the other machine to download.
It is not a public release and is not marked Latest.
