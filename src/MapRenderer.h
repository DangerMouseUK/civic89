// Civic 89 map presentation. SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "Camera2D.h"
#include "SpriteRenderer.h"
#include "Texture.h"
#include "OverlayModel.h"
#include "GraphicsArt.h"
#include <optional>

struct MapPreview
{
    SDL_FRect worldRect{};
    const Texture* ghost{};
};
class MapRenderer
{
public:
    explicit MapRenderer(SDL_Renderer* renderer, GraphicsStyle style = GraphicsStyle::Classic);
    void invalidate() { mDirty = true; }
    void toggleBlink() { mBlink = !mBlink; invalidate(); }
    void render(const Camera2D& camera, Vector<float> shake = {}, std::optional<MapPreview> preview = {},
        DataOverlay overlay = DataOverlay::None, float opacity = .6f, bool accessibleColors = false);
    std::shared_ptr<const GraphicsArt> art() const { return mArt; }
    Texture prepareMap(GraphicsStyle) const;
    void graphics(std::shared_ptr<const GraphicsArt>, Texture) noexcept;
private:
    void updateTiles(Point<int> begin, Point<int> end);
    SDL_Renderer* mRenderer;
    std::shared_ptr<const GraphicsArt> mArt;
    Texture mMap;
    Texture mOverlay;
    DataOverlay mOverlayType{DataOverlay::None};
    bool mAccessibleColors{};
    SpriteRenderer mSprites;
    bool mDirty{true}, mBlink{false};
    Point<int> mBegin{-1, -1}, mEnd{-1, -1};
};
