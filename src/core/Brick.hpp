#pragma once

#include <algorithm>
#include <optional>

#include "core/PowerUp.hpp"
#include "core/Rect.hpp"

namespace bb::core
{
// A single brick. Replaces the original's five parallel arrays (xb1, xb2, yb1, yb2 and
// arectanglereflect) plus a separate array of Rectangle pointers: everything one brick needs is in one
// object, so the arrays can no longer get out of step with each other.
//
// "Normal" vs "two-hit" bricks are not separate types or magic numbers any more: a brick simply has
// hit points. A brick with 2 hit points is the old "red" brick - after the first hit it has 1 left
// and is drawn like a normal brick, which is exactly the original behaviour, with no special case.
//
// C++ Concept: class vs struct, invariants and encapsulation
// Brick has an invariant: "hit points are never negative". Because the data members are private and
// can only be changed through TakeHit(), no outside code can break that rule. (Compare Vec2, which
// has no invariant and is therefore a plain struct.)
class Brick
{
public:
    // C++ Concept: Constructors with default arguments and std::optional parameters
    // `drop` is "which power-up appears when this brick is destroyed, if any". std::nullopt means
    // "none". A default argument keeps the common case (`Brick{bounds, 1}`) short.
    Brick(const Rect& bounds, int hitPoints, std::optional<PowerUpKind> drop = std::nullopt) noexcept : m_bounds(bounds), m_hitPoints(std::max(hitPoints, 1)), m_maxHitPoints(m_hitPoints), m_drop(drop)
    {
    }

    [[nodiscard]] const Rect& Bounds() const noexcept
    {
        return m_bounds;
    }

    [[nodiscard]] int HitPoints() const noexcept
    {
        return m_hitPoints;
    }

    [[nodiscard]] int MaxHitPoints() const noexcept
    {
        return m_maxHitPoints;
    }

    [[nodiscard]] bool IsDestroyed() const noexcept
    {
        return m_hitPoints <= 0;
    }

    [[nodiscard]] std::optional<PowerUpKind> Drop() const noexcept
    {
        return m_drop;
    }

    // Removes one hit point (never below zero).
    void TakeHit() noexcept
    {
        if (m_hitPoints > 0)
        {
            --m_hitPoints;
        }
    }

private:
    Rect m_bounds;
    int m_hitPoints;
    int m_maxHitPoints;
    std::optional<PowerUpKind> m_drop;
};
} // namespace bb::core
