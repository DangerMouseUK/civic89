// Civic 89 presentation-only data overlays. SPDX-License-Identifier: GPL-3.0-or-later
#include "OverlayModel.h"
#include "Map.h"
#include "s_alloc.h"

int overlayValue(DataOverlay type, Point<int> tile)
{
    if (tile.x < 0 || tile.x >= SimWidth || tile.y < 0 || tile.y >= SimHeight) { return 0; }
    const EffectMap* map{};
    int cell = 2;
    switch (type)
    {
    case DataOverlay::Traffic: map = &TrafficDensityMap; break;
    case DataOverlay::Crime: map = &CrimeMap; break;
    case DataOverlay::LandValue: map = &LandValueMap; break;
    case DataOverlay::Pollution: map = &PollutionMap; break;
    case DataOverlay::Population: map = &PopulationDensityMap; break;
    case DataOverlay::Fire: map = &FireProtectionMap; cell = 8; break;
    case DataOverlay::Police: map = &PoliceProtectionMap; cell = 8; break;
    case DataOverlay::Growth: map = &RateOfGrowthMap; cell = 8; break;
    default: break;
    }
    if (map) { return map->value({tile.x / cell, tile.y / cell}); }
    const auto raw = tileValue(tile.x, tile.y);
    const auto value = maskedTileValue(raw);
    switch (type)
    {
    case DataOverlay::Power: return value <= FireLast ? 0 : (raw & ZonedBit) ? ((raw & PowerBit) ? 2 : 1) : (raw & ConductiveBit) ? 3 : 0;
    case DataOverlay::Transport: return (value >= BridgeBase && value <= RoadLast) ||
        (value >= RailHorizontalPowerVertical && value <= RailVerticalPowerHorizontal) ||
        (value >= RailBase && value <= RoadVerticalPowerHorizontal) ? 1 : 0;
    case DataOverlay::Residential: return value >= ResidentialBase && value < CommercialBase ? 1 : 0;
    case DataOverlay::Commercial: return value >= CommercialBase && value < IndustryBase ? 1 : 0;
    case DataOverlay::Industrial: return value >= IndustryBase && value < PortBase ? 1 : 0;
    default: return 0;
    }
}
OverlayColor overlayColor(DataOverlay type, int value, bool accessible)
{
    constexpr std::array<OverlayColor, 5> heat{{{0,0,0,0}, {180,190,200,255}, {245,210,85,255}, {244,145,55,255}, {230,72,65,255}}};
    constexpr std::array<OverlayColor, 5> blue{{{0,0,0,0}, {180,200,220,255}, {105,180,220,255}, {45,125,185,255}, {15,65,130,255}}};
    const auto& colors = accessible ? blue : heat;
    if (type == DataOverlay::None || !value) { return {}; }
    if (type == DataOverlay::Growth)
    {
        if (value > 100) { return {105,205,170,255}; }
        if (value > 20) { return {35,140,120,255}; }
        if (value < -100) { return {245,195,75,255}; }
        if (value < -20) { return {225,115,45,255}; }
        return {};
    }
    if (type == DataOverlay::Power)
    {
        if (value == 1) { return {105,185,240,255}; }
        if (value == 2) { return {245,165,55,255}; }
        return {195,205,215,255};
    }
    if (type >= DataOverlay::Transport) { return colors[3]; }
    return colors[value < 50 ? 0 : value < 100 ? 1 : value < 150 ? 2 : value < 200 ? 3 : 4];
}
std::string_view overlayLegend(DataOverlay type)
{
    if (type == DataOverlay::Power) { return "Blue: unpowered | amber: powered | gray: wire"; }
    if (type == DataOverlay::Growth) { return "Decline < -20 | stable | growth > 20 (strong: 100)"; }
    if (type >= DataOverlay::Transport) { return "Highlighted: matching tiles"; }
    if (type == DataOverlay::None) { return "Choose city data to inspect"; }
    return "<50: clear | 50 / 100 / 150 / 200: increasing";
}
