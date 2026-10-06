// Civic 89 native lifecycle tests. SPDX-License-Identifier: GPL-3.0-or-later
#include "Budget.h"
#include "CityProperties.h"
#include "FileIo.h"
#include "Map.h"
#include "main.h"

#include <array>
#include <filesystem>
#include <iostream>
#include <stdexcept>

namespace
{
    void require(bool condition, const char* message)
    {
        if (!condition) { throw std::runtime_error(message); }
    }
}

void runMilestoneTests(Budget& budget, CityProperties& properties)
{
    constexpr std::array years{1900, 1906, 1944, 1965, 1957, 1972, 2010, 2047};
    constexpr std::array disasterWait{2, 10, 5, 20, 3, 5, 5, 96};
    constexpr std::array scoreWait{1440, 240, 240, 480, 240, 480, 240, 480};
    for (int index = 0; index < 8; ++index)
    {
        LoadScenario(static_cast<Scenario>(index), properties, budget);
        require(ScenarioID == index + 1 && DisasterEvent == index + 1 && ScoreType == index + 1,
            "Scenario ID/counter mapping changed");
        require(CityTime == (years[index] - 1900) * 48 + 2 &&
            budget.CurrentFunds() == (index == 0 ? 5000 : 20000), "Scenario metadata changed");
        require(DisasterWait == disasterWait[index] && ScoreWait == scoreWait[index],
            "Scenario disaster/score deadlines changed");
        require(getMapData().size == 48000 && properties.GameLevel() == 0,
            "Scenario map dimensions/level changed");
    }
    // Characterize the inherited failure before replacing the startup route.
    const auto originalDirectory = std::filesystem::current_path();
    std::filesystem::current_path(originalDirectory.parent_path());
    LoadScenario(Scenario::Dullsville, properties, budget);
    std::filesystem::current_path(originalDirectory);
    require(ScenarioID == 1 && tileValue(60, 50) == 0,
        "Inherited missing-file startup overwrites the session with a blank map");
    std::cout << "Native scenario baseline characterization passed\n";
}
