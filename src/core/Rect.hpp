#pragma once

#include "core/Vec2.hpp"

namespace bb::core
{
// Axis-aligned rectangle described by its top-left corner and its size.
// Screen coordinates: x grows to the right, y grows DOWNWARDS (like SFML, SDL and most 2D APIs).
//
// C++ Concept: Designated initializers (C++20)
// Because Rect is an aggregate you can write `Rect{.position = {10.0f, 10.0f}, .size = {580.0f, 580.0f}}`.
// Each value is labelled with the member it initialises, so the call site stays readable when a
// type has several members of the same type (compare with the original C++ code's unlabelled
// `Rectangle(300, 300, 580, 580)` where you had to remember the argument order). Designated
// initializers must follow the declaration order of the members.
struct Rect
{
    Vec2 position{};
    Vec2 size{};

    // C++ Concept: Named constructors (static factory functions)
    // A constructor cannot say WHAT its arguments mean: Rect(a, b) could be (topLeft, size) or
    // (center, size). A static function with a descriptive name removes the ambiguity at the call
    // site: `Rect::FromCenter(center, size)`. The parameters are deliberately not called `size`
    // so they do not shadow the data member (-Wshadow).
    [[nodiscard]] static constexpr Rect FromCenter(Vec2 centre, Vec2 dimensions) noexcept
    {
        return {.position = {centre.x - dimensions.x * 0.5f, centre.y - dimensions.y * 0.5f}, .size = dimensions};
    }

    [[nodiscard]] constexpr float Left() const noexcept
    {
        return position.x;
    }

    [[nodiscard]] constexpr float Right() const noexcept
    {
        return position.x + size.x;
    }

    [[nodiscard]] constexpr float Top() const noexcept
    {
        return position.y;
    }

    [[nodiscard]] constexpr float Bottom() const noexcept
    {
        return position.y + size.y;
    }

    [[nodiscard]] constexpr Vec2 Center() const noexcept
    {
        return {position.x + size.x * 0.5f, position.y + size.y * 0.5f};
    }

    [[nodiscard]] constexpr bool Contains(Vec2 point) const noexcept
    {
        return point.x >= Left() && point.x <= Right() && point.y >= Top() && point.y <= Bottom();
    }

    [[nodiscard]] constexpr bool Intersects(const Rect& other) const noexcept
    {
        return Left() < other.Right() && Right() > other.Left() && Top() < other.Bottom() && Bottom() > other.Top();
    }

    [[nodiscard]] constexpr bool operator==(const Rect& other) const noexcept = default;
};
} // namespace bb::core
