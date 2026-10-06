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

#include <string>
#include <filesystem>

enum class Scenario
{
    Dullsville,
    SanFransisco,
    Hamburg,
    Bern,
    Tokyo,
    Detroit,
    Boston,
    Rio
};

class Budget;
class CityProperties;

enum class ScenarioResult { Success, InvalidScenario, MissingFile, InvalidFile };

bool LoadCity(const std::string& filename, CityProperties&, Budget&);
void SaveCity(const std::string&, const CityProperties&, const Budget&);
ScenarioResult LoadScenario(Scenario, CityProperties&, Budget&,
    const std::filesystem::path& directory = "scenarios");
