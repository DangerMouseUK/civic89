# Civic 89 - Project Status

**Current phase:** Milestone 1 complete on `codex/m1-remove-legacy-plumbing`; published for PR review; hosted CI pending
**Updated:** 6 October 2026
**Current product name:** Civic 89  
**Repository root:** `C:\Dev\Projects\civic89`  
**External vcpkg root:** `C:\Dev\vcpkg`

## Authoritative local repository state

- Branch: `codex/m1-remove-legacy-plumbing`, published and tracking origin. Commits `1f49500` / `c51174d` complete M1. The user has authorized complete milestones per branch/PR going forward.
- Local and remote `main`: merge commit `d2b278a357293d35fb1d4f21efcaa990cfc54c93` from [PR #5](https://github.com/DangerMouseUK/civic89/pull/5), merged by the user on 6 October 2026. Bootstrap and earlier typed routes remain in its history.
- The merged local `codex/typed-earthquake-presentation` was safely deleted with `git branch -d`; its deleted remote reference was pruned. At cleanup, origin had only `main`; no stale feature branches remained. Upstream refs and the baseline tag are preserved.
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

**M1 - Remove Legacy Command Plumbing is complete.** The user's delivery unit is now one whole milestone per branch/PR, recorded in `AGENTS.md` and the roadmap. M2 engine extraction is the next milestone and is not mixed into this PR.

Earlier typed increments are merged through PRs #2-#5. PR #5's [CI](https://github.com/DangerMouseUK/civic89/actions/runs/37428051380) and [merged-main CI](https://github.com/DangerMouseUK/civic89/actions/runs/37429117027) passed both configurations for `4f12745` and `d2b278a`.

- [x] **M1-01:** Complete command inventory, with all former routes and retirement decisions recorded in [LEGACY_EVAL_INVENTORY.md](project/reference/LEGACY_EVAL_INVENTORY.md).
- [x] **M1-02:** All audio calls are typed; dashboard messages, navigation requests, generation, earthquake and scenario outcomes use application-owned presentation interfaces. Null audio/navigation/visual behavior is preserved; dormant notification audio is not activated.
- [x] **M1-03:** Native `ScenarioController` validates/loads/starts all eight packaged scenarios. F6 opens a native selector; `civic89.exe --scenario 1..8` selects at startup. Successful starts clear stale earthquake state and old save selection. Missing/invalid inputs preserve the active session and emit no started event. Won/lost outcomes retain thresholds and reach the dashboard.
- [x] **M1-04:** Remove the `Eval` declaration/definition and all call expressions, string sound wrappers, unused picture wrapper and inactive options command. CTest prevents reintroduction.
- [x] Characterize all eight inherited loads, metadata, disaster/scoring deadlines and the missing-file failure before implementation (`1f49500`), in both configurations.
- [x] Pin all eight fixture byte digests, check decoding and actual initialized state, then run 64 native simulation frames per scenario. Cover invalid IDs, missing/empty/truncated/oversized/old-format/corrupt files, session preservation, restart and initialized-event ordering.
- [x] Verify message/focus order, message expiry, dormant audio, generation-before-map ordering, all scoring boundaries and actual won/lost dispatch.
- [x] Pass **7/7 CTests in Debug and Release**, including actual application lifecycle under SDL dummy video/software rendering. Retained Visual Studio Release also builds.
- [x] Source comparisons reproduce complete sprite/tool/generation files by reversing only typed replacements. Simulation, disaster/RNG, existing city load/save functions, 47-source list, dependencies and inherited project files are unchanged (`out/audit/m1-source-parity.log`).
- [x] Publish the entire milestone branch for one PR at the user's request.
- [ ] Verify hosted Debug/Release CI.

[ADR 0002](project/decisions/0002_M1_COMPLETION.md) records deliberate startup repairs: validate before committing data, reset arrays before restoring histories, catalog difficulty before initialization, and emit the formerly absent win notification. Script speed modifiers are retired without guessing a multiplier; typed `ShipHorn` and `Monster` identities retain intent for the future backend. Existing sound setting/serialization behavior is preserved.

Exact commands and paths are in [BUILDING.md](project/BUILDING.md). [Milestone evidence](tests/baseline/M1_2026-10-06.md) records fixture checks, tests, warnings, source comparisons and UI limits. The interactive GUI smoke could not be completed: computer-use capture returned `IGraphicsCaptureItemInterop.CreateForMonitor ... 0x80070057`, and activation returned `GetCursorPos ... 0x80070005`. Only the task-created smoke process was closed. The selector has not been visually verified on this desktop; automated native lifecycle tests do not establish interactive layout/DPI behavior.

## Verified environment and dependency baseline

- Installed Visual Studio Enterprise 2026: **18.10.3**. CMake selects the installed VS 2026 BuildTools instance, MSVC **19.51.36260.0**, toolset directory **14.51.36231**.
- Bundled CMake: **4.3.1-msvc1**. Presets require CMake 4.2+ for the VS 2026 generator; no developer shell/Ninja needed.
- vcpkg executable version: `2026-09-26-51bf87ca6e9bf3e622d84ff323bd202ab1ca0c0b`.
- Clean external checkout/manifest registry baseline: **19780d9cdf84d0944cf9a318666703b89ab6629c**. The executable version and registry commit are different identifiers.
- Original five direct dependencies retained: SDL3 3.4.18, SDL3_image 3.4.4#1, SDL3_ttf 3.2.2#1, nativefiledialog-extended 1.4.1, nlohmann-json 3.12.0#2. No production dependencies added.

Exact commands/target mapping: [project/BUILDING.md](project/BUILDING.md). Local evidence: [tests/baseline/BOOTSTRAP_2026-10-05.md](tests/baseline/BOOTSTRAP_2026-10-05.md). Asset provenance and replacement decision: [project/reference/RUNTIME_ASSETS.md](project/reference/RUNTIME_ASSETS.md), `assets/runtime-assets.json`, `assets/ASSET-LICENSES.yml`.

Bootstrap and earlier typed increments are merged. This branch completes M1 without source moves, new production translation units or new dependencies. Hosted milestone CI is pending PR verification. No upstream push or history/tag rewrite occurred.

## Inherited limitations and release gates

- Missing fonts were an inherited packaging defect, not a Civic 89 regression. Startup now works with approved fonts/substitute. Original developer font versions and pixel-identical text metrics cannot be established.
- **Historical `.cty` compatibility is unproven.** The inherited writer uses native 32-bit arrays and produced a **51,360-byte** save; some supplied cities are **27,120 bytes**. The existing city reader in `FileIo.cpp` does not validate short reads. A current-version round trip does not establish historical save compatibility; M1 validates the eight packaged Windows scenario files separately. Investigate separately with tests; no format change is made here.
- `icons/LICENSE.txt` supplies OpenSVG attribution without identifying original icon sets/licences. Public binary redistribution remains blocked on that audit. Other retained graphics/fixtures preserve inherited project-level GPL/additional-terms provenance, not a completed per-asset rights review.
- Audio playback, camera auto-goto and earthquake shake/timing remain silent/no-op presentation adapters for later milestones. M1 removes all command plumbing and adds working native scenario startup; it does not add an audio/renderer backend. Dormant notification audio and inherited mute/initialization semantics remain unchanged. Simulation RNG seeding, deterministic digests, save safety beyond scenario startup, and resource lifetime are later milestones.
- Historical bootstrap `/W4` capture exposed **50 inherited warnings per configuration**: 40 C4100, one C4189, two C4389, one C4456, six C4459. The inherited `/W3` comparison build reported none. No warning-as-error policy introduced.
- A final inherited MSBuild rerun passed after the manifest change with four CS1668 environment warnings: two missing `LIB` search directories reported twice by Roslyn inline tasks (Enterprise ATL/MFC and `lib\um\x64`). These are workstation/global-integration warnings, not C++ errors; no unrelated environment repair was made.
- Original UI/window/resource branding is retained for comparison. `civic89.exe` is the new target name; public product branding/packaging remains later work.
- Retained `.sln`/`.vcxproj` remain unedited and build after the implemented typed routes. Catch2, engine extraction, deterministic simulation digests, ASan and exhaustive UI/asset compliance testing remain later milestones.
