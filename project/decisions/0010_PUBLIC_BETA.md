# ADR 0010 — Public testing beta

Date: 7 October 2026. Status: accepted owner decision.

M8 and M9 are merged in PRs #13 and #14. Their software checks passed, while
physical desktop acceptance remains pending. The owner requested a public beta
so people can download the game and provide that testing evidence.

## Decision

- Prepare `0.9.0-beta.1`, tag `v0.9.0-beta.1`, as a public GitHub prerelease,
  excluded from Latest. Supply x64 and native ARM64 installers/portable ZIPs,
  exact corresponding source, build identity and combined SHA-256 checksums.
- Record brand review as fully approved by the owner. This is owner confirmation,
  not an agent claim of independently performed legal clearance.
- The owner reviewed the [asset investigation](../ASSET_LICENSE_AUDIT.md) and
  requested publication with the inherited assets and disclosed provenance
  follow-ups. Do not claim the unknown OpenSVG pack notices or two source-only
  branding origins have been recovered. Preserve inherited notices and source;
  do not replace artwork or invent licences without a further decision.
- Publish this beta unsigned, clearly labelled. Signing and physical desktop
  acceptance are deferred for this testing release. Automated checks do not
  establish physical acceptance. Certificate provisioning can be undertaken
  independently of publication; a previous public release is not a technical
  requirement for Authenticode signing.
- Require clean, matching builds, successful CI and verified complete delivery
  before publication. Existing playtest tags/releases remain immutable.

This supersedes earlier requirements to keep M8/M9 PRs in draft and to block
**every** public download until signing and physical checks complete. It does
not mark those checks passed or remove the stable signed-release gates in
`packaging/release-gates.json`. Approval is scoped to this beta version; another
public version needs its own recorded release decision.

## Compatibility and follow-up

No gameplay, assets, dependencies, city formats, goldens or save writers change
for the beta. Classic remains the graphics default. Existing M7 `.c89` identities
and 51,360-byte `.cty` saves retain their existing support and snapshot limitations.

Collect real-machine bug reports, complete the remaining provenance/attribution
work and provision trusted signing before stable distribution. Bug-fix releases
remain within the faithful-modernisation scope of [ADR 0009](0009_FAITHFUL_MODERNISATION_AND_GRAPHICS.md).
