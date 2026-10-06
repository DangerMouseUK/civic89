// Civic 89 original synthesised effects. SPDX-License-Identifier: GPL-3.0-or-later
#include "AudioManager.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <climits>
#include <numbers>
#include <vector>
namespace
{
    constexpr SDL_AudioSpec Format{SDL_AUDIO_F32, 1, 22050};
    struct Effect { float frequency; float duration; float noise; };
    constexpr std::array<Effect, 12> Effects{{{70, .7f, .8f}, {95, .5f, .6f}, {150, .45f, .8f},
        {110, .6f, .7f}, {220, .25f, 0}, {110, .8f, 0}, {55, .8f, .35f},
        {330, .25f, 0}, {440, .25f, 0}, {650, 1.2f, 0}, {180, .18f, 0}, {120, .3f, 0}}};
    std::vector<float> synthesize(size_t id)
    {
        const auto effect = Effects.at(id);
        std::vector<float> samples(static_cast<size_t>(effect.duration * Format.freq));
        std::uint32_t noise = 0x898989u + static_cast<std::uint32_t>(id);
        double phase = 0;
        for (size_t i = 0; i < samples.size(); ++i)
        {
            const double time = static_cast<double>(i) / Format.freq;
            const double progress = static_cast<double>(i) / static_cast<double>(samples.size());
            double frequency = effect.frequency;
            if (id == static_cast<size_t>(SoundId::Siren)) { frequency += 180 * std::sin(time * 9); }
            if (id == static_cast<size_t>(SoundId::Monster)) { frequency += 25 * std::sin(time * 15); }
            phase += 2 * std::numbers::pi * frequency / Format.freq;
            noise = noise * 1664525u + 1013904223u;
            const double random = static_cast<double>(noise >> 8) / 8388608.0 - 1;
            const double envelope = std::min(1.0, time * 100) * std::min(1.0, (effect.duration - time) * 100);
            const double decay = id == 0 || id == 2 ? 1 - progress : 1;
            samples[i] = static_cast<float>(.28 * envelope * decay *
                ((1 - effect.noise) * (std::sin(phase) + .2 * std::sin(phase * 2)) + effect.noise * random));
        }
        return samples;
    }
    float volume(float value) { return std::isfinite(value) ? std::clamp(value, 0.0f, 1.0f) : 0.0f; }
}
AudioManager::AudioManager(DiagnosticLog& diagnostics, AudioOutput output, SDL_AudioDeviceID device) :
    log(diagnostics), offline(output == AudioOutput::Memory)
{
    try
    {
        initialized = MIX_Init();
        if (initialized) { mixer = offline ? MIX_CreateMixer(&Format) : MIX_CreateMixerDevice(device, &Format); }
        if (mixer)
        {
            for (size_t i = 0; i < sounds.size(); ++i)
            {
                const auto data = synthesize(i);
                sounds[i] = MIX_LoadRawAudio(mixer, data.data(), data.size() * sizeof(float), &Format);
                if (!sounds[i]) { break; }
            }
            for (auto& voice : voices) { voice.track = MIX_CreateTrack(mixer); if (!voice.track) { break; } }
            for (auto& loop : loops) { loop = MIX_CreateTrack(mixer); if (!loop) { break; } }
            if (std::ranges::all_of(sounds, [](auto* value) { return value != nullptr; }) &&
                std::ranges::all_of(voices, [](const auto& value) { return value.track != nullptr; }) &&
                std::ranges::all_of(loops, [](auto* value) { return value != nullptr; }))
                { masterVolume(master); return; }
        }
        const std::string error = SDL_GetError();
        release();
        log.write(Severity::Warning, DiagnosticCode::AudioUnavailable, "Sound unavailable; continuing silently: " + error);
    }
    catch (...) { release(); throw; }
}
AudioManager::~AudioManager() { release(); }
void AudioManager::release() noexcept
{
    if (mixer) { MIX_DestroyMixer(mixer); mixer = nullptr; }
    for (auto*& sound : sounds) { if (sound) { MIX_DestroyAudio(sound); sound = nullptr; } }
    for (auto& voice : voices) { voice.track = nullptr; }
    loops.fill(nullptr);
    if (initialized) { MIX_Quit(); initialized = false; }
}
void AudioManager::start(MIX_Track* track, SoundId sound, AudioChannel channel, bool loop)
{
    const auto id = static_cast<size_t>(sound), category = static_cast<size_t>(channel);
    if (!mixer || !soundEnabled || id >= sounds.size() || category >= gains.size()) { return; }
    SDL_PropertiesID options = loop ? SDL_CreateProperties() : 0;
    if (loop && (!options || !SDL_SetNumberProperty(options, MIX_PROP_PLAY_LOOPS_NUMBER, -1)))
        { if (options) { SDL_DestroyProperties(options); } log.write(Severity::Warning, DiagnosticCode::AudioPlayback, SDL_GetError()); return; }
    const bool played = MIX_SetTrackAudio(track, sounds[id]) && MIX_SetTrackGain(track, gains[category]) && MIX_PlayTrack(track, options);
    if (options) { SDL_DestroyProperties(options); }
    if (!played) { log.write(Severity::Warning, DiagnosticCode::AudioPlayback, SDL_GetError()); }
}
void AudioManager::playEffect(SoundId sound, AudioChannel channel)
{
    if (!mixer || !soundEnabled) { return; }
    auto& voice = voices[nextVoice++ % voices.size()]; voice.channel = channel;
    start(voice.track, sound, channel, false);
}
void AudioManager::startLoop(SoundId sound, AudioChannel channel)
{
    const auto index = static_cast<size_t>(channel);
    if (index < loops.size()) { start(loops[index], sound, channel, true); }
}
void AudioManager::stopLoop(AudioChannel channel)
{
    const auto index = static_cast<size_t>(channel);
    if (mixer && index < loops.size()) { MIX_StopTrack(loops[index], 0); }
}
void AudioManager::stopAll() { if (mixer) { MIX_StopAllTracks(mixer, 0); } }
void AudioManager::enabled(bool value) { soundEnabled = value; if (!value) { stopAll(); } }
void AudioManager::masterVolume(float value) { master = volume(value); if (mixer) { MIX_SetMixerGain(mixer, master); } }
void AudioManager::channelVolume(AudioChannel channel, float value)
{
    const auto index = static_cast<size_t>(channel);
    if (index >= gains.size()) { return; }
    gains[index] = volume(value);
    if (!mixer) { return; }
    MIX_SetTrackGain(loops[index], gains[index]);
    for (auto& voice : voices) { if (voice.channel == channel) { MIX_SetTrackGain(voice.track, gains[index]); } }
}
bool AudioManager::render(std::span<float> samples)
{
    return mixer && offline && samples.size_bytes() <= static_cast<size_t>(INT_MAX) &&
        MIX_Generate(mixer, samples.data(), static_cast<int>(samples.size_bytes())) >= 0;
}
