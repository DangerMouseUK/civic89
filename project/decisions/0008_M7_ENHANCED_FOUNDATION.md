# ADR 0008 — M7 Enhanced Mode foundation

Date: 6 October 2026. Base: merged M6 `fa3534b` (PR #11).
Branch: `codex/m7-enhanced-foundation`.

**Historical decision; future expansion direction superseded:** M7 is merged in
PR #12. The user's later 6 October 2026 decision in
[ADR 0009](0009_FAITHFUL_MODERNISATION_AND_GRAPHICS.md) withdraws the gameplay
candidates mentioned below. Preserve this M7 delivery record and save compatibility;
put the interface adjustment in M8 and optional graphics in M9.

The user selected the complete foundation scope, rather than all nine speculative
gameplay candidates. Define the Classic development contract first; do not label
the product a stable public release or waive the existing release gates.

## Decisions

1. [Classic v1](../CLASSIC_COMPATIBILITY.md) specifies the actually supported engine,
   platform and 51,360-byte save behavior, including historical/replay limitations.
2. Use a typed versioned ruleset registry. Classic v1 and Enhanced v1 currently
   share simulation mechanics and dimensions. Identity is session data, not a UI
   preference that can silently change an existing city. Unknown versions fail closed.
3. Select a mode explicitly when creating a city. The app and headless runner expose
   the active identity. Inherited scenarios are Classic v1; their outcomes stay intact.
4. Preserve `.cty` bytes. Enhanced v1 uses a bounded, checksummed `.c89` container
   around the existing ordinary-city payload with its ruleset and UTF-8 city name.
   Explicit import preserves the source and selects Enhanced; explicit export leaves
   the active mode unchanged. Modes have separate autosave slots; recovery identifies the mode.
5. Test the complete boundary and retain unchanged Classic goldens, fresh-process
   generated-city baselines, source parity and all M6 native delivery checks.

## Compatibility and scope

No engine algorithm, random-number call, tool cost, map size, financial type or
legacy city-file layout changes. Enhanced v1 is a tagged foundation, not a larger-map
or richer-traffic implementation. Those features, new buildings/scenarios, data/mod
exploration, visual cycles and achievements remain the future candidate backlog.
Every future ruleset/feature must state its Classic import/export capability.

All five items are delivered in [PR #12](https://github.com/DangerMouseUK/civic89/pull/12).
[PROJECT_STATUS.md](../../PROJECT_STATUS.md) and [M7 evidence](../../tests/baseline/M7_2026-10-06.md)
record 53 application/43 headless checks, native ARM64 and complete delivery acceptance.
All 25 Classic goldens remain unchanged; Enhanced gameplay candidates remain future work. Signing provisioning and public
asset/brand/physical acceptance remain external gates.
