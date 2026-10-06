# ADR 0009 — Faithful modernisation and optional graphics

Date: 6 October 2026. Base: merged M7 `eb73f64` (PR #12).
Status: accepted project scope; M8 implementation delivered on its branch with
physical acceptance pending. Optional M9 graphics remain planned.

The user clarified that Civic 89 must preserve the original game's gameplay and
mechanics while modernising its appearance and Windows experience. The only
optional enhancement considered is a graphics toggle. The user then requested
that these remaining milestones be written up and the M7 adjustment placed in M8,
because M7 has already been merged.

## Decision

1. Maintain one faithful simulation contract. Larger maps, wider finance/population
   limits, new buildings/tools/scenarios, altered traffic/utilities/balance, mods and
   achievements/challenges are outside scope. Day/night and seasonal additions are
   excluded from the graphics milestone.
2. M8 delivers the faithfulness audit, historical-save investigation, Windows polish
   and desktop acceptance. It also adjusts the merged M7 city-mode interface so
   city creation, status and help do not imply alternative gameplay.
3. Retain M7's save compatibility: known `.c89` identities remain readable, `.cty`
   output stays unchanged, and existing files/recovery slots are not forcibly
   rewritten, converted, retagged or deleted. Retain supported save/export paths,
   atomic failure preservation and rejection of unknown/corrupt input before mutation.
4. Optional M9 graphics belong in Settings and can switch during play. Graphics
   must not affect city state, ruleset identity, RNG consumption, simulation or
   animation timing, or serialized city bytes. Store the preference outside saves;
   use the same existing tile/sprite identities, footprints, states and sequences.
5. M9 needs agreed art direction and a complete licensed asset mapping before
   implementation. Keep original graphics available and recover safely if enhanced
   resources fail to load. No new mechanics or city conversion accompany the toggle.
6. Preserve all baseline goldens. Inherited SDLPP parity is evidence about that
   baseline, not proof of every original retail behaviour. Record fidelity
   discrepancies and obtain explicit user direction before outcome-changing fixes.

## Superseded direction and retained history

This decision supersedes the future gameplay-expansion direction of the charter's
ADR-007, the former post-M7 candidate backlog and the future-candidate clauses of
[ADR 0008](0008_M7_ENHANCED_FOUNDATION.md). Those records describe the earlier scope;
M7 remains completed and merged. Neither the M7 implementation nor its acceptance
evidence is retroactively removed. Its interface adjustment is part of M8.

M7 `0.7.0-dev` exposed Classic/Enhanced city modes with the same mechanics.
M8 `0.8.0-dev` starts one original-gameplay city path, shows the current extension
in Files and keeps M7 load/save/import/export/recovery and CLI identities. A failed
Save As retains the previous destination; no existing city is forcibly retagged.

## Delivery and acceptance

The [roadmap](../engineering/08_ROADMAP_AND_IMPLEMENTATION_BACKLOG.md) defines complete
M8/M9 backlog items and acceptance. Deliver one whole milestone per engineering
branch/PR. The initial scope commit was documentation only; the M8 branch now
includes the runtime adjustment, [audit](../FAITHFULNESS_AUDIT.md), regression
checks and [evidence](../../tests/baseline/M8_2026-10-06.md). M8 acceptance remains
pending until the required physical checks have actual evidence. M9 is not
implemented. [Project status](../../PROJECT_STATUS.md) records progress.

Historical file variants and exact replay remain limited by the supported snapshot
format. Do not invent format compatibility without verified references/fixtures.
Physical mixed-DPI/audio/accessibility and clean-machine checks need actual evidence;
dummy/software tests cannot establish them. Asset/brand clearance and trusted signer
provisioning remain separate public-release gates.
