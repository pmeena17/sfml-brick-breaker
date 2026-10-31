#include "app/Application.hpp"

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>

#include "app/FontLocator.hpp"
#include "app/SfmlInput.hpp"
#include "core/Config.hpp"

namespace bb::app
{
namespace
{
[[nodiscard]] sf::Font LoadFont(const AssetPaths& assets)
{
    const std::optional<std::filesystem::path> path = FindFontFile(assets);
    if (!path)
    {
        throw std::runtime_error("no usable font found; copy any .ttf file into " + assets.Fonts().generic_string());
    }

    sf::Font font;
    if (!font.openFromFile(*path))
    {
        throw std::runtime_error("cannot load font " + path->generic_string());
    }
    return font;
}

[[nodiscard]] core::LevelRepository LoadLevels(const AssetPaths& assets)
{
    core::LevelRepository levels = core::LevelRepository::LoadFromDirectory(assets.Levels());
    for (const core::LevelLoadIssue& issue : levels.Issues())
    {
        std::cerr << "warning: " << issue.file.generic_string() << ": " << issue.message << '\n';
    }
    return levels;
}

[[nodiscard]] sf::VideoMode WindowMode() noexcept
{
    return sf::VideoMode({static_cast<unsigned int>(core::config::kWindowSize.x), static_cast<unsigned int>(core::config::kWindowSize.y)});
}
} // namespace

Application::Application(const AssetPaths& assets) : m_levels(LoadLevels(assets)), m_window(WindowMode(), "Brick Breaker", sf::Style::Titlebar | sf::Style::Close), m_font(LoadFont(assets)), m_renderer(m_window, m_font), m_navigator(m_levels)
{
    m_window.setVerticalSyncEnabled(true);
    m_window.setKeyRepeatEnabled(false); // we track held keys ourselves; auto-repeat would make P toggle pause repeatedly
}

void Application::PumpEvents()
{
    while (const std::optional<sf::Event> event = m_window.pollEvent())
    {
        if (event->is<sf::Event::Closed>())
        {
            m_window.close();
            continue;
        }

        if (const std::optional<ui::InputEvent> input = TranslateEvent(*event, m_window))
        {
            m_navigator.HandleEvent(*input);
        }
    }
}

int Application::Run()
{
    // C++ Concept: A fixed time-step game loop ("Fix Your Timestep")
    // Rendering speed varies (60 Hz, 144 Hz, a slow laptop), but game physics must not: the ball should travel the
    // same distance per SECOND everywhere. So real elapsed time is accumulated, and the simulation is advanced in
    // equal slices of kFixedTimeStep for as long as a full slice is available. Frames are drawn once per loop
    // iteration regardless. Benefits: identical gameplay on every machine, deterministic behaviour (which is what
    // makes the unit tests possible), and no tunnelling through bricks when a frame takes long (the elapsed time is
    // capped by kMaxFrameTime so a pause - say, dragging the window - cannot cause a huge catch-up burst).
    // The original advanced the ball by a fixed number of pixels per loop iteration (`ball.move(vx, vy)`) and its only
    // explicit timing was one `wait(10)` on the game-over screen, so the ball's speed depended on how long each
    // iteration happened to take on that computer.
    sf::Clock clock;
    float accumulator = 0.0f;

    while (m_window.isOpen() && !m_navigator.ShouldQuit())
    {
        PumpEvents();

        accumulator += std::min(clock.restart().asSeconds(), core::config::kMaxFrameTime);
        while (accumulator >= core::config::kFixedTimeStep)
        {
            m_navigator.Update(core::config::kFixedTimeStep);
            accumulator -= core::config::kFixedTimeStep;
        }

        m_navigator.Render(m_renderer);
        m_window.display();
    }

    return 0;
}
} // namespace bb::app
