// Civic 89 headless runner. SPDX-License-Identifier: GPL-3.0-or-later
#include "Budget.h"
#include "CityProperties.h"
#include "EngineDigest.h"
#include "EngineState.h"
#include "EngineStep.h"
#include "Evaluation.h"
#include "FileIo.h"
#include "ScenarioData.h"
#include "Util.h"
#include "s_gen.h"
#include "s_sim.h"

#include <charconv>
#include <iomanip>
#include <iostream>
#include <limits>
#include <optional>
#include <string_view>

namespace
{
    bool number(std::string_view text, unsigned int& value)
    {
        const auto result = std::from_chars(text.data(), text.data() + text.size(), value);
        return result.ec == std::errc{} && result.ptr == text.data() + text.size();
    }
    int usage()
    {
        std::cerr << "Usage: civic89_runner (--city FILE | --scenario 1..8 | --generate) "
            "[--scenario-dir DIR] [--seed UINT32] [--speed 1..4] [--ticks 0..1000000] [--mode classic|enhanced]\n"
            "A tick is one inherited SimFrame phase; 16 phases advance the city clock once.\n"
            "--mode retains M7 save identities; both use the same original gameplay.\n";
        return 2;
    }
}

int main(int argc, char* argv[])
{
    std::optional<unsigned int> seed;
    std::optional<Scenario> scenario;
    std::optional<SimulationSpeed> speed;
    std::optional<RulesetId> mode;
    std::string city;
    std::filesystem::path directory = "scenarios";
    unsigned int ticks = 0;
    bool generate = false;
    bool ticksSet = false, directorySet = false;
    for (int i = 1; i < argc; ++i)
    {
        const std::string_view argument(argv[i]);
        if (argument == "--generate")
        {
            if (generate) { return usage(); }
            generate = true;
            continue;
        }
        if (i + 1 == argc) { return usage(); }
        const std::string_view value(argv[++i]);
        unsigned int parsed = 0;
        if (argument == "--city" && city.empty() && !value.empty()) { city = value; }
        else if (argument == "--scenario" && !scenario && number(value, parsed) && parsed >= 1 && parsed <= 8)
            { scenario = static_cast<Scenario>(parsed - 1); }
        else if (argument == "--scenario-dir" && !directorySet && !value.empty())
            { directory = value; directorySet = true; }
        else if (argument == "--seed" && !seed && number(value, parsed)) { seed = parsed; }
        else if (argument == "--speed" && !speed && number(value, parsed) && parsed >= 1 && parsed <= 4)
            { speed = static_cast<SimulationSpeed>(parsed); }
        else if (argument == "--ticks" && !ticksSet && number(value, parsed) && parsed <= 1000000)
            { ticks = parsed; ticksSet = true; }
        else if (argument=="--mode" && !mode) { mode=rulesetFromMode(value); if (!mode) { return usage(); } }
        else { return usage(); }
    }
    if (static_cast<int>(scenario.has_value()) + static_cast<int>(!city.empty()) + static_cast<int>(generate) != 1 ||
        (directorySet && !scenario) || (scenario && mode && *mode!=RulesetId::ClassicV1)) { return usage(); }

    try
    {
        Budget budget;
        CityProperties properties;
        initializeEngine(properties, budget, mode.value_or(RulesetId::ClassicV1));
        if (seed) { seedSimulationRandom(*seed); }
        if (scenario)
        {
            if (LoadScenario(*scenario, properties, budget, directory) != ScenarioResult::Success)
                { std::cerr << "Cannot load scenario from " << directory << '\n'; return 1; }
        }
        else if (generate)
        {
            GenerateNewCity(properties, budget); simSpeed(SimulationSpeed::Normal);
            if (properties.rulesetId()==RulesetId::EnhancedV1) { properties.CityName("NoWhere"); }
        }
        else
        {
            RulesetId identity{};
            const auto inspected=InspectCity(pathFromUtf8(city),identity);
            if (!inspected || (mode && *mode!=identity))
                { std::cerr << "City is invalid or its stored ruleset conflicts with --mode. Use explicit Classic import to convert.\n"; return 1; }
            if (!LoadCity(city,properties,budget)) { return 1; }
        }
        if (speed) { simSpeed(*speed); }
        // Bound the inherited signed city clock before stepping external state.
        if (CityTime > std::numeric_limits<int>::max() - static_cast<int>(ticks / 16 + 1))
            { std::cerr << "City clock exceeds supported run range\n"; return 1; }
        for (unsigned int tick = 0; tick < ticks; ++tick) { stepEngine(properties, budget); }
        const auto& rci = currentRCI();
        std::cout << "city=" << properties.CityName() << " ticks=" << ticks << " city_time=" << CityTime
            << " funds=" << budget.CurrentFunds() << " population=" << cityPopulation()
            << " score=" << cityScore() << " rci=" << rci.residentialDemand() << ','
            << rci.commercialDemand() << ',' << rci.industrialDemand() << " seed=";
        if (seed) { std::cout << *seed; } else { std::cout << "random"; }
        std::cout << " ruleset=" << findRuleset(properties.rulesetId())->name;
        std::cout << " digest_v1=" << std::hex << std::setfill('0') << std::setw(16)
            << engineStateDigest(properties, budget) << '\n';
        return 0;
    }
    catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
