# Windows Development Environment and Developer Onboarding

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

## 1. Supported primary workstation

**Verified bootstrap update, 5 October 2026:** Enterprise 2026 18.10.3 is installed. Committed CMake presets use the Visual Studio 18 2026 generator and require CMake 4.2+ (bundled version 4.3.1-msvc1), without a developer shell or Ninja. Current origin/build/asset state supersedes the historical setup snapshots below; see `PROJECT_STATUS.md` and `project/BUILDING.md`.

### Required

- Windows 11 x64, fully updated.
- Visual Studio **Enterprise 2026** with the **Desktop development with C++** workload.
- Git for Windows (already installed on the primary workstation).
- CMake/Ninja support from Visual Studio.
- Standalone vcpkg checkout at `C:\Dev\vcpkg`.
- Civic 89 repository at `C:\Dev\Projects\civic89`.
- A GitHub account with access to the `civic89` repository once its new origin is created.

### Installed Visual Studio components for Civic 89

The primary workstation baseline selected in Visual Studio Enterprise 2026 is:

- C++ core desktop features;
- **MSVC Build Tools for x64/x86 (Latest)**;
- C++ Build Insights;
- Just-In-Time debugger;
- C++ profiling tools;
- C++ CMake tools for Windows;
- MSVC AddressSanitizer;
- **Windows 11 SDK 10.0.26100.8249**;
- vcpkg package manager;
- IntelliCode;
- **C++ Clang tools for Windows 22.1.3 (x64/x86)**.

ATL, MFC, C++/CLI, Boost.Test adapter, Google Test adapter, IntelliTrace, preview/legacy MSVC toolsets, extra legacy SDKs, Incredibuild and Live Share are not required for the baseline. The SDLPP project uses C++20 and a VS2026-era toolset, so the current MSVC toolchain is the least disruptive starting point.

## 2. Authoritative local folder layout

For the primary workstation, this path layout is now **fixed**:

```text
C:\Dev\
├── vcpkg\                         # separate Microsoft/vcpkg Git checkout
└── Projects\
    └── civic89\                   # Civic 89 Git repository root
        ├── .git\
        ├── .github\
        ├── cmake\
        ├── docs\                  # inherited SDLPP documentation
        ├── project\               # Civic 89 project/agent documentation
        │   ├── engineering\
        │   ├── decisions\
        │   ├── roadmap\
        │   └── reference\
        ├── tests\
        ├── tools\
        ├── src\
        ├── images\
        ├── icons\
        ├── res\
        ├── AGENTS.md
        └── PROJECT_STATUS.md
```

`C:\Dev\vcpkg` is **not part of the Civic 89 repository**. It is a separate tooling checkout and must never be added to Civic 89's Git index. Build output will later live under the repository's ignored `out/` directory.

Avoid OneDrive-synchronised folders and paths containing unusual characters for the primary native build workspace.

## 3. Install and bootstrap vcpkg

Open PowerShell:

```powershell
New-Item -ItemType Directory -Force C:\Dev | Out-Null
Set-Location C:\Dev

git clone https://github.com/microsoft/vcpkg.git
Set-Location .\vcpkg
.\bootstrap-vcpkg.bat -disableMetrics
```

Set a user environment variable:

```powershell
[Environment]::SetEnvironmentVariable(
    "VCPKG_ROOT",
    "C:\Dev\vcpkg",
    "User"
)
```

Open a new terminal and confirm:

```powershell
$env:VCPKG_ROOT
& "$env:VCPKG_ROOT\vcpkg.exe" version
```

Do **not** require global `vcpkg integrate install` for the CMake workflow. The repository should explicitly point CMake at the vcpkg toolchain through presets.

## 4. Create the Civic 89 repository from SDLPP history

**Primary workstation status (5 October 2026): complete.** The repository exists at `C:\Dev\Projects\civic89`, the baseline/tag remain `9c4e85a0decd57ba6f76d9e1ec82461940ecc3ad`, `origin` is `https://github.com/DangerMouseUK/civic89.git`, and upstream push is disabled. `feature/cmake-bootstrap` is active. Do not repeat these repository creation steps on the established workspace.

**Do not run `git init` manually.** The repository is created by cloning SDLPP directly into the final Civic 89 folder, which automatically creates `C:\Dev\Projects\civic89\.git`.

```powershell
New-Item -ItemType Directory -Force C:\Dev\Projects | Out-Null
Set-Location C:\Dev\Projects

git clone https://github.com/ldicker83/Micropolis-SDLPP.git civic89
Set-Location C:\Dev\Projects\civic89

git status
git remote rename origin upstream-sdlpp
git rev-parse HEAD
git tag -a upstream-sdlpp-baseline -m "Civic 89: audited SDLPP baseline before modernisation"
```

The commands above are already complete on the primary workstation. Next, create an **empty** GitHub repository named `civic89` (no generated README, licence or `.gitignore`), then add it as the new origin:

```powershell
git remote add origin https://github.com/<owner>/civic89.git
git remote -v
git push -u origin main
git push origin upstream-sdlpp-baseline
```

Expected remotes:

```text
origin          https://github.com/<owner>/civic89.git
upstream-sdlpp  https://github.com/ldicker83/Micropolis-SDLPP.git
```

After the Civic 89 documentation/bootstrap commit and after confirming the inherited `.sln` builds/runs, create the first engineering branch:

```powershell
git switch -c feature/cmake-bootstrap
```

## 5. Configure and build

Once CMake Presets exist:

```powershell
cmake --preset windows-x64-debug
cmake --build --preset windows-x64-debug
ctest --preset windows-x64-debug --output-on-failure
```

Release:

```powershell
cmake --preset windows-x64-release
cmake --build --preset windows-x64-release
ctest --preset windows-x64-release --output-on-failure
```

Visual Studio can also open the repository folder directly and consume CMake Presets. The command-line path remains important because it is the same conceptual path used by CI.

## 6. Debugging configuration

### Debug build

Use the Debug preset for day-to-day debugging. Ensure debug symbols are generated and that the working directory resolves game assets correctly.

### AddressSanitizer preset

Maintain a separate Windows x64 ASan preset. Use it for:

- use-after-free;
- heap/stack buffer overflows;
- invalid object lifetime;
- memory corruption.

Do not rely on MSVC AddressSanitizer as the only leak detector. Use Visual Studio Diagnostic Tools, explicit SDL resource ownership checks and targeted lifetime tests for resource leaks.

### Logging

Use `spdlog` or an equivalent small logging layer. Log at minimum:

- application version and Git revision (for development builds);
- renderer and GPU backend selected by SDL;
- display/DPI information;
- save/load errors;
- audio device initialisation;
- scenario load failures;
- unexpected simulation/file-format assertions.

Do not log personal user paths unnecessarily in release telemetry/log bundles.

## 7. Developer dependency policy

The repository manifest should own the library list. Developers should not manually copy SDL DLLs into source folders or point project settings at local SDK folders.

Planned direct dependencies:

- SDL3;
- SDL3_image;
- SDL3_ttf;
- SDL3_mixer;
- nativefiledialog-extended;
- nlohmann-json;
- Catch2 for tests;
- spdlog for structured logging.

Dependencies should be upgraded in dedicated pull requests so build/test changes are easy to attribute.

## 8. Coding tools

### Required conventions

- `.editorconfig` for line endings/whitespace.
- `.clang-format` for new/materially edited C++.
- `.clang-tidy` configuration introduced progressively.
- UTF-8 source files.
- warnings enabled at a high level (`/W4` on MSVC), but do not enable warnings-as-errors for the inherited tree until the baseline warning debt is classified.

### IDE recommendations

Use Visual Studio's native debugger and profiler first. Additional editors are optional; repository correctness must not depend on editor-specific files.

## 9. First-day onboarding checklist

A developer is successfully onboarded only when all items pass:

- [ ] Visual Studio C++ toolchain installed.
- [ ] `git --version` succeeds.
- [ ] `cmake --version` succeeds.
- [ ] `ninja --version` succeeds or the selected CMake generator is available.
- [ ] `VCPKG_ROOT` is set and vcpkg executes.
- [ ] Repository clones without local patching.
- [ ] Debug configure succeeds.
- [ ] vcpkg restores dependencies automatically.
- [ ] Debug build succeeds.
- [ ] Unit/smoke tests run.
- [ ] Game launches.
- [ ] A city can be created, saved, reloaded and exited cleanly.

## 10. Troubleshooting principles

### Dependency not found

Do not manually add include/lib paths. Check:

1. `VCPKG_ROOT`;
2. preset toolchain path;
3. `vcpkg.json` dependency name;
4. target triplet;
5. pinned baseline compatibility.

### Assets not found at runtime

The build should have an explicit runtime asset-copy/install step. Do not fix asset failures by changing the developer's working directory permanently.

### Different behaviour on another PC

Record:

- project Git SHA;
- vcpkg baseline;
- compiler version;
- Windows build;
- SDL version/backend;
- exact city fixture and checksum.

Reproducibility is a project feature, not a developer responsibility.
