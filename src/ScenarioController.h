// Civic 89 native scenario lifecycle. SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "FileIo.h"
#include "PresentationEvents.h"

#include <utility>

class ScenarioController final
{
public:
    ScenarioController(CityProperties& properties, Budget& budget, PresentationEvents& presentation,
        std::filesystem::path directory = "scenarios")
        : mProperties(properties), mBudget(budget), mPresentation(presentation), mDirectory(std::move(directory)) {}

    ScenarioResult start(Scenario scenario)
    {
        const auto result = LoadScenario(scenario, mProperties, mBudget, mDirectory);
        if (result == ScenarioResult::Success) { mPresentation.scenarioStarted(scenario); }
        return result;
    }

private:
    CityProperties& mProperties;
    Budget& mBudget;
    PresentationEvents& mPresentation;
    std::filesystem::path mDirectory;
};

struct ScenarioScore
{
    bool metropolis;
    int traffic;
    int score;
    int crime;
};

inline ScenarioOutcome scenarioOutcome(Scenario scenario, ScenarioScore values)
{
    bool won = false;
    switch (scenario)
    {
    case Scenario::Dullsville:
    case Scenario::SanFransisco:
    case Scenario::Hamburg: won = values.metropolis; break;
    case Scenario::Bern: won = values.traffic < 80; break;
    case Scenario::Tokyo:
    case Scenario::Boston:
    case Scenario::Rio: won = values.score > 500; break;
    case Scenario::Detroit: won = values.crime < 60; break;
    }
    return won ? ScenarioOutcome::Won : ScenarioOutcome::Lost;
}
