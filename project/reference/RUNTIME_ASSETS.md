# Civic 89 runtime asset baseline

**M6 update:** CMake embeds the original Civic 89 geometric city icon from
`assets/branding/civic89.ico`, with its GPL source SVG and standard-library
generator recorded in the ledger. Inherited embedded resources remain only in
the retained Visual Studio comparison project. Staging now validates embedded
checksums as well as the unchanged 122 runtime assets. Release packages add
version/provenance, complete notices/docs, matching compiler release CRT and a
per-file manifest; see [RELEASING.md](../RELEASING.md). Inherited OpenSVG atlas
pack attribution remains a follow-up. The owner-approved unsigned testing beta
follows [ADR 0010](../decisions/0010_PUBLIC_BETA.md); stable gates remain separate.
The [7 October investigation](../ASSET_LICENSE_AUDIT.md) verifies original-release
art/cities, author-release fonts and source-only material without changing assets.

Audited 5 October 2026 against SDLPP `9c4e85a0decd57ba6f76d9e1ec82461940ecc3ad`.

`assets/runtime-assets.json` is the complete staged file inventory, including sizes, SHA-256, immutable source URLs and source references. For text assets, `hash_mode: lf` records the hash/size after CRLF-to-LF normalization so Git's Windows line-ending settings cannot invalidate a fresh checkout. Binary assets use exact bytes. `assets/ASSET-LICENSES.yml` records licence groups and unresolved inherited rights. No asset acquisition runs during configure, build or startup: approved fonts are committed, and a fresh checkout contains the runtime assets.

## Historical source audit — 5 October 2026

The loader details below describe the inherited UI, retained for build comparison.
The current app instantiates `ModernInterface`, not the legacy panels. M3 audio,
M4 display/camera, M5 UI and M9 graphics settings are implemented; see the current
notes below and [BUILDING.md](../BUILDING.md).

- `src/Constants.h`, `GameDataLoader.cpp`: `res/strings.json`, `res/tools.json` (months, tool names and construction data).
- `src/main.cpp`: `images/tiles.xpm`.
- `src/UI/InterfaceManager.cpp` constructs every panel at startup. Budget and dashboard use Raleway Medium at 14/12; evaluation/query use Medium, Bold and Bold Italic at 13; the dashboard title uses `res/virtue.ttf` at 12. Mixed case font references resolve on the supported Windows filesystem.
- Budget, dashboard, evaluation, graph, options, query and palette require seven PNG backgrounds. Palette ghost previews require `res`, `com`, `ind`, `fire`, `police`, `stadium`, `seaport`, `coal`, `nuclear`, `airport` XPM files and `icons/buttons.png`.
- `src/UI/MiniMapWindow.cpp`: `images/tilessm.xpm`, `icons/minimap.png`.
- `src/Sprite.cpp::initSprite` constructs 61 XPM paths: train `obj1-0..4`, helicopter `obj2-0..8`, airplane `obj3-0..11`, ship `obj4-0..8`, monster `obj5-0..16`, tornado `obj6-0..2`, explosion `obj7-0..5`. These are required when sprites spawn, even though startup need not spawn them all.
- `src/FileIo.cpp`: eight `scenarios/snro.111..888` paths; user-selected `.cty` files are not startup requirements. The 24 inherited `cities/*.cty` are staged for manual comparison.
- `res/hexa.*`, `res/stri.*`, other XPMs and `obj8-*` are inherited, inactive files with no current loader references; they are retained in the repository and excluded from the development runtime. At the audit baseline sound was stubbed; current M3 procedural audio is described below.
- `micropolis-sdlpp.rc` embeds `micropolis.ico` and `Micropolis.png` at compile time. The inventory records both under `embedded_files` rather than staging them separately. These remain comparison-only resources; CMake embeds Civic 89 branding.
  Settings were absent at the audit baseline; the current application persists
  audio, display, camera, UI and graphics preferences separately from city saves.

## Font decision and provenance

The inherited `.gitignore` excluded all TTFs. The missing Raleway file was an upstream packaging defect. Three static Raleway TTFs come from the author's repository at `6c67ab1f7aa65c442bd2745bb9d4ef1cd7bc01fa`, under `fonts/v3.000 Fontlab/TTF/`. They are unmodified:

| Runtime filename | Original filename |
|---|---|
| `Raleway-Medium.ttf` | `Raleway-Medium.ttf` |
| `Raleway-Bold.ttf` | `Raleway-Bold.ttf` |
| `Raleway-BoldItalic.ttf` | `Raleway-Bold-Italic.ttf` |
| `virtue.ttf` | `Raleway-Bold.ttf` (compatibility alias) |

The retained OFL 1.1 and copyright notice permit bundling/redistribution subject to its conditions. Fonts remain under OFL, must not be sold alone, and modified fonts must respect Reserved Font Names. Filename aliases do not modify internal names or font data. The original version of the developer's missing Raleway files cannot be established, so exact prior text metrics are not claimed.

Virtue is attributed to Marty P. Pfeiffer/Scooter Graphics. Its downloadable package includes `readme.htm` with a distribution notification request and `eul.htm` restricting copying/sublicensing and embedding. That package does not establish an unrestricted game redistribution grant. It is excluded. For reproducible comparison, `res/virtue.ttf` contains unmodified OFL Raleway Bold; both inherited and CMake applications see the same substitute. This is an explicit presentation compatibility decision, not a simulation change. Raleway Bold has different glyph shapes, widths and line height from Virtue's Chicago/Charcoal-inspired design. The title remains at 12 points and is centred by measured width; pixel-identical historical title presentation is not promised.

## Runtime layout

The CMake build stages only inventoried files and retained notices in `out/build/<preset>/bin/<configuration>/`, beside the executable and vcpkg app-local DLLs. It validates SHA-256 before copying. Source assets and local untracked files are never copied wholesale. Updating an asset requires reviewing its rights, running `python tools/update-runtime-inventory.py` and reviewing the inventory change. Developers launch through the `run` target or directly; the current CMake app
finds assets beside its executable regardless of working directory. Visual Studio debugger working directory is set explicitly.

`cmake --install` copies this same layout to a chosen portable directory. It is a development staging mechanism, not public-release clearance. The implemented Inno Setup per-user installer preserves these relative asset
paths beside the executable. Preferences/recovery live in `%APPDATA%\Civic89\Civic89`;
user-selected saves stay at their chosen paths and are preserved by updates/uninstall.

The inherited `.vcxproj` still runs with the repository as its default working directory. The checked-in fonts resolve its startup failure without changing that project. It can also be launched from the CMake staging directory to compare the exact same asset set.

## Current asset findings — 7 October 2026

The [investigation](../ASSET_LICENSE_AUDIT.md) verifies exact original-release
matches for 73 XPMs and 24 cities, scenario conversion history, and unmodified
author-release Raleway fonts. Original non-text assets carry inherited GPL/EA
additional terms. OpenSVG's current terms permit project use, but exact source
packs/notices for `icons/buttons.png`, `icons/minimap.png` **and**
`images/DashboardWindow.png` remain unidentified. Two legacy source-only branding
origins also remain unverified. The full source archive retains those files.
The owner requested the public beta with these disclosed follow-ups; this is not
a completed per-icon clearance claim. No retail assets were imported or artwork replaced.

## M3 procedural audio

The 122-file staged inventory is unchanged. M3 generates twelve original effects
from `src/AudioManager.cpp` and loads their PCM through SDL3_mixer. The effect bank
is completely defined by the GPL source; it needs no packaged WAV/codec content.
All 49 inherited `sounds/*.wav` exactly match the original open-source release
and its non-text-assets notice; they remain source-only and unstaged. SDL3_mixer's pinned dependency copyright is staged
with the other dependency notices. Remaining attribution follow-ups and the
scoped beta decision are recorded in the investigation and ADR 0010.
