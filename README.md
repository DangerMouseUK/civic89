# Civic 89

A native Windows city-building game modernising the open-source Micropolis / original
SimCity simulation lineage. Civic 89 keeps the inherited simulation and adds a modern
C++20 / SDL3 desktop interface. It is an independent project, unaffiliated with EA or Maxis.

![Civic 89 single-window interface](tests/baseline/M5_UI_CITY.png)

## What works

- One window with a dashboard, construction tools, minimap and budget/evaluation/history panels.
- Mouse-wheel zoom, camera panning, windowed/maximised/borderless display and DPI-aware layout.
- Traffic, crime, pollution, value, population, power and protection overlays.
- Adjustable UI scale, larger text, high contrast, configurable keys and audio volumes.
- Eight inherited scenarios, native file pickers, atomic city saves and ordinary-city autosave recovery.
- A separate SDL-free engine, headless runner and Classic simulation regression tests.
- Classic v1 and Enhanced v1 city modes, versioned Enhanced saves and explicit Classic import/export.

Enhanced v1 establishes the mode and save foundation and currently uses Classic
mechanics and map dimensions. Larger maps and other Enhanced gameplay remain future work.

## Status and compatibility

This is development software (`0.7.0-dev`), with no public release yet. Release tooling
produces portable ZIPs, per-user installers, matching source archives and checksums.
Public publication remains gated on asset rights, brand review, signing and physical
desktop acceptance. See [release instructions](project/RELEASING.md) and
[current engineering status](PROJECT_STATUS.md).

Windows 11 x64 is the primary target. Native Windows ARM64 builds, tests and packaging
also pass CI; desktop/hardware support must be validated before release. Other platforms are unsupported.

**Save compatibility:** current 51,360-byte `.cty` files and the supplied scenarios are
tested. Older 27,120-byte city files are unsupported. RNG, sprites and scenario progress
are not serialised, so loading is not an exact replay checkpoint. Automatic recovery
covers ordinary cities; explicit scenario exports reload as ordinary cities.

Classic v1 saves remain `.cty`. Enhanced v1 uses a checksummed `.c89` container that
records the ruleset and city name around the same ordinary-city snapshot. Opening
a save selects its recorded mode. F7 offers new-city mode selection, explicit
**Import Classic to Enhanced**, and **Export Classic copy**; import preserves the
source and export leaves the active mode unchanged. Unknown versions and corrupt
files are rejected before replacing the current city. See the
[Classic contract](project/CLASSIC_COMPATIBILITY.md) and
[Enhanced format](project/ENHANCED_CITY_FORMAT.md).

## Build and run

Install Visual Studio 2026 C++ tools and CMake **4.2+**. Use an external vcpkg checkout
at the manifest baseline `19780d9cdf84d0944cf9a318666703b89ab6629c`; bootstrap it with
`bootstrap-vcpkg.bat -disableMetrics`. Do not put vcpkg inside this repository.

From the repository root in PowerShell, with CMake/CTest on `PATH`:

```powershell
$env:VCPKG_ROOT = 'C:\Dev\vcpkg'
cmake --preset windows-x64-release
cmake --build --preset windows-x64-release -- /m
ctest --preset windows-x64-release --output-on-failure
./out/build/windows-x64-release/bin/Release/civic89.exe
```

Assets are staged beside the executable, and launches work from any working directory.
Dependencies and fonts are pinned; no assets are downloaded at game startup.
See [BUILDING.md](project/BUILDING.md) for Debug, ASan, ARM64, headless tools and the
retained Visual Studio comparison build.

## Controls and user data

Wheel zooms; right-drag or arrow keys pan; Home centres the camera. Space pauses,
1-4 set speed, F2 saves (Shift+F2 selects a destination), F3 opens, F4 toggles the
minimap, F5 evaluation, F6 scenarios, F7 new city, F8/F12 settings, F9 history,
F10 budget and F11 borderless fullscreen. Tab/Shift+Tab and Enter navigate panels.
Tool and command keys can be reassigned in Settings.

The status bar identifies the active ruleset. To start in Enhanced mode from the
command line, add `--mode enhanced`; `--mode classic` is the default. The eight
inherited scenarios always use Classic mode.

Preferences, diagnostics and ordinary-city recovery live in
`%APPDATA%\Civic89\Civic89`. User-selected saves stay where you chose them.
Installer removal and portable updates preserve these files. Save As is required
after opening a city. Classic and Enhanced autosaves occupy separate slots;
startup offers the newest valid recovery and identifies its mode.

## Development and licensing

Deliver one complete [roadmap milestone](project/engineering/08_ROADMAP_AND_IMPLEMENTATION_BACKLOG.md)
per branch/PR. Keep simulation changes separate and prove compatibility with tests.
The exact upstream baseline and full history are preserved under `upstream-sdlpp-baseline`.

Civic 89 inherits GNU GPL v3 and the additional terms in [__README_OG](__README_OG).
Read [COPYING](COPYING), [NOTICE.md](NOTICE.md), [AUTHORS.md](AUTHORS.md) and the
[asset ledger](assets/ASSET-LICENSES.yml). Raleway remains under OFL; dependencies
have separate notices. No unlicensed retail assets or original retail sound recordings are added.

Thanks to Will Wright, Don Hopkins, Leeor Dicker and the upstream contributors.
The [original SDLPP README](project/reference/README_SDLPP.md) and
[provenance record](project/reference/UPSTREAMS.md) document the lineage.
