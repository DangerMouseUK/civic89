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

#include "AudioService.h"

#include <string>

void InitializeSound();
void ShutDownSound();
void MakeSound(const std::string& channel, const std::string& id);
void MakeSound(SoundId sound, AudioChannel channel, AudioService& audio);
void MakeSoundOn(const char* channel, const char* id);
void StartBulldozer();
void StopBulldozer();
void SoundOff();
void DoStartSound(const char* channel, const char* id);
void DoStopSound(const char* id);

bool userSoundOn();
void userSoundOn(bool);
