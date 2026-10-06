# Civic 89 - Project Status

**Current phase:** Milestone 5 complete on `codex/m5-modern-ui`; hosted CI is tracked on the delivered PR
**Updated:** 6 October 2026
**Current product name:** Civic 89  
**Repository root:** `C:\Dev\Projects\civic89`  
**External vcpkg root:** `C:\Dev\vcpkg`

## Authoritative local repository state

- Engineering branch: `codex/m5-modern-ui`, delivering all six M5 items together. Continue complete milestones per branch/PR.
- Local and remote `main`: `fc0c4f653daa582902ef0db3d329c8e2eff06021`, user-merged [PR #9](https://github.com/DangerMouseUK/civic89/pull/9) on 6 October 2026. [M4 hosted CI](https://github.com/DangerMouseUK/civic89/actions/runs/37466981537) passed Debug/Release/application ASan 42/42 and headless ASan 34/34 for `0e766c9` before merge.
- Synced with fetch/prune, switch main and ff-only pull. Verified ancestry, deleted `codex/m4-window-camera-rendering` locally; origin had already deleted it. No stale feature branches remain. Upstream refs/tag are preserved.
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

**M5 - Modern UI/UX is complete.** The branch contains M5-01 through M5-06,
acceptance checks and compatibility notes. M6 follows after review/merge.

- [x] **M5-01:** In-window minimap on the main renderer, shared overlay colors/sources, camera viewport and click/drag navigation. No detached window is created.
- [x] **M5-02:** Responsive dashboard with funds, date, population, numeric RCI, active tool/cost and status/tooltip messages.
- [x] **M5-03:** Existing tool icons/ghosts, scalable palette, keyboard selection and tooltips, live affordability, free inspection during debt, and cancellation of pending drags when tools change.
- [x] **M5-04:** New budget, evaluation, short/long history and query panels retain engine data and typed actions. In-window scenarios and new-city confirmation complete the interface.
- [x] **M5-05:** Fourteen data choices including traffic, crime, land value, pollution, population, power, protection, growth, transport and zones. Full map and minimap share the read-only model; opacity, legends, numeric probes and a blue sequential palette are available.
- [x] **M5-06:** Atomic validated UI preferences; 100/125/150% requested scale, large text, high contrast, 30 configurable unique tool/command/pan bindings, keyboard traversal, camera speed/reverse zoom, three volume categories and display/gameplay controls.
- [x] Application Debug/Release and application ASan pass **43/43** checks each; independent headless Debug/ASan pass **35/35** each. All 25 M2 goldens remain unchanged. Final UI refinements pass 2/2 targeted checks in each application configuration; no sanitizer findings.
- [x] Native hidden-window Windows software and Direct3D 11 UI/camera/mode/reset/twelve-session checks pass. Render captures visually reviewed at compact/wide layouts and accessibility settings. Physical mixed-DPI monitors and assistive-technology integration remain release validation.
- [x] Retained Visual Studio Release builds all **64** production source entries (31 engine / 33 application). No source moves, new dependencies, simulation algorithm changes, runtime assets or city-format changes.
- [ ] Hosted Debug/Release/headless ASan/application ASan verification is recorded on the delivered PR after completion; this file is the pre-publication snapshot.

[ADR 0006](project/decisions/0006_M5_MODERN_INTERFACE.md) records UI/runtime compatibility
and limits. [M5 evidence](tests/baseline/M5_2026-10-06.md) records commands,
results, rendered captures, paths and inherited differences. [BUILDING.md](project/BUILDING.md)
documents controls and preferences. Earlier [M4 evidence](tests/baseline/M4_2026-10-06.md)
and [M3 evidence](tests/baseline/M3_2026-10-06.md) retain the preceding decisions.

## Verified environment and dependency baseline

- Installed Visual Studio Enterprise 2026: **18.10.3**. CMake selects the installed VS 2026 BuildTools instance, MSVC **19.51.36260.0**, toolset directory **14.51.36231**.
- Bundled CMake: **4.3.1-msvc1**. Presets require CMake 4.2+ for the VS 2026 generator; no developer shell/Ninja needed.
- vcpkg executable version: `2026-09-26-51bf87ca6e9bf3e622d84ff323bd202ab1ca0c0b`.
- Clean external checkout/manifest registry baseline: **19780d9cdf84d0944cf9a318666703b89ab6629c**. The executable version and registry commit are different identifiers.
- Original five direct dependencies retained: SDL3 3.4.18, SDL3_image 3.4.4#1, SDL3_ttf 3.2.2#1, nativefiledialog-extended 1.4.1, nlohmann-json 3.12.0#2. M3 adds SDL3_mixer 3.2.4 with optional codec features disabled at the same registry baseline.

Exact commands/target mapping: [project/BUILDING.md](project/BUILDING.md). Local evidence: [tests/baseline/BOOTSTRAP_2026-10-05.md](tests/baseline/BOOTSTRAP_2026-10-05.md). Asset provenance and replacement decision: [project/reference/RUNTIME_ASSETS.md](project/reference/RUNTIME_ASSETS.md), `assets/runtime-assets.json`, `assets/ASSET-LICENSES.yml`.

M0–M4 are merged. This branch completes M5 with a single-window native interface,
shared overlays and validated UI preferences. The engine remains platform-free. All 122 staged
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
- The main window and new dialogs identify Civic 89; inherited UI art/resources remain for comparison. `civic89.exe` is the new target name; public product branding/packaging remains later work.
- Retained `.sln`/`.vcxproj` build with the M2 split/support files and M3/M4/M5 services. Engine extraction, deterministic digests and ASan are verified. Exhaustive desktop/DPI/resource/asset compliance testing remains later work; automated native tests use SDL dummy/software rendering.
