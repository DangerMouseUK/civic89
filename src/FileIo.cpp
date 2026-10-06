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
#include "CityDocument.h"
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
#include <cctype>

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

namespace
{
    struct CityCandidate
    {
        ScenarioData data;
        std::string name;
        RulesetId ruleset{RulesetId::ClassicV1};
    };
    std::string extension(const std::filesystem::path& path)
    {
        auto value=pathUtf8(path.extension());
        std::transform(value.begin(),value.end(),value.begin(),[](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
        return value;
    }
    CityIoResult readCandidate(const std::filesystem::path& filename, CityCandidate& candidate, bool classicOnly=false)
    {
        if (extension(filename)==".c89")
        {
            if (classicOnly) { return {CityIoCode::IncompatibleMode,filename,"Choose a Classic city to import; Enhanced containers already identify their mode."}; }
            CityDocument document;
            const auto read=readCityDocument(filename,document);
            if (!read) { return read; }
            if (decodeScenarioData(document.classicPayload,candidate.data)!=ScenarioResult::Success)
                { return {CityIoCode::InvalidFormat,filename,"Enhanced city has an invalid Classic payload."}; }
            candidate.name=std::move(document.name);
            candidate.ruleset=document.ruleset;
        }
        else
        {
            const auto read=readScenarioData(filename,candidate.data);
            if (read!=ScenarioResult::Success)
                { return {read==ScenarioResult::MissingFile ? CityIoCode::MissingFile : CityIoCode::InvalidFormat,
                    filename,"City file is missing, unreadable, or has an unsupported size/tile layout."}; }
            candidate.name=pathUtf8(filename.stem());
        }
        const auto& misc=candidate.data.histories[6];
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
        return {CityIoCode::Success,filename,{}};
    }
    CityIoResult applyCandidate(const std::filesystem::path& filename, const CityCandidate& candidate, CityProperties& properties, Budget& budget)
    {
        // No session mutation occurs until the entire candidate is validated.
        const auto& data=candidate.data;
        const auto& misc=data.histories[6];
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
        properties.CityName(candidate.name);
        properties.rulesetId(candidate.ruleset);
        return {CityIoCode::Success, filename, {}};
    }
}

CityIoResult LoadCityDetailed(const std::filesystem::path& filename, CityProperties& properties, Budget& budget)
{
    CityCandidate candidate;
    const auto read=readCandidate(filename,candidate);
    if (!read) { return read; }
    return applyCandidate(filename,candidate,properties,budget);
}

CityIoResult InspectCity(const std::filesystem::path& filename, RulesetId& ruleset)
{
    CityCandidate candidate;
    const auto result=readCandidate(filename,candidate);
    if (result) { ruleset=candidate.ruleset; }
    return result;
}

CityIoResult ImportClassicCity(const std::filesystem::path& filename, CityProperties& properties, Budget& budget)
{
    CityCandidate candidate;
    const auto read=readCandidate(filename,candidate,true);
    if (!read) { return read; }
    if (!findRuleset(RulesetId::EnhancedV1)->importsClassic)
        { return {CityIoCode::IncompatibleMode,filename,"This ruleset cannot import Classic cities."}; }
    if (!validEnhancedCityName(candidate.name))
        { return {CityIoCode::InvalidData,filename,"Classic file name cannot be represented as an Enhanced UTF-8 city name (1-255 bytes)."}; }
    candidate.ruleset=RulesetId::EnhancedV1;
    return applyCandidate(filename,candidate,properties,budget);
}

bool LoadCity(const std::string& filename, CityProperties& properties, Budget& budget)
{
    return static_cast<bool>(LoadCityDetailed(pathFromUtf8(filename), properties, budget));
}

namespace
{
    std::vector<char> snapshotCity(const CityProperties& properties, const Budget& budget)
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
        return bytes;
    }
}

CityIoResult SaveCity(const std::filesystem::path& filename, const CityProperties& properties, const Budget& budget, AtomicFileWriter& writer)
{
    const auto* rules=findRuleset(properties.rulesetId());
    if (!rules) { return {CityIoCode::UnsupportedRuleset,filename,"Unsupported city ruleset."}; }
    const auto suffix=extension(filename);
    if (rules->id==RulesetId::ClassicV1)
    {
        if (suffix==".c89") { return {CityIoCode::IncompatibleMode,filename,"Classic cities use .cty; explicitly import to create an Enhanced city."}; }
        return writer.write(filename,snapshotCity(properties,budget));
    }
    if (suffix!=".c89") { return {CityIoCode::IncompatibleMode,filename,"Enhanced cities require .c89. Use Export Classic copy for .cty."}; }
    const CityDocument document{rules->id,properties.CityName(),snapshotCity(properties,budget)};
    std::vector<char> bytes;
    auto result=encodeCityDocument(document,bytes);
    if (!result) { result.path=filename; return result; }
    return writer.write(filename,bytes);
}

CityIoResult ExportClassicCity(const std::filesystem::path& filename, const CityProperties& properties, const Budget& budget, AtomicFileWriter& writer)
{
    const auto* rules=findRuleset(properties.rulesetId());
    if (!rules || !rules->exportsClassic || extension(filename)!=".cty")
        { return {CityIoCode::IncompatibleMode,filename,"Choose .cty for a supported explicit Classic export."}; }
    return writer.write(filename,snapshotCity(properties,budget));
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
    properties.rulesetId(RulesetId::ClassicV1);
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
