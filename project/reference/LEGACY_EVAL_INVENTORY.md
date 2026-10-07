# Legacy command inventory - Milestone 1 complete

This is the dated M1 replacement record. M3 subsequently implements procedural
playback and M4 the visible earthquake adapter/timer. The no-op descriptions below
describe M1 acceptance, not the current application. Current delivery is recorded
in [project status](../../PROJECT_STATUS.md).

Initial audit: 5 October 2026 against `b6fc77e`, with inherited C++ identical to `upstream-sdlpp-baseline` (`9c4e85a0decd57ba6f76d9e1ec82461940ecc3ad`). Initial count: 12 live `Eval` expressions, plus one inactive options call. The bridge only printed commands and returned false; no interpreter or return-value consumer existed.

**Current state:** zero `Eval` declarations, definitions or calls in `src/`; no string sound overload or `MakeSoundOn` remains. M1-01 through M1-04 are complete. The following table accounts for every former live path and the inactive options command.

| Former command | Source / replacement | Behavior and evidence |
|---|---|---|
| `UIMakeSound` | `w_sound.cpp`; typed `MakeSound(SoundId, AudioChannel[, AudioService&])` | Lazy initialization, ordering and the saved sound flag are retained; null playback stays silent. Sprite/tool caller source comparisons preserve RNG/map/cost logic. |
| Empty tool-error command | `ToolActions.cpp`; `MustBulldoze` / `InsufficientFunds` on `Construction` | Restores the intended typed request, without a window string or playback. Tool errors still update the existing messages first. |
| `UISoundOff` | `SoundOff(AudioService&)` -> `stopAll()` | Initializes before dispatch, clears `Dozing` afterwards; existing ineffective mute semantics stay unchanged. |
| `UIStartSound edit 1` | `StartBulldozer(AudioService&)` -> `startLoop(Bulldozer, Construction)` | Original enable/initialization guards and repeated-start suppression are covered. |
| `UIStopSound 1` | `StopBulldozer(AudioService&)` -> `stopLoop(Construction)` | Cold stop returns without clearing `Dozing`; initialized repeated stops dispatch before clearing it. |
| `UIEarthQuake` | `DoEarthQuake(AudioService&, PresentationEvents&)` -> `earthquakeStarted()` | Sound -> event -> shake/timer state order; repeated start/stop/restart tests. Visual adapter remains a no-op and the old timer remains disabled. |
| `UIStartScenario` | `ScenarioController::start(Scenario)` -> validated `LoadScenario` -> `scenarioStarted()` | All eight fixtures load and advance with native simulation code. Invalid enum/file inputs preserve the session and emit no start event. F6 and `--scenario 1..8` use this controller. |
| `UIDidGenerateNewCity` | `s_gen.cpp`; `notifyGenerationStarted()` -> `generationStarted()` | Keeps dispatch before `GenerateMap`. Test observes reset state before generation; complete source comparison confirms no generation/RNG algorithm changes. |
| `UIAutoGoto` | `s_msg.cpp`; `focusMap(Point<int>)` | Dashboard message precedes focus request; coordinates clear after dispatch. Production focus stays a no-op. Enabled/disabled/duplicate/expiry tests cover the message flow. |
| `UIShowPicture` | Unused `DoShowPicture` and inactive caller block removed | No functioning caller or native picture feature existed. No picture-message route is activated. |
| `UILoseGame` | `DoScenarioScore` -> `scenarioFinished(Lost)` | Existing scoring predicates retained, message cleared first, outcome shown on the dashboard. |
| `UIWinGame` | Uncalled wrapper removed; `DoScenarioScore` -> `scenarioFinished(Won)` | Corrects the missing winning notification; all eight threshold boundaries and actual win/loss dispatch are tested. |
| Inactive `UISetOptions` | Empty, uncalled `UpdateOptionsMenu` removed | Native `GameOptions` continues unchanged. |

## Sound identities and dormant paths

`ExplosionLow`, `ExplosionHigh`, `HeavyTraffic`, `HonkLow`, `Bulldozer`, `ShipHorn`, `Monster`, `HonkMedium`, `HonkHigh`, `Siren`, `MustBulldoze` and `InsufficientFunds` are typed IDs. Channels remain `City` and `Construction`; the application owns the null audio adapter.

The old `-speed 80` and `[MonsterSpeed]` texts were never evaluated: they were characters inside a quoted sound ID. M1 deliberately retires those script modifiers without inventing native rate units. `ShipHorn` preserves the distinct ship request and `Monster` the roar; the future audio backend must define sample/rate policy with evidence. No parser or replacement string-command API exists.

Notification sounds remain inside the inherited unreachable `firstTime == false` branch. Their calls now use typed IDs, but no extra RNG calls or sounds are activated. Tests confirm traffic/monster messages leave an uninitialized sound service uninitialized. The `userSoundOn` getter/setter still addresses initialization rather than the private mute flag; `MiscHistory[55]` and `.cty` writing remain unchanged. Fixing sound settings and providing licensed playback belong to later milestones.

## Native scenario contract

| Existing enum | Legacy ID | Fixture | Year | Funds |
|---|---:|---|---:|---:|
| `Dullsville` | 1 | `snro.111` | 1900 | 5,000 |
| `SanFransisco` (inherited spelling) | 2 | `snro.222` | 1906 | 20,000 |
| `Hamburg` | 3 | `snro.333` | 1944 | 20,000 |
| `Bern` | 4 | `snro.444` | 1965 | 20,000 |
| `Tokyo` | 5 | `snro.555` | 1957 | 20,000 |
| `Detroit` | 6 | `snro.666` | 1972 | 20,000 |
| `Boston` | 7 | `snro.777` | 2010 | 20,000 |
| `Rio` | 8 | `snro.888` | 2047 | 20,000 |

The `Scenario` enum remains numbered 0-7; explicit catalog mapping handles legacy IDs 1-8. Time remains `(year - 1900) * 48 + 2`. The catalog supplies difficulty 0, name, funds, time and ID. Actual simulation initialization supplies the unchanged disaster/scoring deadlines.

Bundled `scenarios/` files are 51,360-byte little-endian 32-bit Windows data: seven history arrays of 120 words and 12,000 x-major map words. Validation requires exact size, complete read, valid tile/flag range and valid serialized difficulty before any session mutation. All eight byte digests are pinned in native tests. `scenarios_old/` and historical 27,120-byte cities are not reinterpreted; the inherited city reader/writer is unchanged.

The old loader ignored reads and reset histories after loading. Baseline commit `1f49500` reproduces missing-file startup overwriting the session with a blank map. The new route validates first and resets arrays before restoring histories. Tests cover missing, empty, truncated, oversized, old-format and corrupt input, invalid selections, state preservation and recovery after failure. Success cancels stale earthquake state, initializes, then emits one start event. The application clears the old save filename only after success.

## Verification and limits

`tests/LegacyBridge.cpp` exercises actual audio/earthquake helper code with recording/null adapters. `tests/MilestoneLifecycle.cpp` compiles the same 47 application sources with an alternate test entry path under SDL dummy video/software rendering. It checks all eight scenario fixtures/startups, 64 simulation frames per scenario, message/focus/expiry order, generation timing, scoring and the real application startup adapter.

The source gate `cmake/VerifyNoLegacyCommands.cmake` rejects any reintroduced `Eval` expression or string sound call. Debug and Release pass all seven CTests, and the retained Visual Studio Release build passes. Exact commands/paths are in [BUILDING.md](../BUILDING.md); [milestone evidence](../../tests/baseline/M1_2026-10-06.md) records results.

These tests do not establish deterministic whole-simulation parity, historical save compatibility, resource leak freedom, audible playback, actual camera/earthquake visuals or interactive selector layout/DPI behavior. The attempted interactive capture failed on this desktop; no visual smoke success is claimed. [ADR 0002](../decisions/0002_M1_COMPLETION.md) records the deliberate compatibility decisions and remaining milestone boundaries.
