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

#include "GameOptions.h"
#include "Ruleset.h"

 /* Constants */
extern int CurrentTile; // unmasked tile value
extern int CurrentTileMasked; // masked tile value

extern int PopulationTotal, PreviousPopulationTotal;
extern int ResidentialZoneCount, CommercialZoneCount, IndustrialZoneCount, CombinedZoneCount;
extern int HospitalCount, ChurchCount, StadiumCount;
extern int PoliceStationCount, FireStationCount;
extern int CoalPowerPlantCount, NuclearPowerPlantCount, SeaPortCount, AirportCount;
extern int HospitalBuildCount, ChurchBuildCount;
extern int CrimeAverage, PolluteAverage, LVAverage;

extern int StartingYear;
extern int CityTime;
extern int ScenarioID;
extern int ShakeNow;

extern int RoadEffect, PoliceEffect, FireEffect;

extern int DisasterEvent;
extern int DisasterWait;

extern int ResCap, ComCap, IndCap;

extern int ScoreType;
extern int ScoreWait;

extern int InitSimLoad;
extern bool DoInitialEval;


class Budget;
class CityProperties;
void initWillStuff();
// Initializes the inherited process-global session; call before load/edit/step.
// Concurrent independent cities and simulation threads are not supported.
void initializeEngine(CityProperties&, Budget&, RulesetId = RulesetId::ClassicV1);
GameOptions& gameplayOptions();
