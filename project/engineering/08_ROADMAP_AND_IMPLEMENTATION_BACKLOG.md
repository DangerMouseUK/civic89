# Roadmap and Detailed Implementation Backlog

**Status:** Active engineering baseline  
**Prepared:** 5 October 2026  
**Repository baseline captured:** 5 October 2026  
**Scope:** **Civic 89** - native Windows modernisation of the open-source Micropolis / original SimCity code lineage

**Current checkpoint (7 October 2026):** M0–M9 software work is merged, including
M8 [PR #13](https://github.com/DangerMouseUK/civic89/pull/13) and M9
[PR #14](https://github.com/DangerMouseUK/civic89/pull/14). Physical acceptance is
pending. No further numbered milestone is agreed. Dated delivery records below
retain their original versions/results; current beta policy is
[ADR 0010](../decisions/0010_PUBLIC_BETA.md) and [project status](../../PROJECT_STATUS.md).

## 1. Roadmap philosophy

The milestones are ordered to reduce risk. Do not begin the visible remaster work until the project can prove that the inherited simulation still builds and behaves consistently.

As requested by the user on 6 October 2026, deliver complete milestones on one engineering branch and PR. Separate implementation/test commits within that branch are encouraged; do not publish one- or two-item increments unless explicitly requested.

**Product scope update (6 October 2026):** preserve the original gameplay and mechanics throughout. Modernise Windows presentation and usability; the only optional enhancement is a graphics toggle. Larger maps, wider gameplay limits, new buildings/tools/scenarios, changed traffic/utilities/balance, mods and achievements/challenges are excluded. Day/night and seasonal additions are outside the graphics milestone. [ADR 0009](../decisions/0009_FAITHFUL_MODERNISATION_AND_GRAPHICS.md) supersedes the earlier expansion direction without undoing merged M7.

Effort labels are relative only:

- **S** - contained change;
- **M** - multi-file change with tests;
- **L** - architectural milestone;
- **XL** - broad feature requiring substantial implementation and review within its milestone.

## 2. Milestone 0 - Baseline Bootstrap

**Goal:** reproducible native Windows build with no intentional gameplay change.

### M0-01 Preserve upstream history - S

- clone SDLPP directly to `C:\Dev\Projects\civic89`;
- rename the inherited remote to `upstream-sdlpp`;
- add the empty Civic 89 GitHub repository as new `origin`;
- retain `upstream-sdlpp` remote;
- tag audited baseline SHA;
- add `project/reference/UPSTREAMS.md`.

**Acceptance:** repository history contains upstream commits and baseline tag resolves to the audited SHA.

### M0-02 Add CMake build - L

- reproduce existing x64 Debug/Release builds;
- retain source layout initially;
- copy/install runtime assets correctly;
- ensure no manual include/lib paths are required.

**Acceptance:** clean checkout builds from command line and Visual Studio folder-open flow.

### M0-03 Reproducible vcpkg manifest - M

- migrate existing dependencies;
- add planned test/logging dependencies;
- pin baseline;
- document update process.

**Acceptance:** a second machine restores the same dependency graph.

### M0-04 CI - M

- Windows VS2026 x64 Debug + Release;
- test execution;
- build artifact for Release.

**Acceptance:** PR cannot merge when build/tests fail.

### M0-05 Baseline evidence - M

- smoke checklist;
- baseline city/save fixtures;
- screenshots;
- known-issues list.

## 3. Milestone 1 - Remove Legacy Command Plumbing

**Goal:** eliminate stubbed Tcl/Tk-style `Eval()` dependencies.

### M1-01 Inventory every `Eval()` call - S

Produce a table of command name, call site, intended effect and replacement service.

### M1-02 Typed presentation events - L

Introduce minimal interfaces for UI notifications/navigation/disaster presentation.

### M1-03 Scenario lifecycle - M

Replace string command startup with typed scenario selection/load/start.

**Acceptance:** all eight scenarios can be selected and started through native code.

### M1-04 Delete obsolete `Eval()` bridge - M

Only after tests prove no remaining functional dependency.

**Acceptance:** repository search contains no runtime `Eval("UI...`) path.

**Completion record (6 October 2026):** M1-01 through M1-04 are implemented on `codex/m1-remove-legacy-plumbing`. All eight scenarios load and advance through native code; F6 and startup selection use the typed controller. No runtime `Eval` or string audio API remains. Debug/Release pass 7/7 checks and the inherited Release build passes. Merged in PR #6 (`de424f3`); hosted Debug/Release CI passed; [ADR 0002](../decisions/0002_M1_COMPLETION.md) and [milestone evidence](../../tests/baseline/M1_2026-10-06.md) record compatibility decisions and interactive UI limits.

## 4. Milestone 2 - Engine Boundary and Testability

**Goal:** simulation becomes a headless library target.

### M2-01 Identify SDL/UI contamination - M

Map direct dependencies from simulation files to presentation/platform code.

### M2-02 Create `civic89_engine` target - XL

Move incrementally; do not redesign algorithms during extraction.

### M2-03 Headless simulation runner - M

CLI/tool target capable of loading a city, running N ticks and outputting summary/digest.

### M2-04 Deterministic test mode - L

Provide explicit test seeding/control without changing normal play semantics.

### M2-05 Parity fixtures - L

Golden state digests for known city/tick counts.

**Completion record (6 October 2026):** M2-01 through M2-05 are implemented on `codex/m2-engine-boundary`. The native app links a platform-free engine; a standalone headless runner, explicit deterministic control and 25 pre-extraction golden cases are verified. Application Debug/Release pass 39/39 checks each; headless and ASan pass 32/32 each; retained Visual Studio Release builds. [ADR 0003](../decisions/0003_M2_ENGINE_BOUNDARY.md), [dependency audit](../reference/ENGINE_DEPENDENCIES.md) and [evidence](../../tests/baseline/M2_2026-10-06.md) record the complete milestone and limits. Merged in PR #7 (`090633d`); hosted Debug/Release/headless ASan CI passed on `d0986f5`.

## 5. Milestone 3 - Functional Completion

**Goal:** complete missing native functionality before cosmetic redesign.

### M3-01 AudioManager with SDL3_mixer - L

- map typed sound IDs;
- load/release resources;
- volume controls;
- device failure fallback;
- no string command bridge.

### M3-02 Resource ownership cleanup - L

- RAII wrappers for SDL resources where helpful;
- deterministic teardown order;
- stress test repeated new/load/quit/session cycles.

### M3-03 Save reliability - M

- atomic save flow;
- clearer parse/write errors;
- autosave/recovery foundation.

### M3-04 Error/logging layer - M

Replace silent/console-only critical failures with structured reporting.

**Completion record (6 October 2026):** M3-01 through M3-04 are implemented together on `codex/m3-functional-completion`: typed SDL3_mixer audio with original effects and persisted native volume controls; RAII/exception-safe resource teardown; same-directory atomic city publication with validated restoration and autosave/recovery; typed diagnostics and native error reporting. Application Debug/Release/ASan pass 41/41 tests; headless Debug/ASan pass 33/33; all 25 M2 golden cases and the retained Visual Studio Release build pass. [ADR 0004](../decisions/0004_M3_FUNCTIONAL_COMPLETION.md) and [evidence](../../tests/baseline/M3_2026-10-06.md) record acceptance and compatibility limits, including ordinary-city recovery and unsupported historical formats. Merged in PR #8 (`555b7ba`); hosted Debug/Release/application ASan 41/41 and headless ASan 33/33 passed on `39a9399`.

## 6. Milestone 4 - Modern Window, Camera and Rendering

**Goal:** the game begins to feel like a 2026 Windows application without changing Classic simulation rules.

### M4-01 Display modes - M

- windowed;
- maximised;
- borderless/fullscreen;
- persisted setting.

### M4-02 DPI-aware layout - L

Test 100-200% scaling and mixed-DPI monitors.

### M4-03 Camera2D - L

- smooth pan;
- mouse-wheel zoom;
- zoom-to-cursor;
- map bounds;
- keyboard movement;
- pixel-perfect option.

### M4-04 VSync/frame pacing - M

Separate render cadence from simulation tick cadence.

### M4-05 Renderer cleanup - L

Centralise tile/sprite/overlay rendering and remove map drawing responsibilities from the application loop.

**Completion record (6 October 2026):** M4-01 through M4-05 are implemented together on `codex/m4-window-camera-rendering`: persisted windowed/maximized/borderless modes; per-window DPI/input/layout; Camera2D with pan, cursor zoom, bounds, keyboard and pixel scaling; main-thread simulation/animation scheduling and VSync/frame limiting; centralized map/sprite/tool/quake rendering. Application Debug/Release/ASan pass 42/42 checks; headless Debug/ASan pass 34/34; all 25 M2 goldens and retained Visual Studio Release pass. Native Windows mode APIs and the automated 100â€“200%/mixed-scale matrix pass. Physical mixed-DPI/visible desktop/hardware refresh remain release checks. [ADR 0005](../decisions/0005_M4_WINDOW_CAMERA_RENDERING.md) and [evidence](../../tests/baseline/M4_2026-10-06.md) record acceptance and compatibility decisions. Hosted CI passed all four jobs on [PR #9](https://github.com/DangerMouseUK/civic89/pull/9), merged as `fc0c4f6`.

## 7. Milestone 5 - Modern UI/UX

**Goal:** cohesive single-window remaster experience.

### M5-01 In-window minimap - L

Reuse existing overlay data; detach option can be later.

### M5-02 Responsive dashboard/status bar - L

Funds, date, population, RCI, message state and active tool.

### M5-03 Tool palette redesign - L

Icons, tooltips, hotkeys, disabled/affordability states and scalable layout.

### M5-04 Budget/evaluation/graphs redesign - XL

Preserve data but modernise presentation and layout.

### M5-05 Full-map data overlays - L

Traffic, crime, land value, pollution, population, power/protection data with adjustable opacity.

### M5-06 Settings/accessibility - L

- UI scale;
- font/readability options;
- configurable hotkeys;
- volume categories;
- camera controls;
- colour/overlay accessibility considerations.

**Completion record (6 October 2026):** M5-01 through M5-06 are delivered together on `codex/m5-modern-ui`: in-window minimap, responsive dashboard, tool palette/affordability/hotkeys, budget/evaluation/history/query panels, shared full-map data layers and opacity, and persisted readability/key/camera/audio preferences. Native in-window scenario/new-city panels complete the workflow. Application Debug/Release/ASan pass 43/43; headless Debug/ASan pass 35/35; all 25 Classic goldens and retained Visual Studio Release pass. Compact/wide rendered captures were visually reviewed; native software/Direct3D 11 checks pass. Physical mixed-DPI and assistive-technology integration remain release validation. [ADR 0006](../decisions/0006_M5_MODERN_INTERFACE.md) and [evidence](../../tests/baseline/M5_2026-10-06.md) record compatibility and acceptance. Hosted CI is tracked on the delivered PR.

## 8. Milestone 6 - Release Engineering

### M6-01 Portable release ZIP - M

Self-contained x64 artifact with licences/notices.

### M6-02 Installer - M

Inno Setup or WiX; uninstall and user-data preservation.

### M6-03 Code-signing integration - M

Separate secure signing stage.

### M6-04 Crash-safe update strategy - L

Only after stable versioning and release hosting exist.

### M6-05 Windows ARM64 - L

Compile/test after the engine/platform boundary is clean.

**Completion record (6 October 2026):** M6-01 through M6-05 and the root README are delivered together on `codex/m6-release-engineering` in [PR #11](https://github.com/DangerMouseUK/civic89/pull/11). Clean versioned portable ZIPs, per-user installers, matching source/checksums, complete notices, separate protected signing integration and native ARM64 Release CI are implemented. M6-04 supplies verified offline whole-version staging/atomic selection/rollback; automatic network updates remain conditional on stable release hosting. All five hosted jobs pass on `3bbea9a`: application x64/ARM64 configurations 43/43 and headless ASan 35/35, with all Classic goldens unchanged. Both architectures pass ZIP launch, DLL closure, update/failure/rollback and install/upgrade/uninstall retention. Source-export build 35/35 and retained Visual Studio Release pass locally. [ADR 0007](../decisions/0007_M6_RELEASE_ENGINEERING.md), [evidence](../../tests/baseline/M6_2026-10-06.md) and [release instructions](../RELEASING.md) record checks and compatibility limits. Trusted signer provisioning and public asset/brand/physical acceptance remain external gates; no public release or tag is created.

## 9. Milestone 7 - Enhanced Mode foundation

Do not start until Classic Mode has a supported contract.

**Scope decision (6 October 2026):** the user selected the complete foundation,
not implementation of every candidate gameplay feature. Establish the supported
development contract first; stable public release remains separately gated.

### M7-01 Classic development contract

Specify supported mechanics, platforms, file layout and historical/replay limits.

### M7-02 Versioned rulesets

Typed Classic v1/Enhanced v1 identities and capability definitions; reject unknown
versions before mutation. Enhanced v1 initially shares Classic mechanics.

### M7-03 Explicit mode selection

New-city mode selection, active identity in the UI and headless runner, and Classic
scenario policy. Existing sessions must not change modes through a preference.

### M7-04 Save/import/export/recovery boundary

Preserve `.cty`; add a bounded, checksummed, versioned Enhanced container, explicit
Classic import/export and separate recovery slots. Retain atomic publication.

### M7-05 Compatibility acceptance

Unchanged Classic goldens and generated-city baseline; both mode paths; save round
trips; unknown/corrupt/truncated input and publication failure isolation; UI acceptance;
Debug/Release/ASan/native ARM64, delivery checks and retained Visual Studio parity.

**Completion record (6 October 2026):** M7-01 through M7-05 are delivered together on
`codex/m7-enhanced-foundation` in [PR #12](https://github.com/DangerMouseUK/civic89/pull/12).
Classic v1 has a supported development contract; exact versioned identities, native/CLI
selection, bounded/checksummed Enhanced saves, explicit Classic import/export and
separate mode-aware recovery are implemented. All five hosted jobs pass at `105ddc9`
([run](https://github.com/DangerMouseUK/civic89/actions/runs/37493323959)): application
Debug/Release/ASan and native ARM64 53/53; headless ASan 43/43. All 25 Classic goldens
and fresh-process generated-city references remain unchanged. Both architectures pass
ZIP launch, static DLL closure, update/rollback/failure and installer retention for
both save formats. Local source export passes 43/43; retained Visual Studio Release
passes all 65 production entries. M6-to-M7 portable update/rollback also preserves
both save formats. [ADR 0008](../decisions/0008_M7_ENHANCED_FOUNDATION.md),
[Classic contract](../CLASSIC_COMPATIBILITY.md), [Enhanced format](../ENHANCED_CITY_FORMAT.md)
and [evidence](../../tests/baseline/M7_2026-10-06.md) state compatibility and acceptance.
Enhanced v1 still uses Classic mechanics/map size; public-release gates remain unresolved.
Final compatibility/evidence commit checks are tracked on the PR.

**Merged record:** PR #12 is merged as `eb73f64230e35c2f0aaf8134c4cd5e6100ed155d`.
The final branch head `6d73e53` passed all five jobs in
[run 37494533588](https://github.com/DangerMouseUK/civic89/actions/runs/37494533588).
The earlier Enhanced gameplay candidate backlog is withdrawn by the scope update
above. M7 remains complete; its interface adjustment is M8-02 below.

## 10. Milestone 8 - Faithfulness and Windows polish

**Status:** software merged in PR #13; physical desktop acceptance pending and deferred for the public testing beta (ADR 0010).

**Goal:** finish fidelity and desktop acceptance work around the original game, and
align the merged M7 interface with the single gameplay contract.

### M8-01 Faithfulness audit and regression coverage - L

- Check inherited map dimensions, tools/footprints/costs, construction behaviour,
  budget/tax/funding, RCI/economy, traffic/utilities, disasters and all eight scenarios.
- Preserve the 25 M2 goldens and four generated-city parity cases. Add targeted
  regressions for uncovered behaviour using provenance-backed references.
- Distinguish parity with the inherited SDLPP baseline from evidence about original
  retail behaviour; record disagreements before proposing outcome-changing fixes.

**Acceptance:** a traceable audit records what was checked, sources, gaps and any
discrepancies. Existing parity references stay unchanged. No new or rebalanced mechanics;
an outcome-changing fidelity fix requires explicit user direction and compatibility evidence.

### M8-02 Adjust the merged M7 interface - L

- Present one faithful new-city/gameplay path; remove the suggestion that choosing
  Enhanced changes the simulation. Update new-city, status, recovery and help text.
- Keep existing `.cty` and `.c89` opening, saving, explicit export and recovery
  working. Retain known M7 identities as save compatibility data where required;
  preserve existing CLI compatibility or document compatible aliases clearly.
- Keep graphics out of city creation, conversion and save identity. M9 will add the
  optional graphics preference in Settings; M8 must not advertise an implemented toggle.
- Retain M7's merged implementation/evidence as history and update current product docs.

**Acceptance:** new-city and all eight scenario workflows expose the same mechanics.
Existing M7 `.c89` files and recovery slots still load; save/export/recovery errors
preserve the active city and destination. No forced conversion, retagging or deletion
of existing files; unknown/corrupt inputs still fail before mutation. UI and CLI
acceptance verifies the revised wording and supported workflows.

### M8-03 Historical city compatibility investigation - M

- Investigate the known unsupported 27,120-byte `.cty` variant using verified format
  references and legally usable fixtures; document which historical variants are tested.
- Add an isolated import decoder only if the format and faithful interpretation are
  established. Preserve the existing 51,360-byte writer and validation/publication path.
- Keep ordinary-city snapshot, RNG, sprite and scenario-progress limitations explicit.

**Acceptance:** evidence either establishes a tested import path for a specific variant,
including malformed-input isolation, or records the unresolved limitation without a
compatibility claim. Current `.cty`/`.c89` round trips and failure preservation pass;
no speculative decoder or casual legacy format change.

### M8-04 Windows display, input and accessibility polish - L

- Validate 1080p/1440p/4K and supported ultrawide layouts, 100–200% scaling and
  physical mixed-DPI transitions; fix clipping, hit targets, camera and readability issues.
- Exercise windowed/maximised/borderless modes, resizing, Alt+Tab, minimise/restore,
  keyboard focus/navigation and configurable keys on the visible desktop.
- Check existing high-contrast/larger-text controls and assistive-technology limits.

**Acceptance:** an evidence matrix records actual displays/scales/backends and visible
results. Panels remain readable and operable; focus and input recover correctly.
Required physical checks need actual hardware evidence; automated dummy/software tests
do not substitute for it. Unavailable configurations stay explicitly pending.

### M8-05 Desktop audio, dialogs and lifecycle - M

- Verify actual device audibility, category volumes/mute, device failure fallback,
  native open/save cancellation/errors and Unicode paths.
- Exercise repeated new/load/scenario/quit sessions, display transitions and recovery
  prompts; fix application lifecycle issues without changing simulation timing or RNG.

**Acceptance:** visible desktop/device checks and targeted regressions cover these
workflows. Dialog cancellation/failure preserves the session, audio failures remain
non-fatal and teardown is clean. Record hardware and any unresolved acceptance limits.

### M8-06 Complete milestone validation and documentation - M

- Update README, compatibility contract, status, controls/help and milestone evidence
  to describe the delivered single gameplay contract and retained save support.
- Run Debug/Release, application/headless ASan, native ARM64, retained Visual Studio
  and relevant portable/installer/update/rollback acceptance, with parity unchanged.
- Keep asset/brand clearance, trusted signer provisioning and remaining physical
  release acceptance visible; do not declare a public release from automated checks.

**Acceptance:** one branch/PR delivers all M8 items with compatibility notes, CI and
manual evidence. Outstanding required desktop checks prevent claiming complete acceptance;
external rights/brand/signing gates remain separately recorded until cleared.

**M8 implementation checkpoint (6 October 2026):** `codex/m8-faithfulness-windows-polish`
delivers all six items' software scope: [audit](../FAITHFULNESS_AUDIT.md), source/data
contract and mechanics regressions; original-gameplay interface with M7 saves/CLI;
documented historical format mismatch; Windows chord/focus/native-owner/save-location
fixes; expanded software/Direct3D 11 display and lifecycle matrix; version 0.8.0-dev
and current documentation. Application Debug/Release/ASan pass 55/55 and headless
Debug/ASan 45/45 locally; retained Visual Studio Release passes. All five hosted
jobs pass on `e6039b1` in [run 37503989275](https://github.com/DangerMouseUK/civic89/actions/runs/37503989275),
including native ARM64 55/55 and both complete Release deliveries. Default-device
opening succeeds. Required physical visibility/input/dialog/audibility/mixed-DPI
checks remain pending because desktop capture/input access is unavailable.
[M8 evidence/checklist](../../tests/baseline/M8_2026-10-06.md) records results and
delivery/CI progress. This is an implementation checkpoint, not full milestone
acceptance. The PR has since merged; ADR 0010 permits the public testing beta
while these physical checks remain pending.

## 11. Milestone 9 - Optional enhanced graphics

**Status:** direction approved and software merged in PR #14 on 7 October 2026,
after M8 PR #13. Physical acceptance remains pending and is deferred for the
public testing beta. The implementation checkpoint below retains dated evidence.

**Goal:** offer improved graphics for the same game, selectable during play in Settings.

### M9-01 Faithful visual specification and asset rights - L

- Agree the art style and define a one-to-one mapping for existing tiles, buildings,
  sprites, overlays and animation frames. Preserve identities, footprints and states.
- Create/use appropriately licensed assets, with attribution and per-asset provenance.
- Retain the Classic graphics option. No additional buildings/tools/scenarios,
  visual day/night or seasons, or mechanics hidden behind the graphics toggle.

**Acceptance:** the approved specification covers the existing visual catalogue;
asset rights and mappings are complete before the new art is distributed.

### M9-02 Settings-only graphics selection - M

- Add Classic/Enhanced graphics to application preferences, independent of the city
  and its save format, switchable during play without reload or conversion.
- Make renderer resource switching safe and recover to Classic graphics if enhanced
  resources cannot be loaded; report the failure clearly.

**Acceptance:** either graphics option works with the same new, scenario and loaded
`.cty`/`.c89` cities. Switching preserves city/ruleset state, RNG consumption, simulation
and animation cadence, save destination and payload bytes. Preferences persist outside
city files; failure leaves the session usable.

### M9-03 Complete graphics integration - XL

- Integrate the approved replacement tile/sprite resources and existing effects,
  overlays, minimap and tool previews across camera zoom and display scaling.
- Preserve all existing visual states, selection/query information and animation
  sequences/timing; ensure the game remains legible in both graphics options.

**Acceptance:** catalogue coverage and reviewed captures show no missing/misidentified
tiles or sprites, changed footprints, misleading overlays or altered animation timing.

### M9-04 Rendering performance and desktop acceptance - L

- Measure resource use and frame pacing at the supported display sizes/backends;
  verify switching, resizing, fullscreen transitions and renderer recovery.
- Check 4K, mixed DPI and accessibility/readability with the complete graphics set.

**Acceptance:** a recorded before/after performance and visible-desktop matrix meets
agreed rendering targets without affecting simulation speed. Resource failure/recovery
and repeated toggling do not leak resources or damage the city.

### M9-05 Parity, saves and delivery - M

- Compare the same deterministic city/inputs under both graphics options, including
  switching mid-session; check state digests, RNG, tick/animation counts and save output.
- Retain all existing baseline goldens and validate both save formats, preferences,
  native build/ASan/ARM64 and complete package/update/installer asset delivery.
- Document the graphics preference, asset attribution and complete milestone evidence.

**Acceptance:** one branch/PR completes the graphics milestone with unchanged gameplay,
save compatibility and original graphics available. Stable release requires the
remaining rights/signing/physical gates; ADR 0010 governs the approved testing beta. If graphics are declined, redefine M9
only through a separate user decision; do not substitute gameplay expansion.

**M9 implementation checkpoint (7 October 2026):** the user chose sharper pixel art
closely following the original. The [approved specification](../GRAPHICS_SPECIFICATION.md)
implements palette-preserving 2x edge refinement, rather than newly illustrated
artwork, with complete [input mapping/provenance](../../assets/graphics-catalogue.json).
All 960 tiles/minimap entries, 61 sprite frames and ten textured previews are covered.
Classic is the default; `graphics.cfg` is independent of both city identities and
M8's `ui.cfg`. Complete resource preparation precedes both ownership swaps; load/reset
failures can recover Classic and failed live preparation retains the existing session.
Fresh-process comparisons cover phase digests, animation/RNG and both save formats;
catalogue captures, all scenarios/new city, DPI/zoom/backends, repeated toggling and
failure/reset acceptance are included. [M9 evidence](../../tests/baseline/M9_2026-10-07.md)
records measurements and build/package checks. This is a software delivery checkpoint;
required physical readability/mixed-DPI/clean-machine checks remain pending.
M8/M9 are now merged; the owner approved brand review and the unsigned public beta
after the asset investigation. ADR 0010 records the specific remaining follow-ups.

### Release readiness after M8/M9

Publish the approved `0.9.0-beta.2` prerelease only from green, clean, verified
x64/ARM64 deliveries with exact source and combined checksums. Brand review is
owner-approved; signing/physical checks are deferred for beta. Disclose the
[asset follow-ups](../ASSET_LICENSE_AUDIT.md). The owner's publication decision is
[ADR 0011](../decisions/0011_BETA_2_SETTINGS_FIXES.md), continuing ADR 0010's beta
terms, not an inference from passing tests.
Complete provenance/physical acceptance and provision signing before stable
distribution; keep [release policy](../RELEASING.md) and status accurate.

## 12. Historical first implementation sequence for a coding agent

The safest first sequence is:

1. create new origin while preserving history;
2. add docs/UPSTREAMS and baseline tag;
3. convert current Visual Studio source list into CMake without moving files;
4. create vcpkg manifest + pinned baseline;
5. build x64 Debug/Release;
6. add Catch2 and a trivial executable/engine smoke test;
7. add GitHub Actions;
8. record baseline known issues;
9. open a separate PR for `Eval()` inventory and typed replacement design.

Do not combine steps 3 and 9 in one PR.
