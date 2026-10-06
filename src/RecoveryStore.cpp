// Civic 89 autosave foundation. SPDX-License-Identifier: GPL-3.0-or-later
#include "RecoveryStore.h"
#include "FileIo.h"
#include "CityProperties.h"
bool RecoveryStore::available() const
{
    RulesetId ruleset{};
    return static_cast<bool>(InspectCity(path(),ruleset));
}
std::optional<std::filesystem::path> RecoveryStore::latest() const
{
    std::optional<std::filesystem::path> selected;
    std::filesystem::file_time_type newest{};
    for (const auto& rules : Rulesets)
    {
        const auto candidate=path(rules.id);
        RulesetId identity{};
        if (!InspectCity(candidate,identity)) { continue; }
        std::error_code error;
        const auto modified=std::filesystem::last_write_time(candidate,error);
        if (!error && (!selected || modified>newest)) { selected=candidate; newest=modified; }
    }
    return selected;
}
CityIoResult RecoveryStore::save(const CityProperties& properties, const Budget& budget, AtomicFileWriter& writer)
{
    std::error_code error;
    std::filesystem::create_directories(directory, error);
    const auto destination=path(properties.rulesetId());
    if (error) { return {CityIoCode::CreateFailed, destination, error.message()}; }
    return SaveCity(destination, properties, budget, writer);
}
