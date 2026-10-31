#pragma once

#include <optional>

#include <SFML/Graphics.hpp>

#include "ui/InputEvent.hpp"

namespace bb::app
{
// Converts an SFML event into the game's own InputEvent, or std::nullopt for events the game ignores
// (mouse movement, text input, resize, ...). Mouse positions are mapped from window pixels to logical coordinates.
[[nodiscard]] std::optional<ui::InputEvent> TranslateEvent(const sf::Event& event, const sf::RenderWindow& window);
} // namespace bb::app
