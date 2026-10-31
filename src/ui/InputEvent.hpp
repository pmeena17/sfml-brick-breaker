#pragma once

#include <variant>

#include "core/Vec2.hpp"

namespace bb::ui
{
// The keys the game cares about. The platform layer translates its own key codes into these, so the
// game never sees SFML (or Win32) key codes. The original compared against raw console scan codes:
// 'M' meant the RIGHT arrow, 'K' the LEFT arrow and 32 meant space - unreadable without a lookup table.
enum class Key
{
    Unknown,
    Left,
    Right,
    A,
    D,
    Space,
    Enter,
    Escape,
    P
};

struct KeyPressed
{
    Key key{Key::Unknown};
};

struct KeyReleased
{
    Key key{Key::Unknown};
};

// A left-click, already converted to logical window coordinates.
struct MouseClicked
{
    core::Vec2 position{};
};

struct FocusLost
{
};

// One input occurrence, delivered to the active screen. See core/GameEvents.hpp for why a std::variant
// of small structs is preferred over one fat struct with a `type` field and unused members.
using InputEvent = std::variant<KeyPressed, KeyReleased, MouseClicked, FocusLost>;
} // namespace bb::ui
