// Civic 89 structured application diagnostics. SPDX-License-Identifier: GPL-3.0-or-later
#include "DiagnosticLog.h"
#include "CityIo.h"
#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>

DiagnosticLog::DiagnosticLog(const std::filesystem::path& path)
{
    if (!path.empty()) { output.open(path, std::ios::app); }
    if (!path.empty() && !output) { std::cerr << "Civic 89: diagnostic file unavailable: " << path << '\n'; }
}
void DiagnosticLog::write(Severity severity, DiagnosticCode code, std::string_view detail, const std::filesystem::path& path)
{
    std::string message(detail);
    for (char& value : message) { if (value == '\n' || value == '\r') { value = ' '; } }
    std::ostringstream record;
    record << "time=" << std::chrono::system_clock::now().time_since_epoch().count()
        << " severity=" << static_cast<int>(severity) << " code=" << static_cast<int>(code)
        << " message=" << std::quoted(message) << " path=" << std::quoted(pathUtf8(path)) << '\n';
    std::cerr << record.str();
    if (output) { output << record.str(); output.flush(); }
}
