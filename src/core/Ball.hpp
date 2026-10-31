#pragma once

#include "core/Vec2.hpp"

namespace bb::core
{
// The ball: a position, a travel DIRECTION and a speed.
//
// Direction and speed are stored separately on purpose. Bouncing only ever changes the direction,
// and power-ups only ever change the speed scale, so neither operation can corrupt the other. The
// original stored a combined velocity (vx, vy) and implemented "double the speed" by literally
// writing `vx = vx * 2` and later `vx = vx / 2` in several places - if the two halves ever get out of
// step (say, a second power-up arrives in between) the ball's speed is changed permanently.
class Ball
{
public:
    // C++ Concept: Member initializer lists
    // `: m_radius(radius), m_baseSpeed(baseSpeed)` initialises the members directly, BEFORE the
    // constructor body runs. Assigning inside the body would first default-construct the members and
    // then overwrite them; for const members and references the initializer list is the only option.
    // Members are initialised in the order they are DECLARED in the class, not the order you list them.
    Ball(float radius, float baseSpeed) noexcept : m_radius(radius), m_baseSpeed(baseSpeed)
    {
    }

    [[nodiscard]] Vec2 Position() const noexcept
    {
        return m_position;
    }

    [[nodiscard]] float Radius() const noexcept
    {
        return m_radius;
    }

    // Unit vector the ball is travelling along; {0,0} while the ball has not been launched.
    [[nodiscard]] Vec2 Direction() const noexcept
    {
        return m_direction;
    }

    [[nodiscard]] float Speed() const noexcept
    {
        return m_baseSpeed * m_speedScale;
    }

    [[nodiscard]] Vec2 Velocity() const noexcept
    {
        return m_direction * Speed();
    }

    void SetPosition(Vec2 position) noexcept
    {
        m_position = position;
    }

    void Launch(Vec2 direction) noexcept
    {
        m_direction = direction.Normalized();
    }

    void SetSpeedScale(float scale) noexcept
    {
        m_speedScale = scale;
    }

    void Advance(float seconds) noexcept
    {
        m_position += Velocity() * seconds;
    }

    // Mirrors the direction about a surface with the given unit normal (pointing AWAY from the
    // surface). Does nothing if the ball is already moving away from it: this stops the ball from
    // "sticking" to a wall or brick by flipping back and forth on consecutive frames.
    void Reflect(Vec2 normal) noexcept
    {
        const float approach = m_direction.Dot(normal);
        if (approach < 0.0f)
        {
            m_direction -= normal * (2.0f * approach);
        }
    }

private:
    Vec2 m_position{};
    Vec2 m_direction{};
    float m_radius;
    float m_baseSpeed;
    float m_speedScale{1.0f};
};
} // namespace bb::core
