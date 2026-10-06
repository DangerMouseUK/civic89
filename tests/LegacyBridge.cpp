// Civic 89 legacy characterization tests. SPDX-License-Identifier: GPL-3.0-or-later
#include "PresentationEvents.h"
#include "w_sound.h"
#include "w_tk.h"

#include <array>
#include <iostream>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

// Only the presentation bridge is linked. Supply its simulation-owned state;
// map damage, RNG, renderer behavior and scenario loading are outside this test.
int ShakeNow = 0;
extern int earthquake_timer_set;
extern int earthquake_delay;
extern int Dozing;

namespace
{
    void require(bool condition, const std::string& message)
    {
        if (!condition)
        {
            throw std::runtime_error(message);
        }
    }

    class ConsoleCapture final
    {
    public:
        ConsoleCapture() : previous(std::cout.rdbuf(output.rdbuf())) {}
        ~ConsoleCapture() { std::cout.rdbuf(previous); }
        ConsoleCapture(const ConsoleCapture&) = delete;
        ConsoleCapture& operator=(const ConsoleCapture&) = delete;

        std::string take()
        {
            const auto text = output.str();
            output.str("");
            return text;
        }

        std::string contents() const { return output.str(); }

    private:
        std::ostringstream output;
        std::streambuf* previous;
    };

    enum class AudioRequestKind { Effect, StartLoop, StopLoop, StopAll };

    struct RecordedAudioRequest
    {
        AudioRequestKind kind;
        std::optional<SoundId> sound;
        std::optional<AudioChannel> channel;
        std::string consoleBeforeEffect;
        int shakeBeforeEffect;
        int timerBeforeEffect;
        bool soundInitialized;
        int dozingBeforeRequest;
    };

    class RecordingAudioService final : public AudioService
    {
    public:
        explicit RecordingAudioService(const ConsoleCapture& console) : capture(console) {}

        void playEffect(SoundId sound, AudioChannel channel) override
        {
            record(AudioRequestKind::Effect, sound, channel);
        }

        void startLoop(SoundId sound, AudioChannel channel) override
        {
            record(AudioRequestKind::StartLoop, sound, channel);
        }

        void stopLoop(AudioChannel channel) override
        {
            record(AudioRequestKind::StopLoop, std::nullopt, channel);
        }

        void stopAll() override
        {
            record(AudioRequestKind::StopAll, std::nullopt, std::nullopt);
        }

        std::vector<RecordedAudioRequest> requests;

    private:
        void record(AudioRequestKind kind, std::optional<SoundId> sound, std::optional<AudioChannel> channel)
        {
            requests.push_back({ kind, sound, channel, capture.contents(), ShakeNow, earthquake_timer_set,
                userSoundOn(), Dozing });
        }

        const ConsoleCapture& capture;
    };

    struct RecordedEarthquake
    {
        size_t audioRequestsBeforeEvent;
        std::string consoleBeforeEvent;
        int shakeBeforeEvent;
        int timerBeforeEvent;
        bool soundInitialized;
    };

    class RecordingPresentationEvents final : public PresentationEvents
    {
    public:
        RecordingPresentationEvents(const ConsoleCapture& console, const RecordingAudioService& recordedAudio)
            : capture(console), audio(recordedAudio) {}

        void earthquakeStarted() override
        {
            requests.push_back({ audio.requests.size(), capture.contents(), ShakeNow,
                earthquake_timer_set, userSoundOn() });
        }

        std::vector<RecordedEarthquake> requests;

    private:
        const ConsoleCapture& capture;
        const RecordingAudioService& audio;
    };

    void resetBridge()
    {
        userSoundOn(false);
        Dozing = 0;
        StopEarthquake();
    }

    void checkEvalStub()
    {
        ConsoleCapture console;
        require(!Eval("unsupported command"), "The inherited Eval stub must report false");
        require(console.take() == "Eval: unsupported command\n", "Eval logging changed");
    }

    void checkSoundRouting()
    {
        resetBridge();
        ConsoleCapture console;
        require(!userSoundOn(), "Sound should begin uninitialized");
        MakeSound("city", "Explosion-Low");
        require(userSoundOn(), "MakeSound should lazily initialize sound");
        require(console.take() == "Eval: UIMakeSound \"city\" \"Explosion-Low\"\n",
            "City effect routing changed");

        // These are literal legacy operands, not parsed rates or filenames.
        MakeSound("city", "HonkHonk-Low -speed 80");
        MakeSound("city", "Monster -speed [MonsterSpeed]");
        require(console.take() ==
            "Eval: UIMakeSound \"city\" \"HonkHonk-Low -speed 80\"\n"
            "Eval: UIMakeSound \"city\" \"Monster -speed [MonsterSpeed]\"\n",
            "Legacy speed operands changed before a compatibility decision");

        // Characterize inherited defects so a later fix has an explicit baseline.
        // userSoundOn accesses initialization state, not the private mute flag.
        userSoundOn(false);
        MakeSound("city", "Siren");
        require(userSoundOn() && console.take() == "Eval: UIMakeSound \"city\" \"Siren\"\n",
            "The inherited mute/initialization behavior changed");
        ShutDownSound();
        require(userSoundOn(), "Inherited shutdown does not clear initialization state");
        require(console.take().empty(), "Inherited shutdown should not emit a command");

        userSoundOn(false);
        MakeSoundOn("edit", "UhUh");
        MakeSoundOn("edit", "Sorry");
        require(userSoundOn(), "Tool error sound should lazily initialize sound");
        require(console.take() == "Eval: \nEval: \n",
            "Inherited MakeSoundOn emits empty commands; migration must address this explicitly");
    }

    void checkDefaultEffects()
    {
        resetBridge();
        ConsoleCapture console;
        RecordingAudioService audio(console);
        MakeSound(SoundId::ExplosionHigh, AudioChannel::City, audio);
        MakeSound(SoundId::ExplosionLow, AudioChannel::City, audio);
        MakeSound(SoundId::HeavyTraffic, AudioChannel::City, audio);
        MakeSound(SoundId::HonkLow, AudioChannel::City, audio);
        require(userSoundOn() && Dozing == 0 && ShakeNow == 0 && earthquake_timer_set == 0,
            "Default effects must initialize sound without changing loop or earthquake state");
        constexpr std::array expected{
            SoundId::ExplosionHigh, SoundId::ExplosionLow, SoundId::HeavyTraffic, SoundId::HonkLow };
        require(audio.requests.size() == expected.size(), "Each default effect must dispatch once");
        for (size_t i = 0; i < expected.size(); ++i)
        {
            const auto& request = audio.requests[i];
            require(request.kind == AudioRequestKind::Effect && request.sound == expected[i] &&
                request.channel == AudioChannel::City && request.soundInitialized &&
                request.consoleBeforeEffect.empty() && request.dozingBeforeRequest == 0 &&
                request.shakeBeforeEffect == 0 && request.timerBeforeEffect == 0,
                "Default effects must retain their order and initialize before typed city dispatch");
        }
        userSoundOn(false);
        MakeSound(SoundId::ExplosionHigh, AudioChannel::City, audio);
        require(userSoundOn() && audio.requests.size() == 5 && audio.requests.back().soundInitialized,
            "Typed default effects must retain the inherited mute/initialization behavior");
        require(console.take().empty(), "Typed default effects must not emit string commands");

        resetBridge();
        NullAudioService nullAudio;
        for (const auto sound : expected)
        {
            MakeSound(sound, AudioChannel::City, nullAudio);
        }
        require(userSoundOn() && Dozing == 0 && ShakeNow == 0 && earthquake_timer_set == 0 &&
            console.take().empty(), "Null default effects must preserve state and remain silent");
    }

    void checkBulldozerLifecycle()
    {
        resetBridge();
        ConsoleCapture console;
        RecordingAudioService audio(console);
        StopBulldozer(audio);
        require(audio.requests.empty(), "Stopping uninitialized sound should not dispatch");
        require(!userSoundOn() && Dozing == 0, "Cold stop should not initialize sound or change loop state");

        SoundOff(audio);
        require(userSoundOn() && Dozing == 0, "Cold SoundOff should initialize sound and clear loop state");
        require(audio.requests.size() == 1 && audio.requests.back().kind == AudioRequestKind::StopAll &&
            audio.requests.back().soundInitialized && audio.requests.back().dozingBeforeRequest == 0,
            "Cold SoundOff should initialize before dispatching stop-all");
        resetBridge();
        audio.requests.clear();

        StartBulldozer(audio);
        StartBulldozer(audio);
        require(userSoundOn() && Dozing == 1, "Bulldozer start should set initialization and loop state");
        require(audio.requests.size() == 1 && audio.requests.front().kind == AudioRequestKind::StartLoop &&
            audio.requests.front().sound == SoundId::Bulldozer &&
            audio.requests.front().channel == AudioChannel::Construction &&
            audio.requests.front().soundInitialized && audio.requests.front().dozingBeforeRequest == 0,
            "Repeated start must dispatch one typed construction loop before setting Dozing");

        StopBulldozer(audio);
        StopBulldozer(audio);
        require(Dozing == 0, "Bulldozer stop should clear loop state");
        require(audio.requests.size() == 3 && audio.requests[1].kind == AudioRequestKind::StopLoop &&
            audio.requests[2].kind == AudioRequestKind::StopLoop &&
            audio.requests[1].channel == AudioChannel::Construction &&
            audio.requests[2].channel == AudioChannel::Construction &&
            audio.requests[1].dozingBeforeRequest == 1 && audio.requests[2].dozingBeforeRequest == 0,
            "Repeated stop must dispatch construction stop requests before clearing Dozing");

        audio.requests.clear();
        StartBulldozer(audio);
        SoundOff(audio);
        require(Dozing == 0 && userSoundOn(), "SoundOff clears the loop but leaves initialization set");
        require(audio.requests.size() == 2 && audio.requests[0].kind == AudioRequestKind::StartLoop &&
            audio.requests[1].kind == AudioRequestKind::StopAll && audio.requests[1].dozingBeforeRequest == 1 &&
            audio.requests[1].soundInitialized && !audio.requests[1].sound && !audio.requests[1].channel,
            "SoundOff must stop all before clearing loop state");
        require(console.take().empty(), "Typed controls must not emit legacy string commands");
        MakeSound("city", "Siren");
        require(console.take() == "Eval: UIMakeSound \"city\" \"Siren\"\n",
            "Inherited SoundOff does not prevent later effect requests");

        audio.requests.clear();
        StartBulldozer(audio);
        require(audio.requests.size() == 1 && Dozing == 1, "Loop should restart after SoundOff");
        userSoundOn(false);
        StopBulldozer(audio);
        require(!userSoundOn() && Dozing == 1 && audio.requests.size() == 1,
            "Stop while uninitialized must retain the inherited guard and loop state");
        SoundOff(audio);
        require(userSoundOn() && Dozing == 0 && audio.requests.size() == 2 &&
            audio.requests.back().kind == AudioRequestKind::StopAll &&
            audio.requests.back().dozingBeforeRequest == 1 && audio.requests.back().soundInitialized,
            "SoundOff should reinitialize and clear a guarded loop");
        require(console.take().empty(), "Guarded controls must not emit legacy string commands");

        resetBridge();
        NullAudioService nullAudio;
        StopBulldozer(nullAudio);
        require(!userSoundOn() && Dozing == 0, "Null cold stop must keep the initialization guard");
        StartBulldozer(nullAudio);
        require(userSoundOn() && Dozing == 1, "Null loop start must retain state changes");
        StopBulldozer(nullAudio);
        require(Dozing == 0 && userSoundOn(), "Null loop stop must retain state changes");
        StartBulldozer(nullAudio);
        SoundOff(nullAudio);
        require(Dozing == 0 && userSoundOn() && console.take().empty(),
            "Null sound-off must clear loop state and remain silent");
    }

    void checkEarthquakeLifecycle()
    {
        resetBridge();
        ConsoleCapture console;
        RecordingAudioService audio(console);
        RecordingPresentationEvents presentation(console, audio);
        const std::string expected = "DoEarthQuake\n";
        require(ShakeNow == 0 && earthquake_timer_set == 0, "Earthquake should start reset");
        require(earthquake_delay == 3000, "Inherited earthquake delay changed");

        DoEarthQuake(audio, presentation);
        require(ShakeNow == 1 && earthquake_timer_set == 1, "Earthquake should set shake/timer state");
        require(audio.requests.size() == 1, "Earthquake should emit one typed sound request");
        const auto& first = audio.requests.front();
        require(first.kind == AudioRequestKind::Effect && first.sound == SoundId::ExplosionLow &&
            first.channel == AudioChannel::City,
            "Earthquake typed sound/channel changed");
        require(first.consoleBeforeEffect == "DoEarthQuake\n" && first.shakeBeforeEffect == 0 &&
            first.timerBeforeEffect == 0 && first.soundInitialized,
            "Sound must initialize and route before the presentation event and shake/timer mutation");
        require(presentation.requests.size() == 1 && presentation.requests.front().audioRequestsBeforeEvent == 1 &&
            presentation.requests.front().consoleBeforeEvent == expected &&
            presentation.requests.front().shakeBeforeEvent == 0 &&
            presentation.requests.front().timerBeforeEvent == 0 && presentation.requests.front().soundInitialized,
            "Presentation must follow initialized audio and precede shake/timer mutation");
        require(console.take() == expected, "Earthquake must not emit a string visual command");

        DoEarthQuake(audio, presentation);
        require(ShakeNow == 2 && earthquake_timer_set == 1, "Repeated earthquake should increment shake state");
        require(audio.requests.size() == 2, "Each earthquake should request its sound once");
        const auto& second = audio.requests.back();
        require(second.kind == AudioRequestKind::Effect && second.sound == SoundId::ExplosionLow &&
            second.channel == AudioChannel::City &&
            second.consoleBeforeEffect == "DoEarthQuake\n" && second.shakeBeforeEffect == 1 &&
            second.timerBeforeEffect == 1 && second.soundInitialized,
            "Repeated earthquake sound must precede its presentation event and state increment");
        require(presentation.requests.size() == 2 && presentation.requests.back().audioRequestsBeforeEvent == 2 &&
            presentation.requests.back().consoleBeforeEvent == expected &&
            presentation.requests.back().shakeBeforeEvent == 1 &&
            presentation.requests.back().timerBeforeEvent == 1 && presentation.requests.back().soundInitialized,
            "Repeated presentation must preserve audio/event/state ordering");
        require(console.take() == expected, "Repeated earthquake must not emit a string visual command");

        StopEarthquake();
        StopEarthquake();
        require(ShakeNow == 0 && earthquake_timer_set == 0, "Earthquake stop should clear both state values");
        require(console.take().empty(), "Earthquake stop should not emit legacy commands");
        require(presentation.requests.size() == 2, "Stopping must not emit a new earthquake-start event");

        DoEarthQuake(audio, presentation);
        require(ShakeNow == 1 && earthquake_timer_set == 1 && audio.requests.size() == 3,
            "Earthquake must restart from cleared shake/timer state");
        const auto& restarted = audio.requests.back();
        require(restarted.kind == AudioRequestKind::Effect && restarted.sound == SoundId::ExplosionLow &&
            restarted.channel == AudioChannel::City && restarted.shakeBeforeEffect == 0 &&
            restarted.timerBeforeEffect == 0 && restarted.soundInitialized &&
            restarted.consoleBeforeEffect == "DoEarthQuake\n",
            "Restart sound must precede the presentation event and reset state increment");
        require(presentation.requests.size() == 3 && presentation.requests.back().audioRequestsBeforeEvent == 3 &&
            presentation.requests.back().consoleBeforeEvent == expected &&
            presentation.requests.back().shakeBeforeEvent == 0 &&
            presentation.requests.back().timerBeforeEvent == 0 && presentation.requests.back().soundInitialized,
            "Restart presentation must observe the reset state after its audio request");
        require(console.take() == expected, "Restart must not emit a string visual command");
        StopEarthquake();

        NullAudioService nullAudio;
        NullPresentationEvents nullPresentation;
        userSoundOn(false);
        DoEarthQuake(nullAudio, nullPresentation);
        require(ShakeNow == 1 && earthquake_timer_set == 1 && userSoundOn(),
            "Null adapters must preserve earthquake state and saved initialization flag");
        require(console.take() == expected, "Null adapters must not reintroduce string commands");
        StopEarthquake();
    }
}

int main()
{
    try
    {
        checkEvalStub();
        checkSoundRouting();
        checkDefaultEffects();
        checkBulldozerLifecycle();
        checkEarthquakeLifecycle();
        std::cout << "Legacy effects and typed audio/presentation lifecycle tests passed\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
