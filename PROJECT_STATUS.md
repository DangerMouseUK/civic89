# Civic 89 — Project status

**Updated:** 7 October 2026. **Current delivery:** public testing beta `0.9.0-beta.1`.
M0–M9 software work is merged; physical desktop acceptance remains pending.

## Repository and verification

- M8 [PR #13](https://github.com/DangerMouseUK/civic89/pull/13) and M9
  [PR #14](https://github.com/DangerMouseUK/civic89/pull/14) are merged.
  The post-M9 main baseline is `e4986334bb5cda661d95b1018605dedb401ad7b4`.
  Merged M8/M9 feature branches were removed after ancestry checks and fetch/prune.
- All five jobs pass on merged main in [run 37616383766](https://github.com/DangerMouseUK/civic89/actions/runs/37616383766):
  x64 Debug/Release/application ASan and native ARM64 Release **56/56** each;
  headless ASan **45/45**. Both Release architectures pass ZIP/source/installer,
  static DLL closure, update/failure/rollback and install/uninstall acceptance.
- The beta branch `codex/beta-release` reconciles repository documentation,
  records the asset investigation/owner decisions, versions the build and adds
  verified beta staging. It changes no simulation, artwork, dependencies or saves.
  The beta's exact build commit and delivery checksums are recorded in `release.json`
  on the [release page](https://github.com/DangerMouseUK/civic89/releases/tag/v0.9.0-beta.1).
  The associated PR/checks record validation of the beta inputs separately from
  the merged-main evidence above. Release publication follows successful CI.
- The earlier `playtest-m8-1` draft remains an immutable private development
  candidate at `b004537701945e9945df7fbadb6e272b8fbdfc34`; it is not the public beta.

## Owner decisions and remaining acceptance

The owner confirmed brand review fully approved, reviewed the asset investigation
and requested a public testing beta. Signing and physical acceptance are explicitly
deferred for this beta. [ADR 0010](project/decisions/0010_PUBLIC_BETA.md) governs the
version-scoped beta approval; stable release gates remain separate.

[Asset evidence](project/ASSET_LICENSE_AUDIT.md) establishes exact original-release
matches for all 73 shipped XPMs and 24 cities, author-release matches/OFL for the
four fonts, and source provenance for Civic 89 additions. OpenSVG permits project
use in its current terms, but exact packs/notices for three inherited UI images
and the origins of two legacy source-only branding resources remain follow-ups.
No confirmed prohibited icon or licence incompatibility was established. This
does not claim a completed per-icon rights audit; inherited notices are preserved.

Still pending: physical monitor/mixed-DPI/readability/frame-pacing checks, audible
audio/native-dialog/accessibility acceptance on real machines, the above asset
follow-ups and trusted signing provisioning. Automated/virtual-display results
do not replace physical evidence. Beta testers should use
[testing instructions](packaging/BETA_RELEASE_NOTES.md) and report issues.

## Product and compatibility

One faithful original-gameplay contract. No larger maps, wider limits, new tools/
buildings/scenarios, balance changes, mods, achievements, day/night or seasons.
Classic graphics are default; optional palette-preserving 2x edge refinement
switches during play and persists in `graphics.cfg`, independently of city state,
RNG, timing, saves and `ui.cfg`. See [ADR 0009](project/decisions/0009_FAITHFUL_MODERNISATION_AND_GRAPHICS.md).

Keep current 51,360-byte `.cty` output and known M7 `.c89` compatibility. Historical
27,120-byte cities and exact RNG/sprite/scenario replay are unsupported. Scenario
exports reopen as ordinary cities. See [Classic contract](project/CLASSIC_COMPATIBILITY.md),
[Enhanced format](project/ENHANCED_CITY_FORMAT.md) and [fidelity audit](project/FAITHFULNESS_AUDIT.md).

## Next work and historical evidence

No further numbered milestone is agreed. Gather beta feedback, fix demonstrated
issues within scope, complete provenance/desktop acceptance and provision signing
before a stable release. The [roadmap](project/engineering/08_ROADMAP_AND_IMPLEMENTATION_BACKLOG.md)
retains the complete M0–M9 plan and dated delivery records.

Historical commands/results are under [tests/baseline](tests/baseline/), including
[bootstrap](tests/baseline/BOOTSTRAP_2026-10-05.md),
[M7](tests/baseline/M7_2026-10-06.md), [M8](tests/baseline/M8_2026-10-06.md)
and [M9](tests/baseline/M9_2026-10-07.md). Their older branch/draft/version statements
describe their capture date; current policy is ADR 0010. Do not rewrite historical
results as newly passed checks.

## Preserved provenance and workspace

- Root: `C:\Dev\Projects\civic89`; external vcpkg: `C:\Dev\vcpkg`.
- Origin: `https://github.com/DangerMouseUK/civic89.git`.
- Upstream: `https://github.com/ldicker83/Micropolis-SDLPP.git`, remote
  `upstream-sdlpp`, push URL `DISABLED`; complete history is retained.
- Fixed annotated tag `upstream-sdlpp-baseline` resolves to
  `9c4e85a0decd57ba6f76d9e1ec82461940ecc3ad` (captured 5 October 2026).
- Build/run commands: [BUILDING.md](project/BUILDING.md). Distribution and
  signing: [RELEASING.md](project/RELEASING.md). All inherited licences/notices
  and the retained Visual Studio comparison build remain intact.
