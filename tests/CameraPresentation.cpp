// Civic 89 camera, timing and preferences acceptance. SPDX-License-Identifier: GPL-3.0-or-later
#include "Camera2D.h"
#include "DisplayLayout.h"
#include "DisplaySettings.h"
#include "FrameScheduler.h"
#include "QuakeEffect.h"
#include "WindowsFileStorage.h"
#include <array>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>
#include <limits>
#include <stdexcept>

namespace
{
    void require(bool value, const char* message) { if (!value) { throw std::runtime_error(message); } }
    bool near(float a, float b) { return std::abs(a - b) < .001f; }
    void cameraChecks()
    {
        Camera2D camera;
        camera.position({400, 300});
        const Point<float> cursor{321.5f, 245.25f};
        const auto world = camera.screenToWorld(cursor);
        camera.zoomAt(2.25f, cursor);
        require(near(camera.screenToWorld(cursor).x, world.x) && near(camera.screenToWorld(cursor).y, world.y), "Zoom anchor drift");
        const auto screen = camera.worldToScreen(world);
        require(near(screen.x, cursor.x) && near(screen.y, cursor.y), "Camera transform roundtrip");
        camera.pan({-30, -60});
        require(near(camera.worldToScreen(world).x, cursor.x + 30), "Screen-space pan must respect zoom");
        camera.position({-100, -100});
        require(camera.position() == Point<float>{0, 0}, "Negative camera bounds");
        camera.position({10000, 10000});
        require(near(camera.position().x + camera.visibleWorld().x, 1920), "Right camera bound");
        camera.viewport({3840, 2160}); camera.zoomAt(.25f, {});
        require(camera.position() == Point<float>{0, 0} && !camera.containsWorld({1920, 0}), "Large viewport/map edge");
        camera.viewport({800, 600}); camera.zoomAt(1, {}); camera.position({400.37f, 300.19f});
        for (float scale : {1.f, 1.25f, 1.5f, 2.f, 1.25f, 1.f})
        {
            camera.displayScale(scale); camera.pixelPerfect(true); camera.zoomAt(1.8f, {400, 300});
            require(near(camera.zoom() * scale, std::round(camera.zoom() * scale)), "Integer physical pixel zoom");
            const auto origin = camera.worldToScreen({0, 0});
            require(near(origin.x * scale, std::round(origin.x * scale)), "Pixel-perfect pan alignment");
            const auto point = camera.screenToWorld(camera.worldToScreen({510, 490}));
            require(near(point.x, 510) && near(point.y, 490), "Mixed-DPI picking roundtrip");
        }
        const auto previous = camera.zoom();
        camera.zoomAt(std::numeric_limits<float>::quiet_NaN(), {});
        require(camera.zoom() == previous, "Nonfinite zoom rejected");
    }
    void layoutChecks()
    {
        for (float scale : {1.f, 1.25f, 1.5f, 2.f, 1.25f})
        {
            const auto layout = DisplayLayout::fromPixels(3840, 2160, scale);
            require(near(layout.logical.x * layout.scale, 3840) && near(layout.logical.y * layout.scale, 2160), "DPI output extent");
            require(layout.scale == scale, "Expected monitor content scale");
        }
        for (auto size : {Vector<int>{800, 600}, {1366, 768}, {1920, 1080}, {3440, 1440}})
        {
            const auto layout = DisplayLayout::fromPixels(size.x, size.y, 2);
            require(layout.logical.x >= 800 && layout.logical.y >= 600, "Small display panel reachability");
        }
        require(DisplayLayout::fromPixels(1600, 1200, 0).scale == 1, "Invalid scale fallback");
    }
    void cadenceChecks()
    {
        for (unsigned interval : {100u, 50u, 25u, 5u})
        {
            std::vector<FrameScheduler::Event> reference;
            for (unsigned frameMs : {33u, 16u, 7u})
            {
                FrameScheduler scheduler; scheduler.reset(0);
                std::vector<FrameScheduler::Event> events;
                for (uint64_t time = 0; time < 6000; time += frameMs)
                {
                    const auto due = scheduler.advance(time, interval, false);
                    events.insert(events.end(), due.begin(), due.end());
                }
                const auto due = scheduler.advance(6000, interval, false);
                events.insert(events.end(), due.begin(), due.end());
                require(std::count(events.begin(), events.end(), FrameScheduler::Event::Simulation) == (6000 - 100) / interval + 1, "Render rate changed simulation count");
                require(std::count(events.begin(), events.end(), FrameScheduler::Event::Animation) == 40, "Animation cadence");
                if (reference.empty()) { reference = events; }
                else { require(reference == events, "Render rate changed RNG event ordering"); }
            }
        }
        FrameScheduler scheduler; scheduler.reset(0);
        const auto suspended = scheduler.advance(10000, 5, true);
        require(std::none_of(suspended.begin(), suspended.end(), [](auto event) { return event == FrameScheduler::Event::Simulation || event == FrameScheduler::Event::Animation; }), "Modal must suspend engine changes");
        require(scheduler.next() > 10000 && scheduler.advance(10000, 5, false).empty(), "No burst after suspension");
        require(scheduler.advance(100000, 5, false).size() <= 32, "Bounded stall catch-up");
        FramePacer pacer;
        int frames = 0;
        for (uint64_t time = 0; time < 1000000000; time += 100000) { frames += pacer.ready(time, 60); }
        require(frames == 60, "60 Hz frame limit must not round to 62.5 Hz");
        QuakeEffect quake; quake.start(100);
        require(quake.active(100) && quake.active(3099) && !quake.active(3100), "Earthquake deadline");
        require(quake.offset(3100) == Vector<float>{} && quake.offset(101).lengthSquared() <= 32, "Bounded earthquake offset");
        quake.start(2000); require(quake.active(4999), "Repeated quake refreshes deadline");
        quake.cancel(); require(!quake.active(2000), "Quake cancellation");
    }
    void settingsChecks()
    {
        const auto path = std::filesystem::temp_directory_path() / ("civic89-display-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()) + ".cfg");
        struct Cleanup { std::filesystem::path path; ~Cleanup() { std::error_code error; std::filesystem::remove(path, error); } } cleanup{path};
        WindowsFileStorage storage;
        require(!DisplaySettings::load(path), "Missing settings defaults");
        for (const auto mode : {WindowMode::Windowed, WindowMode::Maximized, WindowMode::Borderless})
        {
            DisplaySettings settings{mode, 1234, 789, false, true};
            require(static_cast<bool>(settings.save(path, storage)), "Atomic display settings save");
            const auto loaded = DisplaySettings::load(path);
            require(loaded && loaded->mode == mode && loaded->width == 1234 && loaded->height == 789 && !loaded->vsync && loaded->pixelPerfect, "Persisted settings roundtrip");
        }
        for (const auto* invalid : {"", "1 0 800", "2 0 800 600 1 0", "1 3 800 600 1 0", "1 0 -1 600 1 0", "1 0 800 600 2 0", "1 0 800 600 1 0 extra"})
        {
            { std::ofstream file(path); file << invalid; }
            require(!DisplaySettings::load(path), "Malformed display preferences rejected");
        }
    }
}
int main()
{
    try { cameraChecks(); layoutChecks(); cadenceChecks(); settingsChecks(); }
    catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
    std::cout << "Camera, DPI/mixed-monitor layout, independent cadence, quake and settings passed\n";
}
