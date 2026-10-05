// Civic 89 legacy characterization tests. SPDX-License-Identifier: GPL-3.0-or-later
#include "w_sound.h"
#include "w_tk.h"

#include <iostream>
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

    struct RecordedEffect
    {
        SoundId sound;
        AudioChannel channel;
        std::string consoleBeforeEffect;
        int shakeBeforeEffect;
        int timerBeforeEffect;
        bool soundInitialized;
    };

    class RecordingAudioService final : public AudioService
    {
    public:
        explicit RecordingAudioService(const ConsoleCapture& console) : capture(console) {}

        void playEffect(SoundId sound, AudioChannel channel) override
        {
            effects.push_back({ sound, channel, capture.contents(), ShakeNow, earthquake_timer_set, userSoundOn() });
        }

        std::vector<RecordedEffect> effects;

    private:
        const ConsoleCapture& capture;
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

    void checkBulldozerLifecycle()
    {
        resetBridge();
        ConsoleCapture console;
        StopBulldozer();
        require(console.take().empty(), "Stopping uninitialized sound should do nothing");

        StartBulldozer();
        StartBulldozer();
        require(userSoundOn() && Dozing == 1, "Bulldozer start should set initialization and loop state");
        require(console.take() == "Eval: UIStartSound edit 1\n", "Repeated start should emit one loop request");

        StopBulldozer();
        StopBulldozer();
        require(Dozing == 0, "Bulldozer stop should clear loop state");
        require(console.take() == "Eval: UIStopSound 1\nEval: UIStopSound 1\n",
            "Inherited repeated stop behavior changed");

        StartBulldozer();
        SoundOff();
        require(Dozing == 0 && userSoundOn(), "SoundOff clears the loop but leaves initialization set");
        require(console.take() == "Eval: UIStartSound edit 1\nEval: UISoundOff\n",
            "SoundOff routing changed");
        MakeSound("city", "Siren");
        require(console.take() == "Eval: UIMakeSound \"city\" \"Siren\"\n",
            "Inherited SoundOff does not prevent later effect requests");
    }

    void checkEarthquakeLifecycle()
    {
        resetBridge();
        ConsoleCapture console;
        RecordingAudioService audio(console);
        const std::string expected = "DoEarthQuake\nEval: UIEarthQuake\n";
        require(ShakeNow == 0 && earthquake_timer_set == 0, "Earthquake should start reset");
        require(earthquake_delay == 3000, "Inherited earthquake delay changed");

        DoEarthQuake(audio);
        require(ShakeNow == 1 && earthquake_timer_set == 1, "Earthquake should set shake/timer state");
        require(audio.effects.size() == 1, "Earthquake should emit one typed sound request");
        const auto& first = audio.effects.front();
        require(first.sound == SoundId::ExplosionLow && first.channel == AudioChannel::City,
            "Earthquake typed sound/channel changed");
        require(first.consoleBeforeEffect == "DoEarthQuake\n" && first.shakeBeforeEffect == 0 &&
            first.timerBeforeEffect == 0 && first.soundInitialized,
            "Sound must initialize and route before the visual command and shake/timer mutation");
        require(console.take() == expected, "Earthquake should retain only its legacy visual command");

        DoEarthQuake(audio);
        require(ShakeNow == 2 && earthquake_timer_set == 1, "Repeated earthquake should increment shake state");
        require(audio.effects.size() == 2, "Each earthquake should request its sound once");
        const auto& second = audio.effects.back();
        require(second.sound == SoundId::ExplosionLow && second.channel == AudioChannel::City &&
            second.consoleBeforeEffect == "DoEarthQuake\n" && second.shakeBeforeEffect == 1 &&
            second.timerBeforeEffect == 1 && second.soundInitialized,
            "Repeated earthquake sound must precede its visual command and state increment");
        require(console.take() == expected, "Repeated earthquake visual routing changed");

        StopEarthquake();
        StopEarthquake();
        require(ShakeNow == 0 && earthquake_timer_set == 0, "Earthquake stop should clear both state values");
        require(console.take().empty(), "Earthquake stop should not emit legacy commands");

        NullAudioService nullAudio;
        userSoundOn(false);
        DoEarthQuake(nullAudio);
        require(ShakeNow == 1 && earthquake_timer_set == 1 && userSoundOn(),
            "Null audio must preserve earthquake state and saved initialization flag");
        require(console.take() == expected, "Null audio must not reintroduce a string sound command");
        StopEarthquake();
    }
}

int main()
{
    try
    {
        checkEvalStub();
        checkSoundRouting();
        checkBulldozerLifecycle();
        checkEarthquakeLifecycle();
        std::cout << "Legacy audio and typed earthquake routing/lifecycle tests passed\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
