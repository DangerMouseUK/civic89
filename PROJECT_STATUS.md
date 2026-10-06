# Civic 89 - Project Status

**Current phase:** M0–M7 merged; M8 faithfulness/Windows polish in progress; optional M9 graphics planned
**Updated:** 6 October 2026
**Current product name:** Civic 89  
**Repository root:** `C:\Dev\Projects\civic89`  
**External vcpkg root:** `C:\Dev\vcpkg`

## Authoritative local repository state

- Engineering branch: `codex/m8-faithfulness-windows-polish`, delivering M8-01 through M8-06 together, including the agreed scope documentation.
- Scope selected by the user: faithful original gameplay/mechanics, modern Windows presentation and optional improved graphics only. The earlier Enhanced gameplay candidates are withdrawn; [ADR 0009](project/decisions/0009_FAITHFUL_MODERNISATION_AND_GRAPHICS.md) records the decision.
- M7 delivery: merged [PR #12](https://github.com/DangerMouseUK/civic89/pull/12). Final head `6d73e53` passed all five jobs in [run 37494533588](https://github.com/DangerMouseUK/civic89/actions/runs/37494533588): application Debug/Release/ASan and native ARM64 **53/53**, headless ASan **43/43**. Both Release jobs passed complete packaging/update/install acceptance.
- Local and remote `main`: `eb73f64230e35c2f0aaf8134c4cd5e6100ed155d`, the user-merged M7 PR. The recorded CI result verifies the final M7 branch head.
- Synced with fetch/prune and a fast-forward to origin/main; verified M7 branch-head ancestry and deleted merged `codex/m7-enhanced-foundation`. Origin had already deleted it. The uncommitted planning update was carried into M8; its empty planning branch is removed. Upstream refs/tag are preserved.
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

## Completed M7

**M7 - Enhanced Mode foundation.** All five items were delivered together in merged PR #12.

- [x] M7-01: supported Classic v1 development contract, with historical/replay/platform limitations and unchanged public-release gates.
- [x] M7-02: typed versioned registry, exact ruleset identities/capabilities and rejection of unknown modes before mutation. Enhanced v1 currently uses Classic mechanics.
- [x] M7-03: new-city mode selection, active UI/runner identity, CLI selection and Classic scenario policy; cancelling a pending selection preserves the live mode.
- [x] M7-04: unchanged `.cty` layout, bounded/checksummed `.c89`, explicit import/export, atomic failure preservation and separate mode-aware recovery slots.
- [x] M7-05: complete local and hosted native acceptance. Debug/Release and application ASan pass **53/53** each; headless ASan and corresponding-source export pass **43/43**. All 25 Classic goldens and four generated-city parity cases are unchanged. Real ZIP/installer/update/rollback, both save formats' retention, production Enhanced smoke under ASan and retained Visual Studio Release pass.

[ADR 0008](project/decisions/0008_M7_ENHANCED_FOUNDATION.md),
[Classic contract](project/CLASSIC_COMPATIBILITY.md),
[Enhanced format](project/ENHANCED_CITY_FORMAT.md) and
[M7 evidence](tests/baseline/M7_2026-10-06.md) record the foundation and acceptance.
M0–M7 are merged. The current application still exposes the M7 city-mode interface;
both identities use Classic mechanics. Its adjustment is planned in M8, not a redo
of M7. Public asset/brand/physical acceptance and trusted signing provisioning remain
external gates; development candidates are unsigned.

## Current engineering task and remaining milestones

Deliver the complete M8 scope alongside the user's revised roadmap, charter, agent
instructions, README and compatibility documents. Record implementation, tests and
actual desktop evidence before claiming acceptance.
The [roadmap](project/engineering/08_ROADMAP_AND_IMPLEMENTATION_BACKLOG.md) defines:

- [ ] **M8 - Faithfulness and Windows polish:** original-mechanics audit/regressions,
  historical-save investigation, physical desktop/display/input/audio/dialog checks
  and complete milestone validation. Includes simplifying the merged M7 interface
  around one gameplay contract while preserving existing `.c89`/`.cty` workflows,
  recovery, CLI compatibility and failure isolation.
- [ ] **M9 - Optional enhanced graphics:** agreed faithful art direction/licensed
  assets, Classic/Enhanced graphics in Settings, complete rendering integration,
  performance/desktop acceptance and unchanged state/RNG/timing/save output. Graphics
  may switch during play and never select a different simulation or require conversion.

M9 is optional and requires agreed graphics adoption/art direction before implementation.
No larger maps, wider limits, additional buildings/tools/scenarios, changed traffic/
utilities/balance, mods, achievements/challenges or day/night/seasons are planned.
One whole implementation milestone remains the unit of each engineering branch/PR.
Public release additionally needs asset/brand clearance, trusted signing and remaining
physical clean-machine acceptance; these gates are not cleared by this plan.

## Verified environment and dependency baseline

- Installed Visual Studio Enterprise 2026: **18.10.3**. CMake selects the installed VS 2026 BuildTools instance, MSVC **19.51.36260.0**, toolset directory **14.51.36231**.
- Bundled CMake: **4.3.1-msvc1**. Presets require CMake 4.2+ for the VS 2026 generator; no developer shell/Ninja needed.
- vcpkg executable version: `2026-09-26-51bf87ca6e9bf3e622d84ff323bd202ab1ca0c0b`.
- Clean external checkout/manifest registry baseline: **19780d9cdf84d0944cf9a318666703b89ab6629c**. The executable version and registry commit are different identifiers.
- Original five direct dependencies retained: SDL3 3.4.18, SDL3_image 3.4.4#1, SDL3_ttf 3.2.2#1, nativefiledialog-extended 1.4.1, nlohmann-json 3.12.0#2. M3 adds SDL3_mixer 3.2.4 with optional codec features disabled at the same registry baseline.

Exact commands/target mapping: [project/BUILDING.md](project/BUILDING.md). Local evidence: [tests/baseline/BOOTSTRAP_2026-10-05.md](tests/baseline/BOOTSTRAP_2026-10-05.md). Asset provenance and replacement decision: [project/reference/RUNTIME_ASSETS.md](project/reference/RUNTIME_ASSETS.md), `assets/runtime-assets.json`, `assets/ASSET-LICENSES.yml`.

M0–M7 are merged. M7 added explicit ruleset and save compatibility boundaries around
the completed native interface and delivery tooling. The engine remains platform-free. All 122 staged
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
- Retained `.sln`/`.vcxproj` build with the M2 split/support files, M3/M4/M5 services and M7 container codec. Engine extraction, deterministic digests and ASan are verified. Exhaustive desktop/DPI/resource/asset compliance testing remains later work; automated native tests use SDL dummy/software rendering.
