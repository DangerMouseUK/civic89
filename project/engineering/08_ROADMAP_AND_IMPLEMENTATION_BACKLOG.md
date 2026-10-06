# Roadmap and Detailed Implementation Backlog

**Status:** Active engineering baseline  
**Prepared:** 5 October 2026  
**Repository baseline captured:** 5 October 2026  
**Scope:** **Civic 89** - native Windows modernisation of the open-source Micropolis / original SimCity code lineage

## 1. Roadmap philosophy

The milestones are ordered to reduce risk. Do not begin the visible remaster work until the project can prove that the inherited simulation still builds and behaves consistently.

As requested by the user on 6 October 2026, deliver complete milestones on one engineering branch and PR. Separate implementation/test commits within that branch are encouraged; do not publish one- or two-item increments unless explicitly requested.

Effort labels are relative only:

- **S** - contained change;
- **M** - multi-file change with tests;
- **L** - architectural milestone;
- **XL** - broad feature requiring substantial implementation and review within its milestone.

## 2. Milestone 0 - Baseline Bootstrap

**Goal:** reproducible native Windows build with no intentional gameplay change.

### M0-01 Preserve upstream history - S

- clone SDLPP directly to `C:\Dev\Projects\civic89`;
- rename the inherited remote to `upstream-sdlpp`;
- add the empty Civic 89 GitHub repository as new `origin`;
- retain `upstream-sdlpp` remote;
- tag audited baseline SHA;
- add `project/reference/UPSTREAMS.md`.

**Acceptance:** repository history contains upstream commits and baseline tag resolves to the audited SHA.

### M0-02 Add CMake build - L

- reproduce existing x64 Debug/Release builds;
- retain source layout initially;
- copy/install runtime assets correctly;
- ensure no manual include/lib paths are required.

**Acceptance:** clean checkout builds from command line and Visual Studio folder-open flow.

### M0-03 Reproducible vcpkg manifest - M

- migrate existing dependencies;
- add planned test/logging dependencies;
- pin baseline;
- document update process.

**Acceptance:** a second machine restores the same dependency graph.

### M0-04 CI - M

- Windows VS2026 x64 Debug + Release;
- test execution;
- build artifact for Release.

**Acceptance:** PR cannot merge when build/tests fail.

### M0-05 Baseline evidence - M

- smoke checklist;
- baseline city/save fixtures;
- screenshots;
- known-issues list.

## 3. Milestone 1 - Remove Legacy Command Plumbing

**Goal:** eliminate stubbed Tcl/Tk-style `Eval()` dependencies.

### M1-01 Inventory every `Eval()` call - S

Produce a table of command name, call site, intended effect and replacement service.

### M1-02 Typed presentation events - L

Introduce minimal interfaces for UI notifications/navigation/disaster presentation.

### M1-03 Scenario lifecycle - M

Replace string command startup with typed scenario selection/load/start.

**Acceptance:** all eight scenarios can be selected and started through native code.

### M1-04 Delete obsolete `Eval()` bridge - M

Only after tests prove no remaining functional dependency.

**Acceptance:** repository search contains no runtime `Eval("UI...`) path.

**Completion record (6 October 2026):** M1-01 through M1-04 are implemented on `codex/m1-remove-legacy-plumbing`. All eight scenarios load and advance through native code; F6 and startup selection use the typed controller. No runtime `Eval` or string audio API remains. Debug/Release pass 7/7 checks and the inherited Release build passes. Merged in PR #6 (`de424f3`); hosted Debug/Release CI passed; [ADR 0002](../decisions/0002_M1_COMPLETION.md) and [milestone evidence](../../tests/baseline/M1_2026-10-06.md) record compatibility decisions and interactive UI limits.

## 4. Milestone 2 - Engine Boundary and Testability

**Goal:** simulation becomes a headless library target.

### M2-01 Identify SDL/UI contamination - M

Map direct dependencies from simulation files to presentation/platform code.

### M2-02 Create `civic89_engine` target - XL

Move incrementally; do not redesign algorithms during extraction.

### M2-03 Headless simulation runner - M

CLI/tool target capable of loading a city, running N ticks and outputting summary/digest.

### M2-04 Deterministic test mode - L

Provide explicit test seeding/control without changing normal play semantics.

### M2-05 Parity fixtures - L

Golden state digests for known city/tick counts.

**Completion record (6 October 2026):** M2-01 through M2-05 are implemented on `codex/m2-engine-boundary`. The native app links a platform-free engine; a standalone headless runner, explicit deterministic control and 25 pre-extraction golden cases are verified. Application Debug/Release pass 39/39 checks each; headless and ASan pass 32/32 each; retained Visual Studio Release builds. [ADR 0003](../decisions/0003_M2_ENGINE_BOUNDARY.md), [dependency audit](../reference/ENGINE_DEPENDENCIES.md) and [evidence](../../tests/baseline/M2_2026-10-06.md) record the complete milestone and limits. Merged in PR #7 (`090633d`); hosted Debug/Release/headless ASan CI passed on `d0986f5`.

## 5. Milestone 3 - Functional Completion

**Goal:** complete missing native functionality before cosmetic redesign.

### M3-01 AudioManager with SDL3_mixer - L

- map typed sound IDs;
- load/release resources;
- volume controls;
- device failure fallback;
- no string command bridge.

### M3-02 Resource ownership cleanup - L

- RAII wrappers for SDL resources where helpful;
- deterministic teardown order;
- stress test repeated new/load/quit/session cycles.

### M3-03 Save reliability - M

- atomic save flow;
- clearer parse/write errors;
- autosave/recovery foundation.

### M3-04 Error/logging layer - M

Replace silent/console-only critical failures with structured reporting.

**Completion record (6 October 2026):** M3-01 through M3-04 are implemented together on `codex/m3-functional-completion`: typed SDL3_mixer audio with original effects and persisted native volume controls; RAII/exception-safe resource teardown; same-directory atomic city publication with validated restoration and autosave/recovery; typed diagnostics and native error reporting. Application Debug/Release/ASan pass 41/41 tests; headless Debug/ASan pass 33/33; all 25 M2 golden cases and the retained Visual Studio Release build pass. [ADR 0004](../decisions/0004_M3_FUNCTIONAL_COMPLETION.md) and [evidence](../../tests/baseline/M3_2026-10-06.md) record acceptance and compatibility limits, including ordinary-city recovery and unsupported historical formats. Merged in PR #8 (`555b7ba`); hosted Debug/Release/application ASan 41/41 and headless ASan 33/33 passed on `39a9399`.

## 6. Milestone 4 - Modern Window, Camera and Rendering

**Goal:** the game begins to feel like a 2026 Windows application without changing Classic simulation rules.

### M4-01 Display modes - M

- windowed;
- maximised;
- borderless/fullscreen;
- persisted setting.

### M4-02 DPI-aware layout - L

Test 100-200% scaling and mixed-DPI monitors.

### M4-03 Camera2D - L

- smooth pan;
- mouse-wheel zoom;
- zoom-to-cursor;
- map bounds;
- keyboard movement;
- pixel-perfect option.

### M4-04 VSync/frame pacing - M

Separate render cadence from simulation tick cadence.

### M4-05 Renderer cleanup - L

Centralise tile/sprite/overlay rendering and remove map drawing responsibilities from the application loop.

**Completion record (6 October 2026):** M4-01 through M4-05 are implemented together on `codex/m4-window-camera-rendering`: persisted windowed/maximized/borderless modes; per-window DPI/input/layout; Camera2D with pan, cursor zoom, bounds, keyboard and pixel scaling; main-thread simulation/animation scheduling and VSync/frame limiting; centralized map/sprite/tool/quake rendering. Application Debug/Release/ASan pass 42/42 checks; headless Debug/ASan pass 34/34; all 25 M2 goldens and retained Visual Studio Release pass. Native Windows mode APIs and the automated 100â€“200%/mixed-scale matrix pass. Physical mixed-DPI/visible desktop/hardware refresh remain release checks. [ADR 0005](../decisions/0005_M4_WINDOW_CAMERA_RENDERING.md) and [evidence](../../tests/baseline/M4_2026-10-06.md) record acceptance and compatibility decisions. Hosted CI passed all four jobs on [PR #9](https://github.com/DangerMouseUK/civic89/pull/9), merged as `fc0c4f6`.

## 7. Milestone 5 - Modern UI/UX

**Goal:** cohesive single-window remaster experience.

### M5-01 In-window minimap - L

Reuse existing overlay data; detach option can be later.

### M5-02 Responsive dashboard/status bar - L

Funds, date, population, RCI, message state and active tool.

### M5-03 Tool palette redesign - L

Icons, tooltips, hotkeys, disabled/affordability states and scalable layout.

### M5-04 Budget/evaluation/graphs redesign - XL

Preserve data but modernise presentation and layout.

### M5-05 Full-map data overlays - L

Traffic, crime, land value, pollution, population, power/protection data with adjustable opacity.

### M5-06 Settings/accessibility - L

- UI scale;
- font/readability options;
- configurable hotkeys;
- volume categories;
- camera controls;
- colour/overlay accessibility considerations.

**Completion record (6 October 2026):** M5-01 through M5-06 are delivered together on `codex/m5-modern-ui`: in-window minimap, responsive dashboard, tool palette/affordability/hotkeys, budget/evaluation/history/query panels, shared full-map data layers and opacity, and persisted readability/key/camera/audio preferences. Native in-window scenario/new-city panels complete the workflow. Application Debug/Release/ASan pass 43/43; headless Debug/ASan pass 35/35; all 25 Classic goldens and retained Visual Studio Release pass. Compact/wide rendered captures were visually reviewed; native software/Direct3D 11 checks pass. Physical mixed-DPI and assistive-technology integration remain release validation. [ADR 0006](../decisions/0006_M5_MODERN_INTERFACE.md) and [evidence](../../tests/baseline/M5_2026-10-06.md) record compatibility and acceptance. Hosted CI is tracked on the delivered PR.

## 8. Milestone 6 - Release Engineering

### M6-01 Portable release ZIP - M

Self-contained x64 artifact with licences/notices.

### M6-02 Installer - M

Inno Setup or WiX; uninstall and user-data preservation.

### M6-03 Code-signing integration - M

Separate secure signing stage.

### M6-04 Crash-safe update strategy - L

Only after stable versioning and release hosting exist.

### M6-05 Windows ARM64 - L

Compile/test after the engine/platform boundary is clean.

**Completion record (6 October 2026):** M6-01 through M6-05 and the root README are delivered together on `codex/m6-release-engineering` in [PR #11](https://github.com/DangerMouseUK/civic89/pull/11). Clean versioned portable ZIPs, per-user installers, matching source/checksums, complete notices, separate protected signing integration and native ARM64 Release CI are implemented. M6-04 supplies verified offline whole-version staging/atomic selection/rollback; automatic network updates remain conditional on stable release hosting. All five hosted jobs pass on `3bbea9a`: application x64/ARM64 configurations 43/43 and headless ASan 35/35, with all Classic goldens unchanged. Both architectures pass ZIP launch, DLL closure, update/failure/rollback and install/upgrade/uninstall retention. Source-export build 35/35 and retained Visual Studio Release pass locally. [ADR 0007](../decisions/0007_M6_RELEASE_ENGINEERING.md), [evidence](../../tests/baseline/M6_2026-10-06.md) and [release instructions](../RELEASING.md) record checks and compatibility limits. Trusted signer provisioning and public asset/brand/physical acceptance remain external gates; no public release or tag is created.

## 9. Milestone 7 - Enhanced Mode foundation

Do not start until Classic Mode has a supported contract.

**Scope decision (6 October 2026):** the user selected the complete foundation,
not implementation of every candidate gameplay feature. Establish the supported
development contract first; stable public release remains separately gated.

### M7-01 Classic development contract

Specify supported mechanics, platforms, file layout and historical/replay limits.

### M7-02 Versioned rulesets

Typed Classic v1/Enhanced v1 identities and capability definitions; reject unknown
versions before mutation. Enhanced v1 initially shares Classic mechanics.

### M7-03 Explicit mode selection

New-city mode selection, active identity in the UI and headless runner, and Classic
scenario policy. Existing sessions must not change modes through a preference.

### M7-04 Save/import/export/recovery boundary

Preserve `.cty`; add a bounded, checksummed, versioned Enhanced container, explicit
Classic import/export and separate recovery slots. Retain atomic publication.

### M7-05 Compatibility acceptance

Unchanged Classic goldens and generated-city baseline; both mode paths; save round
trips; unknown/corrupt/truncated input and publication failure isolation; UI acceptance;
Debug/Release/ASan/native ARM64, delivery checks and retained Visual Studio parity.

Future Enhanced candidate backlog (outside the M7 foundation):

- larger maps;
- expanded finance/population types;
- additional buildings/tools;
- new scenarios;
- mod/data definition exploration;
- richer traffic simulation;
- day/night and seasonal rendering;
- achievements/challenges.

Every Enhanced feature must declare whether it can import/export a Classic city unchanged.

## 10. First implementation sequence for a coding agent

The safest first sequence is:

1. create new origin while preserving history;
2. add docs/UPSTREAMS and baseline tag;
3. convert current Visual Studio source list into CMake without moving files;
4. create vcpkg manifest + pinned baseline;
5. build x64 Debug/Release;
6. add Catch2 and a trivial executable/engine smoke test;
7. add GitHub Actions;
8. record baseline known issues;
9. open a separate PR for `Eval()` inventory and typed replacement design.

Do not combine steps 3 and 9 in one PR.
