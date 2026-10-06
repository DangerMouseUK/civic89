// Civic 89 headless tests. SPDX-License-Identifier: GPL-3.0-or-later
#include "Budget.h"
#include "CityProperties.h"
#include "EngineDigest.h"
#include "EngineServices.h"
#include "EngineState.h"
#include "EngineStep.h"
#include "FileIo.h"
#include "Map.h"
#include "ScenarioData.h"
#include "Sprite.h"
#include "ToolActions.h"
#include "ToolManager.h"
#include "Util.h"
#include "s_alloc.h"
#include "s_gen.h"
#include "s_sim.h"

#include <array>
#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace
{
    void require(bool value, const char* message)
    {
        if (!value) { throw std::runtime_error(message); }
    }
    struct TemporaryDirectory
    {
        std::filesystem::path path;
        TemporaryDirectory()
        {
            const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
            path = std::filesystem::temp_directory_path() / ("civic89-m2-" + std::to_string(stamp));
            require(std::filesystem::create_directory(path), "Cannot create isolated test directory");
        }
        ~TemporaryDirectory() { std::error_code error; std::filesystem::remove_all(path, error); }
    };

    void golden(const std::filesystem::path& root, int scenario, unsigned ticks)
    {
        // Each golden runs in a fresh process, matching the captured M1 startup.
        Budget budget;
        CityProperties properties;
        initializeEngine(properties, budget);
        seedSimulationRandom(12345);
        require(LoadScenario(static_cast<Scenario>(scenario - 1), properties, budget, root / "scenarios")
            == ScenarioResult::Success, "Golden scenario failed to load");
        for (unsigned tick = 0; tick < ticks; ++tick) { stepEngine(properties, budget); }
        const auto actual = engineStateDigest(properties, budget);
        std::ifstream fixture(root / "tests/fixtures/m2-goldens.txt");
        require(static_cast<bool>(fixture), "Cannot open golden fixture");
        std::string line;
        while (std::getline(fixture, line))
        {
            if (line.empty() || line[0] == '#') { continue; }
            std::istringstream row(line);
            int id; unsigned phases; std::uint64_t debug, release;
            row >> id >> phases >> std::hex >> debug >> release;
            require(static_cast<bool>(row), "Malformed golden fixture");
            if (id != scenario || phases != ticks) { continue; }
#ifdef CIVIC89_FAST_FLOAT
            const auto expected = release;
#else
            const auto expected = debug;
#endif
            std::cout << "scenario=" << id << " ticks=" << phases << " expected=" << std::hex
                << expected << " actual=" << actual << '\n';
            require(actual == expected, "Engine diverged from pre-extraction M1 baseline");
            return;
        }
        throw std::runtime_error("No matching golden fixture");
    }

    void smoke(const std::filesystem::path& root)
    {
        Budget budget;
        CityProperties properties;
        initializeEngine(properties, budget);
        seedSimulationRandom(42);
        const int first = randomRange(0, 1000000);
        seedSimulationRandom(42);
        require(randomRange(0, 1000000) == first, "Explicit RNG seeding is not repeatable");
        require(coordinatesValid({118,98}) && !coordinatesValid({119,99}) && !coordinatesValid({-1,0}),
            "Inherited exclusive map edges changed");
        require(LoadScenario(Scenario::Detroit, properties, budget, root / "scenarios") == ScenarioResult::Success,
            "Headless scenario load failed");
        const auto loaded = engineStateDigest(properties, budget);
        simSpeed(SimulationSpeed::Paused);
        stepEngine(properties, budget);
        require(engineStateDigest(properties, budget) == loaded, "Paused simulation changed city state");
        simSpeed(SimulationSpeed::Normal);
        for (unsigned i = 0; i < 256; ++i) { stepEngine(properties, budget); }
        require(engineStateDigest(properties, budget) != loaded, "Simulation stepping did not advance");

        TemporaryDirectory temporary;
        const auto save = temporary.path / "headless.cty";
        SaveCity(save.string(), properties, budget);
        require(std::filesystem::file_size(save) == ScenarioFileSize, "Legacy writer layout changed");
        ScenarioData saved;
        require(readScenarioData(save, saved) == ScenarioResult::Success, "Own-save bytes cannot be decoded");
        for (int x = 0; x < SimWidth; ++x)
            for (int y = 0; y < SimHeight; ++y)
                require(saved.tiles[static_cast<size_t>(x) * SimHeight + y] == tileValue(x,y), "Writer changed tile ordering");
        const int funds = budget.CurrentFunds();
        require(LoadCity(save.string(), properties, budget), "Headless current-format city load failed");
        require(properties.CityName() == "headless" && budget.CurrentFunds() == funds, "City metadata load changed");
        const auto beforeFailure = engineStateDigest(properties, budget);
        const auto bad = temporary.path / "bad.cty";
        for (const size_t size : std::array<size_t, 5>{0, 1, 27120, ScenarioFileSize - 1, ScenarioFileSize + 1})
        {
            std::ofstream stream(bad, std::ios::binary);
            const std::string bytes(size, '\0'); stream.write(bytes.data(), static_cast<std::streamsize>(bytes.size())); stream.close();
            require(!LoadCity(bad.string(), properties, budget), "Truncated or oversized city accepted");
            require(engineStateDigest(properties, budget) == beforeFailure && properties.CityName() == "headless",
                "Rejected city changed live state");
        }
        const auto writeBad = [&]()
        {
            std::ofstream stream(bad, std::ios::binary);
            for (auto& history : saved.histories) { stream.write(reinterpret_cast<const char*>(history.data()), sizeof(GraphHistory)); }
            stream.write(reinterpret_cast<const char*>(saved.tiles.data()), sizeof(saved.tiles));
        };
        for (int speed : {-1, 5, 100})
        {
            saved.histories[6][57] = speed; writeBad();
            require(!LoadCity(bad.string(), properties, budget), "Invalid simulation speed accepted");
            require(engineStateDigest(properties, budget) == beforeFailure, "Bad speed changed live state");
        }
        saved.histories[6][57] = 2;
        saved.tiles[0] = TILE_COUNT; writeBad();
        require(!LoadCity(bad.string(), properties, budget), "Invalid tile accepted");
        require(engineStateDigest(properties, budget) == beforeFailure, "Bad tile changed live state");
        require(!LoadCity((temporary.path / "missing.cty").string(), properties, budget), "Missing city accepted");

        // Editing data is passed into the engine; no JSON or texture assets needed.
        initializeEngine(properties, budget);
        ToolManager tools({Tool{}, {Tool::Type::Road, 10, 1, 0, true, "Road"},
            {Tool::Type::Wire, 5, 1, 0, true, "Wire"}});
        tools.currentTool(Tool::Type::Road);
        const int originalFunds = budget.CurrentFunds();
        ToolActions::executeTool({60,50}, budget, tools);
        require(tileIsRoad({60,50}) && budget.CurrentFunds() == originalFunds - 10, "Headless road placement/cost changed");
        tools.currentTool(Tool::Type::Wire);
        ToolActions::executeTool({60,51}, budget, tools);
        require((tileValue(60,51) & ConductiveBit) && budget.CurrentFunds() == originalFunds - 15,
            "Headless wire placement/cost changed");
        ToolActions::executeTool({-1,-1}, budget, tools);
        require(budget.CurrentFunds() == originalFunds - 15, "Out-of-bounds edit charged funds");

        initializeEngine(properties, budget);
        seedSimulationRandom(2468);
        GenerateNewCity(properties, budget);
        std::vector<int> terrain;
        for (int x = 0; x < SimWidth; ++x)
            for (int y = 0; y < SimHeight; ++y) { terrain.push_back(tileValue(x,y)); }
        initializeEngine(properties, budget);
        seedSimulationRandom(2468);
        GenerateNewCity(properties, budget);
        for (int x = 0; x < SimWidth; ++x)
            for (int y = 0; y < SimHeight; ++y)
                require(tileValue(x,y) == terrain[static_cast<size_t>(x) * SimHeight + y], "Seeded generation changed between runs");
        simSpeed(SimulationSpeed::Normal);
        generateExplosion({300,300});
        for (unsigned i = 0; i < 8; ++i) { updateSprites(); }
        destroyAllSprites();
        std::cout << "Headless load/step/edit/save/reload/rejection/generation/sprite smoke passed\n";
    }
}

int main(int argc, char* argv[])
{
    try
    {
        if (argc == 3 && std::string_view(argv[1]) == "--smoke") { smoke(argv[2]); }
        else if (argc == 5 && std::string_view(argv[1]) == "--golden")
            { golden(argv[2], std::stoi(argv[3]), static_cast<unsigned>(std::stoul(argv[4]))); }
        else { throw std::runtime_error("Invalid engine test arguments"); }
        return 0;
    }
    catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
