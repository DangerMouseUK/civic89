// Civic 89 single-window interface. SPDX-License-Identifier: GPL-3.0-or-later
#include "ModernInterface.h"
#include "../AudioManager.h"
#include "../Budget.h"
#include "../CityProperties.h"
#include "../Evaluation.h"
#include "../GameOptions.h"
#include "../Map.h"
#include "../Month.h"
#include "../RCI.h"
#include "../ScenarioData.h"
#include "../ToolManager.h"
#include "../s_alloc.h"
#include "../s_sim.h"
#include "../w_update.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace
{
    constexpr SDL_Color ink{231,238,245,255}, muted{165,182,196,255}, accent{88,211,192,255};
    constexpr std::array<int,16> iconRows{0,1,2,3,4,5,6,7,8,9,12,13,14,15,16,17};
    constexpr std::array<const char*,16> ghostFiles{"res","com","ind","fire",nullptr,"police",nullptr,nullptr,nullptr,nullptr,"stadium",nullptr,"seaport","coal","nuclear","airport"};
    constexpr std::array<const char*,10> commandNames{"Pause","Save","Open","Minimap","Evaluation","Scenarios","New city","Settings","Graphs","Budget"};
    constexpr std::array<const char*,4> panNames{"Pan left","Pan right","Pan up","Pan down"};
    bool contains(SDL_FRect rect, Point<int> point)
    {
        return point.x >= rect.x && point.y >= rect.y && point.x < rect.x + rect.w && point.y < rect.y + rect.h;
    }
    std::string dollars(int value) { return "$" + std::to_string(value); }
    SDL_FRect inset(SDL_FRect rect, float value) { return {rect.x + value, rect.y + value, rect.w - value * 2, rect.h - value * 2}; }
}

UiSettings ModernInterface::defaultSettings()
{
    UiSettings value;
    value.keys = {SDLK_R,SDLK_C,SDLK_I,SDLK_F,SDLK_Q,SDLK_L,SDLK_W,SDLK_B,SDLK_T,SDLK_D,SDLK_S,SDLK_K,SDLK_H,SDLK_O,SDLK_N,SDLK_A,
        SDLK_SPACE,SDLK_F2,SDLK_F3,SDLK_F4,SDLK_F5,SDLK_F6,SDLK_F7,SDLK_F8,SDLK_F9,SDLK_F10,
        SDLK_LEFT,SDLK_RIGHT,SDLK_UP,SDLK_DOWN};
    return value;
}
bool ModernInterface::reservedKey(SDL_Keycode key)
{
    return !key || key == SDLK_ESCAPE || key == SDLK_TAB || key == SDLK_RETURN || key == SDLK_F11 ||
        key == SDLK_F12 || key == SDLK_HOME || (key >= SDLK_0 && key <= SDLK_4) ||
        key == SDLK_LSHIFT || key == SDLK_RSHIFT || key == SDLK_LCTRL || key == SDLK_RCTRL ||
        key == SDLK_LALT || key == SDLK_RALT || key == SDLK_LGUI || key == SDLK_RGUI;
}
ModernInterface::ModernInterface(SDL_Renderer* value, Budget& b, const RCI& r, ToolManager& t,
    const CityProperties& c, AudioManager& a, UiSettings& prefs, const DisplaySettings& native) : renderer(value), budget(b), rci(r), tools(t), city(c), audio(a), settings(prefs), display(native),
    icons(loadTexture(value,"icons/buttons.png")), miniTiles(loadTexture(value,"images/tilessm.xpm")), miniCache(newTexture(value,{360,300}))
{
    if (!TTF_WasInit() && !TTF_Init()) { throw std::runtime_error(SDL_GetError()); }
    for (size_t i = 0; i < ghosts.size(); ++i)
    {
        if (!ghostFiles[i]) { continue; }
        ghosts[i] = loadTexture(renderer, std::string("images/") + ghostFiles[i] + ".xpm");
        SDL_SetTextureBlendMode(ghosts[i].texture, SDL_BLENDMODE_BLEND);
        SDL_SetTextureAlphaMod(ghosts[i].texture, 125);
    }
    SDL_SetTextureScaleMode(icons.texture, SDL_SCALEMODE_NEAREST);
    SDL_SetTextureScaleMode(miniCache.texture, SDL_SCALEMODE_NEAREST);
    layout(size, density);
}
void ModernInterface::layout(Vector<float> viewport, float scale)
{
    const bool geometryChanged = size != viewport || density != scale;
    size = viewport; sidebar = std::clamp(size.x * .20f, 184.f, 240.f);
    const float fontSize = (settings.largeText ? 16.f : 14.f) * scale;
    // Regenerate glyphs at physical pixel density, keeping the UI in logical coordinates.
    if (!normalFont || density != scale || std::abs(TTF_GetFontSize(normalFont.get()) - fontSize) > .01f)
    {
        density = scale;
        textCache.clear();
        normalFont.reset(TTF_OpenFont("res/Raleway-Medium.ttf", fontSize));
        boldFont.reset(TTF_OpenFont("res/Raleway-Bold.ttf", fontSize));
        if (!normalFont || !boldFont) { throw std::runtime_error(std::string("Unable to load UI fonts: ") + SDL_GetError()); }
    }
    controls.clear(); if (geometryChanged) { focus = -1; }
}
void ModernInterface::fill(SDL_FRect rect, SDL_Color color)
{
    SDL_SetRenderDrawColor(renderer,color.r,color.g,color.b,color.a); SDL_RenderFillRect(renderer,&rect);
}
void ModernInterface::text(std::string_view value, SDL_FRect rect, bool bold, SDL_Color color)
{
    if (value.empty() || rect.w <= 0 || rect.h <= 0) { return; }
    const std::string key = std::string(bold ? "B" : "N") + std::string(value);
    if (textCache.size() >= 512 && !textCache.contains(key)) { textCache.clear(); }
    auto it = textCache.find(key);
    if (it == textCache.end())
    {
        SurfaceOwner pixels(TTF_RenderText_Blended(bold ? boldFont.get() : normalFont.get(), value.data(), value.size(), {255,255,255,255}));
        if (!pixels) { throw std::runtime_error(SDL_GetError()); }
        Texture texture = buildTexture(SDL_CreateTextureFromSurface(renderer,pixels.get()));
        if (!texture.texture) { throw std::runtime_error(SDL_GetError()); }
        SDL_SetTextureScaleMode(texture.texture, SDL_SCALEMODE_LINEAR);
        it = textCache.emplace(key,std::move(texture)).first;
    }
    auto& texture = it->second;
    SDL_SetTextureColorMod(texture.texture,color.r,color.g,color.b);
    SDL_FRect destination{rect.x,rect.y + std::max(0.f,(rect.h - texture.area.h / density) / 2),texture.area.w / density,texture.area.h / density};
    const bool clipped = SDL_RenderClipEnabled(renderer);
    SDL_Rect previous{}; SDL_GetRenderClipRect(renderer,&previous);
    SDL_Rect clip{static_cast<int>(rect.x),static_cast<int>(rect.y),static_cast<int>(rect.w),static_cast<int>(rect.h)};
    if (clipped) { SDL_GetRectIntersection(&clip,&previous,&clip); }
    SDL_SetRenderClipRect(renderer,&clip);
    SDL_RenderTexture(renderer,texture.texture,nullptr,&destination);
    SDL_SetRenderClipRect(renderer,clipped ? &previous : nullptr);
}
void ModernInterface::button(std::string label, SDL_FRect rect, std::function<void()> action, bool active, bool enabled, const char* caption)
{
    const bool hovered = contains(rect,pointer);
    fill(rect,active ? SDL_Color{31,99,96,255} : hovered && enabled ? SDL_Color{56,77,91,255} : SDL_Color{35,48,62,255});
    text(caption ? caption : label,inset(rect,4),active,enabled ? ink : muted);
    if (hovered) { tooltip = label; }
    controls.push_back({rect,std::move(label),enabled,std::move(action)});
}
SDL_FRect ModernInterface::panelArea() const
{
    const float width = std::min(740.f,size.x - 40), height = std::min(470.f,size.y - 150);
    return {(size.x - width) / 2,118,width,height};
}
SDL_FRect ModernInterface::minimapArea() const
{
    const float width = std::min(sidebar - 24, (size.y - 482) * 1.2f), height = width / 1.2f;
    return {(sidebar - width) / 2,size.y - 34 - height,width,height};
}
SDL_FRect ModernInterface::controlArea(const std::string& label) const
{
    for (const auto& control : controls) { if (control.label == label) { return control.area; } }
    return {};
}
void ModernInterface::dashboard()
{
    fill({0,0,size.x,108},settings.highContrast ? SDL_Color{0,0,0,255} : SDL_Color{20,30,43,255});
    text("CIVIC 89",{14,8,100,30},true,accent);
    text(city.CityName(),{120,8,std::max(90.f,size.x - 625),30},true);
    const float x = size.x - 495;
    text("Funds " + dollars(budget.CurrentFunds()),{x,8,160,30},true);
    text("Pop. " + std::to_string(std::max(0,cityPopulation())),{x+166,8,145,30});
    text(Month::toString(static_cast<Month::Enum>((CityTime / 4) % 12)) + " " + std::to_string(CityTime / 48 + StartingYear),{x+318,8,165,30});
    constexpr std::array<size_t,10> commands{2,1,6,5,9,4,8,7,3,0};
    const float width = (size.x - 28) / commands.size();
    for (size_t i = 0; i < commands.size(); ++i)
    {
        const auto id = commands[i];
        const std::string label = id == 0 ? (paused() ? "Resume" : "Pause") : commandNames[id];
        button(label,{14 + i * width,43,width-4,29},[this,id] { if (command) { command(static_cast<UiCommand>(id)); } },false,true,id==4 ? "Eval" : id==5 ? "Scenario" : nullptr);
    }
    button(std::string(OverlayNames[static_cast<size_t>(selectedOverlay)]),{14,77,150,26},[this] { show(Panel::Overlays); },selectedOverlay != DataOverlay::None);
    button("-",{170,77,26,26},[this] { settings.overlayOpacity = std::max(.1f,settings.overlayOpacity-.1f); if(settingsChanged) settingsChanged(); });
    text(std::to_string(static_cast<int>(std::round(settings.overlayOpacity*100))) + "%",{201,77,40,26});
    button("+",{244,77,26,26},[this] { settings.overlayOpacity = std::min(1.f,settings.overlayOpacity+.1f); if(settingsChanged) settingsChanged(); });
    const auto& tool = tools.currentTool();
    text(tool.type == Tool::Type::None ? "Select a tool" : tool.name + " " + dollars(tool.cost),{282,77,220,26});
    text("R " + std::to_string(rci.residentialDemand()) + "   C " + std::to_string(rci.commercialDemand()) + "   I " + std::to_string(rci.industrialDemand()),{size.x-286,77,274,26},false,accent);
}
void ModernInterface::selectTool(size_t i)
{
    const auto& tool = tools.tool(static_cast<Tool::Type>(i));
    if (tool.cost > 0 && !budget.CanAfford(tool.cost)) { message("Insufficient funds for " + tool.name); return; }
    tools.currentTool(tool.type);
}
void ModernInterface::cancelTool() { tools.currentTool(Tool::Type::None); }
const Texture* ModernInterface::ghost() const
{
    const auto index = static_cast<size_t>(tools.currentTool().type);
    return index < ghosts.size() ? &ghosts[index] : nullptr;
}
void ModernInterface::palette()
{
    fill({0,108,sidebar,size.y-140},settings.highContrast ? SDL_Color{0,0,0,255} : SDL_Color{20,30,43,245});
    const float width = (sidebar - 24) / 2;
    for (size_t i = 0; i < 16; ++i)
    {
        const auto& tool = tools.tool(static_cast<Tool::Type>(i));
        const SDL_FRect rect{12+(i%2)*width,116+(i/2)*38.f,width-4,34};
        const bool affordable = tool.cost == 0 || budget.CanAfford(tool.cost), active = tools.currentTool().type == tool.type;
        button(tool.name,rect,[this,i] { selectTool(i); },active,affordable,"");
        const SDL_FRect source{0,iconRows[i]*32.f,32,32}, target{rect.x+3,rect.y+3,28,28};
        SDL_SetTextureAlphaMod(icons.texture,affordable ? 255 : 90);
        SDL_RenderTexture(renderer,icons.texture,&source,&target);
        text(SDL_GetKeyName(settings.keys[i]),{rect.x+35,rect.y+2,rect.w-39,30},false,affordable ? ink : muted);
        if (contains(rect,pointer) || focus == static_cast<int>(controls.size())-1)
            { tooltip = tool.name + " | " + dollars(tool.cost) + " | " + SDL_GetKeyName(settings.keys[i]) + (affordable ? "" : " | Insufficient funds"); }
    }
}
void ModernInterface::minimap(const Camera2D& camera)
{
    if (!minimapVisible) { return; }
    const auto rect = minimapArea();
    text("CITY OVERVIEW",{12,rect.y-26,sidebar-24,22},true,muted);
    const auto now = SDL_GetTicks();
    if (minimapDirty || now-minimapUpdated >= 1000)
    {
        if (!SDL_SetRenderTarget(renderer,miniCache.texture)) { throw std::runtime_error(SDL_GetError()); }
        for (int x=0;x<SimWidth;++x) for (int y=0;y<SimHeight;++y)
        {
            const SDL_FRect source{0,maskedTileValue(tileValue(x,y))*3.f,3,3}, target{x*3.f,y*3.f,3,3};
            SDL_RenderTexture(renderer,miniTiles.texture,&source,&target);
            const auto color = overlayColor(selectedOverlay,overlayValue(selectedOverlay,{x,y}),settings.accessibleColors);
            if (color.a) { fill(target,{color.r,color.g,color.b,static_cast<Uint8>(settings.overlayOpacity*255)}); }
        }
        SDL_SetRenderTarget(renderer,nullptr);
        minimapUpdated = now; minimapDirty = false;
    }
    SDL_RenderTexture(renderer,miniCache.texture,nullptr,&rect);
    const auto position = camera.position(); const auto visible = camera.visibleWorld();
    SDL_FRect selector{rect.x+position.x/1920*rect.w,rect.y+position.y/1600*rect.h,
        std::min(visible.x/1920*rect.w,rect.w),std::min(visible.y/1600*rect.h,rect.h)};
    SDL_SetRenderDrawColor(renderer,255,255,255,255); SDL_RenderRect(renderer,&selector);
}
void ModernInterface::show(Panel value)
{
    if (value==Panel::NewCity) { selectedRuleset=city.rulesetId(); }
    panel = value; focus = -1; binding = -1; minimapDragging = false; controls.clear();
}
void ModernInterface::budgetPanel(SDL_FRect area)
{
    text("Funds " + dollars(budget.CurrentFunds()) + "   Previous " + dollars(budget.PreviousFunds()),{area.x,area.y,area.w,28},true);
    text("Tax income " + dollars(budget.TaxIncome()) + "   Cash flow " + dollars(budget.CashFlow()),{area.x,area.y+30,area.w,28});
    constexpr std::array<const char*,4> names{"Tax rate","Road funding","Police funding","Fire funding"};
    for (int i=0;i<4;++i)
    {
        const float y = area.y + 70 + i*47;
        const float percent = i==1 ? budget.RoadPercent() : i==2 ? budget.PolicePercent() : budget.FirePercent();
        const int needed = i==1 ? budget.RoadFundsNeeded() : i==2 ? budget.PoliceFundsNeeded() : budget.FireFundsNeeded();
        const int granted = i==1 ? budget.RoadFundsGranted() : i==2 ? budget.PoliceFundsGranted() : budget.FireFundsGranted();
        text(std::string(names[i]),{area.x,y,160,30},true);
        text(i==0 ? std::to_string(budget.TaxRate())+"%" : std::to_string(static_cast<int>(std::round(percent*100)))+"%  "+dollars(granted)+" / "+dollars(needed),{area.x+165,y,area.w-255,30});
        for (int direction : {-1,1})
        {
            button(std::string(names[i]) + (direction<0 ? " -" : " +"),{area.x+area.w-84+(direction>0 ? 44 : 0),y,40,30},[this,i,direction] {
                if (i==0) { budget.TaxRate(std::clamp(budget.TaxRate()+direction,Budget::MinTaxRate,Budget::MaxTaxRate)); }
                else
                {
                    const float value = std::clamp((i==1 ? budget.RoadPercent() : i==2 ? budget.PolicePercent() : budget.FirePercent()) + direction*Budget::FundingRateStep,0.f,1.f);
                    if(i==1) budget.RoadPercent(value); else if(i==2) budget.PolicePercent(value); else budget.FirePercent(value);
                }
            },false,true,direction<0 ? "-" : "+");
        }
    }
    text("Changes apply immediately. Closing resumes the simulation.",{area.x,area.y+268,area.w,30},false,muted);
}
void ModernInterface::evaluationPanel(SDL_FRect area)
{
    const auto& e = currentEvaluation();
    text("Approval " + e.goodyes + " yes / " + e.goodno + " no",{area.x,area.y,area.w,28},true);
    text("Score " + e.score + "    Change " + e.changed,{area.x,area.y+34,area.w,28},true,accent);
    const std::array<std::string,5> rows{"Population "+e.pop+"    Migration "+e.delta,"Assessed value "+e.assessed_dollars,"City class "+e.cityclass,"Difficulty "+e.citylevel,"Leading concerns"};
    for(size_t i=0;i<rows.size();++i) text(rows[i],{area.x,area.y+76+i*29.f,area.w,28},i==4);
    for(size_t i=0;i<4;++i) text(e.problemString[i]+"   "+e.problemVote[i],{area.x+16,area.y+221+i*28.f,area.w-16,27});
}
void ModernInterface::graphsPanel(SDL_FRect area)
{
    constexpr std::array<const char*,6> names{"Residential","Commercial","Industrial","Pollution","Crime","Cash flow"};
    const std::array<const GraphHistory*,6> histories = longHistory ? std::array<const GraphHistory*,6>{&ResHis120Years,&ComHis120Years,&IndHis120Years,&PollutionHis120Years,&CrimeHis120Years,&MoneyHis120Years} : std::array<const GraphHistory*,6>{&ResidentialPopulationHistory,&CommercialPopulationHistory,&IndustrialPopulationHistory,&PollutionHistory,&CrimeHistory,&MoneyHis};
    constexpr std::array<SDL_Color,6> colors{{{88,211,192,255},{105,170,240,255},{245,190,75,255},{200,130,240,255},{235,110,85,255},{225,225,230,255}}};
    const float width=area.w/3;
    for(size_t i=0;i<6;++i) button(std::to_string(i+1)+" "+names[i],{area.x+(i%3)*width,area.y+(i/3)*30.f,width-4,27},[this,i] {graphVisible[i]=!graphVisible[i];},graphVisible[i]);
    button(longHistory ? "120 years" : "10 years",{area.x,area.y+65,120,28},[this] {longHistory=!longHistory;});
    const SDL_FRect plot{area.x+30,area.y+112,area.w-40,std::max(100.f,area.h-150)};
    fill(plot,{12,20,30,255});
    for(int i=0;i<5;++i)
    {
        SDL_SetRenderDrawColor(renderer,55,65,80,255);
        const float y=plot.y+i*plot.h/4; SDL_RenderLine(renderer,plot.x,y,plot.x+plot.w,y);
    }
    int maximum=256;
    for(size_t i=0;i<6;++i) if(graphVisible[i]) for(const auto value : *histories[i]) maximum=std::max(maximum,value);
    text(std::to_string(maximum),{area.x,plot.y-22,100,22},false,muted);
    for(size_t series=0;series<6;++series)
    {
        if(!graphVisible[series]) continue;
        std::array<SDL_FPoint,HistoryLength> points{};
        for(size_t i=0;i<points.size();++i) points[i]={plot.x+i*plot.w/(HistoryLength-1),plot.y+plot.h-std::clamp((*histories[series])[i]/static_cast<float>(maximum),0.f,1.f)*plot.h};
        const auto c=colors[series]; SDL_SetRenderDrawColor(renderer,c.r,c.g,c.b,255); SDL_RenderLines(renderer,points.data(),static_cast<int>(points.size()));
        text(std::to_string(series+1),{points.front().x,points.front().y-20,20,20},true,c);
    }
    text("Newest",{plot.x,plot.y+plot.h+4,100,22},false,muted);
    text("Oldest",{plot.x+plot.w-70,plot.y+plot.h+4,70,22},false,muted);
}
void ModernInterface::overlaysPanel(SDL_FRect area)
{
    const float width=area.w/3;
    for(size_t i=0;i<OverlayNames.size();++i) button(std::string(OverlayNames[i]),{area.x+(i%3)*width,area.y+(i/3)*42.f,width-5,36},[this,i] {
        selectedOverlay=static_cast<DataOverlay>(i); minimapDirty=true; show(Panel::None);
    },selectedOverlay==static_cast<DataOverlay>(i));
    text(overlayLegend(selectedOverlay),{area.x,area.y+224,area.w,28},false,accent);
    text("Use - / + in the dashboard to adjust opacity.",{area.x,area.y+260,area.w,28},false,muted);
}
void ModernInterface::settingsPanel(SDL_FRect area)
{
    constexpr std::array<const char*,4> tabs{"Readability","Controls","Sound","Gameplay"};
    for(int i=0;i<4;++i) button(tabs[i],{area.x+i*area.w/4,area.y,area.w/4-4,30},[this,i] {settingsTab=i; binding=-1;focus=-1;},settingsTab==i);
    const float y=area.y+46, width=area.w/2-6;
    auto changed=[this] {minimapDirty=true; if(settingsChanged) settingsChanged();};
    if(settingsTab==0)
    {
        button("UI scale: "+std::to_string(static_cast<int>(settings.scale*100))+"%",{area.x,y,width,34},[this,changed] { settings.scale=settings.scale>=1.5f ? 1.f : settings.scale+.25f;changed();});
        button(settings.largeText ? "Large text: on" : "Large text: off",{area.x+width+6,y,width,34},[this,changed] {settings.largeText=!settings.largeText;changed();},settings.largeText);
        button(settings.highContrast ? "High contrast: on" : "High contrast: off",{area.x,y+44,width,34},[this,changed] {settings.highContrast=!settings.highContrast;changed();},settings.highContrast);
        button(settings.accessibleColors ? "Blue overlays: on" : "Blue overlays: off",{area.x+width+6,y+44,width,34},[this,changed] {settings.accessibleColors=!settings.accessibleColors;changed();},settings.accessibleColors);
        const std::array<std::string,5> labels{"Windowed","Maximized","Fullscreen",display.vsync ? "VSync: on" : "VSync: off",display.pixelPerfect ? "Pixel perfect: on" : "Pixel perfect: off"};
        for(int i=0;i<5;++i) button(labels[i],{area.x+(i%3)*area.w/3,y+104+(i/3)*40.f,area.w/3-5,34},[this,i] {if(displayAction) displayAction(i);},i<3 ? static_cast<int>(display.mode)==i : i==3 ? display.vsync : display.pixelPerfect);
        text("UI scale fits the display. Tab / Enter navigate; Esc closes.",{area.x,y+204,area.w,28},false,muted);
        text("F11 fullscreen | F12 settings | Home centers the city",{area.x,y+238,area.w,28},false,muted);
    }
    else if(settingsTab==1)
    {
        button(settings.reverseZoom ? "Reverse zoom: on" : "Reverse zoom: off",{area.x,y,width,30},[this,changed] {settings.reverseZoom=!settings.reverseZoom;changed();});
        button("Pan speed: "+std::to_string(static_cast<int>(settings.panSpeed)),{area.x+width+6,y,width,30},[this,changed] {settings.panSpeed=settings.panSpeed>=1000 ? 100.f : settings.panSpeed+100;changed();});
        const size_t first=static_cast<size_t>(keyPage*10), end=std::min(first+10,settings.keys.size());
        for(size_t i=first;i<end;++i)
        {
            const size_t row=i-first;
            const auto name=i<16 ? tools.tool(static_cast<Tool::Type>(i)).name : i<26 ? std::string(commandNames[i-16]) : std::string(panNames[i-26]);
            button(name+": "+(binding==static_cast<int>(i) ? "Press a key..." : SDL_GetKeyName(settings.keys[i])),{area.x+(row%2)*(width+6),y+42+(row/2)*35.f,width,31},[this,i] {binding=static_cast<int>(i);},binding==static_cast<int>(i));
        }
        button("Previous bindings",{area.x,y+226,width,30},[this] {keyPage=(keyPage+2)%3;binding=-1;});
        button("Next bindings",{area.x+width+6,y+226,width,30},[this] {keyPage=(keyPage+1)%3;binding=-1;});
        text("Keys must be unique. Esc cancels. Arrows / right-drag pan.",{area.x,y+262,area.w,28},false,muted);
    }
    else if(settingsTab==2)
    {
        constexpr std::array<const char*,3> names{"Master","City effects","Construction"};
        for(int i=0;i<3;++i)
        {
            const float gain=i==0 ? audio.masterVolume() : audio.channelVolume(i==1 ? AudioChannel::City : AudioChannel::Construction);
            text(std::string(names[i])+" "+std::to_string(static_cast<int>(std::round(gain*100)))+"%",{area.x,y+i*48.f,area.w-100,36},true);
            for(int d : {-1,1}) button(std::string(names[i])+(d<0 ? " -" : " +"),{area.x+area.w-96+(d>0 ? 50 : 0),y+i*48.f,46,34},[this,i,d,changed] {
                const auto channel=i==1 ? AudioChannel::City : AudioChannel::Construction;
                const float current=i==0 ? audio.masterVolume() : audio.channelVolume(channel);
                const float value=std::clamp(current+d*.1f,0.f,1.f);
                if(i==0) audio.masterVolume(value); else audio.channelVolume(channel,value); changed();
            },false,true,d<0 ? "-" : "+");
        }
        text(audio.available() ? "Sound output connected" : "No sound device; city play remains available",{area.x,y+160,area.w,30},false,muted);
    }
    else
    {
        auto& options=gameplayOptions();
        const std::array<bool GameOptions::*,6> fields{&GameOptions::animationEnabled,&GameOptions::autoBudget,&GameOptions::autoBulldoze,&GameOptions::autoGoto,&GameOptions::disastersEnabled,&GameOptions::soundEnabled};
        constexpr std::array<const char*,6> names{"Animation","Auto budget","Auto bulldoze","Auto goto","Disasters","Sound"};
        for(size_t i=0;i<fields.size();++i)
        {
            const auto field=fields[i]; const bool enabled=options.*field;
            button(std::string(names[i])+(enabled ? ": on" : ": off"),{area.x+(i%2)*(width+6),y+(i/2)*44.f,width,34},[this,field] {auto& current=gameplayOptions();current.*field=!(current.*field);if(optionsChanged) optionsChanged();},enabled);
        }
        text("City saves: budget, bulldoze, goto and sound.",{area.x,y+160,area.w,30},false,muted);
        text("Animation and disasters apply for this session.",{area.x,y+194,area.w,30},false,muted);
    }
}
void ModernInterface::sheet()
{
    const auto area=panelArea();
    fill({0,0,size.x,size.y-32},{0,0,0,135});
    fill(area,settings.highContrast ? SDL_Color{0,0,0,255} : SDL_Color{22,34,48,255});
    constexpr std::array<const char*,9> titles{"","CITY BUDGET","CITY EVALUATION","CITY HISTORY","SETTINGS","ZONE QUERY","CITY DATA","SCENARIOS","NEW CITY"};
    text(titles[static_cast<size_t>(panel)],{area.x+20,area.y+8,area.w-90,32},true,accent);
    button("Close",{area.x+area.w-76,area.y+8,60,30},[this] {show(Panel::None);});
    const SDL_FRect content{area.x+20,area.y+52,area.w-40,area.h-65};
    switch(panel)
    {
    case Panel::Budget: budgetPanel(content); break;
    case Panel::Evaluation: evaluationPanel(content); break;
    case Panel::Graphs: graphsPanel(content); break;
    case Panel::Settings: settingsPanel(content); break;
    case Panel::Overlays: overlaysPanel(content); break;
    case Panel::Scenarios:
        text("Classic v1 scenarios replace this city. Save first if needed.",{content.x,content.y,content.w,32},false,muted);
        for (size_t i=0;i<ScenarioDefinitions.size();++i)
        {
            const auto& scenario=ScenarioDefinitions[i];
            button(scenario.cityName,{content.x+(i%2)*content.w/2,content.y+48+(i/2)*53.f,content.w/2-5,42},[this,id=scenario.legacyId] {
                if(scenarioSelected) scenarioSelected(id);
                show(Panel::None);
            });
        }
        break;
    case Panel::NewCity:
    {
        const float row=std::min(44.f,content.h/6), height=row-4;
        text("New city/import replaces this city. Save first.",{content.x,content.y,content.w,height},false,muted);
        button("Classic mode",{content.x,content.y+row,content.w/2-4,height},[this] {selectedRuleset=RulesetId::ClassicV1;},selectedRuleset==RulesetId::ClassicV1);
        button("Enhanced mode",{content.x+content.w/2,content.y+row,content.w/2-4,height},[this] {selectedRuleset=RulesetId::EnhancedV1;},selectedRuleset==RulesetId::EnhancedV1);
        text("Enhanced v1 uses Classic mechanics; saves use .c89.",{content.x,content.y+row*2,content.w,height},false,muted);
        button("Start new city",{content.x,content.y+row*3,content.w,height},[this] {if(command) command(UiCommand::StartNewCity);show(Panel::None);});
        button("Import Classic to Enhanced",{content.x,content.y+row*4,content.w,height},[this] {if(command) command(UiCommand::ImportClassic);});
        button("Export Classic copy",{content.x,content.y+row*5,content.w,height},[this] {if(command) command(UiCommand::ExportClassic);});
        break;
    }
    case Panel::Query:
    {
        const auto& q=tools.queryResult();
        const std::array<std::string,6> rows{q.title,"Density: "+q.density,"Land value: "+q.landValue,"Crime: "+q.crime,"Pollution: "+q.pollution,"Growth: "+q.populationGrowth};
        for(size_t i=0;i<rows.size();++i) text(rows[i],{content.x,content.y+i*44.f,content.w,34},i==0);
        break;
    }
    default: break;
    }
}
void ModernInterface::draw(const Camera2D& camera)
{
    controls.clear(); tooltip.clear();
    dashboard(); palette(); minimap(camera);
    if (selectedOverlay != DataOverlay::None)
    {
        const SDL_FRect legend{sidebar+12,116,size.x-sidebar-24,54};
        fill(legend,{20,30,43,235});
        text(overlayLegend(selectedOverlay),{legend.x+8,legend.y+2,legend.w-16,26},false,accent);
        const auto world = camera.screenToWorld(pointer.to<float>());
        const Point<int> tile{static_cast<int>(std::floor(world.x/16)),static_cast<int>(std::floor(world.y/16))};
        text(std::string(OverlayNames[static_cast<size_t>(selectedOverlay)])+" at ("+std::to_string(tile.x)+","+std::to_string(tile.y)+"): "+std::to_string(overlayValue(selectedOverlay,tile)),{legend.x+8,legend.y+27,legend.w-16,24});
    }
    if(panel!=Panel::None) {controls.clear();tooltip.clear();sheet();}
    if(focus>=0 && focus<static_cast<int>(controls.size()) && controls[focus].enabled)
    {
        const auto& control=controls[focus];
        SDL_SetRenderDrawColor(renderer,accent.r,accent.g,accent.b,255); SDL_RenderRect(renderer,&control.area);
        if (tooltip.empty()) { tooltip = control.label; }
    }
    fill({0,size.y-32,size.x,32},settings.highContrast ? SDL_Color{0,0,0,255} : SDL_Color{20,30,43,255});
    const bool showStatus = tooltip.empty() || SDL_GetTicks() < statusUntil;
    text(showStatus ? status : tooltip,{14,size.y-29,size.x-180,25},false,showStatus ? ink : accent);
    text(findRuleset(city.rulesetId())->label,{size.x-150,size.y-29,136,25},true,accent);
}
bool ModernInterface::pointInWindow(Point<int> point) const
{
    return modalWindowVisible() || point.y<108 || point.y>=size.y-32 || point.x<sidebar ||
        (selectedOverlay!=DataOverlay::None && contains({sidebar+12,116,size.x-sidebar-24,54},point));
}
void ModernInterface::focusMinimap(Point<int> point, Camera2D& camera)
{
    const auto area=minimapArea();
    camera.focus({std::clamp((point.x-area.x)/area.w,0.f,1.f)*1920,std::clamp((point.y-area.y)/area.h,0.f,1.f)*1600});
}
bool ModernInterface::mouseDown(Point<int> point, Camera2D& camera)
{
    pointer=point;
    for(size_t i=0;i<controls.size();++i)
    {
        if(!contains(controls[i].area,point)) continue;
        if(controls[i].enabled) {focus=static_cast<int>(i);const auto action=controls[i].action;action();}
        return true;
    }
    if(panel==Panel::None && minimapVisible && contains(minimapArea(),point)) {minimapDragging=true;focusMinimap(point,camera);return true;}
    return pointInWindow(point);
}
void ModernInterface::mouseMotion(Point<int> point, bool leftDown, Camera2D& camera)
{
    pointer=point;
    if(minimapDragging && leftDown) {focusMinimap(point,camera);}
}
bool ModernInterface::wheel(float direction)
{
    if(panel==Panel::Settings && settingsTab==1) {keyPage=(keyPage+(direction<0 ? 1 : 2))%3;binding=-1;return true;}
    return modalWindowVisible();
}
bool ModernInterface::keyDown(SDL_Keycode key, SDL_Keymod modifiers)
{
    if(binding>=0)
    {
        if(key==SDLK_ESCAPE) {binding=-1;return true;}
        if(reservedKey(key) || modifiers & (SDL_KMOD_CTRL|SDL_KMOD_ALT|SDL_KMOD_GUI) || !settings.bind(static_cast<size_t>(binding),key))
            {message("That key is reserved or already assigned.");return true;}
        binding=-1;message("Key binding saved.");if(settingsChanged) settingsChanged();return true;
    }
    if(key==SDLK_ESCAPE) {show(panel==Panel::None ? Panel::Settings : Panel::None);return true;}
    if(key==SDLK_TAB)
    {
        if(controls.empty()) return true;
        const bool backwards = (modifiers & SDL_KMOD_SHIFT) != 0;
        if (focus < 0 && backwards) { focus = 0; }
        for(size_t i=0;i<controls.size();++i)
        {
            focus=(focus+(backwards ? -1 : 1)+static_cast<int>(controls.size()))%static_cast<int>(controls.size());
            if(controls[focus].enabled) break;
        }
        return true;
    }
    if(key==SDLK_RETURN && focus>=0 && focus<static_cast<int>(controls.size()))
    {
        const auto action=controls[focus].action; if(controls[focus].enabled) action();return true;
    }
    if(modifiers & (SDL_KMOD_CTRL|SDL_KMOD_ALT|SDL_KMOD_GUI)) return false;
    if(modalWindowVisible()) return true;
    for(size_t i=0;i<26;++i)
    {
        if(settings.keys[i]!=key) continue;
        if(i<16) selectTool(i); else if(command) command(static_cast<UiCommand>(i-16));
        return true;
    }
    return false;
}
