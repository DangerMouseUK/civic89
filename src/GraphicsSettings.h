// Civic 89 presentation preference. SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "FileIo.h"
#include <filesystem>
#include <optional>

enum class GraphicsStyle { Classic, Enhanced };
struct GraphicsSettings
{
    GraphicsStyle style{GraphicsStyle::Classic};
    static std::optional<GraphicsSettings> load(const std::filesystem::path&);
    CityIoResult save(const std::filesystem::path&, AtomicFileWriter&) const;
};
