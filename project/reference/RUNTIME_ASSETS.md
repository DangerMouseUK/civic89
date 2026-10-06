# Civic 89 runtime asset baseline

Audited 5 October 2026 against SDLPP `9c4e85a0decd57ba6f76d9e1ec82461940ecc3ad`.

`assets/runtime-assets.json` is the complete staged file inventory, including sizes, SHA-256, immutable source URLs and source references. For text assets, `hash_mode: lf` records the hash/size after CRLF-to-LF normalization so Git's Windows line-ending settings cannot invalidate a fresh checkout. Binary assets use exact bytes. `assets/ASSET-LICENSES.yml` records licence groups and unresolved inherited rights. No asset acquisition runs during configure, build or startup: approved fonts are committed, and a fresh checkout contains the runtime assets.

## Source audit

- `src/Constants.h`, `GameDataLoader.cpp`: `res/strings.json`, `res/tools.json` (months, tool names and construction data).
- `src/main.cpp`: `images/tiles.xpm`.
- `src/UI/InterfaceManager.cpp` constructs every panel at startup. Budget and dashboard use Raleway Medium at 14/12; evaluation/query use Medium, Bold and Bold Italic at 13; the dashboard title uses `res/virtue.ttf` at 12. Mixed case font references resolve on the supported Windows filesystem.
- Budget, dashboard, evaluation, graph, options, query and palette require seven PNG backgrounds. Palette ghost previews require `res`, `com`, `ind`, `fire`, `police`, `stadium`, `seaport`, `coal`, `nuclear`, `airport` XPM files and `icons/buttons.png`.
- `src/UI/MiniMapWindow.cpp`: `images/tilessm.xpm`, `icons/minimap.png`.
- `src/Sprite.cpp::initSprite` constructs 61 XPM paths: train `obj1-0..4`, helicopter `obj2-0..8`, airplane `obj3-0..11`, ship `obj4-0..8`, monster `obj5-0..16`, tornado `obj6-0..2`, explosion `obj7-0..5`. These are required when sprites spawn, even though startup need not spawn them all.
- `src/FileIo.cpp`: eight `scenarios/snro.111..888` paths; user-selected `.cty` files are not startup requirements. The 24 inherited `cities/*.cty` are staged for manual comparison.
- `res/hexa.*`, `res/stri.*`, other XPMs and `obj8-*` are inherited, inactive files with no current loader references; they are retained in the repository and excluded from the development runtime. Sound is stubbed in `w_sound.cpp`/`w_tk.cpp`: there are no current sound/music file loads.
- `micropolis-sdlpp.rc` embeds `micropolis.ico` and `Micropolis.png` at compile time. The inventory records both under `embedded_files` rather than staging them separately. These remain inherited comparison resources; replacing their branding is later work. No settings/INI file is read by the current application.

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

The CMake build stages only inventoried files and retained notices in `out/build/<preset>/bin/<configuration>/`, beside the executable and vcpkg app-local DLLs. It validates SHA-256 before copying. Source assets and local untracked files are never copied wholesale. Updating an asset requires reviewing its rights, running `python tools/update-runtime-inventory.py` and reviewing the inventory change. Developers launch through the `run` target or with that output directory as the working directory. Visual Studio debugger working directory is set explicitly.

`cmake --install` copies this same layout to a chosen portable directory. It is a development staging mechanism, not public-release clearance. A future installer should keep these relative `images/`, `icons/`, `res/`, `scenarios/`, `cities/` and notice paths beside the executable until resource lookup is deliberately refactored. No per-user settings/save layout change is made here.

The inherited `.vcxproj` still runs with the repository as its default working directory. The checked-in fonts resolve its startup failure without changing that project. It can also be launched from the CMake staging directory to compare the exact same asset set.

## Remaining release questions

`icons/LICENSE.txt` says only “Icons by OpenSVG” and links an aggregator. It does not identify individual icon collections or licence terms. Public redistribution of those atlases remains unresolved. The remaining graphics/fixtures preserve their inherited project-level GPL/additional-terms provenance; individual rights should be audited before public binary release. No retail-game or arbitrary matching-name assets were imported.

## M3 procedural audio

The 122-file staged inventory is unchanged. M3 generates twelve original effects
from `src/AudioManager.cpp` and loads their PCM through SDL3_mixer. The effect bank
is completely defined by the GPL source; it needs no packaged WAV/codec content.
Inherited `sounds/*.wav` have no established per-asset licence record in the
inventory and remain unstaged. SDL3_mixer's pinned dependency copyright is staged
with the other dependency notices. The existing icon/per-asset audit gate remains.
