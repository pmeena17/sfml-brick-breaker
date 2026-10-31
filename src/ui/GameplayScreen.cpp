#include "ui/GameplayScreen.hpp"

#include <algorithm>
#include <cmath>
#include <utility>
#include <variant>

#include "core/Config.hpp"
#include "ui/Theme.hpp"

namespace bb::ui
{
namespace
{
using core::Rect;
using core::Vec2;

constexpr float kToastSeconds = 2.0f;
constexpr float kHudTopRowY = 612.0f;
constexpr float kHudBottomRowY = 629.0f;
constexpr float kHudMargin = 20.0f;
constexpr float kCapsuleBorder = 3.0f;
} // namespace

// C++ Concept: std::visit with a function object (the visitor pattern for std::variant)
// std::visit(visitor, variant) calls the overload of `visitor` that matches whichever alternative the
// variant currently holds. The compiler checks that EVERY alternative can be handled, so adding a new
// GameEvent type that no overload accepts becomes a compile error rather than a silently ignored event.
// Here the visitor is a small struct with several operator() overloads. The catch-all member TEMPLATE
// handles every event type we do not care about; the non-template overload is preferred for the one we do.
// (An alternative is the "overloaded" idiom: `std::visit(overloaded{[](A){...}, [](B){...}}, v)`, which builds
// the same thing out of lambdas.)
struct GameplayScreen::EventHandler
{
    GameplayScreen& screen;

    void operator()(const core::PowerUpCollected& event) const
    {
        const core::PowerUpDefinition& definition = core::GetDefinition(event.kind);
        screen.ShowToast(std::string(definition.displayName) + ": " + std::string(definition.description));
    }

    template <typename OtherEvent>
    void operator()(const OtherEvent& /*event*/) const
    {
    }
};

GameplayScreen::GameplayScreen(const core::LevelRepository& levels, std::size_t levelIndex) : m_session(levels.At(levelIndex)), m_levelIndex(levelIndex)
{
}

void GameplayScreen::ShowToast(std::string text)
{
    m_toast = std::move(text);
    m_toastSeconds = kToastSeconds;
}

std::optional<ScreenCommand> GameplayScreen::HandleEvent(const InputEvent& event)
{
    if (const auto* pressed = std::get_if<KeyPressed>(&event))
    {
        switch (pressed->key)
        {
        case Key::Left:
        case Key::A:
            m_leftHeld = true;
            break;
        case Key::Right:
        case Key::D:
            m_rightHeld = true;
            break;
        case Key::Space:
            if (!m_paused)
            {
                m_session.LaunchBall();
            }
            break;
        case Key::P:
            m_paused = !m_paused;
            break;
        case Key::Escape:
            return ShowMenu{m_levelIndex};
        default:
            break;
        }
        return std::nullopt;
    }

    if (const auto* released = std::get_if<KeyReleased>(&event))
    {
        if (released->key == Key::Left || released->key == Key::A)
        {
            m_leftHeld = false;
        }
        else if (released->key == Key::Right || released->key == Key::D)
        {
            m_rightHeld = false;
        }
        return std::nullopt;
    }

    // C++ Concept: std::holds_alternative
    // For an alternative that carries no data (FocusLost is an empty struct) there is nothing to read, so the
    // simple yes/no test `std::holds_alternative<T>(variant)` is enough.
    if (std::holds_alternative<FocusLost>(event))
    {
        // Key-release events are not delivered to an unfocused window, so forget the held keys (otherwise the
        // paddle would keep running) and pause a running game.
        m_leftHeld = false;
        m_rightHeld = false;
        if (m_session.Phase() == core::SessionPhase::Playing)
        {
            m_paused = true;
        }
    }
    return std::nullopt;
}

std::optional<ScreenCommand> GameplayScreen::Update(float seconds)
{
    m_toastSeconds = std::max(0.0f, m_toastSeconds - seconds);
    if (m_paused)
    {
        return std::nullopt;
    }

    m_session.SetPaddleDirection((m_rightHeld ? 1 : 0) - (m_leftHeld ? 1 : 0));
    m_session.Update(seconds);

    // Range-for over the returned vector is safe: the temporary lives until the loop ends.
    for (const core::GameEvent& event : m_session.DrainEvents())
    {
        std::visit(EventHandler{*this}, event);
    }

    switch (m_session.Phase())
    {
    case core::SessionPhase::Won:
        return ShowResult{ResultKind::Won, m_session.Score(), m_levelIndex};
    case core::SessionPhase::Lost:
        return ShowResult{ResultKind::Lost, m_session.Score(), m_levelIndex};
    case core::SessionPhase::Serving:
    case core::SessionPhase::Playing:
        break;
    }
    return std::nullopt;
}

void GameplayScreen::Render(IRenderer& renderer) const
{
    RenderPlayfield(renderer);
    RenderHud(renderer);

    if (m_paused)
    {
        renderer.FillRect(core::config::kPlayfield, theme::kOverlay);
        renderer.DrawLabel("PAUSED", core::config::kPlayfield.Center(), 40.0f, theme::kScreenText, TextAnchor::Center);
        renderer.DrawLabel("PRESS P TO RESUME", core::config::kPlayfield.Center() + Vec2{0.0f, 40.0f}, 18.0f, theme::kScreenText, TextAnchor::Center);
    }
}

void GameplayScreen::RenderPlayfield(IRenderer& renderer) const
{
    renderer.Clear(theme::kWindowBorder);
    renderer.FillRect(core::config::kPlayfield, theme::kPlayfieldFill);

    for (const core::Brick& brick : m_session.Bricks())
    {
        if (!brick.IsDestroyed())
        {
            renderer.FillRect(brick.Bounds(), theme::BrickColor(brick.HitPoints()));
        }
    }

    for (const core::FallingPowerUp& powerUp : m_session.FallingPowerUps())
    {
        const Rect bounds = powerUp.Bounds();
        const Rect inner{.position = bounds.position + Vec2{kCapsuleBorder, kCapsuleBorder}, .size = bounds.size - Vec2{2.0f * kCapsuleBorder, 2.0f * kCapsuleBorder}};
        renderer.FillRect(bounds, theme::kPanel);
        renderer.FillRect(inner, theme::PowerUpColor(powerUp.kind));
    }

    renderer.FillRect(m_session.GetPaddle().Bounds(), theme::kPaddle);
    renderer.FillCircle(m_session.GetBall().Position(), m_session.GetBall().Radius(), theme::kBall);
}

std::string GameplayScreen::StatusLine() const
{
    if (m_toastSeconds > 0.0f)
    {
        return m_toast;
    }

    std::string effects;
    for (const core::PowerUpDefinition& definition : core::kPowerUpDefinitions)
    {
        if (m_session.Effects().IsActive(definition.kind))
        {
            if (!effects.empty())
            {
                effects += "   ";
            }
            effects += std::string(definition.displayName) + " " + std::to_string(static_cast<int>(std::ceil(m_session.Effects().RemainingSeconds(definition.kind)))) + "s";
        }
    }
    if (!effects.empty())
    {
        return effects;
    }

    if (m_session.Phase() == core::SessionPhase::Serving)
    {
        return "PRESS SPACE TO LAUNCH   -   P: PAUSE   -   ESC: MENU";
    }
    return "P: PAUSE   -   ESC: MENU";
}

void GameplayScreen::RenderHud(IRenderer& renderer) const
{
    const float width = core::config::kWindowSize.x;
    renderer.DrawLabel("SCORE " + std::to_string(m_session.Score()), {kHudMargin, kHudTopRowY}, 16.0f, theme::kHudText, TextAnchor::MiddleLeft);
    renderer.DrawLabel(m_session.LevelName(), {width * 0.5f, kHudTopRowY}, 16.0f, theme::kHudText, TextAnchor::Center);
    renderer.DrawLabel("BRICKS LEFT " + std::to_string(m_session.RemainingBricks()), {width - kHudMargin, kHudTopRowY}, 16.0f, theme::kHudText, TextAnchor::MiddleRight);

    const Color statusColor = (m_toastSeconds > 0.0f) ? theme::kHighlight : theme::kHudText;
    renderer.DrawLabel(StatusLine(), {width * 0.5f, kHudBottomRowY}, 14.0f, statusColor, TextAnchor::Center);
}
} // namespace bb::ui
