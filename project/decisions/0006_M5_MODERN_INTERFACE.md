# ADR 0006 — M5 single-window modern interface

Status: accepted for complete M5 delivery, 6 October 2026.
Base: merged M4 `fc0c4f653daa582902ef0db3d329c8e2eff06021` (PR #9).

## Decision

Use a native `ModernInterface` on the existing SDL3 renderer. Retain inherited UI
implementations as compiled references; do not move sources or add dependencies.
The application wires typed UI commands to existing city/tool/scenario/options APIs.
Fonts are the existing OFL Raleway Medium/Bold; icons, ghosts and minimap tiles reuse
inventoried assets. Glyphs render as UTF-8 through SDL3_ttf at physical density, then
use logical coordinates. A bounded 512-entry texture cache is owned by the interface.

- M5-01: Minimap shares the main renderer. Cache refresh remains one second, with
  immediate invalidation on city replacement and overlay/preferences changes.
  The viewport outline uses camera world bounds. Click/drag centers the camera and
  consumes the gesture, preventing accidental construction beneath UI.
- M5-02/03: Dashboard/status and a responsive dock show city data, tool/cost, numeric
  RCI, messages and tooltips. The palette uses original icons/ghosts, displays assigned
  keys and disables selections that cannot afford the base tool cost. Free Query
  remains usable in debt. Engine validation remains authoritative for terrain/drag
  costs. Keyboard tool changes cancel pending map gestures.
- M5-04: Budget/evaluation/history/query are native sheets. Budget changes use the
  same immediate typed setters, 0–20 tax clamp and existing 10% funding step. Display
  all evaluation fields, all six histories, series toggles and 10-/120-year views.
  Histories retain original sample order; common graph range expands above 256 if
  required. Scenario selection and new-city confirmation are also in-window.
- M5-05: `OverlayModel` reads the existing 2x2 and 8x8 effect maps, signed growth,
  tile power bits, transport and zone bands. Both renderers use the same colors;
  no simulation map/calculation is duplicated. Preserve scalar thresholds 50/100/
  150/200 and growth ±20/±100. Nearest full-map overlay textures follow camera and
  earthquake transforms. Opacity is 10–100%. Legends, numeric probes and a blue
  sequential alternative make data available beyond hue alone. Power distinguishes
  powered/unpowered/conductive tiles; growth uses distinct teal/amber directions.
- M5-06: Versioned `ui.cfg` validates scale, numeric ranges, booleans, key conflicts
  and reserved keys; malformed input falls back to defaults with diagnostics.
  Publish through the existing atomic Windows writer. Native window size stays
  independent of requested UI magnification. Three settings tabs expose readability,
  key/camera controls and Master/City/Construction gains; a fourth exposes existing
  gameplay options. Tab/Shift+Tab/Enter traverse controls; Escape closes/cancels.
  Requested 100/125/150% scale fits an 800x600 minimum logical layout. Large text is
  independent of that fit cap. No metadata is added to `.cty`.

## Compatibility decisions

Classic algorithms, costs, random-number calls, golden references and file layout
are unchanged. Presentation rendering/navigation/settings do not advance engine
state; automated digest checks cover the panels, overlays and graphics resets.

All sheets now suspend simulation/animation deadlines, including evaluation/history/
query/data selectors (the old interface made only budget/options modal). Closing a
sheet preserves pause and speed. This is an intentional wall-clock UI policy, not a
new simulation rule. Explicit gameplay settings retain existing semantics; the
Animation toggle now gates scheduled tile/sprite animation. Default animation remains
on, preserving Classic golden runs. Animation/disasters are session options; auto
budget/bulldoze/goto/sound retain existing city-file fields. New-city confirmation adds
one deliberate action before replacement. Scenarios still validate before mutation
and clear the previous save target on success. Native file pickers remain unchanged.

Recoverable graphics resets rebuild textures/fonts while retaining city, camera,
current panel, selected overlay, minimap visibility and status. The file picker is
application-owned, so reconstruction cannot discard its save target. All renderer
resources are released before SDL renderer/window shutdown. Existing `.sln`/`.vcxproj`
and CMake compile the same 64 production sources; the engine remains SDL-free.

## Validation and limits

See [M5 evidence](../../tests/baseline/M5_2026-10-06.md). Full application Debug/Release/
ASan and independent headless Debug/ASan checks pass; all 25 M2 goldens are retained.
Actual SDL input/pixels, compact/wide layouts, scale transitions, settings, affordability,
scenario/query routing and reconstruction have automated acceptance. Selected render
captures were visually inspected, including compact large text/high contrast.

This provides keyboard, scale/readability and color aids, not Windows screen-reader
integration. Physical mixed-DPI monitors, hardware VSync, speaker audibility and
interactive native file-picker QA remain release-matrix work. Historical 27,120-byte
cities, complete RNG/scenario restoration and ambiguous OpenSVG icon attribution
remain pre-existing release gates. The milestone adds no new assets/dependencies.
