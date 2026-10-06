// Civic 89 engine adapters. SPDX-License-Identifier: GPL-3.0-or-later
#include "EngineServices.h"
#include "w_sound.h"
#include "w_tk.h"

#include <chrono>

namespace
{
    NullAudioService nullAudio;
    AudioService* audio = &nullAudio;
    int steadyMilliseconds()
    {
        static const auto origin = std::chrono::steady_clock::now();
        const auto elapsed = std::chrono::steady_clock::now() - origin;
        return static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count());
    }
    EngineClock engineClock = steadyMilliseconds;
}

void shareAudioService(AudioService& service) { audio = &service; }
void setEngineClock(EngineClock value) { engineClock = value ? value : steadyMilliseconds; }
int engineMilliseconds() { return engineClock(); }
void MakeSound(SoundId sound, AudioChannel channel) { MakeSound(sound, channel, *audio); }
void DoEarthQuake() { DoEarthQuake(*audio, currentPresentationEvents()); }
void StartBulldozer() { StartBulldozer(*audio); }
void StopBulldozer() { StopBulldozer(*audio); }
void SoundOff() { SoundOff(*audio); }
