# Licensing, Attribution, Naming and Asset Compliance

**Status:** Active engineering baseline  
**Prepared:** 5 October 2026  
**Repository baseline captured:** 5 October 2026  
**Scope:** **Civic 89** - native Windows modernisation of the open-source Micropolis / original SimCity code lineage

## 1. Scope and disclaimer

This document is an engineering compliance plan, not formal legal advice. Before a commercial or high-profile public release, the final branding, asset rights and distribution method should receive appropriate legal review.

## 2. Code licence baseline

The open-source Micropolis code lineage and SDLPP are distributed under GPL terms, with additional terms referenced by the original EA/Don Hopkins release materials. The inherited `COPYING`, original README/additional-terms text and relevant copyright headers must remain available.

Engineering assumption:

- modifications to GPL-covered code that are distributed must comply with the GPL;
- source corresponding to distributed binaries must be made available in the required manner;
- our repository should make compliance simple by keeping source public/available with release tags and build instructions.

## 3. Trademarks and product naming

### SimCity

Do not use `SimCity` as the product/repository brand in a way that suggests an official EA release. Use the term only where necessary for factual historical description.

### Micropolis

Do not assume that an open-source code licence grants unrestricted trademark/product-name rights. The contemporary Micropolis project includes a separate public-name licence.

### Civic 89 naming baseline

The selected project/product name is **Civic 89**. Use `Civic 89` for human-facing product text and `civic89`/`Civic89` for technical identifiers. SimCity and Micropolis remain factual historical/upstream references only.

A preliminary web collision search found no obvious exact software/game project using the Civic 89 name. That search is useful engineering due diligence but **is not formal trademark clearance**. Before a high-profile public or commercial release, perform a proper trademark/name review in relevant territories and classes.

## 4. Source provenance

Preserve history and add:

```text
project/reference/UPSTREAMS.md
AUTHORS.md
NOTICE.md
```

`UPSTREAMS.md` should record at minimum:

- original Micropolis repository/source lineage;
- Micropolis-SDLPP URL and baseline SHA;
- MicropolisCore URL as reference upstream;
- important selectively ported fixes with source links.

## 5. Copyright headers

Do not indiscriminately replace inherited copyright headers.

For materially new files, use a consistent project copyright header and licence identifier compatible with the GPL distribution model.

For files derived from upstream, retain upstream notices and add ours only where appropriate.

## 6. Third-party dependency licences

Track all direct dependencies and their licences. At release time, generate or maintain a third-party notice bundle covering at least:

- SDL3;
- SDL3_image;
- SDL3_ttf;
- SDL3_mixer;
- nativefiledialog-extended;
- nlohmann-json;
- spdlog;
- Catch2 (development/test dependency, still record for completeness).

Do not assume vcpkg package availability is itself a redistribution licence.

## 7. Asset ledger

Code licensing does not automatically settle every art, sound, font or scenario asset.

Create `assets/ASSET-LICENSES.yml` or `.csv` with one record per asset/group:

```text
id
path/pattern
human-readable name
origin/source URL
original author/owner
licence
redistribution allowed? yes/no/conditional
modification allowed? yes/no/conditional
attribution text
notes
checksum/version
```

No new asset is merged without a complete ledger entry.

## 8. Asset categories requiring special attention

### Fonts

SDLPP references font files. Verify each font's redistribution and modification rights. If uncertain, replace it with a clearly licensed font rather than copying it forward by assumption.

### Sound

The existing audio backend is unfinished, which is an opportunity to start clean. Only add sound effects/music after recording the licence and source.

### Original/classic visual assets

Keep a clear distinction between assets included under the open-source release terms and assets taken from unrelated retail copies or websites. Do not import retail-game assets simply because the code is open source.

### Icons/branding

New launcher icons and product branding should be original or specifically licensed.

## 9. Clean asset workflow

For every asset PR:

1. identify source;
2. confirm redistribution/modification terms;
3. record attribution;
4. record original checksum if useful;
5. add to ledger;
6. review visually/technically;
7. only then merge.

## 10. Release compliance checklist

Before public binary distribution:

- [ ] GPL/inherited licence files included.
- [ ] Corresponding source for the released binary is tagged/available.
- [ ] Build instructions are sufficient to reproduce the binary in principle.
- [ ] Third-party notices included.
- [ ] Asset ledger has no unknown/uncleared entries.
- [ ] **Civic 89** product name/logo formally cleared for intended public/commercial use.
- [ ] No `SimCity` branding in executable name, icon, installer identity or store listing.
- [ ] Any use of `Micropolis` in public-facing branding has been reviewed against the public-name licence.
- [ ] Debug-only proprietary/local assets are not present in the package.
- [ ] Source/binary package contains the correct copyright notices.
