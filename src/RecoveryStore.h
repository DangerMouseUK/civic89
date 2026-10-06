// Civic 89 autosave foundation. SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "CityIo.h"
#include "Ruleset.h"
#include <optional>
class Budget;
class CityProperties;
class RecoveryStore
{
public:
    explicit RecoveryStore(std::filesystem::path directory) : directory(std::move(directory)) {}
    std::filesystem::path path() const { return directory / "autosave.cty"; }
    std::filesystem::path path(RulesetId ruleset) const { return directory / (ruleset==RulesetId::EnhancedV1 ? "autosave.c89" : "autosave.cty"); }
    bool available() const;
    std::optional<std::filesystem::path> latest() const;
    CityIoResult save(const CityProperties&, const Budget&, AtomicFileWriter&);
private:
    std::filesystem::path directory;
};
