// Civic 89 overlay/preferences model acceptance. SPDX-License-Identifier: GPL-3.0-or-later
#include "OverlayModel.h"
#include "UiSettings.h"
#include "WindowsFileStorage.h"
#include "Map.h"
#include "s_alloc.h"
#include <chrono>
#include <fstream>
#include <iostream>
#include <stdexcept>

static void require(bool value,const char* message) { if(!value) throw std::runtime_error(message); }
int main()
{
    try
    {
        initMapArrays(); ResetMap();
        TrafficDensityMap.value({59,49})=201;
        FireProtectionMap.value({14,12})=155;
        RateOfGrowthMap.value({14,12})=-110;
        require(overlayValue(DataOverlay::Traffic,{119,99})==201,"2x2 overlay edge addressing");
        require(overlayValue(DataOverlay::Fire,{119,99})==155,"8x8 overlay partial last row");
        require(overlayValue(DataOverlay::Growth,{119,99})==-110,"Signed growth edge addressing");
        require(overlayValue(DataOverlay::Traffic,{-1,0})==0 && overlayValue(DataOverlay::Fire,{120,100})==0,"Overlay bounds");
        for(auto type : {DataOverlay::Traffic,DataOverlay::Crime,DataOverlay::LandValue,DataOverlay::Pollution,DataOverlay::Population,DataOverlay::Fire,DataOverlay::Police})
        {
            require(overlayColor(type,49,false).a==0 && overlayColor(type,50,false).a==255,"Legacy threshold 50");
            require(overlayColor(type,99,false).r!=overlayColor(type,100,false).r,"Legacy threshold 100");
            require(overlayColor(type,149,false).r!=overlayColor(type,150,false).r,"Legacy threshold 150");
            require(overlayColor(type,199,false).r!=overlayColor(type,200,false).r,"Legacy threshold 200");
            require(overlayColor(type,200,true).b>overlayColor(type,200,true).r,"Accessible blue palette");
        }
        require(overlayColor(DataOverlay::Growth,20,false).a==0 && overlayColor(DataOverlay::Growth,-20,false).a==0,"Stable growth range");
        require(overlayColor(DataOverlay::Growth,21,false).a && overlayColor(DataOverlay::Growth,-21,false).a,"Signed growth thresholds");
        tileValue(10,10)=ResidentialEmpty|ZonedBit;
        require(overlayValue(DataOverlay::Power,{10,10})==1,"Unpowered zone");
        tileValue(10,10)|=PowerBit;
        require(overlayValue(DataOverlay::Power,{10,10})==2,"Powered zone");
        require(overlayValue(DataOverlay::Residential,{10,10})==1 && !overlayValue(DataOverlay::Commercial,{10,10}),"Zone overlay classification");
        tileValue(10,10)=RailHorizontalPowerVertical;
        require(overlayValue(DataOverlay::Transport,{10,10})==1,"Rail/power crossing missing from transport overlay");
        const auto path=std::filesystem::temp_directory_path()/("civic89-ui-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count())+".cfg");
        struct Cleanup { std::filesystem::path path; ~Cleanup() {std::error_code ec;std::filesystem::remove(path,ec);} } cleanup{path};
        WindowsFileStorage storage;
        UiSettings settings;
        for(size_t i=0;i<settings.keys.size();++i) require(settings.bind(i,static_cast<uint32_t>(100+i)),"Initial bindings");
        require(!settings.bind(2,settings.keys[0]) && !settings.bind(30,900) && !settings.bind(2,27),"Conflicting/reserved binding rejection");
        settings.scale=1.5f;settings.largeText=true;settings.highContrast=true;settings.accessibleColors=true;settings.reverseZoom=true;settings.panSpeed=1000;settings.overlayOpacity=1;
        require(static_cast<bool>(settings.save(path,storage)),"Atomic UI settings publication");
        const auto loaded=UiSettings::load(path);
        require(loaded && loaded->keys==settings.keys && loaded->scale==1.5f && loaded->largeText && loaded->highContrast && loaded->accessibleColors && loaded->reverseZoom && loaded->panSpeed==1000 && loaded->overlayOpacity==1,"UI settings round trip");
        for(const auto prefix : {"2 1 0 0 0 0 500 .6", "1 .5 0 0 0 0 500 .6", "1 1 2 0 0 0 500 .6", "1 1 0 0 0 0 1001 .6", "1 1 0 0 0 0 500 1.1", "1 nan 0 0 0 0 500 .6"})
        {
            std::ofstream out(path);out<<prefix;for(auto key : settings.keys) out<<' '<<key;out.close();
            require(!UiSettings::load(path),"Invalid UI preferences accepted");
        }
        {std::ofstream out(path);out<<"1 1 0 0 0 0 500 .6";for(size_t i=0;i<30;++i) out<<" 100";}
        require(!UiSettings::load(path),"Duplicate disk bindings accepted");
        std::cout<<"Shared overlay sources, thresholds, colors, edges and validated atomic UI settings passed\n";
    }
    catch(const std::exception& error) {std::cerr<<error.what()<<'\n';return 1;}
}
