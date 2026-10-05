// Civic 89 bootstrap tests. SPDX-License-Identifier: GPL-3.0-or-later
#include "Font.h"
#include "GameDataLoader.h"
#include "Tool.h"

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <algorithm>
#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

SDL_Renderer* MainWindowRenderer = nullptr;

namespace
{
    void require(bool condition, const std::string& message)
    {
        if (!condition)
        {
            throw std::runtime_error(message);
        }
    }

    void checkGameData()
    {
        const auto months = GameDataLoader::loadMonthStrings();
        require(months.front() == "January" && months.back() == "December", "Month data did not load correctly");

        const auto tools = GameDataLoader::loadTools();
        require(tools.size() == 17 && tools.front().type == Tool::Type::None, "Expected 16 tools and the empty tool");
        const auto road = std::find_if(tools.begin(), tools.end(), [](const Tool& tool) {
            return tool.type == Tool::Type::Road;
        });
        require(road != tools.end() && road->cost == 10 && road->draggable && road->name == "Roads",
            "The inherited road tool data changed or did not load");
    }

    void checkTextures()
    {
        std::ifstream input("assets/runtime-assets.json");
        require(input.is_open(), "The staged runtime inventory is missing");
        const auto inventory = nlohmann::json::parse(input);
        std::size_t textures = 0;
        for (const auto& file : inventory.at("files"))
        {
            const auto path = file.at("path").get<std::string>();
            const auto extension = std::filesystem::path(path).extension();
            if (extension != ".png" && extension != ".xpm")
            {
                continue;
            }
            std::unique_ptr<SDL_Surface, decltype(&SDL_DestroySurface)> surface(IMG_Load(path.c_str()), SDL_DestroySurface);
            require(surface != nullptr, "Cannot decode " + path + ": " + SDL_GetError());
            require(surface->w > 0 && surface->h > 0, "Empty image: " + path);
            std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)> texture(
                SDL_CreateTextureFromSurface(MainWindowRenderer, surface.get()), SDL_DestroyTexture);
            require(texture != nullptr, "Cannot create texture for " + path + ": " + SDL_GetError());
            require(SDL_RenderTexture(MainWindowRenderer, texture.get(), nullptr, nullptr), "Cannot render " + path);
            ++textures;
        }
        require(textures >= 80, "The runtime image inventory is incomplete");
        std::cout << "Decoded, uploaded and rendered " << textures << " inherited textures\n";
    }

    void checkFonts()
    {
        const std::array<std::pair<const char*, unsigned int>, 7> fonts{{
            {"res/Raleway-Medium.ttf", 12}, {"res/Raleway-Medium.ttf", 13}, {"res/Raleway-Medium.ttf", 14},
            {"res/Raleway-Bold.ttf", 13}, {"res/Raleway-BoldItalic.ttf", 13},
            {"res/raleway-bolditalic.ttf", 13}, {"res/virtue.ttf", 12}
        }};
        for (const auto& [path, size] : fonts)
        {
            Font font(path, size);
            require(font.texture() && font.height() > 0 && font.width("Civic 89 - $20,000") > 0,
                std::string("Font atlas/metrics failed: ") + path);
            require(font.metrics().size() == 256, std::string("Font glyph atlas incomplete: ") + path);
        }
        std::cout << "Built all startup font atlases at their inherited sizes\n";
    }
}

int main()
{
    int result = 0;
    try
    {
        require(SDL_Init(SDL_INIT_VIDEO), std::string("SDL initialisation failed: ") + SDL_GetError());
        {
            std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)> window(
                SDL_CreateWindow("Civic 89 runtime test", 64, 64, SDL_WINDOW_HIDDEN), SDL_DestroyWindow);
            require(window != nullptr, std::string("Window creation failed: ") + SDL_GetError());
            std::unique_ptr<SDL_Renderer, decltype(&SDL_DestroyRenderer)> renderer(
                SDL_CreateRenderer(window.get(), "software"), SDL_DestroyRenderer);
            require(renderer != nullptr, std::string("Software renderer creation failed: ") + SDL_GetError());
            MainWindowRenderer = renderer.get();
            checkGameData();
            checkTextures();
            checkFonts();
            require(SDL_RenderPresent(renderer.get()), "Software renderer presentation failed");
            MainWindowRenderer = nullptr;
        }
        std::cout << "Runtime asset/data smoke test passed\n";
    }
    catch (const std::exception& error)
    {
        std::cerr << error.what() << '\n';
        result = 1;
    }
    if (TTF_WasInit())
    {
        TTF_Quit();
    }
    SDL_Quit();
    return result;
}
