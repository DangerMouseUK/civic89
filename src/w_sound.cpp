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
#include "w_sound.h"
#include "AudioService.h"



/* Sound routines */


static bool SoundInitialized = false;
static bool UserSoundOn = true;
int Dozing = 0;


bool userSoundOn()
{
    return UserSoundOn;
}

void userSoundOn(bool val)
{
    UserSoundOn = val;
}



void InitializeSound()
{
    SoundInitialized = true;

    // The application AudioManager owns samples and the playback device.
}


void ShutDownSound()
{
    SoundInitialized = false;
    Dozing = 0;
}


void MakeSound(SoundId sound, AudioChannel channel, AudioService& audio)
{
    if (!UserSoundOn)
    {
        return;
    }
    if (!SoundInitialized)
    {
        InitializeSound();
    }

    audio.playEffect(sound, channel);
}


void SoundOff(AudioService& audio)
{
    if (!SoundInitialized)
    {
        InitializeSound();
    }
    audio.stopAll();
    Dozing = 0;
}


void StartBulldozer(AudioService& audio)
{
    if (!UserSoundOn)
    {
        return;
    }
    if (!SoundInitialized)
    {
        InitializeSound();
    }
    if (!Dozing)
    {
        audio.startLoop(SoundId::Bulldozer, AudioChannel::Construction);
        Dozing = 1;
    }
}


void StopBulldozer(AudioService& audio)
{
    if (!SoundInitialized)
    {
        return;
    }

    audio.stopLoop(AudioChannel::Construction);
    Dozing = 0;
}
