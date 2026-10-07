// Civic 89 actual single-window UI acceptance. SPDX-License-Identifier: GPL-3.0-or-later
#include "UI/ModernInterface.h"
#include "Budget.h"
#include "DisplayLayout.h"
#include "DisplaySettings.h"
#include "CityProperties.h"
#include "EngineDigest.h"
#include "EngineStep.h"
#include "ScenarioData.h"
#include "s_sim.h"
#include "Evaluation.h"
#include "MapRenderer.h"
#include "Map.h"
#include "ToolManager.h"
#include "FileIo.h"
#include "main.h"
#include <SDL3_image/SDL_image.h>
#include <array>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>

extern SDL_Window* MainWindow;
extern uint32_t MainWindowId;
ModernInterface& applicationInterface();
UiSettings& applicationUiSettings();
void handleMouseEvent(SDL_Event&);
void handleKeyEvent(SDL_Event&);
void handleWindowEvent(SDL_Event&);
DisplaySettings applicationDisplaySettings();
void rebuildPresentationGraphics();
void windowResized();
void gameInit(std::optional<Scenario>);
void advancePresentation(Uint64);
CityIoResult applicationSaveCityTo(const std::filesystem::path&);
std::string applicationSaveDestination();

namespace
{
    void require(bool value,const char* message) {if(!value) throw std::runtime_error(message);}
    void render()
    {
        auto& ui=applicationInterface();auto& settings=applicationUiSettings();
        applicationMapRenderer().render(applicationCamera(),{},{},ui.overlay(),settings.overlayOpacity,settings.accessibleColors);
        ui.draw(applicationCamera());
    }
    void control(const std::string& name)
    {
        render();const auto rect=applicationInterface().controlArea(name);
        require(rect.w>0,"UI control missing");
        SDL_Event event{};event.type=SDL_EVENT_MOUSE_BUTTON_DOWN;event.button.windowID=MainWindowId;event.button.button=SDL_BUTTON_LEFT;
        event.button.x=rect.x+rect.w/2;event.button.y=rect.y+rect.h/2;handleMouseEvent(event);
        event.type=SDL_EVENT_MOUSE_BUTTON_UP;handleMouseEvent(event);
    }
    void key(SDL_Keycode code,SDL_Keymod modifiers=SDL_KMOD_NONE)
    {
        SDL_Event event{};event.type=SDL_EVENT_KEY_DOWN;event.key.windowID=MainWindowId;event.key.key=code;event.key.mod=modifiers;handleKeyEvent(event);
    }
    void snapshot(const std::string& name)
    {
        render();SurfaceOwner pixels(SDL_RenderReadPixels(MainWindowRenderer,nullptr));
        require(pixels!=nullptr,"UI render capture");
        const auto path=std::filesystem::current_path()/"ui-captures";
        std::filesystem::create_directories(path);
        require(IMG_SavePNG(pixels.get(),(path/(name+".png")).string().c_str()),"UI capture publication");
    }
    void requireAudioSettings(const std::filesystem::path& directory,std::array<float,3> expected)
    {
        std::ifstream file(directory/"audio.cfg");
        std::array<float,3> actual{};
        require(static_cast<bool>(file>>actual[0]>>actual[1]>>actual[2]),"Saved audio settings unreadable");
        for(size_t i=0;i<actual.size();++i)
            require(std::abs(actual[i]-expected[i])<.001f,"Audio settings did not persist after startup with an existing file");
    }
}
void prepareModernUiAcceptance(const std::filesystem::path& directory)
{
    // Model a returning player before the production startup reader opens this file.
    std::ofstream file(directory/"audio.cfg");
    file<<"0.8 0.7 0.6\n";
    require(static_cast<bool>(file),"Audio settings fixture creation failed");
}
void runModernUiAcceptance(Budget& budget,CityProperties& city,ToolManager& tools,const std::filesystem::path& settingsDirectory)
{
    const auto original=applicationUiSettings();
    const int originalTax=budget.TaxRate();
    budget.CurrentFunds(20000);ResetMap();
    auto& camera=applicationCamera();camera.position({});camera.zoomAt(1,{});
    applicationMapRenderer().invalidate();
    for(const auto pixels : {Vector<int>{800,600},Vector<int>{1366,768},Vector<int>{1920,1080},Vector<int>{2560,1440},Vector<int>{3440,1440},Vector<int>{3840,2160}})
    {
        require(SDL_SetWindowSize(MainWindow,pixels.x,pixels.y),"Responsive UI output resize");windowResized();
        for(float density : {1.f,1.25f,1.5f,2.f})
        {
            const auto layout=DisplayLayout::fromPixels(pixels.x,pixels.y,density);
            require(SDL_SetRenderScale(MainWindowRenderer,layout.scale,layout.scale),"UI content scale");
            camera.displayScale(layout.scale);camera.viewport(layout.logical);
            auto& ui=applicationInterface();ui.layout(layout.logical,layout.scale);ui.hideAllWindows();
            const auto before=engineStateDigest(city,budget);
            for(const auto panel : {ModernInterface::Panel::None,ModernInterface::Panel::Budget,ModernInterface::Panel::Evaluation,ModernInterface::Panel::Graphs,ModernInterface::Panel::Settings,ModernInterface::Panel::Overlays,ModernInterface::Panel::NewCity,ModernInterface::Panel::Files,ModernInterface::Panel::Scenarios})
            {
                ui.show(panel);render();
                const auto area=ui.panelArea();require(area.x>=0 && area.x+area.w<=layout.logical.x && area.y+area.h<=layout.logical.y-32,"Responsive panel clipped");
            }
            require(engineStateDigest(city,budget)==before,"UI rendering changed Classic state");
        }
    }
    require(SDL_SetWindowSize(MainWindow,800,600),"Compact UI restore");windowResized();
    applicationInterface().hideAllWindows();snapshot("compact-dashboard");
    key(SDLK_D);require(tools.currentTool().type==Tool::Type::Road,"Default tool hotkey route");
    const auto before=engineStateDigest(city,budget);
    applicationUiSettings().overlayOpacity=.6f;
    control("Increase overlay opacity");
    require(applicationInterface().overlay()==DataOverlay::None,"Opacity enabled an overlay by itself");
    const auto savedOpacity=UiSettings::load(settingsDirectory/"ui.cfg");
    require(savedOpacity && std::abs(savedOpacity->overlayOpacity-.7f)<.001f,"Dashboard opacity did not persist");
    control("Decrease overlay opacity");
    require(std::abs(applicationUiSettings().overlayOpacity-.6f)<.001f && engineStateDigest(city,budget)==before,"Opacity changed simulation state");
    control("Budget");require(applicationInterface().currentPanel()==ModernInterface::Panel::Budget,"Dashboard budget routing");
    control("Tax rate +");require(budget.TaxRate()==originalTax+1,"Typed budget edit");budget.TaxRate(originalTax);
    snapshot("compact-budget");control("Close");
    const float fireFunding=budget.FirePercent();budget.FirePercent(.5f);
    applicationInterface().show(ModernInterface::Panel::Budget);render();
    key(SDLK_TAB,SDL_KMOD_SHIFT);key(SDLK_RETURN);
    require(std::abs(budget.FirePercent()-.6f)<.001f,"Initial Shift+Tab did not select the final control");
    budget.FirePercent(fireFunding);key(SDLK_ESCAPE);
    control("Evaluation");snapshot("compact-evaluation");control("Close");
    control("Graphs");snapshot("compact-graphs");control("1 Residential");control("10 years");snapshot("compact-long-history");control("Close");
    require(engineStateDigest(city,budget)==before,"Information panels mutated simulation");
    applicationUiSettings().scale=1.5f;
    require(SDL_SetWindowSize(MainWindow,1200,900),"Magnified window resize");windowResized();
    SDL_Event resized{};resized.type=SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED;resized.window.windowID=MainWindowId;
    handleWindowEvent(resized);
    int outputWidth{},outputHeight{};SDL_GetRenderOutputSize(MainWindowRenderer,&outputWidth,&outputHeight);
    const auto native=DisplayLayout::fromPixels(outputWidth,outputHeight,SDL_GetWindowDisplayScale(MainWindow));
    require(applicationDisplaySettings().width==static_cast<int>(native.logical.x) && applicationDisplaySettings().height==static_cast<int>(native.logical.y),"UI magnification changed persisted window size");
    applicationUiSettings().scale=1;require(SDL_SetWindowSize(MainWindow,800,600),"Compact window restore");windowResized();
    auto& ui=applicationInterface();
    ui.show(ModernInterface::Panel::Overlays);control("Traffic");require(ui.overlay()==DataOverlay::Traffic,"Data selector route");
    control("Increase overlay opacity");
    require(std::abs(applicationUiSettings().overlayOpacity-.7f)<.001f,"Active overlay opacity control failed");
    snapshot("compact-overlay");
    require(engineStateDigest(city,budget)==before,"Overlay changed simulation");
    control("Settings");control("Controls");
    const auto oldKey=applicationUiSettings().keys[0];
    control(tools.tool(Tool::Type::Residential).name+": "+SDL_GetKeyName(oldKey));
    key(SDLK_C);require(applicationUiSettings().keys[0]==oldKey,"Conflicting binding accepted");
    key(SDLK_Z);require(applicationUiSettings().keys[0]==SDLK_Z,"Rebinding did not persist");
    snapshot("compact-controls");key(SDLK_ESCAPE);key(SDLK_Z);require(tools.currentTool().type==Tool::Type::Residential,"Configured hotkey route");
    control("Settings");control("Readability");control("Large text: off");control("High contrast: off");control("Blue overlays: off");snapshot("compact-accessibility");
    control("Sound");control("Master -");
    requireAudioSettings(settingsDirectory,{.7f,.7f,.6f});
    control("Master +");
    requireAudioSettings(settingsDirectory,{.8f,.7f,.6f});
    snapshot("compact-sound");
    control("Gameplay");snapshot("compact-gameplay");
    const bool autoGoto=gameplayOptions().autoGoto;
    control(autoGoto ? "Auto goto: on" : "Auto goto: off");
    require(gameplayOptions().autoGoto!=autoGoto,"Typed gameplay toggle route");
    control(autoGoto ? "Auto goto: off" : "Auto goto: on");
    key(SDLK_TAB);render();key(SDLK_RETURN); // Keyboard traversal activates a real UI control.
    ui.show(ModernInterface::Panel::Budget);ui.overlay(DataOverlay::Crime);ui.minimapShown(false);
    const auto beforeReset=engineStateDigest(city,budget);
    rebuildPresentationGraphics();
    auto& restored=applicationInterface();
    require(restored.currentPanel()==ModernInterface::Panel::Budget && restored.overlay()==DataOverlay::Crime && !restored.minimapShown(),"Graphics reset lost UI state");
    require(engineStateDigest(city,budget)==beforeReset,"Graphics reset altered Classic state");
    restored.hideAllWindows();restored.overlay(DataOverlay::None);restored.minimapShown(true);
    applicationUiSettings()=original;windowResized();render();
    const auto mini=restored.minimapArea();
    const auto funds=budget.CurrentFunds();const auto position=camera.position();
    SDL_Event event{};event.type=SDL_EVENT_MOUSE_BUTTON_DOWN;event.button.windowID=MainWindowId;event.button.button=SDL_BUTTON_LEFT;event.button.x=mini.x+mini.w*.9f;event.button.y=mini.y+mini.h*.9f;
    handleMouseEvent(event);event.type=SDL_EVENT_MOUSE_BUTTON_UP;handleMouseEvent(event);
    require(camera.position()!=position && budget.CurrentFunds()==funds,"Minimap navigation leaked into construction");
    tools.currentTool(Tool::Type::None);budget.CurrentFunds(0);key(SDLK_R);
    control(tools.tool(Tool::Type::Residential).name);
    require(tools.currentTool().type==Tool::Type::None,"Unaffordable tool remained enabled");budget.CurrentFunds(funds);
    budget.CurrentFunds(-50);key(SDLK_Q);require(tools.currentTool().type==Tool::Type::Query,"Debt disabled free inspection");budget.CurrentFunds(funds);
    tools.currentTool(Tool::Type::Road);render();
    const auto beforeGesture=engineStateDigest(city,budget);
    event.type=SDL_EVENT_MOUSE_BUTTON_DOWN;event.button.x=500;event.button.y=350;handleMouseEvent(event);
    key(SDLK_B);event.type=SDL_EVENT_MOUSE_BUTTON_UP;handleMouseEvent(event);
    require(engineStateDigest(city,budget)==beforeGesture,"Changing tools committed an earlier drag");
    require(SDL_SetWindowSize(MainWindow,1366,768),"Wide UI output resize");windowResized();snapshot("wide-dashboard");
    restored.show(ModernInterface::Panel::Settings);snapshot("wide-settings");restored.hideAllWindows();
    control("Scenarios");snapshot("wide-scenarios");control("Tokyo");
    for(int i=0;i<64;++i) stepEngine(city,budget);
    applicationInterface().invalidateMinimap();
    camera.focus({960,800});applicationMapRenderer().invalidate();snapshot("wide-city");
    restored.show(ModernInterface::Panel::Overlays);control("Pollution");snapshot("wide-pollution");
    for (const auto type : {DataOverlay::None,DataOverlay::Traffic,DataOverlay::Crime,DataOverlay::LandValue,DataOverlay::Pollution,DataOverlay::Population,DataOverlay::Power,DataOverlay::Fire,DataOverlay::Police,DataOverlay::Growth,DataOverlay::Transport,DataOverlay::Residential,DataOverlay::Commercial,DataOverlay::Industrial})
    {
        const auto digest=engineStateDigest(city,budget);
        restored.overlay(type);render();
        require(engineStateDigest(city,budget)==digest,"Data layer consumed Classic state");
    }
    restored.overlay(DataOverlay::None);key(SDLK_Q);
    event.type=SDL_EVENT_MOUSE_BUTTON_DOWN;event.button.x=600;event.button.y=350;handleMouseEvent(event);
    event.type=SDL_EVENT_MOUSE_BUTTON_UP;handleMouseEvent(event);
    require(restored.currentPanel()==ModernInterface::Panel::Query,"Query tool did not open inspection");snapshot("wide-query");restored.hideAllWindows();
    control("New city");require(restored.currentPanel()==ModernInterface::Panel::NewCity,"New city confirmation missing");snapshot("wide-new-city");control("Close");
    const auto modeBefore=city.rulesetId(); const auto stateBeforeMode=engineStateDigest(city,budget);
    for (const auto pixels : {Vector<int>{800,600},Vector<int>{1366,768}})
    {
        require(SDL_SetWindowSize(MainWindow,pixels.x,pixels.y),"M8 files UI resize");windowResized();
        control("New city");render();
        require(restored.controlArea("Classic mode").w==0 && restored.controlArea("Enhanced mode").w==0,
            "New-city workflow still offers alternate gameplay modes");
        snapshot(pixels.x==800 ? "m8-compact-new-city" : "m8-wide-new-city");control("Close");
        control("Files");render();
        const auto area=restored.panelArea();
        for (const auto* label : {"Open city","Save city","Import .cty copy","Export .cty copy"})
        {
            const auto controlArea=restored.controlArea(label);
            require(controlArea.w>0 && controlArea.h>0 && controlArea.y>=area.y &&
                controlArea.y+controlArea.h<=area.y+area.h,"Save compatibility control escaped the panel");
        }
        snapshot(pixels.x==800 ? "m8-compact-files" : "m8-wide-files");control("Close");
    }
    require(city.rulesetId()==modeBefore && engineStateDigest(city,budget)==stateBeforeMode,"Inspecting/cancelling files/new-city changed the session");
    require(static_cast<bool>(ImportClassicCity("scenarios/snro.666",city,budget)),"M7 import compatibility failed");
    control("New city");control("Close");
    require(city.rulesetId()==RulesetId::EnhancedV1,"Cancelling new city retagged an existing M7 city");
    control("Files");snapshot("m8-c89-files");control("Close");
    control("New city");control("Start new city");
    require(city.rulesetId()==RulesetId::ClassicV1 && restored.currentPanel()==ModernInterface::Panel::None,
        "New-city path did not return to the faithful default");
    snapshot("m8-original-gameplay");
    control("Budget");render();key(SDLK_TAB);
    key(SDLK_RETURN,SDL_KMOD_ALT);key(SDLK_ESCAPE,SDL_KMOD_CTRL);
    require(restored.currentPanel()==ModernInterface::Panel::Budget,"Windows chords activated or closed a panel");
    SDL_Event focusLost{};focusLost.type=SDL_EVENT_WINDOW_FOCUS_LOST;focusLost.window.windowID=MainWindowId;
    handleWindowEvent(focusLost);key(SDLK_RETURN);
    require(restored.currentPanel()==ModernInterface::Panel::Budget,"Restoring focus activated a stale control");
    control("Close");
    const auto savePath=std::filesystem::current_path()/"ui-captures"/std::filesystem::path(L"M8-city-\u57ce\u5e02.cty");
    const auto beforeSave=engineStateDigest(city,budget);
    require(static_cast<bool>(applicationSaveCityTo(savePath)),"Application Unicode save failed");
    const auto previousDestination=applicationSaveDestination();
    std::ifstream saved(savePath,std::ios::binary);
    const std::string originalBytes{std::istreambuf_iterator<char>(saved),{}};saved.close();
    require(!applicationSaveCityTo(savePath.parent_path()/"missing-m8-directory"/"city.cty"),"Save to missing directory succeeded");
    require(!applicationSaveCityTo(savePath.parent_path()/"wrong-format.c89"),"Save silently converted the active city");
    require(applicationSaveDestination()==previousDestination && engineStateDigest(city,budget)==beforeSave,
        "Failed Save As changed the active city or previous destination");
    require(static_cast<bool>(applicationSaveCityTo(pathFromUtf8(previousDestination))),"Retry at previous destination failed");
    std::ifstream retried(savePath,std::ios::binary);
    require(std::string{std::istreambuf_iterator<char>(retried),{}}==originalBytes,"Failed Save As changed existing save bytes");
    // Exercise the animation option against actual scheduler/tile behavior, with no
    // simulation deadline between the 100 ms and 150 ms observations.
    gameInit(std::nullopt);simSpeed(SimulationSpeed::Slow);
    tileValue(40,40)=Radar0|AnimatedBit;
    gameplayOptions().animationEnabled=false;
    const auto now=SDL_GetTicks();advancePresentation(now+100);
    const auto unanimated=engineStateDigest(city,budget);
    advancePresentation(now+150);
    require(maskedTileValue(tileValue(40,40))==Radar0 && engineStateDigest(city,budget)==unanimated,"Disabled animation advanced tiles/engine state");
    gameplayOptions().animationEnabled=true;advancePresentation(now+300);
    require(maskedTileValue(tileValue(40,40))!=Radar0,"Enabled animation did not advance radar tile");
    pause();
    std::cout<<"M5/M8 responsive panels, faithful city workflow, save destination isolation, Windows chords/focus, tools, overlays, settings and graphics reset passed\n";
}
