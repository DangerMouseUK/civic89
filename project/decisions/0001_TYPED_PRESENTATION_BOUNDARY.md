# ADR 0001: Typed presentation boundary

**Status:** Proposed; no production interface or behavior change in this increment.

**Date:** 5 October 2026

**Scope:** M1-01 inventory and the initial design for M1-02/M1-03.

## Context

The [bridge inventory](../reference/LEGACY_EVAL_INVENTORY.md) identifies 12 live `Eval()` calls. The current implementation only logs and returns `false`. Several wrappers are dormant, scenario startup is disconnected from loading, and audio payloads retain script syntax. Replacing strings with a generic string-keyed event bus would preserve the same ambiguity.

Follow the governing [phase-safe refactoring sequence and typed boundary](../engineering/05_ARCHITECTURE_AND_REFACTORING.md): test, introduce an interface, route one path, verify parity, delete the obsolete path, then consider source moves. Engine extraction and functioning audio are later milestones.

## Proposed boundary

Use small synchronous C++ interfaces, called on the existing application/simulation thread. The application owns and injects the adapters; no global string registry, queued event framework, SDL types or window names cross this boundary. Begin with recording adapters for tests and a null audio adapter. Construct adapters before their callers and keep them alive until the game/session stops.

Illustrative signatures, to be introduced incrementally rather than as unused production scaffolding:

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

1. This increment records the inventory and tests the inherited audio/earthquake helper behavior without production edits.
2. Add recording/null adapters and migrate the default earthquake sound request first, preserving sound-before-visual order. Keep visual timing and simulation damage separate. Verify this focused route before broadening migration.
3. Migrate other covered audio operands individually, then navigation/notifications with their own tests. Resolve rate-bearing payloads and dormant wrappers explicitly.
4. Introduce the scenario-loading seam and native controller with all eight scenario fixtures and lifecycle/failure tests. Add selection only after loading works.
5. Delete `Eval` and its obsolete wrappers only when every inventory entry is migrated or deliberately retired and no runtime dependency remains. Keep source layout unchanged throughout this milestone.

The new CTest coverage links inherited `w_sound.cpp`/`w_tk.cpp`; it characterizes routing/state, not real audio, camera shaking or simulation parity. Scenario smoke and deterministic disaster/map digests remain open gates. No new production dependency, source move, save-format change or Classic Mode outcome is proposed in this increment.
