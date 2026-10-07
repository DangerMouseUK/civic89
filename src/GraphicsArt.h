// Civic 89 palette-preserving pixel presentation. SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "GraphicsSettings.h"
#include "SdlResources.h"
#include "Texture.h"
#include <array>
#include <memory>
#include <vector>

inline constexpr std::array<int,7> GraphicsFrameCounts{5,9,12,9,17,3,6};
inline constexpr std::array<const char*,16> GraphicsGhostFiles{
    "res","com","ind","fire",nullptr,"police",nullptr,nullptr,nullptr,nullptr,
    "stadium",nullptr,"seaport","coal","nuclear","airport"};

// A whole set is prepared before either map or UI adopts it.
struct GraphicsArt
{
    GraphicsStyle style{GraphicsStyle::Classic};
    int density{1};
    Texture tiles, minimap;
    std::array<std::vector<Texture>,7> sprites;
    std::array<Texture,16> ghosts;
    size_t textureCount{};
    size_t textureBytes{};
    static std::shared_ptr<const GraphicsArt> load(SDL_Renderer*, GraphicsStyle);
};
SurfaceOwner refinePixelRegion(SDL_Surface*, SDL_Rect region, GraphicsStyle);
#if defined(CIVIC89_MILESTONE_TESTS)
// Fail partway through enhanced preparation, before any live ownership swap.
void failEnhancedGraphicsAfter(int textures);
#endif
