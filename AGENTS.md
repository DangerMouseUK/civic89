# AGENTS.md - Civic 89

This repository is **Civic 89**, a native Windows modernisation of the GPL Micropolis / original SimCity code lineage.

## Delivery scope

Complete one whole milestone from `project/engineering/08_ROADMAP_AND_IMPLEMENTATION_BACKLOG.md` per engineering branch and PR, including its acceptance checks and compatibility notes. Do not stop after one or two backlog items unless the user explicitly requests a narrower scope. Work as a single agent unless the user explicitly approves delegation for the current task.

## Product scope

The user's 6 October 2026 decision is faithful modernisation of the original gameplay and mechanics. Preserve one simulation contract; modernise Windows presentation, controls and usability. Larger maps, expanded finance/population limits, new buildings/tools/scenarios, changed traffic/utilities/balance, modding and achievements/challenges are outside scope. Day/night and seasonal additions are also excluded from the graphics milestone.

The only optional enhancement is a graphics preference. It must not change city state, ruleset identity, RNG consumption, simulation/animation timing or save bytes. Store it in application settings and allow switching during play. Keep existing M7 `.c89` saves supported and the `.cty` writer unchanged.

M0–M9 software work is merged. M8 supplies the single-gameplay interface; M9 supplies optional graphics. Physical desktop acceptance is still pending. Follow the roadmap and [ADR 0009](project/decisions/0009_FAITHFUL_MODERNISATION_AND_GRAPHICS.md); preserve dated milestone evidence as historical records. No next numbered milestone is agreed.

## Workspace contract

- Repository root: `C:\Dev\Projects\civic89`
- External vcpkg checkout: `C:\Dev\vcpkg`
- Product/display name: `Civic 89`
- Technical identifier/repository: `civic89`
- CMake project: `Civic89`
- Primary executable target: `civic89` -> `civic89.exe`
- Engine target: `civic89_engine`
- Tests target: `civic89_tests`
- Primary platform: Windows 11 x64
- Toolchain: Visual Studio Enterprise 2026 / current MSVC / C++20 / CMake / vcpkg / SDL3

`C:\Dev\vcpkg` is external tooling. Never copy it into this repository or add it to Git.

## Read before architectural work

Read all files in `project/engineering/`, especially:

1. `00_README_INDEX.md`
2. `01_PROJECT_CHARTER_AND_DECISIONS.md`
3. `02_UPSTREAM_AUDIT_AND_BASELINE.md`
4. `03_DEVELOPMENT_ENVIRONMENT_WINDOWS.md`
5. `04_REPOSITORY_BUILD_AND_CI.md`
6. `05_ARCHITECTURE_AND_REFACTORING.md`
7. `06_TESTING_QUALITY_AND_PARITY.md`
8. `07_LICENSING_ATTRIBUTION_AND_ASSETS.md`
9. `08_ROADMAP_AND_IMPLEMENTATION_BACKLOG.md`
10. `09_CODING_AGENT_HANDOFF.md`
11. `10_STARTER_CONFIGURATION_TEMPLATES.md`
12. `11_SOURCES_AND_REFERENCES.md`
13. `12_LOCAL_REPOSITORY_BOOTSTRAP.md`

Also read `PROJECT_STATUS.md` for the current task/state.

## Non-negotiable rules

- Preserve the complete Micropolis-SDLPP Git history, exact baseline `9c4e85a0decd57ba6f76d9e1ec82461940ecc3ad`, fixed tag `upstream-sdlpp-baseline`, and the `upstream-sdlpp` remote.
- Preserve inherited GPL/licence/additional-terms files and copyright notices.
- Do not use SimCity or Micropolis as the Civic 89 product brand.
- Do not rewrite the simulation from scratch.
- Do not introduce Unity, Unreal, Electron or a browser runtime.
- Do not mass-format inherited code during infrastructure changes.
- Do not mix source-tree moves with behaviour changes.
- Preserve original gameplay and simulation outcomes. Report any fidelity discrepancy before making an outcome-changing fix; tests alone do not authorise changed mechanics.
- Do not change the legacy `.cty` format casually.
- Do not add unlicensed retail-game assets, fonts, sounds or icons.
- Do not replace the inherited `Eval()` bridge with another string-command pseudo-API.
- Keep the inherited `.sln`/`.vcxproj` working until the CMake baseline is demonstrably equivalent.

## Provenance checkpoint

The inherited baseline was captured locally on 5 October 2026:

- SHA: `9c4e85a0decd57ba6f76d9e1ec82461940ecc3ad`;
- tag: `upstream-sdlpp-baseline`;
- upstream remote: `upstream-sdlpp` -> `https://github.com/ldicker83/Micropolis-SDLPP.git`;
- working tree: clean at capture.

`PROJECT_STATUS.md` is authoritative for current delivery state. Bootstrap is complete;
`feature/cmake-bootstrap` is historical, not a new assignment. Keep the retained
Visual Studio comparison path working alongside the CMake build.

## Beta release policy

The owner requested public `0.9.0-beta.1` after reviewing the asset investigation,
confirmed brand review fully approved, and deferred code signing and physical
desktop acceptance for this beta. Follow [ADR 0010](project/decisions/0010_PUBLIC_BETA.md),
[release instructions](project/RELEASING.md) and `packaging/release-gates.json`.
Do not reinstate superseded draft-only/publication blockers or ask again for
already-authorised beta publication. Keep the missing OpenSVG pack notices and
two source-only branding origins disclosed; do not mark their audit complete,
invent licences, replace artwork or contact upstream without user direction.

Require clean versioned builds, green CI, both architectures, exact corresponding
source and verified checksums. Publish the approved version as an unsigned
prerelease excluded from Latest. Keep stable signed-release gates separate.
Never move an existing release tag or replace published assets. A later version
needs a new recorded release decision. For documentation/release or bug-fix work,
use the user's requested scope rather than inventing another milestone.

After changes, report targeted checks, unresolved limitations and any checks
that could not run. Physical acceptance requires real evidence.
