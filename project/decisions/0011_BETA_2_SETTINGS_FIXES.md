# ADR 0011 — Beta.2 settings fixes

Date: 7 October 2026. Status: accepted owner decision.

After merging [PR #17](https://github.com/DangerMouseUK/civic89/pull/17), the owner
approved proceeding with the proposed `0.9.0-beta.2` release: prepare a release
branch/PR, update version numbers and release records, verify both architectures
through CI, and publish a fresh beta with matching source and checksums.

## Decision

- Publish `0.9.0-beta.2`, tag `v0.9.0-beta.2`, as an unsigned public GitHub
  prerelease excluded from Latest. Supply x64 and native ARM64 installers and
  portable ZIPs, exact corresponding source, build identity and SHA-256 checksums.
- Include the merged fix that closes the audio-preferences reader after startup,
  allowing settings to be replaced on later launches. Clarify the overlay-opacity
  control and include the returning-player persistence regression.
- Continue [ADR 0010](0010_PUBLIC_BETA.md)'s reviewed asset decision and owner brand
  approval. Artwork and dependencies are unchanged; exact OpenSVG pack notices
  and two legacy source-only branding origins remain disclosed follow-ups.
- Continue deferring trusted signing and physical desktop acceptance for this
  testing beta. Do not represent either as passed or change stable-release gates.
- Require all five CI jobs green, clean matching versioned builds, both
  architectures, exact source and verified complete delivery before publication.
- Keep beta.1 and earlier playtest tags/assets immutable. This decision authorises
  beta.2 publication; it does not authorise merging the release PR on the owner's
  behalf. A verified CI integration commit may be used while the PR awaits review.

## Compatibility

The settings fix and clearer control affect presentation and preference persistence.
Simulation, artwork, dependency versions, goldens, `.cty` output and M7 `.c89`
compatibility are unchanged. Existing user preferences and saves remain in their
existing locations. Portable users should close the game and extract the new ZIP
to a fresh folder; there is no automatic network updater.

Later public versions need a new recorded owner decision. Continue gathering
real-machine feedback and completing the follow-ups in ADR 0010 before stable
signed distribution.
