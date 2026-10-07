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
#include "SpriteRenderer.h"
#include "Sprite.h"
#include "GraphicsArt.h"

void SpriteRenderer::draw(SDL_Renderer* renderer, const Camera2D& camera, Vector<float> shake, const GraphicsArt& art) const
{
    for (const auto& sprite : simulationSprites())
    {
        if (!sprite.active) { continue; }
        const auto& frames = art.sprites.at(static_cast<size_t>(sprite.type));
        const auto& frame = frames.at(static_cast<size_t>(sprite.frame));
        const auto point = camera.worldToScreen({static_cast<float>(sprite.position.x + sprite.offset.x),
            static_cast<float>(sprite.position.y + sprite.offset.y)}) + shake;
        const SDL_FRect rect{point.x, point.y, frame.dimensions.x * camera.zoom()/art.density, frame.dimensions.y * camera.zoom()/art.density};
        SDL_RenderTexture(renderer, frame.texture, &frame.area, &rect);
    }
}
