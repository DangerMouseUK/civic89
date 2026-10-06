# ADR 0003: Headless engine and deterministic parity

**Status:** Implemented for the complete M2 branch; see project status for publication/CI.
**Date:** 6 October 2026

M2 extracts the inherited simulation into `civic89_engine`, used by both the
native application and `civic89_runner`. The library uses only C++20 and the
standard library. Its sources and transitive local headers do not import SDL,
Win32, UI, textures or JSON asset loading. The separate headless presets do not
configure vcpkg, SDL, resource compilation or runtime asset staging.

The dependency inventory is [ENGINE_DEPENDENCIES.md](../reference/ENGINE_DEPENDENCIES.md).
Original source paths remain in place. Renderer functions, SDL rectangle helpers
and the JSON-loading tool constructor are split into application files. Engine
initialization/options and service adapters move out of application main. The
retained Visual Studio project includes those splits and still builds.

Compatibility decisions:

- Preserve simulation phase ordering, algorithms, map layout, costs, deadlines,
  score thresholds and random distributions. Preserve the inherited exclusive
  final row/column coordinate checks. Do not reseed normal application play.
- `seedSimulationRandom()` is an explicit opt-in control for tools/tests. It
  seeds the existing `mt19937` without replacing the generator or distributions.
  Seed before loading or generating: initialization scans also consume RNG.
  The inherited `GenerateCityFromSeed` parameter remains ineffective; the new
  explicit RNG control supplies reproducibility without changing that algorithm.
- A runner tick is one inherited `SimFrame` phase, followed by date/message and
  evaluation refresh. Sixteen phases advance the city clock once. Sprite/tile
  animation remains a separate API and GUI schedule; a headless phase run is not
  claimed to reproduce wall-clock GUI animation timing. `--speed` optionally
  overrides loaded speed; otherwise a city's saved pause/speed is respected.
- Sprite state contains frame counts, not textures. Preserve all seven counts
  and movement/update functions. Application textures load on first draw and
  live in a renderer-owned cache released before renderer destruction. This
  narrow ownership split does not complete M3's wider resource-lifetime work.
- Keep silent default audio/presentation adapters. The app injects its services
  and SDL millisecond clock; headless message timing uses a steady clock.
  Budget/tool-reset callbacks are typed events. No command bridge is added.
- Reuse M1's validated 51,360-byte array decoder for city reads. Reject incomplete,
  oversized, invalid-tile, invalid-difficulty, invalid-speed and out-of-range
  funding-percentage data before mutation. This deliberately removes inherited
  partial-read acceptance and unsafe speed-table indices. Tests verify failure
  isolation. Valid city-load initialization order and the writer are preserved,
  including the inherited history reset after city loading. Full state restore,
  historical 27,120-byte import and safe/atomic saving remain M3 work. No bytes or
  fields are added to `.cty` output.
- Retain inherited process-global, single-threaded state. This milestone does
  not provide simultaneous independent engine instances. Golden cases run in
  fresh processes, matching baseline startup rather than assuming a full reset
  of every hidden legacy static variable.

Parity references were captured **before extraction** from merged M1 `de424f3`,
with only additive seed/digest/probe instrumentation. All eight packaged cities
use seed 12345 at 0, 16 and 1,024 phases; Detroit also uses 16,384 phases. Debug
and Release references match except Bern at 1,024 phases under inherited
Release `/fp:fast`. Preserve both values rather than changing floating-point
mode or replacing references with extracted-engine output. The fixture is tied
to MSVC 19.51's inherited RNG/distribution and floating-point behavior; portability
across compilers is not established.

Digest v1 hashes little-endian 32-bit words: map tiles, fourteen history arrays,
twelve effect maps, city/scenario/census aggregates, RCI, budget/funding bits,
evaluation and numeric sprite state in type order. It excludes addresses,
textures, names, wall-clock timing and presentation. It is a regression digest,
not a complete resumable state serialization or a cryptographic integrity check.

Validation and exact commands are in [M2_2026-10-06.md](../../tests/baseline/M2_2026-10-06.md).
New code uses existing test infrastructure and adds no production dependency.
