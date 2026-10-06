// Civic 89 display preferences. SPDX-License-Identifier: GPL-3.0-or-later
#include "DisplaySettings.h"
#include <fstream>
#include <sstream>

std::optional<DisplaySettings> DisplaySettings::load(const std::filesystem::path& path)
{
    std::ifstream file(path);
    int version, mode, width, height, vsync, pixel;
    std::string extra;
    if (!(file >> version >> mode >> width >> height >> vsync >> pixel) || (file >> extra) ||
        version != 1 || mode < 0 || mode > 2 || width < 800 || width > 7680 || height < 600 || height > 4320 ||
        (vsync != 0 && vsync != 1) || (pixel != 0 && pixel != 1)) { return std::nullopt; }
    return DisplaySettings{static_cast<WindowMode>(mode), width, height, vsync != 0, pixel != 0};
}
CityIoResult DisplaySettings::save(const std::filesystem::path& path, AtomicFileWriter& writer) const
{
    std::ostringstream output;
    output << "1 " << static_cast<int>(mode) << ' ' << width << ' ' << height << ' ' << vsync << ' ' << pixelPerfect << '\n';
    const auto bytes = output.str();
    return writer.write(path, std::span<const char>(bytes.data(), bytes.size()));
}
