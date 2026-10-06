// Civic 89 main-thread deadlines. SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <algorithm>
#include <array>
#include <cstdint>
#include <vector>

class FrameScheduler
{
public:
    enum class Event { Simulation, Animation, Blink, Minimap };
    void reset(uint64_t now) { mNext = {now + 100, now + 150, now + 500, now + 1000}; }
    std::vector<Event> advance(uint64_t now, unsigned simulationInterval, bool suspended)
    {
        const std::array<uint64_t, 4> intervals{simulationInterval, 150, 500, 1000};
        std::vector<Event> result;
        // A bounded chronological catch-up preserves simulation/animation RNG ordering.
        for (int count = 0; count < 32; ++count)
        {
            const auto first = std::min_element(mNext.begin(), mNext.end());
            if (*first > now) { break; }
            const auto index = static_cast<size_t>(first - mNext.begin());
            *first += intervals[index];
            if (!suspended || index >= 2) { result.push_back(static_cast<Event>(index)); }
        }
        // Native dialogs / suspend must not trigger an unbounded burst on return.
        for (size_t i = 0; i < mNext.size(); ++i)
            { if (mNext[i] <= now) { mNext[i] = now + intervals[i]; } }
        return result;
    }
    uint64_t next() const { return *std::min_element(mNext.begin(), mNext.end()); }
private:
    std::array<uint64_t, 4> mNext{};
};

class FramePacer
{
public:
    bool ready(uint64_t now, float refresh)
    {
        if (now < mNext) { return false; }
        const auto period = static_cast<uint64_t>(1000000000.0 / std::max(1.f, refresh));
        mNext += period;
        if (mNext <= now) { mNext = now + period; }
        return true;
    }
    uint64_t next() const { return mNext; }
private:
    uint64_t mNext{};
};
