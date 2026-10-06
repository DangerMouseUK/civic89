// Civic 89 typed audio boundary. SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

// Add sounds/channels as their call paths migrate from the legacy bridge.
enum class SoundId { ExplosionLow, Bulldozer };
enum class AudioChannel { City, Construction };

class AudioService
{
public:
    virtual ~AudioService() = default;
    virtual void playEffect(SoundId sound, AudioChannel channel) = 0;
    virtual void startLoop(SoundId sound, AudioChannel channel) = 0;
    virtual void stopLoop(AudioChannel channel) = 0;
    virtual void stopAll() = 0;
};

// Preserve silent playback until licensed assets and an audio backend exist.
class NullAudioService final : public AudioService
{
public:
    void playEffect(SoundId, AudioChannel) override {}
    void startLoop(SoundId, AudioChannel) override {}
    void stopLoop(AudioChannel) override {}
    void stopAll() override {}
};
