// Civic 89 Windows file adapter. SPDX-License-Identifier: GPL-3.0-or-later
#include "WindowsFileStorage.h"
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <algorithm>
#include <atomic>
#include <limits>

CityIoResult WindowsFileStorage::write(const std::filesystem::path& path, std::span<const char> bytes)
{
    static std::atomic<unsigned long> sequence{};
    auto temporary = path;
    temporary += L".tmp-" + std::to_wstring(GetCurrentProcessId()) + L"-" + std::to_wstring(sequence++);
    const auto file = CreateFileW(temporary.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_NEW,
        FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE)
        { return {CityIoCode::CreateFailed, path, "Cannot create temporary save (Windows error " + std::to_string(GetLastError()) + ")"}; }
    struct Cleanup
    {
        HANDLE handle;
        const std::filesystem::path& temporary;
        ~Cleanup() { if (handle != INVALID_HANDLE_VALUE) { CloseHandle(handle); } DeleteFileW(temporary.c_str()); }
    } cleanup{file, temporary};
    size_t offset = 0;
    while (offset < bytes.size())
    {
        const auto count = static_cast<DWORD>(std::min(bytes.size() - offset, static_cast<size_t>(std::numeric_limits<DWORD>::max())));
        DWORD written = 0;
        if (!WriteFile(file, bytes.data() + offset, count, &written, nullptr) || written == 0)
            { return {CityIoCode::WriteFailed, path, "Cannot write complete save (Windows error " + std::to_string(GetLastError()) + ")"}; }
        offset += written;
    }
    if (!FlushFileBuffers(file))
        { return {CityIoCode::FlushFailed, path, "Cannot flush save (Windows error " + std::to_string(GetLastError()) + ")"}; }
    if (!CloseHandle(file))
        { return {CityIoCode::FlushFailed, path, "Cannot close save (Windows error " + std::to_string(GetLastError()) + ")"}; }
    cleanup.handle = INVALID_HANDLE_VALUE;
    // Same-directory publication; the existing city remains intact if this fails.
    if (!MoveFileExW(temporary.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
        { return {CityIoCode::ReplaceFailed, path, "Cannot replace save (Windows error " + std::to_string(GetLastError()) + ")"}; }
    return {CityIoCode::Success, path, {}};
}
