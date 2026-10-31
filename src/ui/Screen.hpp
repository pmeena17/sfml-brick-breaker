#pragma once

#include <optional>

#include "ui/IRenderer.hpp"
#include "ui/InputEvent.hpp"
#include "ui/ScreenCommand.hpp"

namespace bb::ui
{
// One full-window UI state: the menu, the instructions, the game itself, the result screen.
//
// Every screen has the same three jobs, so the application loop can drive any of them without knowing which
// one is active - this replaces the original's single `main` with a start-screen block, a level block, the
// game loop and the game-over block all sharing one big set of local variables.
//
// C++ Concept: Polymorphism with `override` and `final` (see MenuScreen, GameplayScreen, ...)
// Derived screens mark their implementations `override`: the compiler then verifies that a function with
// that exact signature exists in the base class. Without it, a typo (or a changed base signature) silently
// creates a NEW unrelated function and the base version is called instead - one of the nastiest bugs in
// older C++. Concrete leaf classes are marked `final` ("nobody derives from me"), which documents intent
// and lets the compiler call their virtual functions directly (devirtualisation).
class Screen
{
public:
    virtual ~Screen() = default;

    Screen(const Screen&) = delete;
    Screen& operator=(const Screen&) = delete;
    Screen(Screen&&) = delete;
    Screen& operator=(Screen&&) = delete;

    // Reacts to one input event. Returns a command if the application should switch screens.
    [[nodiscard]] virtual std::optional<ScreenCommand> HandleEvent(const InputEvent& event) = 0;

    // Advances the screen by `seconds` (a fixed step). Returns a command if the application should switch screens.
    [[nodiscard]] virtual std::optional<ScreenCommand> Update(float seconds) = 0;

    virtual void Render(IRenderer& renderer) const = 0;

protected:
    Screen() = default;
};
} // namespace bb::ui
