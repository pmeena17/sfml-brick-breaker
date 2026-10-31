#pragma once

#include <memory>

#include "core/LevelRepository.hpp"
#include "ui/IRenderer.hpp"
#include "ui/InputEvent.hpp"
#include "ui/Screen.hpp"
#include "ui/ScreenCommand.hpp"

namespace bb::ui
{
// Owns the active screen and is the ONE place that knows how to build every kind of screen.
//
// C++ Concept: std::unique_ptr, RAII and make_unique - ownership you can see in the type
// The navigator holds the current screen as `std::unique_ptr<Screen>`: "I am the only owner; when I let go,
// the screen is destroyed". Assigning a new screen automatically destroys the old one, and the pointer's
// destructor deletes it on every exit path, including exceptions. There is no `new`, no `delete`, no leak.
// (The original allocated every brick with `b[j+5*i] = new Rectangle(...)` and contains no `delete` at all.)
//   * Prefer std::make_unique<T>(args...) to `new T(args...)`: it is exception-safe and shorter.
//   * unique_ptr cannot be copied (that would make two owners), only moved: ownership is transferred explicitly.
//   * Holding the BASE type (`Screen`) lets one pointer refer to any screen; the virtual destructor makes
//     deleting through it correct.
// Rule of thumb: use unique_ptr by default, std::shared_ptr only when ownership is genuinely shared, and a raw
// pointer or reference only to OBSERVE something that someone else owns (like `m_levels` below).
class ScreenNavigator
{
public:
    // `levels` is only observed and must outlive the navigator. The first screen is the menu.
    explicit ScreenNavigator(const core::LevelRepository& levels);

    void HandleEvent(const InputEvent& event);
    void Update(float seconds);
    void Render(IRenderer& renderer) const;

    [[nodiscard]] bool ShouldQuit() const noexcept
    {
        return m_quit;
    }

    [[nodiscard]] const Screen& Current() const noexcept
    {
        return *m_current;
    }

private:
    struct CommandVisitor;

    void Apply(const ScreenCommand& command);
    void ShowMenuScreen(std::size_t focusLevel);
    void StartLevelScreen(std::size_t levelIndex);

    const core::LevelRepository& m_levels;
    std::unique_ptr<Screen> m_current;
    bool m_quit{false};
};
} // namespace bb::ui
