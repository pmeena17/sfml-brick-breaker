#include "ui/Button.hpp"

#include "ui/Theme.hpp"

namespace bb::ui
{
void Button::Render(IRenderer& renderer) const
{
    renderer.FillRect(m_bounds, theme::kPanel);
    renderer.DrawLabel(m_label, m_bounds.Center(), m_fontSize, theme::kPanelText, TextAnchor::Center);
}
} // namespace bb::ui
