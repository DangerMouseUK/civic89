// Civic 89 Enhanced container boundary. SPDX-License-Identifier: GPL-3.0-or-later
#include "CityDocument.h"
#include "ScenarioData.h"
#include <algorithm>
#include <fstream>

namespace
{
    constexpr std::array<char,8> magic{'C','I','V','I','C','8','9','\0'};
    constexpr size_t headerSize = 44;
    constexpr size_t maximumName = 255;
    constexpr size_t maximumSize = headerSize + maximumName + static_cast<size_t>(ScenarioFileSize);
    std::uint32_t word(std::span<const char> bytes, size_t offset)
    {
        std::uint32_t result = 0;
        for (unsigned i=0;i<4;++i)
            { result |= static_cast<std::uint32_t>(static_cast<unsigned char>(bytes[offset+i])) << (i*8); }
        return result;
    }
    void appendWord(std::vector<char>& bytes, std::uint32_t value)
    {
        for (unsigned shift=0;shift<32;shift+=8) { bytes.push_back(static_cast<char>((value >> shift) & 255)); }
    }
    std::uint32_t checksum(std::span<const char> bytes)
    {
        std::uint32_t crc = 0xffffffffU;
        for (size_t i=0;i<bytes.size();++i)
        {
            if (i>=40 && i<headerSize) { continue; }
            crc ^= static_cast<unsigned char>(bytes[i]);
            for (unsigned bit=0;bit<8;++bit) { crc = (crc >> 1) ^ (0xedb88320U & (0U - (crc & 1))); }
        }
        return ~crc;
    }
    bool validName(std::string_view name)
    {
        if (name.empty() || name.size()>maximumName) { return false; }
        for (size_t i=0;i<name.size();)
        {
            const auto first=static_cast<unsigned char>(name[i++]);
            if (first<0x80) { if (first<32 || first==127) { return false; } continue; }
            unsigned count=0; std::uint32_t code=0, minimum=0;
            if (first>=0xc2 && first<=0xdf) { count=1; code=first & 0x1f; minimum=0x80; }
            else if (first>=0xe0 && first<=0xef) { count=2; code=first & 0x0f; minimum=0x800; }
            else if (first>=0xf0 && first<=0xf4) { count=3; code=first & 7; minimum=0x10000; }
            else { return false; }
            if (count>name.size()-i) { return false; }
            for (unsigned n=0;n<count;++n)
            {
                const auto next=static_cast<unsigned char>(name[i++]);
                if ((next & 0xc0)!=0x80) { return false; }
                code=(code << 6) | (next & 0x3f);
            }
            if (code<minimum || code>0x10ffff || (code>=0xd800 && code<=0xdfff)) { return false; }
        }
        return true;
    }
}

bool validEnhancedCityName(std::string_view name) { return validName(name); }

CityIoResult encodeCityDocument(const CityDocument& document, std::vector<char>& output)
{
    const auto* rules=findRuleset(document.ruleset);
    if (!rules || rules->id!=RulesetId::EnhancedV1)
        { return {CityIoCode::UnsupportedRuleset,{},"Unsupported Enhanced ruleset."}; }
    if (!validName(document.name) || document.classicPayload.size()!=static_cast<size_t>(ScenarioFileSize))
        { return {CityIoCode::InvalidData,{},"Enhanced city needs a valid UTF-8 name (1-255 bytes) and Classic v1 payload."}; }
    std::vector<char> bytes(magic.begin(),magic.end());
    bytes.reserve(headerSize+document.name.size()+document.classicPayload.size());
    for (const std::uint32_t value : {1U,rules->family,rules->version,
        static_cast<std::uint32_t>(rules->mapWidth),static_cast<std::uint32_t>(rules->mapHeight),
        static_cast<std::uint32_t>(document.name.size()),static_cast<std::uint32_t>(document.classicPayload.size()),0U,0U})
        { appendWord(bytes,value); }
    bytes.insert(bytes.end(),document.name.begin(),document.name.end());
    bytes.insert(bytes.end(),document.classicPayload.begin(),document.classicPayload.end());
    const auto crc=checksum(bytes);
    for (unsigned i=0;i<4;++i) { bytes[40+i]=static_cast<char>((crc >> (i*8)) & 255); }
    output=std::move(bytes);
    return {};
}

CityIoResult readCityDocument(const std::filesystem::path& path, CityDocument& output)
{
    const auto invalid=[&](const char* detail) { return CityIoResult{CityIoCode::InvalidFormat,path,detail}; };
    std::ifstream stream(path,std::ios::binary | std::ios::ate);
    if (!stream)
    {
        std::error_code error;
        return {std::filesystem::exists(path,error) || error ? CityIoCode::InvalidFormat : CityIoCode::MissingFile,
            path,"Enhanced city is missing or unreadable."};
    }
    const auto length=stream.tellg();
    if (length<static_cast<std::streamoff>(headerSize) || length>static_cast<std::streamoff>(maximumSize))
        { return invalid("Enhanced container has an unsupported length."); }
    std::vector<char> bytes(static_cast<size_t>(length));
    stream.seekg(0); stream.read(bytes.data(),static_cast<std::streamsize>(bytes.size()));
    if (!stream || stream.peek()!=std::char_traits<char>::eof()) { return invalid("Incomplete Enhanced container."); }
    if (!std::equal(magic.begin(),magic.end(),bytes.begin()) || word(bytes,8)!=1 || word(bytes,36)!=0)
        { return invalid("Unsupported Enhanced container schema or flags."); }
    const auto* rules=findRuleset(word(bytes,12),word(bytes,16));
    if (!rules || rules->id!=RulesetId::EnhancedV1)
        { return {CityIoCode::UnsupportedRuleset,path,"This Enhanced ruleset version is not supported."}; }
    const auto nameSize=word(bytes,28), payloadSize=word(bytes,32);
    if (word(bytes,20)!=static_cast<std::uint32_t>(rules->mapWidth) || word(bytes,24)!=static_cast<std::uint32_t>(rules->mapHeight) ||
        nameSize==0 || nameSize>maximumName || payloadSize!=static_cast<size_t>(ScenarioFileSize) ||
        headerSize+nameSize+static_cast<size_t>(payloadSize)!=bytes.size())
        { return invalid("Unsupported Enhanced dimensions or payload/name length."); }
    if (word(bytes,40)!=checksum(bytes)) { return invalid("Enhanced container checksum mismatch."); }
    CityDocument candidate;
    candidate.ruleset=rules->id;
    candidate.name.assign(bytes.data()+headerSize,nameSize);
    if (!validName(candidate.name)) { return invalid("Invalid Enhanced UTF-8 city name."); }
    candidate.classicPayload.assign(bytes.begin()+headerSize+nameSize,bytes.end());
    output=std::move(candidate);
    return {CityIoCode::Success,path,{}};
}
