#include "core/Collision.hpp"

#include <algorithm>
#include <cmath>

namespace bb::core
{
namespace
{
// C++ Concept: Anonymous namespaces (internal linkage)
// Everything inside `namespace { }` is visible only in this .cpp file - the modern replacement for
// C's `static` functions/variables. Helpers that are not part of the public interface go here so they
// cannot clash with identically named helpers in other files and are not exposed in the header.
constexpr float kInsideEpsilon = 1e-12f;

[[nodiscard]] Vec2 DominantAxisNormal(Vec2 offset) noexcept
{
    if (std::abs(offset.x) > std::abs(offset.y))
    {
        return {offset.x > 0.0f ? 1.0f : -1.0f, 0.0f};
    }
    return {0.0f, offset.y > 0.0f ? 1.0f : -1.0f};
}
} // namespace

std::optional<CircleHit> TestCircleRect(Vec2 circleCenter, float radius, const Rect& rect) noexcept
{
    // Closest point of the rectangle to the circle's centre. std::clamp(v, lo, hi) (C++17) returns v
    // limited to [lo, hi]; `lo` must not be greater than `hi`.
    const Vec2 closest{std::clamp(circleCenter.x, rect.Left(), rect.Right()), std::clamp(circleCenter.y, rect.Top(), rect.Bottom())};
    const Vec2 offset = circleCenter - closest;
    const float distanceSquared = offset.LengthSquared();

    // Compare SQUARED distances: avoids a square root for the common "no collision" case.
    if (distanceSquared > radius * radius)
    {
        return std::nullopt;
    }

    if (distanceSquared > kInsideEpsilon)
    {
        // The centre is outside the rectangle: push the circle straight back along the line joining
        // the two closest points, but bounce off the dominant axis (so corner hits behave sensibly).
        const float distance = std::sqrt(distanceSquared);
        return CircleHit{.normal = DominantAxisNormal(offset), .correction = offset * ((radius - distance) / distance)};
    }

    // The centre is inside (or exactly on the edge of) the rectangle: leave through the nearest face.
    const float toLeft = circleCenter.x - rect.Left();
    const float toRight = rect.Right() - circleCenter.x;
    const float toTop = circleCenter.y - rect.Top();
    const float toBottom = rect.Bottom() - circleCenter.y;
    const float nearest = std::min({toLeft, toRight, toTop, toBottom});

    Vec2 normal{0.0f, 1.0f};
    if (nearest == toLeft)
    {
        normal = {-1.0f, 0.0f};
    }
    else if (nearest == toRight)
    {
        normal = {1.0f, 0.0f};
    }
    else if (nearest == toTop)
    {
        normal = {0.0f, -1.0f};
    }
    return CircleHit{.normal = normal, .correction = normal * (nearest + radius)};
}
} // namespace bb::core
