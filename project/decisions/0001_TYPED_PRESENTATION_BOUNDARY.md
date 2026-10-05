# ADR 0001: Typed presentation boundary

**Status:** First audio route implemented; the broader presentation/scenario boundary remains proposed.

**Date:** 5 October 2026

**Scope:** M1-01 inventory, the first M1-02 audio route and the remaining design for M1-02/M1-03.

## Context

The [bridge inventory](../reference/LEGACY_EVAL_INVENTORY.md) identifies 12 live `Eval()` calls. The current implementation only logs and returns `false`. Several wrappers are dormant, scenario startup is disconnected from loading, and audio payloads retain script syntax. Replacing strings with a generic string-keyed event bus would preserve the same ambiguity.

Follow the governing [phase-safe refactoring sequence and typed boundary](../engineering/05_ARCHITECTURE_AND_REFACTORING.md): test, introduce an interface, route one path, verify parity, delete the obsolete path, then consider source moves. Engine extraction and functioning audio are later milestones.

## Proposed boundary

Use small synchronous C++ interfaces, called on the existing application/simulation thread. The application owns and injects the adapters; no global string registry, queued event framework, SDL types or window names cross this boundary. Begin with recording adapters for tests and a null audio adapter. Construct adapters before their callers and keep them alive until the game/session stops.

The following remains the broader illustrative design. The production [AudioService.h](../../src/AudioService.h) currently contains only `SoundId::ExplosionLow`, `AudioChannel::City` and `playEffect(SoundId, AudioChannel)`, plus the null adapter. Rates, additional sounds/channels, loops and presentation/scenario interfaces will be added when their callers migrate:

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

The typed sound overload retains the same private enable guard and lazy `SoundInitialized` update as the string overload. This deliberately preserves the value saved through `userSoundOn()` / `MiscHistory[55]`; mute/shutdown repairs remain separate decisions. The null implementation opens no device, loads no asset and adds no dependency. The only diagnostic difference on this path is removal of `Eval: UIMakeSound "city" "Explosion-Low"`; `DoEarthQuake` and `Eval: UIEarthQuake` still log. Playback stays silent. No other audio operand is migrated.

The recording adapter exists only in `tests/LegacyBridge.cpp`. Tests check the exact typed sound/channel, one request per earthquake, initialization before dispatch, and console/shake/timer snapshots at dispatch to prove it precedes the visual command and state increment. Repeated start and explicit stop retain their prior state assertions. Running the helper with the actual null adapter also verifies lazy initialization and the absence of a string sound command.

This header-only service keeps the existing 47 production translation units and leaves `.sln`/`.vcxproj` unchanged. CMake Debug/Release builds and all three CTests pass. The inherited Visual Studio Release build is checked separately. Commands remain in [BUILDING.md](../BUILDING.md); local logs use `out/audit/typed-audio-*`.

Tests characterize helper routing/state, not real audio, camera shaking or simulation parity. Scenario smoke and deterministic disaster/map digests remain open gates. The disaster RNG/damage code, save implementation and disabled visual timers are unchanged; no source move or new production dependency is introduced.
