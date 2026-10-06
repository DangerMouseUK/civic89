// Civic 89 structured application diagnostics. SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <filesystem>
#include <fstream>
#include <string_view>
enum class Severity { Info, Warning, Error };
enum class DiagnosticCode { Startup, AudioUnavailable, AudioPlayback, CityLoad, CitySave, Autosave, FileDialog, Shutdown, Settings };
class DiagnosticLog
{
public:
    explicit DiagnosticLog(const std::filesystem::path& path = {});
    void write(Severity, DiagnosticCode, std::string_view detail, const std::filesystem::path& path = {});
private:
    std::ofstream output;
};
