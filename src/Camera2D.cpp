// Civic 89 presentation camera. SPDX-License-Identifier: GPL-3.0-or-later
#include "Camera2D.h"
#include <algorithm>
#include <cmath>

void Camera2D::clamp()
{
    const auto visible = visibleWorld();
    mPosition.x = std::clamp(mPosition.x, 0.f, std::max(0.f, mWorld.x - visible.x));
    mPosition.y = std::clamp(mPosition.y, 0.f, std::max(0.f, mWorld.y - visible.y));
}
void Camera2D::viewport(Vector<float> size)
{
    mViewport = {std::max(1.f, size.x), std::max(1.f, size.y)};
    clamp();
}
void Camera2D::position(Point<float> value) { mPosition = value; clamp(); }
Point<float> Camera2D::position() const
{
    if (!mPixelPerfect) { return mPosition; }
    const float pixels = mZoom * mDisplayScale;
    return {std::floor(mPosition.x * pixels) / pixels, std::floor(mPosition.y * pixels) / pixels};
}
void Camera2D::pan(Vector<float> screenDelta) { mPosition += screenDelta / mZoom; clamp(); }
void Camera2D::focus(Point<float> worldPoint) { position(worldPoint - visibleWorld() / 2.f); }
float Camera2D::constrainedZoom(float value) const
{
    if (!std::isfinite(value)) { return mZoom; }
    value = std::clamp(value, .25f, 4.f);
    // Integer physical pixels per source pixel, even at fractional Windows DPI.
    if (mPixelPerfect) { value = std::max(1.f, std::round(value * mDisplayScale)) / mDisplayScale; }
    return value;
}
void Camera2D::zoomAt(float value, Point<float> screenPoint)
{
    const auto anchor = screenToWorld(screenPoint);
    mZoom = constrainedZoom(value);
    position(anchor - Vector<float>{screenPoint.x / mZoom, screenPoint.y / mZoom});
}
void Camera2D::pixelPerfect(bool enabled)
{
    mPixelPerfect = enabled;
    zoomAt(mZoom, {mViewport.x / 2, mViewport.y / 2});
}
void Camera2D::displayScale(float value)
{
    mDisplayScale = std::isfinite(value) && value > 0 ? value : 1;
    zoomAt(mZoom, {mViewport.x / 2, mViewport.y / 2});
}
Point<float> Camera2D::screenToWorld(Point<float> value) const
{
    return position() + Vector<float>{value.x / mZoom, value.y / mZoom};
}
Point<float> Camera2D::worldToScreen(Point<float> value) const
{
    const auto delta = (value - position()) * mZoom;
    return {delta.x, delta.y};
}
bool Camera2D::containsWorld(Point<float> value) const
{
    return value.x >= 0 && value.y >= 0 && value.x < mWorld.x && value.y < mWorld.y;
}
