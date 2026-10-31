#pragma once

#include <optional>

#include "core/Rect.hpp"
#include "core/Vec2.hpp"

namespace bb::core
{
// Result of a circle-versus-rectangle overlap test.
struct CircleHit
{
    // Axis-aligned unit normal of the face that was hit, pointing from the rectangle towards the
    // circle: {0,-1} = top face, {0,1} = bottom face, {-1,0} = left face, {1,0} = right face.
    // Bricks are axis-aligned, so reflecting about this normal flips exactly one velocity component.
    Vec2 normal{};

    // Translation that moves the circle's centre just far enough to stop overlapping the rectangle.
    Vec2 correction{};

    // How deeply the circle is embedded in the rectangle.
    [[nodiscard]] float Depth() const noexcept
    {
        return correction.Length();
    }
};

// Tests a circle against an axis-aligned rectangle. Returns std::nullopt when they do not overlap.
//
// This single exact test replaces the original's hand-tuned bands such as
// `bcy >= yb2 + 9 && bcy <= yb2 + 15 && bcx <= xb2 + 14 ...`, which only worked for one ball speed
// (it compared floats with == against lists of nearby integers) and treated the ball as a point.
[[nodiscard]] std::optional<CircleHit> TestCircleRect(Vec2 circleCenter, float radius, const Rect& rect) noexcept;
} // namespace bb::core
