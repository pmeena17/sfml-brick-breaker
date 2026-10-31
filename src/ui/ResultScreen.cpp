#include "ui/ResultScreen.hpp"

#include <string>
#include <variant>

#include "ui/Theme.hpp"

namespace bb::ui
{
namespace
{
using core::Rect;
using core::Vec2;

constexpr Vec2 kHeadlineCenter{300.0f, 150.0f};
constexpr Vec2 kScoreCenter{300.0f, 240.0f};
constexpr Vec2 kScoreSize{340.0f, 50.0f};
constexpr Vec2 kActionSize{240.0f, 50.0f};
constexpr float kFirstActionY = 340.0f;
constexpr float kActionSpacing = 70.0f;
} // namespace

ResultScreen::ResultScreen(ResultKind kind, int score, std::size_t levelIndex, bool hasNextLevel) : m_kind(kind), m_score(score), m_levelIndex(levelIndex)
{
    // C++ Concept: emplace_back - constructing elements in place
    // `v.push_back(Action{...})` builds a temporary and then moves it into the vector; `v.emplace_back(args...)`
    // forwards the arguments to the element's constructor and builds it directly inside the vector's storage.
    // For aggregates like Action, emplace_back with parentheses needs C++20's parenthesised aggregate
    // initialisation (one of several C++20 features this project relies on).
    const auto addAction = [this](const char* label, ScreenCommand command)
    {
        const float y = kFirstActionY + static_cast<float>(m_actions.size()) * kActionSpacing;
        m_actions.emplace_back(Button{Rect::FromCenter({300.0f, y}, kActionSize), label, 20.0f}, command);
    };

    if (kind == ResultKind::Won && hasNextLevel)
    {
        addAction("NEXT LEVEL", StartLevel{levelIndex + 1});
    }
    addAction(kind == ResultKind::Won ? "PLAY AGAIN" : "TRY AGAIN", StartLevel{levelIndex});
    addAction("MENU", ShowMenu{levelIndex});
}

std::optional<ScreenCommand> ResultScreen::HandleEvent(const InputEvent& event)
{
    if (const auto* key = std::get_if<KeyPressed>(&event))
    {
        if (key->key == Key::Enter || key->key == Key::Space)
        {
            return m_actions.front().command;
        }
        if (key->key == Key::Escape)
        {
            return ShowMenu{m_levelIndex};
        }
        return std::nullopt;
    }

    if (const auto* click = std::get_if<MouseClicked>(&event))
    {
        for (const Action& action : m_actions)
        {
            if (action.button.Contains(click->position))
            {
                return action.command;
            }
        }
    }
    return std::nullopt;
}

std::optional<ScreenCommand> ResultScreen::Update(float /*seconds*/)
{
    return std::nullopt;
}

void ResultScreen::Render(IRenderer& renderer) const
{
    const bool won = (m_kind == ResultKind::Won);
    renderer.Clear(won ? theme::kMenuBackground : theme::kDefeatBackground);

    renderer.DrawLabel(won ? "LEVEL CLEARED!" : "GAME OVER", kHeadlineCenter, 44.0f, theme::kScreenText, TextAnchor::Center);

    const Button scorePanel(Rect::FromCenter(kScoreCenter, kScoreSize), "YOUR SCORE IS : " + std::to_string(m_score), 22.0f);
    scorePanel.Render(renderer);

    for (const Action& action : m_actions)
    {
        action.button.Render(renderer);
    }
}
} // namespace bb::ui
