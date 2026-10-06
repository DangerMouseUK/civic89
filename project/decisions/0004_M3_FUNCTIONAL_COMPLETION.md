# ADR 0004: M3 functional completion

Accepted on 6 October 2026 for `codex/m3-functional-completion`, based on merged
M2 (`090633d`). This branch delivers M3-01 through M3-04 together.

## Audio

The application implements `AudioService` with SDL3_mixer 3.2.4 from the existing
pinned vcpkg baseline. The engine remains SDL-free. Twelve typed IDs map to
original synthesised PCM effects; each is loaded into a mixer-owned audio
resource. No inherited WAV file or retail sound is staged. The synthesiser has
its own fixed local noise generator and never consumes simulation randomness.
Its GPL source is the complete provenance for these new effects.

Sixteen bounded effect voices and separate city/construction loops support
overlapping playback without unbounded resource growth. Master/category gain
applies to current playback. F8 opens native volume controls; gains persist in
`audio.cfg` in SDL's Civic89 preference directory. The existing sound checkbox
now gates playback correctly and its city-file field stores the user preference.
Unavailable devices/resources produce a structured warning and a silent adapter;
the game continues. No soundtrack or music asset is introduced.

**Compatibility decision:** the old `userSoundOn()` accessor accidentally read
the initialization flag, so disabling sound could re-enable it on the next
effect. M3 fixes that bug. Stop requests remain effective while muted; new,
loaded and scenario cities stop the preceding city's loops. Simulation rules,
RNG draws, disaster scheduling and scenario outcomes are unchanged.

## Ownership and shutdown

`Texture` owns its pointer and is move-only. UI members and sprite caches release
textures before their renderer. Minimap members own their window/renderer in
that order; construction exceptions also unwind them. Font loading owns its
temporary TTF font/surfaces, dashboard fonts belong to the dashboard instance,
and texture properties remain borrowed from SDL.

An application lifetime guard runs on both normal exit and exceptions: remove
timers, release UI/minimap/tools, detach engine services, stop/release audio,
release sprites/textures, destroy the main renderer/window, clear callbacks,
quit TTF, then quit SDL. Global raw pointers are cleared. Tests exercise three
partial startup failures followed by twelve complete application sessions.

## City persistence

The engine serializes a snapshot through `AtomicFileWriter`; Windows operations
live in `WindowsFileStorage`, outside `civic89_engine`. The adapter creates an
exclusive temporary file beside the destination, checks every write, flushes
and closes it, then publishes with `MoveFileExW(REPLACE_EXISTING | WRITE_THROUGH)`.
No cross-volume copy fallback is requested. Failure returns a typed stage/path/
detail and removes the temporary file; an existing destination remains intact.
Tests cover real replacement failure under a locked destination, creation
failure and an injected writer failure. This is a local-file publication
contract, not a guarantee against faulty storage hardware or remote filesystem
semantics.

**Compatibility decision:** retain the inherited 51,360-byte, little-endian
32-bit layout: seven 120-word histories followed by 120 x 100 x-major tiles.
No header/version or field relocation is added. Save updates happen on the
snapshot, not live history. Difficulty is explicitly serialized in existing
MiscHistory[15]. Load validates before changing the current session, resets
arrays before restoring histories, and uses `InitSimLoad = 1` so stored
difficulty/census/evaluation metadata is actually initialized. Funds, date,
budget percentages and existing options are restored. These fixes intentionally
replace M2's characterized history erasure and incorrect initialization path.

The existing post-load simulation scan still runs and may update tiles/RNG.
The format lacks RNG state, sprite state and scenario deadlines; it cannot be
an exact simulation replay snapshot. Historical 27,120-byte files remain
unsupported and rejected safely. Import support requires its own format audit.

Normal cities autosave every five minutes to `autosave.cty` in the user directory,
using the same writer. Startup offers recovery before play. Autosave errors are
logged and shown on the dashboard; failed saves retain the preceding recovery.
Automatic scenario autosaves are skipped because this format cannot restore
scenario objectives/deadlines; explicit scenario exports retain the inherited
ordinary-city behavior. Recovery/volume files never live beside packaged assets.

## Errors and acceptance

Critical startup, city load/save, file-picker and autosave failures have typed
diagnostic codes, severity, timestamp, quoted detail and path. Records are
flushed to `civic89.log` and stderr. Native errors explain the failed operation;
canceling a picker is distinct from failure. Opening an invalid file neither
resets the city nor changes the save target. Failed saves prompt selection again.

[M3 evidence](../../tests/baseline/M3_2026-10-06.md) records builds and tests.
All 25 M2 golden cases remain unchanged. Application ASan extends CI alongside
Debug, Release and headless ASan. Retained Visual Studio sources/build continue
to work; no source-tree moves, unrelated formatting or simulation rewrites occur.
Desktop audibility/native-dialog interaction and DPI checks remain manual limits;
automated audio tests verify real mixer samples and dummy-device playback.

Primary API references: [SDL_mixer raw audio](https://wiki.libsdl.org/SDL3_mixer/MIX_LoadRawAudio),
[playback](https://wiki.libsdl.org/SDL3_mixer/MIX_PlayTrack),
[device creation](https://wiki.libsdl.org/SDL3_mixer/MIX_CreateMixerDevice),
[Windows publication](https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-movefileexw),
[SDL preference path](https://wiki.libsdl.org/SDL3/SDL_GetPrefPath).
