// Civic 89 actual SDL rendering/input acceptance. SPDX-License-Identifier: GPL-3.0-or-later
#include "Camera2D.h"
#include "DisplayLayout.h"
#include "DisplaySettings.h"
#include "EngineDigest.h"
#include "MapRenderer.h"
#include "Map.h"
#include "PresentationEvents.h"
#include "SdlResources.h"
#include "ToolManager.h"
#include "main.h"
#include "w_tk.h"
#include "UI/InterfaceManager.h"
#include "UI/MiniMapWindow.h"
#include <SDL3_image/SDL_image.h>
#include <array>
#include <cmath>
#include <iostream>
#include <stdexcept>

extern SDL_Window* MainWindow;
extern uint32_t MainWindowId;
void handleMouseEvent(SDL_Event&);
void handleKeyEvent(SDL_Event&);
void handleWindowEvent(SDL_Event&);
void pumpEvents();
void windowResized();
void advancePresentation(Uint64);
bool applyDisplayMode(WindowMode);
void persistDisplaySettings();

namespace
{
    void require(bool value, const char* message) { if (!value) { throw std::runtime_error(message); } }
    std::array<Uint8, 4> pixel(SDL_Surface* surface, int x, int y)
    {
        std::array<Uint8, 4> result{};
        require(SDL_ReadSurfacePixel(surface, x, y, &result[0], &result[1], &result[2], &result[3]), "Read pixel failed");
        return result;
    }
    void click(Point<float> point, float scale)
    {
        SDL_Event down{};
        down.type = SDL_EVENT_MOUSE_BUTTON_DOWN; down.button.windowID = MainWindowId;
        down.button.button = SDL_BUTTON_LEFT; down.button.x = point.x * scale; down.button.y = point.y * scale;
        require(SDL_ConvertEventToRenderCoordinates(MainWindowRenderer, &down), "DPI mouse down conversion");
        handleMouseEvent(down);
        SDL_Event up = down; up.type = SDL_EVENT_MOUSE_BUTTON_UP;
        handleMouseEvent(up);
    }
}

void runWindowCameraRenderingTests(Budget& budget, CityProperties& properties, PresentationEvents& presentation,
    ToolManager& tools, InterfaceManager& ui, MiniMapWindow& minimap)
{
    auto& camera = applicationCamera();
    auto& renderer = applicationMapRenderer();
    ui.hideAllWindows();
    SurfaceOwner atlas(IMG_Load("images/tiles.xpm"));
    require(static_cast<bool>(atlas), "Tile atlas fixture");
    for (const float scale : {1.f, 1.25f, 1.5f, 2.f, 1.25f, 1.f})
    {
        require(SDL_SetWindowSize(MainWindow, 1600, 1200), "Test output resize");
        pumpEvents();
        require(SDL_SetRenderScale(MainWindowRenderer, scale, scale), "Test content scale");
        camera.displayScale(scale); camera.pixelPerfect(false); camera.viewport({1600 / scale, 1200 / scale});
        camera.zoomAt(1.5f, {}); camera.position({320, 320});
        ResetMap(); renderer.invalidate();
        renderer.render(camera);
        const Point<float> center = camera.worldToScreen({648, 648}); // Center of tile (40,40).
        SurfaceOwner output(SDL_RenderReadPixels(MainWindowRenderer, nullptr));
        require(output && pixel(output.get(), static_cast<int>(center.x * scale), static_cast<int>(center.y * scale)) == pixel(atlas.get(), 8, 8), "DPI/zoom tile sampling mismatch");
        const auto before = engineStateDigest(properties, budget);
        renderer.render(camera, {}, MapPreview{{640, 640, 16, 16}, nullptr});
        SurfaceOwner preview(SDL_RenderReadPixels(MainWindowRenderer, nullptr));
        require(preview && pixel(preview.get(), static_cast<int>(center.x * scale), static_cast<int>(center.y * scale)) != pixel(output.get(), static_cast<int>(center.x * scale), static_cast<int>(center.y * scale)), "Tool overlay did not use camera transform");
        require(engineStateDigest(properties, budget) == before, "Rendering mutated simulation");
        tools.currentTool(Tool::Type::Road); budget.CurrentFunds(10000);
        click(center, scale);
        require(tileIsRoad({40, 40}) && budget.CurrentFunds() == 9990, "DPI/zoom construction coordinate or cost mismatch");
        renderer.invalidate(); renderer.render(camera);
        ui.centerWindow(InterfaceManager::Window::Budget);
        const auto area = ui.budgetWindow().area();
        require(std::abs(area.position.x - (static_cast<int>(camera.viewport().x) - area.size.x) / 2) <= 1, "DPI panel centering");
        minimap.updateViewportSize(camera.visibleWorld()); minimap.updateMapViewPosition(camera.position());
        minimap.draw(); minimap.drawUI(); // Includes existing data overlay targets.
    }
    ui.hideAllWindows();
    windowResized(); camera.pixelPerfect(false); camera.zoomAt(1, {}); camera.position({400, 300});
    camera.viewport({800, 600});
    SDL_Event wheel{}; wheel.type = SDL_EVENT_MOUSE_WHEEL; wheel.wheel.windowID = MainWindowId;
    wheel.wheel.mouse_x = 500.75f; wheel.wheel.mouse_y = 450.25f; wheel.wheel.y = 2;
    const auto anchor = camera.screenToWorld({wheel.wheel.mouse_x, wheel.wheel.mouse_y});
    handleMouseEvent(wheel);
    const auto zoomed = camera.screenToWorld({wheel.wheel.mouse_x, wheel.wheel.mouse_y});
    require(std::abs(anchor.x - zoomed.x) < .001f && std::abs(anchor.y - zoomed.y) < .001f, "Actual wheel zoom anchor");
    const auto position = camera.position();
    SDL_Event motion{}; motion.type = SDL_EVENT_MOUSE_MOTION; motion.motion.windowID = MainWindowId;
    motion.motion.x = 520.75f; motion.motion.y = 470.25f; motion.motion.xrel = 20; motion.motion.yrel = 20;
    motion.motion.state = SDL_BUTTON_RMASK;
    handleMouseEvent(motion);
    require(std::abs(camera.position().x - (position.x - 20 / camera.zoom())) < .001f, "Actual right-drag camera pan");
    SDL_Event home{}; home.type = SDL_EVENT_KEY_DOWN; home.key.windowID = MainWindowId; home.key.key = SDLK_HOME;
    handleKeyEvent(home);
    require(std::abs(camera.worldToScreen({960, 800}).x - 400) < .001f, "Actual Home navigation key");
    windowResized();
    const auto beforeFocus = engineStateDigest(properties, budget);
    presentation.focusMap({60, 50});
    const auto focus = camera.worldToScreen({960, 800});
    require(std::abs(focus.x - camera.viewport().x / 2) < 1 && std::abs(focus.y - camera.viewport().y / 2) < 1,
        "Actual auto-goto presentation route");
    require(engineStateDigest(properties, budget) == beforeFocus, "Navigation consumed engine RNG/state");
    DoEarthQuake(); require(ShakeNow > 0, "Actual earthquake route");
    // A modal suspends simulation while the presentation deadline still expires.
    ui.showWindow(InterfaceManager::Window::Budget);
    advancePresentation(SDL_GetTicks() + 3100);
    require(ShakeNow == 0, "Actual earthquake must expire after three seconds");
    ui.hideAllWindows();
    require(applyDisplayMode(WindowMode::Windowed), "Windowed mode transition");
    if (std::string_view(SDL_GetCurrentVideoDriver()) != "dummy")
    {
        require(applyDisplayMode(WindowMode::Maximized), "Maximized mode transition");
        require((SDL_GetWindowFlags(MainWindow) & SDL_WINDOW_MAXIMIZED) != 0, "Maximized flag");
        int count = 0;
        std::unique_ptr<SDL_DisplayID, SdlDeleter<SDL_DisplayID, SDL_free>> displays(SDL_GetDisplays(&count));
        require(displays && count > 0, "Native display enumeration");
        std::cout << "Native display count: " << count << '\n';
        require(applyDisplayMode(WindowMode::Windowed), "Restore before monitor migration");
        for (int i = 0; i < count; ++i)
        {
            SDL_Rect bounds{};
            require(SDL_GetDisplayBounds(displays.get()[i], &bounds), "Native monitor bounds");
            require(SDL_SetWindowPosition(MainWindow, bounds.x + 20, bounds.y + 20) && SDL_SyncWindow(MainWindow), "Native monitor migration");
            pumpEvents(); windowResized();
            require(std::isfinite(camera.viewport().x) && camera.viewport().x >= 800, "Native migration layout");
            std::cout << "Native monitor content scale: " << SDL_GetWindowDisplayScale(MainWindow) << '\n';
        }
    }
    require(applyDisplayMode(WindowMode::Borderless), "Borderless fullscreen transition");
    require((SDL_GetWindowFlags(MainWindow) & SDL_WINDOW_FULLSCREEN) != 0, "Fullscreen flag");
    require(applyDisplayMode(WindowMode::Windowed), "Return from fullscreen");
    require((SDL_GetWindowFlags(MainWindow) & (SDL_WINDOW_FULLSCREEN | SDL_WINDOW_MAXIMIZED)) == 0, "Restored window flags");
    persistDisplaySettings();
    // Events from the detached minimap cannot resize/close the main window.
    const auto viewport = camera.viewport();
    SDL_Event unrelated{}; unrelated.type = SDL_EVENT_WINDOW_RESIZED; unrelated.window.windowID = minimap.id();
    unrelated.window.data1 = 10; unrelated.window.data2 = 10;
    handleWindowEvent(unrelated);
    require(camera.viewport() == viewport, "Minimap event changed main layout");
    camera.pixelPerfect(false); windowResized(); renderer.invalidate();
    const auto beforeReset = engineStateDigest(properties, budget);
    const auto cameraBefore = camera.position();
    for (const auto type : {SDL_EVENT_RENDER_TARGETS_RESET, SDL_EVENT_RENDER_DEVICE_RESET})
    {
        SDL_Event reset{}; reset.type = type; reset.render.windowID = MainWindowId;
        require(SDL_PushEvent(&reset), "Queue graphics reset");
        pumpEvents();
        applicationMapRenderer().render(camera);
        require(engineStateDigest(properties, budget) == beforeReset && camera.position() == cameraBefore,
            "Graphics reset lost engine/camera state");
    }
    std::cout << "SDL 100/125/150/200% rendering, mixed-scale input, construction, panels, modes and earthquake expiry passed\n";
}
