// Civic 89 Windows file adapter. SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "CityIo.h"
class WindowsFileStorage final : public AtomicFileWriter
{
public:
    CityIoResult write(const std::filesystem::path&, std::span<const char>) override;
};
