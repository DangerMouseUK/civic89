// Civic 89 save publication boundary. SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <filesystem>
#include <span>
#include <string>
#include <string_view>
inline std::filesystem::path pathFromUtf8(std::string_view value)
{
    std::u8string bytes;
    bytes.reserve(value.size());
    for (unsigned char character : value) { bytes.push_back(static_cast<char8_t>(character)); }
    return std::filesystem::path(bytes);
}
inline std::string pathUtf8(const std::filesystem::path& path)
{
    const auto bytes = path.u8string();
    return std::string(bytes.begin(), bytes.end());
}

enum class CityIoCode { Success, MissingFile, InvalidFormat, InvalidData, CreateFailed, WriteFailed, FlushFailed, ReplaceFailed };
struct CityIoResult
{
    CityIoCode code{CityIoCode::Success};
    std::filesystem::path path;
    std::string detail;
    explicit operator bool() const noexcept { return code == CityIoCode::Success; }
};
class AtomicFileWriter
{
public:
    virtual ~AtomicFileWriter() = default;
    virtual CityIoResult write(const std::filesystem::path&, std::span<const char>) = 0;
};
