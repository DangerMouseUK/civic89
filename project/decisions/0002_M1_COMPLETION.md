# ADR 0002: Complete Milestone 1

**Status:** Merged in [PR #6](https://github.com/DangerMouseUK/civic89/pull/6), `de424f3`; [hosted Debug/Release CI](https://github.com/DangerMouseUK/civic89/actions/runs/37432056603) passed 7/7 checks each. Remaining statements record the M1 checkpoint.
**Date:** 6 October 2026

The user now requires complete milestones per branch/PR. M1 includes the remaining typed presentation/audio routes, native scenario selection/load/start, all eight fixture checks, and deletion of the command bridge. M2 engine extraction and M3 audio/rendering backends retain their roadmap scope.

Compatibility decisions:

- Keep audio silent through the existing null adapter. Retire unexecuted script speed modifiers without inventing a multiplier: a distinct `ShipHorn` sound identifies the ship variant and `Monster` identifies the roar. A future audio backend must choose licensed samples and playback policy explicitly. Keep the unreachable notification audio branch unreachable, preserving RNG consumption and existing sound initialization/save behavior.
- Replace empty tool-error commands with typed `MustBulldoze` / `InsufficientFunds` requests on `Construction`. This recovers the intended request without introducing playback or changing tool results/costs.
- Deliver dashboard messages through the application's presentation adapter. Keep auto-goto, generation and earthquake visuals as no-ops while emitting typed observations. `generationStarted()` retains the inherited position before `GenerateMap`; it never claims completed generation. Earthquake timing remains disabled.
- Remove unused picture and options command wrappers. Neither has a functioning caller.
- Restore native scenario startup using the exact bundled Windows format: seven 120-element signed 32-bit history arrays and a 120-by-100 signed 32-bit map, 51,360 bytes total. Validate size, complete read and tile range before changing the session. Do not reinterpret `scenarios_old/`, rewrite `.cty` output, or claim historical 27,120-byte compatibility.
- Failed or invalid scenario starts preserve map, funds, properties, time and lifecycle counters, and emit no started event. Successful starts retain the existing metadata and simulation initialization/disaster/scoring deadlines. Selection uses the existing `Scenario` enum with an explicit mapping to legacy IDs 1-8.
- The inherited loader called `initWillStuff()` after reading the file, clearing all restored histories (including replacing the fixture's zero money history with 128). Reset arrays before restoring validated data. Fixture/history tests cover this startup repair; catalog difficulty is applied before simulation initialization.
- Emit typed won and lost outcomes after clearing the old message. Preserve all score thresholds; correct the missing win notification as a presentation-only lifecycle repair. The application displays the outcome on the dashboard and keeps play running.

Baseline evidence: the native lifecycle test compiles the same 47 application sources with an alternate test entry path and SDL dummy video/software rendering. It characterizes metadata, disaster/scoring counters and all eight native loads. It also reproduces missing-file startup replacing the session with a blank map. There are no source moves, new production translation units or dependencies.

Debug and Release pass 7/7 CTests; retained Visual Studio Release builds. The same 47-source native lifecycle test validates pinned fixture bytes/decoding, all initialized scenario deadlines, 64 simulation frames per scenario, rejection/recovery and session preservation, message/focus/expiry, generation timing, score boundaries and actual app startup. A source gate prevents bridge/string-audio reintroduction. Existing simulation/disaster/RNG code and city save/load functions are unchanged; reversing only typed sprite/tool/generation replacements exactly reproduces merged main.

F6 opens a native scenario selector; startup accepts `--scenario 1..8` with strict validation. Selection success clears the old save filename; failure/cancel preserves the session. Win/loss appears on the dashboard without stopping play. No production dependency or translation unit is added. Tests use dummy/software SDL and an alternate entry path rather than extracting the engine early.

Interactive verification was attempted using the computer-use skill, but monitor capture and activation failed (`0x80070057` / `0x80070005`). The task-owned process was closed; no screenshot, selector layout/DPI or normal interactive exit success is claimed. Actual audio, focus/shake timing, historical city-format interoperability, deterministic whole-simulation parity and resource lifetime remain later milestones. Commands/paths and detailed evidence are in [BUILDING.md](../BUILDING.md) and [M1_2026-10-06.md](../../tests/baseline/M1_2026-10-06.md).
