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
#include "w_tk.h"
#include "AudioService.h"

#include <string>


/* Sound routines */


static bool SoundInitialized = false;
static bool UserSoundOn = true;
int Dozing = 0;


bool userSoundOn()
{
    return SoundInitialized;
}

void userSoundOn(bool val)
{
    SoundInitialized = val;
}



void InitializeSound()
{
    SoundInitialized = true;

    // load sound samples here
}


void ShutDownSound()
{
    if (SoundInitialized)
    {
        // unload sound samples here
    }
}


void MakeSound(const std::string& channel, const std::string& id)
{
    if (!UserSoundOn)
    {
        return;
    }
    if (!SoundInitialized)
    {
        InitializeSound();
    }

    Eval("UIMakeSound \"" + channel + "\" \"" + id + "\"");
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


void MakeSoundOn(const char* channel, const char* id)
{
    char buf[256]{};

    if (!UserSoundOn)
    {
        return;
    }
    if (!SoundInitialized)
    {
        InitializeSound();
    }

    //sprintf(buf, "UIMakeSoundOn %s \"%s\" \"%s\"", /*Tk_PathName(view->tkwin)*/ "window-path", channel, id);
    Eval(buf);
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
    if ((!UserSoundOn) || (!SoundInitialized))
    {
        return;
    }

    audio.stopLoop(AudioChannel::Construction);
    Dozing = 0;
}
