#pragma once

#include <cstddef>
#include <optional>
#include <vector>

#include "ui/Button.hpp"
#include "ui/Screen.hpp"

namespace bb::ui
{
// Shown when a level ends: "LEVEL CLEARED" or "GAME OVER", the score, and what to do next.
//
// Each button is stored TOGETHER with the command it triggers (the command pattern), so the click handler is
// one loop instead of one `if` per button, and a variable number of buttons (no NEXT LEVEL after the last
// level) needs no special cases.
class ResultScreen final : public Screen
{
public:
    ResultScreen(ResultKind kind, int score, std::size_t levelIndex, bool hasNextLevel);

    [[nodiscard]] std::optional<ScreenCommand> HandleEvent(const InputEvent& event) override;
    [[nodiscard]] std::optional<ScreenCommand> Update(float seconds) override;
    void Render(IRenderer& renderer) const override;

    [[nodiscard]] std::size_t ActionCount() const noexcept
    {
        return m_actions.size();
    }

    [[nodiscard]] const Button& ActionButton(std::size_t index) const
    {
        return m_actions.at(index).button;
    }

private:
    struct Action
    {
        Button button;
        ScreenCommand command;
    };

    ResultKind m_kind;
    int m_score;
    std::size_t m_levelIndex;
    std::vector<Action> m_actions;
};
} // namespace bb::ui
