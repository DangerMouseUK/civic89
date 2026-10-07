# Upstream Audit, Known Gaps and Fork Baseline

> Current checkpoint — 7 October 2026: M0–M9 software work is merged.
> This document retains the 5 October planning/audit snapshot and dated updates;
> bootstrap tasks, starter templates, proposed dependencies and future-tense
> migration steps are historical, not a new assignment. Implemented build/targets
> and dependency choices are in [BUILDING.md](../BUILDING.md); current state is
> [PROJECT_STATUS.md](../../PROJECT_STATUS.md). Faithful scope is governed by
> [ADR 0009](../decisions/0009_FAITHFUL_MODERNISATION_AND_GRAPHICS.md), and the
> owner-approved unsigned beta by [ADR 0010](../decisions/0010_PUBLIC_BETA.md).
> Physical desktop acceptance remains pending. Catch2/spdlog were proposals;
> they are not current dependencies. Original results are not retroactive checks.

**Status:** Active engineering baseline  
**Prepared:** 5 October 2026  
**Repository baseline captured:** 5 October 2026  
**Scope:** **Civic 89** - native Windows modernisation of the open-source Micropolis / original SimCity code lineage

## 1. Upstream lineage

The project derives from the original SimCity code released through the Micropolis open-source lineage. The open-source code is GPL-based, with additional terms carried in the original release material. Product branding and trademarks are separate from source-code licensing.

The two modern codebases most relevant to this project are:

- **Micropolis-SDLPP** - native C++/SDL port, Windows as the main development platform;
- **MicropolisCore** - heavily refactored engine/web project using C++, Emscripten/WASM, TypeScript/Svelte and modern web rendering.

## 2. Micropolis-SDLPP audited state

The audit for this plan used upstream `main` at the commit below, and the subsequent local Civic 89 clone captured **the exact same SHA**:

```text
9c4e85a0decd57ba6f76d9e1ec82461940ecc3ad
```

Confirmed local repository facts on 5 October 2026:

- repository root: `C:\Dev\Projects\civic89`;
- branch immediately after clone: `main`;
- working tree immediately after clone/remote rename: clean;
- `.git` confirmed inside the project root;
- preserved upstream remote: `upstream-sdlpp` -> `https://github.com/ldicker83/Micropolis-SDLPP.git`;
- annotated local tag: `upstream-sdlpp-baseline` -> `9c4e85a0decd57ba6f76d9e1ec82461940ecc3ad`;
- Civic 89 `origin`: not yet configured at this capture point.

### 2.1 What is already present

The native port contains a real desktop game shell rather than only a compileable simulation library. The audited project includes:

- C++20 source and Visual Studio project configuration;
- SDL3 window creation and renderer use;
- a resizable main window;
- the classic simulation modules for maps, zones, traffic, power, disasters, scanning and sprites;
- native construction/tool handling;
- budget and city properties;
- evaluation;
- graph/history UI;
- a dashboard;
- options and query windows;
- a tool palette;
- native open/save dialogs;
- city load/save implementation;
- eight classic scenarios represented in file I/O data;
- an SDL minimap implementation with multiple data overlays;
- vcpkg dependencies for SDL3, SDL3_ttf, SDL3_image, nativefiledialog-extended and nlohmann-json.

### 2.2 Important incomplete areas found in code

#### Audio is a placeholder

`w_sound.cpp` still contains explicit comments for loading and unloading sound samples rather than a complete sound backend. Sound commands continue to route through legacy-style command calls.

**Action:** treat complete audio implementation as a functional-completion milestone, not cosmetic polish.

#### `Eval()` is still a stub

The compatibility function currently prints the requested command and returns false. Several code paths still use command strings such as scenario start, earthquake UI effects and sound operations.

**Action:** inventory every `Eval()` call and replace it with typed application services/events. Delete the compatibility function only after all call sites have migrated and tests cover the paths.

#### Scenario start is not fully modernised

The scenario table and files exist, but current startup code still routes scenario initiation through the `Eval()` bridge.

**Action:** implement typed scenario selection/start and confirm each of the eight scenarios loads, starts and evaluates correctly.

#### Camera/presentation is basic

The main window is resizable, but the audited main code did not show a modern mouse-wheel camera zoom path, explicit fullscreen/borderless support, an explicit high-pixel-density window path or explicit VSync configuration.

**Action:** add a dedicated Camera and DisplayMode layer rather than patching ad-hoc transforms through main.cpp.

#### Minimap is a separate OS window

The minimap currently creates its own SDL window/renderer. That is valid, but a modern default should place it inside the main game UI, with optional detachment later if desired.

#### Resource lifetime has a known open issue

SDLPP issue #18 documents texture lifetime/cleanup concerns. Some UI resources rely on process/SDL teardown rather than explicit ownership discipline.

**Action:** introduce RAII wrappers or clear ownership rules and add shutdown/reopen stress tests.

### 2.3 Existing strengths we should preserve

- Native SDL event loop and rendering path.
- Original simulation code instead of a behavioural reimplementation.
- Existing native windows/panels as functional reference while redesigning UI.
- Existing file format/scenario tables.
- Windows-first project focus.
- Small dependency set.

## 3. MicropolisCore audited state

The reference audit used `main` at:

```text
f9ae6a57bbe5f5ff94c149bccb3015757f18241d
```

### 3.1 Strengths

MicropolisCore contains useful modern work in several areas:

- explicit C++ `loadCity`, `saveCity`, scenario and simulation APIs;
- detailed file-endianness handling and `.cty` work;
- test infrastructure in the JavaScript/TypeScript application;
- a modern WebGL tile renderer with pan/zoom concepts;
- ongoing WebGPU/render architecture work;
- active investigation of simulation bugs and reproducibility;
- useful issue reports with reproducible probes.

### 3.2 Why it is not the preferred native baseline

The C++ engine boundary contains Emscripten-specific types such as `emscripten::val`. The checked-in engine build uses Emscripten/Embind and the main client is a web application. Turning it into a clean native library would therefore require first removing web-platform assumptions.

That work is possible, but the SDLPP project has already solved a larger amount of the native desktop problem.

### 3.3 Upstream issues relevant to our fork

Current MicropolisCore issue history is useful as a regression watch-list. Examples inspected during this audit include:

- sprite/train behaviour under investigation;
- an out-of-bounds crash associated with an unconstructed `std::string` in sprite allocation;
- deterministic seeding/reproducibility problems;
- previously identified simulation-ratio logic problems.

Do not blindly port a fix. Reproduce the issue in our fork, add a regression test, then port/adapt the smallest correct change.

## 4. Fork strategy

**Repository root:** `C:\Dev\Projects\civic89`. The `.git` directory belongs inside this folder because this folder is the Civic 89 repository. `C:\Dev\vcpkg` is separate tooling and must never be nested inside or committed to Civic 89.


### 4.1 Preserve history

The local history-preservation steps have now been executed successfully. The authoritative result is:

```text
Repository:       C:\Dev\Projects\civic89
Branch:           main
Baseline SHA:     9c4e85a0decd57ba6f76d9e1ec82461940ecc3ad
Baseline tag:     upstream-sdlpp-baseline
Upstream remote:  upstream-sdlpp -> https://github.com/ldicker83/Micropolis-SDLPP.git
Working tree:     clean at capture
New origin:       not yet configured
```

For repeat/new-machine onboarding, the executed commands were:

```powershell
New-Item -ItemType Directory -Force C:\Dev\Projects | Out-Null
Set-Location C:\Dev\Projects

git clone https://github.com/ldicker83/Micropolis-SDLPP.git civic89
Set-Location C:\Dev\Projects\civic89

# The clone is already a Git repository. Do NOT run git init.
git remote rename origin upstream-sdlpp
git rev-parse HEAD
git tag -a upstream-sdlpp-baseline -m "Civic 89: audited SDLPP baseline before modernisation"
```

The next repository step, after creating an **empty** Civic 89 GitHub repository, is:

```powershell
git remote add origin https://github.com/<owner>/civic89.git
git remote -v
git push -u origin main
git push origin upstream-sdlpp-baseline
```

Keep `upstream-sdlpp` permanently configured so later upstream changes can be inspected. The local baseline tag must not be moved to follow future upstream commits.

### 4.2 Track MicropolisCore separately

Do not merge its Git history. Record it in `project/reference/UPSTREAMS.md` and use issue/commit references in port commits.

Example commit message for a ported fix:

```text
engine: fix sprite construction lifetime

Reproduces and adapts the underlying fix discussed in
SimHacker/MicropolisCore#11.

Adds a native regression test before changing behaviour.
```

## 5. Baseline acceptance before refactoring

The unmodernised baseline should be documented and executable before code movement begins.

### 5.1 Functional smoke checklist

- Application starts from a clean Windows x64 build.
- New city generation works.
- Road, rail, wire and R/C/I zoning tools work.
- Power simulation runs.
- RCI values update.
- Budget window opens and updates.
- Evaluation window opens and updates.
- Graphs display data.
- Minimap opens and renders.
- A city can be saved.
- The saved city can be reloaded.
- A supplied known city can be loaded.
- Each scenario file can at least be parsed successfully, even before scenario UI is repaired.
- Application exits without a crash.

### 5.2 Baseline evidence to capture

Store under `tests/baseline/` or `docs/baseline/`:

- exact upstream SHA;
- compiler/build environment summary;
- a baseline save file created by SDLPP;
- one or more known historical `.cty` fixtures permitted for redistribution;
- screenshots of the initial UI;
- checksums of fixture files;
- smoke-test results;
- list of known failures accepted at baseline.

## 6. Upstream sync policy

After our architecture diverges, avoid routine merges from SDLPP. Instead:

1. `git fetch upstream-sdlpp`;
2. review the new commits;
3. classify them as simulation fix, platform fix, UI change or irrelevant;
4. cherry-pick only clean changes or manually port them;
5. add/adjust tests;
6. document provenance in the commit message.

This keeps our repository auditable and prevents old structural choices from repeatedly re-entering the modernised tree.
