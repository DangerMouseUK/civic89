// Civic 89 typed presentation boundary. SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

// Add events only as their call paths migrate from the legacy bridge.
class PresentationEvents
{
public:
    virtual ~PresentationEvents() = default;
    virtual void earthquakeStarted() = 0;
};

// Preserve the inherited no-op visuals until renderer-owned timing exists.
class NullPresentationEvents final : public PresentationEvents
{
public:
    void earthquakeStarted() override {}
};
