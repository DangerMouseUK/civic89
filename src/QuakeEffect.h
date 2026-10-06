// Civic 89 presentation-only earthquake. SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "Math/Vector.h"
#include <cmath>
#include <cstdint>

class QuakeEffect
{
public:
    void start(uint64_t now) { mEnd = now + 3000; }
    void cancel() { mEnd = 0; }
    bool active(uint64_t now) const { return mEnd != 0 && now < mEnd; }
    Vector<float> offset(uint64_t now) const
    {
        if (!active(now)) { return {}; }
        const auto remaining = static_cast<float>(mEnd - now);
        const float strength = 4.f * remaining / 3000.f;
        return {std::round(std::sin(remaining * .073f) * strength),
            std::round(std::cos(remaining * .091f) * strength)};
    }
private:
    uint64_t mEnd{};
};
