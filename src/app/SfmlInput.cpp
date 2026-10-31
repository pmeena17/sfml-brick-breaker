#include "app/SfmlInput.hpp"

namespace bb::app
{
namespace
{
[[nodiscard]] ui::Key TranslateKey(sf::Keyboard::Key key) noexcept
{
    switch (key)
    {
    case sf::Keyboard::Key::Left:
        return ui::Key::Left;
    case sf::Keyboard::Key::Right:
        return ui::Key::Right;
    case sf::Keyboard::Key::A:
        return ui::Key::A;
    case sf::Keyboard::Key::D:
        return ui::Key::D;
    case sf::Keyboard::Key::Space:
        return ui::Key::Space;
    case sf::Keyboard::Key::Enter:
        return ui::Key::Enter;
    case sf::Keyboard::Key::Escape:
        return ui::Key::Escape;
    case sf::Keyboard::Key::P:
        return ui::Key::P;
    default:
        return ui::Key::Unknown;
    }
}
} // namespace

std::optional<ui::InputEvent> TranslateEvent(const sf::Event& event, const sf::RenderWindow& window)
{
    // C++ Concept: SFML 3's event model is itself built on std::variant / std::optional
    // `event.getIf<T>()` returns a pointer to the event's data if it is a T, otherwise nullptr - the same idea as
    // std::get_if on a variant (see MenuScreen.cpp). Each event type is its own small struct (KeyPressed,
    // MouseButtonPressed, ...) carrying just its own fields, instead of SFML 2's one big union with a `type` tag.
    if (const auto* pressed = event.getIf<sf::Event::KeyPressed>())
    {
        return ui::KeyPressed{TranslateKey(pressed->code)};
    }

    if (const auto* released = event.getIf<sf::Event::KeyReleased>())
    {
        return ui::KeyReleased{TranslateKey(released->code)};
    }

    if (const auto* click = event.getIf<sf::Event::MouseButtonPressed>())
    {
        if (click->button == sf::Mouse::Button::Left)
        {
            const sf::Vector2f logical = window.mapPixelToCoords(click->position);
            return ui::MouseClicked{{logical.x, logical.y}};
        }
        return std::nullopt;
    }

    if (event.is<sf::Event::FocusLost>())
    {
        return ui::FocusLost{};
    }

    return std::nullopt;
}
} // namespace bb::app
