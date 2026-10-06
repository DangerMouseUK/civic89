// Civic 89 Enhanced container boundary. SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "CityIo.h"
#include "Ruleset.h"
#include <vector>

struct CityDocument
{
    RulesetId ruleset{RulesetId::EnhancedV1};
    std::string name;
    std::vector<char> classicPayload;
};
// Pure codec: parsing does not touch the process-global engine. Publication is
// still performed through AtomicFileWriter after the whole candidate is encoded.
CityIoResult readCityDocument(const std::filesystem::path&, CityDocument&);
CityIoResult encodeCityDocument(const CityDocument&, std::vector<char>&);
bool validEnhancedCityName(std::string_view);
