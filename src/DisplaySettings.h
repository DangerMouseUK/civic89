// Civic 89 display preferences, separate from classic saves. SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "CityIo.h"
#include <optional>

enum class WindowMode { Windowed, Maximized, Borderless };
struct DisplaySettings
{
    WindowMode mode{WindowMode::Windowed};
    int width{800}, height{600}; // Normal window size in logical units.
    bool vsync{true}, pixelPerfect{false};
    static std::optional<DisplaySettings> load(const std::filesystem::path& path);
    CityIoResult save(const std::filesystem::path& path, AtomicFileWriter& writer) const;
};
