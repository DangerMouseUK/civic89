// Civic 89 presentation camera. SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "Math/Point.h"
#include "Math/Vector.h"

class Camera2D
{
public:
    explicit Camera2D(Vector<float> world = {1920, 1600}) : mWorld(world) {}
    void viewport(Vector<float> size);
    Vector<float> viewport() const { return mViewport; }
    Vector<float> visibleWorld() const { return mViewport / mZoom; }
    Point<float> position() const;
    void position(Point<float> value);
    void pan(Vector<float> screenDelta);
    void focus(Point<float> worldPoint);
    float zoom() const { return mZoom; }
    void zoomAt(float value, Point<float> screenPoint);
    bool pixelPerfect() const { return mPixelPerfect; }
    void pixelPerfect(bool enabled);
    void displayScale(float value);
    Point<float> screenToWorld(Point<float> value) const;
    Point<float> worldToScreen(Point<float> value) const;
    bool containsWorld(Point<float> value) const;
private:
    void clamp();
    float constrainedZoom(float value) const;
    Vector<float> mWorld;
    Vector<float> mViewport{800, 600};
    Point<float> mPosition{};
    float mZoom{1};
    float mDisplayScale{1};
    bool mPixelPerfect{false};
};
