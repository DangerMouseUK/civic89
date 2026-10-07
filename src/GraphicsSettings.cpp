// Civic 89 presentation preference. SPDX-License-Identifier: GPL-3.0-or-later
#include "GraphicsSettings.h"
#include <fstream>
#include <string>

std::optional<GraphicsSettings> GraphicsSettings::load(const std::filesystem::path& path)
{
    std::ifstream file(path);
    int schema{}, choice{};
    std::string extra;
    if (!(file >> schema >> choice) || schema != 1 || choice < 0 || choice > 1 || file >> extra) { return {}; }
    return GraphicsSettings{static_cast<GraphicsStyle>(choice)};
}
CityIoResult GraphicsSettings::save(const std::filesystem::path& path, AtomicFileWriter& writer) const
{
    if (style != GraphicsStyle::Classic && style != GraphicsStyle::Enhanced)
        { return {CityIoCode::InvalidFormat, path, "Invalid graphics preference"}; }
    const std::string bytes = style == GraphicsStyle::Classic ? "1 0\n" : "1 1\n";
    return writer.write(path, std::span<const char>(bytes.data(), bytes.size()));
}
