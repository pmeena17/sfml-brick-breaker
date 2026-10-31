#include "core/EffectTracker.hpp"

#include <algorithm>
#include <cstddef>

namespace bb::core
{
void EffectTracker::Activate(PowerUpKind kind) noexcept
{
    m_remainingSeconds[ToIndex(kind)] = GetDefinition(kind).durationSeconds;
}

std::vector<PowerUpKind> EffectTracker::Update(float seconds)
{
    std::vector<PowerUpKind> expired;

    for (std::size_t index = 0; index < m_remainingSeconds.size(); ++index)
    {
        float& remaining = m_remainingSeconds[index];
        if (remaining <= 0.0f)
        {
            continue;
        }

        remaining -= seconds;
        if (remaining <= 0.0f)
        {
            remaining = 0.0f;
            expired.push_back(kPowerUpDefinitions[index].kind);
        }
    }

    // C++ Concept: Returning containers by value (move semantics / copy elision)
    // Returning a std::vector by value is the right thing to do: the compiler constructs `expired`
    // directly in the caller's variable (named return value optimisation) or, failing that, MOVES it
    // (a pointer swap, not an element-by-element copy). An empty vector does not even allocate.
    // Do NOT write `return std::move(expired);` - it defeats the optimisation.
    return expired;
}

EffectModifiers EffectTracker::Modifiers() const noexcept
{
    EffectModifiers modifiers;
    for (const PowerUpDefinition& definition : kPowerUpDefinitions)
    {
        if (IsActive(definition.kind))
        {
            modifiers.ballSpeedScale *= definition.ballSpeedScale;
            modifiers.paddleWidthScale *= definition.paddleWidthScale;
        }
    }
    return modifiers;
}

bool EffectTracker::IsActive(PowerUpKind kind) const noexcept
{
    return m_remainingSeconds[ToIndex(kind)] > 0.0f;
}

float EffectTracker::RemainingSeconds(PowerUpKind kind) const noexcept
{
    return m_remainingSeconds[ToIndex(kind)];
}

bool EffectTracker::AnyActive() const noexcept
{
    // C++ Concept: std::ranges algorithms and lambdas (C++20)
    // `std::ranges::any_of(container, predicate)` takes the whole container, unlike the classic
    // `std::any_of(c.begin(), c.end(), predicate)` where mismatched begin/end iterators are a classic
    // bug. The predicate is a LAMBDA: an unnamed function object written in place. `[]` is its capture
    // list (here empty: it needs nothing from the surrounding scope), `(float seconds)` its parameters.
    // Prefer a named algorithm over a hand-written loop: it states WHAT you want ("is any value > 0")
    // rather than HOW to iterate.
    return std::ranges::any_of(m_remainingSeconds, [](float seconds) { return seconds > 0.0f; });
}

void EffectTracker::Clear() noexcept
{
    m_remainingSeconds.fill(0.0f);
}
} // namespace bb::core
