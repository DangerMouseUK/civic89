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
#include "PresentationUtil.h"

bool pointInRect(const Point<int>& point, const SDL_Rect& rect)
{
    return point.x >= rect.x && point.x < rect.x + rect.w && point.y >= rect.y && point.y < rect.y + rect.h;
}

bool pointInFRect(const Point<int>& point, const SDL_FRect& rect)
{
    return point.x >= rect.x && point.x < rect.x + rect.w && point.y >= rect.y && point.y < rect.y + rect.h;
}

SDL_FRect fRectFromRect(const SDL_Rect& rect)
{
    SDL_FRect fRect{};
    SDL_RectToFRect(&rect, &fRect);
    return fRect;
}
