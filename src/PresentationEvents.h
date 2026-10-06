// Civic 89 typed presentation boundary. SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "FileIo.h"
#include "Math/Point.h"

enum class ScenarioOutcome { Won, Lost };

// Add events only as their call paths migrate from the legacy bridge.
class PresentationEvents
{
public:
    virtual ~PresentationEvents() = default;
    virtual void toolsReset() {}
    virtual void budgetRequested() {}
    virtual void earthquakeStarted() = 0;
    virtual void showMessage(const std::string& message) = 0;
    virtual void focusMap(Point<int> location) = 0;
    virtual void generationStarted() = 0;
    virtual void scenarioStarted(Scenario scenario) = 0;
    virtual void scenarioFinished(ScenarioOutcome outcome) = 0;
};

// Preserve the inherited no-op visuals until renderer-owned timing exists.
class NullPresentationEvents final : public PresentationEvents
{
public:
    void earthquakeStarted() override {}
    void showMessage(const std::string&) override {}
    void focusMap(Point<int>) override {}
    void generationStarted() override {}
    void scenarioStarted(Scenario) override {}
    void scenarioFinished(ScenarioOutcome) override {}
};
