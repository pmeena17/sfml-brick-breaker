#include "app/SfmlRenderer.hpp"

#include <string>

namespace bb::app
{
namespace
{
[[nodiscard]] sf::Color ToSfml(ui::Color color) noexcept
{
    return sf::Color(color.r, color.g, color.b, color.a);
}

constexpr std::size_t kCirclePointCount = 48;
} // namespace

SfmlRenderer::SfmlRenderer(sf::RenderWindow& window, const sf::Font& font) : m_window(window), m_font(font)
{
    m_circle.setPointCount(kCirclePointCount);
}

void SfmlRenderer::Clear(ui::Color color)
{
    m_window.clear(ToSfml(color));
}

void SfmlRenderer::FillRect(const core::Rect& rect, ui::Color color)
{
    m_rectangle.setPosition({rect.position.x, rect.position.y});
    m_rectangle.setSize({rect.size.x, rect.size.y});
    m_rectangle.setFillColor(ToSfml(color));
    m_window.draw(m_rectangle);
}

void SfmlRenderer::FillCircle(core::Vec2 center, float radius, ui::Color color)
{
    m_circle.setRadius(radius);
    m_circle.setOrigin({radius, radius});
    m_circle.setPosition({center.x, center.y});
    m_circle.setFillColor(ToSfml(color));
    m_window.draw(m_circle);
}

void SfmlRenderer::DrawLabel(std::string_view text, core::Vec2 anchor, float fontSize, ui::Color color, ui::TextAnchor alignment)
{
    // sf::Text needs the font at construction time (SFML 3 removed the default constructor).
    sf::Text label(m_font, std::string(text), static_cast<unsigned int>(fontSize));
    label.setFillColor(ToSfml(color));

    // Shift the text's origin so that `anchor` lands where the caller asked: centre, left-middle or right-middle.
    const sf::FloatRect bounds = label.getLocalBounds();
    float originX = bounds.position.x;
    switch (alignment)
    {
    case ui::TextAnchor::Center:
        originX += bounds.size.x * 0.5f;
        break;
    case ui::TextAnchor::MiddleLeft:
        break;
    case ui::TextAnchor::MiddleRight:
        originX += bounds.size.x;
        break;
    }
    label.setOrigin({originX, bounds.position.y + bounds.size.y * 0.5f});
    label.setPosition({anchor.x, anchor.y});
    m_window.draw(label);
}
} // namespace bb::app
