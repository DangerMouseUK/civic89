# Civic 89 Windows bootstrap build

Use Windows x64, Visual Studio 2026 C++ tools and CMake **4.2+**. The committed presets reproduce the inherited application using C++20/MSVC and explicit vcpkg manifest mode. Visual Studio can open the repository folder and use these presets. The primary workstation's bundled CMake is 4.3.1-msvc1; it selects the installed VS 2026 BuildTools instance without needing a developer shell.

## PowerShell setup

From `C:\Dev\Projects\civic89`:

```powershell
$env:VCPKG_ROOT = 'C:\Dev\vcpkg'
# In an ordinary terminal if CMake/CTest are not already on PATH:
$cmakeBin = 'C:\Program Files\Microsoft Visual Studio\18\Enterprise\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin'
$env:Path = "$cmakeBin;$env:Path"
```

For another machine, use an **external** vcpkg checkout at manifest baseline `19780d9cdf84d0944cf9a318666703b89ab6629c`, run `bootstrap-vcpkg.bat -disableMetrics`, and set `VCPKG_ROOT`. Never put vcpkg inside Civic 89. No `vcpkg integrate install` is required by CMake. Generated projects explicitly set `VcpkgEnabled=false` and `VcpkgEnableManifest=false` to prevent global MSBuild integration injecting paths, libraries or another restore.

## Configure, build and test

```powershell
cmake --preset windows-x64-debug
cmake --build --preset windows-x64-debug -- /m
ctest --preset windows-x64-debug --output-on-failure

cmake --preset windows-x64-release
cmake --build --preset windows-x64-release -- /m
ctest --preset windows-x64-release --output-on-failure
```

Presets use `Visual Studio 18 2026`, x64, separate directories and one configuration per directory. Original direct dependencies at the pinned baseline are SDL3 3.4.18, SDL3_image 3.4.4#1, SDL3_ttf 3.2.2#1, nativefiledialog-extended 1.4.1 and nlohmann-json 3.12.0#2. The local vcpkg executable version string is distinct from its registry commit.

Executable paths:

```text
C:\Dev\Projects\civic89\out\build\windows-x64-debug\bin\Debug\civic89.exe
C:\Dev\Projects\civic89\out\build\windows-x64-release\bin\Release\civic89.exe
```

The build verifies/stages 122 inventoried asset/notice files, retained GPL/additional-terms notices and dependency copyright files. vcpkg app-local deployment stages DLLs beside each executable. Approved fonts are checked in; no font download or system installation runs during build/startup.

## Run and install

The run target supplies the correct working directory:

```powershell
cmake --build --preset windows-x64-debug --target run
cmake --build --preset windows-x64-release --target run
```

Equivalent direct Release run used for the local GUI smoke:

```powershell
Start-Process -FilePath 'C:\Dev\Projects\civic89\out\build\windows-x64-release\bin\Release\civic89.exe' -WorkingDirectory 'C:\Dev\Projects\civic89\out\build\windows-x64-release\bin\Release'
```

Development-only portable staging:

```powershell
cmake --install out/build/windows-x64-release --config Release --prefix out/install/windows-x64-release
```

Installation includes only the application, pinned dependency DLLs, inventoried assets and notices. It excludes user saves, tests, PDBs and arbitrary build-directory files. Run the installed executable from its directory. This does not clear inherited icon rights or historical save compatibility for public release; see `PROJECT_STATUS.md`.

## Inherited comparison

The retained solution/project are **micropolis-sdlpp.sln** and **micropolis-cpp.vcxproj**, without modifications:

```powershell
& 'C:\Program Files\Microsoft Visual Studio\18\Enterprise\MSBuild\Current\Bin\MSBuild.exe' micropolis-sdlpp.sln /m /p:Configuration=Release /p:Platform=x64 /verbosity:minimal /nologo
Start-Process -FilePath 'C:\Dev\Projects\civic89\x64\Release\micropolis-sdlpp.exe' -WorkingDirectory 'C:\Dev\Projects\civic89'
```

Global MSBuild vcpkg integration remains available for that inherited comparison. Checked-in font paths resolve its startup failure. Both applications can also launch from the CMake asset directory to use identical staging.

## Target mapping

| Inherited project | CMake |
|---|---|
| 47 `ClCompile` entries | Explicit `cmake/InheritedSources.cmake` list, directly in `civic89` |
| `micropolis-sdlpp.rc` | Same embedded PNG/icon and notices |
| C++20 / WINDOWS / Unicode | C++20 / WINDOWS / UNICODE / _UNICODE |
| `/W3`, `/sdl`, conformance | `/W4`, `/sdl`, `/permissive-`; warning debt recorded |
| Debug console, Release Windows subsystem | Same configuration-specific subsystems |
| Release `/fp:fast`, whole-program optimization | Same `/fp:fast`, link-time optimization, debug symbols |
| Global auto-link/restore | Explicit vcpkg package targets/toolchain |
| Debug forces Release dependencies | Normal matching Debug CRT/dependency DLLs |
| Repository cwd / no asset staging | Inventoried assets beside executable, explicit debugger/run cwd |
| Unrelated OGG DLL post-build copy | Actual dependency DLL deployment by vcpkg; audio still stubbed |

## Tests and limits

The presets run three CTest checks:

- `runtime-assets-and-data`: `civic89_tests` uses inherited `Font.cpp`/`GameDataLoader.cpp`, a small C++ runner, SDL dummy video and software rendering. It decodes, uploads and renders all 82 required textures, constructs every startup font size and validates months/tool data.
- `inherited-source-list`: compares all application sources with the retained `.vcxproj`.
- `legacy-audio-and-earthquake`: `civic89_legacy_tests` links the actual `w_sound.cpp`/`w_tk.cpp`, checks remaining legacy routing/state and known stub defects, and verifies typed default effects, earthquake audio/presentation and bulldozer/sound-off controls with recording/null adapters. It covers sound-before-presentation-before-state ordering, initialization, inherited mute behavior, stop guards and repeated start/stop/restart. It opens no window or audio device and needs no runtime assets. It does not execute sprite/tool callers, scenario loading, earthquake damage/RNG, actual visual shake/timeout or audible playback; see the [inventory and migration gates](reference/LEGACY_EVAL_INVENTORY.md).

To run just the new characterization check in either configuration:

```powershell
ctest --preset windows-x64-debug -R '^legacy-audio-and-earthquake$' --output-on-failure
ctest --preset windows-x64-release -R '^legacy-audio-and-earthquake$' --output-on-failure
```

`civic89_legacy_tests.exe` is beside the corresponding `civic89.exe` in the paths above. New audio/test code compiles without warnings; recompiling inherited `w_sound.cpp` for this target repeats two existing C4100 warnings. Typed default effects, earthquake audio and loop/stop controls use a header-only interface and application-owned null adapter; both build paths still compile the same 47 translation units. Default effects reuse configured presets; configure commands remain above. Caller verification restores the 12 mapped sound operands and compares `Sprite.cpp` / `ToolActions.cpp` exactly with merged `main` (`out/audit/typed-city-callsite-check.log`). This supplements helper tests without claiming a simulation digest. Build/test logs are `out/audit/typed-city-*`. Catch2/engine extraction, deterministic simulation digests, historical file parsing, ASan and exhaustive UI tests are deferred.

See `tests/baseline/BOOTSTRAP_2026-10-05.md` for checkpoint results; raw local configure/build/test/run logs are in ignored `out/audit/`. See `project/reference/RUNTIME_ASSETS.md` for complete resource and licence details.

Earthquake presentation also uses a header-only interface and application-owned null adapter. The configured Debug/Release presets and retained Visual Studio Release build pass after migration; build/test logs are `out/audit/typed-presentation-*`. Reversing only the presentation include, parameter and dispatch restores `w_tk.cpp` exactly from merged `main` (`out/audit/typed-presentation-source-check.log`). Recording tests observe state at event dispatch; real renderer timing/cancellation remains deferred. No new translation unit or dependency is added.
