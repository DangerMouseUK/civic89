// Civic 89 application resource owners. SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <SDL3/SDL.h>
#include <memory>

template<class T, auto Destroy> struct SdlDeleter
{
    void operator()(T* value) const noexcept { if (value) { Destroy(value); } }
};
using SurfaceOwner = std::unique_ptr<SDL_Surface, SdlDeleter<SDL_Surface, SDL_DestroySurface>>;
using WindowOwner = std::unique_ptr<SDL_Window, SdlDeleter<SDL_Window, SDL_DestroyWindow>>;
using RendererOwner = std::unique_ptr<SDL_Renderer, SdlDeleter<SDL_Renderer, SDL_DestroyRenderer>>;
