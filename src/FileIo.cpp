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
#include <fstream>
#include <iostream>
#include <limits>
#include <string>


namespace
{
    void copyBufIntoArray(const int(&buf)[HistoryLength], GraphHistory& graph)
    {
        for (size_t i = 0; i < ResidentialPopulationHistory.size(); ++i)
        {
            graph[i] = buf[i];
        }
    }


    bool loadFile(const std::string filename)
    {
        std::ifstream infile(filename, std::ofstream::binary);
        if (infile.fail())
        {
            infile.close();
            return false;
        }

        int buff[HistoryLength]{};

        infile.read(reinterpret_cast<char*>(&buff[0]), sizeof(GraphHistory));
        copyBufIntoArray(buff, ResidentialPopulationHistory);

        infile.read(reinterpret_cast<char*>(&buff[0]), sizeof(GraphHistory));
        copyBufIntoArray(buff, CommercialPopulationHistory);

        infile.read(reinterpret_cast<char*>(&buff[0]), sizeof(GraphHistory));
        copyBufIntoArray(buff, IndustrialPopulationHistory);

        infile.read(reinterpret_cast<char*>(&buff[0]), sizeof(GraphHistory));
        copyBufIntoArray(buff, CrimeHistory);

        infile.read(reinterpret_cast<char*>(&buff[0]), sizeof(GraphHistory));
        copyBufIntoArray(buff, PollutionHistory);

        infile.read(reinterpret_cast<char*>(&buff[0]), sizeof(GraphHistory));
        copyBufIntoArray(buff, MoneyHis);

        infile.read(reinterpret_cast<char*>(&buff[0]), sizeof(GraphHistory));
        copyBufIntoArray(buff, MiscHistory);

        const auto mapData = getMapData();
        infile.read(const_cast<char*>(mapData.data), mapData.size);

        infile.close();

        return true;
    }


    bool loadGame(const std::string& filename, CityProperties& properties, Budget& budget)
    {
        if (!loadFile(filename))
        {
            return false;
        }

        CityTime = std::clamp(MiscHistory[8], 0, std::numeric_limits<int>::max());
        budget.CurrentFunds(MiscHistory[50]);
        budget.PreviousFunds(MiscHistory[51]);
        gameplayOptions().autoBulldoze = MiscHistory[52];
        gameplayOptions().autoBudget = MiscHistory[53];
        gameplayOptions().autoGoto = MiscHistory[54];

        userSoundOn(MiscHistory[55]);
        budget.TaxRate(std::clamp(MiscHistory[56], 0, 20));
        simSpeed(static_cast<SimulationSpeed>(MiscHistory[57]));

        budget.PolicePercent(static_cast<float>(MiscHistory[58] / 100.0f));
        budget.FirePercent(static_cast<float>(MiscHistory[60] / 100.0f));
        budget.RoadPercent(static_cast<float>(MiscHistory[62] / 100.0f));

        initWillStuff();
        ScenarioID = 0;
        initSimulation(properties, budget);

        return true;
    }


    bool saveGame(const std::string& filename, const CityProperties&, const Budget& budget)
    {
        std::ofstream outfile(filename, std::ofstream::binary);
        if (outfile.fail())
        {
            outfile.close();
            return false;
        }

        MiscHistory[8] = CityTime;
        MiscHistory[50] = budget.CurrentFunds();
        MiscHistory[51] = budget.PreviousFunds();

        MiscHistory[52] = gameplayOptions().autoBulldoze;
        MiscHistory[53] = gameplayOptions().autoBudget;
        MiscHistory[54] = gameplayOptions().autoGoto;
        MiscHistory[55] = userSoundOn();
        MiscHistory[57] = static_cast<int>(simSpeed());
        MiscHistory[56] = budget.TaxRate();

        MiscHistory[58] = static_cast<int>(budget.PolicePercent() * 100.0f);
        MiscHistory[60] = static_cast<int>(budget.FirePercent() * 100.0f);
        MiscHistory[62] = static_cast<int>(budget.RoadPercent() * 100.0f);

        outfile.write(reinterpret_cast<char*>(ResidentialPopulationHistory.data()), sizeof(GraphHistory));
        outfile.write(reinterpret_cast<char*>(CommercialPopulationHistory.data()), sizeof(GraphHistory));
        outfile.write(reinterpret_cast<char*>(IndustrialPopulationHistory.data()), sizeof(GraphHistory));
        outfile.write(reinterpret_cast<char*>(CrimeHistory.data()), sizeof(GraphHistory));
        outfile.write(reinterpret_cast<char*>(PollutionHistory.data()), sizeof(GraphHistory));
        outfile.write(reinterpret_cast<char*>(MoneyHis.data()), sizeof(GraphHistory));
        outfile.write(reinterpret_cast<char*>(MiscHistory.data()), sizeof(GraphHistory));

        const auto mapData = getMapData();
        outfile.write(mapData.data, mapData.size);

        outfile.close();
        return true;
    }


    std::string extractFilenameWithoutExtension(const std::string& filepath)
    {
        return std::filesystem::path(filepath).stem().string();
    }
}


bool LoadCity(const std::string& filename, CityProperties& properties, Budget& budget)
{
    if(!loadGame(filename, properties, budget))
    {
        std::cout << "Unable to load a city from the file named '" << filename << "'" << std::endl;
        return false;
    }

    properties.CityName(extractFilenameWithoutExtension(filename));

    return true;
}


void SaveCity(const std::string& filename, const CityProperties& properties, const Budget& budget)
{
    if (saveGame(filename, properties, budget))
    {
        std::cout << "City saved as '" << filename << "'" << std::endl;
    }
    else
    {
        std::cout << "Unable to save the city to the file named '" << filename << "'" << std::endl;
    }
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
