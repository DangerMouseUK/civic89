# Civic 89 - Project Status

**Current phase:** M6 release engineering and root README complete on `codex/m6-release-engineering`; final checks tracked on PR #11
**Updated:** 6 October 2026
**Current product name:** Civic 89  
**Repository root:** `C:\Dev\Projects\civic89`  
**External vcpkg root:** `C:\Dev\vcpkg`

## Authoritative local repository state

- Engineering branch: `codex/m6-release-engineering`, delivering M6-01 through M6-05 plus the root README together.
- Delivery: [PR #11](https://github.com/DangerMouseUK/civic89/pull/11). Hosted [run 37484465857](https://github.com/DangerMouseUK/civic89/actions/runs/37484465857) passes all five jobs on `3bbea9a`, including complete native ARM64/x64 delivery acceptance and candidate upload. Final documentation/signing-history checks are tracked on the PR.
- Local and remote `main`: `8db4b1e51e40f35764edb7566b098f96dcedfaf6`, user-merged [PR #10](https://github.com/DangerMouseUK/civic89/pull/10). M5 hosted CI passed all four jobs on `b781fd7` ([run](https://github.com/DangerMouseUK/civic89/actions/runs/37474183236)): application 43/43, headless ASan 35/35.
- Synced with fetch/prune and ff-only pull; verified ancestry and deleted the merged `codex/m5-modern-ui` branch. Origin had already deleted it. No stale feature branches remain; upstream refs/tag are preserved.
- `.git`: confirmed at `C:\Dev\Projects\civic89\.git`.
- Baseline SHA: `9c4e85a0decd57ba6f76d9e1ec82461940ecc3ad`.
- Baseline tag: `upstream-sdlpp-baseline` (annotated; verified locally and on origin).
- Preserved upstream remote: `upstream-sdlpp` -> `https://github.com/ldicker83/Micropolis-SDLPP.git`.
- Civic 89 `origin`: `https://github.com/DangerMouseUK/civic89.git`.
- Upstream push URL remains `DISABLED`. Complete inherited history is preserved.
- Existing documentation/policy commits: `72608d8`, `a36e360`.
- Bootstrap commits now on `main`: `d83be2c` (assets), `ae0ba9d` (build/tests), `2ca122b` (CI), `9f63d86` and `a47cd47` (evidence/status).
- The actual cloned SHA exactly matches the SDLPP SHA used by the engineering audit.

## Bootstrap checklist

- [x] Install/select the Visual Studio Enterprise 2026 C++ development environment.
- [x] Confirm Git is installed.
- [x] Clone Micropolis-SDLPP directly into `C:\Dev\Projects\civic89` (no manual `git init`).
- [x] Confirm `.git` exists inside the project root.
- [x] Rename inherited `origin` to `upstream-sdlpp`.
- [x] Capture exact HEAD `9c4e85a0decd57ba6f76d9e1ec82461940ecc3ad`.
- [x] Create annotated tag `upstream-sdlpp-baseline`.
- [x] Configure Civic 89 origin; verify remote `main`, branch and baseline tag.
- [x] Verify inherited **micropolis-sdlpp.sln** / **micropolis-cpp.vcxproj** Release x64 build again before changes.
- [x] Commit the initial project/agent documentation on `main`.
- [x] Create `feature/cmake-bootstrap` and hand the workspace to the coding agent.
- [x] Inventory 122 runtime asset/notice files, including 82 textures and four font paths.
- [x] Restore missing Raleway fonts from a pinned author repository under OFL 1.1, with copyright/licence.
- [x] Exclude original Virtue due to restrictive bundled terms; use the documented unmodified Raleway Bold alias at `res/virtue.ttf` for both builds.
- [x] Configure/build CMake x64 Debug and Release via explicit pinned vcpkg toolchain; disable global integration on generated projects.
- [x] Pass both CTest checks in both configurations: inherited runtime font/data/texture smoke and all 47 source-file entries matching the `.vcxproj`.
- [x] Inherited GUI smoke: map, date advancement, construction, minimap, native save/open, own-save reload and normal exit.
- [x] CMake Release GUI comparison: map, date advancement, construction, minimap, native save/open, own-save reload and normal exit.
- [x] CMake Debug and portable Release installation startup/date/exit checks (exit code 0).
- [x] Stage all 122 assets from a Git archive, without ignored workstation files.
- [x] Configure/build/test both presets from a fresh full-source Git archive of `9f63d86`, with MSBuild vcpkg integration disabled in that process; 2/2 tests pass in each configuration.
- [x] Add Debug/Release GitHub Actions on the VS 2026 Windows runner after local build/run checks; validate YAML locally.
- [x] Verify hosted CI: [run 37317705607](https://github.com/DangerMouseUK/civic89/actions/runs/37317705607) passed Debug and Release configure/build plus 2/2 tests in each configuration for `9f63d86`, using separate VS 2026 Windows runners and pinned external vcpkg checkouts.
- [x] Capture a fresh-source Release screenshot and verify its normal exit with code 0.
- [x] Record commands, results, inherited differences and generated save fixtures under `tests/baseline/`.
- [x] Merge PR #1, synchronize local `main` with `git pull --ff-only`, and remove the merged feature branch.
- [x] Verify merged-main hosted CI: [run 37355668174](https://github.com/DangerMouseUK/civic89/actions/runs/37355668174) passed for `b6fc77e`.

## Current engineering task

**M6 - Release Engineering plus the root README.** All five items share this branch/PR.

- [x] Version `0.6.0-dev`, commit/architecture/dirty provenance, original Civic 89 executable resources, AUTHORS/NOTICE/CHANGELOG and rewritten README; original upstream README preserved.
- [x] Portable ZIP + matching source + hashes; selected-compiler release CRT and complete asset/dependency notices; strict per-file/PE/identity checks and real packaged smoke path.
- [x] Per-user Inno Setup installer, uninstall/user-data preservation and install/upgrade/removal acceptance harness.
- [x] Separate protected manual signing integration. Public gates fail closed; no signing key/certificate is created or exported.
- [x] Offline portable update staging, flushed atomic selection, previous-version retention and rollback; no automatic network updater before stable releases exist.
- [x] ARM64 Release preset and native build/test/package CI. Native 43/43 tests, ZIP launch, update/rollback and installer acceptance pass.
- [x] Initial local Debug/Release builds and **43/43** tests in each, including all 25 Classic goldens.
- [x] Complete x64/ARM64 packages, transaction and installer acceptance; source-archive build 35/35; application ASan 43/43; all five hosted CI jobs pass, including 25 unchanged Classic goldens.

[ADR 0007](project/decisions/0007_M6_RELEASE_ENGINEERING.md),
[M6 evidence](tests/baseline/M6_2026-10-06.md), [RELEASING.md](project/RELEASING.md)
and [BUILDING.md](project/BUILDING.md) record acceptance and compatibility.
M0–M5 are merged; M6 is delivered in PR #11. Public release rights, brand/physical
acceptance and trusted signing provisioning remain explicit gates; engineering
candidates are unsigned. M7 is the remaining planned milestone and requires a
supported Classic contract before Enhanced Mode work starts.

## Verified environment and dependency baseline

- Installed Visual Studio Enterprise 2026: **18.10.3**. CMake selects the installed VS 2026 BuildTools instance, MSVC **19.51.36260.0**, toolset directory **14.51.36231**.
- Bundled CMake: **4.3.1-msvc1**. Presets require CMake 4.2+ for the VS 2026 generator; no developer shell/Ninja needed.
- vcpkg executable version: `2026-09-26-51bf87ca6e9bf3e622d84ff323bd202ab1ca0c0b`.
- Clean external checkout/manifest registry baseline: **19780d9cdf84d0944cf9a318666703b89ab6629c**. The executable version and registry commit are different identifiers.
- Original five direct dependencies retained: SDL3 3.4.18, SDL3_image 3.4.4#1, SDL3_ttf 3.2.2#1, nativefiledialog-extended 1.4.1, nlohmann-json 3.12.0#2. M3 adds SDL3_mixer 3.2.4 with optional codec features disabled at the same registry baseline.

Exact commands/target mapping: [project/BUILDING.md](project/BUILDING.md). Local evidence: [tests/baseline/BOOTSTRAP_2026-10-05.md](tests/baseline/BOOTSTRAP_2026-10-05.md). Asset provenance and replacement decision: [project/reference/RUNTIME_ASSETS.md](project/reference/RUNTIME_ASSETS.md), `assets/runtime-assets.json`, `assets/ASSET-LICENSES.yml`.

M0–M5 are merged. M6 adds versioned release tooling and delivery checks around the
completed native interface. The engine remains platform-free. All 122 staged
assets/notices are retained; new audio is original GPL source-generated PCM.
No upstream push, history rewrite or tag modification occurred.

## Inherited limitations and release gates

- Missing fonts were an inherited packaging defect, not a Civic 89 regression. Startup now works with approved fonts/substitute. Original developer font versions and pixel-identical text metrics cannot be established.
- **Historical `.cty` compatibility is unproven.** The inherited writer uses native 32-bit arrays and produced a **51,360-byte** save; some supplied cities are **27,120 bytes**. M2 rejects wrong-size/corrupt city input before mutation using the existing 32-bit decoder. Current-version serialization/load tests do not establish historical import compatibility or full-state restoration. M3 fixes history/difficulty restoration and safe publication without changing the byte layout. Post-load scans still run; RNG, sprites and scenario progress are not serialized.
- `icons/LICENSE.txt` supplies OpenSVG attribution without identifying original icon sets/licences. Public binary redistribution remains blocked on that audit. Other retained graphics/fixtures preserve inherited project-level GPL/additional-terms provenance, not a completed per-asset rights review.
- M3 supplies functional audio, mute/volume, resource teardown and reliable native city saves. M4 supplies camera auto-goto, timed earthquake presentation, DPI and rendering changes. Original procedural effects are used; no music asset is introduced.
- Autosave/recovery covers ordinary cities. Automatic scenario autosave is skipped because the inherited format cannot restore scenario objectives/deadlines. Explicit scenario exports retain ordinary-city behavior. Desktop audibility/native-dialog interaction are not established by dummy-driver tests.
- Historical bootstrap `/W4` capture exposed **50 inherited warnings per configuration**: 40 C4100, one C4189, two C4389, one C4456, six C4459. The inherited `/W3` comparison build reported none. No warning-as-error policy introduced.
- A final inherited MSBuild rerun passed after the manifest change with four CS1668 environment warnings: two missing `LIB` search directories reported twice by Roslyn inline tasks (Enterprise ATL/MFC and `lib\um\x64`). These are workstation/global-integration warnings, not C++ errors; no unrelated environment repair was made.
- The main window and new dialogs identify Civic 89; inherited UI art/resources remain for comparison. `civic89.exe` is the new target name; M6 adds Civic 89 executable resources and development packaging; public release clearance remains gated.
- Retained `.sln`/`.vcxproj` build with the M2 split/support files and M3/M4/M5 services. Engine extraction, deterministic digests and ASan are verified. Exhaustive desktop/DPI/resource/asset compliance testing remains later work; automated native tests use SDL dummy/software rendering.
