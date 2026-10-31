#pragma once

#include <SFML/Graphics.hpp>

#include "app/AssetLocator.hpp"
#include "app/SfmlRenderer.hpp"
#include "core/LevelRepository.hpp"
#include "ui/ScreenNavigator.hpp"

namespace bb::app
{
// The composition root: creates the window, loads the assets and runs the main loop. This is the only class
// that wires the pieces (levels, window, font, renderer, navigator) together.
class Application
{
public:
    // Throws std::runtime_error if no font can be found or loaded.
    explicit Application(const AssetPaths& assets);

    // C++ Concept: Deleting copy and move operations on an object that other objects point into
    // m_renderer holds references to m_window and m_font, and m_navigator holds a reference to m_levels.
    // Copying or moving an Application would leave those references pointing at the old object's members.
    // `= delete` turns that mistake into a compile error. (Rule of Five: if you must think about copy/move/
    // destruction at all, decide ALL of them explicitly - here by forbidding them.)
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;
    Application(Application&&) = delete;
    Application& operator=(Application&&) = delete;
    ~Application() = default;

    // Runs until the window is closed or the player quits. Returns the process exit code.
    [[nodiscard]] int Run();

private:
    void PumpEvents();

    // C++ Concept: Member initialisation order = declaration order
    // Members are constructed top to bottom in the order they are DECLARED (and destroyed bottom to top), no matter
    // how the constructor's initializer list is written. The order below is therefore a dependency list: the
    // renderer needs the window and font, the navigator needs the levels.
    core::LevelRepository m_levels;
    sf::RenderWindow m_window;
    sf::Font m_font;
    SfmlRenderer m_renderer;
    ui::ScreenNavigator m_navigator;
};
} // namespace bb::app
