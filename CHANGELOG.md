# Changelog

Entries below the beta are dated development delivery records, not current
branch or publication status.

## 0.9.0-beta.2 — 7 October 2026

- Close the audio-preferences reader after startup so changing settings on a
  subsequent launch can replace `audio.cfg` without a Windows access-denied error.
- Label the dashboard percentage as overlay opacity and explain its effect.
- Regression coverage starts with existing audio preferences and verifies repeated
  volume saves, persisted opacity and unchanged simulation state.
- New x64/ARM64 testing-beta deliveries with the same unsigned-release policy.

No gameplay, city format, artwork, dependency or golden-reference change.

## 0.9.0-beta.1 — 7 October 2026

- First public testing beta combining merged M8 Windows polish and M9 optional graphics.
- x64/ARM64 portable ZIPs and per-user installers, exact source and combined checksums.
- Repository-wide documentation reconciliation, durable asset investigation and
  explicit owner-approved beta policy. Brand review approved; signing and physical
  desktop acceptance deferred for this unsigned prerelease. Provenance follow-ups disclosed.
- Verified beta staging requires the approved version/tag, clean matching builds
  and both architectures before upload. Stable signed-release gates remain separate.

No gameplay, city format, artwork, dependency or golden-reference change.

## 0.9.0-dev — M9 optional faithful graphics

- Classic/Enhanced graphics in Settings, independent of city identity and stored in `graphics.cfg`.
- Palette-preserving 2x pixel-edge refinement across all 960 tiles/minimap cells,
  61 sprite frames and ten textured previews, using unchanged pinned inherited assets.
- Complete resource preparation before live switching; recover to Classic after
  enhanced startup/reset failure, and retain the current session on a failed switch.
- Full catalogue captures, display/zoom matrix, resource/performance measurements,
  injected failure/recovery and fresh-process state/RNG/animation/save comparisons.
- Both graphics options covered by portable/installer smoke verification.

M9 is stacked on pending M8. No simulation algorithm, golden, animation sequence,
city format or legacy writer changes. Physical acceptance and existing public
asset/brand/signing gates remain pending. Enhanced is refined original pixel art,
not a newly illustrated asset pack.

## 0.8.0-dev — M8 faithfulness and Windows polish

- Original-gameplay new-city flow/status; Files retains both M7 save formats and import/export.
- Save As commits its destination after publication; cancellation/failure preserves the earlier location.
- Windows key chords no longer activate panel controls; focus loss clears keyboard capture and gestures.
- Audited mechanics/data, tool/economy/network regressions and safe rejection of 24 historical cities.
- Expanded 1080p/1440p/4K/ultrawide layouts at 100–200% scale and compatibility/desktop evidence.
- Verified GitHub draft playtest delivery with both architectures, matching source,
  combined checksums and a manual release workflow; public publication remains gated.

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
