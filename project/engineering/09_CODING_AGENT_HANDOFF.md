# Coding Agent Handoff and Operating Instructions

**Status:** Active engineering baseline  
**Prepared:** 5 October 2026  
**Repository baseline captured:** 5 October 2026  
**Scope:** **Civic 89** - native Windows modernisation of the open-source Micropolis / original SimCity code lineage

## 1. Mission

Build **Civic 89** from the authentic open-source Micropolis simulation into a polished native Windows game while preserving Classic Mode compatibility. The immediate assignment is infrastructure and baseline preservation, not feature invention.

## 2. Authoritative planning documents

Repository root: `C:\Dev\Projects\civic89`.

Read these before changing code (they live under `project/engineering/`):

1. `00_README_INDEX.md`
2. `01_PROJECT_CHARTER_AND_DECISIONS.md`
3. `02_UPSTREAM_AUDIT_AND_BASELINE.md`
4. `03_DEVELOPMENT_ENVIRONMENT_WINDOWS.md`
5. `04_REPOSITORY_BUILD_AND_CI.md`
6. `05_ARCHITECTURE_AND_REFACTORING.md`
7. `06_TESTING_QUALITY_AND_PARITY.md`
8. `07_LICENSING_ATTRIBUTION_AND_ASSETS.md`
9. `08_ROADMAP_AND_IMPLEMENTATION_BACKLOG.md`

When code and planning disagree, do not silently reinterpret the plan. Record the conflict in the PR/implementation notes and choose the smallest reversible change.

## 3. Non-negotiable guardrails

- Preserve the existing Git history.
- Do not rewrite the simulation from scratch.
- Do not introduce Unity, Unreal, Electron or a browser runtime.
- Do not migrate the codebase to Rust/C#/JavaScript.
- Do not mass-format inherited files in infrastructure PRs.
- Do not mix file moves with behavioural changes unless unavoidable.
- Do not remove inherited licence/attribution files.
- Product brand is **Civic 89**. Do not use SimCity or Micropolis as the Civic 89 product brand.
- Do not add unlicensed fonts, sounds, icons or retail-game assets.
- Do not alter the legacy `.cty` format casually.
- Do not replace the `Eval()` bridge with another string-keyed command system.
- Do not make Classic Mode behaviour changes without tests and an explicit compatibility note.

## 4. Local workspace contract

- Civic 89 repository root: `C:\Dev\Projects\civic89`.
- vcpkg root: `C:\Dev\vcpkg` (`VCPKG_ROOT` should point here).
- The Civic 89 `.git` directory is inside the repository root.
- Authoritative inherited baseline: `9c4e85a0decd57ba6f76d9e1ec82461940ecc3ad` tagged locally as `upstream-sdlpp-baseline`.
- Preserved upstream remote: `upstream-sdlpp` -> `https://github.com/ldicker83/Micropolis-SDLPP.git`.
- `C:\Dev\vcpkg` is external tooling and must never be committed or copied into Civic 89.
- Preserve remote names: `origin` for Civic 89, `upstream-sdlpp` for the inherited SDLPP repository. `origin` must be configured before the coding agent starts branch work.
- During the first CMake bootstrap, leave the inherited `.sln`/`.vcxproj` intact as a known-good comparison path.

## 5. First assignment: Baseline Bootstrap

### Deliverables

- top-level CMake build that reproduces the existing Windows app;
- CMake Presets for x64 Debug and Release;
- vcpkg manifest with pinned baseline/version strategy;
- successful clean build using Visual Studio 2026/MSVC v145;
- Catch2 test target;
- at least one non-trivial smoke test where feasible without refactoring behaviour;
- GitHub Actions build/test workflow using the VS2026 Windows runner;
- docs noting any source files that cannot yet cleanly fit the target architecture;
- no intentional gameplay changes.

### Expected PR separation

Prefer:

- PR 1: CMake/vcpkg migration;
- PR 2: CI/test harness;
- PR 3: baseline warning/resource diagnostics;

Do not also modernise UI or `Eval()` in PR 1.

## 6. Before coding

Inspect:

- current `.vcxproj` source lists and configuration;
- `vcpkg.json`;
- `src/main.cpp`;
- `src/w_tk.cpp`;
- `src/w_sound.cpp`;
- `src/FileIo.cpp`;
- `src/UI/*` ownership/lifecycle;
- asset-copy behaviour and runtime working-directory assumptions.

Produce a concise implementation note describing how the CMake build will reproduce the existing project before editing source files.

## 7. Commit discipline

Good examples:

```text
build: add CMake bootstrap for existing SDLPP sources
build: move dependencies to pinned vcpkg manifest
ci: build and test win-x64 on VS2026 runner
test: add city load/save smoke fixture
```

Avoid commits like:

```text
huge cleanup
modernise everything
format code and fix bugs
```

Each commit should be independently understandable.

## 8. Evidence required from agent work

For infrastructure changes report:

- commands used to configure/build/test;
- compiler/tool versions;
- generated executable path;
- test results;
- any warning changes;
- any runtime/manual smoke tests performed;
- files intentionally left under the legacy build only, if any.

For simulation changes additionally report:

- fixture used;
- before/after state;
- parity impact;
- upstream issue/commit if the change was ported.

## 9. Stop conditions

Pause architectural expansion and raise a clear issue if any of the following is discovered:

- an asset's redistribution right cannot be established;
- the inherited file format appears to differ from expected Classic compatibility;
- CMake build changes runtime behaviour compared with the original VS project;
- a proposed engine separation requires changing simulation logic merely to compile;
- an upstream bug fix changes city outcomes in Classic Mode without a documented decision.

## 10. Ready-to-paste first-agent prompt

```text
You are working on **Civic 89**, a native Windows modernisation of the GPL Micropolis/SimCity Classic code lineage. The repository root is `C:\Dev\Projects\civic89`; the external vcpkg checkout is `C:\Dev\vcpkg`. The repository preserves Micropolis-SDLPP history and uses its audited main baseline as the starting point.

The current working branch for your first engineering task is `feature/cmake-bootstrap`. Your first task is Baseline Bootstrap only. Do not redesign the UI, rewrite simulation logic, or remove legacy callback plumbing yet.

Goals:
1. Make CMake the build-system source of truth for Civic 89 while preserving the current source layout.
2. Reproduce the existing Windows x64 Debug and Release application using MSVC/C++20.
3. Use vcpkg manifest mode with a pinned baseline/version strategy. Existing direct dependencies include SDL3, SDL3_ttf, SDL3_image, nativefiledialog-extended and nlohmann-json. Add Catch2 for tests and spdlog only if/when it is actually used. Do not manually vendor SDL binaries.
4. Add CMake Presets for windows-x64-debug and windows-x64-release. Prepare an ASan preset if it can be added without destabilising the bootstrap.
5. Add a Catch2 test target and the safest meaningful smoke test that does not require behavioural refactoring.
6. Add GitHub Actions using the Windows VS2026 runner to configure, build and test Debug and Release.
7. Preserve all inherited licensing/attribution files.
8. Avoid mass formatting or source moves.

Before changing code, inspect the existing .vcxproj and vcpkg manifest and write a short implementation note explaining how your CMake targets will map to the existing build. After implementation, report exact configure/build/test commands and results, plus any known differences from the original Visual Studio project.
```

## 11. Second-agent mission after baseline is green

Only after Baseline Bootstrap is merged, the next job is to inventory `Eval()` and design typed replacements. That work should begin by adding tests or demonstrable smoke paths for scenario start, earthquake effects and audio event routing before deleting legacy call sites.
