# AGENTS.md - Civic 89

This repository is **Civic 89**, a native Windows modernisation of the GPL Micropolis / original SimCity code lineage.

## Workspace contract

- Repository root: `C:\Dev\Projects\civic89`
- External vcpkg checkout: `C:\Dev\vcpkg`
- Product/display name: `Civic 89`
- Technical identifier/repository: `civic89`
- CMake project: `Civic89`
- Primary executable target: `civic89` -> `civic89.exe`
- Engine target (after extraction): `civic89_engine`
- Tests target: `civic89_tests`
- Primary platform: Windows 11 x64
- Toolchain: Visual Studio Enterprise 2026 / current MSVC / C++20 / CMake / vcpkg / SDL3

`C:\Dev\vcpkg` is external tooling. Never copy it into this repository or add it to Git.

## Read before architectural work

Read all files in `project/engineering/`, especially:

1. `00_README_INDEX.md`
2. `01_PROJECT_CHARTER_AND_DECISIONS.md`
3. `02_UPSTREAM_AUDIT_AND_BASELINE.md`
4. `03_DEVELOPMENT_ENVIRONMENT_WINDOWS.md`
5. `04_REPOSITORY_BUILD_AND_CI.md`
6. `05_ARCHITECTURE_AND_REFACTORING.md`
7. `06_TESTING_QUALITY_AND_PARITY.md`
8. `07_LICENSING_ATTRIBUTION_AND_ASSETS.md`
9. `08_ROADMAP_AND_IMPLEMENTATION_BACKLOG.md`
10. `09_CODING_AGENT_HANDOFF.md`
11. `10_STARTER_CONFIGURATION_TEMPLATES.md`
12. `11_SOURCES_AND_REFERENCES.md`
13. `12_LOCAL_REPOSITORY_BOOTSTRAP.md`

Also read `PROJECT_STATUS.md` for the current task/state.

## Non-negotiable rules

- Preserve the complete Micropolis-SDLPP Git history, exact baseline `9c4e85a0decd57ba6f76d9e1ec82461940ecc3ad`, fixed tag `upstream-sdlpp-baseline`, and the `upstream-sdlpp` remote.
- Preserve inherited GPL/licence/additional-terms files and copyright notices.
- Do not use SimCity or Micropolis as the Civic 89 product brand.
- Do not rewrite the simulation from scratch.
- Do not introduce Unity, Unreal, Electron or a browser runtime.
- Do not mass-format inherited code during infrastructure changes.
- Do not mix source-tree moves with behaviour changes.
- Do not change Classic Mode simulation outcomes without tests and an explicit compatibility decision.
- Do not change the legacy `.cty` format casually.
- Do not add unlicensed retail-game assets, fonts, sounds or icons.
- Do not replace the inherited `Eval()` bridge with another string-command pseudo-API.
- Keep the inherited `.sln`/`.vcxproj` working until the CMake baseline is demonstrably equivalent.

## Provenance checkpoint

The inherited baseline was captured locally on 5 October 2026:

- SHA: `9c4e85a0decd57ba6f76d9e1ec82461940ecc3ad`;
- tag: `upstream-sdlpp-baseline`;
- upstream remote: `upstream-sdlpp` -> `https://github.com/ldicker83/Micropolis-SDLPP.git`;
- working tree: clean at capture.

`PROJECT_STATUS.md` is authoritative for whether the new Civic 89 `origin`, inherited build verification and first engineering branch have been completed.

## First engineering mission

The first engineering branch is `feature/cmake-bootstrap`.

Its goal is only to make CMake/vcpkg a reproducible build path for the existing Windows application. Do not redesign gameplay, UI, audio or architecture in that branch.

Before editing source, inspect the existing `.vcxproj`, `vcpkg.json`, runtime asset assumptions and current build. Record how the proposed CMake targets map to the inherited project.

After work, report exact configure/build/test commands, results, executable path, warnings/errors and any differences from the inherited Visual Studio build.
