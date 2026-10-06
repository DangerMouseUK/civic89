// Civic 89 interface preferences. SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "FileIo.h"
#include <array>
#include <cstdint>
#include <filesystem>
#include <optional>

// Values are SDL keycodes on disk; this model has no SDL dependency.
inline constexpr size_t UiActionCount = 30; // 16 tools, 10 commands, 4 pan directions.
struct UiSettings
{
    float scale{1};
    bool largeText{false};
    bool highContrast{false};
    bool accessibleColors{false};
    bool reverseZoom{false};
    float panSpeed{500};
    float overlayOpacity{.6f};
    std::array<uint32_t, UiActionCount> keys{};
    static std::optional<UiSettings> load(const std::filesystem::path&);
    CityIoResult save(const std::filesystem::path&, AtomicFileWriter&) const;
    bool bind(size_t action, uint32_t key);
};
