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

**Completion record (6 October 2026):** M1-01 through M1-04 are implemented on `codex/m1-remove-legacy-plumbing`. All eight scenarios load and advance through native code; F6 and startup selection use the typed controller. No runtime `Eval` or string audio API remains. Debug/Release pass 7/7 checks and the inherited Release build passes. Branch is published for PR review; hosted CI is pending; [ADR 0002](../decisions/0002_M1_COMPLETION.md) and [milestone evidence](../../tests/baseline/M1_2026-10-06.md) record compatibility decisions and interactive UI limits.

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

## 9. Milestone 7 - Enhanced Mode foundation

Do not start until Classic Mode has a supported contract.

Candidate backlog:

- versioned ruleset abstraction;
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
