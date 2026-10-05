# Legacy Eval inventory

Audited 5 October 2026 against merged bootstrap `b6fc77e561bb3bbcc60c75603b942e755f4992a9`, with inherited C++ still identical to `upstream-sdlpp-baseline` (`9c4e85a0decd57ba6f76d9e1ec82461940ecc3ad`). This completes the documented inventory for M1-01; it does not complete scenario startup or remove the bridge.

[`Eval()`](../../src/w_tk.cpp) prints `Eval: <command>` to `std::cout` and always returns `false`. There is no interpreter, command dispatch or consumer of its return value. Count: **12 live call expressions**, **one commented-out call**, one definition and one declaration. A live expression inside an otherwise uncalled function is distinguished below from a reachable game path.

## Calls and intended replacements

Replacement names refer to the [proposed typed boundary](../decisions/0001_TYPED_PRESENTATION_BOUNDARY.md), not existing implementations.

| Command actually emitted | Call site | Reachability and current effect | Intended effect / replacement |
|---|---|---|---|
| `UIStartScenario <integer>` | [main.cpp](../../src/main.cpp), `doStartScenario` | Only reached through the scenario arm of `primeGame`; both current callers pass `-1` (new city). Logs only; does not call `LoadScenario`. | Application `ScenarioController::start(Scenario)`, followed by a typed start notification after successful loading. |
| `UIDidGenerateNewCity` | [s_gen.cpp](../../src/s_gen.cpp), `GenerateCityFromSeed` | Reachable during new city generation. Logs **before** `GenerateMap(seed)`, after simulation initialization. | Typed generation notification; decide and test its timing before naming it a completed-generation event. |
| `UIAutoGoto <x> <y>` | [s_msg.cpp](../../src/s_msg.cpp), `DoAutoGoto` | Reachable from `doMessage`. First updates the actual dashboard message through `SetMessageField`; the command never moves the camera. | `PresentationEvents::focusMap(MapPosition)`; retain the existing message update independently. |
| `UIShowPicture <id>` | [s_msg.cpp](../../src/s_msg.cpp), `DoShowPicture` | Function contains a live call; its only caller is inside the commented-out picture-message block in `doMessage`. Logs if called directly. | Explicit typed notification/picture presentation if retained; otherwise remove the dormant wrapper after confirming no use. |
| `UILoseGame` | [s_msg.cpp](../../src/s_msg.cpp), `DoLoseGame` | `DoScenarioScore` calls it on loss. No scenario launcher currently reaches normal scenario play. Logs only. | `PresentationEvents::scenarioFinished(ScenarioOutcome::Lost)`. |
| `UIWinGame` | [s_msg.cpp](../../src/s_msg.cpp), `DoWinGame` | No caller. A passing `DoScenarioScore` does **not** call this wrapper. | Typed won outcome after an explicit, tested lifecycle correction; do not infer that changing only this wrapper fixes scenario wins. |
| `UIMakeSound "<channel>" "<id>"` | [w_sound.cpp](../../src/w_sound.cpp), `MakeSound` | Reachable from earthquake, sprites and bulldozing; lazily sets sound initialization and logs. Message-audio callers are discussed below. No samples/devices are used. | `AudioService::playEffect(SoundId, AudioChannel, PlaybackRate)`. |
| Empty string | [w_sound.cpp](../../src/w_sound.cpp), `MakeSoundOn` | Reachable from `ToolActions::executeTool` for `RequiresBulldozing` / `InsufficientFunds`. Its formatting is commented out, so both operands are lost and output is `Eval: ` alone. | Typed UI/construction error effects, subject to an explicit behavior correction. The intended commented command was `UIMakeSoundOn <window> "<channel>" "<id>"`; no window string belongs in the replacement. |
| `UISoundOff` | [w_sound.cpp](../../src/w_sound.cpp), `SoundOff` | No game caller. Lazily initializes, logs, and sets `Dozing = 0`; does not mute later effects. | `AudioService::stopAll()`; persistent mute belongs in explicit audio settings, not initialization state. |
| `UIStartSound <channel> <id>` | [w_sound.cpp](../../src/w_sound.cpp), `DoStartSound` | Called by `StartBulldozer`, which has no game caller. Repeated starts emit only once while `Dozing` is set. | `AudioService::startLoop(SoundId::Bulldozer, AudioChannel::Construction)`. |
| `UIStopSound <id>` | [w_sound.cpp](../../src/w_sound.cpp), `DoStopSound` | Called by dormant `StopBulldozer`; repeated stops still emit requests when initialized. | `AudioService::stopLoop(AudioChannel::Construction)`; the only loop ID in this baseline is `1`. |
| `UIEarthQuake` | [w_tk.cpp](../../src/w_tk.cpp), `DoEarthQuake` | Reachable through `MakeEarthquake`. Emits explosion sound first, logs the visual request, increments `ShakeNow` and sets the timer flag. Timer creation/cancellation is commented out, and no renderer reads `ShakeNow`. | `PresentationEvents::earthquakeStarted()` with application-owned visual timing; keep simulation damage/RNG separate. |

The thirteenth search result is `Eval(buf)` in [`w_update.cpp`](../../src/w_update.cpp), `UpdateOptionsMenu`. Its whole body is commented out, including formatting `UISetOptions` with eight option-bit booleans. There is no caller or runtime options command to migrate. Native options already access `GameOptions`; do not revive this command merely to replace it.

## Audio operands and state

| Legacy operand | Sources | Proposed typed sound |
|---|---|---|
| `Explosion-Low` / `Explosion-High` | `w_tk.cpp`, `Sprite.cpp`, `ToolActions.cpp`, dormant message-audio cases | `ExplosionLow` / `ExplosionHigh` |
| `HeavyTraffic` | `Sprite.cpp`, helicopter path | `HeavyTraffic` |
| `HonkHonk-Low`, `HonkHonk-Med`, `HonkHonk-High` | `Sprite.cpp` (low), dormant message-audio cases (all three) | `HonkLow`, `HonkMedium`, `HonkHigh` |
| `HonkHonk-Low -speed 80` | `Sprite.cpp`, ship path | `HonkLow` plus an explicit rate; the legacy unit is unresolved. |
| `Monster -speed [MonsterSpeed]` | `Sprite.cpp`, monster path; dormant message-audio case | `Monster` plus a computed rate after its semantics are established. `MonsterSpeed` has no implementation in this repository. |
| `Siren` | Dormant message-audio cases | `Siren` |
| `UhUh` / `Sorry` | `ToolActions.cpp`, tool failure paths via `MakeSoundOn` | `MustBulldoze` / `InsufficientFunds` |
| `1` | Dormant bulldozer loop wrappers | `Bulldozer` |

Only channels `city` and `edit` are supplied: proposed `City` and `Construction`. The `-speed` operands are currently literal text enclosed inside the quoted ID, not parsed playback options. Do not carry script syntax into the typed API or guess its conversion to a playback multiplier.

Additional inherited behavior relevant to migration:

- `doMessage` initializes its local `firstTime` to `false` and never sets it to `true`; its sound switch is unreachable. Activating those sounds would be a separate behavior change and could introduce new RNG calls, so it must not accompany a mechanical bridge migration.
- `userSoundOn()` and its setter access `SoundInitialized`, while the private `UserSoundOn` remains `true`. Calling `userSoundOn(false)` does not suppress a later `MakeSound`, which reinitializes. `ShutDownSound` does not clear initialization either.
- `FileIo.cpp` persists `userSoundOn()` in `MiscHistory[55]`. Correcting the sound setting also touches save semantics; isolate that decision and test legacy round trips without changing the binary format.
- `w_sound.h` declares `DoStartSound(const char*, const char*)`, but the definition takes `const std::string&` parameters. Only the internal caller currently resolves the defined overload; introducing an external caller through that header would fail to link.
- No inventoried sound files or audio backend implement these requests. A null audio adapter preserves the current silent result while licensed assets and SDL3_mixer remain a later milestone.

## Scenario baseline

There is no native scenario-selection UI, and [`LoadScenario`](../../src/FileIo.cpp) has no caller. `primeGame(-1)` runs at application startup and reset. Passing a positive number to `doStartScenario` would only print the command. Consequently, there is **no successful scenario-start smoke path to claim** at this checkpoint.

The existing data table is authoritative for a later typed controller:

| Existing `Scenario` enumerator | Enum value | Legacy scenario ID | File | Year | Starting funds |
|---|---:|---:|---|---:|---:|
| `Dullsville` | 0 | 1 | `snro.111` | 1900 | 5,000 |
| `SanFransisco` (inherited spelling) | 1 | 2 | `snro.222` | 1906 | 20,000 |
| `Hamburg` | 2 | 3 | `snro.333` | 1944 | 20,000 |
| `Bern` | 3 | 4 | `snro.444` | 1965 | 20,000 |
| `Tokyo` | 4 | 5 | `snro.555` | 1957 | 20,000 |
| `Detroit` | 5 | 6 | `snro.666` | 1972 | 20,000 |
| `Boston` | 6 | 7 | `snro.777` | 2010 | 20,000 |
| `Rio` | 7 | 8 | `snro.888` | 2047 | 20,000 |

Files live under `scenarios/`; table time is `(year - 1900) * 48 + 2`. **Enum ordinals and legacy IDs differ by one**; do not cast the startup integer directly to `Scenario`. The loader sets level/name/funds/time/ID, resets the map, ignores the `loadFile` result, sets normal speed, calls UI-dependent `initWillStuff` and initializes simulation. `loadFile` also does not reject short reads. All eight scenarios need fixture checks and actual start tests before enabling this route or deleting its legacy caller. These defects were found by source inspection, not reproduced scenario play.

## Characterization and migration gates

[`tests/LegacyBridge.cpp`](../../tests/LegacyBridge.cpp) links the **actual** inherited `w_tk.cpp` and `w_sound.cpp`. It supplies only the simulation-owned `ShakeNow` storage and captures standard output. It checks lazy initialization, literal speed operands, empty tool-error requests, the inherited mute/shutdown behavior, bulldozer start/stop state and earthquake request ordering/repeated start/explicit stop. CTest name: `legacy-audio-and-earthquake`. It needs no SDL initialization, window, audio device or asset working directory.

These tests document the current defects; they do not endorse them as permanent compatibility requirements. Change the expectations only with the matching reviewed behavior decision and replacement tests. They do not validate real playback, visual shaking, automatic timer expiry, scenario loading or earthquake simulation damage/RNG.

Before deleting live bridge calls:

1. Introduce typed interfaces and recording/null adapters, then migrate one covered path at a time.
2. Preserve existing message updates and simulation call order, including all RNG calls and map mutations.
3. Add a scenario-loading seam and tests for all eight fixtures, enum-to-ID mapping, lifecycle state and missing/truncated input before routing native startup.
4. Establish a deliberate timing contract for generation and earthquake presentation; verify timer ownership and cancellation without changing the damage algorithm.
5. Replace bridge-output assertions with typed recording-adapter assertions as paths migrate; remove each legacy call only after its new tests pass.

Reproduce the audit with `rg -n '\bEval\s*\(' src`, then inspect surrounding comments and callers. Text search alone counts the inactive options call and does not establish reachability.
