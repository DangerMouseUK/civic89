// This file is part of Micropolis-SDLPP
// Micropolis-SDLPP is based on Micropolis
//
// Copyright © 2022 - 2026 Leeor Dicker
//
// Portions Copyright © 1989-2007 Electronic Arts Inc.
//
// Micropolis-SDLPP is free software; you can redistribute it and/or modify
// it under the terms of the GNU GPLv3, with additional terms. See the README
// file, included in this distribution, for details.
#include "MapRenderer.h"
#include "DisplayLayout.h"
#include "DisplaySettings.h"
#include "FrameScheduler.h"
#include "QuakeEffect.h"
#include "EngineServices.h"
#include "main.h"

#include "AudioService.h"
#include "AudioManager.h"
#include "DiagnosticLog.h"
#include "RecoveryStore.h"
#include "WindowsFileStorage.h"
#include "SdlResources.h"
#include <SDL3_ttf/SDL_ttf.h>
#include <fstream>
#include "Budget.h"
#include "CityProperties.h"
#include "Colors.h"
#include "Connection.h"
#include "Evaluation.h"
#include "FileIo.h"
#include "Font.h"
#include "g_ani.h"
#include "GameOptions.h"
#include "Graph.h"
#include "Map.h"
#include "Month.h"
#include "PresentationEvents.h"
#include "ScenarioController.h"
#include "ScenarioData.h"
#include "s_alloc.h"
#include "s_disast.h"
#include "s_gen.h"
#include "s_msg.h"
#include "s_sim.h"
#include "Scan.h"
#include "Sprite.h"
#include "StringRender.h"
#include "Texture.h"
#include "ToolActions.h"
#include "ToolManager.h"
#include "Util.h"
#include "w_sound.h"
#include "w_tk.h"
#include "w_update.h"

#include "Math/Rectangle.h"

#include "UI/InterfaceManager.h"
#include "UI/MiniMapWindow.h"

#include <algorithm>
#include <cstdint>
#include <charconv>
#include <optional>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <array>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#undef NOMINMAX
#undef WIN32_LEAN_AND_MEAN
#endif

const std::string MicropolisVersion = "4.0";

SDL_Window* MainWindow = nullptr;
SDL_Renderer* MainWindowRenderer = nullptr;

uint32_t MainWindowId{};




namespace
{
    constexpr auto TileSize = 16;
    constexpr auto MiniTileSize = 3;


	constexpr Point<int> ToolPaletteDefaultPosition{ 10, 100 };

    Camera2D camera;
    QuakeEffect quake;
    Vector<float> cameraShake{};
    std::unique_ptr<MapRenderer> cityRenderer;
    DisplaySettings displaySettings;
    DisplayLayout displayLayout;
    FrameScheduler scheduler;
    bool settingsDirty{false};
    bool vsyncActive{false};
    Uint64 settingsChangedAt{};
    Vector<float> panelDragRemainder{};

    Vector<int> WindowSize{};
    Vector<int> DraggableToolVector{};


    Point<int> TilePointedAt{};

    bool Exit{ false };
    bool RightButtonDrag{ false };
    bool MapToolGesture{ false };



    std::array<unsigned int, 5> SpeedModifierTable{ 0, 0, 50, 75, 95 };



    Budget budget{};
    CityProperties cityProperties{};
    NullAudioService silentAudio;
    NullPresentationEvents silentPresentation;
    std::unique_ptr<AudioManager> audioService;
    std::unique_ptr<DiagnosticLog> diagnostics;
    std::unique_ptr<RecoveryStore> recovery;
    WindowsFileStorage fileStorage;
    std::filesystem::path userDirectory;
    Uint64 lastAutosave{};
    void reportCityError(const CityIoResult& result, DiagnosticCode code)
    {
        diagnostics->write(Severity::Error, code, result.detail, result.path);
        const auto message = std::string(code == DiagnosticCode::CityLoad ? "Could not open the city. Your current city is unchanged.\n" :
            "Could not save the city. Choose another location and try again.\n") + pathUtf8(result.path) + "\n" + result.detail;
#if !defined(CIVIC89_MILESTONE_TESTS)
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Civic 89", message.c_str(), MainWindow);
#endif
    }
    void syncAudioOptions()
    {
        audioService->enabled(userSoundOn());
        if (!userSoundOn()) { SoundOff(); }
    }
    void audioSettings()
    {
        const std::array<SDL_MessageBoxButtonData,4> categories{{{0,0,"Master"}, {0,1,"City effects"}, {0,2,"Construction"}, {SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT,3,"Close"}}};
        const SDL_MessageBoxData categoryBox{SDL_MESSAGEBOX_INFORMATION, MainWindow, "Civic 89 sound", "Choose a volume category.", 4, categories.data(), nullptr};
        int category = 3;
        if (!SDL_ShowMessageBox(&categoryBox, &category) || category < 0 || category > 2) { return; }
        const std::array<SDL_MessageBoxButtonData,6> levels{{{0,0,"Mute"}, {0,1,"25%"}, {0,2,"50%"}, {0,3,"75%"}, {0,4,"100%"}, {SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT,5,"Cancel"}}};
        const SDL_MessageBoxData volumeBox{SDL_MESSAGEBOX_INFORMATION, MainWindow, "Civic 89 sound", "Choose the volume.", 6, levels.data(), nullptr};
        int level = 5;
        if (!SDL_ShowMessageBox(&volumeBox, &level) || level < 0 || level > 4) { return; }
        const float gain = level * .25f;
        if (category == 0) { audioService->masterVolume(gain); }
        else { audioService->channelVolume(category == 1 ? AudioChannel::City : AudioChannel::Construction, gain); }
        std::ostringstream values;
        values << audioService->masterVolume() << ' ' << audioService->channelVolume(AudioChannel::City) << ' ' << audioService->channelVolume(AudioChannel::Construction);
        const auto bytes = values.str();
        const auto result = fileStorage.write(userDirectory / "audio.cfg", std::span<const char>(bytes.data(), bytes.size()));
        if (!result)
        {
            diagnostics->write(Severity::Error, DiagnosticCode::Settings, result.detail, result.path);
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Civic 89 sound",
                "Could not save sound settings. They will apply for this session.", MainWindow);
        }
    }
    class ApplicationPresentationEvents final : public PresentationEvents
    {
    public:
        void toolsReset() override;
        void budgetRequested() override;
        void earthquakeStarted() override { quake.start(SDL_GetTicks()); }
        void focusMap(Point<int> point) override { camera.focus({point.x * 16.f, point.y * 16.f}); }
        void generationStarted() override {} // Notification precedes GenerateMap.
        void showMessage(const std::string& message) override;
        void scenarioStarted(Scenario scenario) override;
        void scenarioFinished(ScenarioOutcome outcome) override;
    };
    ApplicationPresentationEvents presentationEvents;

    std::unique_ptr<MiniMapWindow> miniMapWindow;

	std::shared_ptr<InterfaceManager> interfaceManager;
	std::shared_ptr<ToolManager> toolManager;


    unsigned int speedModifier()
    {
        return SpeedModifierTable[static_cast<unsigned int>(simSpeed())];
    }

    void showBudgetIfNeeded()
    {
        if (!gameplayOptions().autoBudget && budget.NeedsAttention())
        {
			interfaceManager->showWindow(InterfaceManager::Window::Budget);
        }
    }

    void positionDashboardWindow()
    {
        const Point<int> dashboardPosition{ WindowSize.x / 2 - interfaceManager->dashboardWindow().size().x / 2, 10 };
        interfaceManager->positionWindow(InterfaceManager::Window::Dashboard, dashboardPosition);
    }
};


namespace EventHandling
{
    Point<int> MouseDownPosition{};
    Point<int> MouseClickPosition{};
    Point<int> MousePosition{};
    Point<float> SceneMousePosition{};

    bool MouseLeftDown{ false };
};


MapRenderer& applicationMapRenderer() { return *cityRenderer; }
Camera2D& applicationCamera() { return camera; }

void showBudgetWindow()
{
    interfaceManager->showWindow(InterfaceManager::Window::Budget);
}


void simInit()
{
    initializeEngine(cityProperties, budget);
    quake.cancel();
    cameraShake = {};
    syncAudioOptions();
    Exit = false;
}

void simExit()
{
    Exit = true;
}


void simUpdate()
{
    updateDate();

    if (newMonth())
    {
        interfaceManager->newMonth();
    }

    showBudgetIfNeeded();

    refreshCityEvaluation(cityProperties);
}


void advancePresentation(Uint64 now)
{
    for (const auto event : scheduler.advance(now, 100 - speedModifier(), interfaceManager->modalWindowVisible()))
    {
        switch (event)
        {
        case FrameScheduler::Event::Simulation:
            if (interfaceManager->modalWindowVisible()) { break; }
            SimFrame(cityProperties, budget);
            cityRenderer->invalidate();
            simUpdate();
            break;
        case FrameScheduler::Event::Animation:
            if (!interfaceManager->modalWindowVisible() && !paused()) { animateTiles(); updateSprites(); cityRenderer->invalidate(); }
            break;
        case FrameScheduler::Event::Blink:
            cityRenderer->toggleBlink();
            break;
        case FrameScheduler::Event::Minimap:
            miniMapWindow->draw();
            break;
        }
    }
    if (!ShakeNow) { quake.cancel(); }
    else if (!quake.active(now)) { StopEarthquake(); quake.cancel(); }
    cameraShake = quake.offset(now);
}


void ApplicationPresentationEvents::toolsReset()
{
    if (toolManager) { toolManager->currentTool(Tool::Type::None); }
}

void ApplicationPresentationEvents::budgetRequested()
{
    if (interfaceManager) { interfaceManager->showWindow(InterfaceManager::Window::Budget); }
}


void ApplicationPresentationEvents::showMessage(const std::string& message)
{
    if (interfaceManager) { interfaceManager->dashboardWindow().setMessage(message); }
}

void ApplicationPresentationEvents::scenarioStarted(Scenario scenario)
{
    if (interfaceManager)
    {
        interfaceManager->dashboardWindow().cityName(cityProperties.CityName());
        showMessage(std::string("Scenario started: ") + scenarioDefinition(scenario)->cityName);
    }
}

void ApplicationPresentationEvents::scenarioFinished(ScenarioOutcome outcome)
{
    showMessage(outcome == ScenarioOutcome::Won ? "Scenario won." : "Scenario lost.");
}


void doPlayNewCity(CityProperties& properties, Budget& cityBudget)
{
    GenerateNewCity(properties, cityBudget);
    resume();
    simSpeed(SimulationSpeed::Normal);
}


ScenarioResult doStartScenario(Scenario scenario)
{
    ScenarioController controller(cityProperties, budget, presentationEvents);
    const auto result = controller.start(scenario);
    if (result == ScenarioResult::Success)
    {
        interfaceManager->fileIoDialog().clearSaveFilename();
        interfaceManager->toolPalette().cancelTool();
        cityRenderer->invalidate();
    }
    return result;
}

void selectScenario()
{
    std::array<SDL_MessageBoxButtonData, 9> buttons{};
    buttons[0] = {SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 0, "Cancel"};
    for (size_t i = 0; i < ScenarioDefinitions.size(); ++i)
    {
        const auto& definition = ScenarioDefinitions[i];
        buttons[i + 1] = {0, definition.legacyId, definition.cityName};
    }
    const SDL_MessageBoxData dialog{SDL_MESSAGEBOX_INFORMATION, MainWindow, "Civic 89 - Scenarios",
        "Choose a scenario. Starting one replaces the current city; save it first if needed.",
        static_cast<int>(buttons.size()), buttons.data(), nullptr};
    int selected = 0;
    if (!SDL_ShowMessageBox(&dialog, &selected))
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Civic 89", SDL_GetError(), MainWindow);
        return;
    }
    const auto scenario = scenarioFromLegacyId(selected);
    if (!scenario) { return; } // Cancel or closed dialog.
    const auto result = doStartScenario(*scenario);
    if (result != ScenarioResult::Success)
    {
        const auto* error = result == ScenarioResult::MissingFile ?
            "The scenario file is missing. The current city has been kept." :
            "The scenario file is invalid or unreadable. The current city has been kept.";
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Civic 89 - Scenario", error, MainWindow);
    }
}


void primeGame(const int startFlag, CityProperties& properties, Budget& cityBudget)
{
    switch (startFlag)
    {
    case -2: // Load a city
        if (LoadCity("filename", properties, cityBudget))
        {
			interfaceManager->dashboardWindow().cityName(properties.CityName());
            break;
        }
        // If load fails, simply create a new city
        [[fallthrough]];

    case -1:
        properties.GameLevel(0);
        properties.CityName("NoWhere");
		interfaceManager->dashboardWindow().cityName(properties.CityName());
        doPlayNewCity(properties, cityBudget);
        break;

    case 0:
        throw std::runtime_error("Unexpected startup switch: " + std::to_string(startFlag));
        break;

    default: // scenario number
        const auto scenario = scenarioFromLegacyId(startFlag);
        if (!scenario || doStartScenario(*scenario) != ScenarioResult::Success)
        {
            throw std::runtime_error("Unable to start scenario " + std::to_string(startFlag));
        }
        break;
    }
}


void resetGame()
{
    simInit();
    primeGame(-1, cityProperties, budget);
}


void newGame()
{
    interfaceManager->fileIoDialog().clearSaveFilename();
    resetGame();
    cityRenderer->invalidate();
}


void openGame()
{
    if (interfaceManager->fileIoDialog().pickOpenFile())
    {
        const auto result = LoadCityDetailed(pathFromUtf8(interfaceManager->fileIoDialog().openPath()), cityProperties, budget);
        if (!result) { reportCityError(result, DiagnosticCode::CityLoad); return; }
        interfaceManager->fileIoDialog().clearSaveFilename();
        syncAudioOptions();
        interfaceManager->toolPalette().cancelTool();
        interfaceManager->dashboardWindow().cityName(cityProperties.CityName());
        updateDate();
        cityRenderer->invalidate();
    }
}


void saveGame()
{
    if (!interfaceManager->fileIoDialog().filePicked() || SDL_GetModState() & SDL_KMOD_SHIFT)
    {
        if (!interfaceManager->fileIoDialog().pickSaveFile())
        {
            {
                return;
            }
        }
    }

    const auto result = SaveCity(pathFromUtf8(interfaceManager->fileIoDialog().fullPath()), cityProperties, budget, fileStorage);
    if (!result)
    {
        interfaceManager->fileIoDialog().clearSaveFilename();
        reportCityError(result, DiagnosticCode::CitySave);
    }
    else { diagnostics->write(Severity::Info, DiagnosticCode::CitySave, "City saved", result.path); }
}


void loadGraphics()
{
    cityRenderer = std::make_unique<MapRenderer>(MainWindowRenderer);
}

void updateMapDrawParameters()
{
    if (!miniMapWindow) { return; }
    miniMapWindow->updateViewportSize(camera.visibleWorld());
    miniMapWindow->updateMapViewPosition(camera.position());
}

void windowSize()
{
    int width, height;
    if (!SDL_GetRenderOutputSize(MainWindowRenderer, &width, &height)) { throw std::runtime_error(SDL_GetError()); }
    if (width <= 0 || height <= 0) { return; }
    displayLayout = DisplayLayout::fromPixels(width, height, SDL_GetWindowDisplayScale(MainWindow));
    if (!SDL_SetRenderScale(MainWindowRenderer, displayLayout.scale, displayLayout.scale)) { throw std::runtime_error(SDL_GetError()); }
    WindowSize = {static_cast<int>(displayLayout.logical.x), static_cast<int>(displayLayout.logical.y)};
    camera.displayScale(displayLayout.scale);
    camera.viewport(displayLayout.logical);
    const float windowScale = displayLayout.scale / SDL_GetWindowPixelDensity(MainWindow);
    SDL_SetWindowMinimumSize(MainWindow, static_cast<int>(800 * windowScale), static_cast<int>(600 * windowScale));
}

void minimapViewUpdated(const Point<int>& tile)
{
    camera.focus({tile.x * 16.f + 8, tile.y * 16.f + 8});
    updateMapDrawParameters();
}

void windowResized()
{
    if (SDL_GetWindowFlags(MainWindow) & SDL_WINDOW_MINIMIZED) { return; }
    windowSize();
    updateMapDrawParameters();
    if (!interfaceManager) { return; }
    interfaceManager->centerWindows({ InterfaceManager::Window::Budget,
        InterfaceManager::Window::Evaluation, InterfaceManager::Window::Graph,
        InterfaceManager::Window::Options, InterfaceManager::Window::Query});
    positionDashboardWindow();
    interfaceManager->positionWindow(InterfaceManager::Window::ToolPalette, ToolPaletteDefaultPosition);
}

void calculateMouseToWorld()
{
    const auto world = camera.screenToWorld(EventHandling::SceneMousePosition - cameraShake);
    TilePointedAt = {static_cast<int>(std::floor(world.x / TileSize)), static_cast<int>(std::floor(world.y / TileSize))};
    if (miniMapWindow) { miniMapWindow->updateTilePointedAt(TilePointedAt); }
}


bool IgnoreToolMouseUp()
{
    if (interfaceManager->pointInWindow(EventHandling::MousePosition))
    {
        return true;
    }

    if (EventHandling::MouseDownPosition != EventHandling::MousePosition &&
        interfaceManager->pointInWindow(EventHandling::MouseDownPosition))
    {
        return true;
    }

    return false;
}


void SetSpeed(SimulationSpeed speed)
{
    if (paused())
    {
        resume();
    }

    simSpeed(speed);
}


void TogglePause()
{
    paused() ? resume() : pause();
}


void ToggleMiniMapVisibility()
{
    miniMapWindow->hidden() ? miniMapWindow->show() : miniMapWindow->hide();
}


void showEvaluationWindow()
{
    interfaceManager->evaluationWindow().setEvaluation(currentEvaluation());
    interfaceManager->showWindow(InterfaceManager::Window::Evaluation);
    currentEvaluationSeen();
}


void showSystemWindow()
{
    interfaceManager->hideAllWindows();
    interfaceManager->optionsWindow().setOptions(gameplayOptions());
    interfaceManager->optionsWindow().show();
}


void markDisplaySettingsChanged()
{
    settingsDirty = true;
    settingsChangedAt = SDL_GetTicks();
}

void persistDisplaySettings()
{
    if (!settingsDirty) { return; }
    settingsDirty = false;
    const auto result = displaySettings.save(userDirectory / "display.cfg", fileStorage);
    if (!result)
    {
        diagnostics->write(Severity::Error, DiagnosticCode::Settings, result.detail, result.path);
#if !defined(CIVIC89_MILESTONE_TESTS)
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Civic 89 display",
            "Could not save display settings. They will apply for this session.", MainWindow);
#endif
    }
}

bool applyDisplayMode(WindowMode mode)
{
    const auto flags = SDL_GetWindowFlags(MainWindow);
    bool success = !(flags & SDL_WINDOW_FULLSCREEN) || SDL_SetWindowFullscreen(MainWindow, false);
    if (success && (flags & (SDL_WINDOW_MAXIMIZED | SDL_WINDOW_MINIMIZED))) { success = SDL_RestoreWindow(MainWindow); }
    if (mode == WindowMode::Borderless)
    {
        success = success && SDL_SetWindowFullscreenMode(MainWindow, nullptr) && SDL_SetWindowFullscreen(MainWindow, true);
    }
    else
    {
        const float scale = SDL_GetWindowDisplayScale(MainWindow) / SDL_GetWindowPixelDensity(MainWindow);
        SDL_Rect usable{};
        const bool bounded = SDL_GetDisplayUsableBounds(SDL_GetDisplayForWindow(MainWindow), &usable);
        const int width = static_cast<int>(displaySettings.width * scale);
        const int height = static_cast<int>(displaySettings.height * scale);
        success = success && SDL_SetWindowSize(MainWindow, bounded ? std::min(width, usable.w) : width,
            bounded ? std::min(height, std::max(1, usable.h - 50)) : height);
        if (mode == WindowMode::Maximized) { success = success && SDL_MaximizeWindow(MainWindow); }
    }
    success = success && SDL_SyncWindow(MainWindow);
    if (success)
    {
        displaySettings.mode = mode;
        windowResized();
        markDisplaySettingsChanged();
        diagnostics->write(Severity::Info, DiagnosticCode::Display, "Window mode " + std::to_string(static_cast<int>(mode)));
    }
    else
    {
        diagnostics->write(Severity::Warning, DiagnosticCode::Display, SDL_GetError());
#if !defined(CIVIC89_MILESTONE_TESTS)
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Civic 89 display", SDL_GetError(), MainWindow);
#endif
    }
    return success;
}

void applyVSync()
{
    vsyncActive = displaySettings.vsync && SDL_SetRenderVSync(MainWindowRenderer, 1);
    if (!vsyncActive)
    {
        SDL_SetRenderVSync(MainWindowRenderer, 0);
        if (displaySettings.vsync) { diagnostics->write(Severity::Warning, DiagnosticCode::Display, "VSync unavailable; using a 60 Hz frame limit"); }
    }
}

void displayOptions()
{
    const std::array<SDL_MessageBoxButtonData,6> buttons{{{0,0,"Windowed"}, {0,1,"Maximized"}, {0,2,"Borderless fullscreen"},
        {0,3,"Toggle VSync"}, {0,4,"Toggle pixel perfect"}, {SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT,5,"Close"}}};
    const std::string message = std::string("VSync: ") + (displaySettings.vsync ? "on" : "off") +
        "; pixel perfect: " + (displaySettings.pixelPerfect ? "on" : "off") +
        "\nMouse wheel: zoom; right-drag or arrow keys: pan; Home: center.\nF11: toggle borderless fullscreen; F12: display settings.";
    const SDL_MessageBoxData box{SDL_MESSAGEBOX_INFORMATION, MainWindow, "Civic 89 display", message.c_str(),
        static_cast<int>(buttons.size()), buttons.data(), nullptr};
    int selected = 5;
    if (!SDL_ShowMessageBox(&box, &selected)) { diagnostics->write(Severity::Warning, DiagnosticCode::Display, SDL_GetError()); return; }
    if (selected >= 0 && selected <= 2) { applyDisplayMode(static_cast<WindowMode>(selected)); }
    else if (selected == 3) { displaySettings.vsync = !displaySettings.vsync; applyVSync(); markDisplaySettingsChanged(); }
    else if (selected == 4)
    {
        displaySettings.pixelPerfect = !displaySettings.pixelPerfect;
        camera.pixelPerfect(displaySettings.pixelPerfect);
        markDisplaySettingsChanged();
    }
    persistDisplaySettings();
}


void handleKeyEvent(SDL_Event& event)
{
    if (event.key.windowID != MainWindowId || event.key.repeat) { return; }
    if (interfaceManager->optionsWindow().visible())
    {
        interfaceManager->optionsWindow().injectKeyDown(event.key.key);
        return;
    }

    switch (event.key.key)
    {
    case SDLK_ESCAPE:
        showSystemWindow();
        break;

    case SDLK_0:
    case SDLK_P:
    case SDLK_SPACE:
        TogglePause();
        break;

    case SDLK_1:
        SetSpeed(SimulationSpeed::Slow);
        break;

    case SDLK_2:
        SetSpeed(SimulationSpeed::Normal);
        break;

    case SDLK_3:
        SetSpeed(SimulationSpeed::Fast);
        break;

    case SDLK_4:
        SetSpeed(SimulationSpeed::AfricanSwallow);
        break;

    case SDLK_F2:
        saveGame();
        break;

    case SDLK_F3:
        openGame();
        break;

    case SDLK_F4:
        ToggleMiniMapVisibility();
        break;

    case SDLK_F5:
        showEvaluationWindow();
        break;

    case SDLK_F6:
        selectScenario();
        break;

    case SDLK_F7:
        newGame();
        break;

    case SDLK_F8:
        audioSettings();
        break;

    case SDLK_F9:
        interfaceManager->showWindow(InterfaceManager::Window::Graph);
        break;

    case SDLK_F11:
        applyDisplayMode(displaySettings.mode == WindowMode::Borderless ? WindowMode::Windowed : WindowMode::Borderless);
        break;
    case SDLK_F12:
        displayOptions();
        break;
    case SDLK_HOME:
        camera.focus({SimWidth * 8.f, SimHeight * 8.f});
        break;
    case SDLK_F10:
		interfaceManager->showWindow(InterfaceManager::Window::Budget);
        break;

    default:
        break;

    }
}


void updateDragVector()
{
    DraggableToolVector = {};
    if (toolManager->currentTool().draggable && MapToolGesture && EventHandling::MouseLeftDown && toolManager->dragStart() != TilePointedAt)
    {
        DraggableToolVector = vectorFromPoints(toolManager->dragStart(), TilePointedAt);
        ToolActions::validateDragVector(DraggableToolVector, budget, *toolManager);
    }
}

void handleMouseEvent(SDL_Event& event)
{
    if (event.window.windowID != MainWindowId) { return; }
    switch (event.type)
    {
    case SDL_EVENT_MOUSE_MOTION:
    {
        EventHandling::SceneMousePosition = {event.motion.x, event.motion.y};
        EventHandling::MousePosition = EventHandling::SceneMousePosition.to<int>();
        panelDragRemainder += Vector<float>{event.motion.xrel, event.motion.yrel};
        const auto delta = panelDragRemainder.to<int>();
        panelDragRemainder -= delta.to<float>();
        interfaceManager->injectMouseMotion(delta);
        if ((event.motion.state & SDL_BUTTON_RMASK) && !interfaceManager->modalWindowVisible())
        {
            camera.pan({-event.motion.xrel, -event.motion.yrel});
            updateMapDrawParameters();
            RightButtonDrag = true;
        }
        calculateMouseToWorld();
        updateDragVector();
        break;
    }
    case SDL_EVENT_MOUSE_WHEEL:
    {
        EventHandling::SceneMousePosition = {event.wheel.mouse_x, event.wheel.mouse_y};
        EventHandling::MousePosition = EventHandling::SceneMousePosition.to<int>();
        if (interfaceManager->modalWindowVisible() || EventHandling::MouseLeftDown ||
            interfaceManager->pointInWindow(EventHandling::MousePosition)) { break; }
        const float direction = event.wheel.direction == SDL_MOUSEWHEEL_FLIPPED ? -event.wheel.y : event.wheel.y;
        const float zoom = camera.pixelPerfect() ? camera.zoom() + direction / displayLayout.scale :
            camera.zoom() * std::pow(1.125f, direction);
        camera.zoomAt(zoom, {event.wheel.mouse_x, event.wheel.mouse_y});
        updateMapDrawParameters(); calculateMouseToWorld();
        break;
    }
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
        EventHandling::SceneMousePosition = {event.button.x, event.button.y};
        EventHandling::MousePosition = EventHandling::SceneMousePosition.to<int>();
        calculateMouseToWorld();
        if (event.button.button == SDL_BUTTON_LEFT)
        {
            EventHandling::MouseLeftDown = true;
            MapToolGesture = false;
            EventHandling::MouseDownPosition = EventHandling::MousePosition;
            if (interfaceManager->injectMouseDown(EventHandling::MousePosition) || interfaceManager->modalWindowVisible()) { return; }
            if (!camera.containsWorld(camera.screenToWorld(EventHandling::SceneMousePosition - cameraShake))) { return; }
            MapToolGesture = true;
            toolManager->dragStart(TilePointedAt); updateDragVector();
            if (!toolManager->currentTool().draggable)
            {
                ToolActions::executeTool(TilePointedAt, budget, *toolManager);
                cityRenderer->invalidate();
            }
            if (toolManager->currentTool().type == Tool::Type::Query)
            {
                interfaceManager->queryWindow().setQueryResult(toolManager->queryResult());
                interfaceManager->showWindow(InterfaceManager::Window::Query);
            }
        }
        break;
    case SDL_EVENT_MOUSE_BUTTON_UP:
        EventHandling::SceneMousePosition = {event.button.x, event.button.y};
        EventHandling::MousePosition = EventHandling::SceneMousePosition.to<int>();
        calculateMouseToWorld(); updateDragVector();
        if (event.button.button == SDL_BUTTON_LEFT)
        {
            const bool gesture = MapToolGesture;
            MapToolGesture = false;
            EventHandling::MouseLeftDown = false;
            EventHandling::MouseClickPosition = EventHandling::MousePosition;
            interfaceManager->injectMouseUp();
            if (!gesture || IgnoreToolMouseUp() || interfaceManager->modalWindowVisible()) { return; }
            if (!camera.containsWorld(camera.screenToWorld(EventHandling::SceneMousePosition - cameraShake))) { return; }
            toolManager->dragEnd(TilePointedAt);
            if (toolManager->currentTool().draggable)
            {
                ToolActions::executeDrag(DraggableToolVector, TilePointedAt, budget, *toolManager);
                cityRenderer->invalidate();
            }
        }
        else if (event.button.button == SDL_BUTTON_RIGHT)
        {
            if (!RightButtonDrag) { toolManager->currentTool(Tool::Type::None); interfaceManager->toolPalette().cancelTool(); }
            RightButtonDrag = false;
        }
        break;
    default: break;
    }
}


void handleWindowEvent(SDL_Event& event)
{
    if (event.window.windowID != MainWindowId) { return; }
    switch (event.type)
    {
    case SDL_EVENT_WINDOW_RESIZED:
    case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
    case SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED:
    case SDL_EVENT_WINDOW_DISPLAY_CHANGED:
        windowResized();
        if (!(SDL_GetWindowFlags(MainWindow) & (SDL_WINDOW_MAXIMIZED | SDL_WINDOW_FULLSCREEN | SDL_WINDOW_MINIMIZED)))
        {
            displaySettings.width = std::clamp(WindowSize.x, 800, 7680);
            displaySettings.height = std::clamp(WindowSize.y, 600, 4320);
            markDisplaySettingsChanged();
        }
        break;
    case SDL_EVENT_WINDOW_MAXIMIZED:
    case SDL_EVENT_WINDOW_RESTORED:
        if (!(SDL_GetWindowFlags(MainWindow) & SDL_WINDOW_FULLSCREEN))
        {
            displaySettings.mode = SDL_GetWindowFlags(MainWindow) & SDL_WINDOW_MAXIMIZED ? WindowMode::Maximized : WindowMode::Windowed;
            markDisplaySettingsChanged();
        }
        break;
    case SDL_EVENT_WINDOW_FOCUS_LOST:
        MapToolGesture = false;
        EventHandling::MouseLeftDown = false;
        RightButtonDrag = false;
        interfaceManager->injectMouseUp();
        break;
    case SDL_EVENT_WINDOW_CLOSE_REQUESTED: simExit(); break;
    default: break;
    }
}


void rebuildPresentationGraphics();

void pumpEvents()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        miniMapWindow->injectEvent(event);
        if (event.type == SDL_EVENT_RENDER_DEVICE_RESET || event.type == SDL_EVENT_RENDER_TARGETS_RESET)
        {
            rebuildPresentationGraphics();
            continue;
        }
        if (event.type == SDL_EVENT_RENDER_DEVICE_LOST)
            { throw std::runtime_error("The graphics device was lost. Restart Civic 89. If an autosave exists, recovery is available."); }
        if (event.window.windowID == MainWindowId && !SDL_ConvertEventToRenderCoordinates(MainWindowRenderer, &event))
            { throw std::runtime_error(SDL_GetError()); }

        if (event.type >= SDL_EVENT_WINDOW_FIRST && event.type <= SDL_EVENT_WINDOW_LAST)
        {
            handleWindowEvent(event);
            continue;
        }

        switch (event.type)
        {
        case SDL_EVENT_KEY_DOWN:
            handleKeyEvent(event);
            break;

        case SDL_EVENT_MOUSE_MOTION:
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        case SDL_EVENT_MOUSE_BUTTON_UP:
        case SDL_EVENT_MOUSE_WHEEL:
            handleMouseEvent(event);
            break;

        case SDL_EVENT_QUIT:
            simExit();
            break;
           
        default:
            break;
        }
    }
}


void initMainWindow()
{
    auto flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY;
#if defined(CIVIC89_MILESTONE_TESTS)
    flags |= SDL_WINDOW_HIDDEN;
#endif
	MainWindow = SDL_CreateWindow("Civic 89", 800, 600, flags);
    if (!MainWindow)
    {
        throw std::runtime_error("initRenderer(): Unable to create primary window: " + std::string(SDL_GetError()));
    }
    

	SDL_SetWindowPosition(MainWindow, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
}


void initRenderer()
{
    initMainWindow();

    MainWindowRenderer = SDL_CreateRenderer(MainWindow, nullptr);
    
    if (!MainWindowRenderer)
    {
        throw std::runtime_error("initRenderer(): Unable to create renderer: " + std::string(SDL_GetError()));
    }

    SDL_SetRenderDrawBlendMode(MainWindowRenderer, SDL_BLENDMODE_BLEND);

    MainWindowId = SDL_GetWindowID(MainWindow);
    applyVSync();
    diagnostics->write(Severity::Info, DiagnosticCode::Display, std::string("Renderer: ") + SDL_GetRendererName(MainWindowRenderer));
    applyDisplayMode(displaySettings.mode);
}


void initViewParamters()
{
    windowSize();

    camera.pixelPerfect(displaySettings.pixelPerfect);
}


std::optional<MapPreview> pendingToolPreview()
{
    if (interfaceManager->modalWindowVisible() || interfaceManager->pointInWindow(EventHandling::MousePosition) ||
        (EventHandling::MouseLeftDown && !MapToolGesture) ||
        (interfaceManager->pointInWindow(EventHandling::MouseDownPosition) && EventHandling::MouseLeftDown) ||
        !camera.containsWorld(camera.screenToWorld(EventHandling::SceneMousePosition - cameraShake))) { return {}; }
    const auto& tool = toolManager->currentTool();
    if (tool.type == Tool::Type::None) { return {}; }
    MapPreview preview{{(TilePointedAt.x - tool.offset) * 16.f, (TilePointedAt.y - tool.offset) * 16.f,
        tool.size * 16.f, tool.size * 16.f}, &interfaceManager->toolPalette().toolGost()};
    if (tool.draggable && EventHandling::MouseLeftDown)
    {
        const auto start = toolManager->dragStart();
        preview = {{start.x * 16.f, start.y * 16.f, 16, 16}, nullptr};
        const bool xAxis = std::abs(DraggableToolVector.x) > std::abs(DraggableToolVector.y);
        const int axis = xAxis ? DraggableToolVector.x : DraggableToolVector.y;
        const float size = (std::abs(axis) + 1) * 16.f;
        if (xAxis) { preview.worldRect.w = size; if (axis < 0) { preview.worldRect.x -= size - 16; } }
        else { preview.worldRect.h = size; if (axis < 0) { preview.worldRect.y -= size - 16; } }
    }
    return preview;
}


void gameInit(std::optional<Scenario> scenario)
{
    simInit();

    if (scenario)
    {
        if (doStartScenario(*scenario) != ScenarioResult::Success)
        {
            throw std::runtime_error("Unable to load the selected scenario file.");
        }
    }
    else { primeGame(-1, cityProperties, budget); }

    updateMapDrawParameters();
    scheduler.reset(SDL_GetTicks());
}


void optionsChanged(const GameOptions& options)
{
	gameplayOptions() = options;
    userSoundOn(options.soundEnabled);
    syncAudioOptions();
}


void registerCallbacks()
{
    registerNewMonthCallback([](int month) { interfaceManager->dashboardWindow().onNewMonth(month); });
    registerNewYearCallback([](int year) { interfaceManager->dashboardWindow().onNewYear(year); });

	auto& dashboard = interfaceManager->dashboardWindow();

	dashboard.registerButtonHandler(DashboardWindow::ButtonId::Budget, []() { showBudgetWindow(); });
	dashboard.registerButtonHandler(DashboardWindow::ButtonId::Evaluation, []() { showEvaluationWindow(); });
	dashboard.registerButtonHandler(DashboardWindow::ButtonId::MiniMap, []() { ToggleMiniMapVisibility(); });
	dashboard.registerButtonHandler(DashboardWindow::ButtonId::Graph, []() { interfaceManager->showWindow(InterfaceManager::Window::Graph); });
	dashboard.registerButtonHandler(DashboardWindow::ButtonId::Save, []() { saveGame(); });
	dashboard.registerButtonHandler(DashboardWindow::ButtonId::System, []() { showSystemWindow(); });
}


MiniMapWindow::EffectMapButtonMapping buildEffectMapMapping()
{
    return {
        { MiniMapWindow::ButtonId::Crime, CrimeMap},
        { MiniMapWindow::ButtonId::FireProtection, FireProtectionMap},
        { MiniMapWindow::ButtonId::LandValue, LandValueMap },
        { MiniMapWindow::ButtonId::PoliceProtection, PoliceProtectionMap },
        { MiniMapWindow::ButtonId::Pollution, PollutionMap },
        { MiniMapWindow::ButtonId::PopulationDensity, PopulationDensityMap },
        { MiniMapWindow::ButtonId::PopulationGrowth, RateOfGrowthMap },
        { MiniMapWindow::ButtonId::TrafficDensity, TrafficDensityMap }
    };
}


void initMinimap(const Point<int>& mainWindowPosition, const SDL_DisplayMode* mode)
{
    const Point<int> miniMapWindowPosition
    {
        std::clamp(mainWindowPosition.x - (SimWidth * MiniTileSize) - 10, 10, mode->w),
        std::clamp(mainWindowPosition.y, 10, mode->h)
    };

    miniMapWindow = std::make_unique<MiniMapWindow>(miniMapWindowPosition, Vector<int>{ SimWidth, SimHeight });
    miniMapWindow->updateViewportSize(camera.visibleWorld());
    miniMapWindow->focusOnMapCoordBind(&minimapViewUpdated);
    miniMapWindow->linkEffectMaps(buildEffectMapMapping());
}


void assertModeNotNull(const SDL_DisplayMode* mode)
{
    if (!mode)
    {
        throw std::runtime_error(std::string("initUI(): Unable to get desktop display mode: ") + SDL_GetError());
    }
}


void initUI()
{
    Point<int> mainWindowPosition{};
    SDL_GetWindowPosition(MainWindow, &mainWindowPosition.x, &mainWindowPosition.y);

    const auto mode = SDL_GetDesktopDisplayMode(SDL_GetPrimaryDisplay());
    assertModeNotNull(mode);

	initMinimap(mainWindowPosition, mode);

    interfaceManager = std::make_shared<InterfaceManager>(MainWindowRenderer, MainWindow, budget, currentRCI(), *toolManager);
	sharePresentationEvents(presentationEvents);
    shareAudioService(*audioService);
    setEngineClock([]() { return static_cast<int>(SDL_GetTicks()); });

    positionDashboardWindow();
    interfaceManager->positionWindow(InterfaceManager::Window::ToolPalette, ToolPaletteDefaultPosition);

    interfaceManager->optionsWindow().optionsChangedConnect(optionsChanged);
    interfaceManager->optionsWindow().newGameCallbackConnect(newGame);
    interfaceManager->optionsWindow().saveGameCallbackConnect(saveGame);
    interfaceManager->optionsWindow().openGameCallbackConnect(openGame);

	interfaceManager->toolPalette().toolChangedCallback([]() {
        interfaceManager->dashboardWindow().onToolChanged(interfaceManager->toolPalette().tool());
        });

    interfaceManager->fileIoDialog().errorHandler([](const std::string& message) {
        diagnostics->write(Severity::Error, DiagnosticCode::FileDialog, message);
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Civic 89 file picker", message.c_str(), MainWindow);
    });
    registerCallbacks();
}


void rebuildPresentationGraphics()
{
    const bool minimapVisible = miniMapWindow && !miniMapWindow->hidden();
    clearNewMonthCallbacks(); clearNewYearCallbacks();
    interfaceManager.reset();
    miniMapWindow.reset();
    cityRenderer.reset();
    EventHandling::MouseLeftDown = false;
    MapToolGesture = false;
    RightButtonDrag = false;
    toolManager->currentTool(Tool::Type::None);
    loadGraphics();
    initUI();
    interfaceManager->dashboardWindow().cityName(cityProperties.CityName());
    interfaceManager->dashboardWindow().onNewMonth((CityTime / 4) % 12);
    interfaceManager->dashboardWindow().onNewYear(CityTime / 48 + StartingYear);
    interfaceManager->dashboardWindow().setMessage("Graphics restored.");
    if (minimapVisible) { miniMapWindow->show(); }
    diagnostics->write(Severity::Info, DiagnosticCode::Display, "Presentation resources recreated after graphics reset");
}


void cleanUp()
{
    quake.cancel();
    cameraShake = {};
    camera = Camera2D{};
    EventHandling::MouseLeftDown = false;
    MapToolGesture = false;
    RightButtonDrag = false;
    panelDragRemainder = {};
    EventHandling::SceneMousePosition = {};
    EventHandling::MousePosition = {};
    EventHandling::MouseDownPosition = {};

    miniMapWindow.reset(nullptr);
    
    interfaceManager.reset();
    toolManager.reset();

    shareAudioService(silentAudio);
    sharePresentationEvents(silentPresentation);
    setEngineClock(nullptr);
    audioService.reset();
    ShutDownSound();
    destroyAllSprites();
    cityRenderer.reset();
    if (MainWindowRenderer) { SDL_DestroyRenderer(MainWindowRenderer); MainWindowRenderer = nullptr; }
    if (MainWindow) { SDL_DestroyWindow(MainWindow); MainWindow = nullptr; }

    clearNewMonthCallbacks();
	clearNewYearCallbacks();
}


void GameLoop()
{
    miniMapWindow->draw();
    cityRenderer->invalidate();
    FramePacer renderPacer;
    Uint64 previous = SDL_GetTicks();
    while (!Exit)
    {
        pumpEvents();
        const auto now = SDL_GetTicks();
        toolManager->currentTool(interfaceManager->toolPalette().tool());
        if (SDL_GetKeyboardFocus() == MainWindow && !interfaceManager->modalWindowVisible())
        {
            const auto* keys = SDL_GetKeyboardState(nullptr);
            Vector<float> direction{static_cast<float>(keys[SDL_SCANCODE_RIGHT] - keys[SDL_SCANCODE_LEFT]),
                static_cast<float>(keys[SDL_SCANCODE_DOWN] - keys[SDL_SCANCODE_UP])};
            const float length = std::hypot(direction.x, direction.y);
            if (length > 0) { camera.pan(direction / length * (static_cast<float>(std::min<Uint64>(now - previous, 50)) * .5f)); }
        }
        previous = now;
        advancePresentation(now);
        updateMapDrawParameters();
        calculateMouseToWorld(); updateDragVector();
        if (settingsDirty && now - settingsChangedAt >= 1000) { persistDisplaySettings(); }
        if (ScenarioID == 0 && now - lastAutosave >= 300000)
        {
            lastAutosave = now;
            const auto result = recovery->save(cityProperties, budget, fileStorage);
            diagnostics->write(result ? Severity::Info : Severity::Error, DiagnosticCode::Autosave,
                result ? "Recovery city saved" : result.detail, result.path);
            if (!result) { interfaceManager->dashboardWindow().setMessage("Autosave failed. Please save your city manually."); }
        }
        const auto* mode = SDL_GetCurrentDisplayMode(SDL_GetDisplayForWindow(MainWindow));
        const float refresh = vsyncActive && mode && mode->refresh_rate > 0 ? mode->refresh_rate : 60.f;
        if (renderPacer.ready(SDL_GetTicksNS(), refresh))
        {
            if (!(SDL_GetWindowFlags(MainWindow) & SDL_WINDOW_MINIMIZED))
            {
                cityRenderer->render(camera, cameraShake, pendingToolPreview());
                if (!interfaceManager->modalWindowVisible() && currentEvaluation().needsAttention)
                {
                    interfaceManager->evaluationWindow().setEvaluation(currentEvaluation());
                    currentEvaluationSeen();
                }
                interfaceManager->draw();
                SDL_RenderPresent(MainWindowRenderer);
                if (!miniMapWindow->hidden()) { miniMapWindow->drawUI(); }
                newMap(false);
            }
        }
        const auto deadline = std::min(scheduler.next() * 1000000, renderPacer.next());
        const auto after = SDL_GetTicksNS();
        if (deadline > after) { SDL_DelayNS(std::min<Uint64>(deadline - after, 10000000)); }
    }
    persistDisplaySettings();
}


int main(int argc, char* argv[])
{
    std::optional<Scenario> startupScenario;
    if (argc != 1)
    {
        int id = 0;
        bool valid = argc == 3 && std::string_view(argv[1]) == "--scenario";
        if (valid)
        {
            const std::string_view argument(argv[2]);
            const auto parsed = std::from_chars(argument.data(), argument.data() + argument.size(), id);
            valid = parsed.ec == std::errc{} && parsed.ptr == argument.data() + argument.size();
        }
        if (valid) { startupScenario = scenarioFromLegacyId(id); }
        if (!startupScenario)
        {
            std::cerr << "Usage: civic89.exe [--scenario 1..8]\n";
            return 2;
        }
    }
    setLocale();

    std::cout << "Starting Civic 89, based on Micropolis-SDLPP version " << MicropolisVersion << " originally by Will Wright and Don Hopkins." << std::endl;
    std::cout << "Original code Copyright (C) 2002 by Electronic Arts, Maxis. Released under the GPL v3" << std::endl;
    std::cout << "Modifications Copyright (C) 2022 - 2026 by Leeor Dicker. Available under the terms of the GPL v3" << std::endl << std::endl;

    std::cout << "Micropolis-SDLPP is not afiliated with Electronic Arts." << std::endl << std::endl;
    try
    {
#if defined(CIVIC89_MILESTONE_TESTS)
        userDirectory = std::filesystem::temp_directory_path() / ("civic89-session-" + std::to_string(GetCurrentProcessId()));
#else
        std::unique_ptr<char, SdlDeleter<char, SDL_free>> preferencePath(SDL_GetPrefPath("Civic89", "Civic89"));
        if (!preferencePath) { throw std::runtime_error("Unable to locate Civic 89 user data."); }
        userDirectory = pathFromUtf8(preferencePath.get());
#endif
        std::filesystem::create_directories(userDirectory);
        diagnostics = std::make_unique<DiagnosticLog>(userDirectory / "civic89.log");
        diagnostics->write(Severity::Info, DiagnosticCode::Startup, "Civic 89 starting");
        recovery = std::make_unique<RecoveryStore>(userDirectory);
        if (const auto settings = DisplaySettings::load(userDirectory / "display.cfg")) { displaySettings = *settings; }
        else if (std::filesystem::exists(userDirectory / "display.cfg"))
            { diagnostics->write(Severity::Warning, DiagnosticCode::Settings, "Invalid display settings; using defaults"); }
#if defined(CIVIC89_MILESTONE_TESTS)
        constexpr int sessions = 12;
#else
        constexpr int sessions = 1;
#endif
#if defined(CIVIC89_MILESTONE_TESTS)
        for (int stage = 0; stage < 3; ++stage)
        {
            try
            {
                struct FailedStartupLifetime
                {
                    ~FailedStartupLifetime() { cleanUp(); if (TTF_WasInit()) { TTF_Quit(); } SDL_Quit(); }
                } lifetime;
                if (!SDL_Init(SDL_INIT_VIDEO)) { throw std::runtime_error(SDL_GetError()); }
                audioService = std::make_unique<AudioManager>(*diagnostics);
                initRenderer();
                if (stage > 0) { loadGraphics(); }
                if (stage > 1)
                {
                    toolManager = std::make_shared<ToolManager>();
                    initViewParamters(); initUI();
                }
                throw std::runtime_error("M3 injected startup failure");
            }
            catch (const std::runtime_error& error)
            {
                if (std::string_view(error.what()) != "M3 injected startup failure") { throw; }
                if (SDL_WasInit(0) || TTF_WasInit() || MainWindow || MainWindowRenderer || audioService ||
                    interfaceManager || miniMapWindow || cityRenderer)
                    { throw std::runtime_error("Partial startup retained resources"); }
            }
        }
#endif
        for (int session = 0; session < sessions; ++session)
        {
            struct ApplicationLifetime
            {
                ~ApplicationLifetime() { cleanUp(); if (TTF_WasInit()) { TTF_Quit(); } SDL_Quit(); }
            } lifetime;
            if (!SDL_Init(SDL_INIT_VIDEO)) { throw std::runtime_error(std::string("Unable to initialize SDL: ") + SDL_GetError()); }
            audioService = std::make_unique<AudioManager>(*diagnostics);
            std::ifstream settings(userDirectory / "audio.cfg");
            float master, city, construction;
            if (settings >> master >> city >> construction)
            {
                audioService->masterVolume(master);
                audioService->channelVolume(AudioChannel::City, city);
                audioService->channelVolume(AudioChannel::Construction, construction);
            }
            initRenderer();
            loadGraphics();
            toolManager = std::make_shared<ToolManager>();
            initViewParamters();
            initUI();
#if defined(CIVIC89_MILESTONE_TESTS)
            void runMilestoneTests(Budget&, CityProperties&, PresentationEvents&);
            void runWindowCameraRenderingTests(Budget&, CityProperties&, PresentationEvents&, ToolManager&, InterfaceManager&, MiniMapWindow&);
            simInit();
            scheduler.reset(SDL_GetTicks());
            if (session == 0) { runWindowCameraRenderingTests(budget, cityProperties, presentationEvents, *toolManager, *interfaceManager, *miniMapWindow); }
            runMilestoneTests(budget, cityProperties, presentationEvents);
            const auto result = recovery->save(cityProperties, budget, fileStorage);
            if (!result || !LoadCityDetailed(recovery->path(), cityProperties, budget)) { throw std::runtime_error("Session save/reload failed"); }
#else
            gameInit(startupScenario);
            if (!startupScenario && recovery->available())
            {
                const std::array<SDL_MessageBoxButtonData,2> buttons{{{SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT,1,"Recover"}, {SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT,0,"Start new city"}}};
                const SDL_MessageBoxData box{SDL_MESSAGEBOX_INFORMATION, MainWindow, "Civic 89 recovery", "A previous autosave is available. Recover it?", 2, buttons.data(), nullptr};
                int selected = 0;
                if (SDL_ShowMessageBox(&box, &selected) && selected == 1)
                {
                    const auto result = LoadCityDetailed(recovery->path(), cityProperties, budget);
                    if (!result) { reportCityError(result, DiagnosticCode::CityLoad); }
                    else { syncAudioOptions(); interfaceManager->dashboardWindow().cityName(cityProperties.CityName()); }
                }
            }
            lastAutosave = SDL_GetTicks();
            GameLoop();
#endif
        }
#if defined(CIVIC89_MILESTONE_TESTS)
        diagnostics->write(Severity::Info, DiagnosticCode::Shutdown, "12 application sessions closed");
        diagnostics.reset();
        recovery.reset();
        std::filesystem::remove_all(userDirectory);
        std::cout << "12 complete SDL application sessions with new/scenario/save/load/quit passed\n";
#else
        diagnostics->write(Severity::Info, DiagnosticCode::Shutdown, "Civic 89 closed");
#endif
    }
    catch(const std::exception& error)
    {
        if (diagnostics) { diagnostics->write(Severity::Error, DiagnosticCode::Startup, error.what()); }
#if defined(CIVIC89_MILESTONE_TESTS)
        std::cerr << error.what() << '\n';
#else
        const std::string message = std::string(error.what()) + "\n\nCivic 89 will now close.";
        MessageBoxA(nullptr, message.c_str(), "Civic 89", MB_ICONERROR | MB_OK);
#endif
        return 1;
    }
    return 0;
}
