# Civic 89 - Project Status

**Current phase:** Milestone 2 complete on `codex/m2-engine-boundary`; PR delivery and hosted verification follow local gates
**Updated:** 6 October 2026
**Current product name:** Civic 89  
**Repository root:** `C:\Dev\Projects\civic89`  
**External vcpkg root:** `C:\Dev\vcpkg`

## Authoritative local repository state

- Engineering branch: `codex/m2-engine-boundary`, completing all M2 items in one PR. Continue using complete milestones per branch/PR.
- Local and remote `main`: `de424f37959ca9864ffdebb5f4517a1dc5998e4f`, user-merged [PR #6](https://github.com/DangerMouseUK/civic89/pull/6) on 6 October 2026. M1's [hosted CI](https://github.com/DangerMouseUK/civic89/actions/runs/37432056603) passed Debug/Release 7/7 checks on `810b859` before merge.
- Synced main with `git fetch origin --prune`, `git switch main`, `git pull --ff-only origin main`. Verified the M1 branch was an ancestor and deleted it with `git branch -d`; origin had already deleted it. No stale feature branches remain. Upstream refs/tag are preserved.
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

**M2 - Engine Boundary and Testability is complete locally.** This branch includes the entire milestone, its baseline capture, compatibility decisions, runner, tests and CI gates. M3 functional completion is the next milestone after this PR is reviewed/merged.

- [x] **M2-01:** Audit direct and transitive platform/presentation dependencies in [ENGINE_DEPENDENCIES.md](project/reference/ENGINE_DEPENDENCIES.md).
- [x] **M2-02:** Compile the platform-free simulation once into `civic89_engine`; native application and headless tools link it. Engine-only presets need no vcpkg, SDL, Win32/RC, JSON assets, textures or fonts. The include/import gate enforces the boundary. Retained Visual Studio project includes eight support/split files, with all 47 original source paths preserved.
- [x] **M2-03:** `civic89_runner` loads a current-format city or packaged scenario, generates terrain, steps N inherited phases and reports summary/digest. Strict arguments, optional seed/speed control, missing/invalid file errors and clock-range checks are included.
- [x] **M2-04:** Opt-in seeding of the existing RNG, without changing normal-play seeding/distributions. GUI scheduling and independent animation remain intact; injectable message clock and silent typed defaults allow headless operation.
- [x] **M2-05:** Capture 25 golden cases from merged M1 **before extraction** (all eight scenarios at 0/16/1,024 phases and Detroit at 16,384). Extracted Debug/Release/ASan results match their original references; preserve Bern's existing `/fp:fast` difference.
- [x] Final Debug and Release configure/build plus **39/39 CTests** each. Final engine-only Debug and ASan RelWithDebInfo configure/build plus **32/32 CTests** each, including long-run parity and no sanitizer findings.
- [x] Native dummy/software SDL tests retain all M1 lifecycle checks and exercise sprite cache draw/release/recreation. Inherited Visual Studio Release build passes.
- [x] Current-format byte-size/tile-order/own-save-load tests and transactional rejection of truncated, oversized, corrupt-tile or unsafe-speed city inputs. No writer format change; valid-city history reset remains inherited behavior pending M3.
- [x] Source comparisons confirm 13 simulation/data implementations unchanged, only an unused SDL include removed from `s_sim.cpp`, and all eight sprite updates unchanged apart from equal frame-count substitution.
- [ ] Verify hosted Debug/Release/ASan CI on the delivered PR; record its result in the PR description after completion.

[ADR 0003](project/decisions/0003_M2_ENGINE_BOUNDARY.md) documents compatibility decisions and limits. [M2 evidence](tests/baseline/M2_2026-10-06.md) records the pre-extraction capture, commands, results, paths and warning debt. [BUILDING.md](project/BUILDING.md) documents application/headless/ASan commands and runner use. M1's [ADR 0002](project/decisions/0002_M1_COMPLETION.md) and [evidence](tests/baseline/M1_2026-10-06.md) remain historical checkpoints.

## Verified environment and dependency baseline

- Installed Visual Studio Enterprise 2026: **18.10.3**. CMake selects the installed VS 2026 BuildTools instance, MSVC **19.51.36260.0**, toolset directory **14.51.36231**.
- Bundled CMake: **4.3.1-msvc1**. Presets require CMake 4.2+ for the VS 2026 generator; no developer shell/Ninja needed.
- vcpkg executable version: `2026-09-26-51bf87ca6e9bf3e622d84ff323bd202ab1ca0c0b`.
- Clean external checkout/manifest registry baseline: **19780d9cdf84d0944cf9a318666703b89ab6629c**. The executable version and registry commit are different identifiers.
- Original five direct dependencies retained: SDL3 3.4.18, SDL3_image 3.4.4#1, SDL3_ttf 3.2.2#1, nativefiledialog-extended 1.4.1, nlohmann-json 3.12.0#2. No production dependencies added.

Exact commands/target mapping: [project/BUILDING.md](project/BUILDING.md). Local evidence: [tests/baseline/BOOTSTRAP_2026-10-05.md](tests/baseline/BOOTSTRAP_2026-10-05.md). Asset provenance and replacement decision: [project/reference/RUNTIME_ASSETS.md](project/reference/RUNTIME_ASSETS.md), `assets/runtime-assets.json`, `assets/ASSET-LICENSES.yml`.

M0 and M1 are merged. This branch completes M2, preserving original source paths and adding eight implementation support/split files plus the runner. No production dependencies, asset changes, upstream push or history/tag rewrite occurred. Hosted verification is delivered with the PR.

## Inherited limitations and release gates

- Missing fonts were an inherited packaging defect, not a Civic 89 regression. Startup now works with approved fonts/substitute. Original developer font versions and pixel-identical text metrics cannot be established.
- **Historical `.cty` compatibility is unproven.** The inherited writer uses native 32-bit arrays and produced a **51,360-byte** save; some supplied cities are **27,120 bytes**. M2 rejects wrong-size/corrupt city input before mutation using the existing 32-bit decoder. Current-version serialization/load tests do not establish historical import compatibility or full-state restoration. The inherited city-load history reset and writer error/atomicity limitations remain M3 work; no format change is made here.
- `icons/LICENSE.txt` supplies OpenSVG attribution without identifying original icon sets/licences. Public binary redistribution remains blocked on that audit. Other retained graphics/fixtures preserve inherited project-level GPL/additional-terms provenance, not a completed per-asset rights review.
- Audio playback, camera auto-goto and earthquake shake/timing remain silent/no-op presentation adapters for later milestones. M1 removes all command plumbing and adds working native scenario startup; it does not add an audio/renderer backend. Dormant notification audio and inherited mute/initialization semantics remain unchanged. M2 supplies explicit RNG control, deterministic digests and read validation. Wider resource lifetime, full city-state restoration and safe saving remain M3 work.
- Historical bootstrap `/W4` capture exposed **50 inherited warnings per configuration**: 40 C4100, one C4189, two C4389, one C4456, six C4459. The inherited `/W3` comparison build reported none. No warning-as-error policy introduced.
- A final inherited MSBuild rerun passed after the manifest change with four CS1668 environment warnings: two missing `LIB` search directories reported twice by Roslyn inline tasks (Enterprise ATL/MFC and `lib\um\x64`). These are workstation/global-integration warnings, not C++ errors; no unrelated environment repair was made.
- Original UI/window/resource branding is retained for comparison. `civic89.exe` is the new target name; public product branding/packaging remains later work.
- Retained `.sln`/`.vcxproj` build with the M2 split/support files. Engine extraction, deterministic digests and ASan are verified. Exhaustive desktop/DPI/resource/asset compliance testing remains later work; automated native tests use SDL dummy/software rendering.
