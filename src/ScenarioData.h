// Civic 89 scenario data contract. SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "FileIo.h"
#include "Map.h"
#include "s_alloc.h"

#include <array>
#include <bit>
#include <fstream>
#include <optional>
#include <cstring>
#include <span>

struct ScenarioDefinition
{
    Scenario scenario;
    int legacyId;
    const char* filename;
    const char* cityName;
    int year;
    int funds;
};

inline constexpr std::array ScenarioDefinitions{
    ScenarioDefinition{Scenario::Dullsville, 1, "snro.111", "Dullsville", 1900, 5000},
    ScenarioDefinition{Scenario::SanFransisco, 2, "snro.222", "San Francisco", 1906, 20000},
    ScenarioDefinition{Scenario::Hamburg, 3, "snro.333", "Hamburg", 1944, 20000},
    ScenarioDefinition{Scenario::Bern, 4, "snro.444", "Bern", 1965, 20000},
    ScenarioDefinition{Scenario::Tokyo, 5, "snro.555", "Tokyo", 1957, 20000},
    ScenarioDefinition{Scenario::Detroit, 6, "snro.666", "Detroit", 1972, 20000},
    ScenarioDefinition{Scenario::Boston, 7, "snro.777", "Boston", 2010, 20000},
    ScenarioDefinition{Scenario::Rio, 8, "snro.888", "Rio de Janeiro", 2047, 20000}
};

inline const ScenarioDefinition* scenarioDefinition(Scenario scenario)
{
    for (const auto& definition : ScenarioDefinitions)
    {
        if (definition.scenario == scenario) { return &definition; }
    }
    return nullptr;
}

inline std::optional<Scenario> scenarioFromLegacyId(int id)
{
    for (const auto& definition : ScenarioDefinitions)
    {
        if (definition.legacyId == id) { return definition.scenario; }
    }
    return std::nullopt;
}

// The bundled SDLPP Windows files use 32-bit little-endian words. This is not
// the older 16-bit city format; the inherited .cty writer remains unchanged.
struct ScenarioData
{
    std::array<GraphHistory, 7> histories{};
    std::array<int, SimWidth * SimHeight> tiles{};
};

inline constexpr std::streamoff ScenarioFileSize = (7 * HistoryLength + SimWidth * SimHeight) * 4;

inline bool validScenarioData(const ScenarioData& candidate)
{
    for (int tile : candidate.tiles)
        { if (tile < 0 || tile > 0xffff || (tile & LowerMask) >= TILE_COUNT) { return false; } }
    return candidate.histories[6][15] >= 0 && candidate.histories[6][15] <= 2;
}

inline ScenarioResult decodeScenarioData(std::span<const char> bytes, ScenarioData& output)
{
    static_assert(sizeof(int)==4 && std::endian::native==std::endian::little);
    static_assert(sizeof(ScenarioData)==ScenarioFileSize);
    if (bytes.size()!=static_cast<size_t>(ScenarioFileSize)) { return ScenarioResult::InvalidFile; }
    ScenarioData candidate;
    std::memcpy(&candidate,bytes.data(),sizeof(candidate));
    if (!validScenarioData(candidate)) { return ScenarioResult::InvalidFile; }
    output=candidate;
    return ScenarioResult::Success;
}

inline ScenarioResult readScenarioData(const std::filesystem::path& path, ScenarioData& output)
{
    static_assert(sizeof(int) == 4 && std::endian::native == std::endian::little);
    std::ifstream input(path, std::ios::binary | std::ios::ate);
    if (!input)
    {
        std::error_code error;
        return std::filesystem::exists(path, error) || error ?
            ScenarioResult::InvalidFile : ScenarioResult::MissingFile;
    }
    if (input.tellg() != ScenarioFileSize) { return ScenarioResult::InvalidFile; }
    input.seekg(0);
    ScenarioData candidate;
    for (auto& history : candidate.histories)
    {
        input.read(reinterpret_cast<char*>(history.data()), sizeof(history));
    }
    input.read(reinterpret_cast<char*>(candidate.tiles.data()), sizeof(candidate.tiles));
    if (!input || input.peek() != std::char_traits<char>::eof()) { return ScenarioResult::InvalidFile; }
    // SimLoadInit uses the serialized game level to index simulation tables.
    if (!validScenarioData(candidate)) { return ScenarioResult::InvalidFile; }
    output = candidate;
    return ScenarioResult::Success;
}
