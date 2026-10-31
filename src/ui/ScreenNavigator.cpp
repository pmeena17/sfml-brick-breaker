#include "ui/ScreenNavigator.hpp"

#include <memory>
#include <variant>

#include "ui/GameplayScreen.hpp"
#include "ui/InstructionsScreen.hpp"
#include "ui/MenuScreen.hpp"
#include "ui/ResultScreen.hpp"

namespace bb::ui
{
// Carries out one ScreenCommand. Each overload is the complete "how" for one kind of request; see
// GameplayScreen.cpp for how std::visit selects the overload.
struct ScreenNavigator::CommandVisitor
{
    ScreenNavigator& navigator;

    void operator()(const ShowMenu& command) const
    {
        navigator.ShowMenuScreen(command.focusLevel);
    }

    void operator()(const ShowInstructions& /*command*/) const
    {
        navigator.m_current = std::make_unique<InstructionsScreen>();
    }

    void operator()(const StartLevel& command) const
    {
        navigator.StartLevelScreen(command.levelIndex);
    }

    void operator()(const ShowResult& command) const
    {
        const bool hasNextLevel = navigator.m_levels.HasNext(command.levelIndex);
        navigator.m_current = std::make_unique<ResultScreen>(command.kind, command.score, command.levelIndex, hasNextLevel);
    }

    void operator()(const QuitApplication& /*command*/) const
    {
        navigator.m_quit = true;
    }
};

ScreenNavigator::ScreenNavigator(const core::LevelRepository& levels) : m_levels(levels)
{
    ShowMenuScreen(0);
}

void ScreenNavigator::HandleEvent(const InputEvent& event)
{
    if (const std::optional<ScreenCommand> command = m_current->HandleEvent(event))
    {
        Apply(*command);
    }
}

void ScreenNavigator::Update(float seconds)
{
    if (const std::optional<ScreenCommand> command = m_current->Update(seconds))
    {
        Apply(*command);
    }
}

void ScreenNavigator::Render(IRenderer& renderer) const
{
    m_current->Render(renderer);
}

void ScreenNavigator::Apply(const ScreenCommand& command)
{
    // Assigning to m_current destroys the screen that produced `command`. That is safe because `command` is a
    // copy owned by the caller, not a reference into the old screen - never keep pointers into a screen across Apply().
    std::visit(CommandVisitor{*this}, command);
}

void ScreenNavigator::ShowMenuScreen(std::size_t focusLevel)
{
    m_current = std::make_unique<MenuScreen>(m_levels, focusLevel);
}

void ScreenNavigator::StartLevelScreen(std::size_t levelIndex)
{
    // A stale or corrupt request (e.g. "next level" after the last one) falls back to the menu instead of crashing.
    if (levelIndex >= m_levels.Count())
    {
        ShowMenuScreen(0);
        return;
    }
    m_current = std::make_unique<GameplayScreen>(m_levels, levelIndex);
}
} // namespace bb::ui
