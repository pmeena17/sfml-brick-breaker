#include "core/GameSession.hpp"

#include <algorithm>
#include <optional>

#include "core/Collision.hpp"
#include "core/Config.hpp"

namespace bb::core
{
GameSession::GameSession(const LevelData& level) : m_levelName(level.name), m_bricks(level.bricks), m_remainingBricks(level.bricks.size()), m_paddle(Vec2{config::kPlayfield.Center().x, config::paddle::kCenterY}, Vec2{config::paddle::kWidth, config::paddle::kHeight}, config::paddle::kSpeed, config::kPlayfield.Left(), config::kPlayfield.Right()), m_ball(config::ball::kRadius, config::ball::kBaseSpeed)
{
    StickBallToPaddle();
}

void GameSession::SetPaddleDirection(int direction) noexcept
{
    m_paddleDirection = std::clamp(direction, -1, 1);
}

void GameSession::LaunchBall()
{
    if (m_phase != SessionPhase::Serving)
    {
        return;
    }

    m_ball.Launch(config::ball::kLaunchDirection);
    m_phase = SessionPhase::Playing;
    m_events.emplace_back(BallLaunched{});
}

void GameSession::Update(float seconds)
{
    if (m_phase == SessionPhase::Won || m_phase == SessionPhase::Lost)
    {
        return;
    }

    UpdateEffects(seconds);
    m_paddle.Move(seconds, m_paddleDirection);

    if (m_phase == SessionPhase::Serving)
    {
        StickBallToPaddle();
        return;
    }

    // The order matters: move first, then resolve contacts from the outside in (walls, paddle, bricks),
    // then let falling capsules react to the paddle's final position, and only then decide whether the
    // level is won or lost.
    m_ball.Advance(seconds);
    CollideBallWithWalls();
    CollideBallWithPaddle();
    CollideBallWithBricks();
    UpdateFallingPowerUps(seconds);
    CheckEndConditions();
}

std::vector<GameEvent> GameSession::DrainEvents()
{
    // Swapping with an empty vector hands the caller everything and leaves us empty, without copying elements.
    std::vector<GameEvent> drained;
    drained.swap(m_events);
    return drained;
}

void GameSession::StickBallToPaddle() noexcept
{
    const Rect paddleBounds = m_paddle.Bounds();
    m_ball.SetPosition({paddleBounds.Center().x, paddleBounds.Top() - m_ball.Radius()});
}

void GameSession::UpdateEffects(float seconds)
{
    const std::vector<PowerUpKind> expired = m_effects.Update(seconds);
    if (expired.empty())
    {
        return;
    }

    for (const PowerUpKind kind : expired)
    {
        m_events.emplace_back(PowerUpExpired{kind});
    }
    ApplyEffectModifiers();
}

void GameSession::ApplyEffectModifiers() noexcept
{
    // The single place where active effects touch the entities. Speed and width are always recomputed
    // from the BASE values, so they can never drift no matter in which order effects start and stop.
    const EffectModifiers modifiers = m_effects.Modifiers();
    m_ball.SetSpeedScale(modifiers.ballSpeedScale);
    m_paddle.SetWidth(config::paddle::kWidth * modifiers.paddleWidthScale);
}

void GameSession::CollideBallWithWalls() noexcept
{
    const Rect& field = config::kPlayfield;
    const float radius = m_ball.Radius();
    Vec2 position = m_ball.Position();

    // The ball is pushed back inside the field AND its direction is mirrored. Reflect() ignores the call
    // if the ball is already heading away from the wall, so the ball can never get stuck in a wall.
    if (position.x - radius < field.Left())
    {
        position.x = field.Left() + radius;
        m_ball.Reflect({1.0f, 0.0f});
    }
    else if (position.x + radius > field.Right())
    {
        position.x = field.Right() - radius;
        m_ball.Reflect({-1.0f, 0.0f});
    }

    if (position.y - radius < field.Top())
    {
        position.y = field.Top() + radius;
        m_ball.Reflect({0.0f, 1.0f});
    }

    m_ball.SetPosition(position);
    // There is deliberately no bottom wall: a ball that leaves through the bottom loses the level.
}

void GameSession::CollideBallWithPaddle() noexcept
{
    const Rect paddleBounds = m_paddle.Bounds();
    const Vec2 position = m_ball.Position();
    const bool movingDown = m_ball.Direction().y > 0.0f;
    const bool aboveCenterLine = position.y < paddleBounds.Center().y; // a ball that has slipped below the paddle's middle is lost

    // C++ Concept: `if` with an initializer (C++17)
    // `if (init; condition)` declares a variable that exists only inside the if/else. `hit` cannot leak
    // into the rest of the function, so it cannot be misused later - the same reason you write
    // `for (int i = 0; ...)` instead of declaring `i` above the loop. Here `hit` is a std::optional: it is
    // contextually converted to bool, so `hit && ...` means "there was a collision and ...".
    if (const std::optional<CircleHit> hit = TestCircleRect(position, m_ball.Radius(), paddleBounds); hit && movingDown && aboveCenterLine)
    {
        m_ball.SetPosition({position.x, paddleBounds.Top() - m_ball.Radius()});
        m_ball.Reflect({0.0f, -1.0f});
    }
}

void GameSession::CollideBallWithBricks()
{
    // Only the DEEPEST overlapping brick reacts in a given step. If the ball touches two bricks at once
    // (a corner between neighbours), reacting to both would flip its direction twice - i.e. not at all -
    // and the ball would carve straight through the wall of bricks.
    std::optional<std::size_t> hitIndex;
    CircleHit deepest;
    float deepestDepth = 0.0f;

    for (std::size_t index = 0; index < m_bricks.size(); ++index)
    {
        if (m_bricks[index].IsDestroyed())
        {
            continue;
        }

        const std::optional<CircleHit> hit = TestCircleRect(m_ball.Position(), m_ball.Radius(), m_bricks[index].Bounds());
        if (!hit || m_ball.Direction().Dot(hit->normal) >= 0.0f)
        {
            continue; // no overlap, or the ball is already moving away from that face
        }

        const float depth = hit->Depth();
        if (!hitIndex || depth > deepestDepth)
        {
            hitIndex = index;
            deepest = *hit;
            deepestDepth = depth;
        }
    }

    if (hitIndex)
    {
        m_ball.SetPosition(m_ball.Position() + deepest.correction);
        m_ball.Reflect(deepest.normal);
        DamageBrick(*hitIndex);
    }
}

void GameSession::DamageBrick(std::size_t index)
{
    Brick& brick = m_bricks[index];
    brick.TakeHit();

    const bool destroyed = brick.IsDestroyed();
    m_events.emplace_back(BrickHit{.brickIndex = index, .destroyed = destroyed});
    if (!destroyed)
    {
        return;
    }

    m_score += config::kScorePerBrick;
    --m_remainingBricks;

    if (const std::optional<PowerUpKind> drop = brick.Drop())
    {
        m_fallingPowerUps.push_back({.kind = *drop, .center = brick.Bounds().Center()});
        m_events.emplace_back(PowerUpSpawned{*drop});
    }
}

void GameSession::UpdateFallingPowerUps(float seconds)
{
    for (FallingPowerUp& powerUp : m_fallingPowerUps)
    {
        powerUp.center.y += config::power_up::kFallSpeed * seconds;
    }

    const Rect paddleBounds = m_paddle.Bounds();
    for (const FallingPowerUp& powerUp : m_fallingPowerUps)
    {
        if (powerUp.Bounds().Intersects(paddleBounds))
        {
            CollectPowerUp(powerUp.kind);
        }
    }

    // C++ Concept: std::erase_if (C++20) and lambda captures
    // Removing elements from a vector used to need the "erase-remove idiom":
    //     v.erase(std::remove_if(v.begin(), v.end(), pred), v.end());
    // which is easy to get wrong (forgetting the second argument to erase leaves garbage at the end).
    // `std::erase_if(v, pred)` does it in one call. The predicate is a lambda; `[&]` captures the local
    // variables it uses (here `paddleBounds`) BY REFERENCE. Capture by value (`[=]`) copies them
    // instead; prefer capturing explicitly (`[&paddleBounds]`) in larger lambdas so the reader sees
    // exactly what the lambda depends on, and be careful never to let a by-reference lambda outlive the
    // variables it refers to.
    // A capsule is removed when the paddle caught it or when it has fallen out of the playfield.
    std::erase_if(m_fallingPowerUps, [&](const FallingPowerUp& powerUp) { return powerUp.Bounds().Intersects(paddleBounds) || powerUp.Bounds().Bottom() > config::kPlayfield.Bottom(); });
}

void GameSession::CollectPowerUp(PowerUpKind kind)
{
    m_effects.Activate(kind);
    ApplyEffectModifiers();
    m_events.emplace_back(PowerUpCollected{kind});
}

void GameSession::CheckEndConditions()
{
    if (m_remainingBricks == 0)
    {
        m_phase = SessionPhase::Won;
        m_events.emplace_back(LevelCleared{});
        return;
    }

    // The ball is lost once it is completely below the playfield.
    if (m_ball.Position().y - m_ball.Radius() > config::kPlayfield.Bottom())
    {
        m_phase = SessionPhase::Lost;
        m_events.emplace_back(BallLost{});
    }
}
} // namespace bb::core
