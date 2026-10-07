# M9 faithful pixel graphics

Direction selected by the user on 7 October 2026: sharper pixel art closely following
the original. M9 develops on `codex/m9-enhanced-graphics`, based on unmerged M8
`c3ba234`. Its PR targets the M8 branch until M8 is accepted and merged.

Enhanced graphics refine the existing pixel edges at twice the source resolution,
using an original implementation of a local, palette-preserving neighbour rule.
They are not a newly illustrated building set. Each original pixel produces four
pixels; matching perpendicular neighbours refine corners only when the opposite
neighbours disagree. Samples are clamped within each tile/frame, never taken from
an adjacent atlas entry. No blur, invented colours, lighting, seasons or new frames.
Classic graphics remain the default and retain their original pixels.

## Complete mapping and provenance

- All 960 tile IDs map to the same ID, including vacant zones, construction,
  buildings, networks, crossings, powered/unpowered indicators and animated states.
  Source rectangle: `(0, id*16, 16, 16)` in `images/tiles.xpm`; destination rectangle:
  `((id%32)*16*d, (id/32)*16*d, 16*d, 16*d)`, where `d` is 1 or 2.
- All 960 minimap entries retain `(0, id*3, 3, 3)` from `images/tilessm.xpm`.
  Its unused fourth source column stays unused. Enhanced cells are 6 by 6.
- All 61 frames of seven sprite types retain their frame numbers and offsets:
  train 5, helicopter 9, airplane 12, ship 9, monster 17, tornado 3, explosion 6.
- All ten existing textured construction previews retain tool IDs, transparency
  and world footprints. Untextured tool outlines, overlays, selection/query
  information and interface icons keep their current meanings and geometry.
- `assets/graphics-catalogue.json` records every input path, pinned source/hash,
  licence group, frame identity and dimensions. No replacement bitmap is imported
  or edited on disk. Derived GPU textures are reproducible from those inputs and
  `src/GraphicsArt.cpp`; original files stay byte-for-byte intact.

The inputs come from the pinned open-source SDLPP baseline, under the inherited
GPL/additional terms recorded in the asset ledger; derived art retains those terms.
The transformation implementation is GPL-3.0-or-later Civic 89 source. This does
not clear the separate per-asset/public-release, icon, brand or signing gates.

## State and resource contract

Graphics are a separate `graphics.cfg` application preference, with strict schema
and invalid/missing preference fallback to Classic. M8's `ui.cfg` stays unchanged
so an M8 rollback retains other preferences. The choice is independent of the
`.cty`/`.c89` city identity and never appears in city bytes.

Prepare the complete art set and map/minimap targets before committing a switch.
Commit with ownership swaps; retain camera, selected tool, panel/overlay, blink,
city/save destination, RNG and scheduler deadlines. On enhanced-load failure keep
the usable Classic session and report it. Device reset/startup retries Classic
if Enhanced cannot be reconstructed. Resources die before their SDL renderer.

Use nearest-neighbour sampling in both options. Map/sprite world units stay 16
pixels per tile; only backing textures increase. Overlay samples and animation
cadence stay unchanged. No simulation source change is required.

## Acceptance targets

Exercise all catalogue entries and both options at 800x600, 1366x768, 1080p, 1440p,
ultrawide and 4K, with 100/125/150/200% scale and fractional/integer camera zoom.
Compare deterministic city/RNG/scheduler/save output through repeated switches,
all scenarios, both save formats, resource failure and renderer reset. Preserve all
goldens. Record resource counts/bytes and frame-time distributions; target 60 fps
at 1080p and 30 fps at 4K on the tested native backend, with no claims for untested
hardware. Software rendering supplies correctness evidence, not GPU frame-rate
acceptance. Physical mixed-DPI/readability and visible-desktop checks remain human
acceptance gates and cannot be inferred from automated captures.
