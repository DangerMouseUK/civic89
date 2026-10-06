// This file is part of Micropolis-SDLPP
// Micropolis-SDLPP is based on Micropolis
//
// Copyright © 2022 - 2026 Leeor Dicker
//
// Portions Copyright © 1989-2007 Electronic Arts Inc.
//
// Micropolis-SDLPP is free software; you can redistribute it and/or modify
// it under the terms of the GNU GPLv3, with additional terms. See the README
// file, included in this distribution, for details.
#include "s_msg.h"

#include "Budget.h"
#include "Census.h"
#include "Evaluation.h"
#include "EngineState.h"
#include "EngineServices.h"
#include "s_sim.h"
#include "w_resrc.h"
#include "w_sound.h"
#include "PresentationEvents.h"
#include "ScenarioController.h"
#include "ScenarioData.h"
#include "Util.h"

#include "Math/Point.h"


#include <algorithm>
#include <string>


namespace
{
    int LastCityPop{};
    int LastPictureId{};
    int lastMessageTime{};

    NotificationId messageId{};
    NotificationId LastCategory{};

    bool AutoGotoLocation{ false };

    Point<int> messageLocation{};
    std::string lastMessage;

    constexpr auto DefaultMessageDisplayTime{ 3000 };

    int messageDisplayTime{ DefaultMessageDisplayTime };

    NullPresentationEvents nullPresentation;
    PresentationEvents* presentation = &nullPresentation;

    int TickCount()
    {
        return engineMilliseconds();
    }

};


void sharePresentationEvents(PresentationEvents& events)
{
    presentation = &events;
}

void notifyGenerationStarted()
{
    presentation->generationStarted();
}

void MessageDisplayTime(int time)
{
    messageDisplayTime = time;
}


int MessageDisplayTime()
{
    return messageDisplayTime;
}


void AutoGotoMessageLocation(bool autogo)
{
    AutoGotoLocation = autogo;
}


bool AutoGotoMessageLocation()
{
    return AutoGotoLocation;
}


void LastMessage(const std::string& message)
{
    lastMessage = message;
}


const std::string& LastMessage()
{
    return lastMessage;
}


void LastMessageTime(int tick)
{
    lastMessageTime = tick;
}


int LastMessageTime()
{
    return lastMessageTime;
}


NotificationId MessageId()
{
    return messageId;
}


void MessageId(NotificationId id)
{
    messageId = id;
}


void MessageLocation(Point<int> location)
{
    messageLocation = location;
}


const Point<int>& MessageLocation()
{
    return messageLocation;
}


void ClearMes()
{
    MessageId(NotificationId::None);
    MessageLocation({ 0, 0 });
    LastPictureId = 0;
    LastMessageTime(0);
    LastMessage("");
    presentation->showMessage("");
}


void SendMes(NotificationId id)
{
    MessageId(id);

    if (id == NotificationId::None)
    {
        ClearMes();
    }

    LastMessageTime(TickCount());
}


void SendMesAt(NotificationId id, int x, int y)
{
    SendMes(id);

    if (id != NotificationId::None)
    {
        MessageLocation({ x, y });
    }
}


void SetMessageField(const std::string& msg)
{
    if (LastMessage() != msg)
    {
        LastMessage(msg);
		presentation->showMessage(msg);
    }
}


void DoAutoGoto(int x, int y, const std::string& msg)
{
    SetMessageField(msg);
    presentation->focusMap({x, y});
}

void DoScenarioScore(int type)
{
    const auto scenario = scenarioFromLegacyId(type);
    if (!scenario) { return; }
    const auto outcome = scenarioOutcome(*scenario,
        {cityClass() >= CityClass::Metropolis, trafficAverage(), cityScore(), CrimeAverage});
    ClearMes();
    presentation->scenarioFinished(outcome);
}


void CheckGrowth(const Census& census)
{
    if (CityTime % 4 != 0)
    {
        return;
    }

    int currentPopulation = ((census.ResidentialPopulationCount)+(census.CommercialPopulationCount * 8) + (census.IndustrialPopulationCount * 8)) * 20;
    NotificationId growthMessageId = NotificationId::None;

    if (LastCityPop)
    {
        if ((LastCityPop < 2000) && (currentPopulation >= 2000))
        {
            growthMessageId = NotificationId::ReachedTown;
        }
        if ((LastCityPop < 10000) && (currentPopulation >= 10000))
        {
            growthMessageId = NotificationId::ReachedCity;
        }
        if ((LastCityPop < 50000L) && (currentPopulation >= 50000L))
        {
            growthMessageId = NotificationId::ReachedCapital;
        }
        if ((LastCityPop < 100000L) && (currentPopulation >= 100000L))
        {
            growthMessageId = NotificationId::ReachedMetropolis;
        }
        if ((LastCityPop < 500000L) && (currentPopulation >= 500000L))
        {
            growthMessageId = NotificationId::ReachedMegalopolis;
        }
    }
    if (growthMessageId != NotificationId::None &&
        growthMessageId != LastCategory)
    {
        SendMes(growthMessageId);
        LastCategory = growthMessageId;
    }

    LastCityPop = currentPopulation;

}


void SendMessages(const Budget& budget, const Census& census)
{
    if ((ScenarioID) && (ScoreType) && (ScoreWait))
    {
        ScoreWait--;
        if (!ScoreWait)
        {
            DoScenarioScore(ScoreType);
        }
    }

    CheckGrowth(census);

    CombinedZoneCount = census.ResidentialZoneCount + census.CommercialZoneCount + census.IndustrialZoneCount;
    int PowerPop = NuclearPowerPlantCount + CoalPowerPlantCount;

    switch (CityTime % 64)
    {

    case 1:
        if ((CombinedZoneCount / 4) >= ResidentialZoneCount) /* need Res */
        {
            SendMes(NotificationId::ResidentialNeeded);
        }
        break;

    case 5:
        if ((CombinedZoneCount / 8) >= CommercialZoneCount) /* need Com */
        {
            SendMes(NotificationId::CommercialNeeded);
        }
        break;

    case 10:
        if ((CombinedZoneCount / 8) >= IndustrialZoneCount) /* need Ind */
        {
            SendMes(NotificationId::IndustrialNeeded);
        }
        break;

    case 14:
        if ((CombinedZoneCount > 10) && ((CombinedZoneCount << 1) > census.RoadCount))
        {
            SendMes(NotificationId::RoadsNeeded);
        }
        break;

    case 18:
        if ((CombinedZoneCount > 50) && (CombinedZoneCount > census.RailCount))
        {
            SendMes(NotificationId::RailNeeded);
        }
        break;

    case 22:
        if ((CombinedZoneCount > 10) && (PowerPop == 0)) /* need Power */
        {
            SendMes(NotificationId::PowerNeeded);
        }
        break;

    case 26:
        if ((census.ResidentialPopulationCount > 500) && (StadiumCount == 0)) /* need Stad */
        {
            SendMes(NotificationId::StadiumNeeded);
            ResCap = 1;
        }
        else
        {
            ResCap = 0;
        }
        break;

    case 28:
        if ((census.IndustrialPopulationCount > 70) && (SeaPortCount == 0))
        {
            SendMes(NotificationId::SeaportNeeded);
            IndCap = 1;
        }
        else IndCap = 0;
        break;

    case 30:
        if ((census.CommercialPopulationCount > 100) && (AirportCount == 0))
        {
            SendMes(NotificationId::AirportNeeded);
            ComCap = 1;
        }
        else ComCap = 0;
        break;

    case 32: /* dec score for unpowered zones */
    {
        float TM = static_cast<float>(census.UnpoweredZoneCount + census.PoweredZoneCount);

        if (TM)
        {
            if ((census.PoweredZoneCount / TM) < .7)
            {
                SendMes(NotificationId::BlackoutsReported);
            }
        }
    }
        break;

    case 35:
        if (PolluteAverage > 80 /*60*/)
        {
            SendMes(NotificationId::PollutionHigh);
        }
        break;

    case 42:
        if (CrimeAverage > 100)
        {
            SendMes(NotificationId::CrimeHigh);
        }
        break;

    case 45:
        if ((PopulationTotal > 60) && (FireStationCount == 0))
        {
            SendMes(NotificationId::FireDepartmentNeeded);
        }
        break;

    case 48:
        if ((PopulationTotal > 60) && (PoliceStationCount == 0))
        {
            SendMes(NotificationId::PoliceDepartmentNeeded);
        }
        break;

    case 51:
        if (budget.TaxRate() > 12)
        {
            SendMes(NotificationId::TaxesHigh);
        }
        break;

    case 54:
        if ((RoadEffect < 20) && (census.RoadCount > 30))
        {
            SendMes(NotificationId::RoadsDeteriorating);
        }
        break;

    case 57:
        if ((FireEffect < 700) && (PopulationTotal > 20))
        {
            SendMes(NotificationId::FireDefunded);
        }
        break;

    case 60:
        if ((PoliceEffect < 700) && (PopulationTotal > 20))
        {
            SendMes(NotificationId::PoliceDefunded);
        }
        break;

    case 63:
        if (trafficAverage() > 60)
        {
            SendMes(NotificationId::TrafficJamsReported);
        }
        break;
    }
}


void doMessage()
{
    bool firstTime = false;
    
    if (MessageId() == NotificationId::None)
    {
        return;
    }
    else if (MessageId() != NotificationId::None &&
             TickCount() - LastMessageTime() > messageDisplayTime)
    {
        ClearMes();
        return;
    }

    if (firstTime)
    {
        switch (MessageId())
        {
        case NotificationId::TrafficJamsReported:
            if (randomRange(0, 5) == 1)
            {
                MakeSound(SoundId::HonkMedium, AudioChannel::City);
            }
            else if (randomRange(0, 5) == 1)
            {
                MakeSound(SoundId::HonkLow, AudioChannel::City);
            }
            else if (randomRange(0, 5) == 1)
            {
                MakeSound(SoundId::HonkHigh, AudioChannel::City);
            }
            break;

        case NotificationId::CrimeHigh:
        case NotificationId::FireReported:
        case NotificationId::TornadoReported:
        case NotificationId::EarthquakeReported:
        case NotificationId::PlaneCrashed:
        case NotificationId::ShipWrecked:
        case NotificationId::TrainCrashed:
        case NotificationId::HelicopterCrashed:
            MakeSound(SoundId::Siren, AudioChannel::City);
            break;

        case NotificationId::MonsterReported:
            MakeSound(SoundId::Monster, AudioChannel::City);
            break;

        case NotificationId::FirebombingReported:
            MakeSound(SoundId::ExplosionLow, AudioChannel::City);
            MakeSound(SoundId::Siren, AudioChannel::City);
            break;

        case  NotificationId::NuclearMeltdownReported:
            MakeSound(SoundId::ExplosionHigh, AudioChannel::City);
            MakeSound(SoundId::ExplosionLow, AudioChannel::City);
            MakeSound(SoundId::Siren, AudioChannel::City);
            break;

        case  NotificationId::RiotsReported:
            MakeSound(SoundId::Siren, AudioChannel::City);
            break;
                
            default:
                break;
        }

        firstTime = false;
    }

    if (MessageId() != NotificationId::None)
    {
        if (MessageLocation() != Point<int>{0, 0})
        {
            // TODO: draw goto button
        }

        if (AutoGotoMessageLocation() && (MessageLocation() != Point<int>{0, 0}))
        {
            DoAutoGoto(MessageLocation().x, MessageLocation().y, NotificationString(MessageId()));
            MessageLocation({ 0, 0 });
        }
        else
        {
            SetMessageField(NotificationString(MessageId()));
        }
    }

}

PresentationEvents& currentPresentationEvents()
{
    return *presentation;
}
