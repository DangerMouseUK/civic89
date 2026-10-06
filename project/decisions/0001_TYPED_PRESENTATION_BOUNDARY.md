# ADR 0001: Typed presentation boundary

**Status:** M1 completed. This ADR retains the original design/checkpoint history; [ADR 0002](0002_M1_COMPLETION.md) records final implementation, compatibility decisions and milestone evidence.

**Date:** 5 October 2026

**Scope:** M1-01 through M1-04. All runtime bridge paths are migrated or explicitly retired.

## Context

The [bridge inventory](../reference/LEGACY_EVAL_INVENTORY.md) initially identified 12 live `Eval()` calls; none remain after completing M1. At the initial audit the bridge only logged and returned `false`. Several wrappers were dormant, scenario startup was disconnected from loading, and audio payloads retained script syntax. Replacing strings with a generic string-keyed event bus would preserve the same ambiguity.

Follow the governing [phase-safe refactoring sequence and typed boundary](../engineering/05_ARCHITECTURE_AND_REFACTORING.md): test, introduce an interface, route one path, verify parity, delete the obsolete path, then consider source moves. Engine extraction and functioning audio are later milestones.

## Proposed boundary

Use small synchronous C++ interfaces, called on the existing application/simulation thread. The application owns and injects the adapters; no global string registry, queued event framework, SDL types or window names cross this boundary. Begin with recording adapters for tests and a null audio adapter. Construct adapters before their callers and keep them alive until the game/session stops.

The following is the original illustrative design. Production `AudioService.h` now represents every retained sound identity, with `City` / `Construction` and effect/loop/stop methods. `PresentationEvents.h` implements the typed message, focus, earthquake, generation and scenario observations. `ScenarioController.h` supplies validated native startup and typed scoring. The wider rate API below was not implemented: unexecuted script speed modifiers were retired explicitly without guessing a multiplier (ADR 0002).

```cpp
enum class AudioChannel { City, Construction };
enum class SoundId {
    ExplosionLow, ExplosionHigh, HeavyTraffic,
    HonkLow, HonkMedium, HonkHigh, Monster, Siren,
    MustBulldoze, InsufficientFunds, Bulldozer
};
struct PlaybackRate { double multiplier = 1.0; }; // finite, positive; 1 is normal
struct MapPosition { int x; int y; };            // map tile coordinates
enum class ScenarioOutcome { Won, Lost };

struct AudioService {
    virtual ~AudioService() = default;
    virtual void playEffect(SoundId, AudioChannel, PlaybackRate) = 0;
    virtual void startLoop(SoundId, AudioChannel) = 0;
    virtual void stopLoop(AudioChannel) = 0;
    virtual void stopAll() = 0;
};

struct PresentationEvents {
    virtual ~PresentationEvents() = default;
    virtual void focusMap(MapPosition) = 0;
    virtual void earthquakeStarted() = 0;
    virtual void scenarioStarted(Scenario) = 0; // existing FileIo.h enum
    virtual void scenarioFinished(ScenarioOutcome) = 0;
};

enum class ScenarioStartResult { Started, InvalidScenario, MissingFile, InvalidFile };
struct ScenarioController {
    virtual ~ScenarioController() = default;
    virtual ScenarioStartResult start(Scenario) = 0;
};
```

`ScenarioController` is an application command: it validates and loads, then notifies presentation on success. `scenarioStarted` is an observation and must not trigger loading recursively. Preserve the existing `Scenario` enumerators and explicitly map them to legacy IDs 1-8; the enum itself is numbered 0-7. The final implementation has validated fixture loading, failure isolation, initialized-event tests, F6 selection and startup selection.

Keep dashboard message delivery through its existing direct callback initially. Add a typed message value only when migrating that boundary, using the existing `NotificationId` plus optional map coordinates. Decide whether dormant picture notifications are needed before adding an API for them. The implemented `generationStarted` observation is tested at the inherited position before map generation; it does not claim generation completion.

Loop stop can use a channel because the baseline has a single construction loop (`1`). If future requirements permit multiple simultaneous loops on one channel, introduce typed handles then. Explicitly separate persistent mute/volume settings from device/resource initialization; preserve `MiscHistory[55]` serialization until its compatibility decision has a round-trip test.

`PlaybackRate` describes the future native contract. It is **not** an interpretation of legacy `-speed 80` or `[MonsterSpeed]`; their units/computation remain unresolved. Default effects can migrate first. Those call sites now use distinct typed sound identities through a null backend; the script modifiers were explicitly retired in ADR 0002. No parser or script evaluator was added.

## Decisions beyond M1

- **Audio:** a null adapter keeps silent playback. Typed tool-error identities are restored; audible playback, mute/shutdown repairs, activation of dormant notification sounds and licensed assets remain later work. Activating that switch changes RNG consumption.
- **Earthquake:** keep `MakeEarthquake` damage and RNG order untouched. The renderer/application should own visual duration and cancellation. The current 3,000 ms timer is disabled and `ShakeNow` is unread by rendering; actual visual shake is new presentation behavior. Do not reinterpret its increment count as simulation strength.
- **Scenario:** all eight native startups, lifecycle counters, thresholds and failure isolation are now tested. The missing win notification and history-reset defects are repaired in the lifecycle change. Historical city-format interoperability and deterministic long-running outcome parity remain later work.
- **Navigation and generation:** activating auto-goto affects presentation; moving the generation notification changes ordering. Preserve dashboard text and engine results and add focused tests for the chosen behavior.

## Migration sequence and evidence

1. Completed the inventory and inherited audio/earthquake characterization without production edits in `8f7d3e1` / `8b4d5c2`.
2. Implemented the typed earthquake sound request and recording/null adapters, preserving sound-before-visual order. Visual timing and simulation damage remain separate; evidence is recorded below.
3. Completed audio controls, all effect identities and typed presentation observations with recording/null/native lifecycle tests; script modifiers and unused wrappers are explicitly retired.
4. Completed validated scenario loading/controller, all eight fixture/native startup checks, failure tests and selection routes.
5. Deleted `Eval` and obsolete wrappers after migration/retirement; CTest guards against reintroduction. Source layout remains unchanged.

## Historical checkpoint: first route

At this first checkpoint, the application-owned `NullAudioService` was passed through the inherited no-argument `DoEarthQuake()` adapter to `DoEarthQuake(AudioService&)`. `MakeEarthquake` and its callers remained untouched. The helper sent `ExplosionLow` on `City` through the typed `MakeSound` overload, then retained the legacy visual command and shake/timer mutations in their original order. The subsequent presentation migration below adds the second explicit service parameter.

The typed sound overload retains the same private enable guard and lazy `SoundInitialized` update as the string overload. This deliberately preserves the value saved through `userSoundOn()` / `MiscHistory[55]`; mute/shutdown repairs remain separate decisions. The null implementation opens no device, loads no asset and adds no dependency. The diagnostic difference at this checkpoint was removal of `Eval: UIMakeSound "city" "Explosion-Low"`; `DoEarthQuake` and `Eval: UIEarthQuake` still logged. Playback stayed silent. Other effects remained on the string path at this first checkpoint; subsequent migrations are recorded below.

The recording adapter exists only in `tests/LegacyBridge.cpp`. Tests check the exact typed sound/channel, one request per earthquake, initialization before dispatch, and console/shake/timer snapshots at dispatch to prove it precedes the visual command and state increment. Repeated start and explicit stop retain their prior state assertions. Running the helper with the actual null adapter also verifies lazy initialization and the absence of a string sound command.

This header-only service keeps the existing 47 production translation units and leaves `.sln`/`.vcxproj` unchanged. CMake Debug/Release builds and all three CTests pass. The inherited Visual Studio Release build is checked separately. Commands remain in [BUILDING.md](../BUILDING.md); local logs use `out/audit/typed-audio-*`.

Tests characterize helper routing/state, not real audio, camera shaking or simulation parity. Scenario smoke and deterministic disaster/map digests remain open gates. The disaster RNG/damage code, save implementation and disabled visual timers are unchanged; no source move or new production dependency is introduced.

## Historical checkpoint: audio controls

Before migration, cold `SoundOff` and the stop guard were characterized against the actual legacy code in both configurations (`f25acee`). The guard test starts a loop, clears initialization through the existing sound setter, then verifies that `StopBulldozer` leaves `Dozing` set and that `SoundOff` reinitializes and clears it. This records an inherited behavior rather than repairing it during migration.

The application retains its no-argument `StartBulldozer`, `StopBulldozer` and `SoundOff` entry points as adapters to overloads that accept `AudioService&`. `StartBulldozer` requests `startLoop(Bulldozer, Construction)` before setting `Dozing`; repeat starts remain suppressed. `StopBulldozer` requests `stopLoop(Construction)` before clearing `Dozing`; repeated initialized stops still dispatch, while cold/disabled guards remain. `SoundOff` lazily initializes, requests `stopAll`, then clears `Dozing` without changing persistent mute behavior.

The old internal `DoStartSound` / `DoStopSound` functions and declarations have no remaining callers and are removed. Along with `UISoundOff`, this retires three live `Eval` calls. It also retires the old `DoStartSound` declaration/definition mismatch. No strings, loop-number parsing or registry replace these commands; the baseline's sole loop (`1` on `edit`) is represented by typed `Bulldozer` and `Construction`.

Recording tests assert request kinds, sound/channel, initialization and loop state at dispatch, request order and repeat suppression. The production null adapter is also exercised for cold stop, start, stop and stop-all. The controls still have no game callers, and no new UI route enables them. Expected diagnostic changes are removal of `UISoundOff`, `UIStartSound` and `UIStopSound` logs. No device, sample, save-format or simulation behavior is changed. Verification logs use `out/audit/typed-control-*`; build/test commands remain in [BUILDING.md](../BUILDING.md).

## Historical checkpoint: default city effects

After PR #3 merged, the legacy helper was characterized for high/low explosions, traffic reports and low horns (`3985437`); targeted tests passed in Debug and Release before production edits. The application now provides `MakeSound(SoundId, AudioChannel)` as an adapter to the existing explicit-service overload, using its owned null service. This follows the application adapter pattern already used by earthquake and loop controls without a service registry or string conversion.

Four calls in `Sprite.cpp` and eight calls in `ToolActions.cpp` now use `ExplosionHigh`, `ExplosionLow`, `HeavyTraffic` or `HonkLow` on `City`. Calls stay at the same positions relative to messages, rubble creation and other state updates. Reversing only these operand replacements reproduces both entire files from merged `main` exactly. The ship's `-speed 80` and monster's `[MonsterSpeed]` requests remain literal legacy strings; unreachable message audio and empty tool-error commands are not activated.

Recording tests verify one request per effect, high-before-low ordering, initialization before dispatch and unchanged loop/earthquake state. They also retain the inherited ineffective mute behavior and check silent null playback. These tests exercise the actual sound helper, not sprite/tool execution or a simulation digest. Both CMake builds pass 3/3 tests and the retained Visual Studio Release build passes. Commands remain in [BUILDING.md](../BUILDING.md); logs use `out/audit/typed-city-*`.

The migrated effects cease printing `UIMakeSound` diagnostics and remain silent. No audio device, sample, dependency, save change or RNG/map logic is introduced. The shared string helper remained for deferred callers, so this checkpoint left nine live `Eval` expressions.

## Historical checkpoint: earthquake presentation

After PR #4 merged, restart-after-stop characterization was extended and passed against the legacy visual command in Debug and Release (`3bbda3a`). The application now owns a `NullPresentationEvents` beside its audio service and passes both through the inherited no-argument earthquake adapter to `DoEarthQuake(AudioService&, PresentationEvents&)`.

The helper emits the existing low-explosion sound, calls `earthquakeStarted()`, then performs its unchanged `ShakeNow` increment and timer-flag update. The event is synchronous and carries no strength parameter: the legacy increment counter is not simulation strength. Repeated starts and explicit stops retain their state behavior; stop emits no new start event. The renderer still has no shake implementation, and the 3,000 ms timer remains disabled. Actual visual duration/cancellation needs its own application/renderer contract before a real adapter is added.

Recording tests snapshot the audio-request count, console, initialization and shake/timer state at presentation dispatch for first, repeated and restarted earthquakes. They verify one presentation event after its audio request and before state mutations. The actual null adapters preserve the same state while removing the visual string log. `DoEarthQuake` itself still logs. Reversing only the new include, service parameter and event dispatch reproduces all of `w_tk.cpp` from merged `main` exactly; `MakeEarthquake` damage/RNG and the audio helpers are untouched.

Both CMake builds pass 3/3 tests and the retained Visual Studio Release build passes. The header-only interface retains all 47 production translation units and unchanged project files. Commands remain in [BUILDING.md](../BUILDING.md); logs use `out/audit/typed-presentation-*`. This verifies helper routing/state with a null visual adapter, not real shake, automatic expiry, renderer teardown or deterministic simulation parity. Live `Eval` expressions decrease from nine to eight.
