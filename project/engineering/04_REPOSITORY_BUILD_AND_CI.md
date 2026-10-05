# Repository, Build System, Branching, CI and Release Engineering

**Status:** Active engineering baseline  
**Prepared:** 5 October 2026  
**Repository baseline captured:** 5 October 2026  
**Scope:** **Civic 89** - native Windows modernisation of the open-source Micropolis / original SimCity code lineage

## 1. Repository principles

The repository root is **`C:\Dev\Projects\civic89`** on the primary workstation. It must make a clean checkout buildable without manual IDE surgery while preserving upstream history and clearly separating inherited code, Civic 89 modernisation work and third-party provenance.

## 2. Proposed repository layout

The layout should evolve in two stages.

### Stage A - bootstrap without a giant move

```text
/                              # C:\Dev\Projects\civic89
  AGENTS.md                    # coding-agent constitution
  PROJECT_STATUS.md            # short current-state/next-task record
  CMakeLists.txt               # added during bootstrap
  CMakePresets.json            # added during bootstrap
  vcpkg.json
  vcpkg-configuration.json     # if required
  COPYING / inherited licence files
  README.md                    # inherited initially; later Civic 89 landing README
  docs/                        # inherited SDLPP docs - do not repurpose during bootstrap
  project/
    engineering/               # this numbered documentation set
    decisions/                 # ADRs / decision notes
    roadmap/
    reference/
      UPSTREAMS.md
  src/                         # initially close to SDLPP layout
  images/
  icons/
  res/
  tests/
  tools/
  cmake/
  .github/
    workflows/
```

### Stage B - after baseline tests exist

```text
src/
  app/
  engine/
  render/
  audio/
  ui/
  io/
  platform/
    windows/

tests/
  unit/
  integration/
  parity/
  fixtures/
```

Do not perform Stage B as part of the initial build-system conversion.

## 3. CMake target model

### Bootstrap target model

First prove the old source can be built by CMake with minimal source movement.

```text
civic89  -> inherited SDLPP source + SDL dependencies
```

### Target model after engine extraction

```text
civic89_engine        # static/shared library, no SDL
civic89               # primary executable target -> civic89.exe
civic89_render        # SDL renderer/camera layer
civic89_ui            # UI widgets/panels
civic89_audio         # SDL3_mixer integration
civic89_io            # city/settings/save services
civic89_tests         # Catch2 tests, primarily engine/io
```

Preferred dependency direction:

```text
civic89 -> civic89_ui/civic89_render/civic89_audio/civic89_io -> civic89_engine
                                                   civic89_io -> civic89_engine
                                                civic89_tests -> civic89_engine/civic89_io
civic89_engine -> C++ standard library only (or the smallest justified utilities)
```

The engine must not depend on UI, SDL, Win32, file dialogs or audio.

## 4. vcpkg policy

Use manifest mode. Commit:

- `vcpkg.json`;
- `vcpkg-configuration.json` if required;
- a pinned `builtin-baseline` or equivalent version lock.

Do not commit `vcpkg_installed/`.

Dependency upgrades should be isolated PRs and should include a successful full CI run.

## 5. Preset policy

Commit standard CMake presets:

- `windows-x64-debug`;
- `windows-x64-release`;
- `windows-x64-asan`;
- later `windows-arm64-release`.

Each preset gets a matching build preset and, where useful, test preset. Developers may create user-specific `CMakeUserPresets.json`, but that file must remain untracked.

## 6. Git root and remotes

The Git repository starts at exactly:

```text
C:\Dev\Projects\civic89
```

Its metadata directory is:

```text
C:\Dev\Projects\civic89\.git
```

Because Civic 89 is created with `git clone ... civic89`, **do not run `git init`**. `C:\Dev\vcpkg` is a different Git repository and is never a parent/child Git dependency of Civic 89.

Current local Git record (5 October 2026):

```text
repository root        C:\Dev\Projects\civic89
baseline SHA           9c4e85a0decd57ba6f76d9e1ec82461940ecc3ad
baseline tag           upstream-sdlpp-baseline
preserved remote       upstream-sdlpp -> https://github.com/ldicker83/Micropolis-SDLPP.git
origin                  not yet configured at capture
```

Required remote naming once the new GitHub repository is added:

```text
origin          -> our Civic 89 repository
upstream-sdlpp  -> https://github.com/ldicker83/Micropolis-SDLPP.git
```

## 7. Branch model

Use trunk-based development around `main`.

```text
main
  feature/cmake-bootstrap
  feature/typed-events
  feature/audio
  feature/camera
  fix/city-file-endian
```

Do not create a permanent `develop` branch unless team size/workflow later proves it necessary.

### Rules for `main`

- always expected to build;
- required CI checks before merge;
- protected from direct force pushes;
- release tags point to reviewed commits;
- no generated build output committed.

## 8. Pull request discipline

Each PR should answer:

1. What behaviour or architecture changes?
2. What inherited/upstream behaviour is being preserved?
3. What tests prove the change?
4. Does it affect save compatibility or simulation parity?
5. Does it add/replace an asset or dependency?
6. Is there an upstream issue/commit being ported?
7. Are screenshots needed for a UI change?

Prefer small, reviewable commits. Avoid mixing formatting, file moves and behaviour changes.

## 9. Suggested labels

```text
area:engine
area:render
area:ui
area:audio
area:io
area:build
area:ci
area:licensing

kind:bug
kind:feature
kind:refactor
kind:test
kind:upstream-port
kind:dependency

compat:classic
compat:save-format
risk:high
```

## 10. CI pipeline

Use the GitHub-hosted Visual Studio 2026 Windows runner label `windows-2025-vs2026` for the main x64 pipeline.

Minimum pull-request pipeline:

```text
checkout
  -> restore/bootstrap pinned vcpkg
  -> configure Debug
  -> build Debug
  -> run tests
  -> configure Release
  -> build Release
  -> run release-safe tests
```

Recommended additional jobs after bootstrap:

- clang-format check for touched/new files;
- clang-tidy on selected targets;
- ASan smoke run;
- packaged-app smoke test;
- dependency licence inventory generation;
- ARM64 compile job once supported.

Cache vcpkg binary artifacts only if the cache key includes the manifest/baseline and relevant compiler/architecture information.

## 11. Build provenance

Development/release builds should expose a version string containing at least:

- semantic project version;
- Git commit short SHA;
- clean/dirty indicator for local dev builds if feasible;
- build architecture.

Example:

```text
0.3.0-dev+8f3a21c (win-x64)
```

Do not bake full developer usernames or absolute source paths into user-visible diagnostics.

## 12. Versioning

Use semantic versioning for our project:

- `0.x` while architecture and save extensions are unstable;
- `1.0.0` when Classic Mode has a supported compatibility contract and release-quality Windows packaging.

Save-format extensions must have their own explicit versioning rules and migration strategy. Do not equate app version with city file version.

## 13. Packaging

### Development artifacts

Produce a portable ZIP first. It is easy to inspect and avoids installer complexity while the application is evolving.

### Release packaging

Later provide a normal Windows installer using an explicit packaging system such as Inno Setup or WiX. Include:

- executable and required runtime libraries;
- assets;
- licence/notice files;
- uninstall support;
- Start Menu entry;
- optional desktop shortcut;
- clean per-user settings/save locations.

### Code signing

Structure release automation so the unsigned artifact can be built reproducibly and a signing step can be added without changing binaries for unrelated reasons.

## 14. Release workflow

A release tag should only be created after:

- all CI gates pass;
- classic parity suite passes;
- clean-machine launch test passes;
- save/load fixture tests pass;
- licensing/asset ledger is current;
- package contents are reviewed;
- version/CHANGELOG updated;
- source corresponding to the GPL binary release is available.
