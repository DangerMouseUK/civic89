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
#include "Map.h"
#include "Texture.h"
#include "main.h"

extern Texture BigTileset;
extern Texture MainMapTexture;
namespace {
    SDL_FRect TileDrawRect{0, 0, 16, 16};
    bool Blink{false};
}
void toggleBlinkFlag()
{
	Blink = !Blink;
}

void drawBigMapSegment(const Point<int>& begin, const Point<int>& end)
{
	SDL_SetRenderTarget(MainWindowRenderer, MainMapTexture.texture);

	SDL_FRect drawRect{ 0.0f, 0.0f, 16.0f, 16.0f };
	unsigned int tile = 0;

	for (int row = begin.x; row < end.x; row++)
	{
		for (int col = begin.y; col < end.y; col++)
		{
			tile = tileValue(row, col);
			// Blink lightning bolt in unpowered zone center
			if (Blink && tileIsZoned({ row, col }) && !tileIsPowered({ row, col }))
			{
				tile = LightningBolt;
			}

			drawRect = { row * drawRect.w, col * drawRect.h, drawRect.w, drawRect.h };

			const unsigned int masked = maskedTileValue(tile);
			TileDrawRect =
			{
				static_cast<float>((static_cast<int>(masked) % 32) * 16),
				static_cast<float>((static_cast<int>(masked) / 32) * 16),
				16.0f, 16.0f
			};

			SDL_RenderTexture(MainWindowRenderer, BigTileset.texture, &TileDrawRect, &drawRect);
		}
	}

	SDL_RenderPresent(MainWindowRenderer);
	SDL_SetRenderTarget(MainWindowRenderer, nullptr);
}

void drawBigMap()
{
	drawBigMapSegment(Point<int>{0, 0}, Point<int>{SimWidth, SimHeight});
}
