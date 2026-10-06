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

#include "PresentationEvents.h"
#include "w_sound.h"

#include <iostream>


int earthquake_timer_set = 0;
int earthquake_delay = 3000;


void StopEarthquake()
{
    ShakeNow = 0;
    if (earthquake_timer_set)
    {
        //Tk_DeleteTimerHandler(earthquake_timer_token);
    }
    earthquake_timer_set = 0;
}


void DoEarthQuake(AudioService& audio, PresentationEvents& presentation)
{
    std::cout << "DoEarthQuake" << std::endl;
    MakeSound(SoundId::ExplosionLow, AudioChannel::City, audio);
    presentation.earthquakeStarted();
    ShakeNow++;
    if (earthquake_timer_set)
    {
        //Tk_DeleteTimerHandler(earthquake_timer_token);
    }
    //Tk_CreateTimerHandler(earthquake_delay, (void (*)())StopEarthquake, (ClientData)0);
    earthquake_timer_set = 1;
}
