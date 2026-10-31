#pragma once

#include <cstddef>
#include <variant>

namespace bb::ui
{
enum class ResultKind
{
    Won,
    Lost
};

// Requests a screen can make to the application ("go to the menu", "start level 3", ...).
//
// A screen never creates or destroys other screens itself - that would make every screen depend on every
// other one. It just RETURNS a command and ScreenNavigator, the one place that knows how to build screens,
// carries it out. (The original program navigated with `goto b;` jumping back to a label in the middle of
// `main`, which could not be reasoned about, tested, or extended.)
struct ShowMenu
{
    std::size_t focusLevel{0}; // the menu opens on the page that contains this level
};

struct ShowInstructions
{
};

struct StartLevel
{
    std::size_t levelIndex{0};
};

struct ShowResult
{
    ResultKind kind{ResultKind::Lost};
    int score{0};
    std::size_t levelIndex{0};
};

struct QuitApplication
{
};

using ScreenCommand = std::variant<ShowMenu, ShowInstructions, StartLevel, ShowResult, QuitApplication>;
} // namespace bb::ui
