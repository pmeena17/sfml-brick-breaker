#pragma once

#include <algorithm>
#include <array>
#include <cstddef>

#include "core/PowerUp.hpp"
#include "ui/Color.hpp"

// Every colour used by the game, in one place. Changing the look of the game (or adding a high-contrast
// mode) means editing this file only; no screen contains a colour literal.
namespace bb::ui::theme
{
inline constexpr Color kWindowBorder{20, 20, 20};
inline constexpr Color kPlayfieldFill{2, 2, 2};
inline constexpr Color kMenuBackground{0, 128, 0};
inline constexpr Color kDefeatBackground{190, 20, 20};
inline constexpr Color kPanel{255, 255, 255};
inline constexpr Color kPanelText{0, 0, 0};
inline constexpr Color kScreenText{255, 255, 255};
inline constexpr Color kHudText{230, 230, 230};
inline constexpr Color kHighlight{255, 215, 0};
inline constexpr Color kPaddle{220, 30, 30};
inline constexpr Color kBall{40, 220, 40};
inline constexpr Color kOverlay{0, 0, 0, 170};

// Brick colour by remaining hit points: 1 = blue (a normal brick), 2 = red (the original "two-hit" brick),
// then orange and purple for tougher bricks a level designer might add.
inline constexpr std::array<Color, 4> kBrickPalette = {Color{30, 90, 255}, Color{220, 40, 40}, Color{240, 140, 20}, Color{150, 60, 200}};

[[nodiscard]] constexpr Color BrickColor(int hitPoints) noexcept
{
    const int clamped = std::clamp(hitPoints, 1, static_cast<int>(kBrickPalette.size()));
    return kBrickPalette[static_cast<std::size_t>(clamped - 1)];
}

// C++ Concept: switch over an enum class WITHOUT a `default:` label
// Because every enumerator is listed and there is no `default`, the compiler warns (-Wswitch) the moment
// someone adds a new PowerUpKind and forgets to give it a colour. A `default:` branch would hide that
// warning and quietly return a wrong colour. The `return` after the switch only exists to satisfy
// compilers when the enum holds an out-of-range value.
[[nodiscard]] constexpr Color PowerUpColor(core::PowerUpKind kind) noexcept
{
    switch (kind)
    {
    case core::PowerUpKind::SlowBall:
        return Color{230, 60, 60};
    case core::PowerUpKind::FastBall:
        return Color{60, 210, 60};
    case core::PowerUpKind::ShrinkPaddle:
        return Color{70, 130, 255};
    }
    return kPanel;
}
} // namespace bb::ui::theme
