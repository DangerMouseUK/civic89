// Civic 89 SDL3_mixer adapter. SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "AudioService.h"
#include "DiagnosticLog.h"
#include <SDL3_mixer/SDL_mixer.h>
#include <array>
#include <span>
enum class AudioOutput { Device, Memory };
class AudioManager final : public AudioService
{
public:
    explicit AudioManager(DiagnosticLog&, AudioOutput = AudioOutput::Device,
        SDL_AudioDeviceID device = SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK);
    ~AudioManager();
    AudioManager(const AudioManager&) = delete;
    AudioManager& operator=(const AudioManager&) = delete;
    bool available() const noexcept { return mixer != nullptr; }
    void playEffect(SoundId, AudioChannel) override;
    void startLoop(SoundId, AudioChannel) override;
    void stopLoop(AudioChannel) override;
    void stopAll() override;
    void enabled(bool);
    void masterVolume(float);
    void channelVolume(AudioChannel, float);
    float masterVolume() const noexcept { return master; }
    float channelVolume(AudioChannel channel) const { return gains.at(static_cast<size_t>(channel)); }
    bool render(std::span<float>);
private:
    struct Voice { MIX_Track* track{}; AudioChannel channel{AudioChannel::City}; };
    void release() noexcept;
    void start(MIX_Track*, SoundId, AudioChannel, bool loop);
    DiagnosticLog& log;
    bool initialized{};
    bool soundEnabled{true};
    bool offline{};
    float master{0.7f};
    std::array<float, 2> gains{1.0f, 1.0f};
    MIX_Mixer* mixer{};
    std::array<MIX_Audio*, 12> sounds{};
    std::array<Voice, 16> voices{};
    std::array<MIX_Track*, 2> loops{};
    size_t nextVoice{};
};
