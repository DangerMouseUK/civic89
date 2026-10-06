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
#include "Texture.h"
#include "PresentationUtil.h"
#include "main.h"
#include <array>

namespace { std::array<std::vector<Texture>, 7> images; }

void clearSpriteImages()
{
    for (auto& frames : images)
    {
        for (auto& frame : frames) { SDL_DestroyTexture(frame.texture); }
        frames.clear();
    }
}

void drawSprites()
{
    for (const auto& sprite : simulationSprites())
    {
        if (!sprite.active) { continue; }
        auto& frames = images.at(static_cast<size_t>(sprite.type));
        if (frames.empty())
        {
            for (int i = 0; i < sprite.frameCount; ++i)
            {
                const auto name = "images/obj" + std::to_string(static_cast<int>(sprite.type) + 1)
                    + "-" + std::to_string(i) + ".xpm";
                frames.push_back(loadTexture(MainWindowRenderer, name));
            }
        }
        const auto& frame = frames.at(static_cast<size_t>(sprite.frame));
        const auto rect = fRectFromRect({sprite.position.x - viewOffset().x + sprite.offset.x,
            sprite.position.y - viewOffset().y + sprite.offset.y, frame.dimensions.x, frame.dimensions.y});
        SDL_RenderTexture(MainWindowRenderer, frame.texture, &frame.area, &rect);
    }
}
