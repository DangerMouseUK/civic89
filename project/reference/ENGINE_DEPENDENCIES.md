# M2 engine dependency audit

Baseline: merged M1 `de424f37959ca9864ffdebb5f4517a1dc5998e4f`.
The extraction keeps inherited simulation files in place and changes adapters,
not the simulation algorithms. This table records M2-01's direct and transitive
dependencies and the chosen boundary.

| Existing path | Presentation/platform dependency | M2 boundary |
|---|---|---|
| `s_alloc.h`, `Connection.cpp`, `ToolActions.cpp`, `s_msg.cpp`, `w_update.cpp`, `w_tk.cpp` | `main.h` imports SDL, textures and application globals | Platform-free `EngineState.h`; application `main.h` includes it |
| `Map.h`, `Map.cpp` | SDL rectangle, renderer, map texture, blinking and draw calls mixed with tile storage | Preserve storage in `Map.cpp`; draw/blink code in `MapRenderer.cpp` |
| `Sprite.h`, `Sprite.cpp` | Texture ownership, image loading and camera/render calls mixed with movement | Plain sprite state plus frame count; application `SpriteRenderer.cpp` owns image cache |
| `Util.h`, `Util.cpp` | SDL rectangle conversions alongside RNG and simulation speed | SDL helpers in `PresentationUtil`; engine keeps RNG/speed/coordinate logic |
| `s_sim.cpp` | Unused direct SDL include; most contamination comes through map/sprite/allocation headers | Remove unused include; inherited phase algorithms remain unchanged |
| `main.cpp` initialization, `gameplayOptions()` | Engine initialization requires a tool palette and application-owned options/globals | `EngineState.cpp` owns state/initialization; typed event resets application tools |
| `s_msg.cpp` | `SDL_GetTicks()` for message expiry | Injectable millisecond clock; app supplies SDL ticks, headless default uses steady clock |
| `w_update.cpp` | Calls `showBudgetWindow()` during January update | Dedicated presentation event; headless implementation is silent |
| `Sprite.cpp`, `ToolActions.cpp`, `s_disast.cpp` | Audio and earthquake entry points implemented in application main | Platform-free service adapters with silent defaults; app injects services |
| `ToolManager.cpp` | Constructor reads JSON assets through `GameDataLoader` | Engine accepts tool definitions; application constructor loads existing JSON |
| `FileIo.cpp`, `ScenarioData.h` | Platform-free filesystem, inherited array format, option/sound helpers | Remain engine code; legacy format is unchanged |
| `Budget`, `RCI`, `Evaluation`, `Scan`, `Power`, `Traffic`, `Zone`, `Connection`, `s_gen`, `g_ani`, `w_resrc`, math | No direct graphics/platform requirement after header cleanup | Compiled once into `civic89_engine` |
| `Month.cpp`, `GameDataLoader.cpp`, UI, graphics, font, texture, main | JSON assets, SDL/Win32, windows, input and dialogs | Application-only sources |

The inherited engine retains process-global state and single-threaded operation.
M2 does not promise simultaneous independent cities, scheduling equivalence to a
wall-clock GUI session, historical 16-bit city import, or M3 resource/audio work.
