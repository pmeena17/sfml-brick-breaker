#pragma once

#include <SFML/Graphics.hpp>

#include "ui/IRenderer.hpp"

namespace bb::app
{
// The SFML implementation of ui::IRenderer: the only drawing code that knows SFML exists.
//
// C++ Concept: Holding references to objects you do not own
// The renderer stores `sf::RenderWindow&` and `const sf::Font&` instead of owning copies. A reference member
// means "this object must be bound at construction and always refers to the same thing", and it makes the class
// non-assignable, which is correct here. The price is a lifetime rule: the window and font must outlive the
// renderer. Application guarantees that by declaring them BEFORE the renderer (members are destroyed in
// reverse order of declaration).
class SfmlRenderer final : public ui::IRenderer
{
public:
    SfmlRenderer(sf::RenderWindow& window, const sf::Font& font);

    void Clear(ui::Color color) override;
    void FillRect(const core::Rect& rect, ui::Color color) override;
    void FillCircle(core::Vec2 center, float radius, ui::Color color) override;
    void DrawLabel(std::string_view text, core::Vec2 anchor, float fontSize, ui::Color color, ui::TextAnchor alignment) override;

private:
    sf::RenderWindow& m_window;
    const sf::Font& m_font;
    // Reused for every draw call instead of constructing a new shape each time.
    sf::RectangleShape m_rectangle;
    sf::CircleShape m_circle;
};
} // namespace bb::app
