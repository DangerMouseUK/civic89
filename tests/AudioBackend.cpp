// Civic 89 M3 mixer acceptance. SPDX-License-Identifier: GPL-3.0-or-later
#include "AudioManager.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
namespace
{
    void require(bool value, const char* message) { if (!value) { throw std::runtime_error(message); } }
    double energy(const std::array<float, 4096>& samples)
    {
        double sum = 0;
        for (float value : samples) { require(std::isfinite(value), "Non-finite audio output"); sum += value * value; }
        return sum;
    }
}
int main()
{
    try
    {
        DiagnosticLog log;
        for (int cycle = 0; cycle < 32; ++cycle)
        {
            AudioManager audio(log, AudioOutput::Memory);
            require(audio.available(), "Offline mixer failed");
            std::array<float,4096> output{};
            for (int id = 0; id < 12; ++id)
            {
                audio.stopAll(); audio.playEffect(static_cast<SoundId>(id), AudioChannel::City);
                require(audio.render(output) && energy(output) > .01, "Typed effect produced silence");
            }
            audio.stopAll(); audio.startLoop(SoundId::Bulldozer, AudioChannel::Construction);
            for (int i = 0; i < 8; ++i) { require(audio.render(output) && energy(output) > .01, "Loop stopped at sample end"); }
            audio.stopLoop(AudioChannel::City);
            require(audio.render(output) && energy(output) > .01, "Stopping another category stopped construction");
            audio.channelVolume(AudioChannel::Construction, 0);
            require(audio.render(output) && energy(output) == 0, "Live loop gain did not update");
            audio.channelVolume(AudioChannel::Construction, 1);
            require(audio.render(output) && energy(output) > .01, "Live loop did not unmute");
            audio.stopLoop(AudioChannel::Construction);
            require(audio.render(output) && energy(output) == 0, "Loop did not stop");
            audio.masterVolume(1); audio.channelVolume(AudioChannel::City, 1);
            audio.playEffect(SoundId::HonkLow, AudioChannel::City); require(audio.render(output), "Render failed");
            const auto full = energy(output);
            audio.stopAll(); audio.masterVolume(.5f);
            audio.playEffect(SoundId::HonkLow, AudioChannel::City); require(audio.render(output), "Render failed");
            require(std::abs(energy(output) / full - .25) < .001, "Master gain incorrect");
            audio.stopAll(); audio.channelVolume(AudioChannel::City, 0);
            audio.playEffect(SoundId::Siren, AudioChannel::City);
            require(audio.render(output) && energy(output) == 0, "Category mute failed");
            audio.startLoop(SoundId::Bulldozer, AudioChannel::Construction); audio.enabled(false);
            audio.playEffect(SoundId::ExplosionLow, AudioChannel::City);
            require(audio.render(output) && energy(output) == 0, "Mute did not stop all playback");
            audio.masterVolume(std::numeric_limits<float>::quiet_NaN());
            require(audio.masterVolume() == 0, "Non-finite volume was not sanitized");
            audio.masterVolume(2); require(audio.masterVolume() == 1, "Volume upper bound was not clamped");
            audio.enabled(true); audio.startLoop(SoundId::Bulldozer, AudioChannel::Construction);
            require(audio.render(output) && energy(output) > .01, "Unmuted mixer did not restart");
        }
        { AudioManager failed(log, AudioOutput::Device, 123456); require(!failed.available(), "Invalid device accepted");
          failed.playEffect(SoundId::Siren, AudioChannel::City); failed.stopAll(); }
        { AudioManager device(log); require(device.available(), "Dummy playback device failed");
          device.playEffect(SoundId::Siren, AudioChannel::City); device.stopAll(); }
        SDL_Quit();
        std::cout << "12 effects, gain/mute, category loops, 32 mixer lifetimes and device failure fallback passed\n";
        return 0;
    }
    catch (const std::exception& error) { std::cerr << error.what() << '\n'; SDL_Quit(); return 1; }
}
