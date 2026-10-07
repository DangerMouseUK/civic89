# Civic 89 0.9.0-beta.2

This testing beta fixes settings-save errors reported in beta.1 and clarifies the
dashboard's overlay-opacity percentage. It includes the merged M8 Windows polish
and M9 optional graphics, preserving the original gameplay and mechanics.

## Changes since beta.1

- Settings can be saved after relaunching with an existing `audio.cfg`. The startup
  reader now closes before Windows replaces the preferences file.
- The dashboard percentage reads **Opacity**, with descriptive hover text and
  help. It controls how strongly the selected data overlay covers the map and has
  no visible effect with **No overlay** selected.
- Regression checks cover existing settings, repeated volume saves and persisted
  opacity. Simulation, city formats, artwork and dependencies are unchanged.

## Download and play

Choose **windows-x64-setup.exe** for an Intel/AMD Windows 11 PC, or
**windows-arm64-setup.exe** for an ARM Windows 11 PC. The installer installs
for your Windows account without requiring developer tools. Alternatively,
download the matching **windows-x64.zip** or **windows-arm64.zip**, extract
the whole ZIP to a folder and double-click **civic89.exe**. Keep all DLLs and
asset folders together. PowerShell, Visual Studio and vcpkg are not needed to play.

**This beta is unsigned.** Windows may show an unknown-publisher/SmartScreen
warning. Check that you downloaded from DangerMouseUK/civic89 on GitHub before
deciding to run it. SHA256SUMS.txt checks download integrity; it is not a digital
signature or a substitute for trusting the download source.

To update from beta.1, save your city and close the game. Run the new installer,
or extract the whole new ZIP to a **fresh folder**. Preferences remain in
`%APPDATA%\Civic89\Civic89`; keep any city files you saved in the old game folder.
There is no automatic network updater. Beta.1 downloads remain unchanged.

## What to test

- Change **Opacity** beside the overlay selector, select Traffic or Pollution,
  then restart and confirm the percentage was retained. No overlay means there
  is no coloured data layer to fade. Check that no settings-save error appears.
- Change Settings → Sound volumes, close/relaunch and confirm they were retained.
- Start a city with F7 or choose one of the eight scenarios with F6. Build roads,
  zones and utilities, change speed (1–4), pause (Space) and try budget/history.
- Save and reopen a **copy** of a city; cancel the file picker and try Unicode
  filenames. Test ordinary-city recovery after closing/restarting the game.
- Resize, Alt+Tab, minimise/restore, toggle borderless with F11, and try UI scale,
  larger text and high contrast on your actual monitor(s), including mixed DPI.
- Check audible effects, volume/mute and keyboard/panel navigation.
- In Settings → Readability switch **Graphics: Classic/Enhanced** while playing.
  Compare buildings, sprites, previews, overlays and minimap at different zooms.
  Enhanced sharpens the original pixel edges; buildings and mechanics stay the same.
  Report stutters and unreadable details, especially at 1080p and 4K.

Report problems through [GitHub Issues](https://github.com/DangerMouseUK/civic89/issues).
Include the game version/commit (`civic89.exe --version`), Windows version,
Intel/AMD or ARM, graphics adapter, screen resolution/scaling, graphics option,
steps to reproduce and expected/actual behaviour. Attach diagnostics from
`%APPDATA%\Civic89\Civic89` when useful, checking them for personal paths first.

## Known limits and release status

- Physical display/audio/mixed-DPI acceptance is pending; this beta gathers that
  evidence. Automated native x64/ARM64 checks do not prove every device works.
  Frame-rate targets on physical GPUs remain unverified.
- Supported city files are current 51,360-byte `.cty` and known M7 `.c89` saves.
  Older 27,120-byte cities are rejected. Saves do not preserve exact RNG/sprite/
  scenario progress; scenario exports reopen as ordinary cities. Back up real saves.
- Keyboard navigation, high contrast and larger text are supported; a Windows
  UI Automation tree and screen-reader support are not implemented.
- There is no automatic network updater. Install/uninstall preserves user saves
  and preferences; portable users should extract a new version to a new folder.
- The owner approved brand review and this unsigned public beta after the asset
  investigation. Original XPM art/cities have direct open-source release evidence;
  fonts are unmodified OFL Raleway. Exact OpenSVG pack attribution for three UI
  images and the origins of two legacy source-only branding resources remain
  follow-ups. No prohibited icon or confirmed licence incompatibility was found;
  a complete per-icon rights audit is not claimed. See the
  [asset investigation](https://github.com/DangerMouseUK/civic89/blob/v0.9.0-beta.2/project/ASSET_LICENSE_AUDIT.md)
  and [beta.2 decision](https://github.com/DangerMouseUK/civic89/blob/v0.9.0-beta.2/project/decisions/0011_BETA_2_SETTINGS_FIXES.md).

The matching source ZIP and inherited GPL/additional terms accompany the binaries.
Release metadata identifies the exact clean build commit. This is a prerelease,
not a stable signed release, and is excluded from Latest.
