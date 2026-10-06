// Civic 89 map presentation. SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "Camera2D.h"
#include "SpriteRenderer.h"
#include "Texture.h"
#include <optional>

struct MapPreview
{
    SDL_FRect worldRect{};
    const Texture* ghost{};
};
class MapRenderer
{
public:
    explicit MapRenderer(SDL_Renderer* renderer);
    void invalidate() { mDirty = true; }
    void toggleBlink() { mBlink = !mBlink; invalidate(); }
    void render(const Camera2D& camera, Vector<float> shake = {}, std::optional<MapPreview> preview = {});
    void clearSpriteImages() { mSprites.clear(); }
private:
    void updateTiles(Point<int> begin, Point<int> end);
    SDL_Renderer* mRenderer;
    Texture mAtlas;
    Texture mMap;
    SpriteRenderer mSprites;
    bool mDirty{true}, mBlink{false};
    Point<int> mBegin{-1, -1}, mEnd{-1, -1};
};
