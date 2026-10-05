// Civic 89 legacy characterization tests. SPDX-License-Identifier: GPL-3.0-or-later
#include "w_sound.h"
#include "w_tk.h"

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

// Only the presentation bridge is linked. Supply its simulation-owned state;
// map damage, RNG, renderer behavior and scenario loading are outside this test.
int ShakeNow = 0;
extern int earthquake_timer_set;
extern int earthquake_delay;
extern int Dozing;
void DoEarthQuake();

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

    private:
        std::ostringstream output;
        std::streambuf* previous;
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
        const std::string expected =
            "DoEarthQuake\nEval: UIMakeSound \"city\" \"Explosion-Low\"\nEval: UIEarthQuake\n";
        require(ShakeNow == 0 && earthquake_timer_set == 0, "Earthquake should start reset");
        require(earthquake_delay == 3000, "Inherited earthquake delay changed");

        DoEarthQuake();
        require(ShakeNow == 1 && earthquake_timer_set == 1, "Earthquake should set shake/timer state");
        require(console.take() == expected, "Earthquake sound/presentation request order changed");
        DoEarthQuake();
        require(ShakeNow == 2 && earthquake_timer_set == 1, "Repeated earthquake should increment shake state");
        require(console.take() == expected, "Repeated earthquake routing changed");

        StopEarthquake();
        StopEarthquake();
        require(ShakeNow == 0 && earthquake_timer_set == 0, "Earthquake stop should clear both state values");
        require(console.take().empty(), "Earthquake stop should not emit legacy commands");
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
        std::cout << "Legacy audio routing and earthquake lifecycle characterization passed\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
