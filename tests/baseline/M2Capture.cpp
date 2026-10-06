// Civic 89 pre-extraction probe. SPDX-License-Identifier: GPL-3.0-or-later
#define _CRT_SECURE_NO_WARNINGS
#include "Budget.h"
#include "CityProperties.h"
#include "EngineDigest.h"
#include "FileIo.h"
#include "PresentationEvents.h"
#include "Util.h"
#include "Evaluation.h"
#include "s_sim.h"
#include "w_update.h"

#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <stdexcept>

void runMilestoneTests(Budget& budget, CityProperties& properties, PresentationEvents&)
{
    const int scenario = std::atoi(std::getenv("CIVIC89_CAPTURE_SCENARIO"));
    const int ticks = std::atoi(std::getenv("CIVIC89_CAPTURE_TICKS"));
    seedSimulationRandom(12345);
    if (LoadScenario(static_cast<Scenario>(scenario - 1), properties, budget) != ScenarioResult::Success)
        throw std::runtime_error("Baseline city load failed");
    for (int i = 0; i < ticks; ++i)
    {
        SimFrame(properties, budget);
        updateDate();
        refreshCityEvaluation(properties);
    }
    std::cout << "GOLDEN " << scenario << " " << ticks << " " << std::hex
        << std::setfill('0') << std::setw(16) << engineStateDigest(properties, budget) << '\n';
}
