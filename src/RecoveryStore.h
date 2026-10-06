// Civic 89 autosave foundation. SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "CityIo.h"
class Budget;
class CityProperties;
class RecoveryStore
{
public:
    explicit RecoveryStore(std::filesystem::path directory) : directory(std::move(directory)) {}
    std::filesystem::path path() const { return directory / "autosave.cty"; }
    bool available() const;
    CityIoResult save(const CityProperties&, const Budget&, AtomicFileWriter&);
private:
    std::filesystem::path directory;
};
