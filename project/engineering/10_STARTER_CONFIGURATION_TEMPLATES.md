# Starter Configuration Templates

**Status:** Active engineering baseline  
**Prepared:** 5 October 2026  
**Repository baseline captured:** 5 October 2026  
**Scope:** **Civic 89** - native Windows modernisation of the open-source Micropolis / original SimCity code lineage

## 1. Purpose

These are **starter templates**, not blindly copyable final files. The coding agent must reconcile actual source lists and installed package target names with the repository it is migrating. The goal is to make the intended structure explicit.

## 2. Civic 89 fixed identifiers

```text
Display name:        Civic 89
Repository:          civic89
Repository root:     C:\Dev\Projects\civic89
vcpkg root:          C:\Dev\vcpkg
CMake project:       Civic89
Executable target:   civic89
Executable filename: civic89.exe
Engine target:       civic89_engine
Tests target:        civic89_tests
```

These identifiers are now project decisions; templates should not revert to placeholders such as `ModernCity` or `moderncity`.

## 3. Top-level CMake skeleton

```cmake
cmake_minimum_required(VERSION 3.28)

project(Civic89
    VERSION 0.1.0
    LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(CIVIC89_BUILD_TESTS "Build Civic 89 tests" ON)
option(CIVIC89_WARNINGS_AS_ERRORS "Treat Civic 89 warnings as errors" OFF)

# Bootstrap phase: add existing SDLPP source tree with minimal movement.
add_subdirectory(src)

if(CIVIC89_BUILD_TESTS)
    include(CTest)
    enable_testing()
    add_subdirectory(tests)
endif()
```

During bootstrap, `src/CMakeLists.txt` should be generated from/reconciled against the existing `.vcxproj` source list. Do not use a recursive `GLOB_RECURSE` as the permanent source-of-truth simply to avoid listing files.

## 4. Intended target structure after extraction

```cmake
add_library(civic89_engine STATIC
    # engine sources
)

target_compile_features(civic89_engine PUBLIC cxx_std_20)

add_executable(civic89
    # application sources
)

find_package(SDL3 CONFIG REQUIRED)
find_package(SDL3_image CONFIG REQUIRED)
find_package(SDL3_ttf CONFIG REQUIRED)
find_package(SDL3_mixer CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

# Native file dialog target naming can differ by packaging/configuration;
# confirm the target exposed by the vcpkg port during bootstrap.
find_package(nfd CONFIG REQUIRED)

target_link_libraries(civic89 PRIVATE
    civic89_engine
    SDL3::SDL3
    SDL3_image::SDL3_image
    SDL3_ttf::SDL3_ttf
    SDL3_mixer::SDL3_mixer
    nlohmann_json::nlohmann_json
    spdlog::spdlog
    nfd
)
```

If the actual `nativefiledialog-extended` vcpkg config exports a namespaced target in the selected baseline, use that target instead. Verify rather than guessing.

## 5. vcpkg manifest template

```json
{
  "name": "civic89",
  "version-string": "0.1.0",
  "builtin-baseline": "<PINNED_VCPKG_COMMIT>",
  "dependencies": [
    "sdl3",
    {
      "name": "sdl3-image",
      "features": ["png"]
    },
    "sdl3-ttf",
    "sdl3-mixer",
    "nativefiledialog-extended",
    "nlohmann-json",
    "catch2",
    "spdlog"
  ]
}
```

If `spdlog` is not yet used, leave it out until the logging layer lands. Dependency manifests should describe reality, not aspirations.

## 6. CMake Presets template

```json
{
  "version": 6,
  "cmakeMinimumRequired": {
    "major": 3,
    "minor": 28,
    "patch": 0
  },
  "configurePresets": [
    {
      "name": "base-windows-x64",
      "hidden": true,
      "generator": "Ninja",
      "binaryDir": "${sourceDir}/out/build/${presetName}",
      "cacheVariables": {
        "CMAKE_TOOLCHAIN_FILE": "$env{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake",
        "VCPKG_TARGET_TRIPLET": "x64-windows"
      }
    },
    {
      "name": "windows-x64-debug",
      "inherits": "base-windows-x64",
      "cacheVariables": {
        "CMAKE_BUILD_TYPE": "Debug",
        "CIVIC89_BUILD_TESTS": "ON"
      }
    },
    {
      "name": "windows-x64-release",
      "inherits": "base-windows-x64",
      "cacheVariables": {
        "CMAKE_BUILD_TYPE": "Release",
        "CIVIC89_BUILD_TESTS": "ON"
      }
    }
  ],
  "buildPresets": [
    {
      "name": "windows-x64-debug",
      "configurePreset": "windows-x64-debug"
    },
    {
      "name": "windows-x64-release",
      "configurePreset": "windows-x64-release"
    }
  ],
  "testPresets": [
    {
      "name": "windows-x64-debug",
      "configurePreset": "windows-x64-debug",
      "output": {"outputOnFailure": true}
    },
    {
      "name": "windows-x64-release",
      "configurePreset": "windows-x64-release",
      "output": {"outputOnFailure": true}
    }
  ]
}
```

## 7. Test target template

```cmake
find_package(Catch2 3 CONFIG REQUIRED)

add_executable(civic89_tests
    smoke/EngineSmokeTests.cpp
)

target_link_libraries(civic89_tests PRIVATE
    civic89_engine
    Catch2::Catch2WithMain
)

include(Catch)
catch_discover_tests(civic89_tests)
```

## 8. GitHub Actions template

```yaml
name: windows-ci

on:
  pull_request:
  push:
    branches: [main]

jobs:
  build-test:
    runs-on: windows-2025-vs2026

    strategy:
      fail-fast: false
      matrix:
        preset: [windows-x64-debug, windows-x64-release]

    steps:
      - name: Checkout
        uses: actions/checkout@v4

      # Preferred final approach: pin/bootstrap the vcpkg revision used by
      # the repository instead of relying silently on whatever happens to
      # be installed on the hosted image.
      - name: Bootstrap vcpkg
        shell: pwsh
        run: |
          git clone https://github.com/microsoft/vcpkg.git "$env:GITHUB_WORKSPACE\\_vcpkg"
          git -C "$env:GITHUB_WORKSPACE\\_vcpkg" checkout <PINNED_VCPKG_COMMIT>
          & "$env:GITHUB_WORKSPACE\\_vcpkg\\bootstrap-vcpkg.bat" -disableMetrics
          "VCPKG_ROOT=$env:GITHUB_WORKSPACE\\_vcpkg" | Out-File -FilePath $env:GITHUB_ENV -Append

      - name: Configure
        shell: pwsh
        run: cmake --preset ${{ matrix.preset }}

      - name: Build
        shell: pwsh
        run: cmake --build --preset ${{ matrix.preset }}

      - name: Test
        shell: pwsh
        run: ctest --preset ${{ matrix.preset }} --output-on-failure
```

A future optimisation can cache vcpkg binary packages, but cache correctness is more important than speed during bootstrap.

## 9. `.editorconfig` starter

```ini
root = true

[*]
charset = utf-8
end_of_line = lf
insert_final_newline = true
trim_trailing_whitespace = true

[*.{cpp,h,hpp,cxx}]
indent_style = space
indent_size = 4

[*.{json,yml,yaml,md}]
indent_style = space
indent_size = 2
```

If the inherited tree currently uses different line endings, do not convert every source file in the bootstrap PR. Apply this deliberately to new files and schedule normalisation separately if desired.

## 10. `.clang-format` starter

Use a conservative style close to the inherited C++ and tune through a dedicated review. Example:

```yaml
BasedOnStyle: Microsoft
IndentWidth: 4
ColumnLimit: 120
SortIncludes: CaseSensitive
DerivePointerAlignment: false
PointerAlignment: Left
```

Do not run it repository-wide on day one.

## 11. `project/reference/UPSTREAMS.md` template

```markdown
# Upstream Sources

## Primary baseline

Micropolis-SDLPP  
Repository: https://github.com/ldicker83/Micropolis-SDLPP  
Engineering audit date: 2026-10-05  
Audit SHA inspected: 9c4e85a0decd57ba6f76d9e1ec82461940ecc3ad  
Actual Civic 89 baseline SHA: `9c4e85a0decd57ba6f76d9e1ec82461940ecc3ad`  
Baseline tag: `upstream-sdlpp-baseline`  
Local preserved remote: `upstream-sdlpp`

This repository preserves the upstream Git history. The original remote is
kept as `upstream-sdlpp` for review of future fixes.

## Reference upstream

MicropolisCore  
Repository: https://github.com/SimHacker/MicropolisCore  
Audited reference SHA: f9ae6a57bbe5f5ff94c149bccb3015757f18241d

MicropolisCore is not merged as an upstream branch. Simulation/file-format
fixes are reproduced, tested and selectively ported with commit/issue
references.
```

## 12. Pull request template

```markdown
## Summary

## Behaviour changed

## Classic simulation impact
- [ ] None
- [ ] Intentional; explained below

## Save-format impact
- [ ] None
- [ ] Intentional; migration/compatibility explained below

## Verification
- [ ] Debug build
- [ ] Release build
- [ ] Automated tests
- [ ] Manual smoke test (if relevant)
- [ ] ASan/high-risk suite (if relevant)

## Upstream provenance
Link any SDLPP/MicropolisCore issue or commit adapted by this change.

## Assets/dependencies
State any new asset/dependency and its licence/source.
```
