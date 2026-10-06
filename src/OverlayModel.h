// Civic 89 presentation-only data overlays. SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "Math/Point.h"
#include <array>
#include <cstdint>
#include <string_view>

enum class DataOverlay { None, Traffic, Crime, LandValue, Pollution, Population, Power, Fire, Police, Growth, Transport, Residential, Commercial, Industrial, Count };
struct OverlayColor { uint8_t r{}, g{}, b{}, a{}; };
inline constexpr std::array<std::string_view, static_cast<size_t>(DataOverlay::Count)> OverlayNames{
    "No overlay", "Traffic", "Crime", "Land value", "Pollution", "Population", "Power", "Fire coverage", "Police coverage", "Growth", "Transport", "Residential", "Commercial", "Industrial"};
int overlayValue(DataOverlay, Point<int> tile);
OverlayColor overlayColor(DataOverlay, int value, bool accessible);
std::string_view overlayLegend(DataOverlay);
