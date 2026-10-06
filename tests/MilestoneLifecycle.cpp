// Civic 89 native lifecycle tests. SPDX-License-Identifier: GPL-3.0-or-later
#include "Sprite.h"
#include "SpriteRenderer.h"
#include "Budget.h"
#include "CityProperties.h"
#include "Evaluation.h"
#include "FileIo.h"
#include "Map.h"
#include "PresentationEvents.h"
#include "ScenarioController.h"
#include "ScenarioData.h"
#include "main.h"
#include "s_gen.h"
#include "s_msg.h"
#include "s_sim.h"
#include "Util.h"
#include "w_sound.h"
#include "w_tk.h"

#include <array>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <vector>

ScenarioResult doStartScenario(Scenario scenario); // Actual application entry point.
extern int earthquake_timer_set;

namespace
{
    void require(bool condition, const char* message)
    {
        if (!condition) { throw std::runtime_error(message); }
    }

    std::vector<char> readBytes(const std::filesystem::path& path)
    {
        std::ifstream input(path, std::ios::binary);
        require(static_cast<bool>(input), "Fixture is missing");
        return {std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
    }

    uint64_t fixtureDigest(const std::vector<char>& bytes)
    {
        uint64_t digest = 14695981039346656037ull;
        for (char byte : bytes) { digest = (digest ^ static_cast<unsigned char>(byte)) * 1099511628211ull; }
        return digest;
    }

    struct RecordingPresentation final : PresentationEvents
    {
        std::vector<std::string> events;
        std::vector<Scenario> started;
        std::vector<ScenarioOutcome> finished;
        Point<int> focused{};
        bool observedInitializedStart = false;
        bool observedGenerationBeforeMap = false;

        void earthquakeStarted() override { events.push_back("earthquake"); }
        void showMessage(const std::string& message) override { events.push_back("message:" + message); }
        void focusMap(Point<int> location) override
        {
            require(MessageLocation() == location, "Focus must dispatch before clearing message coordinates");
            focused = location;
            events.push_back("focus");
        }
        void generationStarted() override
        {
            observedGenerationBeforeMap = ScenarioID == 0 && CityTime == 0 && tileValue(60, 50) == Dirt;
            events.push_back("generation");
        }
        void scenarioStarted(Scenario scenario) override
        {
            const auto* definition = scenarioDefinition(scenario);
            observedInitializedStart = definition && ScenarioID == definition->legacyId &&
                DisasterEvent == ScenarioID && ScoreType == ScenarioID && !paused() &&
                ShakeNow == 0 && earthquake_timer_set == 0;
            started.push_back(scenario);
            events.push_back("started");
        }
        void scenarioFinished(ScenarioOutcome outcome) override
        {
            require(MessageId() == NotificationId::None && LastMessage().empty(),
                "Outcome must follow message clearing");
            finished.push_back(outcome);
            events.push_back("finished");
        }
    };

    struct TemporaryDirectory
    {
        std::filesystem::path path;
        TemporaryDirectory()
        {
            path = std::filesystem::temp_directory_path() /
                ("civic89-m1-" + std::to_string(SDL_GetTicksNS()));
            require(std::filesystem::create_directory(path), "Unable to create unique test directory");
        }
        ~TemporaryDirectory()
        {
            std::error_code error;
            std::filesystem::remove_all(path, error); // Only this test's newly created directory.
        }
    };

    struct SessionSnapshot
    {
        std::vector<char> map;
        std::array<GraphHistory, 7> histories;
        std::string name;
        int level, funds, previousFunds, time, scenario, disaster, disasterWait, score, scoreWait, shake, timer;
        SimulationSpeed speed;
        NotificationId message;
        Point<int> location;
        std::string lastMessage;

        SessionSnapshot(const Budget& budget, const CityProperties& properties)
            : histories{ResidentialPopulationHistory, CommercialPopulationHistory, IndustrialPopulationHistory,
                CrimeHistory, PollutionHistory, MoneyHis, MiscHistory}, name(properties.CityName()),
            level(properties.GameLevel()), funds(budget.CurrentFunds()), previousFunds(budget.PreviousFunds()),
            time(CityTime), scenario(ScenarioID), disaster(DisasterEvent), disasterWait(DisasterWait),
            score(ScoreType), scoreWait(ScoreWait), shake(ShakeNow), timer(earthquake_timer_set), speed(simSpeed()),
            message(MessageId()), location(MessageLocation()), lastMessage(LastMessage())
        {
            const auto data = getMapData();
            map.assign(data.data, data.data + data.size);
        }
        void verify(const Budget& budget, const CityProperties& properties) const
        {
            const auto data = getMapData();
            require(map.size() == data.size && std::memcmp(map.data(), data.data, data.size) == 0,
                "Failed startup changed the map");
            const std::array current{ResidentialPopulationHistory, CommercialPopulationHistory,
                IndustrialPopulationHistory, CrimeHistory, PollutionHistory, MoneyHis, MiscHistory};
            require(current == histories && name == properties.CityName() && level == properties.GameLevel() &&
                funds == budget.CurrentFunds() && previousFunds == budget.PreviousFunds() && time == CityTime &&
                scenario == ScenarioID && disaster == DisasterEvent && disasterWait == DisasterWait &&
                score == ScoreType && scoreWait == ScoreWait && shake == ShakeNow && timer == earthquake_timer_set &&
                speed == simSpeed() && message == MessageId() && location == MessageLocation() && lastMessage == LastMessage(),
                "Failed startup changed session state");
        }
    };

    void checkFixturesAndStartup(Budget& budget, CityProperties& properties, RecordingPresentation& presentation)
    {
        constexpr std::array<uint64_t, 8> digests{0x08951c117e47b254ull, 0x701bae244e2f92cfull,
            0xa7ab847dfc2a595aull, 0x3009e173ccfc011bull, 0xa146a346b7f55dc0ull,
            0xb81660448509b906ull, 0x653c10971c949a88ull, 0xe4cb0d600f1797b6ull};
        constexpr std::array disasterWait{2, 10, 5, 20, 3, 5, 5, 96};
        constexpr std::array scoreWait{1440, 240, 240, 480, 240, 480, 240, 480};
        ScenarioController controller(properties, budget, presentation);
        for (size_t index = 0; index < ScenarioDefinitions.size(); ++index)
        {
            const auto& definition = ScenarioDefinitions[index];
            const auto path = std::filesystem::path("scenarios") / definition.filename;
            require(fixtureDigest(readBytes(path)) == digests[index], "Packaged scenario bytes changed");
            ScenarioData data;
            require(readScenarioData(path, data) == ScenarioResult::Success, "Packaged fixture validation failed");
            const auto bytes = readBytes(path);
            require(std::memcmp(data.tiles.data(), bytes.data() + 7 * HistoryLength * sizeof(int),
                sizeof(data.tiles)) == 0, "Decoded fixture map differs from its stored words");
            require(scenarioFromLegacyId(definition.legacyId) == definition.scenario,
                "Legacy ID must map to the exact Scenario enum");
            require(controller.start(definition.scenario) == ScenarioResult::Success, "Native scenario startup failed");
            require(presentation.started.size() == index + 1 && presentation.started.back() == definition.scenario &&
                presentation.observedInitializedStart, "Started event must follow successful initialization exactly once");
            require(CityTime == (definition.year - 1900) * 48 + 2 && budget.CurrentFunds() == definition.funds &&
                properties.CityName() == definition.cityName && properties.GameLevel() == 0,
                "Scenario metadata changed");
            require(DisasterWait == disasterWait[index] && ScoreWait == scoreWait[index],
                "Scenario disaster/score deadlines changed");
            require(CrimeHistory == data.histories[3] && MoneyHis == data.histories[5], "History loading changed");
            require(tileValue(0, 0) == data.tiles.front(), "Loaded scenario corner differs from the fixture");
            for (int x = 0; x < SimWidth; ++x)
            {
                for (int y = 0; y < SimHeight; ++y)
                {
                    // MapScan can mutate zones/fires and nearby terrain with inherited RNG.
                    require((tileValue(x, y) & LowerMask) < TILE_COUNT, "Startup produced an invalid tile");
                }
            }
            const auto initialTime = CityTime;
            for (int frame = 0; frame < 64; ++frame) { SimFrame(properties, budget); }
            require(CityTime > initialTime, "Scenario did not advance through native simulation frames");
        }
        require(!scenarioFromLegacyId(0) && !scenarioFromLegacyId(9) && !scenarioFromLegacyId(-1),
            "Invalid legacy IDs must be rejected");
    }

    void checkFailureIsolation(Budget& budget, CityProperties& properties, RecordingPresentation& presentation)
    {
        TemporaryDirectory temporary;
        ScenarioController controller(properties, budget, presentation, temporary.path);
        SendMesAt(NotificationId::FireReported, 12, 34);
        DoEarthQuake();
        pause();
        const SessionSnapshot before(budget, properties);
        const auto eventCount = presentation.events.size();
        const auto startCount = presentation.started.size();
        const auto check = [&](Scenario scenario, ScenarioResult expected)
        {
            require(controller.start(scenario) == expected, "Incorrect scenario failure result");
            before.verify(budget, properties);
            require(presentation.events.size() == eventCount && presentation.started.size() == startCount,
                "Failed startup emitted presentation events");
        };
        check(static_cast<Scenario>(-1), ScenarioResult::InvalidScenario);
        check(static_cast<Scenario>(8), ScenarioResult::InvalidScenario);
        check(Scenario::Dullsville, ScenarioResult::MissingFile);
        const auto original = readBytes("scenarios/snro.111");
        const auto write = [&](const std::vector<char>& bytes)
        {
            std::ofstream output(temporary.path / "snro.111", std::ios::binary);
            output.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
            require(static_cast<bool>(output), "Unable to write malformed test fixture");
        };
        for (const size_t size : std::array<size_t, 4>{0, 1, 27120, original.size() - 1})
        {
            write(std::vector<char>(original.begin(), original.begin() + size));
            check(Scenario::Dullsville, ScenarioResult::InvalidFile);
        }
        auto corrupt = original;
        corrupt.push_back(0);
        write(corrupt);
        check(Scenario::Dullsville, ScenarioResult::InvalidFile);
        corrupt = original;
        const int invalidTile = 960;
        std::memcpy(corrupt.data() + 7 * HistoryLength * sizeof(int), &invalidTile, sizeof(int));
        write(corrupt);
        check(Scenario::Dullsville, ScenarioResult::InvalidFile);
        corrupt = original;
        const int invalidLevel = 3;
        std::memcpy(corrupt.data() + (6 * HistoryLength + 15) * sizeof(int), &invalidLevel, sizeof(int));
        write(corrupt);
        check(Scenario::Dullsville, ScenarioResult::InvalidFile);
        write(original);
        require(controller.start(Scenario::Dullsville) == ScenarioResult::Success && !paused() && ShakeNow == 0,
            "A valid restart must recover after rejected files and cancel stale earthquake state");
        require(presentation.started.size() == startCount + 1, "Successful restart must emit one started event");
    }

    void checkMessageAndGenerationRouting(Budget& budget, CityProperties& properties, RecordingPresentation& presentation)
    {
        ClearMes();
        presentation.events.clear();
        userSoundOn(false);
        AutoGotoMessageLocation(true);
        SendMesAt(NotificationId::TrafficJamsReported, 24, 36);
        doMessage();
        require(presentation.events.size() == 2 && presentation.events[0].starts_with("message:") &&
            presentation.events[1] == "focus" && presentation.focused == Point<int>{24, 36} &&
            MessageLocation() == Point<int>{0, 0}, "Message/focus order changed");
        require(!userSoundOn(), "Dormant notification audio must not be activated");
        doMessage();
        require(presentation.events.size() == 2, "Unchanged message should not be resent");
        AutoGotoMessageLocation(false);
        SendMesAt(NotificationId::MonsterReported, 12, 14);
        doMessage();
        require(presentation.events.back().starts_with("message:") && !userSoundOn(),
            "Disabled navigation or dormant monster audio changed");
        MessageDisplayTime(-1);
        doMessage();
        require(presentation.events.back() == "message:" && MessageId() == NotificationId::None,
            "Expired message must clear presentation");
        MessageDisplayTime(3000);
        ResetMap();
        GenerateCityFromSeed(1234, properties, budget);
        require(presentation.observedGenerationBeforeMap, "Generation event moved after map generation");
    }

    void checkScoring(RecordingPresentation& presentation)
    {
        for (const auto& definition : ScenarioDefinitions)
        {
            require(scenarioOutcome(definition.scenario, {false, 80, 500, 60}) == ScenarioOutcome::Lost,
                "Losing score boundary changed");
            require(scenarioOutcome(definition.scenario, {true, 79, 501, 59}) == ScenarioOutcome::Won,
                "Winning score boundary changed");
        }
        cityClass(CityClass::Village);
        DoScenarioScore(1);
        require(presentation.finished.back() == ScenarioOutcome::Lost, "Actual losing route failed");
        cityClass(CityClass::Metropolis);
        DoScenarioScore(1);
        require(presentation.finished.back() == ScenarioOutcome::Won, "Actual winning route failed");
        for (const auto& definition : ScenarioDefinitions)
        {
            const auto expected = scenarioOutcome(definition.scenario,
                {cityClass() >= CityClass::Metropolis, trafficAverage(), cityScore(), CrimeAverage});
            const auto count = presentation.finished.size();
            DoScenarioScore(definition.legacyId);
            require(presentation.finished.size() == count + 1 && presentation.finished.back() == expected,
                "Actual scoring route must emit one matching typed outcome");
        }
        const auto count = presentation.finished.size();
        DoScenarioScore(0);
        DoScenarioScore(9);
        require(presentation.finished.size() == count, "Invalid scenario scoring must emit no outcome");
    }
}

void runMilestoneTests(Budget& budget, CityProperties& properties, PresentationEvents& appPresentation)
{
    RecordingPresentation presentation;
    sharePresentationEvents(presentation);
    checkFixturesAndStartup(budget, properties, presentation);
    checkFailureIsolation(budget, properties, presentation);
    checkMessageAndGenerationRouting(budget, properties, presentation);
    checkScoring(presentation);
    sharePresentationEvents(appPresentation);
    require(doStartScenario(Scenario::Rio) == ScenarioResult::Success && ScenarioID == 8,
        "Application scenario entry point failed");
    require(doStartScenario(static_cast<Scenario>(8)) == ScenarioResult::InvalidScenario && ScenarioID == 8,
        "Application entry point must retain session on invalid selection");
    generateExplosion({300,300});
    drawSprites();
    clearSpriteImages();
    drawSprites(); // Renderer-owned images can be reconstructed independently of engine state.
    clearSpriteImages();
    clearSpriteImages(); // Empty cache teardown is safe.
    std::cout << "All eight native scenarios, simulation startup, failures, messages, generation and outcomes passed\n";
}
