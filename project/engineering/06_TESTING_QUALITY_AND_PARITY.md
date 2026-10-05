# Testing, Quality Gates and Classic Simulation Parity

**Status:** Active engineering baseline  
**Prepared:** 5 October 2026  
**Repository baseline captured:** 5 October 2026  
**Scope:** **Civic 89** - native Windows modernisation of the open-source Micropolis / original SimCity code lineage

## 1. Testing objective

The project needs to distinguish three different questions:

1. Does the program compile and run?
2. Does the simulation still behave like the baseline?
3. Does the modern UI behave correctly across Windows environments?

No single test type answers all three.

## 2. Test layers

### 2.1 Unit tests

Use Catch2 for small deterministic components such as:

- coordinate conversions;
- camera clamping/zoom math;
- budget calculations that can be isolated;
- parsing/byte-order helpers;
- settings serialization;
- typed event mapping;
- save-path and filename normalisation.

### 2.2 Engine integration tests

Headless engine tests should cover:

- new city generation with controlled inputs;
- simulation stepping;
- tool placement and cost;
- power connectivity cases;
- RCI values over controlled scenarios;
- budget/evaluation updates;
- disaster state transitions where deterministic control is possible.

### 2.3 File-format tests

Maintain fixture cities with known checksums and expected properties.

Tests:

- load legacy city;
- inspect expected values;
- save without unrelated mutation;
- reload output;
- compare engine state;
- verify expected byte order/size;
- reject invalid/truncated files safely;
- exercise all scenario files.

### 2.4 Parity/golden simulation tests

Once deterministic seeding/control exists, run a fixed city for fixed tick counts and record a compact state digest.

Digest inputs can include:

- map tile data;
- RCI values;
- funds/tax;
- population aggregates;
- histories;
- scenario counters;
- selected effect maps.

Do not hash pointer addresses, timestamps or presentation state.

Store expected digests with an explanation of the baseline. A changed digest requires explicit review, not automatic replacement.

## 3. Determinism policy

The original simulation uses randomness. Tests need controlled randomness without silently changing production behaviour.

Preferred approach:

- provide an explicit deterministic/test seed path;
- retain historical/random seeding for normal Classic play if that matches baseline behaviour;
- make deterministic runs opt-in for tests/tools;
- document any upstream bug fix that changes initial randomisation.

The current MicropolisCore reproducibility issues are a useful warning: a public `seedRandom()` function is not sufficient if an internal initialisation path reseeds from the wall clock before the test regains control.

## 4. Smoke tests

Maintain a fast smoke test suite that runs on every PR:

- engine initialises;
- blank/new city generates;
- ten/few hundred ticks run without crash;
- basic tool applies;
- save fixture loads;
- save/reload roundtrip succeeds;
- app-level service construction/destruction smoke path succeeds where practical.

## 5. UI and renderer testing

Not every visual detail should be pixel-perfect golden output because graphics backends can vary. Prefer testing deterministic intermediate data and targeted rendering snapshots.

Useful tests:

- screen/world conversion;
- camera zoom anchor;
- minimap viewport rectangle;
- panel layout at representative window sizes;
- DPI scale calculations;
- overlay colour mapping;
- tile atlas indexing.

For selected renderer snapshots, run a controlled renderer/backend and use a tolerance-aware image comparison rather than exact hash if hardware variation affects output.

## 6. Manual compatibility matrix

Before significant releases, test at least:

| Area | Cases |
|---|---|
| Windows scale | 100%, 125%, 150%, 200% |
| Resolution | 1920x1080, 2560x1440, 3840x2160 |
| Window mode | windowed, maximised, borderless/fullscreen |
| Input | mouse, keyboard; controller later if supported |
| Display | single monitor, mixed-DPI dual monitor |
| Save | new save, overwrite, Save As, autosave recovery |
| Audio | default device, device unavailable, device change if supported |

## 7. Static analysis and warnings

### Bootstrap

Enable `/W4` and capture the inherited warning count. Do not attempt to fix every historical warning inside the CMake migration PR.

### Progressive tightening

- fix warnings in touched code;
- enable clang-tidy on modern modules first;
- add narrow suppressions with comments where legacy behaviour requires them;
- eventually make warnings-as-errors possible for the clean modern targets even if a legacy compatibility target remains less strict temporarily.

## 8. Sanitizers and runtime checking

### AddressSanitizer

Use the ASan build to catch invalid memory accesses, use-after-free and buffer problems.

### Resource lifetime

Add explicit tests/counters or diagnostic assertions for SDL resources during repeated UI/session creation/destruction. Visual Studio memory tools can supplement this. The known SDLPP texture-cleanup issue makes shutdown/recreation stress testing particularly important.

### Assertions

Use assertions for impossible internal invariants in debug builds, but validate all external/untrusted data such as city files and config files without relying on debug-only assertions.

## 9. Performance targets

The classic simulation is small by modern standards. Performance regressions should therefore be treated seriously.

Track:

- cold startup time;
- city load time;
- simulation ticks per second in headless benchmark mode;
- render frame time at 1080p/4K;
- memory footprint after repeated new/load cycles;
- autosave latency.

Do not optimise before measuring. The first performance objective is stable frame pacing and responsive UI, not maximum synthetic FPS.

## 10. CI quality gates

### Required for every PR once bootstrap is complete

- x64 Debug configure/build;
- fast tests;
- x64 Release configure/build;
- file-format fixture tests;
- formatting check on applicable files.

### Required for high-risk engine/save PRs

- full parity suite;
- ASan suite;
- known-city long simulation run;
- explicit compatibility note in PR.

## 11. Baseline and bug-fix policy

When discovering a likely original/upstream bug:

1. reproduce it;
2. capture expected/current behaviour;
3. decide whether Classic Mode should preserve or correct it;
4. add a regression test for the chosen rule;
5. document whether a save/simulation output changes;
6. if the fix came from another project, reference its issue/commit.

Classic authenticity does not mean preserving undefined behaviour or memory corruption. Safety fixes should be made; behavioural changes should be deliberate.
