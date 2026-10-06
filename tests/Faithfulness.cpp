// Civic 89 M8 inherited-mechanics acceptance. SPDX-License-Identifier: GPL-3.0-or-later
#include "Budget.h"
#include "CityProperties.h"
#include "Connection.h"
#include "EngineDigest.h"
#include "EngineState.h"
#include "FileIo.h"
#include "Map.h"
#include "Power.h"
#include "RCI.h"
#include "ScenarioData.h"
#include "ToolActions.h"
#include "ToolManager.h"
#include "Traffic.h"
#include "s_alloc.h"
#include "s_sim.h"
#include <array>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <vector>

void powerScan(); // Inherited simulation entry point.

namespace
{
    void require(bool value, const char* message) { if (!value) { throw std::runtime_error(message); } }
    constexpr std::array costs{100,100,100,500,0,500,5,1,20,10,5000,10,3000,3000,5000,10000};
    constexpr std::array sizes{3,3,3,3,1,3,1,1,1,1,4,1,4,4,4,6};

    void construction()
    {
        // These definitions are the fixed upstream res/tools.json contract. The
        // application smoke test independently checks the actual JSON loader.
        std::vector<Tool> definitions{Tool{}};
        for (size_t i=0;i<costs.size();++i)
        {
            definitions.push_back({static_cast<Tool::Type>(i),costs[i],sizes[i],sizes[i]>1 ? 1 : 0,
                i==6 || i==8 || i==9,"Audited tool"});
        }
        ToolManager tools(std::move(definitions));
        for (size_t i=0;i<costs.size();++i)
        {
            Budget budget; CityProperties city;
            initializeEngine(city,budget); ResetMap(); seedSimulationRandom(89);
            budget.CurrentFunds(20000);
            if (i==7) { require(ConnectTile(40,40,Tool::Type::Road,budget)==ToolResult::Success,"Bulldoze setup"); }
            const int funds=budget.CurrentFunds();
            tools.currentTool(static_cast<Tool::Type>(i));
            ToolActions::executeTool({40,40},budget,tools);
            require(budget.CurrentFunds()==funds-costs[i],"Original tool cost changed");
            if (i==4) { require(tileValue(40,40)==Dirt,"Free query mutated the map"); continue; }
            if (i==7) { require(maskedTileValue(40,40)==Dirt,"Bulldoze did not clear the road"); continue; }
            const int origin=sizes[i]>1 ? 39 : 40;
            for (int x=origin;x<origin+sizes[i];++x)
                for (int y=origin;y<origin+sizes[i];++y)
                    { require(tileValue(x,y)!=Dirt,"Original tool footprint changed"); }
            require(tileValue(origin-1,origin-1)==Dirt && tileValue(origin+sizes[i],origin+sizes[i])==Dirt,
                "Construction escaped its footprint");

            initializeEngine(city,budget); ResetMap(); budget.CurrentFunds(costs[i]-1);
            ToolActions::executeTool({40,40},budget,tools);
            // Notifications may change; the map and finances must remain intact.
            require(tileValue(40,40)==Dirt && budget.CurrentFunds()==costs[i]-1,"Unaffordable construction changed city/funds");
            budget.CurrentFunds(20000);
            ToolActions::executeTool({-1,40},budget,tools);
            require(budget.CurrentFunds()==20000,"Out-of-bounds construction charged funds");
        }
    }

    void economy()
    {
        Budget funded;
        funded.CurrentFunds(1000); funded.TaxIncome(200);
        funded.RoadFundsNeeded(100); funded.FireFundsNeeded(100); funded.PoliceFundsNeeded(100);
        funded.update();
        require(funded.CurrentFunds()==900 && funded.PreviousFunds()==1000 && funded.CashFlow()==-100 &&
            funded.OperatingExpenses()==300 && !funded.NeedsAttention(),"Original funded budget changed");
        Budget shortfall;
        shortfall.CurrentFunds(40); shortfall.RoadFundsNeeded(100); shortfall.FireFundsNeeded(100); shortfall.PoliceFundsNeeded(100);
        shortfall.update();
        require(shortfall.CurrentFunds()==0 && shortfall.RoadFundsGranted()==40 && shortfall.FireFundsGranted()==0 &&
            shortfall.PoliceFundsGranted()==0 && shortfall.NeedsAttention(),"Original budget funding priority changed");
        funded.TaxRate(-1); require(funded.TaxRate()==0,"Tax lower bound changed");
        funded.TaxRate(21); require(funded.TaxRate()==20,"Tax upper bound changed");
        RCI demand;
        demand.adjustResidentialDemand(10000); demand.adjustCommercialDemand(10000); demand.adjustIndustrialDemand(10000);
        require(demand.residentialDemand()==2000 && demand.commercialDemand()==1500 && demand.industrialDemand()==1500,
            "Original positive demand limits changed");
        demand.adjustResidentialDemand(-10000); demand.adjustCommercialDemand(-10000); demand.adjustIndustrialDemand(-10000);
        require(demand.residentialDemand()==-2000 && demand.commercialDemand()==-1500 && demand.industrialDemand()==-1500,
            "Original negative demand limits changed");
    }

    void networks()
    {
        Budget budget; CityProperties city;
        initializeEngine(city,budget); ResetMap(); seedSimulationRandom(89);
        SimulationTarget={40,40};
        require(makeTraffic(0)==TrafficResult::NoTransportNearby,"A zone without roads generated traffic");
        require(ConnectTile(40,38,Tool::Type::Road,budget)==ToolResult::Success,"Traffic road setup");
        SimulationTarget={40,40};
        require(makeTraffic(0)==TrafficResult::RouteNotFound,"An isolated road found a destination");
        require(ConnectTile(41,38,Tool::Type::Road,budget)==ToolResult::Success,"Traffic connection setup");
        tileValue(41,37)=CommercialBase;
        SimulationTarget={40,40};
        require(makeTraffic(0)==TrafficResult::RouteFound && SimulationTarget==Point<int>{40,40},
            "Original road/destination traffic path changed");

        ResetMap(); resetPowerStack(); CoalPowerPlantCount=1; NuclearPowerPlantCount=0;
        tileValue(40,40)=PowerPlant|ConductiveBit;
        for (int x=41;x<=43;++x) { tileValue(x,40)=PowerBase|ConductiveBit; }
        tileValue(45,40)=PowerBase|ConductiveBit;
        pushPowerStack({40,40}); powerScan();
        require(testPowerBit({41,40}) && testPowerBit({43,40}) && !testPowerBit({45,40}),
            "Original power connectivity crossed an unconnected gap");
        CoalPowerPlantCount=0; resetPowerStack(); pushPowerStack({40,40}); powerScan();
        require(!testPowerBit({41,40}),"Power distribution ignored generation capacity");
    }

    void historicalRejection(const std::filesystem::path& root)
    {
        Budget budget; CityProperties city;
        initializeEngine(city,budget); city.CityName("Retained city"); budget.CurrentFunds(6789);
        const auto before=engineStateDigest(city,budget);
        int checked=0;
        for (const auto& entry : std::filesystem::directory_iterator(root/"cities"))
        {
            if (entry.path().extension()!=".cty") { continue; }
            require(entry.file_size()==27120,"Inherited historical fixture size changed");
            require(!LoadCityDetailed(entry.path(),city,budget),"Unsupported historical format was silently imported");
            require(engineStateDigest(city,budget)==before && city.CityName()=="Retained city" &&
                city.rulesetId()==RulesetId::ClassicV1,"Historical rejection changed the active city");
            ++checked;
        }
        require(checked==24,"Historical fixture coverage incomplete");
    }
}

int main(int argc, char* argv[])
{
    try
    {
        require(argc==2,"Expected repository root");
        static_assert(SimWidth==120 && SimHeight==100 && HistoryLength==120);
        construction(); economy(); networks(); historicalRejection(argv[1]);
        std::cout<<"Original tools/costs/footprints, budget/demand, traffic/power and 24 historical rejection cases passed\n";
        return 0;
    }
    catch (const std::exception& error) { std::cerr<<error.what()<<'\n'; return 1; }
}
