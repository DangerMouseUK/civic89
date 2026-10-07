# M8 gameplay and historical-save audit

6 October 2026. Comparison: fixed SDLPP baseline
`9c4e85a0decd57ba6f76d9e1ec82461940ecc3ad` and merged M7 `eb73f64`.
No M8 simulation algorithm, tool definition, scenario fixture or city payload changes.

## Mechanics evidence

The [source/data manifest](../tests/fixtures/m8-fidelity-source-manifest.txt) locks
47 audited inputs: 14 simulation source/header files, the tool catalogue, eight
scenario files and 24 inherited historical cities. Text hashes normalise CRLF to
LF; binary hashes use exact bytes. `fidelity-source-contract` also works in source
archives without Git. A mismatch requires fidelity review; updating a reference
is not permission to change gameplay.

43 inputs match the fixed upstream baseline. Four contain previously delivered
platform extraction/typed presentation changes, inspected against that baseline:

| File | Difference from SDLPP |
|---|---|
| `src/s_gen.cpp` | Typed generation notification replaces `Eval()` at the same point before map generation. |
| `src/Sprite.cpp` | Drawing/resources move to the renderer; frame counts retain 5/9/17/9/12/3/6. SDL rectangle checking becomes equivalent exclusive bounds. Sound calls use typed IDs. Movement, collision, random calls and disaster actions remain. |
| `src/s_sim.cpp` | Unused SDL include removed. |
| `src/s_alloc.h` | Application header replaced by the extracted engine-state header. |

The audit covers budget, RCI, traffic, power, zones, disasters, generation,
evaluation, scans, sprite logic, phases, constants and animation table. This
complements behavioral tests; it is not an exhaustive proof of the entire retail
game or every inherited bug. No original retail executable/assets were acquired.

`original-mechanics` checks all 16 tools' costs/footprints, affordability and edge
rejection, tax/demand bounds, budget shortfall allocation, road routing and power
connectivity. `runtime-assets` checks the actual JSON loader against the same tool
definitions. All 25 M2 scenario/tool/disaster/generation goldens and four generated
references at 1,024/16,384 phases remain; both save identities and existing
serialization/failure tests remain. UI tests compare digests around presentation.

The inherited exclusive final construction row/column and sprite bounds remain.
Snapshots omit RNG, sprites and scenario objectives/timers; loads recompute scans.
These are compatibility limits, not claims about original retail behavior. An
outcome-changing correction requires separate user direction and evidence under
[ADR 0009](decisions/0009_FAITHFUL_MODERNISATION_AND_GRAPHICS.md).

## Historical 27,120-byte investigation

All 24 inherited `cities/*.cty` files are 27,120 bytes, with unchanged baseline
hashes and inherited licence/notice provenance. No new retail fixtures were added.
Tests verify rejection of every file before live-city mutation.

The pinned [Micropolis reader](https://github.com/SimHacker/micropolis/blob/c98f6b08519887b450d9be198bfca5237aab6d0c/MicropolisCore/src/MicropolisEngine/src/fileio.cpp#L214-L223)
uses big-endian 16-bit entries: six 240-sample histories (2,880 bytes), 120 misc
entries (240 bytes) and a 120 by 100 map (24,000 bytes). Time/funds occupy paired
misc words; funding uses paired 16.16 values. See its
[restoration code](https://github.com/SimHacker/micropolis/blob/c98f6b08519887b450d9be198bfca5237aab6d0c/MicropolisCore/src/MicropolisEngine/src/fileio.cpp#L241-L281).

Current SDLPP/Civic 89 instead uses seven 120-entry 32-bit histories and 12,000
32-bit tiles: 51,360 bytes. History storage, misc meanings and integer percentage
fields differ. Byte swapping or widening shorts would not faithfully restore the
historical annual history.

M8 establishes the mismatch, not a verified conversion. An importer would need an
agreed history/metadata mapping, validation and expected-state fixtures. No decoder
or writer change is made; this variant remains unsupported. Current `.cty`, `.c89`
and supplied 51,360-byte scenarios retain their paths. See
[Classic compatibility](CLASSIC_COMPATIBILITY.md).

## Windows acceptance boundary

[M8 evidence](../tests/baseline/M8_2026-10-06.md) separates automated layouts/native
APIs from visible remote-desktop observations and physical user checks. Hidden 4K
rendering does not establish physical readability, mixed-monitor behavior, audible
sound or assistive technology integration. Missing physical evidence stays pending.
