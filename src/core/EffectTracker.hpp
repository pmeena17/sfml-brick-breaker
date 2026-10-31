#pragma once

#include <array>
#include <vector>

#include "core/PowerUp.hpp"

namespace bb::core
{
// The combined result of every active power-up, expressed as plain multipliers. GameSession applies
// these to the ball and paddle; it never needs to know WHICH power-ups are active.
struct EffectModifiers
{
    float ballSpeedScale{1.0f};
    float paddleWidthScale{1.0f};
};

// Tracks which power-ups are currently active and for how long.
//
// Each kind has its own countdown, so effects never interfere with each other. The original shared counters
// between powers (`t` for the two ball-speed powers, `k` for the paddle) plus a single `powertaken` flag, and the
// bugs followed: `t` was declared in several nested scopes (so there are several different variables called `t`),
// the `int k=0;` written inside the pick-up block declared a NEW local that shadowed the outer `int k;` - the
// one the timer really uses (`k++; if (k == 1000)`) and which was never given a value - and the speed was
// restored by hand-dividing vx and vy.
// Here the speed is always recomputed from the base speed and the set of active effects.
class EffectTracker
{
public:
    // Starts the effect, or refreshes its timer to the full duration if it is already running.
    void Activate(PowerUpKind kind) noexcept;

    // Advances all timers. Returns the effects that ran out during this step (usually empty).
    [[nodiscard]] std::vector<PowerUpKind> Update(float seconds);

    // Product of the scales of all active effects (no active effects -> everything is 1.0).
    [[nodiscard]] EffectModifiers Modifiers() const noexcept;

    [[nodiscard]] bool IsActive(PowerUpKind kind) const noexcept;
    [[nodiscard]] float RemainingSeconds(PowerUpKind kind) const noexcept;
    [[nodiscard]] bool AnyActive() const noexcept;

    void Clear() noexcept;

private:
    // Seconds left per kind, indexed by ToIndex(kind). 0 means "not active". Value-initialised to zeros.
    std::array<float, kPowerUpKindCount> m_remainingSeconds{};
};
} // namespace bb::core
