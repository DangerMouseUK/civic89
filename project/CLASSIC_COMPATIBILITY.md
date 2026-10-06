# Classic v1 compatibility contract

This is the supported **development engine/data contract**, established from merged
M6 `fa3534b`. It does not declare a stable public release or clear the asset, brand,
signing and physical desktop gates in `packaging/release-gates.json`.

## Rules and execution

`classic/1` preserves the inherited simulation, costs, RCI/economy/disaster rules,
120 by 100 map, eight supplied scenarios and the M2 parity fixtures. The engine
has one process-global city and runs on one thread. A runner tick is one inherited
SimFrame phase; 16 phases advance the clock once. Rendering and animation are
scheduled separately. Normal play retains inherited randomness; deterministic
seeding is explicit in tests/tools. Debug and Release may differ in floating-point
results, so the existing per-configuration goldens remain authoritative.

The supported development platforms are Windows 11 x64 and native ARM64 with the
pinned C++20/MSVC/vcpkg build. Automated native tests do not establish physical
mixed-DPI, audio-device or assistive-technology acceptance.

## City files

- Classic writes the inherited **51,360-byte** little-endian 32-bit `.cty` layout:
  seven histories of 120 integers followed by 120 by 100 tile integers.
- Current writer output, supplied scenario fixtures, Unicode paths, restoration
  of histories/difficulty/funds/options and rejection before mutation are tested.
- Older **27,120-byte** files are unsupported. No compatibility with every original
  retail/Unix/Micropolis city variant is asserted.
- Loads recompute simulation scans. RNG state, sprites, scenario objectives/timers
  and all transient state are not serialized. A save is an ordinary-city snapshot,
  not an exact replay checkpoint. Scenario exports reload as ordinary cities;
  automatic scenario recovery remains disabled.
- Failed parse/import/save/publication must preserve the live city, mode and any
  existing destination. The Windows adapter publishes a flushed temporary file
  in the destination directory. This is not a storage/power-loss guarantee.
- The engine retains its existing integer limits and exclusive construction-edge
  behavior. Larger maps, widened finances/population and changed mechanics require
  an Enhanced ruleset and their own acceptance tests.

## Enhanced boundary

`enhanced/1` is the M7 foundation: it uses the same map and simulation mechanics,
with an explicit identity and a separate versioned `.c89` container. It does not
claim the later gameplay candidates are implemented. New behavior or incompatible
data requires a new supported ruleset version; unknown versions fail before mutation.

Opening a `.cty` selects Classic v1. Importing it into Enhanced requires the explicit
import action and leaves the original file untouched. Ordinary Enhanced saves never
overwrite `.cty`; exporting a Classic copy is explicit and never retags the active
Enhanced city. Enhanced v1 can import/export the Classic payload without changing
its layout or mechanics; existing load scans can recompute fields, so byte-for-byte
replay of the original file is not promised. The snapshot limitations still apply. Later
features must declare whether they preserve that export capability.

Enhanced containers identify their schema, exact ruleset, dimensions and bounded
UTF-8 city name, and checksum the header/metadata/payload. They do not serialize RNG
or claim exact replay. Autosave uses separate Classic and Enhanced slots; recovery
is offered explicitly with its mode. Public publication remains a separate gated action.

## Required verification

Keep all 25 M2 Classic goldens unchanged, plus the M6 generated-city baseline at
1,024 and 16,384 phases. Test both rule identities, mode transitions, Classic byte
layout, Enhanced round trips, explicit import/export, failed/unknown/corrupt inputs,
atomic publication and recovery. Run Debug/Release, both x64 sanitizer configurations,
native ARM64, real package/installer/update acceptance and retained Visual Studio.
