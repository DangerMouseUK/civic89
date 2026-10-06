// Civic 89 autosave foundation. SPDX-License-Identifier: GPL-3.0-or-later
#include "RecoveryStore.h"
#include "FileIo.h"
#include "ScenarioData.h"
bool RecoveryStore::available() const
{
    ScenarioData data;
    return readScenarioData(path(), data) == ScenarioResult::Success;
}
CityIoResult RecoveryStore::save(const CityProperties& properties, const Budget& budget, AtomicFileWriter& writer)
{
    std::error_code error;
    std::filesystem::create_directories(directory, error);
    if (error) { return {CityIoCode::CreateFailed, path(), error.message()}; }
    return SaveCity(path(), properties, budget, writer);
}
