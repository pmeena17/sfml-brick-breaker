#pragma once

#include <cstddef>
#include <variant>

#include "core/PowerUp.hpp"

namespace bb::core
{
// Things that happened during a simulation step. GameSession records them; the layer above (UI, and in
// future sound or particle effects) reacts to them. This keeps the rules of the game free of any
// knowledge about presentation: GameSession never calls "play sound" or "show message" - it just
// reports facts. (The observer pattern, without callbacks or shared mutable state.)
struct BallLaunched
{
};

struct BrickHit
{
    std::size_t brickIndex{0};
    bool destroyed{false};
};

struct PowerUpSpawned
{
    PowerUpKind kind{PowerUpKind::SlowBall};
};

struct PowerUpCollected
{
    PowerUpKind kind{PowerUpKind::SlowBall};
};

struct PowerUpExpired
{
    PowerUpKind kind{PowerUpKind::SlowBall};
};

struct BallLost
{
};

struct LevelCleared
{
};

// C++ Concept: std::variant - a type-safe union ("sum type") - and `using` type aliases
// A GameEvent is EXACTLY ONE of the listed types, and the variant always knows which one. Compared with
// a C union (where you must remember yourself which member is valid) or a base class with a `type`
// field and downcasts, a variant:
//   * stores the value inline (no heap allocation, no pointers),
//   * can never be read as the wrong type (std::get throws, std::get_if returns nullptr),
//   * works with std::visit, which makes the compiler check that you handled every alternative.
// Each event is a tiny struct carrying only the data that event needs.
// `using GameEvent = ...` is the modern spelling of typedef; it reads left to right and also works for
// templates.
using GameEvent = std::variant<BallLaunched, BrickHit, PowerUpSpawned, PowerUpCollected, PowerUpExpired, BallLost, LevelCleared>;
} // namespace bb::core
