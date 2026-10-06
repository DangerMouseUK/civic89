# ADR 0001: Typed presentation boundary

**Status:** Default city effects, earthquake audio and audio controls implemented; the broader presentation/scenario boundary remains proposed.

**Date:** 5 October 2026

**Scope:** M1-01 inventory, the first M1-02 audio routes and the remaining design for M1-02/M1-03.

## Context

The [bridge inventory](../reference/LEGACY_EVAL_INVENTORY.md) initially identified 12 live `Eval()` calls; 9 remain after retiring the audio controls. The bridge only logs and returns `false`. Several wrappers are dormant, scenario startup is disconnected from loading, and audio payloads retain script syntax. Replacing strings with a generic string-keyed event bus would preserve the same ambiguity.

Follow the governing [phase-safe refactoring sequence and typed boundary](../engineering/05_ARCHITECTURE_AND_REFACTORING.md): test, introduce an interface, route one path, verify parity, delete the obsolete path, then consider source moves. Engine extraction and functioning audio are later milestones.

## Proposed boundary

Use small synchronous C++ interfaces, called on the existing application/simulation thread. The application owns and injects the adapters; no global string registry, queued event framework, SDL types or window names cross this boundary. Begin with recording adapters for tests and a null audio adapter. Construct adapters before their callers and keep them alive until the game/session stops.

The following remains the broader illustrative design. The production [AudioService.h](../../src/AudioService.h) currently contains `ExplosionLow`, `Bulldozer`, `ExplosionHigh`, `HeavyTraffic` and `HonkLow`, `City` / `Construction`, and `playEffect`, `startLoop`, `stopLoop` and `stopAll`, plus the null adapter. Rates, additional sounds and presentation/scenario interfaces will be added when their callers migrate:

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

`ScenarioController` is an application command: it validates and loads, then notifies presentation on success. `scenarioStarted` is an observation and must not trigger loading recursively. Preserve the existing `Scenario` enumerators and explicitly map them to legacy IDs 1-8; the enum itself is numbered 0-7. The future implementation needs a loader seam with fixture tests and failure reporting before a selector is exposed.

Keep dashboard message delivery through its existing direct callback initially. Add a typed message value only when migrating that boundary, using the existing `NotificationId` plus optional map coordinates. Decide whether dormant picture notifications are needed before adding an API for them. Likewise, do not introduce `cityGenerated` until a test establishes whether it means initialized state or a fully generated map: the inherited command runs before map generation.

Loop stop can use a channel because the baseline has a single construction loop (`1`). If future requirements permit multiple simultaneous loops on one channel, introduce typed handles then. Explicitly separate persistent mute/volume settings from device/resource initialization; preserve `MiscHistory[55]` serialization until its compatibility decision has a round-trip test.

`PlaybackRate` describes the future native contract. It is **not** an interpretation of legacy `-speed 80` or `[MonsterSpeed]`; their units/computation remain unresolved. Default effects can migrate first. Keep rate-bearing call sites in the old path until the mapping is supported by evidence and tests. No parser or script evaluator should be added.

## Behavior and compatibility decisions still required

- **Audio:** a null adapter keeps current silent playback. Restoring tool-error sounds, making mute/shutdown work, activating the unreachable notification sound switch and adding licensed sound assets require separate tests/decisions. Activating that switch also changes RNG consumption.
- **Earthquake:** keep `MakeEarthquake` damage and RNG order untouched. The renderer/application should own visual duration and cancellation. The current 3,000 ms timer is disabled and `ShakeNow` is unread by rendering; actual visual shake is new presentation behavior. Do not reinterpret its increment count as simulation strength.
- **Scenario:** validate all eight files and load/start state, including scenario counters/scoring and errors, before using the currently uncalled loader. Missing/truncated input must not be reported as a successful start. Correcting the absent win notification belongs in the lifecycle change, not a mechanical wrapper replacement.
- **Navigation and generation:** activating auto-goto affects presentation; moving the generation notification changes ordering. Preserve dashboard text and engine results and add focused tests for the chosen behavior.

## Migration sequence and evidence

1. Completed the inventory and inherited audio/earthquake characterization without production edits in `8f7d3e1` / `8b4d5c2`.
2. Implemented the typed earthquake sound request and recording/null adapters, preserving sound-before-visual order. Visual timing and simulation damage remain separate; evidence is recorded below.
3. Migrate other covered audio operands individually, then navigation/notifications with their own tests. Resolve rate-bearing payloads and dormant wrappers explicitly.
4. Introduce the scenario-loading seam and native controller with all eight scenario fixtures and lifecycle/failure tests. Add selection only after loading works.
5. Delete `Eval` and its obsolete wrappers only when every inventory entry is migrated or deliberately retired and no runtime dependency remains. Keep source layout unchanged throughout this milestone.

## First route: implementation and compatibility record

The application owns a `NullAudioService` beside its other existing services. The inherited no-argument `DoEarthQuake()` entry point is now a one-line application adapter that passes this service to `DoEarthQuake(AudioService&)`. `MakeEarthquake` and its callers remain untouched. The helper sends `ExplosionLow` on `City` through the typed `MakeSound` overload, then retains the legacy visual command and shake/timer mutations in their original order.

The typed sound overload retains the same private enable guard and lazy `SoundInitialized` update as the string overload. This deliberately preserves the value saved through `userSoundOn()` / `MiscHistory[55]`; mute/shutdown repairs remain separate decisions. The null implementation opens no device, loads no asset and adds no dependency. The only diagnostic difference on this path is removal of `Eval: UIMakeSound "city" "Explosion-Low"`; `DoEarthQuake` and `Eval: UIEarthQuake` still log. Playback stays silent. Other effects remained on the string path at this first checkpoint; subsequent loop/default-effect migrations are recorded below.

The recording adapter exists only in `tests/LegacyBridge.cpp`. Tests check the exact typed sound/channel, one request per earthquake, initialization before dispatch, and console/shake/timer snapshots at dispatch to prove it precedes the visual command and state increment. Repeated start and explicit stop retain their prior state assertions. Running the helper with the actual null adapter also verifies lazy initialization and the absence of a string sound command.

This header-only service keeps the existing 47 production translation units and leaves `.sln`/`.vcxproj` unchanged. CMake Debug/Release builds and all three CTests pass. The inherited Visual Studio Release build is checked separately. Commands remain in [BUILDING.md](../BUILDING.md); local logs use `out/audit/typed-audio-*`.

Tests characterize helper routing/state, not real audio, camera shaking or simulation parity. Scenario smoke and deterministic disaster/map digests remain open gates. The disaster RNG/damage code, save implementation and disabled visual timers are unchanged; no source move or new production dependency is introduced.

## Audio controls: implementation and compatibility record

Before migration, cold `SoundOff` and the stop guard were characterized against the actual legacy code in both configurations (`f25acee`). The guard test starts a loop, clears initialization through the existing sound setter, then verifies that `StopBulldozer` leaves `Dozing` set and that `SoundOff` reinitializes and clears it. This records an inherited behavior rather than repairing it during migration.

The application retains its no-argument `StartBulldozer`, `StopBulldozer` and `SoundOff` entry points as adapters to overloads that accept `AudioService&`. `StartBulldozer` requests `startLoop(Bulldozer, Construction)` before setting `Dozing`; repeat starts remain suppressed. `StopBulldozer` requests `stopLoop(Construction)` before clearing `Dozing`; repeated initialized stops still dispatch, while cold/disabled guards remain. `SoundOff` lazily initializes, requests `stopAll`, then clears `Dozing` without changing persistent mute behavior.

The old internal `DoStartSound` / `DoStopSound` functions and declarations have no remaining callers and are removed. Along with `UISoundOff`, this retires three live `Eval` calls. It also retires the old `DoStartSound` declaration/definition mismatch. No strings, loop-number parsing or registry replace these commands; the baseline's sole loop (`1` on `edit`) is represented by typed `Bulldozer` and `Construction`.

Recording tests assert request kinds, sound/channel, initialization and loop state at dispatch, request order and repeat suppression. The production null adapter is also exercised for cold stop, start, stop and stop-all. The controls still have no game callers, and no new UI route enables them. Expected diagnostic changes are removal of `UISoundOff`, `UIStartSound` and `UIStopSound` logs. No device, sample, save-format or simulation behavior is changed. Verification logs use `out/audit/typed-control-*`; build/test commands remain in [BUILDING.md](../BUILDING.md).

## Default city effects: implementation and compatibility record

After PR #3 merged, the legacy helper was characterized for high/low explosions, traffic reports and low horns (`3985437`); targeted tests passed in Debug and Release before production edits. The application now provides `MakeSound(SoundId, AudioChannel)` as an adapter to the existing explicit-service overload, using its owned null service. This follows the application adapter pattern already used by earthquake and loop controls without a service registry or string conversion.

Four calls in `Sprite.cpp` and eight calls in `ToolActions.cpp` now use `ExplosionHigh`, `ExplosionLow`, `HeavyTraffic` or `HonkLow` on `City`. Calls stay at the same positions relative to messages, rubble creation and other state updates. Reversing only these operand replacements reproduces both entire files from merged `main` exactly. The ship's `-speed 80` and monster's `[MonsterSpeed]` requests remain literal legacy strings; unreachable message audio and empty tool-error commands are not activated.

Recording tests verify one request per effect, high-before-low ordering, initialization before dispatch and unchanged loop/earthquake state. They also retain the inherited ineffective mute behavior and check silent null playback. These tests exercise the actual sound helper, not sprite/tool execution or a simulation digest. Both CMake builds pass 3/3 tests and the retained Visual Studio Release build passes. Commands remain in [BUILDING.md](../BUILDING.md); logs use `out/audit/typed-city-*`.

The migrated effects cease printing `UIMakeSound` diagnostics and remain silent. No audio device, sample, dependency, save change or RNG/map logic is introduced. The shared string helper remains for deferred callers, so the count stays at nine live `Eval` expressions.
