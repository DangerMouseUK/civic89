# Civic 89 - Engineering Documentation Index

**Status:** Active engineering baseline  
**Prepared:** 5 October 2026  
**Repository baseline captured:** 5 October 2026  
**Scope:** **Civic 89** - native Windows modernisation of the open-source Micropolis / original SimCity code lineage

## 1. Purpose of this document set

**Bootstrap update (5 October 2026):** Repository setup, origin/tag push and `feature/cmake-bootstrap` are complete. The inherited `micropolis-sdlpp.sln` builds and now runs with the documented approved font baseline. CMake Debug/Release and runtime/source-list tests pass locally. Earlier capture-point statements in this set are historical; use `PROJECT_STATUS.md`, `project/BUILDING.md` and `project/reference/RUNTIME_ASSETS.md` for current facts and remaining limitations.

This pack is the working engineering specification for **Civic 89**, a polished, modern, native Windows application built from the authentic open-source Micropolis / original SimCity simulation lineage.

The set is deliberately split into focused documents so that it can be used in three ways:

1. as the project planning record for human developers;
2. as the `project/engineering/` foundation inside the source repository; and
3. as a handoff package for a coding agent that needs explicit architectural, build, quality and licensing constraints.

The project should not begin by redesigning gameplay. The first objective is to create a reproducible, testable, native Windows baseline that preserves the existing simulation and save compatibility. Modernisation starts only after that baseline is proven.

## 2. Recommended technical direction

The recommended primary codebase is **Micropolis-SDLPP** by Leeor Dicker. It already provides a native C++20 / SDL3 Windows application, a Visual Studio 2026-era project, native file dialogs, a substantial SDL user interface, the original simulation code, save/load logic, the classic scenarios, graphs, budget, evaluation, a tool palette and a minimap.

The recommended reference codebase is **MicropolisCore** by Don Hopkins / SimHacker. It should be used as a source of simulation fixes, test ideas, `.cty` file-handling improvements and historical reference, but not merged wholesale into the native Windows fork. Its current core boundary is coupled to Emscripten/JavaScript through `emscripten::val`, and its main presentation stack is web-oriented.

The native project therefore follows this model:

```text
Micropolis-SDLPP history and native SDL3 shell
                 |
                 +---- selected fixes/tests from MicropolisCore
                 |
                 v
       Civic 89 maintained C++ simulation core
                 |
          typed C++ interfaces
                 |
     app / renderer / UI / audio / I/O
                 |
              SDL3
                 |
        native Windows executable
```

## 3. Verified snapshot used by this plan

This pack was prepared against the state inspected on 5 October 2026.

- Micropolis-SDLPP **actual Civic 89 cloned baseline**: `9c4e85a0decd57ba6f76d9e1ec82461940ecc3ad`. This exactly matches the `main` head used by the engineering audit.
- MicropolisCore audited `main` head: `f9ae6a57bbe5f5ff94c149bccb3015757f18241d`.
- Local repository clone confirmed at `C:\Dev\Projects\civic89`; `.git` exists at the repository root.
- Clone was on branch `main` with a clean working tree immediately after checkout.
- Inherited remote has been renamed and verified as `upstream-sdlpp` -> `https://github.com/ldicker83/Micropolis-SDLPP.git`.
- Annotated local tag `upstream-sdlpp-baseline` has been created at `9c4e85a0decd57ba6f76d9e1ec82461940ecc3ad`.
- A new Civic 89 `origin` has **not yet** been configured or pushed at this capture point.
- Visual Studio 2026 current September update: 18.10.1, released 15 September 2026.
- SDL current stable release inspected: 3.4.18, released 2 October 2026.
- SDL3_image current stable release inspected: 3.4.6, released 2 September 2026.
- SDL3_mixer stable release inspected: 3.2.4; its current branch requires SDL 3.4.0 or later.
- GitHub Actions supports `windows-2025-vs2026` and Windows 11 ARM64 Visual Studio 2026 runner labels.
- Microsoft recommends vcpkg manifest mode for most projects.

Replace these snapshot values only through an explicit dependency/update change, not casually during unrelated feature work.

## 4. Documents in this set

When copied into the repository, the numbered files belong under `project/engineering/`. Repository-root agent files `AGENTS.md` and `PROJECT_STATUS.md` are supplied separately in this pack.


| Document | Purpose |
|---|---|
| `01_PROJECT_CHARTER_AND_DECISIONS.md` | Goals, non-goals, design principles and architectural decisions |
| `02_UPSTREAM_AUDIT_AND_BASELINE.md` | What already exists, known gaps, fork strategy and baseline criteria |
| `03_DEVELOPMENT_ENVIRONMENT_WINDOWS.md` | Exact Windows developer workstation and onboarding setup |
| `04_REPOSITORY_BUILD_AND_CI.md` | Repository structure, CMake, vcpkg, branches, CI and release engineering |
| `05_ARCHITECTURE_AND_REFACTORING.md` | Target code architecture and staged migration from the inherited code |
| `06_TESTING_QUALITY_AND_PARITY.md` | Classic-mode parity, test strategy, sanitizers and quality gates |
| `07_LICENSING_ATTRIBUTION_AND_ASSETS.md` | GPL obligations, naming/trademarks, source provenance and asset controls |
| `08_ROADMAP_AND_IMPLEMENTATION_BACKLOG.md` | Phased implementation backlog with acceptance criteria |
| `09_CODING_AGENT_HANDOFF.md` | Guardrails and a ready-to-use agent handoff brief |
| `10_STARTER_CONFIGURATION_TEMPLATES.md` | CMake, presets, vcpkg, CI and repository template examples |
| `11_SOURCES_AND_REFERENCES.md` | Primary upstream and tooling references used by the plan |
| `12_LOCAL_REPOSITORY_BOOTSTRAP.md` | Exact `C:\Dev` layout, Git/upstream setup and first coding-agent handoff |

## 5. Project definition of success

The first meaningful release candidate is not defined by new gameplay. It is defined by a high-quality Windows experience that can run the classic simulation reliably.

A successful classic release should provide:

- a signed or signable native Windows x64 executable;
- clean install and portable build options;
- a resizable, DPI-aware, high-refresh/VSync-aware main window;
- smooth pan and zoom while retaining crisp pixel-art rendering;
- working original construction tools and simulation systems;
- working budget, evaluation, graphs, overlays and minimap;
- complete sound implementation;
- reliable `.cty` load/save compatibility and scenarios;
- autosave and crash-safe save handling;
- clean shutdown with no known invalid memory/resource lifetime bugs;
- automated regression tests around the simulation and file format;
- documented GPL/source and third-party asset compliance;
- no dependency on Tcl/Tk, Emscripten, a browser runtime, Electron, Unity or Unreal.

## 6. Product naming

The project name is now **Civic 89**. Use the following identifiers consistently:

| Purpose | Value |
|---|---|
| Product/display name | `Civic 89` |
| Repository name | `civic89` |
| Local repository root | `C:\Dev\Projects\civic89` |
| CMake project | `Civic89` |
| Primary executable | `civic89.exe` |
| C++ project namespace/prefix | `civic89` |
| Main engine target | `civic89_engine` |
| Test target | `civic89_tests` |
| vcpkg root | `C:\Dev\vcpkg` |

Do not use **SimCity** as the shipping product name; use it only for factual historical description. Do not treat **Micropolis** as the Civic 89 product brand. A preliminary collision search found no obvious exact game/software project called Civic 89, but this is **not formal trademark clearance**; complete a proper trademark/brand review before high-profile public or commercial release.

## 7. Current bootstrap position

The first engineering milestone is **Baseline Bootstrap**. The local repository has now crossed the first provenance checkpoint:

- [x] Clone Micropolis-SDLPP directly into `C:\Dev\Projects\civic89`, preserving complete history.
- [x] Confirm `.git` exists inside the project root; no manual `git init` was used.
- [x] Rename inherited `origin` to `upstream-sdlpp`.
- [x] Capture exact baseline SHA `9c4e85a0decd57ba6f76d9e1ec82461940ecc3ad` and create annotated tag `upstream-sdlpp-baseline`.
- [x] Create an empty Civic 89 GitHub repository and add it as the new `origin`.
- [x] Push `main` and `upstream-sdlpp-baseline`.
- [x] Verify the inherited `micropolis-sdlpp.sln` builds and runs with approved font staging (historical file compatibility remains unproven).
- [x] Add this documentation/agent pack and make the first Civic 89-owned documentation commit.
- [x] Create `feature/cmake-bootstrap`.
- [x] Introduce CMake, pinned/reproducible vcpkg use, CI and smoke tests without altering gameplay (local fresh-source and hosted Debug/Release checks pass; see `PROJECT_STATUS.md`).

These prerequisites were completed before engineering work. Current verification and remaining inherited limitations are in `PROJECT_STATUS.md`.
