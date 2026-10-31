#pragma once

#include <algorithm>
#include <cassert>

#include "core/Rect.hpp"
#include "core/Vec2.hpp"

namespace bb::core
{
// The player's paddle. It only moves horizontally and can never leave the [minX, maxX] corridor it is
// constructed with. The original clamped its position with the magic numbers `xc < 530` / `xc > 50`
// (pasted twice, once for the big and once for the small paddle, and tied to one paddle width and step size).
// Here the clamp is derived from the CURRENT width, so changing the width is always safe.
class Paddle
{
public:
    Paddle(Vec2 center, Vec2 size, float speed, float minX, float maxX) noexcept : m_center(center), m_size(size), m_speed(speed), m_minX(minX), m_maxX(maxX)
    {
        ClampToCorridor();
    }

    [[nodiscard]] Rect Bounds() const noexcept
    {
        return Rect::FromCenter(m_center, m_size);
    }

    [[nodiscard]] Vec2 Center() const noexcept
    {
        return m_center;
    }

    [[nodiscard]] float Width() const noexcept
    {
        return m_size.x;
    }

    // `direction` is -1 (left), 0 (stay) or +1 (right), i.e. the player's INTENT, independent of the
    // key bindings or the input device that produced it.
    void Move(float seconds, int direction) noexcept
    {
        m_center.x += static_cast<float>(direction) * m_speed * seconds;
        ClampToCorridor();
    }

    // Changes the paddle's width while keeping it inside the corridor (e.g. growing next to a wall).
    void SetWidth(float width) noexcept
    {
        m_size.x = width;
        ClampToCorridor();
    }

private:
    // C++ Concept: std::clamp (C++17) and assert
    //   * std::clamp(value, low, high) returns `value` limited to [low, high] - one call instead of a
    //     hand-written `if (v < lo) v = lo; else if (v > hi) v = hi;` that is easy to get backwards.
    //     It requires low <= high; violating that is undefined behaviour.
    //   * assert(condition && "message") checks a PROGRAMMER assumption in debug builds and compiles to nothing
    //     in release builds (NDEBUG). Use it for "this can never happen unless the code is wrong"; use
    //     exceptions or return values for things that can go wrong at run time (bad files, bad input).
    void ClampToCorridor() noexcept
    {
        const float halfWidth = m_size.x * 0.5f;
        assert(m_minX + halfWidth <= m_maxX - halfWidth && "the paddle is wider than the corridor it moves in");
        m_center.x = std::clamp(m_center.x, m_minX + halfWidth, m_maxX - halfWidth);
    }

    Vec2 m_center;
    Vec2 m_size;
    float m_speed;
    float m_minX;
    float m_maxX;
};
} // namespace bb::core
