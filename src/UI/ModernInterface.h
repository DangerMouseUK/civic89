// Civic 89 single-window interface. SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "../Texture.h"
#include "../UiSettings.h"
#include "../DisplaySettings.h"
#include "../OverlayModel.h"
#include "../Camera2D.h"
#include "../SdlResources.h"
#include "../Ruleset.h"
#include <SDL3_ttf/SDL_ttf.h>
#include <functional>
#include <unordered_map>
#include <vector>

class Budget;
class RCI;
class ToolManager;
class CityProperties;
class AudioManager;
struct ZoneStats;
enum class UiCommand { Pause, Save, Open, Minimap, Evaluation, Scenarios, NewCity, Settings, Graphs, Budget, StartNewCity, ImportClassic, ExportClassic };

class ModernInterface
{
public:
    enum class Panel { None, Budget, Evaluation, Graphs, Settings, Query, Overlays, Scenarios, NewCity };
    ModernInterface(SDL_Renderer*, Budget&, const RCI&, ToolManager&, const CityProperties&, AudioManager&, UiSettings&, const DisplaySettings&);
    void layout(Vector<float> size, float density);
    void draw(const Camera2D&);
    bool pointInWindow(Point<int>) const;
    bool mouseDown(Point<int>, Camera2D&);
    void mouseMotion(Point<int>, bool leftDown, Camera2D&);
    void mouseUp() { minimapDragging = false; }
    bool keyDown(SDL_Keycode, SDL_Keymod);
    bool wheel(float);
    bool modalWindowVisible() const { return panel != Panel::None; }
    void show(Panel value);
    void hideAllWindows() { show(Panel::None); }
    Panel currentPanel() const { return panel; }
    RulesetId newCityRuleset() const { return selectedRuleset; }
    SDL_FRect panelArea() const;
    SDL_FRect minimapArea() const;
    void toggleMinimap() { minimapVisible = !minimapVisible; }
    bool minimapShown() const { return minimapVisible; }
    void minimapShown(bool value) { minimapVisible = value; }
    void invalidateMinimap() { minimapDirty = true; }
    void message(const std::string& value) { status = value; statusUntil = SDL_GetTicks() + 4000; }
    const std::string& message() const { return status; }
    void cancelTool();
    const Texture* ghost() const;
    DataOverlay overlay() const { return selectedOverlay; }
    void overlay(DataOverlay value) { selectedOverlay = value; minimapDirty = true; }
    std::function<void(UiCommand)> command;
    std::function<void()> settingsChanged;
    std::function<void()> optionsChanged;
    std::function<void(int)> displayAction;
    std::function<void(int)> scenarioSelected;
    static UiSettings defaultSettings();
    static bool reservedKey(SDL_Keycode);
    // Public geometry supports native interaction acceptance without a second UI path.
    SDL_FRect controlArea(const std::string& label) const;
private:
    struct Control { SDL_FRect area; std::string label; bool enabled; std::function<void()> action; };
    SDL_Renderer* renderer;
    Budget& budget;
    const RCI& rci;
    ToolManager& tools;
    const CityProperties& city;
    AudioManager& audio;
    UiSettings& settings;
    const DisplaySettings& display;
    Vector<float> size{800,600};
    float density{1};
    float sidebar{184};
    Panel panel{Panel::None};
    RulesetId selectedRuleset{RulesetId::ClassicV1};
    DataOverlay selectedOverlay{DataOverlay::None};
    bool minimapVisible{true};
    bool minimapDragging{false};
    bool longHistory{false};
    std::array<bool,6> graphVisible{true,true,true,true,true,true};
    int settingsTab{};
    int keyPage{};
    int binding{-1};
    int focus{-1};
    Point<int> pointer{};
    std::string status{"Welcome to Civic 89"};
    Uint64 statusUntil{};
    std::string tooltip;
    std::vector<Control> controls;
    Texture icons;
    Texture miniTiles;
    Texture miniCache;
    Uint64 minimapUpdated{};
    bool minimapDirty{true};
    std::array<Texture,16> ghosts;
    using FontOwner = std::unique_ptr<TTF_Font, SdlDeleter<TTF_Font, TTF_CloseFont>>;
    FontOwner normalFont;
    FontOwner boldFont;
    std::unordered_map<std::string,Texture> textCache;
    void fill(SDL_FRect, SDL_Color);
    void text(std::string_view, SDL_FRect, bool bold = false, SDL_Color color = {231,238,245,255});
    void button(std::string, SDL_FRect, std::function<void()>, bool active = false, bool enabled = true, const char* caption = nullptr);
    void dashboard();
    void palette();
    void minimap(const Camera2D&);
    void sheet();
    void budgetPanel(SDL_FRect);
    void evaluationPanel(SDL_FRect);
    void graphsPanel(SDL_FRect);
    void settingsPanel(SDL_FRect);
    void overlaysPanel(SDL_FRect);
    void selectTool(size_t);
    void focusMinimap(Point<int>, Camera2D&);
};
