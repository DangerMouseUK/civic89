// Civic 89 engine adapters. SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "AudioService.h"
#include "PresentationEvents.h"

// Services are non-owning and must outlive their registration. Use null services
// before destroying an application adapter. The legacy engine is single-threaded.
void shareAudioService(AudioService&);
PresentationEvents& currentPresentationEvents();
using EngineClock = int(*)();
void setEngineClock(EngineClock); // nullptr restores the steady-clock default.
int engineMilliseconds();
