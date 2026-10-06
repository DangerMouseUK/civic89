// Civic 89 typed audio lifecycle acceptance. SPDX-License-Identifier: GPL-3.0-or-later
#include "AudioService.h"
#include "PresentationEvents.h"
#include "w_sound.h"
#include "w_tk.h"
#include <iostream>
#include <stdexcept>
int ShakeNow = 0;
extern int earthquake_timer_set;
extern int Dozing;
namespace
{
    void require(bool value, const char* message) { if (!value) { throw std::runtime_error(message); } }
    class RecordingAudio final : public AudioService
    {
    public:
        int effects{}, starts{}, stops{}, all{};
        SoundId last{};
        AudioChannel channel{};
        void playEffect(SoundId sound, AudioChannel target) override { ++effects; last = sound; channel = target; }
        void startLoop(SoundId sound, AudioChannel target) override { ++starts; last = sound; channel = target; }
        void stopLoop(AudioChannel target) override { ++stops; channel = target; }
        void stopAll() override { ++all; }
    };
}
int main()
{
    try
    {
        RecordingAudio audio;
        NullPresentationEvents presentation;
        ShutDownSound(); userSoundOn(true);
        for (int id = 0; id < 12; ++id)
        {
            MakeSound(static_cast<SoundId>(id), AudioChannel::City, audio);
            require(audio.effects == id + 1 && audio.last == static_cast<SoundId>(id) && audio.channel == AudioChannel::City,
                "Typed effect routing failed");
        }
        StartBulldozer(audio); StartBulldozer(audio);
        require(audio.starts == 1 && Dozing == 1 && audio.channel == AudioChannel::Construction,
            "Bulldozer loop did not coalesce");
        userSoundOn(false);
        MakeSound(SoundId::Siren, AudioChannel::City, audio); StartBulldozer(audio);
        require(audio.effects == 12 && audio.starts == 1 && !userSoundOn(), "Disabled sound dispatched playback");
        StopBulldozer(audio);
        require(audio.stops == 1 && Dozing == 0, "Mute prevented stopping an existing loop");
        SoundOff(audio);
        require(audio.all == 1 && !userSoundOn(), "Stop-all changed saved sound preference");
        StopEarthquake(); DoEarthQuake(audio, presentation);
        require(ShakeNow == 1 && earthquake_timer_set == 1 && audio.effects == 12,
            "Muted quake changed presentation or dispatched sound");
        StopEarthquake(); userSoundOn(true); DoEarthQuake(audio, presentation);
        require(ShakeNow == 1 && audio.effects == 13 && audio.last == SoundId::ExplosionLow,
            "Earthquake sound mapping failed");
        StopEarthquake(); ShutDownSound();
        require(Dozing == 0, "Shutdown retained loop state");
        std::cout << "Typed routing, mute persistence, loop and earthquake lifecycle passed\n";
        return 0;
    }
    catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
