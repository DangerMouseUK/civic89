// Civic 89 engine support. SPDX-License-Identifier: GPL-3.0-or-later
#include "EngineDigest.h"

#include "Budget.h"
#include "CityProperties.h"
#include "Evaluation.h"
#include "Map.h"
#include "Sprite.h"
#include "s_alloc.h"
#include "s_sim.h"

#include <bit>

std::uint64_t engineStateDigest(const CityProperties& properties, const Budget& budget)
{
    std::uint64_t digest = 14695981039346656037ULL;
    const auto add = [&digest](int value)
    {
        const auto word = static_cast<std::uint32_t>(value);
        for (unsigned shift = 0; shift < 32; shift += 8)
        {
            digest ^= (word >> shift) & 255U;
            digest *= 1099511628211ULL;
        }
    };
    add(1); // Digest schema version.
    for (int x = 0; x < SimWidth; ++x)
        for (int y = 0; y < SimHeight; ++y) { add(tileValue(x, y)); }
    for (const auto* history : {&ResidentialPopulationHistory, &CommercialPopulationHistory,
        &IndustrialPopulationHistory, &CrimeHistory, &PollutionHistory, &MoneyHis, &MiscHistory,
        &ResHis120Years, &ComHis120Years, &IndHis120Years, &CrimeHis120Years,
        &PollutionHis120Years, &MoneyHis120Years, &MiscHis120Years})
        for (int value : *history) { add(value); }
    for (const auto* map : {&PopulationDensityMap, &TrafficDensityMap, &PollutionMap,
        &LandValueMap, &CrimeMap, &TerrainMem, &RateOfGrowthMap, &FireStationMap,
        &PoliceStationMap, &PoliceProtectionMap, &FireProtectionMap, &ComRate})
    {
        const auto size = map->dimensions();
        for (int y = 0; y < size.y; ++y)
            for (int x = 0; x < size.x; ++x) { add(map->value({x, y})); }
    }
    const auto& rci = currentRCI();
    for (int value : {CityTime, StartingYear, ScenarioID, DisasterEvent, DisasterWait,
        ScoreType, ScoreWait, PopulationTotal, PreviousPopulationTotal,
        ResidentialZoneCount, CommercialZoneCount, IndustrialZoneCount, CombinedZoneCount,
        HospitalCount, ChurchCount, StadiumCount, PoliceStationCount, FireStationCount,
        CoalPowerPlantCount, NuclearPowerPlantCount, SeaPortCount, AirportCount,
        CrimeAverage, PolluteAverage, LVAverage, RoadEffect, PoliceEffect, FireEffect,
        ResCap, ComCap, IndCap, cityPopulation(), cityScore(), static_cast<int>(cityClass()),
        cityAssessedValue(), trafficAverage(), properties.GameLevel(),
        rci.residentialDemand(), rci.commercialDemand(), rci.industrialDemand(),
        budget.CurrentFunds(), budget.PreviousFunds(), budget.TaxRate(), budget.TaxIncome(),
        budget.RoadFundsNeeded(), budget.PoliceFundsNeeded(), budget.FireFundsNeeded(),
        budget.RoadFundsGranted(), budget.PoliceFundsGranted(), budget.FireFundsGranted()}) { add(value); }
    add(static_cast<int>(std::bit_cast<std::uint32_t>(budget.RoadPercent())));
    add(static_cast<int>(std::bit_cast<std::uint32_t>(budget.PolicePercent())));
    add(static_cast<int>(std::bit_cast<std::uint32_t>(budget.FirePercent())));
    // One inherited sprite of each type; texture storage is intentionally excluded.
    for (int type = 0; type < 7; ++type)
    {
        auto* sprite = getSprite(static_cast<SimSprite::Type>(type));
        add(sprite != nullptr);
        if (!sprite) { continue; }
        for (int value : {sprite->frame, sprite->position.x, sprite->position.y,
            sprite->origin.x, sprite->origin.y, sprite->destination.x, sprite->destination.y,
            sprite->count, sprite->sound_count, sprite->dir, sprite->new_dir,
            sprite->step, sprite->flag, sprite->turn, sprite->accel, sprite->speed,
            static_cast<int>(sprite->active)}) { add(value); }
    }
    return digest;
}
