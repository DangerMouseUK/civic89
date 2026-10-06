// Civic 89 M3 persistence acceptance. SPDX-License-Identifier: GPL-3.0-or-later
#include "Budget.h"
#include "DiagnosticLog.h"
#include "CityProperties.h"
#include "EngineState.h"
#include "EngineDigest.h"
#include "FileIo.h"
#include "RecoveryStore.h"
#include "ScenarioData.h"
#include "WindowsFileStorage.h"
#include "w_sound.h"
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <chrono>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>

namespace
{
    void require(bool value, const char* message) { if (!value) { throw std::runtime_error(message); } }
    std::string bytes(const std::filesystem::path& path)
    {
        std::ifstream input(path, std::ios::binary);
        return {std::istreambuf_iterator<char>(input), {}};
    }
    class FailedWriter : public AtomicFileWriter
    {
    public:
        CityIoResult write(const std::filesystem::path& path, std::span<const char>) override
            { return {CityIoCode::WriteFailed, path, "Injected short write"}; }
    };
}
int main()
{
    const auto directory = std::filesystem::temp_directory_path() / ("civic89-io-" +
        std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    struct Cleanup { std::filesystem::path path; ~Cleanup() { std::error_code error; std::filesystem::remove_all(path, error); } } cleanup{directory};
    try
    {
        require(std::filesystem::create_directory(directory), "Cannot create isolated test directory");
        Budget budget; CityProperties properties; WindowsFileStorage storage;
        const auto save = directory / L"city-\u57ce.cty";
        const auto rejected = directory / "invalid.cty";
        RecoveryStore recovery(directory / "recovery");
        require(!recovery.available(), "Unexpected recovery candidate");
        for (int cycle = 0; cycle < 100; ++cycle)
        {
            initializeEngine(properties, budget);
            properties.GameLevel(2);
            budget.CurrentFunds(123456 + cycle);
            budget.TaxRate(12);
            budget.PolicePercent(.75f); budget.FirePercent(.5f); budget.RoadPercent(.25f);
            CityTime = 1234;
            userSoundOn(false);
            ResidentialPopulationHistory.fill(cycle + 20);
            const auto history = ResidentialPopulationHistory;
            const auto beforeSave = engineStateDigest(properties, budget);
            require(static_cast<bool>(SaveCity(save, properties, budget, storage)), "Save publication failed");
            require(engineStateDigest(properties, budget) == beforeSave, "Saving mutated live state");
            require(std::filesystem::file_size(save) == ScenarioFileSize, "Legacy byte layout changed");
            initializeEngine(properties, budget);
            const auto loaded = LoadCityDetailed(save, properties, budget);
            require(static_cast<bool>(loaded), "Unicode city reload failed");
            require(ResidentialPopulationHistory == history, "Load erased restored histories");
            require(CityTime == 1234 && budget.CurrentFunds() == 123456 + cycle && properties.GameLevel() == 2,
                "Load lost city metadata/difficulty/funds");
            require(!userSoundOn() && budget.TaxRate() == 12 && budget.PolicePercent() == .75f &&
                budget.FirePercent() == .5f && budget.RoadPercent() == .25f, "Load lost sound or budget options");
        }
        const auto logPath = directory / "diagnostics.log";
        {
            DiagnosticLog log(logPath);
            log.write(Severity::Error, DiagnosticCode::CitySave, "failure\nwith \"quotes\"", save);
        }
        const auto logText = bytes(logPath);
        require(std::count(logText.begin(), logText.end(), '\n') == 1 &&
            logText.find("severity=2 code=4") != std::string::npos && logText.find("city-") != std::string::npos,
            "Structured diagnostic record missing or split across lines");
        const auto original = bytes(save);
        const auto state = engineStateDigest(properties, budget);
        FailedWriter failed;
        require(SaveCity(save, properties, budget, failed).code == CityIoCode::WriteFailed, "Write failure not propagated");
        require(bytes(save) == original && engineStateDigest(properties, budget) == state, "Failed save damaged original or state");
        const HANDLE lock = CreateFileW(save.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
        require(lock != INVALID_HANDLE_VALUE, "Cannot lock target for replacement failure");
        const auto lockedResult = SaveCity(save, properties, budget, storage);
        CloseHandle(lock);
        require(lockedResult.code == CityIoCode::ReplaceFailed && bytes(save) == original, "Locked replacement damaged original");
        require(SaveCity(directory / "missing" / "city.cty", properties, budget, storage).code == CityIoCode::CreateFailed,
            "Missing destination not reported");
        for (auto& entry : std::filesystem::directory_iterator(directory))
            require(pathUtf8(entry.path()).find(".tmp-") == std::string::npos, "Failed save leaked a temporary file");
        std::ofstream(rejected, std::ios::binary) << "truncated";
        require(LoadCityDetailed(rejected, properties, budget).code == CityIoCode::InvalidFormat,
            "Malformed city accepted");
        require(LoadCityDetailed(directory / "absent.cty", properties, budget).code == CityIoCode::MissingFile,
            "Missing city not distinguished");
        require(engineStateDigest(properties, budget) == state, "Rejected load changed active city");
        require(static_cast<bool>(recovery.save(properties, budget, storage)) && recovery.available(), "Recovery save unavailable");
        budget.CurrentFunds(1);
        require(static_cast<bool>(LoadCityDetailed(recovery.path(), properties, budget)) && budget.CurrentFunds() == 123555,
            "Recovery load lost latest city");
        const auto recoveryBefore = bytes(recovery.path());
        require(!recovery.save(properties, budget, failed) && bytes(recovery.path()) == recoveryBefore,
            "Failed autosave erased recovery");
        std::cout << "100 new/save/load cycles, Unicode paths, atomic failure isolation, histories and recovery passed\n";
        return 0;
    }
    catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
