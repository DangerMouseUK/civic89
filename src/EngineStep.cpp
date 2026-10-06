// Civic 89 engine support. SPDX-License-Identifier: GPL-3.0-or-later
#include "EngineStep.h"
#include "Evaluation.h"
#include "s_sim.h"
#include "w_update.h"

void stepEngine(CityProperties& properties, Budget& budget)
{
    SimFrame(properties, budget);
    updateDate();
    refreshCityEvaluation(properties);
}
