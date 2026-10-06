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

#include "Budget.h"
#include "CityProperties.h"
#include "Map.h"

#include "s_alloc.h"
#include "FileIo.h"
#include "ScenarioData.h"
#include "s_sim.h"
#include "s_msg.h"

#include "w_sound.h"
#include "w_tk.h"
#include "w_update.h"
#include "Util.h"

#include <algorithm>
#include <filesystem>
#include <limits>
#include <string>
#include <vector>

namespace
{
    void restoreData(const ScenarioData& data)
    {
        ResidentialPopulationHistory = data.histories[0];
        CommercialPopulationHistory = data.histories[1];
        IndustrialPopulationHistory = data.histories[2];
        CrimeHistory = data.histories[3];
        PollutionHistory = data.histories[4];
        MoneyHis = data.histories[5];
        MiscHistory = data.histories[6];
        for (int x = 0; x < SimWidth; ++x)
            for (int y = 0; y < SimHeight; ++y)
                tileValue(x,y) = data.tiles[static_cast<size_t>(x) * SimHeight + y];
    }
}

CityIoResult LoadCityDetailed(const std::filesystem::path& filename, CityProperties& properties, Budget& budget)
{
    ScenarioData data;
    const auto read = readScenarioData(filename, data);
    if (read != ScenarioResult::Success)
        { return {read == ScenarioResult::MissingFile ? CityIoCode::MissingFile : CityIoCode::InvalidFormat,
            filename, "City file is missing, unreadable, or has an unsupported size/tile layout."}; }
    const auto& misc = data.histories[6];
    if (misc[57] < 0 || misc[57] > static_cast<int>(SimulationSpeed::AfricanSwallow) ||
        misc[58] < 0 || misc[58] > 100 || misc[60] < 0 || misc[60] > 100 || misc[62] < 0 || misc[62] > 100 ||
        misc[8] < 0 || misc[8] > std::numeric_limits<int>::max() - 1000000)
        { return {CityIoCode::InvalidData, filename, "City contains invalid speed, funding, or time values."}; }
    for (int index : {2,3,4,10,11,12,13,14})
        if (misc[index] < 0 || misc[index] > 1000000)
            { return {CityIoCode::InvalidData, filename, "City contains invalid population or census values."}; }
    for (int index : {5,6,7})
        if (misc[index] < -1000000 || misc[index] > 1000000)
            { return {CityIoCode::InvalidData, filename, "City contains invalid demand values."}; }
    // No session mutation occurs until the entire candidate is validated.
    const auto name = pathUtf8(filename.stem());
    SoundOff();
    StopEarthquake();
    ClearMes();
    initWillStuff();
    restoreData(data);
    CityTime = misc[8];
    budget.CurrentFunds(misc[50]);
    budget.PreviousFunds(misc[51]);
    gameplayOptions().autoBulldoze = misc[52] != 0;
    gameplayOptions().autoBudget = misc[53] != 0;
    gameplayOptions().autoGoto = misc[54] != 0;
    userSoundOn(misc[55] != 0);
    gameplayOptions().soundEnabled = userSoundOn();
    budget.TaxRate(std::clamp(misc[56], 0, 20));
    simSpeed(static_cast<SimulationSpeed>(misc[57]));
    budget.PolicePercent(misc[58] / 100.0f);
    budget.FirePercent(misc[60] / 100.0f);
    budget.RoadPercent(misc[62] / 100.0f);
    ScenarioID = 0;
    InitSimLoad = 1;
    DoInitialEval = false;
    initSimulation(properties, budget);
    properties.CityName(name);
    return {CityIoCode::Success, filename, {}};
}

bool LoadCity(const std::string& filename, CityProperties& properties, Budget& budget)
{
    return static_cast<bool>(LoadCityDetailed(pathFromUtf8(filename), properties, budget));
}

CityIoResult SaveCity(const std::filesystem::path& filename, const CityProperties& properties, const Budget& budget, AtomicFileWriter& writer)
{
    // Serialize a snapshot: a failed save must not alter live history/options.
    std::array<GraphHistory, 7> histories{ResidentialPopulationHistory, CommercialPopulationHistory,
        IndustrialPopulationHistory, CrimeHistory, PollutionHistory, MoneyHis, MiscHistory};
    auto& misc = histories[6];
    misc[8] = CityTime;
    misc[15] = properties.GameLevel();
    misc[50] = budget.CurrentFunds();
    misc[51] = budget.PreviousFunds();
    misc[52] = gameplayOptions().autoBulldoze;
    misc[53] = gameplayOptions().autoBudget;
    misc[54] = gameplayOptions().autoGoto;
    misc[55] = userSoundOn();
    misc[56] = budget.TaxRate();
    misc[57] = static_cast<int>(simSpeed());
    misc[58] = static_cast<int>(budget.PolicePercent() * 100.0f);
    misc[60] = static_cast<int>(budget.FirePercent() * 100.0f);
    misc[62] = static_cast<int>(budget.RoadPercent() * 100.0f);
    std::vector<char> bytes;
    bytes.reserve(static_cast<size_t>(ScenarioFileSize));
    for (const auto& history : histories)
    {
        const auto* begin = reinterpret_cast<const char*>(history.data());
        bytes.insert(bytes.end(), begin, begin + sizeof(history));
    }
    const auto map = getMapData();
    bytes.insert(bytes.end(), map.data, map.data + map.size);
    return writer.write(filename, bytes);
}

ScenarioResult LoadScenario(Scenario scenario, CityProperties& properties, Budget& budget,
    const std::filesystem::path& directory)
{
    const auto* definition = scenarioDefinition(scenario);
    if (!definition) { return ScenarioResult::InvalidScenario; }
    ScenarioData data;
    const auto result = readScenarioData(directory / definition->filename, data);
    if (result != ScenarioResult::Success) { return result; }

    // Validation is complete before any active session state changes.
    SoundOff();
    StopEarthquake();
    ClearMes();
    properties.GameLevel(0);
    properties.CityName(definition->cityName);
    budget.CurrentFunds(definition->funds);
    CityTime = (definition->year - 1900) * 48 + 2;
    ScenarioID = definition->legacyId;

    ResetMap();
    initWillStuff(); // Reset arrays before restoring the validated histories.
    ResidentialPopulationHistory = data.histories[0];
    CommercialPopulationHistory = data.histories[1];
    IndustrialPopulationHistory = data.histories[2];
    CrimeHistory = data.histories[3];
    PollutionHistory = data.histories[4];
    MoneyHis = data.histories[5];
    MiscHistory = data.histories[6];
    MiscHistory[15] = 0; // Catalog difficulty must be applied before SimLoadInit.
    for (int x = 0; x < SimWidth; ++x)
    {
        for (int y = 0; y < SimHeight; ++y)
        {
            tileValue(x, y) = data.tiles[static_cast<size_t>(x) * SimHeight + y];
        }
    }

    simSpeed(SimulationSpeed::Normal);
    updateFunds(budget);
    InitSimLoad = 1;
    DoInitialEval = false;
    initSimulation(properties, budget);
    return ScenarioResult::Success;
}
