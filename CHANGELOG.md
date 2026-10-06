# Changelog

## 0.8.0-dev — M8 faithfulness and Windows polish

- Original-gameplay new-city flow/status; Files retains both M7 save formats and import/export.
- Save As commits its destination after publication; cancellation/failure preserves the earlier location.
- Windows key chords no longer activate panel controls; focus loss clears keyboard capture and gestures.
- Audited mechanics/data, tool/economy/network regressions and safe rejection of 24 historical cities.
- Expanded 1080p/1440p/4K/ultrawide layouts at 100–200% scale and compatibility/desktop evidence.

No simulation algorithm, golden result or city payload/schema change. Physical
display/audibility acceptance remains pending; see M8 evidence. Optional M9 graphics
are planned. Enhanced gameplay expansion is withdrawn by ADR 0009.

## 0.7.0-dev — M7 Enhanced Mode foundation

- Supported Classic v1 development contract with explicit historical/replay limits.
- Typed versioned Classic v1/Enhanced v1 rulesets, native new-city selection and CLI mode selection.
- Bounded, checksummed `.c89` saves with ruleset/name metadata; unchanged Classic `.cty` layout.
- Explicit Classic import/export, atomic publication and separate mode-aware autosave recovery.
- Native mode-selection acceptance, malformed/unknown-version save rejection and fresh-process parity tests.

Enhanced v1 currently uses Classic mechanics and the 120 by 100 map. All existing
Classic golden results are retained. The future gameplay-expansion direction was
subsequently withdrawn by ADR 0009; public-release clearance remains separate work.

## 0.6.0-dev — M6 development candidates

- Portable ZIP, per-user installer, corresponding source and SHA-256 manifests.
- Version/commit/architecture provenance in executable resources, CLI and logs.
- Separate manually dispatched signing integration and explicit public-release gates.
- Offline portable staging, atomic selection and rollback with failure acceptance.
- Native Windows ARM64 Release build/test/packaging CI.
- Civic 89 root README, original application icon and preserved upstream attribution.

The Classic simulation and current city-file layout are unchanged. Public release
clearance, trusted signing and physical desktop acceptance remain pending.
