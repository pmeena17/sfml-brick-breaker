#pragma once

#include <string>
#include <utility>

#include "core/Rect.hpp"
#include "core/Vec2.hpp"
#include "ui/IRenderer.hpp"

namespace bb::ui
{
// A labelled rectangle that can be drawn and hit-tested. It replaces the original's pairs of
// `Rectangle a1(160,300,100,50); a1.setFill(); a1.setColor(...)` + `Text text2(160,300,"LEVEL 1")` and the
// click handling that compared the raw click value against hand-computed numbers:
//     clickpos/65536 < 210 && clickpos/65536 > 110 && clickpos%65536 < 325 && clickpos%65536 > 275
// (the click's x and y were packed into one int, and the BACK handler even tested the PREVIOUS click `clickpos`
// instead of the new one `clickpos1`, so it reacted to the wrong click).
class Button
{
public:
    // The label is taken BY VALUE and moved into the member: see "Sink parameters" in LevelRepository.cpp.
    Button(const core::Rect& bounds, std::string label, float fontSize = 20.0f) : m_bounds(bounds), m_label(std::move(label)), m_fontSize(fontSize)
    {
    }

    [[nodiscard]] bool Contains(core::Vec2 point) const noexcept
    {
        return m_bounds.Contains(point);
    }

    [[nodiscard]] const core::Rect& Bounds() const noexcept
    {
        return m_bounds;
    }

    [[nodiscard]] const std::string& Label() const noexcept
    {
        return m_label;
    }

    void Render(IRenderer& renderer) const;

private:
    core::Rect m_bounds;
    std::string m_label;
    float m_fontSize;
};
} // namespace bb::ui
