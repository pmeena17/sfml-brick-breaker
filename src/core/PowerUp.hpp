#pragma once

#include <array>
#include <cstddef>
#include <optional>
#include <string_view>

#include "core/Config.hpp"
#include "core/Rect.hpp"
#include "core/Vec2.hpp"

namespace bb::core
{
// C++ Concept: Scoped enumerations (`enum class`)
// The original code encoded brick state and power-ups as bare integers: 2 = normal, 3 = hidden,
// 4 = two-hit brick, and `if (bj + 5 * bi == 15)` meant "the power attached to brick number 15". Nothing stopped you from
// writing 7 or comparing a brick state to a paddle size.
// An `enum class` fixes that:
//   * the names live inside the enum (PowerUpKind::SlowBall), so they cannot collide with other names;
//   * there are NO implicit conversions to/from int, so `PowerUpKind k = 2;` does not compile;
//   * a `switch` over it makes the compiler warn (-Wswitch) when you forget an enumerator, which is
//     exactly what you want when someone adds a new power-up later.
enum class PowerUpKind
{
    SlowBall,
    FastBall,
    ShrinkPaddle,
};

// Everything that distinguishes one power-up from another, as DATA. Adding a power-up that only
// changes ball speed and/or paddle width means adding an enumerator above plus one row in the table
// below - no new control flow. (Behaviour that needs real code can extend EffectModifiers.)
struct PowerUpDefinition
{
    PowerUpKind kind;
    std::string_view id;          // identifier used in level files, e.g. "slow_ball"
    std::string_view displayName; // shown in the HUD
    std::string_view description; // shown in the instructions screen
    float durationSeconds;
    float ballSpeedScale;    // multiplies the ball's speed while the effect is active
    float paddleWidthScale;  // multiplies the paddle's width while the effect is active
};

// C++ Concept: Class template argument deduction (CTAD, C++17), std::array and std::string_view
//   * std::array<T, N> is a fixed-size array that knows its own size, can be returned/copied like a
//     value and works with the standard algorithms. It replaces raw C arrays like `int xb1[25]`
//     whose size had to be repeated (and kept in sync) everywhere they were used.
//   * CTAD lets the compiler deduce `std::array<PowerUpDefinition, 3>` from the initialiser, so the
//     element count can never disagree with the number of rows.
//   * std::string_view is a NON-OWNING view (pointer + length) of characters. String literals live for
//     the whole program, so views of them are always safe, and no memory is allocated or copied.
// The whole table is constexpr: it is built by the compiler and stored in read-only memory.
inline constexpr std::array kPowerUpDefinitions = {
    PowerUpDefinition{.kind = PowerUpKind::SlowBall, .id = "slow_ball", .displayName = "SLOW BALL", .description = "The ball moves at half speed.", .durationSeconds = 10.0f, .ballSpeedScale = 0.5f, .paddleWidthScale = 1.0f},
    PowerUpDefinition{.kind = PowerUpKind::FastBall, .id = "fast_ball", .displayName = "FAST BALL", .description = "The ball moves at double speed.", .durationSeconds = 10.0f, .ballSpeedScale = 2.0f, .paddleWidthScale = 1.0f},
    PowerUpDefinition{.kind = PowerUpKind::ShrinkPaddle, .id = "shrink_paddle", .displayName = "SMALL PADDLE", .description = "Your paddle is half as wide.", .durationSeconds = 10.0f, .ballSpeedScale = 1.0f, .paddleWidthScale = 0.5f},
};

inline constexpr std::size_t kPowerUpKindCount = kPowerUpDefinitions.size();

[[nodiscard]] constexpr std::size_t ToIndex(PowerUpKind kind) noexcept
{
    // static_cast is the explicit, searchable way to say "I know this converts". (C++23 adds
    // std::to_underlying for exactly this enum -> integer conversion.)
    return static_cast<std::size_t>(kind);
}

namespace detail
{
[[nodiscard]] constexpr bool IsTableOrderedByKind() noexcept
{
    for (std::size_t index = 0; index < kPowerUpDefinitions.size(); ++index)
    {
        if (ToIndex(kPowerUpDefinitions[index].kind) != index)
        {
            return false;
        }
    }
    return true;
}
} // namespace detail

// The table is indexed directly by the enumerator, so its rows MUST be in enum order. This check runs
// at compile time: reorder the rows (or forget one) and the build stops with this message.
static_assert(detail::IsTableOrderedByKind(), "kPowerUpDefinitions must list one row per PowerUpKind, in enum order");

[[nodiscard]] constexpr const PowerUpDefinition& GetDefinition(PowerUpKind kind) noexcept
{
    return kPowerUpDefinitions[ToIndex(kind)];
}

// C++ Concept: std::optional - "a value or nothing"
// Parsing can legitimately fail ("bogus_power" is not a power-up). Instead of returning a magic value
// (-1, nullptr, an "Invalid" enumerator) or throwing for an expected situation, return
// std::optional<T>: the type itself tells the caller "check me". Use `if (auto kind = ...)`,
// `*kind` / `kind.value()` to read, and `kind.has_value()` / `if (kind)` to test.
[[nodiscard]] constexpr std::optional<PowerUpKind> ParsePowerUpId(std::string_view id) noexcept
{
    for (const PowerUpDefinition& definition : kPowerUpDefinitions)
    {
        if (definition.id == id)
        {
            return definition.kind;
        }
    }
    return std::nullopt;
}

// A power-up capsule that has been released by a destroyed brick and is falling towards the paddle.
struct FallingPowerUp
{
    PowerUpKind kind{PowerUpKind::SlowBall};
    Vec2 center{};

    [[nodiscard]] constexpr Rect Bounds() const noexcept
    {
        return Rect::FromCenter(center, config::power_up::kSize);
    }
};
} // namespace bb::core
