#pragma once

#include <cstddef>
#include <optional>
#include <string>

#include "core/GameSession.hpp"
#include "core/LevelRepository.hpp"
#include "ui/Screen.hpp"

namespace bb::ui
{
// Plays one level: turns key events into GameSession input, advances the simulation, and draws the result.
// All the game RULES live in core::GameSession; this class is only the glue between a session and the
// player (keys in, pictures out).
class GameplayScreen final : public Screen
{
public:
    // `levelIndex` must be valid for `levels`.
    GameplayScreen(const core::LevelRepository& levels, std::size_t levelIndex);

    [[nodiscard]] std::optional<ScreenCommand> HandleEvent(const InputEvent& event) override;
    [[nodiscard]] std::optional<ScreenCommand> Update(float seconds) override;
    void Render(IRenderer& renderer) const override;

    [[nodiscard]] const core::GameSession& Session() const noexcept
    {
        return m_session;
    }

    [[nodiscard]] bool IsPaused() const noexcept
    {
        return m_paused;
    }

private:
    struct EventHandler;

    void ShowToast(std::string text);
    [[nodiscard]] std::string StatusLine() const;
    void RenderPlayfield(IRenderer& renderer) const;
    void RenderHud(IRenderer& renderer) const;

    core::GameSession m_session;
    std::size_t m_levelIndex;
    bool m_paused{false};
    bool m_leftHeld{false};
    bool m_rightHeld{false};
    std::string m_toast;
    float m_toastSeconds{0.0f};
};
} // namespace bb::ui
