#pragma once

#include <cstdint>

namespace bb::ui
{
// An 8-bit-per-channel RGBA colour. This is OUR colour type: the game never mentions sf::Color outside
// the SFML adapter, so swapping the graphics library only means rewriting that one adapter.
//
// C++ Concept: Fixed-width integer types (<cstdint>)
// `int`, `long` and `char` have sizes that vary between platforms. When the size is part of the
// meaning - "one byte per colour channel" - use std::uint8_t, std::int32_t, std::uint64_t and friends.
// Brace initialisation also protects you: `Color{300, 0, 0}` is a compile error (narrowing conversion)
// instead of silently wrapping around to 44.
struct Color
{
    std::uint8_t r{0};
    std::uint8_t g{0};
    std::uint8_t b{0};
    std::uint8_t a{255};
};
} // namespace bb::ui
