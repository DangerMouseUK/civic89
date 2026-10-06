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
#include "EngineState.h"
#include "EngineServices.h"
#include "Util.h"
#include "Budget.h"
#include "CityProperties.h"
#include "Evaluation.h"
#include "Map.h"
#include "Sprite.h"
#include "s_alloc.h"
#include "s_msg.h"
#include "w_sound.h"
#include "w_tk.h"
#include "w_update.h"

int InitSimLoad;
int ScenarioID;
namespace { GameOptions gameOptions; }
GameOptions& gameplayOptions() { return gameOptions; }

void initWillStuff()
{
    RoadEffect = 32;
    PoliceEffect = 1000;
    FireEffect = 1000;
    cityScore(500);
    cityPopulation(-1);
    lastCityTime(-1);
    lastCityYear(1);
    lastCityMonth(Month::Enum::Jan);
	currentPresentationEvents().toolsReset();
    MessageId(NotificationId::None);
    destroyAllSprites();
    DisasterEvent = 0;
    initMapArrays();
}

void initializeEngine(CityProperties& cityProperties, Budget& budget)
{
    SoundOff(); // A new city must release the previous session's loops.
    userSoundOn(true);

    ScenarioID = 0;
    StartingYear = 1900;
    AutoGotoMessageLocation(true);
    CityTime = 50;
    gameOptions = GameOptions{};
    MessageId(NotificationId::None);
    ClearMes();
    simSpeed(SimulationSpeed::Normal);
    ChangeEval();
    MessageLocation({ 0, 0 });

    InitSimLoad = 2;


    InitializeSound();
    StopEarthquake();
    ResetMap();
    initWillStuff();
    budget.CurrentFunds(5000);
    setGameLevelFunds(0, cityProperties, budget);
    simSpeed(SimulationSpeed::Paused);
}
