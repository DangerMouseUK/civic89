# Target Architecture and Refactoring Plan

**Status:** Active engineering baseline  
**Prepared:** 5 October 2026  
**Repository baseline captured:** 5 October 2026  
**Scope:** **Civic 89** - native Windows modernisation of the open-source Micropolis / original SimCity code lineage

## 1. Architectural objective

The target is not a total rewrite. It is a staged separation of concerns around the authentic simulation.

The desired end-state is:

```text
+------------------------------------------------------+
| Application                                           |
| startup, main loop, settings, lifecycle               |
+----------------------+-------------------------------+
                       |
     +-----------------+-------------------+
     |                 |                   |
+----v-----+      +----v-----+       +-----v-----+
| UI       |      | Renderer |       | Audio     |
| panels   |      | camera   |       | mixer     |
| dialogs  |      | tiles    |       | buses     |
+----+-----+      +----+-----+       +-----+-----+
     |                 |                   |
     +-----------------+---------+---------+
                               |
                       +-------v--------+
                       | Application API |
                       | typed events    |
                       +-------+--------+
                               |
                       +-------v--------+
                       | Engine          |
                       | simulation      |
                       | map/economy     |
                       | no SDL/Win32    |
                       +----------------+
```

## 2. Phase-safe refactoring rule

Do not move and redesign at the same time. Prefer this sequence:

1. add a test around existing behaviour;
2. introduce an interface/adapter while old code still works;
3. route one call path through the interface;
4. verify parity;
5. delete the obsolete bridge;
6. only then move/rename source files.

## 3. Engine boundary

The engine should eventually expose explicit concepts such as:

- initialise/reset simulation;
- generate city from seed/options;
- load/save city state through a data interface;
- tick/step simulation;
- set/get simulation speed;
- apply editing tool at map coordinate;
- query map tile/cell state;
- query RCI, budget, evaluation and histories;
- retrieve active sprites in a renderer-friendly representation;
- emit typed simulation events.

The engine must not know:

- which SDL window exists;
- where the mouse is;
- how a dialog is drawn;
- how audio is played;
- where a save-file dialog came from;
- what DPI the display uses.

## 4. Replace the `Eval()` bridge

### 4.1 Inventory

Before deletion, create a machine-readable or documented inventory of all `Eval()` call sites and the command strings they can produce.

Classify each as one of:

- audio command;
- UI notification;
- scenario lifecycle;
- camera/navigation request;
- disaster visual effect;
- obsolete/no-op legacy path.

### 4.2 Typed replacement

A simple first design can use interfaces rather than a complicated event framework:

```cpp
struct AudioService {
    virtual ~AudioService() = default;
    virtual void playEffect(SoundId sound, AudioChannel channel) = 0;
    virtual void startLoop(SoundId sound, AudioChannel channel) = 0;
    virtual void stopLoop(AudioChannel channel) = 0;
};

struct GamePresentationEvents {
    virtual ~GamePresentationEvents() = default;
    virtual void showMessage(const GameMessage& message) = 0;
    virtual void focusMap(MapPosition position) = 0;
    virtual void earthquakeStarted(int strength) = 0;
    virtual void scenarioStarted(ScenarioId id) = 0;
};
```

Avoid a new string-keyed event bus that recreates the original problem.

## 5. Application lifecycle

Introduce an `Application` object that owns high-level services and makes teardown order explicit.

Example ownership:

```text
Application
  SDLContext
  Window
  Renderer
  AssetManager
  AudioManager
  SaveManager
  SettingsService
  GameSession
    Engine
  UIManager
```

Destruction order must ensure GPU/textures/fonts/UI are released before renderer/window/SDL shutdown. This directly addresses the type of lifetime problem documented in SDLPP issue #18.

## 6. Renderer design

### 6.1 Short-term

Keep SDL_Renderer while modernising presentation. SDL can select the appropriate backend and this keeps the migration small.

### 6.2 Camera

Create a dedicated `Camera2D` with:

- world position in tile/pixel coordinates;
- zoom;
- viewport size;
- conversion between screen and world coordinates;
- clamping to map bounds;
- zoom-to-cursor behaviour;
- optional pixel-perfect zoom levels.

Mouse wheel zoom should preserve the world point under the cursor where possible.

### 6.3 Pixel-art rules

Classic tiles should remain crisp. Provide two scaling modes if useful:

- Pixel Perfect - nearest-neighbour integer or controlled scaling;
- Smooth View - allows fractional camera scaling while keeping UI text crisp.

The user interface itself should be resolution independent even when the map art is pixel-based.

### 6.4 Frame pacing

Introduce explicit display settings for:

- VSync on/off;
- fullscreen/windowed/borderless;
- target update behaviour if simulation and rendering rates differ;
- renderer backend diagnostics.

Do not tie simulation ticks directly to monitor refresh rate.

## 7. DPI and display handling

The app should be per-monitor-DPI-aware through SDL's modern window/display APIs. Test:

- 100%, 125%, 150%, 200% scale;
- moving the window between monitors with different scale factors;
- 1080p and 4K;
- small laptop screens;
- ultrawide aspect ratios.

UI layout should use logical dimensions and measured text, not only fixed 800x600 offsets inherited from the classic shell.

## 8. UI architecture

The existing native windows are useful functional references. The modern default should use a single main window with in-game panels/docks/overlays.

Core panels:

- main dashboard/status strip;
- tool palette;
- minimap;
- budget;
- evaluation;
- graphs;
- data overlays;
- query/inspection panel;
- options/settings;
- new/load/scenario launcher.

Panels should be independently showable and layout-managed. Avoid scattering absolute coordinate constants through simulation/application code.

## 9. Minimap strategy

Move the default minimap into the main renderer/UI. Preserve current overlay data sources. Optionally support a detachable minimap later if it remains useful for multi-monitor users.

The overlay model should be reusable by both minimap and full-map overlays:

```text
OverlaySource -> normalised data field -> colour mapping -> renderer
```

This avoids duplicating crime/pollution/traffic calculations for different views.

## 10. Audio architecture

Use an `AudioManager` that translates typed game sound IDs into SDL3_mixer tracks/resources.

Suggested logical groups:

- UI;
- construction;
- city ambient;
- disasters;
- vehicles;
- music (if legally distributable assets are later added).

Provide separate master/effects/music sliders even if the baseline ships without music.

Audio assets must be licensed and tracked independently from code.

## 11. Save and settings architecture

### 11.1 Classic city files

Keep original `.cty` read/write logic isolated and tested. Do not add arbitrary modern metadata to the legacy binary without a compatibility design.

### 11.2 Modern metadata

If thumbnails, play time, autosave information or Enhanced Mode data are needed, prefer a sidecar/container format rather than silently modifying legacy `.cty` layout.

Possible approach:

```text
My City.cty              # classic-compatible city
My City.citymeta.json    # optional modern metadata
My City.png              # optional thumbnail, or embed in future container
```

A future new container format can wrap classic state once requirements are known, but legacy import/export should remain available.

### 11.3 Safe save strategy

Use atomic-style writes:

1. write to temporary file;
2. flush/close;
3. validate basic structure/checksum where applicable;
4. replace target;
5. retain recovery/autosave according to policy.

## 12. Logging and error handling

Replace silent failures and console-only errors with structured errors that can be surfaced appropriately.

Examples:

- asset missing -> user-facing startup error + log detail;
- city parse error -> clear "cannot load this city" message + diagnostic;
- audio device failure -> non-fatal fallback, game remains playable;
- renderer creation failure -> actionable error listing attempted backend.

Exceptions should not be used as a substitute for normal game-state control flow.

## 13. Threading

Keep the simulation single-threaded initially. The classic code is small and correctness is more valuable than speculative concurrency.

Potential later parallel work can include:

- asset decoding;
- screenshot encoding;
- autosave serialization if proven safe;
- expensive Enhanced Mode analytics.

Do not parallelise the simulation until determinism and shared-state assumptions are understood.

## 14. Enhanced Mode extension points

Prepare interfaces for future expansion without implementing features early:

- map dimensions supplied through configuration instead of constants where practical;
- typed building/tool identifiers;
- versioned rule sets;
- new save metadata/versioning;
- renderer layers independent of simulation arrays;
- mod/data hooks only after security/versioning requirements are defined.
