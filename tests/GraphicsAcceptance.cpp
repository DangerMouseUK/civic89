// Civic 89 full-catalogue graphics acceptance. SPDX-License-Identifier: GPL-3.0-or-later
#include "GraphicsArt.h"
#include "Budget.h"
#include "CityProperties.h"
#include "EngineDigest.h"
#include "EngineStep.h"
#include "EngineServices.h"
#include "Sprite.h"
#include "g_ani.h"
#include "s_gen.h"
#include "MapRenderer.h"
#include "Map.h"
#include "ToolManager.h"
#include "Util.h"
#include "WindowsFileStorage.h"
#include "UI/ModernInterface.h"
#include "main.h"
#include <SDL3_image/SDL_image.h>
#include <nlohmann/json.hpp>
#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <unordered_set>

extern SDL_Window* MainWindow;
extern uint32_t MainWindowId;
void handleMouseEvent(SDL_Event&);
ModernInterface& applicationInterface();
UiSettings& applicationUiSettings();
void windowResized();
void rebuildPresentationGraphics();
uint64_t applicationScheduledDeadline();
CityIoResult applicationSaveCityTo(const std::filesystem::path&);
std::string applicationSaveDestination();
ScenarioResult doStartScenario(Scenario);

namespace
{
    void require(bool value,const char* why) { if (!value) { throw std::runtime_error(why); } }
    std::string bytes(const std::filesystem::path& path)
    {
        std::ifstream input(path,std::ios::binary);
        require(static_cast<bool>(input),"Missing graphics acceptance file");
        return {std::istreambuf_iterator<char>(input),{}};
    }
    uint32_t pixel(SDL_Surface* surface,int x,int y)
    {
        return reinterpret_cast<uint32_t*>(static_cast<Uint8*>(surface->pixels)+y*surface->pitch)[x];
    }
    void render()
    {
        auto& ui=applicationInterface();auto& settings=applicationUiSettings();
        applicationMapRenderer().render(applicationCamera(),{},{},ui.overlay(),settings.overlayOpacity,settings.accessibleColors);
        ui.draw(applicationCamera());
    }
    void capture(const std::filesystem::path& path)
    {
        render();SurfaceOwner pixels(SDL_RenderReadPixels(MainWindowRenderer,nullptr));
        require(pixels && IMG_SavePNG(pixels.get(),path.string().c_str()),"Graphics capture failed");
    }
    void catalogue()
    {
        const auto catalogue=nlohmann::json::parse(bytes("assets/graphics-catalogue.json"));
        require(catalogue.at("tile_count")==TILE_COUNT && catalogue.at("sprite_frames")==61 &&
            catalogue.at("inputs").size()==73,"Graphics catalogue coverage differs");
        SurfaceOwner source(IMG_Load("images/tiles.xpm"));
        require(source!=nullptr,"Missing pixel catalogue");
        size_t refined=0;
        for (int tile=0;tile<TILE_COUNT;++tile)
        {
            auto classic=refinePixelRegion(source.get(),{0,tile*16,16,16},GraphicsStyle::Classic);
            auto enhanced=refinePixelRegion(source.get(),{0,tile*16,16,16},GraphicsStyle::Enhanced);
            std::unordered_set<uint32_t> palette;
            for (int y=0;y<16;++y) for (int x=0;x<16;++x) { palette.insert(pixel(classic.get(),x,y)); }
            for (int y=0;y<32;++y) for (int x=0;x<32;++x)
            {
                const auto color=pixel(enhanced.get(),x,y);
                require(palette.contains(color),"Refinement invented colours or sampled another tile");
                if (color!=pixel(classic.get(),x/2,y/2)) { ++refined; }
            }
        }
        require(refined>1000,"Enhanced graphics are only nearest-neighbour enlargement");
        std::cout<<"960 complete tile palettes/regions passed; refined subpixels: "<<refined<<'\n';
    }
    void contactSheet(const std::filesystem::path& path,const GraphicsArt& art)
    {
        auto target=newTexture(MainWindowRenderer,{1024,1760});
        require(SDL_SetRenderTarget(MainWindowRenderer,target.texture),"Catalogue target");
        SDL_SetRenderDrawColor(MainWindowRenderer,24,28,30,255);SDL_RenderClear(MainWindowRenderer);
        const SDL_FRect source{0,0,512.f*art.density,480.f*art.density},destination{0,0,1024,960};
        require(SDL_RenderTexture(MainWindowRenderer,art.tiles.texture,&source,&destination),"Complete tile atlas");
        for (size_t type=0;type<art.sprites.size();++type) for (size_t frame=0;frame<art.sprites[type].size();++frame)
        {
            const auto& texture=art.sprites[type][frame];
            const float scale=std::min(58.f/texture.dimensions.x,90.f/texture.dimensions.y);
            const SDL_FRect rectangle{frame*60.f,968+type*96.f,texture.dimensions.x*scale,texture.dimensions.y*scale};
            require(SDL_RenderTexture(MainWindowRenderer,texture.texture,nullptr,&rectangle),"Complete sprite frames");
        }
        int index=0;
        for (const auto& texture : art.ghosts) if (texture.texture)
        {
            Uint8 alpha{};require(SDL_GetTextureAlphaMod(texture.texture,&alpha) && alpha==125,"Preview opacity changed");
            const float scale=std::min(96.f/texture.dimensions.x,112.f/texture.dimensions.y);
            const SDL_FRect rectangle{index++*102.f,1640,texture.dimensions.x*scale,texture.dimensions.y*scale};
            require(SDL_RenderTexture(MainWindowRenderer,texture.texture,nullptr,&rectangle),"Complete construction previews");
        }
        require(index==10,"Missing construction preview");
        SurfaceOwner output(SDL_RenderReadPixels(MainWindowRenderer,nullptr));
        require(SDL_SetRenderTarget(MainWindowRenderer,nullptr),"Restore catalogue target");
        require(output && IMG_SavePNG(output.get(),path.string().c_str()),"Catalogue capture");
    }
}

// Run in separate fresh application processes: inherited session globals are not replay checkpoints.
void runGraphicsParity(Budget& budget,CityProperties& city,const std::string& name)
{
    require(name=="classic" || name=="enhanced" || name=="switched","Unknown graphics parity mode");
    setEngineClock([] { return 0; });seedSimulationRandom(12345);
    require(doStartScenario(Scenario::Tokyo)==ScenarioResult::Success,"Graphics parity fixture");
    require(applyGraphicsStyle(name=="enhanced" ? GraphicsStyle::Enhanced : GraphicsStyle::Classic),"Parity style");
    applicationInterface().hideAllWindows();
    nlohmann::json trace=nlohmann::json::array();
    int animations=0;
    for (int tick=0;tick<256;++tick)
    {
        if (name=="switched" && (tick==32 || tick==96 || tick==160))
            { require(applyGraphicsStyle(tick==96 ? GraphicsStyle::Classic : GraphicsStyle::Enhanced),"Mid-session parity switch"); }
        if (tick==64) { budget.TaxRate(9); }
        stepEngine(city,budget);
        if (tick%3==0) { animateTiles();updateSprites();++animations; }
        applicationMapRenderer().invalidate();render();
        trace.push_back({engineStateDigest(city,budget),randomRange(0,1000000)});
    }
    WindowsFileStorage storage;
    const auto stem="graphics-parity-"+name;
    require(static_cast<bool>(SaveCity(stem+".cty",city,budget,storage)),"Parity .cty save");
    require(static_cast<bool>(ImportClassicCity(stem+".cty",city,budget)),"Parity city import");
    city.CityName("Graphics parity city");
    require(static_cast<bool>(SaveCity(stem+".c89",city,budget,storage)),"Parity .c89 save");
    std::ofstream output(stem+".json");output<<nlohmann::json{{"phases",256},{"animations",animations},{"trace",trace}};
    require(static_cast<bool>(output),"Parity trace publication");
}

void runGraphicsAcceptance(Budget& budget,CityProperties& city,ToolManager& tools)
{
    const auto root=std::filesystem::current_path()/"ui-captures";
    std::filesystem::create_directories(root);
    catalogue();
    require(doStartScenario(Scenario::Tokyo)==ScenarioResult::Success,"Graphics scenario fixture");
    for (int i=0;i<64;++i) { stepEngine(city,budget); }
    pause();
    auto& camera=applicationCamera();camera.focus({960,800});
    tools.currentTool(Tool::Type::Residential);
    WindowsFileStorage storage;
    for (const auto mode : {RulesetId::ClassicV1,RulesetId::EnhancedV1})
    {
        const auto path=root/(mode==RulesetId::ClassicV1 ? "m9-city.cty" : "m9-city.c89");
        if (mode==RulesetId::EnhancedV1)
            { require(static_cast<bool>(ImportClassicCity(root/"m9-city.cty",city,budget)),"Graphics .c89 import"); }
        tools.currentTool(Tool::Type::Residential);
        require(static_cast<bool>(applicationSaveCityTo(path)),"Graphics baseline publication");
        const auto original=bytes(path), destination=applicationSaveDestination();
        const auto digest=engineStateDigest(city,budget);
        const auto deadline=applicationScheduledDeadline();
        const auto position=camera.position();const auto zoom=camera.zoom();
        for (int toggle=0;toggle<8;++toggle)
        {
            const auto style=toggle%2 ? GraphicsStyle::Classic : GraphicsStyle::Enhanced;
            seedSimulationRandom(123456);const auto expected=randomRange(0,1000000);seedSimulationRandom(123456);
            const std::weak_ptr<const GraphicsArt> previous=applicationMapRenderer().art();
            require(applyGraphicsStyle(style),"Graphics switch failed");
            require(previous.expired(),"Graphics switch retained the old art resources");
            auto& ui=applicationInterface();ui.overlay(DataOverlay::Power);ui.show(ModernInterface::Panel::Settings);
            render();
            require(randomRange(0,1000000)==expected,"Graphics consumed simulation RNG");
            require(city.rulesetId()==mode && engineStateDigest(city,budget)==digest,"Graphics changed city state/identity");
            require(applicationScheduledDeadline()==deadline,"Graphics changed scheduler deadline");
            require(camera.position()==position && camera.zoom()==zoom,"Graphics changed camera");
            require(tools.currentTool().type==Tool::Type::Residential,"Graphics changed selected tool");
            require(applicationSaveDestination()==destination,"Graphics changed save destination");
            require(static_cast<bool>(SaveCity(path,city,budget,storage)) && bytes(path)==original,"Graphics changed save bytes");
            auto art=applicationMapRenderer().art();
            require(art->textureCount==73 && art->tiles.dimensions==Vector<int>{512*art->density,512*art->density},"Incomplete graphics resource set");
            for (size_t type=0;type<art->sprites.size();++type)
                { require(art->sprites[type].size()==static_cast<size_t>(GraphicsFrameCounts[type]),"Missing animation frame"); }
            ui.hideAllWindows();ui.overlay(DataOverlay::None);
        }
    }
    const auto before=engineStateDigest(city,budget);
    auto originalArt=applicationMapRenderer().art();
    for (const int allocation : {14,73})
    {
        failEnhancedGraphicsAfter(allocation);
        require(!applyGraphicsStyle(GraphicsStyle::Enhanced) && applicationMapRenderer().art()==originalArt &&
            engineStateDigest(city,budget)==before,"Partial art/minimap preparation damaged live session");
    }
    require(applicationInterface().message().find("unchanged")!=std::string::npos,"Graphics failure not explained");
    originalArt.reset();
    require(applyGraphicsStyle(GraphicsStyle::Enhanced),"Retry after partial failure");
    auto& ui=applicationInterface();ui.show(ModernInterface::Panel::Settings);ui.overlay(DataOverlay::Crime);
    const auto position=camera.position();
    rebuildPresentationGraphics();
    require(applicationMapRenderer().art()->style==GraphicsStyle::Enhanced &&
        applicationInterface().currentPanel()==ModernInterface::Panel::Settings && applicationInterface().overlay()==DataOverlay::Crime &&
        camera.position()==position && engineStateDigest(city,budget)==before,"Enhanced device reset lost presentation/city");
    failEnhancedGraphicsAfter(3);rebuildPresentationGraphics();
    require(applicationMapRenderer().art()->style==GraphicsStyle::Classic && engineStateDigest(city,budget)==before &&
        applicationInterface().message().find("Classic graphics")!=std::string::npos,"Reset did not recover Classic graphics");
    require(applyGraphicsStyle(GraphicsStyle::Enhanced),"Enhanced before minimap reset failure");
    failEnhancedGraphicsAfter(73);rebuildPresentationGraphics();
    require(applicationMapRenderer().art()->style==GraphicsStyle::Classic && engineStateDigest(city,budget)==before &&
        applicationInterface().message().find("Classic graphics")!=std::string::npos,"Minimap reset failure did not recover Classic");

    for (const auto style : {GraphicsStyle::Classic,GraphicsStyle::Enhanced})
    {
        require(applyGraphicsStyle(style),"Performance graphics selection");
        auto& active=applicationInterface();active.hideAllWindows();active.overlay(DataOverlay::None);
        const auto name=style==GraphicsStyle::Classic ? "classic" : "enhanced";
        auto art=applicationMapRenderer().art();
        contactSheet(root/(std::string("m9-")+name+"-catalogue.png"),*art);
        std::cout<<"Graphics "<<name<<": textures="<<art->textureCount<<", art-bytes="<<art->textureBytes
            <<", map/minimap-bytes="<<(1920ull*1600+360ull*300)*4*art->density*art->density<<'\n';
        for (const auto size : {Vector<int>{800,600},Vector<int>{1366,768},Vector<int>{1920,1080},Vector<int>{2560,1440},Vector<int>{3440,1440},Vector<int>{3840,2160}})
        {
            require(SDL_SetWindowSize(MainWindow,size.x,size.y),"Graphics output size");windowResized();
            std::vector<double> frames;
            for (const float scale : {1.f,1.25f,1.5f,2.f})
            {
                require(SDL_SetRenderScale(MainWindowRenderer,scale,scale),"Graphics density");
                camera.displayScale(scale);camera.viewport({size.x/scale,size.y/scale});
                active.layout(camera.viewport(),scale);
                for (const float zoomValue : {1.f,1.5f,3.f})
                {
                    camera.zoomAt(zoomValue,{});camera.focus({960,800});applicationMapRenderer().invalidate();
                    const auto begin=std::chrono::steady_clock::now();
                    render();require(SDL_RenderPresent(MainWindowRenderer),"Graphics presentation");
                    frames.push_back(std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-begin).count());
                    require(engineStateDigest(city,budget)==before,"Display/zoom matrix changed simulation");
                }
            }
            std::sort(frames.begin(),frames.end());
            std::cout<<"Graphics "<<name<<' '<<size.x<<'x'<<size.y<<" render ms median="<<frames[frames.size()/2]<<", p95="<<frames.back()<<'\n';
            // Uncapped repeated full-map redraws, separate from cold zoom/DPI changes above.
            int vsync{};SDL_GetRenderVSync(MainWindowRenderer,&vsync);SDL_SetRenderVSync(MainWindowRenderer,0);
            require(SDL_SetRenderScale(MainWindowRenderer,1,1),"Steady render scale");
            camera.displayScale(1);camera.viewport({static_cast<float>(size.x),static_cast<float>(size.y)});
            camera.zoomAt(1,{});camera.focus({960,800});active.layout(camera.viewport(),1);
            frames.clear();
            for (int frame=0;frame<35;++frame)
            {
                applicationMapRenderer().invalidate();
                const auto begin=std::chrono::steady_clock::now();render();
                require(SDL_RenderPresent(MainWindowRenderer),"Steady graphics presentation");
                if (frame>=5) { frames.push_back(std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-begin).count()); }
            }
            SDL_SetRenderVSync(MainWindowRenderer,vsync);
            std::sort(frames.begin(),frames.end());
            std::cout<<"Graphics "<<name<<' '<<size.x<<'x'<<size.y<<" steady full-redraw ms median="<<frames[15]<<", p95="<<frames[28]<<'\n';
        }
        require(SDL_SetWindowSize(MainWindow,1366,768),"Graphics capture size");windowResized();
        camera.pixelPerfect(false);camera.zoomAt(3,{});camera.focus({960,800});
        capture(root/(std::string("m9-")+name+"-city.png"));
        active.show(ModernInterface::Panel::Settings);capture(root/(std::string("m9-")+name+"-settings.png"));
        const auto control=active.controlArea(style==GraphicsStyle::Classic ? "Graphics: Classic" : "Graphics: Enhanced");
        require(control.w>0,"Graphics Settings button missing");
        SDL_Event click{};click.type=SDL_EVENT_MOUSE_BUTTON_DOWN;click.button.windowID=MainWindowId;click.button.button=SDL_BUTTON_LEFT;
        click.button.x=control.x+control.w/2;click.button.y=control.y+control.h/2;handleMouseEvent(click);
        click.type=SDL_EVENT_MOUSE_BUTTON_UP;handleMouseEvent(click);
        require(applicationMapRenderer().art()->style!=style,"Settings button did not switch graphics");
        require(applyGraphicsStyle(style),"Restore capture style");
        active.hideAllWindows();
        for (const auto overlay : {DataOverlay::Power,DataOverlay::Traffic,DataOverlay::Crime,DataOverlay::LandValue,DataOverlay::Pollution,DataOverlay::Population,
            DataOverlay::Fire,DataOverlay::Police,DataOverlay::Growth,DataOverlay::Transport,DataOverlay::Residential,DataOverlay::Commercial,DataOverlay::Industrial})
            { active.overlay(overlay);render();require(engineStateDigest(city,budget)==before,"Enhanced overlay changed city"); }
        active.overlay(DataOverlay::None);
    }
    for (int fixture=0;fixture<=8;++fixture)
    {
        if (fixture==0) { seedSimulationRandom(89);GenerateNewCity(city,budget); }
        else { require(doStartScenario(static_cast<Scenario>(fixture-1))==ScenarioResult::Success,"Graphics scenario coverage"); }
        const auto digest=engineStateDigest(city,budget);
        for (const auto style : {GraphicsStyle::Enhanced,GraphicsStyle::Classic})
        {
            require(applyGraphicsStyle(style),"New/scenario graphics switch");render();
            require(engineStateDigest(city,budget)==digest,"Graphics changed new/scenario city");
        }
    }
    require(applyGraphicsStyle(GraphicsStyle::Classic),"Restore original graphics after acceptance");
    std::cout<<"M9 catalogue, both city formats, RNG/deadlines, switches, failure/recovery and display matrix passed\n";
}
