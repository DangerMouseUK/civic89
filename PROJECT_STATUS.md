# Civic 89 - Project Status

**Current phase:** M1-01 Eval inventory and replacement design on `codex/eval-inventory`
**Updated:** 5 October 2026
**Current product name:** Civic 89  
**Repository root:** `C:\Dev\Projects\civic89`  
**External vcpkg root:** `C:\Dev\vcpkg`

## Authoritative local repository state

- Branch: `codex/eval-inventory`, created locally from synchronized `main`; no upstream/push yet.
- Local and remote `main`: merge commit `b6fc77e561bb3bbcc60c75603b942e755f4992a9` from [PR #1](https://github.com/DangerMouseUK/civic89/pull/1), merged by the user on 5 October 2026.
- The merged local `feature/cmake-bootstrap` was safely deleted with `git branch -d`; its remote branch had already been deleted. No other stale feature branches existed. Upstream refs and the baseline tag are preserved.
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

Baseline Bootstrap is merged. The next bounded increment completes **M1-01**, proposes the typed boundary for M1-02/M1-03 and adds inherited audio/earthquake characterization before production migration:

- [x] [Inventory](project/reference/LEGACY_EVAL_INVENTORY.md) all 12 live `Eval` calls plus the inactive options call, including reachability, operands and intended replacements.
- [x] [Propose typed interfaces](project/decisions/0001_TYPED_PRESENTATION_BOUNDARY.md) and a migration sequence without adding unused production scaffolding.
- [x] Link actual inherited `w_sound.cpp` and `w_tk.cpp` into `civic89_legacy_tests`; characterize routing, mute/initialization defects, bulldozer loop state and earthquake start/stop ordering.
- [x] Configure/build Debug and Release; all **3/3 CTest checks pass in each configuration**. Exact commands remain in [project/BUILDING.md](project/BUILDING.md); logs are `out/audit/eval-{configure,build,test}-{debug,release}.log`.
- [ ] Introduce production adapters and migrate one tested route (M1-02).
- [ ] Establish a scenario loader seam, validate all eight fixtures and test native start/failure paths (M1-03). There is currently no working scenario startup smoke path.
- [ ] Delete the bridge only after all entries are covered or deliberately retired (M1-04).

No inherited C++ edits, UI redesign, simulation changes, save-format changes or source moves in this increment. The design remains proposed. New test code compiles without warnings; the extra compilation of inherited `w_sound.cpp` repeats two existing C4100 warnings for unused `MakeSoundOn` parameters per configuration.

## Verified environment and dependency baseline

- Installed Visual Studio Enterprise 2026: **18.10.3**. CMake selects the installed VS 2026 BuildTools instance, MSVC **19.51.36260.0**, toolset directory **14.51.36231**.
- Bundled CMake: **4.3.1-msvc1**. Presets require CMake 4.2+ for the VS 2026 generator; no developer shell/Ninja needed.
- vcpkg executable version: `2026-09-26-51bf87ca6e9bf3e622d84ff323bd202ab1ca0c0b`.
- Clean external checkout/manifest registry baseline: **19780d9cdf84d0944cf9a318666703b89ab6629c**. The executable version and registry commit are different identifiers.
- Original five direct dependencies retained: SDL3 3.4.18, SDL3_image 3.4.4#1, SDL3_ttf 3.2.2#1, nativefiledialog-extended 1.4.1, nlohmann-json 3.12.0#2. No production dependencies added.

Exact commands/target mapping: [project/BUILDING.md](project/BUILDING.md). Local evidence: [tests/baseline/BOOTSTRAP_2026-10-05.md](tests/baseline/BOOTSTRAP_2026-10-05.md). Asset provenance and replacement decision: [project/reference/RUNTIME_ASSETS.md](project/reference/RUNTIME_ASSETS.md), `assets/runtime-assets.json`, `assets/ASSET-LICENSES.yml`.

Bootstrap review and merge are complete, with local and hosted build/test evidence retained. No upstream push or history/tag rewrite occurred. Hosted CI verifies separate clean runner builds; manual GUI testing remains on the primary workstation. The subsequent inventory/design increment has local Debug/Release evidence and has not been pushed or checked by hosted CI.

## Inherited limitations and release gates

- Missing fonts were an inherited packaging defect, not a Civic 89 regression. Startup now works with approved fonts/substitute. Original developer font versions and pixel-identical text metrics cannot be established.
- **Historical `.cty` compatibility is unproven.** The inherited writer uses native 32-bit arrays and produced a **51,360-byte** save; some supplied cities are **27,120 bytes**. `FileIo.cpp` does not validate short reads. A current-version round trip does not establish historical/scenario compatibility. Investigate separately with tests; no format change is made here.
- `icons/LICENSE.txt` supplies OpenSVG attribution without identifying original icon sets/licences. Public binary redistribution remains blocked on that audit. Other retained graphics/fixtures preserve inherited project-level GPL/additional-terms provenance, not a completed per-asset rights review.
- Audio/scenario/earthquake paths retain the stubbed `Eval()` bridge. The [inventory](project/reference/LEGACY_EVAL_INVENTORY.md) records disconnected scenario startup, empty tool-error commands, an ineffective mute setter, dormant message audio/win paths and disabled earthquake timers. Characterization tests cover the audio/earthquake helpers, not real playback, visual effects or scenario loading. Resource lifetime concerns remain inherited; normal GUI exit testing does not prove leak-free teardown.
- `/W4` exposes **50 inherited warnings per configuration**: 40 C4100, one C4189, two C4389, one C4456, six C4459. The inherited `/W3` comparison build reported none. No warning-as-error policy introduced.
- A final inherited MSBuild rerun passed after the manifest change with four CS1668 environment warnings: two missing `LIB` search directories reported twice by Roslyn inline tasks (Enterprise ATL/MFC and `lib\um\x64`). These are workstation/global-integration warnings, not C++ errors; no unrelated environment repair was made.
- Original UI/window/resource branding is retained for comparison. `civic89.exe` is the new target name; public product branding/packaging remains later work.
- No inherited C++ source or `.sln`/`.vcxproj` edits. Catch2, engine extraction, deterministic simulation digests, ASan and exhaustive UI/asset compliance testing remain later milestones.
