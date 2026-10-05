# Civic 89 Local Repository Bootstrap

**Status:** Authoritative local bootstrap procedure  
**Prepared:** 5 October 2026  
**Product:** Civic 89  
**Repository:** `civic89`

## 1. The answer to "where does Git start?"

The Civic 89 Git repository root is exactly:

```text
C:\Dev\Projects\civic89
```

Its Git metadata lives at:

```text
C:\Dev\Projects\civic89\.git
```

However, **do not run `git init`** for the initial Civic 89 setup. We need to preserve the complete Micropolis-SDLPP history, so the correct operation is to **clone SDLPP directly into the final `civic89` folder**. `git clone` creates `.git` automatically.

`C:\Dev\vcpkg` is separate tooling. It is its own Microsoft/vcpkg Git checkout and must never be placed inside Civic 89 or committed to Civic 89.

## 2. Final local layout

```text
C:\Dev\
├── vcpkg\
└── Projects\
    └── civic89\
        ├── .git\
        ├── .github\
        │   └── workflows\
        ├── cmake\
        ├── docs\                  # existing upstream SDLPP docs
        ├── project\
        │   ├── engineering\       # this documentation set
        │   ├── decisions\
        │   ├── roadmap\
        │   └── reference\
        │       └── UPSTREAMS.md
        ├── tests\
        ├── tools\
        ├── src\
        ├── images\
        ├── icons\
        ├── res\
        ├── AGENTS.md
        ├── PROJECT_STATUS.md
        ├── COPYING
        ├── __README_OG
        ├── README.md
        ├── vcpkg.json
        ├── micropolis-cpp.sln
        └── micropolis-cpp.vcxproj
```

Do not create the future `src/app`, `src/engine`, `src/render`, etc. hierarchy manually before baseline tests. That restructuring is engineering work for later reviewed commits.

## 3. Create the repository locally

**Primary workstation status (5 October 2026): completed successfully.** The clone exists at `C:\Dev\Projects\civic89`; `git status` reported `main` up to date with the inherited remote and a clean working tree, and `C:\Dev\Projects\civic89\.git` was explicitly confirmed to exist.

For repeat/new-machine onboarding, open PowerShell:

```powershell
New-Item -ItemType Directory -Force C:\Dev\Projects | Out-Null
Set-Location C:\Dev\Projects

git clone https://github.com/ldicker83/Micropolis-SDLPP.git civic89
Set-Location C:\Dev\Projects\civic89

git status
git remote -v
```

At this point the folder is already a Git repository.

## 4. Preserve the upstream remote and baseline

**Primary workstation status (5 October 2026): completed successfully.**

```text
Branch:           main
Working tree:     clean at capture
Baseline SHA:     9c4e85a0decd57ba6f76d9e1ec82461940ecc3ad
Baseline tag:     upstream-sdlpp-baseline (annotated, created locally)
Remote:           upstream-sdlpp -> https://github.com/ldicker83/Micropolis-SDLPP.git
New origin:       not yet configured
```

The commands used/retained for repeat onboarding are:

```powershell
git remote rename origin upstream-sdlpp

git rev-parse HEAD
git tag -a upstream-sdlpp-baseline -m "Civic 89: audited SDLPP baseline before modernisation"

git remote -v
git tag --list
```

Expected remote at this moment:

```text
upstream-sdlpp  https://github.com/ldicker83/Micropolis-SDLPP.git
```

The exact SHA returned by `git rev-parse HEAD` is `9c4e85a0decd57ba6f76d9e1ec82461940ecc3ad`. It matches the SHA used in the earlier engineering audit, so there is no audit/clone discrepancy. `project/reference/UPSTREAMS.md` records this value as authoritative. The `upstream-sdlpp-baseline` tag is intentionally fixed at this commit even if upstream moves later.

## 5. Create the Civic 89 GitHub origin

**Current next repository action:** this step is still pending. No `origin` existed when the baseline record above was captured.

Create a new repository named:

```text
civic89
```

For the initial project, making it **Private** is sensible. Create it **empty**:

- no generated README;
- no generated `.gitignore`;
- no generated licence.

Then:

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

## 6. Verify the inherited build before the coding agent changes it

Open:

```text
C:\Dev\Projects\civic89\micropolis-cpp.sln
```

in Visual Studio Enterprise 2026. Build **x64 Release** (and Debug if practical), then run the inherited application.

Record whether it:

- compiles successfully;
- launches;
- creates a city;
- permits basic zoning/road placement;
- saves a city;
- reloads the save;
- exits without an obvious crash.

This gives the CMake migration a known-good comparison path.

## 7. Add the Civic 89 project/agent documentation

Do not overwrite the inherited `docs/` directory. Civic 89-specific planning lives under:

```text
project\engineering\
project\decisions\
project\roadmap\
project\reference\
```

Copy the numbered engineering Markdown documents into `project\engineering\` and put `UPSTREAMS.md` in `project\reference\`.

At repository root add:

```text
AGENTS.md
PROJECT_STATUS.md
```

Then create the empty bootstrap directories if not already present:

```powershell
New-Item -ItemType Directory -Force .\project\engineering | Out-Null
New-Item -ItemType Directory -Force .\project\decisions | Out-Null
New-Item -ItemType Directory -Force .\project\roadmap | Out-Null
New-Item -ItemType Directory -Force .\project\reference | Out-Null
New-Item -ItemType Directory -Force .\tests | Out-Null
New-Item -ItemType Directory -Force .\tools | Out-Null
New-Item -ItemType Directory -Force .\cmake | Out-Null
New-Item -ItemType Directory -Force .\.github\workflows | Out-Null
```

Git does not track empty directories, so empty bootstrap directories may require a `.gitkeep` only if there is a real reason to preserve them before files land. Do not add meaningless placeholders merely to make the tree look complete.

## 8. Make the first Civic 89-owned commit

After copying the documentation and root agent files:

```powershell
git status
git add AGENTS.md PROJECT_STATUS.md project
# Add other intentional bootstrap files only if they actually contain content.
git status
git commit -m "docs: establish Civic 89 engineering baseline"
git push origin main
```

Do **not** include unrelated generated Visual Studio files, build output, vcpkg packages or accidental source changes.

## 9. Create the first engineering branch

Only after the inherited build has been checked and the documentation baseline is on `main`:

```powershell
git switch -c feature/cmake-bootstrap
git push -u origin feature/cmake-bootstrap
```

The first coding-agent task is deliberately narrow: reproduce the inherited native Windows application through CMake/vcpkg without changing gameplay, moving the source tree wholesale or deleting the existing Visual Studio build path.

## 10. What folder to give the local ChatGPT coding agent

Give the coding agent access to:

```text
C:\Dev\Projects\civic89
```

Do **not** give it `C:\Dev` as the project root. `C:\Dev\vcpkg` is an external tool dependency and should be referenced through `VCPKG_ROOT`, not treated as part of the source workspace.

Before coding, the agent must read:

```text
AGENTS.md
PROJECT_STATUS.md
project\engineering\00_README_INDEX.md
project\engineering\01_PROJECT_CHARTER_AND_DECISIONS.md
...
project\engineering\12_LOCAL_REPOSITORY_BOOTSTRAP.md
```

## 11. First-agent task boundary

The first coding-agent assignment is **Baseline Bootstrap only**:

- inspect the inherited `.vcxproj`, source list and `vcpkg.json`;
- add a top-level CMake build without gameplay changes;
- add CMake presets for x64 Debug/Release;
- keep the original `.sln`/`.vcxproj` intact until CMake parity is proven;
- add the safest initial Catch2 smoke-test target;
- establish CI after local build parity;
- do not redesign UI, audio, simulation, saves or folder architecture yet.
