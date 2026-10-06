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

#include "GameOptions.h"
#include "Texture.h"

#include "Math/Point.h"

#include <string>

#include <SDL3/SDL.h>

#include "EngineState.h"

extern SDL_Renderer* MainWindowRenderer;
class Camera2D;
class MapRenderer;
Camera2D& applicationCamera();
MapRenderer& applicationMapRenderer();
