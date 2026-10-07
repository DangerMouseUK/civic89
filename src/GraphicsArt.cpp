// Civic 89 palette-preserving pixel presentation. SPDX-License-Identifier: GPL-3.0-or-later
#include "GraphicsArt.h"
#include "Map.h"
#include <SDL3_image/SDL_image.h>
#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <string>

namespace
{
#if defined(CIVIC89_MILESTONE_TESTS)
    int failAfter = -1;
#endif
    void check(bool success) { if (!success) { throw std::runtime_error(SDL_GetError()); } }
    SurfaceOwner read(const std::string& path)
    {
        SurfaceOwner surface(IMG_Load(path.c_str()));
        if (!surface) { throw std::runtime_error("Cannot load graphics " + path + ": " + SDL_GetError()); }
        return surface;
    }
    Texture upload(SDL_Renderer* renderer, SDL_Surface* surface, GraphicsArt& art)
    {
#if defined(CIVIC89_MILESTONE_TESTS)
        if (art.style == GraphicsStyle::Enhanced && failAfter >= 0 && failAfter-- == 0)
            { failAfter = -1; throw std::runtime_error("Injected enhanced graphics allocation failure"); }
#endif
        auto* value = SDL_CreateTextureFromSurface(renderer, surface);
        if (!value) { throw std::runtime_error(SDL_GetError()); }
        auto texture = buildTexture(value);
        check(SDL_SetTextureScaleMode(texture.texture, SDL_SCALEMODE_NEAREST));
        ++art.textureCount;
        art.textureBytes += static_cast<size_t>(surface->w) * surface->h * 4;
        return texture;
    }
    Texture image(SDL_Renderer* renderer, const std::string& path, GraphicsArt& art)
    {
        auto source = read(path);
        auto pixels = refinePixelRegion(source.get(), {0,0,source->w,source->h}, art.style);
        return upload(renderer, pixels.get(), art);
    }
}
#if defined(CIVIC89_MILESTONE_TESTS)
void failEnhancedGraphicsAfter(int textures) { failAfter = textures; }
#endif

SurfaceOwner refinePixelRegion(SDL_Surface* source, SDL_Rect region, GraphicsStyle style)
{
    if (!source || region.x < 0 || region.y < 0 || region.w <= 0 || region.h <= 0 ||
        region.x + region.w > source->w || region.y + region.h > source->h ||
        (style != GraphicsStyle::Classic && style != GraphicsStyle::Enhanced)) { throw std::runtime_error("Invalid graphics region/style"); }
    SurfaceOwner plain(SDL_CreateSurface(region.w, region.h, SDL_PIXELFORMAT_RGBA32));
    if (!plain) { throw std::runtime_error(SDL_GetError()); }
    check(SDL_SetSurfaceBlendMode(source, SDL_BLENDMODE_NONE));
    check(SDL_BlitSurface(source, &region, plain.get(), nullptr));
    if (style == GraphicsStyle::Classic) { return plain; }
    SurfaceOwner output(SDL_CreateSurface(region.w*2, region.h*2, SDL_PIXELFORMAT_RGBA32));
    if (!output) { throw std::runtime_error(SDL_GetError()); }
    auto sample = [&](int x, int y) {
        x = std::clamp(x, 0, region.w-1); y = std::clamp(y, 0, region.h-1);
        return reinterpret_cast<const uint32_t*>(static_cast<const Uint8*>(plain->pixels)+y*plain->pitch)[x];
    };
    for (int y=0; y<region.h; ++y) for (int x=0; x<region.w; ++x)
    {
        const auto e=sample(x,y), b=sample(x,y-1), d=sample(x-1,y), f=sample(x+1,y), h=sample(x,y+1);
        auto* top=reinterpret_cast<uint32_t*>(static_cast<Uint8*>(output->pixels)+y*2*output->pitch)+x*2;
        auto* bottom=reinterpret_cast<uint32_t*>(static_cast<Uint8*>(output->pixels)+(y*2+1)*output->pitch)+x*2;
        const bool corner=b!=h && d!=f;
        top[0]=corner && d==b ? d : e; top[1]=corner && b==f ? f : e;
        bottom[0]=corner && d==h ? d : e; bottom[1]=corner && h==f ? f : e;
    }
    return output;
}

std::shared_ptr<const GraphicsArt> GraphicsArt::load(SDL_Renderer* renderer, GraphicsStyle style)
{
    auto art=std::make_shared<GraphicsArt>();
    art->style=style; art->density=style==GraphicsStyle::Enhanced ? 2 : 1;
    const int d=art->density;
    auto source=read("images/tiles.xpm");
    if (source->w!=16 || source->h!=TILE_COUNT*16) { throw std::runtime_error("Unexpected tile catalogue dimensions"); }
    SurfaceOwner atlas(SDL_CreateSurface(512*d,512*d,SDL_PIXELFORMAT_RGBA32));
    if (!atlas) { throw std::runtime_error(SDL_GetError()); }
    for (int tile=0; tile<TILE_COUNT; ++tile)
    {
        auto pixels=refinePixelRegion(source.get(),{0,tile*16,16,16},style);
        const SDL_Rect destination{(tile%32)*16*d,(tile/32)*16*d,16*d,16*d};
        check(SDL_BlitSurface(pixels.get(),nullptr,atlas.get(),&destination));
    }
    art->tiles=upload(renderer,atlas.get(),*art);
    source=read("images/tilessm.xpm");
    if (source->w!=4 || source->h!=TILE_COUNT*3) { throw std::runtime_error("Unexpected minimap catalogue dimensions"); }
    SurfaceOwner mini(SDL_CreateSurface(3*d,TILE_COUNT*3*d,SDL_PIXELFORMAT_RGBA32));
    if (!mini) { throw std::runtime_error(SDL_GetError()); }
    for (int tile=0; tile<TILE_COUNT; ++tile)
    {
        auto pixels=refinePixelRegion(source.get(),{0,tile*3,3,3},style);
        const SDL_Rect destination{0,tile*3*d,3*d,3*d};
        check(SDL_BlitSurface(pixels.get(),nullptr,mini.get(),&destination));
    }
    art->minimap=upload(renderer,mini.get(),*art);
    for (size_t type=0; type<art->sprites.size(); ++type) for (int frame=0; frame<GraphicsFrameCounts[type]; ++frame)
    {
        auto texture=image(renderer,"images/obj"+std::to_string(type+1)+"-"+std::to_string(frame)+".xpm",*art);
        check(SDL_SetTextureBlendMode(texture.texture,SDL_BLENDMODE_BLEND));
        art->sprites[type].push_back(std::move(texture));
    }
    for (size_t tool=0; tool<art->ghosts.size(); ++tool) if (GraphicsGhostFiles[tool])
    {
        art->ghosts[tool]=image(renderer,std::string("images/")+GraphicsGhostFiles[tool]+".xpm",*art);
        check(SDL_SetTextureBlendMode(art->ghosts[tool].texture,SDL_BLENDMODE_BLEND));
        check(SDL_SetTextureAlphaMod(art->ghosts[tool].texture,125));
    }
    return art;
}
