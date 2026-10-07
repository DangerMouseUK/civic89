# Sources and Reference Links

> Current checkpoint — 7 October 2026: M0–M9 software work is merged.
> This document retains the 5 October planning/audit snapshot and dated updates;
> bootstrap tasks, starter templates, proposed dependencies and future-tense
> migration steps are historical, not a new assignment. Implemented build/targets
> and dependency choices are in [BUILDING.md](../BUILDING.md); current state is
> [PROJECT_STATUS.md](../../PROJECT_STATUS.md). Faithful scope is governed by
> [ADR 0009](../decisions/0009_FAITHFUL_MODERNISATION_AND_GRAPHICS.md), and the
> owner-approved unsigned beta by [ADR 0010](../decisions/0010_PUBLIC_BETA.md).
> Physical desktop acceptance remains pending. Catch2/spdlog were proposals;
> they are not current dependencies. Original results are not retroactive checks.

**Status:** Active engineering baseline  
**Prepared:** 5 October 2026  
**Repository baseline captured:** 5 October 2026  
**Scope:** **Civic 89** - native Windows modernisation of the open-source Micropolis / original SimCity code lineage

## 1. Primary source repositories

- Micropolis-SDLPP: https://github.com/ldicker83/Micropolis-SDLPP
- MicropolisCore: https://github.com/SimHacker/MicropolisCore
- Original Micropolis repository: https://github.com/SimHacker/micropolis

Audited commits used by this engineering set:

- Micropolis-SDLPP: `9c4e85a0decd57ba6f76d9e1ec82461940ecc3ad` - also confirmed as the **actual Civic 89 local baseline** on 5 October 2026.
- MicropolisCore: `f9ae6a57bbe5f5ff94c149bccb3015757f18241d`

Local provenance record:

- repository: `C:\Dev\Projects\civic89`;
- remote: `upstream-sdlpp` -> `https://github.com/ldicker83/Micropolis-SDLPP.git`;
- baseline tag: `upstream-sdlpp-baseline` -> `9c4e85a0decd57ba6f76d9e1ec82461940ecc3ad`.

## 2. Key SDLPP files inspected

- README: https://github.com/ldicker83/Micropolis-SDLPP/blob/main/README.md
- vcpkg manifest: https://github.com/ldicker83/Micropolis-SDLPP/blob/main/vcpkg.json
- Visual Studio project: https://github.com/ldicker83/Micropolis-SDLPP/blob/main/micropolis-cpp.vcxproj
- main application: https://github.com/ldicker83/Micropolis-SDLPP/blob/main/src/main.cpp
- legacy UI bridge: https://github.com/ldicker83/Micropolis-SDLPP/blob/main/src/w_tk.cpp
- sound layer: https://github.com/ldicker83/Micropolis-SDLPP/blob/main/src/w_sound.cpp
- file I/O: https://github.com/ldicker83/Micropolis-SDLPP/blob/main/src/FileIo.cpp
- resource-lifetime issue #18: https://github.com/ldicker83/Micropolis-SDLPP/issues/18

## 3. Key MicropolisCore references

- README: https://github.com/SimHacker/MicropolisCore/blob/main/README.md
- engine file I/O: https://github.com/SimHacker/MicropolisCore/blob/main/packages/micropolis-engine/src/fileio.cpp
- engine header: https://github.com/SimHacker/MicropolisCore/blob/main/packages/micropolis-engine/src/micropolis.h
- WebGL tile renderer: https://github.com/SimHacker/MicropolisCore/blob/main/packages/tile-renderer/src/WebGLTileRenderer.ts
- issue #4, sprites/trains: https://github.com/SimHacker/MicropolisCore/issues/4
- issue #11, sprite lifetime/out-of-bounds: https://github.com/SimHacker/MicropolisCore/issues/11
- issue #13, deterministic seed/reproducibility: https://github.com/SimHacker/MicropolisCore/issues/13
- Micropolis public name licence: https://github.com/SimHacker/MicropolisCore/blob/main/MicropolisPublicNameLicense.md

## 4. Toolchain references verified for this plan

- Visual Studio 2026 release notes: https://learn.microsoft.com/en-us/visualstudio/releases/2026/release-notes
- vcpkg manifest mode: https://learn.microsoft.com/en-gb/vcpkg/concepts/manifest-mode
- vcpkg manifest tutorial/CMake integration: https://learn.microsoft.com/en-us/vcpkg/consume/manifest-mode
- vcpkg CMake integration: https://learn.microsoft.com/en-us/vcpkg/users/buildsystems/cmake-integration/
- GitHub Actions runner images: https://github.com/actions/runner-images
- Windows 2025 / Visual Studio 2026 runner inventory: https://github.com/actions/runner-images/blob/main/images/windows/Windows2025-VS2026-Readme.md
- SDL releases: https://github.com/libsdl-org/SDL/releases
- SDL_image releases: https://github.com/libsdl-org/SDL_image/releases
- SDL_ttf releases: https://github.com/libsdl-org/SDL_ttf/releases
- SDL_mixer repository: https://github.com/libsdl-org/SDL_mixer
- nativefiledialog-extended: https://github.com/btzy/nativefiledialog-extended

## 5. Snapshot notes

As of the planning date:

- Visual Studio 2026 release notes list version 18.10.1 as released 15 September 2026.
- SDL release listings show SDL 3.4.18 released 2 October 2026 with Windows x64 and ARM64 artifacts.
- SDL_image release listings show 3.4.6 released 2 September 2026.
- SDL_ttf release listings show stable 3.2.2.
- SDL_mixer stable release listings show 3.2.4, while the current branch identifies the next development version and requires SDL 3.4.0 or later.
- GitHub runner-image documentation lists `windows-2025-vs2026` for x64 and `windows-11-vs2026-arm` for Windows 11 ARM64 with Visual Studio 2026.

Re-check versions before intentional dependency upgrades. The architecture does not depend on these exact patch versions.


## 7 October 2026 asset investigation and release decision

- [Committed asset evidence](../ASSET_LICENSE_AUDIT.md), including exact upstream commits.
- [OpenSVG project-use terms](https://opensvg.dev/terms-of-service), inspected 7 October;
  current terms dated 5 July 2026 do not identify the imported April icon packs.
- [Pinned author Raleway OFL](https://github.com/impallari/Raleway/blob/6c67ab1f7aa65c442bd2745bb9d4ef1cd7bc01fa/OFL.txt).
- [Owner-approved public beta policy](../decisions/0010_PUBLIC_BETA.md).
