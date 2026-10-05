# Civic 89 - Project Status

**Current phase:** M1-02 typed audio controls on `codex/typed-audio-control`
**Updated:** 5 October 2026
**Current product name:** Civic 89  
**Repository root:** `C:\Dev\Projects\civic89`  
**External vcpkg root:** `C:\Dev\vcpkg`

## Authoritative local repository state

- Branch: `codex/typed-audio-control`, created locally from synchronized `main`; no upstream/push yet.
- Local and remote `main`: merge commit `b066399b18ccc66bf893e5eb8e42703eac671839` from [PR #2](https://github.com/DangerMouseUK/civic89/pull/2), merged by the user on 5 October 2026. Bootstrap PR #1 remains in its history.
- The merged local `codex/eval-inventory` was safely deleted with `git branch -d`; GitHub had already deleted its remote branch. The earlier bootstrap branch is also deleted. No other stale feature branches existed. Upstream refs and the baseline tag are preserved.
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

Baseline Bootstrap and [PR #2](https://github.com/DangerMouseUK/civic89/pull/2) are merged. **M1-01** inventory/design (`8f7d3e1` / `8b4d5c2`) and the first **M1-02** earthquake audio route (`9baadda`) are now on `main`. PR #2's [second CI attempt](https://github.com/DangerMouseUK/civic89/actions/runs/37371023429/attempts/2) passed both configurations after the initial runner-allocation failure. [Merged-main CI](https://github.com/DangerMouseUK/civic89/actions/runs/37379422558) also passed for `b066399`.

- [x] [Inventory](project/reference/LEGACY_EVAL_INVENTORY.md) all 12 live `Eval` calls plus the inactive options call, including reachability, operands and intended replacements.
- [x] [Record typed interfaces](project/decisions/0001_TYPED_PRESENTATION_BOUNDARY.md) and the migration/compatibility decisions. The first audio seam is implemented; the broader design remains proposed.
- [x] Link actual inherited `w_sound.cpp` and `w_tk.cpp` into `civic89_legacy_tests`; characterize routing, mute/initialization defects, bulldozer loop state and earthquake start/stop ordering.
- [x] Configure/build Debug and Release; all **3/3 CTest checks pass in each configuration**. Exact commands remain in [project/BUILDING.md](project/BUILDING.md); logs are `out/audit/eval-{configure,build,test}-{debug,release}.log`.
- [x] Add the minimal typed `AudioService` and application-owned null adapter; inject it into the earthquake helper through the inherited application entry point.
- [x] Migrate only the earthquake low-explosion sound request, retaining lazy initialization, sound-before-visual order and the existing shake/timer state changes. The visual `Eval` and other audio paths remain.
- [x] Verify the typed sound/channel and dispatch-time state with a recording test adapter; verify the production null adapter stays silent and preserves the saved sound flag.
- [x] Build Debug/Release and pass **3/3 CTests in each** after migration; retained Visual Studio Release also builds. Commands are in [project/BUILDING.md](project/BUILDING.md); logs are `out/audit/typed-audio-{build,test}-{debug,release}.log` and `out/audit/typed-audio-inherited-release.log`.
- [x] Synchronize local `main` with `git pull --ff-only` after PR #2 and delete the merged branch.
- [x] Characterize cold `SoundOff` and uninitialized stop guards before migration (`f25acee`); targeted tests pass against the old controls in both configurations.
- [x] Add typed loop-start, loop-stop and stop-all methods to the recording/null adapters; migrate `StartBulldozer`, `StopBulldozer` and `SoundOff` with their existing guards/state ordering.
- [x] Retire the internal `DoStartSound` / `DoStopSound` string wrappers after their callers migrate. Live `Eval` expressions decrease from **12 to 9**. No dormant control is enabled in the UI.
- [x] Build Debug/Release and pass **3/3 CTests in each** after control migration; retained Visual Studio Release also builds. Logs are `out/audit/typed-control-{build,test}-{debug,release}.log` and `out/audit/typed-control-inherited-release.log`.
- [ ] Migrate remaining audio operands and introduce typed presentation/navigation adapters with tests (M1-02).
- [ ] Establish a scenario loader seam, validate all eight fixtures and test native start/failure paths (M1-03). There is currently no working scenario startup smoke path.
- [ ] Delete the bridge only after all entries are covered or deliberately retired (M1-04).

The current production increment touches only `main.cpp`, `w_sound.cpp` / `.h` and `AudioService.h`. The application owns the null adapter and retains no-argument entry points; tests exercise the overloads with explicit services. The 47 translation units, retained `.sln`/`.vcxproj`, disaster damage/RNG, tools/sprites, save implementation and disabled visual timers are unchanged. Migrated controls no longer print string commands; playback remains silent and inherited mute/initialization behavior is preserved. New audio/test code compiles without warnings; rebuilt inherited code emits existing warnings, including the two C4100 warnings in the legacy test target and four CS1668 environment warnings in the inherited Release build.

## Verified environment and dependency baseline

- Installed Visual Studio Enterprise 2026: **18.10.3**. CMake selects the installed VS 2026 BuildTools instance, MSVC **19.51.36260.0**, toolset directory **14.51.36231**.
- Bundled CMake: **4.3.1-msvc1**. Presets require CMake 4.2+ for the VS 2026 generator; no developer shell/Ninja needed.
- vcpkg executable version: `2026-09-26-51bf87ca6e9bf3e622d84ff323bd202ab1ca0c0b`.
- Clean external checkout/manifest registry baseline: **19780d9cdf84d0944cf9a318666703b89ab6629c**. The executable version and registry commit are different identifiers.
- Original five direct dependencies retained: SDL3 3.4.18, SDL3_image 3.4.4#1, SDL3_ttf 3.2.2#1, nativefiledialog-extended 1.4.1, nlohmann-json 3.12.0#2. No production dependencies added.

Exact commands/target mapping: [project/BUILDING.md](project/BUILDING.md). Local evidence: [tests/baseline/BOOTSTRAP_2026-10-05.md](tests/baseline/BOOTSTRAP_2026-10-05.md). Asset provenance and replacement decision: [project/reference/RUNTIME_ASSETS.md](project/reference/RUNTIME_ASSETS.md), `assets/runtime-assets.json`, `assets/ASSET-LICENSES.yml`.

Bootstrap and the first typed earthquake route are merged, with local and hosted build/test evidence retained. No upstream push or history/tag rewrite occurred. Hosted CI verifies separate clean runner builds; manual GUI testing remains on the primary workstation. The current typed audio-control increment has local Debug/Release evidence and has not been pushed or checked by hosted CI. No new GUI or actual audio-device test was run for this narrow route; recording/null helper tests and both build paths provide its evidence.

## Inherited limitations and release gates

- Missing fonts were an inherited packaging defect, not a Civic 89 regression. Startup now works with approved fonts/substitute. Original developer font versions and pixel-identical text metrics cannot be established.
- **Historical `.cty` compatibility is unproven.** The inherited writer uses native 32-bit arrays and produced a **51,360-byte** save; some supplied cities are **27,120 bytes**. `FileIo.cpp` does not validate short reads. A current-version round trip does not establish historical/scenario compatibility. Investigate separately with tests; no format change is made here.
- `icons/LICENSE.txt` supplies OpenSVG attribution without identifying original icon sets/licences. Public binary redistribution remains blocked on that audit. Other retained graphics/fixtures preserve inherited project-level GPL/additional-terms provenance, not a completed per-asset rights review.
- Remaining string effects/scenario/earthquake visual paths retain the stubbed `Eval()` bridge. Earthquake sound and the dormant bulldozer/sound-off controls use typed audio. The [inventory](project/reference/LEGACY_EVAL_INVENTORY.md) records disconnected scenario startup, empty tool-error commands, an ineffective mute setter, dormant message audio/win paths and disabled earthquake timers. Tests cover the audio/earthquake helpers, not real playback, visual effects or scenario loading. Resource lifetime concerns remain inherited; normal GUI exit testing does not prove leak-free teardown.
- `/W4` exposes **50 inherited warnings per configuration**: 40 C4100, one C4189, two C4389, one C4456, six C4459. The inherited `/W3` comparison build reported none. No warning-as-error policy introduced.
- A final inherited MSBuild rerun passed after the manifest change with four CS1668 environment warnings: two missing `LIB` search directories reported twice by Roslyn inline tasks (Enterprise ATL/MFC and `lib\um\x64`). These are workstation/global-integration warnings, not C++ errors; no unrelated environment repair was made.
- Original UI/window/resource branding is retained for comparison. `civic89.exe` is the new target name; public product branding/packaging remains later work.
- Retained `.sln`/`.vcxproj` remain unedited and build after the first typed route. Catch2, engine extraction, deterministic simulation digests, ASan and exhaustive UI/asset compliance testing remain later milestones.
