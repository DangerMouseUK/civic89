// This file is part of Micropolis-SDLPP
// Micropolis-SDLPP is based on Micropolis
//
// Copyright © 2022 - 2026 Leeor Dicker
//
// Portions Copyright © 1989-2007 Electronic Arts Inc.
//
// Micropolis-SDLPP is free software; you can redistribute it and/or modify
// it under the terms of the GNU GPLv3, with additional terms. See the README
// file, included in this distribution, for details.
#pragma once

#include <string>
#include "Ruleset.h"

class CityProperties
{
public:
	CityProperties() = default;
	~CityProperties() = default;

	const std::string& CityName() const { return mCityName; }
	void CityName(const std::string& name) { mCityName = name; }
	
	int GameLevel() const { return mGameLevel; }
	void GameLevel(const int level) { mGameLevel = level; }

    RulesetId rulesetId() const { return mRuleset; }
    // Session setup/load/import only; changing presentation preferences must not retag a city.
    bool rulesetId(RulesetId value)
    {
        if (!findRuleset(value)) { return false; }
        mRuleset=value;
        return true;
    }

private:
	int mGameLevel{};
	std::string mCityName{};
    RulesetId mRuleset{RulesetId::ClassicV1};
};
