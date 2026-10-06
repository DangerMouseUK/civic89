// Civic 89 versioned simulation contracts. SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <array>
#include <cstdint>
#include <optional>
#include <string_view>

enum class RulesetId { ClassicV1, EnhancedV1 };
struct RulesetDefinition
{
    RulesetId id;
    std::uint32_t family;
    std::uint32_t version;
    std::string_view name;
    std::string_view label;
    std::string_view extension;
    int mapWidth;
    int mapHeight;
    bool importsClassic;
    bool exportsClassic;
};
// Enhanced v1 intentionally uses the Classic simulation. New mechanics/dimensions
// must get an explicit new definition, data version and compatibility decision.
inline constexpr std::array Rulesets{
    RulesetDefinition{RulesetId::ClassicV1,1,1,"classic/1","Classic v1",".cty",120,100,true,true},
    RulesetDefinition{RulesetId::EnhancedV1,2,1,"enhanced/1","Enhanced v1",".c89",120,100,true,true}
};
inline constexpr const RulesetDefinition* findRuleset(RulesetId id)
{
    for (const auto& rules : Rulesets) { if (rules.id == id) { return &rules; } }
    return nullptr;
}
inline constexpr const RulesetDefinition* findRuleset(std::uint32_t family, std::uint32_t version)
{
    for (const auto& rules : Rulesets)
        { if (rules.family == family && rules.version == version) { return &rules; } }
    return nullptr;
}
inline std::optional<RulesetId> rulesetFromMode(std::string_view name)
{
    if (name == "classic") { return RulesetId::ClassicV1; }
    if (name == "enhanced") { return RulesetId::EnhancedV1; }
    return std::nullopt;
}
