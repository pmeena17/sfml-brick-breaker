#pragma once

#include <cstddef>
#include <span>
#include <string>
#include <vector>

#include "core/Ball.hpp"
#include "core/Brick.hpp"
#include "core/EffectTracker.hpp"
#include "core/GameEvents.hpp"
#include "core/LevelData.hpp"
#include "core/Paddle.hpp"
#include "core/PowerUp.hpp"

namespace bb::core
{
enum class SessionPhase
{
    Serving, // the ball rests on the paddle, waiting for the player to launch it
    Playing,
    Won,
    Lost,
};

// One play-through of one level: all the rules of the game, and nothing else.
//
// GameSession knows nothing about windows, keyboards, colours or fonts. You feed it intent
// (SetPaddleDirection, LaunchBall) and elapsed time (Update); it exposes read-only state for rendering
// and a list of events. That is what makes it unit-testable without opening a window, and what lets the
// SFML front-end be replaced without touching a single rule. The original interleaved rule code, drawing
// calls (`ball.move`, `b[i]->hide()`), key polling (`getch`) and blocking loops in one `main`.
class GameSession
{
public:
    // C++ Concept: `explicit` constructors
    // A constructor callable with ONE argument also acts as an implicit conversion, so without `explicit`
    // a function taking GameSession would silently accept a LevelData (and build a whole session from it).
    // Mark single-argument constructors `explicit` unless an implicit conversion is exactly what you want.
    explicit GameSession(const LevelData& level);

    // --- Input: the player's intent ----------------------------------------------------------------
    void SetPaddleDirection(int direction) noexcept;
    void LaunchBall();

    // --- Simulation ---------------------------------------------------------------------------------
    // Advances the game by `seconds`. Call with a small, fixed step (config::kFixedTimeStep).
    void Update(float seconds);

    // Returns and clears the events recorded since the last call.
    [[nodiscard]] std::vector<GameEvent> DrainEvents();

    // --- Read-only state for presentation and tests -------------------------------------------------
    [[nodiscard]] SessionPhase Phase() const noexcept
    {
        return m_phase;
    }

    [[nodiscard]] int Score() const noexcept
    {
        return m_score;
    }

    [[nodiscard]] std::size_t RemainingBricks() const noexcept
    {
        return m_remainingBricks;
    }

    [[nodiscard]] const std::string& LevelName() const noexcept
    {
        return m_levelName;
    }

    [[nodiscard]] const Paddle& GetPaddle() const noexcept
    {
        return m_paddle;
    }

    [[nodiscard]] const Ball& GetBall() const noexcept
    {
        return m_ball;
    }

    [[nodiscard]] const EffectTracker& Effects() const noexcept
    {
        return m_effects;
    }

    [[nodiscard]] std::span<const Brick> Bricks() const noexcept
    {
        return m_bricks;
    }

    [[nodiscard]] std::span<const FallingPowerUp> FallingPowerUps() const noexcept
    {
        return m_fallingPowerUps;
    }

private:
    void StickBallToPaddle() noexcept;
    void UpdateEffects(float seconds);
    void ApplyEffectModifiers() noexcept;
    void CollideBallWithWalls() noexcept;
    void CollideBallWithPaddle() noexcept;
    void CollideBallWithBricks();
    void DamageBrick(std::size_t index);
    void UpdateFallingPowerUps(float seconds);
    void CollectPowerUp(PowerUpKind kind);
    void CheckEndConditions();

    std::string m_levelName;
    std::vector<Brick> m_bricks;
    std::size_t m_remainingBricks;
    Paddle m_paddle;
    Ball m_ball;
    EffectTracker m_effects;
    std::vector<FallingPowerUp> m_fallingPowerUps;
    std::vector<GameEvent> m_events;
    SessionPhase m_phase{SessionPhase::Serving};
    int m_score{0};
    int m_paddleDirection{0};
};
} // namespace bb::core
