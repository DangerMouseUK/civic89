# Project Charter and Architecture Decision Record

**Status:** Active engineering baseline  
**Prepared:** 5 October 2026  
**Repository baseline captured:** 5 October 2026  
**Scope:** **Civic 89** - native Windows modernisation of the open-source Micropolis / original SimCity code lineage

## 1. Project charter

### 1.1 Objective

Create **Civic 89**, a polished native Windows edition built from the original Micropolis / SimCity Classic simulation lineage while preserving its original gameplay and mechanics. Modernise the appearance, controls and Windows experience around one faithful simulation contract. The only optional enhancement is a graphics toggle, as decided by the user on 6 October 2026 in [ADR 0009](../decisions/0009_FAITHFUL_MODERNISATION_AND_GRAPHICS.md).

### 1.2 Primary goals

- Native Windows x64 first, with a credible ARM64 path later.
- Preserve the authentic classic simulation and `.cty` interoperability.
- Replace residual Tcl/Tk-style command plumbing with typed C++ interfaces.
- Use a modern SDL3 rendering/input/audio stack.
- Make the application comfortable on 1080p, 1440p, 4K, ultrawide and scaled Windows desktops.
- Preserve automated parity/regression evidence throughout modernisation.
- Keep upstream provenance and GPL compliance straightforward.
- Make the codebase understandable enough for long-term maintenance and agent-assisted development.
- Allow optional improved graphics without changing city state, mechanics, timing, RNG or saves.

### 1.3 Non-goals

The following are outside the agreed project scope:

- rewriting the simulation from scratch;
- converting to Unity, Unreal, Electron or a browser shell;
- adding multiplayer;
- replacing all original pixel art immediately;
- larger maps or expanded finance/population limits;
- changing zoning, traffic, utilities, demand, tax, budget or disaster mechanics;
- additional buildings, tools, scenarios, modding or achievements/challenges;
- day/night and seasonal additions;
- shipping under the SimCity name;
- broad cross-platform support before the Windows code path is stable and tested.

## 2. Design principles

### 2.1 Preserve first, improve second

Every major refactor should start with a test or baseline that demonstrates current behaviour. Preserve the original gameplay and mechanics. Record any fidelity discrepancy and obtain explicit user direction before an outcome-changing fix; a regression test alone does not authorise new or rebalanced mechanics.

### 2.2 Native but portable

The user experience is Windows-first, but the game logic should not depend directly on Win32 or SDL. Platform-specific behaviour belongs behind interfaces. SDL3 is the preferred abstraction for the application layer.

### 2.3 Small dependency surface

Use mature libraries with a clear reason to exist. Avoid frameworks that duplicate functionality already provided by SDL3 or the C++ standard library.

### 2.4 Data and simulation are more important than presentation

The renderer and UI can be replaced. The simulation, city format and scenario behaviour are the historical asset. Architect accordingly.

### 2.5 No string-command pseudo-API

The inherited `Eval("UI...")` compatibility bridge must disappear. Internal events and commands should use typed C++ methods/objects.

### 2.6 Modernisation must be measurable

The project needs explicit acceptance criteria for build reproducibility, simulation parity, save-file compatibility, memory safety, startup/shutdown, frame pacing and DPI behaviour.

## 3. Architecture decision records

### ADR-000 - Product and repository identity: Civic 89

**Decision:** The project/product working name is **Civic 89**. Technical identifiers use `civic89`/`Civic89`; the local repository root is `C:\Dev\Projects\civic89`.

**Why:** A stable identity avoids churn in repository naming, CMake targets, executable names, CI artifacts, local settings paths and coding-agent instructions. SimCity and Micropolis remain historical/upstream references, not the product brand.

**Constraint:** The current name decision is an engineering/branding baseline, not a substitute for formal trademark clearance before public commercial release.


### ADR-001 - Primary baseline: Micropolis-SDLPP

**Decision:** Preserve and fork Micropolis-SDLPP history as the starting codebase.

**Why:** It already solves the native Windows/SDL3 shell problem and contains substantial native UI work. Rebuilding all of that around MicropolisCore would recreate work that already exists.

**Consequence:** We inherit unfinished areas and some legacy structure, but can improve them incrementally while keeping a runnable desktop game.

### ADR-002 - MicropolisCore is a reference upstream, not a merge target

**Decision:** Track MicropolisCore as a reference for fixes, file-format logic and test cases. Do not merge its repository history into the native fork.

**Why:** Its web/WASM architecture and Emscripten callback types create unnecessary work for a native Windows target. Selective, reviewed ports are safer.

### ADR-003 - C++20 baseline

**Decision:** Keep C++20 during bootstrap.

**Why:** SDLPP already builds as C++20. Raising the language version while migrating the build system would add unrelated change. C++23 can be introduced later as a separate decision.

### ADR-004 - CMake becomes the build-system source of truth

**Decision:** Replace the hand-maintained Visual Studio project as the authoritative build definition with CMake and CMake Presets.

**Why:** One build definition can drive Visual Studio, CI and future non-Windows ports. It also reduces project-file churn when sources move.

### ADR-005 - vcpkg manifest mode with a pinned baseline

**Decision:** Declare dependencies in `vcpkg.json` and use a pinned vcpkg baseline/version strategy.

**Why:** Developer machines and CI must resolve the same dependency graph. Microsoft recommends manifest mode for most vcpkg projects.

### ADR-006 - SDL3 stays the application platform layer

**Decision:** Keep SDL3 for windowing, input, rendering abstraction, timers and controllers. Add SDL3_mixer for sound unless direct SDL audio proves preferable for a specific need.

**Why:** The existing port is already SDL3-based and SDL provides the right level of portability without a heavyweight engine.

### ADR-007 - Classic Mode and Enhanced Mode are separate behavioural contracts

**Superseded direction (6 October 2026):** the following records the original plan.
[ADR 0009](../decisions/0009_FAITHFUL_MODERNISATION_AND_GRAPHICS.md) replaces future
gameplay expansion with faithful modernisation and optional graphics. M7 remains
merged; its interface adjustment belongs in M8.

**Decision:** Classic Mode aims to preserve simulation behaviour. Enhanced Mode is the future location for intentionally changed or expanded mechanics.

**Why:** This protects historical compatibility while still allowing the project to grow.

### ADR-008 - The simulation core must become SDL-free

**Decision:** Over time, simulation code is built into a separate library target with no SDL dependency.

**Why:** Headless tests, deterministic runs, tools and future frontends become dramatically easier.

### ADR-009 - Typed application events replace `Eval()`

**Decision:** Introduce explicit interfaces/events for sound, UI notifications, navigation, scenarios and other callbacks.

**Why:** The current stubbed `Eval()` function hides incomplete behaviour and makes refactoring unsafe.

### ADR-010 - Windows x64 first, ARM64 second

**Decision:** The supported development/release target is Windows x64 until the baseline and modern UI are stable. Windows ARM64 is added as a secondary build after the architecture is clean.

**Why:** It keeps the first target narrow while preserving a straightforward future expansion path.

### ADR-011 - Preserve upstream history and attribution

**Decision:** Clone the upstream repository, rename its remote, add a new origin and tag the exact baseline SHA. Do not start from a source ZIP.

**Why:** History provides provenance, attribution, auditability and easier comparison with future SDLPP changes.

**Implementation record - 5 October 2026:** The history-preserving clone is complete at `C:\Dev\Projects\civic89`. The inherited remote is now `upstream-sdlpp`, the exact baseline is `9c4e85a0decd57ba6f76d9e1ec82461940ecc3ad`, and annotated tag `upstream-sdlpp-baseline` has been created locally. A new Civic 89 `origin` is the next repository step and was not yet configured when this record was captured.

### ADR-012 - No mass formatting during bootstrap

**Decision:** Do not reformat the entire inherited codebase during the CMake/CI migration.

**Why:** Large whitespace diffs destroy blame/history value and make behavioural review harder. Apply formatting to new or materially edited code first.

## 4. Gameplay and graphics

### 4.1 One faithful gameplay contract

Preserve:

- original map dimensions and gameplay limits;
- original simulation tick logic;
- original RCI/economy/disaster mechanics;
- original scenario objectives and start states;
- compatible city load/save;
- visual/UI improvements that do not change simulation results;
- fidelity fixes only with verified evidence and explicit direction for changed outcomes.

### 4.2 Optional graphics preference

M9 may add Classic/Enhanced graphics in Settings, switchable during play. Both draw
the same city and existing visual states/animations. The preference must not change
rules, city/ruleset state, RNG, simulation/animation timing or save bytes. Keep
licensed assets, faithful tile/sprite identities/footprints and original graphics available.

### 4.3 Merged M7 compatibility and planned M8 adjustment

The current M7 application exposes Classic v1/Enhanced v1 city identities and saves;
both use the same mechanics. M8 will simplify the interface around one gameplay
contract and remove the implication of an alternate simulation. Preserve existing
`.c89` reading/saving/export/recovery and unchanged `.cty` output. A graphics switch
must never require city conversion or save retagging. This is planned work, not a
claim that the M7 interface has already changed.

## 5. Quality bar

A feature is not complete merely because it works on the developer's PC. It should include the appropriate combination of:

- automated tests;
- clean debug and release builds;
- clean code analysis for newly touched code;
- documentation for externally visible behaviour;
- reproducible steps from a clean checkout;
- no known crash/resource-lifetime regression;
- compatibility notes if save or simulation behaviour is touched.
