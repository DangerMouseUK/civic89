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
#include "Math/Point.h"
#include <SDL3/SDL.h>
bool pointInRect(const Point<int>&, const SDL_Rect&);
bool pointInFRect(const Point<int>&, const SDL_FRect&);
SDL_FRect fRectFromRect(const SDL_Rect&);
