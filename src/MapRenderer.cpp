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
#include "Map.h"
#include "SdlResources.h"
#include <SDL3_image/SDL_image.h>
#include <algorithm>
#include <cmath>
#include <stdexcept>

MapRenderer::MapRenderer(SDL_Renderer* renderer) : mRenderer(renderer)
{
    SurfaceOwner source(IMG_Load("images/tiles.xpm"));
    SurfaceOwner destination(SDL_CreateSurface(512, 512, SDL_PIXELFORMAT_RGBA32));
    if (!source || !destination) { throw std::runtime_error(std::string("Unable to build tile atlas: ") + SDL_GetError()); }
    for (int tile = 0; tile < TILE_COUNT; ++tile)
    {
        const SDL_Rect src{0, tile * 16, 16, 16};
        const SDL_Rect dst{(tile % 32) * 16, (tile / 32) * 16, 16, 16};
        if (!SDL_BlitSurface(source.get(), &src, destination.get(), &dst)) { throw std::runtime_error(SDL_GetError()); }
    }
    auto* texture = SDL_CreateTextureFromSurface(renderer, destination.get());
    if (!texture) { throw std::runtime_error(std::string("Unable to create tile atlas: ") + SDL_GetError()); }
    mAtlas = buildTexture(texture);
    mMap = newTexture(renderer, {SimWidth * 16, SimHeight * 16});
    SDL_SetTextureScaleMode(mAtlas.texture, SDL_SCALEMODE_NEAREST);
    SDL_SetTextureScaleMode(mMap.texture, SDL_SCALEMODE_NEAREST);
}
void MapRenderer::updateTiles(Point<int> begin, Point<int> end)
{
    if (!SDL_SetRenderTarget(mRenderer, mMap.texture)) { throw std::runtime_error(SDL_GetError()); }
    for (int x = begin.x; x < end.x; ++x)
    {
        for (int y = begin.y; y < end.y; ++y)
        {
            const auto tile = mBlink && tileIsZoned({x, y}) && !tileIsPowered({x, y}) ? LightningBolt : tileValue(x, y);
            const auto masked = maskedTileValue(tile);
            const SDL_FRect src{static_cast<float>((masked % 32) * 16), static_cast<float>((masked / 32) * 16), 16, 16};
            const SDL_FRect dst{static_cast<float>(x * 16), static_cast<float>(y * 16), 16, 16};
            SDL_RenderTexture(mRenderer, mAtlas.texture, &src, &dst);
        }
    }
    // Texture targets have their own scale; present only the window, after UI drawing.
    if (!SDL_SetRenderTarget(mRenderer, nullptr)) { throw std::runtime_error(SDL_GetError()); }
    mDirty = false;
    mBegin = begin; mEnd = end;
}
void MapRenderer::render(const Camera2D& camera, Vector<float> shake, std::optional<MapPreview> preview)
{
    const auto position = camera.position();
    const auto size = camera.visibleWorld();
    const Point<int> begin{std::clamp(static_cast<int>(std::floor(position.x / 16)), 0, SimWidth),
        std::clamp(static_cast<int>(std::floor(position.y / 16)), 0, SimHeight)};
    const Point<int> end{std::clamp(static_cast<int>(std::ceil((position.x + size.x) / 16)), 0, SimWidth),
        std::clamp(static_cast<int>(std::ceil((position.y + size.y) / 16)), 0, SimHeight)};
    if (mDirty || begin != mBegin || end != mEnd) { updateTiles(begin, end); }
    SDL_SetRenderDrawColor(mRenderer, 24, 28, 30, 255);
    SDL_RenderClear(mRenderer);
    const SDL_FRect source{position.x, position.y, std::min(size.x, SimWidth * 16.f - position.x),
        std::min(size.y, SimHeight * 16.f - position.y)};
    const SDL_FRect destination{shake.x, shake.y, source.w * camera.zoom(), source.h * camera.zoom()};
    SDL_RenderTexture(mRenderer, mMap.texture, &source, &destination);
    mSprites.draw(mRenderer, camera, shake);
    if (preview)
    {
        const auto point = camera.worldToScreen({preview->worldRect.x, preview->worldRect.y}) + shake;
        const SDL_FRect rect{point.x, point.y, preview->worldRect.w * camera.zoom(), preview->worldRect.h * camera.zoom()};
        if (preview->ghost && preview->ghost->texture)
            { SDL_RenderTexture(mRenderer, preview->ghost->texture, &preview->ghost->area, &rect); }
        else
        {
            SDL_SetRenderDrawColor(mRenderer, 255, 255, 255, 100);
            SDL_RenderFillRect(mRenderer, &rect);
            SDL_SetRenderDrawColor(mRenderer, 255, 255, 255, 255);
            SDL_RenderRect(mRenderer, &rect);
        }
    }
}
