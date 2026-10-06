// Civic 89 logical layout. SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "Math/Vector.h"
#include <algorithm>
#include <cmath>

struct DisplayLayout
{
    float scale{1};
    Vector<float> logical{800, 600};
    static DisplayLayout fromPixels(int width, int height, float scale)
    {
        if (!std::isfinite(scale) || scale <= 0) { scale = 1; }
        // Keep the inherited panels reachable on small high-DPI displays until M5 layout.
        scale = std::min({scale, std::max(1, width) / 800.f, std::max(1, height) / 600.f});
        return {scale, {std::max(1, width) / scale, std::max(1, height) / scale}};
    }
};
