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
#pragma once
#include "Camera2D.h"

struct SDL_Renderer;
struct GraphicsArt;
class SpriteRenderer
{
public:
    void draw(SDL_Renderer* renderer, const Camera2D& camera, Vector<float> shake, const GraphicsArt&) const;
};
