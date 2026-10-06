# ADR 0008 — M7 Enhanced Mode foundation

Date: 6 October 2026. Base: merged M6 `fa3534b` (PR #11).
Branch: `codex/m7-enhanced-foundation`.

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
   Explicit import/export preserves originals and does not silently retag the active
   session. Different modes have different autosave slots; recovery identifies the mode.
5. Test the complete boundary and retain unchanged Classic goldens, fresh-process
   generated-city baselines, source parity and all M6 native delivery checks.

## Compatibility and scope

No engine algorithm, random-number call, tool cost, map size, financial type or
legacy city-file layout changes. Enhanced v1 is a tagged foundation, not a larger-map
or richer-traffic implementation. Those features, new buildings/scenarios, data/mod
exploration, visual cycles and achievements remain the future candidate backlog.
Every future ruleset/feature must state its Classic import/export capability.

Implementation and final acceptance are recorded in PROJECT_STATUS.md and the M7
evidence before delivery of the whole milestone PR. Signing provisioning and public
asset/brand/physical acceptance remain external gates.
