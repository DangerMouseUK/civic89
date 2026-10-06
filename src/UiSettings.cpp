// Civic 89 interface preferences. SPDX-License-Identifier: GPL-3.0-or-later
#include "UiSettings.h"
#include <cmath>
#include <fstream>
#include <sstream>

bool UiSettings::bind(size_t action, uint32_t key)
{
    if (action >= keys.size() || !key || key == 27 || key == 9 || key == 13) { return false; }
    for (size_t i = 0; i < keys.size(); ++i)
        { if (i != action && keys[i] == key) { return false; } }
    keys[action] = key;
    return true;
}
std::optional<UiSettings> UiSettings::load(const std::filesystem::path& path)
{
    std::ifstream file(path);
    UiSettings value;
    int version, large, contrast, colors, reverse;
    if (!(file >> version >> value.scale >> large >> contrast >> colors >> reverse >> value.panSpeed >> value.overlayOpacity) ||
        version != 1 || !std::isfinite(value.scale) || value.scale < 1 || value.scale > 1.5f ||
        !std::isfinite(value.panSpeed) || value.panSpeed < 100 || value.panSpeed > 1000 ||
        !std::isfinite(value.overlayOpacity) || value.overlayOpacity < .1f || value.overlayOpacity > 1 ||
        large < 0 || large > 1 || contrast < 0 || contrast > 1 || colors < 0 || colors > 1 || reverse < 0 || reverse > 1)
        { return {}; }
    for (size_t i = 0; i < value.keys.size(); ++i)
    {
        uint32_t key{};
        if (!(file >> key) || !value.bind(i, key)) { return {}; }
    }
    std::string extra;
    if (file >> extra) { return {}; }
    value.largeText = large != 0; value.highContrast = contrast != 0;
    value.accessibleColors = colors != 0; value.reverseZoom = reverse != 0;
    return value;
}
CityIoResult UiSettings::save(const std::filesystem::path& path, AtomicFileWriter& writer) const
{
    std::ostringstream out;
    out << "1 " << scale << ' ' << largeText << ' ' << highContrast << ' ' << accessibleColors << ' '
        << reverseZoom << ' ' << panSpeed << ' ' << overlayOpacity;
    for (const auto key : keys) { out << ' ' << key; }
    out << '\n';
    const auto bytes = out.str();
    return writer.write(path, std::span<const char>(bytes.data(), bytes.size()));
}
