#pragma once

#include <cmath>

// C++ Concept: Nested namespace definitions (C++17)
// `namespace bb::core { }` is shorthand for `namespace bb { namespace core { } }`.
// Namespaces replace the C habit of prefixing every function with a project name (bb_core_...).
// They stop symbols from clashing with other libraries (SFML also has a `Vector2`, `Rect`, `Color`...)
// without forcing long names on the caller. Never put `using namespace ...;` in a header: it leaks
// into every file that includes it.
namespace bb::core
{
// C++ Concept: Aggregates, default member initializers and value types
// Vec2 is a plain "value type": it is cheap to copy (8 bytes), has no invariants, and therefore is a
// `struct` with public members (C++ Core Guidelines C.2: use struct when data members can vary
// independently, class when there is an invariant to protect).
//   * `float x{0.0f};` is a default member initializer. It guarantees a Vec2 is NEVER left holding
//     garbage - the original code declared variables such as `int bi, bj;` and `int k;` without any
//     value, and `k` was later compared against 1000 while still uninitialised. Always give members a default.
//   * Because Vec2 has no user-declared constructors it is an aggregate, so `Vec2{3.0f, 4.0f}` and
//     the C++20 designated form `Vec2{.x = 3.0f, .y = 4.0f}` both work, with zero boilerplate.
//   * Pass small value types BY VALUE (`Vec2 other`), larger/expensive types by const reference.
struct Vec2
{
    float x{0.0f};
    float y{0.0f};

    // C++ Concept: constexpr, noexcept and [[nodiscard]]
    //   * constexpr : the function CAN be evaluated at compile time (used for constants such as
    //                 config::kPlayfield and in static_assert). At run time it is an ordinary function.
    //   * noexcept  : promises never to throw. It documents intent and lets the compiler/standard
    //                 library pick faster code paths (e.g. std::vector uses noexcept moves).
    //   * [[nodiscard]] : the compiler warns if the caller ignores the return value. For a pure
    //                 function like operator+ ignoring the result is always a bug (`a + b;` does nothing).
    // Mark every pure getter/calculation [[nodiscard]]; it catches mistakes like `v.Normalized();`
    // (which does NOT normalise v, it returns a new vector).
    [[nodiscard]] constexpr Vec2 operator+(Vec2 other) const noexcept
    {
        return {x + other.x, y + other.y};
    }

    [[nodiscard]] constexpr Vec2 operator-(Vec2 other) const noexcept
    {
        return {x - other.x, y - other.y};
    }

    [[nodiscard]] constexpr Vec2 operator*(float scalar) const noexcept
    {
        return {x * scalar, y * scalar};
    }

    constexpr Vec2& operator+=(Vec2 other) noexcept
    {
        x += other.x;
        y += other.y;
        return *this;
    }

    constexpr Vec2& operator-=(Vec2 other) noexcept
    {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    [[nodiscard]] constexpr float Dot(Vec2 other) const noexcept
    {
        return x * other.x + y * other.y;
    }

    [[nodiscard]] constexpr float LengthSquared() const noexcept
    {
        return Dot(*this);
    }

    // std::sqrt is not constexpr before C++26, so Length() is a normal (non-constexpr) function.
    [[nodiscard]] float Length() const noexcept
    {
        return std::sqrt(LengthSquared());
    }

    // Returns a unit-length copy, or a zero vector if this vector has no direction.
    [[nodiscard]] Vec2 Normalized() const noexcept
    {
        const float length = Length();
        if (length <= 0.0f)
        {
            return {};
        }
        return {x / length, y / length};
    }

    // C++ Concept: Defaulted comparison operators (C++20)
    // `= default` asks the compiler to generate memberwise ==  (and, with it, !=). Before C++20 you
    // had to hand-write both operators and keep them in sync with the data members, which is how
    // "I added a member but forgot to update operator==" bugs happen.
    // Note: this is an EXACT floating-point comparison. For "close enough" comparisons (physics,
    // tests) compare the difference against an epsilon instead.
    [[nodiscard]] constexpr bool operator==(const Vec2& other) const noexcept = default;
};

[[nodiscard]] constexpr Vec2 operator*(float scalar, Vec2 vector) noexcept
{
    return vector * scalar;
}
} // namespace bb::core
