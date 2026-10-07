# Enhanced v1 city container

M7 adds `.c89` for `enhanced/1`. Classic v1 keeps the inherited 51,360-byte `.cty`
layout. Enhanced v1 uses the same simulation and 120 by 100 map; the container
provides explicit versioning, name retention and corruption detection. This is an
ordinary-city snapshot, not an exact replay checkpoint. The format remains supported
under the [faithful modernisation decision](decisions/0009_FAITHFUL_MODERNISATION_AND_GRAPHICS.md);
optional graphics are application settings independent of this file and its identity.
Merged M8/M9 and `0.9.0-beta.2` retain this schema unchanged. The public testing
beta policy is [ADR 0011](decisions/0011_BETA_2_SETTINGS_FIXES.md); it does not widen file support.

## Schema 1

All header words are unsigned 32-bit little-endian values. The header is 44 bytes.
It is followed by the city name and then the unchanged Classic v1 payload, with
no padding or trailing data.

| Offset | Field | Accepted value |
|---:|---|---|
| 0 | Eight magic bytes | `CIVIC89` followed by NUL |
| 8 | Container schema | 1 |
| 12 | Ruleset family | 2 (Enhanced) |
| 16 | Ruleset version | 1 |
| 20 | Map width | 120 |
| 24 | Map height | 100 |
| 28 | Name byte count | 1–255 |
| 32 | Payload byte count | 51,360 |
| 36 | Reserved flags | 0 |
| 40 | CRC-32 | Checksum described below |
| 44 | Name | Counted UTF-8 bytes, no terminator |
| 44 + name bytes | City payload | Existing Classic v1 32-bit snapshot |

The checksum uses reflected IEEE CRC-32: polynomial `0xedb88320`, initial value
`0xffffffff`, final bitwise complement. It covers every container byte except
offsets 40–43, which are omitted rather than zeroed. It detects accidental
corruption; it does not authenticate a publisher or replace release signatures.

Names must be valid UTF-8. Empty names, ASCII controls/NUL/DEL, overlong sequences,
surrogates and code points beyond U+10FFFF are rejected. The name limit is bytes,
not characters. Enhanced names survive renaming the containing file. Classic
names continue to derive from the filename; explicit import rejects a name that
cannot be represented in this container before replacing the live city.

## Validation and publication

The reader bounds total input at 44 + 255 + 51,360 bytes before allocating it.
It checks exact length, magic/schema/flags, known exact ruleset, dimensions,
name/payload lengths, checksum and UTF-8. The payload then goes through the same
tile/difficulty/funding/clock/population/demand validation as Classic input.
Only a fully validated candidate is applied to engine state. Unknown versions
do not fall back to Classic or a newer ruleset. Failed inspection/load/import
leaves the city, mode and output candidate unchanged.

Saves use the existing Windows atomic writer: same-directory temporary file,
checked write/flush/close and replacement. Failure preserves the old destination
and current city. Classic and Enhanced keep separate recovery slots, and startup
offers the newest valid ordinary-city recovery with its mode.

## Mode and conversion policy

The typed registry defines `classic/1` (family 1/version 1) and `enhanced/1`
(family 2/version 1). Both currently support Classic import/export. New-city
selection was delivered in M7. M8 removes that interface choice and starts Classic
v1 through F7; existing CLI identities remain compatible.
Scenarios are always Classic v1. Opening `.cty` selects Classic; opening `.c89`
selects its supported recorded Enhanced ruleset.

The Files panel's **Import .cty copy** is explicit, preserves the source file and clears
the normal save destination. **Export .cty copy** writes an ordinary `.cty`
snapshot while preserving the Enhanced mode and normal save destination.
Ordinary Enhanced saves require `.c89`, preventing accidental replacement of a
Classic file. Export cannot restore fields absent from the legacy layout.
Earlier Civic 89 versions cannot load `.c89`; export a `.cty` copy before using
them. Portable rollback preserves both formats without converting either one.

RNG, sprites, scenario objectives/timers and transient state remain unserialized;
load scans can recompute fields. Scenario exports remain ordinary cities and
automatic scenario recovery is disabled. Larger maps, wider values and new mechanics
are outside the agreed scope. M8 adjusts the city-mode interface while retaining
known M7 saves, supported export/recovery and unknown-input rejection. No forced
conversion or file rewrite is required. M9's optional graphics must not change this
schema, city state, ruleset identity, RNG, simulation/animation timing or save bytes.
See [Classic contract](CLASSIC_COMPATIBILITY.md) and
[ADR 0008](decisions/0008_M7_ENHANCED_FOUNDATION.md).
