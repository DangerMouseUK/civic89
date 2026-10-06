# Civic 89 Windows application and headless engine build

Use Windows 11 x64 (primary) or ARM64 (secondary), Visual Studio 2026 C++ tools and CMake **4.2+**. The committed presets reproduce the inherited application using C++20/MSVC and explicit vcpkg manifest mode. Visual Studio can open the repository folder and use these presets. The primary workstation's bundled CMake is 4.3.1-msvc1; it selects the installed VS 2026 BuildTools instance without needing a developer shell.

## PowerShell setup

From `C:\Dev\Projects\civic89`:

```powershell
$env:VCPKG_ROOT = 'C:\Dev\vcpkg'
# In an ordinary terminal if CMake/CTest are not already on PATH:
$cmakeBin = 'C:\Program Files\Microsoft Visual Studio\18\Enterprise\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin'
$env:Path = "$cmakeBin;$env:Path"
```

For another machine, use an **external** vcpkg checkout at manifest baseline `19780d9cdf84d0944cf9a318666703b89ab6629c`, run `bootstrap-vcpkg.bat -disableMetrics`, and set `VCPKG_ROOT`. Never put vcpkg inside Civic 89. No `vcpkg integrate install` is required by CMake. Generated projects explicitly set `VcpkgEnabled=false` and `VcpkgEnableManifest=false` to prevent global MSBuild integration injecting paths, libraries or another restore.

## Configure, build and test

```powershell
cmake --preset windows-x64-debug
cmake --build --preset windows-x64-debug -- /m
ctest --preset windows-x64-debug --output-on-failure

cmake --preset windows-x64-release
cmake --build --preset windows-x64-release -- /m
ctest --preset windows-x64-release --output-on-failure
```

Presets use `Visual Studio 18 2026`, x64, separate directories and one configuration per directory. Original direct dependencies at the pinned baseline are SDL3 3.4.18, SDL3_image 3.4.4#1, SDL3_ttf 3.2.2#1, nativefiledialog-extended 1.4.1 and nlohmann-json 3.12.0#2. M3 adds SDL3_mixer 3.2.4 with optional codec features disabled. The local vcpkg executable version string is distinct from its registry commit.

Executable paths:

```text
C:\Dev\Projects\civic89\out\build\windows-x64-debug\bin\Debug\civic89.exe
C:\Dev\Projects\civic89\out\build\windows-x64-release\bin\Release\civic89.exe
```

The build verifies/stages 122 inventoried asset/notice files, retained GPL/additional-terms notices and dependency copyright files. vcpkg app-local deployment stages DLLs beside each executable. Approved fonts are checked in; no font download or system installation runs during build/startup.

## Run and install

The run target supplies the correct working directory:

```powershell
cmake --build --preset windows-x64-debug --target run
cmake --build --preset windows-x64-release --target run
```

Equivalent direct Release run used for the local GUI smoke:

```powershell
Start-Process -FilePath 'C:\Dev\Projects\civic89\out\build\windows-x64-release\bin\Release\civic89.exe' -WorkingDirectory 'C:\Dev\Projects\civic89\out\build\windows-x64-release\bin\Release'
```

Development-only portable staging:

```powershell
cmake --install out/build/windows-x64-release --config Release --prefix out/install/windows-x64-release
```

Installation includes only the application, pinned dependency DLLs, inventoried assets and notices. It excludes user saves, test binaries, PDBs and arbitrary build-directory files. Run the installed executable from its directory. This does not clear inherited icon rights or historical save compatibility for public release; see `PROJECT_STATUS.md`.

## Inherited comparison

The retained solution/project are **micropolis-sdlpp.sln** and **micropolis-cpp.vcxproj**. M2 adds eight split/support implementation entries while preserving all 47 original source paths:

```powershell
& 'C:\Program Files\Microsoft Visual Studio\18\Enterprise\MSBuild\Current\Bin\MSBuild.exe' micropolis-sdlpp.sln /m /p:Configuration=Release /p:Platform=x64 /verbosity:minimal /nologo
Start-Process -FilePath 'C:\Dev\Projects\civic89\x64\Release\micropolis-sdlpp.exe' -WorkingDirectory 'C:\Dev\Projects\civic89'
```

Global MSBuild vcpkg integration remains available for that inherited comparison. Checked-in font paths resolve its startup failure. Both applications can also launch from the CMake asset directory to use identical staging.

## Target mapping

| Inherited project | CMake |
|---|---|
| 64 `ClCompile` entries (47 original + 8 M2 + 4 M3 + 2 M4 + 3 M5 implementations) | Explicit source parity list; 31 engine units in `civic89_engine`, 33 application units in `civic89` |
| `micropolis-sdlpp.rc` | Civic 89 original icon and generated version resource; inherited comparison retains its resource |
| C++20 / WINDOWS / Unicode | C++20 / WINDOWS / UNICODE / _UNICODE |
| `/W3`, `/sdl`, conformance | `/W4`, `/sdl`, `/permissive-`; warning debt recorded |
| Debug console, Release Windows subsystem | Same configuration-specific subsystems |
| Release `/fp:fast`, whole-program optimization | `/fp:fast` in engine/app, app link-time optimization and debug symbols; original golden results verified |
| Global auto-link/restore | Explicit vcpkg package targets/toolchain |
| Debug forces Release dependencies | Normal matching Debug CRT/dependency DLLs |
| Repository cwd / no asset staging | Inventoried assets beside executable, explicit debugger/run cwd |
| Unrelated OGG DLL post-build copy | Actual dependency DLL deployment by vcpkg; SDL3_mixer playback and actual dependency deployment |

## Headless build and runner

These presets need only CMake and the MSVC C++ toolchain. No vcpkg toolchain,
SDL packages, JSON assets, window/renderer initialization, fonts or images are
required. Simulation sources are linked once into `civic89_engine` (a static
library); `civic89_runner` and `civic89_tests` link only that library.

```powershell
cmake --preset windows-x64-headless
cmake --build --preset windows-x64-headless -- /m
ctest --preset windows-x64-headless --output-on-failure

# Relative fixture paths below assume the repository working directory.
./out/build/windows-x64-headless/bin/Debug/civic89_runner.exe --scenario 6 --scenario-dir scenarios --seed 12345 --ticks 1024
./out/build/windows-x64-headless/bin/Debug/civic89_runner.exe --city 'path/to/current-format.cty' --seed 12345 --speed 2 --ticks 1024
./out/build/windows-x64-headless/bin/Debug/civic89_runner.exe --generate --seed 2468 --ticks 1024
```

Output includes city clock, funds, population, score, RCI and `digest_v1`.
A tick is one inherited simulation phase; 16 phases advance the city clock once.
Animation remains independently scheduled. `--seed` is optional and explicitly
controls the existing RNG; without it, normal random initialization applies.
`--speed 1..4` optionally overrides the city's stored speed (Slow/Normal/Fast/
African Swallow); without it, a saved pause/speed is respected. `--ticks` defaults
to zero and accepts 0..1,000,000. Exactly one of `--city`, `--scenario` or
`--generate` is required. Argument errors return 2; load/runtime errors return 1.

The library retains inherited process-global single-threaded state: initialize
before loading/editing/stepping. It does not support concurrent independent cities.
The tool manager accepts explicit tool definitions in headless code; its existing
JSON-loading default constructor is an application adapter.

## AddressSanitizer

Install the MSVC AddressSanitizer component with the C++ toolchain. The preset
uses RelWithDebInfo and the independent headless build, instruments engine,
runner and tests, and stages the matching compiler-supplied runtime DLL.

```powershell
cmake --preset windows-x64-asan
cmake --build --preset windows-x64-asan -- /m
ctest --preset windows-x64-asan --output-on-failure
```

GitHub Actions runs application Debug/Release, headless ASan and application ASan. Only application
jobs bootstrap the existing pinned external vcpkg checkout.

## Tests and limits

Application presets run **43** CTests; separate headless/ASan presets run **35**:

- `civic89_tests` covers headless load/step/pause/edit/save-byte-layout/reload,
  transactional invalid-file rejection, seeded terrain generation and sprites.
- Twenty-five golden checks compare fixed seeded city runs with results captured
  from merged M1 before extraction. Bern at 1,024 phases retains separate Debug
  and Release `/fp:fast` references. Every case uses a fresh process; the long
  case runs Detroit for 16,384 simulation phases.
- Runner checks exercise scenario/current-format city load, terrain generation,
  missing files and strict arguments.
- `engine-boundary` follows transitive local includes and inspects direct binary
  imports, rejecting engine SDL/Win32/UI/texture/JSON-loader dependencies.
- Application-only checks retain M1's runtime assets/data test
  (`civic89_runtime_tests`), source-list parity, typed audio/earthquake helpers,
  native lifecycle under SDL dummy/software rendering, legacy-command gate and
  invalid startup arguments. Lifecycle checks also draw, clear and reconstruct
  renderer-owned sprite images.

All 47 original translation-unit paths remain. M2 adds eight support/split files;
M3 adds four services and M4 adds two camera/settings implementations, for **61** entries shared with the retained
Visual Studio project. The engine retains 31 files, with 28 application files.
`civic89_storage` implements the Windows atomic writer for persistence tests;
the engine consumes only the platform-free writer interface.

M3 acceptance includes `functional-persistence` (100 new/save/load cycles,
Unicode filenames, metadata/history preservation, failed replacement and
recovery), `mixer-backend` (all 12 effects, actual sample energy/gain, loops,
mute, 32 mixer lifetimes and device failure), and three partial startup failures
plus 12 complete native application sessions. Existing parity cases are retained.

The full application sanitizer preset uses the same pinned SDL dependencies:

```powershell
cmake --preset windows-x64-app-asan
cmake --build --preset windows-x64-app-asan -- /m
ctest --preset windows-x64-app-asan --output-on-failure
```

## M3 audio, saving and diagnostics

Press **F8** for native master, city-effect and construction volume controls
(0/25/50/75/100%). The options window's sound checkbox controls effects and loops.
Master defaults to 70%; category gains default to 100%. Twelve original effects
are synthesised/loaded at startup; inherited WAV files are not shipped. Device
failure leaves a playable silent session and records the reason.

User files use `SDL_GetPrefPath("Civic89", "Civic89")`, normally
`%APPDATA%\Civic89\Civic89` on Windows: `audio.cfg`, `autosave.cty`, `civic89.log`.
Ordinary cities autosave every five minutes. Startup offers the last valid
recovery. Scenario autosave is skipped because the retained city format does
not store scenario progress; explicit scenario exports remain ordinary cities.
F2 saves, Shift+F2 selects a new destination, and F3 opens. Opening a city requires
Save As on the next save so opening a packaged fixture cannot silently overwrite
it. Failures preserve the current city/existing destination and report the cause.

Save publication writes a same-directory temporary file, checks/flushes/closes
it, and replaces the target. The 51,360-byte native 32-bit layout is retained;
loaded histories and difficulty are restored correctly. The post-load scan and
format's omitted RNG/sprite/scenario state preclude exact replay. Older 27,120-byte
files remain unsupported. Automated checks use dummy SDL drivers; desktop sound,
native dialog interaction and DPI are not established by these tests.

See [ADR 0004](decisions/0004_M3_FUNCTIONAL_COMPLETION.md) and
[M3 evidence](../tests/baseline/M3_2026-10-06.md). M2's [ADR](decisions/0003_M2_ENGINE_BOUNDARY.md),
[dependency audit](reference/ENGINE_DEPENDENCIES.md) and [evidence](../tests/baseline/M2_2026-10-06.md)
remain the pre-M3 extraction records. Raw logs remain in ignored `out/audit/`.

## Milestone 1 checkpoint (merged)

Use **F6** during play for the native scenario selector, or start with `civic89.exe --scenario 1` through `--scenario 8` from the executable's asset directory. Selection replaces the current city on success; save it first if needed. Cancel or missing/invalid scenario data preserves the current session. A successful start clears the old save filename. Scenario win/loss appears on the dashboard; play continues. Invalid startup arguments return exit code 2; startup exceptions return 1.

M1 retains all 47 production translation units, project files, dependencies, simulation/disaster/RNG algorithms and city-file writer. It adds header-only typed interfaces/validated scenario data and compiles the same sources again for the native lifecycle test, using `CIVIC89_MILESTONE_TESTS` only in that test target. M2 subsequently extracts the engine; the statements in this section record the M1 checkpoint.

Both preset builds pass 7/7 tests, and the retained Visual Studio Release build passes. Baseline characterization and final results are in [M1_2026-10-06.md](../tests/baseline/M1_2026-10-06.md); logs use `out/audit/m1-*`. Source comparisons verify mechanical call-site changes and unchanged existing city load/save functions. Audio remains silent, focus/shake remain no-ops, and deterministic simulation/historical file compatibility are not claimed. Interactive selector verification was unavailable due to monitor capture/access errors; the automated tests cover native lifecycle under dummy/software SDL, not desktop layout/DPI.


## M4 display, camera and rendering

F11 toggles desktop borderless fullscreen. F12 offers windowed/maximized/borderless,
VSync and pixel-perfect preferences. Normal window size and these preferences persist
atomically in `display.cfg` in the existing user-data directory. Invalid preferences
fall back to defaults with a diagnostic; save failures apply for the current session.

Use the wheel to zoom at the cursor, right-drag or held arrows to pan, and Home to
center. Camera zoom, tool preview/placement, sprites and minimap viewport share world
coordinates. Pixel perfect aligns source pixels to physical pixels; fractional zoom
retains nearest tile filtering. Auto-goto messages now move the camera when enabled.
Earthquakes shake the map for three seconds while leaving UI steady.

Windows DPI/display/pixel-size changes update per-window logical layout and input.
The existing 800Ãƒâ€”600 panel layout scales down to fit smaller high-DPI screens until
M5's responsive redesign. Render pacing uses checked VSync or a 60 Hz limiter;
simulation and animation run from separate main-thread deadlines with bounded
chronological catch-up. Recoverable GPU device/target resets recreate presentation resources while keeping
the city; unrecoverable device loss closes with an error and recovery on restart.

`camera-layout-cadence-settings` runs without SDL in every test preset. The application
lifecycle test additionally samples actual software-renderer output and construction
at 100/125/150/200% plus scale transitions. Native hidden-window mode checks passed on
this Windows workstation (one display at 100%). Physical mixed-DPI displays, visible
appearance and hardware VSync remain release checks.

See [ADR 0005](decisions/0005_M4_WINDOW_CAMERA_RENDERING.md) and
[M4 evidence](../tests/baseline/M4_2026-10-06.md) for full commands and compatibility.

## M5 single-window interface

The default interface uses one SDL window: dashboard, tool palette, minimap and
in-window budget/evaluation/history/query/scenario/settings panels. Native file
pickers remain for Open and Save. All information/settings sheets suspend simulation
and animation; closing a sheet resumes the previous paused/speed state.

The dashboard shows funds, date, population, numeric RCI and current tool/cost.
Hover or keyboard-focus a tool for its name, cost and assigned key. Unaffordable
construction selections are disabled; Query remains available even in debt. A tool
change cancels an earlier held construction drag. Click or drag the minimap to
center the camera; F4 toggles it. Home centers the city, wheel zooms, right-drag and
held pan bindings move the camera.

Choose the data button at top left for overlays. Full map and minimap share traffic,
crime, value, pollution, population, power, fire/police coverage, growth, transport
and zone data. The dashboard -/+ changes opacity. Legends and numeric tile probes
provide information independent of color; Settings offers a blue sequential palette.

Esc, F8 or F12 opens Settings. Tab/Shift+Tab and Enter navigate controls; Esc closes
or cancels key capture. Readability offers 100/125/150% requested UI scale, larger
text, high contrast and display/VSync/pixel-perfect controls. Effective scale is
capped to keep an 800x600 logical layout reachable on smaller screens. Window-size
preferences use native logical units, so UI scale cannot shrink the next launch.
Controls offers reverse zoom, pan speed and three pages of 30 unique bindings
(16 tools, 10 commands, 4 pan directions); the wheel also changes binding pages.
Reserved keys (Esc, Tab, Enter, modifiers, 0-4, F11/F12/Home) cannot be rebound.
Default tool keys are R/C/I/F/Q/L/W/B/T/D/S/K/H/O/N/A. Space pauses, 1-4 select
speed, F2 saves (Shift requests Save As), F3 opens, F4 minimap, F5 evaluation,
F6 scenarios, F7 new-city confirmation, F8 settings, F9 history and F10 budget.
F11 remains fullscreen and F12 settings. Sound has Master/City/Construction gains.
Gameplay exposes existing typed options; animation and disasters are session options,
while budget/bulldoze/goto/sound retain their existing city-file fields.

`ui.cfg`, `audio.cfg` and `display.cfg` live in the existing Civic89 user-data directory.
UI preferences are versioned, range/conflict checked and atomically published;
invalid files fall back to defaults with diagnostics. UI choices never extend `.cty`.
The interface reuses approved Raleway fonts and inventoried icons/art; no new asset
or production dependency is required. Inherited panel/minimap implementations remain
compiled as references for source/build parity, but are not instantiated by the app.

`ui-overlay-preferences` runs without SDL. `milestone-lifecycle` additionally renders
all panels across 800x600/1366x768/1920x1080/3440x1440 and 100/125/150/200% effective
scale, drives actual input, saves PNG captures and checks simulation digest invariance.
Captures are under each application's output directory in `ui-captures/`; selected
reviewed captures and exact commands are in [M5 evidence](../tests/baseline/M5_2026-10-06.md).

## M6 release and ARM64 paths

The root README now describes Civic 89 rather than upstream SDLPP. Release builds
identify version `0.6.0-dev`, full/short Git revision, architecture and dirty state.
`civic89.exe --version` does not initialise SDL or user files. CMake executables
find assets beside themselves, so a shortcut or an unrelated working directory
works. The retained comparison executable still permits repository-relative assets.
CMake uses an original Civic 89 icon; the inherited `.rc` is preserved for comparison.
All **64** production source entries remain, with **31** engine / **33** application units.

`--smoke-test` launches the real game hidden with an isolated temporary user directory,
loads Detroit, renders, saves/reloads and exits. Release tooling supplies dummy SDL
and removes developer PATH DLLs. This mode does not touch real preferences/recovery.

On native ARM64 Windows, install the VS 2026 ARM64 toolchain and use
`windows-arm64-release` configure/build/test presets. Both target and native host
triplets are `arm64-windows`. Cross-compiling from x64 can override
`-D VCPKG_HOST_TRIPLET=x64-windows`; that does not establish native runtime acceptance.
The five-job CI adds native ARM64 Release to the existing four configurations.

See [RELEASING.md](RELEASING.md) for portable ZIPs, installers, corresponding source,
signing, release gates and crash-safe offline updates. `cmake --install` remains
an intermediate development stage; complete release tooling adds CRT and manifest
verification. No simulation algorithm, golden digest or city format changes in M6.
