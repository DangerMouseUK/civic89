# Civic 89

A native Windows city-building game modernising the open-source Micropolis / original
SimCity simulation lineage. Civic 89 keeps the inherited simulation and adds a modern
C++20 / SDL3 desktop interface. It is an independent project, unaffiliated with EA or Maxis.

![Civic 89 single-window interface](tests/baseline/M8_ORIGINAL_GAMEPLAY.png)

## What works

- One window with a dashboard, construction tools, minimap and budget/evaluation/history panels.
- Mouse-wheel zoom, camera panning, windowed/maximised/borderless display and DPI-aware layout.
- Traffic, crime, pollution, value, population, power and protection overlays.
- Adjustable UI scale, larger text, high contrast, configurable keys and audio volumes.
- Eight inherited scenarios, native file pickers, atomic city saves and ordinary-city autosave recovery.
- A separate SDL-free engine, headless runner and Classic simulation regression tests.
- One original-gameplay path, with `.cty` and versioned `.c89` saves and explicit import/export.
- Classic graphics and optional sharper pixel edges, switchable during play in Settings.

Civic 89's agreed scope is faithful original gameplay with modern Windows
presentation; gameplay expansion is outside scope. Existing M7 `.c89` identities
remain supported and use the same mechanics and map dimensions as `.cty` cities.

## Download the beta

[**Download Civic 89 0.9.0-beta.1**](https://github.com/DangerMouseUK/civic89/releases/tag/v0.9.0-beta.1)
from the public GitHub prerelease. No developer tools or GitHub write access are needed.

| Your Windows 11 PC | Installer | Portable ZIP |
|---|---|---|
| Intel / AMD (x64) | [x64 installer](https://github.com/DangerMouseUK/civic89/releases/download/v0.9.0-beta.1/civic89-0.9.0-beta.1-windows-x64-setup.exe) | [x64 ZIP](https://github.com/DangerMouseUK/civic89/releases/download/v0.9.0-beta.1/civic89-0.9.0-beta.1-windows-x64.zip) |
| ARM (ARM64) | [ARM64 installer](https://github.com/DangerMouseUK/civic89/releases/download/v0.9.0-beta.1/civic89-0.9.0-beta.1-windows-arm64-setup.exe) | [ARM64 ZIP](https://github.com/DangerMouseUK/civic89/releases/download/v0.9.0-beta.1/civic89-0.9.0-beta.1-windows-arm64.zip) |

Run the installer, or extract the **whole ZIP** and double-click `civic89.exe`.
Keep all DLLs and asset folders together. Matching source and SHA-256 checksums
are on the same release page.

**This is an unsigned testing beta.** Windows may display an unknown-publisher
warning. Physical desktop/device acceptance is pending; please read the
[testing instructions and known limits](packaging/BETA_RELEASE_NOTES.md) and
[report bugs](https://github.com/DangerMouseUK/civic89/issues).

## Status and compatibility

M0–M9 software work is merged, including M8 Windows polish and M9 optional
graphics. No further numbered milestone is currently agreed; remaining work is
real-machine testing, fixes, provenance follow-ups and trusted signing for stable
distribution. Native x64/ARM64 builds and packaging pass CI; this does not establish
physical hardware support. Other operating systems are unsupported.

The owner approved brand review and public beta publication after the asset
investigation, and deferred signing/physical acceptance for the beta. Exact OpenSVG
pack attribution and two legacy source-only image origins remain follow-ups;
the [asset investigation](project/ASSET_LICENSE_AUDIT.md) records the evidence.
See [release instructions](project/RELEASING.md), [beta policy](project/decisions/0010_PUBLIC_BETA.md)
and [current engineering status](PROJECT_STATUS.md).

**Save compatibility:** current 51,360-byte `.cty` files and supplied scenarios are
tested. Older 27,120-byte city files are unsupported. RNG, sprites and scenario progress
are not serialised, so loading is not an exact replay checkpoint. Automatic recovery
covers ordinary cities; explicit scenario exports reload as ordinary cities.

Classic v1 saves remain `.cty`. Enhanced v1 uses a checksummed `.c89` container that
records the ruleset and city name around the same ordinary-city snapshot. Opening
a save retains its recorded identity. F7 starts a new original-gameplay city.
The dashboard's **Files** panel offers **Open city**, **Save city**, **Import .cty copy**
and **Export .cty copy**. Import preserves the source and creates a copy saved as
`.c89`; export leaves the active city and save destination unchanged. Unknown versions
and corrupt files are rejected before replacing the city. See the
[Classic contract](project/CLASSIC_COMPATIBILITY.md), [Enhanced format](project/ENHANCED_CITY_FORMAT.md)
and [faithfulness audit](project/FAITHFULNESS_AUDIT.md).

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

**Overlays:** select a data layer using the dashboard's **No overlay** button.
The adjacent **Opacity** percentage controls how strongly that layer covers the
map; use **− / +** to adjust it. It has no visible effect while **No overlay** is
selected, and does not change taxes, funding or simulation speed.

**Graphics:** Settings → Readability → **Graphics: Classic/Enhanced** switches without
reloading your city. Classic is the default. Enhanced refines the original pixel edges
at twice their source resolution, retaining the palette and all tiles, sprite frames
and construction footprints; it is not a newly illustrated building set. The choice
persists in `graphics.cfg`, separately from city saves and M8's `ui.cfg`. It does not
change gameplay, simulation/animation timing or save bytes, and is independent of
the older Enhanced save-format identity. See the [graphics specification](project/GRAPHICS_SPECIFICATION.md)
and [complete input catalogue](assets/graphics-catalogue.json).

The status bar identifies original gameplay; Files shows the current save format.
The existing `--mode classic|enhanced` option is retained for save-format compatibility.
Both identities use the same mechanics; Classic is the default and the eight
inherited scenarios use it. New cities started through F7 use `.cty`.

Preferences, diagnostics and ordinary-city recovery live in
`%APPDATA%\Civic89\Civic89`. User-selected saves stay where you chose them.
Installer removal and portable updates preserve these files. Save As is required
after opening a city. A failed Save As retains the previous save destination.
Classic and Enhanced autosaves occupy separate slots; startup offers the newest
valid recovery and identifies its file format.

## Development and licensing

Deliver one complete [roadmap milestone](project/engineering/08_ROADMAP_AND_IMPLEMENTATION_BACKLOG.md)
per branch/PR. Preserve original gameplay and mechanics, and prove compatibility with tests.
The [scope decision](project/decisions/0009_FAITHFUL_MODERNISATION_AND_GRAPHICS.md)
records the graphics-only enhancement boundary and retained M7 save compatibility.
The exact upstream baseline and full history are preserved under `upstream-sdlpp-baseline`.

Civic 89 inherits GNU GPL v3 and the additional terms in [__README_OG](__README_OG).
Read [COPYING](COPYING), [NOTICE.md](NOTICE.md), [AUTHORS.md](AUTHORS.md) and the
[asset ledger](assets/ASSET-LICENSES.yml). Raleway remains under OFL; dependencies
have separate notices. No unlicensed retail assets or original retail sound recordings are added.

Thanks to Will Wright, Don Hopkins, Leeor Dicker and the upstream contributors.
The [original SDLPP README](project/reference/README_SDLPP.md) and
[provenance record](project/reference/UPSTREAMS.md) document the lineage.
