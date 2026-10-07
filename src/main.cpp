// This file is part of Micropolis-SDLPP
// Micropolis-SDLPP is based on Micropolis
//
// Copyright ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â© 2022 - 2026 Leeor Dicker
//
// Portions Copyright ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â© 1989-2007 Electronic Arts Inc.
//
// Micropolis-SDLPP is free software; you can redistribute it and/or modify
// it under the terms of the GNU GPLv3, with additional terms. See the README
// file, included in this distribution, for details.
#include "MapRenderer.h"
#include "BuildInfo.h"
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

#include "UI/ModernInterface.h"
#include "UI/FileIoDialog.h"

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
    bool packageSmokeTest{false};
    bool desktopAcceptance{false};
    constexpr auto TileSize = 16;



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
    UiSettings uiSettings = ModernInterface::defaultSettings();
    GraphicsSettings graphicsSettings;
    std::string graphicsFallback;

    Vector<int> WindowSize{};
    Vector<int> DraggableToolVector{};


    Point<int> TilePointedAt{};

    bool Exit{ false };
    bool RightButtonDrag{ false };
    bool MapToolGesture{ false };



    std::array<unsigned int, 5> SpeedModifierTable{ 0, 0, 50, 75, 95 };



    Budget budget{};
    CityProperties cityProperties{};
    RulesetId startupRuleset{RulesetId::ClassicV1};
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

    std::unique_ptr<FileIoDialog> fileDialog;
    std::unique_ptr<ModernInterface> interfaceManager;
	std::shared_ptr<ToolManager> toolManager;


    unsigned int speedModifier()
    {
        return SpeedModifierTable[static_cast<unsigned int>(simSpeed())];
    }

    void showBudgetIfNeeded()
    {
        if (!gameplayOptions().autoBudget && budget.NeedsAttention())
        {
			interfaceManager->show(ModernInterface::Panel::Budget);
        }
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
    interfaceManager->show(ModernInterface::Panel::Budget);
}


void simInit(RulesetId ruleset = RulesetId::ClassicV1)
{
    initializeEngine(cityProperties, budget, ruleset);
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
            if (!interfaceManager->modalWindowVisible() && !paused() && gameplayOptions().animationEnabled) { animateTiles(); updateSprites(); cityRenderer->invalidate(); }
            break;
        case FrameScheduler::Event::Blink:
            cityRenderer->toggleBlink();
            break;
        case FrameScheduler::Event::Minimap:
            break; // The in-window minimap refreshes its own presentation cache.
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
    if (interfaceManager) { interfaceManager->show(ModernInterface::Panel::Budget); }
}


void ApplicationPresentationEvents::showMessage(const std::string& message)
{
    if (interfaceManager) { interfaceManager->message(message); }
}

void ApplicationPresentationEvents::scenarioStarted(Scenario scenario)
{
    if (interfaceManager)
    {
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
        fileDialog->clearSaveFilename();
        interfaceManager->cancelTool();
        interfaceManager->invalidateMinimap();
        cityRenderer->invalidate();
    }
    return result;
}


void primeGame(const int startFlag, CityProperties& properties, Budget& cityBudget)
{
    switch (startFlag)
    {
    case -2: // Load a city
        if (LoadCity("filename", properties, cityBudget))
        {
            break;
        }
        // If load fails, simply create a new city
        [[fallthrough]];

    case -1:
        properties.GameLevel(0);
        properties.CityName("NoWhere");
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


void resetGame(RulesetId ruleset)
{
    simInit(ruleset);
    primeGame(-1, cityProperties, budget);
}


void newGame()
{
    fileDialog->clearSaveFilename();
    resetGame(RulesetId::ClassicV1);
    interfaceManager->invalidateMinimap();
    cityRenderer->invalidate();
}


void openGame()
{
    if (fileDialog->pickOpenFile())
    {
        const auto result = LoadCityDetailed(pathFromUtf8(fileDialog->openPath()), cityProperties, budget);
        if (!result) { reportCityError(result, DiagnosticCode::CityLoad); return; }
        fileDialog->clearSaveFilename();
        syncAudioOptions();
        interfaceManager->cancelTool();
        interfaceManager->hideAllWindows();
        updateDate();
        interfaceManager->invalidateMinimap();
        cityRenderer->invalidate();
    }
}


CityIoResult saveCityTo(const std::filesystem::path& destination)
{
    const auto result = SaveCity(destination, cityProperties, budget, fileStorage);
    if (result) { fileDialog->saveDestination(destination); }
    return result;
}

void saveGame()
{
    auto destination = pathFromUtf8(fileDialog->fullPath());
    if (!fileDialog->filePicked() || SDL_GetModState() & SDL_KMOD_SHIFT)
    {
        if (!fileDialog->pickSaveFile(cityProperties.rulesetId())) { return; }
        destination = pathFromUtf8(fileDialog->pickedSavePath());
    }

    const auto result = saveCityTo(destination);
    if (!result)
    {
        reportCityError(result, DiagnosticCode::CitySave);
    }
    else { diagnostics->write(Severity::Info, DiagnosticCode::CitySave, "City saved", result.path); }
}

void importClassicGame()
{
    if (!fileDialog->pickImportFile()) { return; }
    const auto result=ImportClassicCity(pathFromUtf8(fileDialog->openPath()),cityProperties,budget);
    if (!result) { reportCityError(result,DiagnosticCode::CityLoad); return; }
    fileDialog->clearSaveFilename();
    syncAudioOptions(); updateDate();
    interfaceManager->cancelTool(); interfaceManager->hideAllWindows();
    interfaceManager->invalidateMinimap(); cityRenderer->invalidate();
    interfaceManager->message("Imported a city copy. Save As .c89; the source file is unchanged.");
}

void exportClassicGame()
{
    if (!fileDialog->pickExportFile()) { return; }
    const auto result=ExportClassicCity(pathFromUtf8(fileDialog->exportPath()),cityProperties,budget,fileStorage);
    if (!result) { reportCityError(result,DiagnosticCode::CitySave); return; }
    diagnostics->write(Severity::Info,DiagnosticCode::CitySave,"Classic copy exported",result.path);
    interfaceManager->message("Exported a .cty copy. Your current city and save destination are unchanged.");
}


void loadGraphics()
{
    graphicsFallback.clear();
    try { cityRenderer = std::make_unique<MapRenderer>(MainWindowRenderer,graphicsSettings.style); }
    catch (const std::exception& error)
    {
        if (graphicsSettings.style==GraphicsStyle::Classic) { throw; }
        diagnostics->write(Severity::Warning,DiagnosticCode::Display,error.what());
        cityRenderer = std::make_unique<MapRenderer>(MainWindowRenderer,GraphicsStyle::Classic);
        graphicsSettings.style=GraphicsStyle::Classic;
        graphicsFallback="Enhanced graphics unavailable. Classic graphics are active.";
    }
}

bool applyGraphicsStyle(GraphicsStyle style)
{
    if (style!=GraphicsStyle::Classic && style!=GraphicsStyle::Enhanced) { return false; }
    if (cityRenderer->art()->style==style) { return true; }
    try
    {
        auto images=GraphicsArt::load(MainWindowRenderer,style);
        auto map=cityRenderer->prepareMap(style);
        auto minimap=interfaceManager->prepareMinimap(style);
        // No city/UI reconstruction and no scheduler reset. Both commits only swap ownership.
        cityRenderer->graphics(images,std::move(map));
        interfaceManager->graphics(std::move(images),std::move(minimap));
    }
    catch (const std::exception& error)
    {
        diagnostics->write(Severity::Warning,DiagnosticCode::Display,error.what());
        interfaceManager->message("Could not switch graphics. Your current graphics and city are unchanged.");
        return false;
    }
    graphicsSettings.style=style;
    const auto result=graphicsSettings.save(userDirectory/"graphics.cfg",fileStorage);
    if (!result)
    {
        diagnostics->write(Severity::Error,DiagnosticCode::Settings,result.detail,result.path);
        interfaceManager->message("Graphics changed for this session; could not save the preference.");
    }
    else { interfaceManager->message(style==GraphicsStyle::Enhanced ? "Enhanced graphics active. Same original gameplay." : "Classic graphics active. Same original gameplay."); }
    return true;
}


void windowSize()
{
    int width, height;
    if (!SDL_GetRenderOutputSize(MainWindowRenderer, &width, &height)) { throw std::runtime_error(SDL_GetError()); }
    if (width <= 0 || height <= 0) { return; }
    displayLayout = DisplayLayout::fromPixels(width, height, SDL_GetWindowDisplayScale(MainWindow) * uiSettings.scale);
    if (!SDL_SetRenderScale(MainWindowRenderer, displayLayout.scale, displayLayout.scale)) { throw std::runtime_error(SDL_GetError()); }
    WindowSize = {static_cast<int>(displayLayout.logical.x), static_cast<int>(displayLayout.logical.y)};
    camera.displayScale(displayLayout.scale);
    camera.viewport(displayLayout.logical);
    const float windowScale = SDL_GetWindowDisplayScale(MainWindow) / SDL_GetWindowPixelDensity(MainWindow);
    SDL_SetWindowMinimumSize(MainWindow, static_cast<int>(800 * windowScale), static_cast<int>(600 * windowScale));
}


void windowResized()
{
    if (SDL_GetWindowFlags(MainWindow) & SDL_WINDOW_MINIMIZED) { return; }
    windowSize();
    if (!interfaceManager) { return; }
    interfaceManager->layout(displayLayout.logical, displayLayout.scale);
}

void calculateMouseToWorld()
{
    const auto world = camera.screenToWorld(EventHandling::SceneMousePosition - cameraShake);
    TilePointedAt = {static_cast<int>(std::floor(world.x / TileSize)), static_cast<int>(std::floor(world.y / TileSize))};

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
    interfaceManager->toggleMinimap();
}


void showEvaluationWindow()
{
    interfaceManager->show(ModernInterface::Panel::Evaluation);
    currentEvaluationSeen();
}
void showSystemWindow() { interfaceManager->show(ModernInterface::Panel::Settings); }


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

void performUiCommand(UiCommand command)
{
    switch (command)
    {
    case UiCommand::Pause: TogglePause(); break;
    case UiCommand::Save: saveGame(); break;
    case UiCommand::Open: openGame(); break;
    case UiCommand::Minimap: ToggleMiniMapVisibility(); break;
    case UiCommand::Evaluation: showEvaluationWindow(); break;
    case UiCommand::Scenarios: interfaceManager->show(ModernInterface::Panel::Scenarios); break;
    case UiCommand::NewCity: interfaceManager->show(ModernInterface::Panel::NewCity); break;
    case UiCommand::Settings: showSystemWindow(); break;
    case UiCommand::Graphs: interfaceManager->show(ModernInterface::Panel::Graphs); break;
    case UiCommand::Budget: showBudgetWindow(); break;
    case UiCommand::StartNewCity: newGame(); break;
    case UiCommand::ImportClassic: importClassicGame(); break;
    case UiCommand::ExportClassic: exportClassicGame(); break;
    case UiCommand::Files: interfaceManager->show(ModernInterface::Panel::Files); break;
    default: break;
    }
}
void handleKeyEvent(SDL_Event& event)
{
    if (event.key.windowID != MainWindowId || event.key.repeat) { return; }
    const auto toolBefore = toolManager->currentTool().type;
    if (interfaceManager->keyDown(event.key.key, event.key.mod))
    {
        if (toolManager->currentTool().type != toolBefore || interfaceManager->modalWindowVisible()) { MapToolGesture = false; }
        return;
    }
    if (event.key.mod & (SDL_KMOD_CTRL | SDL_KMOD_ALT | SDL_KMOD_GUI)) { return; }
    switch (event.key.key)
    {
    case SDLK_0: TogglePause(); break;
    case SDLK_1: SetSpeed(SimulationSpeed::Slow); break;
    case SDLK_2: SetSpeed(SimulationSpeed::Normal); break;
    case SDLK_3: SetSpeed(SimulationSpeed::Fast); break;
    case SDLK_4: SetSpeed(SimulationSpeed::AfricanSwallow); break;
    case SDLK_F11: applyDisplayMode(displaySettings.mode == WindowMode::Borderless ? WindowMode::Windowed : WindowMode::Borderless); break;
    case SDLK_F12: showSystemWindow(); break;
    case SDLK_HOME: camera.focus({SimWidth * 8.f, SimHeight * 8.f}); break;
    default: break;
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
        interfaceManager->mouseMotion(EventHandling::MousePosition, EventHandling::MouseLeftDown, camera);
        if ((event.motion.state & SDL_BUTTON_RMASK) && !interfaceManager->pointInWindow(EventHandling::MousePosition))
        {
            camera.pan({-event.motion.xrel, -event.motion.yrel});
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
        if (interfaceManager->wheel(event.wheel.y) || EventHandling::MouseLeftDown ||
            interfaceManager->pointInWindow(EventHandling::MousePosition)) { break; }
        const float direction = (event.wheel.direction == SDL_MOUSEWHEEL_FLIPPED ? -event.wheel.y : event.wheel.y) * (uiSettings.reverseZoom ? -1.f : 1.f);
        const float zoom = camera.pixelPerfect() ? camera.zoom() + direction / displayLayout.scale :
            camera.zoom() * std::pow(1.125f, direction);
        camera.zoomAt(zoom, {event.wheel.mouse_x, event.wheel.mouse_y});
        calculateMouseToWorld();
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
            if (interfaceManager->mouseDown(EventHandling::MousePosition, camera) || interfaceManager->modalWindowVisible()) { return; }
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
                interfaceManager->show(ModernInterface::Panel::Query);
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
            interfaceManager->mouseUp();
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
            if (!RightButtonDrag) { toolManager->currentTool(Tool::Type::None); interfaceManager->cancelTool(); }
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
            // Window preferences use native logical units, independent of UI magnification.
            int width{}, height{};
            SDL_GetRenderOutputSize(MainWindowRenderer, &width, &height);
            const auto nativeLayout = DisplayLayout::fromPixels(width, height, SDL_GetWindowDisplayScale(MainWindow));
            displaySettings.width = std::clamp(static_cast<int>(nativeLayout.logical.x), 800, 7680);
            displaySettings.height = std::clamp(static_cast<int>(nativeLayout.logical.y), 600, 4320);
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
        interfaceManager->focusLost();
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
    if (packageSmokeTest) { flags |= SDL_WINDOW_HIDDEN; }
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
        tool.size * 16.f, tool.size * 16.f}, interfaceManager->ghost()};
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
    simInit(startupRuleset);

    if (scenario)
    {
        if (doStartScenario(*scenario) != ScenarioResult::Success)
        {
            throw std::runtime_error("Unable to load the selected scenario file.");
        }
    }
    else { primeGame(-1, cityProperties, budget); }

    scheduler.reset(SDL_GetTicks());
}


void optionsChanged(const GameOptions& options)
{
	gameplayOptions() = options;
    userSoundOn(options.soundEnabled);
    syncAudioOptions();
}


void persistUiSettings()
{
    const auto result = uiSettings.save(userDirectory / "ui.cfg", fileStorage);
    std::ostringstream values;
    values << audioService->masterVolume() << ' ' << audioService->channelVolume(AudioChannel::City) << ' ' << audioService->channelVolume(AudioChannel::Construction);
    const auto bytes = values.str();
    const auto audioResult = fileStorage.write(userDirectory / "audio.cfg", std::span<const char>(bytes.data(), bytes.size()));
    if (!result || !audioResult)
    {
        const auto& failure = !result ? result : audioResult;
        diagnostics->write(Severity::Error, DiagnosticCode::Settings, failure.detail, failure.path);
        interfaceManager->message("Could not save settings. Changes apply for this session.");
    }
    windowResized();
}
void initUI()
{
    if (!fileDialog)
    {
        fileDialog = std::make_unique<FileIoDialog>(*MainWindow);
        fileDialog->errorHandler([](const std::string& message) {
            diagnostics->write(Severity::Error, DiagnosticCode::FileDialog, message);
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Civic 89 file picker", message.c_str(), MainWindow);
        });
    }
    const auto createInterface=[] {
        return std::make_unique<ModernInterface>(MainWindowRenderer,budget,currentRCI(),*toolManager,cityProperties,*audioService,uiSettings,displaySettings,cityRenderer->art());
    };
    try { interfaceManager=createInterface(); }
    catch (const std::exception& error)
    {
        if (graphicsSettings.style!=GraphicsStyle::Enhanced) { throw; }
        diagnostics->write(Severity::Warning,DiagnosticCode::Display,error.what());
        interfaceManager.reset();cityRenderer.reset();
        graphicsSettings.style=GraphicsStyle::Classic;
        loadGraphics();
        graphicsFallback="Enhanced graphics unavailable. Classic graphics are active.";
        interfaceManager=createInterface();
    }
    interfaceManager->command = performUiCommand;
    interfaceManager->scenarioSelected = [](int id) {
        const auto scenario = scenarioFromLegacyId(id);
        if (!scenario || doStartScenario(*scenario) != ScenarioResult::Success)
            { interfaceManager->message("Could not start the scenario. The current city is unchanged."); }
    };
    interfaceManager->settingsChanged = persistUiSettings;
    interfaceManager->graphicsChanged = [](GraphicsStyle style) { applyGraphicsStyle(style); };
    interfaceManager->optionsChanged = [] { optionsChanged(gameplayOptions()); };
    interfaceManager->displayAction = [](int action) {
        if (action <= 2) { applyDisplayMode(static_cast<WindowMode>(action)); }
        else if (action == 3) { displaySettings.vsync = !displaySettings.vsync; applyVSync(); markDisplaySettingsChanged(); }
        else { displaySettings.pixelPerfect = !displaySettings.pixelPerfect; camera.pixelPerfect(displaySettings.pixelPerfect); markDisplaySettingsChanged(); }
        persistDisplaySettings();
    };
    interfaceManager->layout(displayLayout.logical,displayLayout.scale);
    sharePresentationEvents(presentationEvents);
    shareAudioService(*audioService);
    setEngineClock([] { return static_cast<int>(SDL_GetTicks()); });
    if (!graphicsFallback.empty()) { interfaceManager->message(graphicsFallback); }
}
ModernInterface& applicationInterface() { return *interfaceManager; }
UiSettings& applicationUiSettings() { return uiSettings; }
#if defined(CIVIC89_MILESTONE_TESTS)
CityIoResult applicationSaveCityTo(const std::filesystem::path& path) { return saveCityTo(path); }
std::string applicationSaveDestination() { return fileDialog->filePicked() ? fileDialog->fullPath() : ""; }
DisplaySettings applicationDisplaySettings() { return displaySettings; }
uint64_t applicationScheduledDeadline() { return scheduler.next(); }
#endif
void rebuildPresentationGraphics()
{
    const auto panel = interfaceManager->currentPanel();
    const auto overlay = interfaceManager->overlay();
    const bool minimap = interfaceManager->minimapShown();
    const auto message = interfaceManager->message();
    interfaceManager.reset(); cityRenderer.reset();
    EventHandling::MouseLeftDown = false; MapToolGesture = false; RightButtonDrag = false;
    loadGraphics(); initUI();
    interfaceManager->show(panel); interfaceManager->overlay(overlay);
    interfaceManager->minimapShown(minimap);
    interfaceManager->message(graphicsFallback.empty() ? message : graphicsFallback);
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
    EventHandling::SceneMousePosition = {};
    EventHandling::MousePosition = {};
    EventHandling::MouseDownPosition = {};

    fileDialog.reset();
    
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
    cityRenderer->invalidate();
    FramePacer renderPacer;
    Uint64 previous = SDL_GetTicks();
    while (!Exit)
    {
        pumpEvents();
        const auto now = SDL_GetTicks();
        if (SDL_GetKeyboardFocus() == MainWindow && !interfaceManager->modalWindowVisible())
        {
            const auto* keys = SDL_GetKeyboardState(nullptr);
            Vector<float> direction{static_cast<float>(keys[SDL_GetScancodeFromKey(uiSettings.keys[27], nullptr)] - keys[SDL_GetScancodeFromKey(uiSettings.keys[26], nullptr)]),
                static_cast<float>(keys[SDL_GetScancodeFromKey(uiSettings.keys[29], nullptr)] - keys[SDL_GetScancodeFromKey(uiSettings.keys[28], nullptr)])};
            const float length = std::hypot(direction.x, direction.y);
            if (length > 0) { camera.pan(direction / length * (static_cast<float>(std::min<Uint64>(now - previous, 50)) * uiSettings.panSpeed / 1000.f)); }
        }
        previous = now;
        advancePresentation(now);
        calculateMouseToWorld(); updateDragVector();
        if (settingsDirty && now - settingsChangedAt >= 1000) { persistDisplaySettings(); }
        if (ScenarioID == 0 && now - lastAutosave >= 300000)
        {
            lastAutosave = now;
            const auto result = recovery->save(cityProperties, budget, fileStorage);
            diagnostics->write(result ? Severity::Info : Severity::Error, DiagnosticCode::Autosave,
                result ? "Recovery city saved" : result.detail, result.path);
            if (!result) { interfaceManager->message("Autosave failed. Please save your city manually."); }
        }
        const auto* mode = SDL_GetCurrentDisplayMode(SDL_GetDisplayForWindow(MainWindow));
        const float refresh = vsyncActive && mode && mode->refresh_rate > 0 ? mode->refresh_rate : 60.f;
        if (renderPacer.ready(SDL_GetTicksNS(), refresh))
        {
            if (!(SDL_GetWindowFlags(MainWindow) & SDL_WINDOW_MINIMIZED))
            {
                cityRenderer->render(camera, cameraShake, pendingToolPreview(), interfaceManager->overlay(), uiSettings.overlayOpacity, uiSettings.accessibleColors);
                if (!interfaceManager->modalWindowVisible() && currentEvaluation().needsAttention)
                {
                    currentEvaluationSeen();
                }
                interfaceManager->draw(camera);
                SDL_RenderPresent(MainWindowRenderer);
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
    if (argc == 2 && std::string_view(argv[1]) == "--version")
    {
        std::cout << Civic89BuildIdentity << '\n';
        return 0;
    }
    if (argc == 2 && std::string_view(argv[1]) == "--smoke-test")
    {
        packageSmokeTest = true;
        startupScenario = Scenario::Detroit;
    }
    else if (argc == 2 && std::string_view(argv[1]) == "--desktop-test")
    {
        desktopAcceptance = true;
        startupScenario = Scenario::Detroit;
    }
    else
    {
        bool valid=true, modeSet=false;
        for (int i=1;i<argc && valid;++i)
        {
            const std::string_view option(argv[i]);
            if (i+1==argc) { valid=false; break; }
            const std::string_view value(argv[++i]);
            if (option=="--mode" && !modeSet)
            {
                const auto ruleset=rulesetFromMode(value);
                valid=ruleset.has_value();
                if (ruleset) { startupRuleset=*ruleset; }
                modeSet=true;
            }
            else if (option=="--scenario" && !startupScenario)
            {
                int id=0;
                const auto parsed=std::from_chars(value.data(),value.data()+value.size(),id);
                valid=parsed.ec==std::errc{} && parsed.ptr==value.data()+value.size();
                if (valid) { startupScenario=scenarioFromLegacyId(id); valid=startupScenario.has_value(); }
            }
            else { valid=false; }
        }
        if (!valid || (startupScenario && startupRuleset!=RulesetId::ClassicV1))
        {
            std::cerr << "Usage: civic89.exe [--mode classic|enhanced] [--scenario 1..8]\n"
                "       civic89.exe --version | --smoke-test | --desktop-test\n"
                "--mode is a legacy save-format selector; both use original gameplay.\n"
                "Scenarios require --mode classic.\n";
            return 2;
        }
    }
    setLocale();

    std::cout << "Starting " << Civic89BuildIdentity << ", based on Micropolis-SDLPP " << MicropolisVersion << " originally by Will Wright and Don Hopkins." << std::endl;
    std::cout << "Original code Copyright (C) 2002 by Electronic Arts, Maxis. Released under the GPL v3" << std::endl;
    std::cout << "Modifications Copyright (C) 2022 - 2026 by Leeor Dicker. Available under the terms of the GPL v3" << std::endl << std::endl;

    std::cout << "Micropolis-SDLPP is not afiliated with Electronic Arts." << std::endl << std::endl;
    try
    {
        // Windows shortcuts and portable launches can have an unrelated cwd.
        // The retained comparison build can still use repository-relative assets.
#if !defined(CIVIC89_MILESTONE_TESTS)
        const auto* base = SDL_GetBasePath();
        if (!base) { throw std::runtime_error("Unable to locate Civic 89 runtime assets."); }
        const auto runtimeDirectory = pathFromUtf8(base);
        if (std::filesystem::exists(runtimeDirectory / "res" / "tools.json"))
            { std::filesystem::current_path(runtimeDirectory); }
#if defined(CIVIC89_CMAKE_BUILD)
        else { throw std::runtime_error("Runtime assets are missing beside civic89.exe."); }
#endif
#endif
#if defined(CIVIC89_MILESTONE_TESTS)
        userDirectory = std::filesystem::temp_directory_path() / ("civic89-session-" + std::to_string(GetCurrentProcessId()));
#else
        if (packageSmokeTest || desktopAcceptance)
        {
            userDirectory = std::filesystem::temp_directory_path() /
                (std::string(desktopAcceptance ? "civic89-desktop-test-" : "civic89-package-test-") +
                    std::to_string(GetCurrentProcessId()) + "-" + std::to_string(GetTickCount64()));
        }
        else
        {
            std::unique_ptr<char, SdlDeleter<char, SDL_free>> preferencePath(SDL_GetPrefPath("Civic89", "Civic89"));
            if (!preferencePath) { throw std::runtime_error("Unable to locate Civic 89 user data."); }
            userDirectory = pathFromUtf8(preferencePath.get());
        }
#endif
        struct SmokeDirectoryLifetime
        {
            ~SmokeDirectoryLifetime()
            {
                if (packageSmokeTest)
                {
                    diagnostics.reset(); recovery.reset();
                    std::error_code ignored;
                    std::filesystem::remove_all(userDirectory, ignored);
                }
            }
        } smokeLifetime;
        std::filesystem::create_directories(userDirectory);
        diagnostics = std::make_unique<DiagnosticLog>(userDirectory / "civic89.log");
        diagnostics->write(Severity::Info, DiagnosticCode::Startup, Civic89BuildIdentity);
        if (desktopAcceptance)
        {
            diagnostics->write(Severity::Info, DiagnosticCode::Startup,"Desktop acceptance: isolated preferences/recovery",userDirectory);
            std::cout << "Desktop test data: " << pathUtf8(userDirectory) << '\n';
        }
        recovery = std::make_unique<RecoveryStore>(userDirectory);
        if (const auto settings = UiSettings::load(userDirectory / "ui.cfg"))
        {
            bool valid = true;
            for (const auto key : settings->keys) { if (ModernInterface::reservedKey(key) || !*SDL_GetKeyName(key)) { valid = false; } }
            if (valid) { uiSettings = *settings; }
            else { diagnostics->write(Severity::Warning, DiagnosticCode::Settings, "Reserved UI key; using defaults"); }
        }
        else if (std::filesystem::exists(userDirectory / "ui.cfg"))
            { diagnostics->write(Severity::Warning, DiagnosticCode::Settings, "Invalid UI settings; using defaults"); }
        if (const auto settings = DisplaySettings::load(userDirectory / "display.cfg")) { displaySettings = *settings; }
        else if (std::filesystem::exists(userDirectory / "display.cfg"))
            { diagnostics->write(Severity::Warning, DiagnosticCode::Settings, "Invalid display settings; using defaults"); }
        if (const auto settings=GraphicsSettings::load(userDirectory/"graphics.cfg")) { graphicsSettings=*settings; }
        else if (std::filesystem::exists(userDirectory/"graphics.cfg"))
            { diagnostics->write(Severity::Warning,DiagnosticCode::Settings,"Invalid graphics preference; using Classic"); }
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
                    interfaceManager || fileDialog || cityRenderer)
                    { throw std::runtime_error("Partial startup retained resources"); }
            }
        }
        void prepareModernUiAcceptance(const std::filesystem::path&);
        prepareModernUiAcceptance(userDirectory);
#endif
        for (int session = 0; session < sessions; ++session)
        {
            struct ApplicationLifetime
            {
                ~ApplicationLifetime() { cleanUp(); if (TTF_WasInit()) { TTF_Quit(); } SDL_Quit(); }
            } lifetime;
            if (!SDL_Init(SDL_INIT_VIDEO)) { throw std::runtime_error(std::string("Unable to initialize SDL: ") + SDL_GetError()); }
            audioService = std::make_unique<AudioManager>(*diagnostics);
            {
                // Release the reader before settings can be atomically replaced on Windows.
                std::ifstream settings(userDirectory / "audio.cfg");
                float master, city, construction;
                if (settings >> master >> city >> construction)
                {
                    audioService->masterVolume(master);
                    audioService->channelVolume(AudioChannel::City, city);
                    audioService->channelVolume(AudioChannel::Construction, construction);
                }
            }
            initRenderer();
            loadGraphics();
            toolManager = std::make_shared<ToolManager>();
            initViewParamters();
            initUI();
#if defined(CIVIC89_MILESTONE_TESTS)
            void runMilestoneTests(Budget&, CityProperties&, PresentationEvents&);
            void runWindowCameraRenderingTests(Budget&, CityProperties&, PresentationEvents&, ToolManager&, ModernInterface&);
            void runModernUiAcceptance(Budget&, CityProperties&, ToolManager&, const std::filesystem::path&);
            void runGraphicsAcceptance(Budget&, CityProperties&, ToolManager&);
            simInit();
            scheduler.reset(SDL_GetTicks());
            if (const auto* parity=SDL_getenv("CIVIC89_GRAPHICS_PARITY"))
            {
                void runGraphicsParity(Budget&,CityProperties&,const std::string&);
                runGraphicsParity(budget,cityProperties,parity);
                break;
            }
            if (session == 0) { runModernUiAcceptance(budget, cityProperties, *toolManager, userDirectory); }
            if (session == 0) { runWindowCameraRenderingTests(budget, cityProperties, presentationEvents, *toolManager, *interfaceManager); }
            if (session == 0) { runGraphicsAcceptance(budget,cityProperties,*toolManager); }
            runMilestoneTests(budget, cityProperties, presentationEvents);
            const auto result = recovery->save(cityProperties, budget, fileStorage);
            if (!result || !LoadCityDetailed(recovery->path(), cityProperties, budget)) { throw std::runtime_error("Session save/reload failed"); }
#else
            gameInit(startupScenario);
            if (packageSmokeTest)
            {
                cityRenderer->render(camera);
                interfaceManager->draw(camera);
                if (!SDL_RenderPresent(MainWindowRenderer)) { throw std::runtime_error(SDL_GetError()); }
                const auto saved = SaveCity(userDirectory / "smoke.cty", cityProperties, budget, fileStorage);
                if (!saved || !LoadCityDetailed(saved.path, cityProperties, budget))
                    { throw std::runtime_error("Packaged city save/reload failed"); }
                std::cout << "Packaged startup, scenario, render, save/reload and shutdown passed\n";
                const auto imported=ImportClassicCity(saved.path,cityProperties,budget);
                const auto enhancedPath=userDirectory/"smoke.c89";
                if (!imported || !SaveCity(enhancedPath,cityProperties,budget,fileStorage) ||
                    !LoadCityDetailed(enhancedPath,cityProperties,budget) || cityProperties.rulesetId()!=RulesetId::EnhancedV1 ||
                    !ExportClassicCity(userDirectory/"export.cty",cityProperties,budget,fileStorage))
                    { throw std::runtime_error("Packaged Enhanced import/save/reload/export failed"); }
                cityRenderer->render(camera); interfaceManager->draw(camera);
                if (!SDL_RenderPresent(MainWindowRenderer)) { throw std::runtime_error(SDL_GetError()); }
                std::cout << "Enhanced ruleset, tagged save/load and Classic export passed\n";
                auto bytes=[](const std::filesystem::path& path) {
                    std::ifstream file(path,std::ios::binary);
                    if (!file) { throw std::runtime_error("Cannot inspect graphics smoke save"); }
                    return std::string{std::istreambuf_iterator<char>(file),{}};
                };
                if (!SaveCity(enhancedPath,cityProperties,budget,fileStorage)) { throw std::runtime_error("Graphics baseline save failed"); }
                const auto original=bytes(enhancedPath);
                for (const auto style : {GraphicsStyle::Enhanced,GraphicsStyle::Classic})
                {
                    if (!applyGraphicsStyle(style)) { throw std::runtime_error("Packaged graphics switch failed"); }
                    cityRenderer->render(camera);interfaceManager->draw(camera);
                    if (!SDL_RenderPresent(MainWindowRenderer) || !SaveCity(enhancedPath,cityProperties,budget,fileStorage) ||
                        bytes(enhancedPath)!=original) { throw std::runtime_error("Graphics changed packaged save bytes"); }
                }
                std::cout << "Classic/Enhanced graphics, identical city bytes and safe switching passed\n";
            }
            else if (!startupScenario)
            {
                const auto candidate=recovery->latest();
                if (candidate)
                {
                    RulesetId identity{};
                    const auto inspected=InspectCity(*candidate,identity);
                    if (!inspected) { diagnostics->write(Severity::Warning,DiagnosticCode::CityLoad,inspected.detail,inspected.path); }
                    else
                    {
                        const std::string prompt="A city autosave (" + std::string(findRuleset(identity)->extension) + ") is available. Recover it?";
                        const std::array<SDL_MessageBoxButtonData,2> buttons{{{SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT,1,"Recover"}, {SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT,0,"Start new city"}}};
                        const SDL_MessageBoxData box{SDL_MESSAGEBOX_INFORMATION, MainWindow, "Civic 89 recovery", prompt.c_str(), 2, buttons.data(), nullptr};
                        int selected = 0;
                        if (SDL_ShowMessageBox(&box, &selected) && selected == 1)
                        {
                            const auto result = LoadCityDetailed(*candidate, cityProperties, budget);
                            if (!result) { reportCityError(result, DiagnosticCode::CityLoad); }
                            else { syncAudioOptions(); }
                        }
                    }
                }
            }
            lastAutosave = SDL_GetTicks();
            if (!packageSmokeTest) { GameLoop(); }
#endif
        }
#if defined(CIVIC89_MILESTONE_TESTS)
        const bool graphicsParity=SDL_getenv("CIVIC89_GRAPHICS_PARITY")!=nullptr;
        diagnostics->write(Severity::Info, DiagnosticCode::Shutdown, graphicsParity ? "Graphics parity application closed" : "12 application sessions closed");
        diagnostics.reset();
        recovery.reset();
        std::filesystem::remove_all(userDirectory);
        if (!graphicsParity) { std::cout << "12 complete SDL application sessions with new/scenario/save/load/quit passed\n"; }
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
        if (packageSmokeTest) { std::cerr << message << '\n'; }
        else { MessageBoxA(nullptr, message.c_str(), "Civic 89", MB_ICONERROR | MB_OK); }
#endif
        return 1;
    }
    return 0;
}
