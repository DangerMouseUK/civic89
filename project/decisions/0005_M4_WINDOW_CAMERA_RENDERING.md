# ADR 0005 — M4 window, camera and rendering

Status: accepted for the complete M4 engineering branch, 6 October 2026.
Base: merged M3 `555b7ba6f5b67cccb5917a05ec991a7aff6a8e87` (PR #8).

## Decision

Keep SDL_Renderer and the existing application/UI architecture. Add a presentation
camera, logical display layout, validated atomic display preferences and main-thread
deadlines. Do not move source files or introduce a new dependency.

- M4-01: Windowed, maximized and desktop borderless fullscreen use SDL window APIs.
  F11 toggles borderless; F12 opens native mode/VSync/pixel-perfect controls.
  Normal logical size and preferences are stored in `display.cfg` beside M3 user
  data, with version/range validation and atomic replacement. Window resize writes
  are debounced; close and explicit settings changes publish immediately. Fullscreen
  does not change monitor resolution. Mode failures are logged and surfaced.
- M4-02: Main rendering scales from output pixels to logical UI units using the
  window's display scale. Pixel-size/display/scale events refresh layout and camera
  bounds; detached-window events cannot resize the main view. Input uses each
  window's own SDL render-coordinate conversion, retaining fractional map positions.
  Existing panels center in logical units. On a screen too small for the inherited
  800×600 layout at requested DPI, reduce effective scale to keep panels reachable;
  M5 remains responsible for responsive panel redesign. The minimap uses its own
  logical presentation and display scale, independent of the main monitor.
- M4-03: Camera2D owns fractional world position, viewport, zoom and conversions.
  Right-drag and held arrows pan smoothly; wheel zoom anchors at the cursor; Home
  centers the map. Zoom is normally 0.25–4. Pixel perfect quantizes the physical
  source-pixel scale and aligns the world origin; art uses nearest filtering.
  Views larger than the map show background rather than stretched/invalid cells.
  Preview, sprite and tool placement use the same transform. Minimap selection uses
  visible world extent rather than window size and centers on the selected tile.
- M4-04: Remove all four SDL timer callbacks and their cross-thread flags.
  Main-thread deadlines retain simulation intervals 100/50/25/5 ms by speed,
  animation 150 ms, blink 500 ms and minimap refresh 1 s. Process simulation and
  animation deadlines chronologically so render refresh cannot reorder engine RNG
  calls. At most 32 due events are processed per iteration; excess time after a long
  stall is discarded. Modal windows discard simulation/animation deadlines. VSync
  requests are checked; unsupported/off uses a 60 Hz nanosecond frame limiter.
  Simulation continues between rendered frames, including the 5 ms speed.
- M4-05: MapRenderer owns atlas, map cache and sprite cache. It centralizes tile,
  sprite, construction-preview and quake-offset drawing. The application schedules
  updates and invokes one render operation; it no longer builds atlases, draws tiles
  or computes screen rectangles. Cache refresh covers visible tiles on state/blink
  changes or newly visible bounds. Only the window frame is presented after UI.
  Existing minimap data-overlay rendering remains in its renderer; in-window
  minimap/full-map data-layer UX is M5, without duplicating simulation calculations.

## Explicit compatibility decisions

Classic simulation algorithms, seed handling, RNG, golden fixtures, map dimensions,
construction costs and `.cty` bytes remain unchanged. Scheduling makes elapsed-time
progress independent of display refresh; this changes wall-clock behavior under
render stalls compared with the inherited boolean timer coalescing. Chronological
bounded catch-up and modal suspension are intentional presentation/runtime policy,
not new simulation rules. Parity continues to compare fixed engine phase counts.

The existing typed auto-goto event now centers the camera when enabled. Earthquake
events now shake the map/sprites/previews for three seconds, refresh on another
quake and stop on city reset or expiry. UI stays steady. Presentation offsets use
no engine randomness; only the inherited presentation counters are cleared on
expiry. Disaster map mutations, sounds and scenario outcomes are untouched.

Recoverable render-device/target reset recreates presentation textures/UI and
callbacks, cancels incomplete tool gestures and preserves city/camera state. Actual
unrecoverable device loss closes with a logged error and existing recovery on restart.
Public redistribution remains subject to the inherited asset audit in M3; no new
assets or licences were added.

## Verification and limits

[M4 evidence](../../tests/baseline/M4_2026-10-06.md) records commands, software-renderer
pixel sampling, scaled construction/input, native Windows mode transitions, settings,
timing/order checks and unchanged engine goldens. Automated 100/125/150/200% and
mixed-scale sequences exercise the same transforms used by monitor changes. Native
hidden-window API checks supplement dummy-driver CI; they do not establish visual
quality or actual physical mixed-DPI display behavior. Hardware refresh, visible
desktop review and physical monitor scaling remain release-matrix checks.

SDL's [high-DPI guidance](https://wiki.libsdl.org/SDL3/README-highdpi) defines the
pixel/display-scale distinction. [Event conversion](https://wiki.libsdl.org/SDL3/SDL_ConvertEventToRenderCoordinates)
includes relative motion. [Render scale](https://wiki.libsdl.org/SDL3/SDL_SetRenderScale)
is per target. [Fullscreen](https://wiki.libsdl.org/SDL3/SDL_SetWindowFullscreen)
defaults to desktop borderless and [window synchronization](https://wiki.libsdl.org/SDL3/SDL_SyncWindow)
waits for transitions. [VSync](https://wiki.libsdl.org/SDL3/SDL_SetRenderVSync)
is backend-dependent and must be checked. These contracts were checked against the
pinned SDL3 3.4.18 headers as well as the official reference.
