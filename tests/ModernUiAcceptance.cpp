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
#include "main.h"
#include <SDL3_image/SDL_image.h>
#include <filesystem>
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
}
void runModernUiAcceptance(Budget& budget,CityProperties& city,ToolManager& tools)
{
    const auto original=applicationUiSettings();
    const int originalTax=budget.TaxRate();
    budget.CurrentFunds(20000);ResetMap();
    auto& camera=applicationCamera();camera.position({});camera.zoomAt(1,{});
    applicationMapRenderer().invalidate();
    for(const auto pixels : {Vector<int>{800,600},Vector<int>{1366,768},Vector<int>{1920,1080},Vector<int>{3440,1440}})
    {
        require(SDL_SetWindowSize(MainWindow,pixels.x,pixels.y),"Responsive UI output resize");windowResized();
        for(float density : {1.f,1.25f,1.5f,2.f})
        {
            const auto layout=DisplayLayout::fromPixels(pixels.x,pixels.y,density);
            require(SDL_SetRenderScale(MainWindowRenderer,layout.scale,layout.scale),"UI content scale");
            camera.displayScale(layout.scale);camera.viewport(layout.logical);
            auto& ui=applicationInterface();ui.layout(layout.logical,layout.scale);ui.hideAllWindows();
            const auto before=engineStateDigest(city,budget);
            for(const auto panel : {ModernInterface::Panel::None,ModernInterface::Panel::Budget,ModernInterface::Panel::Evaluation,ModernInterface::Panel::Graphs,ModernInterface::Panel::Settings,ModernInterface::Panel::Overlays})
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
    snapshot("compact-overlay");
    require(engineStateDigest(city,budget)==before,"Overlay changed simulation");
    control("Settings");control("Controls");
    const auto oldKey=applicationUiSettings().keys[0];
    control(tools.tool(Tool::Type::Residential).name+": "+SDL_GetKeyName(oldKey));
    key(SDLK_C);require(applicationUiSettings().keys[0]==oldKey,"Conflicting binding accepted");
    key(SDLK_Z);require(applicationUiSettings().keys[0]==SDLK_Z,"Rebinding did not persist");
    snapshot("compact-controls");key(SDLK_ESCAPE);key(SDLK_Z);require(tools.currentTool().type==Tool::Type::Residential,"Configured hotkey route");
    control("Settings");control("Readability");control("Large text: off");control("High contrast: off");control("Blue overlays: off");snapshot("compact-accessibility");
    control("Sound");control("Master -");snapshot("compact-sound");
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
    std::cout<<"M5 responsive panels, tool affordability, minimap isolation, overlays, settings, rebinding, keyboard and graphics reset passed\n";
}
